/*************************************************************************

    Super Slams

*************************************************************************/

class suprslam_state : public driver_device
{
public:
	suprslam_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    screen_videoram;
	UINT16 *    bg_videoram;
	UINT16 *    sp_videoram;
	UINT16 *    spriteram;
//  UINT16 *    paletteram; // this currently uses generic palette handling

	/* video-related */
	tilemap_t     *screen_tilemap, *bg_tilemap;
	UINT16      screen_bank, bg_bank;
	UINT16      *spr_ctrl;
	UINT16      *screen_vregs;

	/* misc */
	int         pending_command;

	/* devices */
	running_device *audiocpu;
	running_device *k053936;
};


/*----------- defined in video/suprslam.c -----------*/

WRITE16_HANDLER( suprslam_screen_videoram_w );
WRITE16_HANDLER( suprslam_bg_videoram_w );
WRITE16_HANDLER( suprslam_bank_w );

VIDEO_START( suprslam );
VIDEO_UPDATE( suprslam );
