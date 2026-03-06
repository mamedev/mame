// license:BSD-3-Clause
// copyright-holders:Couriersud
//============================================================
//
//  testkeys.cpp - A small utility to analyze SDL keycodes
//
//  SDLMAME by Olivier Galibert and R. Belmont
//  testkeys by couriersud
//
//============================================================

#ifdef SDLMAME_SDL3
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#endif

#include "osdcore.h"

#ifndef SDLMAME_SDL3
#include "SDL2/SDL.h"
#endif

#include <iostream>
#include <string>

//#include "unicode.h"

#ifdef SDLMAME_SDL3
#if defined(SDL_PLATFORM_WINDOWS)
#ifndef WINAPI
	#define WINAPI __stdcall
#endif

// TODO: Why is this is necessary here but not for MAME itself?
typedef struct HINSTANCE__ * HINSTANCE;
typedef char *LPSTR;
typedef wchar_t *PWSTR;

extern "C" {
	int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
	{
		(void)hInst;
		(void)hPrev;
		(void)szCmdLine;
		(void)sw;
		return SDL_RunApp(0, NULL, SDL_main, NULL);
	}
} /* extern "C" */
#endif
#endif
struct key_lookup_table { int code; const char *name; };

#define KE(x) { SDL_SCANCODE_##x, "SDL_SCANCODE_" #x },

static constexpr key_lookup_table sdl_lookup[] =
{
	KE(UNKNOWN)

	KE(A)
	KE(B)
	KE(C)
	KE(D)
	KE(E)
	KE(F)
	KE(G)
	KE(H)
	KE(I)
	KE(J)
	KE(K)
	KE(L)
	KE(M)
	KE(N)
	KE(O)
	KE(P)
	KE(Q)
	KE(R)
	KE(S)
	KE(T)
	KE(U)
	KE(V)
	KE(W)
	KE(X)
	KE(Y)
	KE(Z)

	KE(1)
	KE(2)
	KE(3)
	KE(4)
	KE(5)
	KE(6)
	KE(7)
	KE(8)
	KE(9)
	KE(0)

	KE(RETURN)
	KE(ESCAPE)
	KE(BACKSPACE)
	KE(TAB)
	KE(SPACE)

	KE(MINUS)
	KE(EQUALS)
	KE(LEFTBRACKET)
	KE(RIGHTBRACKET)
	KE(BACKSLASH)
	KE(NONUSHASH)
	KE(SEMICOLON)
	KE(APOSTROPHE)
	KE(GRAVE)
	KE(COMMA)
	KE(PERIOD)
	KE(SLASH)

	KE(CAPSLOCK)

	KE(F1)
	KE(F2)
	KE(F3)
	KE(F4)
	KE(F5)
	KE(F6)
	KE(F7)
	KE(F8)
	KE(F9)
	KE(F10)
	KE(F11)
	KE(F12)

	KE(PRINTSCREEN)
	KE(SCROLLLOCK)
	KE(PAUSE)
	KE(INSERT)
	KE(HOME)
	KE(PAGEUP)
	KE(DELETE)
	KE(END)
	KE(PAGEDOWN)
	KE(RIGHT)
	KE(LEFT)
	KE(DOWN)
	KE(UP)

	KE(NUMLOCKCLEAR)
	KE(KP_DIVIDE)
	KE(KP_MULTIPLY)
	KE(KP_MINUS)
	KE(KP_PLUS)
	KE(KP_ENTER)
	KE(KP_1)
	KE(KP_2)
	KE(KP_3)
	KE(KP_4)
	KE(KP_5)
	KE(KP_6)
	KE(KP_7)
	KE(KP_8)
	KE(KP_9)
	KE(KP_0)
	KE(KP_PERIOD)

	KE(NONUSBACKSLASH)
	KE(APPLICATION)
	KE(POWER)
	KE(KP_EQUALS)
	KE(F13)
	KE(F14)
	KE(F15)
	KE(F16)
	KE(F17)
	KE(F18)
	KE(F19)
	KE(F20)
	KE(F21)
	KE(F22)
	KE(F23)
	KE(F24)
	KE(EXECUTE)
	KE(HELP)
	KE(MENU)
	KE(SELECT)
	KE(STOP)
	KE(AGAIN)
	KE(UNDO)
	KE(CUT)
	KE(COPY)
	KE(PASTE)
	KE(FIND)
	KE(MUTE)
	KE(VOLUMEUP)
	KE(VOLUMEDOWN)
	KE(KP_COMMA)
	KE(KP_EQUALSAS400)

	KE(INTERNATIONAL1)
	KE(INTERNATIONAL2)
	KE(INTERNATIONAL3)
	KE(INTERNATIONAL4)
	KE(INTERNATIONAL5)
	KE(INTERNATIONAL6)
	KE(INTERNATIONAL7)
	KE(INTERNATIONAL8)
	KE(INTERNATIONAL9)
	KE(LANG1)
	KE(LANG2)
	KE(LANG3)
	KE(LANG4)
	KE(LANG5)
	KE(LANG6)
	KE(LANG7)
	KE(LANG8)
	KE(LANG9)

	KE(ALTERASE)
	KE(SYSREQ)
	KE(CANCEL)
	KE(CLEAR)
	KE(PRIOR)
	KE(RETURN2)
	KE(SEPARATOR)
	KE(OUT)
	KE(OPER)
	KE(CLEARAGAIN)
	KE(CRSEL)
	KE(EXSEL)

	KE(KP_00)
	KE(KP_000)
	KE(THOUSANDSSEPARATOR)
	KE(DECIMALSEPARATOR)
	KE(CURRENCYUNIT)
	KE(CURRENCYSUBUNIT)
	KE(KP_LEFTPAREN)
	KE(KP_RIGHTPAREN)
	KE(KP_LEFTBRACE)
	KE(KP_RIGHTBRACE)
	KE(KP_TAB)
	KE(KP_BACKSPACE)
	KE(KP_A)
	KE(KP_B)
	KE(KP_C)
	KE(KP_D)
	KE(KP_E)
	KE(KP_F)
	KE(KP_XOR)
	KE(KP_POWER)
	KE(KP_PERCENT)
	KE(KP_LESS)
	KE(KP_GREATER)
	KE(KP_AMPERSAND)
	KE(KP_DBLAMPERSAND)
	KE(KP_VERTICALBAR)
	KE(KP_DBLVERTICALBAR)
	KE(KP_COLON)
	KE(KP_HASH)
	KE(KP_SPACE)
	KE(KP_AT)
	KE(KP_EXCLAM)
	KE(KP_MEMSTORE)
	KE(KP_MEMRECALL)
	KE(KP_MEMCLEAR)
	KE(KP_MEMADD)
	KE(KP_MEMSUBTRACT)
	KE(KP_MEMMULTIPLY)
	KE(KP_MEMDIVIDE)
	KE(KP_PLUSMINUS)
	KE(KP_CLEAR)
	KE(KP_CLEARENTRY)
	KE(KP_BINARY)
	KE(KP_OCTAL)
	KE(KP_DECIMAL)
	KE(KP_HEXADECIMAL)

	KE(LCTRL)
	KE(LSHIFT)
	KE(LALT)
	KE(LGUI)
	KE(RCTRL)
	KE(RSHIFT)
	KE(RALT)
	KE(RGUI)

	KE(MODE)
#ifdef SDLMAME_SDL3
	KE(MEDIA_NEXT_TRACK)
	KE(MEDIA_PREVIOUS_TRACK)
	KE(MEDIA_STOP)
	KE(MEDIA_PLAY)
	KE(MUTE)
	KE(MEDIA_SELECT)
#else
	KE(AUDIONEXT)
	KE(AUDIOPREV)
	KE(AUDIOSTOP)
	KE(AUDIOPLAY)
	KE(AUDIOMUTE)
	KE(MEDIASELECT)
	KE(WWW)
	KE(MAIL)
	KE(CALCULATOR)
	KE(COMPUTER)
#endif
	KE(AC_SEARCH)
	KE(AC_HOME)
	KE(AC_BACK)
	KE(AC_FORWARD)
	KE(AC_STOP)
	KE(AC_REFRESH)
	KE(AC_BOOKMARKS)

#ifdef SDLMAME_SDL3
	KE(MEDIA_EJECT)
	KE(SLEEP)
#else

	KE(BRIGHTNESSUP)
	KE(DISPLAYSWITCH)
	KE(KBDILLUMTOGGLE)
	KE(KBDILLUMDOWN)
	KE(KBDILLUMUP)
	KE(EJECT)
	KE(SLEEP)

	KE(APP1)
	KE(APP2)
#endif
};

static char const *lookup_key_name(int kc)
{
	for (key_lookup_table const &k : sdl_lookup)
	{
		if (k.code == kc)
			return k.name;
	}
	return nullptr;
}

int main(int argc, char *argv[])
{
#ifdef SDLMAME_SDL3
	if (!SDL_Init(SDL_INIT_VIDEO))
#else
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
#endif
	{
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}
#ifdef SDLMAME_SDL3
	SDL_CreateWindow("Input Test", 100, 100, 0);
#else
	SDL_CreateWindow("Input Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 100, 100, 0);
#endif

	SDL_Event event;
	bool quit = false;
	std::string lasttext;
	while (SDL_PollEvent(&event) || !quit) {
		switch(event.type) {
#ifdef SDLMAME_SDL3
		case SDL_EVENT_QUIT:
#else
		case SDL_QUIT:
#endif
			quit = true;
			break;
#ifdef SDLMAME_SDL3
		case SDL_EVENT_KEY_DOWN:
			if (event.key.scancode == SDLK_ESCAPE) {
#else
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE) {
#endif
				quit = true;
			} else {
				std::cout
					<< "ITEM_ID_XY "
#ifdef SDLMAME_SDL3
					<< lookup_key_name(event.key.scancode)
#else
					<< lookup_key_name(event.key.keysym.scancode)
#endif
					<< ' '
					<< std::endl;
				lasttext.clear();
			}
			break;
#ifdef SDLMAME_SDL3
		case SDL_EVENT_KEY_UP:
#else
		case SDL_KEYUP:
#endif
			std::cout
				<< "ITEM_ID_XY "
#ifdef SDLMAME_SDL3
				<< lookup_key_name(event.key.scancode)
#else
				<< lookup_key_name(event.key.keysym.scancode)
#endif
				<< ' '
				<< lasttext
				<< std::endl;
			break;
#ifdef SDLMAME_SDL3
		case SDL_EVENT_TEXT_INPUT:
#else
		case SDL_TEXTINPUT:
#endif
			lasttext = event.text.text;
			break;
		}
		event.type = 0;
	}
	SDL_Quit();
	return(0);
}
