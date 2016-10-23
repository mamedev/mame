// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
#include "includes/nb1413m3.h"

class nbmj8900_state : public driver_device
{
public:
	enum
	{
		TIMER_BLITTER
	};

	nbmj8900_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_nb1413m3(*this, "nb1413m3"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")   { }

	required_device<cpu_device> m_maincpu;
	required_device<nb1413m3_device> m_nb1413m3;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

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
	int m_screen_height;
	int m_screen_width;
	bitmap_ind16 m_tmpbitmap0;
	bitmap_ind16 m_tmpbitmap1;
	std::unique_ptr<uint8_t[]> m_videoram0;
	std::unique_ptr<uint8_t[]> m_videoram1;
	std::unique_ptr<uint8_t[]> m_palette_ptr;
	std::unique_ptr<uint8_t[]> m_clut;
	int m_flipscreen_old;
	emu_timer *m_blitter_timer;

	uint8_t palette_type1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void palette_type1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void clutsel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t clut_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void clut_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blitter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vramsel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void romsel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_togenkyo();
	void init_ohpaipee();
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vramflip(int vram);
	void update_pixel0(int x, int y);
	void update_pixel1(int x, int y);
	void gfxdraw();
	void postload();

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
