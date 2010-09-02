

class fgoal_state : public driver_device
{
public:
	fgoal_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    video_ram;

	/* video-related */
	bitmap_t   *bgbitmap, *fgbitmap;
	UINT8      xpos, ypos;
	int        current_color;

	/* misc */
	int        fgoal_player;
	UINT8      row, col;
	int        prev_coin;

	/* devices */
	running_device *maincpu;
	running_device *mb14241;
};


/*----------- defined in video/fgoal.c -----------*/

VIDEO_START( fgoal );
VIDEO_UPDATE( fgoal );

WRITE8_HANDLER( fgoal_color_w );
WRITE8_HANDLER( fgoal_xpos_w );
WRITE8_HANDLER( fgoal_ypos_w );

