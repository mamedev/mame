// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_NAMCO_NAMCOS2_ROZ_H
#define MAME_NAMCO_NAMCOS2_ROZ_H

#pragma once

#include "screen.h"
#include "tilemap.h"


class namcos2_roz_device : public device_t, public device_gfx_interface, public device_video_interface
{
public:
	// construction/destruction
	namcos2_roz_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename T> void set_rozram_tag(T &&tag) { m_rozram.set_tag(std::forward<T>(tag)); }

	void rozram_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// C102 registers
	uint16_t control_r(offs_t offset);
	void control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void draw_roz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t gfx_ctrl, uint8_t prival = 0, uint8_t primask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	// C102 state at the start of a line
	struct line_state
	{
		u32 startx, starty; // accumulators
		s32 incxx, incxy;   // per pixel deltas
		u16 ctrl;           // chip select / mask bits
	};

	TILE_GET_INFO_MEMBER(roz_tile_info);
	TIMER_CALLBACK_MEMBER(line_start);

	tilemap_t *m_tilemap_roz = nullptr;

	required_shared_ptr<uint16_t> m_rozram;
	DECLARE_GFXDECODE_MEMBER(gfxinfo);

	uint16_t m_roz_ctrl[8];
	u32 m_accx;
	u32 m_accy;
	std::unique_ptr<line_state []> m_lines;
	emu_timer *m_line_timer;
};

// device type definition
DECLARE_DEVICE_TYPE(NAMCOS2_ROZ, namcos2_roz_device)

#endif // MAME_NAMCO_NAMCOS2_ROZ_H
