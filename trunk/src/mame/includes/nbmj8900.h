class nbmj8900_state : public driver_device
{
public:
	nbmj8900_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_scrolly;
	int m_blitter_destx;
	int m_blitter_desty;
	int m_blitter_sizex;
	int m_blitter_sizey;
	int m_blitter_src_addr;
	int m_blitter_direction_x;
	int m_blitter_direction_y;
	int m_vram;
	int m_gfxrom;
	int m_dispflag;
	int m_flipscreen;
	int m_clutsel;
	int m_screen_refresh;
	int m_gfxdraw_mode;
	int m_screen_height;
	int m_screen_width;
	bitmap_t *m_tmpbitmap0;
	bitmap_t *m_tmpbitmap1;
	UINT8 *m_videoram0;
	UINT8 *m_videoram1;
	UINT8 *m_palette;
	UINT8 *m_clut;
	int m_flipscreen_old;
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
