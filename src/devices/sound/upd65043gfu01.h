// license: BSD-3-Clause
// copyright-holders: Devin Acker

#ifndef MAME_SOUND_UPD65043GFU01_H
#define MAME_SOUND_UPD65043GFU01_H

#pragma once

class upd65043gfu01_device : public device_t, public device_sound_interface
{
public:
	upd65043gfu01_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	auto irq_cb() { return m_irq_cb.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	TIMER_CALLBACK_MEMBER(irq_timer);
	void update_irq();

	void update_noise();

	sound_stream *m_stream;
	emu_timer *m_irq_timer;

	devcb_write_line m_irq_cb;

	u8 m_control;

	u16 m_period[4];
	u16 m_count[4];
	u8 m_volume[4];
	u8 m_output[4];
	u8 m_noise_mode;

	u8 m_pcm_period;
	u8 m_pcm_count;

	s8 m_pcm_buffer[0x200];
	u16 m_pcm_buffer_read, m_pcm_buffer_write;
};

DECLARE_DEVICE_TYPE(UPD65043GFU01, upd65043gfu01_device)

#endif // MAME_SOUND_UPD65043GFU01_H
