// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    Super A'Can sound driver

**********************************************************************/

#ifndef MAME_SOUND_ACAN_H
#define MAME_SOUND_ACAN_H

#pragma once

class acan_sound_device : public device_t, public device_sound_interface
{
public:
	acan_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto ram_read() { return m_ram_read.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	struct acan_channel
	{
		uint16_t pitch;
		uint16_t length;
		uint16_t start_addr;
		uint16_t curr_addr;
		uint16_t end_addr;
		uint32_t addr_increment;
		uint32_t frac;
		uint8_t  envelope[4];
		uint8_t  volume;
		uint8_t  volume_l;
		uint8_t  volume_r;
	};

	sound_stream *m_stream;
	devcb_read8 m_ram_read;
	uint16_t m_active_channels;
	acan_channel m_channels[15];
	uint8_t m_regs[256];
};

DECLARE_DEVICE_TYPE(ACANSND, acan_sound_device)

#endif // MAME_SOUND_ACAN_H
