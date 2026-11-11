// license:BSD-3-Clause
// copyright-holders:MrBaristaNZ
/**********************************************************************

    Cirrus Logic CS4341 Stereo DAC with I2C control port

**********************************************************************/

#ifndef MAME_SOUND_CS4341_H
#define MAME_SOUND_CS4341_H

#pragma once

#include "machine/i2chle.h"

class cs4341_device : public device_t,
					  public device_sound_interface,
					  public i2c_hle_interface
{
public:
	cs4341_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// MCLK / LRCK ratio used by the board
	void set_mclk_ratio(u32 ratio) { m_mclk_ratio = ratio; }

	u32 sample_rate() const { return clock() / m_mclk_ratio; }

	// one 16-bit sample for channel A (0) or B (1)
	void write(offs_t channel, s16 data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

	// i2c_hle_interface implementation
	virtual u8 read_data(u16 offset) override;
	virtual void write_data(u16 offset, u8 data) override;
	virtual const char *get_tag() override { return tag(); }

private:
	static constexpr u32 BUFFER_SIZE = 4096;

	void hold_pointer(u16 offset);
	void update_volume();

	sound_stream *m_stream;
	u32 m_mclk_ratio;

	u8 m_regs[5];
	sound_stream::sample_t m_volume[2];

	s16 m_buffer[2][BUFFER_SIZE];
	u32 m_bufin[2];
	u32 m_bufout[2];
};

DECLARE_DEVICE_TYPE(CS4341, cs4341_device)

#endif // MAME_SOUND_CS4341_H
