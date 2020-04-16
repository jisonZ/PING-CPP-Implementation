# PING-CPP-Implementation
This is a C++ implementation of a PING CLI program

To run this code in Makefile:
- Use `make all`

To run this code using g++ compiler:
- Use `sudo g++ -o PING PING.cpp`

The compiled CLI program take two arguments:
1. Target IP address or hostname
2. 'y' or 'n' for printing TTL statistic results

To run this program:
- Use `sudo ./ping.out <address> y/n`

Clean Compiled file:
- Use `make clean`

Example:
Input: `sudo ./ping.out www.google.com y`
Output: ```
Resolving DNS..

Trying to connect to 'www.google.com' IP: 172.217.0.4

Reverse Lookup domain: ord38s04-in-f4.1e100.net
Socket file descriptor 3 received

Socket set to TTL..
64 bytes from ord38s04-in-f4.1e100.net (h: www.google.com) (172.217.0.4) msg_seq=1 ttl=64 rtt = 8.243000 ms.
64 bytes from ord38s04-in-f4.1e100.net (h: www.google.com) (172.217.0.4) msg_seq=2 ttl=64 rtt = 8.018000 ms.
64 bytes from ord38s04-in-f4.1e100.net (h: www.google.com) (172.217.0.4) msg_seq=3 ttl=64 rtt = 7.954000 ms.
64 bytes from ord38s04-in-f4.1e100.net (h: www.google.com) (172.217.0.4) msg_seq=4 ttl=64 rtt = 7.732000 ms.
64 bytes from ord38s04-in-f4.1e100.net (h: www.google.com) (172.217.0.4) msg_seq=5 ttl=64 rtt = 7.634000 ms.
64 bytes from ord38s04-in-f4.1e100.net (h: www.google.com) (172.217.0.4) msg_seq=6 ttl=64 rtt = 8.203000 ms.
^C64 bytes from ord38s04-in-f4.1e100.net (h: www.google.com) (172.217.0.4) msg_seq=7 ttl=64 rtt = 8.631000 ms.

===172.217.0.4 ping statistics===

7 packets sent, 7 packets received, 0.000000 percent packet loss. Total time: 6947.254000 ms.
round-trip min/avg/max/stddev = 7.634000 / 8.631000 / 8.059286 / 0.000000 ms.
```
