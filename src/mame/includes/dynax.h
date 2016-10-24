// license:BSD-3-Clause
// copyright-holders:Luca Elia, Nicola Salmoria
/***************************************************************************

      Dynax hardware

***************************************************************************/
#include "machine/msm6242.h"
#include "sound/ym2413.h"
#include "sound/msm5205.h"
#include "sound/okim6295.h"

class dynax_state : public driver_device
{
public:
	dynax_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_ym2413(*this, "ym2413")
		, m_oki(*this, "oki")
		, m_msm(*this, "msm")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_rtc(*this, "rtc")
		, m_gfx_region1(*this, "gfx1")
		, m_gfx_region2(*this, "gfx2")
		, m_gfx_region3(*this, "gfx3")
		, m_gfx_region4(*this, "gfx4")
		, m_gfx_region5(*this, "gfx5")
		, m_gfx_region6(*this, "gfx6")
		, m_gfx_region7(*this, "gfx7")
		, m_gfx_region8(*this, "gfx8")

	{
	}

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_soundcpu;
	optional_device<ym2413_device> m_ym2413;
	optional_device<okim6295_device> m_oki;
	optional_device<msm5205_device> m_msm;
	optional_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<msm6242_device> m_rtc;
	optional_region_ptr<uint8_t> m_gfx_region1;
	optional_region_ptr<uint8_t> m_gfx_region2;
	optional_region_ptr<uint8_t> m_gfx_region3;
	optional_region_ptr<uint8_t> m_gfx_region4;
	optional_region_ptr<uint8_t> m_gfx_region5;
	optional_region_ptr<uint8_t> m_gfx_region6;
	optional_region_ptr<uint8_t> m_gfx_region7;
	optional_region_ptr<uint8_t> m_gfx_region8;

	memory_region * m_gfxregions[8];

	// up to 8 layers, 2 images per layer (interleaved on screen)
	std::unique_ptr<uint8_t[]>  m_pixmap[8][2];

	/* irq */
	typedef void (dynax_state::*irq_func)();    // some games trigger IRQ at blitter end, some don't
	irq_func m_update_irq_func;
	uint8_t m_sound_irq;
	uint8_t m_vblank_irq;
	uint8_t m_blitter_irq;
	uint8_t m_blitter2_irq;
	uint8_t m_soundlatch_irq;
	uint8_t m_sound_vblank_irq;

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
	uint8_t m_input_sel;
	uint8_t m_dsw_sel;
	uint8_t m_keyb;
	uint8_t m_coins;
	uint8_t m_hopper;

	/* misc */
	int m_hnoridur_bank;
	uint8_t m_palette_ram[16*256*2];
	int m_palbank;
	int m_msm5205next;
	int m_resetkludge;
	int m_toggle;
	int m_toggle_cpu1;
	int m_yarunara_clk_toggle;
	uint8_t m_soundlatch_ack;
	uint8_t m_soundlatch_full;
	uint8_t m_latch;
	int m_rombank;
	uint8_t m_tenkai_p5_val;
	int m_tenkai_6c;
	int m_tenkai_70;
	uint8_t m_gekisha_val[2];
	uint8_t m_gekisha_rom_enable;
	uint8_t *m_romptr;
	uint8_t *m_hnoridur_ptr;

	void dynax_vblank_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_blitter_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jantouki_vblank_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jantouki_blitter_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jantouki_blitter2_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jantouki_sound_vblank_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_coincounter_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_coincounter_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ret_ff(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t hanamai_keyboard_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t hanamai_keyboard_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hanamai_keyboard_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jantouki_sound_rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cdracula_sound_rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hnoridur_rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hnoridur_palbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hnoridur_palette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void yarunara_palette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nanajign_palette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adpcm_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void yarunara_layer_half_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void yarunara_layer_half2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hjingi_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hjingi_lockout_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hjingi_hopper_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t hjingi_hopper_bit();
	uint8_t hjingi_keyboard_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t hjingi_keyboard_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void yarunara_input_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t yarunara_input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void yarunara_rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void yarunara_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void yarunara_flipscreen_inv_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void yarunara_blit_romregion_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t jantouki_soundlatch_ack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void jantouki_soundlatch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t jantouki_blitter_busy_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void jantouki_rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jantouki_soundlatch_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t jantouki_soundlatch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t jantouki_soundlatch_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mjelctrn_keyboard_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mjelctrn_dsw_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mjelctrn_blitter_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tenkai_ipsel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tenkai_ip_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tenkai_ip_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t tenkai_palette_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tenkai_palette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tenkai_p3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tenkai_p3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tenkai_p4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tenkai_p5_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tenkai_p6_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tenkai_p7_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tenkai_p8_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tenkai_p8_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t tenkai_8000_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tenkai_8000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tenkai_6c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tenkai_70_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tenkai_blit_romregion_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t gekisha_keyboard_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t gekisha_keyboard_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void gekisha_hopper_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gekisha_p4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t gekisha_8000_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void gekisha_8000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_extra_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_extra_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_blit_pen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_blit2_pen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_blit_dest_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_blit2_dest_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tenkai_blit_dest_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mjembase_blit_dest_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_blit_backpen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_blit_flags_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_blit_palette01_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tenkai_blit_palette01_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_blit_palette45_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_blit_palette23_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tenkai_blit_palette23_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mjembase_blit_palette23_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_blit_palette67_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_blit_palbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_blit2_palbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hanamai_layer_half_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hnoridur_layer_half2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mjdialq2_blit_dest_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_layer_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jantouki_layer_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mjdialq2_layer_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_blit_romregion_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_blit2_romregion_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_blit_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tenkai_blit_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_blit2_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_blitter_rev2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tenkai_blitter_rev2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cdracula_blitter_rev2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jantouki_blitter_rev2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jantouki_blitter2_rev2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hanamai_priority_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tenkai_priority_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mjembase_priority_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_mjelct3();
	void init_blktouch();
	void init_mjelct3a();
	void init_mjreach();
	void init_maya_common();
	void init_mayac();
	void init_maya();

	uint32_t screen_update_hanamai(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_hnoridur(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_sprtmtch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_mjdialq2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_jantouki_top(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_jantouki_bottom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cdracula(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void sprtmtch_vblank_interrupt(device_t &device);
	void jantouki_vblank_interrupt(device_t &device);
	void jantouki_sound_vblank_interrupt(device_t &device);
	void yarunara_clock_interrupt(device_t &device);
	void mjelctrn_vblank_interrupt(device_t &device);

	void neruton_irq_scanline(timer_device &timer, void *ptr, int32_t param);
	void majxtal7_vblank_interrupt(timer_device &timer, void *ptr, int32_t param);
	void tenkai_interrupt(timer_device &timer, void *ptr, int32_t param);

	void tenkai_update_rombank();
	void gekisha_bank_postload();

	void sprtmtch_sound_callback(int state);
	void jantouki_sound_callback(int state);
	void adpcm_int(int state);
	void adpcm_int_cpu1(int state);

	void machine_reset_adpcm();
	void adpcm_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void machine_start_gekisha();
	void machine_reset_gekisha();
	void machine_start_tenkai();
	void tenkai_dswsel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tenkai_dsw_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tenkai_rtc_irq(int state);
	void machine_reset_dynax();
	void machine_start_dynax();
	void machine_start_hanamai();
	void video_start_hanamai();
	void machine_start_hnoridur();
	void video_start_hnoridur();
	void palette_init_sprtmtch(palette_device &palette);
	void video_start_sprtmtch();
	void machine_start_jantouki();
	void video_start_jantouki();
	void video_start_mjelctrn();
	void video_start_mjembase();
	void video_start_mjdialq2();
	void video_start_mcnpshnt();
	void palette_init_janyuki(palette_device &palette);
	void video_start_neruton();

	inline void blitter_plot_pixel( int layer, int mask, int x, int y, int pen, int wrap, int flags );
	int blitter_drawgfx( int layer, int mask, memory_region *gfx, int src, int pen, int x, int y, int wrap, int flags );
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
	void gekisha_set_rombank( uint8_t data );
};
