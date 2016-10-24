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
	optional_region_ptr<uint8_t> m_clut_ptr;

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
	std::unique_ptr<uint8_t[]> m_videoram0;
	std::unique_ptr<uint8_t[]> m_videoram1;
	std::unique_ptr<uint8_t[]> m_palette_ptr;
	std::unique_ptr<uint8_t[]> m_clut;
	int m_param_old[0x10];
	int m_param_cnt;
	int m_flipscreen_old;
	emu_timer *m_blitter_timer;

	uint8_t palette_type1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void palette_type1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t palette_type2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void palette_type2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t palette_type3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void palette_type3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void clutsel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t clut_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void clut_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blitter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vramsel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void romsel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t taiwanmb_unk_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void taiwanmb_blitter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void taiwanmb_gfxdraw_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void taiwanmb_gfxflag_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void taiwanmb_mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	ioport_value nb1413m3_busyflag_r(ioport_field &field, void *param);
	ioport_value nb1413m3_outcoin_flag_r(ioport_field &field, void *param);

	void init_pairsten();
	void init_telmahjn();
	void init_gionbana();
	void init_omotesnd();
	void init_scandal();
	void init_mgmen89();
	void init_mjfocusm();
	void init_mjfocus();
	void init_pairsnb();
	void init_mjnanpas();
	virtual void video_start() override;
	void video_start__1layer();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vramflip(int vram);
	void update_pixel0(int x, int y);
	void update_pixel1(int x, int y);
	void gfxdraw();

	void common_save_state();
	void postload();

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
