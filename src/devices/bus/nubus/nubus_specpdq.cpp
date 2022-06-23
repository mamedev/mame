// license:BSD-3-Clause
// copyright-holders:R. Belmont, Vas Crabb
/***************************************************************************

  SuperMac Spectrum PDQ video card

  Accelerated only in 256 color mode.

  blitter info:

  06 = ?
  07 = command - 002 = pattern fill, 100/101 = copy forward/backward
  08 = pattern offset - X in bits 0-2, Y in bits 3-6
  09 = VRAM destination * 4
  0a = VRAM source * 4
  0b = height (inclusive)
  0e = width (exclusive)

  Busy flag at Fs800000 (bit 8)

  There is 256 bytes of pattern RAM arranged as 32 pixels horizontally by
  8 vertically.

***************************************************************************/

#include "emu.h"
#include "nubus_specpdq.h"
#include "screen.h"

#include <algorithm>

//#define VERBOSE 1
#include "logmacro.h"


#define SPECPDQ_SCREEN_NAME "specpdq_screen"
#define SPECPDQ_ROM_REGION  "specpdq_rom"

#define VRAM_SIZE   (0x400000)


static INPUT_PORTS_START( specpdq )
	PORT_START("USEROSC")
	PORT_CONFNAME(0x07, 0x00, "Alternate oscillator")
	PORT_CONFSETTING(   0x00, "55.00 MHz (SuperMac 16\")")
	PORT_CONFSETTING(   0x01, "57.28 MHz (Apple Portrait)")
	PORT_CONFSETTING(   0x02, "64.00 MHz (SuperMac 19\" 60hz)")
INPUT_PORTS_END

ROM_START( specpdq )
	ROM_REGION(0x10000, SPECPDQ_ROM_REGION, 0)
	ROM_LOAD( "specpdq.bin",  0x000000, 0x010000, CRC(82a35f78) SHA1(9511c2df47140f4279196d3b8836b53429879dd9) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(NUBUS_SPECPDQ, nubus_specpdq_device, "nb_spdq", "SuperMac Spectrum PDQ video card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nubus_specpdq_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, SPECPDQ_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(nubus_specpdq_device::screen_update));
	screen.set_raw(100.000_MHz_XTAL, 1'456, 108, 1'260, 915, 39, 909);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nubus_specpdq_device::device_rom_region() const
{
	return ROM_NAME( specpdq );
}

//-------------------------------------------------
//  input_ports - device-specific I/O ports
//-------------------------------------------------

ioport_constructor nubus_specpdq_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( specpdq );
}

//-------------------------------------------------
//  palette_entries - entries in color palette
//-------------------------------------------------

uint32_t nubus_specpdq_device::palette_entries() const
{
	return 256;
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_specpdq_device - constructor
//-------------------------------------------------

nubus_specpdq_device::nubus_specpdq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nubus_specpdq_device(mconfig, NUBUS_SPECPDQ, tag, owner, clock)
{
}

nubus_specpdq_device::nubus_specpdq_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_nubus_card_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	device_palette_interface(mconfig, *this),
	m_userosc(*this, "USEROSC"),
	m_timer(nullptr),
	m_mode(0), m_vbl_disable(0),
	m_count(0), m_clutoffs(0),
	m_width(0), m_height(0), m_patofsx(0), m_patofsy(0), m_vram_addr(0), m_vram_src(0)
{
	set_screen(*this, SPECPDQ_SCREEN_NAME);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_specpdq_device::device_start()
{
	install_declaration_rom(SPECPDQ_ROM_REGION);

	uint32_t const slotspace = get_slotspace();
	LOG("[specpdq %p] slotspace = %x\n", this, slotspace);

	m_vram.resize(VRAM_SIZE / sizeof(uint32_t));
	nubus().install_device(slotspace, slotspace+VRAM_SIZE-1, read32s_delegate(*this, FUNC(nubus_specpdq_device::vram_r)), write32s_delegate(*this, FUNC(nubus_specpdq_device::vram_w)));
	nubus().install_device(slotspace+0x400000, slotspace+0xfbffff, read32s_delegate(*this, FUNC(nubus_specpdq_device::specpdq_r)), write32s_delegate(*this, FUNC(nubus_specpdq_device::specpdq_w)));

	m_timer = timer_alloc(FUNC(nubus_specpdq_device::vbl_tick), this);
	
	m_crtc.register_save(*this);
	m_shiftreg.register_save(*this);

	save_item(NAME(m_vram));
	save_item(NAME(m_mode));
	save_item(NAME(m_vbl_disable));
	save_item(NAME(m_colors));
	save_item(NAME(m_count));
	save_item(NAME(m_clutoffs));
	save_item(NAME(m_stride));
	save_item(NAME(m_vint));
	save_item(NAME(m_hdelay));
	save_item(NAME(m_osc));
	save_item(NAME(m_7xxxxx_regs));
	save_item(NAME(m_width));
	save_item(NAME(m_height));
	save_item(NAME(m_patofsx));
	save_item(NAME(m_patofsy));
	save_item(NAME(m_vram_addr));
	save_item(NAME(m_vram_src));
	save_item(NAME(m_pattern));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_specpdq_device::device_reset()
{
	m_crtc.reset();
	m_shiftreg.reset();

	std::fill(m_vram.begin(), m_vram.end(), 0);
	m_mode = 0;
	m_vbl_disable = 1;
	std::fill(std::begin(m_colors), std::end(m_colors), 0);
	m_count = 0;
	m_clutoffs = 0;
	m_stride = 512 / 4;
	m_vint = 0;
	m_hdelay = 0;
	m_osc = 0;

	std::fill(std::begin(m_7xxxxx_regs), std::end(m_7xxxxx_regs), 0);
	m_width = 0;
	m_height = 0;
	m_patofsx = 0;
	m_patofsy = 0;
	m_vram_addr = 0;
	m_vram_src = 0;
}


TIMER_CALLBACK_MEMBER(nubus_specpdq_device::vbl_tick)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	m_timer->adjust(
			screen().time_until_pos(
				m_vint + m_crtc.v_start() - 1,
				m_crtc.h_total(16) - m_crtc.h_sync(16)));
}

/***************************************************************************

  Spectrum 24 PDQ section

***************************************************************************/

void nubus_specpdq_device::update_crtc()
{
	XTAL oscillator = 100.000_MHz_XTAL; // no default constructor
	switch (m_osc)
	{
	case 0:
		switch (m_userosc->read())
		{
		case 0:
			oscillator = 55.00_MHz_XTAL;
			break;
		case 1:
			oscillator = 57.28_MHz_XTAL;
			break;
		case 2:
			oscillator = 64.00_MHz_XTAL;
			break;
		default:
			throw emu_fatalerror("%s: specpdq: invalid user oscillator selection %d\n", tag(), m_userosc->read());
		}
		break;
	case 1:
		oscillator = 80.00_MHz_XTAL;
		break;
	case 2:
		oscillator = 100.00_MHz_XTAL;
		break;
	case 3:
		oscillator = 30.24_MHz_XTAL;
		break;
	}

	// for some reason you temporarily get invalid screen parameters - ignore them
	if (m_crtc.valid(*this))
	{
		rectangle active(
				m_crtc.h_start(16) + (m_hdelay * 4),
				m_crtc.h_end(16) - 1,
				m_crtc.v_start(),
				m_crtc.v_end() - 1);

		// FIXME: work out how it actually configures the RAMDAC end-of-line
		// this is a horrible hack
		if (active.width() < 832)
			active.set_width(640);
		else if (active.width() < 1024)
			active.set_width(832);
		else if (active.width() < 1152)
			active.set_width(1024);
		else
			active.set_width(1152);

		screen().configure(
				m_crtc.h_total(16),
				m_crtc.v_total(),
				active,
				m_crtc.frame_time(16, oscillator));

		m_timer->adjust(
				screen().time_until_pos(
					m_vint + m_crtc.v_start() - 1,
					m_crtc.h_total(16) - m_crtc.h_sync(16)));
	}
}

uint32_t nubus_specpdq_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	auto const screenbase = util::big_endian_cast<uint8_t const>(&m_vram[0]) + 0x9000;

	int const hstart = m_crtc.h_start(16);
	int const width = m_crtc.h_active(16);
	int const vstart = m_crtc.v_start();
	int const vend = m_crtc.v_end();
	int const hdelay = m_hdelay * 4;

	switch (m_mode)
	{
		case 0: // 1 bpp
			for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
			{
				uint32_t *scanline = &bitmap.pix(y, hstart);
				if ((y >= vstart) && (y < vend))
				{
					scanline = std::fill_n(scanline, hdelay, 0);
					auto const rowbase = screenbase + ((y - vstart) * m_stride * 4);
					for (int x = 0; x < ((width - hdelay + 7) / 8); x++)
					{
						uint8_t const pixels = rowbase[x];

						*scanline++ = pen_color((pixels << 0) & 0x80);
						*scanline++ = pen_color((pixels << 1) & 0x80);
						*scanline++ = pen_color((pixels << 2) & 0x80);
						*scanline++ = pen_color((pixels << 3) & 0x80);
						*scanline++ = pen_color((pixels << 4) & 0x80);
						*scanline++ = pen_color((pixels << 5) & 0x80);
						*scanline++ = pen_color((pixels << 6) & 0x80);
						*scanline++ = pen_color((pixels << 7) & 0x80);
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
					scanline = std::fill_n(scanline, hdelay, 0);
					auto const rowbase = screenbase + ((y - vstart) * m_stride * 4);
					for (int x = 0; x < ((width - hdelay) / 4); x++)
					{
						uint8_t const pixels = rowbase[x];

						*scanline++ = pen_color((pixels << 0) & 0xc0);
						*scanline++ = pen_color((pixels << 2) & 0xc0);
						*scanline++ = pen_color((pixels << 4) & 0xc0);
						*scanline++ = pen_color((pixels << 6) & 0xc0);
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
					scanline = std::fill_n(scanline, hdelay, 0);
					auto const rowbase = screenbase + ((y - vstart) * m_stride * 4);
					for (int x = 0; x < ((width - hdelay) / 2); x++)
					{
						uint8_t const pixels = rowbase[x];

						*scanline++ = pen_color((pixels << 0) & 0xf0);
						*scanline++ = pen_color((pixels << 4) & 0xf0);
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
					scanline = std::fill_n(scanline, hdelay, 0);
					auto const rowbase = screenbase + ((y - vstart) * m_stride * 4);
					for (int x = 0; x < (width - hdelay); x++)
					{
						uint8_t const pixels = rowbase[x];
						*scanline++ = pen_color(pixels);
					}
				}
				else
				{
					std::fill_n(scanline, width, 0);
				}
			}
			break;

		default:
			throw emu_fatalerror("specpdq: unknown video mode %d\n", m_mode);
	}
	return 0;
}

void nubus_specpdq_device::specpdq_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset >= 0xc0000 && offset < 0x100000)
	{
		COMBINE_DATA(&m_7xxxxx_regs[offset-0xc0000]);
	}

	if ((offset >= 0xc0000) && (offset <= 0xc002a))
	{
		m_crtc.write(*this, offset - 0xc0000, data >> 24);
		update_crtc();
	}
	else if ((offset >= 0x181000) && (offset <= 0x18103f))
	{
		// 256 pixels worth at 8 bpp
		m_pattern[offset & 0x3f] = ~data;
	}
	else switch (offset)
	{
		case 0xc0030:
		case 0xc0032:
			m_vint &= BIT(offset, 1) ? 0x00ff : 0xff00;
			m_vint |= (~data & 0xff000000) >> (BIT(offset, 1) ? 16 : 24);
			update_crtc();
			LOG("%s: %u to vint\n", machine().describe_context(), m_vint);
			break;

		case 0xc0054:
		case 0xc0056:
			// same value written here and to c007a:c0078
			m_stride &= BIT(offset, 1) ? 0x00ff : 0xff00;
			m_stride |= (~data & 0xff000000) >> (BIT(offset, 1) ? 16 : 24);
			LOG("%s: %u to stride\n", machine().describe_context(), m_stride);
			break;

		case 0xc005c:   // interrupt control
			if (!(data & 0x8000))
			{
				m_vbl_disable = 1;
			}
			else
			{
				m_vbl_disable = 0;
				lower_slot_irq();
			}
			break;

		case 0xc005e:   // not sure, interrupt related?
			break;

		case 0xc006a:
			LOG("%s: %u to hdelay\n", machine().describe_context(), ~data & 0xff);
			m_hdelay = ~data & 0xff;
			update_crtc();
			break;

		case 0x120000:  // DAC address
			LOG("%s: %08x to DAC control\n", machine().describe_context(), data);
			m_clutoffs = ~data & 0xff;
			break;

		case 0x120001:  // DAC data
			m_colors[m_count++] = ~(data >> 8) & 0xff;

			if (m_count == 3)
			{
				LOG("RAMDAC: color %d = %02x %02x %02x %s\n", m_clutoffs, m_colors[0], m_colors[1], m_colors[2], machine().describe_context());
				set_pen_color(m_clutoffs, rgb_t(m_colors[0], m_colors[1], m_colors[2]));
				m_clutoffs = (m_clutoffs + 1) & 0xff;
				m_count = 0;
			}
			break;

		case 0x140000:
			m_shiftreg.write_data(*this, data);
			if (m_shiftreg.ready())
			{
				switch (m_shiftreg.select())
				{
				case 0:
					// bit depth in low bits, other bits unknown
					LOG("%s: %x to mode\n", machine().describe_context(), m_shiftreg.value());
					m_mode = m_shiftreg.value() & 0x03;
					break;
				default:
					LOG("%s: %x to param %x\n", machine().describe_context(), m_shiftreg.value(), m_shiftreg.select());
				}
			}
			break;

		case 0x160000:
		case 0x160001:
			m_osc &= ~(1 << (BIT(offset, 0)));
			m_osc |= BIT(~data, 0) << BIT(offset, 0);
			LOG("%s: %u to osc\n", machine().describe_context(), m_osc);
			break;

		case 0x160007:
			m_shiftreg.write_control(*this, data);
			break;

		// blitter control
		case 0x182006:
			data = ~data;
			LOG("%08x (%d) to blitter ctrl 1 %s rectangle\n", data, data, machine().describe_context());
			break;

		case 0x182007:
			data = ~data;
			LOG("%s: %08x to blitter command\n", machine().describe_context(), data);

			if (data == 2)
			{
				// fill rectangle
				auto const source = util::big_endian_cast<uint8_t const>(m_pattern);
				auto dest = util::big_endian_cast<uint8_t>(&m_vram[0]) + m_vram_addr;
				uint32_t const patofsx = (m_vram_addr & 0x3) + m_patofsx;
				LOG("Fill rectangle with %02x %02x %02x %02x, adr %x (%d, %d) width %d height %d delta %d %d\n", source[0], source[1], source[2], source[3], m_vram_addr, m_vram_addr % 1152, m_vram_addr / 1152, m_width, m_height, m_patofsx, m_patofsy);
				for (int y = 0; y <= m_height; y++)
				{
					for (int x = 0; x < m_width; x++)
					{
						dest[x] = source[(((m_patofsy + y) & 0x7) << 5) + ((patofsx + x) & 0x1f)];
					}
					dest += m_stride * 4;
				}
			}
			else if (data == 0x100)
			{
				LOG("Copy rectangle forwards, width %d height %d dst %x (%d, %d) src %x (%d, %d)\n", m_width, m_height, m_vram_addr, m_vram_addr % 1152, m_vram_addr / 1152, m_vram_src, m_vram_src % 1152, m_vram_src / 1152);
				auto source = util::big_endian_cast<uint8_t const>(&m_vram[0]) + m_vram_src;
				auto dest = util::big_endian_cast<uint8_t>(&m_vram[0]) + m_vram_addr;
				for (int y = 0; y <= m_height; y++)
				{
					for (int x = 0; x < m_width; x++)
					{
						dest[x] = source[x];
					}
					source += m_stride * 4;
					dest += m_stride * 4;
				}
			}
			else if (data == 0x101)
			{
				LOG("Copy rectangle backwards, width %d height %d dst %x (%d, %d) src %x (%d, %d)\n", m_width, m_height, m_vram_addr, m_vram_addr % 1152, m_vram_addr / 1152, m_vram_src, m_vram_src % 1152, m_vram_src / 1152);
				auto source = util::big_endian_cast<uint8_t const>(&m_vram[0]) + m_vram_src;
				auto dest = util::big_endian_cast<uint8_t>(&m_vram[0]) + m_vram_addr;
				for (int y = 0; y <= m_height; y++)
				{
					for (int x = 0; x < m_width; x++)
					{
						dest[-x] = source[-x];
					}
					source -= m_stride * 4;
					dest -= m_stride * 4;
				}
			}
			else
			{
				logerror("Unknown blitter command %08x\n", data);
			}
			break;

		case 0x182008:
			data = ~data;
			LOG("%s: %08x (%d) to blitter ctrl 2 rectangle\n", machine().describe_context(), data, data);
			m_patofsx = BIT(data, 0, 3);
			m_patofsy = BIT(data, 3, 3);
			break;

		case 0x182009:
			data = ~data;
			LOG("%s: %08x to blitter destination\n", machine().describe_context(), data);
			m_vram_addr = data >> 2;
			break;

		case 0x18200a:
			data = ~data;
			LOG("%s: %08x to blitter source\n", machine().describe_context(), data);
			m_vram_src = data >> 2;
			break;

		case 0x18200b:
			data = ~data;
			LOG("%s: %08x (%d) to blitter height\n", machine().describe_context(), data, data);
			m_height = data & 0xffff;
			break;

		case 0x18200e:
			data = ~data;
			LOG("%s: %08x (%d) to blitter width\n", machine().describe_context(), data, data);
			m_width = data;
			break;

		default:
			LOG("%s: specpdq_w: %08x @ %x (mask %08x)\n", machine().describe_context(), data^0xffffffff, offset, mem_mask);
			break;
	}
}

uint32_t nubus_specpdq_device::specpdq_r(offs_t offset, uint32_t mem_mask)
{
//  if (offset != 0xc005c && offset != 0xc005e) logerror("specpdq_r: @ %x (mask %08x  %s)\n", offset, mem_mask, machine().describe_context());

	switch (offset)
	{
		case 0xc002c:
		case 0xc002e:
			/*
			 * FIXME: Oscillator calibration is failing.
			 * Set breakpoint at 0x2e0c on Mac II with card in
			 * slot 9 to catch user oscillator measurement.  See the
			 * measured value in D0.b when PC = 0x2e48.
			 *
			 * Measured:
			 *   55.00 MHz  64    detected as 55.00 MHz
			 *   57.28 MHz   1    detected as 55.00 MHz
			 *   64.00 MHz   9    detected as 57.28 MHz
			 */
			return ~((u32(m_crtc.v_pos(screen())) << (BIT(offset, 1) ? 16 : 24)) & 0xff000000);
	}

	if (offset >= 0xc0000 && offset < 0x100000)
	{
		return m_7xxxxx_regs[offset-0xc0000];
	}

	return 0;
}

void nubus_specpdq_device::vram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data = ~data;
	COMBINE_DATA(&m_vram[offset]);
}

uint32_t nubus_specpdq_device::vram_r(offs_t offset, uint32_t mem_mask)
{
	return ~m_vram[offset];
}
