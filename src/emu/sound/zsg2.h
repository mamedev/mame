/*
    ZOOM ZSG-2 custom wavetable synthesizer
*/

#pragma once

#ifndef __ZSG2_H__
#define __ZSG2_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ZSG2_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, ZSG2, _clock)
#define MCFG_ZSG2_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, ZSG2, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct zsg2_interface
{
	const char *samplergn;
};

// 16 registers per channel, 48 channels
struct zchan
{
    zchan()
    {
        memset(v, 0, sizeof(UINT16)*16);
    }
    
	UINT16 v[16];
};


// ======================> zsg2_device

class zsg2_device : public device_t,
					public device_sound_interface
{
public:
	zsg2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~zsg2_device() { }

protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

public:
    DECLARE_READ16_MEMBER( zsg2_r );
    DECLARE_WRITE16_MEMBER( zsg2_w );

public:
    void chan_w(int chan, int reg, UINT16 data);
    UINT16 chan_r(int chan, int reg);
    void check_channel(int chan);
    void keyon(int chan);
    void control_w(int reg, UINT16 data);
    UINT16 control_r(int reg);

private:
	zchan m_zc[48];
	UINT16 m_act[3];
	UINT16 m_alow;
    UINT16 m_ahigh;
	UINT8 *m_bank_samples;

	int m_sample_rate;
	sound_stream *m_stream;
};

extern const device_type ZSG2;


#endif  /* __ZSG2_H__ */
