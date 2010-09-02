/* This it the best way to allow game specific kludges until the system is fully understood */
enum {
	ARKUNK = 0,  /* unknown bootlegs for inclusion of possible new sets */
	ARKANGC,
	ARKANGC2,
	BLOCK2,
	ARKBLOCK,
	ARKBLOC2,
	ARKGCBL,
	PADDLE2
};

class arkanoid_state : public driver_device
{
public:
	arkanoid_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  spriteram;
	size_t   spriteram_size;
	size_t   videoram_size;

	/* video-related */
	tilemap_t  *bg_tilemap;
	UINT8    gfxbank, palettebank;

	/* input-related */
	UINT8    paddle_select;

	/* misc */
	int      bootleg_id;
	UINT8    z80write, fromz80, m68705write, toz80;
	UINT8    port_a_in, port_a_out, ddr_a;
	UINT8    port_c_out, ddr_c;
	UINT8    bootleg_cmd;

	/* devices */
	running_device *mcu;
};



/*----------- defined in video/arkanoid.c -----------*/

extern WRITE8_HANDLER( arkanoid_videoram_w );

extern WRITE8_HANDLER( arkanoid_d008_w );
extern WRITE8_HANDLER( tetrsark_d008_w );
extern WRITE8_HANDLER( hexa_d008_w );

extern VIDEO_START( arkanoid );
extern VIDEO_UPDATE( arkanoid );
extern VIDEO_UPDATE( hexa );


/*----------- defined in machine/arkanoid.c -----------*/

extern READ8_HANDLER( arkanoid_Z80_mcu_r );
extern WRITE8_HANDLER( arkanoid_Z80_mcu_w );

extern READ8_HANDLER( arkanoid_68705_port_a_r );
extern WRITE8_HANDLER( arkanoid_68705_port_a_w );
extern WRITE8_HANDLER( arkanoid_68705_ddr_a_w );

extern READ8_HANDLER( arkanoid_68705_port_c_r );
extern WRITE8_HANDLER( arkanoid_68705_port_c_w );
extern WRITE8_HANDLER( arkanoid_68705_ddr_c_w );

extern CUSTOM_INPUT( arkanoid_68705_input_r );
extern CUSTOM_INPUT( arkanoid_input_mux );

extern READ8_HANDLER( arkanoid_bootleg_f000_r );
extern READ8_HANDLER( arkanoid_bootleg_f002_r );
extern WRITE8_HANDLER( arkanoid_bootleg_d018_w );
extern READ8_HANDLER( arkanoid_bootleg_d008_r );
