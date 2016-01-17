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
	toaplan1_state(const machine_config &mconfig, device_type type, std::string tag)
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

	required_shared_ptr<UINT16> m_bgpaletteram;
	required_shared_ptr<UINT16> m_fgpaletteram;

	optional_shared_ptr<UINT8> m_sharedram;

	int m_coin_count; /* coin count increments on startup ? , so don't count it */
	int m_intenable;

	/* Demon world */
	int m_dsp_on;
	int m_dsp_BIO;
	int m_dsp_execute;
	UINT32 m_dsp_addr_w;
	UINT32 m_main_ram_seg;

	UINT8 m_vimana_coins[2];
	UINT8 m_vimana_credits;
	UINT8 m_vimana_latch;

	std::unique_ptr<UINT16[]> m_pf4_tilevram16;   /*  ||  Drawn in this order */
	std::unique_ptr<UINT16[]> m_pf3_tilevram16;   /*  ||  */
	std::unique_ptr<UINT16[]> m_pf2_tilevram16;   /* \||/ */
	std::unique_ptr<UINT16[]> m_pf1_tilevram16;   /*  \/  */

	optional_shared_ptr<UINT16> m_spriteram;
	std::unique_ptr<UINT16[]> m_buffered_spriteram;
	std::unique_ptr<UINT16[]> m_spritesizeram16;
	std::unique_ptr<UINT16[]> m_buffered_spritesizeram16;

	INT32 m_bcu_flipscreen;     /* Tile   controller flip flag */
	INT32 m_fcu_flipscreen;     /* Sprite controller flip flag */

	INT32 m_pf_voffs;
	INT32 m_spriteram_offs;

	INT32 m_pf1_scrollx;
	INT32 m_pf1_scrolly;
	INT32 m_pf2_scrollx;
	INT32 m_pf2_scrolly;
	INT32 m_pf3_scrollx;
	INT32 m_pf3_scrolly;
	INT32 m_pf4_scrollx;
	INT32 m_pf4_scrolly;

#ifdef MAME_DEBUG
	int m_display_pf1;
	int m_display_pf2;
	int m_display_pf3;
	int m_display_pf4;
	int m_displog;
#endif

	INT32 m_tiles_offsetx;
	INT32 m_tiles_offsety;

	tilemap_t *m_pf1_tilemap;
	tilemap_t *m_pf2_tilemap;
	tilemap_t *m_pf3_tilemap;
	tilemap_t *m_pf4_tilemap;

	DECLARE_WRITE16_MEMBER(toaplan1_intenable_w);
	DECLARE_WRITE16_MEMBER(demonwld_dsp_addrsel_w);
	DECLARE_READ16_MEMBER(demonwld_dsp_r);
	DECLARE_WRITE16_MEMBER(demonwld_dsp_w);
	DECLARE_WRITE16_MEMBER(demonwld_dsp_bio_w);
	DECLARE_READ16_MEMBER(demonwld_BIO_r);
	DECLARE_WRITE16_MEMBER(demonwld_dsp_ctrl_w);
	DECLARE_READ16_MEMBER(samesame_port_6_word_r);
	DECLARE_READ16_MEMBER(vimana_system_port_r);
	DECLARE_READ16_MEMBER(vimana_mcu_r);
	DECLARE_WRITE16_MEMBER(vimana_mcu_w);
	DECLARE_READ16_MEMBER(toaplan1_shared_r);
	DECLARE_WRITE16_MEMBER(toaplan1_shared_w);
	DECLARE_WRITE16_MEMBER(toaplan1_reset_sound_w);
	DECLARE_WRITE8_MEMBER(toaplan1_coin_w);
	DECLARE_WRITE16_MEMBER(samesame_coin_w);

	DECLARE_READ16_MEMBER(toaplan1_frame_done_r);
	DECLARE_WRITE16_MEMBER(toaplan1_tile_offsets_w);
	DECLARE_WRITE16_MEMBER(toaplan1_bcu_flipscreen_w);
	DECLARE_WRITE16_MEMBER(toaplan1_fcu_flipscreen_w);
	DECLARE_READ16_MEMBER(toaplan1_spriteram_offs_r);
	DECLARE_WRITE16_MEMBER(toaplan1_spriteram_offs_w);
	DECLARE_WRITE16_MEMBER(toaplan1_bgpalette_w);
	DECLARE_WRITE16_MEMBER(toaplan1_fgpalette_w);
	DECLARE_READ16_MEMBER(toaplan1_spriteram16_r);
	DECLARE_WRITE16_MEMBER(toaplan1_spriteram16_w);
	DECLARE_READ16_MEMBER(toaplan1_spritesizeram16_r);
	DECLARE_WRITE16_MEMBER(toaplan1_spritesizeram16_w);
	DECLARE_WRITE16_MEMBER(toaplan1_bcu_control_w);
	DECLARE_READ16_MEMBER(toaplan1_tileram_offs_r);
	DECLARE_WRITE16_MEMBER(toaplan1_tileram_offs_w);
	DECLARE_READ16_MEMBER(toaplan1_tileram16_r);
	DECLARE_WRITE16_MEMBER(toaplan1_tileram16_w);
	DECLARE_READ16_MEMBER(toaplan1_scroll_regs_r);
	DECLARE_WRITE16_MEMBER(toaplan1_scroll_regs_w);

	DECLARE_DRIVER_INIT(toaplan1);
	DECLARE_DRIVER_INIT(demonwld);
	DECLARE_DRIVER_INIT(vimana);
	TILE_GET_INFO_MEMBER(get_pf1_tile_info);
	TILE_GET_INFO_MEMBER(get_pf2_tile_info);
	TILE_GET_INFO_MEMBER(get_pf3_tile_info);
	TILE_GET_INFO_MEMBER(get_pf4_tile_info);
	DECLARE_MACHINE_RESET(toaplan1);
	DECLARE_VIDEO_START(toaplan1);
	DECLARE_MACHINE_RESET(zerowing);
	DECLARE_MACHINE_RESET(demonwld);
	DECLARE_MACHINE_RESET(vimana);
	UINT32 screen_update_toaplan1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void screen_eof_toaplan1(screen_device &screen, bool state);
	void screen_eof_samesame(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(toaplan1_interrupt);

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
	DECLARE_WRITE_LINE_MEMBER(toaplan1_reset_callback);
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
	toaplan1_rallybik_state(const machine_config &mconfig, device_type type, std::string tag)
		: toaplan1_state(mconfig, type, tag),
		m_spritegen(*this, "scu")
	{
	}

	DECLARE_WRITE8_MEMBER(rallybik_coin_w);
	DECLARE_READ16_MEMBER(rallybik_tileram16_r);
	DECLARE_VIDEO_START(rallybik);
	UINT32 screen_update_rallybik(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_rallybik(screen_device &screen, bool state);

	required_device<toaplan_scu_device> m_spritegen;
};
