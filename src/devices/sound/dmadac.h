// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/**********************************************************************************************
 *
 *   DMA-driven DAC driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#ifndef MAME_SOUND_DMADAC_H
#define MAME_SOUND_DMADAC_H

#pragma once


class dmadac_sound_device : public device_t, public device_sound_interface
{
public:
	dmadac_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void initialize_state();
	void flush();

	template <typename T> void transfer(int channel, offs_t channel_spacing, offs_t frame_spacing, offs_t total_frames, T* data) {
		int j;

		constexpr sound_stream::sample_t sample_scale = 1.0 / double(std::numeric_limits<T>::max());

		if (m_enabled)
		{
			int maxin = (m_bufout + BUFFER_SIZE - 1) % BUFFER_SIZE;
			T* src = data + channel * channel_spacing;
			int curin = m_bufin;

			/* copy the data */
			for (j = 0; j < total_frames && curin != maxin; j++)
			{
				m_buffer[curin] = sound_stream::sample_t(*src) * sample_scale;
				curin = (curin + 1) % BUFFER_SIZE;
				src += frame_spacing;
			}
			m_bufin = curin;

			/* log overruns */
			if (j != total_frames)
				logerror("dmadac_transfer: buffer overrun (short %d frames)\n", total_frames - j);
		}
	}

	void enable(uint8_t enable);
	void set_frequency(double frequency);
	void set_volume(uint16_t volume);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	// internal state
	/* sound stream and buffers */
	sound_stream *  m_channel;
	std::vector<sound_stream::sample_t> m_buffer;
	uint32_t          m_bufin;
	uint32_t          m_bufout;

	/* per-channel parameters */
	sound_stream::sample_t m_volume;
	uint8_t           m_enabled;

	static constexpr int BUFFER_SIZE = 32768;
};

DECLARE_DEVICE_TYPE(DMADAC, dmadac_sound_device)


void dmadac_transfer(dmadac_sound_device **devlist, uint8_t num_channels, offs_t channel_spacing, offs_t frame_spacing, offs_t total_frames, int16_t *data);
void dmadac_enable(dmadac_sound_device **devlist, uint8_t num_channels, uint8_t enable);
void dmadac_set_frequency(dmadac_sound_device **devlist, uint8_t num_channels, double frequency);
void dmadac_set_volume(dmadac_sound_device **devlist, uint8_t num_channels, uint16_t volume);

#endif // MAME_SOUND_DMADAC_H
