// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Curt Coder
#pragma once

#ifndef __MPF1__
#define __MPF1__


#include "emu.h"
#include "machine/spchrom.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "imagedev/cassette.h"
#include "machine/i8255.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "sound/speaker.h"
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
			m_pc0(*this, "PC0"),
			m_pc1(*this, "PC1"),
			m_pc2(*this, "PC2"),
			m_pc3(*this, "PC3"),
			m_pc4(*this, "PC4"),
			m_pc5(*this, "PC5"),
			m_special(*this, "SPECIAL")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_ioport m_pc0;
	required_ioport m_pc1;
	required_ioport m_pc2;
	required_ioport m_pc3;
	required_ioport m_pc4;
	required_ioport m_pc5;
	required_ioport m_special;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ8_MEMBER( ppi_pa_r );
	DECLARE_WRITE8_MEMBER( ppi_pb_w );
	DECLARE_WRITE8_MEMBER( ppi_pc_w );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_nmi );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_irq );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_res );
	DECLARE_DIRECT_UPDATE_MEMBER(mpf1_direct_update_handler);

	int m_break;
	int m_m1;

	UINT8 m_lednum;

	emu_timer *m_led_refresh_timer;
	DECLARE_DRIVER_INIT(mpf1);
	TIMER_CALLBACK_MEMBER(led_refresh);
	TIMER_DEVICE_CALLBACK_MEMBER(check_halt_callback);
};

#endif
