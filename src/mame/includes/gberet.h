// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Green Beret

***************************************************************************/

#include "sound/sn76496.h"

class gberet_state : public driver_device
{
public:
	gberet_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram2(*this, "spriteram2"),
		m_spriteram(*this, "spriteram"),
		m_scrollram(*this, "scrollram"),
		m_soundlatch(*this, "soundlatch"),
		m_sn(*this, "snsnd") ,
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_spriteram2;
	required_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_scrollram;
	optional_shared_ptr<uint8_t> m_soundlatch;

	/* devices */
	required_device<sn76489a_device> m_sn;

	/* video-related */
	tilemap_t * m_bg_tilemap;
	uint8_t       m_spritebank;

	/* misc */
	uint8_t       m_interrupt_mask;
	uint8_t       m_interrupt_ticks;
	void gberet_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mrgoemon_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gberet_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gberet_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gberetb_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t gberetb_irq_ack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void gberetb_nmi_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gberet_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gberet_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gberet_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gberet_sprite_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gberetb_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_mrgoemon();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_gberet();
	void machine_reset_gberet();
	void video_start_gberet();
	void palette_init_gberet(palette_device &palette);
	uint32_t screen_update_gberet(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_gberetb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gberet_interrupt_tick(timer_device &timer, void *ptr, int32_t param);
	void gberet_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void gberetb_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
