// license:BSD-3-Clause
// copyright-holders:David Haywood
/**********************************************************************

    Datel Game Booster for Playstation 1

    Gameboy emulator with Gameboy cartridge slot

**********************************************************************/

#include "emu.h"
#include "gamebooster.h"

#include "bus/gameboy/carts.h"
#include "bus/gameboy/gbslot.h"

#include "softlist_dev.h"

#include <utility>

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PSX_GAMEBOOSTER, psx_gamebooster_device, "psxgboost", "Datel Game Booster for Playstation")

//-------------------------------------------------
//  ROM( psxgboost )
//-------------------------------------------------

ROM_START( psxgboost )
	ROM_REGION16_LE(0x40000, "rom", 0)
	ROM_LOAD("game booster.rom", 0x0000, 0x40000, CRC(c8e459b8) SHA1(c20ab073f61242f37665f12199b95cfa3a83e9fc) )
ROM_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  psx_gamebooster_device - constructor
//-------------------------------------------------

psx_gamebooster_device::psx_gamebooster_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PSX_GAMEBOOSTER, tag, owner, clock)
	, psx_parallel_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_cart_config("cart", ENDIANNESS_LITTLE, 8, 16, 0)
{
}


//**************************************************************************
//  DEVICE_T IMPLEMENTATION
//**************************************************************************

const tiny_rom_entry *psx_gamebooster_device::device_rom_region() const
{
	return ROM_NAME( psxgboost );
}

void psx_gamebooster_device::device_add_mconfig(machine_config &config)
{
	GB_CART_SLOT(config, "gbslot", gameboy_cartridges, nullptr).set_space(DEVICE_SELF, 0);

	SOFTWARE_LIST(config, "cart_list").set_original("gameboy");
	SOFTWARE_LIST(config, "gbc_list").set_compatible("gbcolor");
}

void psx_gamebooster_device::device_start()
{
	space(0).specific(m_cart_space);
}

void psx_gamebooster_device::device_reset()
{
}


//**************************************************************************
//  DEVICE_MEMORY_INTERFACE IMPLEMENTATION
//**************************************************************************

device_memory_interface::space_config_vector psx_gamebooster_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(0, &m_cart_config) };
}


//**************************************************************************
//  PSX_PARALLEL_INTERFACE IMPLEMENTATION
//**************************************************************************

uint16_t psx_gamebooster_device::exp_r(offs_t offset, uint16_t mem_mask)
{
	if (offset < 0x20000)
	{
		return m_rom[offset & 0x1ffff];
	}
	else if (offset < 0x24000)
	{
		offset -= 0x20000;
		uint16_t retval = 0;


		if (mem_mask & 0xff00) retval |= uint16_t(m_cart_space.read_byte((offset << 1) + 1)) << 8;
		if (mem_mask & 0x00ff) retval |= uint16_t(m_cart_space.read_byte((offset << 1) + 0)) << 0;

		return retval;
	}
	else
	{
		logerror("%s: psx_gamebooster_device::exp_r %04x\n", machine().describe_context(), offset << 1);

		return 0x0000;
	}
}

void psx_gamebooster_device::exp_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{

	if (offset < 0x20000)
	{
		logerror("%s: psx_gamebooster_device::exp_w %04x %04x\n", machine().describe_context(), offset << 1, data);
	}
	else if (offset < 0x24000)
	{
		offset -= 0x20000;
		LOG("%s: psx_gamebooster_device::exp_w %04x %04x\n", machine().describe_context(), offset << 1, data);

		if (mem_mask & 0xff00) m_cart_space.write_byte((offset << 1) + 1, data >> 8);
		if (mem_mask & 0x00ff) m_cart_space.write_byte((offset << 1) + 0, data >> 0); // send this 2nd or it erases the bank with the above
	}
	else
	{
		logerror("%s: psx_gamebooster_device::exp_w %04x %04x\n", machine().describe_context(), offset << 1, data);
	}
}
