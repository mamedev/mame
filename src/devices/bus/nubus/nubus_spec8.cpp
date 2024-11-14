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

  The user-supplied oscillator calibration result is only saved if the
  value in PRAM is invalid.  If you change the oscillator in Machine
  Configuration settings, you need to zap the PRAM before it will be
  recognised properly.

  Set breakpoint at 0x2d1e on Mac II with Ver. 1.3 card in slot 9 to catch
  user oscillator measurement.  See the measured value in D0.b when
  PC = 0x2d6c.

  Expected:
     2- 5    57.28 MHz
     6-19    bad
    20-22    14.32 MHz
    23-24    15.67 MHz
    25-27    17.73 MHz
    27-      bad

  Measured:
    14.32 MHz  20
    15.67 MHz  23
    17.73 MHz  26
    57.28 MHz   5

  TODO:
  * Having no oscillator installed at Y1 is not emulated.
  * Interlaced modes are not understood.

***************************************************************************/

#include "emu.h"
#include "nubus_spec8.h"

#include "supermac.h"

#include "layout/generic.h"
#include "screen.h"

#include <algorithm>

//#define VERBOSE 1
#include "logmacro.h"


#define SPEC8S3_SCREEN_NAME "screen"
#define SPEC8S3_ROM_REGION  "spec8s3_rom"

#define VRAM_SIZE   (0x10'0000) // 1M of VRAM for 1024x768 @ 8 bit


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class nubus_spec8s3_device :
		public device_t,
		public device_nubus_card_interface,
		public device_video_interface,
		public device_palette_interface
{
public:
	// construction/destruction
	nubus_spec8s3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	nubus_spec8s3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// palette implementation
	virtual uint32_t palette_entries() const noexcept override;

	TIMER_CALLBACK_MEMBER(vbl_tick);

private:
	uint32_t spec8s3_r(offs_t offset, uint32_t mem_mask = ~0);
	void spec8s3_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t vram_r(offs_t offset, uint32_t mem_mask = ~0);
	void vram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void update_crtc();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_ioport m_userosc;
	emu_timer *m_timer;

	supermac_spec_crtc m_crtc;
	supermac_spec_shift_reg m_shiftreg;

	std::vector<uint32_t> m_vram;
	uint32_t m_mode, m_vbl_disable;
	uint32_t m_colors[3], m_count, m_clutoffs;

	uint8_t m_osc;
	bool m_interlace;

	uint16_t m_hpan, m_vpan;
	uint8_t m_zoom;

	bool m_vbl_pending;
};


INPUT_PORTS_START( spec8s3 )
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


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nubus_spec8s3_device::device_add_mconfig(machine_config &config)
{
	config.set_default_layout(layout_monitors);

	screen_device &screen(SCREEN(config, SPEC8S3_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(nubus_spec8s3_device::screen_update));
	screen.set_raw(80.000_MHz_XTAL, 332*4, 64*4, 320*4, 804, 33, 801);
	screen.set_video_attributes(VIDEO_UPDATE_SCANLINE);
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

uint32_t nubus_spec8s3_device::palette_entries() const noexcept
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

	m_crtc.register_save(*this);
	m_shiftreg.register_save(*this);

	save_item(NAME(m_vram));
	save_item(NAME(m_mode));
	save_item(NAME(m_vbl_disable));
	save_item(NAME(m_colors));
	save_item(NAME(m_count));
	save_item(NAME(m_clutoffs));
	save_item(NAME(m_osc));
	save_item(NAME(m_interlace));
	save_item(NAME(m_hpan));
	save_item(NAME(m_vpan));
	save_item(NAME(m_zoom));
	save_item(NAME(m_vbl_pending));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_spec8s3_device::device_reset()
{
	m_crtc.reset();
	m_shiftreg.reset();

	std::fill(m_vram.begin(), m_vram.end(), 0);
	m_mode = 0;
	m_vbl_disable = 1;
	std::fill(std::begin(m_colors), std::end(m_colors), 0);
	m_count = 0;
	m_clutoffs = 0;
	m_osc = 0;
	m_interlace = false;
	m_hpan = 0;
	m_vpan = 0;
	m_zoom = 0;
	m_vbl_pending = false;
}


TIMER_CALLBACK_MEMBER(nubus_spec8s3_device::vbl_tick)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
		m_vbl_pending = true;
	}

	// TODO: confirm vertical blanking interrupt timing
	m_timer->adjust(screen().time_until_pos((m_crtc.v_end() - 1) * (m_interlace ? 2 : 1), 0));
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
			oscillator = 57.28_MHz_XTAL;
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
	m_interlace = m_crtc.v_total() < 320;

	// for some reason you temporarily get invalid screen parameters - ignore them
	if (m_crtc.valid(*this))
	{
		screen().configure(
				m_crtc.h_total(4),
				m_crtc.v_total() * (m_interlace ? 2 : 1),
				rectangle(
					m_crtc.h_start(4),
					m_crtc.h_end(4) - 1,
					m_crtc.v_start() * (m_interlace ? 2 : 1),
					(m_crtc.v_end() * (m_interlace ? 2 : 1)) - 1),
				m_crtc.frame_time(4, oscillator));

		// TODO: confirm vertical blanking interrupt timing
		m_timer->adjust(screen().time_until_pos((m_crtc.v_end() - 1) * (m_interlace ? 2 : 1), 0));
	}
}

uint32_t nubus_spec8s3_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	auto const screenbase = util::big_endian_cast<uint8_t const>(&m_vram[0]) + (m_vpan * 2048) + 0x400;

	int const hstart = m_crtc.h_start(4);
	int const width = m_crtc.h_active(4);
	int const pixels = width >> (m_zoom ? 1 : 0);
	int const vstart = m_crtc.v_start() * (m_interlace ? 2 : 1);
	int const vend = m_crtc.v_end() * (m_interlace ? 2 : 1);

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
	if ((offset >= 0x3800) && (offset <= 0x382a))
	{
		m_crtc.write(*this, offset - 0x3800, data);
		update_crtc();
	}
	else switch (offset)
	{
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
				case 1:
					// bits 0-2 and 7 are unknown
					LOG("%s: %x to hpan\n", machine().describe_context(), m_shiftreg.value());
					m_hpan = (m_hpan & 0x07f0) | ((m_shiftreg.value() >> 3) & 0x0f);
					break;
				default:
					LOG("%s: %x to param %x\n", machine().describe_context(), m_shiftreg.value(), m_shiftreg.select());
				}
			}
			break;

		case 0x3e02:
			m_shiftreg.write_control(*this, data);
			break;

		case 0x3e05:
		case 0x3e06:
		case 0x3e07:
			m_osc &= ~(1 << (offset - 0x3e05));
			m_osc |= BIT(~data, 0) << (offset - 0x3e05);
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
				// hack for interlace modes because screen_device doesn't support them
				int vpos = screen().vpos();
				if (m_interlace)
					vpos /= 2;
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

} // anonymous namespace


//**************************************************************************
//  DEVICE TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(NUBUS_SPEC8S3, device_nubus_card_interface, nubus_spec8s3_device, "nb_sp8s3", "SuperMac Spectrum/8 Series III video card")
