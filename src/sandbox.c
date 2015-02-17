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

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef USE_SANDBOX

#include <sys/prctl.h>
#include <linux/seccomp.h>
#include <linux/filter.h>
#include <linux/audit.h>
#include <sys/syscall.h>

#define ALLOW_SYSCALL(syscall) \
  BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, __NR_##syscall, 0, 1), \
  BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_ALLOW)

#endif

void enable_sandbox(void)
{
  if (!(config.options & TRUNCATEDNSD_SANDBOX))
    return;
#ifndef USE_SANDBOX

  fprintf(stderr, "Sandbox support not enabled\n");
  exit(1);

#else

  fprintf(stderr, "Enabling sandbox\n");

  struct sock_filter filters[] = {
    // Checking the architecture is useless as we don't allow exec and friends.
    BPF_STMT(BPF_LD+BPF_W+BPF_ABS, offsetof(struct seccomp_data, nr)),
    // Currently only implemented for architecture without SYS_socketcall:
    ALLOW_SYSCALL(sendto),
    ALLOW_SYSCALL(recvfrom),
    ALLOW_SYSCALL(exit),
    BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_KILL)
  };

  struct sock_fprog prog
    = { sizeof(filters) / sizeof(struct sock_filter), filters };

  if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1) {
    perror("PR_SET_NO_NEW_PRIVS");
    exit(1);
  }
  if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog) == -1) {
    perror("PR_SET_SECCOMP");
    exit(1);
  }

#endif
}
