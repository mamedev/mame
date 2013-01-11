#pragma once

#ifndef __MSM5205_H__
#define __MSM5205_H__

#include "devlegcy.h"

/* an interface for the MSM5205 and similar chips */

/* prescaler selector defines   */
/* MSM5205 default master clock is 384KHz */
#define MSM5205_S96_3B 0     /* prescaler 1/96(4KHz) , data 3bit */
#define MSM5205_S48_3B 1     /* prescaler 1/48(8KHz) , data 3bit */
#define MSM5205_S64_3B 2     /* prescaler 1/64(6KHz) , data 3bit */
#define MSM5205_SEX_3B 3     /* VCLK slave mode      , data 3bit */
#define MSM5205_S96_4B 4     /* prescaler 1/96(4KHz) , data 4bit */
#define MSM5205_S48_4B 5     /* prescaler 1/48(8KHz) , data 4bit */
#define MSM5205_S64_4B 6     /* prescaler 1/64(6KHz) , data 4bit */
#define MSM5205_SEX_4B 7     /* VCLK slave mode      , data 4bit */

/* MSM6585 default master clock is 640KHz */
#define MSM6585_S160  (4+8)  /* prescaler 1/160(4KHz), data 4bit */
#define MSM6585_S40   (5+8)  /* prescaler 1/40(16KHz), data 4bit */
#define MSM6585_S80   (6+8)  /* prescaler 1/80 (8KHz), data 4bit */
#define MSM6585_S20   (7+8)  /* prescaler 1/20(32KHz), data 4bit */

struct msm5205_interface
{
	void (*vclk_callback)(device_t *);   /* VCLK callback              */
	int select;       /* prescaler / bit width selector        */
};

/* reset signal should keep for 2cycle of VCLK      */
void msm5205_reset_w (device_t *device, int reset);
/* adpcmata is latched after vclk_interrupt callback */
void msm5205_data_w (device_t *device, int data);
/* VCLK slave mode option                                        */
/* if VCLK and reset or data is changed at the same time,        */
/* Call msm5205_vclk_w after msm5205_data_w and msm5205_reset_w. */
void msm5205_vclk_w (device_t *device, int reset);
/* option , selected pin seletor */
void msm5205_playmode_w(device_t *device, int _select);

void msm5205_set_volume(device_t *device,int volume);

void msm5205_change_clock_w(device_t *device, INT32 clock);

class msm5205_device : public device_t,
									public device_sound_interface
{
public:
	msm5205_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	msm5205_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~msm5205_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type MSM5205;

class msm6585_device : public msm5205_device
{
public:
	msm6585_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type MSM6585;


#endif /* __MSM5205_H__ */
