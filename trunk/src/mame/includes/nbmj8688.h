class nbmj8688_state : public driver_device
{
public:
	nbmj8688_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_mjsikaku_scrolly;
	int m_blitter_destx;
	int m_blitter_desty;
	int m_blitter_sizex;
	int m_blitter_sizey;
	int m_blitter_direction_x;
	int m_blitter_direction_y;
	int m_blitter_src_addr;
	int m_mjsikaku_gfxrom;
	int m_mjsikaku_dispflag;
	int m_mjsikaku_gfxflag2;
	int m_mjsikaku_gfxflag3;
	int m_mjsikaku_flipscreen;
	int m_mjsikaku_screen_refresh;
	int m_mjsikaku_gfxmode;
	bitmap_ind16 *m_mjsikaku_tmpbitmap;
	UINT16 *m_mjsikaku_videoram;
	UINT8 *m_clut;
	UINT8 *m_HD61830B_ram[2];
	int m_HD61830B_instr[2];
	int m_HD61830B_addr[2];
	int m_mjsikaku_flipscreen_old;
	DECLARE_READ8_MEMBER(ff_r);
	DECLARE_WRITE8_MEMBER(barline_output_w);
	DECLARE_WRITE8_MEMBER(nbmj8688_clut_w);
	DECLARE_WRITE8_MEMBER(nbmj8688_blitter_w);
	DECLARE_WRITE8_MEMBER(mjsikaku_gfxflag2_w);
	DECLARE_WRITE8_MEMBER(mjsikaku_gfxflag3_w);
	DECLARE_WRITE8_MEMBER(mjsikaku_scrolly_w);
	DECLARE_WRITE8_MEMBER(mjsikaku_romsel_w);
	DECLARE_WRITE8_MEMBER(secolove_romsel_w);
	DECLARE_WRITE8_MEMBER(crystalg_romsel_w);
	DECLARE_WRITE8_MEMBER(seiha_romsel_w);
	DECLARE_WRITE8_MEMBER(nbmj8688_HD61830B_0_instr_w);
	DECLARE_WRITE8_MEMBER(nbmj8688_HD61830B_1_instr_w);
	DECLARE_WRITE8_MEMBER(nbmj8688_HD61830B_both_instr_w);
	DECLARE_WRITE8_MEMBER(nbmj8688_HD61830B_0_data_w);
	DECLARE_WRITE8_MEMBER(nbmj8688_HD61830B_1_data_w);
	DECLARE_WRITE8_MEMBER(nbmj8688_HD61830B_both_data_w);
	void mjsikaku_vramflip();
};


/*----------- defined in video/nbmj8688.c -----------*/

PALETTE_INIT( mbmj8688_8bit );
PALETTE_INIT( mbmj8688_12bit );
PALETTE_INIT( mbmj8688_16bit );
SCREEN_UPDATE_IND16( mbmj8688 );
SCREEN_UPDATE_IND16( mbmj8688_lcd0 );
SCREEN_UPDATE_IND16( mbmj8688_lcd1 );
VIDEO_START( mbmj8688_8bit );
VIDEO_START( mbmj8688_hybrid_12bit );
VIDEO_START( mbmj8688_pure_12bit );
VIDEO_START( mbmj8688_hybrid_16bit );
VIDEO_START( mbmj8688_pure_16bit );
VIDEO_START( mbmj8688_pure_16bit_LCD );


