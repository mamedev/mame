#pragma once

#ifndef __SAMPLES_H__
#define __SAMPLES_H__

struct loaded_sample
{
    int         length;         /* length in samples */
    int         frequency;      /* frequency of the sample */
    INT16 *     data;           /* 16-bit signed data */
};

struct loaded_samples
{
    int         total;          /* number of samples */
    struct loaded_sample sample[1]; /* array of samples */
};

typedef struct _samples_interface samples_interface;
struct _samples_interface
{
    int         channels;   /* number of discrete audio channels needed */
    const char *const *samplenames;
    void        (*start)(const device_config *device);
};

#define SAMPLES_START(name) void name(const device_config *device)


void sample_start(const device_config *device,int channel,int samplenum,int loop);
void sample_start_raw(const device_config *device,int channel,const INT16 *sampledata,int samples,int frequency,int loop);
void sample_set_freq(const device_config *device,int channel,int freq);
void sample_set_volume(const device_config *device,int channel,float volume);
void sample_set_pause(const device_config *device,int channel,int pause);
void sample_stop(const device_config *device,int channel);
int sample_get_base_freq(const device_config *device,int channel);
int sample_playing(const device_config *device,int channel);

/* helper function that reads samples from disk - this can be used by other */
/* drivers as well (e.g. a sound chip emulator needing drum samples) */
struct loaded_samples *readsamples(const char *const *samplenames, const char *name);

DEVICE_GET_INFO( samples );
#define SOUND_SAMPLES DEVICE_GET_INFO_NAME( samples )

#endif /* __SAMPLES_H__ */
