// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

  Northbridge implementation

***************************************************************************/

#include "emu.h"
#include "northbridge.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

northbridge_device::northbridge_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	m_maincpu(*this, ":maincpu"),
	m_ram(*this, ":" RAM_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void northbridge_device::device_start()
{
	address_space& space = machine().device(":maincpu")->memory().space(AS_PROGRAM);

	machine().root_device().membank("bank10")->set_base(m_ram->pointer());

	if (m_ram->size() > 0x0a0000)
	{
		offs_t ram_limit = 0x100000 + m_ram->size() - 0x0a0000;
		space.install_read_bank(0x100000,  ram_limit - 1, "bank1");
		space.install_write_bank(0x100000,  ram_limit - 1, "bank1");
		machine().root_device().membank("bank1")->set_base(m_ram->pointer() + 0xa0000);
	}
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void northbridge_device::device_reset()
{
}
