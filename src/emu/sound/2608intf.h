#pragma once

#ifndef __2608INTF_H__
#define __2608INTF_H__

#include "emu.h"
#include "ay8910.h"

void ym2608_update_request(void *param);

#define MCFG_YM2608_IRQ_HANDLER(_devcb) \
	devcb = &ym2608_device::set_irq_handler(*device, DEVCB2_##_devcb);

#define MCFG_YM2608_AY8910_INTF(_ay8910_config) \
	ym2608_device::set_ay8910_config(*device, _ay8910_config);

class ym2608_device : public device_t,
									public device_sound_interface
{
public:
	ym2608_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb2_base &set_irq_handler(device_t &device, _Object object) { return downcast<ym2608_device &>(device).m_irq_handler.set_callback(object); }
	static void set_ay8910_config(device_t &device, const ay8910_interface *ay8910_config) { downcast<ym2608_device &>(device).m_ay8910_config = ay8910_config; }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	void *_psg();
	void _IRQHandler(int irq);
	void _timer_handler(int c,int count,int clock);
	void _ym2608_update_request();

protected:
	// device-level overrides
	virtual const rom_entry *device_rom_region() const;
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_post_load();
	virtual void device_stop();
	virtual void device_reset();

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

private:
	// internal state
	sound_stream *  m_stream;
	emu_timer *     m_timer[2];
	void *          m_chip;
	void *          m_psg;
	devcb2_write_line m_irq_handler;
	const ay8910_interface *m_ay8910_config;
};

extern const device_type YM2608;


#endif /* __2608INTF_H__ */
