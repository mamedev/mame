// license:BSD-3-Clause
// copyright-holders:windyfairy
#ifndef MAME_SOUND_DAC3350A_H
#define MAME_SOUND_DAC3350A_H

#pragma once

class dac3350a_device : public device_t, public device_sound_interface
{
public:
	dac3350a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void i2c_scl_w(int line);
	void i2c_sda_w(int line);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream) override;

private:
	void i2c_device_handle_write();
	float calculate_volume(int val);

	sound_stream *m_stream;

	uint8_t m_i2c_bus_state;
	uint8_t m_i2c_bus_address;

	uint8_t m_i2c_scli, m_i2c_sdai;
	int32_t m_i2c_bus_curbit;
	uint8_t m_i2c_bus_curval;
	uint32_t m_i2c_bytecount;

	uint8_t m_i2c_subadr;
	uint16_t m_i2c_data;

	bool m_dac_enable;

	float m_volume[2];
};

DECLARE_DEVICE_TYPE(DAC3350A, dac3350a_device)

#endif // MAME_SOUND_DAC3350A_H
