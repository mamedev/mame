// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
#include "video/hd61830.h"
#include "includes/nb1413m3.h"

class nbmj8688_state : public driver_device
{
public:
	enum
	{
		TIMER_BLITTER
	};

	nbmj8688_state(const machine_config &mconfig, device_type type, std::string tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nb1413m3(*this, "nb1413m3"),
		m_lcdc0(*this, "lcdc0"),
		m_lcdc1(*this, "lcdc1")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<nb1413m3_device> m_nb1413m3;
	optional_device<hd61830_device> m_lcdc0;
	optional_device<hd61830_device> m_lcdc1;

	// defined in video_start
	int m_gfxmode;

	int m_scrolly;
	int m_blitter_destx;
	int m_blitter_desty;
	int m_blitter_sizex;
	int m_blitter_sizey;
	int m_blitter_direction_x;
	int m_blitter_direction_y;
	int m_blitter_src_addr;
	int m_gfxrom;
	int m_dispflag;
	int m_gfxflag2;
	int m_gfxflag3;
	int m_flipscreen;
	int m_screen_refresh;
	std::unique_ptr<bitmap_ind16> m_tmpbitmap;
	std::unique_ptr<UINT16[]> m_videoram;
	std::unique_ptr<UINT8[]> m_clut;
	int m_flipscreen_old;
	emu_timer *m_blitter_timer;

	// common
	DECLARE_READ8_MEMBER(ff_r);
	DECLARE_WRITE8_MEMBER(clut_w);
	DECLARE_WRITE8_MEMBER(blitter_w);
	DECLARE_WRITE8_MEMBER(scrolly_w);


	DECLARE_WRITE8_MEMBER(mjsikaku_gfxflag2_w);
	DECLARE_WRITE8_MEMBER(mjsikaku_gfxflag3_w);
	DECLARE_WRITE8_MEMBER(mjsikaku_romsel_w);
	DECLARE_WRITE8_MEMBER(secolove_romsel_w);
	DECLARE_WRITE8_MEMBER(crystalg_romsel_w);
	DECLARE_WRITE8_MEMBER(seiha_romsel_w);
	DECLARE_WRITE8_MEMBER(HD61830B_both_instr_w);
	DECLARE_WRITE8_MEMBER(HD61830B_both_data_w);
	DECLARE_READ8_MEMBER(dipsw1_r);
	DECLARE_READ8_MEMBER(dipsw2_r);
	DECLARE_WRITE8_MEMBER(barline_output_w);

	DECLARE_CUSTOM_INPUT_MEMBER(nb1413m3_busyflag_r);

	DECLARE_DRIVER_INIT(kyuhito);
	DECLARE_DRIVER_INIT(idhimitu);
	DECLARE_DRIVER_INIT(kaguya2);
	DECLARE_DRIVER_INIT(mjcamera);
	DECLARE_DRIVER_INIT(kanatuen);
	DECLARE_VIDEO_START(mbmj8688_pure_12bit);
	DECLARE_PALETTE_INIT(mbmj8688_12bit);
	DECLARE_VIDEO_START(mbmj8688_pure_16bit_LCD);
	DECLARE_PALETTE_INIT(mbmj8688_16bit);
	DECLARE_PALETTE_INIT(mbmj8688_lcd);
	DECLARE_VIDEO_START(mbmj8688_8bit);
	DECLARE_PALETTE_INIT(mbmj8688_8bit);
	DECLARE_VIDEO_START(mbmj8688_hybrid_16bit);
	DECLARE_VIDEO_START(mbmj8688_hybrid_12bit);
	DECLARE_VIDEO_START(mbmj8688_pure_16bit);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vramflip();
	void update_pixel(int x, int y);
	void writeram_low(int x, int y, int color);
	void writeram_high(int x, int y, int color);
	void gfxdraw(int gfxtype);
	void common_video_start();
	void postload();

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
