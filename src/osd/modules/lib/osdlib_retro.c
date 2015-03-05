
#include <stdlib.h>
#include <unistd.h>
#ifndef _WIN32
#include <sys/mman.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#endif
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#ifdef SDLMAME_EMSCRIPTEN
#include <emscripten.h>
#endif

// MAME headers
#include "osdcore.h"
#include "osdlib.h"

//============================================================
//  osd_getenv
//============================================================

const char *osd_getenv(const char *name)
{
    return getenv(name);
}

//============================================================
//  osd_setenv
//============================================================

int osd_setenv(const char *name, const char *value, int overwrite)
{
#if defined(_WIN32)
   char *buf;
   int result;

   if (!overwrite)
   {
      if (osd_getenv(name) != NULL)
         return 0;
   }
   buf = (char *) osd_malloc_array(strlen(name)+strlen(value)+2);
   sprintf(buf, "%s=%s", name, value);
   result = putenv(buf);

   /* will be referenced by environment
    * Therefore it is not freed here
    */

   return result;
#else
   return setenv(name, value, overwrite);
#endif
}

//============================================================
//  osd_num_processors
//============================================================

int osd_get_num_processors(void)
{
#ifdef _WIN32
	SYSTEM_INFO info;

	// otherwise, fetch the info from the system
	GetSystemInfo(&info);

	// max out at 4 for now since scaling above that seems to do poorly
	return MIN(info.dwNumberOfProcessors, 4);
#else
	int processors = 1;

#if defined(_SC_NPROCESSORS_ONLN)
	processors = sysconf(_SC_NPROCESSORS_ONLN);
#endif
	return processors;
	
#endif
}

//============================================================
//  osd_process_kill
//============================================================

void osd_process_kill(void)
{
#ifndef _WIN32
    kill(getpid(), SIGKILL);
#else
    TerminateProcess(GetCurrentProcess(), -1);
#endif
}

//============================================================
//  osd_malloc
//============================================================

void *osd_malloc(size_t size)
{
#ifndef MALLOC_DEBUG
    return malloc(size);
#else
#error "MALLOC_DEBUG not yet supported"
#endif
}


//============================================================
//  osd_malloc_array
//============================================================

void *osd_malloc_array(size_t size)
{
#ifndef MALLOC_DEBUG
    return malloc(size);
#else
#error "MALLOC_DEBUG not yet supported"
#endif
}


//============================================================
//  osd_free
//============================================================

void osd_free(void *ptr)
{
#ifndef MALLOC_DEBUG
    free(ptr);
#else
#error "MALLOC_DEBUG not yet supported"
#endif
}

//============================================================
//   osd_cycles
//============================================================
#ifdef _WIN32
static osd_ticks_t ticks_per_second = 0;
static osd_ticks_t suspend_ticks = 0;
#endif

osd_ticks_t osd_ticks(void)
{
#ifdef _WIN32
	LARGE_INTEGER performance_count;

	// if we're suspended, just return that
	if (suspend_ticks != 0)
		return suspend_ticks;

	// if we have a per second count, just go for it
	if (ticks_per_second != 0)
   {
      QueryPerformanceCounter(&performance_count);
      return (osd_ticks_t)performance_count.QuadPart - suspend_ticks;
   }

	// if not, we have to determine it
	QueryPerformanceFrequency(&performance_count) && (performance_count.QuadPart != 0);
   ticks_per_second = (osd_ticks_t)performance_count.QuadPart;

	// call ourselves to get the first value
	return osd_ticks();
#else
   struct timeval    tp;
   static osd_ticks_t start_sec = 0;

   gettimeofday(&tp, NULL);
   if (start_sec==0)
      start_sec = tp.tv_sec;
   return (tp.tv_sec - start_sec) * (osd_ticks_t) 1000000 + tp.tv_usec;
#endif
}

osd_ticks_t osd_ticks_per_second(void)
{
#ifdef _WIN32
	if (ticks_per_second == 0)
		osd_ticks();
	return ticks_per_second;
#else
	return (osd_ticks_t) 1000000;
#endif
}
//============================================================
//  osd_sleep
//============================================================
void osd_sleep(osd_ticks_t duration)
{
#ifdef _WIN32
	DWORD msec;

	// make sure we've computed ticks_per_second
	if (ticks_per_second == 0)
		(void)osd_ticks();

	// convert to milliseconds, rounding down
	msec = (DWORD)(duration * 1000 / ticks_per_second);

	// only sleep if at least 2 full milliseconds
	if (msec >= 2)
	{
		HANDLE current_thread = GetCurrentThread();
		int old_priority = GetThreadPriority(current_thread);

		// take a couple of msecs off the top for good measure
		msec -= 2;

		// bump our thread priority super high so that we get
		// priority when we need it
		SetThreadPriority(current_thread, THREAD_PRIORITY_TIME_CRITICAL);
		Sleep(msec);
		SetThreadPriority(current_thread, old_priority);
	}
#else
	UINT32 msec;

	// convert to milliseconds, rounding down
	msec = (UINT32)(duration * 1000 / osd_ticks_per_second());

	// only sleep if at least 2 full milliseconds
	if (msec >= 2)
	{
		// take a couple of msecs off the top for good measure
		msec -= 2;
		usleep(msec*1000);
	}
#endif
}

//============================================================
//  osd_alloc_executable
//
//  allocates "size" bytes of executable memory.  this must take
//  things like NX support into account.
//============================================================

void *osd_alloc_executable(size_t size)
{
#if defined(_WIN32)
   return VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
#else
	return (void *)mmap(0, size, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);
#endif
}

//============================================================
//  osd_free_executable
//
//  frees memory allocated with osd_alloc_executable
//============================================================

void osd_free_executable(void *ptr, size_t size)
{
#if defined(_WIN32)
   VirtualFree(ptr, 0, MEM_RELEASE);
#else
	munmap(ptr, size);
#endif
}

//============================================================
//  osd_break_into_debugger
//============================================================

void osd_break_into_debugger(const char *message)
{
	#ifdef MAME_DEBUG
	printf("MAME exception: %s\n", message);
	printf("Attempting to fall into debugger\n");
	kill(getpid(), SIGTRAP);
	#else
	printf("Ignoring MAME exception: %s\n", message);
	#endif
}
