/*************************************************************************

    Taito H system

*************************************************************************/

typedef struct _taitoh_state taitoh_state;
struct _taitoh_state
{
	/* memory pointers */
	UINT16 *    m68000_mainram;
//  UINT16 *    paletteram;    // currently this uses generic palette handling

	/* misc */
	INT32       banknum;

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *tc0080vco;
	const device_config *tc0220ioc;
};

/*----------- defined in video/taito_h.c -----------*/

VIDEO_UPDATE( syvalion );
VIDEO_UPDATE( recordbr );
VIDEO_UPDATE( dleague );
