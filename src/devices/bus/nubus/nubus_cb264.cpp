// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  RasterOps ColorBoard 264 NuBus video card emulation

  fixed resolution 640x480 NuBus video card, 1/4/8/16/24 bit color
  1.5? MB of VRAM (tests up to 0x1fffff), Bt473 RAMDAC, and two custom gate arrays.

  0xfsff6004 is color depth: 0 for 1bpp, 1 for 2bpp, 2 for 4bpp, 3 for 8bpp, 4 for 24bpp
  0xfsff6014 is VBL ack: write 1 to ack
  0xfsff603c is VBL disable: write 1 to disable, 0 to enable

***************************************************************************/

#include "emu.h"
#include "nubus_cb264.h"
#include "screen.h"

#include <algorithm>


#define CB264_SCREEN_NAME   "cb264_screen"
#define CB264_ROM_REGION    "cb264_rom"

#define VRAM_SIZE   (0x200000)  // 2 megs, maxed out


ROM_START( cb264 )
	ROM_REGION(0x4000, CB264_ROM_REGION, 0)
	ROM_LOAD16_BYTE( "264-1915.bin", 0x000000, 0x002000, CRC(26c19ee5) SHA1(2b2853d04cc6b0258e85eccd23ebfd4f4f63a084) )
	ROM_LOAD16_BYTE( "264-1914.bin", 0x000001, 0x002000, CRC(d5fbd5ad) SHA1(98d35ed3fb0bca4a9bee1cdb2af0d3f22b379386) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(NUBUS_CB264, nubus_cb264_device, "nb_c264", "RasterOps ColorBoard 264 video card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nubus_cb264_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, CB264_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(nubus_cb264_device::screen_update));
	screen.set_raw(25175000, 800, 0, 640, 525, 0, 480);
	screen.set_size(1024, 768);
	screen.set_visarea(0, 640-1, 0, 480-1);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nubus_cb264_device::device_rom_region() const
{
	return ROM_NAME( cb264 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_cb264_device - constructor
//-------------------------------------------------

nubus_cb264_device::nubus_cb264_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nubus_cb264_device(mconfig, NUBUS_CB264, tag, owner, clock)
{
}

nubus_cb264_device::nubus_cb264_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_nubus_card_interface(mconfig, *this),
	m_cb264_mode(0), m_cb264_vbl_disable(0), m_cb264_toggle(0), m_count(0), m_clutoffs(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_cb264_device::device_start()
{
	uint32_t slotspace;

	install_declaration_rom(CB264_ROM_REGION);

	slotspace = get_slotspace();

//  printf("[cb264 %p] slotspace = %x\n", this, slotspace);

	m_vram.resize(VRAM_SIZE / sizeof(uint32_t));
	install_bank(slotspace, slotspace+VRAM_SIZE-1, &m_vram[0]);

	nubus().install_device(slotspace+0xff6000, slotspace+0xff60ff, read32s_delegate(*this, FUNC(nubus_cb264_device::cb264_r)), write32s_delegate(*this, FUNC(nubus_cb264_device::cb264_w)));
	nubus().install_device(slotspace+0xff7000, slotspace+0xff70ff, read32sm_delegate(*this, FUNC(nubus_cb264_device::cb264_ramdac_r)), write32sm_delegate(*this, FUNC(nubus_cb264_device::cb264_ramdac_w)));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_cb264_device::device_reset()
{
	m_cb264_toggle = 0;
	m_count = 0;
	m_clutoffs = 0;
	m_cb264_vbl_disable = 1;
	m_cb264_mode = 0;
	std::fill(m_vram.begin(), m_vram.end(), 0);
	memset(m_palette, 0, sizeof(m_palette));
}

/***************************************************************************

  ColorBoard 264 section

***************************************************************************/

uint32_t nubus_cb264_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!m_cb264_vbl_disable)
	{
		raise_slot_irq();
	}

	auto const vram8 = util::big_endian_cast<uint8_t const>(&m_vram[0]);
	switch (m_cb264_mode)
	{
		case 0: // 1 bpp
			for (int y = 0; y < 480; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/8; x++)
				{
					uint8_t const pixels = vram8[(y * 1024) + x];

					*scanline++ = m_palette[pixels&0x80];
					*scanline++ = m_palette[(pixels<<1)&0x80];
					*scanline++ = m_palette[(pixels<<2)&0x80];
					*scanline++ = m_palette[(pixels<<3)&0x80];
					*scanline++ = m_palette[(pixels<<4)&0x80];
					*scanline++ = m_palette[(pixels<<5)&0x80];
					*scanline++ = m_palette[(pixels<<6)&0x80];
					*scanline++ = m_palette[(pixels<<7)&0x80];
				}
			}
			break;

		case 1: // 2 bpp (3f/7f/bf/ff)
			for (int y = 0; y < 480; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/4; x++)
				{
					uint8_t const pixels = vram8[(y * 1024) + x];

					*scanline++ = m_palette[pixels&0xc0];
					*scanline++ = m_palette[(pixels<<2)&0xc0];
					*scanline++ = m_palette[(pixels<<4)&0xc0];
					*scanline++ = m_palette[(pixels<<6)&0xc0];
				}
			}
			break;

		case 2: // 4 bpp
			for (int y = 0; y < 480; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/2; x++)
				{
					uint8_t const pixels = vram8[(y * 1024) + x];

					*scanline++ = m_palette[pixels&0xf0];
					*scanline++ = m_palette[(pixels<<4)&0xf0];
				}
			}
			break;

		case 3: // 8 bpp
			for (int y = 0; y < 480; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640; x++)
				{
					uint8_t const pixels = vram8[(y * 1024) + x];
					*scanline++ = m_palette[pixels];
				}
			}
			break;

		case 4: // 24 bpp
		case 7: // ???
			for (int y = 0; y < 480; y++)
			{
				std::copy_n(&m_vram[y * 1024], 640, &bitmap.pix(y));
			}
			break;

		default:
			fatalerror("cb264: unknown video mode %d\n", m_cb264_mode);
	}

	return 0;
}

void nubus_cb264_device::cb264_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0x4/4: // 0 = 1 bpp, 1 = 2bpp, 2 = 4bpp, 3 = 8bpp, 4 = 24bpp
			m_cb264_mode = data & 0x7;
			break;

		case 0x14/4:    // VBL ack
			lower_slot_irq();
			break;

		case 0x3c/4:    // VBL disable
			m_cb264_vbl_disable = data;
			break;

		default:
//          printf("%s cb264_w: %x to reg %x (mask %x)\n", machine().describe_context().c_str(), data, offset*4, mem_mask);
			break;
	}
}

uint32_t nubus_cb264_device::cb264_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0x0c/4:
		case 0x28/4:
			break;

		case 0x34/4:
			m_cb264_toggle ^= 1;
			return m_cb264_toggle;  // bit 0 is vblank?

		default:
			logerror("cb264_r: reg %x (mask %x %s)\n", offset*4, mem_mask, machine().describe_context());
			break;
	}

	return 0;
}

void nubus_cb264_device::cb264_ramdac_w(offs_t offset, uint32_t data)
{
	switch (offset)
	{
		case 0:
			m_clutoffs = data>>24;
			m_count = 0;
			break;

		case 1:
			m_colors[m_count++] = data>>24;

			if (m_count == 3)
			{
				m_palette[m_clutoffs] = rgb_t(m_colors[0], m_colors[1], m_colors[2]);
				m_clutoffs++;
				m_count = 0;
			}
			break;

		default:
//          printf("%x to unknown RAMDAC register @ %x\n", data, offset);
			break;
	}
}

uint32_t nubus_cb264_device::cb264_ramdac_r(offs_t offset)
{
	return 0;
}
