#include "video/bufsprite.h"

class toki_state : public driver_device
{
public:
	toki_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") { }

	UINT16 *m_videoram;
	int m_msm5205next;
	int m_toggle;
	UINT16 *m_background1_videoram16;
	UINT16 *m_background2_videoram16;
	UINT16 *m_scrollram16;
	tilemap_t *m_background_layer;
	tilemap_t *m_foreground_layer;
	tilemap_t *m_text_layer;
	required_device<buffered_spriteram16_device> m_spriteram;
	DECLARE_WRITE16_MEMBER(tokib_soundcommand16_w);
	DECLARE_READ16_MEMBER(pip16_r);
	DECLARE_WRITE8_MEMBER(toki_adpcm_data_w);
	DECLARE_WRITE16_MEMBER(toki_control_w);
	DECLARE_WRITE16_MEMBER(toki_foreground_videoram16_w);
	DECLARE_WRITE16_MEMBER(toki_background1_videoram16_w);
	DECLARE_WRITE16_MEMBER(toki_background2_videoram16_w);
};


/*----------- defined in video/toki.c -----------*/

VIDEO_START( toki );
SCREEN_UPDATE_IND16( toki );
SCREEN_UPDATE_IND16( tokib );
