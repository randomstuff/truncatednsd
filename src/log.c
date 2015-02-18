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

#include <stdarg.h>
#include <stdlib.h>

#include <syslog.h>

static char* simple_log_prefixes[8] =  {
  "ERMERG:", "ALERT:", "CRIT:", "ERR:",
  "WARNING:", "NOTICE:", "INFO:", "DEBUG:"
};

static char* log_format(const char *format, va_list ap)
{
  if (config.log_buffer_size == 0) {
    config.log_buffer = malloc(256 * sizeof(char));
    config.log_buffer_size = 256;
  }
  while (1) {
    int res = vsnprintf(config.log_buffer, config.log_buffer_size, format, ap);
    if (res == -1)
      return NULL;
    if (res >= config.log_buffer_size) {
      config.log_buffer_size = res;
      config.log_buffer = realloc(config.log_buffer,
        config.log_buffer_size * sizeof(char));
    } else {
      return config.log_buffer;
    }
  }
}

void log_message(int priority, const char *format, ...)
{
  if (priority > config.log_level)
    return;
  if (priority < 0)
    priority = 0;

  va_list ap;
  va_start(ap, format);

  switch(config.log_mode) {

  case TRUNCATEDNSD_LOG_STDIO:
    {
      char* buffer = log_format(format, ap);
      if (buffer == NULL)
        goto err;
      if (fprintf(config.log_file, "%s%s\n",
        simple_log_prefixes[priority], buffer) < 0)
        goto err;
    }
    break;

  case TRUNCATEDNSD_LOG_SYSLOG:
    vsyslog(priority, format, ap);
    break;

  default:
    fprintf(stderr, "Unexpected logging mode\n");
    exit(1);
  }

  va_end(ap);
  return;

err:
  log_message(LOG_CRIT, "Could not log message");
}
