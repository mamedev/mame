#include "machine/nmk112.h"

class powerins_state : public driver_device
{
public:
	powerins_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_nmk112(*this, "nmk112"),
		m_vctrl_0(*this, "vctrl_0"),
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
		{ }


	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_soundcpu;
	optional_device<nmk112_device> m_nmk112;
	required_shared_ptr<UINT16> m_vctrl_0;
	required_shared_ptr<UINT16> m_vram_0;
	required_shared_ptr<UINT16> m_vram_1;
	required_shared_ptr<UINT16> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	UINT16 *m_vctrl_1;
	tilemap_t *m_tilemap_0;
	tilemap_t *m_tilemap_1;
	int m_oki_bank;
	int m_tile_bank;
	DECLARE_WRITE16_MEMBER(powerins_okibank_w);
	DECLARE_WRITE16_MEMBER(powerins_soundlatch_w);
	DECLARE_READ8_MEMBER(powerinb_fake_ym2203_r);
	DECLARE_WRITE16_MEMBER(powerins_flipscreen_w);
	DECLARE_WRITE16_MEMBER(powerins_tilebank_w);
	DECLARE_WRITE16_MEMBER(powerins_vram_0_w);
	DECLARE_WRITE16_MEMBER(powerins_vram_1_w);
	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILEMAP_MAPPER_MEMBER(powerins_get_memory_offset_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_powerins(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
};
