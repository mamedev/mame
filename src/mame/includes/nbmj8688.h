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

	nbmj8688_state(const machine_config &mconfig, device_type type, const char *tag) :
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
	std::unique_ptr<uint16_t[]> m_videoram;
	std::unique_ptr<uint8_t[]> m_clut;
	int m_flipscreen_old;
	emu_timer *m_blitter_timer;

	// common
	uint8_t ff_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void clut_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blitter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);


	void mjsikaku_gfxflag2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mjsikaku_gfxflag3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mjsikaku_romsel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void secolove_romsel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void crystalg_romsel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void seiha_romsel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void HD61830B_both_instr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void HD61830B_both_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dipsw1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dipsw2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void barline_output_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	ioport_value nb1413m3_busyflag_r(ioport_field &field, void *param);

	void init_kyuhito();
	void init_idhimitu();
	void init_kaguya2();
	void init_mjcamera();
	void init_kanatuen();
	void video_start_mbmj8688_pure_12bit();
	void palette_init_mbmj8688_12bit(palette_device &palette);
	void video_start_mbmj8688_pure_16bit_LCD();
	void palette_init_mbmj8688_16bit(palette_device &palette);
	void palette_init_mbmj8688_lcd(palette_device &palette);
	void video_start_mbmj8688_8bit();
	void palette_init_mbmj8688_8bit(palette_device &palette);
	void video_start_mbmj8688_hybrid_16bit();
	void video_start_mbmj8688_hybrid_12bit();
	void video_start_mbmj8688_pure_16bit();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
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
