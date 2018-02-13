// license:BSD-3-Clause
// copyright-holders:Luca Elia, Nicola Salmoria
/***************************************************************************

      Dynax hardware

***************************************************************************/

#include "machine/bankdev.h"
#include "sound/msm5205.h"
#include "sound/okim6295.h"
#include "machine/74259.h"
#include "screen.h"

class dynax_state : public driver_device
{
public:
	dynax_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_oki(*this, "oki")
		, m_msm(*this, "msm")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_bankdev(*this, "bankdev")
		, m_mainlatch(*this, "mainlatch")
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
	optional_device<okim6295_device> m_oki;
	optional_device<msm5205_device> m_msm;
	optional_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<address_map_bank_device> m_bankdev;
	optional_device<ls259_device> m_mainlatch;
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
	bool m_sound_irq;
	bool m_vblank_irq;
	bool m_blitter_irq;
	bool m_blitter_irq_mask;
	bool m_blitter2_irq;
	bool m_blitter2_irq_mask;
	bool m_soundlatch_irq;
	bool m_sound_vblank_irq;

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

	DECLARE_WRITE8_MEMBER(dynax_vblank_ack_w);
	DECLARE_WRITE_LINE_MEMBER(blitter_ack_w);
	DECLARE_WRITE8_MEMBER(jantouki_vblank_ack_w);
	DECLARE_WRITE_LINE_MEMBER(jantouki_blitter_ack_w);
	DECLARE_WRITE_LINE_MEMBER(jantouki_blitter2_ack_w);
	DECLARE_WRITE8_MEMBER(jantouki_sound_vblank_ack_w);
	DECLARE_WRITE_LINE_MEMBER(coincounter_0_w);
	DECLARE_WRITE_LINE_MEMBER(coincounter_1_w);
	DECLARE_READ8_MEMBER(ret_ff);
	DECLARE_READ8_MEMBER(hanamai_keyboard_0_r);
	DECLARE_READ8_MEMBER(hanamai_keyboard_1_r);
	DECLARE_WRITE8_MEMBER(hanamai_keyboard_w);
	DECLARE_WRITE8_MEMBER(dynax_rombank_w);
	DECLARE_WRITE8_MEMBER(jantouki_sound_rombank_w);
	DECLARE_WRITE8_MEMBER(cdracula_sound_rombank_w);
	DECLARE_WRITE8_MEMBER(hnoridur_rombank_w);
	DECLARE_WRITE8_MEMBER(hnoridur_palbank_w);
	DECLARE_WRITE8_MEMBER(hnoridur_palette_lo_w);
	DECLARE_WRITE8_MEMBER(hnoridur_palette_hi_w);
	void hnoridur_palette_update(offs_t offset);
	DECLARE_WRITE8_MEMBER(nanajign_palette_lo_w);
	DECLARE_WRITE8_MEMBER(nanajign_palette_hi_w);
	void nanajign_palette_update(offs_t offset);
	DECLARE_WRITE8_MEMBER(adpcm_data_w);
	DECLARE_WRITE_LINE_MEMBER(hjingi_lockout_w);
	DECLARE_WRITE_LINE_MEMBER(hjingi_hopper_w);
	uint8_t hjingi_hopper_bit();
	DECLARE_READ8_MEMBER(hjingi_keyboard_0_r);
	DECLARE_READ8_MEMBER(hjingi_keyboard_1_r);
	DECLARE_WRITE8_MEMBER(yarunara_input_w);
	DECLARE_READ8_MEMBER(yarunara_input_r);
	DECLARE_WRITE8_MEMBER(yarunara_rombank_w);
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
	DECLARE_WRITE_LINE_MEMBER(tenkai_6c_w);
	DECLARE_WRITE_LINE_MEMBER(tenkai_70_w);
	DECLARE_WRITE8_MEMBER(tenkai_blit_romregion_w);
	DECLARE_WRITE8_MEMBER(tenkai_mainlatch_w);
	DECLARE_READ8_MEMBER(gekisha_keyboard_0_r);
	DECLARE_READ8_MEMBER(gekisha_keyboard_1_r);
	DECLARE_WRITE8_MEMBER(gekisha_hopper_w);
	DECLARE_WRITE8_MEMBER(gekisha_p4_w);
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
	DECLARE_WRITE_LINE_MEMBER(blit_palbank_w);
	DECLARE_WRITE_LINE_MEMBER(blit2_palbank_w);
	DECLARE_WRITE_LINE_MEMBER(layer_half_w);
	DECLARE_WRITE_LINE_MEMBER(layer_half2_w);
	DECLARE_WRITE_LINE_MEMBER(mjdialq2_blit_dest0_w);
	DECLARE_WRITE_LINE_MEMBER(mjdialq2_blit_dest1_w);
	DECLARE_WRITE8_MEMBER(dynax_layer_enable_w);
	DECLARE_WRITE8_MEMBER(jantouki_layer_enable_w);
	DECLARE_WRITE_LINE_MEMBER(mjdialq2_layer0_enable_w);
	DECLARE_WRITE_LINE_MEMBER(mjdialq2_layer1_enable_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
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
	DECLARE_DRIVER_INIT(maya_common);
	DECLARE_DRIVER_INIT(mayac);
	DECLARE_DRIVER_INIT(maya);

	uint32_t screen_update_hanamai(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_hnoridur(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_sprtmtch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_mjdialq2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_jantouki_top(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_jantouki_bottom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cdracula(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(sprtmtch_vblank_interrupt);
	INTERRUPT_GEN_MEMBER(jantouki_vblank_interrupt);
	INTERRUPT_GEN_MEMBER(jantouki_sound_vblank_interrupt);
	INTERRUPT_GEN_MEMBER(yarunara_clock_interrupt);

	void tenkai_update_rombank();

	DECLARE_WRITE_LINE_MEMBER(sprtmtch_sound_callback);
	DECLARE_WRITE_LINE_MEMBER(jantouki_sound_callback);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int_cpu1);

	DECLARE_MACHINE_RESET(adpcm);
	DECLARE_WRITE8_MEMBER(adpcm_reset_w);
	DECLARE_WRITE_LINE_MEMBER(adpcm_reset_kludge_w);
	DECLARE_WRITE8_MEMBER(tenkai_dswsel_w);
	DECLARE_READ8_MEMBER(tenkai_dsw_r);
	DECLARE_WRITE_LINE_MEMBER(tenkai_blitter_ack_w);
	DECLARE_MACHINE_RESET(dynax);
	DECLARE_MACHINE_START(dynax);
	DECLARE_MACHINE_START(hanamai);
	DECLARE_MACHINE_START(hjingi);
	DECLARE_VIDEO_START(hanamai);
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
	DECLARE_VIDEO_START(tenkai);

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
	void tenkai_update_irq();
	void jantouki_sound_update_irq();
	void tenkai_show_6c();
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
	void htengoku_banked_map(address_map &map);
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
};
