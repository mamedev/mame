// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    Cirrus Logic CS43L43 I2S Stereo DAC

**********************************************************************/

#ifndef MAME_SOUND_CS43L43_H
#define MAME_SOUND_CS43L43_H

#pragma once

#include "machine/i2chle.h"

class cs43l43_device : public device_t,
					   public device_sound_interface,
					   public i2c_hle_interface
{
public:
	cs43l43_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write_i2s(offs_t offset, u16 data);
	void set_frequency(u32 frequency);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// i2c_hle_interface overrides
	virtual u8 read_data(u16 offset) override;
	virtual void write_data(u16 offset, u8 data) override;
	virtual const char *get_tag() override { return tag(); }

private:
	static constexpr double BASE_FREQUENCY = 44100;
	static constexpr size_t BUFFER_SIZE = 65536;

	u8 m_power_ctrl;
	u8 m_chan_a_analog_ctrl;
	u8 m_chan_b_analog_ctrl;
	u8 m_chan_a_digital_ctrl;
	u8 m_chan_b_digital_ctrl;
	u8 m_tone_ctrl;
	u8 m_mode_ctrl;
	u8 m_limiter_attack_rate;
	u8 m_limiter_release_rate;
	u8 m_volume_mix_ctrl;
	u8 m_mode_ctrl_2;

	sound_stream *m_stream;
	std::vector<sound_stream::sample_t> m_buffer[2];
	u32 m_bufin[2];
	u32 m_bufout[2];
	sound_stream::sample_t m_volume;
	double m_frequency;

	static constexpr u8 I2C_ADDRESS = 0x20;
};

DECLARE_DEVICE_TYPE(CS43L43, cs43l43_device)

#endif // MAME_SOUND_CS43L43_H
