// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_AUDIO_TIMEPLT_H
#define MAME_AUDIO_TIMEPLT_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "sound/flt_rc.h"

class timeplt_audio_device : public device_t, public device_sound_interface
{
public:
	timeplt_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER(sound_data_w);
	DECLARE_WRITE_LINE_MEMBER(sh_irqtrigger_w);
	DECLARE_WRITE_LINE_MEMBER(mute_w);
	DECLARE_WRITE8_MEMBER(filter_w);
	DECLARE_READ8_MEMBER(portB_r);

	void timeplt_sound(machine_config &config);
	void locomotn_sound(machine_config &config);

	void locomotn_sound_map(address_map &map);
	void timeplt_sound_map(address_map &map);
protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	required_device<cpu_device> m_soundcpu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device_array<filter_rc_device, 3> m_filter_0;
	required_device_array<filter_rc_device, 3> m_filter_1;

	uint8_t    m_last_irq_state;

	void filter_w(filter_rc_device &device, int data);
};

DECLARE_DEVICE_TYPE(TIMEPLT_AUDIO, timeplt_audio_device)

#endif // MAME_AUDIO_TIMEPLT_H
