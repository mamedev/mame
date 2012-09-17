class nbmj8991_state : public driver_device
{
public:
	nbmj8991_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_scrollx;
	int m_scrolly;
	int m_blitter_destx;
	int m_blitter_desty;
	int m_blitter_sizex;
	int m_blitter_sizey;
	int m_blitter_src_addr;
	int m_blitter_direction_x;
	int m_blitter_direction_y;
	int m_gfxrom;
	int m_dispflag;
	int m_flipscreen;
	int m_clutsel;
	int m_screen_refresh;
	bitmap_ind16 m_tmpbitmap;
	UINT8 *m_videoram;
	UINT8 *m_clut;
	int m_flipscreen_old;
	DECLARE_WRITE8_MEMBER(nbmj8991_soundbank_w);
	DECLARE_WRITE8_MEMBER(nbmj8991_sound_w);
	DECLARE_READ8_MEMBER(nbmj8991_sound_r);
	DECLARE_WRITE8_MEMBER(nbmj8991_palette_type1_w);
	DECLARE_WRITE8_MEMBER(nbmj8991_palette_type2_w);
	DECLARE_WRITE8_MEMBER(nbmj8991_palette_type3_w);
	DECLARE_WRITE8_MEMBER(nbmj8991_blitter_w);
	DECLARE_READ8_MEMBER(nbmj8991_clut_r);
	DECLARE_WRITE8_MEMBER(nbmj8991_clut_w);
	DECLARE_DRIVER_INIT(triplew1);
	DECLARE_DRIVER_INIT(mjlstory);
	DECLARE_DRIVER_INIT(mjgottub);
	DECLARE_DRIVER_INIT(ntopstar);
	DECLARE_DRIVER_INIT(galkoku);
	DECLARE_DRIVER_INIT(triplew2);
	DECLARE_DRIVER_INIT(uchuuai);
	DECLARE_DRIVER_INIT(pstadium);
	DECLARE_DRIVER_INIT(av2mj2rg);
	DECLARE_DRIVER_INIT(galkaika);
	DECLARE_DRIVER_INIT(hyouban);
	DECLARE_DRIVER_INIT(vanilla);
	DECLARE_DRIVER_INIT(av2mj1bb);
	DECLARE_DRIVER_INIT(tokimbsj);
	DECLARE_DRIVER_INIT(tokyogal);
	DECLARE_DRIVER_INIT(mcontest);
	DECLARE_DRIVER_INIT(finalbny);
	DECLARE_DRIVER_INIT(qmhayaku);
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_nbmj8991_type1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_nbmj8991_type2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/nbmj8991.c -----------*/





