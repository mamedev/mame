// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    nbmj9195 - Nichibutsu Mahjong games for years 1991-1995

******************************************************************************/

#include "cpu/z80/tmpz84c011.h"
#include "sound/dac.h"

#define VRAM_MAX    2

#define SCANLINE_MIN    0
#define SCANLINE_MAX    512


class nbmj9195_state : public driver_device
{
public:
	enum
	{
		TIMER_BLITTER
	};

	nbmj9195_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_dac1(*this, "dac1"),
		m_dac2(*this, "dac2"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_palette_ptr(*this, "paletteram")
	{ }

	required_device<tmpz84c011_device> m_maincpu;
	required_device<dac_device> m_dac1;
	required_device<dac_device> m_dac2;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	
	optional_shared_ptr<UINT8> m_palette_ptr; //shabdama doesn't use it at least for now

	int m_inputport;
	int m_dipswbitsel;
	int m_outcoin_flag;
	int m_mscoutm_inputport;
	int m_scrollx[VRAM_MAX];
	int m_scrolly[VRAM_MAX];
	int m_scrollx_raster[VRAM_MAX][SCANLINE_MAX];
	int m_scanline[VRAM_MAX];
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
	int m_clutsel;
	int m_screen_refresh;
	int m_gfxflag2;
	int m_gfxdraw_mode;
	int m_nb19010_busyctr;
	int m_nb19010_busyflag;
	bitmap_ind16 m_tmpbitmap[VRAM_MAX];
	UINT16 *m_videoram[VRAM_MAX];
	UINT16 *m_videoworkram[VRAM_MAX];
	UINT8 *m_clut[VRAM_MAX];
	int m_flipscreen_old[VRAM_MAX];
	emu_timer *m_blitter_timer;

	DECLARE_WRITE8_MEMBER(soundbank_w);
	DECLARE_WRITE8_MEMBER(inputportsel_w);
	DECLARE_READ8_MEMBER(mscoutm_dipsw_0_r);
	DECLARE_READ8_MEMBER(mscoutm_dipsw_1_r);
	DECLARE_READ8_MEMBER(mscoutm_cpu_portb_r);
	DECLARE_READ8_MEMBER(mscoutm_cpu_portc_r);
	DECLARE_READ8_MEMBER(others_cpu_porta_r);
	DECLARE_READ8_MEMBER(others_cpu_portb_r);
	DECLARE_READ8_MEMBER(others_cpu_portc_r);
	DECLARE_WRITE8_MEMBER(soundcpu_porte_w);
	DECLARE_WRITE8_MEMBER(palette_w);
	DECLARE_WRITE8_MEMBER(nb22090_palette_w);
	DECLARE_WRITE8_MEMBER(blitter_0_w);
	DECLARE_WRITE8_MEMBER(blitter_1_w);
	DECLARE_READ8_MEMBER(blitter_0_r);
	DECLARE_READ8_MEMBER(blitter_1_r);
	DECLARE_WRITE8_MEMBER(clut_0_w);
	DECLARE_WRITE8_MEMBER(clut_1_w);
	DECLARE_WRITE8_MEMBER(clutsel_w);
	DECLARE_WRITE8_MEMBER(gfxflag2_w);
	DECLARE_WRITE8_MEMBER(outcoin_flag_w);
	DECLARE_WRITE8_MEMBER(dipswbitsel_w);
	DECLARE_WRITE8_MEMBER(mscoutm_inputportsel_w);

	DECLARE_DRIVER_INIT(nbmj9195);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_VIDEO_START(_1layer);
	DECLARE_VIDEO_START(nb22090);

	INTERRUPT_GEN_MEMBER(ctc0_trg1);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int blitter_r(int offset, int vram);
	void blitter_w(int offset, int data, int vram);
	void clut_w(int offset, int data, int vram);
	void vramflip(int vram);
	void update_pixel(int vram, int x, int y);
	void gfxdraw(int vram);
	int dipsw_r();
	void postload();

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};
