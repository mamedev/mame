
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
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

char *osd_getenv(const char *name)
{
    return getenv(name);
}

//============================================================
//  osd_setenv
//============================================================

int osd_setenv(const char *name, const char *value, int overwrite)
{
    return setenv(name, value, overwrite);
}

//============================================================
//  osd_num_processors
//============================================================

int osd_get_num_processors(void)
{
    int processors = 1;

#if defined(_SC_NPROCESSORS_ONLN)
    processors = sysconf(_SC_NPROCESSORS_ONLN);
#endif
    return processors;
}

//============================================================
//  osd_process_kill
//============================================================

void osd_process_kill(void)
{
    kill(getpid(), SIGKILL);
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

osd_ticks_t osd_ticks(void)
{
#ifdef SDLMAME_EMSCRIPTEN
        return (osd_ticks_t)(emscripten_get_now() * 1000.0);
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
    return (osd_ticks_t) 1000000;
}

//============================================================
//  osd_sleep
//============================================================

void osd_sleep(osd_ticks_t duration)
{
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
}
