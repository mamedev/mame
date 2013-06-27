/*********************************************************/
/*    ricoh RF5C68(or clone) PCM controller              */
/*********************************************************/

#pragma once

#ifndef __RF5C68_H__
#define __RF5C68_H__

#define RF5C68_NUM_CHANNELS (8)


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_RF5C68_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, RF5C68, _clock)
#define MCFG_RF5C68_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, RF5C68, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct rf5c68_interface
{
	void (*sample_end_callback)(device_t* device, int channel);
};


struct rf5c68_pcm_channel
{
	rf5c68_pcm_channel() :
		enable(0),
		env(0),
		pan(0),
		start(0),
		addr(0),
		step(0),
		loopst(0) {}

	UINT8       enable;
	UINT8       env;
	UINT8       pan;
	UINT8       start;
	UINT32      addr;
	UINT16      step;
	UINT16      loopst;
};



// ======================> rf5c68_device

class rf5c68_device : public device_t,
						public device_sound_interface
{
public:
	rf5c68_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~rf5c68_device() { }

protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

public:
	DECLARE_READ8_MEMBER( rf5c68_r );
	DECLARE_WRITE8_MEMBER( rf5c68_w );

	DECLARE_READ8_MEMBER( rf5c68_mem_r );
	DECLARE_WRITE8_MEMBER( rf5c68_mem_w );

private:
	sound_stream*        m_stream;
	rf5c68_pcm_channel   m_chan[RF5C68_NUM_CHANNELS];
	UINT8                m_cbank;
	UINT8                m_wbank;
	UINT8                m_enable;
	UINT8                m_data[0x10000];
	void                (*m_sample_callback)(device_t* device,int channel);
};

extern const device_type RF5C68;


#endif /* __RF5C68_H__ */
