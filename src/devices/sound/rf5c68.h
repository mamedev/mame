// license:BSD-3-Clause
// copyright-holders:Olivier Galibert,Aaron Giles
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

#define MCFG_RF5C68_SAMPLE_END_CB(_class, _method) \
	rf5c68_device::set_end_callback(*device, rf5c68_sample_end_cb_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef device_delegate<void (int channel)> rf5c68_sample_end_cb_delegate;

#define RF5C68_SAMPLE_END_CB_MEMBER(_name)   void _name(int channel)


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

	static void set_end_callback(device_t &device, rf5c68_sample_end_cb_delegate callback) { downcast<rf5c68_device &>(device).m_sample_end_cb = callback; }

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

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

	rf5c68_sample_end_cb_delegate m_sample_end_cb;
};

extern const device_type RF5C68;


#endif /* __RF5C68_H__ */
