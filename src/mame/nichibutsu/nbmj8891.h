// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi

#include "nb1413m3.h"
#include "emupal.h"
#include "screen.h"

class nbmj8891_state : public driver_device
{
public:
	nbmj8891_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nb1413m3(*this, "nb1413m3")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_clut_ptr(*this, "protection")
	{
	}

	void mjfocusm(machine_config &config);
	void mjfocus(machine_config &config);
	void bananadr(machine_config &config);
	void scandal(machine_config &config);
	void hanamomo(machine_config &config);
	void telmahjn(machine_config &config);
	void pairsten(machine_config &config);
	void club90s(machine_config &config);
	void mgion(machine_config &config);
	void chinmoku(machine_config &config);
	void msjiken(machine_config &config);
	void hnageman(machine_config &config);
	void mjcamerb(machine_config &config);
	void mjnanpas(machine_config &config);
	void mmcamera(machine_config &config);
	void pairsnb(machine_config &config);
	void taiwanmb(machine_config &config);
	void hanaoji(machine_config &config);
	void lovehous(machine_config &config);
	void hnxmasev(machine_config &config);
	void mmaiko(machine_config &config);
	void maiko(machine_config &config);
	void mladyhtr(machine_config &config);
	void omotesnd(machine_config &config);
	void abunai(machine_config &config);
	void gionbana(machine_config &config);
	void mgmen89(machine_config &config);
	void scandalm(machine_config &config);

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

	int nb1413m3_outcoin_flag_r();

private:
	required_device<cpu_device> m_maincpu;
	required_device<nb1413m3_device> m_nb1413m3;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_region_ptr<uint8_t> m_clut_ptr;

	int m_scrolly = 0;
	int m_blitter_destx = 0;
	int m_blitter_desty = 0;
	int m_blitter_sizex = 0;
	int m_blitter_sizey = 0;
	int m_blitter_src_addr = 0;
	int m_blitter_direction_x = 0;
	int m_blitter_direction_y = 0;
	int m_vram = 0;
	int m_gfxrom = 0;
	int m_dispflag = 0;
	int m_flipscreen = 0;
	int m_clutsel = 0;
	int m_screen_refresh = 0;
	int m_gfxdraw_mode = 0;
	bitmap_ind16 m_tmpbitmap0;
	bitmap_ind16 m_tmpbitmap1;
	std::unique_ptr<uint8_t[]> m_videoram0;
	std::unique_ptr<uint8_t[]> m_videoram1;
	std::unique_ptr<uint8_t[]> m_palette_ptr;
	std::unique_ptr<uint8_t[]> m_clut;
	int m_param_old[0x10];
	int m_param_cnt =0;
	int m_flipscreen_old = 0;
	emu_timer *m_blitter_timer = nullptr;

	uint8_t palette_type1_r(offs_t offset);
	void palette_type1_w(offs_t offset, uint8_t data);
	uint8_t palette_type2_r(offs_t offset);
	void palette_type2_w(offs_t offset, uint8_t data);
	uint8_t palette_type3_r(offs_t offset);
	void palette_type3_w(offs_t offset, uint8_t data);
	void clutsel_w(uint8_t data);
	uint8_t clut_r(offs_t offset);
	void clut_w(offs_t offset, uint8_t data);
	void blitter_w(offs_t offset, uint8_t data);
	void scrolly_w(uint8_t data);
	void vramsel_w(uint8_t data);
	void romsel_w(uint8_t data);

	uint8_t taiwanmb_unk_r();
	void taiwanmb_blitter_w(offs_t offset, uint8_t data);
	void taiwanmb_gfxdraw_w(uint8_t data);
	void taiwanmb_gfxflag_w(uint8_t data);
	void taiwanmb_mcu_w(uint8_t data);

	virtual void video_start() override ATTR_COLD;
	DECLARE_VIDEO_START(_1layer);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vramflip(int vram);
	void update_pixel0(int x, int y);
	void update_pixel1(int x, int y);
	void gfxdraw();

	void common_save_state();
	void postload();

	void bananadr_io_map(address_map &map) ATTR_COLD;
	void club90s_map(address_map &map) ATTR_COLD;
	void gionbana_io_map(address_map &map) ATTR_COLD;
	void gionbana_map(address_map &map) ATTR_COLD;
	void hanamomo_io_map(address_map &map) ATTR_COLD;
	void hanamomo_map(address_map &map) ATTR_COLD;
	void hanaoji_map(address_map &map) ATTR_COLD;
	void hnageman_map(address_map &map) ATTR_COLD;
	void hnxmasev_map(address_map &map) ATTR_COLD;
	void lovehous_io_map(address_map &map) ATTR_COLD;
	void lovehous_map(address_map &map) ATTR_COLD;
	void maiko_io_map(address_map &map) ATTR_COLD;
	void maiko_map(address_map &map) ATTR_COLD;
	void mgion_io_map(address_map &map) ATTR_COLD;
	void mgion_map(address_map &map) ATTR_COLD;
	void mmaiko_map(address_map &map) ATTR_COLD;
	void msjiken_io_map(address_map &map) ATTR_COLD;
	void omotesnd_io_map(address_map &map) ATTR_COLD;
	void omotesnd_map(address_map &map) ATTR_COLD;
	void scandal_io_map(address_map &map) ATTR_COLD;
	void scandalm_io_map(address_map &map) ATTR_COLD;
	void scandalm_map(address_map &map) ATTR_COLD;
	void taiwanmb_io_map(address_map &map) ATTR_COLD;
	void taiwanmb_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(clear_busy_flag);
};
