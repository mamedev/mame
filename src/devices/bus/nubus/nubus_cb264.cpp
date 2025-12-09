// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  RasterOps ColorBoard 264 NuBus video card emulation

  Fixed resolution 640x480 NuBus video card, 1/2/4/8/24 bit color
  1.5 MiB of VRAM, Bt473KPJ35 RAMDAC, and two custom gate arrays.

  Crystal (pixel clock) is 30.24 MHz. A 12.3356 MHz crystal is also present
  for 30 Hz interlaced NTSC output.

  The card has 1.5 MiB of VRAM mapped into a 2 MiB space.  It's divided into
  3 banks of 512KiB each, divided up as 512K of red, 512K of green, and 512K
  of blue.

  In paletted modes (1, 2, 4, and 8 bits per pixel), the framebuffer
  is the 512K red bank exclusively on every byte lane.

  In 24-bit mode, for each 32-bit word, byte 0 is from the red bank, byte 1
  is from the green bank, and byte 2 is from the blue bank.  Byte 3 reads as zero
  and writes are discarded.  We don't currently emulate this mechanism, but
  may in the future.

***************************************************************************/

#include "emu.h"

#include "nubus_cb264.h"

#include "video/bt47x.h"

#include "emupal.h"
#include "screen.h"

#include <algorithm>

#define VERBOSE (0)

#include "logmacro.h"
namespace {

	// 12x TC524256J (256K x 4 bit VRAM) for 1.5 MiB total.
	// To make emulation easier/more performant, we allocate the 4th 512K bank
	// and just zero it out n 24-bit mode.
	static constexpr u32 VRAM_SIZE = 0x20'0000;

	enum
	{
		VSyncEnd = 0,
		VBlankStart,
		VTotal,
		HSyncEnd,
		HBlankEnd,
		HBlankStart,
		HTotal,
		HHalfLineCount,
		VBlankEnd,

		CRTC_Length
	};

	class nubus_cb264_device : public device_t,
							   public device_nubus_card_interface,
							   public device_video_interface
	{
	public:
		// construction/destruction
		nubus_cb264_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	protected:
		nubus_cb264_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

		// device-level overrides
		virtual void device_start() override ATTR_COLD;
		virtual void device_reset() override ATTR_COLD;

		// optional information overrides
		virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
		virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
		virtual ioport_constructor device_input_ports() const override ATTR_COLD;

		u32 cb264_r(offs_t offset, u32 mem_mask = ~0);
		void cb264_w(offs_t offset, u32 data, u32 mem_mask = ~0);
		u32 cb264_ramdac_r(offs_t offset);
		void cb264_ramdac_w(offs_t offset, u32 data);
		u32 cb264_vram_r(offs_t offset);
		void cb264_vram_w(offs_t offset, u32 data, u32 mem_mask);

	private:
		u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

		void card_map(address_map &map);

		TIMER_CALLBACK_MEMBER(vbl_tick);

		required_device<screen_device> m_screen;
		required_device<palette_device> m_palette;
		required_ioport m_config;
		std::unique_ptr<u32[]> m_vram;

		emu_timer *m_vbl_timer;

		u32 m_cb264_mode, m_cb264_vbl_disable;
		u32 m_colors[3], m_count, m_clutoffs;
		u32 m_force_blank, m_osc_select;
		u32 m_crtc[CRTC_Length];
};

ROM_START( cb264 )
	ROM_REGION(0x4000, "declrom", 0)
	ROM_LOAD16_BYTE( "264-1915.bin", 0x000000, 0x002000, CRC(26c19ee5) SHA1(2b2853d04cc6b0258e85eccd23ebfd4f4f63a084) )
	ROM_LOAD16_BYTE( "264-1914.bin", 0x000001, 0x002000, CRC(d5fbd5ad) SHA1(98d35ed3fb0bca4a9bee1cdb2af0d3f22b379386) )
ROM_END

INPUT_PORTS_START( cb264 )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x00, "Monitor Type" )
	PORT_CONFSETTING( 0x00, "Macintosh 13\" 640x480");
	PORT_CONFSETTING( 0x01, "NTSC");
INPUT_PORTS_END

ioport_constructor nubus_cb264_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(cb264);
}

void nubus_cb264_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(nubus_cb264_device::screen_update));
	screen.set_raw(30.24_MHz_XTAL, 864, 0, 640, 525, 0, 480); // 35 kHz horizontal rate, 66.67 Hz vertical rate

	PALETTE(config, m_palette).set_entries(256);
}

const tiny_rom_entry *nubus_cb264_device::device_rom_region() const
{
	return ROM_NAME( cb264 );
}

nubus_cb264_device::nubus_cb264_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	nubus_cb264_device(mconfig, NUBUS_CB264, tag, owner, clock)
{
}

nubus_cb264_device::nubus_cb264_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_nubus_card_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_screen(*this, "screen"),
	m_palette(*this, "palette"),
	m_config(*this, "CONFIG"),
	m_cb264_mode(0), m_cb264_vbl_disable(0), m_count(0), m_clutoffs(0),
	m_force_blank(0), m_osc_select(0)
{
	std::fill_n(&m_crtc[0], CRTC_Length, 0);
	set_screen(*this, "screen");
}

void nubus_cb264_device::card_map(address_map &map)
{
	map(0x00'0000, 0x1f'ffff).rw(FUNC(nubus_cb264_device::cb264_vram_r), FUNC(nubus_cb264_device::cb264_vram_w));
	map(0xff'6000, 0xff'60ff).rw(FUNC(nubus_cb264_device::cb264_r), FUNC(nubus_cb264_device::cb264_w));
	map(0xff'7000, 0xff'70ff).rw(FUNC(nubus_cb264_device::cb264_ramdac_r), FUNC(nubus_cb264_device::cb264_ramdac_w));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_cb264_device::device_start()
{
	install_declaration_rom("declrom");

	m_vram = std::make_unique<u32[]>(VRAM_SIZE / sizeof(u32));
	nubus().install_map(*this, &nubus_cb264_device::card_map);

	save_item(NAME(m_cb264_mode));
	save_item(NAME(m_cb264_vbl_disable));
	save_item(NAME(m_colors));
	save_item(NAME(m_count));
	save_item(NAME(m_clutoffs));
	save_item(NAME(m_force_blank));
	save_item(NAME(m_osc_select));
	save_pointer(NAME(m_vram), VRAM_SIZE / sizeof(u32));

	m_vbl_timer = timer_alloc(FUNC(nubus_cb264_device::vbl_tick), this);
}

void nubus_cb264_device::device_reset()
{
	m_count = 0;
	m_clutoffs = 0;
	m_cb264_vbl_disable = 1;
	m_cb264_mode = 0;
	m_force_blank = 0;
	std::fill_n(&m_vram[0], VRAM_SIZE / sizeof(u32), 0);
}

u32 nubus_cb264_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!BIT(m_force_blank, 0))
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]);
	const pen_t *pens = m_palette->pens();

	switch (m_cb264_mode)
	{
		case 0: // 1 bpp
			for (int y = 0; y < 480; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/8; x++)
				{
					u8 const pixels = vram8[(y * 1024) + x];

					*scanline++ = pens[pixels&0x80];
					*scanline++ = pens[(pixels<<1)&0x80];
					*scanline++ = pens[(pixels<<2)&0x80];
					*scanline++ = pens[(pixels<<3)&0x80];
					*scanline++ = pens[(pixels<<4)&0x80];
					*scanline++ = pens[(pixels<<5)&0x80];
					*scanline++ = pens[(pixels<<6)&0x80];
					*scanline++ = pens[(pixels<<7)&0x80];
				}
			}
			break;

		case 1: // 2 bpp (3f/7f/bf/ff)
			for (int y = 0; y < 480; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/4; x++)
				{
					u8 const pixels = vram8[(y * 1024) + x];

					*scanline++ = pens[pixels&0xc0];
					*scanline++ = pens[(pixels<<2)&0xc0];
					*scanline++ = pens[(pixels<<4)&0xc0];
					*scanline++ = pens[(pixels<<6)&0xc0];
				}
			}
			break;

		case 2: // 4 bpp
			for (int y = 0; y < 480; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/2; x++)
				{
					u8 const pixels = vram8[(y * 1024) + x];

					*scanline++ = pens[pixels&0xf0];
					*scanline++ = pens[(pixels<<4)&0xf0];
				}
			}
			break;

		case 3: // 8 bpp
			for (int y = 0; y < 480; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640; x++)
				{
					u8 const pixels = vram8[(y * 1024) + x];
					*scanline++ = pens[pixels];
				}
			}
			break;

		case 4: // 24 bpp
		case 5:
		case 6:
		case 7: // if bit 2 is set the ASIC forces 24bpp regardless of bits 0 & 1
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

void nubus_cb264_device::cb264_w(offs_t offset, u32 data, u32 mem_mask)
{
	switch (offset<<2)
	{
		case 0x4:       // 0 = 1 bpp, 1 = 2bpp, 2 = 4bpp, 3 = 8bpp, 4 = 24bpp
			m_cb264_mode = data & 7;
			break;

		case 0xc:       // Oscillator select
			m_osc_select = data;
			break;

		case 0x14:      // VBL clear
			lower_slot_irq();
			break;

		case 0x1c:      // force blank
			data &= 1;
			if ((data != m_force_blank) && (data = 1))
			{
				const u32 hres = (m_crtc[HBlankStart] - m_crtc[HBlankEnd]) * 4;
				const u32 htotal = m_crtc[HTotal] * 4;
				u32 vres = m_crtc[VBlankStart] - m_crtc[VBlankEnd];
				u32 vtotal = m_crtc[VTotal] + 1;

				// bit 1 is the actual oscillator select
				const u32 pixel_clock = BIT(m_osc_select, 1) ? 12'335'600 : 30'240'000;

				// bit 0 is NTSC mode
				if (BIT(m_osc_select, 0))
				{
					vres *= 2;
					vtotal *= 2;
				}

				LOG("osc_select %d pixel_clock %d\n", m_osc_select, pixel_clock);
				LOG("hres %d vres %d htotal %d vtotal %d\n", hres, vres, htotal, vtotal);

				const rectangle visarea(0, hres - 1, 0, vres - 1);
				m_screen->configure(htotal, vtotal, visarea, attotime::from_ticks(htotal * vtotal, 30'240'000).as_attoseconds());
			}

			m_force_blank = data;
			break;

		case 0x3c:      // VBL disable
			m_cb264_vbl_disable = data;

			if (!data)
			{
				const u32 hres = (m_crtc[HBlankStart] - m_crtc[HBlankEnd]) * 4;
				m_vbl_timer->adjust(m_screen->time_until_pos(hres - 1, 0), 0);
			}
			break;

		case 0x40: case 0x44: case 0x48: case 0x4c: case 0x50: case 0x54: case 0x58: case 0x5c:
		case 0x60:
			m_crtc[(offset - (0x40 >> 2))] = data;
			break;

		default:
			LOG("%s cb264_w: %x to reg %x (mask %x)\n", machine().describe_context().c_str(), data, offset*4, mem_mask);
			break;
	}
}

u32 nubus_cb264_device::cb264_r(offs_t offset, u32 mem_mask)
{
	switch (offset<<2)
	{
		case 0xc:
			return m_osc_select;

		case 0x1c:
			return m_force_blank;

		case 0x28:
			return 0x4 | (m_config->read() << 1);   // bit 2 = 1 is monitor connected, bit 1 = 0 for 60 Hz non-interlaced, bit 0 = 0 for option RAM connected

		case 0x34:
			return m_screen->vblank();  // bit 0 is vblank

		default:
			logerror("cb264_r: reg %x (mask %x %s)\n", offset*4, mem_mask, machine().describe_context());
			break;
	}

	return 0;
}

void nubus_cb264_device::cb264_ramdac_w(offs_t offset, u32 data)
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
				m_palette->set_pen_red_level(m_clutoffs, m_colors[0]);
				m_palette->set_pen_green_level(m_clutoffs, m_colors[1]);
				m_palette->set_pen_blue_level(m_clutoffs, m_colors[2]);
				m_clutoffs++;
				m_count = 0;
			}
			break;

		default:
			LOG("%x to unknown RAMDAC register @ %x\n", data, offset);
			break;
	}
}

u32 nubus_cb264_device::cb264_ramdac_r(offs_t offset)
{
	return 0;
}

u32 nubus_cb264_device::cb264_vram_r(offs_t offset)
{
	if (BIT(m_cb264_mode, 2))
	{
		return m_vram[offset] & 0x00ffffff;
	}

	return m_vram[offset];
}

void nubus_cb264_device::cb264_vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (BIT(m_cb264_mode, 2))
	{
		data &= 0x00ffffff;
	}
	COMBINE_DATA(&m_vram[offset]);
}

TIMER_CALLBACK_MEMBER(nubus_cb264_device::vbl_tick)
{
	raise_slot_irq();
	m_vbl_timer->adjust(m_screen->time_until_pos(479, 0), 0);
}

}   // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(NUBUS_CB264, device_nubus_card_interface, nubus_cb264_device, "nb_c264", "RasterOps ColorBoard 264 video card")
