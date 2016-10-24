// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

    Double Dribble

***************************************************************************/

#include "sound/flt_rc.h"
#include "sound/vlm5030.h"

class ddribble_state : public driver_device
{
public:
	ddribble_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_fg_videoram(*this, "fg_videoram"),
		m_spriteram_1(*this, "spriteram_1"),
		m_sharedram(*this, "sharedram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram_2(*this, "spriteram_2"),
		m_snd_sharedram(*this, "snd_sharedram"),
		m_maincpu(*this, "maincpu"),
		m_vlm(*this, "vlm"),
		m_filter1(*this, "filter1"),
		m_filter2(*this, "filter2"),
		m_filter3(*this, "filter3"),
		m_gfxdecode(*this, "gfxdecode") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_spriteram_1;
	required_shared_ptr<uint8_t> m_sharedram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_spriteram_2;
	required_shared_ptr<uint8_t> m_snd_sharedram;

	/* video-related */
	tilemap_t     *m_fg_tilemap;
	tilemap_t     *m_bg_tilemap;
	int         m_vregs[2][5];
	int         m_charbank[2];

	/* misc */
	int         m_int_enable_0;
	int         m_int_enable_1;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<vlm5030_device> m_vlm;
	required_device<filter_rc_device> m_filter1;
	required_device<filter_rc_device> m_filter2;
	required_device<filter_rc_device> m_filter3;
	required_device<gfxdecode_device> m_gfxdecode;

	void ddribble_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ddribble_sharedram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ddribble_sharedram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ddribble_snd_sharedram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ddribble_snd_sharedram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ddribble_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void K005885_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void K005885_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ddribble_fg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ddribble_bg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ddribble_vlm5030_busy_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ddribble_vlm5030_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	tilemap_memory_index tilemap_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_ddribble(palette_device &palette);
	uint32_t screen_update_ddribble(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ddribble_interrupt_0(device_t &device);
	void ddribble_interrupt_1(device_t &device);
	void draw_sprites(  bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t* source, int lenght, int gfxset, int flipscreen );
};
