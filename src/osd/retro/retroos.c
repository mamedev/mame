#include <stdlib.h>
#include <unistd.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/time.h>
#endif

#include <time.h>
#include <sys/stat.h>

// MAME headers
#include "osdcore.h"

#ifdef WIN32
static osd_ticks_t ticks_per_second = 0;
static osd_ticks_t suspend_ticks = 0;
#endif

//============================================================
//   osd_cycles
//============================================================

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
#ifdef WIN32
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
#ifdef WIN32
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
//  osd_num_processors
//============================================================

int osd_get_num_processors(void)
{
#ifdef WIN32

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
#if defined(WIN32)
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

#if defined (SDLMAME_SDL2)

//============================================================
//  osd_get_clipboard_text
//============================================================

char *osd_get_clipboard_text(void)
{
	char *result = NULL;

	if (SDL_HasClipboardText())
	{
		char *temp = SDL_GetClipboardText();
		result = (char *) osd_malloc_array(strlen(temp) + 1);
		strcpy(result, temp);
		SDL_free(temp);
	}
	return result;
}

#elif defined(SDL_VIDEO_DRIVER_X11) && defined(SDLMAME_X11)

//============================================================
//  osd_get_clipboard_text
//============================================================

char *osd_get_clipboard_text(void)
{
	SDL_SysWMinfo info;
	Display* display;
	Window our_win;
	Window selection_win;
	Atom data_type;
	int data_format;
	unsigned long nitems;
	unsigned long bytes_remaining;
	unsigned char* prop;
	char* result;
	XEvent event;
	Uint32 t0, t1;
	Atom types[2];
	int i;

	/* get & validate SDL sys-wm info */
	SDL_VERSION(&info.version);
	if ( ! SDL_GetWMInfo( &info ) )
		return NULL;
	if ( info.subsystem != SDL_SYSWM_X11 )
		return NULL;
	if ( (display = info.info.x11.display) == NULL )
		return NULL;
	if ( (our_win = info.info.x11.window) == None )
		return NULL;

	/* request data to owner */
	selection_win = XGetSelectionOwner( display, XA_PRIMARY );
	if ( selection_win == None )
		return NULL;

	/* first, try UTF-8, then latin-1 */
	types[0] = XInternAtom( display, "UTF8_STRING", False );
	types[1] = XA_STRING; /* latin-1 */

	for ( i = 0; i < ARRAY_LENGTH(types); i++ )
	{
		XConvertSelection( display, XA_PRIMARY, types[i], types[i], our_win, CurrentTime );

		/* wait for SelectionNotify, but no more than 100 ms */
		t0 = t1 = SDL_GetTicks();
		while ( 1 )
		{
			if (  XCheckTypedWindowEvent( display, our_win,  SelectionNotify, &event ) ) break;
			SDL_Delay( 1 );
			t1 = SDL_GetTicks();
			if ( t1 - t0 > 100 )
				return NULL;
		}
		if ( event.xselection.property == None )
			continue;

		/* get property & check its type */
		if ( XGetWindowProperty( display, our_win, types[i], 0, 65536, False, types[i],
						&data_type, &data_format, &nitems, &bytes_remaining, &prop )
				!= Success )
			continue;
		if ( ! prop )
			continue;
		if ( (data_format != 8) || (data_type != types[i]) )
		{
			XFree( prop );
			continue;
		}

		/* return a copy & free original */
		if (prop != NULL)
		{
			result = (char *) osd_malloc_array(strlen((char *)prop)+1);
			strcpy(result, (char *)prop);
		}
		else
			result = NULL;
		XFree( prop );
		return result;
	}

	return NULL;
}

#else
//============================================================
//  osd_get_clipboard_text
//============================================================

char *osd_get_clipboard_text(void)
{
	char *result = NULL;

	return result;
}
#endif

//============================================================
//  osd_stat
//============================================================

osd_directory_entry *osd_stat(const char *path)
{
	int err;
	osd_directory_entry *result = NULL;
	#if defined(SDLMAME_NO64BITIO) || defined(RETRO_AND) || defined(WIN32) || defined(SDLMAME_BSD)
	struct stat st;
	#else
	struct stat64 st;
	#endif

	#if defined(SDLMAME_NO64BITIO) || defined(RETRO_AND) || defined(WIN32) || defined(SDLMAME_BSD)
	err = stat(path, &st);
	#else
	err = stat64(path, &st);
	#endif

	if( err == -1) return NULL;

	// create an osd_directory_entry; be sure to make sure that the caller can
	// free all resources by just freeing the resulting osd_directory_entry
	result = (osd_directory_entry *) osd_malloc_array(sizeof(*result) + strlen(path) + 1);
	strcpy(((char *) result) + sizeof(*result), path);
	result->name = ((char *) result) + sizeof(*result);
	result->type = S_ISDIR(st.st_mode) ? ENTTYPE_DIR : ENTTYPE_FILE;
	result->size = (UINT64)st.st_size;

	return result;
}

//============================================================
//  osd_get_volume_name
//============================================================

const char *osd_get_volume_name(int idx)
{
	if (idx!=0) return NULL;
	return "/";
}

//============================================================
//  osd_get_slider_list
//============================================================

const void *osd_get_slider_list()
{
	return NULL;
}

//============================================================
//  osd_get_full_path
//============================================================

file_error osd_get_full_path(char **dst, const char *path)
{
	file_error err;
	char path_buffer[512];

	err = FILERR_NONE;

	if (getcwd(path_buffer, 511) == NULL)
	{
		printf("osd_get_full_path: failed!\n");
		err = FILERR_FAILURE;
	}
	else
	{
		*dst = (char *)osd_malloc_array(strlen(path_buffer)+strlen(path)+3);

		// if it's already a full path, just pass it through
		if (path[0] == '/')
			strcpy(*dst, path);
		else
			sprintf(*dst, "%s%s%s", path_buffer, PATH_SEPARATOR, path);
	}

	return err;
}
