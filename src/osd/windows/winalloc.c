//============================================================
//
//  winalloc.c - Win32 memory allocation routines
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#define _WIN32_WINNT 0x0400

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// MAME headers
#include "osdcore.h"

// undefine any redefines we have in the prefix
#undef malloc
#undef calloc
#undef realloc



//============================================================
//  CONSTANTS
//============================================================

#define PAGE_SIZE		4096
#define COOKIE_VAL		0x11335577

// set this to 0 to not use guard pages
#define USE_GUARD_PAGES	1

// set this to 1 to align memory blocks to the start of a page;
// otherwise, they are aligned to the end, thus catching array
// overruns
#define ALIGN_START		0

// set this to 1 to record all mallocs and frees in the logfile
#define LOG_CALLS		0



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
static UINT8 memory_lock_initialized = FALSE;



//============================================================
//  INLINES
//============================================================

INLINE void memory_lock_acquire(void)
{
	if (!memory_lock_initialized)
	{
		memory_lock_initialized = TRUE;
		InitializeCriticalSection(&memory_lock);
	}
	EnterCriticalSection(&memory_lock);
}


INLINE void memory_lock_release(void)
{
	LeaveCriticalSection(&memory_lock);
}


INLINE memory_entry *allocate_entry(void)
{
	memory_entry *entry;

	memory_lock_acquire();

	// if we're out of entries, allocate some more
	if (free_list == NULL)
	{
		int entries_per_page = PAGE_SIZE / sizeof(memory_entry);

		// allocate a new pages' worth of entry
		entry = (memory_entry *)VirtualAlloc(NULL, PAGE_SIZE, MEM_COMMIT, PAGE_READWRITE);
		if (entry == NULL)
		{
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

	memory_lock_release();

	return entry;
}


INLINE memory_entry *find_entry(void *pointer)
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


INLINE void free_entry(memory_entry *entry)
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
//  IMPLEMENTATION
//============================================================

void *malloc_file_line(size_t size, const char *file, int line)
{
	UINT8 *page_base, *block_base;
	size_t rounded_size;
	memory_entry *entry;

	if (USE_GUARD_PAGES)
	{
		// round the size up to a page boundary
		rounded_size = ((size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;

		// reserve that much memory, plus two guard pages
		page_base = VirtualAlloc(NULL, rounded_size + 2 * PAGE_SIZE, MEM_RESERVE, PAGE_NOACCESS);
		if (page_base == NULL)
			return NULL;

		// now allow access to everything but the first and last pages
		page_base = VirtualAlloc(page_base + PAGE_SIZE, rounded_size, MEM_COMMIT, PAGE_READWRITE);
		if (page_base == NULL)
			return NULL;

		// work backwards from the page base to get to the block base
		if (ALIGN_START)
			block_base = page_base;
		else
			block_base = page_base + rounded_size - size;
	}
	else
	{
		block_base = (UINT8 *)GlobalAlloc(GMEM_FIXED, size);
	}

	// fill in the entry
	entry = allocate_entry();
	entry->size = size;
	entry->base = block_base;
	entry->file = file;
	entry->line = line;
	entry->id = current_id++;

//if (entry->size == 72 && IsDebuggerPresent()) DebugBreak();

#if LOG_CALLS
	// logging
	if (entry->file)
		logerror("malloc #%06d size = %d (%s:%d)\n", entry->size, entry->file, entry->line);
	else
		logerror("malloc #%06d size = %d\n", entry->id, entry->size);
#endif

	return block_base;
}


void *CLIB_DECL malloc(size_t size)
{
	return malloc_file_line(size, NULL, 0);
}


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


void *CLIB_DECL calloc(size_t size, size_t count)
{
	return calloc_file_line(size, count, NULL, 0);
}


// this function is called by beginthreadex
void *CLIB_DECL _calloc_crt(size_t size, size_t count)
{
	return calloc_file_line(size, count, NULL, 0);
}


void *realloc_file_line(void *memory, size_t size, const char *file, int line)
{
	void *newmemory = NULL;

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

	return newmemory;
}


void *CLIB_DECL realloc(void *memory, size_t size)
{
	return realloc_file_line(memory, size, NULL, 0);
}


void CLIB_DECL free(void *memory)
{
	memory_entry *entry;

	// allow NULL frees
	if (memory == NULL)
		return;

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
	if (USE_GUARD_PAGES)
		VirtualFree((UINT8 *)memory - ((size_t)memory & (PAGE_SIZE-1)) - PAGE_SIZE, 0, MEM_RELEASE);
	else
		GlobalFree(memory);

#if LOG_CALLS
	logerror("free #%06d size = %d\n", entry->id, entry->size);
#endif
}


size_t CLIB_DECL _msize(void *memory)
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
	return entry->size;
}


void check_unfreed_mem(void)
{
	memory_entry *entry;
	int total = 0;

	memory_lock_acquire();

	// check for leaked memory
	for (entry = alloc_list; entry; entry = entry->next)
		if (entry->file != NULL)
		{
			if (total == 0)
				fprintf(stderr, "--- memory leak warning ---\n");
			total += entry->size;
			fprintf(stderr, "allocation #%06d, %d bytes (%s:%d)\n", entry->id, entry->size, entry->file, entry->line);
		}

	memory_lock_release();

	if (total > 0)
		fprintf(stderr, "a total of %d bytes were not free()'d\n", total);
}
