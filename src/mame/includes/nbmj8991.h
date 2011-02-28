class nbmj8991_state : public driver_device
{
public:
	nbmj8991_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int scrollx;
	int scrolly;
	int blitter_destx;
	int blitter_desty;
	int blitter_sizex;
	int blitter_sizey;
	int blitter_src_addr;
	int blitter_direction_x;
	int blitter_direction_y;
	int gfxrom;
	int dispflag;
	int flipscreen;
	int clutsel;
	int screen_refresh;
	bitmap_t *tmpbitmap;
	UINT8 *videoram;
	UINT8 *clut;
	int flipscreen_old;
};


/*----------- defined in video/nbmj8991.c -----------*/

SCREEN_UPDATE( nbmj8991_type1 );
SCREEN_UPDATE( nbmj8991_type2 );
VIDEO_START( nbmj8991 );

WRITE8_HANDLER( nbmj8991_palette_type1_w );
WRITE8_HANDLER( nbmj8991_palette_type2_w );
WRITE8_HANDLER( nbmj8991_palette_type3_w );
WRITE8_HANDLER( nbmj8991_blitter_w );
READ8_HANDLER( nbmj8991_clut_r );
WRITE8_HANDLER( nbmj8991_clut_w );
