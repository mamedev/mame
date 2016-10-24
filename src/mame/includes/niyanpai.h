// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi

#include "machine/gen_latch.h"
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
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	required_device<cpu_device> m_maincpu;
	required_device<tmp68301_device> m_tmp68301;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

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
	std::unique_ptr<uint16_t[]> m_videoram[VRAM_MAX];
	std::unique_ptr<uint16_t[]> m_videoworkram[VRAM_MAX];
	std::unique_ptr<uint16_t[]> m_palette_ptr;
	std::unique_ptr<uint8_t[]> m_clut[VRAM_MAX];
	int m_flipscreen_old[VRAM_MAX];
	emu_timer *m_blitter_timer;

	// musobana and derived machine configs
	int m_musobana_inputport;
	int m_musobana_outcoin_flag;
	uint8_t m_motor_on;

	// common
	void soundbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void soundlatch_clear_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t dipsw_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t palette_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void palette_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void blitter_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blitter_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blitter_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t blitter_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t blitter_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t blitter_2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void clut_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void clut_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void clut_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void clutsel_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void clutsel_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void clutsel_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tmp68301_parallel_port_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// musobana and derived machine configs
	uint16_t musobana_inputport_0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void musobana_inputport_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	ioport_value musobana_outcoin_flag_r(ioport_field &field, void *param);

	void init_niyanpai();
	virtual void video_start() override;
	void machine_start_musobana();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int blitter_r(int vram, int offset);
	void blitter_w(int vram, int offset, uint8_t data);
	void clutsel_w(int vram, uint8_t data);
	void clut_w(int vram, int offset, uint8_t data);
	void vramflip(int vram);
	void update_pixel(int vram, int x, int y);
	void gfxdraw(int vram);

	void interrupt(device_t &device);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
