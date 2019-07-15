// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#ifndef MAME_SOUND_2612INTF_H
#define MAME_SOUND_2612INTF_H

#pragma once


class ym2612_device : public device_t, public device_sound_interface
{
public:
	ym2612_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	// update request from fm.cpp
	static void update_request(device_t *param) { downcast<ym2612_device *>(param)->update_request(); }

protected:
	ym2612_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_post_load() override;
	virtual void device_stop() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	void irq_handler(int irq);
	void timer_handler(int c, int count, int clock);
	void update_request() { m_stream->update(); }

	static void static_irq_handler(device_t *param, int irq) { downcast<ym2612_device *>(param)->irq_handler(irq); }
	static void static_timer_handler(device_t *param, int c, int count, int clock) { downcast<ym2612_device *>(param)->timer_handler(c, count, clock); }

	void calculate_rates();

	sound_stream *  m_stream;
	emu_timer *     m_timer[2];
	void *          m_chip;
	devcb_write_line m_irq_handler;
};


class ym3438_device : public ym2612_device
{
public:
	ym3438_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(YM2612, ym2612_device)
DECLARE_DEVICE_TYPE(YM3438, ym3438_device)

#endif // MAME_SOUND_2612INTF_H
