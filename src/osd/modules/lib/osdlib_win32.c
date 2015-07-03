// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sdlos_*.c - OS specific low level code
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include <stdlib.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif

// MAME headers
#include "osdlib.h"
#include "osdcomm.h"
#include "osdcore.h"

#ifdef OSD_WINDOWS
#include "winutf8.h"
#endif

//============================================================
//  MACROS
//============================================================

// presumed size of a page of memory
#define PAGE_SIZE           4096

// align allocations to start or end of the page?
#define GUARD_ALIGN_START   0

#if defined(__BIGGEST_ALIGNMENT__)
#define MAX_ALIGNMENT       __BIGGEST_ALIGNMENT__
#elif defined(__AVX__)
#define MAX_ALIGNMENT       32
#elif defined(__SSE__) || defined(__x86_64__) || defined(_M_X64)
#define MAX_ALIGNMENT       16
#else
#define MAX_ALIGNMENT       sizeof(INT64)
#endif


//============================================================
//  GLOBAL VARIABLES
//============================================================

#ifdef OSD_WINDOWS
void (*s_debugger_stack_crawler)() = NULL;
#endif


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
}

//============================================================
//  osd_process_kill
//============================================================

void osd_process_kill(void)
{
	TerminateProcess(GetCurrentProcess(), -1);
}

//============================================================
//  osd_num_processors
//============================================================

int osd_get_num_processors(void)
{
	SYSTEM_INFO info;

	// otherwise, fetch the info from the system
	GetSystemInfo(&info);

	// max out at 4 for now since scaling above that seems to do poorly
	return MIN(info.dwNumberOfProcessors, 4);
}

//============================================================
//  osd_malloc
//============================================================

void *osd_malloc(size_t size)
{
#ifndef MALLOC_DEBUG
	return malloc(size);
#else
	// add in space for the size and offset
	size += MAX_ALIGNMENT + sizeof(size_t) + 2;
	size &= ~size_t(1);

	// basic objects just come from the heap
	UINT8 *const block = reinterpret_cast<UINT8 *>(HeapAlloc(GetProcessHeap(), 0, size));
	if (block == NULL)
		return NULL;
	UINT8 *const result = reinterpret_cast<UINT8 *>(reinterpret_cast<FPTR>(block + sizeof(size_t) + MAX_ALIGNMENT) & ~(FPTR(MAX_ALIGNMENT) - 1));

	// store the size and return and pointer to the data afterward
	*reinterpret_cast<size_t *>(block) = size;
	*(result - 1) = result - block;
	return result;
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
	// add in space for the size and offset
	size += MAX_ALIGNMENT + sizeof(size_t) + 2;
	size &= ~size_t(1);

	// round the size up to a page boundary
	size_t const rounded_size = ((size + sizeof(void *) + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;

	// reserve that much memory, plus two guard pages
	void *page_base = VirtualAlloc(NULL, rounded_size + 2 * PAGE_SIZE, MEM_RESERVE, PAGE_NOACCESS);
	if (page_base == NULL)
		return NULL;

	// now allow access to everything but the first and last pages
	page_base = VirtualAlloc(reinterpret_cast<UINT8 *>(page_base) + PAGE_SIZE, rounded_size, MEM_COMMIT, PAGE_READWRITE);
	if (page_base == NULL)
		return NULL;

	// work backwards from the page base to get to the block base
	UINT8 *const block = GUARD_ALIGN_START ? reinterpret_cast<UINT8 *>(page_base) : (reinterpret_cast<UINT8 *>(page_base) + rounded_size - size);
	UINT8 *const result = reinterpret_cast<UINT8 *>(reinterpret_cast<FPTR>(block + sizeof(size_t) + MAX_ALIGNMENT) & ~(FPTR(MAX_ALIGNMENT) - 1));

	// store the size at the start with a flag indicating it has a guard page
	*reinterpret_cast<size_t *>(block) = size | 1;
	*(result - 1) = result - block;
	return result;
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
	UINT8 const offset = *(reinterpret_cast<UINT8 *>(ptr) - 1);
	UINT8 *const block = reinterpret_cast<UINT8 *>(ptr) - offset;
	size_t const size = *reinterpret_cast<size_t *>(block);

	if ((size & 0x1) == 0)
	{
		// if no guard page, just free the pointer
		HeapFree(GetProcessHeap(), 0, block);
	}
	else
	{
		// large items need more care
		ULONG_PTR const page_base = reinterpret_cast<ULONG_PTR>(block) & ~(PAGE_SIZE - 1);
		VirtualFree(reinterpret_cast<void *>(page_base - PAGE_SIZE), 0, MEM_RELEASE);
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
	return VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
}


//============================================================
//  osd_free_executable
//
//  frees memory allocated with osd_alloc_executable
//============================================================

void osd_free_executable(void *ptr, size_t size)
{
	VirtualFree(ptr, 0, MEM_RELEASE);
}


//============================================================
//  osd_break_into_debugger
//============================================================

void osd_break_into_debugger(const char *message)
{
#ifdef OSD_WINDOWS
	if (IsDebuggerPresent())
	{
		win_output_debug_string_utf8(message);
		DebugBreak();
	}
	else if (s_debugger_stack_crawler != NULL)
		(*s_debugger_stack_crawler)();
#else
	if (IsDebuggerPresent())
	{
		OutputDebugStringA(message);
		DebugBreak();
	}
#endif
}

//============================================================
//  GLOBAL VARIABLES
//============================================================

static osd_ticks_t ticks_per_second = 0;
static osd_ticks_t suspend_ticks = 0;
static BOOL using_qpc = TRUE;



//============================================================
//  osd_ticks
//============================================================

osd_ticks_t osd_ticks(void)
{
	LARGE_INTEGER performance_count;

	// if we're suspended, just return that
	if (suspend_ticks != 0)
		return suspend_ticks;

	// if we have a per second count, just go for it
	if (ticks_per_second != 0)
	{
		// QueryPerformanceCounter if we can
		if (using_qpc)
		{
			QueryPerformanceCounter(&performance_count);
			return (osd_ticks_t)performance_count.QuadPart - suspend_ticks;
		}

		// otherwise, fall back to timeGetTime
		else
			return (osd_ticks_t)timeGetTime() - suspend_ticks;
	}

	// if not, we have to determine it
	using_qpc = QueryPerformanceFrequency(&performance_count) && (performance_count.QuadPart != 0);
	if (using_qpc)
		ticks_per_second = (osd_ticks_t)performance_count.QuadPart;
	else
		ticks_per_second = 1000;

	// call ourselves to get the first value
	return osd_ticks();
}


//============================================================
//  osd_ticks_per_second
//============================================================

osd_ticks_t osd_ticks_per_second(void)
{
	if (ticks_per_second == 0)
		osd_ticks();
	return ticks_per_second;
}

//============================================================
//  osd_sleep
//============================================================

void osd_sleep(osd_ticks_t duration)
{
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
}
