#include "sound/okim6295.h"

class paradise_state : public driver_device
{
public:
	paradise_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_vram_2(*this, "vram_2"),
		m_videoram(*this, "videoram"),
		m_paletteram(*this, "paletteram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_oki2(*this, "oki2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_vram_0;
	required_shared_ptr<UINT8> m_vram_1;
	required_shared_ptr<UINT8> m_vram_2;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_paletteram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t *m_tilemap_0;
	tilemap_t *m_tilemap_1;
	tilemap_t *m_tilemap_2;
	bitmap_ind16 m_tmpbitmap;
	UINT8 m_palbank;
	UINT8 m_priority;
	UINT8 m_pixbank;
	int m_sprite_inc;

	int irq_count;
	DECLARE_WRITE8_MEMBER(paradise_rombank_w);
	DECLARE_WRITE8_MEMBER(torus_coin_counter_w);
	DECLARE_WRITE8_MEMBER(paradise_flipscreen_w);
	DECLARE_WRITE8_MEMBER(tgtball_flipscreen_w);
	DECLARE_WRITE8_MEMBER(paradise_palette_w);
	DECLARE_WRITE8_MEMBER(paradise_vram_0_w);
	DECLARE_WRITE8_MEMBER(paradise_palbank_w);
	DECLARE_WRITE8_MEMBER(paradise_vram_1_w);
	DECLARE_WRITE8_MEMBER(paradise_vram_2_w);
	DECLARE_WRITE8_MEMBER(paradise_pixmap_w);
	DECLARE_WRITE8_MEMBER(paradise_priority_w);
	DECLARE_WRITE8_MEMBER(paradise_okibank_w);
	DECLARE_DRIVER_INIT(torus);
	DECLARE_DRIVER_INIT(paradise);
	DECLARE_DRIVER_INIT(tgtball);
	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
	TILE_GET_INFO_MEMBER(get_tile_info_2);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_paradise(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_torus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_madball(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(paradise_irq);
	void update_pix_palbank();
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	optional_device<okim6295_device> m_oki2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
