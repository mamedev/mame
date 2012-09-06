#include "video/bufsprite.h"

class toki_state : public driver_device
{
public:
	toki_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") ,
		m_background1_videoram16(*this, "bg1_vram16"),
		m_background2_videoram16(*this, "bg2_vram16"),
		m_videoram(*this, "videoram"),
		m_scrollram16(*this, "scrollram16"){ }

	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<UINT16> m_background1_videoram16;
	required_shared_ptr<UINT16> m_background2_videoram16;
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_scrollram16;

	int m_msm5205next;
	int m_toggle;
	tilemap_t *m_background_layer;
	tilemap_t *m_foreground_layer;
	tilemap_t *m_text_layer;

	DECLARE_WRITE16_MEMBER(tokib_soundcommand16_w);
	DECLARE_READ16_MEMBER(pip16_r);
	DECLARE_WRITE8_MEMBER(toki_adpcm_data_w);
	DECLARE_WRITE16_MEMBER(toki_control_w);
	DECLARE_WRITE16_MEMBER(toki_foreground_videoram16_w);
	DECLARE_WRITE16_MEMBER(toki_background1_videoram16_w);
	DECLARE_WRITE16_MEMBER(toki_background2_videoram16_w);
	DECLARE_WRITE8_MEMBER(toki_adpcm_control_w);
	DECLARE_DRIVER_INIT(tokib);
	DECLARE_DRIVER_INIT(jujuba);
	DECLARE_DRIVER_INIT(toki);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_fore_tile_info);
};


/*----------- defined in video/toki.c -----------*/

VIDEO_START( toki );
SCREEN_UPDATE_IND16( toki );
SCREEN_UPDATE_IND16( tokib );
