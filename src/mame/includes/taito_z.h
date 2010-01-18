/*************************************************************************

    Taito Z system

*************************************************************************/

typedef struct _taitoz_state taitoz_state;
struct _taitoz_state
{
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
	running_device *maincpu;
	running_device *audiocpu;
	running_device *subcpu;
	running_device *eeprom;
	running_device *tc0480scp;
	running_device *tc0150rod;
	running_device *tc0100scn;
	running_device *tc0220ioc;
	running_device *tc0140syt;
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
