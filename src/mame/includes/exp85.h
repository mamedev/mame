// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_INCLUDES_EXP85_H
#define MAME_INCLUDES_EXP85_H

#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "sound/spkrdev.h"

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

	void exp85(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER( trigger_reset );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_rst75 );

private:
	required_device<i8085a_cpu_device> m_maincpu;
	required_device<rs232_port_device> m_rs232;
	required_device<cassette_image_device> m_cassette;
	required_device<speaker_sound_device> m_speaker;
	required_memory_region m_rom;

	virtual void machine_start() override;

	DECLARE_READ8_MEMBER( i8355_a_r );
	DECLARE_WRITE8_MEMBER( i8355_a_w );
	DECLARE_READ_LINE_MEMBER( sid_r );
	DECLARE_WRITE_LINE_MEMBER( sod_w );

	/* cassette state */
	int m_tape_control;
	void exp85_io(address_map &map);
	void exp85_mem(address_map &map);
};

#endif // MAME_INCLUDES_EXP85_H
