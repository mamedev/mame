// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Brad Hughes
//============================================================
//
//  input_common.cpp - Common code for all MAME input modules
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "input_module.h"
#include "modules/lib/osdobj_common.h"

#include <memory>

// MAME headers
#include "emu.h"

// winnt.h defines this
#ifdef DELETE
#undef DELETE
#endif

#include "input_common.h"

//============================================================
//  Keyboard translation table
//============================================================

#if defined(OSD_WINDOWS)
#include <windows.h>
#define KEY_TRANS_ENTRY0(mame, sdlsc, sdlkey, disc, virtual, ascii, UI) { ITEM_ID_##mame, KEY_ ## disc, virtual, ascii, "ITEM_ID_"#mame, (char *) UI }
#define KEY_TRANS_ENTRY1(mame, sdlsc, sdlkey, disc, virtual, ascii)     { ITEM_ID_##mame, KEY_ ## disc, virtual, ascii, "ITEM_ID_"#mame, (char*) #mame }
#elif defined(OSD_SDL)
// SDL include
#include <sdlinc.h>
#define KEY_TRANS_ENTRY0(mame, sdlsc, sdlkey, disc, virtual, ascii, UI) { ITEM_ID_##mame, SDL_SCANCODE_ ## sdlsc, SDLK_ ## sdlkey, ascii, "ITEM_ID_"#mame, (char *) UI }
#define KEY_TRANS_ENTRY1(mame, sdlsc, sdlkey, disc, virtual, ascii)     { ITEM_ID_##mame, SDL_SCANCODE_ ## sdlsc, SDLK_ ## sdlkey, ascii, "ITEM_ID_"#mame, (char*) #mame }
#else
// osd mini
#endif

#if defined(OSD_WINDOWS) || defined(OSD_SDL)
key_trans_entry keyboard_trans_table::s_default_table[] =
{
	//              MAME key       sdl scancode  sdl key       di scancode     virtual key     ascii     ui
	KEY_TRANS_ENTRY0(ESC,          ESCAPE,       ESCAPE,       ESCAPE,         VK_ESCAPE,      27,       "ESCAPE"),
	KEY_TRANS_ENTRY1(1,            1,            1,            1,              '1',            '1'),
	KEY_TRANS_ENTRY1(2,            2,            2,            2,              '2',            '2'),
	KEY_TRANS_ENTRY1(3,            3,            3,            3,              '3',            '3'),
	KEY_TRANS_ENTRY1(4,            4,            4,            4,              '4',            '4'),
	KEY_TRANS_ENTRY1(5,            5,            5,            5,              '5',            '5'),
	KEY_TRANS_ENTRY1(6,            6,            6,            6,              '6',            '6'),
	KEY_TRANS_ENTRY1(7,            7,            7,            7,              '7',            '7'),
	KEY_TRANS_ENTRY1(8,            8,            8,            8,              '8',            '8'),
	KEY_TRANS_ENTRY1(9,            9,            9,            9,              '9',            '9'),
	KEY_TRANS_ENTRY1(0,            0,            0,            0,              '0',            '0'),
	KEY_TRANS_ENTRY1(MINUS,        MINUS,        MINUS,        MINUS,          VK_OEM_MINUS,   '-'),
	KEY_TRANS_ENTRY1(EQUALS,       EQUALS,       EQUALS,       EQUALS,         VK_OEM_PLUS,    '='),
	KEY_TRANS_ENTRY1(BACKSPACE,    BACKSPACE,    BACKSPACE,    BACK,           VK_BACK,         8),
	KEY_TRANS_ENTRY1(TAB,          TAB,          TAB,          TAB,            VK_TAB,          9),
	KEY_TRANS_ENTRY1(Q,            Q,            q,            Q,              'Q',            'Q'),
	KEY_TRANS_ENTRY1(W,            W,            w,            W,              'W',            'W'),
	KEY_TRANS_ENTRY1(E,            E,            e,            E,              'E',            'E'),
	KEY_TRANS_ENTRY1(R,            R,            r,            R,              'R',            'R'),
	KEY_TRANS_ENTRY1(T,            T,            t,            T,              'T',            'T'),
	KEY_TRANS_ENTRY1(Y,            Y,            y,            Y,              'Y',            'Y'),
	KEY_TRANS_ENTRY1(U,            U,            u,            U,              'U',            'U'),
	KEY_TRANS_ENTRY1(I,            I,            i,            I,              'I',            'I'),
	KEY_TRANS_ENTRY1(O,            O,            o,            O,              'O',            'O'),
	KEY_TRANS_ENTRY1(P,            P,            p,            P,              'P',            'P'),
	KEY_TRANS_ENTRY1(OPENBRACE,    LEFTBRACKET,  LEFTBRACKET,  LBRACKET,       VK_OEM_4,       '['),
	KEY_TRANS_ENTRY1(CLOSEBRACE,   RIGHTBRACKET, RIGHTBRACKET, RBRACKET,       VK_OEM_6,       ']'),
	KEY_TRANS_ENTRY0(ENTER,        RETURN,       RETURN,       RETURN,         VK_RETURN,       13,      "RETURN"),
	KEY_TRANS_ENTRY1(LCONTROL,     LCTRL,        LCTRL,        LCONTROL,       VK_LCONTROL,     0),
	KEY_TRANS_ENTRY1(A,            A,            a,            A,              'A',            'A'),
	KEY_TRANS_ENTRY1(S,            S,            s,            S,              'S',            'S'),
	KEY_TRANS_ENTRY1(D,            D,            d,            D,              'D',            'D'),
	KEY_TRANS_ENTRY1(F,            F,            f,            F,              'F',            'F'),
	KEY_TRANS_ENTRY1(G,            G,            g,            G,              'G',            'G'),
	KEY_TRANS_ENTRY1(H,            H,            h,            H,              'H',            'H'),
	KEY_TRANS_ENTRY1(J,            J,            j,            J,              'J',            'J'),
	KEY_TRANS_ENTRY1(K,            K,            k,            K,              'K',            'K'),
	KEY_TRANS_ENTRY1(L,            L,            l,            L,              'L',            'L'),
	KEY_TRANS_ENTRY1(COLON,        SEMICOLON,    SEMICOLON,    SEMICOLON,      VK_OEM_1,       ';'),
	KEY_TRANS_ENTRY1(QUOTE,        APOSTROPHE,   QUOTE,        APOSTROPHE,     VK_OEM_7,       '\''),
	KEY_TRANS_ENTRY1(TILDE,        GRAVE,        BACKQUOTE,    GRAVE,          VK_OEM_3,       '`'),
	KEY_TRANS_ENTRY1(LSHIFT,       LSHIFT,       LSHIFT,       LSHIFT,         VK_LSHIFT,      0),
	KEY_TRANS_ENTRY1(BACKSLASH,    BACKSLASH,    BACKSLASH,    BACKSLASH,      VK_OEM_5,       '\\'),
	KEY_TRANS_ENTRY1(BACKSLASH2,   0,            0,            OEM_102,        VK_OEM_102,     '<'),
	KEY_TRANS_ENTRY1(Z,            Z,            z,            Z,              'Z',            'Z'),
	KEY_TRANS_ENTRY1(X,            X,            x,            X,              'X',            'X'),
	KEY_TRANS_ENTRY1(C,            C,            c,            C,              'C',            'C'),
	KEY_TRANS_ENTRY1(V,            V,            v,            V,              'V',            'V'),
	KEY_TRANS_ENTRY1(B,            B,            b,            B,              'B',            'B'),
	KEY_TRANS_ENTRY1(N,            N,            n,            N,              'N',            'N'),
	KEY_TRANS_ENTRY1(M,            M,            m,            M,              'M',            'M'),
	KEY_TRANS_ENTRY1(COMMA,        COMMA,        COMMA,        COMMA,          VK_OEM_COMMA,   ','),
	KEY_TRANS_ENTRY1(STOP,         PERIOD,       PERIOD,       PERIOD,         VK_OEM_PERIOD,  '.'),
	KEY_TRANS_ENTRY1(SLASH,        SLASH,        SLASH,        SLASH,          VK_OEM_2,       '/'),
	KEY_TRANS_ENTRY1(RSHIFT,       RSHIFT,       RSHIFT,       RSHIFT,         VK_RSHIFT,      0),
	KEY_TRANS_ENTRY1(ASTERISK,     KP_MULTIPLY,  KP_MULTIPLY,  MULTIPLY,       VK_MULTIPLY,    '*'),
	KEY_TRANS_ENTRY1(LALT,         LALT,         LALT,         LMENU,          VK_LMENU,       0),
	KEY_TRANS_ENTRY1(SPACE,        SPACE,        SPACE,        SPACE,          VK_SPACE,       ' '),
	KEY_TRANS_ENTRY1(CAPSLOCK,     CAPSLOCK,     CAPSLOCK,     CAPITAL,        VK_CAPITAL,     0),
	KEY_TRANS_ENTRY1(F1,           F1,           F1,           F1,             VK_F1,          0),
	KEY_TRANS_ENTRY1(F2,           F2,           F2,           F2,             VK_F2,          0),
	KEY_TRANS_ENTRY1(F3,           F3,           F3,           F3,             VK_F3,          0),
	KEY_TRANS_ENTRY1(F4,           F4,           F4,           F4,             VK_F4,          0),
	KEY_TRANS_ENTRY1(F5,           F5,           F5,           F5,             VK_F5,          0),
	KEY_TRANS_ENTRY1(F6,           F6,           F6,           F6,             VK_F6,          0),
	KEY_TRANS_ENTRY1(F7,           F7,           F7,           F7,             VK_F7,          0),
	KEY_TRANS_ENTRY1(F8,           F8,           F8,           F8,             VK_F8,          0),
	KEY_TRANS_ENTRY1(F9,           F9,           F9,           F9,             VK_F9,          0),
	KEY_TRANS_ENTRY1(F10,          F10,          F10,          F10,            VK_F10,         0),
	KEY_TRANS_ENTRY1(NUMLOCK,      NUMLOCKCLEAR, NUMLOCKCLEAR, NUMLOCK,        VK_NUMLOCK,     0),
	KEY_TRANS_ENTRY1(SCRLOCK,      SCROLLLOCK,   SCROLLLOCK,   SCROLL,         VK_SCROLL,      0),
	KEY_TRANS_ENTRY1(7_PAD,        KP_7,         KP_7,         NUMPAD7,        VK_NUMPAD7,     0),
	KEY_TRANS_ENTRY1(8_PAD,        KP_8,         KP_8,         NUMPAD8,        VK_NUMPAD8,     0),
	KEY_TRANS_ENTRY1(9_PAD,        KP_9,         KP_9,         NUMPAD9,        VK_NUMPAD9,     0),
	KEY_TRANS_ENTRY1(MINUS_PAD,    KP_MINUS,     KP_MINUS,     SUBTRACT,       VK_SUBTRACT,    0),
	KEY_TRANS_ENTRY1(4_PAD,        KP_4,         KP_4,         NUMPAD4,        VK_NUMPAD4,     0),
	KEY_TRANS_ENTRY1(5_PAD,        KP_5,         KP_5,         NUMPAD5,        VK_NUMPAD5,     0),
	KEY_TRANS_ENTRY1(6_PAD,        KP_6,         KP_6,         NUMPAD6,        VK_NUMPAD6,     0),
	KEY_TRANS_ENTRY1(PLUS_PAD,     KP_PLUS,      KP_PLUS,      ADD,            VK_ADD,         0),
	KEY_TRANS_ENTRY1(1_PAD,        KP_1,         KP_1,         NUMPAD1,        VK_NUMPAD1,     0),
	KEY_TRANS_ENTRY1(2_PAD,        KP_2,         KP_2,         NUMPAD2,        VK_NUMPAD2,     0),
	KEY_TRANS_ENTRY1(3_PAD,        KP_3,         KP_3,         NUMPAD3,        VK_NUMPAD3,     0),
	KEY_TRANS_ENTRY1(0_PAD,        KP_0,         KP_0,         NUMPAD0,        VK_NUMPAD0,     0),
	KEY_TRANS_ENTRY1(DEL_PAD,      KP_PERIOD,    KP_PERIOD,    DECIMAL,        VK_DECIMAL,     0),
	KEY_TRANS_ENTRY1(F11,          F11,          F11,          F11,            VK_F11,         0),
	KEY_TRANS_ENTRY1(F12,          F12,          F12,          F12,            VK_F12,         0),
	KEY_TRANS_ENTRY1(F13,          F13,          F13,          F13,            VK_F13,         0),
	KEY_TRANS_ENTRY1(F14,          F14,          F14,          F14,            VK_F14,         0),
	KEY_TRANS_ENTRY1(F15,          F15,          F15,          F15,            VK_F15,         0),
	KEY_TRANS_ENTRY1(ENTER_PAD,    KP_ENTER,     KP_ENTER,     NUMPADENTER,    VK_RETURN,      0),
	KEY_TRANS_ENTRY1(RCONTROL,     RCTRL,        RCTRL,        RCONTROL,       VK_RCONTROL,    0),
	KEY_TRANS_ENTRY1(SLASH_PAD,    KP_DIVIDE,    KP_DIVIDE,    DIVIDE,         VK_DIVIDE,      0),
	KEY_TRANS_ENTRY1(PRTSCR,       PRINTSCREEN,  PRINTSCREEN,  SYSRQ,          0,              0),
	KEY_TRANS_ENTRY1(RALT,         RALT,         RALT,         RMENU,          VK_RMENU,       0),
	KEY_TRANS_ENTRY1(HOME,         HOME,         HOME,         HOME,           VK_HOME,        0),
	KEY_TRANS_ENTRY1(UP,           UP,           UP,           UP,             VK_UP,          0),
	KEY_TRANS_ENTRY1(PGUP,         PAGEUP,       PAGEUP,       PRIOR,          VK_PRIOR,       0),
	KEY_TRANS_ENTRY1(LEFT,         LEFT,         LEFT,         LEFT,           VK_LEFT,        0),
	KEY_TRANS_ENTRY1(RIGHT,        RIGHT,        RIGHT,        RIGHT,          VK_RIGHT,       0),
	KEY_TRANS_ENTRY1(END,          END,          END,          END,            VK_END,         0),
	KEY_TRANS_ENTRY1(DOWN,         DOWN,         DOWN,         DOWN,           VK_DOWN,        0),
	KEY_TRANS_ENTRY1(PGDN,         PAGEDOWN,     PAGEDOWN,     NEXT,           VK_NEXT,        0),
	KEY_TRANS_ENTRY1(INSERT,       INSERT,       INSERT,       INSERT,         VK_INSERT,      0),
	KEY_TRANS_ENTRY0(DEL,          DELETE,       DELETE,       DELETE,         VK_DELETE,      0,        "DELETE"),
	KEY_TRANS_ENTRY1(LWIN,         LGUI,         LGUI,         LWIN,           VK_LWIN,        0),
	KEY_TRANS_ENTRY1(RWIN,         RGUI,         RGUI,         RWIN,           VK_RWIN,        0),
	KEY_TRANS_ENTRY1(MENU,         MENU,         MENU,         APPS,           VK_APPS,        0),
	KEY_TRANS_ENTRY1(PAUSE,        PAUSE,        PAUSE,        PAUSE,          VK_PAUSE,       0),
	KEY_TRANS_ENTRY0(CANCEL,       UNKNOWN,      UNKNOWN,      UNKNOWN,        0,              0,        "CANCEL"),

	// New keys introduced in Windows 2000. These have no MAME codes to
	// preserve compatibility with old config files that may refer to them
	// as e.g. FORWARD instead of e.g. KEYCODE_WEBFORWARD. They need table
	// entries anyway because otherwise they aren't recognized when
	// GetAsyncKeyState polling is used (as happens currently when MAME is
	// paused). Some codes are missing because the mapping to vkey codes
	// isn't clear, and MapVirtualKey is no help.
	KEY_TRANS_ENTRY1(OTHER_SWITCH, MUTE,         MUTE,           MUTE,           VK_VOLUME_MUTE,         0),
	KEY_TRANS_ENTRY1(OTHER_SWITCH, VOLUMEDOWN,   VOLUMEDOWN,     VOLUMEDOWN,     VK_VOLUME_DOWN,         0),
	KEY_TRANS_ENTRY1(OTHER_SWITCH, VOLUMEUP,     VOLUMEUP,       VOLUMEUP,       VK_VOLUME_UP,           0),
	KEY_TRANS_ENTRY1(OTHER_SWITCH, AC_HOME,      AC_HOME,        WEBHOME,        VK_BROWSER_HOME,        0),
	KEY_TRANS_ENTRY1(OTHER_SWITCH, AC_SEARCH,    AC_SEARCH,      WEBSEARCH,      VK_BROWSER_SEARCH,      0),
	KEY_TRANS_ENTRY1(OTHER_SWITCH, AC_BOOKMARKS, AC_BOOKMARKS,   WEBFAVORITES,   VK_BROWSER_FAVORITES,   0),
	KEY_TRANS_ENTRY1(OTHER_SWITCH, AC_REFRESH,   AC_REFRESH,     WEBREFRESH,     VK_BROWSER_REFRESH,     0),
	KEY_TRANS_ENTRY1(OTHER_SWITCH, AC_STOP,      AC_STOP,        WEBSTOP,        VK_BROWSER_STOP,        0),
	KEY_TRANS_ENTRY1(OTHER_SWITCH, AC_FORWARD,   AC_FORWARD,     WEBFORWARD,     VK_BROWSER_FORWARD,     0),
	KEY_TRANS_ENTRY1(OTHER_SWITCH, AC_BACK,      AC_BACK,        WEBBACK,        VK_BROWSER_BACK,        0),
	KEY_TRANS_ENTRY1(OTHER_SWITCH, MAIL,         MAIL,           MAIL,           VK_LAUNCH_MAIL,         0),
	KEY_TRANS_ENTRY1(OTHER_SWITCH, MEDIASELECT,  MEDIASELECT,    MEDIASELECT,    VK_LAUNCH_MEDIA_SELECT, 0),
	KEY_TRANS_ENTRY0(INVALID,      UNKNOWN,      UNKNOWN,        ESCAPE,         0,                      0,      "INVALID")
};

// The private constructor to create the default instance
keyboard_trans_table::keyboard_trans_table()
{
	m_table = s_default_table;
	m_table_size = ARRAY_LENGTH(s_default_table);
}
#else
keyboard_trans_table::keyboard_trans_table()
{
	m_table = nullptr;
	m_table_size = 0;
}

#endif
// public constructor to allow creation of non-default instances
keyboard_trans_table::keyboard_trans_table(std::unique_ptr<key_trans_entry[]> entries, unsigned int size)
{
	m_custom_table = std::move(entries);
	m_table = m_custom_table.get();
	m_table_size = size;
}

int keyboard_trans_table::lookup_mame_index(const char *scode) const
{
	for (int i = 0; i < m_table_size; i++)
	{
		if (!strcmp(scode, m_table[i].mame_key_name))
			return i;
	}
	return -1;
}

input_item_id keyboard_trans_table::lookup_mame_code(const char *scode) const
{
	int const index = lookup_mame_index(scode);
	if (index >= 0)
		return m_table[index].mame_key;
	else
		return ITEM_ID_INVALID;
}

// Windows specific lookup methods
#if defined(OSD_WINDOWS)

input_item_id keyboard_trans_table::map_di_scancode_to_itemid(int scancode) const
{
	int tablenum;

	// scan the table for a match
	for (tablenum = 0; tablenum < m_table_size; tablenum++)
		if (m_table[tablenum].scan_code == scancode)
			return m_table[tablenum].mame_key;

	// default to an "other" switch
	return ITEM_ID_OTHER_SWITCH;
}

//============================================================
//  wininput_vkey_for_mame_code
//============================================================

int keyboard_trans_table::vkey_for_mame_code(input_code code) const
{
	// only works for keyboard switches
	if (code.device_class() == DEVICE_CLASS_KEYBOARD && code.item_class() == ITEM_CLASS_SWITCH)
	{
		input_item_id id = code.item_id();
		int tablenum;

		// scan the table for a match
		for (tablenum = 0; tablenum < m_table_size; tablenum++)
			if (m_table[tablenum].mame_key == id)
				return m_table[tablenum].virtual_key;
	}
	return 0;
}

#endif


int input_module_base::init(const osd_options &options)
{
	m_options = &options;

	m_mouse_enabled = options.mouse();
	m_lightgun_enabled = options.lightgun();

	int result = init_internal();
	if (result != 0)
		return result;

	m_input_paused = false;
	m_input_enabled = true;

	return 0;
}
