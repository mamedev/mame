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
#ifndef MAME_INCLUDES_ELECTRON_H
#define MAME_INCLUDES_ELECTRON_H

#pragma once

#include "machine/ram.h"
#include "imagedev/cassette.h"
#include "sound/beep.h"
#include "emupal.h"

#include "bus/electron/exp.h"

/* Interrupts */
#define INT_HIGH_TONE       0x40
#define INT_TRANSMIT_EMPTY  0x20
#define INT_RECEIVE_FULL    0x10
#define INT_RTC             0x08
#define INT_DISPLAY_END     0x04
#define INT_SET             0x100
#define INT_CLEAR           0x200

class electron_state : public driver_device
{
public:
	electron_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_cassette(*this, "cassette")
		, m_beeper(*this, "beeper")
		, m_region_basic(*this, "basic")
		, m_region_mos(*this, "mos")
		, m_keybd(*this, "LINE.%u", 0)
		, m_exp(*this, "exp")
		, m_ram(*this, RAM_TAG)
		, m_mrb(*this, "MRB")
	{ }

	void electron(machine_config &config);
	void btm2105(machine_config &config);

	void electron64(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER( trigger_reset );

private:
	enum
	{
		TIMER_TAPE_HANDLER,
		TIMER_SETUP_BEEP,
		TIMER_SCANLINE_INTERRUPT
	};

	emu_timer *m_tape_timer;
	int m_map4[256];
	int m_map16[256];
	emu_timer *m_scanline_timer;
	DECLARE_READ8_MEMBER(electron64_fetch_r);
	DECLARE_READ8_MEMBER(electron_mem_r);
	DECLARE_WRITE8_MEMBER(electron_mem_w);
	DECLARE_READ8_MEMBER(electron_paged_r);
	DECLARE_WRITE8_MEMBER(electron_paged_w);
	DECLARE_READ8_MEMBER(electron_mos_r);
	DECLARE_WRITE8_MEMBER(electron_mos_w);
	DECLARE_READ8_MEMBER(electron_fred_r);
	DECLARE_WRITE8_MEMBER(electron_fred_w);
	DECLARE_READ8_MEMBER(electron_jim_r);
	DECLARE_WRITE8_MEMBER(electron_jim_w);
	DECLARE_READ8_MEMBER(electron_sheila_r);
	DECLARE_WRITE8_MEMBER(electron_sheila_w);

	void electron_colours(palette_device &palette) const;
	uint32_t screen_update_electron(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(electron_tape_timer_handler);
	TIMER_CALLBACK_MEMBER(setup_beep);
	TIMER_CALLBACK_MEMBER(electron_scanline_interrupt);

	inline uint8_t read_vram( uint16_t addr );
	inline void electron_plot_pixel(bitmap_ind16 &bitmap, int x, int y, uint32_t color);
	void electron_interrupt_handler(int mode, int interrupt);

	void electron_mem(address_map &map);

	void electron64_opcodes(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<cassette_image_device> m_cassette;
	required_device<beep_device> m_beeper;
	required_memory_region m_region_basic;
	required_memory_region m_region_mos;
	required_ioport_array<14> m_keybd;
	required_device<electron_expansion_slot_device> m_exp;
	required_device<ram_device> m_ram;
	optional_ioport m_mrb;

	void waitforramsync();
	void electron_tape_start();
	void electron_tape_stop();

	/* ULA context */
	struct ULA
	{
		uint8_t interrupt_status;
		uint8_t interrupt_control;
		uint8_t rompage;
		uint16_t screen_start;
		uint16_t screen_base;
		uint16_t screen_size;
		uint16_t screen_addr;
		int screen_dispend;
		int current_pal[16];
		int communication_mode;
		int screen_mode;
		int cassette_motor_mode;
		int capslock_mode;
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

	ULA m_ula;
	bool m_mrb_mapped;
	bool m_vdu_drivers;
};

#endif // MAME_INCLUDES_ELECTRON_H
