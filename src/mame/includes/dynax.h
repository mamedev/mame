// license:BSD-3-Clause
// copyright-holders:Luca Elia, Nicola Salmoria
/***************************************************************************

      Dynax hardware

***************************************************************************/
#include "machine/msm6242.h"
#include "sound/2413intf.h"
#include "sound/msm5205.h"
#include "sound/okim6295.h"

class dynax_state : public driver_device
{
public:
	dynax_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_soundcpu(*this, "soundcpu"),
			m_ym2413(*this, "ym2413"),
			m_oki(*this, "oki"),
			m_msm(*this, "msm"),
			m_screen(*this, "screen"),
			m_palette(*this, "palette"),
			m_rtc(*this, "rtc")
		{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_soundcpu;
	optional_device<ym2413_device> m_ym2413;
	optional_device<okim6295_device> m_oki;
	optional_device<msm5205_device> m_msm;
	optional_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<msm6242_device> m_rtc;

	// up to 8 layers, 2 images per layer (interleaved on screen)
	std::unique_ptr<UINT8[]>  m_pixmap[8][2];

	/* irq */
	typedef void (dynax_state::*irq_func)();    // some games trigger IRQ at blitter end, some don't
	irq_func m_update_irq_func;
	UINT8 m_sound_irq;
	UINT8 m_vblank_irq;
	UINT8 m_blitter_irq;
	UINT8 m_blitter2_irq;
	UINT8 m_soundlatch_irq;
	UINT8 m_sound_vblank_irq;

	/* blitters */
	int m_blit_scroll_x;
	int m_blit2_scroll_x;
	int m_blit_scroll_y;
	int m_blit2_scroll_y;
	int m_blit_wrap_enable;
	int m_blit2_wrap_enable;
	int m_blit_x;
	int m_blit_y;
	int m_blit2_x;
	int m_blit2_y;
	int m_blit_src;
	int m_blit2_src;
	int m_blit_romregion;
	int m_blit2_romregion;
	int m_blit_dest;
	int m_blit2_dest;
	int m_blit_pen;
	int m_blit2_pen;
	int m_blit_palbank;
	int m_blit2_palbank;
	int m_blit_palettes;
	int m_blit2_palettes;
	int m_layer_enable;
	int m_blit_backpen;

	int m_hanamai_layer_half;
	int m_hnoridur_layer_half2;

	int m_extra_scroll_x;
	int m_extra_scroll_y;
	int m_flipscreen;

	int m_layer_layout;

	const int *m_priority_table;
	int m_hanamai_priority;

	/* input */
	UINT8 m_input_sel;
	UINT8 m_dsw_sel;
	UINT8 m_keyb;
	UINT8 m_coins;
	UINT8 m_hopper;

	/* misc */
	int m_hnoridur_bank;
	UINT8 m_palette_ram[16*256*2];
	int m_palbank;
	int m_msm5205next;
	int m_resetkludge;
	int m_toggle;
	int m_toggle_cpu1;
	int m_yarunara_clk_toggle;
	UINT8 m_soundlatch_ack;
	UINT8 m_soundlatch_full;
	UINT8 m_latch;
	int m_rombank;
	UINT8 m_tenkai_p5_val;
	int m_tenkai_6c;
	int m_tenkai_70;
	UINT8 m_gekisha_val[2];
	UINT8 m_gekisha_rom_enable;
	UINT8 *m_romptr;

	DECLARE_WRITE8_MEMBER(dynax_vblank_ack_w);
	DECLARE_WRITE8_MEMBER(dynax_blitter_ack_w);
	DECLARE_WRITE8_MEMBER(jantouki_vblank_ack_w);
	DECLARE_WRITE8_MEMBER(jantouki_blitter_ack_w);
	DECLARE_WRITE8_MEMBER(jantouki_blitter2_ack_w);
	DECLARE_WRITE8_MEMBER(jantouki_sound_vblank_ack_w);
	DECLARE_WRITE8_MEMBER(dynax_coincounter_0_w);
	DECLARE_WRITE8_MEMBER(dynax_coincounter_1_w);
	DECLARE_READ8_MEMBER(ret_ff);
	DECLARE_READ8_MEMBER(hanamai_keyboard_0_r);
	DECLARE_READ8_MEMBER(hanamai_keyboard_1_r);
	DECLARE_WRITE8_MEMBER(hanamai_keyboard_w);
	DECLARE_WRITE8_MEMBER(dynax_rombank_w);
	DECLARE_WRITE8_MEMBER(jantouki_sound_rombank_w);
	DECLARE_WRITE8_MEMBER(cdracula_sound_rombank_w);
	DECLARE_WRITE8_MEMBER(hnoridur_rombank_w);
	DECLARE_WRITE8_MEMBER(hnoridur_palbank_w);
	DECLARE_WRITE8_MEMBER(hnoridur_palette_w);
	DECLARE_WRITE8_MEMBER(yarunara_palette_w);
	DECLARE_WRITE8_MEMBER(nanajign_palette_w);
	DECLARE_WRITE8_MEMBER(adpcm_data_w);
	DECLARE_WRITE8_MEMBER(yarunara_layer_half_w);
	DECLARE_WRITE8_MEMBER(yarunara_layer_half2_w);
	DECLARE_WRITE8_MEMBER(hjingi_bank_w);
	DECLARE_WRITE8_MEMBER(hjingi_lockout_w);
	DECLARE_WRITE8_MEMBER(hjingi_hopper_w);
	UINT8 hjingi_hopper_bit();
	DECLARE_READ8_MEMBER(hjingi_keyboard_0_r);
	DECLARE_READ8_MEMBER(hjingi_keyboard_1_r);
	DECLARE_WRITE8_MEMBER(yarunara_input_w);
	DECLARE_READ8_MEMBER(yarunara_input_r);
	DECLARE_WRITE8_MEMBER(yarunara_rombank_w);
	DECLARE_WRITE8_MEMBER(yarunara_flipscreen_w);
	DECLARE_WRITE8_MEMBER(yarunara_flipscreen_inv_w);
	DECLARE_WRITE8_MEMBER(yarunara_blit_romregion_w);
	DECLARE_READ8_MEMBER(jantouki_soundlatch_ack_r);
	DECLARE_WRITE8_MEMBER(jantouki_soundlatch_w);
	DECLARE_READ8_MEMBER(jantouki_blitter_busy_r);
	DECLARE_WRITE8_MEMBER(jantouki_rombank_w);
	DECLARE_WRITE8_MEMBER(jantouki_soundlatch_ack_w);
	DECLARE_READ8_MEMBER(jantouki_soundlatch_r);
	DECLARE_READ8_MEMBER(jantouki_soundlatch_status_r);
	DECLARE_READ8_MEMBER(mjelctrn_keyboard_1_r);
	DECLARE_READ8_MEMBER(mjelctrn_dsw_r);
	DECLARE_WRITE8_MEMBER(mjelctrn_blitter_ack_w);
	DECLARE_WRITE8_MEMBER(tenkai_ipsel_w);
	DECLARE_WRITE8_MEMBER(tenkai_ip_w);
	DECLARE_READ8_MEMBER(tenkai_ip_r);
	DECLARE_READ8_MEMBER(tenkai_palette_r);
	DECLARE_WRITE8_MEMBER(tenkai_palette_w);
	DECLARE_READ8_MEMBER(tenkai_p3_r);
	DECLARE_WRITE8_MEMBER(tenkai_p3_w);
	DECLARE_WRITE8_MEMBER(tenkai_p4_w);
	DECLARE_READ8_MEMBER(tenkai_p5_r);
	DECLARE_WRITE8_MEMBER(tenkai_p6_w);
	DECLARE_WRITE8_MEMBER(tenkai_p7_w);
	DECLARE_WRITE8_MEMBER(tenkai_p8_w);
	DECLARE_READ8_MEMBER(tenkai_p8_r);
	DECLARE_READ8_MEMBER(tenkai_8000_r);
	DECLARE_WRITE8_MEMBER(tenkai_8000_w);
	DECLARE_WRITE8_MEMBER(tenkai_6c_w);
	DECLARE_WRITE8_MEMBER(tenkai_70_w);
	DECLARE_WRITE8_MEMBER(tenkai_blit_romregion_w);
	DECLARE_READ8_MEMBER(gekisha_keyboard_0_r);
	DECLARE_READ8_MEMBER(gekisha_keyboard_1_r);
	DECLARE_WRITE8_MEMBER(gekisha_hopper_w);
	DECLARE_WRITE8_MEMBER(gekisha_p4_w);
	DECLARE_READ8_MEMBER(gekisha_8000_r);
	DECLARE_WRITE8_MEMBER(gekisha_8000_w);
	DECLARE_WRITE8_MEMBER(dynax_extra_scrollx_w);
	DECLARE_WRITE8_MEMBER(dynax_extra_scrolly_w);
	DECLARE_WRITE8_MEMBER(dynax_blit_pen_w);
	DECLARE_WRITE8_MEMBER(dynax_blit2_pen_w);
	DECLARE_WRITE8_MEMBER(dynax_blit_dest_w);
	DECLARE_WRITE8_MEMBER(dynax_blit2_dest_w);
	DECLARE_WRITE8_MEMBER(tenkai_blit_dest_w);
	DECLARE_WRITE8_MEMBER(mjembase_blit_dest_w);
	DECLARE_WRITE8_MEMBER(dynax_blit_backpen_w);
	DECLARE_WRITE8_MEMBER(dynax_blit_flags_w);
	DECLARE_WRITE8_MEMBER(dynax_blit_palette01_w);
	DECLARE_WRITE8_MEMBER(tenkai_blit_palette01_w);
	DECLARE_WRITE8_MEMBER(dynax_blit_palette45_w);
	DECLARE_WRITE8_MEMBER(dynax_blit_palette23_w);
	DECLARE_WRITE8_MEMBER(tenkai_blit_palette23_w);
	DECLARE_WRITE8_MEMBER(mjembase_blit_palette23_w);
	DECLARE_WRITE8_MEMBER(dynax_blit_palette67_w);
	DECLARE_WRITE8_MEMBER(dynax_blit_palbank_w);
	DECLARE_WRITE8_MEMBER(dynax_blit2_palbank_w);
	DECLARE_WRITE8_MEMBER(hanamai_layer_half_w);
	DECLARE_WRITE8_MEMBER(hnoridur_layer_half2_w);
	DECLARE_WRITE8_MEMBER(mjdialq2_blit_dest_w);
	DECLARE_WRITE8_MEMBER(dynax_layer_enable_w);
	DECLARE_WRITE8_MEMBER(jantouki_layer_enable_w);
	DECLARE_WRITE8_MEMBER(mjdialq2_layer_enable_w);
	DECLARE_WRITE8_MEMBER(dynax_flipscreen_w);
	DECLARE_WRITE8_MEMBER(dynax_blit_romregion_w);
	DECLARE_WRITE8_MEMBER(dynax_blit2_romregion_w);
	DECLARE_WRITE8_MEMBER(dynax_blit_scroll_w);
	DECLARE_WRITE8_MEMBER(tenkai_blit_scroll_w);
	DECLARE_WRITE8_MEMBER(dynax_blit2_scroll_w);
	DECLARE_WRITE8_MEMBER(dynax_blitter_rev2_w);
	DECLARE_WRITE8_MEMBER(tenkai_blitter_rev2_w);
	DECLARE_WRITE8_MEMBER(cdracula_blitter_rev2_w);
	DECLARE_WRITE8_MEMBER(jantouki_blitter_rev2_w);
	DECLARE_WRITE8_MEMBER(jantouki_blitter2_rev2_w);
	DECLARE_WRITE8_MEMBER(hanamai_priority_w);
	DECLARE_WRITE8_MEMBER(tenkai_priority_w);
	DECLARE_WRITE8_MEMBER(mjembase_priority_w);

	DECLARE_DRIVER_INIT(mjelct3);
	DECLARE_DRIVER_INIT(blktouch);
	DECLARE_DRIVER_INIT(mjelct3a);
	DECLARE_DRIVER_INIT(mjreach);
	DECLARE_DRIVER_INIT(maya_common);
	DECLARE_DRIVER_INIT(mayac);
	DECLARE_DRIVER_INIT(maya);

	UINT32 screen_update_hanamai(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_hnoridur(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_sprtmtch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_mjdialq2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_jantouki_top(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_jantouki_bottom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_cdracula(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(sprtmtch_vblank_interrupt);
	INTERRUPT_GEN_MEMBER(jantouki_vblank_interrupt);
	INTERRUPT_GEN_MEMBER(jantouki_sound_vblank_interrupt);
	INTERRUPT_GEN_MEMBER(yarunara_clock_interrupt);
	INTERRUPT_GEN_MEMBER(mjelctrn_vblank_interrupt);

	TIMER_DEVICE_CALLBACK_MEMBER(neruton_irq_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(majxtal7_vblank_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(tenkai_interrupt);

	void tenkai_update_rombank();
	void gekisha_bank_postload();

	DECLARE_WRITE_LINE_MEMBER(sprtmtch_sound_callback);
	DECLARE_WRITE_LINE_MEMBER(jantouki_sound_callback);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int_cpu1);

	DECLARE_MACHINE_RESET(adpcm);
	DECLARE_WRITE8_MEMBER(adpcm_reset_w);
	DECLARE_MACHINE_START(gekisha);
	DECLARE_MACHINE_RESET(gekisha);
	DECLARE_MACHINE_START(tenkai);
	DECLARE_WRITE8_MEMBER(tenkai_dswsel_w);
	DECLARE_READ8_MEMBER(tenkai_dsw_r);
	DECLARE_WRITE_LINE_MEMBER(tenkai_rtc_irq);
	DECLARE_MACHINE_RESET(dynax);
	DECLARE_MACHINE_START(dynax);
	DECLARE_MACHINE_START(hanamai);
	DECLARE_VIDEO_START(hanamai);
	DECLARE_MACHINE_START(hnoridur);
	DECLARE_VIDEO_START(hnoridur);
	DECLARE_PALETTE_INIT(sprtmtch);
	DECLARE_VIDEO_START(sprtmtch);
	DECLARE_MACHINE_START(jantouki);
	DECLARE_VIDEO_START(jantouki);
	DECLARE_VIDEO_START(mjelctrn);
	DECLARE_VIDEO_START(mjembase);
	DECLARE_VIDEO_START(mjdialq2);
	DECLARE_VIDEO_START(mcnpshnt);
	DECLARE_PALETTE_INIT(janyuki);
	DECLARE_VIDEO_START(neruton);

	inline void blitter_plot_pixel( int layer, int mask, int x, int y, int pen, int wrap, int flags );
	int blitter_drawgfx( int layer, int mask, const char *gfx, int src, int pen, int x, int y, int wrap, int flags );
	void dynax_blitter_start( int flags );
	void jantouki_blitter_start( int flags );
	void jantouki_blitter2_start( int flags );
	void jantouki_copylayer( bitmap_ind16 &bitmap, const rectangle &cliprect, int i, int y );
	void mjdialq2_copylayer( bitmap_ind16 &bitmap, const rectangle &cliprect, int i );
	void hanamai_copylayer(bitmap_ind16 &bitmap, const rectangle &cliprect, int i );
	int debug_mask();
	int debug_viewer( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void dynax_common_reset();
	void sprtmtch_update_irq();
	void jantouki_update_irq();
	void mjelctrn_update_irq();
	void neruton_update_irq();
	void jantouki_sound_update_irq();
	void tenkai_show_6c();
	void gekisha_set_rombank( UINT8 data );
};
