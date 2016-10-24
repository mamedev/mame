// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************

    Super Slams

*************************************************************************/

#include "machine/gen_latch.h"
#include "video/k053936.h"

class suprslam_state : public driver_device
{
public:
	suprslam_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_screen_videoram(*this, "screen_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_sp_videoram(*this, "sp_videoram"),
		m_spriteram(*this, "spriteram"),
		m_spr_ctrl(*this, "spr_ctrl"),
		m_screen_vregs(*this, "screen_vregs"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k053936(*this, "k053936"),
		m_spr(*this, "vsystem_spr"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_screen_videoram;
	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_sp_videoram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_spr_ctrl;
	required_shared_ptr<uint16_t> m_screen_vregs;

	/* video-related */
	tilemap_t     *m_screen_tilemap;
	tilemap_t     *m_bg_tilemap;
	uint16_t      m_screen_bank;
	uint16_t      m_bg_bank;
	uint32_t  suprslam_tile_callback( uint32_t code );

	/* misc */
	int         m_pending_command;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k053936_device> m_k053936;
	required_device<vsystem_spr_device> m_spr;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<generic_latch_8_device> m_soundlatch;

	void sound_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void pending_command_clear_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void suprslam_sh_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void suprslam_screen_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void suprslam_bg_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void suprslam_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void get_suprslam_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_suprslam_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_suprslam(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
