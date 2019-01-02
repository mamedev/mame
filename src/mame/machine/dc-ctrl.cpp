// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "dc-ctrl.h"

/*******************************
 *
 * Common abstract class
 *
 ******************************/

dc_common_device::dc_common_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	maple_device(mconfig, type, tag, owner, clock)
	, port(*this, {finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG})
{
}

void dc_common_device::device_start()
{
	maple_device::device_start();
}

void dc_common_device::maple_w(const uint32_t *data, uint32_t in_size)
{
	switch(data[0] & 0xff) {
	case 0x01: // Device request
		reply_start(5, 0x20, 29);
		fixed_status(reply_buffer+1);
		reply_ready_with_delay();
		break;

	case 0x02: // All status request
		reply_start(6, 0x20, 49);
		fixed_status(reply_buffer+1);
		free_status(reply_buffer+29);
		reply_ready_with_delay();
		break;

	case 0x03: // reset - we're stateless where it matters
		reply_start(7, 0x20, 0);
		reply_ready_with_delay();
		break;

	case 0x09: // get condition
		if(1 || (in_size >= 2 && data[1] == 0x01000000)) {
			reply_start(8, 0x20, 4);
			read(reply_buffer+1);
			reply_ready_with_delay();
		}
		break;
	}
}

/*******************************
 *
 * Dreamcast Controller
 *
 ******************************/

DEFINE_DEVICE_TYPE(DC_CONTROLLER, dc_controller_device, "dcctrl", "Dreamcast Controller")

dc_controller_device::dc_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dc_common_device(mconfig, DC_CONTROLLER, tag, owner, clock)
{
	model = "Dreamcast Controller";
	license = "Produced By or Under License From SEGA ENTERPRISES,LTD.";
	versions = "Version 1.000,1998/05/11,315-6215-AB   ,Analog Module: The 4th Edition. 05/08";
	id = 0x01000000; // Controller
	electric_current = 0x01f401ae; // max 50mA, standby 43mA
	region = 0x00ff;
}

void dc_controller_device::fixed_status(uint32_t *dest)
{
	dest[0] = id;
	dest[1] =
		((port[2] != nullptr) ? 0x0100 : 0) |
		((port[3] != nullptr) ? 0x0200 : 0) |
		((port[4] != nullptr) ? 0x0400 : 0) |
		((port[5] != nullptr) ? 0x0800 : 0) |
		((port[6] != nullptr) ? 0x1000 : 0) |
		((port[7] != nullptr) ? 0x2000 : 0) |
		((port[0] ? port[0]->active() : 0) << 24) |
		((port[1] ? port[1]->active() : 0) << 16); // 1st function - controller
	dest[2] = 0; // No 2nd function
	dest[3] = 0; // No 3rd function
	dest[4] = region; // Every region, no expansion
	copy_with_spaces(((uint8_t *)dest) + 18, model, 30);
	copy_with_spaces(((uint8_t *)dest) + 48, license, 60);
	dest[27] = electric_current;
}

void dc_controller_device::free_status(uint32_t *dest)
{
	copy_with_spaces((uint8_t *)dest, versions, 80);
}

void dc_controller_device::read(uint32_t *dest)
{
	dest[0] = id; // Controller
	dest[1] =
		(port[0] ? port[0]->read() : 0xff) |
		((port[1] ? port[1]->read() : 0xff) << 8) |
		((port[2] ? port[2]->read() : 0) << 16) |
		((port[3] ? port[3]->read() : 0) << 24);
	dest[2] =
		(port[4] ? port[4]->read() : 0x80) |
		((port[5] ? port[5]->read() : 0x80) << 8) |
		((port[6] ? port[6]->read() : 0x80) << 16) |
		((port[7] ? port[7]->read() : 0x80) << 24);
}

/*******************************
 *
 * Dreamcast Keyboard
 *
 ******************************/

DEFINE_DEVICE_TYPE(DC_KEYBOARD, dc_keyboard_device, "dckb", "Dreamcast Keyboard")

dc_keyboard_device::dc_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dc_common_device(mconfig, DC_KEYBOARD, tag, owner, clock)
{
	model = "92key Keyboard for JPN";
	license = "Produced By or Under License From SEGA ENTERPRISES,LTD.";
	versions = "Version 1.000,1998/06/12,315-6215-AD   ,Key Scan Module: The 1st Edition. 05/20";
	id = 0x40000000; // Keyboard
	electric_current = 0x0190015e; // max 40mA, standby 35mA
	region = 0x0002; // Japan region, no expansion
}

void dc_keyboard_device::fixed_status(uint32_t *dest)
{
	dest[0] = id; // Keyboard
	dest[1] = 0x00201000; // 1st function
	dest[2] = 0x00000008; // No 2nd function (doc returns 8 here tho?)
	dest[3] = 0x00000000; // No 3rd function
	dest[4] = region;
	copy_with_spaces(((uint8_t *)dest) + 18, model, 30);
	copy_with_spaces(((uint8_t *)dest) + 48, license, 60);
	dest[27] = electric_current;
}

void dc_keyboard_device::free_status(uint32_t *dest)
{
	copy_with_spaces((uint8_t *)dest, versions, 80);
}

void dc_keyboard_device::read(uint32_t *dest)
{
	dest[0] = id;
	// key code
	dest[1] =
		(port[0] ? port[0]->read() : 0) |
		((port[1] ? port[1]->read() : 0) << 8) |
		((port[2] ? port[2]->read() : 0) << 16) |
		((port[3] ? port[3]->read() : 0) << 24);
	dest[2] =
		(port[4] ? port[4]->read() : 0) |
		((port[5] ? port[5]->read() : 0) << 8) |
		((port[6] ? port[6]->read() : 0) << 16) |
		((port[7] ? port[7]->read() : 0) << 24);
}

