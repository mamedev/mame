// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef MAME_INCLUDES_POLY880_H
#define MAME_INCLUDES_POLY880_H


#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "imagedev/cassette.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"
#include "sound/wave.h"
#include "speaker.h"

#define SCREEN_TAG      "screen"
#define Z80_TAG         "i1"
#define Z80CTC_TAG      "i4"
#define Z80PIO1_TAG     "i2"
#define Z80PIO2_TAG     "i3"

class poly880_state : public driver_device
{
public:
	poly880_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80_TAG)
		, m_cassette(*this, "cassette")
		, m_ki(*this, "KI%u", 1U)
		, m_speaker(*this, "speaker")
		, m_digits(*this, "digit%u", 0U)
		, m_nmi(false)
	{ }

	void poly880(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER( trigger_reset );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_nmi );

private:
	required_device<z80_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_ioport_array<3> m_ki;
	required_device<speaker_sound_device> m_speaker;
	output_finder<8> m_digits;

	virtual void machine_start() override;

	DECLARE_WRITE8_MEMBER( cldig_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_z0_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_z1_w );
	DECLARE_WRITE8_MEMBER( pio1_pa_w );
	DECLARE_READ8_MEMBER( pio1_pb_r );
	DECLARE_WRITE8_MEMBER( pio1_pb_w );

	void update_display();

	/* display state */
	uint8_t m_digit;
	uint8_t m_segment;
	bool m_nmi;
	void poly880_io(address_map &map);
	void poly880_mem(address_map &map);
};

#endif
