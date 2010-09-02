/*************************************************************************

    Megazone

*************************************************************************/

class megazone_state : public driver_device
{
public:
	megazone_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *       scrollx;
	UINT8 *       scrolly;
	UINT8 *       videoram;
	UINT8 *       colorram;
	UINT8 *       videoram2;
	UINT8 *       colorram2;
	UINT8 *       spriteram;
	size_t        spriteram_size;
	size_t        videoram_size;
	size_t        videoram2_size;

	/* video-related */
	bitmap_t      *tmpbitmap;
	int           flipscreen;

	/* misc */
	int           i8039_status;

	/* devices */
	cpu_device *maincpu;
	cpu_device *audiocpu;
	cpu_device *daccpu;
};



/*----------- defined in video/megazone.c -----------*/

WRITE8_HANDLER( megazone_flipscreen_w );

PALETTE_INIT( megazone );
VIDEO_START( megazone );
VIDEO_UPDATE( megazone );
