#include "video/bufsprite.h"

class dynduke_state : public driver_device
{
public:
	dynduke_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") ,
		m_scroll_ram(*this, "scroll_ram"),
		m_videoram(*this, "videoram"),
		m_back_data(*this, "back_data"),
		m_fore_data(*this, "fore_data"){ }

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

	DECLARE_WRITE16_MEMBER(dynduke_paletteram_w);
	DECLARE_WRITE16_MEMBER(dynduke_background_w);
	DECLARE_WRITE16_MEMBER(dynduke_foreground_w);
	DECLARE_WRITE16_MEMBER(dynduke_text_w);
	DECLARE_WRITE16_MEMBER(dynduke_gfxbank_w);
	DECLARE_WRITE16_MEMBER(dynduke_control_w);
	DECLARE_DRIVER_INIT(dynduke);
};


/*----------- defined in video/dynduke.c -----------*/

VIDEO_START( dynduke );
SCREEN_UPDATE_IND16( dynduke );
