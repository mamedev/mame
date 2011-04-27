class dooyong_state : public driver_device
{
public:
	dooyong_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

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

SCREEN_UPDATE( lastday );
SCREEN_UPDATE( gulfstrm );
SCREEN_UPDATE( pollux );
SCREEN_UPDATE( bluehawk );
SCREEN_UPDATE( flytiger );
SCREEN_UPDATE( primella );
SCREEN_UPDATE( rshark );
SCREEN_UPDATE( popbingo );

VIDEO_START( lastday );
VIDEO_START( gulfstrm );
VIDEO_START( pollux );
VIDEO_START( bluehawk );
VIDEO_START( flytiger );
VIDEO_START( primella );
VIDEO_START( rshark );
VIDEO_START( popbingo );

SCREEN_EOF( dooyong );
SCREEN_EOF( rshark );
