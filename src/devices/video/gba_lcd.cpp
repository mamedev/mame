// license:BSD-3-Clause
// copyright-holders:R. Belmont,Ryan Holtz
/***************************************************************************

    gba_lcd.cpp

    File to handle emulation of the video hardware of the Game Boy Advance

    By R. Belmont, Ryan Holtz

***************************************************************************/

#include "emu.h"
#include "gba_lcd.h"

#include "screen.h"


DEFINE_DEVICE_TYPE(GBA_LCD, gba_lcd_device, "gba_lcd", "GBA LCD")

gba_lcd_device::gba_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gba_ppu_device(mconfig, GBA_LCD, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, device_palette_interface(mconfig, *this)
	, m_int_hblank_cb(*this)
	, m_int_vblank_cb(*this)
	, m_int_vcount_cb(*this)
	, m_dma_hblank_cb(*this)
	, m_dma_vblank_cb(*this)
	, m_scan_timer(nullptr)
	, m_hbl_timer(nullptr)
	, m_dispstat(0)
{
}

inline void gba_lcd_device::set(dispstat flag)
{
	m_dispstat |= underlying_value(flag);
}

inline void gba_lcd_device::clear(dispstat flag)
{
	m_dispstat &= ~underlying_value(flag);
}

inline bool gba_lcd_device::is_set(dispstat flag)
{
	return m_dispstat & underlying_value(flag);
}

uint32_t gba_lcd_device::video_r(offs_t offset, uint32_t mem_mask)
{
	if (offset == (0x004 / 4))
	{
		return m_dispstat | (screen().vpos() << 16);
	}

	return regs_r(offset);
}

void gba_lcd_device::video_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset == (0x004 / 4))
	{
		// the status flags are read-only, and so is VCOUNT
		if (ACCESSING_BITS_0_15)
		{
			m_dispstat = (m_dispstat & 0x0007) | (data & 0xfff8);
		}
		return;
	}

	regs_w(offset, data, mem_mask);
}

uint32_t gba_lcd_device::gba_pram_r(offs_t offset)
{
	return m_pram[offset];
}

void gba_lcd_device::gba_pram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pram[offset]);
}

uint32_t gba_lcd_device::gba_vram_r(offs_t offset)
{
	return m_vram[offset];
}

void gba_lcd_device::gba_vram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
}

uint32_t gba_lcd_device::gba_oam_r(offs_t offset)
{
	return m_oam[offset];
}

void gba_lcd_device::gba_oam_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_oam[offset]);
}

TIMER_CALLBACK_MEMBER(gba_lcd_device::perform_hbl)
{
	int scanline = screen().vpos();

	// draw only visible scanlines
	if (scanline < 160)
	{
		draw_scanline(scanline, &m_bitmap.pix(scanline));
		m_dma_hblank_cb(ASSERT_LINE);
	}

	if (is_set(dispstat::hblank_irq_en))
	{
		m_int_hblank_cb(ASSERT_LINE);
	}

	set(dispstat::hblank);

	m_hbl_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(gba_lcd_device::perform_scan)
{
	clear(dispstat::hblank);
	clear(dispstat::vcount);

	int scanline = screen().vpos();

	// VBLANK is set for scanlines 160 through 226 (but not 227, which is the last line)
	if (scanline >= 160 && scanline < 227)
	{
		set(dispstat::vblank);

		// VBL IRQ and DMA on line 160
		if (scanline == 160)
		{
			if (is_set(dispstat::vblank_irq_en))
				m_int_vblank_cb(ASSERT_LINE);

			m_dma_vblank_cb(ASSERT_LINE);
		}
	}
	else
	{
		clear(dispstat::vblank);
	}

	// handle VCOUNT match interrupt flag
	if (scanline == ((m_dispstat >> 8) & 0xff))
	{
		set(dispstat::vcount);

		if (is_set(dispstat::vcount_irq_en))
			m_int_vcount_cb(ASSERT_LINE);
	}

	m_hbl_timer->adjust(screen().time_until_pos(scanline, 240));
	m_scan_timer->adjust(screen().time_until_pos((scanline + 1) % 228, 0));
}

void gba_lcd_device::palette_init()
{
	for (uint8_t b = 0; b < 32; b++)
	{
		for (uint8_t g = 0; g < 32; g++)
		{
			for (uint8_t r = 0; r < 32; r++)
			{
				set_pen_color((b << 10) | (g << 5) | r, pal5bit(r), pal5bit(g), pal5bit(b));
			}
		}
	}
}

uint32_t gba_lcd_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}

void gba_lcd_device::device_start()
{
	gba_ppu_device::device_start();

	palette_init();

	m_pram = make_unique_clear<uint32_t[]>(0x400 / 4);
	m_vram = make_unique_clear<uint32_t[]>(0x18000 / 4);
	m_oam = make_unique_clear<uint32_t[]>(0x400 / 4);

	// the engine fetches from 64K of BG data followed by 32K of OBJ tiles
	set_palette_ram(m_pram.get());
	set_oam(m_oam.get());
	for (int page = 0; page < 4; page++)
	{
		set_vram_page(VRAM_BG, page, &m_vram[page * (0x4000 / 4)]);
	}
	
	for (int page = 0; page < 2; page++)
	{
		set_vram_page(VRAM_OBJ, page, &m_vram[(0x10000 / 4) + (page * (0x4000 / 4))]);
	}

	screen().register_screen_bitmap(m_bitmap);

	/* create a timer to fire scanline functions */
	m_scan_timer = timer_alloc(FUNC(gba_lcd_device::perform_scan), this);
	m_hbl_timer = timer_alloc(FUNC(gba_lcd_device::perform_hbl), this);
	m_scan_timer->adjust(screen().time_until_pos(0, 0));

	save_pointer(NAME(m_pram), 0x400 / 4);
	save_pointer(NAME(m_vram), 0x18000 / 4);
	save_pointer(NAME(m_oam), 0x400 / 4);

	save_item(NAME(m_dispstat));
}

void gba_lcd_device::device_reset()
{
	gba_ppu_device::device_reset();

	m_dispstat = 0;

	m_scan_timer->adjust(screen().time_until_pos(0, 0));
	m_hbl_timer->adjust(attotime::never);
}

void gba_lcd_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen").set_lcd());
	screen.set_raw(XTAL(16'777'216) / 4, 308, 0, 240, 228, 0, 160);
	screen.set_screen_update(FUNC(gba_lcd_device::screen_update));
	screen.set_palette(*this);
}
