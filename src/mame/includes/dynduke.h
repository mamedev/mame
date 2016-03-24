// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
#include "audio/seibu.h"
#include "video/bufsprite.h"

class dynduke_state : public driver_device
{
public:
	dynduke_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_seibu_sound(*this, "seibu_sound"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram") ,
		m_scroll_ram(*this, "scroll_ram"),
		m_videoram(*this, "videoram"),
		m_back_data(*this, "back_data"),
		m_fore_data(*this, "fore_data") { }

	required_device<cpu_device> m_maincpu;
	required_device<seibu_sound_device> m_seibu_sound;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram16_device> m_spriteram;

	required_shared_ptr<UINT16> m_scroll_ram;
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_back_data;
	required_shared_ptr<UINT16> m_fore_data;

	tilemap_t *m_bg_layer;
	tilemap_t *m_fg_layer;
	tilemap_t *m_tx_layer;
	int m_back_bankbase;
	int m_fore_bankbase;
	int m_back_enable;
	int m_fore_enable;
	int m_sprite_enable;
	int m_txt_enable;
	int m_old_back;
	int m_old_fore;

	DECLARE_WRITE16_MEMBER(background_w);
	DECLARE_WRITE16_MEMBER(foreground_w);
	DECLARE_WRITE16_MEMBER(text_w);
	DECLARE_WRITE16_MEMBER(gfxbank_w);
	DECLARE_WRITE16_MEMBER(control_w);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);

	virtual void video_start() override;
	DECLARE_DRIVER_INIT(dynduke);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect,int pri);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri );

	INTERRUPT_GEN_MEMBER(interrupt);
};
