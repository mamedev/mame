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
	DECLARE_DRIVER_INIT(housemn2);
	DECLARE_DRIVER_INIT(bijokkoy);
	DECLARE_DRIVER_INIT(vipclub);
	DECLARE_DRIVER_INIT(iemotom);
	DECLARE_DRIVER_INIT(orangec);
	DECLARE_DRIVER_INIT(apparel);
	DECLARE_DRIVER_INIT(ojousanm);
	DECLARE_DRIVER_INIT(mjsikaku);
	DECLARE_DRIVER_INIT(mcitylov);
	DECLARE_DRIVER_INIT(otonano);
	DECLARE_DRIVER_INIT(iemoto);
	DECLARE_DRIVER_INIT(mmsikaku);
	DECLARE_DRIVER_INIT(seiham);
	DECLARE_DRIVER_INIT(barline);
	DECLARE_DRIVER_INIT(livegal);
	DECLARE_DRIVER_INIT(korinaim);
	DECLARE_DRIVER_INIT(kyuhito);
	DECLARE_DRIVER_INIT(idhimitu);
	DECLARE_DRIVER_INIT(kaguya);
	DECLARE_DRIVER_INIT(housemnq);
	DECLARE_DRIVER_INIT(seiha);
	DECLARE_DRIVER_INIT(crystal2);
	DECLARE_DRIVER_INIT(secolove);
	DECLARE_DRIVER_INIT(bijokkog);
	DECLARE_DRIVER_INIT(orangeci);
	DECLARE_DRIVER_INIT(ojousan);
	DECLARE_DRIVER_INIT(nightlov);
	DECLARE_DRIVER_INIT(kaguya2);
	DECLARE_DRIVER_INIT(korinai);
	DECLARE_DRIVER_INIT(mjcamera);
	DECLARE_DRIVER_INIT(ryuuha);
	DECLARE_DRIVER_INIT(crystalg);
	DECLARE_DRIVER_INIT(citylove);
	DECLARE_DRIVER_INIT(kanatuen);
	DECLARE_VIDEO_START(mbmj8688_pure_12bit);
	DECLARE_PALETTE_INIT(mbmj8688_12bit);
	DECLARE_VIDEO_START(mbmj8688_pure_16bit_LCD);
	DECLARE_PALETTE_INIT(mbmj8688_16bit);
	DECLARE_VIDEO_START(mbmj8688_8bit);
	DECLARE_PALETTE_INIT(mbmj8688_8bit);
	DECLARE_VIDEO_START(mbmj8688_hybrid_16bit);
	DECLARE_VIDEO_START(mbmj8688_hybrid_12bit);
	DECLARE_VIDEO_START(mbmj8688_pure_16bit);
	UINT32 screen_update_mbmj8688(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_mbmj8688_lcd0(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_mbmj8688_lcd1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/nbmj8688.c -----------*/















