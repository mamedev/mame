// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************

    Angel Kids

*************************************************************************/
#ifndef MAME_INCLUDES_ANGELKDS_H
#define MAME_INCLUDES_ANGELKDS_H

#pragma once

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
	DECLARE_READ8_MEMBER(angeklds_ff_r) { return 0xff; };
	DECLARE_WRITE8_MEMBER(angelkds_cpu_bank_write);
	DECLARE_WRITE8_MEMBER(angelkds_main_sound_w);
	DECLARE_READ8_MEMBER(angelkds_main_sound_r);
	DECLARE_WRITE8_MEMBER(angelkds_sub_sound_w);
	DECLARE_READ8_MEMBER(angelkds_sub_sound_r);
	DECLARE_WRITE8_MEMBER(angelkds_txvideoram_w);
	DECLARE_WRITE8_MEMBER(angelkds_txbank_write);
	DECLARE_WRITE8_MEMBER(angelkds_bgtopvideoram_w);
	DECLARE_WRITE8_MEMBER(angelkds_bgtopbank_write);
	DECLARE_WRITE8_MEMBER(angelkds_bgtopscroll_write);
	DECLARE_WRITE8_MEMBER(angelkds_bgbotvideoram_w);
	DECLARE_WRITE8_MEMBER(angelkds_bgbotbank_write);
	DECLARE_WRITE8_MEMBER(angelkds_bgbotscroll_write);
	DECLARE_WRITE8_MEMBER(angelkds_layer_ctrl_write);
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
