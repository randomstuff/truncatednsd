# TruncateDNSd

A DNS server which replies to all DNS queries which the truncate bit (TC=1).
When seeing this, a DNS client should fall back to TCP mode. This might be used
when the local DNS server is only handling TCP (for example because it is
implemented as a `stunnel`).

## Context

On systems using the GNU libc, the `use-vs` `resolv.conf` option can be used
to force the usage of TCP.

This can be done either in `resolv.conf`:

~~~
options use-vc
~~~

or as an environment variable:

~~~
RES_OPTIONS=use-vc
export RES_OPTIONS
~~~

A similar option is handled in different libraries:

  * the GNU libc (`use-vc)`;

  * the OpenBSD libc (`tcp`).

It is not implemented in:

  * the FreeBSD libc;

  * the DragonFlyBSD libc;

  * the MacOS X / Darwvin libresolv;

  * the bionic libc (used by Android).

When such as option is not available (because the suitable libc is not
available; because the program is not using the system-wide resolver such as
`dig`, `lwresd`; because the program overrides the system resolver
configuration), truncatednsd can be used: when receiving a TC=1 reply the DNS
client should fall back to TCP.

## ROADMAP

  * argument parsing;

  * setuid/setgid;

  * daemon mode;

  * inetd mode;

  * systemd mode;

  * bind to one inet address;

  * bind to several inet addresses.
