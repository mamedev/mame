
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <signal.h>

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
