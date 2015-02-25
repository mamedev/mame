/*****************************************************************************
 *
 * includes/gamate.h
 *
 ****************************************************************************/

#ifndef GAMATE_H_
#define GAMATE_H_

#include "cpu/m6502/m6502.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

struct GAMATE_CHANNEL
{
	GAMATE_CHANNEL() :
//      on(0),
//      waveform(0),
		volume(0),
		pos(0),
		size(0)
//      count(0)
	{
	}

	int on;
	int /*waveform,*/ volume;
	int pos;
	int size;
//  int count;
};


// ======================> gamate_sound_device

class gamate_sound_device : public device_t,
								public device_sound_interface
{
public:
	gamate_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~gamate_sound_device() { }

protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

public:
	DECLARE_WRITE8_MEMBER( device_w );

private:

	sound_stream *m_mixer_channel;
	GAMATE_CHANNEL m_channels[3];
	UINT8 reg[14];
};

extern const device_type GAMATE_SND;

#endif /* GAMATE_H_ */
