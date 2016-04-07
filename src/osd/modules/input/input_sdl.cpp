// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Brad Hughes
//============================================================
//
//  input_sdl.cpp - SDL 2.0 implementation of MAME input routines
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//  SixAxis info: left analog is axes 0 & 1, right analog is axes 2 & 3,
//                analog L2 is axis 12 and analog L3 is axis 13
//
//============================================================

#include "input_module.h"
#include "modules/osdmodule.h"

#if defined(SDLMAME_SDL2)

// standard sdl header
#include "sdlinc.h"
#include <ctype.h>
#include <stddef.h>
#include <mutex>
#include <memory>
#include <queue>

// MAME headers
#include "emu.h"
#include "osdepend.h"
#include "ui/ui.h"
#include "uiinput.h"
#include "strconv.h"

// MAMEOS headers
#include "input_common.h"
#include "../lib/osdobj_common.h"
#include "input_sdlcommon.h"
#include "../../sdl/osdsdl.h"
#include "../../sdl/window.h"

// winnt.h defines this
#ifdef DELETE
#undef DELETE
#endif


// FIXME: sdl does not properly report the window for certain OS.
#define GET_FOCUS_WINDOW(ev) focus_window()
//#define GET_FOCUS_WINDOW(ev) window_from_id((ev)->windowID)

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

static key_lookup_table sdl_lookup_table[] =
{
	KE7(UNKNOWN,    BACKSPACE,  TAB,        CLEAR,      RETURN,     PAUSE,      ESCAPE)
	KE(SPACE)
	KE5(COMMA,      MINUS,      PERIOD,     SLASH,      0)
	KE8(1,          2,          3,              4,          5,          6,          7,          8)
	KE3(9,          SEMICOLON,      EQUALS)
	KE5(LEFTBRACKET,BACKSLASH,  RIGHTBRACKET,   A,          B)
	KE8(C,          D,          E,              F,          G,          H,          I,          J)
	KE8(K,          L,          M,              N,          O,          P,          Q,          R)
	KE8(S,          T,          U,              V,          W,          X,          Y,          Z)
	KE8(DELETE,     KP_0,       KP_1,           KP_2,       KP_3,       KP_4,       KP_5,       KP_6)
	KE8(KP_7,       KP_8,       KP_9,           KP_PERIOD,  KP_DIVIDE,  KP_MULTIPLY,KP_MINUS,   KP_PLUS)
	KE8(KP_ENTER,   KP_EQUALS,  UP,             DOWN,       RIGHT,      LEFT,       INSERT,     HOME)
	KE8(END,        PAGEUP,     PAGEDOWN,       F1,         F2,         F3,         F4,         F5)
	KE8(F6,         F7,         F8,             F9,         F10,        F11,        F12,        F13)
	KE8(F14,        F15,        NUMLOCKCLEAR,   CAPSLOCK,   SCROLLLOCK, RSHIFT,     LSHIFT,     RCTRL)
	KE5(LCTRL,      RALT,       LALT,           LGUI,       RGUI)
	KE8(GRAVE,      LEFTBRACKET,RIGHTBRACKET,   SEMICOLON,  APOSTROPHE, BACKSLASH,  PRINTSCREEN,MENU)
	KE(UNDO)
{
	-1, ""
}
};

//============================================================
//  lookup_sdl_code
//============================================================

static int lookup_sdl_code(const char *scode)
{
	int i = 0;

	while (sdl_lookup_table[i].code >= 0)
	{
		if (!strcmp(scode, sdl_lookup_table[i].name))
			return sdl_lookup_table[i].code;
		i++;
	}
	return -1;
}

//============================================================
//  sdl_device
//============================================================

class sdl_device : public event_based_device<SDL_Event>
{
public:
	sdl_device(running_machine &machine, const char* name, input_device_class devclass, input_module &module)
		: event_based_device(machine, name, devclass, module)
	{
	}

protected:
	sdl_window_info * focus_window()
	{
		return sdl_event_manager::instance().focus_window();
	}
};

//============================================================
//  sdl_keyboard_device
//============================================================

#define OSD_SDL_INDEX_KEYSYM(keysym) ((keysym)->scancode)
class sdl_keyboard_device : public sdl_device
{
public:
	keyboard_state keyboard;

	sdl_keyboard_device(running_machine &machine, const char* name, input_module &module)
		: sdl_device(machine, name, DEVICE_CLASS_KEYBOARD, module),
		keyboard({{0}})
	{
	}

	void process_event(SDL_Event &sdlevent) override
	{
		switch (sdlevent.type)
		{
		case SDL_KEYDOWN:
			keyboard.state[OSD_SDL_INDEX_KEYSYM(&sdlevent.key.keysym)] = 0x80;
			if (sdlevent.key.keysym.sym < 0x20)
				machine().ui_input().push_char_event(sdl_window_list->target(), sdlevent.key.keysym.sym);
			break;

		case SDL_KEYUP:
			keyboard.state[OSD_SDL_INDEX_KEYSYM(&sdlevent.key.keysym)] = 0x00;
			break;

		case SDL_TEXTINPUT:
			if (*sdlevent.text.text)
			{
				sdl_window_info *window = GET_FOCUS_WINDOW(&event.text);
				//printf("Focus window is %p - wl %p\n", window, sdl_window_list);
				unicode_char result;
				if (window != NULL)
				{
					osd_uchar_from_osdchar(&result, sdlevent.text.text, 1);
					machine().ui_input().push_char_event(window->target(), result);
				}
			}
			break;
		}
	}

	void reset() override
	{
		memset(&keyboard.state, 0, sizeof(keyboard.state));
	}
};

//============================================================
//  sdl_mouse_device
//============================================================

class sdl_mouse_device : public sdl_device
{
public:
	mouse_state mouse;

	sdl_mouse_device(running_machine &machine, const char* name, input_module &module)
		: sdl_device(machine, name, DEVICE_CLASS_MOUSE, module),
		mouse({0})
	{
	}

	void reset() override
	{
		memset(&mouse, 0, sizeof(mouse));
	}

	void poll() override
	{
		mouse.lX = 0;
		mouse.lY = 0;
		sdl_device::poll();
	}

	virtual void process_event(SDL_Event &sdlevent) override
	{
		switch (sdlevent.type)
		{
		case SDL_MOUSEMOTION:
			mouse.lX += sdlevent.motion.xrel * INPUT_RELATIVE_PER_PIXEL;
			mouse.lY += sdlevent.motion.yrel * INPUT_RELATIVE_PER_PIXEL;

			{
				int cx = -1, cy = -1;
				sdl_window_info *window = GET_FOCUS_WINDOW(&sdlevent.motion);

				if (window != NULL && window->xy_to_render_target(sdlevent.motion.x, sdlevent.motion.y, &cx, &cy))
					machine().ui_input().push_mouse_move_event(window->target(), cx, cy);
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
			mouse.buttons[sdlevent.button.button - 1] = 0x80;
			//printf("But down %d %d %d %d %s\n", event.button.which, event.button.button, event.button.x, event.button.y, devinfo->name.c_str());
			if (sdlevent.button.button == 1)
			{
				// FIXME Move static declaration
				static osd_ticks_t last_click = 0;
				static int last_x = 0;
				static int last_y = 0;
				int cx, cy;
				osd_ticks_t click = osd_ticks() * 1000 / osd_ticks_per_second();
				sdl_window_info *window = GET_FOCUS_WINDOW(&sdlevent.button);
				if (window != NULL && window->xy_to_render_target(sdlevent.button.x, sdlevent.button.y, &cx, &cy))
				{
					machine().ui_input().push_mouse_down_event(window->target(), cx, cy);
					// FIXME Parameter ?
					if ((click - last_click < 250)
						&& (cx >= last_x - 4 && cx <= last_x + 4)
						&& (cy >= last_y - 4 && cy <= last_y + 4))
					{
						last_click = 0;
						machine().ui_input().push_mouse_double_click_event(window->target(), cx, cy);
					}
					else
					{
						last_click = click;
						last_x = cx;
						last_y = cy;
					}
				}
			}
			break;

		case SDL_MOUSEBUTTONUP:
			mouse.buttons[sdlevent.button.button - 1] = 0;
			//printf("But up %d %d %d %d\n", event.button.which, event.button.button, event.button.x, event.button.y);

			if (sdlevent.button.button == 1)
			{
				int cx, cy;
				sdl_window_info *window = GET_FOCUS_WINDOW(&sdlevent.button);

				if (window != NULL && window->xy_to_render_target(sdlevent.button.x, sdlevent.button.y, &cx, &cy))
				{
					machine().ui_input().push_mouse_up_event(window->target(), cx, cy);
				}
			}
			break;

		case SDL_MOUSEWHEEL:
			sdl_window_info *window = GET_FOCUS_WINDOW(&sdlevent.wheel);
			if (window != NULL)
				machine().ui_input().push_mouse_wheel_event(window->target(), 0, 0, sdlevent.wheel.y, 3);
			break;
		}
	}
};

//============================================================
//  sdl_joystick_device
//============================================================

// state information for a joystick
struct sdl_joystick_state
{
	INT32 axes[MAX_AXES];
	INT32 buttons[MAX_BUTTONS];
	INT32 hatsU[MAX_HATS], hatsD[MAX_HATS], hatsL[MAX_HATS], hatsR[MAX_HATS];
	INT32 balls[MAX_AXES];
};

struct sdl_api_state
{
	SDL_Joystick *device;
	SDL_JoystickID joystick_id;
};

class sdl_joystick_device : public sdl_device
{
public:
	sdl_joystick_state    joystick;
	sdl_api_state         sdl_state;

	sdl_joystick_device(running_machine &machine, const char *name, input_module &module)
		: sdl_device(machine, name, DEVICE_CLASS_JOYSTICK, module),
			joystick({{0}}),
			sdl_state({0})
	{
	}

	~sdl_joystick_device()
	{
		if (sdl_state.device != nullptr)
		{
			SDL_JoystickClose(sdl_state.device);
			sdl_state.device = nullptr;
		}
	}

	void reset() override
	{
		memset(&joystick, 0, sizeof(joystick));
	}

	void process_event(SDL_Event &sdlevent) override
	{
		switch (sdlevent.type)
		{
		case SDL_JOYAXISMOTION:
			joystick.axes[sdlevent.jaxis.axis] = (sdlevent.jaxis.value * 2);
			break;

		case SDL_JOYBALLMOTION:
			//printf("Ball %d %d\n", sdlevent.jball.xrel, sdlevent.jball.yrel);
			joystick.balls[sdlevent.jball.ball * 2] = sdlevent.jball.xrel * INPUT_RELATIVE_PER_PIXEL;
			joystick.balls[sdlevent.jball.ball * 2 + 1] = sdlevent.jball.yrel * INPUT_RELATIVE_PER_PIXEL;
			break;

		case SDL_JOYHATMOTION:
			if (sdlevent.jhat.value & SDL_HAT_UP)
			{
				joystick.hatsU[sdlevent.jhat.hat] = 0x80;
			}
			else
			{
				joystick.hatsU[sdlevent.jhat.hat] = 0;
			}
			if (sdlevent.jhat.value & SDL_HAT_DOWN)
			{
				joystick.hatsD[sdlevent.jhat.hat] = 0x80;
			}
			else
			{
				joystick.hatsD[sdlevent.jhat.hat] = 0;
			}
			if (sdlevent.jhat.value & SDL_HAT_LEFT)
			{
				joystick.hatsL[sdlevent.jhat.hat] = 0x80;
			}
			else
			{
				joystick.hatsL[sdlevent.jhat.hat] = 0;
			}
			if (sdlevent.jhat.value & SDL_HAT_RIGHT)
			{
				joystick.hatsR[sdlevent.jhat.hat] = 0x80;
			}
			else
			{
				joystick.hatsR[sdlevent.jhat.hat] = 0;
			}
			break;

		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			joystick.buttons[sdlevent.jbutton.button] = (sdlevent.jbutton.state == SDL_PRESSED) ? 0x80 : 0;
			break;
		}
	}
};

class sdl_sixaxis_joystick_device : public sdl_joystick_device
{
public:
	sdl_sixaxis_joystick_device(running_machine &machine, const char *name, input_module &module)
		: sdl_joystick_device(machine, name, module)
	{
	}

	void process_event(SDL_Event &sdlevent) override
	{
		switch (sdlevent.type)
		{
		case SDL_JOYAXISMOTION:
			{
				int axis = sdlevent.jaxis.axis;

				if (axis <= 3)
				{
					joystick.axes[sdlevent.jaxis.axis] = (sdlevent.jaxis.value * 2);
				}
				else
				{
					int magic = (sdlevent.jaxis.value / 2) + 16384;
					joystick.axes[sdlevent.jaxis.axis] = magic;
				}
			}
			break;

		default:
			// Call the base for other events
			sdl_joystick_device::process_event(sdlevent);
			break;
		}
	}
};

//============================================================
//  sdl_input_module
//============================================================

class sdl_input_module : public input_module_base, public sdl_event_subscriber
{
public:
	sdl_input_module(const char *type)
		: input_module_base(type, "sdl")
	{
	}

	void input_init(running_machine &machine) override
	{
		if (machine.debug_flags & DEBUG_FLAG_OSD_ENABLED)
		{
			osd_printf_warning("Debug Build: Disabling input grab for -debug\n");
			set_mouse_enabled(false);
		}
	}

	void exit() override
	{
		// unsubscribe for events
		sdl_event_manager::instance().unsubscribe(this);

		input_module_base::exit();
	}

	void before_poll(running_machine& machine) override
	{
		// Tell the event manager to process events and push them to the devices
		sdl_event_manager::instance().process_events(machine);
	}

	bool should_poll_devices(running_machine& machine) override
	{
		return sdl_event_manager::instance().has_focus() && input_enabled();
	}

	virtual void handle_event(SDL_Event &sdlevent) override
	{
		// By default dispatch event to every device
		for (int i = 0; i < devicelist()->size(); i++)
			downcast<sdl_device*>(devicelist()->at(i))->queue_events(&sdlevent, 1);
	}
};

//============================================================
//  sdl_keyboard_module
//============================================================

class sdl_keyboard_module : public sdl_input_module
{
	keyboard_trans_table * m_key_trans_table;
public:
	sdl_keyboard_module()
		: sdl_input_module(OSD_KEYBOARDINPUT_PROVIDER)
	{
	}

	void input_init(running_machine &machine) override
	{
		sdl_input_module::input_init(machine);

		SDL_EventType event_types[] = { SDL_KEYDOWN, SDL_KEYUP, SDL_TEXTINPUT };
		sdl_event_manager::instance().subscribe(reinterpret_cast<int*>(event_types), ARRAY_LENGTH(event_types), this);

		sdl_keyboard_device *devinfo;

		// Read our keymap and store a pointer to our table
		m_key_trans_table = sdlinput_read_keymap(machine);

		keyboard_trans_table& local_table = *m_key_trans_table;

		osd_printf_verbose("Keyboard: Start initialization\n");

		// SDL only has 1 keyboard add it now
		devinfo = devicelist()->create_device<sdl_keyboard_device>(machine, "System keyboard", *this);

		// populate it
		for (int keynum = 0; local_table[keynum].mame_key != ITEM_ID_INVALID; keynum++)
		{
			input_item_id itemid = local_table[keynum].mame_key;

			// generate the default / modified name
			char defname[20];
			snprintf(defname, sizeof(defname) - 1, "%s", local_table[keynum].ui_name);

			devinfo->device()->add_item(defname, itemid, generic_button_get_state, &devinfo->keyboard.state[local_table[keynum].sdl_scancode]);
		}

		osd_printf_verbose("Keyboard: Registered %s\n", devinfo->name());
		osd_printf_verbose("Keyboard: End initialization\n");
	}

private:
	static keyboard_trans_table* sdlinput_read_keymap(running_machine &machine)
	{
		char *keymap_filename;
		FILE *keymap_file;
		int line = 1;
		int index, i, sk, vk, ak;
		char buf[256];
		char mks[41];
		char sks[41];
		char kns[41];
		int  sdl2section = 0;

		keyboard_trans_table &default_table = keyboard_trans_table::instance();

		if (!machine.options().bool_value(SDLOPTION_KEYMAP))
			return &default_table;

		keymap_filename = (char *)downcast<sdl_options &>(machine.options()).keymap_file();
		osd_printf_verbose("Keymap: Start reading keymap_file %s\n", keymap_filename);

		keymap_file = fopen(keymap_filename, "r");
		if (keymap_file == NULL)
		{
			osd_printf_warning("Keymap: Unable to open keymap %s, using default\n", keymap_filename);
			return &default_table;
		}

		// Allocate a block of translation entries big enough to hold what's in the default table
		auto key_trans_entries = std::make_unique<key_trans_entry[]>(default_table.size());

		// copy the elements from the default table
		for (int i = 0; i < default_table.size(); i++)
			key_trans_entries[i] = default_table[i];

		// Allocate the trans table to be associated with the machine so we don't have to free it
		keyboard_trans_table *custom_table = auto_alloc(machine, keyboard_trans_table(std::move(key_trans_entries), default_table.size()));

		while (!feof(keymap_file))
		{
			char *ret = fgets(buf, 255, keymap_file);
			if (ret && buf[0] != '\n' && buf[0] != '#')
			{
				buf[255] = 0;
				i = strlen(buf);
				if (i && buf[i - 1] == '\n')
					buf[i - 1] = 0;
				if (strncmp(buf, "[SDL2]", 6) == 0)
				{
					sdl2section = 1;
				}
				else if (((SDLMAME_SDL2) ^ sdl2section) == 0)
				{
					mks[0] = 0;
					sks[0] = 0;
					memset(kns, 0, ARRAY_LENGTH(kns));
					sscanf(buf, "%40s %40s %x %x %40c\n",
						mks, sks, &vk, &ak, kns);

					index = default_table.lookup_mame_index(mks);
					sk = lookup_sdl_code(sks);

					if (sk >= 0 && index >= 0)
					{
						key_trans_entry &entry = (*custom_table)[index];
						entry.sdl_key = sk;
						// vk and ak are not really needed
						//key_trans_table[index][VIRTUAL_KEY] = vk;
						//key_trans_table[index][ASCII_KEY] = ak;
						entry.ui_name = auto_alloc_array(machine, char, strlen(kns) + 1);
						strcpy(entry.ui_name, kns);
						osd_printf_verbose("Keymap: Mapped <%s> to <%s> with ui-text <%s>\n", sks, mks, kns);
					}
					else
						osd_printf_warning("Keymap: Error on line %d - %s key not found: %s\n", line, (sk<0) ? "sdl" : "mame", buf);
				}
			}
			line++;
		}
		fclose(keymap_file);
		osd_printf_verbose("Keymap: Processed %d lines\n", line);

		return custom_table;
	}
};

//============================================================
//  sdl_mouse_module
//============================================================

class sdl_mouse_module : public sdl_input_module
{
public:
	sdl_mouse_module()
		: sdl_input_module(OSD_MOUSEINPUT_PROVIDER)
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		sdl_input_module::input_init(machine);

		SDL_EventType event_types[] = { SDL_MOUSEMOTION, SDL_MOUSEBUTTONDOWN, SDL_MOUSEBUTTONUP, SDL_MOUSEWHEEL };
		sdl_event_manager::instance().subscribe(reinterpret_cast<int*>(event_types), ARRAY_LENGTH(event_types), this);

		sdl_mouse_device *devinfo;
		char defname[20];
		int button;

		osd_printf_verbose("Mouse: Start initialization\n");

		// SDL currently only supports one mouse
		devinfo = devicelist()->create_device<sdl_mouse_device>(machine, "System mouse", *this);

		// add the axes
		devinfo->device()->add_item("X", ITEM_ID_XAXIS, generic_axis_get_state, &devinfo->mouse.lX);
		devinfo->device()->add_item("Y", ITEM_ID_YAXIS, generic_axis_get_state, &devinfo->mouse.lY);

		for (button = 0; button < 4; button++)
		{
			input_item_id itemid = (input_item_id)(ITEM_ID_BUTTON1 + button);
			snprintf(defname, sizeof(defname), "B%d", button + 1);

			devinfo->device()->add_item(defname, itemid, generic_button_get_state, &devinfo->mouse.buttons[button]);
		}

		osd_printf_verbose("Mouse: Registered %s\n", devinfo->name());
		osd_printf_verbose("Mouse: End initialization\n");
	}
};


static void devmap_register(device_map_t *devmap, int physical_idx, const std::string &name)
{
	int found = 0;
	int stick, i;

	for (i = 0; i < MAX_DEVMAP_ENTRIES; i++)
	{
		if (strcmp(name.c_str(), devmap->map[i].name.c_str()) == 0 && devmap->map[i].physical < 0)
		{
			devmap->map[i].physical = physical_idx;
			found = 1;
			devmap->logical[physical_idx] = i;
		}
	}

	if (found == 0)
	{
		stick = devmap_leastfree(devmap);
		devmap->map[stick].physical = physical_idx;
		devmap->map[stick].name = name;
		devmap->logical[physical_idx] = stick;
	}

}

//============================================================
//  sdl_joystick_module
//============================================================

class sdl_joystick_module : public sdl_input_module
{
private:
	device_map_t   m_joy_map;
	bool           m_sixaxis_mode;
public:
	sdl_joystick_module()
		: sdl_input_module(OSD_JOYSTICKINPUT_PROVIDER)
	{
	}
	
	virtual void exit() override
	{
		sdl_input_module::exit();
	
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
	}	
	
	virtual void input_init(running_machine &machine) override
	{
	    SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");

		if (SDL_InitSubSystem(SDL_INIT_JOYSTICK))
		{
			osd_printf_error("Could not initialize SDL Joystick: %s.\n", SDL_GetError());
			return;
		}
		
		sdl_input_module::input_init(machine);

		char tempname[512];

		m_sixaxis_mode = downcast<const sdl_options*>(options())->sixaxis();

		devmap_init(machine, &m_joy_map, SDLOPTION_JOYINDEX, 8, "Joystick mapping");

		osd_printf_verbose("Joystick: Start initialization\n");
		int physical_stick;
		for (physical_stick = 0; physical_stick < SDL_NumJoysticks(); physical_stick++)
		{
				std::string joy_name = remove_spaces(SDL_JoystickNameForIndex(physical_stick));
				devmap_register(&m_joy_map, physical_stick, joy_name.c_str());
		}

		for (int stick = 0; stick < MAX_DEVMAP_ENTRIES; stick++)
		{
			sdl_joystick_device *devinfo = create_joystick_device(machine, &m_joy_map, stick, DEVICE_CLASS_JOYSTICK);

			if (devinfo == NULL)
				continue;

			physical_stick = m_joy_map.map[stick].physical;
			SDL_Joystick *joy = SDL_JoystickOpen(physical_stick);
			devinfo->sdl_state.device = joy;
			devinfo->sdl_state.joystick_id = SDL_JoystickInstanceID(joy);

			osd_printf_verbose("Joystick: %s\n", SDL_JoystickNameForIndex(physical_stick));
			osd_printf_verbose("Joystick:   ...  %d axes, %d buttons %d hats %d balls\n", SDL_JoystickNumAxes(joy), SDL_JoystickNumButtons(joy), SDL_JoystickNumHats(joy), SDL_JoystickNumBalls(joy));
			osd_printf_verbose("Joystick:   ...  Physical id %d mapped to logical id %d\n", physical_stick, stick + 1);

			// loop over all axes
			for (int axis = 0; axis < SDL_JoystickNumAxes(joy); axis++)
			{
				input_item_id itemid;

				if (axis < INPUT_MAX_AXIS)
					itemid = (input_item_id)(ITEM_ID_XAXIS + axis);
				else if (axis < INPUT_MAX_AXIS + INPUT_MAX_ADD_ABSOLUTE)
					itemid = (input_item_id)(ITEM_ID_ADD_ABSOLUTE1 - INPUT_MAX_AXIS + axis);
				else
					itemid = ITEM_ID_OTHER_AXIS_ABSOLUTE;

				snprintf(tempname, sizeof(tempname), "A%d %s", axis, devinfo->name());
				devinfo->device()->add_item(tempname, itemid, generic_axis_get_state, &devinfo->joystick.axes[axis]);
			}

			// loop over all buttons
			for (int button = 0; button < SDL_JoystickNumButtons(joy); button++)
			{
				input_item_id itemid;

				devinfo->joystick.buttons[button] = 0;

				if (button < INPUT_MAX_BUTTONS)
					itemid = (input_item_id)(ITEM_ID_BUTTON1 + button);
				else if (button < INPUT_MAX_BUTTONS + INPUT_MAX_ADD_SWITCH)
					itemid = (input_item_id)(ITEM_ID_ADD_SWITCH1 - INPUT_MAX_BUTTONS + button);
				else
					itemid = ITEM_ID_OTHER_SWITCH;

				snprintf(tempname, sizeof(tempname), "button %d", button);
				devinfo->device()->add_item(tempname, itemid, generic_button_get_state, &devinfo->joystick.buttons[button]);
			}

			// loop over all hats
			for (int hat = 0; hat < SDL_JoystickNumHats(joy); hat++)
			{
				input_item_id itemid;

				snprintf(tempname, sizeof(tempname), "hat %d Up", hat);
				itemid = (input_item_id)((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1UP + 4 * hat : ITEM_ID_OTHER_SWITCH);
				devinfo->device()->add_item(tempname, itemid, generic_button_get_state, &devinfo->joystick.hatsU[hat]);
				snprintf(tempname, sizeof(tempname), "hat %d Down", hat);
				itemid = (input_item_id)((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1DOWN + 4 * hat : ITEM_ID_OTHER_SWITCH);
				devinfo->device()->add_item(tempname, itemid, generic_button_get_state, &devinfo->joystick.hatsD[hat]);
				snprintf(tempname, sizeof(tempname), "hat %d Left", hat);
				itemid = (input_item_id)((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1LEFT + 4 * hat : ITEM_ID_OTHER_SWITCH);
				devinfo->device()->add_item(tempname, itemid, generic_button_get_state, &devinfo->joystick.hatsL[hat]);
				snprintf(tempname, sizeof(tempname), "hat %d Right", hat);
				itemid = (input_item_id)((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1RIGHT + 4 * hat : ITEM_ID_OTHER_SWITCH);
				devinfo->device()->add_item(tempname, itemid, generic_button_get_state, &devinfo->joystick.hatsR[hat]);
			}

			// loop over all (track)balls
			for (int ball = 0; ball < SDL_JoystickNumBalls(joy); ball++)
			{
				int itemid;

				if (ball * 2 < INPUT_MAX_ADD_RELATIVE)
					itemid = ITEM_ID_ADD_RELATIVE1 + ball * 2;
				else
					itemid = ITEM_ID_OTHER_AXIS_RELATIVE;

				snprintf(tempname, sizeof(tempname), "R%d %s", ball * 2, devinfo->name());
				devinfo->device()->add_item(tempname, (input_item_id)itemid, generic_axis_get_state, &devinfo->joystick.balls[ball * 2]);
				snprintf(tempname, sizeof(tempname), "R%d %s", ball * 2 + 1, devinfo->name());
				devinfo->device()->add_item(tempname, (input_item_id)(itemid + 1), generic_axis_get_state, &devinfo->joystick.balls[ball * 2 + 1]);
			}
		}

		SDL_EventType event_types[] = { SDL_JOYAXISMOTION, SDL_JOYBALLMOTION, SDL_JOYHATMOTION, SDL_JOYBUTTONDOWN, SDL_JOYBUTTONUP };
		sdl_event_manager::instance().subscribe(reinterpret_cast<int*>(event_types), ARRAY_LENGTH(event_types), this);

		osd_printf_verbose("Joystick: End initialization\n");
	}

	virtual void handle_event(SDL_Event &sdlevent) override
	{
		// Figure out which joystick this event id destined for
		for (int i = 0; i < devicelist()->size(); i++)
		{
			auto joy = downcast<sdl_joystick_device*>(devicelist()->at(i));

			// If we find a matching joystick, dispatch the event to the joystick
			if (joy->sdl_state.joystick_id == sdlevent.jdevice.which)
				joy->queue_events(&sdlevent, 1);
		}
	}

private:
	sdl_joystick_device* create_joystick_device(running_machine &machine, device_map_t *devmap, int index, input_device_class devclass)
	{
		sdl_joystick_device *devinfo = NULL;
		char tempname[20];

		if (devmap->map[index].name.length() == 0)
		{
			/* only map place holders if there were mappings specified is enabled */
			if (devmap->initialized)
			{
				snprintf(tempname, ARRAY_LENGTH(tempname), "NC%d", index);
				devinfo = m_sixaxis_mode
					? devicelist()->create_device<sdl_sixaxis_joystick_device>(machine, tempname, *this)
					: devicelist()->create_device<sdl_joystick_device>(machine, tempname, *this);
			}

			return NULL;
		}
		else
		{
			devinfo = m_sixaxis_mode
				? devicelist()->create_device<sdl_sixaxis_joystick_device>(machine, devmap->map[index].name.c_str(), *this)
				: devicelist()->create_device<sdl_joystick_device>(machine, devmap->map[index].name.c_str(), *this);
		}

		return devinfo;
	}
};

#else
MODULE_NOT_SUPPORTED(sdl_keyboard_module, OSD_KEYBOARDINPUT_PROVIDER, "sdl")
MODULE_NOT_SUPPORTED(sdl_mouse_module, OSD_MOUSEINPUT_PROVIDER, "sdl")
MODULE_NOT_SUPPORTED(sdl_joystick_module, OSD_JOYSTICKINPUT_PROVIDER, "sdl")
#endif

MODULE_DEFINITION(KEYBOARDINPUT_SDL, sdl_keyboard_module)
MODULE_DEFINITION(MOUSEINPUT_SDL, sdl_mouse_module)
MODULE_DEFINITION(JOYSTICKINPUT_SDL, sdl_joystick_module)
