// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Melco/Buffalo LGY-98 NIC (NE2000 based)

TODO:
- stub, enough for base LGYSETUP.EXE to pass identification and nothing else;
- Has no dipswitches on board, the port and INT lines are PnP configured thru LGYSETUP + either
  EEPROM data or something else (perhaps the ROM initialized that?)

**************************************************************************************************/

#include "emu.h"
#include "lgy98.h"

DEFINE_DEVICE_TYPE(LGY98, lgy98_device, "lgy98", "Melco LGY-98 NIC interface")

lgy98_device::lgy98_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, LGY98, tag, owner, clock)
	, device_pc98_cbus_slot_interface(mconfig, *this)
{
}

//void lgy98_device::irq_out(int state)
//{
//  m_bus->int_w(0, state);
//}


void lgy98_device::device_add_mconfig(machine_config &config)
{
	// ...
}

void lgy98_device::device_start()
{
}

void lgy98_device::device_reset()
{
}

void lgy98_device::remap(int space_id, offs_t start, offs_t end)
{
	// TODO: seems to employ an (undumped) ROM socket too
	if (space_id == AS_IO)
	{
		m_bus->install_device(0x0000, 0xffff, *this, &lgy98_device::io_map);
	}
}

void lgy98_device::io_map(address_map &map)
{
	// 0*d* ~ 7*d*
	const u16 base_io = 0x10d0;

	// identification flags
	map(base_io + 0x30a, base_io + 0x30a).lr8(NAME([] () { return 0x00; }));
	map(base_io + 0x30b, base_io + 0x30b).lr8(NAME([] () { return 0x40; }));
	map(base_io + 0x30c, base_io + 0x30c).lr8(NAME([] () { return 0x26; }));
	// TODO: following really looks a 93C46 interface + something else
	// (bit 5 tested low during ID sequence)
	map(base_io + 0x30d, base_io + 0x30d).lr8(NAME([] () { return 0x0b; }));
}
