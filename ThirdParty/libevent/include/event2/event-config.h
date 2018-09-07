/* event-config.h
 *
 * This file was generated by cmake when the makefiles were generated.
 *
 * DO NOT EDIT THIS FILE.
 *
 * Do not rely on macros in this file existing in later versions.
 */
#ifndef EVENT2_EVENT_CONFIG_H_INCLUDED_
#define EVENT2_EVENT_CONFIG_H_INCLUDED_

/* Numeric representation of the version */
#define EVENT__NUMERIC_VERSION 0x02010500

#define EVENT__PACKAGE_VERSION 2.1.5

/* Version number of package */
#define EVENT__VERSION "2.1.5-alpha-beta"

/* Name of package */
#define EVENT__PACKAGE "libevent"

/* Define to the address where bug reports for this package should be sent. */
#define EVENT__PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define EVENT__PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define EVENT__PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define EVENT__PACKAGE_TARNAME ""

/* Define if libevent should build without support for a debug mode */
/* #undef EVENT__DISABLE_DEBUG_MODE */

/* Define if libevent should not allow replacing the mm functions */
/* #undef EVENT__DISABLE_MM_REPLACEMENT */

/* Define if libevent should not be compiled with thread support */
/* #undef EVENT__DISABLE_THREAD_SUPPORT */

/* Define to 1 if you have the `accept4' function. */
/* #undef EVENT__HAVE_ACCEPT4 */

/* Define to 1 if you have the `arc4random' function. */
/* #undef EVENT__HAVE_ARC4RANDOM */

/* Define to 1 if you have the `arc4random_buf' function. */
/* #undef EVENT__HAVE_ARC4RANDOM_BUF */

/* Define if clock_gettime is available in libc */
/* #undef EVENT__DNS_USE_CPU_CLOCK_FOR_ID */

/* Define is no secure id variant is available */
/* #undef EVENT__DNS_USE_GETTIMEOFDAY_FOR_ID */
#define EVENT__DNS_USE_FTIME_FOR_ID 1

/* Define to 1 if you have the <arpa/inet.h> header file. */
/* #undef EVENT__HAVE_ARPA_INET_H */

/* Define to 1 if you have the `clock_gettime' function. */
/* #undef EVENT__HAVE_CLOCK_GETTIME */

/* Define to 1 if you have the declaration of `CTL_KERN'. */
/* #undef EVENT__HAVE_DECL_CTL_KERN */

/* Define to 1 if you have the declaration of `KERN_ARND'. */
/* #undef EVENT__HAVE_DECL_KERN_ARND */

/* Define to 1 if you have the declaration of `KERN_RANDOM'. */
/* #undef EVENT__HAVE_DECL_KERN_RANDOM */

/* Define if /dev/poll is available */
/* #undef EVENT__HAVE_DEVPOLL */

/* Define to 1 if you have the <netdb.h> header file. */
/* #undef EVENT__HAVE_NETDB_H */

/* Define to 1 if fd_mask type is defined */
/* #undef EVENT__HAVE_FD_MASK */

/* Define to 1 if the <sys/queue.h> header file defines TAILQ_FOREACH. */
/* #undef EVENT__HAVE_TAILQFOREACH */

/* Define to 1 if you have the <dlfcn.h> header file. */
/* #undef EVENT__HAVE_DLFCN_H */

/* Define if your system supports the epoll system calls */
/* #undef EVENT__HAVE_EPOLL */

/* Define to 1 if you have the `epoll_create1' function. */
/* #undef EVENT__HAVE_EPOLL_CREATE1 */

/* Define to 1 if you have the `epoll_ctl' function. */
/* #undef EVENT__HAVE_EPOLL_CTL */

/* Define to 1 if you have the `eventfd' function. */
/* #undef EVENT__HAVE_EVENTFD */

/* Define if your system supports event ports */
/* #undef EVENT__HAVE_EVENT_PORTS */

/* Define to 1 if you have the `fcntl' function. */
/* #undef EVENT__HAVE_FCNTL */

/* Define to 1 if you have the <fcntl.h> header file. */
#define EVENT__HAVE_FCNTL_H 1

/* Define to 1 if you have the `getaddrinfo' function. */
#define EVENT__HAVE_GETADDRINFO 1

/* Define to 1 if you have the `getegid' function. */
/* #undef EVENT__HAVE_GETEGID */

/* Define to 1 if you have the `geteuid' function. */
/* #undef EVENT__HAVE_GETEUID */

/* TODO: Check for different gethostname argument counts. CheckPrototypeDefinition.cmake can be used. */
/* Define this if you have any gethostbyname_r() */
/* #undef EVENT__HAVE_GETHOSTBYNAME_R */

/* Define this if gethostbyname_r takes 3 arguments */
/* #undef EVENT__HAVE_GETHOSTBYNAME_R_3_ARG */

/* Define this if gethostbyname_r takes 5 arguments */
/* #undef EVENT__HAVE_GETHOSTBYNAME_R_5_ARG */

/* Define this if gethostbyname_r takes 6 arguments */
/* #undef EVENT__HAVE_GETHOSTBYNAME_R_6_ARG */

/* Define to 1 if you have the `getifaddrs' function. */
/* #undef EVENT__HAVE_GETIFADDRS */

/* Define to 1 if you have the `getnameinfo' function. */
#define EVENT__HAVE_GETNAMEINFO 1

/* Define to 1 if you have the `getprotobynumber' function. */
#define EVENT__HAVE_GETPROTOBYNUMBER 1

/* Define to 1 if you have the `getservbyname' function. */
#define EVENT__HAVE_GETSERVBYNAME 1

/* Define to 1 if you have the `gettimeofday' function. */
/* #undef EVENT__HAVE_GETTIMEOFDAY */

/* Define to 1 if you have the <ifaddrs.h> header file. */
/* #undef EVENT__HAVE_IFADDRS_H */

/* Define to 1 if you have the `inet_ntop' function. */
/* #undef EVENT__HAVE_INET_NTOP */

/* Define to 1 if you have the `inet_pton' function. */
/* #undef EVENT__HAVE_INET_PTON */

/* Define to 1 if you have the <inttypes.h> header file. */
#define EVENT__HAVE_INTTYPES_H 1

/* Define to 1 if you have the `issetugid' function. */
/* #undef EVENT__HAVE_ISSETUGID */

/* Define to 1 if you have the `kqueue' function. */
/* #undef EVENT__HAVE_KQUEUE */

/* Define if the system has zlib */
/* #undef EVENT__HAVE_LIBZ */

/* Define to 1 if you have the `mach_absolute_time' function. */
/* #undef EVENT__HAVE_MACH_ABSOLUTE_TIME */

/* Define to 1 if you have the <mach/mach_time.h> header file. */
/* #undef EVENT__HAVE_MACH_MACH_TIME_H */

/* Define to 1 if you have the <memory.h> header file. */
#define EVENT__HAVE_MEMORY_H 1

/* Define to 1 if you have the `mmap' function. */
/* #undef EVENT__HAVE_MMAP */

/* Define to 1 if you have the `nanosleep' function. */
/* #undef EVENT__HAVE_NANOSLEEP */

/* Define to 1 if you have the `usleep' function. */
/* #undef EVENT__HAVE_USLEEP */

/* Define to 1 if you have the <netdb.h> header file. */
/* #undef EVENT__HAVE_NETDB_H */

/* Define to 1 if you have the <netinet/in6.h> header file. */
/* #undef EVENT__HAVE_NETINET_IN6_H */

/* Define to 1 if you have the <netinet/in.h> header file. */
/* #undef EVENT__HAVE_NETINET_IN_H */

/* Define to 1 if you have the <netinet/tcp.h> header file. */
/* #undef EVENT__HAVE_NETINET_TCP_H */

/* Define if the system has openssl */
#define EVENT__HAVE_OPENSSL 1

/* Defines if the system has zlib */
/* #undef EVENT__HAVE_ZLIB */

/* Define to 1 if you have the `pipe' function. */
/* #undef EVENT__HAVE_PIPE */

/* Define to 1 if you have the `pipe2' function. */
/* #undef EVENT__HAVE_PIPE2 */

/* Define to 1 if you have the `poll' function. */
/* #undef EVENT__HAVE_POLL */

/* Define to 1 if you have the <poll.h> header file. */
/* #undef EVENT__HAVE_POLL_H */

/* Define to 1 if you have the `port_create' function. */
/* #undef EVENT__HAVE_PORT_CREATE */

/* Define to 1 if you have the <port.h> header file. */
/* #undef EVENT__HAVE_PORT_H */

/* Define if you have POSIX threads libraries and header files. */
/* #undef EVENT__HAVE_PTHREAD */

/* Define if we have pthreads on this system */
/* #undef EVENT__HAVE_PTHREADS */

/* Define to 1 if you have the `putenv' function. */
#define EVENT__HAVE_PUTENV 1

/* Define to 1 if the system has the type `sa_family_t'. */
/* #undef EVENT__HAVE_SA_FAMILY_T */

/* Define to 1 if you have the `select' function. */
/* #undef EVENT__HAVE_SELECT */

/* Define to 1 if you have the `setenv' function. */
/* #undef EVENT__HAVE_SETENV */

/* Define if F_SETFD is defined in <fcntl.h> */
/* #undef EVENT__HAVE_SETFD */

/* Define to 1 if you have the `setrlimit' function. */
/* #undef EVENT__HAVE_SETRLIMIT */

/* Define to 1 if you have the `sendfile' function. */
/* #undef EVENT__HAVE_SENDFILE */

/* Define if F_SETFD is defined in <fcntl.h> */
/* #undef EVENT__HAVE_SETFD */

/* Define to 1 if you have the `sigaction' function. */
/* #undef EVENT__HAVE_SIGACTION */

/* Define to 1 if you have the `signal' function. */
/* #undef EVENT__HAVE_SIGNAL */

/* Define to 1 if you have the `splice' function. */
/* #undef EVENT__HAVE_SPLICE */

/* Define to 1 if you have the <stdarg.h> header file. */
#define EVENT__HAVE_STDARG_H 1

/* Define to 1 if you have the <stddef.h> header file. */
#define EVENT__HAVE_STDDEF_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define EVENT__HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define EVENT__HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
/* #undef EVENT__HAVE_STRINGS_H */

/* Define to 1 if you have the <string.h> header file. */
#define EVENT__HAVE_STRING_H 1

/* Define to 1 if you have the `strlcpy' function. */
/* #undef EVENT__HAVE_STRLCPY */

/* Define to 1 if you have the `strsep' function. */
/* #undef EVENT__HAVE_STRSEP */

/* Define to 1 if you have the `strtok_r' function. */
/* #undef EVENT__HAVE_STRTOK_R */

/* Define to 1 if you have the `strtoll' function. */
#define EVENT__HAVE_STRTOLL 1

/* Define to 1 if the system has the type `struct addrinfo'. */
#define EVENT__HAVE_STRUCT_ADDRINFO 1

/* Define to 1 if the system has the type `struct in6_addr'. */
#define EVENT__HAVE_STRUCT_IN6_ADDR 1

/* Define to 1 if `s6_addr16' is member of `struct in6_addr'. */
/* #undef EVENT__HAVE_STRUCT_IN6_ADDR_S6_ADDR16 */

/* Define to 1 if `s6_addr32' is member of `struct in6_addr'. */
/* #undef EVENT__HAVE_STRUCT_IN6_ADDR_S6_ADDR32 */

/* Define to 1 if the system has the type `struct sockaddr_in6'. */
#define EVENT__HAVE_STRUCT_SOCKADDR_IN6 1

/* Define to 1 if `sin6_len' is member of `struct sockaddr_in6'. */
/* #undef EVENT__HAVE_STRUCT_SOCKADDR_IN6_SIN6_LEN */

/* Define to 1 if `sin_len' is member of `struct sockaddr_in'. */
/* #undef EVENT__HAVE_STRUCT_SOCKADDR_IN_SIN_LEN */

/* Define to 1 if the system has the type `struct sockaddr_storage'. */
#define EVENT__HAVE_STRUCT_SOCKADDR_STORAGE 1

/* Define to 1 if `ss_family' is a member of `struct sockaddr_storage'. */
#define EVENT__HAVE_STRUCT_SOCKADDR_STORAGE_SS_FAMILY 1

/* Define to 1 if `__ss_family' is a member of `struct sockaddr_storage'. */
/* #undef EVENT__HAVE_STRUCT_SOCKADDR_STORAGE___SS_FAMILY */

/* Define to 1 if you have the `sysctl' function. */
/* #undef EVENT__HAVE_SYSCTL */

/* Define to 1 if you have the <sys/devpoll.h> header file. */
/* #undef EVENT__HAVE_SYS_DEVPOLL_H */

/* Define to 1 if you have the <sys/epoll.h> header file. */
/* #undef EVENT__HAVE_SYS_EPOLL_H */

/* Define to 1 if you have the <sys/eventfd.h> header file. */
/* #undef EVENT__HAVE_SYS_EVENTFD_H */

/* Define to 1 if you have the <sys/event.h> header file. */
/* #undef EVENT__HAVE_SYS_EVENT_H */

/* Define to 1 if you have the <sys/ioctl.h> header file. */
/* #undef EVENT__HAVE_SYS_IOCTL_H */

/* Define to 1 if you have the <sys/mman.h> header file. */
/* #undef EVENT__HAVE_SYS_MMAN_H */

/* Define to 1 if you have the <sys/param.h> header file. */
/* #undef EVENT__HAVE_SYS_PARAM_H */

/* Define to 1 if you have the <sys/queue.h> header file. */
/* #undef EVENT__HAVE_SYS_QUEUE_H */

/* Define to 1 if you have the <sys/resource.h> header file. */
/* #undef EVENT__HAVE_SYS_RESOURCE_H */

/* Define to 1 if you have the <sys/select.h> header file. */
/* #undef EVENT__HAVE_SYS_SELECT_H */

/* Define to 1 if you have the <sys/sendfile.h> header file. */
/* #undef EVENT__HAVE_SYS_SENDFILE_H */

/* Define to 1 if you have the <sys/socket.h> header file. */
/* #undef EVENT__HAVE_SYS_SOCKET_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define EVENT__HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/sysctl.h> header file. */
/* #undef EVENT__HAVE_SYS_SYSCTL_H */

/* Define to 1 if you have the <sys/timerfd.h> header file. */
/* #undef EVENT__HAVE_SYS_TIMERFD_H */

/* Define to 1 if you have the <sys/time.h> header file. */
/* #undef EVENT__HAVE_SYS_TIME_H */

/* Define to 1 if you have the <sys/types.h> header file. */
#define EVENT__HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/uio.h> header file. */
/* #undef EVENT__HAVE_SYS_UIO_H */

/* Define to 1 if you have the <sys/wait.h> header file. */
/* #undef EVENT__HAVE_SYS_WAIT_H */

/* Define if TAILQ_FOREACH is defined in <sys/queue.h> */
/* #undef EVENT__HAVE_TAILQFOREACH */

/* Define if timeradd is defined in <sys/time.h> */
/* #undef EVENT__HAVE_TIMERADD */

/* Define if timerclear is defined in <sys/time.h> */
/* #undef EVENT__HAVE_TIMERCLEAR */

/* Define if timercmp is defined in <sys/time.h> */
/* #undef EVENT__HAVE_TIMERCMP */

/* Define to 1 if you have the `timerfd_create' function. */
/* #undef EVENT__HAVE_TIMERFD_CREATE */

/* Define if timerisset is defined in <sys/time.h> */
/* #undef EVENT__HAVE_TIMERISSET */

/* Define to 1 if the system has the type `uint8_t'. */
#define EVENT__HAVE_UINT8_T 1

/* Define to 1 if the system has the type `uint16_t'. */
#define EVENT__HAVE_UINT16_T 1

/* Define to 1 if the system has the type `uint32_t'. */
#define EVENT__HAVE_UINT32_T 1

/* Define to 1 if the system has the type `uint64_t'. */
#define EVENT__HAVE_UINT64_T 1

/* Define to 1 if the system has the type `uintptr_t'. */
#define EVENT__HAVE_UINTPTR_T 1

/* Define to 1 if you have the `umask' function. */
/* #undef EVENT__HAVE_UMASK */

/* Define to 1 if you have the <unistd.h> header file. */
/* #undef EVENT__HAVE_UNISTD_H */

/* Define to 1 if you have the `unsetenv' function. */
/* #undef EVENT__HAVE_UNSETENV */

/* Define to 1 if you have the `vasprintf' function. */
/* #undef EVENT__HAVE_VASPRINTF */

/* Define if kqueue works correctly with pipes */
/* #undef EVENT__HAVE_WORKING_KQUEUE */

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
/* #undef EVENT__PTHREAD_CREATE_JOINABLE */

/* The size of `pthread_t', as computed by sizeof. */
/* #undef EVENT__SIZEOF_PTHREAD_T */

/* The size of a `int', as computed by sizeof. */
#define EVENT__SIZEOF_INT 4

/* The size of a `long', as computed by sizeof. */
#define EVENT__SIZEOF_LONG 4

/* The size of a `long long', as computed by sizeof. */
#define EVENT__SIZEOF_LONG_LONG 8

/* The size of `off_t', as computed by sizeof. */
#define EVENT__SIZEOF_OFF_T 4

/* The size of a `short', as computed by sizeof. */
#define EVENT__SIZEOF_SHORT 2

/* The size of `size_t', as computed by sizeof. */
#define EVENT__SIZEOF_SIZE_T 4

/* Define to 1 if you have the ANSI C header files. */
/* #undef EVENT__STDC_HEADERS */

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
/* #undef EVENT__TIME_WITH_SYS_TIME */

/* The size of `socklen_t', as computed by sizeof. */
#define EVENT__SIZEOF_SOCKLEN_T 4

/* The size of 'void *', as computer by sizeof */
#define EVENT__SIZEOF_VOID_P 4

/* Define to appropriate substitute if compiler doesnt have __func__ */
#define EVENT____func__ __FUNCTION__

/* Number of bits in a file offset, on hosts where this is settable. */
#define EVENT___FILE_OFFSET_BITS _FILE_OFFSET_BITS

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef EVENT__const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#define EVENT__inline __inline
#endif

/* Define to `int' if <sys/types.h> does not define. */
#define EVENT__pid_t int

/* Define to `unsigned' if <sys/types.h> does not define. */
/* #undef EVENT__size_t */

/* Define to unsigned int if you dont have it */
/* #undef EVENT__socklen_t */

/* Define to `int' if <sys/types.h> does not define. */
#define EVENT__ssize_t SSIZE_T

/* #undef EVENT__NEED_DLLIMPORT */

/* Define to 1 if you have ERR_remove_thread_stat(). */
/* #undef EVENT__HAVE_ERR_REMOVE_THREAD_STATE */

#endif
