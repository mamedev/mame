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
	bitmap_ind16 m_tmpbitmap[VRAM_MAX];
	UINT16 *m_videoram[VRAM_MAX];
	UINT16 *m_videoworkram[VRAM_MAX];
	UINT16 *m_palette;
	UINT8 *m_clut[VRAM_MAX];
	int m_flipscreen_old[VRAM_MAX];
	DECLARE_READ8_MEMBER(niyanpai_sound_r);
	DECLARE_WRITE16_MEMBER(niyanpai_sound_w);
	DECLARE_WRITE8_MEMBER(niyanpai_soundclr_w);
	DECLARE_READ8_MEMBER(tmpz84c011_pio_r);
	DECLARE_WRITE8_MEMBER(tmpz84c011_pio_w);
	DECLARE_READ8_MEMBER(tmpz84c011_0_pa_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_pb_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_pc_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_pd_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_pe_r);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_pa_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_pb_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_pc_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_pd_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_pe_w);
	DECLARE_READ8_MEMBER(tmpz84c011_0_dir_pa_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_dir_pb_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_dir_pc_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_dir_pd_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_dir_pe_r);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_dir_pa_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_dir_pb_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_dir_pc_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_dir_pd_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_dir_pe_w);
	DECLARE_READ16_MEMBER(niyanpai_dipsw_r);
	DECLARE_READ16_MEMBER(musobana_inputport_0_r);
	DECLARE_WRITE16_MEMBER(musobana_inputport_w);
	DECLARE_READ16_MEMBER(niyanpai_palette_r);
	DECLARE_WRITE16_MEMBER(niyanpai_palette_w);
	DECLARE_WRITE16_MEMBER(niyanpai_blitter_0_w);
	DECLARE_WRITE16_MEMBER(niyanpai_blitter_1_w);
	DECLARE_WRITE16_MEMBER(niyanpai_blitter_2_w);
	DECLARE_READ16_MEMBER(niyanpai_blitter_0_r);
	DECLARE_READ16_MEMBER(niyanpai_blitter_1_r);
	DECLARE_READ16_MEMBER(niyanpai_blitter_2_r);
	DECLARE_WRITE16_MEMBER(niyanpai_clut_0_w);
	DECLARE_WRITE16_MEMBER(niyanpai_clut_1_w);
	DECLARE_WRITE16_MEMBER(niyanpai_clut_2_w);
	DECLARE_WRITE16_MEMBER(niyanpai_clutsel_0_w);
	DECLARE_WRITE16_MEMBER(niyanpai_clutsel_1_w);
	DECLARE_WRITE16_MEMBER(niyanpai_clutsel_2_w);
};


/*----------- defined in video/niyanpai.c -----------*/

SCREEN_UPDATE_IND16( niyanpai );
VIDEO_START( niyanpai );


