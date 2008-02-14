#ifndef WAVWRITE_H
#define WAVWRITE_H

typedef struct _wav_file wav_file;

wav_file *wav_open(const char *filename, int sample_rate, int channels);
void wav_close(wav_file*wavptr);

void wav_add_data_16(wav_file *wavptr, INT16 *data, int samples);
void wav_add_data_32(wav_file *wavptr, INT32 *data, int samples, int shift);
void wav_add_data_16lr(wav_file *wavptr, INT16 *left, INT16 *right, int samples);
void wav_add_data_32lr(wav_file *wavptr, INT32 *left, INT32 *right, int samples, int shift);

#endif /* WAVWRITE_H */
