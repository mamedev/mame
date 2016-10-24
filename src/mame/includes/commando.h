// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Commando

*************************************************************************/

#include "video/bufsprite.h"

class commando_state : public driver_device
{
public:
	commando_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_spriteram(*this, "spriteram") ,
		m_videoram2(*this, "videoram2"),
		m_colorram2(*this, "colorram2"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_decrypted_opcodes(*this, "decrypted_opcodes") { }

	/* memory pointers */
	required_device<buffered_spriteram8_device> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_colorram2;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	uint8_t m_scroll_x[2];
	uint8_t m_scroll_y[2];

	/* devices */
	required_device<cpu_device> m_audiocpu;
	void commando_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void commando_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void commando_videoram2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void commando_colorram2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void commando_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void commando_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void commando_c804_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_spaceinv();
	void init_commando();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_commando(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void commando_interrupt(device_t &device);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;
};
