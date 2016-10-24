// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*************************************************************************

    Knuckle Joe

*************************************************************************/

#include "machine/gen_latch.h"
#include "sound/ay8910.h"

class kncljoe_state : public driver_device
{
public:
	kncljoe_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_scrollregs(*this, "scrollregs"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_ay8910(*this, "aysnd"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_scrollregs;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int        m_tile_bank;
	int         m_sprite_bank;
	int        m_flipscreen;

	/* misc */
	uint8_t      m_port1;
	uint8_t      m_port2;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<ay8910_device> m_ay8910;
	required_device<generic_latch_8_device> m_soundlatch;

	void sound_cmd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kncljoe_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kncljoe_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kncljoe_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m6803_port1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m6803_port2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t m6803_port1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t m6803_port2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void unused_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_kncljoe(palette_device &palette);
	uint32_t screen_update_kncljoe(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sound_nmi(device_t &device);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
