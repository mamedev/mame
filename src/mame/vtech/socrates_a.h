// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
#ifndef MAME_VTECH_SOCRATES_A_H
#define MAME_VTECH_SOCRATES_A_H

#pragma once

class socrates_snd_device : public device_t,
							public device_sound_interface
{
public:
	socrates_snd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void reg0_w(int data);
	void reg1_w(int data);
	void reg2_w(int data);
	void reg3_w(int data);
	void reg4_w(int data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;
private:
	void snd_clock();
	static const uint8_t s_volumeLUT[];

	// internal state
	sound_stream *  m_stream = nullptr;
	uint8_t           m_freq[2]{};      // channel 1,2 frequencies
	uint8_t           m_vol[2]{};       // channel 1,2 volume
	uint8_t           m_enable[2]{};    // channel 1,2 enable
	uint8_t           m_channel3 = 0;   // channel 3 weird register
	uint8_t           m_state[3]{};     // output states for channels 1,2,3
	uint8_t           m_accum[3]{};     // accumulators for channels 1,2,3
	uint16_t          m_DAC_output = 0; // output
};

DECLARE_DEVICE_TYPE(SOCRATES_SOUND, socrates_snd_device)


#endif // MAME_VTECH_SOCRATES_A_H
