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

#include "imagedev/cassette.h"
#include "sound/beep.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

/* Interrupts */
#define INT_HIGH_TONE       0x40
#define INT_TRANSMIT_EMPTY  0x20
#define INT_RECEIVE_FULL    0x10
#define INT_RTC         0x08
#define INT_DISPLAY_END     0x04
#define INT_SET         0x100
#define INT_CLEAR       0x200

/* ULA context */

struct ULA
{
	UINT8 interrupt_status;
	UINT8 interrupt_control;
	UINT8 rompage;
	UINT16 screen_start;
	UINT16 screen_base;
	int screen_size;
	UINT16 screen_addr;
	UINT8 *vram;
	int current_pal[16];
	int communication_mode;
	int screen_mode;
	int cassette_motor_mode;
	int capslock_mode;
//  int scanline;
	/* tape reading related */
	UINT32 tape_value;
	int tape_steps;
	int bit_count;
	int high_tone_set;
	int start_bit;
	int stop_bit;
	int tape_running;
	UINT8 tape_byte;
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
		m_keybd(*this, "LINE")
	{ }

	ULA m_ula;
	emu_timer *m_tape_timer;
	int m_map4[256];
	int m_map16[256];
	emu_timer *m_scanline_timer;
	DECLARE_READ8_MEMBER(electron_read_keyboard);
	DECLARE_READ8_MEMBER(electron_jim_r);
	DECLARE_WRITE8_MEMBER(electron_jim_w);
	DECLARE_READ8_MEMBER(electron_1mhz_r);
	DECLARE_WRITE8_MEMBER(electron_1mhz_w);
	DECLARE_READ8_MEMBER(electron_ula_r);
	DECLARE_WRITE8_MEMBER(electron_ula_w);
	void electron_tape_start();
	void electron_tape_stop();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(electron);
	UINT32 screen_update_electron(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(electron_tape_timer_handler);
	TIMER_CALLBACK_MEMBER(setup_beep);
	TIMER_CALLBACK_MEMBER(electron_scanline_interrupt);
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<beep_device> m_beeper;
	required_device<generic_slot_device> m_cart;
	required_ioport_array<14> m_keybd;
	inline UINT8 read_vram( UINT16 addr );
	inline void electron_plot_pixel(bitmap_ind16 &bitmap, int x, int y, UINT32 color);
	void electron_interrupt_handler(int mode, int interrupt);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( electron_cart );

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};


#endif /* ELECTRON_H_ */
