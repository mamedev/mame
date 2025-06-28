// license:BSD-3-Clause
// copyright-holders:Luca Elia, Nicola Salmoria
/***************************************************************************

      Dynax hardware

***************************************************************************/
#ifndef MAME_DYNAX_DYNAX_H
#define MAME_DYNAX_DYNAX_H

#pragma once

#include "dynax_blitter_rev2.h"

#include "machine/74259.h"
#include "machine/bankdev.h"
#include "machine/gen_latch.h"
#include "machine/rstbuf.h"
#include "machine/ticket.h"
#include "sound/msm5205.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"


class dynax_state : public driver_device
{
public:
	dynax_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_mainlatch(*this, "mainlatch")
		, m_blitter(*this, "blitter")
		, m_hopper(*this, "hopper")
		, m_bankdev(*this, "bankdev")
		, m_mainirq(*this, "mainirq")
		, m_io_key{ { *this, "KEY%u", 0U }, { *this, "KEY%u", 5U } }
	{
	}

	void mjfriday(machine_config &config) ATTR_COLD;
	void gekisha(machine_config &config) ATTR_COLD;
	void majrjhdx(machine_config &config) ATTR_COLD;
	void tenkai(machine_config &config) ATTR_COLD;
	void ougonhai(machine_config &config) ATTR_COLD;
	void ougonhaib1(machine_config &config) ATTR_COLD;
	void mjreach(machine_config &config) ATTR_COLD;
	void mjreachp2(machine_config &config) ATTR_COLD;
	void mjdialq2(machine_config &config) ATTR_COLD;
	void sprtmtch(machine_config &config) ATTR_COLD;
	void qyjdzjp(machine_config &config) ATTR_COLD;

	void blitter_ack_w(int state);
	void sprtmtch_blitter_irq_w(int state);
	void sprtmtch_vblank_w(int state);
	void layer_half_w(int state);
	void layer_half2_w(int state);
	void flipscreen_w(int state);

	void hnoridur_blit_pixel_w(offs_t offset, uint8_t data);
	void dynax_blit_scrollx_w(uint8_t data);
	void dynax_blit_scrolly_w(uint8_t data);

protected:
	required_device<cpu_device> m_maincpu;
	optional_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<ls259_device> m_mainlatch;
	optional_device<dynax_blitter_rev2_device> m_blitter;
	optional_device<hopper_device> m_hopper;
	optional_device<address_map_bank_device> m_bankdev;
	optional_device<rst_pos_buffer_device> m_mainirq;
	optional_ioport_array<5> m_io_key[2];

	/* input / output */
	uint8_t m_input_sel = 0U;
	uint8_t m_input_mode = 0U;
	uint8_t m_dsw_sel = 0U;
	uint8_t m_keyb = 0U;

	void dynax_vblank_ack_w(uint8_t data);
	void dynax_blit_dest_w(uint8_t data);
	void dynax_blit_romregion_w(uint8_t data);
	void dynax_extra_scrollx_w(uint8_t data);
	void dynax_extra_scrolly_w(uint8_t data);

	void tenkai_palette_w(offs_t offset, uint8_t data);

	void hanamai_copylayer(bitmap_ind16 &bitmap, const rectangle &cliprect, int i);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// Nothing below here is used by ddenlovr.cpp
	// TODO: further decouple dynax.cpp and ddenlovr.cpp
	void coincounter_0_w(int state);
	void coincounter_1_w(int state);
	uint8_t ret_ff();
	template <unsigned N> uint8_t hanamai_keyboard_r();
	uint8_t hjingi_keyboard_0_r();
	uint8_t mjelctrn_keyboard_1_r();
	uint8_t mjelctrn_dsw_r();
	void hanamai_keyboard_w(uint8_t data);
	void dynax_rombank_w(uint8_t data);
	void hnoridur_rombank_w(uint8_t data);
	void dynax_blit_palette23_w(uint8_t data);
	void blit_palbank_w(int state);
	void dynax_blit_backpen_w(uint8_t data);
	void dynax_blit_palette01_w(uint8_t data);
	void dynax_layer_enable_w(uint8_t data);
	void hanamai_priority_w(uint8_t data);
	void yarunara_blit_romregion_w(uint8_t data);
	void hnoridur_palbank_w(uint8_t data);
	void nanajign_palette_lo_w(offs_t offset, uint8_t data);
	void nanajign_palette_hi_w(offs_t offset, uint8_t data);
	void nanajign_palette_update(offs_t offset);

	uint32_t screen_update_hanamai(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_hnoridur(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void sprtmtch_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(hanamai);
	DECLARE_VIDEO_START(hnoridur);
	DECLARE_VIDEO_START(mjelctrn);

	int debug_mask();
	int debug_viewer(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void dynax_common_reset();

	void sprtmtch_mem_map(address_map &map) ATTR_COLD;
	void nanajign_mem_map(address_map &map) ATTR_COLD;
	void mjelctrn_banked_map(address_map &map) ATTR_COLD;

	// up to 8 layers, 2 images per layer (interleaved on screen)
	std::unique_ptr<uint8_t[]>  m_pixmap[8][2]{};

	/* irq */
	bool m_blitter_irq_mask = false;

	/* blitters */
	int m_blit_scroll_x = 0;
	int m_blit_scroll_y = 0;
	int m_blit_dest = 0;
	int m_blit_palbank = 0;
	int m_blit_palettes = 0;
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

private:
	void tenkai_ipsel_w(offs_t offset, uint8_t data);
	void tenkai_ip_w(uint8_t data);
	uint8_t tenkai_ip_r(offs_t offset);
	uint8_t tenkai_palette_r(offs_t offset);
	uint8_t tenkai_p3_r();
	void tenkai_p3_w(uint8_t data);
	void tenkai_p4_w(uint8_t data);
	uint8_t tenkai_p5_r();
	void tenkai_p6_w(uint8_t data);
	void tenkai_p7_w(uint8_t data);
	void tenkai_p8_w(uint8_t data);
	uint8_t tenkai_p8_r();
	void ougonhai_p7_w(uint8_t data);
	void mjreachp2_p8_w(uint8_t data);
	void tenkai_6c_w(int state);
	void tenkai_70_w(int state);
	void tenkai_blit_romregion_w(uint8_t data);
	uint8_t gekisha_keyboard_1_r();
	void gekisha_hopper_w(offs_t offset, uint8_t data);
	void gekisha_p4_w(uint8_t data);
	void tenkai_blit_dest_w(uint8_t data);
	void tenkai_blit_palette01_w(uint8_t data);
	void tenkai_blit_palette23_w(uint8_t data);
	void mjdialq2_blit_dest0_w(int state);
	void mjdialq2_blit_dest1_w(int state);
	void mjdialq2_layer0_enable_w(int state);
	void mjdialq2_layer1_enable_w(int state);
	void drgpunch_blit_pixel_w(offs_t offset, uint8_t data);
	void mjdialq2_blit_pixel_w(offs_t offset, uint8_t data);
	void tenkai_blit_scrollx_w(uint8_t data);
	void tenkai_blit_scrolly_w(uint8_t data);
	void tenkai_priority_w(uint8_t data);

	uint32_t screen_update_sprtmtch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_mjdialq2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mjfriday_vblank_w(int state);

	void tenkai_update_rombank();

	void tenkai_dswsel_w(uint8_t data);
	uint8_t tenkai_dsw_r();
	void tenkai_blitter_irq_w(int state);
	void tenkai_blitter_ack_w(int state);
	DECLARE_MACHINE_START(sprtmtch);
	DECLARE_VIDEO_START(sprtmtch);
	DECLARE_VIDEO_START(mjdialq2);

	//int blitter_drawgfx( int layer, int mask, memory_region *gfx, int src, int pen, int x, int y, int wrap, int flags );
	void mjdialq2_copylayer( bitmap_ind16 &bitmap, const rectangle &cliprect, int i );
	void tenkai_show_6c();

	void mjdialq2_mem_map(address_map &map) ATTR_COLD;

	void sprtmtch_io_map(address_map &map) ATTR_COLD;
	void mjfriday_io_map(address_map &map) ATTR_COLD;

	void tenkai_map(address_map &map) ATTR_COLD;
	void tenkai_banked_map(address_map &map) ATTR_COLD;

	void ougonhai_map(address_map &map) ATTR_COLD;
	void ougonhai_banked_map(address_map &map) ATTR_COLD;

	void gekisha_map(address_map &map) ATTR_COLD;
	void gekisha_banked_map(address_map &map) ATTR_COLD;

	void mjreachp2_map(address_map &map) ATTR_COLD;

	void qyjdzjp_io_map(address_map &map) ATTR_COLD;

	// misc
	int m_rombank = 0;
	uint8_t m_tenkai_p5_val = 0U;
	int m_tenkai_6c = 0;
	int m_tenkai_70 = 0;
	uint8_t m_gekisha_val[2]{};
};


class dynax_adpcm_state : public dynax_state
{
public:
	dynax_adpcm_state(const machine_config &mconfig, device_type type, const char *tag)
		: dynax_state(mconfig, type, tag)
		, m_msm(*this, "msm")
	{
	}

	void hanamai(machine_config &config) ATTR_COLD;
	void hnoridur(machine_config &config) ATTR_COLD;
	void hjingi(machine_config &config) ATTR_COLD;
	void yarunara(machine_config &config) ATTR_COLD;
	void mjangels(machine_config &config) ATTR_COLD;
	void quiztvqq(machine_config &config) ATTR_COLD;
	void mcnpshnt(machine_config &config) ATTR_COLD;
	void nanajign(machine_config &config) ATTR_COLD;
	void mjelctrn(machine_config &config) ATTR_COLD;
	void mjembase(machine_config &config) ATTR_COLD;
	void neruton(machine_config &config) ATTR_COLD;

	void init_mjelct3() ATTR_COLD;
	void init_mjelct3a() ATTR_COLD;

protected:
	required_device<msm5205_device> m_msm;

	// misc
	int m_msm5205next = 0;
	int m_resetkludge = 0;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void adpcm_data_w(uint8_t data);
	void adpcm_reset_w(uint8_t data);

private:
	// misc
	int m_toggle = 0;

	void hnoridur_palette_lo_w(offs_t offset, uint8_t data);
	void hnoridur_palette_hi_w(offs_t offset, uint8_t data);
	void hnoridur_palette_update(offs_t offset);
	void hjingi_lockout_w(int state);
	uint8_t hjingi_keyboard_1_r();
	void yarunara_input_w(offs_t offset, uint8_t data);
	uint8_t yarunara_input_r(offs_t offset);
	void yarunara_rombank_w(uint8_t data);
	void mjembase_blit_dest_w(uint8_t data);
	void mjembase_blit_palette23_w(uint8_t data);
	void hanamai_blit_pixel_w(offs_t offset, uint8_t data);
	void mjembase_priority_w(uint8_t data);

	void adpcm_int(int state);
	void adpcm_reset_kludge_w(int state);

	DECLARE_MACHINE_START(hanamai);
	DECLARE_MACHINE_START(hjingi);
	DECLARE_VIDEO_START(mcnpshnt);
	DECLARE_VIDEO_START(mjembase);
	DECLARE_VIDEO_START(neruton);

	void hnoridur_mem_map(address_map &map) ATTR_COLD;
	void quiztvqq_mem_map(address_map &map) ATTR_COLD;

	void hnoridur_banked_map(address_map &map) ATTR_COLD;
	void nanajign_banked_map(address_map &map) ATTR_COLD;
	void yarunara_banked_map(address_map &map) ATTR_COLD;
	void mjangels_banked_map(address_map &map) ATTR_COLD;

	void hanamai_io_map(address_map &map) ATTR_COLD;
	void hnoridur_io_map(address_map &map) ATTR_COLD;
	void yarunara_io_map(address_map &map) ATTR_COLD;
	void mcnpshnt_io_map(address_map &map) ATTR_COLD;
	void nanajign_io_map(address_map &map) ATTR_COLD;

	void hjingi_mem_map(address_map &map) ATTR_COLD;
	void hjingi_banked_map(address_map &map) ATTR_COLD;
	void hjingi_io_map(address_map &map) ATTR_COLD;

	void mjelctrn_io_map(address_map &map) ATTR_COLD;
	void mjembase_io_map(address_map &map) ATTR_COLD;
};


class jantouki_state : public dynax_adpcm_state
{
public:
	jantouki_state(const machine_config &mconfig, device_type type, const char *tag)
		: dynax_adpcm_state(mconfig, type, tag)
		, m_soundcpu(*this, "soundcpu")
		, m_soundlatch(*this, "soundlatch")
		, m_soundirq(*this, "soundirq")
		, m_blitter2(*this, "blitter2")
		, m_soundbank(*this, "bank2")
		, m_led(*this, "led0")
	{
	}

	void jantouki(machine_config &config) ATTR_COLD;
	void janyuki(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_soundcpu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<rst_pos_buffer_device> m_soundirq;
	required_device<dynax_blitter_rev2_device> m_blitter2;
	required_memory_bank m_soundbank;
	output_finder<> m_led;

	// IRQ
	bool m_blitter2_irq_mask = false;

	// blitters
	int m_blit2_scroll_x = 0;
	int m_blit2_scroll_y = 0;
	int m_blit2_dest = 0;
	int m_blit2_palbank = 0;
	int m_blit2_palettes = 0;

	// misc
	int m_toggle_cpu1 = 0;

	void jantouki_vblank_ack_w(uint8_t data);
	void jantouki_blitter_ack_w(int state);
	void jantouki_blitter_irq_w(int state);
	void jantouki_blitter2_ack_w(int state);
	void jantouki_blitter2_irq_w(int state);
	void jantouki_vblank_w(int state);
	void jantouki_sound_vblank_ack_w(uint8_t data);
	void jantouki_sound_rombank_w(uint8_t data);
	uint8_t jantouki_soundlatch_ack_r();
	uint8_t jantouki_blitter_busy_r();
	void jantouki_rombank_w(uint8_t data);
	uint8_t jantouki_soundlatch_status_r();

	void jantouki_layer_enable_w(offs_t offset, uint8_t data);
	void dynax_blit2_romregion_w(uint8_t data);
	void dynax_blit2_dest_w(uint8_t data);
	void dynax_blit_palette45_w(uint8_t data);
	void dynax_blit_palette67_w(uint8_t data);
	void blit2_palbank_w(int state);
	void jantouki_blit_pixel_w(offs_t offset, uint8_t data);
	void jantouki_blit2_pixel_w(offs_t offset, uint8_t data);
	void dynax_blit2_scrollx_w(uint8_t data);
	void dynax_blit2_scrolly_w(uint8_t data);

	void janyuki_palette(palette_device &palette) const ATTR_COLD;

	uint32_t screen_update_jantouki_top(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_jantouki_bottom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void jantouki_copylayer(bitmap_ind16 &bitmap, const rectangle &cliprect, int i, int y);

	void adpcm_int_cpu1(int state);

	DECLARE_VIDEO_START(jantouki);

	void jantouki_mem_map(address_map &map) ATTR_COLD;
	void jantouki_sound_mem_map(address_map &map) ATTR_COLD;
	void jantouki_io_map(address_map &map) ATTR_COLD;
	void jantouki_sound_io_map(address_map &map) ATTR_COLD;
};


class blktouch_state : public dynax_state
{
public:
	blktouch_state(const machine_config &mconfig, device_type type, const char *tag)
		: dynax_state(mconfig, type, tag)
		, m_blitter_gfx(*this, "blitter")
	{
	}

	void init_blktouch() ATTR_COLD;
	void init_mayac() ATTR_COLD;
	void init_maya() ATTR_COLD;

private:
	required_region_ptr<uint8_t> m_blitter_gfx;

	void init_maya_common() ATTR_COLD;
};


class cdracula_state : public dynax_state
{
public:
	cdracula_state(const machine_config &mconfig, device_type type, const char *tag)
		: dynax_state(mconfig, type, tag)
		, m_oki(*this, "oki")
	{
	}

	void cdracula(machine_config &config) ATTR_COLD;

private:
	required_device<okim6295_device> m_oki;

	void cdracula_sound_rombank_w(uint8_t data);
	void cdracula_blit_pixel_w(offs_t offset, uint8_t data);

	uint32_t screen_update_cdracula(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void cdracula_mem_map(address_map &map) ATTR_COLD;
	void cdracula_io_map(address_map &map) ATTR_COLD;
};


INPUT_PORTS_EXTERN(dynax_mahjong_keys);
INPUT_PORTS_EXTERN(dynax_hanafuda_keys_bet);

#endif // MAME_DYNAX_DYNAX_H
