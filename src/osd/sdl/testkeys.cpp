// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  testkey.c - A small utility to analyze SDL keycodes
//
//  SDLMAME by Olivier Galibert and R. Belmont
//  testkeys by couriersud
//
//============================================================

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <wchar.h>

#include "sdlinc.h"

#include "unicode.h"

struct key_lookup_table
{
	int code;
	const char *name;
};

#define KE(x) { SDL_SCANCODE_ ## x, "SDL_SCANCODE_" #x },
#define KE8(A, B, C, D, E, F, G, H) KE(A) KE(B) KE(C) KE(D) KE(E) KE(F) KE(G) KE(H)
#define KE7(A, B, C, D, E, F, G) KE(A) KE(B) KE(C) KE(D) KE(E) KE(F) KE(G)
#define KE5(A, B, C, D, E) KE(A) KE(B) KE(C) KE(D) KE(E)
#define KE3(A, B, C) KE(A) KE(B) KE(C)


static key_lookup_table sdl_lookup[] =
{
	KE7(UNKNOWN,    BACKSPACE,  TAB,            CLEAR,      RETURN,     PAUSE,      ESCAPE      )
	KE(SPACE)
	KE5(COMMA,      MINUS,      PERIOD,         SLASH,      0           )
	KE8(1,          2,          3,              4,          5,          6,          7,          8           )
	KE5(9,          SEMICOLON,  EQUALS,         PRINTSCREEN,    AC_REFRESH)
	KE5(LEFTBRACKET,BACKSLASH,  RIGHTBRACKET,   A,          B           )
	KE8(C,          D,          E,              F,          G,          H,          I,          J           )
	KE8(K,          L,          M,              N,          O,          P,          Q,          R           )
	KE8(S,          T,          U,              V,          W,          X,          Y,          Z           )
	KE8(DELETE,     KP_0,       KP_1,           KP_2,       KP_3,       KP_4,       KP_5,       KP_6        )
	KE8(KP_7,       KP_8,       KP_9,           KP_PERIOD,  KP_DIVIDE,  KP_MULTIPLY,KP_MINUS,   KP_PLUS     )
	KE8(KP_ENTER,   KP_EQUALS,  UP,             DOWN,       RIGHT,      LEFT,       INSERT,     HOME        )
	KE8(END,        PAGEUP,     PAGEDOWN,       F1,         F2,         F3,         F4,         F5          )
	KE8(F6,         F7,         F8,             F9,         F10,        F11,        F12,        F13         )
	KE8(F14,        F15,        NUMLOCKCLEAR,   CAPSLOCK,   SCROLLLOCK, RSHIFT,     LSHIFT,     RCTRL       )
	KE7(LCTRL,      RALT,       LALT,           LGUI,       RGUI,       KP_DECIMAL, APPLICATION)
	KE5(MENU, NONUSBACKSLASH, UNDO, APOSTROPHE, GRAVE )
	{-1, ""}
};

static const char * lookup_key_name(const key_lookup_table *kt, int kc)
{
	int i=0;
	while (kt[i].code>=0)
	{
		if (kc==kt[i].code)
			return kt[i].name;
		i++;
	}
	return NULL;
}

#ifdef SDLMAME_WIN32
int utf8_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	SDL_Event event;
	int quit = 0;
	char lasttext[20] = "";

	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n",
							SDL_GetError());
		exit(1);
	}
	SDL_CreateWindow("Input Test", 0, 0, 100, 100,0 );
	while(SDL_PollEvent(&event) || !quit) {
		switch(event.type) {
		case SDL_QUIT:
			quit = 1;
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE)
				quit=1;
			else
			{
				printf("ITEM_ID_XY %s 0x%x 0x%x %s\n",
					lookup_key_name(sdl_lookup, event.key.keysym.scancode),
					(int) event.key.keysym.scancode,
					(int) event.key.keysym.sym,
					"");
				lasttext[0] = 0;
			}
			break;
		case SDL_KEYUP:
			printf("ITEM_ID_XY %s 0x%x 0x%x %s\n",
					lookup_key_name(sdl_lookup, event.key.keysym.scancode),
					(int) event.key.keysym.scancode,
					(int) event.key.keysym.sym,
					lasttext);
			break;
		case SDL_TEXTINPUT:
			strcpy(lasttext, event.text.text);
			break;
		}
		event.type = 0;
	}
	SDL_Quit();
	return(0);
}
