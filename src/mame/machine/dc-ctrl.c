// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "dc-ctrl.h"

const device_type DC_CONTROLLER = &device_creator<dc_controller_device>;

void dc_controller_device::static_set_port_tag(device_t &device, int port, const char *tag)
{
	dc_controller_device &ctrl = downcast<dc_controller_device &>(device);
	ctrl.port_tag[port] = tag;
}

void dc_controller_device::static_set_id(device_t &device, const char *id)
{
	dc_controller_device &ctrl = downcast<dc_controller_device &>(device);
	ctrl.id = id;
}

void dc_controller_device::static_set_license(device_t &device, const char *license)
{
	dc_controller_device &ctrl = downcast<dc_controller_device &>(device);
	ctrl.license = license;
}

void dc_controller_device::static_set_versions(device_t &device, const char *versions)
{
	dc_controller_device &ctrl = downcast<dc_controller_device &>(device);
	ctrl.versions = versions;
}

dc_controller_device::dc_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	maple_device(mconfig, DC_CONTROLLER, "Dreamcast Controller", tag, owner, clock, "dcctrl", __FILE__)
{
	memset(port_tag, 0, sizeof(port_tag));

	id = "Dreamcast Controller";
	license = "Produced By or Under License From SEGA ENTERPRISES,LTD.";
	versions = "Version 1.010,1998/09/28,315-6211-AB   ,Analog Module : The 4th Edition.5/8  +DF";
}

void dc_controller_device::maple_w(const UINT32 *data, UINT32 in_size)
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

void dc_controller_device::fixed_status(UINT32 *dest)
{
	dest[0] = 0x20000000; // Controller
	dest[1] =
		((ioport(port_tag[2]) != NULL) ? 0x010000 : 0) |
		((ioport(port_tag[3]) != NULL) ? 0x020000 : 0) |
		((ioport(port_tag[4]) != NULL) ? 0x040000 : 0) |
		((ioport(port_tag[5]) != NULL) ? 0x080000 : 0) |
		((ioport(port_tag[6]) != NULL) ? 0x100000 : 0) |
		((ioport(port_tag[7]) != NULL) ? 0x200000 : 0) |
		(ioport(port_tag[0])->active_safe(0) << 8) |
		ioport(port_tag[1])->active_safe(0); // 1st function - controller
	dest[2] = 0; // No 2nd function
	dest[3] = 0; // No 3rd function
	dest[4] = 0x00ff; // Every region, no expansion
	copy_with_spaces(((UINT8 *)dest) + 18, id, 30);
	copy_with_spaces(((UINT8 *)dest) + 48, license, 60);
	dest[27] = 0x01f401ae; // standby 43mA, max 50mA
}

void dc_controller_device::free_status(UINT32 *dest)
{
	copy_with_spaces((UINT8 *)dest, versions, 80);
}

void dc_controller_device::read(UINT32 *dest)
{
	dest[0] = 0x21000000; // Controller
	dest[1] =
		ioport(port_tag[0])->read_safe(0xff) |
		(ioport(port_tag[1])->read_safe(0xff) << 8) |
		(ioport(port_tag[2])->read_safe(0x00) << 16) |
		(ioport(port_tag[3])->read_safe(0x00) << 24);
	dest[2] =
		ioport(port_tag[4])->read_safe(0x80) |
		(ioport(port_tag[5])->read_safe(0x80) << 8) |
		(ioport(port_tag[6])->read_safe(0x80) << 16) |
		(ioport(port_tag[7])->read_safe(0x80) << 24);
}
