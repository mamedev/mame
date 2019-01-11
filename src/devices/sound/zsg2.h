// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, hap
/*
    ZOOM ZSG-2 custom wavetable synthesizer
*/

#ifndef MAME_SOUND_ZSG2_H
#define MAME_SOUND_ZSG2_H

#pragma once


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ZSG2_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, ZSG2, _clock)
#define MCFG_ZSG2_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, ZSG2, _clock)

#define MCFG_ZSG2_EXT_READ_HANDLER(_devcb) \
	devcb = &downcast<zsg2_device &>(*device).set_ext_read_handler(DEVCB_##_devcb);


// ======================> zsg2_device

class zsg2_device : public device_t,
					public device_sound_interface
{
public:
	zsg2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	template <class Object> devcb_base &set_ext_read_handler(Object &&cb) { return m_ext_read_handler.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ16_MEMBER(read);
	DECLARE_WRITE16_MEMBER(write);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// 16 registers per channel, 48 channels
	struct zchan
	{
		uint16_t v[16];
		bool is_playing;
		int16_t *samples;
		uint32_t cur_pos;
		uint32_t step_ptr;
		uint32_t step;
		uint32_t start_pos;
		uint32_t end_pos;
		uint32_t loop_pos;
		uint32_t page;
		uint16_t vol;
		uint16_t flags;
		uint8_t panl;
		uint8_t panr;
	};

	zchan m_chan[48];

	required_region_ptr<uint32_t> m_mem_base;
	uint32_t m_read_address;
	std::unique_ptr<uint32_t[]> m_mem_copy;
	uint32_t m_mem_blocks;
	std::unique_ptr<int16_t[]> m_full_samples;

	sound_stream *m_stream;

	devcb_read32 m_ext_read_handler;

	uint32_t read_memory(uint32_t offset);
	void chan_w(int ch, int reg, uint16_t data);
	uint16_t chan_r(int ch, int reg);
	void control_w(int reg, uint16_t data);
	uint16_t control_r(int reg);
	int16_t *prepare_samples(uint32_t offset);
};

DECLARE_DEVICE_TYPE(ZSG2, zsg2_device)

#endif // MAME_SOUND_ZSG2_H
