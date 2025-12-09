// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Sigma Designs L-View card

  TODO: This is clearly related closely to the NuBus laserview.cpp card,
  but there are differences.  (The protection latch seems reversed, for
  starters.  Figure it out and merge this into that!)

  ***************************************************************************/

#include "emu.h"
#include "pds30_sigmalview.h"
#include "screen.h"

#include <algorithm>

namespace {

static constexpr u32 VRAM_SIZE = 0x80000;

class nubus_lview_device :
		public device_t,
		public device_video_interface,
		public device_nubus_card_interface
{
public:
	// construction/destruction
	nubus_lview_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(vbl_tick);

private:
	u8 lview_r(offs_t offset);
	void lview_w(offs_t offset, u8 data);
	u32 vram_r(offs_t offset, u32 mem_mask = ~0);
	void vram_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void card_map(address_map &map);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<screen_device> m_screen;
	std::vector<u32> m_vram;
	u32 m_vbl_disable;
	u32 m_palette[256];
	emu_timer *m_timer;
	int m_protstate;
};

ROM_START( lview )
	ROM_REGION(0x4000, "declrom", 0)
	ROM_LOAD( "lv_asi_4_00.bin", 0x000000, 0x004000, CRC(b806f875) SHA1(1e58593b1a8720193d1651b0d8a0d43e4e47563d) )
ROM_END

void nubus_lview_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(nubus_lview_device::screen_update));
	m_screen->set_size(832, 600);
	m_screen->set_refresh_hz(70);
	m_screen->set_visarea(0, 832-1, 0, 600-1);
}

const tiny_rom_entry *nubus_lview_device::device_rom_region() const
{
	return ROM_NAME( lview );
}

nubus_lview_device::nubus_lview_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, PDS030_LVIEW, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_nubus_card_interface(mconfig, *this),
	m_screen(*this, "screen"),
	m_vbl_disable(0), m_timer(nullptr), m_protstate(0)
{
		set_screen(*this, "screen");
}

void nubus_lview_device::card_map(address_map &map)
{
	map(0x0000'0000, 0x0007'ffff).rw(FUNC(nubus_lview_device::vram_r), FUNC(nubus_lview_device::vram_w)).mirror(0x00f0'0000);
	map(0x000b'0000, 0x000b'ffff).rw(FUNC(nubus_lview_device::lview_r), FUNC(nubus_lview_device::lview_w)).mirror(0x00f0'0000);
}

void nubus_lview_device::device_start()
{
	install_declaration_rom("declrom");
	m_vram.resize(VRAM_SIZE / sizeof(u32));

	nubus().install_map(*this, &nubus_lview_device::card_map);

	m_timer = timer_alloc(FUNC(nubus_lview_device::vbl_tick), this);
	m_timer->adjust(screen().time_until_pos(599, 0), 0);
}

void nubus_lview_device::device_reset()
{
	m_vbl_disable = 1;
	m_protstate = 0;
	std::fill(m_vram.begin(), m_vram.end(), 0);
	memset(m_palette, 0, sizeof(m_palette));

	m_palette[0] = rgb_t(255, 255, 255);
	m_palette[0x80] = rgb_t(0, 0, 0);
}


TIMER_CALLBACK_MEMBER(nubus_lview_device::vbl_tick)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	m_timer->adjust(screen().time_until_pos(599, 0), 0);
}

u32 nubus_lview_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]) + 0x20;

	for (int y = 0; y < 600; y++)
	{
		u32 *scanline = &bitmap.pix(y);
		for (int x = 0; x < 832/8; x++)
		{
			u8 const pixels = vram8[(y * (832/8)) + x];

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

	return 0;
}

u8 nubus_lview_device::lview_r(offs_t offset)
{
	u8 rv = 0;

	if (offset == 0xff04)
	{
		return m_screen->vblank();
	}

	if ((m_protstate == 1) || (m_protstate == 10) || (m_protstate == 17) || (m_protstate == 20) || (m_protstate == 22))
	{
		rv = 0x02;
	}

	if (m_protstate == 8)
	{
		rv = 0x01;
	}

	m_protstate++;
	return rv;
}

void nubus_lview_device::lview_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0xffef:
			if (data == 4)
			{
				m_vbl_disable = 0;
			}
			else
			{
				lower_slot_irq();
			}
			break;
	}

	}

void nubus_lview_device::vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
}

u32 nubus_lview_device::vram_r(offs_t offset, u32 mem_mask)
{
	return m_vram[offset];
}

};

DEFINE_DEVICE_TYPE_PRIVATE(PDS030_LVIEW, device_nubus_card_interface, nubus_lview_device, "pd3_lviw", "Sigma Designs L-View")
