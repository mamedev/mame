// license:BSD-3-Clause
// copyright-holders:windyfairy
#ifndef MAME_SOUND_STT_SA1_H
#define MAME_SOUND_STT_SA1_H

#pragma once

#include "dirom.h"

class stt_sa1_device : public device_t,
					   public device_sound_interface,
					   public device_rom_interface<24, 1, 0, ENDIANNESS_LITTLE>
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; } // unemulated and/or unverified effects and envelopes

	stt_sa1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void enable_w(uint16_t data);

	uint16_t read(offs_t offset, uint16_t mem_mask = ~0);
	void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t key_r();
	void key_w(uint16_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	struct voice_t {
		uint64_t addr_start;
		uint64_t addr_end;
		uint64_t addr_cur;
		uint16_t vol_l;
		uint16_t vol_r;
		uint16_t freq;
		bool is_looped;
		bool enabled;
	};

	sound_stream *m_stream;

	voice_t m_voice[8];
	uint16_t m_keyctrl; // Key on/off control bit

	uint16_t m_regs[128];

	bool m_enabled;
};

DECLARE_DEVICE_TYPE(STT_SA1, stt_sa1_device)

#endif // MAME_SOUND_STT_SA1_H
