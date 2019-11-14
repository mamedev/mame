// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

   CPS1 bootleg "Final Crash" hardware

***************************************************************************/

#ifndef MAME_INCLUDES_FCRASH_H
#define MAME_INCLUDES_FCRASH_H

#pragma once

#include "includes/cps1.h"

class fcrash_state : public cps_state
{
public:
	fcrash_state(const machine_config &mconfig, device_type type, const char *tag)
		: cps_state(mconfig, type, tag, 1)
		, m_msm_1(*this, "msm1")
		, m_msm_2(*this, "msm2")
	{ }
	
	void fcrash(machine_config &config);
	void cawingbl(machine_config &config);
	void kodb(machine_config &config);
	void mtwinsb(machine_config &config);
	void sf2m1(machine_config &config);
	void sgyxz(machine_config &config);
	void wofabl(machine_config &config);
	void varthb(machine_config &config);
	
	void init_cawingbl();
	void init_kodb();
	void init_mtwinsb();
	void init_sf2m1();
	void init_wofabl();
	
protected:
	DECLARE_MACHINE_START(fcrash);
	DECLARE_MACHINE_RESET(fcrash);
	DECLARE_MACHINE_START(cawingbl);
	DECLARE_MACHINE_START(kodb);
	DECLARE_MACHINE_START(mtwinsb);
	DECLARE_MACHINE_START(sf2m1);
	DECLARE_MACHINE_START(sgyxz);
	
	DECLARE_WRITE16_MEMBER(fcrash_soundlatch_w);
	DECLARE_WRITE8_MEMBER(fcrash_snd_bankswitch_w);
	DECLARE_WRITE_LINE_MEMBER(m5205_int1);
	DECLARE_WRITE_LINE_MEMBER(m5205_int2);
	DECLARE_WRITE8_MEMBER(fcrash_msm5205_0_data_w);
	DECLARE_WRITE8_MEMBER(fcrash_msm5205_1_data_w);
	DECLARE_WRITE16_MEMBER(cawingbl_soundlatch_w);
	DECLARE_WRITE16_MEMBER(kodb_layer_w);
	DECLARE_WRITE16_MEMBER(mtwinsb_layer_w);
	DECLARE_WRITE16_MEMBER(sf2m1_layer_w);
	DECLARE_WRITE16_MEMBER(varthb_layer_w);
	DECLARE_WRITE16_MEMBER(varthb_layer2_w);
	
	uint32_t screen_update_fcrash(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void fcrash_update_transmasks();
	virtual void bootleg_render_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void fcrash_render_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int primask);
	void fcrash_render_high_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer);
	void fcrash_build_palette();

	void fcrash_map(address_map &map);
	void mtwinsb_map(address_map &map);
	void sf2m1_map(address_map &map);
	void sgyxz_map(address_map &map);
	void wofabl_map(address_map &map);
	void varthb_map(address_map &map);
	
	void fcrash_sound_map(address_map &map);
	void kodb_sound_map(address_map &map);
	void sgyxz_sound_map(address_map &map);
	
	/* sound hw */
	int m_sample_buffer1;
	int m_sample_buffer2;
	int m_sample_select1;
	int m_sample_select2;
	
	/* video config */
	uint8_t m_layer_enable_reg;
	uint8_t m_layer_mask_reg[4];
	int     m_layer_scroll1x_offset;
	int     m_layer_scroll2x_offset;
	int     m_layer_scroll3x_offset;
	int     m_sprite_base;
	int     m_sprite_list_end_marker;
	int     m_sprite_x_offset;
	std::unique_ptr<uint16_t[]> m_bootleg_sprite_ram;
	std::unique_ptr<uint16_t[]> m_bootleg_work_ram;
	
	optional_device<msm5205_device> m_msm_1;
	optional_device<msm5205_device> m_msm_2;
};

#endif // MAME_INCLUDES_FCRASH_H
