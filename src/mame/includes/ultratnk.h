/*************************************************************************

    Atari Ultra Tank hardware

*************************************************************************/


class ultratnk_state : public driver_device
{
public:
	ultratnk_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in video/ultratnk.c -----------*/

extern int ultratnk_collision[4];

PALETTE_INIT( ultratnk );
VIDEO_START( ultratnk );
VIDEO_UPDATE( ultratnk );
VIDEO_EOF( ultratnk );

WRITE8_HANDLER( ultratnk_video_ram_w );
