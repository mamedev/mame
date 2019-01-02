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

	void flush();
	void transfer(int channel, offs_t channel_spacing, offs_t frame_spacing, offs_t total_frames, int16_t *data);
	void enable(uint8_t enable);
	void set_frequency(double frequency);
	void set_volume(uint16_t volume);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	/* sound stream and buffers */
	sound_stream *  m_channel;
	std::unique_ptr<int16_t[]>         m_buffer;
	uint32_t          m_bufin;
	uint32_t          m_bufout;

	/* per-channel parameters */
	int16_t           m_volume;
	uint8_t           m_enabled;
	double          m_frequency;
};

DECLARE_DEVICE_TYPE(DMADAC, dmadac_sound_device)


void dmadac_transfer(dmadac_sound_device **devlist, uint8_t num_channels, offs_t channel_spacing, offs_t frame_spacing, offs_t total_frames, int16_t *data);
void dmadac_enable(dmadac_sound_device **devlist, uint8_t num_channels, uint8_t enable);
void dmadac_set_frequency(dmadac_sound_device **devlist, uint8_t num_channels, double frequency);
void dmadac_set_volume(dmadac_sound_device **devlist, uint8_t num_channels, uint16_t volume);

#endif // MAME_SOUND_DMADAC_H
