#pragma once

#ifndef __CDDA_H__
#define __CDDA_H__

#include "devlegcy.h"

void cdda_set_cdrom(device_t *device, void *file);
device_t *cdda_from_cdrom(running_machine &machine, void *file);

void cdda_start_audio(device_t *device, UINT32 startlba, UINT32 numblocks);
void cdda_stop_audio(device_t *device);
void cdda_pause_audio(device_t *device, int pause);
void cdda_set_volume(device_t *device, int volume);
void cdda_set_channel_volume(device_t *device, int channel, int volume);

UINT32 cdda_get_audio_lba(device_t *device);
int cdda_audio_active(device_t *device);
int cdda_audio_paused(device_t *device);
int cdda_audio_ended(device_t *device);

class cdda_device : public device_t,
                                  public device_sound_interface
{
public:
	cdda_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~cdda_device() { global_free(m_token); }

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

extern const device_type CDDA;


#endif /* __CDDA_H__ */
