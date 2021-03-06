/* The MIT License (MIT)

Copyright (c) 2015 Gabriel Corona

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "truncatednsd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <syslog.h>
#include <pwd.h>
#include <grp.h>

struct truncatedns_config config;

static void unknown_argument(void)
{
  log_message(LOG_ERR, "Unrecognised argument");
  exit(1);
}

static void help(void)
{
  const char* help_lines[] = {
    "truncatednsd - Dummy UDP DNS server which always send truncated answers\n",
    "\n",
    "  -h | --help     Show some help\n",
    "  --su foo        Change user\n",
    "  --standalone    Standalone mode\n",
    "  --inetd         Inetd mode\n",
    "  --port 9000     Choose the UDP port\n",
    "  --sandbox       Enable sandbox\n",
    "  --log stderr    Log to stderr\n",
    "  --log syslog    Log using syslog\n",
    "  --loglevel 5    Choose log verbosity (using a syslog priotity)\n",
  };
  int i;
  for (i=0; i < sizeof(help_lines)/sizeof(char*); ++i)
    fputs(help_lines[i], stderr);
}

static void config_su(char* login)
{
  struct passwd* user = getpwnam(login);
  if (!user) {
    log_message(LOG_ERR, "Unknown user name: %s", login);
    exit(1);
  }
  config.options |= TRUNCATEDNSD_SETUID | TRUNCATEDNSD_SETGID;
  config.uid = user->pw_uid;
  config.gid = user->pw_gid;

  // Scan groups:
  config.groups = NULL;
  config.groups_len = 0;
  setgrent();
  while(1) {
    struct group* group = getgrent();
    if (group == NULL)
      break;
    size_t i;
    for (i = 0; group->gr_mem[i] != NULL; ++i) {
      if (strcmp(group->gr_mem[i], optarg) == 0) {
        config.groups = realloc(config.groups,
          (config.groups_len+1) * sizeof(gid_t));
        config.groups[config.groups_len] = group->gr_gid;
        config.groups_len++;
        break;
      }
    }
  }
  endgrent();
}

static void config_mode(int mode)
{
  if (config.mode && config.mode != mode) {
    log_message(LOG_ERR, "Inconsistent mode option");
    exit(1);
  }
  config.mode = mode;
}

static void config_log(const char* log_mode)
{
  if (strcmp(log_mode, "stderr") == 0) {
    config.log_mode = TRUNCATEDNSD_LOG_STDIO;
    config.log_file = stderr;
  } else if (strcmp(log_mode, "syslog") == 0) {
    openlog(NULL, 0, LOG_DAEMON);
    config.log_mode = TRUNCATEDNSD_LOG_SYSLOG;
    config.log_file = NULL;
  } else {
    log_message(LOG_ERR, "Unknown log mode: %s", log_mode);
    exit(1);
  }
}

static void config_loglevel(const char* level)
{
  config.log_level = atoi(level);
}

static void config_long_option(const char* option, const char* arg)
{
  if (strcmp(option, "su") == 0) {
    config_su(optarg);
  } else if (strcmp(option, "standalone") == 0) {
    config_mode(TRUNCATEDNSD_MODE_STANDALONE);
  } else if (strcmp(option, "inetd") == 0) {
    config_mode(TRUNCATEDNSD_MODE_INETD);
  } else if (strcmp(option, "port") == 0) {
    config.port = atoll(optarg);
  } else if (strcmp(option, "sandbox") == 0) {
    config.options |= TRUNCATEDNSD_SANDBOX;
  } else if (strcmp(option, "chroot") == 0) {
    free(config.chroot);
    config.chroot = strdup(optarg);
  } else if (strcmp(option, "log") == 0) {
    config_log(optarg);
  } else if (strcmp(option, "loglevel") == 0) {
    config_loglevel(optarg);
  } else {
    unknown_argument();
  }
}

void parse_arguments(int argc, char** argv)
{
  config.log_mode = TRUNCATEDNSD_LOG_STDIO;
  config.log_file = stderr;
  config.log_level = LOG_NOTICE;

  static const struct option long_options[] = {
    {"help",       0, NULL, 'h'},
    {"su",         1, NULL,  0},
    {"standalone", 0, NULL,  0},
    {"inetd",      0, NULL,  0},
    {"port",       1, NULL,  0},
    {"sandbox",    0, NULL,  0},
    {"chroot",     1, NULL,  0},
    {"log",        1, NULL,  0},
    {"loglevel",   1, NULL,  0},
    {0,            0, 0,     0}
  };

  config.port = 53;

  while (1) {
    int option_index;
    int c = getopt_long(argc, argv, "hu:g:", long_options, &option_index);
    if (c == -1)
      break;
    switch (c) {
    case 0:
      {
        const char* option = long_options[option_index].name;
        config_long_option(option, optarg);
        break;
      }
      break;

    case 'h':
      help();
      exit(0);
      break;

    default:
      unknown_argument();
      break;
    }
  }
  if (optind < argc) {
    unknown_argument();
  }
}
