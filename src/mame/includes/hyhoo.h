// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
#include "includes/nb1413m3.h"

class hyhoo_state : public driver_device
{
public:
	enum
	{
		TIMER_BLITTER
	};

	hyhoo_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nb1413m3(*this, "nb1413m3"),
		m_screen(*this, "screen"),
		m_clut(*this, "clut") { }

	required_device<cpu_device> m_maincpu;
	required_device<nb1413m3_device> m_nb1413m3;
	required_device<screen_device> m_screen;
	required_shared_ptr<UINT8> m_clut;

	int m_blitter_destx;
	int m_blitter_desty;
	int m_blitter_sizex;
	int m_blitter_sizey;
	int m_blitter_src_addr;
	int m_blitter_direction_x;
	int m_blitter_direction_y;
	int m_gfxrom;
	int m_dispflag;
	int m_highcolorflag;
	int m_flipscreen;
	bitmap_rgb32 m_tmpbitmap;

	DECLARE_WRITE8_MEMBER(hyhoo_blitter_w);
	DECLARE_WRITE8_MEMBER(hyhoo_romsel_w);

	DECLARE_CUSTOM_INPUT_MEMBER(nb1413m3_busyflag_r);

	virtual void video_start() override;

	UINT32 screen_update_hyhoo(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void hyhoo_gfxdraw();

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
