// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Sega g80 common sound hardware

*************************************************************************/
#ifndef MAME_AUDIO_SEGASND_H
#define MAME_AUDIO_SEGASND_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "machine/timer.h"

class segag80snd_common : public driver_device {
public:
	segag80snd_common(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu")
	{ }

	virtual ~segag80snd_common() = default;

	DECLARE_WRITE_LINE_MEMBER(segaspeech_int_w);

	void sega_speech_board(machine_config &config);

protected:
	void speech_map(address_map &map);
	void speech_portmap(address_map &map);

	optional_device<cpu_device> m_audiocpu;
};

#define SEGASND_SEGASPEECH_REGION "segaspeech:speech"

class speech_sound_device : public device_t, public device_sound_interface
{
public:
	speech_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto int_cb() { return m_int_cb.bind(); }

	void data_w(uint8_t data);
	void control_w(uint8_t data);

	DECLARE_READ_LINE_MEMBER( t0_r );
	DECLARE_READ_LINE_MEMBER( t1_r );
	uint8_t p1_r();
	uint8_t rom_r(offs_t offset);
	void p1_w(uint8_t data);
	void p2_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(drq_w);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	devcb_write_line m_int_cb;
	required_memory_region m_speech;

	// internal state
	u8 m_drq;
	u8 m_latch;
	u8 m_t0;
	u8 m_p2;

	TIMER_CALLBACK_MEMBER( delayed_speech_w );
};

DECLARE_DEVICE_TYPE(SEGASPEECH, speech_sound_device)

#endif // MAME_AUDIO_SEGASND_H
