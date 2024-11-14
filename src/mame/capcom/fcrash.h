// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

   CPS1 bootleg "Final Crash" hardware

***************************************************************************/

#ifndef MAME_CAPCOM_FCRASH_H
#define MAME_CAPCOM_FCRASH_H

#pragma once

#include "cps1.h"

class fcrash_state : public cps_state
{
public:
	fcrash_state(const machine_config &mconfig, device_type type, const char *tag)
		: cps_state(mconfig, type, tag, 1)
		, m_msm_1(*this, "msm1")
		, m_msm_2(*this, "msm2")
		, m_okibank(*this, "okibank")
		, m_sgyxz_dsw(*this, { "DSWA", "DSWB", "DSWC" })
	{ }

	void fcrash(machine_config &config);
	void cawingbl(machine_config &config);
	void ffightblb(machine_config &config);
	void kodb(machine_config &config);
	void mtwinsb(machine_config &config);
	void sf2m1(machine_config &config);
	void sgyxz(machine_config &config);
	void wofabl(machine_config &config);
	void wofr1bl(machine_config &config);
	void varthb(machine_config &config);

	void init_cawingbl();
	void init_kodb();
	void init_mtwinsb();
	void init_sf2m1();
	void init_wofr1bl();

protected:
	DECLARE_MACHINE_START(fcrash);
	DECLARE_MACHINE_RESET(fcrash);
	DECLARE_MACHINE_START(cawingbl);
	DECLARE_MACHINE_START(ffightblb);
	DECLARE_MACHINE_START(kodb);
	DECLARE_MACHINE_START(mtwinsb);
	DECLARE_MACHINE_START(sf2m1);
	DECLARE_MACHINE_START(sgyxz);
	DECLARE_MACHINE_RESET(sgyxz);
	DECLARE_MACHINE_START(wofr1bl);

	void fcrash_soundlatch_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fcrash_snd_bankswitch_w(uint8_t data);
	void m5205_int1(int state);
	void m5205_int2(int state);
	void fcrash_msm5205_0_data_w(uint8_t data);
	void fcrash_msm5205_1_data_w(uint8_t data);
	void cawingbl_soundlatch_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void kodb_layer_w(offs_t offset, uint16_t data);
	void mtwinsb_layer_w(offs_t offset, uint16_t data);
	void sf2m1_layer_w(offs_t offset, uint16_t data);
	void varthb_layer_w(offs_t offset, uint16_t data);
	void varthb_layer2_w(uint16_t data);
	uint16_t sgyxz_dsw_r(offs_t offset);
	void wofr1bl_layer_w(offs_t offset, uint16_t data);
	void wofr1bl_layer2_w(uint16_t data);
	void wofr1bl_spr_base_w(uint16_t data);

	uint32_t screen_update_fcrash(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void fcrash_update_transmasks();
	virtual void bootleg_render_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void fcrash_render_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int primask);
	void fcrash_render_high_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer);
	virtual void fcrash_build_palette();

	void fcrash_map(address_map &map) ATTR_COLD;
	void mtwinsb_map(address_map &map) ATTR_COLD;
	void sf2m1_map(address_map &map) ATTR_COLD;
	void sgyxz_map(address_map &map) ATTR_COLD;
	void wofabl_map(address_map &map) ATTR_COLD;
	void wofr1bl_map(address_map &map) ATTR_COLD;
	void varthb_map(address_map &map) ATTR_COLD;

	void fcrash_sound_map(address_map &map) ATTR_COLD;
	void ffightblb_sound_map(address_map &map) ATTR_COLD;
	void ffightblb_oki_map(address_map &map) ATTR_COLD;
	void kodb_sound_map(address_map &map) ATTR_COLD;
	void sgyxz_sound_map(address_map &map) ATTR_COLD;

	/* sound hw */
	int m_sample_buffer1 = 0;
	int m_sample_buffer2 = 0;
	int m_sample_select1 = 0;
	int m_sample_select2 = 0;

	/* video config */
	uint8_t m_layer_enable_reg = 0;
	uint8_t m_layer_mask_reg[4] = {};
	int     m_layer_scroll1x_offset = 0;
	int     m_layer_scroll2x_offset = 0;
	int     m_layer_scroll3x_offset = 0;
	int     m_sprite_base = 0;
	int     m_sprite_list_end_marker = 0;
	int     m_sprite_x_offset = 0;
	std::unique_ptr<uint16_t[]> m_bootleg_sprite_ram;
	std::unique_ptr<uint16_t[]> m_bootleg_work_ram;

	optional_device<msm5205_device> m_msm_1;
	optional_device<msm5205_device> m_msm_2;

	optional_memory_bank m_okibank;

	optional_ioport_array<3> m_sgyxz_dsw;
};

class cps1bl_no_brgt : public fcrash_state
{
public:
	cps1bl_no_brgt(const machine_config &mconfig, device_type type, const char *tag)
		: fcrash_state(mconfig, type, tag)
	{ }

private:
	void fcrash_build_palette() override;
};

#endif // MAME_CAPCOM_FCRASH_H
