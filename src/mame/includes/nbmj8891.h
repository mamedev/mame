class nbmj8891_state : public driver_device
{
public:
	nbmj8891_state(running_machine &machine, const driver_device_config_base &config)
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
	bitmap_t *tmpbitmap0;
	bitmap_t *tmpbitmap1;
	UINT8 *videoram0;
	UINT8 *videoram1;
	UINT8 *palette;
	UINT8 *clut;
	int param_old[0x10];
	int param_cnt;
	int flipscreen_old;
};


/*----------- defined in video/nbmj8891.c -----------*/

SCREEN_UPDATE( nbmj8891 );
VIDEO_START( nbmj8891_1layer );
VIDEO_START( nbmj8891_2layer );

READ8_HANDLER( nbmj8891_palette_type1_r );
WRITE8_HANDLER( nbmj8891_palette_type1_w );
READ8_HANDLER( nbmj8891_palette_type2_r );
WRITE8_HANDLER( nbmj8891_palette_type2_w );
READ8_HANDLER( nbmj8891_palette_type3_r );
WRITE8_HANDLER( nbmj8891_palette_type3_w );
WRITE8_HANDLER( nbmj8891_blitter_w );
WRITE8_HANDLER( nbmj8891_scrolly_w );
WRITE8_HANDLER( nbmj8891_vramsel_w );
WRITE8_HANDLER( nbmj8891_romsel_w );
WRITE8_HANDLER( nbmj8891_clutsel_w );
READ8_HANDLER( nbmj8891_clut_r );
WRITE8_HANDLER( nbmj8891_clut_w );
WRITE8_HANDLER( nbmj8891_taiwanmb_blitter_w );
WRITE8_HANDLER( nbmj8891_taiwanmb_gfxflag_w );
WRITE8_HANDLER( nbmj8891_taiwanmb_gfxdraw_w );
WRITE8_HANDLER( nbmj8891_taiwanmb_mcu_w );
