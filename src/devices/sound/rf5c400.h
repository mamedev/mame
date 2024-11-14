// license:BSD-3-Clause
// copyright-holders:Ville Linde
/* Ricoh RF5C400 emulator */

#ifndef MAME_SOUND_RF5C400_H
#define MAME_SOUND_RF5C400_H

#pragma once

#include "dirom.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> rf5c400_device

class rf5c400_device : public device_t,
					   public device_sound_interface,
					   public device_rom_interface<25, 1, 0, ENDIANNESS_LITTLE>
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; } // unemulated and/or unverified effects and envelopes

	rf5c400_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t rf5c400_r(offs_t offset, uint16_t mem_mask = ~0);
	void rf5c400_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// device_rom_interface overrides
	virtual void rom_bank_pre_change() override;

private:
	struct rf5c400_channel
	{
		rf5c400_channel() { }

		uint16_t startH = 0;
		uint16_t startL = 0;
		uint16_t freq = 0;
		uint16_t endL = 0;
		uint16_t endHloopH = 0;
		uint16_t loopL = 0;
		uint16_t pan = 0;
		uint16_t effect = 0;
		uint16_t volume = 0;

		uint16_t attack = 0;
		uint16_t decay = 0;
		uint16_t release = 0;

		uint16_t cutoff = 0;

		uint64_t pos = 0;
		uint64_t step = 0;
		uint16_t keyon = 0;

		uint8_t env_phase = 0;
		double env_level = 0.0;
		double env_step = 0.0;
		double env_scale = 0.0;
	};

	class envelope_tables
	{
	public:
		envelope_tables();
		void init(uint32_t clock);
		double ar(rf5c400_channel const &chan) const { return m_ar[decode80(chan.attack >> 8)]; }
		double dr(rf5c400_channel const &chan) const { return m_dr[decode80(chan.decay >> 8)]; }
		double rr(rf5c400_channel const &chan) const { return m_rr[decode80(chan.release >> 8)]; }
	private:
		static constexpr uint8_t decode80(uint8_t val) { return (val & 0x80) ? ((val & 0x7f) + 0x1f) : val; }
		double m_ar[0x9f];
		double m_dr[0x9f];
		double m_rr[0x9f];
	};

	uint8_t decode80(uint8_t val);

	sound_stream *m_stream;

	envelope_tables m_env_tables;

	rf5c400_channel m_channels[32];

	uint16_t m_rf5c400_status;
	uint32_t m_ext_mem_address;
	uint16_t m_ext_mem_data;
	uint16_t m_req_channel;
};

DECLARE_DEVICE_TYPE(RF5C400, rf5c400_device)

#endif // MAME_SOUND_RF5C400_H
