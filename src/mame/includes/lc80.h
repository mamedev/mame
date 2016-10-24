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

	void ctc_z0_w(int state);
	void ctc_z1_w(int state);
	void ctc_z2_w(int state);
	void pio1_pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pio1_pb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pio1_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pio2_pb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void trigger_reset(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void trigger_nmi(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

	void update_display();

	// display state
	uint8_t m_digit;
	uint8_t m_segment;
};

#endif
