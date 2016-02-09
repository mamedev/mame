
#include "input_module.h"
#include "modules/osdmodule.h"

#if defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winioctl.h>
#include <tchar.h>
#undef interface

// MAME headers
#include "emu.h"
#include "osdepend.h"
#include "ui/ui.h"

// MAMEOS headers
#include "winmain.h"
#include "window.h"
#include "../../windows/input.h"

#include "input_windows.h"

/****************************************************************************
*      DirectInput compatible keyboard scan codes
****************************************************************************/
#define KEY_ESCAPE          0x01
#define KEY_1               0x02
#define KEY_2               0x03
#define KEY_3               0x04
#define KEY_4               0x05
#define KEY_5               0x06
#define KEY_6               0x07
#define KEY_7               0x08
#define KEY_8               0x09
#define KEY_9               0x0A
#define KEY_0               0x0B
#define KEY_MINUS           0x0C    /* - on main keyboard */
#define KEY_EQUALS          0x0D
#define KEY_BACK            0x0E    /* backspace */
#define KEY_TAB             0x0F
#define KEY_Q               0x10
#define KEY_W               0x11
#define KEY_E               0x12
#define KEY_R               0x13
#define KEY_T               0x14
#define KEY_Y               0x15
#define KEY_U               0x16
#define KEY_I               0x17
#define KEY_O               0x18
#define KEY_P               0x19
#define KEY_LBRACKET        0x1A
#define KEY_RBRACKET        0x1B
#define KEY_RETURN          0x1C    /* Enter on main keyboard */
#define KEY_LCONTROL        0x1D
#define KEY_A               0x1E
#define KEY_S               0x1F
#define KEY_D               0x20
#define KEY_F               0x21
#define KEY_G               0x22
#define KEY_H               0x23
#define KEY_J               0x24
#define KEY_K               0x25
#define KEY_L               0x26
#define KEY_SEMICOLON       0x27
#define KEY_APOSTROPHE      0x28
#define KEY_GRAVE           0x29    /* accent grave */
#define KEY_LSHIFT          0x2A
#define KEY_BACKSLASH       0x2B
#define KEY_Z               0x2C
#define KEY_X               0x2D
#define KEY_C               0x2E
#define KEY_V               0x2F
#define KEY_B               0x30
#define KEY_N               0x31
#define KEY_M               0x32
#define KEY_COMMA           0x33
#define KEY_PERIOD          0x34    /* . on main keyboard */
#define KEY_SLASH           0x35    /* / on main keyboard */
#define KEY_RSHIFT          0x36
#define KEY_MULTIPLY        0x37    /* * on numeric keypad */
#define KEY_LMENU           0x38    /* left Alt */
#define KEY_SPACE           0x39
#define KEY_CAPITAL         0x3A
#define KEY_F1              0x3B
#define KEY_F2              0x3C
#define KEY_F3              0x3D
#define KEY_F4              0x3E
#define KEY_F5              0x3F
#define KEY_F6              0x40
#define KEY_F7              0x41
#define KEY_F8              0x42
#define KEY_F9              0x43
#define KEY_F10             0x44
#define KEY_NUMLOCK         0x45
#define KEY_SCROLL          0x46    /* Scroll Lock */
#define KEY_NUMPAD7         0x47
#define KEY_NUMPAD8         0x48
#define KEY_NUMPAD9         0x49
#define KEY_SUBTRACT        0x4A    /* - on numeric keypad */
#define KEY_NUMPAD4         0x4B
#define KEY_NUMPAD5         0x4C
#define KEY_NUMPAD6         0x4D
#define KEY_ADD             0x4E    /* + on numeric keypad */
#define KEY_NUMPAD1         0x4F
#define KEY_NUMPAD2         0x50
#define KEY_NUMPAD3         0x51
#define KEY_NUMPAD0         0x52
#define KEY_DECIMAL         0x53    /* . on numeric keypad */
#define KEY_OEM_102         0x56    /* <> or \| on RT 102-key keyboard (Non-U.S.) */
#define KEY_F11             0x57
#define KEY_F12             0x58
#define KEY_F13             0x64    /*                     (NEC PC98) */
#define KEY_F14             0x65    /*                     (NEC PC98) */
#define KEY_F15             0x66    /*                     (NEC PC98) */
#define KEY_KANA            0x70    /* (Japanese keyboard)            */
#define KEY_ABNT_C1         0x73    /* /? on Brazilian keyboard */
#define KEY_CONVERT         0x79    /* (Japanese keyboard)            */
#define KEY_NOCONVERT       0x7B    /* (Japanese keyboard)            */
#define KEY_YEN             0x7D    /* (Japanese keyboard)            */
#define KEY_ABNT_C2         0x7E    /* Numpad . on Brazilian keyboard */
#define KEY_NUMPADEQUALS    0x8D    /* = on numeric keypad (NEC PC98) */
#define KEY_PREVTRACK       0x90    /* Previous Track (DIK_CIRCUMFLEX on Japanese keyboard) */
#define KEY_AT              0x91    /*                     (NEC PC98) */
#define KEY_COLON           0x92    /*                     (NEC PC98) */
#define KEY_UNDERLINE       0x93    /*                     (NEC PC98) */
#define KEY_KANJI           0x94    /* (Japanese keyboard)            */
#define KEY_STOP            0x95    /*                     (NEC PC98) */
#define KEY_AX              0x96    /*                     (Japan AX) */
#define KEY_UNLABELED       0x97    /*                        (J3100) */
#define KEY_NEXTTRACK       0x99    /* Next Track */
#define KEY_NUMPADENTER     0x9C    /* Enter on numeric keypad */
#define KEY_RCONTROL        0x9D
#define KEY_MUTE            0xA0    /* Mute */
#define KEY_CALCULATOR      0xA1    /* Calculator */
#define KEY_PLAYPAUSE       0xA2    /* Play / Pause */
#define KEY_MEDIASTOP       0xA4    /* Media Stop */
#define KEY_VOLUMEDOWN      0xAE    /* Volume - */
#define KEY_VOLUMEUP        0xB0    /* Volume + */
#define KEY_WEBHOME         0xB2    /* Web home */
#define KEY_NUMPADCOMMA     0xB3    /* , on numeric keypad (NEC PC98) */
#define KEY_DIVIDE          0xB5    /* / on numeric keypad */
#define KEY_SYSRQ           0xB7
#define KEY_RMENU           0xB8    /* right Alt */
#define KEY_PAUSE           0xC5    /* Pause */
#define KEY_HOME            0xC7    /* Home on arrow keypad */
#define KEY_UP              0xC8    /* UpArrow on arrow keypad */
#define KEY_PRIOR           0xC9    /* PgUp on arrow keypad */
#define KEY_LEFT            0xCB    /* LeftArrow on arrow keypad */
#define KEY_RIGHT           0xCD    /* RightArrow on arrow keypad */
#define KEY_END             0xCF    /* End on arrow keypad */
#define KEY_DOWN            0xD0    /* DownArrow on arrow keypad */
#define KEY_NEXT            0xD1    /* PgDn on arrow keypad */
#define KEY_INSERT          0xD2    /* Insert on arrow keypad */
#define KEY_DELETE          0xD3    /* Delete on arrow keypad */
#define KEY_LWIN            0xDB    /* Left Windows key */
#define KEY_RWIN            0xDC    /* Right Windows key */
#define KEY_APPS            0xDD    /* AppMenu key */
#define KEY_POWER           0xDE    /* System Power */
#define KEY_SLEEP           0xDF    /* System Sleep */
#define KEY_WAKE            0xE3    /* System Wake */
#define KEY_WEBSEARCH       0xE5    /* Web Search */
#define KEY_WEBFAVORITES    0xE6    /* Web Favorites */
#define KEY_WEBREFRESH      0xE7    /* Web Refresh */
#define KEY_WEBSTOP         0xE8    /* Web Stop */
#define KEY_WEBFORWARD      0xE9    /* Web Forward */
#define KEY_WEBBACK         0xEA    /* Web Back */
#define KEY_MYCOMPUTER      0xEB    /* My Computer */
#define KEY_MAIL            0xEC    /* Mail */
#define KEY_MEDIASELECT     0xED    /* Media Select */

//============================================================
//  Keyboard translation table
//============================================================

#define KEY_TRANS_ENTRY(mame, sc, virtual, ascii) { ITEM_ID_##mame, KEY_##sc, virtual, ascii, "ITEM_ID_"#mame }

const key_trans_entry keyboard_trans_table::s_table[] =
{
	//              MAME key      scancode      virtual key     ascii
	KEY_TRANS_ENTRY(ESC,          ESCAPE,         VK_ESCAPE,      27),
	KEY_TRANS_ENTRY(1,            1,              '1',            '1'),
	KEY_TRANS_ENTRY(2,            2,              '2',            '2'),
	KEY_TRANS_ENTRY(3,            3,              '3',            '3'),
	KEY_TRANS_ENTRY(4,            4,              '4',            '4'),
	KEY_TRANS_ENTRY(5,            5,              '5',            '5'),
	KEY_TRANS_ENTRY(6,            6,              '6',            '6'),
	KEY_TRANS_ENTRY(7,            7,              '7',            '7'),
	KEY_TRANS_ENTRY(8,            8,              '8',            '8'),
	KEY_TRANS_ENTRY(9,            9,              '9',            '9'),
	KEY_TRANS_ENTRY(0,            0,              '0',            '0'),
	KEY_TRANS_ENTRY(MINUS,        MINUS,          VK_OEM_MINUS,   '-'),
	KEY_TRANS_ENTRY(EQUALS,       EQUALS,         VK_OEM_PLUS,    '='),
	KEY_TRANS_ENTRY(BACKSPACE,    BACK,           VK_BACK,        8),
	KEY_TRANS_ENTRY(TAB,          TAB,            VK_TAB,         9),
	KEY_TRANS_ENTRY(Q,            Q,              'Q',            'Q'),
	KEY_TRANS_ENTRY(W,            W,              'W',            'W'),
	KEY_TRANS_ENTRY(E,            E,              'E',            'E'),
	KEY_TRANS_ENTRY(R,            R,              'R',            'R'),
	KEY_TRANS_ENTRY(T,            T,              'T',            'T'),
	KEY_TRANS_ENTRY(Y,            Y,              'Y',            'Y'),
	KEY_TRANS_ENTRY(U,            U,              'U',            'U'),
	KEY_TRANS_ENTRY(I,            I,              'I',            'I'),
	KEY_TRANS_ENTRY(O,            O,              'O',            'O'),
	KEY_TRANS_ENTRY(P,            P,              'P',            'P'),
	KEY_TRANS_ENTRY(OPENBRACE,    LBRACKET,       VK_OEM_4,       '['),
	KEY_TRANS_ENTRY(CLOSEBRACE,   RBRACKET,       VK_OEM_6,       ']'),
	KEY_TRANS_ENTRY(ENTER,        RETURN,         VK_RETURN,      13),
	KEY_TRANS_ENTRY(LCONTROL,     LCONTROL,       VK_LCONTROL,    0),
	KEY_TRANS_ENTRY(A,            A,              'A',            'A'),
	KEY_TRANS_ENTRY(S,            S,              'S',            'S'),
	KEY_TRANS_ENTRY(D,            D,              'D',            'D'),
	KEY_TRANS_ENTRY(F,            F,              'F',            'F'),
	KEY_TRANS_ENTRY(G,            G,              'G',            'G'),
	KEY_TRANS_ENTRY(H,            H,              'H',            'H'),
	KEY_TRANS_ENTRY(J,            J,              'J',            'J'),
	KEY_TRANS_ENTRY(K,            K,              'K',            'K'),
	KEY_TRANS_ENTRY(L,            L,              'L',            'L'),
	KEY_TRANS_ENTRY(COLON,        SEMICOLON,      VK_OEM_1,       ';'),
	KEY_TRANS_ENTRY(QUOTE,        APOSTROPHE,     VK_OEM_7,       '\''),
	KEY_TRANS_ENTRY(TILDE,        GRAVE,          VK_OEM_3,       '`'),
	KEY_TRANS_ENTRY(LSHIFT,       LSHIFT,         VK_LSHIFT,      0),
	KEY_TRANS_ENTRY(BACKSLASH,    BACKSLASH,      VK_OEM_5,       '\\'),
	KEY_TRANS_ENTRY(BACKSLASH2,   OEM_102,        VK_OEM_102,     '<'),
	KEY_TRANS_ENTRY(Z,            Z,              'Z',            'Z'),
	KEY_TRANS_ENTRY(X,            X,              'X',            'X'),
	KEY_TRANS_ENTRY(C,            C,              'C',            'C'),
	KEY_TRANS_ENTRY(V,            V,              'V',            'V'),
	KEY_TRANS_ENTRY(B,            B,              'B',            'B'),
	KEY_TRANS_ENTRY(N,            N,              'N',            'N'),
	KEY_TRANS_ENTRY(M,            M,              'M',            'M'),
	KEY_TRANS_ENTRY(COMMA,        COMMA,          VK_OEM_COMMA,   ','),
	KEY_TRANS_ENTRY(STOP,         PERIOD,         VK_OEM_PERIOD,  '.'),
	KEY_TRANS_ENTRY(SLASH,        SLASH,          VK_OEM_2,       '/'),
	KEY_TRANS_ENTRY(RSHIFT,       RSHIFT,         VK_RSHIFT,      0),
	KEY_TRANS_ENTRY(ASTERISK,     MULTIPLY,       VK_MULTIPLY,    '*'),
	KEY_TRANS_ENTRY(LALT,         LMENU,          VK_LMENU,       0),
	KEY_TRANS_ENTRY(SPACE,        SPACE,          VK_SPACE,       ' '),
	KEY_TRANS_ENTRY(CAPSLOCK,     CAPITAL,        VK_CAPITAL,     0),
	KEY_TRANS_ENTRY(F1,           F1,             VK_F1,          0),
	KEY_TRANS_ENTRY(F2,           F2,             VK_F2,          0),
	KEY_TRANS_ENTRY(F3,           F3,             VK_F3,          0),
	KEY_TRANS_ENTRY(F4,           F4,             VK_F4,          0),
	KEY_TRANS_ENTRY(F5,           F5,             VK_F5,          0),
	KEY_TRANS_ENTRY(F6,           F6,             VK_F6,          0),
	KEY_TRANS_ENTRY(F7,           F7,             VK_F7,          0),
	KEY_TRANS_ENTRY(F8,           F8,             VK_F8,          0),
	KEY_TRANS_ENTRY(F9,           F9,             VK_F9,          0),
	KEY_TRANS_ENTRY(F10,          F10,            VK_F10,         0),
	KEY_TRANS_ENTRY(NUMLOCK,      NUMLOCK,        VK_NUMLOCK,     0),
	KEY_TRANS_ENTRY(SCRLOCK,      SCROLL,         VK_SCROLL,      0),
	KEY_TRANS_ENTRY(7_PAD,        NUMPAD7,        VK_NUMPAD7,     0),
	KEY_TRANS_ENTRY(8_PAD,        NUMPAD8,        VK_NUMPAD8,     0),
	KEY_TRANS_ENTRY(9_PAD,        NUMPAD9,        VK_NUMPAD9,     0),
	KEY_TRANS_ENTRY(MINUS_PAD,    SUBTRACT,       VK_SUBTRACT,    0),
	KEY_TRANS_ENTRY(4_PAD,        NUMPAD4,        VK_NUMPAD4,     0),
	KEY_TRANS_ENTRY(5_PAD,        NUMPAD5,        VK_NUMPAD5,     0),
	KEY_TRANS_ENTRY(6_PAD,        NUMPAD6,        VK_NUMPAD6,     0),
	KEY_TRANS_ENTRY(PLUS_PAD,     ADD,            VK_ADD,         0),
	KEY_TRANS_ENTRY(1_PAD,        NUMPAD1,        VK_NUMPAD1,     0),
	KEY_TRANS_ENTRY(2_PAD,        NUMPAD2,        VK_NUMPAD2,     0),
	KEY_TRANS_ENTRY(3_PAD,        NUMPAD3,        VK_NUMPAD3,     0),
	KEY_TRANS_ENTRY(0_PAD,        NUMPAD0,        VK_NUMPAD0,     0),
	KEY_TRANS_ENTRY(DEL_PAD,      DECIMAL,        VK_DECIMAL,     0),
	KEY_TRANS_ENTRY(F11,          F11,            VK_F11,         0),
	KEY_TRANS_ENTRY(F12,          F12,            VK_F12,         0),
	KEY_TRANS_ENTRY(F13,          F13,            VK_F13,         0),
	KEY_TRANS_ENTRY(F14,          F14,            VK_F14,         0),
	KEY_TRANS_ENTRY(F15,          F15,            VK_F15,         0),
	KEY_TRANS_ENTRY(ENTER_PAD,    NUMPADENTER,    VK_RETURN,      0),
	KEY_TRANS_ENTRY(RCONTROL,     RCONTROL,       VK_RCONTROL,    0),
	KEY_TRANS_ENTRY(SLASH_PAD,    DIVIDE,         VK_DIVIDE,      0),
	KEY_TRANS_ENTRY(PRTSCR,       SYSRQ,          0,              0),
	KEY_TRANS_ENTRY(RALT,         RMENU,          VK_RMENU,       0),
	KEY_TRANS_ENTRY(HOME,         HOME,           VK_HOME,        0),
	KEY_TRANS_ENTRY(UP,           UP,             VK_UP,          0),
	KEY_TRANS_ENTRY(PGUP,         PRIOR,          VK_PRIOR,       0),
	KEY_TRANS_ENTRY(LEFT,         LEFT,           VK_LEFT,        0),
	KEY_TRANS_ENTRY(RIGHT,        RIGHT,          VK_RIGHT,       0),
	KEY_TRANS_ENTRY(END,          END,            VK_END,         0),
	KEY_TRANS_ENTRY(DOWN,         DOWN,           VK_DOWN,        0),
	KEY_TRANS_ENTRY(PGDN,         NEXT,           VK_NEXT,        0),
	KEY_TRANS_ENTRY(INSERT,       INSERT,         VK_INSERT,      0),
	KEY_TRANS_ENTRY(DEL,          DELETE,         VK_DELETE,      0),
	KEY_TRANS_ENTRY(LWIN,         LWIN,           VK_LWIN,        0),
	KEY_TRANS_ENTRY(RWIN,         RWIN,           VK_RWIN,        0),
	KEY_TRANS_ENTRY(MENU,         APPS,           VK_APPS,        0),
	KEY_TRANS_ENTRY(PAUSE,        PAUSE,          VK_PAUSE,       0),
	{ ITEM_ID_CANCEL,       0,              VK_CANCEL,      0, "ITEM_ID_CANCEL" },

	// New keys introduced in Windows 2000. These have no MAME codes to
	// preserve compatibility with old config files that may refer to them
	// as e.g. FORWARD instead of e.g. KEYCODE_WEBFORWARD. They need table
	// entries anyway because otherwise they aren't recognized when
	// GetAsyncKeyState polling is used (as happens currently when MAME is
	// paused). Some codes are missing because the mapping to vkey codes
	// isn't clear, and MapVirtualKey is no help.

	KEY_TRANS_ENTRY(OTHER_SWITCH, MUTE,           VK_VOLUME_MUTE,         0),
	KEY_TRANS_ENTRY(OTHER_SWITCH, VOLUMEDOWN,     VK_VOLUME_DOWN,         0),
	KEY_TRANS_ENTRY(OTHER_SWITCH, VOLUMEUP,       VK_VOLUME_UP,           0),
	KEY_TRANS_ENTRY(OTHER_SWITCH, WEBHOME,        VK_BROWSER_HOME,        0),
	KEY_TRANS_ENTRY(OTHER_SWITCH, WEBSEARCH,      VK_BROWSER_SEARCH,      0),
	KEY_TRANS_ENTRY(OTHER_SWITCH, WEBFAVORITES,   VK_BROWSER_FAVORITES,   0),
	KEY_TRANS_ENTRY(OTHER_SWITCH, WEBREFRESH,     VK_BROWSER_REFRESH,     0),
	KEY_TRANS_ENTRY(OTHER_SWITCH, WEBSTOP,        VK_BROWSER_STOP,        0),
	KEY_TRANS_ENTRY(OTHER_SWITCH, WEBFORWARD,     VK_BROWSER_FORWARD,     0),
	KEY_TRANS_ENTRY(OTHER_SWITCH, WEBBACK,        VK_BROWSER_BACK,        0),
	KEY_TRANS_ENTRY(OTHER_SWITCH, MAIL,           VK_LAUNCH_MAIL,         0),
	KEY_TRANS_ENTRY(OTHER_SWITCH, MEDIASELECT,    VK_LAUNCH_MEDIA_SELECT, 0),
};

keyboard_trans_table::keyboard_trans_table()
{
	m_table_size = ARRAY_LENGTH(keyboard_trans_table::s_table);
}

int wininput_vkey_for_mame_code(input_code code)
{
	return keyboard_trans_table::instance().vkey_for_mame_code(code);
}

INT32 generic_button_get_state(void *device_internal, void *item_internal)
{
	device_info *devinfo = (device_info *)device_internal;
	BYTE *itemdata = (BYTE *)item_internal;

	// return the current state
	downcast<wininput_module&>(devinfo->module()).poll_if_necessary(devinfo->machine());
	return *itemdata >> 7;
}

INT32 generic_axis_get_state(void *device_internal, void *item_internal)
{
	device_info *devinfo = (device_info *)device_internal;
	LONG *axisdata = (LONG *)item_internal;

	// return the current state
	downcast<wininput_module&>(devinfo->module()).poll_if_necessary(devinfo->machine());
	return *axisdata;
}

BOOL windows_osd_interface::should_hide_mouse()
{
	BOOL hidemouse = FALSE;
	hidemouse |= downcast<wininput_module*>(m_keyboard_input)->should_hide_mouse();
	hidemouse |= downcast<wininput_module*>(m_mouse_input)->should_hide_mouse();
	hidemouse |= downcast<wininput_module*>(m_lightgun_input)->should_hide_mouse();
	hidemouse |= downcast<wininput_module*>(m_joystick_input)->should_hide_mouse();
	return hidemouse;
}

BOOL windows_osd_interface::handle_input_event(input_event eventid, void* eventdata)
{
	BOOL handled = FALSE;
	handled |= downcast<wininput_module*>(m_keyboard_input)->handle_input_event(eventid, eventdata);
	handled |= downcast<wininput_module*>(m_mouse_input)->handle_input_event(eventid, eventdata);
	handled |= downcast<wininput_module*>(m_lightgun_input)->handle_input_event(eventid, eventdata);
	handled |= downcast<wininput_module*>(m_joystick_input)->handle_input_event(eventid, eventdata);
	return handled;
}

void windows_osd_interface::poll_input(running_machine &machine)
{
	downcast<wininput_module*>(m_keyboard_input)->poll_if_necessary(machine);
	downcast<wininput_module*>(m_mouse_input)->poll_if_necessary(machine);
	downcast<wininput_module*>(m_lightgun_input)->poll_if_necessary(machine);
	downcast<wininput_module*>(m_joystick_input)->poll_if_necessary(machine);
}

//============================================================
//  default_button_name
//============================================================

const TCHAR *default_button_name(int which)
{
	static TCHAR buffer[20];
	_sntprintf(buffer, ARRAY_LENGTH(buffer), TEXT("B%d"), which);
	return buffer;
}

input_item_id keyboard_trans_table::map_scancode_to_itemid(int scancode)
{
	int tablenum;

	// scan the table for a match
	for (tablenum = 0; tablenum < m_table_size; tablenum++)
		if (s_table[tablenum].scan_code == scancode)
			return (input_item_id)s_table[tablenum].mame_key;

	// default to an "other" switch
	return ITEM_ID_OTHER_SWITCH;
}

int keyboard_trans_table::lookup_mame_index(const char *scode)
{
	for (int i = 0; i < m_table_size; i++)
	{
		if (!strcmp(scode, s_table[i].mame_key_name))
			return i;
	}
	return -1;
}

input_item_id keyboard_trans_table::lookup_mame_code(const char *scode)
{
	int const index = lookup_mame_index(scode);
	if (index >= 0)
		return s_table[index].mame_key;
	else
		return ITEM_ID_INVALID;
}

//============================================================
//  wininput_vkey_for_mame_code
//============================================================

int keyboard_trans_table::vkey_for_mame_code(input_code code)
{
	// only works for keyboard switches
	if (code.device_class() == DEVICE_CLASS_KEYBOARD && code.item_class() == ITEM_CLASS_SWITCH)
	{
		input_item_id id = code.item_id();
		int tablenum;

		// scan the table for a match
		for (tablenum = 0; tablenum < m_table_size; tablenum++)
			if (s_table[tablenum].mame_key == id)
				return s_table[tablenum].virtual_key;
	}
	return 0;
}

//============================================================
//  customize_input_type_list
//============================================================

void windows_osd_interface::customize_input_type_list(simple_list<input_type_entry> &typelist)
{
	input_type_entry *entry;
	const char* uimode;

	// loop over the defaults
	for (entry = typelist.first(); entry != NULL; entry = entry->next())
		switch (entry->type())
		{
			// disable the config menu if the ALT key is down
			// (allows ALT-TAB to switch between windows apps)
		case IPT_UI_CONFIGURE:
			entry->defseq(SEQ_TYPE_STANDARD).set(KEYCODE_TAB, input_seq::not_code, KEYCODE_LALT, input_seq::not_code, KEYCODE_RALT);
			break;

			// configurable UI mode switch
		case IPT_UI_TOGGLE_UI:
			uimode = options().ui_mode_key();
			if (strcmp(uimode, "auto"))
			{
				std::string fullmode = "ITEM_ID_";
				fullmode += uimode;
				input_item_id const mameid_code = keyboard_trans_table::instance().lookup_mame_code(fullmode.c_str());
				if (ITEM_ID_INVALID != mameid_code)
				{
					input_code const ui_code = input_code(DEVICE_CLASS_KEYBOARD, 0, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, input_item_id(mameid_code));
					entry->defseq(SEQ_TYPE_STANDARD).set(ui_code);
				}
			}
			break;

			// alt-enter for fullscreen
		case IPT_OSD_1:
			entry->configure_osd("TOGGLE_FULLSCREEN", "Toggle Fullscreen");
			entry->defseq(SEQ_TYPE_STANDARD).set(KEYCODE_ENTER, KEYCODE_LALT, input_seq::or_code, KEYCODE_ENTER, KEYCODE_RALT);
			break;

			// lalt-F12 for fullscreen snap (HLSL)
		case IPT_OSD_2:
			entry->configure_osd("RENDER_SNAP", "Take Rendered Snapshot");
			entry->defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F12, KEYCODE_LALT, input_seq::not_code, KEYCODE_LSHIFT);
			break;
			// add a NOT-lalt to our default F12
		case IPT_UI_SNAPSHOT: // emu/input.c: input_seq(KEYCODE_F12, input_seq::not_code, KEYCODE_LSHIFT)
			entry->defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F12, input_seq::not_code, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_LALT);
			break;

			// lshift-lalt-F12 for fullscreen video (HLSL)
		case IPT_OSD_3:
			entry->configure_osd("RENDER_AVI", "Record Rendered Video");
			entry->defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F12, KEYCODE_LSHIFT, KEYCODE_LALT);
			break;
			// add a NOT-lalt to our default shift-F12
		case IPT_UI_RECORD_MOVIE: // emu/input.c: input_seq(KEYCODE_F12, KEYCODE_LSHIFT)
			entry->defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F12, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_LALT);
			break;

			// add a NOT-lalt to write timecode file
		case IPT_UI_TIMECODE: // emu/input.c: input_seq(KEYCODE_F12, input_seq::not_code, KEYCODE_LSHIFT)
			entry->defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F12, input_seq::not_code, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_LALT);
			break;

			// lctrl-lalt-F5 to toggle post-processing
		case IPT_OSD_4:
			entry->configure_osd("POST_PROCESS", "Toggle Post-Processing");
			entry->defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F5, KEYCODE_LALT, KEYCODE_LCONTROL);
			break;
			// add a NOT-lctrl-lalt to our default F5
		case IPT_UI_TOGGLE_DEBUG: // emu/input.c: input_seq(KEYCODE_F5)
			entry->defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F5, input_seq::not_code, KEYCODE_LCONTROL, input_seq::not_code, KEYCODE_LALT);
			break;

			// leave everything else alone
		default:
			break;
		}
}

INT32 normalize_absolute_axis(INT32 raw, INT32 rawmin, INT32 rawmax)
{
	INT32 center = (rawmax + rawmin) / 2;

	// make sure we have valid data
	if (rawmin >= rawmax)
		return raw;

	// above center
	if (raw >= center)
	{
		INT32 result = (INT64)(raw - center) * (INT64)INPUT_ABSOLUTE_MAX / (INT64)(rawmax - center);
		return MIN(result, INPUT_ABSOLUTE_MAX);
	}

	// below center
	else
	{
		INT32 result = -((INT64)(center - raw) * (INT64)-INPUT_ABSOLUTE_MIN / (INT64)(center - rawmin));
		return MAX(result, INPUT_ABSOLUTE_MIN);
	}
}

#endif