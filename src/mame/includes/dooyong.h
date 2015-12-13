// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "video/bufsprite.h"

class dooyong_state : public driver_device
{
public:
	dooyong_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_palette_bank(0)
	{ }

	DECLARE_WRITE8_MEMBER(bgscroll_w);
	DECLARE_WRITE8_MEMBER(bg2scroll_w);
	DECLARE_WRITE8_MEMBER(fgscroll_w);
	DECLARE_WRITE8_MEMBER(fg2scroll_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg2_tile_info);
	inline void get_tile_info(tile_data &tileinfo, int tile_index, UINT8 const *tilerom, UINT8 const *scroll, int graphics);
	inline void scroll8_w(offs_t offset, UINT8 data, UINT8 *scroll, tilemap_t *map);


	tilemap_t *m_bg_tilemap;
	tilemap_t *m_bg2_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_fg2_tilemap;
	tilemap_t *m_tx_tilemap;
	UINT8 m_bgscroll8[0x10];
	UINT8 m_bg2scroll8[0x10];
	UINT8 m_fgscroll8[0x10];
	UINT8 m_fg2scroll8[0x10];
	UINT8 *m_bg_tilerom;
	UINT8 *m_bg2_tilerom;
	UINT8 *m_fg_tilerom;
	UINT8 *m_fg2_tilerom;
	int m_bg_gfx;
	int m_bg2_gfx;
	int m_fg_gfx;
	int m_fg2_gfx;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	UINT8 m_palette_bank;
};

class dooyong_z80_state : public dooyong_state
{
public:
	dooyong_z80_state(const machine_config &mconfig, device_type type, const char *tag)
		: dooyong_state(mconfig, type, tag),
		m_txvideoram(*this, "txvideoram"),
		m_spriteram(*this, "spriteram")
	{ }

	enum
	{
		SPRITE_12BIT = 0x01,
		SPRITE_HEIGHT = 0x02,
		SPRITE_YSHIFT_BLUEHAWK = 0x04,
		SPRITE_YSHIFT_FLYTIGER = 0x08
	};

	DECLARE_WRITE8_MEMBER(flip_screen_w);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_WRITE8_MEMBER(txvideoram_w);
	DECLARE_WRITE8_MEMBER(primella_ctrl_w);
	DECLARE_READ8_MEMBER(paletteram_flytiger_r);
	DECLARE_WRITE8_MEMBER(paletteram_flytiger_w);
	DECLARE_WRITE8_MEMBER(flytiger_ctrl_w);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, unsigned extensions = 0);
	UINT32 screen_update_bluehawk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_flytiger(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_primella(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_MACHINE_START(cpu_z80);
	DECLARE_VIDEO_START(bluehawk);
	DECLARE_VIDEO_START(flytiger);
	DECLARE_VIDEO_START(primella);

	required_shared_ptr<UINT8> m_txvideoram;
	UINT8* m_paletteram_flytiger;
	UINT8 m_sprites_disabled;
	UINT8 m_flytiger_pri;
	UINT8 m_tx_pri;
	int m_tx_tilemap_mode;

	optional_device<buffered_spriteram8_device> m_spriteram;
};

class dooyong_z80_ym2203_state : public dooyong_z80_state
{
public:
	dooyong_z80_ym2203_state(const machine_config &mconfig, device_type type, const char *tag)
		: dooyong_z80_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(lastday_ctrl_w);
	DECLARE_WRITE8_MEMBER(pollux_ctrl_w);
	DECLARE_WRITE_LINE_MEMBER(irqhandler_2203_1);
	DECLARE_WRITE_LINE_MEMBER(irqhandler_2203_2);
	DECLARE_READ8_MEMBER(unk_r);
	DECLARE_MACHINE_RESET(sound_ym2203);
	UINT32 screen_update_lastday(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_gulfstrm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_pollux(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_VIDEO_START(lastday);
	DECLARE_VIDEO_START(gulfstrm);
	DECLARE_VIDEO_START(pollux);

	int m_interrupt_line_1;
	int m_interrupt_line_2;
};

class dooyong_68k_state : public dooyong_state
{
public:
	dooyong_68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: dooyong_state(mconfig, type, tag),
		m_spriteram(*this, "spriteram")
	{ }

	DECLARE_WRITE16_MEMBER(ctrl_w);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	TILE_GET_INFO_MEMBER(rshark_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(rshark_get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(rshark_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(rshark_get_fg2_tile_info);
	inline void rshark_get_tile_info(tile_data &tileinfo, int tile_index, UINT8 const *tilerom1, UINT8 const *tilerom2, UINT8 const *scroll, int graphics);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_rshark(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_popbingo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_VIDEO_START(rshark);
	DECLARE_VIDEO_START(popbingo);

	UINT8 *m_bg_tilerom2;
	UINT8 *m_bg2_tilerom2;
	UINT8 *m_fg_tilerom2;
	UINT8 *m_fg2_tilerom2;
	UINT16 m_bg2_priority;

	required_device<buffered_spriteram16_device> m_spriteram;
};
