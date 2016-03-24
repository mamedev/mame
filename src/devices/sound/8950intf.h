// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#pragma once

#ifndef __8950INTF_H__
#define __8950INTF_H__

#include "emu.h"

#define MCFG_Y8950_IRQ_HANDLER(_devcb) \
	devcb = &y8950_device::set_irq_handler(*device, DEVCB_##_devcb);

#define MCFG_Y8950_KEYBOARD_READ_HANDLER(_devcb) \
	devcb = &y8950_device::set_keyboard_read_handler(*device, DEVCB_##_devcb);

#define MCFG_Y8950_KEYBOARD_WRITE_HANDLER(_devcb) \
	devcb = &y8950_device::set_keyboard_write_handler(*device, DEVCB_##_devcb);

#define MCFG_Y8950_IO_READ_HANDLER(_devcb) \
	devcb = &y8950_device::set_io_read_handler(*device, DEVCB_##_devcb);

#define MCFG_Y8950_IO_WRITE_HANDLER(_devcb) \
	devcb = &y8950_device::set_io_write_handler(*device, DEVCB_##_devcb);

class y8950_device : public device_t,
						public device_sound_interface
{
public:
	y8950_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<y8950_device &>(device).m_irq_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_keyboard_read_handler(device_t &device, _Object object) { return downcast<y8950_device &>(device).m_keyboard_read_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_keyboard_write_handler(device_t &device, _Object object) { return downcast<y8950_device &>(device).m_keyboard_write_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_io_read_handler(device_t &device, _Object object) { return downcast<y8950_device &>(device).m_io_read_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_io_write_handler(device_t &device, _Object object) { return downcast<y8950_device &>(device).m_io_write_handler.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_READ8_MEMBER( status_port_r );
	DECLARE_READ8_MEMBER( read_port_r );
	DECLARE_WRITE8_MEMBER( control_port_w );
	DECLARE_WRITE8_MEMBER( write_port_w );

	void _IRQHandler(int irq);
	void _timer_handler(int c, const attotime &period);
	void _y8950_update_request();
	unsigned char _Y8950PortHandler_r();
	void _Y8950PortHandler_w(unsigned char data);
	unsigned char _Y8950KeyboardHandler_r();
	void _Y8950KeyboardHandler_w(unsigned char data);

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
	devcb_read8 m_keyboard_read_handler;
	devcb_write8 m_keyboard_write_handler;
	devcb_read8 m_io_read_handler;
	devcb_write8 m_io_write_handler;
};

extern const device_type Y8950;


#endif /* __8950INTF_H__ */
