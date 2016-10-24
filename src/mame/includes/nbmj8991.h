// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi

#include "includes/nb1413m3.h"
#include "machine/gen_latch.h"

class nbmj8991_state : public driver_device
{
public:
	nbmj8991_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_nb1413m3(*this, "nb1413m3"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_generic_paletteram_8(*this, "paletteram") { }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<nb1413m3_device> m_nb1413m3;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_generic_paletteram_8;

	enum
	{
		TIMER_BLITTER
	};

	int m_scrollx;
	int m_scrolly;
	int m_blitter_destx;
	int m_blitter_desty;
	int m_blitter_sizex;
	int m_blitter_sizey;
	int m_blitter_src_addr;
	int m_blitter_direction_x;
	int m_blitter_direction_y;
	int m_gfxrom;
	int m_dispflag;
	int m_flipscreen;
	int m_clutsel;
	int m_screen_refresh;
	bitmap_ind16 m_tmpbitmap;
	std::unique_ptr<uint8_t[]> m_videoram;
	std::unique_ptr<uint8_t[]> m_clut;
	int m_flipscreen_old;
	emu_timer *m_blitter_timer;

	void soundbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void palette_type1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void palette_type2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void palette_type3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blitter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t clut_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void clut_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value nb1413m3_busyflag_r(ioport_field &field, void *param);

	void init_galkaika();
	void init_tokimbsj();
	void init_tokyogal();
	void init_finalbny();
	virtual void machine_reset() override;
	virtual void video_start() override;

	uint32_t screen_update_type1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_type2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vramflip();
	void update_pixel(int x, int y);
	void gfxdraw();

	void postload();

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
