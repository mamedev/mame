// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/vc4000.h
 *
 ****************************************************************************/

#ifndef MAME_INTERTON_VC4000_A_H
#define MAME_INTERTON_VC4000_A_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vc4000_sound_device

class vc4000_sound_device : public device_t, public device_sound_interface
{
public:
	vc4000_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~vc4000_sound_device() { }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

public:
	void soundport_w(int mode, int data);

private:
	sound_stream *m_channel;
	uint8_t m_reg[1];
	int m_size;
	int m_pos;
	unsigned m_level;
};

DECLARE_DEVICE_TYPE(VC4000_SND, vc4000_sound_device)

#endif // MAME_INTERTON_VC4000_A_H
