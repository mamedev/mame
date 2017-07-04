// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  HP98544 high-resolution monochrome board

  VRAM at 0x200000, ROM and registers at 0x560000

***************************************************************************/

#include "emu.h"
#include "hp98544.h"
#include "screen.h"

#define HP98544_SCREEN_NAME   "98544_screen"
#define HP98544_ROM_REGION    "98544_rom"

#define VRAM_SIZE   (0x100000)

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

MACHINE_CONFIG_MEMBER( dio16_98544_device::device_add_mconfig )
	MCFG_SCREEN_ADD(HP98544_SCREEN_NAME, RASTER)
	MCFG_SCREEN_UPDATE_DEVICE(DEVICE_SELF, dio16_98544_device, screen_update)
	MCFG_SCREEN_SIZE(1024,768)
	MCFG_SCREEN_VISIBLE_AREA(0, 1024-1, 0, 768-1)
	MCFG_SCREEN_REFRESH_RATE(70)
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
	device_dio16_card_interface(mconfig, *this)
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

	m_vram.resize(VRAM_SIZE);
	m_dio->install_memory(0x200000, 0x2fffff, read16_delegate(FUNC(dio16_98544_device::vram_r), this),
							write16_delegate(FUNC(dio16_98544_device::vram_w), this));
	m_dio->install_memory(0x560000, 0x563fff, read16_delegate(FUNC(dio16_98544_device::rom_r), this),
							write16_delegate(FUNC(dio16_98544_device::rom_w), this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dio16_98544_device::device_reset()
{
	memset(&m_vram[0], 0, VRAM_SIZE);

	m_palette[1] = rgb_t(255, 255, 255);
	m_palette[0] = rgb_t(0, 0, 0);
}

READ16_MEMBER(dio16_98544_device::vram_r)
{
	return m_vram[offset];
}

WRITE16_MEMBER(dio16_98544_device::vram_w)
{
	COMBINE_DATA(&m_vram[offset]);
}

READ16_MEMBER(dio16_98544_device::rom_r)
{
	return 0xff00 | m_rom[offset];
}

// the video chip registers live here, so these writes are valid
WRITE16_MEMBER(dio16_98544_device::rom_w)
{
	printf("rom_w: %02x at %x (mask %04x)\n", data, offset, mem_mask);
}

uint32_t dio16_98544_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t *scanline;
	int x, y;
	uint32_t pixels;

	for (y = 0; y < 768; y++)
	{
		scanline = &bitmap.pix32(y);
		for (x = 0; x < 1024/2; x++)
		{
			pixels = m_vram[(y * 512) + x];

			*scanline++ = m_palette[(pixels>>8) & 1];
			*scanline++ = m_palette[(pixels & 1)];
		}
	}

	return 0;
}
