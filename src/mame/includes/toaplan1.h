// license:BSD-3-Clause
// copyright-holders:Darren Olafson, Quench
/***************************************************************************
                ToaPlan game hardware from 1988-1991
                ------------------------------------
****************************************************************************/

#include "cpu/m68000/m68000.h"
#include "video/toaplan_scu.h"

class toaplan1_state : public driver_device
{
public:
	toaplan1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bgpaletteram(*this, "bgpalette"),
		m_fgpaletteram(*this, "fgpalette"),
		m_sharedram(*this, "sharedram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_dsp(*this, "dsp"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_shared_ptr<uint16_t> m_bgpaletteram;
	required_shared_ptr<uint16_t> m_fgpaletteram;

	optional_shared_ptr<uint8_t> m_sharedram;

	int m_coin_count; /* coin count increments on startup ? , so don't count it */
	int m_intenable;

	/* Demon world */
	int m_dsp_on;
	int m_dsp_BIO;
	int m_dsp_execute;
	uint32_t m_dsp_addr_w;
	uint32_t m_main_ram_seg;

	uint8_t m_vimana_coins[2];
	uint8_t m_vimana_credits;
	uint8_t m_vimana_latch;

	std::unique_ptr<uint16_t[]> m_pf4_tilevram16;   /*  ||  Drawn in this order */
	std::unique_ptr<uint16_t[]> m_pf3_tilevram16;   /*  ||  */
	std::unique_ptr<uint16_t[]> m_pf2_tilevram16;   /* \||/ */
	std::unique_ptr<uint16_t[]> m_pf1_tilevram16;   /*  \/  */

	optional_shared_ptr<uint16_t> m_spriteram;
	std::unique_ptr<uint16_t[]> m_buffered_spriteram;
	std::unique_ptr<uint16_t[]> m_spritesizeram16;
	std::unique_ptr<uint16_t[]> m_buffered_spritesizeram16;

	int32_t m_bcu_flipscreen;     /* Tile   controller flip flag */
	int32_t m_fcu_flipscreen;     /* Sprite controller flip flag */

	int32_t m_pf_voffs;
	int32_t m_spriteram_offs;

	int32_t m_pf1_scrollx;
	int32_t m_pf1_scrolly;
	int32_t m_pf2_scrollx;
	int32_t m_pf2_scrolly;
	int32_t m_pf3_scrollx;
	int32_t m_pf3_scrolly;
	int32_t m_pf4_scrollx;
	int32_t m_pf4_scrolly;

#ifdef MAME_DEBUG
	int m_display_pf1;
	int m_display_pf2;
	int m_display_pf3;
	int m_display_pf4;
	int m_displog;
#endif

	int32_t m_tiles_offsetx;
	int32_t m_tiles_offsety;

	tilemap_t *m_pf1_tilemap;
	tilemap_t *m_pf2_tilemap;
	tilemap_t *m_pf3_tilemap;
	tilemap_t *m_pf4_tilemap;

	void toaplan1_intenable_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void demonwld_dsp_addrsel_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t demonwld_dsp_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void demonwld_dsp_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void demonwld_dsp_bio_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	int demonwld_BIO_r();
	void demonwld_dsp_ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t samesame_port_6_word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t vimana_system_port_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t vimana_mcu_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void vimana_mcu_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t toaplan1_shared_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void toaplan1_shared_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void toaplan1_reset_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void toaplan1_coin_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void samesame_coin_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t toaplan1_frame_done_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void toaplan1_tile_offsets_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void toaplan1_bcu_flipscreen_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void toaplan1_fcu_flipscreen_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t toaplan1_spriteram_offs_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void toaplan1_spriteram_offs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void toaplan1_bgpalette_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void toaplan1_fgpalette_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t toaplan1_spriteram16_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void toaplan1_spriteram16_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t toaplan1_spritesizeram16_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void toaplan1_spritesizeram16_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void toaplan1_bcu_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t toaplan1_tileram_offs_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void toaplan1_tileram_offs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t toaplan1_tileram16_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void toaplan1_tileram16_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t toaplan1_scroll_regs_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void toaplan1_scroll_regs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void init_toaplan1();
	void init_demonwld();
	void init_vimana();
	void get_pf1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_pf2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_pf3_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_pf4_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_reset_toaplan1();
	void video_start_toaplan1();
	void machine_reset_zerowing();
	void machine_reset_demonwld();
	void machine_reset_vimana();
	uint32_t screen_update_toaplan1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void screen_eof_toaplan1(screen_device &screen, bool state);
	void screen_eof_samesame(screen_device &screen, bool state);
	void toaplan1_interrupt(device_t &device);

	void demonwld_restore_dsp();
	void toaplan1_create_tilemaps();
	void toaplan1_vram_alloc();
	void toaplan1_spritevram_alloc();
	void toaplan1_set_scrolls();
	void register_common();
	void toaplan1_log_vram();
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void demonwld_dsp(int enable);
	void toaplan1_reset_sound();
	void toaplan1_driver_savestate();
	void demonwld_driver_savestate();
	void vimana_driver_savestate();
	void toaplan1_reset_callback(int state);
	required_device<m68000_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_dsp;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};

class toaplan1_rallybik_state : public toaplan1_state
{
public:
	toaplan1_rallybik_state(const machine_config &mconfig, device_type type, const char *tag)
		: toaplan1_state(mconfig, type, tag),
		m_spritegen(*this, "scu")
	{
	}

	void rallybik_coin_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t rallybik_tileram16_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void video_start_rallybik();
	uint32_t screen_update_rallybik(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_rallybik(screen_device &screen, bool state);

	required_device<toaplan_scu_device> m_spritegen;
};
