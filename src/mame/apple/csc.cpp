// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple Color Screen Controller (CSC) video, aka Chips & Technologies 65220
    Emulation by R. Belmont

    References:
    https://github.com/elliotnunn/supermario/blob/master/base/SuperMarioProj.1994-02-09/Internal/Asm/HardwarePrivateEqu.a#L1220
    https://github.com/elliotnunn/supermario/tree/master/base/SuperMarioProj.1994-02-09/DeclData/DeclVideo/CSC

    For Escher (PowerBook Duo 270C), panel 0 is the only option.
    For Yeager (PowerBook Duo280/280C), panel 0 is color 640x480, panel 4 is TFT grayscale, and panel 6 is STN grayscale
    For Blackbird (Powerbook 5xx), panel 0 is Sharp color TFT, 1 is color STN, 2 is NEC color TFT, 3 is Hosiden color TFT,
             4 is Toshiba color TFT, 5 is Sharp grayscale STN, and 6 is Hosiden grayscale TFT
*/

#include "emu.h"
#include "csc.h"

DEFINE_DEVICE_TYPE(CSC, csc_device, "applecsc", "Apple Color Screen Controller video")

static constexpr u8 CSC_PANEL_ID        = 0x02;
static constexpr u8 CSC_PANEL_SETUP     = 0x08;
static constexpr u8 CSC_DISPLAY_FORMAT  = 0x12;
static constexpr u8 CSC_DISPLAY_STATUS  = 0x14;
static constexpr u8 CSC_ADDR_REG_W      = 0x40;
static constexpr u8 CSC_DATA_REG        = 0x42;
static constexpr u8 CSC_ADDR_REG_R      = 0x46;

static constexpr u8 CSC_DISPSTAT_IRQ_ENABLE = 0;
static constexpr u8 CSC_DISPSTAT_IRQ        = 1;

static constexpr u8 CSC_PANELSETUP_480      = 1;
static constexpr u8 CSC_PANELSETUP_COLOR    = 2;
static constexpr u8 CSC_PANELSETUP_ENABLED  = 3;

// mask to determine a color 480-tall panel in 400-tall mode to fit 15bpp in 512K
static constexpr u8 CSC_PANELSETUP_COLOR400_MASK = (1 << CSC_PANELSETUP_480) | (1 << CSC_PANELSETUP_COLOR);

void csc_device::map(address_map & map)
{
	map(0x00f20000, 0x00f2007f).rw(FUNC(csc_device::csc_r), FUNC(csc_device::csc_w));
	map(0x10000000, 0x10ffffff).rw(FUNC(csc_device::vram_r), FUNC(csc_device::vram_w));
}

csc_device::csc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, CSC, tag, owner, clock),
	m_screen(*this, "screen"),
	m_palette(*this, "palette"),
	m_irq(*this),
	m_csc_panel_id(0),
	m_pal_idx(0),
	m_pmu_blank_display(true)
{
	std::fill(std::begin(m_csc_regs), std::end(m_csc_regs), 0);
}

void csc_device::device_start()
{
	m_vram = std::make_unique<u32[]>(0x80000);

	save_item(NAME(m_csc_regs));
	save_item(NAME(m_pal_idx));
	save_item(NAME(m_pmu_blank_display));
	save_pointer(NAME(m_vram), 0x80000);
}

void csc_device::device_reset()
{
	if (m_csc_panel_id >= 4)
	{
		m_screen->set_raw(21604953, 800, 0, 640, 449, 0, 400);
	}
}

void csc_device::device_add_mconfig(machine_config &config)
{
	PALETTE(config, m_palette).set_entries(256);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(25175000, 800, 0, 640, 525, 0, 480);
	m_screen->set_screen_update(FUNC(csc_device::screen_update_csc));
	m_screen->screen_vblank().set(FUNC(csc_device::csc_irq_w));
}

void csc_device::set_panel_id(int panel_id)
{
	m_csc_panel_id = panel_id;
}

u8 csc_device::csc_r(offs_t offset)
{
	if (offset == CSC_PANEL_ID)
	{
		return m_csc_panel_id;
	}
	else if (offset == CSC_DATA_REG)
	{
		u8 component = 0;
		switch (m_pal_idx)
		{
		case 0:
			component = m_palette->pen_color(m_csc_regs[CSC_ADDR_REG_R]).r();
			break;
		case 1:
			component = m_palette->pen_color(m_csc_regs[CSC_ADDR_REG_R]).g();
			break;
		case 2:
			component = m_palette->pen_color(m_csc_regs[CSC_ADDR_REG_R]).b();
			break;
		}
		m_pal_idx++;
		if (m_pal_idx == 3)
		{
			m_pal_idx = 0;
			m_csc_regs[CSC_ADDR_REG_R]++;
		}

		return component;
	}

	return m_csc_regs[offset];
}

void csc_device::csc_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case CSC_DISPLAY_STATUS:
		if (BIT(data, CSC_DISPSTAT_IRQ))
		{
			m_irq(CLEAR_LINE);
			data = m_csc_regs[offset] & ~(1 << CSC_DISPSTAT_IRQ);
		}
		break;

	case CSC_DISPLAY_FORMAT:
		// if this is a monochrome panel, setup the palette correctly for the mode
		if (!BIT(m_csc_regs[CSC_PANEL_SETUP], CSC_PANELSETUP_COLOR))
		{
			switch (data)
			{
				case 0:
					m_palette->set_pen_color(0, rgb_t(255, 255, 255));
					m_palette->set_pen_color(0x80, rgb_t(0, 0, 0));
					break;

				case 1:
					m_palette->set_pen_color(0, rgb_t(255, 255, 255));
					m_palette->set_pen_color(0x40, rgb_t(192, 192, 192));
					m_palette->set_pen_color(0x80, rgb_t(64, 64, 64));
					m_palette->set_pen_color(0xc0, rgb_t(0, 0, 0));
					break;

				case 2:
					for (int i = 15; i >= 0; i--)
					{
						m_palette->set_pen_red_level(i<<4, (15 - i) << 4);
						m_palette->set_pen_green_level(i<<4, (15 - i) << 4);
						m_palette->set_pen_blue_level(i<<4, (15 - i) << 4);
					}
					break;

				case 3:
					for (int i = 255; i >= 0; i--)
					{
						m_palette->set_pen_red_level(i, 255 - i);
						m_palette->set_pen_green_level(i, 255 - i);
						m_palette->set_pen_blue_level(i, 255 - i);
					}
					break;
			}
		}
		break;

	case CSC_ADDR_REG_W: // write CLUT index
		m_pal_idx = 0;
		break;

	case CSC_DATA_REG:
		switch (m_pal_idx)
		{
			case 0:
				m_palette->set_pen_red_level(m_csc_regs[CSC_ADDR_REG_W], data);
				break;
			case 1:
				m_palette->set_pen_green_level(m_csc_regs[CSC_ADDR_REG_W], data);
				break;
			case 2:
				m_palette->set_pen_blue_level(m_csc_regs[CSC_ADDR_REG_W], data);
				break;
		}
		m_pal_idx++;
		if (m_pal_idx == 3)
		{
			m_pal_idx = 0;
			m_csc_regs[CSC_ADDR_REG_W]++;
		}
		break;

	case CSC_ADDR_REG_R: // read CLUT index
		m_pal_idx = 0;
		break;
	}

	m_csc_regs[offset] = data;
}

void csc_device::csc_irq_w(int state)
{
	if (state == ASSERT_LINE)
	{
		m_csc_regs[CSC_DISPLAY_STATUS] |= (1 << CSC_DISPSTAT_IRQ);
		if (BIT(m_csc_regs[CSC_DISPLAY_STATUS], 0))
		{
			m_irq(ASSERT_LINE);
		}
	}
}

u32 csc_device::screen_update_csc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// is the display enabled?
	if (!BIT(m_csc_regs[CSC_PANEL_SETUP], CSC_PANELSETUP_ENABLED))
	{
		bitmap.fill(0xf, cliprect);
		return 0;
	}

	const auto vram8 = util::big_endian_cast<u8 const>(&m_vram[0]);
	const auto vram16 = util::big_endian_cast<u16 const>(&m_vram[0]);
	const pen_t *pens = m_palette->pens();
	const int vres = BIT(m_csc_regs[CSC_PANEL_SETUP], CSC_PANELSETUP_480) ? 480 : 400;
	const int dispoffs = ((m_csc_regs[CSC_PANEL_SETUP] & CSC_PANELSETUP_COLOR400_MASK) == (1 << CSC_PANELSETUP_COLOR)) ? 40 : 0;

	if (dispoffs)
	{
		bitmap.fill(0xf, cliprect);
	}

	switch (m_csc_regs[CSC_DISPLAY_FORMAT])
	{
		case 0:
		{
			for (int y = 0; y < vres; y++)
			{
				u32 *scanline = &bitmap.pix(y + dispoffs);
				for (int x = 0; x < 640; x += 8)
				{
					u8 const pixels = vram8[(y * 80) + (x / 8)];

					*scanline++ = pens[pixels & 0x80];
					*scanline++ = pens[(pixels << 1) & 0x80];
					*scanline++ = pens[(pixels << 2) & 0x80];
					*scanline++ = pens[(pixels << 3) & 0x80];
					*scanline++ = pens[(pixels << 4) & 0x80];
					*scanline++ = pens[(pixels << 5) & 0x80];
					*scanline++ = pens[(pixels << 6) & 0x80];
					*scanline++ = pens[(pixels << 7) & 0x80];
				}
			}
		}
		break;

		case 1: // 2bpp
		{
			for (int y = 0; y < vres; y++)
			{
				u32 *scanline = &bitmap.pix(y + dispoffs);
				for (int x = 0; x < 640 / 4; x++)
				{
					u8 const pixels = vram8[(y * 160) + x];

					*scanline++ = pens[(pixels & 0xc0)];
					*scanline++ = pens[((pixels << 2) & 0xc0)];
					*scanline++ = pens[((pixels << 4) & 0xc0)];
					*scanline++ = pens[((pixels << 6) & 0xc0)];
				}
			}
		}
		break;

		case 2: // 4bpp
		{
			for (int y = 0; y < vres; y++)
			{
				u32 *scanline = &bitmap.pix(y + dispoffs);

				for (int x = 0; x < 640 / 2; x++)
				{
					u8 const pixels = vram8[(y * 320) + x];
					*scanline++ = pens[(pixels & 0xf0)];
					*scanline++ = pens[((pixels << 4) & 0xf0)];
				}
			}
		}
		break;

		case 3: // 8bpp
		{
			for (int y = 0; y < vres; y++)
			{
				u32 *scanline = &bitmap.pix(y + dispoffs);

				for (int x = 0; x < 640; x++)
				{
					u8 const pixels = vram8[(y * 640) + x];
					*scanline++ = pens[pixels];
				}
			}
		}
		break;

		case 4: // 16bpp
			{
				for (int y = 0; y < vres; y++)
				{
					u32 *scanline = &bitmap.pix(y + dispoffs);
					for (int x = 0; x < 640; x++)
					{
						u16 const pixels = vram16[(y * 640) + x];
						*scanline++ = rgb_t(((pixels >> 10) & 0x1f) << 3, ((pixels >> 5) & 0x1f) << 3, (pixels & 0x1f) << 3);
					}
				}
			}
			break;
	}

	return 0;
}

u32 csc_device::vram_r(offs_t offset)
{
	return m_vram[offset & 0x7ffff];
}

void csc_device::vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_vram[offset & 0x7ffff]);
}
