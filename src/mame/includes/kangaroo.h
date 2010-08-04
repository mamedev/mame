/***************************************************************************

    Sun Electronics Kangaroo hardware

    driver by Ville Laitinen

***************************************************************************/

class kangaroo_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, kangaroo_state(machine)); }

	kangaroo_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *      video_control;

	/* video-related */
	UINT32       *videoram;

	/* misc */
	UINT8        clock;
};




/*----------- defined in video/kangaroo.c -----------*/

VIDEO_START( kangaroo );
VIDEO_UPDATE( kangaroo );

WRITE8_HANDLER( kangaroo_videoram_w );
WRITE8_HANDLER( kangaroo_video_control_w );
