// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
#ifndef _sdlinc_h_
#define _sdlinc_h_

#include <SDL2/SDL.h>
#include <SDL2/SDL_version.h>
// on win32 this includes windows.h by itself and breaks us!
#ifndef SDLMAME_WIN32
#include <SDL2/SDL_syswm.h>
#endif

#endif
