// license:BSD-3-Clause
// copyright-holders:David Haywood
/**********************************************************************

    Datel Game Booster for Playstation 1

    Gameboy emulator with Gameboy cartridge slot

**********************************************************************/

#include "emu.h"
#include "gamebooster.h"

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

READ16_MEMBER(psx_gamebooster_device::exp_r)
{
	if (offset < 0x20000)
	{
		return m_rom->base()[(offset * 2) & 0x3ffff] | (m_rom->base()[((offset * 2) + 1) & 0x3ffff] << 8);
	}
	else if (offset < 0x24000)
	{
		offset -= 0x20000;
		uint16_t retval = 0;;

		if (mem_mask & 0xff00) retval |= (m_cartslot->read_rom(space, (offset*2)+1))<<8;
		if (mem_mask & 0x00ff) retval |= m_cartslot->read_rom(space, (offset*2)+0);

		return retval;
	}
	else
	{
		logerror("%s: psx_gamebooster_device::exp_r %04x\n", machine().describe_context(), offset*2);
	}

	return 0x0000;
}

WRITE16_MEMBER(psx_gamebooster_device::exp_w)
{

	if (offset < 0x20000)
	{
		logerror("%s: psx_gamebooster_device::exp_w %04x %04x\n", machine().describe_context(), offset*2, data);
	}
	else if (offset < 0x24000)
	{
		offset -= 0x20000;
		logerror("%s: psx_gamebooster_device::exp_w %04x %04x\n", machine().describe_context(), offset*2, data);

		if (mem_mask & 0xff00) m_cartslot->write_bank(space, (offset*2)+1, data>>8);
		if (mem_mask & 0x00ff) m_cartslot->write_bank(space, (offset*2)+0, data); // send this 2nd or it erases the bank with the above

	}
	else
	{
		logerror("%s: psx_gamebooster_device::exp_w %04x %04x\n", machine().describe_context(), offset*2, data);
	}
}

static void gb_cart(device_slot_interface &device)
{
	device.option_add_internal("rom",         GB_STD_ROM);
	device.option_add_internal("rom_mbc1",    GB_ROM_MBC1);
	device.option_add_internal("rom_mbc1col", GB_ROM_MBC1);
	device.option_add_internal("rom_mbc2",    GB_ROM_MBC2);
	device.option_add_internal("rom_mbc3",    GB_ROM_MBC3);
	device.option_add_internal("rom_huc1",    GB_ROM_MBC3);
	device.option_add_internal("rom_huc3",    GB_ROM_MBC3);
	device.option_add_internal("rom_mbc5",    GB_ROM_MBC5);
	device.option_add_internal("rom_mbc6",    GB_ROM_MBC6);
	device.option_add_internal("rom_mbc7",    GB_ROM_MBC7);
	device.option_add_internal("rom_tama5",   GB_ROM_TAMA5);
	device.option_add_internal("rom_mmm01",   GB_ROM_MMM01);
	device.option_add_internal("rom_m161",    GB_ROM_M161);
	device.option_add_internal("rom_sachen1", GB_ROM_SACHEN1);
	device.option_add_internal("rom_sachen2", GB_ROM_SACHEN2);
	device.option_add_internal("rom_wisdom",  GB_ROM_WISDOM);
	device.option_add_internal("rom_yong",    GB_ROM_YONG);
	device.option_add_internal("rom_lasama",  GB_ROM_LASAMA);
	device.option_add_internal("rom_atvrac",  GB_ROM_ATVRAC);
	device.option_add_internal("rom_camera",  GB_STD_ROM);
	device.option_add_internal("rom_188in1",  GB_ROM_188IN1);
	device.option_add_internal("rom_sintax",  GB_ROM_SINTAX);
	device.option_add_internal("rom_chong",   GB_ROM_CHONGWU);
	device.option_add_internal("rom_licheng", GB_ROM_LICHENG);
	device.option_add_internal("rom_digimon", GB_ROM_DIGIMON);
	device.option_add_internal("rom_rock8",   GB_ROM_ROCKMAN8);
	device.option_add_internal("rom_sm3sp",   GB_ROM_SM3SP);
//  device.option_add_internal("rom_dkong5",  GB_ROM_DKONG5);
//  device.option_add_internal("rom_unk01",   GB_ROM_UNK01);
}

void psx_gamebooster_device::device_add_mconfig(machine_config &config)
{
	/* cartslot */
	GB_CART_SLOT(config, m_cartslot, gb_cart, nullptr);

	SOFTWARE_LIST(config, "cart_list").set_original("gameboy");
	SOFTWARE_LIST(config, "gbc_list").set_compatible("gbcolor");
}
