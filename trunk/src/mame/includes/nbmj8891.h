class nbmj8891_state : public driver_device
{
public:
	nbmj8891_state(const machine_config &mconfig, device_type type, const char *tag)
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
	bitmap_ind16 m_tmpbitmap0;
	bitmap_ind16 m_tmpbitmap1;
	UINT8 *m_videoram0;
	UINT8 *m_videoram1;
	UINT8 *m_palette;
	UINT8 *m_clut;
	int m_param_old[0x10];
	int m_param_cnt;
	int m_flipscreen_old;
	DECLARE_READ8_MEMBER(taiwanmb_unk_r);
	DECLARE_READ8_MEMBER(nbmj8891_palette_type1_r);
	DECLARE_WRITE8_MEMBER(nbmj8891_palette_type1_w);
	DECLARE_READ8_MEMBER(nbmj8891_palette_type2_r);
	DECLARE_WRITE8_MEMBER(nbmj8891_palette_type2_w);
	DECLARE_READ8_MEMBER(nbmj8891_palette_type3_r);
	DECLARE_WRITE8_MEMBER(nbmj8891_palette_type3_w);
	DECLARE_WRITE8_MEMBER(nbmj8891_clutsel_w);
	DECLARE_READ8_MEMBER(nbmj8891_clut_r);
	DECLARE_WRITE8_MEMBER(nbmj8891_clut_w);
	DECLARE_WRITE8_MEMBER(nbmj8891_blitter_w);
	DECLARE_WRITE8_MEMBER(nbmj8891_taiwanmb_blitter_w);
	DECLARE_WRITE8_MEMBER(nbmj8891_taiwanmb_gfxdraw_w);
	DECLARE_WRITE8_MEMBER(nbmj8891_taiwanmb_gfxflag_w);
	DECLARE_WRITE8_MEMBER(nbmj8891_taiwanmb_mcu_w);
	DECLARE_WRITE8_MEMBER(nbmj8891_scrolly_w);
	DECLARE_WRITE8_MEMBER(nbmj8891_vramsel_w);
	DECLARE_WRITE8_MEMBER(nbmj8891_romsel_w);
};


/*----------- defined in video/nbmj8891.c -----------*/

SCREEN_UPDATE_IND16( nbmj8891 );
VIDEO_START( nbmj8891_1layer );
VIDEO_START( nbmj8891_2layer );

