#ifndef SAMPLES_H
#define SAMPLES_H

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

struct Samplesinterface
{
    int         channels;   /* number of discrete audio channels needed */
	const char **samplenames;
    void        (*start)(void);
};


void sample_start_n(int num,int channel,int samplenum,int loop);
void sample_start_raw_n(int num,int channel,INT16 *sampledata,int samples,int frequency,int loop);
void sample_set_freq_n(int num,int channel,int freq);
void sample_set_volume_n(int num,int channel,float volume);
void sample_set_pause_n(int num,int channel,int pause);
void sample_stop_n(int num,int channel);
int sample_get_base_freq_n(int num,int channel);
int sample_playing_n(int num,int channel);
int sample_loaded_n(int num,int samplenum);

/* shortcuts for backwards compatibilty */
void sample_start(int channel,int samplenum,int loop);
void sample_start_raw(int channel,INT16 *sampledata,int samples,int frequency,int loop);
void sample_set_freq(int channel,int freq);
void sample_set_volume(int channel,float volume);
void sample_set_pause(int channel,int pause);
void sample_stop(int channel);
int sample_get_base_freq(int channel);
int sample_playing(int channel);
int sample_loaded(int samplenum);

/* helper function that reads samples from disk - this can be used by other */
/* drivers as well (e.g. a sound chip emulator needing drum samples) */
struct loaded_samples *readsamples(const char **samplenames, const char *name);

#endif
