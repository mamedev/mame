// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************

    Break Thru

***************************************************************************/

#include "machine/gen_latch.h"

class brkthru_state : public driver_device
{
public:
	brkthru_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_fg_videoram(*this, "fg_videoram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	int     m_bgscroll;
	int     m_bgbasecolor;
	int     m_flipscreen;
	//uint8_t *m_brkthru_nmi_enable; /* needs to be tracked down */

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	uint8_t   m_nmi_mask;
	void brkthru_1803_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void darwin_0803_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void brkthru_soundlatch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void brkthru_bgram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void brkthru_fgram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void brkthru_1800_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coin_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void init_brkthru();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_brkthru(palette_device &palette);
	uint32_t screen_update_brkthru(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(device_t &device);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int prio );
};
