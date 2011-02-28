class nbmj8900_state : public driver_device
{
public:
	nbmj8900_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int scrolly;
	int blitter_destx;
	int blitter_desty;
	int blitter_sizex;
	int blitter_sizey;
	int blitter_src_addr;
	int blitter_direction_x;
	int blitter_direction_y;
	int vram;
	int gfxrom;
	int dispflag;
	int flipscreen;
	int clutsel;
	int screen_refresh;
	int gfxdraw_mode;
	int screen_height;
	int screen_width;
	bitmap_t *tmpbitmap0;
	bitmap_t *tmpbitmap1;
	UINT8 *videoram0;
	UINT8 *videoram1;
	UINT8 *palette;
	UINT8 *clut;
	int flipscreen_old;
};


/*----------- defined in video/nbmj8900.c -----------*/

SCREEN_UPDATE( nbmj8900 );
VIDEO_START( nbmj8900_2layer );

READ8_HANDLER( nbmj8900_palette_type1_r );
WRITE8_HANDLER( nbmj8900_palette_type1_w );
WRITE8_HANDLER( nbmj8900_blitter_w );
WRITE8_HANDLER( nbmj8900_scrolly_w );
WRITE8_HANDLER( nbmj8900_vramsel_w );
WRITE8_HANDLER( nbmj8900_romsel_w );
WRITE8_HANDLER( nbmj8900_clutsel_w );
READ8_HANDLER( nbmj8900_clut_r );
WRITE8_HANDLER( nbmj8900_clut_w );
