// This file is a placeholder.

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#include <mach/mach.h>
#include <mach/mach_time.h>
#include <Carbon/Carbon.h>

// MAME headers
#include "osdlib.h"

//============================================================
//  osd_process_kill
//============================================================

void osd_process_kill(void)
{
    kill(getpid(), SIGKILL);
}

//============================================================
//  osd_num_processors
//============================================================

int osd_get_num_processors(void)
{
    int processors = 1;

    struct host_basic_info host_basic_info;
    unsigned int count;
    kern_return_t r;
    mach_port_t my_mach_host_self;

    count = HOST_BASIC_INFO_COUNT;
    my_mach_host_self = mach_host_self();
    if ( ( r = host_info(my_mach_host_self, HOST_BASIC_INFO, (host_info_t)(&host_basic_info), &count)) == KERN_SUCCESS )
    {
        processors = host_basic_info.avail_cpus;
    }
    mach_port_deallocate(mach_task_self(), my_mach_host_self);

    return processors;
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
