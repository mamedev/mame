// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Apple model 820-5037-C "Macintosh II Portrait Video Card"
  PCB is marked "Workstation/Portrait Card"
  640x870, 1, 2 or 4bpp grayscale

  Raster parameters, from an archived copy of Apple's spec sheet:
  Horizontal  68.85 kHz
  Vertical    75.0 Hz
  Dot Clock   57.2832 MHz

  Fs0900e0 = DAC control
  Fs0900e4 = DAC data
  Fs0A0000 = enable / ack VBL IRQ
  Fs0A0004 = disable VBL IRQ

***************************************************************************/

#include "emu.h"

#include "nubus_wsportrait.h"

#include "emupal.h"
#include "screen.h"

#include <algorithm>

namespace {

static constexpr u32 VRAM_SIZE = 0x80000;

class nubus_wsportrait_device : public device_t,
								public device_video_interface,
								public device_nubus_card_interface
{
public:
	// construction/destruction
	nubus_wsportrait_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	nubus_wsportrait_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(vbl_tick);

private:
	u32 wsportrait_r(offs_t offset, u32 mem_mask = ~0);
	void wsportrait_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 vram_r(offs_t offset, u32 mem_mask = ~0);
	void vram_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	std::unique_ptr<u32[]> m_vram;
	u32 m_mode, m_vbl_disable;
	u32 m_colors[3], m_count, m_clutoffs;
	emu_timer *m_timer;
};

ROM_START( wsportrait )
	ROM_REGION(0x1000, "declrom", 0)
	ROM_LOAD( "341-0732.bin", 0x000000, 0x001000, CRC(ddc35b78) SHA1(ce2bf2374bb994c17962dba8f3d11bc1260e2644) )
ROM_END

void nubus_wsportrait_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(nubus_wsportrait_device::screen_update));
	m_screen->set_raw(57.2832_MHz_XTAL, 832, 0, 640, 918, 0, 832);
	m_screen->set_physical_aspect(3, 4);

	PALETTE(config, m_palette).set_entries(256);
}

const tiny_rom_entry *nubus_wsportrait_device::device_rom_region() const
{
	return ROM_NAME( wsportrait );
}

nubus_wsportrait_device::nubus_wsportrait_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	nubus_wsportrait_device(mconfig, NUBUS_WSPORTRAIT, tag, owner, clock)
{
}

nubus_wsportrait_device::nubus_wsportrait_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_nubus_card_interface(mconfig, *this),
	m_screen(*this, "screen"),
	m_palette(*this, "palette"),
	m_mode(0), m_vbl_disable(0), m_count(0), m_clutoffs(0), m_timer(nullptr)
{
	set_screen(*this, "screen");
}

void nubus_wsportrait_device::device_start()
{
	u32 slotspace;

	install_declaration_rom("declrom", true);

	slotspace = get_slotspace();

	m_vram = std::make_unique<u32[]>(VRAM_SIZE / sizeof(u32));

	nubus().install_device(slotspace, slotspace+VRAM_SIZE-1, read32s_delegate(*this, FUNC(nubus_wsportrait_device::vram_r)), write32s_delegate(*this, FUNC(nubus_wsportrait_device::vram_w)));
	nubus().install_device(slotspace+0x900000, slotspace+0x900000+VRAM_SIZE-1, read32s_delegate(*this, FUNC(nubus_wsportrait_device::vram_r)), write32s_delegate(*this, FUNC(nubus_wsportrait_device::vram_w)));
	nubus().install_device(slotspace+0x80000, slotspace+0xeffff, read32s_delegate(*this, FUNC(nubus_wsportrait_device::wsportrait_r)), write32s_delegate(*this, FUNC(nubus_wsportrait_device::wsportrait_w)));

	m_timer = timer_alloc(FUNC(nubus_wsportrait_device::vbl_tick), this);
	m_timer->adjust(screen().time_until_pos(869, 0), 0);

	save_item(NAME(m_mode));
	save_item(NAME(m_vbl_disable));
	save_item(NAME(m_colors));
	save_item(NAME(m_count));
	save_item(NAME(m_clutoffs));
	save_pointer(NAME(m_vram), VRAM_SIZE / sizeof(u32));
}

void nubus_wsportrait_device::device_reset()
{
	m_count = 0;
	m_clutoffs = 0;
	m_vbl_disable = 1;
	m_mode = 0;
	std::fill_n(&m_vram[0], VRAM_SIZE / sizeof(u32), 0);
}

TIMER_CALLBACK_MEMBER(nubus_wsportrait_device::vbl_tick)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	m_timer->adjust(screen().time_until_pos(869, 0), 0);
}

u32 nubus_wsportrait_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]) + 0x80;
	const pen_t *pens = m_palette->pens();

	switch (m_mode)
	{
		case 0: // 1 bpp?
			for (int y = 0; y < 870; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/8; x++)
				{
					u8 const pixels = vram8[(y * 128) + x];

					*scanline++ = pens[BIT(pixels, 7)];
					*scanline++ = pens[BIT(pixels, 6)];
					*scanline++ = pens[BIT(pixels, 5)];
					*scanline++ = pens[BIT(pixels, 4)];
					*scanline++ = pens[BIT(pixels, 3)];
					*scanline++ = pens[BIT(pixels, 2)];
					*scanline++ = pens[BIT(pixels, 1)];
					*scanline++ = pens[BIT(pixels, 0)];
				}
			}
			break;

		case 1: // 2 bpp
			for (int y = 0; y < 870; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/4; x++)
				{
					u8 const pixels = vram8[(y * 256) + x];

					*scanline++ = pens[((pixels>>6)&3)];
					*scanline++ = pens[((pixels>>4)&3)];
					*scanline++ = pens[((pixels>>2)&3)];
					*scanline++ = pens[(pixels&3)];
				}
			}
			break;

		case 2: // 4 bpp
			for (int y = 0; y < 870; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/2; x++)
				{
					u8 const pixels = vram8[(y * 512) + x];

					*scanline++ = pens[((pixels&0xf0)>>4)];
					*scanline++ = pens[(pixels&0xf)];
				}
			}
			break;

		default:
			fatalerror("wsportrait: unknown video mode %d\n", m_mode);
	}
	return 0;
}

void nubus_wsportrait_device::wsportrait_w(offs_t offset, u32 data, u32 mem_mask)
{
	data ^= 0xffffffff;
//  if (offset != 0x8000) printf("wsportrait: Write %08x @ %x, mask %08x\n", data, offset, mem_mask);

	switch (offset)
	{
		case 1:         // mode control
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
			m_count = 0;
			break;

		case 0x4039:    // DAC data - only 4 bits per component!
			m_colors[m_count] = (data>>24) & 0x0f;
			m_colors[m_count] |= (m_colors[m_count]<<4);
			m_count++;

			if (m_count == 3)
			{
				m_palette->set_pen_red_level(m_clutoffs, m_colors[2]);
				m_palette->set_pen_green_level(m_clutoffs, m_colors[2]);
				m_palette->set_pen_blue_level(m_clutoffs, m_colors[2]);
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

u32 nubus_wsportrait_device::wsportrait_r(offs_t offset, u32 mem_mask)
{
	switch (offset)
	{
		case 0x4004:
			return (m_screen->vblank() << 16) | 0xfffc0000;   // bit 16 = vbl status, bits 17-20 = monitor type
	}
	return 0;
}

void nubus_wsportrait_device::vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	data ^= 0xffffffff;
	COMBINE_DATA(&m_vram[offset]);
}

u32 nubus_wsportrait_device::vram_r(offs_t offset, u32 mem_mask)
{
	return m_vram[offset] ^ 0xffffffff;
}

}   // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(NUBUS_WSPORTRAIT, device_nubus_card_interface, nubus_wsportrait_device, "nb_wspt", "Macintosh II Portrait Video Card")
