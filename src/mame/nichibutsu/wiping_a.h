// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
#ifndef MAME_NICHIBUTSU_WIPING_A_H
#define MAME_NICHIBUTSU_WIPING_A_H

#pragma once

class wiping_sound_device : public device_t, public device_sound_interface
{
public:
	wiping_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void sound_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	// 8 voices max
	static constexpr unsigned MAX_VOICES = 8;

	// this structure defines the parameters for a channel
	struct wp_sound_channel
	{
		int frequency;
		int counter;
		int volume;
		const uint8_t *wave;
		int oneshot;
		int oneshotplaying;
	};

	// internal state

	// data about the sound system
	wp_sound_channel m_channel_list[MAX_VOICES];
	wp_sound_channel *m_last_channel;

	// global sound parameters
	required_region_ptr<uint8_t> m_sound_prom;
	required_region_ptr<uint8_t> m_sound_rom;
	int m_num_voices;
	int m_sound_enable;
	sound_stream *m_stream;

	// mixer tables and internal buffers
	std::vector<short> m_mixer_buffer;

	uint8_t m_soundregs[0x4000];

	void make_mixer_table(int voices, int gain);
};

DECLARE_DEVICE_TYPE(WIPING_CUSTOM, wiping_sound_device)

#endif // MAME_NICHIBUTSU_WIPING_A_H
