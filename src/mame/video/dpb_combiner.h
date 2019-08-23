// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    dpb_combiner.h
    DPB-7000/1 - Combiner Card

***************************************************************************/

#ifndef MAME_VIDEO_DPB_COMBINER_H
#define MAME_VIDEO_DPB_COMBINER_H

#pragma once

#include "machine/tmc208k.h"
#include "screen.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dpb7000_combiner_card_device

class dpb7000_combiner_card_device : public device_t
{
public:
	// construction/destruction
	dpb7000_combiner_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void reg_w(uint16_t data);
	void lum1_w(uint8_t data);
	void lum2_w(uint8_t data);
	void blank1(int state);
	void blank2(int state);
	void chr1_w(uint8_t data);
	void chr2_w(uint8_t data);
	void chr_flag_w(int state);
	void ext1_w(uint8_t data);
	void ext2_w(uint8_t data);
	void palette_l_w(int state);
	void cursor_enb_w(int state);
	void cursor_col_w(int state);

	auto lum() { return m_lum.bind(); }
	auto chr() { return m_chr.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	static constexpr device_timer_id FSCK_TIMER = 0;

	void fsck_tick();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void update_matte_selects();

	uint8_t m_lum_in[2];
	uint8_t m_latched_lum[2];
	uint8_t m_selected_lum[2];
	uint8_t m_chr_in[2];
	uint8_t m_latched_chr[2];
	uint8_t m_selected_chr[2];
	uint8_t m_ext_in[2];
	uint8_t m_selected_ext[2];
	uint8_t m_latched_lum_sum;
	uint8_t m_latched_chr_sum;
	uint8_t m_lum_out;
	uint8_t m_chr_out;

	bool m_blank[2];
	bool m_chr_i_in;
	bool m_chr_i;
	bool m_palette_l;
	bool m_cursor_enb;
	bool m_cursor_col;
	bool m_fsck;

	uint8_t m_cursor_y;
	uint8_t m_cursor_u;
	uint8_t m_cursor_v;
	uint8_t m_invert_mask;
	bool m_select_matte[2];
	uint8_t m_matte_ext[2];
	uint8_t m_matte_y[2];
	uint8_t m_matte_u[2];
	uint8_t m_matte_v[2];

	bool m_blank_or_suppress[2]; // FH
	bool m_output_matte_y[2]; // EB
	bool m_output_matte_u[2]; // EG
	bool m_output_matte_v[2]; // EG
	bool m_output_matte_ext[2]; // FA

	devcb_write8 m_lum;
	devcb_write8 m_chr;

	emu_timer *m_fsck_timer;

	required_device<screen_device> m_screen;
	required_device<tmc28ku_device> m_mult_ge;
	required_device<tmc28ku_device> m_mult_gd;
	required_device<tmc28ku_device> m_mult_gc;
	required_device<tmc28ku_device> m_mult_gb;
	required_device<tmc28ku_device> m_mult_ga;
};

// device type definition
DECLARE_DEVICE_TYPE(DPB7000_COMBINER, dpb7000_combiner_card_device)

#endif // MAME_VIDEO_DPB_COMBINER_H
