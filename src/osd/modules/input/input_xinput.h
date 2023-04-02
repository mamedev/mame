// license:BSD-3-Clause
// copyright-holders:Brad Hughes, Vas Crabb
#ifndef MAME_OSD_INPUT_INPUT_XINPUT_H
#define MAME_OSD_INPUT_INPUT_XINPUT_H

#pragma once

#include "input_common.h"

#include "modules/lib/osdlib.h"

#include <memory>

#include <windows.h>
#include <xinput.h>


namespace osd {

class xinput_api_helper
{
public:
	xinput_api_helper() { }

	int initialize();

	std::unique_ptr<device_info> create_xinput_device(UINT index, input_module_base &module);

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

	dynamic_module::ptr m_xinput_dll = nullptr;
	xinput_get_state_fn XInputGetState = nullptr;
	xinput_get_caps_fn  XInputGetCapabilities = nullptr;
};

} // namespace osd

#endif // MAME_OSD_INPUT_INPUT_XINPUT_H
