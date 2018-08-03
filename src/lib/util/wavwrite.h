// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_UTIL_WAVWRITE_H
#define MAME_UTIL_WAVWRITE_H

#pragma once

struct wav_file;

wav_file *wav_open(const char *filename, int sample_rate, int channels);
void wav_close(wav_file*wavptr);

void wav_add_data_16(wav_file *wavptr, int16_t *data, int samples);
void wav_add_data_32(wav_file *wavptr, int32_t *data, int samples, int shift);
void wav_add_data_16lr(wav_file *wavptr, int16_t *left, int16_t *right, int samples);
void wav_add_data_32lr(wav_file *wavptr, int32_t *left, int32_t *right, int samples, int shift);

#endif // MAME_UTIL_WAVWRITE_H
