//============================================================
//
//  winalloc.c - Win32 memory allocation routines
//
//============================================================
//
//  Copyright Aaron Giles
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the
//  following conditions are met:
//
//    * Redistributions of source code must retain the above
//      copyright notice, this list of conditions and the
//      following disclaimer.
//    * Redistributions in binary form must reproduce the
//      above copyright notice, this list of conditions and
//      the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//    * Neither the name 'MAME' nor the names of its
//      contributors may be used to endorse or promote
//      products derived from this software without specific
//      prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
//  EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGE (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
//  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

// MAME headers
#include "osdcore.h"

// undefine any redefines we have in the prefix
#undef malloc
#undef calloc
#undef realloc



//============================================================
//  CONSTANTS
//============================================================

#define OVERRIDE_STANDARD_CALLS		(0)

#define PAGE_SIZE		4096
#define COOKIE_VAL		0x11335577

// set this to 1 to align memory blocks to the start of a page;
// otherwise, they are aligned to the end, thus catching array
// overruns
#define ALIGN_START		0

// set this to 1 to record all mallocs and frees in the logfile
#define LOG_CALLS		0

#if LOG_CALLS
#define LOG(x) do { if (LOG_CALLS) logerror x; } while (0)
void CLIB_DECL logerror(const char *text,...);
#else
#define LOG(x)
#endif


//============================================================
//  TYPEDEFS
//============================================================

typedef struct _memory_entry memory_entry;
struct _memory_entry
{
	memory_entry *	next;
	memory_entry *	prev;
	size_t			size;
	void *			base;
	const char *	file;
	int				line;
	int				id;
};



//============================================================
//  GLOBAL VARIABLES
//============================================================

int winalloc_in_main_code = FALSE;



//============================================================
//  LOCAL VARIABLES
//============================================================

static memory_entry *alloc_list;
static memory_entry *free_list;
static int current_id;

static CRITICAL_SECTION memory_lock;

static UINT8 global_init_done = FALSE;
static UINT8 use_malloc_tracking = FALSE;



//============================================================
//  PROTOTYPES
//============================================================

static memory_entry *allocate_entry(void);
static memory_entry *find_entry(void *pointer);
static void free_entry(memory_entry *entry);

static void global_init(void);



//============================================================
//  INLINES
//============================================================

INLINE void global_init_if_not_done(void)
{
	if (!global_init_done)
	{
		global_init_done = TRUE;
		global_init();
	}
}


INLINE void memory_lock_acquire(void)
{
	EnterCriticalSection(&memory_lock);
}


INLINE void memory_lock_release(void)
{
	LeaveCriticalSection(&memory_lock);
}



//============================================================
//  IMPLEMENTATION
//============================================================

//============================================================
//  malloc_file_line - debugging version of malloc which
//  accepts filename and line number
//============================================================

void *malloc_file_line(size_t size, const char *file, int line)
{
	UINT8 *block_base;
	int id = current_id++;

	// perform global intialization if not already done
	global_init_if_not_done();

	// only proceed if enabled
	if (use_malloc_tracking)
	{
		UINT8 *page_base;
		size_t rounded_size;
		memory_entry *entry;

		// round the size up to a page boundary
		rounded_size = ((size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;

		// reserve that much memory, plus two guard pages
		page_base = (UINT8 *)VirtualAlloc(NULL, rounded_size + 2 * PAGE_SIZE, MEM_RESERVE, PAGE_NOACCESS);
		if (page_base == NULL)
			return NULL;

		// now allow access to everything but the first and last pages
		page_base = (UINT8 *)VirtualAlloc(page_base + PAGE_SIZE, rounded_size, MEM_COMMIT, PAGE_READWRITE);
		if (page_base == NULL)
			return NULL;

		// work backwards from the page base to get to the block base
		if (ALIGN_START)
			block_base = page_base;
		else
			block_base = page_base + rounded_size - size;

		// fill in the entry
		entry = allocate_entry();
		entry->size = size;
		entry->base = block_base;
		entry->file = file;
		entry->line = line;
		entry->id = id;
	}
	else
	{
		block_base = (UINT8 *)GlobalAlloc(GMEM_FIXED, size);
	}

	// logging
	if (file != NULL)
		LOG(("malloc #%06d size = %d (%s:%d)\n", id, size, file, line));
	else
		LOG(("malloc #%06d size = %d\n", id, size));

	return block_base;
}


//============================================================
//  malloc - override for the malloc() function
//============================================================

#if OVERRIDE_STANDARD_CALLS
void *CLIB_DECL malloc(size_t size)
{
	return malloc_file_line(size, NULL, 0);
}
#endif


//============================================================
//  calloc_file_line - debugging version of calloc which
//  accepts filename and line number
//============================================================

void *calloc_file_line(size_t size, size_t count, const char *file, int line)
{
	// first allocate the memory
	void *memory = malloc_file_line(size * count, file, line);
	if (memory == NULL)
		return NULL;

	// then memset it
	memset(memory, 0, size * count);
	return memory;
}


//============================================================
//  calloc - override for the calloc() function
//============================================================

#if OVERRIDE_STANDARD_CALLS
void *CLIB_DECL calloc(size_t size, size_t count)
{
	return calloc_file_line(size, count, NULL, 0);
}
#endif


//============================================================
//  _calloc_crt - override for the _calloc_crt() function,
//  which is called by beginthreadex
//============================================================

#if OVERRIDE_STANDARD_CALLS
void *CLIB_DECL _calloc_crt(size_t size, size_t count)
{
	return calloc_file_line(size, count, NULL, 0);
}
#endif


//============================================================
//  realloc_file_line - debugging version of realloc which
//  accepts filename and line number
//============================================================

void *realloc_file_line(void *memory, size_t size, const char *file, int line)
{
	void *newmemory = NULL;

	// perform global intialization if not already done
	global_init_if_not_done();

	// only proceed if enabled
	if (use_malloc_tracking)
	{
		// if size is non-zero, we need to reallocate memory
		if (size != 0)
		{
			// allocate space for the new amount
			newmemory = malloc_file_line(size, file, line);
			if (newmemory == NULL)
				return NULL;

			// if we have an old pointer, copy it
			if (memory != NULL)
			{
				memory_entry *entry = find_entry(memory);
				if (entry == NULL)
				{
					if (winalloc_in_main_code)
					{
						fprintf(stderr, "Error: realloc a non-existant block (%s:%d)\n", file, line);
						osd_break_into_debugger("Error: realloc a non-existant block\n");
					}
				}
				else
					memcpy(newmemory, memory, (size < entry->size) ? size : entry->size);
			}
		}

		// if we have an original pointer, free it
		if (memory != NULL)
			free(memory);
	}
	else
	{
		if (memory != NULL)
			newmemory = (void *) GlobalReAlloc(memory, size, GMEM_MOVEABLE);
		else
			newmemory = (void *) GlobalAlloc(GMEM_FIXED, size);
	}

	return newmemory;
}


//============================================================
//  realloc - override for the realloc() function
//============================================================

#if OVERRIDE_STANDARD_CALLS
void *CLIB_DECL realloc(void *memory, size_t size)
{
	return realloc_file_line(memory, size, NULL, 0);
}
#endif


//============================================================
//  free_file_line - debugging version of free which
//  accepts filename and line number
//============================================================

void CLIB_DECL free_file_line(void *memory, const char *file, int line)
{
	memory_entry *entry;

	// allow NULL frees
	if (memory == NULL)
		return;

	// only proceed if enabled
	if (use_malloc_tracking)
	{
		// error if no entry found
		entry = find_entry(memory);
		if (entry == NULL)
		{
			if (winalloc_in_main_code)
			{
				fprintf(stderr, "Error: free a non-existant block\n");
				osd_break_into_debugger("Error: free a non-existant block");
			}
			return;
		}
		free_entry(entry);

		// free the memory
		VirtualFree((UINT8 *)memory - ((size_t)memory & (PAGE_SIZE-1)) - PAGE_SIZE, 0, MEM_RELEASE);

		LOG(("free #%06d size = %d\n", entry->id, entry->size));
	}
	else
	{
		GlobalFree(memory);
	}
}


//============================================================
//  free - override for the free() function
//============================================================

#if OVERRIDE_STANDARD_CALLS
void CLIB_DECL free(void *memory)
{
	free_file_line(memory, NULL, 0);
}
#endif


//============================================================
//  _msize - internal MSVC routine that returns the size of
//  a memory block
//============================================================

#if OVERRIDE_STANDARD_CALLS
size_t CLIB_DECL _msize(void *memory)
{
	size_t result;

	// only proceed if enabled
	if (use_malloc_tracking)
	{
		memory_entry *entry = find_entry(memory);
		if (entry == NULL)
		{
			if (winalloc_in_main_code)
			{
				fprintf(stderr, "Error: msize a non-existant block\n");
				osd_break_into_debugger("Error: msize a non-existant block");
			}
			return 0;
		}
		result = entry->size;
	}
	else
	{
		result = GlobalSize(memory);
	}
	return result;
}
#endif


//============================================================
//  check_unfreed_mem - called from the exit path of any
//  code that wants to check for unfreed memory
//============================================================

void check_unfreed_mem(void)
{
	memory_entry *entry;
	int total = 0;

	// only valid if we are tracking
	if (use_malloc_tracking)
	{
		memory_lock_acquire();

		// check for leaked memory
		for (entry = alloc_list; entry; entry = entry->next)
			if (entry->file != NULL)
			{
				if (total == 0)
					fprintf(stderr, "--- memory leak warning ---\n");
				total += entry->size;
				fprintf(stderr, "allocation #%06d, %d bytes (%s:%d)\n", entry->id, entry->size, entry->file, (int)entry->line);
			}

		memory_lock_release();

		if (total > 0)
			fprintf(stderr, "a total of %d bytes were not free()'d\n", total);
	}
}


//============================================================
//  allocate_entry - allocate a new entry and link it into
//  the list of allocated memory
//============================================================

static memory_entry *allocate_entry(void)
{
	memory_entry *entry;

	// always take the lock when allocating
	memory_lock_acquire();

	// if we're out of entries, allocate some more
	if (free_list == NULL)
	{
		int entries_per_page = PAGE_SIZE / sizeof(memory_entry);

		// allocate a new pages' worth of entry
		entry = (memory_entry *)VirtualAlloc(NULL, PAGE_SIZE, MEM_COMMIT, PAGE_READWRITE);
		if (entry == NULL)
		{
			memory_lock_release();
			fprintf(stderr, "Out of memory for malloc tracking!\n");
			exit(1);
		}

		// add all the entries to the list
		while (entries_per_page--)
		{
			entry->next = free_list;
			free_list = entry;
			entry++;
		}
	}

	// grab a free list entry
	entry = free_list;
	free_list = free_list->next;

	// add ourselves to the alloc list
	entry->next = alloc_list;
	if (entry->next)
		entry->next->prev = entry;
	entry->prev = NULL;
	alloc_list = entry;

	// release the lock when finished
	memory_lock_release();

	return entry;
}


//============================================================
//  find_entry - find a memory_object entry in the list that
//  contains the given pointer
//============================================================

static memory_entry *find_entry(void *pointer)
{
	memory_entry *entry;

	// scan the list looking for a matching base
	if (pointer)
	{
		memory_lock_acquire();

		for (entry = alloc_list; entry; entry = entry->next)
			if (entry->base == pointer)
				break;

		memory_lock_release();
		return entry;
	}
	return NULL;
}


//============================================================
//  free_entry - free a memory_entry object
//============================================================

static void free_entry(memory_entry *entry)
{
	memory_lock_acquire();

	// remove ourselves from the alloc list
	if (entry->prev)
		entry->prev->next = entry->next;
	else
		alloc_list = entry->next;
	if (entry->next)
		entry->next->prev = entry->prev;

	// add ourself to the free list
	entry->next = free_list;
	free_list = entry;

	memory_lock_release();
}


//============================================================
//  global_init - global initialization of memory variables
//============================================================

static void global_init(void)
{
	TCHAR *envstring;

	// create the memory lock
	InitializeCriticalSection(&memory_lock);

	// determine if we enabled by default
#ifdef MESS
	{
		extern BOOL win_is_gui_application(void);
		use_malloc_tracking = !win_is_gui_application();
	}
#elif defined(WINUI)
	use_malloc_tracking = FALSE;
#else
	use_malloc_tracking = TRUE;
#endif

	// now allow overrides by the environment
	envstring = _tgetenv(_T("OSDDEBUGMALLOC"));
	if (envstring != NULL)
		use_malloc_tracking = (_ttoi(envstring) != 0);

#ifdef PTR64
	// 64-bit builds also can allocate everything under 4GB, unless disabled
	envstring = _tgetenv(_T("OSDDEBUG4GB"));
	if (envstring == NULL || _ttoi(envstring) != 0)
	{
		INT8 allocshift;

		// loop from 256MB down to 4k (page size)
		for (allocshift = 8 + 20; allocshift >= 12; allocshift--)
		{
			// keep allocating address space at that size until we get something >4gb
			while ((UINT64)VirtualAlloc(NULL, (UINT64)1 << allocshift, MEM_RESERVE, PAGE_NOACCESS) < ((UINT64)1 << 32)) ;
		}

		// loop from 64k down
		for (allocshift = 6 + 10; allocshift >= 1; allocshift--)
		{
			// keep allocating memory until we get something >4gb
			while ((UINT64)GlobalAlloc(GMEM_FIXED, (UINT64)1 << allocshift) < ((UINT64)1 << 32)) ;
		}
	}
#endif
}
