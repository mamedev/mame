// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski
#include "machine/bankdev.h"
#include "video/jalblend.h"

class psychic5_state : public driver_device
{
public:
	psychic5_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_vrambank(*this, "vrambank"),
		m_blend(*this, "blend"),
		m_spriteram(*this, "spriteram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_bg_control(*this, "bg_control"),
		m_ps5_palette_ram_bg(*this, "palette_ram_bg"),
		m_ps5_palette_ram_sp(*this, "palette_ram_sp"),
		m_ps5_palette_ram_tx(*this, "palette_ram_tx")

	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<address_map_bank_device> m_vrambank;
	optional_device<jaleco_blend_device> m_blend;

	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_fg_videoram;
	required_shared_ptr<UINT8> m_bg_videoram;
	required_shared_ptr<UINT8> m_bg_control;
	required_shared_ptr<UINT8> m_ps5_palette_ram_bg;
	required_shared_ptr<UINT8> m_ps5_palette_ram_sp;
	required_shared_ptr<UINT8> m_ps5_palette_ram_tx;

	UINT8 m_bank_latch;
	UINT8 m_ps5_vram_page;
	UINT8 m_bg_clip_mode;
	UINT8 m_title_screen;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	UINT16 m_palette_intensity;
	UINT8 m_bombsa_unknown;
	int m_sx1;
	int m_sy1;
	int m_sy2;

	DECLARE_READ8_MEMBER(bankselect_r);
	DECLARE_READ8_MEMBER(vram_page_select_r);
	DECLARE_WRITE8_MEMBER(vram_page_select_w);
	DECLARE_WRITE8_MEMBER(fg_videoram_w);
	DECLARE_WRITE8_MEMBER(bg_videoram_w);
	DECLARE_WRITE8_MEMBER(sprite_col_w);
	DECLARE_WRITE8_MEMBER(bg_col_w);
	DECLARE_WRITE8_MEMBER(tx_col_w);

	/* psychic5 specific */
	DECLARE_WRITE8_MEMBER(psychic5_coin_counter_w);
	DECLARE_WRITE8_MEMBER(psychic5_bankselect_w);
	DECLARE_WRITE8_MEMBER(psychic5_title_screen_w);

	/* bombsa specific */
	DECLARE_WRITE8_MEMBER(bombsa_bankselect_w);
	DECLARE_WRITE8_MEMBER(bombsa_flipscreen_w);
	DECLARE_WRITE8_MEMBER(bombsa_unknown_w);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	virtual void machine_reset() override;
	DECLARE_MACHINE_START(psychic5);
	DECLARE_MACHINE_START(bombsa);
	virtual void video_start() override;
	DECLARE_VIDEO_START(psychic5);
	DECLARE_VIDEO_START(bombsa);
	DECLARE_VIDEO_RESET(psychic5);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	UINT32 screen_update_psychic5(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_bombsa(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void change_palette(int offset, UINT8* palram, int palbase);
	void change_bg_palette(int color, int lo_offs, int hi_offs);
	void set_background_palette_intensity();
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_background(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect); //only used by psychic5
};
