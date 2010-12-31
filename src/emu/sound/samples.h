#pragma once

#ifndef __SAMPLES_H__
#define __SAMPLES_H__

#include "devlegcy.h"

typedef struct _loaded_sample loaded_sample;
struct _loaded_sample
{
    int         length;         /* length in samples */
    int         frequency;      /* frequency of the sample */
    INT16 *     data;           /* 16-bit signed data */
};

typedef struct _loaded_samples loaded_samples;
struct _loaded_samples
{
    int         total;          /* number of samples */
    loaded_sample sample[1]; /* array of samples */
};

typedef struct _samples_interface samples_interface;
struct _samples_interface
{
    int         channels;   /* number of discrete audio channels needed */
    const char *const *samplenames;
    void        (*start)(device_t *device);
};

#define SAMPLES_START(name) void name(device_t *device)


void sample_start(device_t *device,int channel,int samplenum,int loop);
void sample_start_raw(device_t *device,int channel,const INT16 *sampledata,int samples,int frequency,int loop);
void sample_set_freq(device_t *device,int channel,int freq);
void sample_set_volume(device_t *device,int channel,float volume);
void sample_set_pause(device_t *device,int channel,int pause);
void sample_stop(device_t *device,int channel);
int sample_get_base_freq(device_t *device,int channel);
int sample_playing(device_t *device,int channel);

/* helper function that reads samples from disk - this can be used by other */
/* drivers as well (e.g. a sound chip emulator needing drum samples) */
loaded_samples *readsamples(running_machine *machine, const char *const *samplenames, const char *name);

DECLARE_LEGACY_SOUND_DEVICE(SAMPLES, samples);

#endif /* __SAMPLES_H__ */
