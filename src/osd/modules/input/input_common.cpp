// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Brad Hughes
//============================================================
//
//  input_common.cpp - Common code for all MAME input modules
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "input_common.h"

#include "modules/lib/osdobj_common.h"

#include "emu.h" // so we can get an input manager from the running machine

//============================================================
//  Keyboard translation table
//============================================================

#if defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)
#include <windows.h>
#include <dinput.h>
#define DIK_UNKNOWN 0xffff // intentionally impossible
#define KEY_TRANS_WIN32(disc, virtual) DIK_##disc, virtual,
#else
#define KEY_TRANS_WIN32(disc, virtual)
#endif

#if defined(OSD_SDL) || defined(SDLMAME_WIN32)
#include <SDL2/SDL.h>
#define KEY_TRANS_SDL(sdlsc) SDL_SCANCODE_##sdlsc,
#else
#define KEY_TRANS_SDL(sdlsc)
#endif

#define KEY_TRANS_ENTRY0(mame, sdlsc, disc, virtual, ascii, UI) { ITEM_ID_##mame, KEY_TRANS_SDL(sdlsc) KEY_TRANS_WIN32(disc, virtual) ascii, "ITEM_ID_"#mame, UI }
#define KEY_TRANS_ENTRY1(mame, sdlsc, disc, virtual, ascii)     { ITEM_ID_##mame, KEY_TRANS_SDL(sdlsc) KEY_TRANS_WIN32(disc, virtual) ascii, "ITEM_ID_"#mame, #mame }

// winnt.h defines this
#ifdef DELETE
#undef DELETE
#endif

#if defined(OSD_WINDOWS) || defined(OSD_SDL)

key_trans_entry keyboard_trans_table::s_default_table[] =
{
	//              MAME key       SDL scancode    DI scancode     virtual key             ASCII     UI
	KEY_TRANS_ENTRY0(ESC,          ESCAPE,         ESCAPE,         VK_ESCAPE,              27,       "ESCAPE"),
	KEY_TRANS_ENTRY1(1,            1,              1,              '1',                    '1'),
	KEY_TRANS_ENTRY1(2,            2,              2,              '2',                    '2'),
	KEY_TRANS_ENTRY1(3,            3,              3,              '3',                    '3'),
	KEY_TRANS_ENTRY1(4,            4,              4,              '4',                    '4'),
	KEY_TRANS_ENTRY1(5,            5,              5,              '5',                    '5'),
	KEY_TRANS_ENTRY1(6,            6,              6,              '6',                    '6'),
	KEY_TRANS_ENTRY1(7,            7,              7,              '7',                    '7'),
	KEY_TRANS_ENTRY1(8,            8,              8,              '8',                    '8'),
	KEY_TRANS_ENTRY1(9,            9,              9,              '9',                    '9'),
	KEY_TRANS_ENTRY1(0,            0,              0,              '0',                    '0'),
	KEY_TRANS_ENTRY1(MINUS,        MINUS,          MINUS,          VK_OEM_MINUS,           '-'),
	KEY_TRANS_ENTRY1(EQUALS,       EQUALS,         EQUALS,         VK_OEM_PLUS,            '='),
	KEY_TRANS_ENTRY1(BACKSPACE,    BACKSPACE,      BACK,           VK_BACK,                8),
	KEY_TRANS_ENTRY1(TAB,          TAB,            TAB,            VK_TAB,                 9),
	KEY_TRANS_ENTRY1(Q,            Q,              Q,              'Q',                    'Q'),
	KEY_TRANS_ENTRY1(W,            W,              W,              'W',                    'W'),
	KEY_TRANS_ENTRY1(E,            E,              E,              'E',                    'E'),
	KEY_TRANS_ENTRY1(R,            R,              R,              'R',                    'R'),
	KEY_TRANS_ENTRY1(T,            T,              T,              'T',                    'T'),
	KEY_TRANS_ENTRY1(Y,            Y,              Y,              'Y',                    'Y'),
	KEY_TRANS_ENTRY1(U,            U,              U,              'U',                    'U'),
	KEY_TRANS_ENTRY1(I,            I,              I,              'I',                    'I'),
	KEY_TRANS_ENTRY1(O,            O,              O,              'O',                    'O'),
	KEY_TRANS_ENTRY1(P,            P,              P,              'P',                    'P'),
	KEY_TRANS_ENTRY1(OPENBRACE,    LEFTBRACKET,    LBRACKET,       VK_OEM_4,               '['),
	KEY_TRANS_ENTRY1(CLOSEBRACE,   RIGHTBRACKET,   RBRACKET,       VK_OEM_6,               ']'),
	KEY_TRANS_ENTRY0(ENTER,        RETURN,         RETURN,         VK_RETURN,              13,      "RETURN"),
	KEY_TRANS_ENTRY1(LCONTROL,     LCTRL,          LCONTROL,       VK_LCONTROL,            0),
	KEY_TRANS_ENTRY1(A,            A,              A,              'A',                    'A'),
	KEY_TRANS_ENTRY1(S,            S,              S,              'S',                    'S'),
	KEY_TRANS_ENTRY1(D,            D,              D,              'D',                    'D'),
	KEY_TRANS_ENTRY1(F,            F,              F,              'F',                    'F'),
	KEY_TRANS_ENTRY1(G,            G,              G,              'G',                    'G'),
	KEY_TRANS_ENTRY1(H,            H,              H,              'H',                    'H'),
	KEY_TRANS_ENTRY1(J,            J,              J,              'J',                    'J'),
	KEY_TRANS_ENTRY1(K,            K,              K,              'K',                    'K'),
	KEY_TRANS_ENTRY1(L,            L,              L,              'L',                    'L'),
	KEY_TRANS_ENTRY1(COLON,        SEMICOLON,      SEMICOLON,      VK_OEM_1,               ';'),
	KEY_TRANS_ENTRY1(QUOTE,        APOSTROPHE,     APOSTROPHE,     VK_OEM_7,               '\''),
	KEY_TRANS_ENTRY1(TILDE,        GRAVE,          GRAVE,          VK_OEM_3,               '`'),
	KEY_TRANS_ENTRY1(LSHIFT,       LSHIFT,         LSHIFT,         VK_LSHIFT,              0),
	KEY_TRANS_ENTRY1(BACKSLASH,    BACKSLASH,      BACKSLASH,      VK_OEM_5,               '\\'),
	KEY_TRANS_ENTRY1(BACKSLASH2,   NONUSBACKSLASH, OEM_102,        VK_OEM_102,             '<'), // immediately to the right of left shift on ISO keyboards
	KEY_TRANS_ENTRY1(Z,            Z,              Z,              'Z',                    'Z'),
	KEY_TRANS_ENTRY1(X,            X,              X,              'X',                    'X'),
	KEY_TRANS_ENTRY1(C,            C,              C,              'C',                    'C'),
	KEY_TRANS_ENTRY1(V,            V,              V,              'V',                    'V'),
	KEY_TRANS_ENTRY1(B,            B,              B,              'B',                    'B'),
	KEY_TRANS_ENTRY1(N,            N,              N,              'N',                    'N'),
	KEY_TRANS_ENTRY1(M,            M,              M,              'M',                    'M'),
	KEY_TRANS_ENTRY1(COMMA,        COMMA,          COMMA,          VK_OEM_COMMA,           ','),
	KEY_TRANS_ENTRY1(STOP,         PERIOD,         PERIOD,         VK_OEM_PERIOD,          '.'),
	KEY_TRANS_ENTRY1(SLASH,        SLASH,          SLASH,          VK_OEM_2,               '/'),
	KEY_TRANS_ENTRY1(RSHIFT,       RSHIFT,         RSHIFT,         VK_RSHIFT,              0),
	KEY_TRANS_ENTRY1(ASTERISK,     KP_MULTIPLY,    MULTIPLY,       VK_MULTIPLY,            '*'),
	KEY_TRANS_ENTRY1(LALT,         LALT,           LMENU,          VK_LMENU,               0),
	KEY_TRANS_ENTRY1(SPACE,        SPACE,          SPACE,          VK_SPACE,               ' '),
	KEY_TRANS_ENTRY1(CAPSLOCK,     CAPSLOCK,       CAPITAL,        VK_CAPITAL,             0),
	KEY_TRANS_ENTRY1(F1,           F1,             F1,             VK_F1,                  0),
	KEY_TRANS_ENTRY1(F2,           F2,             F2,             VK_F2,                  0),
	KEY_TRANS_ENTRY1(F3,           F3,             F3,             VK_F3,                  0),
	KEY_TRANS_ENTRY1(F4,           F4,             F4,             VK_F4,                  0),
	KEY_TRANS_ENTRY1(F5,           F5,             F5,             VK_F5,                  0),
	KEY_TRANS_ENTRY1(F6,           F6,             F6,             VK_F6,                  0),
	KEY_TRANS_ENTRY1(F7,           F7,             F7,             VK_F7,                  0),
	KEY_TRANS_ENTRY1(F8,           F8,             F8,             VK_F8,                  0),
	KEY_TRANS_ENTRY1(F9,           F9,             F9,             VK_F9,                  0),
	KEY_TRANS_ENTRY1(F10,          F10,            F10,            VK_F10,                 0),
	KEY_TRANS_ENTRY1(NUMLOCK,      NUMLOCKCLEAR,   NUMLOCK,        VK_NUMLOCK,             0),
	KEY_TRANS_ENTRY1(SCRLOCK,      SCROLLLOCK,     SCROLL,         VK_SCROLL,              0),
	KEY_TRANS_ENTRY1(7_PAD,        KP_7,           NUMPAD7,        VK_NUMPAD7,             0),
	KEY_TRANS_ENTRY1(8_PAD,        KP_8,           NUMPAD8,        VK_NUMPAD8,             0),
	KEY_TRANS_ENTRY1(9_PAD,        KP_9,           NUMPAD9,        VK_NUMPAD9,             0),
	KEY_TRANS_ENTRY1(MINUS_PAD,    KP_MINUS,       SUBTRACT,       VK_SUBTRACT,            0),
	KEY_TRANS_ENTRY1(4_PAD,        KP_4,           NUMPAD4,        VK_NUMPAD4,             0),
	KEY_TRANS_ENTRY1(5_PAD,        KP_5,           NUMPAD5,        VK_NUMPAD5,             0),
	KEY_TRANS_ENTRY1(6_PAD,        KP_6,           NUMPAD6,        VK_NUMPAD6,             0),
	KEY_TRANS_ENTRY1(PLUS_PAD,     KP_PLUS,        ADD,            VK_ADD,                 0),
	KEY_TRANS_ENTRY1(1_PAD,        KP_1,           NUMPAD1,        VK_NUMPAD1,             0),
	KEY_TRANS_ENTRY1(2_PAD,        KP_2,           NUMPAD2,        VK_NUMPAD2,             0),
	KEY_TRANS_ENTRY1(3_PAD,        KP_3,           NUMPAD3,        VK_NUMPAD3,             0),
	KEY_TRANS_ENTRY1(0_PAD,        KP_0,           NUMPAD0,        VK_NUMPAD0,             0),
	KEY_TRANS_ENTRY1(DEL_PAD,      KP_PERIOD,      DECIMAL,        VK_DECIMAL,             0),
	KEY_TRANS_ENTRY1(F11,          F11,            F11,            VK_F11,                 0),
	KEY_TRANS_ENTRY1(F12,          F12,            F12,            VK_F12,                 0),
	KEY_TRANS_ENTRY1(F13,          F13,            F13,            VK_F13,                 0),
	KEY_TRANS_ENTRY1(F14,          F14,            F14,            VK_F14,                 0),
	KEY_TRANS_ENTRY1(F15,          F15,            F15,            VK_F15,                 0),
	KEY_TRANS_ENTRY1(F16,          F16,            UNKNOWN,        VK_F16,                 0),
	KEY_TRANS_ENTRY1(F17,          F17,            UNKNOWN,        VK_F17,                 0),
	KEY_TRANS_ENTRY1(F18,          F18,            UNKNOWN,        VK_F18,                 0),
	KEY_TRANS_ENTRY1(F19,          F19,            UNKNOWN,        VK_F19,                 0),
	KEY_TRANS_ENTRY1(F20,          F20,            UNKNOWN,        VK_F20,                 0),
	KEY_TRANS_ENTRY1(ENTER_PAD,    KP_ENTER,       NUMPADENTER,    VK_RETURN,              0),
	KEY_TRANS_ENTRY1(RCONTROL,     RCTRL,          RCONTROL,       VK_RCONTROL,            0),
	KEY_TRANS_ENTRY1(SLASH_PAD,    KP_DIVIDE,      DIVIDE,         VK_DIVIDE,              0),
	KEY_TRANS_ENTRY1(PRTSCR,       PRINTSCREEN,    SYSRQ,          0,                      0),
	KEY_TRANS_ENTRY1(RALT,         RALT,           RMENU,          VK_RMENU,               0),
	KEY_TRANS_ENTRY1(HOME,         HOME,           HOME,           VK_HOME,                0),
	KEY_TRANS_ENTRY1(UP,           UP,             UP,             VK_UP,                  0),
	KEY_TRANS_ENTRY1(PGUP,         PAGEUP,         PRIOR,          VK_PRIOR,               0),
	KEY_TRANS_ENTRY1(LEFT,         LEFT,           LEFT,           VK_LEFT,                0),
	KEY_TRANS_ENTRY1(RIGHT,        RIGHT,          RIGHT,          VK_RIGHT,               0),
	KEY_TRANS_ENTRY1(END,          END,            END,            VK_END,                 0),
	KEY_TRANS_ENTRY1(DOWN,         DOWN,           DOWN,           VK_DOWN,                0),
	KEY_TRANS_ENTRY1(PGDN,         PAGEDOWN,       NEXT,           VK_NEXT,                0),
	KEY_TRANS_ENTRY1(INSERT,       INSERT,         INSERT,         VK_INSERT,              0),
	KEY_TRANS_ENTRY0(DEL,          DELETE,         DELETE,         VK_DELETE,              0,        "DELETE"),
	KEY_TRANS_ENTRY1(LWIN,         LGUI,           LWIN,           VK_LWIN,                0),
	KEY_TRANS_ENTRY1(RWIN,         RGUI,           RWIN,           VK_RWIN,                0),
	KEY_TRANS_ENTRY1(MENU,         APPLICATION,    APPS,           VK_APPS,                0),
	KEY_TRANS_ENTRY1(PAUSE,        PAUSE,          PAUSE,          VK_PAUSE,               0),
	KEY_TRANS_ENTRY1(CANCEL,       CANCEL,         UNKNOWN,        0,                      0),
	KEY_TRANS_ENTRY1(BS_PAD,       KP_BACKSPACE,   UNKNOWN,        0,                      0),
	KEY_TRANS_ENTRY1(TAB_PAD,      KP_TAB,         UNKNOWN,        0,                      0),
	KEY_TRANS_ENTRY1(00_PAD,       KP_00,          UNKNOWN,        0,                      0),
	KEY_TRANS_ENTRY1(000_PAD,      KP_000,         UNKNOWN,        0,                      0),
	KEY_TRANS_ENTRY1(COMMA_PAD,    KP_COMMA,       NUMPADCOMMA,    VK_SEPARATOR,           0),
	KEY_TRANS_ENTRY1(EQUALS_PAD,   KP_EQUALS,      NUMPADEQUALS,   VK_OEM_NEC_EQUAL,       0),

	// keys that have no specific MAME input item IDs
	KEY_TRANS_ENTRY0(OTHER_SWITCH, F21,            UNKNOWN,        VK_F21,                 0,        "F21"),
	KEY_TRANS_ENTRY0(OTHER_SWITCH, F22,            UNKNOWN,        VK_F22,                 0,        "F22"),
	KEY_TRANS_ENTRY0(OTHER_SWITCH, F23,            UNKNOWN,        VK_F23,                 0,        "F23"),
	KEY_TRANS_ENTRY0(OTHER_SWITCH, F24,            UNKNOWN,        VK_F24,                 0,        "F24"),
	KEY_TRANS_ENTRY0(OTHER_SWITCH, AUDIONEXT,      NEXTTRACK,      VK_MEDIA_NEXT_TRACK,    0,        "AUDIONEXT"),
	KEY_TRANS_ENTRY0(OTHER_SWITCH, AUDIOMUTE,      MUTE,           VK_VOLUME_MUTE,         0,        "VOLUMEMUTE"),
	KEY_TRANS_ENTRY0(OTHER_SWITCH, AUDIOPLAY,      PLAYPAUSE,      VK_MEDIA_PLAY_PAUSE,    0,        "AUDIOPLAY"),
	KEY_TRANS_ENTRY0(OTHER_SWITCH, AUDIOSTOP,      MEDIASTOP,      VK_MEDIA_STOP,          0,        "AUDIOSTOP"),
	KEY_TRANS_ENTRY0(OTHER_SWITCH, VOLUMEDOWN,     VOLUMEDOWN,     VK_VOLUME_DOWN,         0,        "VOLUMEDOWN"),
	KEY_TRANS_ENTRY0(OTHER_SWITCH, VOLUMEUP,       VOLUMEUP,       VK_VOLUME_UP,           0,        "VOLUMEUP"),
	KEY_TRANS_ENTRY0(OTHER_SWITCH, AC_HOME,        WEBHOME,        VK_BROWSER_HOME,        0,        "NAVHOME"),
	KEY_TRANS_ENTRY0(OTHER_SWITCH, AC_SEARCH,      WEBSEARCH,      VK_BROWSER_SEARCH,      0,        "NAVSEARCH"),
	KEY_TRANS_ENTRY0(OTHER_SWITCH, AC_BOOKMARKS,   WEBFAVORITES,   VK_BROWSER_FAVORITES,   0,        "NAVFAVORITES"),
	KEY_TRANS_ENTRY0(OTHER_SWITCH, AC_REFRESH,     WEBREFRESH,     VK_BROWSER_REFRESH,     0,        "NAVREFRESH"),
	KEY_TRANS_ENTRY0(OTHER_SWITCH, AC_STOP,        WEBSTOP,        VK_BROWSER_STOP,        0,        "NAVSTOP"),
	KEY_TRANS_ENTRY0(OTHER_SWITCH, AC_FORWARD,     WEBFORWARD,     VK_BROWSER_FORWARD,     0,        "NAVFORWARD"),
	KEY_TRANS_ENTRY0(OTHER_SWITCH, AC_BACK,        WEBBACK,        VK_BROWSER_BACK,        0,        "NAVBACK"),
	KEY_TRANS_ENTRY0(OTHER_SWITCH, MAIL,           MAIL,           VK_LAUNCH_MAIL,         0,        "MAIL"),
	KEY_TRANS_ENTRY0(OTHER_SWITCH, MEDIASELECT,    MEDIASELECT,    VK_LAUNCH_MEDIA_SELECT, 0,        "MEDIASEL"),

	// sentinel
	KEY_TRANS_ENTRY0(INVALID,      UNKNOWN,        UNKNOWN,        0,                      0,        "INVALID")
};

// The private constructor to create the default instance
keyboard_trans_table::keyboard_trans_table()
{
	m_table = s_default_table;
	m_table_size = std::size(s_default_table);
}

#else // defined(OSD_WINDOWS) || defined(OSD_SDL)

keyboard_trans_table::keyboard_trans_table()
{
	m_table = nullptr;
	m_table_size = 0;
}

#endif // defined(OSD_WINDOWS) || defined(OSD_SDL)


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
#if defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)

input_item_id keyboard_trans_table::map_di_scancode_to_itemid(int scancode) const
{
	// scan the table for a match
	for (int tablenum = 0; tablenum < m_table_size; tablenum++)
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
		const input_item_id id = code.item_id();

		// scan the table for a match
		for (int tablenum = 0; tablenum < m_table_size; tablenum++)
			if (m_table[tablenum].mame_key == id)
				return m_table[tablenum].virtual_key;
	}
	return 0;
}

#endif // defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)

input_module_base::input_module_base(const char *type, const char* name) :
	osd_module(type, name),
	m_clock(),
	m_last_poll(timepoint_type::min()),
	m_background_input(false),
	m_options(nullptr),
	m_manager(nullptr)
{
}

int input_module_base::init(osd_interface &osd, const osd_options &options)
{
	m_options = &options;

	// don't enable background input when debugging
	m_background_input = !options.debug() && options.background_input();

	return 0;
}

void input_module_base::input_init(running_machine &machine)
{
	m_manager = &machine.input();
}

void input_module_base::poll_if_necessary(bool relative_reset)
{
	timepoint_type const now = m_clock.now();
	if (relative_reset || (now >= (m_last_poll + std::chrono::milliseconds(MIN_POLLING_INTERVAL))))
	{
		// grab the current time
		m_last_poll = now;

		before_poll();

		poll(relative_reset);
	}
}
