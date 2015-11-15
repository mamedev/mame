// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef __C80__
#define __C80__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80pio.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"

#define SCREEN_TAG      "screen"
#define Z80_TAG         "d2"
#define Z80PIO1_TAG     "d11"
#define Z80PIO2_TAG     "d12"

class c80_state : public driver_device
{
public:
	c80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
			m_pio1(*this, Z80PIO1_TAG),
			m_cassette(*this, "cassette"),
			m_row0(*this, "ROW0"),
			m_row1(*this, "ROW1"),
			m_row2(*this, "ROW2")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio1;
	required_device<cassette_image_device> m_cassette;
	required_ioport m_row0;
	required_ioport m_row1;
	required_ioport m_row2;

	virtual void machine_start();

	DECLARE_READ8_MEMBER( pio1_pa_r );
	DECLARE_WRITE8_MEMBER( pio1_pa_w );
	DECLARE_WRITE8_MEMBER( pio1_pb_w );
	DECLARE_WRITE_LINE_MEMBER( pio1_brdy_w );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_reset );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_nmi );

	/* keyboard state */
	int m_keylatch;

	/* display state */
	int m_digit;
	int m_pio1_a5;
	int m_pio1_brdy;
};

#endif
