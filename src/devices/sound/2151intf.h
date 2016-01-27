// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

    2151intf.h

    MAME interface to YM2151 emulator.

***************************************************************************/

#pragma once

#ifndef __2151INTF_H__
#define __2151INTF_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_YM2151_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, YM2151, _clock)

#define MCFG_YM2151_IRQ_HANDLER(_devcb) \
	devcb = &ym2151_device::set_irq_handler(*device, DEVCB_##_devcb);
#define MCFG_YM2151_PORT_WRITE_HANDLER(_devcb) \
	devcb = &ym2151_device::set_port_write_handler(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> ym2151_device

class ym2151_device :   public device_t,
						public device_sound_interface
{
public:
	// construction/destruction
	ym2151_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<ym2151_device &>(device).m_irqhandler.set_callback(object); }
	template<class _Object> static devcb_base &set_port_write_handler(device_t &device, _Object object) { return downcast<ym2151_device &>(device).m_portwritehandler.set_callback(object); }

	// read/write
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_READ8_MEMBER( status_r );
	DECLARE_WRITE8_MEMBER( register_w );
	DECLARE_WRITE8_MEMBER( data_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal helpers
	static void irq_frontend(device_t *device, int irq);
	static void port_write_frontend(device_t *device, offs_t offset, UINT8 data);

	// internal state
	sound_stream *          m_stream;
	//emu_timer *             m_timer[2];
	void *                  m_chip;
	UINT8                   m_lastreg;
	devcb_write_line       m_irqhandler;
	devcb_write8           m_portwritehandler;
};


// device type definition
extern const device_type YM2151;


#endif /* __2151INTF_H__ */
