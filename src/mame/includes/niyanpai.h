// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
#include "sound/dac.h"
#include "machine/tmp68301.h"
#define VRAM_MAX    3

class niyanpai_state : public driver_device
{
public:
	enum
	{
		TIMER_BLITTER
	};

	niyanpai_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_tmp68301(*this, "tmp68301"),
		m_dac1(*this, "dac1"),
		m_dac2(*this, "dac2"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_device<cpu_device> m_maincpu;
	required_device<tmp68301_device> m_tmp68301;
	required_device<dac_device> m_dac1;
	required_device<dac_device> m_dac2;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	// common
	int m_scrollx[VRAM_MAX];
	int m_scrolly[VRAM_MAX];
	int m_blitter_destx[VRAM_MAX];
	int m_blitter_desty[VRAM_MAX];
	int m_blitter_sizex[VRAM_MAX];
	int m_blitter_sizey[VRAM_MAX];
	int m_blitter_src_addr[VRAM_MAX];
	int m_blitter_direction_x[VRAM_MAX];
	int m_blitter_direction_y[VRAM_MAX];
	int m_dispflag[VRAM_MAX];
	int m_flipscreen[VRAM_MAX];
	int m_clutmode[VRAM_MAX];
	int m_transparency[VRAM_MAX];
	int m_clutsel[VRAM_MAX];
	int m_screen_refresh;
	int m_nb19010_busyctr;
	int m_nb19010_busyflag;
	bitmap_ind16 m_tmpbitmap[VRAM_MAX];
	UINT16 *m_videoram[VRAM_MAX];
	UINT16 *m_videoworkram[VRAM_MAX];
	UINT16 *m_palette_ptr;
	UINT8 *m_clut[VRAM_MAX];
	int m_flipscreen_old[VRAM_MAX];
	emu_timer *m_blitter_timer;

	// musobana and derived machine configs
	int m_musobana_inputport;
	int m_musobana_outcoin_flag;
	UINT8 m_motor_on;

	// common
	DECLARE_WRITE8_MEMBER(soundbank_w);
	DECLARE_WRITE8_MEMBER(soundlatch_clear_w);
	DECLARE_READ16_MEMBER(dipsw_r);
	DECLARE_READ16_MEMBER(palette_r);
	DECLARE_WRITE16_MEMBER(palette_w);
	DECLARE_WRITE8_MEMBER(blitter_0_w);
	DECLARE_WRITE8_MEMBER(blitter_1_w);
	DECLARE_WRITE8_MEMBER(blitter_2_w);
	DECLARE_READ8_MEMBER(blitter_0_r);
	DECLARE_READ8_MEMBER(blitter_1_r);
	DECLARE_READ8_MEMBER(blitter_2_r);
	DECLARE_WRITE8_MEMBER(clut_0_w);
	DECLARE_WRITE8_MEMBER(clut_1_w);
	DECLARE_WRITE8_MEMBER(clut_2_w);
	DECLARE_WRITE8_MEMBER(clutsel_0_w);
	DECLARE_WRITE8_MEMBER(clutsel_1_w);
	DECLARE_WRITE8_MEMBER(clutsel_2_w);
	DECLARE_WRITE16_MEMBER(tmp68301_parallel_port_w);

	// musobana and derived machine configs
	DECLARE_READ16_MEMBER(musobana_inputport_0_r);
	DECLARE_WRITE16_MEMBER(musobana_inputport_w);

	DECLARE_CUSTOM_INPUT_MEMBER(musobana_outcoin_flag_r);

	DECLARE_DRIVER_INIT(niyanpai);
	virtual void video_start() override;
	DECLARE_MACHINE_START(musobana);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int blitter_r(int vram, int offset);
	void blitter_w(int vram, int offset, UINT8 data);
	void clutsel_w(int vram, UINT8 data);
	void clut_w(int vram, int offset, UINT8 data);
	void vramflip(int vram);
	void update_pixel(int vram, int x, int y);
	void gfxdraw(int vram);

	INTERRUPT_GEN_MEMBER(interrupt);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
