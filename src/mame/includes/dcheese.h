// license:BSD-3-Clause
// copyright-holders:Aaron Giles

/***************************************************************************

    HAR MadMax hardware

**************************************************************************/

#include "machine/gen_latch.h"
#include "sound/bsmt2000.h"

class dcheese_state : public driver_device
{
public:
	enum
	{
		TIMER_BLITTER_SCANLINE,
		TIMER_SIGNAL_IRQ
	};

	dcheese_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen"),
		m_bsmt(*this, "bsmt"),
		m_soundlatch(*this, "soundlatch") { }

	/* video-related */
	uint16_t   m_blitter_color[2];
	uint16_t   m_blitter_xparam[16];
	uint16_t   m_blitter_yparam[16];
	uint16_t   m_blitter_vidparam[32];

	std::unique_ptr<bitmap_ind16> m_dstbitmap;
	emu_timer *m_blitter_timer;

	/* misc */
	uint8_t    m_irq_state[5];
	uint8_t    m_soundlatch_full;
	uint8_t    m_sound_control;
	uint8_t    m_sound_msb_latch;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<screen_device> m_screen;
	required_device<bsmt2000_device> m_bsmt;
	required_device<generic_latch_8_device> m_soundlatch;

	void eeprom_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t sound_command_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sound_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sound_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bsmt_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void madmax_blitter_color_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void madmax_blitter_xparam_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void madmax_blitter_yparam_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void madmax_blitter_vidparam_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void madmax_blitter_unknown_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t madmax_blitter_vidparam_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	ioport_value sound_latch_state_r(ioport_field &field, void *param);
	virtual void machine_start() override;
	virtual void video_start() override;
	void palette_init_dcheese(palette_device &palette);
	uint32_t screen_update_dcheese(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void dcheese_vblank(device_t &device);
	void dcheese_signal_irq(int which);
	void update_irq_state();
	int irq_callback(device_t &device, int irqline);
	void update_scanline_irq();
	void do_clear(  );
	void do_blit(  );

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

/*----------- defined in drivers/dcheese.c -----------*/
