// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Brad Hughes
//============================================================
//
//  input_win32.cpp - Win32 input implementation
//
//============================================================

#include "input_module.h"

#if defined(OSD_WINDOWS)

#include "input_windows.h"

#include "input_wincommon.h"

// osd/windows
#include "window.h"

// emu
#include "inpttype.h"

#include "strconv.h"

// standard windows headers
#include <tchar.h>


namespace osd {

namespace {

//============================================================
//  win32_keyboard_device
//============================================================

// This device is purely event driven so the implementation is in the module
class win32_keyboard_device : public event_based_device<KeyPressEventArgs>
{
public:
	win32_keyboard_device(std::string &&name, std::string &&id, input_module &module) :
		event_based_device(std::move(name), std::move(id), module),
		m_keyboard({{0}})
	{
	}

	virtual void reset() override
	{
		event_based_device::reset();
		memset(&m_keyboard, 0, sizeof(m_keyboard));
	}

	virtual void configure(input_device &device) override
	{
		keyboard_trans_table const &table = keyboard_trans_table::instance();

		for (int keynum = 0; keynum < MAX_KEYS; keynum++)
		{
			input_item_id itemid = table.map_di_scancode_to_itemid(keynum);
			TCHAR keyname[100];

			// generate the name
			// FIXME: GetKeyNameText gives bogus names for media keys and various other things
			// in many cases it ignores the "extended" bit and returns the key name corresponding to the scan code alone
			if (GetKeyNameText(((keynum & 0x7f) << 16) | ((keynum & 0x80) << 17), keyname, std::size(keyname)) == 0)
				_sntprintf(keyname, std::size(keyname), TEXT("Scan%03d"), keynum);
			std::string name = text::from_tstring(keyname);

			// add the item to the device
			device.add_item(
					name,
					util::string_format("SCAN%03d", keynum),
					itemid,
					generic_button_get_state<std::uint8_t>,
					&m_keyboard.state[keynum]);
		}
	}

protected:
	virtual void process_event(KeyPressEventArgs const &args) override
	{
		m_keyboard.state[args.scancode] = args.event_id == INPUT_EVENT_KEYDOWN ? 0x80 : 0x00;
	}

private:
	keyboard_state m_keyboard;
};


//============================================================
//  keyboard_input_win32 - win32 keyboard input module
//============================================================

class keyboard_input_win32 : public wininput_module<win32_keyboard_device>
{
public:
	keyboard_input_win32() : wininput_module<win32_keyboard_device>(OSD_KEYBOARDINPUT_PROVIDER, "win32")
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		wininput_module<win32_keyboard_device>::input_init(machine);

		// Add a single win32 keyboard device that we'll monitor using Win32
		create_device<win32_keyboard_device>(DEVICE_CLASS_KEYBOARD, "Win32 Keyboard 1", "Win32 Keyboard 1");
	}

	virtual bool handle_input_event(input_event eventid, void const *eventdata) override
	{
		switch (eventid)
		{
		case INPUT_EVENT_KEYDOWN:
		case INPUT_EVENT_KEYUP:
			devicelist().for_each_device(
					[args = reinterpret_cast<KeyPressEventArgs const *>(eventdata)] (auto &device)
					{
						device.queue_events(args, 1);
					});
			return false; // we still want text input events to be generated

		default:
			return false;
		}
	}
};


//============================================================
//  win32_mouse_device
//============================================================

class win32_mouse_device : public event_based_device<MouseUpdateEventArgs>
{
public:
	win32_mouse_device(std::string &&name, std::string &&id, input_module &module) :
		event_based_device(std::move(name), std::move(id), module),
		m_mouse({0}),
		m_win32_mouse({{0}}),
		m_vscroll(0),
		m_hscroll(0)
	{
	}

	virtual void poll(bool relative_reset) override
	{
		event_based_device::poll(relative_reset);

		if (!relative_reset)
			return;

		CURSORINFO cursor_info = { 0 };
		cursor_info.cbSize = sizeof(CURSORINFO);
		GetCursorInfo(&cursor_info);

		// We only take over the mouse if the cursor isn't showing
		// This should happen anyway in mouse mode
		if (!(cursor_info.flags & CURSOR_SHOWING))
		{
			// We measure the position change from the previously set center position
			m_mouse.lX = (cursor_info.ptScreenPos.x - m_win32_mouse.last_point.x) * input_device::RELATIVE_PER_PIXEL;
			m_mouse.lY = (cursor_info.ptScreenPos.y - m_win32_mouse.last_point.y) * input_device::RELATIVE_PER_PIXEL;

			RECT window_pos = {0};
			GetWindowRect(
					dynamic_cast<win_window_info &>(*osd_common_t::window_list().front()).platform_window(),
					&window_pos);

			// We reset the cursor position to the middle of the window each frame
			m_win32_mouse.last_point.x = window_pos.left + (window_pos.right - window_pos.left) / 2;
			m_win32_mouse.last_point.y = window_pos.top + (window_pos.bottom - window_pos.top) / 2;

			SetCursorPos(m_win32_mouse.last_point.x, m_win32_mouse.last_point.y);
		}

		// update scroll axes
		m_mouse.lV = std::exchange(m_vscroll, 0) * input_device::RELATIVE_PER_PIXEL;
		m_mouse.lH = std::exchange(m_hscroll, 0) * input_device::RELATIVE_PER_PIXEL;
	}

	virtual void configure(input_device &device) override
	{
		// populate the axes
		device.add_item(
				"X",
				std::string_view(),
				ITEM_ID_XAXIS,
				generic_axis_get_state<LONG>,
				&m_mouse.lX);
		device.add_item(
				"Y",
				std::string_view(),
				ITEM_ID_YAXIS,
				generic_axis_get_state<LONG>,
				&m_mouse.lY);
		device.add_item(
				"Scroll V",
				std::string_view(),
				ITEM_ID_ZAXIS,
				generic_axis_get_state<LONG>,
				&m_mouse.lV);
		device.add_item(
				"Scroll H",
				std::string_view(),
				ITEM_ID_RZAXIS,
				generic_axis_get_state<LONG>,
				&m_mouse.lH);

		// populate the buttons
		for (int butnum = 0; butnum < 5; butnum++)
		{
			device.add_item(
					default_button_name(butnum),
					std::string_view(),
					input_item_id(ITEM_ID_BUTTON1 + butnum),
					generic_button_get_state<BYTE>,
					&m_mouse.rgbButtons[butnum]);
		}
	}

	virtual void reset() override
	{
		event_based_device::reset();
		memset(&m_mouse, 0, sizeof(m_mouse));
		memset(&m_win32_mouse, 0, sizeof(m_win32_mouse));
		m_vscroll = m_hscroll = 0;
	}

protected:
	virtual void process_event(MouseUpdateEventArgs const &args) override
	{
		// set the button state
		assert(!(args.pressed & args.released));
		for (unsigned i = 0; 5 > i; ++i)
		{
			if (BIT(args.pressed, i))
				m_mouse.rgbButtons[i] = 0x80;
			else if (BIT(args.released, i))
				m_mouse.rgbButtons[i] = 0x00;
		}

		// accumulate scroll delta
		m_vscroll += args.vdelta;
		m_hscroll += args.hdelta;
	}

private:
	struct win32_mouse_state
	{
		POINT last_point;
	};

	mouse_state         m_mouse;
	win32_mouse_state   m_win32_mouse;
	long                m_vscroll, m_hscroll;
};


//============================================================
//  mouse_input_win32 - win32 mouse input module
//============================================================

class mouse_input_win32 : public wininput_module<win32_mouse_device>
{
public:
	mouse_input_win32() : wininput_module<win32_mouse_device>(OSD_MOUSEINPUT_PROVIDER, "win32")
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		wininput_module<win32_mouse_device>::input_init(machine);

		if (!options()->mouse())
			return;

		// allocate a device
		create_device<win32_mouse_device>(DEVICE_CLASS_MOUSE, "Win32 Mouse 1", "Win32 Mouse 1");
	}

	virtual bool handle_input_event(input_event eventid, void const *eventdata) override
	{
		if (manager().class_enabled(DEVICE_CLASS_MOUSE))
		{
			if ((eventid == INPUT_EVENT_MOUSE_BUTTON) || (eventid == INPUT_EVENT_MOUSE_WHEEL))
			{
				auto const *const args = reinterpret_cast<MouseUpdateEventArgs const *>(eventdata);
				devicelist().for_each_device(
						[args] (auto &device) { device.queue_events(args, 1); });
				return true;
			}
		}

		return false;
	}
};


//============================================================
//  win32_lightgun_device_base
//============================================================

class win32_lightgun_device_base : public event_based_device<MouseUpdateEventArgs>
{
public:
	virtual void reset() override
	{
		event_based_device::reset();
		memset(&m_mouse, 0, sizeof(m_mouse));
	}

protected:
	win32_lightgun_device_base(
			std::string &&name,
			std::string &&id,
			input_module &module) :
		event_based_device(std::move(name), std::move(id), module),
		m_mouse({ 0 })
	{
	}

	void do_configure(input_device &device, unsigned buttons)
	{
		// populate the axes
		for (int axisnum = 0; axisnum < 2; axisnum++)
		{
			device.add_item(
					default_axis_name[axisnum],
					std::string_view(),
					input_item_id(ITEM_ID_XAXIS + axisnum),
					generic_axis_get_state<LONG>,
					&m_mouse.lX + axisnum);
		}

		// populate the buttons
		for (int butnum = 0; butnum < buttons; butnum++)
		{
			device.add_item(
					default_button_name(butnum),
					std::string_view(),
					input_item_id(ITEM_ID_BUTTON1 + butnum),
					generic_button_get_state<BYTE>,
					&m_mouse.rgbButtons[butnum]);
		}
	}

	mouse_state m_mouse;
};



//============================================================
//  win32_lightgun_device
//============================================================

class win32_lightgun_device : public win32_lightgun_device_base
{
public:
	win32_lightgun_device(
			std::string &&name,
			std::string &&id,
			input_module &module) :
		win32_lightgun_device_base(std::move(name), std::move(id), module),
		m_vscroll(0),
		m_hscroll(0)
	{
	}

	virtual void poll(bool relative_reset) override
	{
		event_based_device::poll(relative_reset);

		int32_t xpos = 0, ypos = 0;

		// get the cursor position and transform into final results
		POINT mousepos;
		GetCursorPos(&mousepos);
		if (!osd_common_t::window_list().empty())
		{
			// get the position relative to the window
			HWND const hwnd = dynamic_cast<win_window_info &>(*osd_common_t::window_list().front()).platform_window();
			RECT client_rect;
			GetClientRect(hwnd, &client_rect);
			ScreenToClient(hwnd, &mousepos);

			// convert to absolute coordinates
			xpos = normalize_absolute_axis(mousepos.x, client_rect.left, client_rect.right);
			ypos = normalize_absolute_axis(mousepos.y, client_rect.top, client_rect.bottom);
		}

		// update the X/Y positions
		m_mouse.lX = xpos;
		m_mouse.lY = ypos;

		// update the scroll axes if appropriate
		if (relative_reset)
		{
			m_mouse.lV = std::exchange(m_vscroll, 0) * input_device::RELATIVE_PER_PIXEL;
			m_mouse.lH = std::exchange(m_hscroll, 0) * input_device::RELATIVE_PER_PIXEL;
		}
	}

	virtual void configure(input_device &device) override
	{
		do_configure(device, 5);

		// add scroll axes
		device.add_item(
				"Scroll V",
				std::string_view(),
				ITEM_ID_ADD_RELATIVE1,
				generic_axis_get_state<LONG>,
				&m_mouse.lV);
		device.add_item(
				"Scroll H",
				std::string_view(),
				ITEM_ID_ADD_RELATIVE2,
				generic_axis_get_state<LONG>,
				&m_mouse.lH);
	}

	virtual void reset() override
	{
		win32_lightgun_device_base::reset();
		m_vscroll = m_hscroll = 0;
	}

protected:
	virtual void process_event(MouseUpdateEventArgs const &args) override
	{
		// In non-shared axis mode, just update the button state
		assert(!(args.pressed & args.released));
		for (unsigned i = 0; 5 > i; ++i)
		{
			if (BIT(args.pressed, i))
				m_mouse.rgbButtons[i] = 0x80;
			else if (BIT(args.released, i))
				m_mouse.rgbButtons[i] = 0x00;
		}

		// accumulate scroll delta
		m_vscroll += args.vdelta;
		m_hscroll += args.hdelta;
	}

private:
	long                m_vscroll, m_hscroll;
};


//============================================================
//  win32_dual_lightgun_device
//============================================================

class win32_dual_lightgun_device : public win32_lightgun_device_base
{
public:
	win32_dual_lightgun_device(
			std::string &&name,
			std::string &&id,
			input_module &module,
			int index) :
		win32_lightgun_device_base(std::move(name), std::move(id), module),
		m_gun_index(index)
	{
	}

	virtual void configure(input_device &device) override
	{
		do_configure(device, 2);
	}

protected:
	virtual void process_event(MouseUpdateEventArgs const &args) override
	{
		// We only handle the first four buttons in shared axis mode
		assert(!(args.pressed & args.released));
		for (unsigned i = 0; 2 > i; ++i)
		{
			// Adjust the button if we're the second lightgun
			unsigned const bit = i + ((1 == m_gun_index) ? 2 : 0);
			if (BIT(args.pressed, bit))
			{
				m_mouse.rgbButtons[i] = 0x80;

				// get the position relative to the window
				HWND const hwnd = dynamic_cast<win_window_info &>(*osd_common_t::window_list().front()).platform_window();
				RECT client_rect;
				GetClientRect(hwnd, &client_rect);

				POINT mousepos;
				mousepos.x = args.xpos;
				mousepos.y = args.ypos;
				ScreenToClient(hwnd, &mousepos);

				// convert to absolute coordinates
				m_mouse.lX = normalize_absolute_axis(mousepos.x, client_rect.left, client_rect.right);
				m_mouse.lY = normalize_absolute_axis(mousepos.y, client_rect.top, client_rect.bottom);
			}
			else if (BIT(args.released, bit))
			{
				m_mouse.rgbButtons[i] = 0x00;
			}
		}
	}

private:
	int const m_gun_index;
};


//============================================================
//  lightgun_input_win32 - win32 lightgun input module
//============================================================

class lightgun_input_win32 : public wininput_module<win32_lightgun_device_base>
{
public:
	lightgun_input_win32() : wininput_module<win32_lightgun_device_base>(OSD_LIGHTGUNINPUT_PROVIDER, "win32")
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		wininput_module<win32_lightgun_device_base>::input_init(machine);

		bool const shared_axis_mode = dynamic_cast<windows_options const &>(*options()).dual_lightgun();
		int const max_guns = shared_axis_mode ? 2 : 1;

		// allocate the lightgun devices
		for (int gunnum = 0; gunnum < max_guns; gunnum++)
		{
			static const char *const gun_names[] = { "Win32 Gun 1", "Win32 Gun 2" };

			// allocate a device
			if (shared_axis_mode)
				create_device<win32_dual_lightgun_device>(DEVICE_CLASS_LIGHTGUN, gun_names[gunnum], gun_names[gunnum], gunnum);
			else
				create_device<win32_lightgun_device>(DEVICE_CLASS_LIGHTGUN, gun_names[gunnum], gun_names[gunnum]);
		}
	}

	virtual bool handle_input_event(input_event eventid, void const *eventdata) override
	{
		if (manager().class_enabled(DEVICE_CLASS_LIGHTGUN))
		{
			if ((eventid == INPUT_EVENT_MOUSE_BUTTON) || (eventid == INPUT_EVENT_MOUSE_WHEEL))
			{
				auto const *const args = reinterpret_cast<MouseUpdateEventArgs const *>(eventdata);
				devicelist().for_each_device(
						[args] (auto &device) { device.queue_events(args, 1); });
				return true;
			}
		}

		return false;
	}
};

} // anonymous namespace

} // namespace osd

#else // defined(OSD_WINDOWS)

namespace osd {

namespace {

MODULE_NOT_SUPPORTED(keyboard_input_win32, OSD_KEYBOARDINPUT_PROVIDER, "win32")
MODULE_NOT_SUPPORTED(mouse_input_win32, OSD_MOUSEINPUT_PROVIDER, "win32")
MODULE_NOT_SUPPORTED(lightgun_input_win32, OSD_LIGHTGUNINPUT_PROVIDER, "win32")

} // anonymous namespace

} // namespace osd

#endif // defined(OSD_WINDOWS)

MODULE_DEFINITION(KEYBOARDINPUT_WIN32, osd::keyboard_input_win32)
MODULE_DEFINITION(MOUSEINPUT_WIN32, osd::mouse_input_win32)
MODULE_DEFINITION(LIGHTGUNINPUT_WIN32, osd::lightgun_input_win32)
