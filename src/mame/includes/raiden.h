#include "video/bufsprite.h"

class raiden_state : public driver_device
{
public:
	raiden_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") ,
		m_shared_ram(*this, "shared_ram"),
		m_videoram(*this, "videoram"),
		m_scroll_ram(*this, "scroll_ram"),
		m_back_data(*this, "back_data"),
		m_fore_data(*this, "fore_data"){ }

	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<UINT16> m_shared_ram;
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_scroll_ram;
	required_shared_ptr<UINT16> m_back_data;
	required_shared_ptr<UINT16> m_fore_data;
	tilemap_t *m_bg_layer;
	tilemap_t *m_fg_layer;
	tilemap_t *m_tx_layer;
	int m_flipscreen;
	int m_alternate;

	DECLARE_READ16_MEMBER(sub_cpu_spin_r);
	DECLARE_WRITE16_MEMBER(raiden_background_w);
	DECLARE_WRITE16_MEMBER(raiden_foreground_w);
	DECLARE_WRITE16_MEMBER(raiden_text_w);
	DECLARE_WRITE16_MEMBER(raiden_control_w);
	DECLARE_WRITE16_MEMBER(raidena_control_w);
	DECLARE_DRIVER_INIT(raidenu);
	DECLARE_DRIVER_INIT(raidenk);
	DECLARE_DRIVER_INIT(raiden);
	DECLARE_DRIVER_INIT(raidena);
	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_fore_tile_info);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
};


/*----------- defined in video/raiden.c -----------*/

VIDEO_START( raiden );
VIDEO_START( raidena );
SCREEN_UPDATE_IND16( raiden );
