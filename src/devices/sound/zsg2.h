// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, hap
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

#define MCFG_ZSG2_EXT_READ_HANDLER(_devcb) \
	devcb = &zsg2_device::set_ext_read_handler(*device, DEVCB_##_devcb);


// ======================> zsg2_device

class zsg2_device : public device_t,
					public device_sound_interface
{
public:
	zsg2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~zsg2_device() { }

	// static configuration helpers
	template<class _Object> static devcb_base &set_ext_read_handler(device_t &device, _Object object) { return downcast<zsg2_device &>(device).m_ext_read_handler.set_callback(object); }

	DECLARE_READ16_MEMBER(read);
	DECLARE_WRITE16_MEMBER(write);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

private:
	// 16 registers per channel, 48 channels
	struct zchan
	{
		UINT16 v[16];
		bool is_playing;
		INT16 *samples;
		UINT32 cur_pos;
		UINT32 step_ptr;
		UINT32 step;
		UINT32 start_pos;
		UINT32 end_pos;
		UINT32 loop_pos;
		UINT32 page;
		UINT16 vol;
		UINT16 flags;
		UINT8 panl;
		UINT8 panr;
	};

	zchan m_chan[48];

	required_region_ptr<UINT32> m_mem_base;
	UINT32 m_read_address;
	UINT32 *m_mem_copy;
	UINT32 m_mem_blocks;
	INT16 *m_full_samples;

	sound_stream *m_stream;

	devcb_read32 m_ext_read_handler;

	UINT32 read_memory(UINT32 offset);
	void chan_w(int ch, int reg, UINT16 data);
	UINT16 chan_r(int ch, int reg);
	void control_w(int reg, UINT16 data);
	UINT16 control_r(int reg);
	INT16 *prepare_samples(UINT32 offset);
};

extern const device_type ZSG2;


#endif  /* __ZSG2_H__ */
