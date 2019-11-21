// license:BSD-3-Clause
// copyright-holders:R. Belmont
//============================================================
//
//  input_mac.cpp - Mac input
//
//  Mac OSD by R. Belmont
//
//============================================================

#include "input_module.h"
#include "modules/osdmodule.h"

#if defined(OSD_MAC)

// System headers
#include <ctype.h>
#include <stddef.h>
#include <mutex>
#include <memory>
#include <algorithm>

// MAME headers
#include "emu.h"
#include "osdepend.h"
#include "ui/uimain.h"
#include "uiinput.h"
#include "window.h"
#include "strconv.h"

#include "../../mac/osdmac.h"
#include "input_common.h"

extern void MacPollInputs();

void mac_osd_interface::customize_input_type_list(std::vector<input_type_entry> &typelist)
{
}

void mac_osd_interface::poll_inputs(running_machine &machine)
{
	MacPollInputs();
}

void mac_osd_interface::release_keys()
{
}

bool mac_osd_interface::should_hide_mouse()
{
	return false;
}

void mac_osd_interface::process_events_buf()
{
}

#endif
