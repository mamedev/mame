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

#ifdef SDLMAME_ANDROID
char *osd_get_clipboard_text(void)
{
	return nullptr;
}
#else
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

#endif
