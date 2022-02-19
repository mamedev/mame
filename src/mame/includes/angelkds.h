// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************

    Angel Kids

*************************************************************************/
#ifndef MAME_INCLUDES_ANGELKDS_H
#define MAME_INCLUDES_ANGELKDS_H

#pragma once

#include "tilemap.h"

class angelkds_state : public driver_device
{
public:
	angelkds_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bgtopvideoram(*this, "bgtopvideoram"),
		m_bgbotvideoram(*this, "bgbotvideoram"),
		m_txvideoram(*this, "txvideoram"),
		m_spriteram(*this, "spriteram"),
		m_subcpu(*this, "sub"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_bgtopvideoram;
	required_shared_ptr<uint8_t> m_bgbotvideoram;
	required_shared_ptr<uint8_t> m_txvideoram;
	required_shared_ptr<uint8_t> m_spriteram;

	tilemap_t    *m_tx_tilemap;
	tilemap_t    *m_bgbot_tilemap;
	tilemap_t    *m_bgtop_tilemap;
	int        m_txbank;
	int        m_bgbotbank;
	int        m_bgtopbank;

	uint8_t      m_sound[4];
	uint8_t      m_sound2[4];
	uint8_t      m_layer_ctrl;

	/* devices */
	required_device<cpu_device> m_subcpu;
	uint8_t angeklds_ff_r() { return 0xff; }
	void angelkds_cpu_bank_write(uint8_t data);
	void angelkds_main_sound_w(offs_t offset, uint8_t data);
	uint8_t angelkds_main_sound_r(offs_t offset);
	void angelkds_sub_sound_w(offs_t offset, uint8_t data);
	uint8_t angelkds_sub_sound_r(offs_t offset);
	void angelkds_txvideoram_w(offs_t offset, uint8_t data);
	void angelkds_txbank_write(uint8_t data);
	void angelkds_bgtopvideoram_w(offs_t offset, uint8_t data);
	void angelkds_bgtopbank_write(uint8_t data);
	void angelkds_bgtopscroll_write(uint8_t data);
	void angelkds_bgbotvideoram_w(offs_t offset, uint8_t data);
	void angelkds_bgbotbank_write(uint8_t data);
	void angelkds_bgbotscroll_write(uint8_t data);
	void angelkds_layer_ctrl_write(uint8_t data);
	void init_angelkds();
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_bgtop_tile_info);
	TILE_GET_INFO_MEMBER(get_bgbot_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_angelkds(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int enable_n);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;
	void angelkds(machine_config &config);
	void spcpostn(machine_config &config);
	void decrypted_opcodes_map(address_map &map);
	void main_map(address_map &map);
	void main_portmap(address_map &map);
	void sub_map(address_map &map);
	void sub_portmap(address_map &map);
};

#endif // MAME_INCLUDES_ANGELKDS_H
