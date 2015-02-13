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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <getopt.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define DNS_PORT 53

// At most 512 bytes for DNS message over UDP as per RFC1035 4.2.1:
#define MAX_DNS_MESSAGE 512

static void help(void)
{
  const char* help_lines[] = {
    "truncatednsd - Dummy UDP DNS server which always send truncated answers\n",
    "\n",
    "  -h | --help     Show some help\n"
  };
  int i;
  for (i=0; i<sizeof(help_lines)/sizeof(char*); ++i)
    fputs(help_lines[i], stderr);
}

static void parse_arguments(int argc, char** argv)
{
  static const struct option long_options[] = {
    {"help", 0, NULL, 'h'},
    {0,      0, 0,    0  }
  };

  while (1) {
    int option_index;
    int c = getopt_long(argc, argv, "h", long_options, &option_index);
    if (c == -1)
      break;
    switch (c) {
    case 0:
      switch(option_index) {
      default:
        help();
        exit(1);
        break;
      }
      break;
    case 'h':
      help();
      exit(0);
      break;
    default:
      help();
      exit(1);
      break;
    }
  }
  if (optind < argc) {
    help();
    exit(1);
  }
}

int main(int argc, char** argv)
{
  parse_arguments(argc, argv);

  int sock = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sock == -1) {
    perror("socket");
    return 1;
  }
  struct sockaddr_in6 addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin6_family = AF_INET6;
  addr.sin6_port   = htons(DNS_PORT);
  if (bind(sock, (const struct sockaddr *) &addr, sizeof(addr)) == -1) {
    perror("bind");
    return 1;
  }

  char buffer[MAX_DNS_MESSAGE];

  while(1) {
    socklen_t socklen = sizeof(addr);
    ssize_t size = recvfrom(sock, buffer, sizeof(buffer), 0,
      (struct sockaddr *) &addr, &socklen);
    if (size == -1) {
      perror("recvfrom");
      return 1;
    }
    if (socklen != sizeof(addr)) {
      continue;
    }
    if (size < 12) {
      continue;
    }
    if (buffer[3] & 1) {
      continue;
    }

    // Set QR=response, TC=1:
    buffer[2] |= 128 | 2;
    // Set RA=1, Z=0, RCODE=0:
    buffer[3] = 128;
    // Set ANCOUNT=1:
    uint16_t ancount = htons(1);
    memcpy(buffer + 4, &ancount, sizeof(ancount));

    if (sendto(sock, buffer, size, 0, (struct sockaddr *)
        &addr, socklen) == -1) {
      continue;
    }
  }

  return 0;
}
