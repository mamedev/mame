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
{
	memset(port_tag, 0, sizeof(port_tag));
}

void dc_common_device::device_start()
{
	maple_device::device_start();

	for (int i = 0; i < 8; i++)
	{
		port[i] = ioport(port_tag[i]);
	}
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
	id = "Dreamcast Controller";
	license = "Produced By or Under License From SEGA ENTERPRISES,LTD.";
	versions = "Version 1.010,1998/09/28,315-6211-AB   ,Analog Module : The 4th Edition.5/8  +DF";
}

void dc_controller_device::fixed_status(uint32_t *dest)
{
	// TODO: is this right? should be 0x01000000
	dest[0] = 0x20000000; // Controller
	dest[1] =
		((port[2] != nullptr) ? 0x010000 : 0) |
		((port[3] != nullptr) ? 0x020000 : 0) |
		((port[4] != nullptr) ? 0x040000 : 0) |
		((port[5] != nullptr) ? 0x080000 : 0) |
		((port[6] != nullptr) ? 0x100000 : 0) |
		((port[7] != nullptr) ? 0x200000 : 0) |
		((port[0] ? port[0]->active() : 0) << 8) |
		(port[1] ? port[1]->active() : 0); // 1st function - controller
	dest[2] = 0; // No 2nd function
	dest[3] = 0; // No 3rd function
	dest[4] = 0x00ff; // Every region, no expansion
	copy_with_spaces(((uint8_t *)dest) + 18, id, 30);
	copy_with_spaces(((uint8_t *)dest) + 48, license, 60);
	dest[27] = 0x01f401ae; // standby 43mA, max 50mA
}

void dc_controller_device::free_status(uint32_t *dest)
{
	copy_with_spaces((uint8_t *)dest, versions, 80);
}

void dc_controller_device::read(uint32_t *dest)
{
	// TODO: is this right? should be 0x01000000
	dest[0] = 0x21000000; // Controller
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
	id = "92key Keyboard for JPN";
	license = "Produced By or Under License From SEGA ENTERPRISES,LTD.";
	versions = "Version 1.000,1998/06/12,315-6215-AD   ,Key Scan Module: The 1st Edition. 05/20";
}

void dc_keyboard_device::fixed_status(uint32_t *dest)
{
	dest[0] = 0x40000000; // Keyboard
	dest[1] = 0x00201000; // 1st function
	dest[2] = 0x00000008; // No 2nd function
	dest[3] = 0x00000000; // No 3rd function
	dest[4] = 0x00000002; // Japan region, no expansion
	copy_with_spaces(((uint8_t *)dest) + 18, id, 30);
	copy_with_spaces(((uint8_t *)dest) + 48, license, 60);
	dest[27] = 0x0190015e; // standby 35mA, max 40mA
}

void dc_keyboard_device::free_status(uint32_t *dest)
{
	copy_with_spaces((uint8_t *)dest, versions, 80);
}

void dc_keyboard_device::read(uint32_t *dest)
{
	dest[0] = 0x40000000; // Keyboard
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

