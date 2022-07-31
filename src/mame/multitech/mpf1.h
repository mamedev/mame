// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Curt Coder
#ifndef MAME_INCLUDES_MPF1_H
#define MAME_INCLUDES_MPF1_H

#pragma once


#include "machine/spchrom.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "imagedev/cassette.h"
#include "machine/i8255.h"
#include "machine/timer.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "sound/spkrdev.h"
#include "sound/tms5220.h"

#define Z80_TAG         "u1"
#define Z80CTC_TAG      "u11"
#define Z80PIO_TAG      "u10"
#define I8255A_TAG      "u14"
#define TMS5220_TAG     "tms5220"

class mpf1_state : public driver_device
{
public:
	mpf1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
			m_ctc(*this, Z80CTC_TAG),
			m_speaker(*this, "speaker"),
			m_cassette(*this, "cassette"),
			m_pc(*this, "PC%u", 0U),
			m_special(*this, "SPECIAL"),
			m_digits(*this, "digit%u", 0U),
			m_leds(*this, "led%u", 0U)
	{ }

	void mpf1p(machine_config &config);
	void mpf1b(machine_config &config);
	void mpf1(machine_config &config);

	void init_mpf1();

	DECLARE_INPUT_CHANGED_MEMBER( trigger_nmi );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_irq );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_res );

private:
	required_device<z80_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_ioport_array<6> m_pc;
	required_ioport m_special;
	output_finder<6> m_digits;
	output_finder<2> m_leds;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint8_t step_r(offs_t offset);
	uint8_t ppi_pa_r();
	void ppi_pb_w(uint8_t data);
	void ppi_pc_w(uint8_t data);

	int m_break = 0;
	int m_m1 = 0;

	uint8_t m_lednum = 0;

	emu_timer *m_led_refresh_timer = nullptr;
	address_space *m_program = nullptr;

	TIMER_CALLBACK_MEMBER(led_refresh);
	TIMER_DEVICE_CALLBACK_MEMBER(check_halt_callback);
	void mpf1_io_map(address_map &map);
	void mpf1_map(address_map &map);
	void mpf1_step(address_map &map);
	void mpf1b_io_map(address_map &map);
	void mpf1b_map(address_map &map);
	void mpf1p_io_map(address_map &map);
	void mpf1p_map(address_map &map);
};

#endif // MAME_INCLUDES_MPF1_H
