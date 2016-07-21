#if defined(__GLIBC__)
#	include_next <signal.h>
#else
#	include <sys/signal.h>
#endif
