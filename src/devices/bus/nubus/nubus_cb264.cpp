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

#define CB264_SCREEN_NAME   "cb264_screen"
#define CB264_ROM_REGION    "cb264_rom"

#define VRAM_SIZE   (0x200000)  // 2 megs, maxed out

MACHINE_CONFIG_FRAGMENT( cb264 )
	MCFG_SCREEN_ADD( CB264_SCREEN_NAME, RASTER)
	MCFG_SCREEN_UPDATE_DEVICE(DEVICE_SELF, nubus_cb264_device, screen_update)
	MCFG_SCREEN_RAW_PARAMS(25175000, 800, 0, 640, 525, 0, 480)
	MCFG_SCREEN_SIZE(1024,768)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
MACHINE_CONFIG_END

ROM_START( cb264 )
	ROM_REGION(0x4000, CB264_ROM_REGION, 0)
	ROM_LOAD16_BYTE( "264-1915.bin", 0x000000, 0x002000, CRC(26c19ee5) SHA1(2b2853d04cc6b0258e85eccd23ebfd4f4f63a084) )
	ROM_LOAD16_BYTE( "264-1914.bin", 0x000001, 0x002000, CRC(d5fbd5ad) SHA1(98d35ed3fb0bca4a9bee1cdb2af0d3f22b379386) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type NUBUS_CB264 = &device_creator<nubus_cb264_device>;


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor nubus_cb264_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cb264 );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *nubus_cb264_device::device_rom_region() const
{
	return ROM_NAME( cb264 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_cb264_device - constructor
//-------------------------------------------------

nubus_cb264_device::nubus_cb264_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, NUBUS_CB264, "RasterOps ColorBoard 264 video card", tag, owner, clock, "nb_cb264", __FILE__),
		device_nubus_card_interface(mconfig, *this), m_cb264_mode(0), m_cb264_vbl_disable(0), m_cb264_toggle(0), m_count(0), m_clutoffs(0)
{
}

nubus_cb264_device::nubus_cb264_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_nubus_card_interface(mconfig, *this), m_cb264_mode(0), m_cb264_vbl_disable(0), m_cb264_toggle(0), m_count(0), m_clutoffs(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_cb264_device::device_start()
{
	UINT32 slotspace;

	// set_nubus_device makes m_slot valid
	set_nubus_device();
	install_declaration_rom(this, CB264_ROM_REGION);

	slotspace = get_slotspace();

//  printf("[cb264 %p] slotspace = %x\n", this, slotspace);

	m_vram.resize(VRAM_SIZE);
	install_bank(slotspace, slotspace+VRAM_SIZE-1, 0, 0, "bank_cb264", &m_vram[0]);

	m_nubus->install_device(slotspace+0xff6000, slotspace+0xff60ff, read32_delegate(FUNC(nubus_cb264_device::cb264_r), this), write32_delegate(FUNC(nubus_cb264_device::cb264_w), this));
	m_nubus->install_device(slotspace+0xff7000, slotspace+0xff70ff, read32_delegate(FUNC(nubus_cb264_device::cb264_ramdac_r), this), write32_delegate(FUNC(nubus_cb264_device::cb264_ramdac_w), this));
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
	memset(&m_vram[0], 0, VRAM_SIZE);
	memset(m_palette, 0, sizeof(m_palette));
}

/***************************************************************************

  ColorBoard 264 section

***************************************************************************/

UINT32 nubus_cb264_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *scanline, *base;
	int x, y;
	UINT8 pixels;

	if (!m_cb264_vbl_disable)
	{
		raise_slot_irq();
	}

	switch (m_cb264_mode)
	{
		case 0: // 1 bpp
			for (y = 0; y < 480; y++)
			{
				scanline = &bitmap.pix32(y);
				for (x = 0; x < 640/8; x++)
				{
					pixels = m_vram[(y * 1024) + (BYTE4_XOR_BE(x))];

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
			for (y = 0; y < 480; y++)
			{
				scanline = &bitmap.pix32(y);
				for (x = 0; x < 640/4; x++)
				{
					pixels = m_vram[(y * 1024) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_palette[pixels&0xc0];
					*scanline++ = m_palette[(pixels<<2)&0xc0];
					*scanline++ = m_palette[(pixels<<4)&0xc0];
					*scanline++ = m_palette[(pixels<<6)&0xc0];
				}
			}
			break;

		case 2: // 4 bpp
			for (y = 0; y < 480; y++)
			{
				scanline = &bitmap.pix32(y);

				for (x = 0; x < 640/2; x++)
				{
					pixels = m_vram[(y * 1024) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_palette[pixels&0xf0];
					*scanline++ = m_palette[(pixels<<4)&0xf0];
				}
			}
			break;

		case 3: // 8 bpp
			for (y = 0; y < 480; y++)
			{
				scanline = &bitmap.pix32(y);

				for (x = 0; x < 640; x++)
				{
					pixels = m_vram[(y * 1024) + (BYTE4_XOR_BE(x))];
					*scanline++ = m_palette[pixels];
				}
			}
			break;

		case 4: // 24 bpp
		case 7: // ???
			{
				UINT32 *vram32 = (UINT32 *)&m_vram[0];

				for (y = 0; y < 480; y++)
				{
					scanline = &bitmap.pix32(y);
					base = &vram32[y * 1024];
					for (x = 0; x < 640; x++)
					{
						*scanline++ = *base++;
					}
				}
			}
			break;

		default:
			fatalerror("cb264: unknown video mode %d\n", m_cb264_mode);
	}

	return 0;
}

WRITE32_MEMBER( nubus_cb264_device::cb264_w )
{
	switch (offset)
	{
		case 0x4/4: // 0 = 1 bpp, 1 = 2bpp, 2 = 4bpp, 3 = 8bpp, 4 = 24bpp
			m_cb264_mode = data;
			break;

		case 0x14/4:    // VBL ack
			lower_slot_irq();
			break;

		case 0x3c/4:    // VBL disable
			m_cb264_vbl_disable = data;
			break;

		default:
//          printf("cb264_w: %x to reg %x (mask %x PC %x)\n", data, offset*4, mem_mask, space.device().safe_pc());
			break;
	}
}

READ32_MEMBER( nubus_cb264_device::cb264_r )
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
			logerror("cb264_r: reg %x (mask %x PC %x)\n", offset*4, mem_mask, space.device().safe_pc());
			break;
	}

	return 0;
}

WRITE32_MEMBER( nubus_cb264_device::cb264_ramdac_w )
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

READ32_MEMBER( nubus_cb264_device::cb264_ramdac_r )
{
	return 0;
}
