// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * SGI IRIS GL1 high-level emulation
 *
 * This device is an interim high-level emulation of a combination of GF1, UC3,
 * DC3 and BP2 boards. Currently, it supports the limited subset of functions
 * used by the IRIS 1400 firmware to display the monitor. When the hardware is
 * better understood and more software becomes usable, this device should be
 * replaced by individual Multibus devices emulating each of the four cards
 * mentioned earlier.
 *
 * Sources:
 *  - IRIS 3.7 source code
 *
 * TODO:
 *  - everything
 */

#include "emu.h"

#include "gl1.h"

#include "emupal.h"
#include "screen.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

namespace {

class sgi_gl1_device
	: public device_t
	, public device_multibus_interface
{
public:
	sgi_gl1_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: device_t(mconfig, SGI_GL1, tag, owner, clock)
		, device_multibus_interface(mconfig, *this)
		, m_screen(*this, "screen")
		, m_cmap(*this, "cmap")
		, m_font(*this, "font", 0x8000, ENDIANNESS_BIG)
		, m_bp(nullptr)
		, m_installed(false)
	{
	}

	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// GF1
	u16 fbc_readdpx_r();
	void fbc_clrint_w(u16 data);
	u16 fbc_flags_r();
	void fbc_flags_w(u16 data);
	u16 fbc_data_r();
	void fbc_data_w(u16 data);
	void ge_flags_w(u16 data);

	// DC3
	void flags_w(u16 data);
	template <unsigned Channel> void colormap_w(offs_t offset, u16 data, u16 mem_mask);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	// BP2
	void bp_w(offs_t offset, u8 data, u8 mask);

private:
	required_device<screen_device> m_screen;
	required_device<palette_device> m_cmap;
	memory_share_creator<u16> m_font;

	std::unique_ptr<u8[]> m_bp;

	//u16 m_ed; // error delta
	//u16 m_ec; // error correct
	u16 m_xs; // x start
	u16 m_xe; // x end
	u16 m_ys; // y start
	u16 m_ye; // y end
	u16 m_fma; // font memory address
	//u16 m_cf; // config

	u16 m_ch; // character height
	u16 m_cw; // character width
	u16 m_cs; // character spacing

	u16 m_ca; // cursor address
	u16 m_cm; // cursor color
	u16 m_cx; // cursor x
	u16 m_cy; // cursor y

	u16 m_color;
	u16 m_wrten;

	u16 m_flags;

	util::fifo<u16, 16> m_params;
	u8 m_expect;

	bool m_installed;
};

void sgi_gl1_device::device_start()
{
	m_bp = std::make_unique<u8[]>(0x10'0000); // 1024x1024x4 (not packed)

	save_pointer(NAME(m_bp), 0x10'0000);
}

void sgi_gl1_device::device_reset()
{
	if (!m_installed)
	{
		// GF1 handlers
		m_bus->space(AS_IO).install_readwrite_handler(0x1000, 0x1001,
			emu::rw_delegate(*this, FUNC(sgi_gl1_device::fbc_readdpx_r)),
			emu::rw_delegate(*this, FUNC(sgi_gl1_device::fbc_clrint_w)));

		m_bus->space(AS_IO).install_readwrite_handler(0x1400, 0x1401,
			emu::rw_delegate(*this, FUNC(sgi_gl1_device::fbc_flags_r)),
			emu::rw_delegate(*this, FUNC(sgi_gl1_device::fbc_flags_w)));

		m_bus->space(AS_IO).install_readwrite_handler(0x1800, 0x1801,
			emu::rw_delegate(*this, FUNC(sgi_gl1_device::fbc_data_r)),
			emu::rw_delegate(*this, FUNC(sgi_gl1_device::fbc_data_w)));

		m_bus->space(AS_IO).install_write_handler(0x1c00, 0x1c01,
			emu::rw_delegate(*this, FUNC(sgi_gl1_device::ge_flags_w)));

		// DC3 handlers
		m_bus->space(AS_IO).install_write_handler(0x4000, 0x4001, emu::rw_delegate(*this, FUNC(sgi_gl1_device::flags_w)));
		m_bus->space(AS_IO).install_write_handler(0x4200, 0x43ff, emu::rw_delegate(*this, FUNC(sgi_gl1_device::colormap_w<0>)));
		m_bus->space(AS_IO).install_write_handler(0x4400, 0x45ff, emu::rw_delegate(*this, FUNC(sgi_gl1_device::colormap_w<1>)));
		m_bus->space(AS_IO).install_write_handler(0x4600, 0x47ff, emu::rw_delegate(*this, FUNC(sgi_gl1_device::colormap_w<2>)));

		m_installed = true;
	}

	m_params.clear();
	m_expect = 0;
}

static const gfx_layout gl1_layout =
{
	8, 16, 1024, 1,
	{ 0 },
	{ 8, 9, 10, 11, 12, 13, 14, 15 } ,
	{ 15 * 16, 14 * 16, 13 * 16, 12 * 16, 11 * 16, 10 * 16, 9 * 16, 8 * 16, 7 * 16, 6 * 16, 5 * 16, 4 * 16, 3 * 16, 2 * 16, 1 * 16, 0 * 16 },
	16 * 16
};

static GFXDECODE_START(gl1_gfx)
	GFXDECODE_RAM("font", 0x0, gl1_layout, 0, 1)
GFXDECODE_END

void sgi_gl1_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(1024 * 768 * 60, 1024, 0, 1024, 768, 0, 768);
	m_screen->set_screen_update(FUNC(sgi_gl1_device::screen_update));

	// MCM6168P45 x8x3 (4096x4 SRAM) 16K x3
	PALETTE(config, m_cmap);
	m_cmap->set_entries(4096);

	// gfxdecode is only to show the font data in the tile viewer
	GFXDECODE(config, "gfx", m_cmap, gl1_gfx);
}

u32 sgi_gl1_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	u8 const *src = m_bp.get();

	for (unsigned y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
	{
		for (unsigned x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x++)
		{
			bitmap.pix(screen.visible_area().max_y - y, x) = m_cmap->pen_color(*src++);
		}
	}

	return 0;
}

u16 sgi_gl1_device::fbc_readdpx_r()
{
	return 0;
}

void sgi_gl1_device::fbc_clrint_w(u16 data)
{
	LOG("%s: fbc_clrint_w 0x%04x\n", machine().describe_context(), data);
}

u16 sgi_gl1_device::fbc_flags_r()
{
	return 0x0002;
}

void sgi_gl1_device::fbc_flags_w(u16 data)
{
	//LOG("%s: fbc_flags_w 0x%04x\n", machine().describe_context(), data);
}

static const unsigned num_params[] =
{
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 4, 0, 2, 2, 1, 8,
	8, 0, 0, 0, 0, 6, 4, 1,

	0, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 3, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 8, 0, 0, 0, 0, 0, 0,
};

u16 sgi_gl1_device::fbc_data_r()
{
	return 0x40; // HACK: always ready
}

void sgi_gl1_device::fbc_data_w(u16 data)
{
	LOG("%s: fbc_data_w 0x%04x expect %u\n", machine().describe_context(), data, m_expect);

	if (m_params.empty())
	{
		// enqueue command
		m_params.enqueue(data);

		m_expect = num_params[data & 0x3f];
	}
	else if (m_expect)
	{
		// enqueue parameter
		m_params.enqueue(data);
		m_expect--;
	}

	// more parameters expected?
	if (m_expect)
		return;

	u8 const command = m_params.dequeue();
	LOG("command 0x%02x params %u\n", command, m_params.queue_length());

	switch (command & 0x3f)
	{
	case 0x12: // point
		m_xs = m_params.dequeue() & 0xfffU;
		m_params.dequeue();
		m_params.dequeue();
		m_ys = m_params.dequeue() & 0xfffU;
		break;

	case 0x14: // color
		m_color = m_params.dequeue();
		m_color &= m_params.dequeue();
		break;
	case 0x15: // wrten
		m_wrten = m_params.dequeue();
		m_wrten |= m_params.dequeue();
		break;
	//case 0x16: // config
	case 0x17: // loadmasks (passthrough)
		m_fma = m_params.dequeue();
		while (!m_params.empty())
			m_font[m_fma++ & 0x3fff] = m_params.dequeue();
		break;
	//case 0x18: // fbviewport

	case 0x1d: // selectcursor
		m_ca = m_params.dequeue();
		m_cm = m_params.dequeue();
		m_params.dequeue();
		m_params.dequeue();
		m_params.dequeue();
		m_params.dequeue();
		break;
	case 0x1e: // drawcursor
		m_cx = m_params.dequeue() & 0xfffU;
		m_params.dequeue();
		m_params.dequeue();
		m_cy = m_params.dequeue() & 0xfffU;

		for (int y = 0; y <= m_ch; y++)
		{
			u8 const data = m_font[(m_ca + (y & 0xfU)) & 0x3fff];

			for (int x = 0; x <= m_cw; x++)
				bp_w((m_cy + y) * 0x400 + m_cx + x, BIT(data, (x & 7) ^ 7) ? m_cm : 0, m_cm);
		}
		break;
	case 0x1f: // undrawcursor
		m_params.dequeue();
		for (int y = 0; y <= m_ch; y++)
			for (int x = 0; x <= m_cw; x++)
				bp_w((m_cy + y) * 0x400 + m_cx + x, 0, m_cm);
		break;

	//case 0x21: // polystipple

	case 0x2c: // fixcharload
		m_ch = m_params.dequeue();
		m_cw = m_params.dequeue();
		m_cs = m_params.dequeue();
		break;
	case 0x2d: // fixchardraw
		m_fma = m_params.dequeue();
		for (int y = 0; y <= m_ch; y++)
		{
			u8 const data = m_font[(m_fma + (y & 0xfU)) & 0x3fff];

			for (int x = 0; x <= m_cw; x++)
				bp_w((m_ys + y) * 0x400 + m_xs + x, BIT(data, (x & 7) ^ 7) ? m_color : 0, m_wrten);
		}
		m_xs = (m_xs + m_cs) & 0x0fff;
		break;

	case 0x39: // blockfill
		m_xs = m_params.dequeue() & 0x0fff;
		m_params.dequeue();
		m_params.dequeue();
		m_ys = m_params.dequeue() & 0x0fff;
		m_xe = m_params.dequeue() & 0x0fff;
		m_params.dequeue();
		m_params.dequeue();
		m_ye = m_params.dequeue() & 0x0fff;

		for (int y = m_ys; y < m_ye; y++)
			for (int x = m_xs; x < m_xe; x++)
				bp_w(y * 0x400 + x, m_color, m_wrten);
		break;

	default:
		LOG("command 0x%02x unemulated\n", command);
		m_params.clear();
		break;
	}
}

void sgi_gl1_device::ge_flags_w(u16 data)
{
	LOG("%s: ge_flags_w 0x%04x\n", machine().describe_context(), data);
}

void sgi_gl1_device::flags_w(u16 data)
{
	LOG("%s: flags_w 0x%04x\n", machine().describe_context(), data);

	m_flags = data;
}

template <unsigned Channel> void sgi_gl1_device::colormap_w(offs_t offset, u16 data, u16 mem_mask)
{
	unsigned const index = BIT(m_flags, 4, 4) << 8 | offset;

	switch (Channel)
	{
	case 0: m_cmap->set_pen_red_level(index, data); break;
	case 1: m_cmap->set_pen_green_level(index, data); break;
	case 2: m_cmap->set_pen_blue_level(index, data); break;
	}
}

void sgi_gl1_device::bp_w(offs_t offset, u8 data, u8 mask)
{
	m_bp[offset] = (m_bp[offset] & ~mask) | (data & mask);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(SGI_GL1, device_multibus_interface, sgi_gl1_device, "sgi_gl1", "Silicon Graphics GL1")
