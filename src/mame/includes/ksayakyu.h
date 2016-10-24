// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina

/*************************************************************************

    Kusayakyuu

*************************************************************************/

#include "machine/gen_latch.h"

class ksayakyu_state : public driver_device
{
public:
	ksayakyu_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t    *m_tilemap;
	tilemap_t    *m_textmap;
	int        m_video_ctrl;
	int        m_flipscreen;

	/* misc */
	int        m_sound_status;
	void bank_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sound_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tomaincpu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ksayakyu_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ksayakyu_videoctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dummy1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dummy2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dummy3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_ksayakyu_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_text_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_ksayakyu(palette_device &palette);
	uint32_t screen_update_ksayakyu(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
};
