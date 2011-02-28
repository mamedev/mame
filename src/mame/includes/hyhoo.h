class hyhoo_state : public driver_device
{
public:
	hyhoo_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *clut;
	int blitter_destx;
	int blitter_desty;
	int blitter_sizex;
	int blitter_sizey;
	int blitter_src_addr;
	int blitter_direction_x;
	int blitter_direction_y;
	int gfxrom;
	int dispflag;
	int highcolorflag;
	int flipscreen;
	bitmap_t *tmpbitmap;
};


/*----------- defined in video/hyhoo.c -----------*/

SCREEN_UPDATE( hyhoo );
VIDEO_START( hyhoo );

WRITE8_HANDLER( hyhoo_blitter_w );
WRITE8_HANDLER( hyhoo_romsel_w );
