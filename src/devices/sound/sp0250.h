// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#pragma once

#ifndef __SP0250_H__
#define __SP0250_H__

class sp0250_device : public device_t,
									public device_sound_interface
{
public:
	sp0250_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~sp0250_device() {}

	template<class _Object> static devcb_base &set_drq_callback(device_t &device, _Object object) { return downcast<sp0250_device &>(device).m_drq.set_callback(object); }

	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t drq_r();

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	int16_t m_amp;
	uint8_t m_pitch;
	uint8_t m_repeat;
	int m_pcount, m_rcount;
	int m_playing;
	uint32_t m_RNG;
	sound_stream * m_stream;
	int m_voiced;
	uint8_t m_fifo[15];
	int m_fifo_pos;
	devcb_write_line m_drq;

	struct
	{
		int16_t F, B;
		int16_t z1, z2;
	} m_filter[6];

	void load_values();
	void timer_tick(void *ptr, int32_t param);
};

extern const device_type SP0250;

#define MCFG_SP0250_DRQ_CALLBACK(_write) \
	devcb = &sp0250_device::set_drq_callback(*device, DEVCB_##_write);



#endif /* __SP0250_H__ */
