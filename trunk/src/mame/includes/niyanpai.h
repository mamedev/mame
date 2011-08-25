#define	VRAM_MAX	3

class niyanpai_state : public driver_device
{
public:
	niyanpai_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_musobana_inputport;
	int m_musobana_outcoin_flag;
	UINT8 m_pio_dir[5];
	UINT8 m_pio_latch[5];
	int m_scrollx[VRAM_MAX];
	int m_scrolly[VRAM_MAX];
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
	int m_clutsel[VRAM_MAX];
	int m_screen_refresh;
	int m_nb19010_busyctr;
	int m_nb19010_busyflag;
	bitmap_t *m_tmpbitmap[VRAM_MAX];
	UINT16 *m_videoram[VRAM_MAX];
	UINT16 *m_videoworkram[VRAM_MAX];
	UINT16 *m_palette;
	UINT8 *m_clut[VRAM_MAX];
	int m_flipscreen_old[VRAM_MAX];
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
