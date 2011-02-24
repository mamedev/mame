class dooyong_state : public driver_device
{
public:
	dooyong_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *txvideoram;
	UINT8 *paletteram_flytiger;
	UINT8 sprites_disabled;
	UINT8 flytiger_palette_bank;
	UINT8 flytiger_pri;
	UINT8 tx_pri;
	UINT16 rshark_pri;
	tilemap_t *bg_tilemap;
	tilemap_t *bg2_tilemap;
	tilemap_t *fg_tilemap;
	tilemap_t *fg2_tilemap;
	tilemap_t *tx_tilemap;
	UINT8 bgscroll8[0x10];
	UINT8 bg2scroll8[0x10];
	UINT8 fgscroll8[0x10];
	UINT8 fg2scroll8[0x10];
	UINT8 *bg_tilerom;
	UINT8 *bg2_tilerom;
	UINT8 *fg_tilerom;
	UINT8 *fg2_tilerom;
	UINT8 *bg_tilerom2;
	UINT8 *bg2_tilerom2;
	UINT8 *fg_tilerom2;
	UINT8 *fg2_tilerom2;
	int bg_gfx;
	int bg2_gfx;
	int fg_gfx;
	int fg2_gfx;
	int tx_tilemap_mode;
	
	int interrupt_line_1, interrupt_line_2;
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
