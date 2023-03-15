// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Apple model 820-5037-C "Macintosh II Portrait Video Card"
  PCB is marked "Workstation/Portrait Card"
  640x870, 1, 2 or 4bpp grayscale

  Fs0900e0 = DAC control
  Fs0900e4 = DAC data
  Fs0A0000 = enable / ack VBL IRQ
  Fs0A0004 = disable VBL IRQ

***************************************************************************/

#include "emu.h"
#include "nubus_wsportrait.h"
#include "screen.h"

#include <algorithm>

#define WSPORTRAIT_SCREEN_NAME  "wsport_screen"
#define WSPORTRAIT_ROM_REGION  "wsport_rom"

#define VRAM_SIZE   (0x80000)   // 512k max


ROM_START( wsportrait )
	ROM_REGION(0x1000, WSPORTRAIT_ROM_REGION, 0)
	ROM_LOAD( "341-0732.bin", 0x000000, 0x001000, CRC(ddc35b78) SHA1(ce2bf2374bb994c17962dba8f3d11bc1260e2644) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(NUBUS_WSPORTRAIT, nubus_wsportrait_device, "nb_wspt", "Macintosh II Portrait Video Card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nubus_wsportrait_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, WSPORTRAIT_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(nubus_wsportrait_device::screen_update));
	screen.set_size(1024, 960);
	screen.set_refresh_hz(75.0);
	screen.set_visarea(0, 640-1, 0, 870-1);
	screen.set_physical_aspect(3, 4);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nubus_wsportrait_device::device_rom_region() const
{
	return ROM_NAME( wsportrait );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_wsportrait_device - constructor
//-------------------------------------------------

nubus_wsportrait_device::nubus_wsportrait_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nubus_wsportrait_device(mconfig, NUBUS_WSPORTRAIT, tag, owner, clock)
{
}

nubus_wsportrait_device::nubus_wsportrait_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_nubus_card_interface(mconfig, *this),
	m_mode(0), m_vbl_disable(0), m_toggle(0), m_count(0), m_clutoffs(0), m_timer(nullptr)
{
	set_screen(*this, WSPORTRAIT_SCREEN_NAME);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_wsportrait_device::device_start()
{
	uint32_t slotspace;

	install_declaration_rom(WSPORTRAIT_ROM_REGION, true);

	slotspace = get_slotspace();

//  printf("[wsportrait %p] slotspace = %x\n", (void *)this, slotspace);

	m_vram.resize(VRAM_SIZE / sizeof(uint32_t));

	nubus().install_device(slotspace, slotspace+VRAM_SIZE-1, read32s_delegate(*this, FUNC(nubus_wsportrait_device::vram_r)), write32s_delegate(*this, FUNC(nubus_wsportrait_device::vram_w)));
	nubus().install_device(slotspace+0x900000, slotspace+0x900000+VRAM_SIZE-1, read32s_delegate(*this, FUNC(nubus_wsportrait_device::vram_r)), write32s_delegate(*this, FUNC(nubus_wsportrait_device::vram_w)));
	nubus().install_device(slotspace+0x80000, slotspace+0xeffff, read32s_delegate(*this, FUNC(nubus_wsportrait_device::wsportrait_r)), write32s_delegate(*this, FUNC(nubus_wsportrait_device::wsportrait_w)));

	m_timer = timer_alloc(FUNC(nubus_wsportrait_device::vbl_tick), this);
	m_timer->adjust(screen().time_until_pos(869, 0), 0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_wsportrait_device::device_reset()
{
	m_count = 0;
	m_clutoffs = 0;
	m_vbl_disable = 1;
	m_mode = 0;
	std::fill(m_vram.begin(), m_vram.end(), 0);
	memset(m_palette, 0, sizeof(m_palette));
}


TIMER_CALLBACK_MEMBER(nubus_wsportrait_device::vbl_tick)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	m_timer->adjust(screen().time_until_pos(869, 0), 0);
}

/***************************************************************************

  Workstation/Portrait emulation

***************************************************************************/

uint32_t nubus_wsportrait_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// first time?  kick off the VBL timer
	auto const vram8 = util::big_endian_cast<uint8_t const>(&m_vram[0]) + 0x80;

	switch (m_mode)
	{
		case 0: // 1 bpp?
			for (int y = 0; y < 870; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/8; x++)
				{
					uint8_t const pixels = vram8[(y * 128) + x];

					*scanline++ = m_palette[BIT(pixels, 7)];
					*scanline++ = m_palette[BIT(pixels, 6)];
					*scanline++ = m_palette[BIT(pixels, 5)];
					*scanline++ = m_palette[BIT(pixels, 4)];
					*scanline++ = m_palette[BIT(pixels, 3)];
					*scanline++ = m_palette[BIT(pixels, 2)];
					*scanline++ = m_palette[BIT(pixels, 1)];
					*scanline++ = m_palette[BIT(pixels, 0)];
				}
			}
			break;

		case 1: // 2 bpp
			for (int y = 0; y < 480; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/4; x++)
				{
					uint8_t const pixels = vram8[(y * 256) + x];

					*scanline++ = m_palette[((pixels>>6)&3)];
					*scanline++ = m_palette[((pixels>>4)&3)];
					*scanline++ = m_palette[((pixels>>2)&3)];
					*scanline++ = m_palette[(pixels&3)];
				}
			}
			break;

		case 2: // 4 bpp
			for (int y = 0; y < 480; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/2; x++)
				{
					uint8_t const pixels = vram8[(y * 512) + x];

					*scanline++ = m_palette[((pixels&0xf0)>>4)];
					*scanline++ = m_palette[(pixels&0xf)];
				}
			}
			break;

		default:
			fatalerror("wsportrait: unknown video mode %d\n", m_mode);
	}
	return 0;
}

void nubus_wsportrait_device::wsportrait_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data ^= 0xffffffff;
//  if (offset != 0x8000) printf("wsportrait: Write %08x @ %x, mask %08x\n", data, offset, mem_mask);

	switch (offset)
	{
		case 1:         // mode control
//          printf("%08x to mode 1\n", data);
			switch (data & 0xff000000)
			{
				case 0x20000000:
				case 0x24000000:
					m_mode = 0;
					break;

				case 0x40000000:
					m_mode = 1;
					break;

				case 0x50000000:
				case 0x80000000:
					m_mode = 2;
					break;
			}
			break;

		case 0x4038:    // DAC control
			m_clutoffs = (data>>24)&0xff;
			break;

		case 0x4039:    // DAC data - only 4 bits per component!
			m_colors[m_count] = (data>>24) & 0x0f;
			m_colors[m_count] |= (m_colors[m_count]<<4);
			m_count++;

			if (m_count == 3)
			{
//              logerror("RAMDAC: color %d = %02x %02x %02x %s\n", m_clutoffs, m_colors[0], m_colors[1], m_colors[2], machine().describe_context());
				m_palette[m_clutoffs] = rgb_t(m_colors[2], m_colors[2], m_colors[2]);
				m_clutoffs++;
				if (m_clutoffs > 255)
				{
					m_clutoffs = 0;
				}
				m_count = 0;
			}
			break;

		case 0x8000:
			lower_slot_irq();
			m_vbl_disable = false;
			break;

		case 0x8001:
			m_vbl_disable = true;
			break;
	}
}

uint32_t nubus_wsportrait_device::wsportrait_r(offs_t offset, uint32_t mem_mask)
{
//  printf("wsportrait: Read @ %x, mask %08x\n", offset, mem_mask);

	/*
	  monitor types

	  0x0 = invalid
	  0x2 = invalid
	  0x4 = color: 640x870 1bpp, 640x480 2bpp and 4bpp
	  0x6 = 1bpp 640x384? and sets weird mode controls
	  0x8 = really odd (bitplaned?)
	  0xa = invalid
	  0xc = 640x480 grayscale
	  0xe = same as 0x6
	*/

	switch (offset)
	{
		case 0x4004:
			m_toggle ^= 0x00010000;
			return m_toggle | 0xfffc0000;   // bit 0 = vbl status, bits 1-3 = monitor type
	}
	return 0;
}

void nubus_wsportrait_device::vram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data ^= 0xffffffff;
	COMBINE_DATA(&m_vram[offset]);
}

uint32_t nubus_wsportrait_device::vram_r(offs_t offset, uint32_t mem_mask)
{
	return m_vram[offset] ^ 0xffffffff;
}
