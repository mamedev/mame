
typedef struct _crshrace_state crshrace_state;
struct _crshrace_state
{
	/* memory pointers */
	UINT16 *  videoram1;
	UINT16 *  videoram2;
//  UINT16 *  spriteram1;   // currently this uses generic buffered spriteram
//  UINT16 *  spriteram2;   // currently this uses generic buffered spriteram
//      UINT16 *  paletteram;   // currently this uses generic palette handling

	/* video-related */
	tilemap_t   *tilemap1, *tilemap2;
	int       roz_bank, gfxctrl, flipscreen;

	/* misc */
	int pending_command;

	/* devices */
	const device_config *audiocpu;
	const device_config *k053936;
};

/*----------- defined in video/crshrace.c -----------*/

WRITE16_HANDLER( crshrace_videoram1_w );
WRITE16_HANDLER( crshrace_videoram2_w );
WRITE16_HANDLER( crshrace_roz_bank_w );
WRITE16_HANDLER( crshrace_gfxctrl_w );

VIDEO_START( crshrace );
VIDEO_EOF( crshrace );
VIDEO_UPDATE( crshrace );
