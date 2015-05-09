// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sdlos_*.c - OS specific low level code
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#ifdef SDLMAME_EMSCRIPTEN
#include <emscripten.h>
#endif

#include "sdlinc.h"

// MAME headers
#include "osdcore.h"




#if (SDLMAME_SDL2)

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
