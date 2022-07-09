// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Apple Macintosh II Video Card (630-0153) emulation

  Video ASIC is "TFB" 344-0001
  RAMDAC is Bt453: control at Fs09001C, data at Fs090018
  Fs08000x is general framebuffer control (video mode at 0)
  Fs0D0000 bit 0 is VBL status

***************************************************************************/

#include "emu.h"
#include "nubus_m2video.h"
#include "screen.h"

#include <algorithm>


#define M2VIDEO_SCREEN_NAME "m2video_screen"
#define M2VIDEO_ROM_REGION  "m2video_rom"

#define VRAM_SIZE   (0x80000)   // 512k max


ROM_START( m2video )
	ROM_REGION(0x1000, M2VIDEO_ROM_REGION, 0)
	ROM_LOAD( "342-0008-a.bin", 0x000000, 0x001000, CRC(bf50850d) SHA1(abe85d8a882bb2b8187a28bd6707fc2f5d77eedd) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(NUBUS_M2VIDEO, nubus_m2video_device, "nb_m2vc", "Apple Macintosh II Video Card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nubus_m2video_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, M2VIDEO_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(nubus_m2video_device::screen_update));
	screen.set_raw(25175000, 800, 0, 640, 525, 0, 480);
	screen.set_size(1024, 768);
	screen.set_visarea(0, 640-1, 0, 480-1);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nubus_m2video_device::device_rom_region() const
{
	return ROM_NAME( m2video );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_m2video_device - constructor
//-------------------------------------------------

nubus_m2video_device::nubus_m2video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nubus_m2video_device(mconfig, NUBUS_M2VIDEO, tag, owner, clock)
{
}

nubus_m2video_device::nubus_m2video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_nubus_card_interface(mconfig, *this),
	m_mode(0), m_vbl_disable(0), m_toggle(0), m_count(0), m_clutoffs(0), m_timer(nullptr)
{
	set_screen(*this, M2VIDEO_SCREEN_NAME);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_m2video_device::device_start()
{
	uint32_t slotspace;

	install_declaration_rom(M2VIDEO_ROM_REGION, true, true);

	slotspace = get_slotspace();

//  logerror("[m2video %p] slotspace = %x\n", this, slotspace);

	m_vram.resize(VRAM_SIZE / sizeof(uint32_t));

	nubus().install_device(slotspace, slotspace+VRAM_SIZE-1, read32s_delegate(*this, FUNC(nubus_m2video_device::vram_r)), write32s_delegate(*this, FUNC(nubus_m2video_device::vram_w)));
	nubus().install_device(slotspace+0x900000, slotspace+VRAM_SIZE-1+0x900000, read32s_delegate(*this, FUNC(nubus_m2video_device::vram_r)), write32s_delegate(*this, FUNC(nubus_m2video_device::vram_w)));
	nubus().install_device(slotspace+0x80000, slotspace+0xeffff, read32s_delegate(*this, FUNC(nubus_m2video_device::m2video_r)), write32s_delegate(*this, FUNC(nubus_m2video_device::m2video_w)));

	m_timer = timer_alloc(FUNC(nubus_m2video_device::vbl_tick), this);
	m_timer->adjust(screen().time_until_pos(479, 0), 0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_m2video_device::device_reset()
{
	m_count = 0;
	m_clutoffs = 0;
	m_vbl_disable = 1;
	m_mode = 0;
	std::fill(m_vram.begin(), m_vram.end(), 0);
	memset(m_palette, 0, sizeof(m_palette));

	m_palette[0] = rgb_t(255, 255, 255);
	m_palette[0x80] = rgb_t(0, 0, 0);
}


TIMER_CALLBACK_MEMBER(nubus_m2video_device::vbl_tick)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	m_timer->adjust(screen().time_until_pos(479, 0), 0);
}

/***************************************************************************

  "TFB" section

***************************************************************************/

uint32_t nubus_m2video_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	auto const vram8 = util::big_endian_cast<uint8_t const>(&m_vram[0]) + 0x20;

	switch (m_mode)
	{
		case 0: // 1 bpp?
			for (int y = 0; y < 480; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/8; x++)
				{
					uint8_t const pixels = vram8[(y * 128) + x];

					*scanline++ = m_palette[(pixels&0x80)];
					*scanline++ = m_palette[((pixels<<1)&0x80)];
					*scanline++ = m_palette[((pixels<<2)&0x80)];
					*scanline++ = m_palette[((pixels<<3)&0x80)];
					*scanline++ = m_palette[((pixels<<4)&0x80)];
					*scanline++ = m_palette[((pixels<<5)&0x80)];
					*scanline++ = m_palette[((pixels<<6)&0x80)];
					*scanline++ = m_palette[((pixels<<7)&0x80)];
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

					*scanline++ = m_palette[(pixels&0xc0)];
					*scanline++ = m_palette[((pixels<<2)&0xc0)];
					*scanline++ = m_palette[((pixels<<4)&0xc0)];
					*scanline++ = m_palette[((pixels<<6)&0xc0)];
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

					*scanline++ = m_palette[(pixels&0xf0)];
					*scanline++ = m_palette[((pixels&0x0f)<<4)];
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

		default:
			fatalerror("m2video: unknown video mode %d\n", m_mode);
	}
	return 0;
}

void nubus_m2video_device::m2video_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data ^= 0xffffffff;
	switch (offset)
	{
		case 0:         // mode
			switch (data & 0xff000000)
			{
				case 0x20000000:
					m_mode = 0;
					break;

				case 0x40000000:
					m_mode = 1;
					break;

				case 0x80000000:
					m_mode = 2;
					break;

				case 0x00000000:
					m_mode = 3;
					break;
			}
			break;

		case 0x4007:    // DAC control
//          logerror("%08x to DAC control %s\n", data, machine().describe_context());
			m_clutoffs = (data>>24)&0xff;
			break;

		case 0x4006: // DAC data
			m_colors[m_count++] = (data>>24) & 0xff;

			if (m_count == 3)
			{
//                logerror("RAMDAC: color %02x = %02x %02x %02x %s\n", m_clutoffs, m_colors[0], m_colors[1], m_colors[2], machine().describe_context() );
				m_palette[m_clutoffs] = rgb_t(m_colors[0], m_colors[1], m_colors[2]);
				m_clutoffs++;
				if (m_clutoffs > 255)
				{
					m_clutoffs = 0;
				}
				m_count = 0;
			}
			break;

		case 0x8000:    // enable and ack VBL
			m_vbl_disable = 0;
			lower_slot_irq();
			break;

		case 0x8001:    // disable VBL
			m_vbl_disable = 1;
			break;

		default:
//          logerror("m2video_w: %08x @ %x, mask %08x %s\n", data, offset, mem_mask, machine().describe_context());
			break;
	}
}

uint32_t nubus_m2video_device::m2video_r(offs_t offset, uint32_t mem_mask)
{
	if (offset == 0x50000/4)    // bit 0 is VBL status
	{
		m_toggle ^= 1;
		return m_toggle;
	}
	else
	{
//      logerror("m2video_r: @ %x, mask %08x %s\n", offset, mem_mask, machine().describe_context());
	}

	return 0;
}

void nubus_m2video_device::vram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data ^= 0xffffffff;
	COMBINE_DATA(&m_vram[offset]);
}

uint32_t nubus_m2video_device::vram_r(offs_t offset, uint32_t mem_mask)
{
	return m_vram[offset] ^ 0xffffffff;
}
