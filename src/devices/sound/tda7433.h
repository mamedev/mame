// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_SOUND_TDA7433_H
#define MAME_SOUND_TDA7433_H

#pragma once

#include "machine/i2chle.h"

class tda7433_device : public device_t, public device_sound_interface, public i2c_hle_interface
{
public:
	tda7433_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream) override;

	virtual u8 read_data(u16 offset) override;
	virtual void write_data(u16 offset, u8 data) override;
	virtual const char *get_tag() override { return tag(); }

private:
	static inline constexpr int NUM_REGISTERS = 7;

	void recalculate_gains();
	float attenuator_gain(u8 reg) const;

	sound_stream *m_stream;
	u8 m_registers[NUM_REGISTERS];
	float m_gain[4];
};

DECLARE_DEVICE_TYPE(TDA7433, tda7433_device)

#endif // MAME_SOUND_TDA7433_H
