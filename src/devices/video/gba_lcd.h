// license:BSD-3-Clause
// copyright-holders:R. Belmont,Ryan Holtz
/***************************************************************************

    gba_lcd.h

    File to handle emulation of the video hardware of the Game Boy Advance

    By R. Belmont, Ryan Holtz

***************************************************************************/
#ifndef MAME_VIDEO_GBA_LCD_H
#define MAME_VIDEO_GBA_LCD_H

#pragma once

#include "gba_ppu.h"

#include "emupal.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(GBA_LCD, gba_lcd_device)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

template <unsigned COUNT, unsigned BASE>
class gba_registers
{
protected:
	static constexpr unsigned REG_BASE = BASE;

	// 32-bit Register
	uint32_t &WORD(unsigned x) { return m_regs[(x - REG_BASE) / 4]; }
	const uint32_t &WORD(unsigned x) const { return m_regs[(x - REG_BASE) / 4]; }

	// 16-bit Register, Upper Half-Word
	uint16_t HWHI(unsigned x) const { return uint16_t(WORD(x) >> 16); }

	// 16-bit Register, Lower Half-Word
	uint16_t HWLO(unsigned x) const { return uint16_t(WORD(x)); }

	uint32_t &WORD_SET(unsigned x, uint32_t y) { return WORD(x) |= y; }
	uint32_t &HWHI_SET(unsigned x, uint16_t y) { return WORD(x) |= uint32_t(y) << 16; }
	uint32_t &HWLO_SET(unsigned x, uint16_t y) { return WORD(x) |= uint32_t(y); }

	uint32_t &WORD_RESET(unsigned x, uint32_t y) { return WORD(x) &= ~y; }
	uint32_t &HWHI_RESET(unsigned x, uint16_t y) { return WORD(x) &= ~(uint32_t(y) << 16); }
	uint32_t &HWLO_RESET(unsigned x, uint16_t y) { return WORD(x) &= ~uint32_t(y); }

	uint32_t m_regs[COUNT];
};


class gba_lcd_device
		: public gba_ppu_device
		, public device_video_interface
		, public device_palette_interface
{
public:
	gba_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint32_t video_r(offs_t offset, uint32_t mem_mask = ~0);
	void video_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t gba_pram_r(offs_t offset);
	void gba_pram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t gba_vram_r(offs_t offset);
	void gba_vram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t gba_oam_r(offs_t offset);
	void gba_oam_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	auto int_hblank_callback() { return m_int_hblank_cb.bind(); }
	auto int_vblank_callback() { return m_int_vblank_cb.bind(); }
	auto int_vcount_callback() { return m_int_vcount_cb.bind(); }
	auto dma_hblank_callback() { return m_dma_hblank_cb.bind(); }
	auto dma_vblank_callback() { return m_dma_vblank_cb.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual u32 palette_entries() const noexcept override { return 32 * 32 * 32; }

private:
	enum class dispstat : uint16_t
	{
		vblank        = 0x0001,
		hblank        = 0x0002,
		vcount        = 0x0004,
		vblank_irq_en = 0x0008,
		hblank_irq_en = 0x0010,
		vcount_irq_en = 0x0020
	};
	void set(dispstat flag);
	void clear(dispstat flag);
	bool is_set(dispstat flag);

	TIMER_CALLBACK_MEMBER(perform_hbl);
	TIMER_CALLBACK_MEMBER(perform_scan);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void palette_init();

	devcb_write_line m_int_hblank_cb;   /* H-Blank interrupt callback function */
	devcb_write_line m_int_vblank_cb;   /* V-Blank interrupt callback function */
	devcb_write_line m_int_vcount_cb;   /* V-Counter Match interrupt callback function */
	devcb_write_line m_dma_hblank_cb;   /* H-Blank DMA request callback function */
	devcb_write_line m_dma_vblank_cb;   /* V-Blank DMA request callback function */

	std::unique_ptr<uint32_t[]> m_pram;
	std::unique_ptr<uint32_t[]> m_vram;
	std::unique_ptr<uint32_t[]> m_oam;

	emu_timer *m_scan_timer, *m_hbl_timer;

	bitmap_ind16 m_bitmap;

	uint16_t m_dispstat;
};

#endif // MAME_VIDEO_GBA_LCD_H
