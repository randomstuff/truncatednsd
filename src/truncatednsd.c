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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <getopt.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <grp.h>

#include <netinet/in.h>

// At most 512 bytes for DNS message over UDP as per RFC1035 4.2.1:
#define MAX_DNS_MESSAGE 512

static void change_credentials(void)
{
  if (config.chroot) {
    if (chroot(config.chroot) == -1) {
      perror("chroot");
      exit(1);
    }
  }
  if (config.groups_len != 0) {
    if (setgroups(config.groups_len, config.groups) == -1) {
      perror("setgroups");
      exit(1);
    }
  }
  if (config.options & TRUNCATEDNSD_SETGID) {
    if (setgid(config.gid) == -1) {
      perror("setgid");
      exit(1);
    }
  }
  if (config.options & TRUNCATEDNSD_SETUID) {
    if (setuid(config.uid) == -1) {
      perror("setuid");
      exit(1);
    }
  }
}

static int open_socket(void)
{
  int sock = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sock == -1) {
    perror("socket");
    exit(1);
  }
  struct sockaddr_in6 addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin6_family = AF_INET6;
  addr.sin6_port   = htons(config.port);
  if (bind(sock, (const struct sockaddr *) &addr, sizeof(addr)) == -1) {
    perror("bind");
    exit(1);
  }
  return sock;
}

static void serve(int sockin, int sockout)
{
  char buffer[MAX_DNS_MESSAGE];

  while(1) {
    struct sockaddr_in6 addr;
    socklen_t socklen = sizeof(addr);
    ssize_t size = recvfrom(sockin, buffer, sizeof(buffer), 0,
      (struct sockaddr *) &addr, &socklen);
    if (size == -1) {
      perror("recvfrom");
      exit(1);
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

    if (sendto(sockout, buffer, size, 0, (struct sockaddr *)
      &addr, socklen) == -1) {
      continue;
    }
  }
}

void run_inetd(void)
{
  change_credentials();
  enable_sandbox();
  serve(STDIN_FILENO, STDOUT_FILENO);
}

void run_standalone(void)
{
  int sock = open_socket();
  change_credentials();
  enable_sandbox();
  serve(sock, sock);
}

int main(int argc, char** argv)
{
  parse_arguments(argc, argv);
  switch (config.mode) {
  case TRUNCATEDNSD_MODE_DEFAULT:
  case TRUNCATEDNSD_MODE_STANDALONE:
    run_standalone();
    break;
  case TRUNCATEDNSD_MODE_INETD:
    run_inetd();
    break;
  }
  return 0;
}
