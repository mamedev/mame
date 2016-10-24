// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef __EXP85__
#define __EXP85__

#include "bus/rs232/rs232.h"
#include "imagedev/cassette.h"
#include "sound/speaker.h"

#define SCREEN_TAG      "screen"
#define I8085A_TAG      "u100"
#define I8155_TAG       "u106"
#define I8355_TAG       "u105"

class exp85_state : public driver_device
{
public:
	exp85_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, I8085A_TAG),
			m_rs232(*this, "rs232"),
			m_cassette(*this, "cassette"),
			m_speaker(*this, "speaker"),
			m_rom(*this, I8085A_TAG),
			m_tape_control(0)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<rs232_port_device> m_rs232;
	required_device<cassette_image_device> m_cassette;
	required_device<speaker_sound_device> m_speaker;
	required_memory_region m_rom;

	virtual void machine_start() override;

	uint8_t i8355_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void i8355_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	int sid_r();
	void sod_w(int state);
	void trigger_reset(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void trigger_rst75(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

	/* cassette state */
	int m_tape_control;
};

#endif
