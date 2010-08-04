/*************************************************************************

    Super Slams

*************************************************************************/

class suprslam_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, suprslam_state(machine)); }

	suprslam_state(running_machine &machine)
		: driver_data_t(machine) { }

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
