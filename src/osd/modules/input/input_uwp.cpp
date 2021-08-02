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
#undef interface

// MAME headers
#include "emu.h"
#include "uiinput.h"
#include "strconv.h"

// MAMEOS headers
#include "winmain.h"
#include "input_common.h"
#include "input_windows.h"


namespace {

#define UWP_BUTTON_COUNT 32

using namespace concurrency;
using namespace Windows::UI::Core;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Gaming::Input;

//============================================================
//  UWP Base device/module implementation
//============================================================

//============================================================
//  UwpInputDevice - base class for implementing an input
//    device in C++/CX. To be used with uwp_input_device
//============================================================

private ref class UwpInputDevice
{
private:
	running_machine & m_machine;
	std::string m_name;
	std::string m_id;
	input_device_class m_devclass;
	input_module & m_module;
	input_device *m_inputdevice;

internal:
	UwpInputDevice(running_machine &machine, std::string &&name, std::string &&id, input_device_class deviceclass, input_module &module) :
		m_machine(machine),
		m_name(std::move(name)),
		m_id(std::move(id)),
		m_devclass(deviceclass),
		m_module(module),
		m_inputdevice(nullptr)
	{
	}

	property running_machine & Machine
	{
		running_machine & get() { return m_machine; }
	}

	property const std::string & Name
	{
		const std::string & get() { return m_name; }
	}

	property const std::string & Id
	{
		const std::string & get() { return m_id; }
	}

	property input_device_class DeviceClass
	{
		input_device_class get() { return m_devclass; }
	}

	property input_module & Module
	{
		input_module & get() { return m_module; }
	}

	property input_device* InputDevice
	{
		input_device* get() { return m_inputdevice; }
		void set(input_device* value) { m_inputdevice = value; }
	}

	virtual void Poll()
	{
	}

	virtual void Reset()
	{
	}
};

//============================================================
//  uwp_input_device - a device that can be used to wrap a
//    C++/CX ref class for an input device implementation
//============================================================

class uwp_input_device : public device_info
{
private:
	UwpInputDevice ^m_wrapped_device;

public:
	uwp_input_device(UwpInputDevice ^device) :
		device_info(device->Machine, std::string(device->Name), std::string(device->Id), device->DeviceClass, device->Module),
		m_wrapped_device(device)
	{
	}

	void poll() override
	{
		m_wrapped_device->Poll();
	}

	void reset() override
	{
		m_wrapped_device->Reset();
	}
};

//============================================================
//  UwpInputModule - a base class that can be used to
//    implement an input module with a C++/CX class.
//    normally used with uwp_wininput_module
//============================================================

class uwp_input_module;

private ref class UwpInputModule
{
private:
	const std::string m_type;
	const std::string m_name;
	uwp_input_module *m_module;

internal:
	UwpInputModule(std::string &&type, std::string &&name) :
		m_type(std::move(type)),
		m_name(std::move(name)),
		m_module(nullptr)
	{
	}

	property const std::string & Type
	{
		const std::string & get() { return m_type; }
	}

	property const std::string & Name
	{
		const std::string & get() { return m_name; }
	}

	property uwp_input_module * NativeModule
	{
		uwp_input_module * get() { return m_module; }
		void set(uwp_input_module * value) { m_module = value; }
	}

	virtual void input_init(running_machine &machine)
	{
	}
};

//============================================================
//  uwp_input_module - an input module that can be
//    used to create an input module with a C++/CX ref class
//============================================================

class uwp_input_module : public wininput_module
{
private:
	UwpInputModule^ m_refmodule;

public:
	uwp_input_module(UwpInputModule^ refmodule) :
		wininput_module(refmodule->Type.c_str(), refmodule->Name.c_str()),
		m_refmodule(refmodule)
	{
		refmodule->NativeModule = this;
	}

	void input_init(running_machine &machine) override
	{
		m_refmodule->input_init(machine);
	}
};

//============================================================
//  UWP Keyboard Implementation
//============================================================

//============================================================
//  UwpKeyboardDevice
//============================================================

private ref class UwpKeyboardDevice : public UwpInputDevice
{
private:
	keyboard_state keyboard;
	Platform::Agile<CoreWindow> m_coreWindow;
	std::mutex m_state_lock;

internal:
	UwpKeyboardDevice(Platform::Agile<CoreWindow> coreWindow, running_machine& machine, std::string &&name, std::string &&id, input_module &module) :
		UwpInputDevice(machine, std::move(name), std::move(id), DEVICE_CLASS_KEYBOARD, module),
		keyboard({{0}}),
		m_coreWindow(coreWindow)
	{
		coreWindow->Dispatcher->AcceleratorKeyActivated += ref new TypedEventHandler<CoreDispatcher^, AcceleratorKeyEventArgs^>(this, &UwpKeyboardDevice::OnAcceleratorKeyActivated);
		coreWindow->KeyDown += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &UwpKeyboardDevice::OnKeyDown);
		coreWindow->KeyUp += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &UwpKeyboardDevice::OnKeyUp);
		coreWindow->CharacterReceived += ref new TypedEventHandler<CoreWindow^, CharacterReceivedEventArgs^>(this, &UwpKeyboardDevice::OnCharacterReceived);
	}

	void Reset() override
	{
		std::lock_guard<std::mutex> scope_lock(m_state_lock);
		memset(&keyboard, 0, sizeof(keyboard));
	}

	void Configure()
	{
		keyboard_trans_table &table = keyboard_trans_table::instance();

		// populate it indexed by the scancode
		for (int keynum = KEY_UNKNOWN + 1; keynum < MAX_KEYS; keynum++)
		{
			input_item_id itemid = table.map_di_scancode_to_itemid(keynum);
			const char *keyname = table.ui_label_for_mame_key(itemid);

			char temp[256];
			if (!keyname)
			{
				snprintf(temp, std::size(temp), "Scan%03d", keynum);
				keyname = temp;
			}

			// add the item to the device
			this->InputDevice->add_item(keyname, itemid, generic_button_get_state<std::uint8_t>, &keyboard.state[keynum]);
		}
	}

	void OnKeyDown(CoreWindow^ win, KeyEventArgs^ args)
	{
		std::lock_guard<std::mutex> scope_lock(m_state_lock);
		CorePhysicalKeyStatus status = args->KeyStatus;
		int discancode = (status.ScanCode & 0x7f) | (status.IsExtendedKey ? 0x80 : 0x00);
		keyboard.state[discancode] = 0x80;
	}

	void OnKeyUp(CoreWindow^ win, KeyEventArgs^ args)
	{
		std::lock_guard<std::mutex> scope_lock(m_state_lock);
		CorePhysicalKeyStatus status = args->KeyStatus;
		int discancode = (status.ScanCode & 0x7f) | (status.IsExtendedKey ? 0x80 : 0x00);
		keyboard.state[discancode] = 0;
	}

	void OnCharacterReceived(CoreWindow ^sender, CharacterReceivedEventArgs ^args)
	{
		this->Machine.ui_input().push_char_event(osd_common_t::s_window_list.front()->target(), args->KeyCode);
	}

	void OnAcceleratorKeyActivated(CoreDispatcher ^sender, AcceleratorKeyEventArgs ^args)
	{
		std::lock_guard<std::mutex> scope_lock(m_state_lock);
		auto eventType = args->EventType;
		if (eventType == CoreAcceleratorKeyEventType::SystemKeyDown ||
			eventType == CoreAcceleratorKeyEventType::SystemKeyUp)
		{
			CorePhysicalKeyStatus status = args->KeyStatus;
			int discancode = (status.ScanCode & 0x7f) | (status.IsExtendedKey ? 0x80 : 0x00);
			keyboard.state[discancode] =
				eventType == CoreAcceleratorKeyEventType::SystemKeyDown ? 0x80 : 0;
		}
	}
};

//============================================================
//  UwpKeyboardModule
//============================================================

private ref class UwpKeyboardModule : public UwpInputModule
{
private:
	running_machine *m_machine;

internal:
	UwpKeyboardModule() : UwpInputModule(OSD_KEYBOARDINPUT_PROVIDER, "uwp")
	{
	}

	void input_init(running_machine &machine) override
	{
		auto first_window = std::static_pointer_cast<uwp_window_info>(osd_common_t::s_window_list.front());
		auto coreWindow = first_window->platform_window();

		// allocate the UWP implementation of the device object
		UwpKeyboardDevice ^refdevice = ref new UwpKeyboardDevice(coreWindow, machine, "UWP Keyboard 1", "UWP Keyboard 1", *this->NativeModule);

		// Allocate the wrapper and add it to the list
		auto created_devinfo = std::make_unique<uwp_input_device>(refdevice);
		uwp_input_device &devinfo = NativeModule->devicelist()->add_device<uwp_input_device>(machine, std::move(created_devinfo));

		// Give the UWP implementation a handle to the input_device
		refdevice->InputDevice = devinfo.device();

		// Configure the device
		refdevice->Configure();
	}
};

//============================================================
//  uwp_keyboard_module
//============================================================

class uwp_keyboard_module : public uwp_input_module
{
public:
	uwp_keyboard_module()
		: uwp_input_module(ref new UwpKeyboardModule())
	{
	}
};

// default axis names
static const char *const uwp_axis_name[] =
{
	"LSX",
	"LSY",
	"RSX",
	"RSY"
};

static const input_item_id uwp_axis_ids[] =
{
	ITEM_ID_XAXIS,
	ITEM_ID_YAXIS,
	ITEM_ID_RXAXIS,
	ITEM_ID_RYAXIS
};

struct gamepad_state
{
	BYTE    buttons[UWP_BUTTON_COUNT];
	LONG    left_trigger;
	LONG    right_trigger;
	LONG    left_thumb_x;
	LONG    left_thumb_y;
	LONG    right_thumb_x;
	LONG    right_thumb_y;
};

// Maps different UWP GameControllerButtonLabels to a halfway-sane input_item_id in many cases
static input_item_id buttonlabel_to_itemid[] =
{
	input_item_id::ITEM_ID_INVALID,       // GameControllerButtonLabel::None
	input_item_id::ITEM_ID_SELECT,        // GameControllerButtonLabel::XboxBack
	input_item_id::ITEM_ID_START,         // GameControllerButtonLabel::XboxStart
	input_item_id::ITEM_ID_START,         // GameControllerButtonLabel::XboxMenu
	input_item_id::ITEM_ID_SELECT,        // GameControllerButtonLabel::XboxView
	input_item_id::ITEM_ID_HAT1UP,        // GameControllerButtonLabel::XboxUp
	input_item_id::ITEM_ID_HAT1DOWN,      // GameControllerButtonLabel::XboxDown
	input_item_id::ITEM_ID_HAT1LEFT,      // GameControllerButtonLabel::XboxLeft
	input_item_id::ITEM_ID_HAT1RIGHT,     // GameControllerButtonLabel::XboxRight
	input_item_id::ITEM_ID_BUTTON1,       // GameControllerButtonLabel::XboxA
	input_item_id::ITEM_ID_BUTTON2,       // GameControllerButtonLabel::XboxB
	input_item_id::ITEM_ID_BUTTON3,       // GameControllerButtonLabel::XboxX
	input_item_id::ITEM_ID_BUTTON4,       // GameControllerButtonLabel::XboxY
	input_item_id::ITEM_ID_BUTTON5,       // GameControllerButtonLabel::XboxLeftBumper
	input_item_id::ITEM_ID_ZAXIS,         // GameControllerButtonLabel::XboxLeftTrigger
	input_item_id::ITEM_ID_BUTTON7,       // GameControllerButtonLabel::XboxLeftStickButton
	input_item_id::ITEM_ID_BUTTON6,       // GameControllerButtonLabel::XboxRightBumper
	input_item_id::ITEM_ID_RZAXIS,        // GameControllerButtonLabel::XboxRightTrigger
	input_item_id::ITEM_ID_BUTTON8,       // GameControllerButtonLabel::XboxRightStickButton
	input_item_id::ITEM_ID_BUTTON9,       // GameControllerButtonLabel::XboxPaddle1
	input_item_id::ITEM_ID_BUTTON10,      // GameControllerButtonLabel::XboxPaddle2
	input_item_id::ITEM_ID_BUTTON11,      // GameControllerButtonLabel::XboxPaddle3
	input_item_id::ITEM_ID_BUTTON12,      // GameControllerButtonLabel::XboxPaddle4
	input_item_id::ITEM_ID_INVALID,       // GameControllerButtonLabel_Mode
	input_item_id::ITEM_ID_SELECT,        // GameControllerButtonLabel_Select
	input_item_id::ITEM_ID_START,         // GameControllerButtonLabel_Menu
	input_item_id::ITEM_ID_SELECT,        // GameControllerButtonLabel_View
	input_item_id::ITEM_ID_SELECT,        // GameControllerButtonLabel_Back
	input_item_id::ITEM_ID_START,         // GameControllerButtonLabel_Start
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_Options
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_Share
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_Up
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_Down
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_Left
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_Right
	input_item_id::ITEM_ID_BUTTON1,       // GameControllerButtonLabel_LetterA
	input_item_id::ITEM_ID_BUTTON2,       // GameControllerButtonLabel_LetterB
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_LetterC
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_LetterL
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_LetterR
	input_item_id::ITEM_ID_BUTTON3,       // GameControllerButtonLabel_LetterX
	input_item_id::ITEM_ID_BUTTON4,       // GameControllerButtonLabel_LetterY
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_LetterZ
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_Cross
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_Circle
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_Square
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_Triangle
	input_item_id::ITEM_ID_BUTTON5,       // GameControllerButtonLabel_LeftBumper
	input_item_id::ITEM_ID_ZAXIS,         // GameControllerButtonLabel_LeftTrigger
	input_item_id::ITEM_ID_BUTTON7,       // GameControllerButtonLabel_LeftStickButton
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_Left1
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_Left2
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_Left3
	input_item_id::ITEM_ID_BUTTON6,       // GameControllerButtonLabel_RightBumper
	input_item_id::ITEM_ID_RZAXIS,        // GameControllerButtonLabel_RightTrigger
	input_item_id::ITEM_ID_BUTTON8,       // GameControllerButtonLabel_RightStickButton
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_Right1
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_Right2
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_Right3
	input_item_id::ITEM_ID_BUTTON9,       // GameControllerButtonLabel_Paddle1
	input_item_id::ITEM_ID_BUTTON10,      // GameControllerButtonLabel_Paddle2
	input_item_id::ITEM_ID_BUTTON11,      // GameControllerButtonLabel_Paddle3
	input_item_id::ITEM_ID_BUTTON12,      // GameControllerButtonLabel_Paddle4
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_Plus
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_Minus
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_DownLeftArrow
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_DialLeft
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_DialRight
	input_item_id::ITEM_ID_OTHER_SWITCH,  // GameControllerButtonLabel_Suspension
};

//============================================================
//  UwpJoystickDevice
//============================================================

private ref class UwpJoystickDevice : public UwpInputDevice
{
private:
	Gamepad ^m_pad;
	bool m_configured;
	gamepad_state state;

internal:
	UwpJoystickDevice(Gamepad^ pad, running_machine &machine, const char *name, const char *id, input_module &module)
		: UwpInputDevice(machine, name, id, DEVICE_CLASS_JOYSTICK, module),
		m_pad(pad),
		m_configured(false),
		state({0})
	{}

	void Poll() override
	{
		// If the device hasn't been configured, don't poll
		if (!m_configured)
			return;

		GamepadReading reading = m_pad->GetCurrentReading();

		for (int butnum = 0; butnum < UWP_BUTTON_COUNT; butnum++)
		{
			GamepadButtons currentButton = GamepadButtons(1 << butnum);
			state.buttons[butnum] = (reading.Buttons & currentButton) != GamepadButtons::None ? 0xFF : 0;
		}

		// Now grab the axis values
		// Each of the thumbstick axis members is a signed value between -32768 and 32767 describing the position of the thumbstick
		// However, the Y axis values are inverted from what MAME expects, so negate the value
		state.left_thumb_x = normalize_absolute_axis(reading.LeftThumbstickX, -1, 1);
		state.left_thumb_y = -normalize_absolute_axis(reading.LeftThumbstickY, -1, 1);
		state.right_thumb_x = normalize_absolute_axis(reading.RightThumbstickX, -1, 1);
		state.right_thumb_y = -normalize_absolute_axis(reading.RightThumbstickY, -1, 1);

		// Get the trigger values
		state.left_trigger = normalize_absolute_axis(reading.LeftTrigger, 0.0, 1.0);
		state.right_trigger = normalize_absolute_axis(reading.RightTrigger, 0.0, 1.0);

		// For the UI, triggering UI_CONFIGURE is odd. It requires a EVENT_CHAR first
		static constexpr int menuhotkey = (int)(GamepadButtons::View | GamepadButtons::X);
		if (((int)reading.Buttons & menuhotkey) == menuhotkey)
		{
			ui_event uiev;
			memset(&uiev, 0, sizeof(uiev));
			uiev.event_type = ui_event::IME_CHAR;
			this->Machine.ui_input().push_event(uiev);
		}
	}

	void Reset() override
	{
		memset(&state, 0, sizeof(state));
	}

	void Configure()
	{
		// If the device has already been configured, don't do it again
		if (m_configured)
			return;

		GamepadReading r = m_pad->GetCurrentReading();

		// Add the axes
		for (int axisnum = 0; axisnum < 4; axisnum++)
		{
			this->InputDevice->add_item(
				uwp_axis_name[axisnum],
				uwp_axis_ids[axisnum],
				generic_axis_get_state<LONG>,
				&state.left_thumb_x + axisnum);
		}

		// populate the buttons
		for (int butnum = 0; butnum < UWP_BUTTON_COUNT; butnum++)
		{
			GamepadButtons button = GamepadButtons(1 << butnum);
			auto label = m_pad->GetButtonLabel(button);
			if (label != GameControllerButtonLabel::None)
			{
				std::string desc = osd::text::from_wstring(label.ToString()->Data());
				this->InputDevice->add_item(
					desc.c_str(),
					buttonlabel_to_itemid[static_cast<int>(label)],
					generic_button_get_state<BYTE>,
					&state.buttons[butnum]);
			}
		}

		this->InputDevice->add_item(
			"Left Trigger",
			ITEM_ID_ZAXIS,
			generic_axis_get_state<LONG>,
			&state.left_trigger);

		this->InputDevice->add_item(
			"Right Trigger",
			ITEM_ID_RZAXIS,
			generic_axis_get_state<LONG>,
			&state.right_trigger);

		m_configured = true;
	}
};

//============================================================
//  UwpJoystickModule
//============================================================

private ref class UwpJoystickModule : public UwpInputModule
{
private:
	boolean m_joysticks_discovered;

internal:
	UwpJoystickModule()
		: UwpInputModule(OSD_JOYSTICKINPUT_PROVIDER, "uwp"),
		m_joysticks_discovered(false)
	{
	}

	void input_init(running_machine &machine) override
	{
		PerformGamepadDiscovery();

		auto pads = Gamepad::Gamepads;

		int padindex = 0;
		std::for_each(begin(pads), end(pads), [&](Gamepad^ pad)
		{
			std::ostringstream namestream;
			namestream << "UWP Gamepad " << (padindex + 1);

			auto name = namestream.str();

			// allocate the UWP implementation of the device object
			UwpJoystickDevice ^refdevice = ref new UwpJoystickDevice(pad, machine, name.c_str(), name.c_str(), *this->NativeModule);

			// Allocate the wrapper and add it to the list
			auto created_devinfo = std::make_unique<uwp_input_device>(refdevice);
			auto &devinfo = NativeModule->devicelist()->add_device<uwp_input_device>(machine, std::move(created_devinfo));

			// Give the UWP implementation a handle to the input_device
			refdevice->InputDevice = devinfo->device();

			// Configure the device
			refdevice->Configure();

			padindex++;
		});
	}

private:
	void PerformGamepadDiscovery()
	{
		Gamepad::GamepadAdded += ref new EventHandler<Gamepad ^>(this, &UwpJoystickModule::OnGamepadAdded);
		auto start = std::chrono::system_clock::now();

		// We need to pause a bit and pump events so gamepads get discovered
		while (std::chrono::system_clock::now() - start < std::chrono::milliseconds(1000))
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

		m_joysticks_discovered = true;
	}

	void OnGamepadAdded(Platform::Object ^sender, Gamepad ^pad)
	{
		if (m_joysticks_discovered)
		{
			osd_printf_error("Input: UWP Compatible %s gamepad plugged in AFTER discovery complete!\n", pad->IsWireless ? "Wireless" : "Wired");
		}
		else
		{
			osd_printf_verbose("Input: UWP Compatible %s gamepad discovered.\n", pad->IsWireless ? "Wireless" : "Wired");
		}
	}
};

//============================================================
//  uwp_joystick_module
//============================================================

class uwp_joystick_module : public uwp_input_module
{
public:
	uwp_joystick_module() : uwp_input_module(ref new UwpJoystickModule())
	{
	}
};

} // anonymous namespace

#else // defined(OSD_UWP)

MODULE_NOT_SUPPORTED(uwp_keyboard_module, OSD_KEYBOARDINPUT_PROVIDER, "uwp")
MODULE_NOT_SUPPORTED(uwp_joystick_module, OSD_JOYSTICKINPUT_PROVIDER, "uwp")

#endif // defined(OSD_UWP)

MODULE_DEFINITION(KEYBOARDINPUT_UWP, uwp_keyboard_module)
MODULE_DEFINITION(JOYSTICKINPUT_UWP, uwp_joystick_module)
