// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood

/*************************************************************************

    Double Dragon 3 & The Combatribes

*************************************************************************/

#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "video/bufsprite.h"


class ddragon3_state : public driver_device
{
public:
	ddragon3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{
		vblank_level = 6;
		raster_level = 5;
	}

	/* memory pointers */
	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_fg_videoram;
//  required_shared_ptr<uint16_t> m_spriteram;
	required_device<buffered_spriteram16_device> m_spriteram;

	/* video-related */
	tilemap_t         *m_fg_tilemap;
	tilemap_t         *m_bg_tilemap;
	uint16_t          m_vreg;
	uint16_t          m_bg_scrollx;
	uint16_t          m_bg_scrolly;
	uint16_t          m_fg_scrollx;
	uint16_t          m_fg_scrolly;
	uint16_t          m_bg_tilebase;

	uint16_t m_sprite_xoff;
	uint16_t m_bg0_dx;
	uint16_t m_bg1_dx[2];

	/* misc */
	uint16_t          m_io_reg[8];
	uint8_t m_pri;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void ddragon3_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ddragon3_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t ddragon3_scroll_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void ddragon3_bg_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ddragon3_fg_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void oki_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_ddragon3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ctribe(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ddragon3_scanline(timer_device &timer, void *ptr, int32_t param);
	void draw_sprites(  bitmap_ind16 &bitmap, const rectangle &cliprect );

	int vblank_level;
	int raster_level;
};


class wwfwfest_state : public ddragon3_state
{
public:
	wwfwfest_state(const machine_config &mconfig, device_type type, const char *tag)
		: ddragon3_state(mconfig, type, tag),
		m_fg0_videoram(*this, "fg0_videoram"),
		m_paletteram(*this, "palette")
	{
		vblank_level = 3;
		raster_level = 2;
	}

	/* wwfwfest has an extra layer */
	required_shared_ptr<uint16_t> m_fg0_videoram;
	required_shared_ptr<uint16_t> m_paletteram;
	tilemap_t *m_fg0_tilemap;
	void wwfwfest_fg0_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);


	//required_device<buffered_spriteram16_device> m_spriteram;
	void wwfwfest_priority_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wwfwfest_irq_ack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void wwfwfest_flipscreen_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t wwfwfest_paletteram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void wwfwfest_paletteram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void wwfwfest_soundwrite(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	ioport_value dsw_3f_r(ioport_field &field, void *param);
	ioport_value dsw_c0_r(ioport_field &field, void *param);
	void get_fg0_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void video_start() override;
	void video_start_wwfwfstb();
	uint32_t screen_update_wwfwfest(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

};
