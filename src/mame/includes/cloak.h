/*************************************************************************

    Atari Cloak & Dagger hardware

*************************************************************************/

class cloak_state : public driver_device
{
public:
	cloak_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in video/cloak.c -----------*/

WRITE8_HANDLER( cloak_videoram_w );
WRITE8_HANDLER( cloak_flipscreen_w );

WRITE8_HANDLER( cloak_paletteram_w );
READ8_HANDLER( graph_processor_r );
WRITE8_HANDLER( graph_processor_w );
WRITE8_HANDLER( cloak_clearbmp_w );

VIDEO_START( cloak );
VIDEO_UPDATE( cloak );
