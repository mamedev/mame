// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_UTIL_WAVWRITE_H
#define MAME_UTIL_WAVWRITE_H

#pragma once

#include <cstdint>
#include <memory>
#include <string_view>


namespace util {

struct wav_file;

void wav_close(wav_file *wavptr);

struct wav_deleter
{
	void operator()(wav_file *wavptr) const
	{
		if (wavptr)
			wav_close(wavptr);
	}
};

using wav_file_ptr = std::unique_ptr<wav_file, wav_deleter>;

wav_file_ptr wav_open(std::string_view filename, int sample_rate, int channels);

void wav_add_data_16(wav_file &wavptr, std::int16_t *data, int samples);
void wav_add_data_32(wav_file &wavptr, std::int32_t *data, int samples, int shift);
void wav_add_data_16lr(wav_file &wavptr, std::int16_t *left, std::int16_t *right, int samples);
void wav_add_data_32lr(wav_file &wavptr, std::int32_t *left, std::int32_t *right, int samples, int shift);

} // namespace util

#endif // MAME_UTIL_WAVWRITE_H
