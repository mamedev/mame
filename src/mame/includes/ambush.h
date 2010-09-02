/*************************************************************************

    Ambush

*************************************************************************/

class ambush_state : public driver_device
{
public:
	ambush_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    spriteram;
	UINT8 *    colorram;
	UINT8 *    scrollram;
	UINT8 *    colorbank;

	size_t     videoram_size;
	size_t     spriteram_size;
};


/*----------- defined in video/ambush.c -----------*/

PALETTE_INIT( ambush );
VIDEO_UPDATE( ambush );
