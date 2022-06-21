// license:BSD-3-Clause
// copyright-holders:R. Belmont, Vas Crabb
/***************************************************************************

  SuperMac Spectrum/8 Series III video card

  This is a high-resolution video card supporting 1-bit, 2-bit, 4-bit and
  8-bit modes with 24-bit palette entires.  Virtual desktop panning and
  two-times zoom are supported in hardware.  No accelerated drawing
  features are present.

  On first boot or with clean PRAM, the firmware will cycle through video
  modes and prompt you to press space when the desired mode is active (on
  the monitors available at the time, only one mode would produce a stable
  image).  If you want to change video mode later, hold Option as soon as
  the machine starts.  The modes offered depend on the user-supplied
  oscillator.

  The CRTC has 16-bit registers with the bytes written at separate
  addresses offset by 2.  The most significant byte is at the higher
  address.

  The video timing control registers are counter preload values, so they
  effectively function as (65'536 - x) to get the actual number of pixel
  cells or lines.  They're represented as signed 16-bit integers as that
  lets you see the negated value.  The horizontal timing register values
  are in units of four pixels.  The sync pulses are treated as being at
  the start of the line/frame in the configuration registers.

  TODO:
  * The user-supplied oscillator measurement gets incorrect results.
  * Interlaced modes are not understood.
  * There are lines of garbage at the bottom of the screen in some high-
    resolution modes (bottom of the virtual desktop if virtual desktop is
    enabled).

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


static INPUT_PORTS_START( spec8s3 )
	PORT_START("USEROSC")
	PORT_CONFNAME(0x07, 0x01, "Oscillator Y1")
	PORT_CONFSETTING(   0x00, "14.32 MHz (NTSC Underscan)")
	PORT_CONFSETTING(   0x01, "15.67 MHz (Apple 12\")")
	PORT_CONFSETTING(   0x02, "17.73 MHz (PAL Underscan)")
	PORT_CONFSETTING(   0x03, "57.28 MHz (Apple 15\" Portrait, Apple 16\")")
INPUT_PORTS_END

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
	screen.set_raw(80.000_MHz_XTAL, 332*4, 64*4, 320*4, 804, 33, 801);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nubus_spec8s3_device::device_rom_region() const
{
	return ROM_NAME( spec8s3 );
}

//-------------------------------------------------
//  input_ports - device-specific I/O ports
//-------------------------------------------------

ioport_constructor nubus_spec8s3_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( spec8s3 );
}

//-------------------------------------------------
//  palette_entries - entries in color palette
//-------------------------------------------------

uint32_t nubus_spec8s3_device::palette_entries() const
{
	return 256;
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
	device_nubus_card_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	device_palette_interface(mconfig, *this),
	m_userosc(*this, "USEROSC"),
	m_timer(nullptr),
	m_mode(0), m_vbl_disable(0),
	m_count(0), m_clutoffs(0),
	m_vbl_pending(false)
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
	save_item(NAME(m_colors));
	save_item(NAME(m_count));
	save_item(NAME(m_clutoffs));
	save_item(NAME(m_osc));
	save_item(NAME(m_hsync));
	save_item(NAME(m_hstart));
	save_item(NAME(m_hend));
	save_item(NAME(m_htotal));
	save_item(NAME(m_vsync));
	save_item(NAME(m_vstart));
	save_item(NAME(m_vend));
	save_item(NAME(m_vtotal));
	save_item(NAME(m_interlace));
	save_item(NAME(m_hpan));
	save_item(NAME(m_vpan));
	save_item(NAME(m_zoom));
	save_item(NAME(m_param_bit));
	save_item(NAME(m_param_sel));
	save_item(NAME(m_param_val));
	save_item(NAME(m_vbl_pending));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_spec8s3_device::device_reset()
{
	std::fill(m_vram.begin(), m_vram.end(), 0);
	m_mode = 0;
	m_vbl_disable = 1;
	std::fill(std::begin(m_colors), std::end(m_colors), 0);
	m_count = 0;
	m_clutoffs = 0;
	m_osc = 0;
	m_hsync = -24;
	m_hstart = -64;
	m_hend = -320;
	m_htotal = -332;
	m_vsync = -3;
	m_vstart = -33;
	m_vend = -801;
	m_vtotal = -804;
	m_interlace = false;
	m_hpan = 0;
	m_vpan = 0;
	m_zoom = 0;
	m_param_bit = 0;
	m_param_sel = 0;
	m_param_val = 0;
	m_vbl_pending = false;

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
	XTAL oscillator = 80.000_MHz_XTAL; // no default constructor
	switch (m_osc)
	{
	case 0: // Y5
		oscillator = 80.000_MHz_XTAL;
		break;
	case 1: // Y1
		switch (m_userosc->read())
		{
		case 0:
			oscillator = 14.318'18_MHz_XTAL;
			break;
		case 1:
			oscillator = 15.67_MHz_XTAL;
			break;
		case 2:
			oscillator = 17.73_MHz_XTAL;
			break;
		case 3:
			oscillator = 57.27_MHz_XTAL;
			break;
		default:
			throw emu_fatalerror("%s: spec8s3: invalid user oscillator selection %d\n", tag(), m_userosc->read());
		}
		break;
	case 2: // Y2
		oscillator = 55.000_MHz_XTAL;
		break;
	case 3: // Y3
		oscillator = 30.240_MHz_XTAL;
		break;
	case 4: // Y4
		oscillator = 64.000_MHz_XTAL;
		break;
	default:
		throw emu_fatalerror("%s: spec8s3: invalid oscillator selection %d\n", tag(), m_osc);
	}

	// FIXME: blatant hack - I don't know how interlace mode is configured
	m_interlace = m_vtotal > -320;

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
				attotime::from_ticks(-4 * m_htotal * -m_vtotal, oscillator).attoseconds());

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
	auto const screenbase = util::big_endian_cast<uint8_t const>(&m_vram[0]) + (m_vpan * 2048) + 0x400;

	int const hstart = 4 * (m_hsync - m_hstart);
	int const width = 4 * (m_hstart - m_hend);
	int const pixels = width >> (m_zoom ? 1 : 0);
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
					auto const rowbase = screenbase + (((y - vstart) >> (m_zoom ? 1 : 0)) * 512);
					for (int x = 0; x < (pixels / 2); x++)
					{
						uint8_t const bits = rowbase[(x + m_hpan) / 4];
						*scanline++ = pen_color((bits << ((((x + m_hpan) & 0x03) << 1) + 0)) & 0x80);
						if (m_zoom)
							*scanline++ = pen_color((bits << ((((x + m_hpan) & 0x03) << 1) + 0)) & 0x80);
						*scanline++ = pen_color((bits << ((((x + m_hpan) & 0x03) << 1) + 1)) & 0x80);
						if (m_zoom)
							*scanline++ = pen_color((bits << ((((x + m_hpan) & 0x03) << 1) + 1)) & 0x80);
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
					auto const rowbase = screenbase + (((y - vstart) >> (m_zoom ? 1 : 0)) * 512);
					for (int x = 0; x < pixels; x++)
					{
						uint8_t const bits = rowbase[(x + m_hpan) / 4];
						*scanline++ = pen_color((bits << (((x + m_hpan) & 0x03) << 1)) & 0xc0);
						if (m_zoom)
							*scanline++ = pen_color((bits << (((x + m_hpan) & 0x03) << 1)) & 0xc0);
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
					auto const rowbase = screenbase + (((y - vstart) >> (m_zoom ? 1 : 0)) * 512);
					for (int x = 0; x < pixels; x++)
					{
						uint8_t const bits = rowbase[(x + (m_hpan / 2)) / 2];
						*scanline++ = pen_color((bits << (((x + (m_hpan / 2)) & 0x01) << 2)) & 0xf0);
						if (m_zoom)
							*scanline++ = pen_color((bits << (((x + (m_hpan / 2)) & 0x01) << 2)) & 0xf0);
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
					auto const rowbase = screenbase + (((y - vstart) >> (m_zoom ? 1 : 0)) * 1024);
					for (int x = 0; x < pixels; x++)
					{
						uint8_t const bits = rowbase[x + (m_hpan / 4)];
						*scanline++ = pen_color(bits);
						if (m_zoom)
							*scanline++ = pen_color(bits);
					}
				}
				else
				{
					std::fill_n(scanline, width, 0);
				}
			}
			break;

		default:
			throw emu_fatalerror("%s: spec8s3: unknown video mode %d\n", tag(), m_mode);
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
			update_crtc();
			break;

		case 0x3808:
		case 0x380a:
			crtc_w(m_hstart, offset, data);
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
			update_crtc();
			break;

		case 0x3818:
		case 0x381a:
			crtc_w(m_vsync, offset, data);
			update_crtc();
			break;

		case 0x381c:
		case 0x381e:
			crtc_w(m_vstart, offset, data);
			update_crtc();
			break;

		case 0x3824:
		case 0x3826:
			crtc_w(m_vend, offset, data);
			update_crtc();
			break;

		case 0x3828:
		case 0x382a:
			crtc_w(m_vtotal, offset, data);
			update_crtc();
			break;

		case 0x3844:
			m_vpan = (m_vpan & 0x0300) | (~data & 0xff);
			break;

		case 0x3846:
			// bits 2-7 of this are important - they're read and written back
			m_vpan = (m_vpan & 0x00ff) | ((~data & 0x03) << 8);
			m_zoom = BIT(~data, 4);
			break;

		case 0x3848:
			m_hpan = (m_hpan & 0x000f) | ((~data & 0xff) << 4);
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
			m_clutoffs = ~data & 0xff;
			break;

		case 0x3a01:
			LOG("%08x to color (%08x invert)\n", data, data ^ 0xffffffff);
			m_colors[m_count++] = ~data & 0xff;

			if (m_count == 3)
			{
				const int actual_color = bitswap<8>(m_clutoffs, 0, 1, 2, 3, 4, 5, 6, 7);

				LOG("RAMDAC: color %d = %02x %02x %02x %s\n", actual_color, m_colors[0], m_colors[1], m_colors[2], machine().describe_context());
				set_pen_color(actual_color, rgb_t(m_colors[0], m_colors[1], m_colors[2]));
				m_clutoffs = (m_clutoffs + 1) & 0xff;
				m_count = 0;
			}
			break;

		case 0x3c00:
			if (m_param_bit < 2)
			{
				// register select
				uint8_t const mask = ~(1 << m_param_bit);
				uint8_t const bit = BIT(~data, 0) << m_param_bit;
				m_param_sel = (m_param_sel & mask) | bit;
			}
			else if (m_param_bit < 10)
			{
				uint8_t const mask = ~(1 << (m_param_bit - 2));
				uint8_t const bit = BIT(~data, 0) << (m_param_bit - 2);
				m_param_val = (m_param_val & mask) | bit;
				if (m_param_bit == 9)
				{
					switch (m_param_sel)
					{
					case 0:
						// bit depth in low bits, other bits unknown
						LOG("%x to mode\n", m_param_val);
						m_mode = m_param_val & 0x03;
						break;
					case 1:
						// bits 0-2 and 7 are unknown
						LOG("%x to hpan\n", m_param_val);
						m_hpan = (m_hpan & 0x07f0) | ((m_param_val >> 3) & 0x0f);
						break;
					default:
						LOG("%x to param %x\n", m_param_val, m_param_sel);
					}
				}
			}
			m_param_bit++;
			break;

		case 0x3e02:
			// This has something to do with setting up for writing to 3c00.
			// Sequence is:
			// * 0 -> 3e02
			// * 1 -> 3c00
			// * 1 -> 3e02
			// * shift ten bits of inverted data out via 3c00
			if (data == 1)
			{
				m_param_bit = 0;
			}
			break;

		case 0x3e05:
		case 0x3e06:
		case 0x3e07:
			m_osc = (m_osc & ~(1 << (offset - 0x3e05))) | (BIT(~data, 0) << (offset - 0x3e05));
			// only update when the high bit is set to avoid bad intermediate values
			if (offset == 0x3e07)
				update_crtc();
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
		case 0x382c:
		case 0x382e:
			{
				/*
				 * FIXME: Something goes wrong with user-supplied oscillator measurement.
				 * Set breakpoint at 0x2d1e on Mac II with card in slot 9 to catch
				 * measurement.  See the measured value in D0.b when PC= 0x2d6c.
				 *
				 * Expected:
				 *    2- 5    57.28 MHz
				 *    6-19    bad
				 *   20-22    14.32 MHz
				 *   23-24    15.67 MHz
				 *   25-27    17.73 MHz
				 *   27-      bad
				 *
				 * Measured:
				 *   14.32 MHz  15
				 *   15.67 MHz  19
				 *   17.73 MHz  25
				 *   57.28 MHz   9
				 */
				int vpos;
				if (m_interlace)
				{
					// hack for interlace modes because screen_device doesn't support them
					vpos = screen().vpos() - (2 * m_vsync);
					if (vpos >= (-2 * m_vtotal))
						vpos += 2 * m_vtotal;
					vpos = vpos % -m_vtotal;
				}
				else
				{
					vpos = screen().vpos() - m_vsync;
					if (vpos >= -m_vtotal)
						vpos += m_vtotal;
				}
				return ~((vpos >> (BIT(offset, 1) ? 8 : 0)) & 0xff);
			}

		case 0x3826:
			return 0xff;

		case 0x3824:
			return (0xa^0xffffffff);

		case 0x3846:
			// it expects at least bits 2-7 will read back what was written
			// only returning emulated feature fields for now
			return ~(((m_zoom & 0x01) << 4) | ((m_vpan >> 8) & 0x03));

		case 0x3848:
			// it expects at least bit 7 will read back what was written
			return ~((m_hpan >> 4) & 0xff);

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
