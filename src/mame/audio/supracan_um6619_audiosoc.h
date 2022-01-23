// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, superctr
/**********************************************************************

    Super A'Can sound driver (UM6619)

**********************************************************************/

#ifndef MAME_AUDIO_SUPRACAN_UM6619_AUDIOSOC_H
#define MAME_AUDIO_SUPRACAN_UM6619_AUDIOSOC_H

#pragma once

#include "supracan_um6619_cpu.h"

class supracan_um6619_audiosoc : public supracan_um6619_cpu_device, public device_sound_interface
{
public:
	supracan_um6619_audiosoc(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t sound_read(offs_t offset) override;
	virtual void sound_write(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

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
		uint8_t  register9;
		uint8_t  envelope[4];
		uint8_t  volume;
		uint8_t  volume_l;
		uint8_t  volume_r;
		bool     one_shot;
	};

	void keyon_voice(uint8_t voice);

	sound_stream *m_stream;
	emu_timer *m_timer;
	uint16_t m_active_channels;
	uint16_t m_dma_channels;
	acan_channel m_channels[16];
	uint8_t m_regs[256];
	std::unique_ptr<int32_t[]> m_mix;
};

DECLARE_DEVICE_TYPE(SUPRACAN_UM6619_AUDIOSOC, supracan_um6619_audiosoc)

#endif // MAME_AUDIO_SUPRACAN_UM6619_AUDIOSOC_H
