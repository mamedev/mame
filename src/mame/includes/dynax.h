// license:BSD-3-Clause
// copyright-holders:Luca Elia, Nicola Salmoria
/***************************************************************************

      Dynax hardware

***************************************************************************/
#ifndef MAME_INCLUDES_DYNAX_H
#define MAME_INCLUDES_DYNAX_H

#pragma once

#include "machine/bankdev.h"
#include "machine/gen_latch.h"
#include "machine/rstbuf.h"
#include "sound/msm5205.h"
#include "sound/okim6295.h"
#include "machine/74259.h"
#include "video/dynax_blitter_rev2.h"
#include "emupal.h"
#include "screen.h"

class dynax_state : public driver_device
{
public:
	dynax_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_oki(*this, "oki")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_mainlatch(*this, "mainlatch")
		, m_bankdev(*this, "bankdev")
		, m_blitter(*this, "blitter")
		, m_msm(*this, "msm")
		, m_mainirq(*this, "mainirq")
		, m_soundirq(*this, "soundirq")
		, m_soundlatch(*this, "soundlatch")
		, m_blitter2(*this, "blitter2")
		, m_blitter_gfx(*this, "blitter")
		, m_led(*this, "led0")
	{
	}

	void mjfriday(machine_config &config);
	void yarunara(machine_config &config);
	void janyuki(machine_config &config);
	void hnoridur(machine_config &config);
	void gekisha(machine_config &config);
	void majrjhdx(machine_config &config);
	void mcnpshnt(machine_config &config);
	void nanajign(machine_config &config);
	void cdracula(machine_config &config);
	void tenkai(machine_config &config);
	void ougonhai(machine_config &config);
	void hjingi(machine_config &config);
	void mjreach(machine_config &config);
	void neruton(machine_config &config);
	void mjdialq2(machine_config &config);
	void jantouki(machine_config &config);
	void quiztvqq(machine_config &config);
	void mjelctrn(machine_config &config);
	void mjembase(machine_config &config);
	void mjangels(machine_config &config);
	void hanamai(machine_config &config);
	void sprtmtch(machine_config &config);

	void init_mjelct3();
	void init_blktouch();
	void init_mjelct3a();
	void init_maya_common();
	void init_mayac();
	void init_maya();

	DECLARE_WRITE_LINE_MEMBER(blitter_ack_w);
	DECLARE_WRITE_LINE_MEMBER(sprtmtch_blitter_irq_w);
	DECLARE_WRITE_LINE_MEMBER(sprtmtch_vblank_w);
	DECLARE_WRITE_LINE_MEMBER(layer_half_w);
	DECLARE_WRITE_LINE_MEMBER(layer_half2_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);

	void hnoridur_blit_pixel_w(offs_t offset, uint8_t data);
	void dynax_blit_scrollx_w(uint8_t data);
	void dynax_blit_scrolly_w(uint8_t data);

protected:
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_soundcpu;
	optional_device<okim6295_device> m_oki;
	optional_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<ls259_device> m_mainlatch;
	optional_device<address_map_bank_device> m_bankdev;
	optional_device<dynax_blitter_rev2_device> m_blitter;

	/* input / output */
	uint8_t m_input_sel = 0U;
	uint8_t m_input_mode = 0U;
	uint8_t m_dsw_sel = 0U;
	uint8_t m_keyb = 0U;
	uint8_t m_coins = 0U;
	uint8_t m_hopper = 0U;

	DECLARE_VIDEO_START(hnoridur);

	void dynax_vblank_ack_w(uint8_t data);
	void dynax_blit_dest_w(uint8_t data);
	void dynax_blit_romregion_w(uint8_t data);
	void dynax_extra_scrollx_w(uint8_t data);
	void dynax_extra_scrolly_w(uint8_t data);

	void hanamai_copylayer(bitmap_ind16 &bitmap, const rectangle &cliprect, int i );

	DECLARE_MACHINE_START(dynax);
	DECLARE_MACHINE_RESET(dynax);

	void htengoku_banked_map(address_map &map);

private:
	void jantouki_vblank_ack_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(jantouki_blitter_ack_w);
	DECLARE_WRITE_LINE_MEMBER(jantouki_blitter_irq_w);
	DECLARE_WRITE_LINE_MEMBER(jantouki_blitter2_ack_w);
	DECLARE_WRITE_LINE_MEMBER(jantouki_blitter2_irq_w);
	void jantouki_sound_vblank_ack_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(coincounter_0_w);
	DECLARE_WRITE_LINE_MEMBER(coincounter_1_w);
	uint8_t ret_ff();
	uint8_t hanamai_keyboard_0_r();
	uint8_t hanamai_keyboard_1_r();
	void hanamai_keyboard_w(uint8_t data);
	void dynax_rombank_w(uint8_t data);
	void jantouki_sound_rombank_w(uint8_t data);
	void cdracula_sound_rombank_w(uint8_t data);
	void hnoridur_rombank_w(uint8_t data);
	void hnoridur_palbank_w(uint8_t data);
	void hnoridur_palette_lo_w(offs_t offset, uint8_t data);
	void hnoridur_palette_hi_w(offs_t offset, uint8_t data);
	void hnoridur_palette_update(offs_t offset);
	void nanajign_palette_lo_w(offs_t offset, uint8_t data);
	void nanajign_palette_hi_w(offs_t offset, uint8_t data);
	void nanajign_palette_update(offs_t offset);
	void adpcm_data_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(hjingi_lockout_w);
	DECLARE_WRITE_LINE_MEMBER(hjingi_hopper_w);
	uint8_t hjingi_hopper_bit();
	uint8_t hjingi_keyboard_0_r();
	uint8_t hjingi_keyboard_1_r();
	void yarunara_input_w(offs_t offset, uint8_t data);
	uint8_t yarunara_input_r(offs_t offset);
	void yarunara_rombank_w(uint8_t data);
	void yarunara_blit_romregion_w(uint8_t data);
	uint8_t jantouki_soundlatch_ack_r();
	void jantouki_soundlatch_w(uint8_t data);
	uint8_t jantouki_blitter_busy_r();
	void jantouki_rombank_w(uint8_t data);
	uint8_t jantouki_soundlatch_status_r();
	uint8_t mjelctrn_keyboard_1_r();
	uint8_t mjelctrn_dsw_r();
	void tenkai_ipsel_w(offs_t offset, uint8_t data);
	void tenkai_ip_w(uint8_t data);
	uint8_t tenkai_ip_r(offs_t offset);
	uint8_t tenkai_palette_r(offs_t offset);
	void tenkai_palette_w(offs_t offset, uint8_t data);
	uint8_t tenkai_p3_r();
	void tenkai_p3_w(uint8_t data);
	void tenkai_p4_w(uint8_t data);
	uint8_t tenkai_p5_r();
	void tenkai_p6_w(uint8_t data);
	void tenkai_p7_w(uint8_t data);
	void tenkai_p8_w(uint8_t data);
	uint8_t tenkai_p8_r();
	void ougonhai_p7_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(tenkai_6c_w);
	DECLARE_WRITE_LINE_MEMBER(tenkai_70_w);
	void tenkai_blit_romregion_w(uint8_t data);
	uint8_t gekisha_keyboard_0_r();
	uint8_t gekisha_keyboard_1_r();
	void gekisha_hopper_w(offs_t offset, uint8_t data);
	void gekisha_p4_w(uint8_t data);
	//void dynax_blit_pen_w(uint8_t data);
	void dynax_blit2_dest_w(uint8_t data);
	void tenkai_blit_dest_w(uint8_t data);
	void mjembase_blit_dest_w(uint8_t data);
	void dynax_blit_backpen_w(uint8_t data);
	void dynax_blit_palette01_w(uint8_t data);
	void tenkai_blit_palette01_w(uint8_t data);
	void dynax_blit_palette45_w(uint8_t data);
	void dynax_blit_palette23_w(uint8_t data);
	void tenkai_blit_palette23_w(uint8_t data);
	void mjembase_blit_palette23_w(uint8_t data);
	void dynax_blit_palette67_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(blit_palbank_w);
	DECLARE_WRITE_LINE_MEMBER(blit2_palbank_w);
	DECLARE_WRITE_LINE_MEMBER(mjdialq2_blit_dest0_w);
	DECLARE_WRITE_LINE_MEMBER(mjdialq2_blit_dest1_w);
	void dynax_layer_enable_w(uint8_t data);
	void jantouki_layer_enable_w(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(mjdialq2_layer0_enable_w);
	DECLARE_WRITE_LINE_MEMBER(mjdialq2_layer1_enable_w);
	void dynax_blit2_romregion_w(uint8_t data);
	void hanamai_blit_pixel_w(offs_t offset, uint8_t data);
	void cdracula_blit_pixel_w(offs_t offset, uint8_t data);
	void drgpunch_blit_pixel_w(offs_t offset, uint8_t data);
	void jantouki_blit_pixel_w(offs_t offset, uint8_t data);
	void jantouki_blit2_pixel_w(offs_t offset, uint8_t data);
	void mjdialq2_blit_pixel_w(offs_t offset, uint8_t data);
	void dynax_blit2_scrollx_w(uint8_t data);
	void dynax_blit2_scrolly_w(uint8_t data);
	void tenkai_blit_scrollx_w(uint8_t data);
	void tenkai_blit_scrolly_w(uint8_t data);
	void hanamai_priority_w(uint8_t data);
	void tenkai_priority_w(uint8_t data);
	void mjembase_priority_w(uint8_t data);

	uint32_t screen_update_hanamai(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_hnoridur(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_sprtmtch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_mjdialq2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_jantouki_top(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_jantouki_bottom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cdracula(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER(jantouki_vblank_w);
	DECLARE_WRITE_LINE_MEMBER(mjfriday_vblank_w);

	void tenkai_update_rombank();

	DECLARE_WRITE_LINE_MEMBER(adpcm_int);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int_cpu1);

	DECLARE_MACHINE_RESET(adpcm);
	void adpcm_reset_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(adpcm_reset_kludge_w);
	void tenkai_dswsel_w(uint8_t data);
	uint8_t tenkai_dsw_r();
	DECLARE_WRITE_LINE_MEMBER(tenkai_blitter_irq_w);
	DECLARE_WRITE_LINE_MEMBER(tenkai_blitter_ack_w);
	DECLARE_MACHINE_START(hanamai);
	DECLARE_MACHINE_START(hjingi);
	DECLARE_VIDEO_START(hanamai);
	void sprtmtch_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(sprtmtch);
	DECLARE_MACHINE_START(jantouki);
	DECLARE_VIDEO_START(jantouki);
	DECLARE_VIDEO_START(mjelctrn);
	DECLARE_VIDEO_START(mjembase);
	DECLARE_VIDEO_START(mjdialq2);
	DECLARE_VIDEO_START(mcnpshnt);
	void janyuki_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(neruton);

	//int blitter_drawgfx( int layer, int mask, memory_region *gfx, int src, int pen, int x, int y, int wrap, int flags );
	void jantouki_copylayer( bitmap_ind16 &bitmap, const rectangle &cliprect, int i, int y );
	void mjdialq2_copylayer( bitmap_ind16 &bitmap, const rectangle &cliprect, int i );
	int debug_mask();
	int debug_viewer( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void dynax_common_reset();
	void tenkai_show_6c();
	void cdracula_io_map(address_map &map);
	void cdracula_mem_map(address_map &map);
	void gekisha_banked_map(address_map &map);
	void gekisha_map(address_map &map);
	void hanamai_io_map(address_map &map);
	void hjingi_banked_map(address_map &map);
	void hjingi_io_map(address_map &map);
	void hjingi_mem_map(address_map &map);
	void hnoridur_banked_map(address_map &map);
	void hnoridur_io_map(address_map &map);
	void hnoridur_mem_map(address_map &map);
	void jantouki_io_map(address_map &map);
	void jantouki_mem_map(address_map &map);
	void jantouki_sound_io_map(address_map &map);
	void jantouki_sound_mem_map(address_map &map);
	void mcnpshnt_io_map(address_map &map);
	void mcnpshnt_mem_map(address_map &map);
	void mjangels_banked_map(address_map &map);
	void mjdialq2_mem_map(address_map &map);
	void mjelctrn_banked_map(address_map &map);
	void mjelctrn_io_map(address_map &map);
	void mjembase_io_map(address_map &map);
	void mjfriday_io_map(address_map &map);
	void nanajign_banked_map(address_map &map);
	void nanajign_io_map(address_map &map);
	void nanajign_mem_map(address_map &map);
	void quiztvqq_mem_map(address_map &map);
	void sprtmtch_io_map(address_map &map);
	void sprtmtch_mem_map(address_map &map);
	void tenkai_banked_map(address_map &map);
	void tenkai_map(address_map &map);
	void yarunara_banked_map(address_map &map);
	void yarunara_io_map(address_map &map);
	void yarunara_mem_map(address_map &map);

	/* devices */
	optional_device<msm5205_device> m_msm;
	optional_device<rst_pos_buffer_device> m_mainirq;
	optional_device<rst_pos_buffer_device> m_soundirq;
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_device<dynax_blitter_rev2_device> m_blitter2;
	optional_region_ptr<uint8_t> m_blitter_gfx;
	output_finder<> m_led;

	// up to 8 layers, 2 images per layer (interleaved on screen)
	std::unique_ptr<uint8_t[]>  m_pixmap[8][2]{};

	/* irq */
	bool m_blitter_irq_mask = false;
	bool m_blitter2_irq_mask = false;

	/* blitters */
	int m_blit_scroll_x = 0;
	int m_blit2_scroll_x = 0;
	int m_blit_scroll_y = 0;
	int m_blit2_scroll_y = 0;
	int m_blit_dest = 0;
	int m_blit2_dest = 0;
	int m_blit_palbank = 0;
	int m_blit2_palbank = 0;
	int m_blit_palettes = 0;
	int m_blit2_palettes = 0;
	int m_layer_enable = 0;
	int m_blit_backpen = 0;

	int m_hanamai_layer_half = 0;
	int m_hnoridur_layer_half2 = 0;

	int m_extra_scroll_x = 0;
	int m_extra_scroll_y = 0;
	int m_flipscreen = 0;

	int m_layer_layout = 0;

	const int *m_priority_table = nullptr;
	int m_hanamai_priority = 0;

	/* misc */
	uint8_t m_palette_ram[16*256*2]{};
	int m_palbank = 0;
	int m_msm5205next = 0;
	int m_resetkludge = 0;
	int m_toggle = 0;
	int m_toggle_cpu1 = 0;
	int m_rombank = 0;
	uint8_t m_tenkai_p5_val = 0U;
	int m_tenkai_6c = 0;
	int m_tenkai_70 = 0;
	uint8_t m_gekisha_val[2]{};

};

#endif // MAME_INCLUDES_DYNAX_H
