// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Vic Flash Plugin cartridge emulation

    http://www.iki.fi/~msmakela/8bit/vfp/

**********************************************************************/

#include "emu.h"
#include "vfp.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

enum
{
	CFG_A21       = 0x01,
	CFG_RAM123    = 0x08,
	CFG_BLK1      = 0x18,
	CFG_RAM_MASK  = 0x18,
	CFG_BLK5_RAM  = 0x20,
	CFG_BLK5_WP   = 0x40,
	CFG_IO2_OFF   = 0x80,
	CFG_MASK      = 0xf9
};



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC20_VFP, vic20_vic_flash_plugin_device, "vic20_vfp", "Vic Flash Plugin")


//-------------------------------------------------
//  ROM( vic20_vfp )
//-------------------------------------------------

ROM_START( vic20_vfp )
	ROM_REGION( 0x400000, "ic1", 0 )
	ROM_LOAD( "menu-s.bin", 0x000000, 0x400000, CRC(de4f023b) SHA1(164666798dfcee7d6b86a39548af4a97d72c0852) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *vic20_vic_flash_plugin_device::device_rom_region() const
{
	return ROM_NAME( vic20_vfp );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void vic20_vic_flash_plugin_device::device_add_mconfig(machine_config &config)
{
	AMD_29F032(config, m_flash);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic20_vic_flash_plugin_device - constructor
//-------------------------------------------------

vic20_vic_flash_plugin_device::vic20_vic_flash_plugin_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VIC20_VFP, tag, owner, clock)
	, device_vic20_expansion_card_interface(mconfig, *this)
	, m_flash(*this, "ic1")
	, m_rom(*this, "ic1")
	, m_ram(*this, "ram", 0x8000, ENDIANNESS_LITTLE)
	, m_bank(0)
	, m_cfg(CFG_BLK5_WP)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic20_vic_flash_plugin_device::device_start()
{
	// the dump is in CPU address order, but CPU A0/A1 are wired to flash A1/A0
	uint8_t *const rom = m_rom->base();

	for (offs_t offset = 0; offset < m_rom->bytes(); offset += 4)
		std::swap(rom[offset + 1], rom[offset + 2]);

	m_slot->ram1()[0].install_ram(0x000, 0x3ff, &m_ram[0x2400]);
	m_slot->ram2()[0].install_ram(0x000, 0x3ff, &m_ram[0x2800]);
	m_slot->ram3()[0].install_ram(0x000, 0x3ff, &m_ram[0x2c00]);
	m_slot->blk1()[0].install_ram(0x0000, 0x1fff, &m_ram[0x2000]);
	m_slot->blk2().install_ram(0x0000, 0x1fff, &m_ram[0x4000]);
	m_slot->blk3().install_ram(0x0000, 0x1fff, &m_ram[0x6000]);
	m_slot->blk5()[0].install_ram(0x0000, 0x1fff, &m_ram[0x0000]);
	m_slot->blk5()[1].install_rom(0x0000, 0x1fff, &m_ram[0x0000]);
	m_slot->blk5()[2].install_readwrite_handler(0x0000, 0x1fff, read8sm_delegate(*this, FUNC(vic20_vic_flash_plugin_device::blk5_r)), write8sm_delegate(*this, FUNC(vic20_vic_flash_plugin_device::blk5_w)));
	m_slot->blk5()[3].install_read_handler(0x0000, 0x1fff, read8sm_delegate(*this, FUNC(vic20_vic_flash_plugin_device::blk5_r)));
	m_slot->io2()[0].install_readwrite_handler(0x000, 0x001, 0x3fe, read8sm_delegate(*this, FUNC(vic20_vic_flash_plugin_device::io2_r)), write8sm_delegate(*this, FUNC(vic20_vic_flash_plugin_device::io2_w)));

	// state saving
	save_item(NAME(m_bank));
	save_item(NAME(m_cfg));

	machine().save().register_postload(save_prepost_delegate(FUNC(vic20_vic_flash_plugin_device::update_map), this));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic20_vic_flash_plugin_device::device_reset()
{
	m_bank = 0;
	m_cfg = CFG_BLK5_WP;

	update_map();
}


//-------------------------------------------------
//  flash_address -
//-------------------------------------------------

offs_t vic20_vic_flash_plugin_device::flash_address(offs_t offset) const
{
	return (BIT(m_cfg, 0) << 21) | (m_bank << 13) | (offset & 0x1ffc) | bitswap<2>(offset, 0, 1);
}


//-------------------------------------------------
//  blk5_r -
//-------------------------------------------------

uint8_t vic20_vic_flash_plugin_device::blk5_r(offs_t offset)
{
	return m_flash->read(flash_address(offset));
}


//-------------------------------------------------
//  blk5_w -
//-------------------------------------------------

void vic20_vic_flash_plugin_device::blk5_w(offs_t offset, uint8_t data)
{
	m_flash->write(flash_address(offset), data);
}


//-------------------------------------------------
//  io2_r -
//-------------------------------------------------

uint8_t vic20_vic_flash_plugin_device::io2_r(offs_t offset)
{
	return offset ? m_cfg : m_bank;
}


//-------------------------------------------------
//  io2_w -
//-------------------------------------------------

void vic20_vic_flash_plugin_device::io2_w(offs_t offset, uint8_t data)
{
	if (offset)
		m_cfg = data & CFG_MASK;
	else
		m_bank = data;

	update_map();
}


//-------------------------------------------------
//  update_map -
//-------------------------------------------------

void vic20_vic_flash_plugin_device::update_map()
{
	if ((m_cfg & CFG_RAM_MASK) == CFG_RAM123)
	{
		m_slot->ram1().select(0);
		m_slot->ram2().select(0);
		m_slot->ram3().select(0);
	}
	else
	{
		m_slot->ram1().unmap();
		m_slot->ram2().unmap();
		m_slot->ram3().unmap();
	}

	if ((m_cfg & CFG_RAM_MASK) == CFG_BLK1)
		m_slot->blk1().select(0);
	else
		m_slot->blk1().unmap();

	m_slot->blk5().select(((m_cfg & CFG_BLK5_RAM) ? 0 : 2) + ((m_cfg & CFG_BLK5_WP) ? 1 : 0));

	if (m_cfg & CFG_IO2_OFF)
		m_slot->io2().unmap();
	else
		m_slot->io2().select(0);
}
