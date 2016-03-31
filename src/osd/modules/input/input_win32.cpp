// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Brad Hughes
//============================================================
//
//  input_win32.cpp - Win32 input implementation
//
//============================================================

#include "input_module.h"
#include "modules/osdmodule.h"

#if defined(OSD_WINDOWS)

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef interface

// MAME headers
#include "emu.h"
#include "osdepend.h"

// MAMEOS headers
#include "winmain.h"
#include "window.h"

#include "input_common.h"
#include "input_windows.h"

//============================================================
//  win32_keyboard_device
//============================================================

// This device is purely event driven so the implementation is in the module
class win32_keyboard_device : public event_based_device<KeyPressEventArgs>
{
public:
	keyboard_state keyboard;

	win32_keyboard_device(running_machine& machine, const char *name, input_module &module)
		: event_based_device(machine, name, DEVICE_CLASS_KEYBOARD, module),
			keyboard({{0}})
	{
	}

	void reset() override
	{
		memset(&keyboard, 0, sizeof(keyboard));
	}

protected:
	void process_event(KeyPressEventArgs &args) override
	{
		keyboard.state[args.scancode] = args.event_id == INPUT_EVENT_KEYDOWN ? 0x80 : 0x00;
	}
};

//============================================================
//  keyboard_input_win32 - win32 keyboard input module
//============================================================

class keyboard_input_win32 : public wininput_module
{
private:

public:
	keyboard_input_win32()
		: wininput_module(OSD_KEYBOARDINPUT_PROVIDER, "win32")
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		// Add a single win32 keyboard device that we'll monitor using Win32
		win32_keyboard_device *devinfo = devicelist()->create_device<win32_keyboard_device>(machine, "Win32 Keyboard 1", *this);

		keyboard_trans_table &table = keyboard_trans_table::instance();

		// populate it
		for (int keynum = 0; keynum < MAX_KEYS; keynum++)
		{
			input_item_id itemid = table.map_di_scancode_to_itemid(keynum);
			char name[20];

			// generate/fetch the name
			_snprintf(name, ARRAY_LENGTH(name), "Scan%03d", keynum);

			// add the item to the device
			devinfo->device()->add_item(name, itemid, generic_button_get_state, &devinfo->keyboard.state[keynum]);
		}
	}

	bool handle_input_event(input_event eventid, void *eventdata) override
	{
		if (!input_enabled())
			return FALSE;

		KeyPressEventArgs *args;

		switch (eventid)
		{
			case INPUT_EVENT_KEYDOWN:
			case INPUT_EVENT_KEYUP:
				args = static_cast<KeyPressEventArgs*>(eventdata);
				for (int i = 0; i < devicelist()->size(); i++)
					downcast<win32_keyboard_device*>(devicelist()->at(i))->queue_events(args, 1);

				return TRUE;

			default:
				return FALSE;
		}
	}
};

//============================================================
//  win32_mouse_device
//============================================================

struct win32_mouse_state
{
	POINT last_point;
};

class win32_mouse_device : public event_based_device<MouseButtonEventArgs>
{
public:
	mouse_state         mouse;
	win32_mouse_state   win32_mouse;

	win32_mouse_device(running_machine& machine, const char *name, input_module &module)
		: event_based_device(machine, name, DEVICE_CLASS_MOUSE, module),
			mouse({0}),
			win32_mouse({{0}})
	{
	}

	void poll() override
	{
		event_based_device::poll();

		CURSORINFO cursor_info = {0};
		cursor_info.cbSize = sizeof(CURSORINFO);
		GetCursorInfo(&cursor_info);

		// We only take over the mouse if the cursor isn't showing
		// This should happen anyway in mouse mode
		if (!(cursor_info.flags & CURSOR_SHOWING))
		{
			// We measure the position change from the previously set center position
			mouse.lX = (cursor_info.ptScreenPos.x - win32_mouse.last_point.x) * INPUT_RELATIVE_PER_PIXEL;
			mouse.lY = (cursor_info.ptScreenPos.y - win32_mouse.last_point.y) * INPUT_RELATIVE_PER_PIXEL;

			RECT window_pos = {0};
			GetWindowRect(win_window_list->m_hwnd, &window_pos);

			// We reset the cursor position to the middle of the window each frame
			win32_mouse.last_point.x = window_pos.left + (window_pos.right - window_pos.left) / 2;
			win32_mouse.last_point.y = window_pos.top + (window_pos.bottom - window_pos.top) / 2;

			SetCursorPos(win32_mouse.last_point.x, win32_mouse.last_point.y);
		}
	}

	void reset() override
	{
		memset(&mouse, 0, sizeof(mouse));
		memset(&win32_mouse, 0, sizeof(win32_mouse));
	}

protected:
	void process_event(MouseButtonEventArgs &args) override
	{
		// set the button state
		mouse.rgbButtons[args.button] = args.keydown ? 0x80 : 0x00;

		// Make sure we have a fresh mouse position on button down
		if (args.keydown)
			module().poll_if_necessary(machine());
	}
};

//============================================================
//  mouse_input_win32 - win32 mouse input module
//============================================================

class mouse_input_win32 : public wininput_module
{
public:
	mouse_input_win32()
		: wininput_module(OSD_MOUSEINPUT_PROVIDER, "win32")
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		win32_mouse_device *devinfo;
		int axisnum, butnum;

		if (!input_enabled() || !mouse_enabled())
			return;

		// allocate a device
		devinfo = devicelist()->create_device<win32_mouse_device>(machine, "Win32 Mouse 1", *this);
		if (devinfo == NULL)
			return;

		// populate the axes
		for (axisnum = 0; axisnum < 2; axisnum++)
		{
			devinfo->device()->add_item(default_axis_name[axisnum], (input_item_id)(ITEM_ID_XAXIS + axisnum), generic_axis_get_state, &devinfo->mouse.lX + axisnum);
		}

		// populate the buttons
		for (butnum = 0; butnum < 2; butnum++)
		{
			devinfo->device()->add_item(default_button_name(butnum), (input_item_id)(ITEM_ID_BUTTON1 + butnum), generic_button_get_state, &devinfo->mouse.rgbButtons[butnum]);
		}
	}

	bool handle_input_event(input_event eventid, void *eventdata) override
	{
		if (!input_enabled() || !mouse_enabled() || eventid != INPUT_EVENT_MOUSE_BUTTON)
			return FALSE;

		auto args = static_cast<MouseButtonEventArgs*>(eventdata);
		for (int i = 0; i < devicelist()->size(); i++)
			downcast<win32_mouse_device*>(devicelist()->at(i))->queue_events(args, 1);

		return TRUE;
	}
};

//============================================================
//  win32_lightgun_device
//============================================================

class win32_lightgun_device : public event_based_device<MouseButtonEventArgs>
{
private:
	BOOL m_lightgun_shared_axis_mode;
	int m_gun_index;

public:
	mouse_state     mouse;

	win32_lightgun_device(running_machine& machine, const char *name, input_module &module)
		: event_based_device(machine, name, DEVICE_CLASS_LIGHTGUN, module),
			m_lightgun_shared_axis_mode(FALSE),
			m_gun_index(0),
			mouse({0})
	{
		m_lightgun_shared_axis_mode = downcast<windows_options &>(machine.options()).dual_lightgun();

		// Since we are about to be added to the list, the current size is the zero-based index of where we will be
		m_gun_index = downcast<wininput_module&>(module).devicelist()->size();
	}

	void poll() override
	{
		event_based_device::poll();

		INT32 xpos = 0, ypos = 0;
		POINT mousepos;

		// if we are using the shared axis hack, the data is updated via Windows messages only
		if (m_lightgun_shared_axis_mode)
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
		mouse.lX = xpos;
		mouse.lY = ypos;
	}

	void reset() override
	{
		memset(&mouse, 0, sizeof(mouse));
	}

protected:
	void process_event(MouseButtonEventArgs &args) override
	{
		// Are we in shared axis mode?
		if (m_lightgun_shared_axis_mode)
		{
			handle_shared_axis_mode(args);
		}
		else
		{
			// In non-shared axis mode, just update the button state
			mouse.rgbButtons[args.button] = args.keydown ? 0x80 : 0x00;
		}
	}

private:
	void handle_shared_axis_mode(MouseButtonEventArgs &args)
	{
		int button = args.button;

		// We only handle the first four buttons in shared axis mode
		if (button > 3)
			return;

		// First gun doesn't handle buttons 2 & 3
		if (button >= 2 && m_gun_index == 0)
			return;

		// Second gun doesn't handle buttons 0 & 1
		if (button < 2 && m_gun_index == 1)
			return;

		// Adjust the button if we're the second lightgun
		int logical_button = m_gun_index == 1 ? button - 2 : button;

		// set the button state
		mouse.rgbButtons[logical_button] = args.keydown ? 0x80 : 0x00;
		if (args.keydown)
		{
			RECT client_rect;
			POINT mousepos;

			// get the position relative to the window
			GetClientRect(win_window_list->m_hwnd, &client_rect);
			mousepos.x = args.xpos;
			mousepos.y = args.ypos;
			ScreenToClient(win_window_list->m_hwnd, &mousepos);

			// convert to absolute coordinates
			mouse.lX = normalize_absolute_axis(mousepos.x, client_rect.left, client_rect.right);
			mouse.lY = normalize_absolute_axis(mousepos.y, client_rect.top, client_rect.bottom);
		}
	}
};

//============================================================
//  lightgun_input_win32 - win32 lightgun input module
//============================================================

class lightgun_input_win32 : public wininput_module
{
public:
	lightgun_input_win32()
		: wininput_module(OSD_LIGHTGUNINPUT_PROVIDER, "win32")
	{
	}

	int init_internal() override
	{
		return 0;
	}

	virtual void input_init(running_machine &machine) override
	{
		int max_guns = downcast<windows_options&>(machine.options()).dual_lightgun() ? 2 : 1;

		// allocate the lightgun devices
		for (int gunnum = 0; gunnum < max_guns; gunnum++)
		{
			static const char *const gun_names[] = { "Win32 Gun 1", "Win32 Gun 2" };
			win32_lightgun_device *devinfo;
			int axisnum, butnum;

			// allocate a device
			devinfo = devicelist()->create_device<win32_lightgun_device>(machine, gun_names[gunnum], *this);
			if (devinfo == NULL)
				break;

			// populate the axes
			for (axisnum = 0; axisnum < 2; axisnum++)
			{
				devinfo->device()->add_item(default_axis_name[axisnum], (input_item_id)(ITEM_ID_XAXIS + axisnum), generic_axis_get_state, &devinfo->mouse.lX + axisnum);
			}

			// populate the buttons
			for (butnum = 0; butnum < 2; butnum++)
			{
				devinfo->device()->add_item(default_button_name(butnum), (input_item_id)(ITEM_ID_BUTTON1 + butnum), generic_button_get_state, &devinfo->mouse.rgbButtons[butnum]);
			}
		}
	}

	bool handle_input_event(input_event eventid, void* eventdata) override
	{
		if (!input_enabled() || !lightgun_enabled() || eventid != INPUT_EVENT_MOUSE_BUTTON)
			return false;

		for (int i = 0; i < devicelist()->size(); i++)
			downcast<win32_lightgun_device*>(devicelist()->at(i))->queue_events(static_cast<MouseButtonEventArgs*>(eventdata), 1);

		return true;
	}
};

#else
MODULE_NOT_SUPPORTED(keyboard_input_win32, OSD_KEYBOARDINPUT_PROVIDER, "win32")
MODULE_NOT_SUPPORTED(mouse_input_win32, OSD_MOUSEINPUT_PROVIDER, "win32")
MODULE_NOT_SUPPORTED(lightgun_input_win32, OSD_LIGHTGUNINPUT_PROVIDER, "win32")
#endif

MODULE_DEFINITION(KEYBOARDINPUT_WIN32, keyboard_input_win32)
MODULE_DEFINITION(MOUSEINPUT_WIN32, mouse_input_win32)
MODULE_DEFINITION(LIGHTGUNINPUT_WIN32, lightgun_input_win32)
