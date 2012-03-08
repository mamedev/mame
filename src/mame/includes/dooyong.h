#include "video/bufsprite.h"

class dooyong_state : public driver_device
{
public:
	dooyong_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram"),
		  m_spriteram16(*this, "spriteram16") { }

	UINT8 *m_txvideoram;
	UINT8 *m_paletteram_flytiger;
	UINT8 m_sprites_disabled;
	UINT8 m_flytiger_palette_bank;
	UINT8 m_flytiger_pri;
	UINT8 m_tx_pri;
	UINT16 m_rshark_pri;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_bg2_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_fg2_tilemap;
	tilemap_t *m_tx_tilemap;
	UINT8 m_bgscroll8[0x10];
	UINT8 m_bg2scroll8[0x10];
	UINT8 m_fgscroll8[0x10];
	UINT8 m_fg2scroll8[0x10];
	UINT8 *m_bg_tilerom;
	UINT8 *m_bg2_tilerom;
	UINT8 *m_fg_tilerom;
	UINT8 *m_fg2_tilerom;
	UINT8 *m_bg_tilerom2;
	UINT8 *m_bg2_tilerom2;
	UINT8 *m_fg_tilerom2;
	UINT8 *m_fg2_tilerom2;
	int m_bg_gfx;
	int m_bg2_gfx;
	int m_fg_gfx;
	int m_fg2_gfx;
	int m_tx_tilemap_mode;

	int m_interrupt_line_1;
	int m_interrupt_line_2;
	optional_device<buffered_spriteram8_device> m_spriteram;
	optional_device<buffered_spriteram16_device> m_spriteram16;
};


/*----------- defined in video/dooyong.c -----------*/

WRITE8_HANDLER( dooyong_bgscroll8_w );
WRITE8_HANDLER( dooyong_fgscroll8_w );
WRITE8_HANDLER( dooyong_fg2scroll8_w );

WRITE16_HANDLER( dooyong_bgscroll16_w );
WRITE16_HANDLER( dooyong_bg2scroll16_w );
WRITE16_HANDLER( dooyong_fgscroll16_w );
WRITE16_HANDLER( dooyong_fg2scroll16_w );

WRITE8_HANDLER( dooyong_txvideoram8_w );

WRITE8_HANDLER( lastday_ctrl_w );
WRITE8_HANDLER( pollux_ctrl_w );
WRITE8_HANDLER( primella_ctrl_w );
WRITE8_HANDLER( paletteram_flytiger_w );
WRITE8_HANDLER( flytiger_ctrl_w );
WRITE16_HANDLER( rshark_ctrl_w );

SCREEN_UPDATE_IND16( lastday );
SCREEN_UPDATE_IND16( gulfstrm );
SCREEN_UPDATE_IND16( pollux );
SCREEN_UPDATE_IND16( bluehawk );
SCREEN_UPDATE_IND16( flytiger );
SCREEN_UPDATE_IND16( primella );
SCREEN_UPDATE_IND16( rshark );
SCREEN_UPDATE_IND16( popbingo );

VIDEO_START( lastday );
VIDEO_START( gulfstrm );
VIDEO_START( pollux );
VIDEO_START( bluehawk );
VIDEO_START( flytiger );
VIDEO_START( primella );
VIDEO_START( rshark );
VIDEO_START( popbingo );
