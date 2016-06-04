// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#pragma once

#ifndef __2608INTF_H__
#define __2608INTF_H__

#include "emu.h"
#include "ay8910.h"

void ym2608_update_request(void *param);

#define MCFG_YM2608_IRQ_HANDLER(_devcb) \
	devcb = &ym2608_device::set_irq_handler(*device, DEVCB_##_devcb);

class ym2608_device : public ay8910_device
{
public:
	ym2608_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<ym2608_device &>(device).m_irq_handler.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	void _IRQHandler(int irq);
	void _timer_handler(int c,int count,int clock);
	void _ym2608_update_request();

protected:
	// device-level overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual void device_start() override;
	virtual void device_post_load() override;
	virtual void device_stop() override;
	virtual void device_reset() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void stream_generate(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	sound_stream *  m_stream;
	emu_timer *     m_timer[2];
	void *          m_chip;
	devcb_write_line m_irq_handler;
	required_memory_region m_region;
};

extern const device_type YM2608;


#endif /* __2608INTF_H__ */
