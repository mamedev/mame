#ifndef MAME_OSD_INPUT_INPUT_XINPUT_H
#define MAME_OSD_INPUT_INPUT_XINPUT_H

#pragma once

#include "input_windows.h"

#include "modules/lib/osdlib.h"

#include <xinput.h>

#include <memory>


class xinput_api_helper : public std::enable_shared_from_this<xinput_api_helper>
{
public:
	xinput_api_helper() { }

	int initialize();
	device_info *create_xinput_device(running_machine &machine, UINT index, wininput_module &module);

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

#endif // MAME_OSD_INPUT_INPUT_XINPUT_H
