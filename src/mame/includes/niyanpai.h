#define	VRAM_MAX	3

class niyanpai_state : public driver_device
{
public:
	niyanpai_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int musobana_inputport;
	int musobana_outcoin_flag;
	UINT8 pio_dir[5];
	UINT8 pio_latch[5];
	int scrollx[VRAM_MAX];
	int scrolly[VRAM_MAX];
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
	int clutsel[VRAM_MAX];
	int screen_refresh;
	int nb19010_busyctr;
	int nb19010_busyflag;
	bitmap_t *tmpbitmap[VRAM_MAX];
	UINT16 *videoram[VRAM_MAX];
	UINT16 *videoworkram[VRAM_MAX];
	UINT16 *palette;
	UINT8 *clut[VRAM_MAX];
	int flipscreen_old[VRAM_MAX];
};


/*----------- defined in video/niyanpai.c -----------*/

SCREEN_UPDATE( niyanpai );
VIDEO_START( niyanpai );

READ16_HANDLER( niyanpai_palette_r );
WRITE16_HANDLER( niyanpai_palette_w );

READ16_HANDLER( niyanpai_blitter_0_r );
READ16_HANDLER( niyanpai_blitter_1_r );
READ16_HANDLER( niyanpai_blitter_2_r );
WRITE16_HANDLER( niyanpai_blitter_0_w );
WRITE16_HANDLER( niyanpai_blitter_1_w );
WRITE16_HANDLER( niyanpai_blitter_2_w );
WRITE16_HANDLER( niyanpai_clut_0_w );
WRITE16_HANDLER( niyanpai_clut_1_w );
WRITE16_HANDLER( niyanpai_clut_2_w );
WRITE16_HANDLER( niyanpai_clutsel_0_w );
WRITE16_HANDLER( niyanpai_clutsel_1_w );
WRITE16_HANDLER( niyanpai_clutsel_2_w );
