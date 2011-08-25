#define	VRAM_MAX	2

#define	SCANLINE_MIN	0
#define	SCANLINE_MAX	512


class nbmj9195_state : public driver_device
{
public:
	nbmj9195_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_inputport;
	int m_dipswbitsel;
	int m_outcoin_flag;
	int m_mscoutm_inputport;
	UINT8 *m_nvram;
	size_t m_nvram_size;
	UINT8 m_pio_dir[5 * 2];
	UINT8 m_pio_latch[5 * 2];
	int m_scrollx[VRAM_MAX];
	int m_scrolly[VRAM_MAX];
	int m_scrollx_raster[VRAM_MAX][SCANLINE_MAX];
	int m_scanline[VRAM_MAX];
	int m_blitter_destx[VRAM_MAX];
	int m_blitter_desty[VRAM_MAX];
	int m_blitter_sizex[VRAM_MAX];
	int m_blitter_sizey[VRAM_MAX];
	int m_blitter_src_addr[VRAM_MAX];
	int m_blitter_direction_x[VRAM_MAX];
	int m_blitter_direction_y[VRAM_MAX];
	int m_dispflag[VRAM_MAX];
	int m_flipscreen[VRAM_MAX];
	int m_clutmode[VRAM_MAX];
	int m_transparency[VRAM_MAX];
	int m_clutsel;
	int m_screen_refresh;
	int m_gfxflag2;
	int m_gfxdraw_mode;
	int m_nb19010_busyctr;
	int m_nb19010_busyflag;
	bitmap_t *m_tmpbitmap[VRAM_MAX];
	UINT16 *m_videoram[VRAM_MAX];
	UINT16 *m_videoworkram[VRAM_MAX];
	UINT8 *m_palette;
	UINT8 *m_nb22090_palette;
	UINT8 *m_clut[VRAM_MAX];
	int m_flipscreen_old[VRAM_MAX];
};


/*----------- defined in video/nbmj9195.c -----------*/

SCREEN_UPDATE( nbmj9195 );
VIDEO_START( nbmj9195_1layer );
VIDEO_START( nbmj9195_2layer );
VIDEO_START( nbmj9195_nb22090 );

READ8_HANDLER( nbmj9195_palette_r );
WRITE8_HANDLER( nbmj9195_palette_w );
READ8_HANDLER( nbmj9195_nb22090_palette_r );
WRITE8_HANDLER( nbmj9195_nb22090_palette_w );

READ8_HANDLER( nbmj9195_blitter_0_r );
READ8_HANDLER( nbmj9195_blitter_1_r );
WRITE8_HANDLER( nbmj9195_blitter_0_w );
WRITE8_HANDLER( nbmj9195_blitter_1_w );
WRITE8_HANDLER( nbmj9195_clut_0_w );
WRITE8_HANDLER( nbmj9195_clut_1_w );

void nbmj9195_clutsel_w(address_space *space, int data);
void nbmj9195_gfxflag2_w(address_space *space, int data);
