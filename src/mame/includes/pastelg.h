class pastelg_state : public driver_device
{
public:
	pastelg_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 mux_data;
	int blitter_destx;
	int blitter_desty;
	int blitter_sizex;
	int blitter_sizey;
	int blitter_src_addr;
	int gfxrom;
	int dispflag;
	int flipscreen;
	int blitter_direction_x;
	int blitter_direction_y;
	int palbank;
	UINT8 *videoram;
	UINT8 *clut;
	int flipscreen_old;
};


/*----------- defined in video/pastelg.c -----------*/

PALETTE_INIT( pastelg );
SCREEN_UPDATE( pastelg );
VIDEO_START( pastelg );

WRITE8_HANDLER( pastelg_clut_w );
WRITE8_HANDLER( pastelg_romsel_w );
WRITE8_HANDLER( threeds_romsel_w );
WRITE8_HANDLER( threeds_output_w );
WRITE8_HANDLER( pastelg_blitter_w );
READ8_HANDLER( threeds_rom_readback_r );

int pastelg_blitter_src_addr_r(address_space *space);
