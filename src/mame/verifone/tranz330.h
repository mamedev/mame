// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#ifndef MAME_VERIFONE_TRANZ330_H
#define MAME_VERIFONE_TRANZ330_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "machine/msm6242.h"
#include "machine/roc10937.h"
#include "bus/rs232/rs232.h"
#include "sound/spkrdev.h"
#include "machine/clock.h"

#define CPU_TAG     "cpu"
#define DART_TAG    "dart"
#define CTC_TAG     "ctc"
#define PIO_TAG     "pio"
#define RTC_TAG     "rtc"
#define VFD_TAG     "vfd"
#define RS232_TAG   "rs232"
#define SPEAKER_TAG "speaker"

class tranz330_state : public driver_device
{
public:
	tranz330_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, CPU_TAG)
		, m_ctc(*this, CTC_TAG)
		, m_dart(*this, DART_TAG)
		, m_pio(*this, PIO_TAG)
		, m_rtc(*this, RTC_TAG)
		, m_vfd(*this, VFD_TAG)
		, m_rs232(*this, RS232_TAG)
		, m_speaker(*this, SPEAKER_TAG)
		, m_keypad(*this, "COL.%u", 0)
	{ }

	void tranz330(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void syncb_w(int state);
	void clock_w(int state);

	void sound_w(int state);

	void pio_a_w(uint8_t data);
	uint8_t pio_b_r();
	uint8_t card_r();

	void tranz330_mem(address_map &map) ATTR_COLD;
	void tranz330_io(address_map &map) ATTR_COLD;

	required_device<z80_device>             m_cpu;
	required_device<z80ctc_device>          m_ctc;
	required_device<z80dart_device>         m_dart;
	required_device<z80pio_device>          m_pio;
	required_device<msm6242_device>         m_rtc;
	required_device<mic10937_device>        m_vfd;
	required_device<rs232_port_device>      m_rs232;
	required_device<speaker_sound_device>   m_speaker;
	required_ioport_array<4>                m_keypad;

	uint8_t m_keypad_col_mask = 0;
};

#endif // MAME_VERIFONE_TRANZ330_H
