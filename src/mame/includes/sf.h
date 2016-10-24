// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*************************************************************************

    Street Fighter hardware

*************************************************************************/

#include "machine/gen_latch.h"
#include "sound/msm5205.h"

class sf_state : public driver_device
{
public:
	sf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm1(*this, "msm1"),
		m_msm2(*this, "msm2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_objectram(*this, "objectram")
	{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm1;
	required_device<msm5205_device> m_msm2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_objectram;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_tx_tilemap;
	int m_active;
	uint16_t m_bgscroll;
	uint16_t m_fgscroll;

	void coin_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void soundcmd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound2_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bg_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fg_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gfxctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void msm1_5205_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void msm2_5205_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline int invert( int nb );
	void draw_sprites( bitmap_ind16 &bitmap,const rectangle &cliprect );
	void write_dword( address_space &space, offs_t offset, uint32_t data );
};
