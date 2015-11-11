// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  input.c - Win32 implementation of MAME input routines
//
//============================================================

// For testing purposes: force DirectInput
#define FORCE_DIRECTINPUT   0

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winioctl.h>
#include <tchar.h>

// undef WINNT for dinput.h to prevent duplicate definition
#undef WINNT
#include <dinput.h>
#undef interface

// standard C headers
#include <conio.h>
#include <ctype.h>
#include <stddef.h>

// MAME headers
#include "emu.h"
#include "osdepend.h"
#include "ui/ui.h"

// MAMEOS headers
#include "winmain.h"
#include "window.h"
#include "input.h"
#include "video.h"
#include "strconv.h"
#include "config.h"

#include "winutil.h"

//============================================================
//  PARAMETERS
//============================================================

enum
{
	POVDIR_LEFT = 0,
	POVDIR_RIGHT,
	POVDIR_UP,
	POVDIR_DOWN
};

#define MAX_KEYS            256



//============================================================
//  MACROS
//============================================================

#define STRUCTSIZE(x)       ((dinput_version == 0x0300) ? sizeof(x##_DX3) : sizeof(x))

#ifdef UNICODE
#define UNICODE_SUFFIX      "W"
#else
#define UNICODE_SUFFIX      "A"
#endif



//============================================================
//  TYPEDEFS
//============================================================

// state information for a keyboard; DirectInput state must be first element
struct keyboard_state
{
	UINT8                   state[MAX_KEYS];
	INT8                    oldkey[MAX_KEYS];
	INT8                    currkey[MAX_KEYS];
};


// state information for a mouse; DirectInput state must be first element
struct mouse_state
{
	DIMOUSESTATE2           state;
	LONG                    raw_x, raw_y, raw_z;
};


// state information for a joystick; DirectInput state must be first element
struct joystick_state
{
	DIJOYSTATE              state;
	LONG                    rangemin[8];
	LONG                    rangemax[8];
};


// DirectInput-specific information about a device
struct dinput_device_info
{
	LPDIRECTINPUTDEVICE     device;
	LPDIRECTINPUTDEVICE2    device2;
	DIDEVCAPS               caps;
	LPCDIDATAFORMAT         format;
};


// RawInput-specific information about a device
struct rawinput_device_info
{
	HANDLE                  device;
};


// generic device information
class device_info
{
public:
	device_info(running_machine &machine)
		: head(NULL), next(NULL), name(NULL), poll(NULL), device(NULL), m_machine(machine) { }

	running_machine &machine() const { return m_machine; }

	// device information
	device_info **          head;
	device_info *           next;
	const char *            name;
	void                    (*poll)(device_info *info);

	// MAME information
	input_device *          device;

	// device state
	union
	{
		keyboard_state      keyboard;
		mouse_state         mouse;
		joystick_state      joystick;
	};

	// DirectInput/RawInput-specific state
	dinput_device_info      dinput;
	rawinput_device_info    rawinput;

private:
	running_machine &       m_machine;
};


// RawInput APIs
typedef /*WINUSERAPI*/ INT (WINAPI *get_rawinput_device_list_ptr)(OUT PRAWINPUTDEVICELIST pRawInputDeviceList, IN OUT PINT puiNumDevices, IN UINT cbSize);
typedef /*WINUSERAPI*/ INT (WINAPI *get_rawinput_data_ptr)(IN HRAWINPUT hRawInput, IN UINT uiCommand, OUT LPVOID pData, IN OUT PINT pcbSize, IN UINT cbSizeHeader);
typedef /*WINUSERAPI*/ INT (WINAPI *get_rawinput_device_info_ptr)(IN HANDLE hDevice, IN UINT uiCommand, OUT LPVOID pData, IN OUT PINT pcbSize);
typedef /*WINUSERAPI*/ BOOL (WINAPI *register_rawinput_devices_ptr)(IN PCRAWINPUTDEVICE pRawInputDevices, IN UINT uiNumDevices, IN UINT cbSize);



//============================================================
//  LOCAL VARIABLES
//============================================================

// global states
static bool                 input_enabled;
static osd_lock *           input_lock;
static bool                 input_paused;
static DWORD                last_poll;

// DirectInput variables
static LPDIRECTINPUT        dinput;
static int                  dinput_version;

// RawInput variables
static get_rawinput_device_list_ptr     get_rawinput_device_list;
static get_rawinput_data_ptr            get_rawinput_data;
static get_rawinput_device_info_ptr     get_rawinput_device_info;
static register_rawinput_devices_ptr    register_rawinput_devices;
static bool                             global_inputs_enabled;

// keyboard states
static bool                 keyboard_win32_reported_key_down;
static device_info *        keyboard_list;

// mouse states
static bool                 mouse_enabled;
static device_info *        mouse_list;

// lightgun states
static UINT8                lightgun_shared_axis_mode;
static bool                 lightgun_enabled;
static device_info *        lightgun_list;

// joystick states
static device_info *        joystick_list;

// default axis names
static const TCHAR *const default_axis_name[] =
{
	TEXT("X"), TEXT("Y"), TEXT("Z"), TEXT("RX"),
	TEXT("RY"), TEXT("RZ"), TEXT("SL1"), TEXT("SL2")
};



//============================================================
//  PROTOTYPES
//============================================================

// device list management
static void device_list_poll_devices(device_info *devlist_head);
static void device_list_reset_devices(device_info *devlist_head);

// generic device management
static device_info *generic_device_alloc(running_machine &machine, device_info **devlist_head_ptr, const TCHAR *name);
static void generic_device_free(device_info *devinfo);
static int generic_device_index(device_info *devlist_head, device_info *devinfo);
static void generic_device_reset(device_info *devinfo);
static INT32 generic_button_get_state(void *device_internal, void *item_internal);
static INT32 generic_axis_get_state(void *device_internal, void *item_internal);

// Win32-specific input code
static void win32_init(running_machine &machine);
static void win32_exit(running_machine &machine);
static void win32_keyboard_poll(device_info *devinfo);
static void win32_lightgun_poll(device_info *devinfo);

// DirectInput-specific code
static void dinput_init(running_machine &machine);
static void dinput_exit(running_machine &machine);
static HRESULT dinput_set_dword_property(LPDIRECTINPUTDEVICE device, REFGUID property_guid, DWORD object, DWORD how, DWORD value);
static device_info *dinput_device_create(running_machine &machine, device_info **devlist_head_ptr, LPCDIDEVICEINSTANCE instance, LPCDIDATAFORMAT format1, LPCDIDATAFORMAT format2, DWORD cooperative_level);
static void dinput_device_release(device_info *devinfo);
static char *dinput_device_item_name(device_info *devinfo, int offset, const TCHAR *defstring, const TCHAR *suffix);
static HRESULT dinput_device_poll(device_info *devinfo);
static BOOL CALLBACK dinput_keyboard_enum(LPCDIDEVICEINSTANCE instance, LPVOID ref);
static void dinput_keyboard_poll(device_info *devinfo);
static BOOL CALLBACK dinput_mouse_enum(LPCDIDEVICEINSTANCE instance, LPVOID ref);
static void dinput_mouse_poll(device_info *devinfo);
static BOOL CALLBACK dinput_joystick_enum(LPCDIDEVICEINSTANCE instance, LPVOID ref);
static void dinput_joystick_poll(device_info *devinfo);
static INT32 dinput_joystick_pov_get_state(void *device_internal, void *item_internal);

// RawInput-specific code
static void rawinput_init(running_machine &machine);
static void rawinput_exit(running_machine &machine);
static device_info *rawinput_device_create(running_machine &machine, device_info **devlist_head_ptr, PRAWINPUTDEVICELIST device);
static void rawinput_device_release(device_info *info);
static TCHAR *rawinput_device_improve_name(TCHAR *name);
static void rawinput_keyboard_enum(running_machine &machine, PRAWINPUTDEVICELIST device);
static void rawinput_keyboard_update(HANDLE device, RAWKEYBOARD *data);
static void rawinput_mouse_enum(running_machine &machine, PRAWINPUTDEVICELIST device);
static void rawinput_mouse_update(HANDLE device, RAWMOUSE *data);
static void rawinput_mouse_poll(device_info *devinfo);

// misc utilities
static TCHAR *reg_query_string(HKEY key, const TCHAR *path);
static const TCHAR *default_button_name(int which);
static const TCHAR *default_pov_name(int which);



//============================================================
//  KEYBOARD/JOYSTICK LIST
//============================================================

// master keyboard translation table
struct key_trans_entry {
	input_item_id   mame_key;
	INT32           di_key;
	unsigned char   virtual_key;
	char            ascii_key;
	char const  *   mame_key_name;
};
#define KEY_TRANS_ENTRY(mame, di, virtual, ascii) { ITEM_ID_##mame, DIK_##di, virtual, ascii, "ITEM_ID_"#mame }
static const key_trans_entry win_key_trans_table[] =
{
	//              MAME key      dinput key      virtual key     ascii
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
	{       ITEM_ID_CANCEL,       0,              VK_CANCEL,      0, "ITEM_ID_CANCEL" },

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


//============================================================
//  INLINE FUNCTIONS
//============================================================

INLINE void poll_if_necessary(running_machine &machine)
{
	// make sure we poll at least once every 1/4 second
	if (GetTickCount() > last_poll + 1000 / 4)
		wininput_poll(machine);
}


INLINE input_item_id keyboard_map_scancode_to_itemid(int scancode)
{
	int tablenum;

	// scan the table for a match
	for (tablenum = 0; tablenum < ARRAY_LENGTH(win_key_trans_table); tablenum++)
		if (win_key_trans_table[tablenum].di_key == scancode)
			return (input_item_id)win_key_trans_table[tablenum].mame_key;

	// default to an "other" switch
	return ITEM_ID_OTHER_SWITCH;
}


INLINE INT32 normalize_absolute_axis(INT32 raw, INT32 rawmin, INT32 rawmax)
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



//============================================================
//  input_init
//============================================================

bool windows_osd_interface::input_init()
{
	// allocate a lock for input synchronizations, since messages sometimes come from another thread
	input_lock = osd_lock_alloc();
	assert_always(input_lock != NULL, "Failed to allocate input_lock");

	// decode the options
	lightgun_shared_axis_mode = downcast<windows_options &>(machine().options()).dual_lightgun();

	// initialize RawInput and DirectInput (RawInput first so we can fall back)
	rawinput_init(machine());
	dinput_init(machine());
	win32_init(machine());

	// poll once to get the initial states
	input_enabled = true;
	wininput_poll(machine());
	return true;
}


//============================================================
//  wininput_pause
//============================================================

void windows_osd_interface::input_pause()
{
	// keep track of the paused state
	input_paused = true;
}

void windows_osd_interface::input_resume()
{
	// keep track of the paused state
	input_paused = false;
}


//============================================================
//  wininput_exit
//============================================================

void windows_osd_interface::input_exit()
{
	// acquire the lock and turn off input (this ensures everyone is done)
	if (input_lock != NULL)
	{
		osd_lock_acquire(input_lock);
		input_enabled = false;
		osd_lock_release(input_lock);

		// free the lock
		osd_lock_free(input_lock);
	}
}


//============================================================
//  wininput_poll
//============================================================

void wininput_poll(running_machine &machine)
{
	// ignore if not enabled
	if (input_enabled)
	{
		// remember when this happened
		last_poll = GetTickCount();

		// periodically process events, in case they're not coming through
		// this also will make sure the mouse state is up-to-date
		winwindow_process_events_periodic(machine);

		// track if mouse/lightgun is enabled, for mouse hiding purposes
		mouse_enabled = machine.input().device_class(DEVICE_CLASS_MOUSE).enabled();
		lightgun_enabled = machine.input().device_class(DEVICE_CLASS_LIGHTGUN).enabled();
	}

	bool polldevices = input_enabled && (global_inputs_enabled || winwindow_has_focus());

	// poll all of the devices
	if (polldevices)
	{
		device_list_poll_devices(keyboard_list);
		device_list_poll_devices(mouse_list);
		device_list_poll_devices(lightgun_list);
		device_list_poll_devices(joystick_list);
	}
	else
	{
		device_list_reset_devices(keyboard_list);
		device_list_reset_devices(mouse_list);
		device_list_reset_devices(lightgun_list);
		device_list_reset_devices(joystick_list);
	}
}


//============================================================
//  wininput_should_hide_mouse
//============================================================

bool wininput_should_hide_mouse(void)
{
	// if we are paused or disabled, no
	if (input_paused || !input_enabled)
		return false;

	// if neither mice nor lightguns enabled in the core, then no
	if (!mouse_enabled && !lightgun_enabled)
		return false;

	// if the window has a menu, no
	if (win_window_list != NULL && win_window_list->win_has_menu())
		return false;

	// otherwise, yes
	return true;
}


//============================================================
//  wininput_handle_mouse_button
//============================================================

BOOL wininput_handle_mouse_button(int button, int down, int x, int y)
{
	device_info *devinfo;

	// ignore if not enabled
	if (!input_enabled)
		return FALSE;

	// only need this for shared axis hack
	if (!lightgun_shared_axis_mode || button >= 4)
		return FALSE;

	// choose a device based on the button
	devinfo = lightgun_list;
	if (button >= 2 && devinfo != NULL)
	{
		button -= 2;
		devinfo = devinfo->next;
	}

	// take the lock
	osd_lock_acquire(input_lock);

	// set the button state
	devinfo->mouse.state.rgbButtons[button] = down ? 0x80 : 0x00;
	if (down)
	{
		RECT client_rect;
		POINT mousepos;

		// get the position relative to the window
		GetClientRect(win_window_list->m_hwnd, &client_rect);
		mousepos.x = x;
		mousepos.y = y;
		ScreenToClient(win_window_list->m_hwnd, &mousepos);

		// convert to absolute coordinates
		devinfo->mouse.state.lX = normalize_absolute_axis(mousepos.x, client_rect.left, client_rect.right);
		devinfo->mouse.state.lY = normalize_absolute_axis(mousepos.y, client_rect.top, client_rect.bottom);
	}

	// release the lock
	osd_lock_release(input_lock);
	return TRUE;
}


//============================================================
//  wininput_handle_raw
//============================================================

BOOL wininput_handle_raw(HANDLE device)
{
	BYTE small_buffer[4096];
	LPBYTE data = small_buffer;
	BOOL result = FALSE;
	int size;

	// ignore if not enabled
	if (!input_enabled)
		return result;

	// determine the size of databuffer we need
	if ((*get_rawinput_data)((HRAWINPUT)device, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER)) != 0)
		return result;

	// if necessary, allocate a temporary buffer and fetch the data
	if (size > sizeof(small_buffer))
	{
		data = global_alloc_array(BYTE, size);
		if (data == NULL)
			return result;
	}

	// fetch the data and process the appropriate message types
	result = (*get_rawinput_data)((HRAWINPUT)device, RID_INPUT, data, &size, sizeof(RAWINPUTHEADER));
	if (result != 0)
	{
		RAWINPUT *input = (RAWINPUT *)data;

		// handle keyboard input
		if (input->header.dwType == RIM_TYPEKEYBOARD)
		{
			osd_lock_acquire(input_lock);
			rawinput_keyboard_update(input->header.hDevice, &input->data.keyboard);
			osd_lock_release(input_lock);
			result = TRUE;
		}

		// handle mouse input
		else if (input->header.dwType == RIM_TYPEMOUSE)
		{
			osd_lock_acquire(input_lock);
			rawinput_mouse_update(input->header.hDevice, &input->data.mouse);
			osd_lock_release(input_lock);
			result = TRUE;
		}
	}

	// free the temporary buffer and return the result
	if (data != small_buffer)
		global_free_array(data);
	return result;
}


//============================================================
//  wininput_vkey_for_mame_code
//============================================================

int wininput_vkey_for_mame_code(input_code code)
{
	// only works for keyboard switches
	if (code.device_class() == DEVICE_CLASS_KEYBOARD && code.item_class() == ITEM_CLASS_SWITCH)
	{
		input_item_id id = code.item_id();
		int tablenum;

		// scan the table for a match
		for (tablenum = 0; tablenum < ARRAY_LENGTH(win_key_trans_table); tablenum++)
			if (win_key_trans_table[tablenum].mame_key == id)
				return win_key_trans_table[tablenum].virtual_key;
	}
	return 0;
}


//============================================================
//  lookup_mame_code
//============================================================

static int lookup_mame_index(const char *scode)
{
	for (int i = 0; i < ARRAY_LENGTH(win_key_trans_table); i++)
	{
		if (!strcmp(scode, win_key_trans_table[i].mame_key_name))
			return i;
	}
	return -1;
}

static input_item_id lookup_mame_code(const char *scode)
{
	int const index = lookup_mame_index(scode);
	if (index >= 0)
		return win_key_trans_table[index].mame_key;
	else
		return ITEM_ID_INVALID;
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
				if (strcmp(uimode,"auto"))
				{
					std::string fullmode = "ITEM_ID_";
					fullmode += uimode;
					input_item_id const mameid_code = lookup_mame_code(fullmode.c_str());
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


//============================================================
//  device_list_poll_devices
//============================================================

static void device_list_poll_devices(device_info *devlist_head)
{
	device_info *curdev;

	for (curdev = devlist_head; curdev != NULL; curdev = curdev->next)
		if (curdev->poll != NULL)
			(*curdev->poll)(curdev);
}


//============================================================
//  device_list_reset_devices
//============================================================

static void device_list_reset_devices(device_info *devlist_head)
{
	device_info *curdev;

	for (curdev = devlist_head; curdev != NULL; curdev = curdev->next)
		generic_device_reset(curdev);
}


//============================================================
//  generic_device_alloc
//============================================================

static device_info *generic_device_alloc(running_machine &machine, device_info **devlist_head_ptr, const TCHAR *name)
{
	device_info **curdev_ptr;
	device_info *devinfo;

	// allocate memory for the device object
	devinfo = global_alloc_clear(device_info(machine));
	devinfo->head = devlist_head_ptr;

	// allocate a UTF8 copy of the name
	devinfo->name = utf8_from_tstring(name);
	if (devinfo->name == NULL)
		goto error;

	// append us to the list
	for (curdev_ptr = devinfo->head; *curdev_ptr != NULL; curdev_ptr = &(*curdev_ptr)->next) ;
	*curdev_ptr = devinfo;

	return devinfo;

error:
	global_free(devinfo);
	return NULL;
}


//============================================================
//  generic_device_free
//============================================================

static void generic_device_free(device_info *devinfo)
{
	device_info **curdev_ptr;

	// remove us from the list
	for (curdev_ptr = devinfo->head; *curdev_ptr != devinfo && *curdev_ptr != NULL; curdev_ptr = &(*curdev_ptr)->next) ;
	if (*curdev_ptr == devinfo)
		*curdev_ptr = devinfo->next;

	// free the copy of the name if present
	if (devinfo->name != NULL)
		osd_free((void *)devinfo->name);
	devinfo->name = NULL;

	// and now free the info
	global_free(devinfo);
}


//============================================================
//  generic_device_index
//============================================================

static int generic_device_index(device_info *devlist_head, device_info *devinfo)
{
	int index = 0;
	while (devlist_head != NULL)
	{
		if (devlist_head == devinfo)
			return index;
		index++;
		devlist_head = devlist_head->next;
	}
	return -1;
}


//============================================================
//  generic_device_reset
//============================================================

static void generic_device_reset(device_info *devinfo)
{
	// keyboard case
	if (devinfo->head == &keyboard_list)
		memset(devinfo->keyboard.state, 0, sizeof(devinfo->keyboard.state));

	// mouse/lightgun case
	else if (devinfo->head == &mouse_list || devinfo->head == &lightgun_list)
		memset(&devinfo->mouse.state, 0, sizeof(devinfo->mouse.state));

	// joystick case
	else if (devinfo->head == &joystick_list)
	{
		int povnum;

		memset(&devinfo->joystick.state, 0, sizeof(devinfo->joystick.state));
		for (povnum = 0; povnum < ARRAY_LENGTH(devinfo->joystick.state.rgdwPOV); povnum++)
			devinfo->joystick.state.rgdwPOV[povnum] = 0xffff;
	}
}


//============================================================
//  generic_button_get_state
//============================================================

static INT32 generic_button_get_state(void *device_internal, void *item_internal)
{
	device_info *devinfo = (device_info *)device_internal;
	BYTE *itemdata = (BYTE *)item_internal;

	// return the current state
	poll_if_necessary(devinfo->machine());
	return *itemdata >> 7;
}


//============================================================
//  generic_axis_get_state
//============================================================

static INT32 generic_axis_get_state(void *device_internal, void *item_internal)
{
	device_info *devinfo = (device_info *)device_internal;
	LONG *axisdata = (LONG *)item_internal;

	// return the current state
	poll_if_necessary(devinfo->machine());
	return *axisdata;
}


//============================================================
//  win32_init
//============================================================

static void win32_init(running_machine &machine)
{
	int gunnum;

	// we don't need any initialization unless we are using shared axis mode for lightguns
	if (!lightgun_shared_axis_mode)
		return;

	// we need an exit callback
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(win32_exit), &machine));

	// allocate two lightgun devices
	for (gunnum = 0; gunnum < 2; gunnum++)
	{
		static const TCHAR *const gun_names[] = { TEXT("Shared Axis Gun 1"), TEXT("Shared Axis Gun 2") };
		device_info *devinfo;
		int axisnum, butnum;

		// allocate a device
		devinfo = generic_device_alloc(machine, &lightgun_list, gun_names[gunnum]);
		if (devinfo == NULL)
			break;

		// add the device
		devinfo->device = machine.input().device_class(DEVICE_CLASS_LIGHTGUN).add_device(devinfo->name, devinfo);

		// populate the axes
		for (axisnum = 0; axisnum < 2; axisnum++)
		{
			char *name = utf8_from_tstring(default_axis_name[axisnum]);
			devinfo->device->add_item(name, (input_item_id)(ITEM_ID_XAXIS + axisnum), generic_axis_get_state, &devinfo->mouse.state.lX + axisnum);
			osd_free(name);
		}

		// populate the buttons
		for (butnum = 0; butnum < 2; butnum++)
		{
			char *name = utf8_from_tstring(default_button_name(butnum));
			devinfo->device->add_item(name, (input_item_id)(ITEM_ID_BUTTON1 + butnum), generic_button_get_state, &devinfo->mouse.state.rgbButtons[butnum]);
			osd_free(name);
		}
	}
}


//============================================================
//  win32_exit
//============================================================

static void win32_exit(running_machine &machine)
{
	// skip if we're in shared axis mode
	if (!lightgun_shared_axis_mode)
		return;

	// delete the lightgun devices
	while (lightgun_list != NULL)
		generic_device_free(lightgun_list);
}


//============================================================
//  win32_keyboard_poll
//============================================================

static void win32_keyboard_poll(device_info *devinfo)
{
	int keynum;

	// clear the flag that says we detected a key down via win32
	keyboard_win32_reported_key_down = false;

	// reset the keyboard state and then repopulate
	memset(devinfo->keyboard.state, 0, sizeof(devinfo->keyboard.state));

	// iterate over keys
	for (keynum = 0; keynum < ARRAY_LENGTH(win_key_trans_table); keynum++)
	{
		int vk = win_key_trans_table[keynum].virtual_key;
		if (vk != 0 && (GetAsyncKeyState(vk) & 0x8000) != 0)
		{
			int dik = win_key_trans_table[keynum].di_key;

			// conver the VK code to a scancode (DIK code)
			if (dik != 0)
				devinfo->keyboard.state[dik] = 0x80;

			// set this flag so that we continue to use win32 until all keys are up
			keyboard_win32_reported_key_down = true;
		}
	}
}


//============================================================
//  win32_lightgun_poll
//============================================================

static void win32_lightgun_poll(device_info *devinfo)
{
	INT32 xpos = 0, ypos = 0;
	POINT mousepos;

	// if we are using the shared axis hack, the data is updated via Windows messages only
	if (lightgun_shared_axis_mode)
		return;

	// get the cursor position and transform into final results
	GetCursorPos(&mousepos);
	if (win_window_list != NULL)
	{
		RECT client_rect;

		// get the position relative to the window
		GetClientRect(win_window_list->m_hwnd, &client_rect);
		ScreenToClient(win_window_list->m_hwnd, &mousepos);

		// convert to absolute coordinates
		xpos = normalize_absolute_axis(mousepos.x, client_rect.left, client_rect.right);
		ypos = normalize_absolute_axis(mousepos.y, client_rect.top, client_rect.bottom);
	}

	// update the X/Y positions
	devinfo->mouse.state.lX = xpos;
	devinfo->mouse.state.lY = ypos;
}


//============================================================
//  dinput_init
//============================================================

static void dinput_init(running_machine &machine)
{
	HRESULT result;
#if DIRECTINPUT_VERSION >= 0x800
	int didevtype_keyboard = DI8DEVCLASS_KEYBOARD;
	int didevtype_mouse = DI8DEVCLASS_POINTER;
	int didevtype_joystick = DI8DEVCLASS_GAMECTRL;

	dinput_version = DIRECTINPUT_VERSION;
	result = DirectInput8Create(GetModuleHandleUni(), dinput_version, IID_IDirectInput8, (void **)&dinput, NULL);
	if (result != DI_OK)
	{
		dinput_version = 0;
		return;
	}
#else
	int didevtype_keyboard = DIDEVTYPE_KEYBOARD;
	int didevtype_mouse = DIDEVTYPE_MOUSE;
	int didevtype_joystick = DIDEVTYPE_JOYSTICK;

	// first attempt to initialize DirectInput at the current version
	dinput_version = DIRECTINPUT_VERSION;
	result = DirectInputCreate(GetModuleHandleUni(), dinput_version, &dinput, NULL);
	if (result != DI_OK)
	{
		// if that fails, try version 5
		dinput_version = 0x0500;
		result = DirectInputCreate(GetModuleHandleUni(), dinput_version, &dinput, NULL);
		if (result != DI_OK)
		{
			// if that fails, try version 3
			dinput_version = 0x0300;
			result = DirectInputCreate(GetModuleHandleUni(), dinput_version, &dinput, NULL);
			if (result != DI_OK)
			{
				dinput_version = 0;
				return;
			}
		}
	}
#endif

	osd_printf_verbose("DirectInput: Using DirectInput %d\n", dinput_version >> 8);

	// we need an exit callback
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(dinput_exit), &machine));

	// initialize keyboard devices, but only if we don't have any yet
	if (keyboard_list == NULL)
	{
		// enumerate the ones we have
		result = IDirectInput_EnumDevices(dinput, didevtype_keyboard, dinput_keyboard_enum, &machine, DIEDFL_ATTACHEDONLY);
		if (result != DI_OK)
			fatalerror("DirectInput: Unable to enumerate keyboards (result=%08X)\n", (UINT32)result);
	}

	// initialize mouse & lightgun devices, but only if we don't have any yet
	if (mouse_list == NULL)
	{
		// enumerate the ones we have
		result = IDirectInput_EnumDevices(dinput, didevtype_mouse, dinput_mouse_enum, &machine, DIEDFL_ATTACHEDONLY);
		if (result != DI_OK)
			fatalerror("DirectInput: Unable to enumerate mice (result=%08X)\n", (UINT32)result);
	}

	// initialize joystick devices
	result = IDirectInput_EnumDevices(dinput, didevtype_joystick, dinput_joystick_enum, &machine, DIEDFL_ATTACHEDONLY);
	if (result != DI_OK)
		fatalerror("DirectInput: Unable to enumerate joysticks (result=%08X)\n", (UINT32)result);
}


//============================================================
//  dinput_exit
//============================================================

static void dinput_exit(running_machine &machine)
{
	// release all our devices
	while (joystick_list != NULL && joystick_list->dinput.device != NULL)
		dinput_device_release(joystick_list);
	while (lightgun_list != NULL)
		generic_device_free(lightgun_list);
	while (mouse_list != NULL && mouse_list->dinput.device != NULL)
		dinput_device_release(mouse_list);
	while (keyboard_list != NULL && keyboard_list->dinput.device != NULL)
		dinput_device_release(keyboard_list);

	// release DirectInput
	if (dinput != NULL)
		IDirectInput_Release(dinput);
	dinput = NULL;
}


//============================================================
//  dinput_set_dword_property
//============================================================

static HRESULT dinput_set_dword_property(LPDIRECTINPUTDEVICE device, REFGUID property_guid, DWORD object, DWORD how, DWORD value)
{
	DIPROPDWORD dipdw;

	dipdw.diph.dwSize       = sizeof(dipdw);
	dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
	dipdw.diph.dwObj        = object;
	dipdw.diph.dwHow        = how;
	dipdw.dwData            = value;

	return IDirectInputDevice_SetProperty(device, property_guid, &dipdw.diph);
}


//============================================================
//  dinput_device_create
//============================================================

static device_info *dinput_device_create(running_machine &machine, device_info **devlist_head_ptr, LPCDIDEVICEINSTANCE instance, LPCDIDATAFORMAT format1, LPCDIDATAFORMAT format2, DWORD cooperative_level)
{
	device_info *devinfo;
	HRESULT result;

	// allocate memory for the device object
	devinfo = generic_device_alloc(machine, devlist_head_ptr, instance->tszInstanceName);

	// attempt to create a device
	result = IDirectInput_CreateDevice(dinput, WRAP_REFIID(instance->guidInstance), &devinfo->dinput.device, NULL);
	if (result != DI_OK)
		goto error;

	// try to get a version 2 device for it
	result = IDirectInputDevice_QueryInterface(devinfo->dinput.device, WRAP_REFIID(IID_IDirectInputDevice2), (void **)&devinfo->dinput.device2);
	if (result != DI_OK)
		devinfo->dinput.device2 = NULL;

	// get the caps
	devinfo->dinput.caps.dwSize = STRUCTSIZE(DIDEVCAPS);
	result = IDirectInputDevice_GetCapabilities(devinfo->dinput.device, &devinfo->dinput.caps);
	if (result != DI_OK)
		goto error;

	// attempt to set the data format
	devinfo->dinput.format = format1;
	result = IDirectInputDevice_SetDataFormat(devinfo->dinput.device, devinfo->dinput.format);
	if (result != DI_OK)
	{
		// use the secondary format if available
		if (format2 != NULL)
		{
			devinfo->dinput.format = format2;
			result = IDirectInputDevice_SetDataFormat(devinfo->dinput.device, devinfo->dinput.format);
		}
		if (result != DI_OK)
			goto error;
	}

	// set the cooperative level
	result = IDirectInputDevice_SetCooperativeLevel(devinfo->dinput.device, win_window_list->m_hwnd, cooperative_level);
	if (result != DI_OK)
		goto error;
	return devinfo;

error:
	dinput_device_release(devinfo);
	return NULL;
}


//============================================================
//  dinput_device_release
//============================================================

static void dinput_device_release(device_info *devinfo)
{
	// release the version 2 device if present
	if (devinfo->dinput.device2 != NULL)
		IDirectInputDevice_Release(devinfo->dinput.device2);
	devinfo->dinput.device2 = NULL;

	// release the regular device if present
	if (devinfo->dinput.device != NULL)
		IDirectInputDevice_Release(devinfo->dinput.device);
	devinfo->dinput.device = NULL;

	// free the item list
	generic_device_free(devinfo);
}


//============================================================
//  dinput_device_item_name
//============================================================

static char *dinput_device_item_name(device_info *devinfo, int offset, const TCHAR *defstring, const TCHAR *suffix)
{
	DIDEVICEOBJECTINSTANCE instance = { 0 };
	const TCHAR *namestring = instance.tszName;
	TCHAR *combined;
	HRESULT result;
	char *utf8;

	// query the key name
	instance.dwSize = STRUCTSIZE(DIDEVICEOBJECTINSTANCE);
	result = IDirectInputDevice_GetObjectInfo(devinfo->dinput.device, &instance, offset, DIPH_BYOFFSET);

	// if we got an error and have no default string, just return NULL
	if (result != DI_OK)
	{
		if (defstring == NULL)
			return NULL;
		namestring = defstring;
	}

	// if no suffix, return as-is
	if (suffix == NULL)
		return utf8_from_tstring(namestring);

	// otherwise, allocate space to add the suffix
	combined = global_alloc_array(TCHAR, _tcslen(namestring) + 1 + _tcslen(suffix) + 1);
	_tcscpy(combined, namestring);
	_tcscat(combined, TEXT(" "));
	_tcscat(combined, suffix);

	// convert to UTF8, free the temporary string, and return
	utf8 = utf8_from_tstring(combined);
	global_free_array(combined);
	return utf8;
}


//============================================================
//  dinput_device_poll
//============================================================

static HRESULT dinput_device_poll(device_info *devinfo)
{
	HRESULT result;

	// first poll the device, then get the state
	if (devinfo->dinput.device2 != NULL)
		IDirectInputDevice2_Poll(devinfo->dinput.device2);
	result = IDirectInputDevice_GetDeviceState(devinfo->dinput.device, devinfo->dinput.format->dwDataSize, &devinfo->joystick.state);

	// handle lost inputs here
	if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED)
	{
		result = IDirectInputDevice_Acquire(devinfo->dinput.device);
		if (result == DI_OK)
			result = IDirectInputDevice_GetDeviceState(devinfo->dinput.device, devinfo->dinput.format->dwDataSize, &devinfo->joystick.state);
	}

	return result;
}


//============================================================
//  dinput_keyboard_enum
//============================================================

static BOOL CALLBACK dinput_keyboard_enum(LPCDIDEVICEINSTANCE instance, LPVOID ref)
{
	running_machine &machine = *(running_machine *)ref;
	device_info *devinfo;
	int keynum;

	// allocate and link in a new device
	devinfo = dinput_device_create(machine, &keyboard_list, instance, &c_dfDIKeyboard, NULL, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (devinfo == NULL)
		goto exit;

	// add the device
	devinfo->device = machine.input().device_class(DEVICE_CLASS_KEYBOARD).add_device(devinfo->name, devinfo);
	devinfo->poll = dinput_keyboard_poll;

	// populate it
	for (keynum = 0; keynum < MAX_KEYS; keynum++)
	{
		input_item_id itemid = keyboard_map_scancode_to_itemid(keynum);
		TCHAR defname[20];
		char *name;

		// generate/fetch the name
		_sntprintf(defname, ARRAY_LENGTH(defname), TEXT("Scan%03d"), keynum);
		name = dinput_device_item_name(devinfo, keynum, defname, NULL);

		// add the item to the device
		devinfo->device->add_item(name, itemid, generic_button_get_state, &devinfo->keyboard.state[keynum]);
		osd_free(name);
	}

exit:
	return DIENUM_CONTINUE;
}


//============================================================
//  dinput_keyboard_poll
//============================================================

static void dinput_keyboard_poll(device_info *devinfo)
{
	HRESULT result = dinput_device_poll(devinfo);

	// for the first device, if we errored, or if we previously reported win32 keys,
	// then ignore the dinput state and poll using win32
	if (devinfo == keyboard_list && (result != DI_OK || keyboard_win32_reported_key_down))
		win32_keyboard_poll(devinfo);
}


//============================================================
//  dinput_mouse_enum
//============================================================

static BOOL CALLBACK dinput_mouse_enum(LPCDIDEVICEINSTANCE instance, LPVOID ref)
{
	device_info *devinfo, *guninfo = NULL;
	running_machine &machine = *(running_machine *)ref;
	int axisnum, butnum;
	HRESULT result;

	// allocate and link in a new device
	devinfo = dinput_device_create(machine, &mouse_list, instance, &c_dfDIMouse2, &c_dfDIMouse, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (devinfo == NULL)
		goto exit;

	// allocate a second device for the gun (unless we are using the shared axis mode)
	// we only support a single gun in dinput mode, so only add one
	if (!lightgun_shared_axis_mode && devinfo == mouse_list)
	{
		guninfo = generic_device_alloc(machine, &lightgun_list, instance->tszInstanceName);
		if (guninfo == NULL)
			goto exit;
	}

	// set relative mode on the mouse device
	result = dinput_set_dword_property(devinfo->dinput.device, DIPROP_AXISMODE, 0, DIPH_DEVICE, DIPROPAXISMODE_REL);
	if (result != DI_OK && result != DI_PROPNOEFFECT)
	{
		osd_printf_error("DirectInput: Unable to set relative mode for mouse %d (%s)\n", generic_device_index(mouse_list, devinfo), devinfo->name);
		goto error;
	}

	// add the device
	devinfo->device = machine.input().device_class(DEVICE_CLASS_MOUSE).add_device(devinfo->name, devinfo);
	devinfo->poll = dinput_mouse_poll;
	if (guninfo != NULL)
	{
		guninfo->device = machine.input().device_class(DEVICE_CLASS_LIGHTGUN).add_device(guninfo->name, guninfo);
		guninfo->poll = win32_lightgun_poll;
	}

	// cap the number of axes and buttons based on the format
	devinfo->dinput.caps.dwAxes = MIN(devinfo->dinput.caps.dwAxes, 3);
	devinfo->dinput.caps.dwButtons = MIN(devinfo->dinput.caps.dwButtons, (devinfo->dinput.format == &c_dfDIMouse) ? 4 : 8);

	// populate the axes
	for (axisnum = 0; axisnum < devinfo->dinput.caps.dwAxes; axisnum++)
	{
		char *name = dinput_device_item_name(devinfo, offsetof(DIMOUSESTATE, lX) + axisnum * sizeof(LONG), default_axis_name[axisnum], NULL);

		// add to the mouse device and optionally to the gun device as well
		devinfo->device->add_item(name, (input_item_id)(ITEM_ID_XAXIS + axisnum), generic_axis_get_state, &devinfo->mouse.state.lX + axisnum);
		if (guninfo != NULL && axisnum < 2)
			guninfo->device->add_item(name, (input_item_id)(ITEM_ID_XAXIS + axisnum), generic_axis_get_state, &guninfo->mouse.state.lX + axisnum);

		osd_free(name);
	}

	// populate the buttons
	for (butnum = 0; butnum < devinfo->dinput.caps.dwButtons; butnum++)
	{
		FPTR offset = (FPTR)(&((DIMOUSESTATE *)NULL)->rgbButtons[butnum]);
		char *name = dinput_device_item_name(devinfo, offset, default_button_name(butnum), NULL);

		// add to the mouse device and optionally to the gun device as well
		// note that the gun device points to the mouse buttons rather than its own
		devinfo->device->add_item(name, (input_item_id)(ITEM_ID_BUTTON1 + butnum), generic_button_get_state, &devinfo->mouse.state.rgbButtons[butnum]);
		if (guninfo != NULL)
			guninfo->device->add_item(name, (input_item_id)(ITEM_ID_BUTTON1 + butnum), generic_button_get_state, &devinfo->mouse.state.rgbButtons[butnum]);

		osd_free(name);
	}

exit:
	return DIENUM_CONTINUE;

error:
	if (guninfo != NULL)
		generic_device_free(guninfo);
	if (devinfo != NULL)
		dinput_device_release(devinfo);
	goto exit;
}


//============================================================
//  dinput_mouse_poll
//============================================================

static void dinput_mouse_poll(device_info *devinfo)
{
	// poll
	dinput_device_poll(devinfo);

	// scale the axis data
	devinfo->mouse.state.lX *= INPUT_RELATIVE_PER_PIXEL;
	devinfo->mouse.state.lY *= INPUT_RELATIVE_PER_PIXEL;
	devinfo->mouse.state.lZ *= INPUT_RELATIVE_PER_PIXEL;
}


//============================================================
//  dinput_joystick_enum
//============================================================

static BOOL CALLBACK dinput_joystick_enum(LPCDIDEVICEINSTANCE instance, LPVOID ref)
{
	DWORD cooperative_level = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
	int axisnum, axiscount, povnum, butnum;
	running_machine &machine = *(running_machine *)ref;
	device_info *devinfo;
	HRESULT result;

	if (win_window_list != NULL && win_window_list->win_has_menu()) {
		cooperative_level = DISCL_BACKGROUND | DISCL_NONEXCLUSIVE;
	}
	// allocate and link in a new device
	devinfo = dinput_device_create(machine, &joystick_list, instance, &c_dfDIJoystick, NULL, cooperative_level);
	if (devinfo == NULL)
		goto exit;

	// set absolute mode
	result = dinput_set_dword_property(devinfo->dinput.device, DIPROP_AXISMODE, 0, DIPH_DEVICE, DIPROPAXISMODE_ABS);
	if (result != DI_OK && result != DI_PROPNOEFFECT)
		osd_printf_warning("DirectInput: Unable to set absolute mode for joystick %d (%s)\n", generic_device_index(joystick_list, devinfo), devinfo->name);

	// turn off deadzone; we do our own calculations
	result = dinput_set_dword_property(devinfo->dinput.device, DIPROP_DEADZONE, 0, DIPH_DEVICE, 0);
	if (result != DI_OK && result != DI_PROPNOEFFECT)
		osd_printf_warning("DirectInput: Unable to reset deadzone for joystick %d (%s)\n", generic_device_index(joystick_list, devinfo), devinfo->name);

	// turn off saturation; we do our own calculations
	result = dinput_set_dword_property(devinfo->dinput.device, DIPROP_SATURATION, 0, DIPH_DEVICE, 10000);
	if (result != DI_OK && result != DI_PROPNOEFFECT)
		osd_printf_warning("DirectInput: Unable to reset saturation for joystick %d (%s)\n", generic_device_index(joystick_list, devinfo), devinfo->name);

	// cap the number of axes, POVs, and buttons based on the format
	devinfo->dinput.caps.dwAxes = MIN(devinfo->dinput.caps.dwAxes, 8);
	devinfo->dinput.caps.dwPOVs = MIN(devinfo->dinput.caps.dwPOVs, 4);
	devinfo->dinput.caps.dwButtons = MIN(devinfo->dinput.caps.dwButtons, 128);

	// add the device
	devinfo->device = machine.input().device_class(DEVICE_CLASS_JOYSTICK).add_device(devinfo->name, devinfo);
	devinfo->poll = dinput_joystick_poll;

	// populate the axes
	for (axisnum = axiscount = 0; axiscount < devinfo->dinput.caps.dwAxes && axisnum < 8; axisnum++)
	{
		DIPROPRANGE dipr;
		char *name;

		// fetch the range of this axis
		dipr.diph.dwSize = sizeof(dipr);
		dipr.diph.dwHeaderSize = sizeof(dipr.diph);
		dipr.diph.dwObj = offsetof(DIJOYSTATE2, lX) + axisnum * sizeof(LONG);
		dipr.diph.dwHow = DIPH_BYOFFSET;
		result = IDirectInputDevice_GetProperty(devinfo->dinput.device, DIPROP_RANGE, &dipr.diph);
		if (result != DI_OK)
			continue;
		devinfo->joystick.rangemin[axisnum] = dipr.lMin;
		devinfo->joystick.rangemax[axisnum] = dipr.lMax;

		// populate the item description as well
		name = dinput_device_item_name(devinfo, offsetof(DIJOYSTATE2, lX) + axisnum * sizeof(LONG), default_axis_name[axisnum], NULL);
		devinfo->device->add_item(name, (input_item_id)(ITEM_ID_XAXIS + axisnum), generic_axis_get_state, &devinfo->joystick.state.lX + axisnum);
		osd_free(name);

		axiscount++;
	}

	// populate the POVs
	for (povnum = 0; povnum < devinfo->dinput.caps.dwPOVs; povnum++)
	{
		char *name;

		// left
		name = dinput_device_item_name(devinfo, offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), TEXT("L"));
		devinfo->device->add_item(name, ITEM_ID_OTHER_SWITCH, dinput_joystick_pov_get_state, (void *)(FPTR)(povnum * 4 + POVDIR_LEFT));
		osd_free(name);

		// right
		name = dinput_device_item_name(devinfo, offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), TEXT("R"));
		devinfo->device->add_item(name, ITEM_ID_OTHER_SWITCH, dinput_joystick_pov_get_state, (void *)(FPTR)(povnum * 4 + POVDIR_RIGHT));
		osd_free(name);

		// up
		name = dinput_device_item_name(devinfo, offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), TEXT("U"));
		devinfo->device->add_item(name, ITEM_ID_OTHER_SWITCH, dinput_joystick_pov_get_state, (void *)(FPTR)(povnum * 4 + POVDIR_UP));
		osd_free(name);

		// down
		name = dinput_device_item_name(devinfo, offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), TEXT("D"));
		devinfo->device->add_item(name, ITEM_ID_OTHER_SWITCH, dinput_joystick_pov_get_state, (void *)(FPTR)(povnum * 4 + POVDIR_DOWN));
		osd_free(name);
	}

	// populate the buttons
	for (butnum = 0; butnum < devinfo->dinput.caps.dwButtons; butnum++)
	{
		FPTR offset = (FPTR)(&((DIJOYSTATE2 *)NULL)->rgbButtons[butnum]);
		char *name = dinput_device_item_name(devinfo, offset, default_button_name(butnum), NULL);

		input_item_id itemid;

		if (butnum < INPUT_MAX_BUTTONS)
			itemid = (input_item_id) (ITEM_ID_BUTTON1 + butnum);
		else if (butnum < INPUT_MAX_BUTTONS + INPUT_MAX_ADD_SWITCH)
			itemid = (input_item_id) (ITEM_ID_ADD_SWITCH1 - INPUT_MAX_BUTTONS + butnum);
		else
			itemid = ITEM_ID_OTHER_SWITCH;

		devinfo->device->add_item(name, itemid, generic_button_get_state, &devinfo->joystick.state.rgbButtons[butnum]);
		osd_free(name);
	}

exit:
	return DIENUM_CONTINUE;
}


//============================================================
//  dinput_joystick_poll
//============================================================

static void dinput_joystick_poll(device_info *devinfo)
{
	int axisnum;

	// poll the device first
	dinput_device_poll(devinfo);

	// normalize axis values
	for (axisnum = 0; axisnum < 8; axisnum++)
	{
		LONG *axis = (&devinfo->joystick.state.lX) + axisnum;
		*axis = normalize_absolute_axis(*axis, devinfo->joystick.rangemin[axisnum], devinfo->joystick.rangemax[axisnum]);
	}
}


//============================================================
//  dinput_joystick_pov_get_state
//============================================================

static INT32 dinput_joystick_pov_get_state(void *device_internal, void *item_internal)
{
	device_info *devinfo = (device_info *)device_internal;
	int povnum = (FPTR)item_internal / 4;
	int povdir = (FPTR)item_internal % 4;
	INT32 result = 0;
	DWORD pov;

	// get the current state
	poll_if_necessary(devinfo->machine());
	pov = devinfo->joystick.state.rgdwPOV[povnum];

	// if invalid, return 0
	if ((pov & 0xffff) == 0xffff)
		return result;

	// return the current state
	switch (povdir)
	{
		case POVDIR_LEFT:   result = (pov >= 22500 && pov <= 31500);    break;
		case POVDIR_RIGHT:  result = (pov >= 4500 && pov <= 13500);     break;
		case POVDIR_UP:     result = (pov >= 31500 || pov <= 4500);     break;
		case POVDIR_DOWN:   result = (pov >= 13500 && pov <= 22500);    break;
	}
	return result;
}


//============================================================
//  rawinput_init
//============================================================

static void rawinput_init(running_machine &machine)
{
	RAWINPUTDEVICELIST *devlist = NULL;
	int device_count, devnum, regcount;
	RAWINPUTDEVICE reglist[2];
	HMODULE user32;

	// we need pause and exit callbacks
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(rawinput_exit), &machine));

	// look in user32 for the raw input APIs
	user32 = LoadLibrary(TEXT("user32.dll"));
	if (user32 == NULL)
		goto error;

	// look up the entry points
	register_rawinput_devices = (register_rawinput_devices_ptr)GetProcAddress(user32, "RegisterRawInputDevices");
	get_rawinput_device_list = (get_rawinput_device_list_ptr)GetProcAddress(user32, "GetRawInputDeviceList");
	get_rawinput_device_info = (get_rawinput_device_info_ptr)GetProcAddress(user32, "GetRawInputDeviceInfo" UNICODE_SUFFIX);
	get_rawinput_data = (get_rawinput_data_ptr)GetProcAddress(user32, "GetRawInputData");
	if (register_rawinput_devices == NULL || get_rawinput_device_list == NULL || get_rawinput_device_info == NULL || get_rawinput_data == NULL)
		goto error;
	osd_printf_verbose("RawInput: APIs detected\n");

	// get the number of devices, allocate a device list, and fetch it
	if ((*get_rawinput_device_list)(NULL, &device_count, sizeof(*devlist)) != 0)
		goto error;
	if (device_count == 0)
		goto error;
	devlist = global_alloc_array(RAWINPUTDEVICELIST, device_count);
	if ((*get_rawinput_device_list)(devlist, &device_count, sizeof(*devlist)) == -1)
		goto error;

	// iterate backwards through devices; new devices are added at the head
	for (devnum = device_count - 1; devnum >= 0; devnum--)
	{
		RAWINPUTDEVICELIST *device = &devlist[devnum];

		// handle keyboards
		if (!FORCE_DIRECTINPUT && device->dwType == RIM_TYPEKEYBOARD)
			rawinput_keyboard_enum(machine, device);

		// handle mice
		else if (!FORCE_DIRECTINPUT && device->dwType == RIM_TYPEMOUSE)
			rawinput_mouse_enum(machine, device);
	}

	// don't enable global inputs when testing direct input or debugging
	if (!FORCE_DIRECTINPUT && !machine.options().debug())
	{
		global_inputs_enabled = downcast<windows_options &>(machine.options()).global_inputs();
	}

	// finally, register to receive raw input WM_INPUT messages
	regcount = 0;
	if (keyboard_list != NULL)
	{
		reglist[regcount].usUsagePage = 0x01;
		reglist[regcount].usUsage = 0x06;

		if (global_inputs_enabled)
		{
			reglist[regcount].dwFlags = 0x00000100;
		}
		else
		{
			reglist[regcount].dwFlags = 0;
		}

		reglist[regcount].hwndTarget = win_window_list->m_hwnd;
		regcount++;
	}
	if (mouse_list != NULL)
	{
		reglist[regcount].usUsagePage = 0x01;
		reglist[regcount].usUsage = 0x02;

		if (global_inputs_enabled)
		{
			reglist[regcount].dwFlags = 0x00000100;
		}
		else
		{
			reglist[regcount].dwFlags = 0;
		}

		reglist[regcount].hwndTarget = win_window_list->m_hwnd;
		regcount++;
	}

	// if the registration fails, we need to back off
	if (regcount > 0)
		if (!(*register_rawinput_devices)(reglist, regcount, sizeof(reglist[0])))
			goto error;

	global_free_array(devlist);
	return;

error:
	if (devlist != NULL)
		global_free_array(devlist);
}


//============================================================
//  rawinput_exit
//============================================================

static void rawinput_exit(running_machine &machine)
{
	// release all our devices
	while (lightgun_list != NULL && lightgun_list->rawinput.device != NULL)
		rawinput_device_release(lightgun_list);
	while (mouse_list != NULL && mouse_list->rawinput.device != NULL)
		rawinput_device_release(mouse_list);
	while (keyboard_list != NULL && keyboard_list->rawinput.device != NULL)
		rawinput_device_release(keyboard_list);
}


//============================================================
//  rawinput_device_create
//============================================================

static device_info *rawinput_device_create(running_machine &machine, device_info **devlist_head_ptr, PRAWINPUTDEVICELIST device)
{
	device_info *devinfo = NULL;
	TCHAR *tname = NULL;
	INT name_length = 0;

	// determine the length of the device name, allocate it, and fetch it if not nameless
	if ((*get_rawinput_device_info)(device->hDevice, RIDI_DEVICENAME, NULL, &name_length) != 0)
		goto error;
	tname = global_alloc_array_clear(TCHAR, name_length+1);
	if (name_length > 1 && (*get_rawinput_device_info)(device->hDevice, RIDI_DEVICENAME, tname, &name_length) == -1)
		goto error;

	// if this is an RDP name, skip it
	if (_tcsstr(tname, TEXT("Root#RDP_")) != NULL)
		goto error;

	// improve the name and then allocate a device
	tname = rawinput_device_improve_name(tname);
	devinfo = generic_device_alloc(machine, devlist_head_ptr, tname);
	global_free_array(tname);

	// copy the handle
	devinfo->rawinput.device = device->hDevice;
	return devinfo;

error:
	if (tname != NULL)
		global_free_array(tname);
	if (devinfo != NULL)
		rawinput_device_release(devinfo);
	return NULL;
}


//============================================================
//  rawinput_device_release
//============================================================

static void rawinput_device_release(device_info *devinfo)
{
	// free the item list
	generic_device_free(devinfo);
}


//============================================================
//  rawinput_device_improve_name
//============================================================

static TCHAR *rawinput_device_improve_name(TCHAR *name)
{
	static const TCHAR usbbasepath[] = TEXT("SYSTEM\\CurrentControlSet\\Enum\\USB");
	static const TCHAR basepath[] = TEXT("SYSTEM\\CurrentControlSet\\Enum\\");
	TCHAR *regstring = NULL;
	TCHAR *parentid = NULL;
	TCHAR *regpath = NULL;
	const TCHAR *chsrc;
	HKEY regkey = NULL;
	int usbindex;
	TCHAR *chdst;
	LONG result;

	// The RAW name received is formatted as:
	//   \??\type-id#hardware-id#instance-id#{DeviceClasses-id}
	// XP starts with "\??\"
	// Vista64 starts with "\\?\"

	// ensure the name is something we can handle
	if (_tcsncmp(name, TEXT("\\\\?\\"), 4) != 0 && _tcsncmp(name, TEXT("\\??\\"), 4) != 0)
		return name;

	// allocate a temporary string and concatenate the base path plus the name
	regpath = global_alloc_array(TCHAR, _tcslen(basepath) + 1 + _tcslen(name));
	_tcscpy(regpath, basepath);
	chdst = regpath + _tcslen(regpath);

	// convert all # to \ in the name
	for (chsrc = name + 4; *chsrc != 0; chsrc++)
		*chdst++ = (*chsrc == '#') ? '\\' : *chsrc;
	*chdst = 0;

	// remove the final chunk
	chdst = _tcsrchr(regpath, '\\');
	if (chdst == NULL)
		goto exit;
	*chdst = 0;

	// now try to open the registry key
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regpath, 0, KEY_READ, &regkey);
	if (result != ERROR_SUCCESS)
		goto exit;

	// fetch the device description; if it exists, we are finished
	regstring = reg_query_string(regkey, TEXT("DeviceDesc"));
	if (regstring != NULL)
		goto convert;

	// close this key
	RegCloseKey(regkey);
	regkey = NULL;

	// if the key name does not contain "HID", it's not going to be in the USB tree; give up
	if (_tcsstr(regpath, TEXT("HID")) == NULL)
		goto exit;

	// extract the expected parent ID from the regpath
	parentid = _tcsrchr(regpath, '\\');
	if (parentid == NULL)
		goto exit;
	parentid++;

	// open the USB key
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, usbbasepath, 0, KEY_READ, &regkey);
	if (result != ERROR_SUCCESS)
		goto exit;

	// enumerate the USB key
	for (usbindex = 0; result == ERROR_SUCCESS && regstring == NULL; usbindex++)
	{
		TCHAR keyname[MAX_PATH];
		DWORD namelen;

		// get the next enumerated subkey and scan it
		namelen = ARRAY_LENGTH(keyname) - 1;
		result = RegEnumKeyEx(regkey, usbindex, keyname, &namelen, NULL, NULL, NULL, NULL);
		if (result == ERROR_SUCCESS)
		{
			LONG subresult;
			int subindex;
			HKEY subkey;

			// open the subkey
			subresult = RegOpenKeyEx(regkey, keyname, 0, KEY_READ, &subkey);
			if (subresult != ERROR_SUCCESS)
				continue;

			// enumerate the subkey
			for (subindex = 0; subresult == ERROR_SUCCESS && regstring == NULL; subindex++)
			{
				// get the next enumerated subkey and scan it
				namelen = ARRAY_LENGTH(keyname) - 1;
				subresult = RegEnumKeyEx(subkey, subindex, keyname, &namelen, NULL, NULL, NULL, NULL);
				if (subresult == ERROR_SUCCESS)
				{
					TCHAR *endparentid;
					LONG endresult;
					HKEY endkey;

					// open this final key
					endresult = RegOpenKeyEx(subkey, keyname, 0, KEY_READ, &endkey);
					if (endresult != ERROR_SUCCESS)
						continue;

					// do we have a match?
					endparentid = reg_query_string(endkey, TEXT("ParentIdPrefix"));
					if (endparentid != NULL && _tcsncmp(parentid, endparentid, _tcslen(endparentid)) == 0)
						regstring = reg_query_string(endkey, TEXT("DeviceDesc"));

					// free memory and close the key
					if (endparentid != NULL)
						global_free_array(endparentid);
					RegCloseKey(endkey);
				}
			}

			// close the subkey
			RegCloseKey(subkey);
		}
	}

	// if we didn't find anything, go to the exit
	if (regstring == NULL)
		goto exit;

convert:
	// replace the name with the nicer one
	global_free_array(name);

	// remove anything prior to the final semicolon
	chsrc = _tcsrchr(regstring, ';');
	if (chsrc != NULL)
		chsrc++;
	else
		chsrc = regstring;
	name = global_alloc_array(TCHAR, _tcslen(chsrc) + 1);
	_tcscpy(name, chsrc);

exit:
	if (regstring != NULL)
		global_free_array(regstring);
	if (regpath != NULL)
		global_free_array(regpath);
	if (regkey != NULL)
		RegCloseKey(regkey);

	return name;
}


//============================================================
//  rawinput_keyboard_enum
//============================================================

static void rawinput_keyboard_enum(running_machine &machine, PRAWINPUTDEVICELIST device)
{
	device_info *devinfo;
	int keynum;

	// allocate and link in a new device
	devinfo = rawinput_device_create(machine, &keyboard_list, device);
	if (devinfo == NULL)
		return;

	// add the device
	devinfo->device = machine.input().device_class(DEVICE_CLASS_KEYBOARD).add_device(devinfo->name, devinfo);

	// populate it
	for (keynum = 0; keynum < MAX_KEYS; keynum++)
	{
		input_item_id itemid = keyboard_map_scancode_to_itemid(keynum);
		TCHAR keyname[100];
		char *name;

		// generate the name
		if (GetKeyNameText(((keynum & 0x7f) << 16) | ((keynum & 0x80) << 17), keyname, ARRAY_LENGTH(keyname)) == 0)
			_sntprintf(keyname, ARRAY_LENGTH(keyname), TEXT("Scan%03d"), keynum);
		name = utf8_from_tstring(keyname);

		// add the item to the device
		devinfo->device->add_item(name, itemid, generic_button_get_state, &devinfo->keyboard.state[keynum]);
		osd_free(name);
	}
}


//============================================================
//  rawinput_keyboard_update
//============================================================

static void rawinput_keyboard_update(HANDLE device, RAWKEYBOARD *data)
{
	device_info *devinfo;

	// find the keyboard in the list and process
	for (devinfo = keyboard_list; devinfo != NULL; devinfo = devinfo->next)
		if (devinfo->rawinput.device == device)
		{
			// determine the full DIK-compatible scancode
			UINT8 scancode = (data->MakeCode & 0x7f) | ((data->Flags & RI_KEY_E0) ? 0x80 : 0x00);

			// scancode 0xaa is a special shift code we need to ignore
			if (scancode == 0xaa)
				break;

			// set or clear the key
			if (!(data->Flags & RI_KEY_BREAK))
				devinfo->keyboard.state[scancode] = 0x80;
			else
				devinfo->keyboard.state[scancode] = 0x00;
			break;
		}
}


//============================================================
//  rawinput_mouse_enum
//============================================================

static void rawinput_mouse_enum(running_machine &machine, PRAWINPUTDEVICELIST device)
{
	device_info *devinfo, *guninfo = NULL;
	int axisnum, butnum;

	// allocate and link in a new mouse device
	devinfo = rawinput_device_create(machine, &mouse_list, device);
	if (devinfo == NULL)
		return;
	devinfo->poll = rawinput_mouse_poll;

	// allocate a second device for the gun (unless we are using the shared axis mode)
	if (!lightgun_shared_axis_mode)
	{
		guninfo = rawinput_device_create(machine, &lightgun_list, device);
		assert(guninfo != NULL);
	}

	// add the device
	devinfo->device = machine.input().device_class(DEVICE_CLASS_MOUSE).add_device(devinfo->name, devinfo);
	if (guninfo != NULL)
	{
		guninfo->device = machine.input().device_class(DEVICE_CLASS_LIGHTGUN).add_device(guninfo->name, guninfo);
		guninfo->poll = NULL;
	}

	// populate the axes
	for (axisnum = 0; axisnum < 3; axisnum++)
	{
		char *name = utf8_from_tstring(default_axis_name[axisnum]);

		// add to the mouse device and optionally to the gun device as well
		devinfo->device->add_item(name, (input_item_id)(ITEM_ID_XAXIS + axisnum), generic_axis_get_state, &devinfo->mouse.state.lX + axisnum);
		if (guninfo != NULL && axisnum < 2)
			guninfo->device->add_item(name, (input_item_id)(ITEM_ID_XAXIS + axisnum), generic_axis_get_state, &guninfo->mouse.state.lX + axisnum);

		osd_free(name);
	}

	// populate the buttons
	for (butnum = 0; butnum < 5; butnum++)
	{
		char *name = utf8_from_tstring(default_button_name(butnum));

		// add to the mouse device and optionally to the gun device as well
		devinfo->device->add_item(name, (input_item_id)(ITEM_ID_BUTTON1 + butnum), generic_button_get_state, &devinfo->mouse.state.rgbButtons[butnum]);
		if (guninfo != NULL)
			guninfo->device->add_item(name, (input_item_id)(ITEM_ID_BUTTON1 + butnum), generic_button_get_state, &guninfo->mouse.state.rgbButtons[butnum]);

		osd_free(name);
	}
}


//============================================================
//  rawinput_mouse_update
//============================================================

static void rawinput_mouse_update(HANDLE device, RAWMOUSE *data)
{
	device_info *devlist = (data->usFlags & MOUSE_MOVE_ABSOLUTE) ? lightgun_list : mouse_list;
	device_info *devinfo;

	// find the mouse in the list and process
	for (devinfo = devlist; devinfo != NULL; devinfo = devinfo->next)
		if (devinfo->rawinput.device == device)
		{
			// if we got relative data, update it as a mouse
			if (!(data->usFlags & MOUSE_MOVE_ABSOLUTE))
			{
				devinfo->mouse.raw_x += data->lLastX * INPUT_RELATIVE_PER_PIXEL;
				devinfo->mouse.raw_y += data->lLastY * INPUT_RELATIVE_PER_PIXEL;

				// update zaxis
				if (data->usButtonFlags & RI_MOUSE_WHEEL)
					devinfo->mouse.raw_z += (INT16)data->usButtonData * INPUT_RELATIVE_PER_PIXEL;
			}

			// otherwise, update it as a lightgun
			else
			{
				devinfo->mouse.state.lX = normalize_absolute_axis(data->lLastX, 0, 0xffff);
				devinfo->mouse.state.lY = normalize_absolute_axis(data->lLastY, 0, 0xffff);
			}

			// update the button states; always update the corresponding mouse buttons
			if (data->usButtonFlags & RI_MOUSE_BUTTON_1_DOWN) devinfo->mouse.state.rgbButtons[0] = 0x80;
			if (data->usButtonFlags & RI_MOUSE_BUTTON_1_UP)   devinfo->mouse.state.rgbButtons[0] = 0x00;
			if (data->usButtonFlags & RI_MOUSE_BUTTON_2_DOWN) devinfo->mouse.state.rgbButtons[1] = 0x80;
			if (data->usButtonFlags & RI_MOUSE_BUTTON_2_UP)   devinfo->mouse.state.rgbButtons[1] = 0x00;
			if (data->usButtonFlags & RI_MOUSE_BUTTON_3_DOWN) devinfo->mouse.state.rgbButtons[2] = 0x80;
			if (data->usButtonFlags & RI_MOUSE_BUTTON_3_UP)   devinfo->mouse.state.rgbButtons[2] = 0x00;
			if (data->usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) devinfo->mouse.state.rgbButtons[3] = 0x80;
			if (data->usButtonFlags & RI_MOUSE_BUTTON_4_UP)   devinfo->mouse.state.rgbButtons[3] = 0x00;
			if (data->usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) devinfo->mouse.state.rgbButtons[4] = 0x80;
			if (data->usButtonFlags & RI_MOUSE_BUTTON_5_UP)   devinfo->mouse.state.rgbButtons[4] = 0x00;
			break;
		}
}


//============================================================
//  rawinput_mouse_poll
//============================================================

static void rawinput_mouse_poll(device_info *devinfo)
{
	poll_if_necessary(devinfo->machine());

	// copy the accumulated raw state to the actual state
	osd_lock_acquire(input_lock);
	devinfo->mouse.state.lX = devinfo->mouse.raw_x;
	devinfo->mouse.state.lY = devinfo->mouse.raw_y;
	devinfo->mouse.state.lZ = devinfo->mouse.raw_z;
	devinfo->mouse.raw_x = 0;
	devinfo->mouse.raw_y = 0;
	devinfo->mouse.raw_z = 0;
	osd_lock_release(input_lock);
}


//============================================================
//  reg_query_string
//============================================================

static TCHAR *reg_query_string(HKEY key, const TCHAR *path)
{
	TCHAR *buffer;
	DWORD datalen;
	LONG result;

	// first query to get the length
	result = RegQueryValueEx(key, path, NULL, NULL, NULL, &datalen);
	if (result != ERROR_SUCCESS)
		return NULL;

	// allocate a buffer
	buffer = global_alloc_array(TCHAR, datalen + sizeof(*buffer));
	buffer[datalen / sizeof(*buffer)] = 0;

	// now get the actual data
	result = RegQueryValueEx(key, path, NULL, NULL, (LPBYTE)buffer, &datalen);
	if (result == ERROR_SUCCESS)
		return buffer;

	// otherwise return a NULL buffer
	global_free_array(buffer);
	return NULL;
}


//============================================================
//  default_button_name
//============================================================

static const TCHAR *default_button_name(int which)
{
	static TCHAR buffer[20];
	_sntprintf(buffer, ARRAY_LENGTH(buffer), TEXT("B%d"), which);
	return buffer;
}


//============================================================
//  default_pov_name
//============================================================

static const TCHAR *default_pov_name(int which)
{
	static TCHAR buffer[20];
	_sntprintf(buffer, ARRAY_LENGTH(buffer), TEXT("POV%d"), which);
	return buffer;
}
