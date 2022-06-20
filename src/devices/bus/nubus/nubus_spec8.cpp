// license:BSD-3-Clause
// copyright-holders:R. Belmont, Vas Crabb
/***************************************************************************

  SuperMac Spectrum/8 Series III video card

  There is no sign of acceleration or blitting in any mode, and the
  acceleration code from the Spectrum PDQ ROM is absent on this one.

  On first boot or with clean PRAM, the firmware will cycle through video
  modes and prompt you to press space when the desired mode is active (on
  the monitors available at the time, only one mode would produce a stable
  image).  If you want to change video later, hold Option as soon as the
  machine starts.

  The CRTC has 16-bit registers with the bytes written at separate
  addresses offset by 2.  The most significant byte is at the higher
  address.

  The video timing control registers are counter preload values, so they
  effectively function as (65'546 - x) to get the actual number of pixels
  cells or lines.  They're represented as signed 16-bit integers as that
  lets you see the negated value.  The horizontal timing register values
  are in units of four pixels.  The sync pulses are treated as being at
  the start of the line/frame in the configuration registers.

***************************************************************************/

#include "emu.h"
#include "nubus_spec8.h"
#include "screen.h"

#include <algorithm>

//#define VERBOSE 1
#include "logmacro.h"


#define SPEC8S3_SCREEN_NAME "spec8s3_screen"
#define SPEC8S3_ROM_REGION  "spec8s3_rom"

#define VRAM_SIZE   (0xc0000)   // 768k of VRAM for 1024x768 @ 8 bit


ROM_START( spec8s3 )
	ROM_DEFAULT_BIOS("ver13")
	ROM_SYSTEM_BIOS( 0, "ver12", "Ver. 1.2 (1990)" )
	ROM_SYSTEM_BIOS( 1, "ver13", "Ver. 1.3 (1993)" )

	ROM_REGION(0x8000, SPEC8S3_ROM_REGION, 0)
	ROMX_LOAD( "1003067-0001d.11b.bin", 0x000000, 0x008000, CRC(12188e2b) SHA1(6552d40364eae99b449842a79843d8c0114c4c70), ROM_BIOS(0) ) // "1003067-0001D Spec/8 Ser III // Ver. 1.2 (C)Copyright 1990 // SuperMac Technology // All Rights Reserved" 27c256 @11B
	ROMX_LOAD( "1003067-0001e.11b.bin", 0x000000, 0x008000, CRC(39fab193) SHA1(124c9847bf07733d131c977c4395cfbbb6470973), ROM_BIOS(1) ) // "1003067-0001E Spec/8 Ser III // Ver. 1.3 (C)Copyright 1993 // SuperMac Technology // All Rights Reserved" NMC27C256Q @11B
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(NUBUS_SPEC8S3, nubus_spec8s3_device, "nb_sp8s3", "SuperMac Spectrum/8 Series III video card")


//-------------------------------------------------
//  crtc_w - write CRTC register byte
//-------------------------------------------------

void nubus_spec8s3_device::crtc_w(int16_t &param, uint32_t offset, uint32_t data)
{
	param &= 0xff << (BIT(offset, 1) ? 0 : 8);
	param |= (data & 0xff) << (BIT(offset, 1) ? 8 : 0);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nubus_spec8s3_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, SPEC8S3_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(nubus_spec8s3_device::screen_update));
	screen.set_raw(64_MHz_XTAL, 332*4, 64*4, 320*4, 804, 33, 801);

	PALETTE(config, m_palette).set_entries(256);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nubus_spec8s3_device::device_rom_region() const
{
	return ROM_NAME( spec8s3 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_spec8s3_device - constructor
//-------------------------------------------------

nubus_spec8s3_device::nubus_spec8s3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nubus_spec8s3_device(mconfig, NUBUS_SPEC8S3, tag, owner, clock)
{
}

nubus_spec8s3_device::nubus_spec8s3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_nubus_card_interface(mconfig, *this),
	m_palette(*this, "palette"),
	m_timer(nullptr),
	m_mode(0), m_vbl_disable(0),
	m_count(0), m_clutoffs(0),
	m_vbl_pending(false), m_parameter(0)
{
	set_screen(*this, SPEC8S3_SCREEN_NAME);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_spec8s3_device::device_start()
{
	install_declaration_rom(SPEC8S3_ROM_REGION);

	uint32_t const slotspace = get_slotspace();
	LOG("[SPEC8S3 %p] slotspace = %x\n", this, slotspace);

	m_vram.resize(VRAM_SIZE / sizeof(uint32_t));
	nubus().install_device(slotspace, slotspace+VRAM_SIZE-1, read32s_delegate(*this, FUNC(nubus_spec8s3_device::vram_r)), write32s_delegate(*this, FUNC(nubus_spec8s3_device::vram_w)));
	nubus().install_device(slotspace+0x900000, slotspace+VRAM_SIZE-1+0x900000, read32s_delegate(*this, FUNC(nubus_spec8s3_device::vram_r)), write32s_delegate(*this, FUNC(nubus_spec8s3_device::vram_w)));
	nubus().install_device(slotspace+0xd0000, slotspace+0xfffff, read32s_delegate(*this, FUNC(nubus_spec8s3_device::spec8s3_r)), write32s_delegate(*this, FUNC(nubus_spec8s3_device::spec8s3_w)));

	m_timer = timer_alloc(FUNC(nubus_spec8s3_device::vbl_tick), this);

	save_item(NAME(m_vram));
	save_item(NAME(m_mode));
	save_item(NAME(m_vbl_disable));
	save_item(NAME(m_palette_val));
	save_item(NAME(m_colors));
	save_item(NAME(m_count));
	save_item(NAME(m_clutoffs));
	save_item(NAME(m_hsync));
	save_item(NAME(m_hstart));
	save_item(NAME(m_hend));
	save_item(NAME(m_htotal));
	save_item(NAME(m_vsync));
	save_item(NAME(m_vstart));
	save_item(NAME(m_vend));
	save_item(NAME(m_vtotal));
	save_item(NAME(m_interlace));
	save_item(NAME(m_vbl_pending));
	save_item(NAME(m_parameter));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_spec8s3_device::device_reset()
{
	std::fill(m_vram.begin(), m_vram.end(), 0);
	m_mode = 0;
	m_vbl_disable = 1;
	std::fill(std::begin(m_palette_val), std::end(m_palette_val), 0);
	std::fill(std::begin(m_colors), std::end(m_colors), 0);
	m_count = 0;
	m_clutoffs = 0;
	m_hsync = -24;
	m_hstart = -64;
	m_hend = -320;
	m_htotal = -332;
	m_vsync = -3;
	m_vstart = -33;
	m_vend = -801;
	m_vtotal = -804;
	m_interlace = false;
	m_vbl_pending = false;
	m_parameter = 0;

	m_palette_val[0] = rgb_t(255, 255, 255);
	m_palette_val[1] = rgb_t(0, 0, 0);

	update_crtc();
}


TIMER_CALLBACK_MEMBER(nubus_spec8s3_device::vbl_tick)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
		m_vbl_pending = true;
	}

	m_timer->adjust(screen().time_until_pos((m_vsync - m_vend) * (m_interlace ? 2 : 1), 0));
}

/***************************************************************************

  Spectrum/8 Series III

***************************************************************************/

void nubus_spec8s3_device::update_crtc()
{
	// FIXME: Blatant hack - I have no idea how the clock source is configured
	// (there's space for five clock modules on the board)
	// Interlace mode configuration is also complicated, so it's hacked here
	// The user-supplied clock module should be a machine configuration option
	uint32_t clock = 64'000'000; // supplied - 1024x768 60Hz
	m_interlace = false;
	switch (m_vtotal)
	{
	case -803:
		clock = 80'000'000; // supplied - 1024x768 75Hz
		break;
	case -654:
		clock = 55'000'000; // supplied with newer revisions - 832x625 75Hz
		break;
	case -525:
		clock = 30'240'000; // supplied - 640x480 67Hz
		break;
	case -411:
		clock = 15'821'851; // user-supplied - 512x384 60.15Hz FIXME: what's the real recommended clock for this?
		break;
	case -262:
		clock = 14'318'180; // user-supplied - 640x480i NTSC
		m_interlace = true;
		break;
	}

	// for some reason you temporarily get invalid screen parameters - ignore them
	if ((m_hstart < m_hsync) && (m_hend < m_hstart) && (m_htotal < m_hend) && (m_vstart < m_vsync) && (m_vend < m_vstart) && (m_vtotal < m_vend))
	{
		screen().configure(
				-4 * m_htotal,
				-m_vtotal * (m_interlace ? 2 : 1),
				rectangle(
					4 * (m_hsync - m_hstart),
					(4 * (m_hsync - m_hend)) - 1,
					(m_vsync - m_vstart) * (m_interlace ? 2 : 1),
					((m_vsync - m_vend) * (m_interlace ? 2 : 1)) - 1),
				attotime::from_ticks(-4 * m_htotal * -m_vtotal, clock).attoseconds());

		m_timer->adjust(screen().time_until_pos((m_vsync - m_vend) * (m_interlace ? 2 : 1), 0));
	}
	else
	{
		LOG("Ignoring invalid CRTC parameters (%d %d %d %d) (%d %d %d %d)\n",
				m_hsync, m_hstart, m_hend, m_htotal,
				m_vsync, m_vstart, m_vend, m_vtotal);
	}
}

uint32_t nubus_spec8s3_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	auto const vram8 = util::big_endian_cast<uint8_t const>(&m_vram[0]) + 0x400;

	int const hstart = 4 * (m_hsync - m_hstart);
	int const width = 4 * (m_hstart - m_hend);
	int const vstart = (m_vsync - m_vstart) * (m_interlace ? 2 : 1);
	int const vend = (m_vsync - m_vend) * (m_interlace ? 2 : 1);

	switch (m_mode)
	{
		case 0: // 1 bpp
			for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
			{
				uint32_t *scanline = &bitmap.pix(y, hstart);
				if ((y >= vstart) && (y < vend))
				{
					for (int x = 0; x < (width / 8); x++)
					{
						uint8_t const pixels = vram8[((y - vstart) * 512) + x];

						*scanline++ = m_palette_val[(pixels << 0) & 0x80];
						*scanline++ = m_palette_val[(pixels << 1) & 0x80];
						*scanline++ = m_palette_val[(pixels << 2) & 0x80];
						*scanline++ = m_palette_val[(pixels << 3) & 0x80];
						*scanline++ = m_palette_val[(pixels << 4) & 0x80];
						*scanline++ = m_palette_val[(pixels << 5) & 0x80];
						*scanline++ = m_palette_val[(pixels << 6) & 0x80];
						*scanline++ = m_palette_val[(pixels << 7) & 0x80];
					}
				}
				else
				{
					std::fill_n(scanline, width, 0);
				}
			}
			break;

		case 1: // 2 bpp
			for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
			{
				uint32_t *scanline = &bitmap.pix(y, hstart);
				if ((y >= vstart) && (y < vend))
				{
					for (int x = 0; x < (width / 4); x++)
					{
						uint8_t const pixels = vram8[((y - vstart) * 512) + x];

						*scanline++ = m_palette_val[(pixels << 0) & 0xc0];
						*scanline++ = m_palette_val[(pixels << 2) & 0xc0];
						*scanline++ = m_palette_val[(pixels << 4) & 0xc0];
						*scanline++ = m_palette_val[(pixels << 6) & 0xc0];
					}
				}
				else
				{
					std::fill_n(scanline, width, 0);
				}
			}
			break;

		case 2: // 4 bpp
			for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
			{
				uint32_t *scanline = &bitmap.pix(y, hstart);
				if ((y >= vstart) && (y < vend))
				{
					for (int x = 0; x < (width / 2); x++)
					{
						uint8_t const pixels = vram8[((y - vstart) * 512) + x];

						*scanline++ = m_palette_val[(pixels << 0) & 0xf0];
						*scanline++ = m_palette_val[(pixels << 4) & 0xf0];
					}
				}
				else
				{
					std::fill_n(scanline, width, 0);
				}
			}
			break;

		case 3: // 8 bpp
			for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
			{
				uint32_t *scanline = &bitmap.pix(y, hstart);
				if ((y >= vstart) && (y < vend))
				{
					for (int x = 0; x < width; x++)
					{
						uint8_t const pixels = vram8[((y - vstart) * 1024) + x];
						*scanline++ = m_palette_val[pixels];
					}
				}
				else
				{
					std::fill_n(scanline, width, 0);
				}
			}
			break;

		default:
			fatalerror("spec8s3: unknown video mode %d\n", m_mode);
	}
	return 0;
}

void nubus_spec8s3_device::spec8s3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0x3804:
		case 0x3806:
			crtc_w(m_hsync, offset, data);
			if (!BIT(offset, 1))
				update_crtc();
			break;

		case 0x3808:
		case 0x380a:
			crtc_w(m_hstart, offset, data);
			if (!BIT(offset, 1))
				update_crtc();
			break;

		case 0x380c:
		case 0x380e:
			crtc_w(m_hend, offset, data);
			if (!BIT(offset, 1))
				update_crtc();
			break;

		case 0x3810:
		case 0x3812:
			crtc_w(m_htotal, offset, data);
			if (!BIT(offset, 1))
				update_crtc();
			break;

		case 0x3818:
		case 0x381a:
			crtc_w(m_vsync, offset, data);
			if (!BIT(offset, 1))
				update_crtc();
			break;

		case 0x381c:
		case 0x381e:
			crtc_w(m_vstart, offset, data);
			if (!BIT(offset, 1))
				update_crtc();
			break;

		case 0x3824:
		case 0x3826:
			crtc_w(m_vend, offset, data);
			if (!BIT(offset, 1))
				update_crtc();
			break;

		case 0x3828:
		case 0x382a:
			crtc_w(m_vtotal, offset, data);
			if (!BIT(offset, 1))
				update_crtc();
			break;

		case 0x385c:    // IRQ enable
			if (data & 0x10)
			{
				m_vbl_disable = 1;
				lower_slot_irq();
				m_vbl_pending = false;
			}
			else
			{
				m_vbl_disable = 0;
			}
			break;

		case 0x385e:
			break;

		case 0x386e:
			break;

		case 0x3a00:
			m_clutoffs = (data & 0xff) ^ 0xff;
			break;

		case 0x3a01:
			LOG("%08x to color (%08x invert)\n", data, data ^ 0xffffffff);
			m_colors[m_count++] = (data & 0xff) ^ 0xff;

			if (m_count == 3)
			{
				const int actual_color = bitswap<8>(m_clutoffs, 0, 1, 2, 3, 4, 5, 6, 7);

				LOG("RAMDAC: color %d = %02x %02x %02x %s\n", actual_color, m_colors[0], m_colors[1], m_colors[2], machine().describe_context());
				m_palette->set_pen_color(actual_color, rgb_t(m_colors[0], m_colors[1], m_colors[2]));
				m_palette_val[actual_color] = rgb_t(m_colors[0], m_colors[1], m_colors[2]);
				m_clutoffs = (m_clutoffs + 1) & 0xff;
				m_count = 0;
			}
			break;

		case 0x3c00:
			if ((m_parameter == 2) && (data != 0xffffffff))
			{
				data &= 0xff;
				LOG("%x to mode\n", data);
				switch (data)
				{
					case 0x5f:
						m_mode = 0;
						break;

					case 0x5e:
						m_mode = 1;
						break;

					case 0x5d:
						m_mode = 2;
						break;

					case 0x5c:
						m_mode = 3;
						break;
				}
			}
			m_parameter++;
			break;

		case 0x3e02:
			if (data == 1)
			{
				m_parameter = 0;
			}
			break;

		default:
//          if (offset >= 0x3800) logerror("spec8s3_w: %08x @ %x (mask %08x  %s)\n", data, offset, mem_mask, machine().describe_context());
			break;
	}
}

uint32_t nubus_spec8s3_device::spec8s3_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0x3826:
		case 0x382e:
			return 0xff;

		case 0x3824:
		case 0x382c:
			return (0xa^0xffffffff);

		case 0x385c:
			if (m_vbl_pending)
			{
				return 0x8;
			}
			return 0;

		case 0x385e:
			return 0;

		default:
//          if (offset >= 0x3800) logerror("spec8s3_r: @ %x (mask %08x  %s)\n", offset, mem_mask, machine().describe_context());
			break;
	}
	return 0;
}

void nubus_spec8s3_device::vram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data ^= 0xffffffff;
	COMBINE_DATA(&m_vram[offset]);
}

uint32_t nubus_spec8s3_device::vram_r(offs_t offset, uint32_t mem_mask)
{
	return m_vram[offset] ^ 0xffffffff;
}
