// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_SOUND_X1_010_H
#define MAME_SOUND_X1_010_H

#pragma once

class x1_010_device : public device_t, public device_sound_interface, public device_rom_interface
{
public:
	x1_010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	u16 word_r(offs_t offset);
	void word_w(offs_t offset, u16 data);

	void enable_w(int data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// device_rom_interface overrides
	virtual void rom_bank_updated() override;

private:
	static constexpr unsigned NUM_CHANNELS = 16;

	// internal state

	/* Variables only used here */
	int m_rate;                              // Output sampling rate (Hz)
	sound_stream *  m_stream;                // Stream handle
	int m_sound_enable;                      // sound output enable/disable
	std::unique_ptr<uint8_t[]>   m_reg;                 // X1-010 Register & wave form area
	std::unique_ptr<uint8_t[]>   m_HI_WORD_BUF;         // X1-010 16bit access ram check avoidance work
	uint32_t  m_smp_offset[NUM_CHANNELS];
	uint32_t  m_env_offset[NUM_CHANNELS];

	uint32_t m_base_clock;
};

DECLARE_DEVICE_TYPE(X1_010, x1_010_device)

#endif // MAME_SOUND_X1_010_H
