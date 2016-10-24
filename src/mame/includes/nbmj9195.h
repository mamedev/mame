// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    nbmj9195 - Nichibutsu Mahjong games for years 1991-1995

******************************************************************************/

#include "cpu/z80/tmpz84c011.h"
#include "machine/gen_latch.h"

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
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_palette_ptr(*this, "paletteram")
	{ }

	required_device<tmpz84c011_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	optional_shared_ptr<uint8_t> m_palette_ptr; //shabdama doesn't use it at least for now

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
	std::unique_ptr<uint16_t[]> m_videoram[VRAM_MAX];
	std::unique_ptr<uint16_t[]> m_videoworkram[VRAM_MAX];
	std::unique_ptr<uint8_t[]> m_clut[VRAM_MAX];
	int m_flipscreen_old[VRAM_MAX];
	emu_timer *m_blitter_timer;

	void soundbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void inputportsel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mscoutm_dipsw_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mscoutm_dipsw_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mscoutm_cpu_portb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mscoutm_cpu_portc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t others_cpu_porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t others_cpu_portb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t others_cpu_portc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void soundcpu_porte_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void palette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nb22090_palette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blitter_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blitter_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t blitter_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t blitter_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void clut_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void clut_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void clutsel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gfxflag2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void outcoin_flag_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dipswbitsel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mscoutm_inputportsel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_nbmj9195();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void video_start__1layer();
	void video_start_nb22090();

	void ctc0_trg1(device_t &device);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int blitter_r(int offset, int vram);
	void blitter_w(int offset, int data, int vram);
	void clut_w(int offset, int data, int vram);
	void vramflip(int vram);
	void update_pixel(int vram, int x, int y);
	void gfxdraw(int vram);
	int dipsw_r();
	void postload();

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
