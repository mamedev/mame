// license:BSD-3-Clause
// copyright-holders:Mike Balfour, Zsolt Vasvari
#ifndef MAME_TAITO_BKING_H
#define MAME_TAITO_BKING_H

#pragma once

#include "taito68705.h"

#include "machine/gen_latch.h"
#include "machine/input_merger.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class bking_state : public driver_device
{
public:
	bking_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_playfield_ram(*this, "playfield_ram"),
		m_audiocpu(*this, "audiocpu"),
		m_bmcu(*this, "bmcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_soundnmi(*this, "soundnmi")
	{
	}

	void bking(machine_config &config);
	void bking3(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_playfield_ram;

	/* video-related */
	bitmap_ind16    m_colmap_bg = 0;
	bitmap_ind16    m_colmap_ball = 0;
	tilemap_t     *m_bg_tilemap = nullptr;
	int         m_pc3259_output[4]{};
	int         m_pc3259_mask = 0;
	uint8_t       m_xld1 = 0U;
	uint8_t       m_xld2 = 0U;
	uint8_t       m_xld3 = 0U;
	uint8_t       m_yld1 = 0U;
	uint8_t       m_yld2 = 0U;
	uint8_t       m_yld3 = 0U;
	int         m_ball1_pic = 0;
	int         m_ball2_pic = 0;
	int         m_crow_pic = 0;
	int         m_crow_flip = 0;
	int         m_palette_bank = 0;
	int         m_controller = 0;
	int         m_hit = 0;

	/* misc */
	int         m_addr_h = 0;
	int         m_addr_l = 0;

	/* devices */
	required_device<cpu_device> m_audiocpu;
	optional_device<taito68705_mcu_device> m_bmcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<input_merger_device> m_soundnmi;

	uint8_t bking_sndnmi_disable_r();
	void bking_sndnmi_enable_w(uint8_t data);
	void bking_soundlatch_w(uint8_t data);
	void bking3_addr_l_w(uint8_t data);
	void bking3_addr_h_w(uint8_t data);
	uint8_t bking3_extrarom_r();
	uint8_t bking3_ext_check_r();
	uint8_t bking3_mcu_status_r();
	void bking_xld1_w(uint8_t data);
	void bking_yld1_w(uint8_t data);
	void bking_xld2_w(uint8_t data);
	void bking_yld2_w(uint8_t data);
	void bking_xld3_w(uint8_t data);
	void bking_yld3_w(uint8_t data);
	void bking_cont1_w(uint8_t data);
	void bking_cont2_w(uint8_t data);
	void bking_cont3_w(uint8_t data);
	void bking_msk_w(uint8_t data);
	void bking_hitclr_w(uint8_t data);
	void bking_playfield_w(offs_t offset, uint8_t data);
	uint8_t bking_input_port_5_r();
	uint8_t bking_input_port_6_r();
	uint8_t bking_pos_r(offs_t offset);
	void unk_w(uint8_t data);
	void port_b_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void bking_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(bking3);
	DECLARE_MACHINE_RESET(bking3);
	DECLARE_MACHINE_RESET(common);
	uint32_t screen_update_bking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank_bking(int state);
	void bking3_io_map(address_map &map) ATTR_COLD;
	void bking_audio_map(address_map &map) ATTR_COLD;
	void bking_io_map(address_map &map) ATTR_COLD;
	void bking_map(address_map &map) ATTR_COLD;
};

#endif // MAME_TAITO_BKING_H
