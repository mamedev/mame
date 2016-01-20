// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#pragma once

#ifndef __3526INTF_H__
#define __3526INTF_H__

#include "emu.h"

#define MCFG_YM3526_IRQ_HANDLER(_devcb) \
	devcb = &ym3526_device::set_irq_handler(*device, DEVCB_##_devcb);

class ym3526_device : public device_t,
									public device_sound_interface
{
public:
	ym3526_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<ym3526_device &>(device).m_irq_handler.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_READ8_MEMBER( status_port_r );
	DECLARE_READ8_MEMBER( read_port_r );
	DECLARE_WRITE8_MEMBER( control_port_w );
	DECLARE_WRITE8_MEMBER( write_port_w );

	void _IRQHandler(int irq);
	void _timer_handler(int c,const attotime &period);
	void _ym3526_update_request();

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	sound_stream *  m_stream;
	emu_timer *     m_timer[2];
	void *          m_chip;
	devcb_write_line m_irq_handler;
};

extern const device_type YM3526;


#endif /* __3526INTF_H__ */
