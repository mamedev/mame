// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*************************************************************************

    King of Boxer - Ring King

*************************************************************************/

#include "machine/gen_latch.h"

class kingofb_state : public driver_device
{
public:
	kingofb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_scroll_y(*this, "scroll_y"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_colorram2(*this, "colorram2"),
		m_spriteram(*this, "spriteram"),
		m_video_cpu(*this, "video"),
		m_sprite_cpu(*this, "sprite"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_scroll_y;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_colorram2;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;
	int        m_palette_bank;

	/* misc */
	int        m_nmi_enable;

	/* devices */
	required_device<cpu_device> m_video_cpu;
	required_device<cpu_device> m_sprite_cpu;
	void video_interrupt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sprite_interrupt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scroll_interrupt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kingofb_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kingofb_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kingofb_videoram2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kingofb_colorram2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kingofb_f800_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_ringkingw();
	void init_ringking3();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void ringking_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void video_start_kingofb();
	void palette_init_kingofb(palette_device &palette);
	void video_start_ringking();
	void palette_init_ringking(palette_device &palette);
	uint32_t screen_update_kingofb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ringking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void kingofb_interrupt(device_t &device);
	void palette_init_common( palette_device &palette, const uint8_t *color_prom, void (kingofb_state::*get_rgb_data)(const uint8_t *, int, int *, int *, int *) );
	void kingofb_get_rgb_data( const uint8_t *color_prom, int i, int *r_data, int *g_data, int *b_data );
	void ringking_get_rgb_data( const uint8_t *color_prom, int i, int *r_data, int *g_data, int *b_data );
	void kingofb_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ringking_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
};
