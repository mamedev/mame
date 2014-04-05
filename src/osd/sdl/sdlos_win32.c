//============================================================
//
//  sdlos_*.c - OS specific low level code
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

// standard sdl header
#include "sdlinc.h"


#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// MAME headers
#include "osdcore.h"
#include "strconv.h"

//============================================================
//  PROTOTYPES
//============================================================

static osd_ticks_t init_cycle_counter(void);
static osd_ticks_t performance_cycle_counter(void);

//============================================================
//  STATIC VARIABLES
//============================================================

// global cycle_counter function and divider
static osd_ticks_t      (*cycle_counter)(void) = init_cycle_counter;
static osd_ticks_t      (*ticks_counter)(void) = init_cycle_counter;
static osd_ticks_t      ticks_per_second;

//============================================================
//  init_cycle_counter
//
//  to avoid total grossness, this function is split by subarch
//============================================================

static osd_ticks_t init_cycle_counter(void)
{
	osd_ticks_t start, end;
	osd_ticks_t a, b;
	int priority = GetThreadPriority(GetCurrentThread());
	LARGE_INTEGER frequency;

	if (QueryPerformanceFrequency( &frequency ))
	{
		// use performance counter if available as it is constant
		cycle_counter = performance_cycle_counter;
		ticks_counter = performance_cycle_counter;

		ticks_per_second = frequency.QuadPart;

		// return the current cycle count
		return (*cycle_counter)();
	}
	else
	{
		fprintf(stderr, "Error!  Unable to QueryPerformanceFrequency!\n");
		exit(-1);
	}

	// temporarily set our priority higher
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	// wait for an edge on the timeGetTime call
	a = SDL_GetTicks();
	do
	{
		b = SDL_GetTicks();
	} while (a == b);

	// get the starting cycle count
	start = (*cycle_counter)();

	// now wait for 1/4 second total
	do
	{
		a = SDL_GetTicks();
	} while (a - b < 250);

	// get the ending cycle count
	end = (*cycle_counter)();

	// compute ticks_per_sec
	ticks_per_second = (end - start) * 4;

	// restore our priority
	SetThreadPriority(GetCurrentThread(), priority);

	// return the current cycle count
	return (*cycle_counter)();
}

//============================================================
//  performance_cycle_counter
//============================================================

static osd_ticks_t performance_cycle_counter(void)
{
	LARGE_INTEGER performance_count;
	QueryPerformanceCounter(&performance_count);
	return (osd_ticks_t)performance_count.QuadPart;
}

//============================================================
//   osd_cycles
//============================================================

osd_ticks_t osd_ticks(void)
{
	return (*cycle_counter)();
}


//============================================================
//  osd_ticks_per_second
//============================================================

osd_ticks_t osd_ticks_per_second(void)
{
	if (ticks_per_second == 0)
	{
		return 1;   // this isn't correct, but it prevents the crash
	}
	return ticks_per_second;
}

//============================================================
//  osd_sleep
//============================================================

void osd_sleep(osd_ticks_t duration)
{
	UINT32 msec;

	// make sure we've computed ticks_per_second
	if (ticks_per_second == 0)
		(void)osd_ticks();

	// convert to milliseconds, rounding down
	msec = (UINT32)(duration * 1000 / ticks_per_second);

	// only sleep if at least 2 full milliseconds
	if (msec >= 2)
	{
		// take a couple of msecs off the top for good measure
		msec -= 2;
		Sleep(msec);
	}
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
//  get_clipboard_text_by_format
//============================================================

static char *get_clipboard_text_by_format(UINT format, char *(*convert)(LPCVOID data))
{
	char *result = NULL;
	HANDLE data_handle;
	LPVOID data;

	// check to see if this format is available
	if (IsClipboardFormatAvailable(format))
	{
		// open the clipboard
		if (OpenClipboard(NULL))
		{
			// try to access clipboard data
			data_handle = GetClipboardData(format);
			if (data_handle != NULL)
			{
				// lock the data
				data = GlobalLock(data_handle);
				if (data != NULL)
				{
					// invoke the convert
					result = (*convert)(data);

					// unlock the data
					GlobalUnlock(data_handle);
				}
			}

			// close out the clipboard
			CloseClipboard();
		}
	}
	return result;
}


//============================================================
//  convert_wide
//============================================================

static char *convert_wide(LPCVOID data)
{
	return utf8_from_wstring((LPCWSTR) data);
}



//============================================================
//  convert_ansi
//============================================================

static char *convert_ansi(LPCVOID data)
{
	return utf8_from_astring((LPCSTR) data);
}



//============================================================
//  osd_get_clipboard_text
//============================================================

char *osd_get_clipboard_text(void)
{
	char *result = NULL;

	// try to access unicode text
	if (result == NULL)
		result = get_clipboard_text_by_format(CF_UNICODETEXT, convert_wide);

	// try to access ANSI text
	if (result == NULL)
		result = get_clipboard_text_by_format(CF_TEXT, convert_ansi);

	return result;
}

//============================================================
//  astring_from_utf8
//============================================================

CHAR *astring_from_utf8(const char *utf8string)
{
	WCHAR *wstring;
	int char_count;
	CHAR *result;

	// convert MAME string (UTF-8) to UTF-16
	char_count = MultiByteToWideChar(CP_UTF8, 0, utf8string, -1, NULL, 0);
	wstring = (WCHAR *)alloca(char_count * sizeof(*wstring));
	MultiByteToWideChar(CP_UTF8, 0, utf8string, -1, wstring, char_count);

	// convert UTF-16 to "ANSI code page" string
	char_count = WideCharToMultiByte(CP_ACP, 0, wstring, -1, NULL, 0, NULL, NULL);
	result = (CHAR *)osd_malloc_array(char_count * sizeof(*result));
	if (result != NULL)
		WideCharToMultiByte(CP_ACP, 0, wstring, -1, result, char_count, NULL, NULL);

	return result;
}

//============================================================
//  wstring_from_utf8
//============================================================

WCHAR *wstring_from_utf8(const char *utf8string)
{
	int char_count;
	WCHAR *result;

	// convert MAME string (UTF-8) to UTF-16
	char_count = MultiByteToWideChar(CP_UTF8, 0, utf8string, -1, NULL, 0);
	result = (WCHAR *)osd_malloc_array(char_count * sizeof(*result));
	if (result != NULL)
		MultiByteToWideChar(CP_UTF8, 0, utf8string, -1, result, char_count);

	return result;
}

//============================================================
//  win_attributes_to_entry_type
//============================================================

static osd_dir_entry_type win_attributes_to_entry_type(DWORD attributes)
{
	if (attributes == 0xFFFFFFFF)
		return ENTTYPE_NONE;
	else if (attributes & FILE_ATTRIBUTE_DIRECTORY)
		return ENTTYPE_DIR;
	else
		return ENTTYPE_FILE;
}

//============================================================
//  osd_stat
//============================================================

osd_directory_entry *osd_stat(const char *path)
{
	osd_directory_entry *result = NULL;
	TCHAR *t_path;
	HANDLE find = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA find_data;

	// convert the path to TCHARs
	t_path = tstring_from_utf8(path);
	if (t_path == NULL)
		goto done;

	// attempt to find the first file
	find = FindFirstFile(t_path, &find_data);
	if (find == INVALID_HANDLE_VALUE)
		goto done;

	// create an osd_directory_entry; be sure to make sure that the caller can
	// free all resources by just freeing the resulting osd_directory_entry
	result = (osd_directory_entry *) osd_malloc_array(sizeof(*result) + strlen(path) + 1);
	if (!result)
		goto done;
	strcpy(((char *) result) + sizeof(*result), path);
	result->name = ((char *) result) + sizeof(*result);
	result->type = win_attributes_to_entry_type(find_data.dwFileAttributes);
	result->size = find_data.nFileSizeLow | ((UINT64) find_data.nFileSizeHigh << 32);

done:
	if (t_path)
		osd_free(t_path);
	return result;
}

//============================================================
//  osd_get_volume_name
//============================================================

const char *osd_get_volume_name(int idx)
{
	static char szBuffer[128];
	const char *p;

	GetLogicalDriveStringsA(ARRAY_LENGTH(szBuffer), szBuffer);

	p = szBuffer;
	while(idx--) {
		p += strlen(p) + 1;
		if (!*p) return NULL;
	}

	return p;
}

//============================================================
//  osd_get_slider_list
//============================================================

const void *osd_get_slider_list()
{
	return NULL;
}

//============================================================
//  win_error_to_mame_file_error
//============================================================

static file_error win_error_to_mame_file_error(DWORD error)
{
	file_error filerr;

	// convert a Windows error to a file_error
	switch (error)
	{
		case ERROR_SUCCESS:
			filerr = FILERR_NONE;
			break;

		case ERROR_OUTOFMEMORY:
			filerr = FILERR_OUT_OF_MEMORY;
			break;

		case ERROR_FILE_NOT_FOUND:
		case ERROR_PATH_NOT_FOUND:
			filerr = FILERR_NOT_FOUND;
			break;

		case ERROR_ACCESS_DENIED:
			filerr = FILERR_ACCESS_DENIED;
			break;

		case ERROR_SHARING_VIOLATION:
			filerr = FILERR_ALREADY_OPEN;
			break;

		default:
			filerr = FILERR_FAILURE;
			break;
	}
	return filerr;
}

//============================================================
//  osd_get_full_path
//============================================================

file_error osd_get_full_path(char **dst, const char *path)
{
	file_error err;
	TCHAR *t_path;
	TCHAR buffer[MAX_PATH];

	// convert the path to TCHARs
	t_path = tstring_from_utf8(path);
	if (t_path == NULL)
	{
		err = FILERR_OUT_OF_MEMORY;
		goto done;
	}

	// cannonicalize the path
	if (!GetFullPathName(t_path, ARRAY_LENGTH(buffer), buffer, NULL))
	{
		err = win_error_to_mame_file_error(GetLastError());
		goto done;
	}

	// convert the result back to UTF-8
	*dst = utf8_from_tstring(buffer);
	if (!*dst)
	{
		err = FILERR_OUT_OF_MEMORY;
		goto done;
	}

	err = FILERR_NONE;

done:
	if (t_path != NULL)
		osd_free(t_path);
	return err;
}
