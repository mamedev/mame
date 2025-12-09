// license:BSD-3-Clause
// copyright-holders:Angelo Salese
#ifndef MAME_SOUND_TMS3631_H
#define MAME_SOUND_TMS3631_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> tms3631_device

class tms3631_device : public device_t, public device_sound_interface
{
public:
	tms3631_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::SOUND; }

	void enable_w(offs_t offset, u8 data);
	void data_w(u8 data);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream) override;

private:
	sound_stream *m_stream;
	int m_samplerate;
	int m_basefreq;
//  int m_counter8[8];
//  int m_output8;
	int m_enable;
	int m_wclk, m_ce;
	u8 m_channel_select;
	bool m_program_enable;
	u8 m_channel_data[8];

	std::string print_audio_state();
};

DECLARE_DEVICE_TYPE(TMS3631, tms3631_device)

#endif // MAME_SOUND_TMS3631_H
