#define	VRAM_MAX	2

#define	SCANLINE_MIN	0
#define	SCANLINE_MAX	512


class nbmj9195_state : public driver_device
{
public:
	nbmj9195_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int inputport;
	int dipswbitsel;
	int outcoin_flag;
	int mscoutm_inputport;
	UINT8 *nvram;
	size_t nvram_size;
	UINT8 pio_dir[5 * 2];
	UINT8 pio_latch[5 * 2];
	int scrollx[VRAM_MAX];
	int scrolly[VRAM_MAX];
	int scrollx_raster[VRAM_MAX][SCANLINE_MAX];
	int scanline[VRAM_MAX];
	int blitter_destx[VRAM_MAX];
	int blitter_desty[VRAM_MAX];
	int blitter_sizex[VRAM_MAX];
	int blitter_sizey[VRAM_MAX];
	int blitter_src_addr[VRAM_MAX];
	int blitter_direction_x[VRAM_MAX];
	int blitter_direction_y[VRAM_MAX];
	int dispflag[VRAM_MAX];
	int flipscreen[VRAM_MAX];
	int clutmode[VRAM_MAX];
	int transparency[VRAM_MAX];
	int clutsel;
	int screen_refresh;
	int gfxflag2;
	int gfxdraw_mode;
	int nb19010_busyctr;
	int nb19010_busyflag;
	bitmap_t *tmpbitmap[VRAM_MAX];
	UINT16 *videoram[VRAM_MAX];
	UINT16 *videoworkram[VRAM_MAX];
	UINT8 *palette;
	UINT8 *nb22090_palette;
	UINT8 *clut[VRAM_MAX];
	int flipscreen_old[VRAM_MAX];
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
