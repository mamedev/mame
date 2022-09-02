// license:BSD-3-Clause
// copyright-holders:David Haywood
/**********************************************************************

    Datel Game Booster for Playstation 1

    Gameboy emulator with Gameboy cartridge slot

**********************************************************************/

#include "emu.h"
#include "gamebooster.h"

#include "bus/gameboy/carts.h"

#include "softlist_dev.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PSX_GAMEBOOSTER, psx_gamebooster_device, "psxgboost", "Datel Game Booster for Playstation")

//-------------------------------------------------
//  ROM( psxgboost )
//-------------------------------------------------

ROM_START( psxgboost )
	ROM_REGION(0x40000, "rom", 0)
	ROM_LOAD("game booster.rom", 0x0000, 0x40000, CRC(c8e459b8) SHA1(c20ab073f61242f37665f12199b95cfa3a83e9fc) )
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *psx_gamebooster_device::device_rom_region() const
{
	return ROM_NAME( psxgboost );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  psx_gamebooster_device - constructor
//-------------------------------------------------

psx_gamebooster_device::psx_gamebooster_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PSX_GAMEBOOSTER, tag, owner, clock)
	, psx_parallel_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_cartslot(*this, "gbslot")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void psx_gamebooster_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void psx_gamebooster_device::device_reset()
{
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint16_t psx_gamebooster_device::exp_r(offs_t offset, uint16_t mem_mask)
{
	if (offset < 0x20000)
	{
		return m_rom->base()[(offset * 2) & 0x3ffff] | (m_rom->base()[((offset * 2) + 1) & 0x3ffff] << 8);
	}
	else if (offset < 0x24000)
	{
		offset -= 0x20000;
		uint16_t retval = 0;

		if (mem_mask & 0xff00) retval |= (m_cartslot->read_rom((offset*2)+1))<<8;
		if (mem_mask & 0x00ff) retval |= m_cartslot->read_rom((offset*2)+0);

		return retval;
	}
	else
	{
		logerror("%s: psx_gamebooster_device::exp_r %04x\n", machine().describe_context(), offset*2);
	}

	return 0x0000;
}

void psx_gamebooster_device::exp_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{

	if (offset < 0x20000)
	{
		logerror("%s: psx_gamebooster_device::exp_w %04x %04x\n", machine().describe_context(), offset*2, data);
	}
	else if (offset < 0x24000)
	{
		offset -= 0x20000;
		logerror("%s: psx_gamebooster_device::exp_w %04x %04x\n", machine().describe_context(), offset*2, data);

		if (mem_mask & 0xff00) m_cartslot->write_bank((offset*2)+1, data>>8);
		if (mem_mask & 0x00ff) m_cartslot->write_bank((offset*2)+0, data); // send this 2nd or it erases the bank with the above

	}
	else
	{
		logerror("%s: psx_gamebooster_device::exp_w %04x %04x\n", machine().describe_context(), offset*2, data);
	}
}

void psx_gamebooster_device::device_add_mconfig(machine_config &config)
{
	// cartslot
	GB_CART_SLOT(config, m_cartslot, gameboy_cartridges, nullptr);

	SOFTWARE_LIST(config, "cart_list").set_original("gameboy");
	SOFTWARE_LIST(config, "gbc_list").set_compatible("gbcolor");
}
