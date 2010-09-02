/***************************************************************************

    Sun Electronics Kangaroo hardware

    driver by Ville Laitinen

***************************************************************************/

class kangaroo_state : public driver_device
{
public:
	kangaroo_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

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
