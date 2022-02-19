// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
#include "machine/nb1413m3.h"
#include "screen.h"

class hyhoo_state : public driver_device
{
public:
	hyhoo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nb1413m3(*this, "nb1413m3"),
		m_screen(*this, "screen"),
		m_clut(*this, "clut") { }

	void hyhoo(machine_config &config);
	void hyhoo2(machine_config &config);

private:
	enum
	{
		TIMER_BLITTER
	};

	required_device<cpu_device> m_maincpu;
	required_device<nb1413m3_device> m_nb1413m3;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint8_t> m_clut;

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
	emu_timer *m_blitter_timer;

	void hyhoo_blitter_w(offs_t offset, uint8_t data);
	void hyhoo_romsel_w(uint8_t data);

	virtual void video_start() override;

	uint32_t screen_update_hyhoo(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void hyhoo_gfxdraw();

	void hyhoo_io_map(address_map &map);
	void hyhoo_map(address_map &map);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;
};
