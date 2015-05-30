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

#if (SDLMAME_SDL2)
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
#else
#define KE(x) { SDLK_ ## x, "SDLK_" #x },
#define KE8(A, B, C, D, E, F, G, H) KE(A) KE(B) KE(C) KE(D) KE(E) KE(F) KE(G) KE(H)

static key_lookup_table sdl_lookup[] =
{
	KE8(UNKNOWN,    FIRST,      BACKSPACE,      TAB,        CLEAR,      RETURN,     PAUSE,      ESCAPE      )
	KE8(SPACE,      EXCLAIM,    QUOTEDBL,       HASH,       DOLLAR,     AMPERSAND,  QUOTE,      LEFTPAREN   )
	KE8(RIGHTPAREN, ASTERISK,   PLUS,           COMMA,      MINUS,      PERIOD,     SLASH,      0           )
	KE8(1,          2,          3,              4,          5,          6,          7,          8           )
	KE8(9,          COLON,      SEMICOLON,      LESS,       EQUALS,     GREATER,    QUESTION,   AT          )
	KE8(LEFTBRACKET,BACKSLASH,  RIGHTBRACKET,   CARET,      UNDERSCORE, BACKQUOTE,  a,          b           )
	KE8(c,          d,          e,              f,          g,          h,          i,          j           )
	KE8(k,          l,          m,              n,          o,          p,          q,          r           )
	KE8(s,          t,          u,              v,          w,          x,          y,          z           )
	KE8(DELETE,     WORLD_0,    WORLD_1,        WORLD_2,    WORLD_3,    WORLD_4,    WORLD_5,    WORLD_6     )
	KE8(WORLD_7,    WORLD_8,    WORLD_9,        WORLD_10,   WORLD_11,   WORLD_12,   WORLD_13,   WORLD_14    )
	KE8(WORLD_15,   WORLD_16,   WORLD_17,       WORLD_18,   WORLD_19,   WORLD_20,   WORLD_21,   WORLD_22    )
	KE8(WORLD_23,   WORLD_24,   WORLD_25,       WORLD_26,   WORLD_27,   WORLD_28,   WORLD_29,   WORLD_30    )
	KE8(WORLD_31,   WORLD_32,   WORLD_33,       WORLD_34,   WORLD_35,   WORLD_36,   WORLD_37,   WORLD_38    )
	KE8(WORLD_39,   WORLD_40,   WORLD_41,       WORLD_42,   WORLD_43,   WORLD_44,   WORLD_45,   WORLD_46    )
	KE8(WORLD_47,   WORLD_48,   WORLD_49,       WORLD_50,   WORLD_51,   WORLD_52,   WORLD_53,   WORLD_54    )
	KE8(WORLD_55,   WORLD_56,   WORLD_57,       WORLD_58,   WORLD_59,   WORLD_60,   WORLD_61,   WORLD_62    )
	KE8(WORLD_63,   WORLD_64,   WORLD_65,       WORLD_66,   WORLD_67,   WORLD_68,   WORLD_69,   WORLD_70    )
	KE8(WORLD_71,   WORLD_72,   WORLD_73,       WORLD_74,   WORLD_75,   WORLD_76,   WORLD_77,   WORLD_78    )
	KE8(WORLD_79,   WORLD_80,   WORLD_81,       WORLD_82,   WORLD_83,   WORLD_84,   WORLD_85,   WORLD_86    )
	KE8(WORLD_87,   WORLD_88,   WORLD_89,       WORLD_90,   WORLD_91,   WORLD_92,   WORLD_93,   WORLD_94    )
	KE8(WORLD_95,   KP0,        KP1,            KP2,        KP3,        KP4,        KP5,        KP6         )
	KE8(KP7,        KP8,        KP9,            KP_PERIOD,  KP_DIVIDE,  KP_MULTIPLY,KP_MINUS,   KP_PLUS     )
	KE8(KP_ENTER,   KP_EQUALS,  UP,             DOWN,       RIGHT,      LEFT,       INSERT,     HOME        )
	KE8(END,        PAGEUP,     PAGEDOWN,       F1,         F2,         F3,         F4,         F5          )
	KE8(F6,         F7,         F8,             F9,         F10,        F11,        F12,        F13         )
	KE8(F14,        F15,        NUMLOCK,        CAPSLOCK,   SCROLLOCK,  RSHIFT,     LSHIFT,     RCTRL       )
	KE8(LCTRL,      RALT,       LALT,           RMETA,      LMETA,      LSUPER,     RSUPER,     MODE        )
	KE8(COMPOSE,    HELP,       PRINT,          SYSREQ,     BREAK,      MENU,       POWER,      EURO        )
	KE(UNDO)
	KE(LAST)
	{-1, ""}
};
#endif

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
#if (SDLMAME_SDL2)
	char lasttext[20] = "";
#else
	char buf[20];
#endif

	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n",
							SDL_GetError());
		exit(1);
	}
#if (SDLMAME_SDL2)
	SDL_CreateWindow("Input Test", 0, 0, 100, 100,0 );
#else
	SDL_SetVideoMode(100, 50, 16, SDL_ANYFORMAT);
	SDL_EnableUNICODE(1);
#endif
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
#if (SDLMAME_SDL2)
				printf("ITEM_ID_XY %s 0x%x 0x%x %s\n",
					lookup_key_name(sdl_lookup, event.key.keysym.scancode),
					(int) event.key.keysym.scancode,
					(int) event.key.keysym.sym,
					"");
				lasttext[0] = 0;
#else
				memset(buf, 0, 19);
				utf8_from_uchar(buf, sizeof(buf), event.key.keysym.unicode);
				printf("ITEM_ID_XY %s 0x%x 0x%x %s\n",
					lookup_key_name(sdl_lookup, event.key.keysym.sym),
					(int) event.key.keysym.scancode,
					(int) event.key.keysym.unicode,
					buf);
#endif
			}
			break;
		case SDL_KEYUP:
#if (SDLMAME_SDL2)
			printf("ITEM_ID_XY %s 0x%x 0x%x %s\n",
					lookup_key_name(sdl_lookup, event.key.keysym.scancode),
					(int) event.key.keysym.scancode,
					(int) event.key.keysym.sym,
					lasttext);
#else
			memset(buf, 0, 19);
			utf8_from_uchar(buf, sizeof(buf), event.key.keysym.unicode);
			printf("ITEM_ID_XY %s 0x%x 0x%x %s\n",
					lookup_key_name(sdl_lookup, event.key.keysym.sym),
					(int) event.key.keysym.scancode,
					(int) event.key.keysym.unicode,
					buf);
#endif
			break;
#if (SDLMAME_SDL2)
		case SDL_TEXTINPUT:
			strcpy(lasttext, event.text.text);
			break;
#endif
		}
		event.type = 0;

		#ifdef SDLMAME_OS2
		SDL_Delay( 10 );
		#endif
	}
	SDL_Quit();
	return(0);
}
