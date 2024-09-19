// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Olivier Galibert
/*************************************************************************

    Taito Air System

*************************************************************************/
#ifndef MAME_TAITO_TAITOAIR_H
#define MAME_TAITO_TAITOAIR_H

#pragma once

#include "cpu/tms32025/tms32025.h"
#include "taitoio.h"
#include "taitoio_yoke.h"
#include "tc0080vco.h"
#include "emupal.h"
#include "screen.h"

enum { TAITOAIR_FRAC_SHIFT = 16, TAITOAIR_POLY_MAX_PT = 16 };

struct taitoair_spoint {
	s32 x = 0, y = 0;
};

struct taitoair_poly {
	struct taitoair_spoint p[TAITOAIR_POLY_MAX_PT];
	int pcount = 0;
	u16 header = 0;
};


class taitoair_state : public driver_device
{
public:
	taitoair_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_m68000_mainram(*this, "m68000_mainram")
		, m_line_ram(*this, "line_ram")
		, m_dsp_ram(*this, "dsp_ram")
		, m_paletteram(*this, "paletteram")
		, m_gradram(*this, "gradram")
		, m_tc0430grw(*this, "tc0430grw")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_dsp(*this, "dsp")
		, m_tc0080vco(*this, "tc0080vco")
		, m_tc0220ioc(*this, "tc0220ioc")
		, m_yoke(*this, "yokectrl")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_z80bank(*this, "z80bank")
	{ }

	void airsys(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<u16> m_m68000_mainram;
	required_shared_ptr<u16> m_line_ram;
	required_shared_ptr<u16> m_dsp_ram;          // Shared 68000/TMS32025 RAM
	required_shared_ptr<u16> m_paletteram;
	required_shared_ptr<u16> m_gradram;
	required_shared_ptr<u16> m_tc0430grw;

	/* video-related */
	taitoair_poly  m_q;

	/* misc */
	int           m_dsp_hold_signal = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<tms32025_device> m_dsp;
	required_device<tc0080vco_device> m_tc0080vco;
	required_device<tc0220ioc_device> m_tc0220ioc;
	required_device<taitoio_yoke_device> m_yoke;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_memory_bank m_z80bank;

	std::unique_ptr<bitmap_ind16> m_framebuffer[2];

	/* 3d info */
	s16 m_frustumLeft = 0;
	s16 m_frustumBottom = 0;
	s16 m_eyecoordBuffer[4]{};  /* homogeneous */

	bool m_gradbank = false;

	u16 m_dsp_test_object_type = 0;
	s16 m_dsp_test_or_clip = 0, m_dsp_test_and_clip = 0;
	s16 m_dsp_test_x = 0, m_dsp_test_y = 0, m_dsp_test_z = 0;

	void dsp_test_start_w(u16 data);
	void dsp_test_x_w(u16 data);
	void dsp_test_y_w(u16 data);
	void dsp_test_z_w(u16 data);
	u16 dsp_test_point_r();
	u16 dsp_test_or_clip_r();
	u16 dsp_test_and_clip_r();

	s16 m_dsp_muldiv_a_1 = 0, m_dsp_muldiv_b_1 = 0, m_dsp_muldiv_c_1 = 0;

	void dsp_muldiv_a_1_w(u16 data);
	void dsp_muldiv_b_1_w(u16 data);
	void dsp_muldiv_c_1_w(u16 data);
	u16 dsp_muldiv_1_r();

	s16 m_dsp_muldiv_a_2 = 0, m_dsp_muldiv_b_2 = 0, m_dsp_muldiv_c_2 = 0;

	void dsp_muldiv_a_2_w(u16 data);
	void dsp_muldiv_b_2_w(u16 data);
	void dsp_muldiv_c_2_w(u16 data);
	u16 dsp_muldiv_2_r();

	//bitmap_ind16 *m_buffer3d;
	void system_control_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 lineram_r(offs_t offset);
	void lineram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 dspram_r(offs_t offset);
	void dspram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 dsp_HOLD_signal_r();
	void dsp_HOLDA_signal_w(offs_t offset, u16 data);
	void paletteram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void gradram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 stick_input_r(offs_t offset);
	u16 stick2_input_r(offs_t offset);
	void coin_control_w(u8 data);
	void sound_bankswitch_w(u8 data);
	void dsp_flags_w(offs_t offset, u16 data);
	void dma_regs_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	int draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int start_offset);
	void fb_copy_op(void);
	void fb_fill_op(void);
	void fb_erase_op(void);

	void fill_slope(bitmap_ind16 &bitmap, const rectangle &cliprect, u16 header, s32 x1, s32 x2, s32 sl1, s32 sl2, s32 y1, s32 y2, s32 *nx1, s32 *nx2);
	void fill_poly(bitmap_ind16 &bitmap, const rectangle &cliprect, const struct taitoair_poly *q);

	void DSP_map_data(address_map &map) ATTR_COLD;
	void DSP_map_program(address_map &map) ATTR_COLD;
	void airsys_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

#endif // MAME_TAITO_TAITOAIR_H
