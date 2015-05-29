// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#pragma once

#ifndef __262INTF_H__
#define __262INTF_H__

#include "emu.h"

#define MCFG_YMF262_IRQ_HANDLER(_devcb) \
	devcb = &ymf262_device::set_irq_handler(*device, DEVCB_##_devcb);

class ymf262_device : public device_t,
									public device_sound_interface
{
public:
	ymf262_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<ymf262_device &>(device).m_irq_handler.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	void _IRQHandler(int irq);
	void _timer_handler(int c, const attotime &period);
	void _ymf262_update_request();

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	sound_stream *  m_stream;
	emu_timer *     m_timer[2];
	void *          m_chip;
	devcb_write_line m_irq_handler;
};

extern const device_type YMF262;


#endif /* __262INTF_H__ */
