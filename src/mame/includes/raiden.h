#include "video/bufsprite.h"

class raiden_state : public driver_device
{
public:
	raiden_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") { }

	UINT16 *m_videoram;
	UINT16 *m_shared_ram;
	UINT16 *m_back_data;
	UINT16 *m_fore_data;
	UINT16 *m_scroll_ram;
	tilemap_t *m_bg_layer;
	tilemap_t *m_fg_layer;
	tilemap_t *m_tx_layer;
	int m_flipscreen;
	int m_alternate;
	required_device<buffered_spriteram16_device> m_spriteram;
	DECLARE_READ16_MEMBER(sub_cpu_spin_r);
};


/*----------- defined in video/raiden.c -----------*/

WRITE16_HANDLER( raiden_background_w );
WRITE16_HANDLER( raiden_foreground_w );
WRITE16_HANDLER( raiden_text_w );
VIDEO_START( raiden );
VIDEO_START( raidena );
WRITE16_HANDLER( raiden_control_w );
WRITE16_HANDLER( raidena_control_w );
SCREEN_UPDATE_IND16( raiden );
