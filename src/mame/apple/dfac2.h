// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_APPLE_DFAC2_H
#define MAME_APPLE_DFAC2_H

#pragma once

#include "machine/i2chle.h"

class dfac2_device : public device_t, public device_sound_interface, public i2c_hle_interface
{
public:
	dfac2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_r overrides
	virtual void device_start() override ATTR_COLD;
	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream) override;
	// i2c_hle_interface overrides
	virtual void write_data(u16 offset, u8 data) override;
	virtual const char *get_tag() override { return tag(); }

private:
	sound_stream *m_stream;
	bool m_data, m_clock, m_dfaclatch;
	u8 m_settings_byte;
};

DECLARE_DEVICE_TYPE(APPLE_DFAC2, dfac2_device)

#endif // MAME_APPLE_DFAC2_H
