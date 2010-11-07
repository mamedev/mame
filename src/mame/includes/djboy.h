/*************************************************************************

    DJ Boy

*************************************************************************/

#define PROT_OUTPUT_BUFFER_SIZE 8

class djboy_state : public driver_device
{
public:
	djboy_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8		*videoram;
	UINT8		*paletteram;

	/* ROM banking */
	UINT8		bankxor;

	/* video-related */
	tilemap_t	*background;
	UINT8		videoreg, scrollx, scrolly;

	/* Kaneko BEAST state */
	UINT8		data_to_beast;
	UINT8		data_to_z80;
	UINT8		beast_to_z80_full;
	UINT8		z80_to_beast_full;
	UINT8		beast_int0_l;
	UINT8		beast_p0;
	UINT8		beast_p1;
	UINT8		beast_p2;
	UINT8		beast_p3;

	/* devices */
	running_device *maincpu;
	running_device *cpu1;
	running_device *cpu2;
	running_device *pandora;
	running_device *beast;
};


/*----------- defined in video/djboy.c -----------*/

WRITE8_HANDLER( djboy_scrollx_w );
WRITE8_HANDLER( djboy_scrolly_w );
WRITE8_HANDLER( djboy_videoram_w );
WRITE8_HANDLER( djboy_paletteram_w );

VIDEO_START( djboy );
VIDEO_UPDATE( djboy );
VIDEO_EOF( djboy );
