// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
#ifndef MAME_SOUND_M61545_H
#define MAME_SOUND_M61545_H

#pragma once

class m61545_device : public device_t, public device_sound_interface
{
public:
	m61545_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void data_w(int state);
	void clock_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	sound_stream *m_stream;
	u32 m_shift;
	u8 m_supply;
	bool m_data;
	bool m_clock;
	float m_gain[2];

	void latch_data();
};

DECLARE_DEVICE_TYPE(M61545, m61545_device)

#endif // MAME_SOUND_M61545_H
