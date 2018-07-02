// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi

#include "machine/nb1413m3.h"
#include "emupal.h"
#include "screen.h"

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

	DECLARE_CUSTOM_INPUT_MEMBER(nb1413m3_busyflag_r);
	DECLARE_CUSTOM_INPUT_MEMBER(nb1413m3_outcoin_flag_r);

private:
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

	virtual void video_start() override;
	DECLARE_VIDEO_START(_1layer);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vramflip(int vram);
	void update_pixel0(int x, int y);
	void update_pixel1(int x, int y);
	void gfxdraw();

	void common_save_state();
	void postload();

	void bananadr_io_map(address_map &map);
	void club90s_map(address_map &map);
	void gionbana_io_map(address_map &map);
	void gionbana_map(address_map &map);
	void hanamomo_io_map(address_map &map);
	void hanamomo_map(address_map &map);
	void hanaoji_map(address_map &map);
	void hnageman_map(address_map &map);
	void hnxmasev_map(address_map &map);
	void lovehous_io_map(address_map &map);
	void lovehous_map(address_map &map);
	void maiko_io_map(address_map &map);
	void maiko_map(address_map &map);
	void mgion_io_map(address_map &map);
	void mgion_map(address_map &map);
	void mmaiko_map(address_map &map);
	void msjiken_io_map(address_map &map);
	void omotesnd_io_map(address_map &map);
	void omotesnd_map(address_map &map);
	void scandal_io_map(address_map &map);
	void scandalm_io_map(address_map &map);
	void scandalm_map(address_map &map);
	void taiwanmb_io_map(address_map &map);
	void taiwanmb_map(address_map &map);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
