// license:BSD-3-Clause
// copyright-holders:R. Belmont, Sven Schnelle
/***************************************************************************

  HP98544 high-resolution monochrome board

  VRAM at 0x200000, ROM and registers at 0x560000

***************************************************************************/

#include "emu.h"
#include "hp98544.h"
#include "screen.h"

#define HP98544_SCREEN_NAME   "98544_screen"
#define HP98544_ROM_REGION    "98544_rom"

ROM_START( hp98544 )
	ROM_REGION( 0x2000, HP98544_ROM_REGION, 0 )
	ROM_LOAD( "98544_1818-1999.bin", 0x000000, 0x002000, CRC(8c7d6480) SHA1(d2bcfd39452c38bc652df39f84c7041cfdf6bd51) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(HPDIO_98544, dio16_98544_device, "dio98544", "HP98544 high-res monochrome DIO video card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(dio16_98544_device::device_add_mconfig)
	MCFG_SCREEN_ADD(HP98544_SCREEN_NAME, RASTER)
	MCFG_SCREEN_UPDATE_DEVICE(DEVICE_SELF, dio16_98544_device, screen_update)
	MCFG_SCREEN_SIZE(1024,768)
	MCFG_SCREEN_VISIBLE_AREA(0, 1024-1, 0, 768-1)
	MCFG_SCREEN_REFRESH_RATE(70)

	MCFG_DEVICE_ADD("topcat", TOPCAT, XTAL(35904000))
	MCFG_TOPCAT_FB_WIDTH(1024)
	MCFG_TOPCAT_FB_HEIGHT(768)
	MCFG_TOPCAT_PLANES(1)
MACHINE_CONFIG_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *dio16_98544_device::device_rom_region() const
{
	return ROM_NAME( hp98544 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dio16_98544_device - constructor
//-------------------------------------------------

dio16_98544_device::dio16_98544_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_98544_device(mconfig, HPDIO_98544, tag, owner, clock)
{
}

dio16_98544_device::dio16_98544_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_dio16_card_interface(mconfig, *this),
	m_topcat(*this, "topcat")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dio16_98544_device::device_start()
{
	// set_nubus_device makes m_slot valid
	set_dio_device();

	m_rom = device().machine().root_device().memregion(this->subtag(HP98544_ROM_REGION).c_str())->base();

	m_dio->install_memory(
			0x200000, 0x2fffff,
			read16_delegate(FUNC(dio16_98544_device::vram_r), this),
			write16_delegate(FUNC(dio16_98544_device::vram_w), this));
	m_dio->install_memory(
			0x560000, 0x563fff,
			read16_delegate(FUNC(dio16_98544_device::rom_r), this),
			write16_delegate(FUNC(dio16_98544_device::rom_w), this));
	m_dio->install_memory(
			0x564000, 0x567fff,
			read16_delegate(FUNC(dio16_98544_device::ctrl_r), this),
			write16_delegate(FUNC(dio16_98544_device::ctrl_w), this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dio16_98544_device::device_reset()
{
}

READ16_MEMBER(dio16_98544_device::vram_r)
{
	return m_topcat->vram_r(space, offset, mem_mask);
}

WRITE16_MEMBER(dio16_98544_device::vram_w)
{
	m_topcat->vram_w(space, offset, data, mem_mask);
}

READ16_MEMBER(dio16_98544_device::rom_r)
{
	return 0xff00 | m_rom[offset];
}

// the video chip registers live here, so these writes are valid
WRITE16_MEMBER(dio16_98544_device::rom_w)
{
}

WRITE16_MEMBER(dio16_98544_device::ctrl_w)
{
	return m_topcat->ctrl_w(space, offset, data);
}

READ16_MEMBER(dio16_98544_device::ctrl_r)
{
	return m_topcat->ctrl_r(space, offset);
}

uint32_t dio16_98544_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return m_topcat->screen_update(screen, bitmap, cliprect);
}
