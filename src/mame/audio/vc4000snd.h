// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/vc4000snd.h
 *
 ****************************************************************************/

#ifndef _VC4000SND_H_
#define _VC4000SND_H_

#include "emu.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vc4000_sound_device

class vc4000_sound_device : public device_t,
								public device_sound_interface
{
public:
	vc4000_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~vc4000_sound_device() { }

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	void soundport_w(int mode, int data);

private:
	sound_stream *m_channel;
	UINT8 m_reg[1];
	int m_size;
	int m_pos;
	unsigned m_level;
};

extern const device_type VC4000_SND;

#endif /* _VC4000SND_H_ */
