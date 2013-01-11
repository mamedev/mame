#pragma once

#ifndef __SOCR_SND_H__
#define __SOCR_SND_H__

void socrates_snd_reg0_w(device_t *device, int data);
void socrates_snd_reg1_w(device_t *device, int data);
void socrates_snd_reg2_w(device_t *device, int data);
void socrates_snd_reg3_w(device_t *device, int data);
void socrates_snd_reg4_w(device_t *device, int data);

class socrates_snd_device : public device_t,
									public device_sound_interface
{
public:
	socrates_snd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~socrates_snd_device() { global_free(m_token); }

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

extern const device_type SOCRATES;


#endif /* __SOCR_SND_H__ */
