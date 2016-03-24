// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#pragma once

#ifndef __SP0250_H__
#define __SP0250_H__

class sp0250_device : public device_t,
									public device_sound_interface
{
public:
	sp0250_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~sp0250_device() {}

	template<class _Object> static devcb_base &set_drq_callback(device_t &device, _Object object) { return downcast<sp0250_device &>(device).m_drq.set_callback(object); }

	DECLARE_WRITE8_MEMBER( write );
	UINT8 drq_r();

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	INT16 m_amp;
	UINT8 m_pitch;
	UINT8 m_repeat;
	int m_pcount, m_rcount;
	int m_playing;
	UINT32 m_RNG;
	sound_stream * m_stream;
	int m_voiced;
	UINT8 m_fifo[15];
	int m_fifo_pos;
	devcb_write_line m_drq;

	struct
	{
		INT16 F, B;
		INT16 z1, z2;
	} m_filter[6];

	void load_values();
	TIMER_CALLBACK_MEMBER( timer_tick );
};

extern const device_type SP0250;

#define MCFG_SP0250_DRQ_CALLBACK(_write) \
	devcb = &sp0250_device::set_drq_callback(*device, DEVCB_##_write);



#endif /* __SP0250_H__ */
