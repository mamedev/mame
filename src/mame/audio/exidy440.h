// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_AUDIO_EXIDY440_H
#define MAME_AUDIO_EXIDY440_H

#pragma once

#define EXIDY440_AUDIO_CLOCK    (XTAL(12'979'200) / 4)
#define EXIDY440_MC3418_CLOCK   (EXIDY440_AUDIO_CLOCK / 4 / 16)
#define EXIDY440_MC3417_CLOCK   (EXIDY440_AUDIO_CLOCK / 4 / 32)


class exidy440_sound_device : public device_t, public device_sound_interface
{
public:
	exidy440_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~exidy440_sound_device() {}

	DECLARE_READ8_MEMBER(sound_command_r);
	DECLARE_READ8_MEMBER(sound_volume_r);
	DECLARE_WRITE8_MEMBER(sound_volume_w);
	DECLARE_WRITE_LINE_MEMBER(sound_interrupt_w);
	DECLARE_WRITE8_MEMBER(sound_interrupt_clear_w);
	DECLARE_WRITE_LINE_MEMBER(sound_reset_w);
	DECLARE_READ8_MEMBER(m6844_r);
	DECLARE_WRITE8_MEMBER(m6844_w);
	DECLARE_WRITE8_MEMBER(sound_banks_w);

	void exidy440_sound_command(uint8_t param);
	uint8_t exidy440_sound_command_ack();
protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_stop() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	void exidy440_audio_map(address_map &map);

	/* channel_data structure holds info about each 6844 DMA channel */
	struct m6844_channel_data
	{
		int active;
		int address;
		int counter;
		uint8_t control;
		int start_address;
		int start_counter;
	};


	/* channel_data structure holds info about each active sound channel */
	struct sound_channel_data
	{
		int16_t *base;
		int offset;
		int remaining;
	};


	/* sound_cache_entry structure contains info on each decoded sample */
	struct sound_cache_entry
	{
		struct sound_cache_entry *next;
		int address;
		int length;
		int bits;
		int frequency;
		int16_t data[1];
	};

	required_device<cpu_device> m_audiocpu;
	required_region_ptr<uint8_t> m_samples;

	// internal state
	uint8_t m_sound_command;
	uint8_t m_sound_command_ack;

	uint8_t m_sound_banks[4];
	//uint8_t m_m6844_data[0x20];
	uint8_t m_sound_volume[0x10];
	std::unique_ptr<int32_t[]> m_mixer_buffer_left;
	std::unique_ptr<int32_t[]> m_mixer_buffer_right;
	sound_cache_entry *m_sound_cache;
	sound_cache_entry *m_sound_cache_end;
	sound_cache_entry *m_sound_cache_max;

	/* 6844 description */
	m6844_channel_data m_m6844_channel[4];
	uint8_t m_m6844_priority;
	uint8_t m_m6844_interrupt;
	uint8_t m_m6844_chain;

	/* sound interface parameters */
	sound_stream *m_stream;
	sound_channel_data m_sound_channel[4];

	/* debugging */
	FILE *m_debuglog;

	/* channel frequency is configurable */
	int m_channel_frequency[4];

	void m6844_update();
	void m6844_finished(m6844_channel_data *channel);
	void play_cvsd(int ch);
	void stop_cvsd(int ch);

	void reset_sound_cache();
	int16_t *add_to_sound_cache(uint8_t *input, int address, int length, int bits, int frequency);
	int16_t *find_or_add_to_sound_cache(int address, int length, int bits, int frequency);

	void decode_and_filter_cvsd(uint8_t *data, int bytes, int maskbits, int frequency, int16_t *dest);
	void fir_filter(int32_t *input, int16_t *output, int count);

	void add_and_scale_samples(int ch, int32_t *dest, int samples, int volume);
	void mix_to_16(int length, stream_sample_t *dest_left, stream_sample_t *dest_right);
};

DECLARE_DEVICE_TYPE(EXIDY440, exidy440_sound_device)


#endif // MAME_AUDIO_EXIDY440_H
