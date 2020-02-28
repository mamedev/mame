// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#ifndef MAME_SOUND_2608INTF_H
#define MAME_SOUND_2608INTF_H

#pragma once

#include "ay8910.h"


struct ssg_callbacks;


class ym2608_device : public ay8910_device,
	public device_rom_interface
{
public:
	ym2608_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	// update request from fm.cpp
	static void update_request(device_t *param) { downcast<ym2608_device *>(param)->update_request(); }

protected:
	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_start() override;
	virtual void device_clock_changed() override;
	virtual void device_post_load() override;
	virtual void device_stop() override;
	virtual void device_reset() override;

	virtual void rom_bank_updated() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	void stream_generate(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

private:
	void irq_handler(int irq);
	void timer_handler(int c, int count, int clock);
	void update_request() { m_stream->update(); }

	static uint8_t static_external_read_byte(device_t *param, offs_t offset) { return downcast<ym2608_device *>(param)->read_byte(offset); }
	static void static_external_write_byte(device_t *param, offs_t offset, uint8_t data) { return downcast<ym2608_device *>(param)->space().write_byte(offset, data); }
	static uint8_t static_internal_read_byte(device_t *param, offs_t offset) { return downcast<ym2608_device *>(param)->m_internal->as_u8(offset % downcast<ym2608_device *>(param)->m_internal->bytes()); };

	static void static_irq_handler(device_t *param, int irq) { downcast<ym2608_device *>(param)->irq_handler(irq); }
	static void static_timer_handler(device_t *param, int c, int count, int clock) { downcast<ym2608_device *>(param)->timer_handler(c, count, clock); }

	// internal state
	sound_stream *  m_stream;
	emu_timer *     m_timer[2];
	void *          m_chip;
	devcb_write_line m_irq_handler;
	required_memory_region m_internal;

	static const ssg_callbacks psgintf;
};

DECLARE_DEVICE_TYPE(YM2608, ym2608_device)

#endif // MAME_SOUND_2608INTF_H
