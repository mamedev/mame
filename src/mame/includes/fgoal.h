// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
#include "machine/mb14241.h"

class fgoal_state : public driver_device
{
public:
	enum
	{
		TIMER_INTERRUPT
	};

	fgoal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mb14241(*this, "mb14241"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_video_ram(*this, "video_ram") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<mb14241_device> m_mb14241;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_video_ram;

	/* video-related */
	bitmap_ind16   m_bgbitmap;
	bitmap_ind16   m_fgbitmap;
	uint8_t      m_xpos;
	uint8_t      m_ypos;
	int        m_current_color;

	/* misc */
	int        m_player;
	uint8_t      m_row;
	uint8_t      m_col;
	int        m_prev_coin;
	emu_timer  *m_interrupt_timer;

	uint8_t analog_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t nmi_reset_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t irq_reset_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t row_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void row_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void col_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t address_hi_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t address_lo_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t shifter_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t shifter_reverse_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sound1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void color_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ypos_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void xpos_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	ioport_value _80_r(ioport_field &field, void *param);

	void interrupt_callback(void *ptr, int32_t param);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_fgoal(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int intensity(int bits);
	unsigned video_ram_address( );

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
