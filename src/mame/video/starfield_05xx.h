// license:BSD-3-Clause
// copyright-holders:Robert Hildinger
#ifndef MAME_VIDEO_STARFIELD_05XX_H
#define MAME_VIDEO_STARFIELD_05XX_H

#pragma once

#include "screen.h"

// used by galaga, bosconian, and their various clones
class starfield_05xx_device : public device_t
{
public:
	starfield_05xx_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	void enable_starfield(uint8_t on);
	void set_scroll_speed(uint8_t index_X, uint8_t index_Y);
	void set_active_starfield_sets(uint8_t set_a, uint8_t set_b);
	void set_starfield_config(uint16_t off_X, uint16_t off_Y, uint16_t lim_X);
	void draw_starfield(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint16_t get_next_lfsr_state(uint16_t lfsr);
	
	uint8_t  m_enable;
	uint16_t m_lfsr;
	uint16_t m_pre_vis_cycle_count;
	uint16_t m_post_vis_cycle_count;
	uint8_t  m_starfield_set_a;
	uint8_t  m_starfield_set_b;
	
	uint16_t m_starfield_offset_X;
	uint16_t m_starfield_offset_Y;
	uint16_t m_starfield_limit_X;
};


DECLARE_DEVICE_TYPE(STARFIELD_05XX, starfield_05xx_device)

#endif // MAME_VIDEO_STARFIELD_05XX_H
