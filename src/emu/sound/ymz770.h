/***************************************************************************

    ymz770.h

***************************************************************************/

#pragma once

#ifndef __YMZ770_H__
#define __YMZ770_H__

//**************************************************************************
//  CONSTANTS
//**************************************************************************

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_YMZ770_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, YMZ770, _clock)

#define MCFG_YMZ770_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, YMZ770, _clock)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward definition
class mpeg_audio;

// ======================> ymz770_device

class ymz770_device : public device_t, public device_sound_interface
{
    struct ymz_channel
    {
        UINT8 phrase;
        UINT8 pan;
        UINT8 volume;
        UINT8 control;

        bool is_playing, last_block;

        mpeg_audio *decoder;

        INT16 output_data[1152];
        int output_remaining;
        int output_ptr;
		int pptr;

    	UINT8 sequence;
    	UINT8 seqcontrol;
    	UINT8 seqdelay;
    	UINT8 *seqdata;
    	bool is_seq_playing;

    };


public:
	// construction/destruction
	ymz770_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	sound_stream *m_stream;

protected:

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

    void internal_reg_write(int offset, UINT8 data);

    // data
    UINT8 cur_reg;
    UINT8 *rom_base;
	int rom_size;

    ymz_channel channels[8];
};


// device type definition
extern const device_type YMZ770;

#endif /* __ymz770_H__ */

