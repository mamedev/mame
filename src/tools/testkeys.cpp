//============================================================
//
//  testkeys.cpp - A small utility to analyze SDL keycodes
//
//  Copyright (c) 1996-2019, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//  testkeys by couriersud
//
//============================================================

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <wchar.h>

#include "SDL2/SDL.h"

//#include "unicode.h"

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
    KE(AC_SEARCH)
    KE(AC_HOME)
    KE(AC_BACK)
    KE(AC_FORWARD)
    KE(AC_STOP)
    KE(AC_REFRESH)
    KE(AC_BOOKMARKS)

    KE(BRIGHTNESSDOWN)
    KE(BRIGHTNESSUP)
    KE(DISPLAYSWITCH) 
    KE(KBDILLUMTOGGLE)
    KE(KBDILLUMDOWN)
    KE(KBDILLUMUP)
    KE(EJECT)
    KE(SLEEP)

    KE(APP1)
    KE(APP2)


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
					0, //(int) event.key.keysym.unicode,
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
					0, //(int) event.key.keysym.unicode,
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
