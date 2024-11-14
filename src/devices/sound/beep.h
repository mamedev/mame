// license:BSD-3-Clause
// copyright-holders:Kevin Thacker
/**********************************************************************

    Simple beeper sound driver

**********************************************************************/

#ifndef MAME_SOUND_BEEP_H
#define MAME_SOUND_BEEP_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class beep_device : public device_t, public device_sound_interface
{
public:
	beep_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_state(int state);          // enable/disable sound output
	void set_clock(uint32_t frequency); // output frequency

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	sound_stream *m_stream;             // stream number
	bool m_enable;                      // enable beep
	uint32_t m_frequency;               // set frequency - this can be changed using the appropriate function
	int32_t m_incr;                     // initial wave state
	stream_buffer::sample_t m_signal;   // current signal
};

DECLARE_DEVICE_TYPE(BEEP, beep_device)

#endif // MAME_SOUND_BEEP_H
