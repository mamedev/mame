//============================================================
//
//  winmisc.c - Win32 OSD core miscellaneous functions
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

// MAMEOS headers
#include "winutf8.h"
#include "strconv.h"
#include "strconv.h"


//============================================================
//  MACROS
//============================================================

// presumed size of a page of memory
#define PAGE_SIZE           4096

// align allocations to start or end of the page?
#define GUARD_ALIGN_START   0



//============================================================
//  GLOBAL VARIABLES
//============================================================

void (*s_debugger_stack_crawler)() = NULL;



//============================================================
//  osd_malloc
//============================================================

void *osd_malloc(size_t size)
{
#ifndef MALLOC_DEBUG
	return HeapAlloc(GetProcessHeap(), 0, size);
#else
	// add in space for the size
	size += sizeof(size_t);

	// basic objects just come from the heap
	void *result = HeapAlloc(GetProcessHeap(), 0, size);

	// store the size and return and pointer to the data afterward
	*reinterpret_cast<size_t *>(result) = size;
	return reinterpret_cast<UINT8 *>(result) + sizeof(size_t);
#endif
}


//============================================================
//  osd_malloc_array
//============================================================

void *osd_malloc_array(size_t size)
{
#ifndef MALLOC_DEBUG
	return HeapAlloc(GetProcessHeap(), 0, size);
#else
	// add in space for the size
	size += sizeof(size_t);

	// round the size up to a page boundary
	size_t rounded_size = ((size + sizeof(void *) + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;

	// reserve that much memory, plus two guard pages
	void *page_base = VirtualAlloc(NULL, rounded_size + 2 * PAGE_SIZE, MEM_RESERVE, PAGE_NOACCESS);
	if (page_base == NULL)
		return NULL;

	// now allow access to everything but the first and last pages
	page_base = VirtualAlloc(reinterpret_cast<UINT8 *>(page_base) + PAGE_SIZE, rounded_size, MEM_COMMIT, PAGE_READWRITE);
	if (page_base == NULL)
		return NULL;

	// work backwards from the page base to get to the block base
	void *result = GUARD_ALIGN_START ? page_base : (reinterpret_cast<UINT8 *>(page_base) + rounded_size - size);

	// store the size at the start with a flag indicating it has a guard page
	*reinterpret_cast<size_t *>(result) = size | 0x80000000;
	return reinterpret_cast<UINT8 *>(result) + sizeof(size_t);
#endif
}


//============================================================
//  osd_free
//============================================================

void osd_free(void *ptr)
{
#ifndef MALLOC_DEBUG
	HeapFree(GetProcessHeap(), 0, ptr);
#else
	size_t size = reinterpret_cast<size_t *>(ptr)[-1];

	// if no guard page, just free the pointer
	if ((size & 0x80000000) == 0)
		HeapFree(GetProcessHeap(), 0, reinterpret_cast<UINT8 *>(ptr) - sizeof(size_t));

	// large items need more care
	else
	{
		ULONG_PTR page_base = (reinterpret_cast<ULONG_PTR>(ptr) - sizeof(size_t)) & ~(PAGE_SIZE - 1);
		VirtualFree(reinterpret_cast<void *>(page_base - PAGE_SIZE), 0, MEM_RELEASE);
	}
#endif
}


//============================================================
//  osd_alloc_executable
//============================================================

void *osd_alloc_executable(size_t size)
{
	return VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
}


//============================================================
//  osd_free_executable
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
	if (IsDebuggerPresent())
	{
		win_output_debug_string_utf8(message);
		DebugBreak();
	}
	else if (s_debugger_stack_crawler != NULL)
		(*s_debugger_stack_crawler)();
}
