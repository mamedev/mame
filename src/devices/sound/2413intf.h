// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#pragma once

#ifndef __2413INTF_H__
#define __2413INTF_H__

#include "emu.h"

class ym2413_device : public device_t,
									public device_sound_interface
{
public:
	ym2413_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE8_MEMBER( register_port_w );
	DECLARE_WRITE8_MEMBER( data_port_w );

	void _ym2413_update_request();

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	sound_stream *  m_stream;
	void *          m_chip;
};

extern const device_type YM2413;


#endif /* __2413INTF_H__ */
