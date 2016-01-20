// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/**********************************************************************************************
 *
 *   DMA-driven DAC driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#pragma once

#ifndef __DMADAC_H__
#define __DMADAC_H__


class dmadac_sound_device : public device_t,
									public device_sound_interface
{
public:
	dmadac_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	void flush();
	void transfer(int channel, offs_t channel_spacing, offs_t frame_spacing, offs_t total_frames, INT16 *data);
	void enable(UINT8 enable);
	void set_frequency(double frequency);
	void set_volume(UINT16 volume);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
private:
	// internal state
	/* sound stream and buffers */
	sound_stream *  m_channel;
	std::unique_ptr<INT16[]>         m_buffer;
	UINT32          m_bufin;
	UINT32          m_bufout;

	/* per-channel parameters */
	INT16           m_volume;
	UINT8           m_enabled;
	double          m_frequency;
};

extern const device_type DMADAC;


void dmadac_transfer(dmadac_sound_device **devlist, UINT8 num_channels, offs_t channel_spacing, offs_t frame_spacing, offs_t total_frames, INT16 *data);
void dmadac_enable(dmadac_sound_device **devlist, UINT8 num_channels, UINT8 enable);
void dmadac_set_frequency(dmadac_sound_device **devlist, UINT8 num_channels, double frequency);
void dmadac_set_volume(dmadac_sound_device **devlist, UINT8 num_channels, UINT16 volume);

#endif /* __DMADAC_H__ */
