// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_APPLE_DFAC_H
#define MAME_APPLE_DFAC_H

#pragma once

class dfac_device : public device_t, public device_sound_interface
{
public:
	dfac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// DFAC uses a serial 3-wire interface where a single control byte is shifted in, and it ignores
	// writes with the gate is not at the right level.  This allowed it to be used on the Egret's I2C bus
	// without it being disturbed by actual I2C transactions.
	void data_write(int state) { m_data = state; }
	void clock_write(int state);
	void latch_write(int state);

protected:
	// device_r overrides
	virtual void device_start() override ATTR_COLD;
	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	sound_stream *m_stream;
	bool m_data, m_clock, m_dfaclatch;
	u8 m_settings_byte, m_latch_byte;
};

DECLARE_DEVICE_TYPE(APPLE_DFAC, dfac_device)

#endif // MAME_APPLE_DFAC_H
