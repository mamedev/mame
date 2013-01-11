/**********************************************************************************************
 *
 *   DMA-driven DAC driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#pragma once

#ifndef __DMADAC_H__
#define __DMADAC_H__

#include "devlegcy.h"

class dmadac_sound_device : public device_t,
									public device_sound_interface
{
public:
	dmadac_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~dmadac_sound_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type DMADAC;


void dmadac_transfer(dmadac_sound_device **devlist, UINT8 num_channels, offs_t channel_spacing, offs_t frame_spacing, offs_t total_frames, INT16 *data);
void dmadac_enable(dmadac_sound_device **devlist, UINT8 num_channels, UINT8 enable);
void dmadac_set_frequency(dmadac_sound_device **devlist, UINT8 num_channels, double frequency);
void dmadac_set_volume(dmadac_sound_device **devlist, UINT8 num_channels, UINT16 volume);

#endif /* __DMADAC_H__ */
