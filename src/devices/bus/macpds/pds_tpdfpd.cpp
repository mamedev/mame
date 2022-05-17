// license:BSD-3-Clause
// copyright-holders:R. Belmont

/***************************************************************************

  Radius Full Page Display card for the Mac SE, assy # 632-0022-A1

  EPROMs are marked
  "(c) 1991 Radius TPD/FPD-ASIC  U6  297-0204-A  V 4.1  256K" and "U7" with
  all other text the same.

  The SE PDS does not auto-configure like NuBus; cards have to snoop the 68k
  address bus and claim spots the motherboard logic doesn't want.

  This card claims these address ranges:
  770000-77000F: Bt9014 RAMDAC
  C00000-C0FFFF: EPROM
  C10000       : read to ack vblank IRQ 2 (returns bit 7 = 0 for vblank active)
  C20000       : read to enable vblank IRQ 2
  C40000-C7FFFF: 256k VRAM
  F80000-F8FFFF: EPROM mirror (the SE ROM looks for signatures and jump tables in this region)

  TODO:
    * suppress SE built-in screen (it stops working after OS boot with this card installed) & make our screen 3:4
    * investigate if there's also a two-page display mode as the rom labels imply; 256K is
      far too much for just 1024x880 but would fit double that nicely.
    * later ROM versions provide System 7 compatibility; our current dump is good only
      up to 6.0.8.  (We have 4.1; the last version is 4.4).

***************************************************************************/

#include "emu.h"
#include "pds_tpdfpd.h"

#include "cpu/m68000/m68000.h"
#include "screen.h"


#define SEDISPLAY_SCREEN_NAME "fpd_screen"
#define SEDISPLAY_ROM_REGION  "fpd_rom"

#define VRAM_SIZE   (256*1024)  // PCB has a jumper for 1MByte; may require different EPROMs

ROM_START( sedisplay )
	ROM_REGION(0x10000, SEDISPLAY_ROM_REGION, ROMREGION_16BIT|ROMREGION_BE)
	ROM_LOAD16_BYTE( "tfd_fpd-asic_u6_297-0205-a_v4_1", 0x0000, 0x8000, CRC(fd363f45) SHA1(3c4c596654647ee6ce1880de329aa675d298dc26) )
	ROM_LOAD16_BYTE( "tfd_fpd-asic_u7_297-0205-a_v4_1", 0x0001, 0x8000, CRC(5872451a) SHA1(4673d9f341766c49ff1264b7819916e28a20518f) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(PDS_SEDISPLAY, macpds_sedisplay_device, "pds_sefp", "Radius SE Full Page Display")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void macpds_sedisplay_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, SEDISPLAY_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(macpds_sedisplay_device::screen_update));
	screen.set_raw(55_MHz_XTAL, 800, 0, 640, 1024, 0, 870);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *macpds_sedisplay_device::device_rom_region() const
{
	return ROM_NAME( sedisplay );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  macpds_sedisplay_device - constructor
//-------------------------------------------------

macpds_sedisplay_device::macpds_sedisplay_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	macpds_sedisplay_device(mconfig, PDS_SEDISPLAY, tag, owner, clock)
{
}

macpds_sedisplay_device::macpds_sedisplay_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_macpds_card_interface(mconfig, *this),
	m_vram(nullptr), m_vbl_disable(0), m_count(0), m_clutoffs(0), m_timer(nullptr)
{
	set_screen(*this, SEDISPLAY_SCREEN_NAME);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void macpds_sedisplay_device::device_start()
{
	set_macpds_device();

	install_rom(this, SEDISPLAY_ROM_REGION, 0xc00000);
	install_rom(this, SEDISPLAY_ROM_REGION, 0xf80000);

	m_vram = std::make_unique<uint8_t[]>(VRAM_SIZE);

	m_macpds->install_bank(0xc40000, 0xc40000+VRAM_SIZE-1, m_vram.get());

	m_macpds->install_device(0x770000, 0x77000f, read16s_delegate(*this, FUNC(macpds_sedisplay_device::ramdac_r)), write16s_delegate(*this, FUNC(macpds_sedisplay_device::ramdac_w)));
	m_macpds->install_device(0xc10000, 0xc2ffff, read16sm_delegate(*this, FUNC(macpds_sedisplay_device::sedisplay_r)), write16sm_delegate(*this, FUNC(macpds_sedisplay_device::sedisplay_w)));

	m_timer = timer_alloc(FUNC(macpds_sedisplay_device::vbl_tick), this);
	m_timer->adjust(screen().time_until_pos(879, 0), 0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void macpds_sedisplay_device::device_reset()
{
	m_count = 0;
	m_clutoffs = 0;
	m_vbl_disable = 1;
	memset(m_vram.get(), 0, VRAM_SIZE);
	memset(m_palette, 0, sizeof(m_palette));

	m_palette[0] = rgb_t(0, 0, 0);
	m_palette[1] = rgb_t(255, 255, 255);
}


TIMER_CALLBACK_MEMBER(macpds_sedisplay_device::vbl_tick)
{
	if (!m_vbl_disable)
	{
		m_macpds->set_irq_line(M68K_IRQ_2, ASSERT_LINE);
	}

	m_timer->adjust(screen().time_until_pos(879, 0), 0);
}

/***************************************************************************

  CB264 section

***************************************************************************/

uint32_t macpds_sedisplay_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const vram = m_vram.get();

	for (int y = 0; y < 870; y++)
	{
		uint32_t *scanline = &bitmap.pix(y);
		for (int x = 0; x < 640/8; x++)
		{
			uint8_t const pixels = vram[(y * (1024/8)) + (x^1)];

			*scanline++ = m_palette[BIT(~pixels, 7)];
			*scanline++ = m_palette[BIT(~pixels, 6)];
			*scanline++ = m_palette[BIT(~pixels, 5)];
			*scanline++ = m_palette[BIT(~pixels, 4)];
			*scanline++ = m_palette[BIT(~pixels, 3)];
			*scanline++ = m_palette[BIT(~pixels, 2)];
			*scanline++ = m_palette[BIT(~pixels, 1)];
			*scanline++ = m_palette[BIT(~pixels, 0)];
		}
	}

	return 0;
}

void macpds_sedisplay_device::sedisplay_w(offs_t offset, uint16_t data)
{
}

uint16_t macpds_sedisplay_device::sedisplay_r(offs_t offset)
{
	if (offset == 0)    // ack vbl
	{
		m_macpds->set_irq_line(M68K_IRQ_2, CLEAR_LINE);
	}
	else if (offset == 0x8000)  // enable vbl
	{
		m_vbl_disable = 0;
	}

	return 0;
}

void macpds_sedisplay_device::ramdac_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset)
	{
		case 4:
			m_clutoffs = data>>8;
			break;

		case 5:
			m_colors[m_count++] = data>>4;  // they only fill in the lower nibble

			if (m_count == 3)
			{                       // only the green channel drives the output
				m_palette[m_clutoffs] = rgb_t(m_colors[1], m_colors[1], m_colors[1]);
				m_clutoffs++;
				m_count = 0;
			}
			break;

		default:
//          printf("RAMDAC: %x to %x (mask %04x)\n", data, offset, mem_mask);
			break;
	}
}

uint16_t macpds_sedisplay_device::ramdac_r(offs_t offset, uint16_t mem_mask)
{
	return 0;
}
