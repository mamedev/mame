/*************************************************************************

    Atari Ultra Tank hardware

*************************************************************************/


class ultratnk_state : public driver_device
{
public:
	ultratnk_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	int da_latch;
	int collision[4];
	tilemap_t* playfield;
	bitmap_t* helper;
};


/*----------- defined in video/ultratnk.c -----------*/

PALETTE_INIT( ultratnk );
VIDEO_START( ultratnk );
VIDEO_UPDATE( ultratnk );
VIDEO_EOF( ultratnk );

WRITE8_HANDLER( ultratnk_video_ram_w );
