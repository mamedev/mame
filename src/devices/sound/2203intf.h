// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#pragma once

#ifndef __2203INTF_H__
#define __2203INTF_H__

#include "emu.h"
#include "ay8910.h"

void ym2203_update_request(void *param);

#define MCFG_YM2203_IRQ_HANDLER(_devcb) \
	devcb = &ym2203_device::set_irq_handler(*device, DEVCB_##_devcb);

class ym2203_device : public ay8910_device
{
public:
	ym2203_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<ym2203_device &>(device).m_irq_handler.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_READ8_MEMBER( status_port_r );
	DECLARE_READ8_MEMBER( read_port_r );
	DECLARE_WRITE8_MEMBER( control_port_w );
	DECLARE_WRITE8_MEMBER( write_port_w );

	void _IRQHandler(int irq);
	void _timer_handler(int c,int count,int clock);
	void _ym2203_update_request();

protected:
	// device-level overrides
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
};

extern const device_type YM2203;


#endif /* __2203INTF_H__ */
