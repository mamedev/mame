// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  RasterOps ColorBoard 264/SE30 video card emulation

  TMS34061 video controller
  Bt473 RAMDAC

***************************************************************************/

#include "emu.h"
#include "pds30_cb264.h"

#include "video/bt47x.h"
#include "video/tms34061.h"

#include "screen.h"

#include <algorithm>

namespace {

static constexpr u32 VRAM_SIZE = 0x200000;

class nubus_cb264se30_device : public device_t,
							   public device_nubus_card_interface
{
public:
	// construction/destruction
	nubus_cb264se30_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	nubus_cb264se30_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	required_device<screen_device> m_screen;
	required_device<bt473_device> m_ramdac;
	required_device<tms34061_device> m_tms34061;

	void card_map(address_map &map);

private:
	u32 tms_r(offs_t offset, u32 mem_mask);
	void tms_w(offs_t offset, u32 data, u32 mem_mask);
	u8 vbl_r();
	u8 mode_r();
	void mode_w(u8 data);
	u32 cb264se30_r(offs_t offset, u32 mem_mask = ~0);
	void cb264se30_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 vram_r(offs_t offset, u32 mem_mask = ~0);
	void vram_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void vblank_w(int state);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	std::unique_ptr<u32[]> m_vram;
	u32 m_mode;
};

ROM_START(cb264se30)
	ROM_REGION(0x8000, "declrom", 0)
	ROM_LOAD( "0002-2019_10-02-90.bin", 0x000000, 0x008000, CRC(5b5b2fab) SHA1(0584deb38b402718f2abef456b0035b34fddb473) )  // EPROM label "264/30 V1.3 0002-2019 10/02/90"
ROM_END

void nubus_cb264se30_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(nubus_cb264se30_device::screen_update));
	m_screen->set_raw(30.24_MHz_XTAL, 864, 0, 640, 525, 0, 480);

	BT473(config, m_ramdac, 0);

	TMS34061(config, m_tms34061, 0);
	m_tms34061->set_rowshift(10); // VRAM address is (row << rowshift) | col
	m_tms34061->set_vram_size(VRAM_SIZE);
	m_tms34061->set_screen(m_screen);
	m_tms34061->int_callback().set(FUNC(nubus_cb264se30_device::vblank_w));
}

const tiny_rom_entry *nubus_cb264se30_device::device_rom_region() const
{
	return ROM_NAME( cb264se30 );
}

nubus_cb264se30_device::nubus_cb264se30_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	nubus_cb264se30_device(mconfig, PDS030_CB264SE30, tag, owner, clock)
{
}

nubus_cb264se30_device::nubus_cb264se30_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_nubus_card_interface(mconfig, *this),
	m_screen(*this, "screen"),
	m_ramdac(*this, "bt473"),
	m_tms34061(*this, "tms34061"),
	m_mode(0)
{
}

void nubus_cb264se30_device::card_map(address_map &map)
{
	map(0x00'0000, 0x1f'ffff).rw(FUNC(nubus_cb264se30_device::vram_r), FUNC(nubus_cb264se30_device::vram_w));
	map(0xfb'0000, 0xfb'00ff).rw(FUNC(nubus_cb264se30_device::tms_r), FUNC(nubus_cb264se30_device::tms_w));
	map(0xfd'0000, 0xfd'0003).r(FUNC(nubus_cb264se30_device::vbl_r)).umask32(0xff000000);
	map(0xfe'0000, 0xfe'0007).m(m_ramdac, FUNC(bt473_device::map)).umask32(0xff00ff00);
	map(0xfe'000c, 0xfe'000f).rw(FUNC(nubus_cb264se30_device::mode_r), FUNC(nubus_cb264se30_device::mode_w)).umask32(0xff000000);
}

void nubus_cb264se30_device::device_start()
{
	install_declaration_rom("declrom");

	m_vram = make_unique_clear<u32[]>(VRAM_SIZE / sizeof(u32));

	nubus().install_map(*this, &nubus_cb264se30_device::card_map);
}

void nubus_cb264se30_device::device_reset()
{
}

u32 nubus_cb264se30_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]) + (8*1024);
	const pen_t *pens = m_ramdac->pens();

	m_tms34061->get_display_state();
	if (m_tms34061->blanked())
	{
		bitmap.fill(rgb_t::black(), cliprect);
		return 0;
	}

	switch (m_mode & 7)
	{
		case 0: // 1 bpp?
			for (int y = 0; y < 480; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/8; x++)
				{
					u8 const pixels = vram8[(y * 1024) + x];

					*scanline++ = pens[(pixels&0x80)];
					*scanline++ = pens[((pixels<<1)&0x80)];
					*scanline++ = pens[((pixels<<2)&0x80)];
					*scanline++ = pens[((pixels<<3)&0x80)];
					*scanline++ = pens[((pixels<<4)&0x80)];
					*scanline++ = pens[((pixels<<5)&0x80)];
					*scanline++ = pens[((pixels<<6)&0x80)];
					*scanline++ = pens[((pixels<<7)&0x80)];
				}
			}
			break;

		case 1: // 2 bpp
			for (int y = 0; y < 480; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/4; x++)
				{
					u8 const pixels = vram8[(y * 1024) + x];

					*scanline++ = pens[(pixels&0xc0)];
					*scanline++ = pens[((pixels<<2)&0xc0)];
					*scanline++ = pens[((pixels<<4)&0xc0)];
					*scanline++ = pens[((pixels<<6)&0xc0)];
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

					*scanline++ = pens[(pixels&0xf0)];
					*scanline++ = pens[((pixels&0x0f)<<4)];
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

		case 7: // 24 bpp
			for (int y = 0; y < 480; y++)
			{
				std::copy_n(&m_vram[y * 1024], 640, &bitmap.pix(y));
			}
			break;

		default:
			fatalerror("cb264se30: unknown video mode %d\n", m_mode);
	}
	return 0;
}

u32 nubus_cb264se30_device::tms_r(offs_t offset, u32 mem_mask)
{
	return m_tms34061->read(offset << 1, 0, 0) << 24;
}

void nubus_cb264se30_device::tms_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (mem_mask == 0xff000000)
	{
		m_tms34061->write(offset << 1, 0, 0, data >> 24);
	}
	else
	{
		logerror("TMS write with unknown mask %08x\n", mem_mask);
	}
}

u8 nubus_cb264se30_device::vbl_r()
{
	return 0;
}

u8 nubus_cb264se30_device::mode_r()
{
	return m_mode;
}

void nubus_cb264se30_device::mode_w(u8 data)
{
	m_mode = data;
}

void nubus_cb264se30_device::vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
}

u32 nubus_cb264se30_device::vram_r(offs_t offset, u32 mem_mask)
{
	return m_vram[offset];
}

void nubus_cb264se30_device::vblank_w(int state)
{
	if (state)
	{
		raise_slot_irq();
	}
	else
	{
		lower_slot_irq();
	}
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(PDS030_CB264SE30, device_nubus_card_interface, nubus_cb264se30_device, "pd3_c264", "RasterOps ColorBoard 264/SE30")
