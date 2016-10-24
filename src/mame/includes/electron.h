// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*****************************************************************************
 *
 * includes/electron.h
 *
 * Acorn Electron
 *
 * Driver by Wilbert Pol
 *
 ****************************************************************************/

#ifndef ELECTRON_H_
#define ELECTRON_H_

#include "machine/ram.h"
#include "imagedev/cassette.h"
#include "sound/beep.h"

#include "bus/electron/exp.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

/* Interrupts */
#define INT_HIGH_TONE       0x40
#define INT_TRANSMIT_EMPTY  0x20
#define INT_RECEIVE_FULL    0x10
#define INT_RTC             0x08
#define INT_DISPLAY_END     0x04
#define INT_SET             0x100
#define INT_CLEAR           0x200

/* ULA context */

struct ULA
{
	uint8_t interrupt_status;
	uint8_t interrupt_control;
	uint8_t rompage;
	uint16_t screen_start;
	uint16_t screen_base;
	int screen_size;
	uint16_t screen_addr;
	uint8_t *vram;
	int current_pal[16];
	int communication_mode;
	int screen_mode;
	int cassette_motor_mode;
	int capslock_mode;
//  int scanline;
	/* tape reading related */
	uint32_t tape_value;
	int tape_steps;
	int bit_count;
	int high_tone_set;
	int start_bit;
	int stop_bit;
	int tape_running;
	uint8_t tape_byte;
};


class electron_state : public driver_device
{
public:
	enum
	{
		TIMER_TAPE_HANDLER,
		TIMER_SETUP_BEEP,
		TIMER_SCANLINE_INTERRUPT
	};

	electron_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cassette(*this, "cassette"),
		m_beeper(*this, "beeper"),
		m_cart(*this, "cartslot"),
		m_keybd(*this, "LINE.%u", 0),
		m_exp(*this, "exp"),
		m_ram(*this, RAM_TAG)
	{ }

	ULA m_ula;
	emu_timer *m_tape_timer;
	int m_map4[256];
	int m_map16[256];
	emu_timer *m_scanline_timer;
	uint8_t electron_read_keyboard(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t electron_mem_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void electron_mem_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t electron_fred_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void electron_fred_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t electron_jim_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void electron_jim_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t electron_sheila_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void electron_sheila_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void waitforramsync();
	void electron_tape_start();
	void electron_tape_stop();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_electron(palette_device &palette);
	uint32_t screen_update_electron(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void electron_tape_timer_handler(void *ptr, int32_t param);
	void setup_beep(void *ptr, int32_t param);
	void electron_scanline_interrupt(void *ptr, int32_t param);
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<beep_device> m_beeper;
	required_device<generic_slot_device> m_cart;
	required_ioport_array<14> m_keybd;
	required_device<electron_expansion_slot_device> m_exp;
	required_device<ram_device> m_ram;
	inline uint8_t read_vram( uint16_t addr );
	inline void electron_plot_pixel(bitmap_ind16 &bitmap, int x, int y, uint32_t color);
	void electron_interrupt_handler(int mode, int interrupt);
	image_init_result device_image_load_electron_cart(device_image_interface &image);
	void trigger_reset(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};


#endif /* ELECTRON_H_ */
