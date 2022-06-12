#ifndef MAME_OSD_INPUT_INPUT_XINPUT_H
#define MAME_OSD_INPUT_INPUT_XINPUT_H

#pragma once

#include "input_windows.h"

#include "modules/lib/osdlib.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <string_view>

#include <xinput.h>


class xinput_joystick_device;


class xinput_api_helper : public std::enable_shared_from_this<xinput_api_helper>
{
public:
	xinput_api_helper() { }

	int initialize();
	xinput_joystick_device *create_xinput_device(running_machine &machine, UINT index, wininput_module &module);

	DWORD xinput_get_state(DWORD dwUserindex, XINPUT_STATE *pState) const
	{
		return (*XInputGetState)(dwUserindex, pState);
	}

	DWORD xinput_get_capabilities(DWORD dwUserindex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities) const
	{
		return (*XInputGetCapabilities)(dwUserindex, dwFlags, pCapabilities);
	}

private:
	// Typedefs for dynamically loaded functions
	typedef DWORD (WINAPI *xinput_get_state_fn)(DWORD, XINPUT_STATE *);
	typedef DWORD (WINAPI *xinput_get_caps_fn)(DWORD, DWORD, XINPUT_CAPABILITIES *);

	osd::dynamic_module::ptr m_xinput_dll = nullptr;
	xinput_get_state_fn      XInputGetState = nullptr;
	xinput_get_caps_fn       XInputGetCapabilities = nullptr;
};


class xinput_joystick_device : public device_info
{
public:
	static inline constexpr int XINPUT_MAX_POV = 4;
	static inline constexpr int XINPUT_MAX_BUTTONS = 10;
	static inline constexpr int XINPUT_MAX_AXIS = 4;

	struct gamepad_state
	{
		BYTE    buttons[XINPUT_MAX_BUTTONS];
		BYTE    povs[XINPUT_MAX_POV];
		LONG    left_trigger;
		LONG    right_trigger;
		LONG    left_thumb_x;
		LONG    left_thumb_y;
		LONG    right_thumb_x;
		LONG    right_thumb_y;
	};

	// state information for a gamepad; state must be first element
	struct xinput_api_state
	{
		uint32_t                player_index;
		XINPUT_STATE            xstate;
		XINPUT_CAPABILITIES     caps;
	};

	gamepad_state      gamepad;
	xinput_api_state   xinput_state;

private:
	std::shared_ptr<xinput_api_helper> m_xinput_helper;
	std::mutex                         m_device_lock;
	bool                               m_configured;

public:
	xinput_joystick_device(running_machine &machine, std::string &&name, std::string &&id, input_module &module, std::shared_ptr<xinput_api_helper> helper);

	void poll() override;
	void reset() override;
	void configure();
};

#endif // MAME_OSD_INPUT_INPUT_XINPUT_H
