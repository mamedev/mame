// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Taxan KY-80

    This 100-pin QFP device seems to be a custom derivative of the
    Kawasaki KC82 CPU core (fast Z80-like with built-in MMU). It seems
    possible that it was built entirely out of Kawasaki LSI macrocells,
    but the on-chip peripherals are at least mapped differently than in
    either KL5C80A12 or KL5C80A16 (in the top 3/8ths of the I/O space
    rather the bottom quarter).

    On-chip features appear to include 1024 bytes of RAM, an interrupt
    controller (not quite the same as KP69), a KP63-like timer/counter
    block, 5 parallel ports and 2 8251-like serial ports.

***************************************************************************/

#include "emu.h"
#include "ky80.h"

// device type definition
DEFINE_DEVICE_TYPE(KY80, ky80_device, "ky80", "Taxan KY-80")


//-------------------------------------------------
//  ky80_device - constructor
//-------------------------------------------------

ky80_device::ky80_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: kc82_device(mconfig, KY80, tag, owner, clock,
					address_map_constructor(FUNC(ky80_device::internal_ram), this),
					address_map_constructor(FUNC(ky80_device::internal_io), this))
{
}


//-------------------------------------------------
//  internal_ram - map for high-speed internal RAM
//-------------------------------------------------

void ky80_device::internal_ram(address_map &map)
{
	map(0xffc00, 0xfffff).ram().share("ram");
}


//-------------------------------------------------
//  internal_io - map for internal I/O registers
//-------------------------------------------------

void ky80_device::internal_io(address_map &map)
{
	map(0xa0, 0xa7).mirror(0xff00).rw(FUNC(ky80_device::mmu_r), FUNC(ky80_device::mmu_w));
}
