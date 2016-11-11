// license:BSD-3-Clause
// copyright-holders:Brad Hughes
//============================================================
//
//  input_uwp.cpp - UWP input implementation
//
//============================================================

#include "input_module.h"
#include "modules/osdmodule.h"

#if defined(OSD_UWP)

#include <agile.h>
#include <ppltasks.h>
#include <collection.h>
#undef min
#undef max
#undef interface

// MAME headers
#include "emu.h"
#include "uiinput.h"

// MAMEOS headers
#include "winmain.h"
#include "input_common.h"
#include "input_windows.h"

using namespace concurrency;
using namespace Windows::UI::Core;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;

// This device is purely event driven so the implementation is in the module
class uwp_keyboard_device : public event_based_device<KeyPressEventArgs>
{
public:
	keyboard_state keyboard;

	uwp_keyboard_device(running_machine& machine, const char *name, const char *id, input_module &module)
		: event_based_device(machine, name, id, DEVICE_CLASS_KEYBOARD, module),
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
		keyboard.state[args.vkey] = args.event_id == INPUT_EVENT_KEYDOWN ? 0x80 : 0x00;
	}
};

private ref class key_processor sealed
{
private:
	Platform::Agile<CoreWindow^> m_window;
	input_device_list &m_devicelist;
	running_machine &m_machine;

internal:
	key_processor(CoreWindow^ window, input_device_list &devicelist, running_machine &machine)
		: m_window(window),
		m_devicelist(devicelist),
		m_machine(machine)
	{
		m_window->KeyDown += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &key_processor::OnKeyDown);
		m_window->KeyUp += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &key_processor::OnKeyUp);
		m_window->CharacterReceived += ref new TypedEventHandler<CoreWindow^, CharacterReceivedEventArgs^>(this, &key_processor::OnCharacterReceived);
	}

	void OnKeyDown(CoreWindow^ win, KeyEventArgs^ args)
	{
		auto &table = keyboard_trans_table::instance();

		//if (args->VirtualKey < Windows::System::VirtualKey::Space)
		//	m_machine.ui_input().push_char_event(osd_common_t::s_window_list.front()->target(), static_cast<char32_t>(args->VirtualKey));

		KeyPressEventArgs tmp;
		tmp.event_id = INPUT_EVENT_KEYDOWN;
		tmp.vkey = uint8_t(args->VirtualKey);
		m_devicelist.for_each_device([&tmp](device_info *device)
		{
			static_cast<uwp_keyboard_device*>(device)->queue_events(&tmp, 1);
		});
	}

	void OnKeyUp(CoreWindow^ win, KeyEventArgs^ args)
	{
		KeyPressEventArgs tmp;
		tmp.event_id = INPUT_EVENT_KEYUP;
		tmp.vkey = uint8_t(args->VirtualKey);
		m_devicelist.for_each_device([&tmp](device_info *device)
		{
			static_cast<uwp_keyboard_device*>(device)->queue_events(&tmp, 1);
		});
	}

	void OnCharacterReceived(CoreWindow ^sender, CharacterReceivedEventArgs ^args)
	{
		m_machine.ui_input().push_char_event(osd_common_t::s_window_list.front()->target(), args->KeyCode);
	}
};

//============================================================
//  uwp_keyboard_module
//============================================================

class uwp_keyboard_module : public wininput_module
{
private:
	key_processor^ m_key_processor;

public:
	uwp_keyboard_module()
		: wininput_module(OSD_KEYBOARDINPUT_PROVIDER, "uwp"),
		m_key_processor(nullptr)
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		auto first_window = std::static_pointer_cast<uwp_window_info>(osd_common_t::s_window_list.front());
		m_key_processor = ref new key_processor(first_window->uwp_window(), *devicelist(), machine);

		// Add a single UWP keyboard device
		uwp_keyboard_device *devinfo = devicelist()->create_device<uwp_keyboard_device>(machine, "UWP Keyboard 1", "UWP Keyboard 1", *this);

		keyboard_trans_table &table = keyboard_trans_table::instance();

		// populate it
		for (int keynum = 0; keynum < MAX_KEYS; keynum++)
		{
			input_item_id itemid = table.map_vkey_to_itemid(Windows::System::VirtualKey(keynum));
			char name[20];

			// generate/fetch the name
			_snprintf(name, ARRAY_LENGTH(name), "Scan%03d", keynum);

			// add the item to the device
			devinfo->device()->add_item(name, itemid, generic_button_get_state<std::uint8_t>, &devinfo->keyboard.state[keynum]);
		}
	}

	void exit() override
	{
		m_key_processor = nullptr;
	}
};

#else
MODULE_NOT_SUPPORTED(uwp_keyboard_module, OSD_KEYBOARDINPUT_PROVIDER, "uwp")
#endif

MODULE_DEFINITION(KEYBOARDINPUT_UWP, uwp_keyboard_module)

