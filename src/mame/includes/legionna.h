#include "sound/okim6295.h"

class legionna_state : public driver_device
{
public:
	legionna_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_spriteram(*this, "spriteram") ,
		m_back_data(*this, "back_data"),
		m_fore_data(*this, "fore_data"),
		m_mid_data(*this, "mid_data"),
		m_textram(*this, "textram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_back_data;
	required_shared_ptr<UINT16> m_fore_data;
	required_shared_ptr<UINT16> m_mid_data;
	required_shared_ptr<UINT16> m_textram;
	UINT16 *m_scrollram16;
	UINT16 m_layer_disable;
	int m_sprite_xoffs;
	int m_sprite_yoffs;
	tilemap_t *m_background_layer;
	tilemap_t *m_foreground_layer;
	tilemap_t *m_midground_layer;
	tilemap_t *m_text_layer;
	int m_has_extended_banking;
	int m_has_extended_priority;
	UINT16 m_back_gfx_bank;
	UINT16 m_fore_gfx_bank;
	UINT16 m_mid_gfx_bank;

	DECLARE_WRITE16_MEMBER(denjin_paletteram16_xBBBBBGGGGGRRRRR_word_w);
	DECLARE_WRITE16_MEMBER(legionna_background_w);
	DECLARE_WRITE16_MEMBER(legionna_midground_w);
	DECLARE_WRITE16_MEMBER(legionna_foreground_w);
	DECLARE_WRITE16_MEMBER(legionna_text_w);
	DECLARE_WRITE8_MEMBER(okim_rombank_w);
	DECLARE_DRIVER_INIT(legiongfx);
	DECLARE_DRIVER_INIT(cupsoc);
	DECLARE_DRIVER_INIT(denjinmk);
	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_mid_tile_info);
	TILE_GET_INFO_MEMBER(get_mid_tile_info_denji);
	TILE_GET_INFO_MEMBER(get_mid_tile_info_cupsoc);
	TILE_GET_INFO_MEMBER(get_fore_tile_info);
	TILE_GET_INFO_MEMBER(get_fore_tile_info_denji);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
	DECLARE_VIDEO_START(legionna);
	DECLARE_VIDEO_START(godzilla);
	DECLARE_VIDEO_START(denjinmk);
	DECLARE_VIDEO_START(grainbow);
	DECLARE_VIDEO_START(cupsoc);
	UINT32 screen_update_legionna(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_godzilla(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_grainbow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
	void descramble_legionnaire_gfx(UINT8* src);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

/*----------- defined in video/legionna.c -----------*/
void heatbrl_setgfxbank(running_machine &machine, UINT16 data);
void denjinmk_setgfxbank(running_machine &machine, UINT16 data);
