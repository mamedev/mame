// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef __LC80__
#define __LC80__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"
#include "sound/speaker.h"

#define SCREEN_TAG      "screen"
#define Z80_TAG         "d201"
#define Z80CTC_TAG      "d208"
#define Z80PIO1_TAG     "d206"
#define Z80PIO2_TAG     "d207"
//#define SPEAKER_TAG       "b237"

class lc80_state : public driver_device
{
public:
	lc80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
			m_pio2(*this, Z80PIO2_TAG),
			m_cassette(*this, "cassette"),
			m_speaker(*this, "speaker"),
			m_ram(*this, RAM_TAG),
			m_y0(*this, "Y0"),
			m_y1(*this, "Y1"),
			m_y2(*this, "Y2"),
			m_y3(*this, "Y3")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio2;
	required_device<cassette_image_device> m_cassette;
	required_device<speaker_sound_device> m_speaker;
	required_device<ram_device> m_ram;
	required_ioport m_y0;
	required_ioport m_y1;
	required_ioport m_y2;
	required_ioport m_y3;

	virtual void machine_start() override;

	DECLARE_WRITE_LINE_MEMBER( ctc_z0_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_z1_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_z2_w );
	DECLARE_WRITE8_MEMBER( pio1_pa_w );
	DECLARE_READ8_MEMBER( pio1_pb_r );
	DECLARE_WRITE8_MEMBER( pio1_pb_w );
	DECLARE_READ8_MEMBER( pio2_pb_r );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_reset );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_nmi );

	void update_display();

	// display state
	UINT8 m_digit;
	UINT8 m_segment;
};

#endif
