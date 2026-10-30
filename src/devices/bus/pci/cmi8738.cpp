// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

TODO:
- stub
- Incredibly common on motherboards (The Retro Web counts 451 MBs!)

**************************************************************************************************/

#include "emu.h"
#include "cmi8738.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"


DEFINE_DEVICE_TYPE(CMI8738, cmi8738_device,   "cmi8738",   "C-Media CMI8738/C3DX sound card")


cmi8738_device::cmi8738_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
{
}

cmi8738_device::cmi8738_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cmi8738_device(mconfig, CMI8738, tag, owner, clock)
{
	// https://admin.pci-ids.ucw.cz/read/PC/13f6/0111
	set_ids(0x13f60111, 0x10, 0x040100, 0x13f60111);
}

void cmi8738_device::device_add_mconfig(machine_config &config)
{

}

void cmi8738_device::device_start()
{
	pci_card_device::device_start();

	// TODO: verify length
	add_map( 256, M_IO, FUNC(cmi8738_device::map));

	// INTA#
	intr_pin = 1;
	intr_line = 5;

	minimum_grant = 0x02;
	maximum_latency = 0x18;
}

void cmi8738_device::device_reset()
{
	pci_card_device::device_reset();

	command = 0x0000;
	command_mask = 0x105;
	// Fast Back-to-Back, medium DEVSEL#, has Capability List
	status = 0x0290;

	remap_cb();
}

u8 cmi8738_device::latency_timer_r()
{
	return 0x20;
}

// NOTE: documentation claims "0x0c" which is clearly a mistake
u8 cmi8738_device::capptr_r()
{
	return 0x40;
}

void cmi8738_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
	// TODO: values not provided by 2.2 documentation
	// 1.2 doc really claims DDMA being there (???)
	map(0x40, 0x43).lr32(NAME([] () { return 0x0601'0001; }));
}

void cmi8738_device::map(address_map &map)
{
}
