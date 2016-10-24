// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

    Gaelco game hardware from 1991-1996

***************************************************************************/

#include "machine/gen_latch.h"

class gaelco_state : public driver_device
{
public:
	gaelco_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_audiocpu(*this, "audiocpu"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram"),
		m_screenram(*this, "screenram") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<cpu_device> m_audiocpu;
	optional_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_vregs;
	required_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_screenram;

	/* video-related */
	tilemap_t      *m_tilemap[2];

	void bigkarnk_sound_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bigkarnk_coin_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void OKIM6295_bankswitch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gaelco_vram_encrypted_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gaelco_encrypted_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void thoop_vram_encrypted_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void thoop_encrypted_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gaelco_vram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void get_tile_info_gaelco_screen0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_gaelco_screen1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	void video_start_bigkarnk();
	void video_start_maniacsq();

	uint32_t screen_update_bigkarnk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_maniacsq(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
};
