// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  RasterOps ColorBoard 264 NuBus video card emulation

  fixed resolution 640x480 NuBus video card, 1/2/4/8/24 bit color
  1.5 MiB of VRAM, Bt473KPJ35 RAMDAC, and two custom gate arrays.

  0xfsff6004 is color depth: 0 for 1bpp, 1 for 2bpp, 2 for 4bpp, 3 for 8bpp, 4 for 24bpp
  0xfsff6014 is VBL ack: write 1 to ack
  0xfsff603c is VBL disable: write 1 to disable, 0 to enable

  Crystal (pixel clock) is 30.24 MHz. A 12.3356 MHz crystal is also present,
  likely used only for 30 Hz interlaced NTSC output.

  TODO:
  - Figure out the vertical part of the CRTC registers and make the mode dynamic.
  - Figure out how 24 bpp mode fits in 1.5 MiB.  Stride is 4096 bytes, each
    scanline is 640 pixels at 4 bytes each, which comes to just below 2 MiB.

***************************************************************************/

#include "emu.h"

#include "nubus_cb264.h"

#include "emupal.h"
#include "screen.h"

#include <algorithm>

namespace {

static constexpr u32 VRAM_SIZE = 0x200000;  // 12x TC524256J (256K x 4 bit VRAM), so 1.5 MiB total

class nubus_cb264_device : public device_t,
							public device_nubus_card_interface
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

	uint32_t cb264_r(offs_t offset, uint32_t mem_mask = ~0);
	void cb264_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cb264_ramdac_r(offs_t offset);
	void cb264_ramdac_w(offs_t offset, uint32_t data);

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	std::unique_ptr<u32[]> m_vram;
	uint32_t m_cb264_mode, m_cb264_vbl_disable;
	uint32_t m_colors[3], m_count, m_clutoffs;
};

ROM_START( cb264 )
	ROM_REGION(0x4000, "cb264_rom", 0)
	ROM_LOAD16_BYTE( "264-1915.bin", 0x000000, 0x002000, CRC(26c19ee5) SHA1(2b2853d04cc6b0258e85eccd23ebfd4f4f63a084) )
	ROM_LOAD16_BYTE( "264-1914.bin", 0x000001, 0x002000, CRC(d5fbd5ad) SHA1(98d35ed3fb0bca4a9bee1cdb2af0d3f22b379386) )
ROM_END

void nubus_cb264_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "cb264_screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(nubus_cb264_device::screen_update));
	screen.set_raw(30.24_MHz_XTAL, 864, 0, 640, 525, 0, 480); // 35 kHz horizontal rate, 66.67 Hz vertical rate

	PALETTE(config, m_palette).set_entries(256);
}

const tiny_rom_entry *nubus_cb264_device::device_rom_region() const
{
	return ROM_NAME( cb264 );
}

nubus_cb264_device::nubus_cb264_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nubus_cb264_device(mconfig, NUBUS_CB264, tag, owner, clock)
{
}

nubus_cb264_device::nubus_cb264_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_nubus_card_interface(mconfig, *this),
	m_screen(*this, "cb264_screen"),
	m_palette(*this, "cb264_palette"),
	m_cb264_mode(0), m_cb264_vbl_disable(0), m_count(0), m_clutoffs(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_cb264_device::device_start()
{
	u32 slotspace = get_slotspace();

	install_declaration_rom("cb264_rom");

	m_vram = std::make_unique<u32[]>(VRAM_SIZE / sizeof(u32));
	install_bank(slotspace, slotspace + VRAM_SIZE - 1, &m_vram[0]);

	nubus().install_device(slotspace+0xff6000, slotspace+0xff60ff, read32s_delegate(*this, FUNC(nubus_cb264_device::cb264_r)), write32s_delegate(*this, FUNC(nubus_cb264_device::cb264_w)));
	nubus().install_device(slotspace+0xff7000, slotspace+0xff70ff, read32sm_delegate(*this, FUNC(nubus_cb264_device::cb264_ramdac_r)), write32sm_delegate(*this, FUNC(nubus_cb264_device::cb264_ramdac_w)));

	save_item(NAME(m_cb264_mode));
	save_item(NAME(m_cb264_vbl_disable));
	save_item(NAME(m_colors));
	save_item(NAME(m_count));
	save_item(NAME(m_clutoffs));
	save_pointer(NAME(m_vram), VRAM_SIZE / sizeof(u32));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_cb264_device::device_reset()
{
	m_count = 0;
	m_clutoffs = 0;
	m_cb264_vbl_disable = 1;
	m_cb264_mode = 0;
	std::fill_n(&m_vram[0], VRAM_SIZE / sizeof(u32), 0);
}

uint32_t nubus_cb264_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!m_cb264_vbl_disable)
	{
		raise_slot_irq();
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

void nubus_cb264_device::cb264_w(offs_t offset, u32 data, u32 mem_mask)
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

		/*
		    CRTC, but vertical doesn't make sense.
		    Vertical:
		    40: 2   - first line of active display?
		    44: 209 - last line of active display?
		    48: 20c - vtotal - 1? (525)
		    4c: f

		    Horizontal:
		    50: 27  - start of active display
		    54: c7  - end of active display (0xc7 - 0x27 = 160, 160 * 4 = 640)
		    58: d7  - htotal/4 - 1? (864)
		    5c: 6b
		*/

		default:
			//printf("%s cb264_w: %x to reg %x (mask %x)\n", machine().describe_context().c_str(), data, offset*4, mem_mask);
			break;
	}
}

u32 nubus_cb264_device::cb264_r(offs_t offset, u32 mem_mask)
{
	switch (offset)
	{
		case 0x0c/4:
		case 0x28/4:
			break;

		case 0x34/4:
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
//          printf("%x to unknown RAMDAC register @ %x\n", data, offset);
			break;
	}
}

u32 nubus_cb264_device::cb264_ramdac_r(offs_t offset)
{
	return 0;
}

}   // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(NUBUS_CB264, device_nubus_card_interface, nubus_cb264_device, "nb_c264", "RasterOps ColorBoard 264 video card")
