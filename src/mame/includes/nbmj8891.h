// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
#include "includes/nb1413m3.h"

class nbmj8891_state : public driver_device
{
public:
	enum
	{
		TIMER_BLITTER
	};

	nbmj8891_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nb1413m3(*this, "nb1413m3")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_clut_ptr(*this, "protection")
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<nb1413m3_device> m_nb1413m3;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_region_ptr<UINT8> m_clut_ptr;

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
	std::unique_ptr<UINT8[]> m_videoram0;
	std::unique_ptr<UINT8[]> m_videoram1;
	std::unique_ptr<UINT8[]> m_palette_ptr;
	std::unique_ptr<UINT8[]> m_clut;
	int m_param_old[0x10];
	int m_param_cnt;
	int m_flipscreen_old;
	emu_timer *m_blitter_timer;

	DECLARE_READ8_MEMBER(palette_type1_r);
	DECLARE_WRITE8_MEMBER(palette_type1_w);
	DECLARE_READ8_MEMBER(palette_type2_r);
	DECLARE_WRITE8_MEMBER(palette_type2_w);
	DECLARE_READ8_MEMBER(palette_type3_r);
	DECLARE_WRITE8_MEMBER(palette_type3_w);
	DECLARE_WRITE8_MEMBER(clutsel_w);
	DECLARE_READ8_MEMBER(clut_r);
	DECLARE_WRITE8_MEMBER(clut_w);
	DECLARE_WRITE8_MEMBER(blitter_w);
	DECLARE_WRITE8_MEMBER(scrolly_w);
	DECLARE_WRITE8_MEMBER(vramsel_w);
	DECLARE_WRITE8_MEMBER(romsel_w);

	DECLARE_READ8_MEMBER(taiwanmb_unk_r);
	DECLARE_WRITE8_MEMBER(taiwanmb_blitter_w);
	DECLARE_WRITE8_MEMBER(taiwanmb_gfxdraw_w);
	DECLARE_WRITE8_MEMBER(taiwanmb_gfxflag_w);
	DECLARE_WRITE8_MEMBER(taiwanmb_mcu_w);

	DECLARE_CUSTOM_INPUT_MEMBER(nb1413m3_busyflag_r);
	DECLARE_CUSTOM_INPUT_MEMBER(nb1413m3_outcoin_flag_r);

	DECLARE_DRIVER_INIT(pairsten);
	DECLARE_DRIVER_INIT(telmahjn);
	DECLARE_DRIVER_INIT(gionbana);
	DECLARE_DRIVER_INIT(omotesnd);
	DECLARE_DRIVER_INIT(scandal);
	DECLARE_DRIVER_INIT(mgmen89);
	DECLARE_DRIVER_INIT(mjfocusm);
	DECLARE_DRIVER_INIT(mjfocus);
	DECLARE_DRIVER_INIT(pairsnb);
	DECLARE_DRIVER_INIT(mjnanpas);
	virtual void video_start() override;
	DECLARE_VIDEO_START(_1layer);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vramflip(int vram);
	void update_pixel0(int x, int y);
	void update_pixel1(int x, int y);
	void gfxdraw();

	void common_save_state();
	void postload();

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
