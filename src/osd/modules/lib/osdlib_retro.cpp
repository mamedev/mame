
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
   buf = (char *) malloc(strlen(name)+strlen(value)+2);
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

//============================================================
//  osd_get_clipboard_text
//============================================================

char *osd_get_clipboard_text(void)
{
	return NULL;
}
