/*************************************************************************


    Taito Z system

*************************************************************************/

class taitoz_state : public driver_device
{
public:
	taitoz_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    spriteram;
//  UINT16 *    paletteram;    // currently this uses generic palette handling
	offs_t      spriteram_size;

	/* video-related */
	int         sci_spriteframe;
	int         road_palbank;

	/* misc */
	int         chasehq_lamps;
	INT32       banknum;
	UINT16      cpua_ctrl;
	INT32       sci_int6;
	INT32       dblaxle_int6;
	INT32       ioc220_port;
	UINT16      eep_latch;

//  UINT8       pandata[4];

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *subcpu;
	device_t *eeprom;
	device_t *tc0480scp;
	device_t *tc0150rod;
	device_t *tc0100scn;
	device_t *tc0220ioc;
	device_t *tc0140syt;

	/* dblaxle motor flag */
	int	    dblaxle_vibration;
};

/*----------- defined in video/taito_z.c -----------*/

WRITE16_HANDLER( contcirc_out_w );
READ16_HANDLER ( sci_spriteframe_r );
WRITE16_HANDLER( sci_spriteframe_w );

VIDEO_START( taitoz );

VIDEO_UPDATE( contcirc );
VIDEO_UPDATE( chasehq );
VIDEO_UPDATE( bshark );
VIDEO_UPDATE( sci );
VIDEO_UPDATE( aquajack );
VIDEO_UPDATE( spacegun );
VIDEO_UPDATE( dblaxle );
