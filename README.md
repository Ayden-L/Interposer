# Linux Syscall Interposer for File Access Control

<h2>Description</h2>
This project implements a Linux system call interposer to monitor and control file access at the kernel boundary. By hooking into open(), read(), write(), and other key syscalls, this tool enforces a fine grained file access policies for user space applciations, providing an additional layer of security for sensitive systems. 

Designed as a preloadable shared object using LD_PRELOAD, this project showcases practical use of dynamic linking, ptrace-style syscall interception, and access control enforcement without modifying kernel code.
<br />

<h2> Technologies Used </h2>

- Linux (tested on Ubuntu 20.04+)
- GCC
- No root access needed

<h2> Key Features </h2>

- Intercepts file related syscalls like open(), fopen(), read(), write()
- Demonstrates user-space sandboxing principles
- Optional config file to define allowed/blocked paths

<h2> Implementation Details </h2>

* Implemented urntime blocking of file access (open) and hiding of directory entries (readdir) based on colon-delimited BLOCKED and HIDDEN environment variables
* Leveraged dlsym(RTLD_NEXT, ...) to call the original libc functions while injecting custom logic before execution
* Applied low level C, pointer manipulation, and variadic arguments (va_list) to ensure compatibility with native POSIX interfaces
* Simulates rootkit-style behavior for educaitonal purposes, demonstrating core concepts in Linux shared libraries, runtime hooking, and basic access control mechanisms

<h2> Security Use Case </h2>
This project is relevant for:

* Security Testing: Catch unexpected file access in test environments
* Audit Trails: Monitor which files are being accessed
* Software Sandboxing: Restrict programs from touching critical files
* Honeypots: Detect malware or rogue processes by observing bait file access

<h2> Future Feature Enhancements </h2>

* Add runtime logging to syslog
* Implement per-user or per-process access policies

<h2> Usage </h2>
(Work In Progress)
