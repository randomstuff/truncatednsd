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

#ifndef TRUNCATEDNSD_H_
#define TRUNCATEDNSD_H_

#include <stdint.h>

#include <sys/types.h>

#define TRUNCATEDNSD_MODE_DEFAULT 0
#define TRUNCATEDNSD_MODE_STANDALONE 1
#define TRUNCATEDNSD_MODE_INETD 2

#define TRUNCATEDNSD_SETUID 1
#define TRUNCATEDNSD_SETGID 2
#define TRUNCATEDNSD_SANDBOX 4

struct truncatedns_config {
  int mode;
  int options;
  uint16_t port;
  uid_t uid;
  gid_t gid;
  gid_t* groups;
  size_t groups_len;
};

extern struct truncatedns_config config;

void parse_arguments(int argc, char** argv);

void enable_sandbox(void);

#endif
