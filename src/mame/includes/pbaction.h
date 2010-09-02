/*************************************************************************

    Pinball Action

*************************************************************************/

class pbaction_state : public driver_device
{
public:
	pbaction_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    videoram2;
	UINT8 *    colorram;
	UINT8 *    colorram2;
	UINT8 *    work_ram;
	UINT8 *    spriteram;
//  UINT8 *    paletteram;    // currently this uses generic palette handling
	size_t     spriteram_size;

	/* video-related */
	tilemap_t  *bg_tilemap, *fg_tilemap;
	int        scroll;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};


/*----------- defined in video/pbaction.c -----------*/

extern WRITE8_HANDLER( pbaction_videoram_w );
extern WRITE8_HANDLER( pbaction_colorram_w );
extern WRITE8_HANDLER( pbaction_videoram2_w );
extern WRITE8_HANDLER( pbaction_colorram2_w );
extern WRITE8_HANDLER( pbaction_flipscreen_w );
extern WRITE8_HANDLER( pbaction_scroll_w );

extern VIDEO_START( pbaction );
extern VIDEO_UPDATE( pbaction );
