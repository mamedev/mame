// license:BSD-3-Clause
// copyright-holders:Curt Coder

#ifndef MAME_INCLUDES_C80_H
#define MAME_INCLUDES_C80_H

#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
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
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80_TAG)
		, m_pio1(*this, Z80PIO1_TAG)
		, m_cassette(*this, "cassette")
		, m_row0(*this, "ROW0")
		, m_row1(*this, "ROW1")
		, m_row2(*this, "ROW2")
		, m_digits(*this, "digit%u", 0U)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER( trigger_reset );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_nmi );
	void c80(machine_config &config);

private:
	required_device<z80_device> m_maincpu;
	required_device<z80pio_device> m_pio1;
	required_device<cassette_image_device> m_cassette;
	required_ioport m_row0;
	required_ioport m_row1;
	required_ioport m_row2;
	output_finder<9> m_digits;

	virtual void machine_start() override;

	uint8_t pio1_pa_r();
	void pio1_pa_w(uint8_t data);
	void pio1_pb_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER( pio1_brdy_w );

	/* keyboard state */
	int m_keylatch = 0;

	/* display state */
	int m_digit = 0;
	int m_pio1_a5 = 0;
	int m_pio1_brdy = 0;
	void c80_io(address_map &map);
	void c80_mem(address_map &map);
};

#endif
