// license:BSD-3-Clause
// copyright-holders:Sandro Ronco


#ifndef MAME_MACHINE_SPG290_PPU_H
#define MAME_MACHINE_SPG290_PPU_H

#pragma once

#include "screen.h"


class spg290_ppu_device : public device_t
{
public:
	spg290_ppu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T>
	spg290_ppu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&screen_tag)
		: spg290_ppu_device(mconfig, tag, owner, clock)
	{
		m_screen.set_tag(std::forward<T>(screen_tag));
	}

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	void map(address_map &map) ATTR_COLD;
	auto vblank_irq_cb() { return m_vblank_irq_cb.bind(); }
	auto space_read_cb() { return m_space_read_cb.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint32_t regs_r(offs_t offset, uint32_t mem_mask);
	void regs_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	rgb_t blend_colors(const rgb_t &c0, const rgb_t &c1, uint8_t level);
	void argb1555(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint16_t posy, uint16_t posx, uint16_t argb, uint8_t blend);
	void rgb565(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint16_t posy, uint16_t posx, uint16_t rgb, uint8_t blend);
	void blit_sprite(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint32_t control, uint32_t attribute, uint32_t buf_start);
	void blit_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint32_t control, uint32_t attribute, int posy, int posx, uint32_t nptr, uint32_t buf_start, uint8_t blend);
	void blit_character(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint32_t control, uint32_t attribute, int posy, int posx, uint32_t nptr, uint32_t buf_start, uint8_t blend);

private:
	required_device<screen_device> m_screen;
	required_shared_ptr<uint32_t> m_sprite_palette_ram;
	required_shared_ptr<uint32_t> m_char_palette_ram;
	required_shared_ptr<uint32_t> m_hoffset_ram;
	required_shared_ptr<uint32_t> m_voffset_ram;
	required_shared_ptr<uint32_t> m_sprite_ram;
	devcb_write_line m_vblank_irq_cb;
	devcb_read32 m_space_read_cb;

	uint32_t m_control;
	uint32_t m_sprite_control;
	uint32_t m_irq_control;
	uint32_t m_irq_status;
	uint32_t m_sprite_max;
	uint32_t m_sprite_buf_start;
	uint32_t m_blend_mode;
	uint32_t m_frame_buff[3];
	uint32_t m_transrgb;
	uint32_t m_vcomp_value;
	uint32_t m_vcomp_offset;
	uint32_t m_vcomp_step;
	uint32_t m_irq_timing_v;
	uint32_t m_irq_timing_h;
	uint32_t m_vblank_lines;

	struct spg290_ppu_tx_t
	{
		uint32_t control;
		uint32_t attribute;
		uint32_t posx;
		uint32_t posy;
		uint32_t nptr;
		uint32_t blend;
		uint32_t buf_start[3];
	} m_txs[3];
};

DECLARE_DEVICE_TYPE(SPG290_PPU, spg290_ppu_device)

#endif // MAME_MACHINE_SPG290_PPU_H
