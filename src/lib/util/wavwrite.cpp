// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#include "wavwrite.h"

#include "osdcomm.h"

#include <cstdio>
#include <new>
#include <string>
#include <vector>


namespace util {

struct wav_file
{
	FILE *file = nullptr;
	std::vector<std::int16_t> temp;
	std::uint32_t total_offs = 0U;
	std::uint32_t data_offs = 0U;
};


wav_file_ptr wav_open(std::string_view filename, int sample_rate, int channels)
{
	std::uint32_t temp32;
	std::uint16_t temp16;

	// allocate memory for the wav struct
	wav_file_ptr wav(new (std::nothrow) wav_file);
	if (!wav)
		return nullptr;

	// create the file
	wav->file = std::fopen(std::string(filename).c_str(), "wb"); // ugly - need to force NUL termination on filename
	if (!wav->file)
		return nullptr;

	// write the 'RIFF' header
	std::fwrite("RIFF", 1, 4, wav->file);

	// write the total size
	temp32 = 0;
	wav->total_offs = std::ftell(wav->file);
	std::fwrite(&temp32, 1, 4, wav->file);

	// write the 'WAVE' type
	std::fwrite("WAVE", 1, 4, wav->file);

	// write the 'fmt ' tag
	std::fwrite("fmt ", 1, 4, wav->file);

	// write the format length
	temp32 = little_endianize_int32(16);
	std::fwrite(&temp32, 1, 4, wav->file);

	// write the format (PCM)
	temp16 = little_endianize_int16(1);
	std::fwrite(&temp16, 1, 2, wav->file);

	// write the channels
	temp16 = little_endianize_int16(channels);
	std::fwrite(&temp16, 1, 2, wav->file);

	// write the sample rate
	temp32 = little_endianize_int32(sample_rate);
	std::fwrite(&temp32, 1, 4, wav->file);

	// write the bytes/second
	std::uint32_t const bps = sample_rate * 2 * channels;
	temp32 = little_endianize_int32(bps);
	std::fwrite(&temp32, 1, 4, wav->file);

	// write the block align
	std::uint16_t const align = 2 * channels;
	temp16 = little_endianize_int16(align);
	std::fwrite(&temp16, 1, 2, wav->file);

	// write the bits/sample
	temp16 = little_endianize_int16(16);
	std::fwrite(&temp16, 1, 2, wav->file);

	// write the 'data' tag
	std::fwrite("data", 1, 4, wav->file);

	// write the data length
	temp32 = 0;
	wav->data_offs = std::ftell(wav->file);
	std::fwrite(&temp32, 1, 4, wav->file);

	return wav;
}


void wav_close(wav_file *wav)
{
	if (!wav)
		return;

	if (wav->file)
	{
		std::uint32_t temp32;
		std::uint32_t const total = std::ftell(wav->file);

		// update the total file size
		std::fseek(wav->file, wav->total_offs, SEEK_SET);
		temp32 = total - (wav->total_offs + 4);
		temp32 = little_endianize_int32(temp32);
		std::fwrite(&temp32, 1, 4, wav->file);

		// update the data size
		std::fseek(wav->file, wav->data_offs, SEEK_SET);
		temp32 = total - (wav->data_offs + 4);
		temp32 = little_endianize_int32(temp32);
		std::fwrite(&temp32, 1, 4, wav->file);

		std::fclose(wav->file);
	}

	delete wav;
}


void wav_add_data_16(wav_file &wav, int16_t *data, int samples)
{
	// just write and flush the data
	std::fwrite(data, 2, samples, wav.file);
	std::fflush(wav.file);
}


void wav_add_data_32(wav_file &wav, int32_t *data, int samples, int shift)
{
	if (!samples)
		return;

	// resize dynamic array - don't want it to copy if it needs to expand
	wav.temp.clear();
	wav.temp.resize(samples);

	// clamp
	for (int i = 0; i < samples; i++)
	{
		int val = data[i] >> shift;
		wav.temp[i] = (val < -32768) ? -32768 : (val > 32767) ? 32767 : val;
	}

	// write and flush
	std::fwrite(&wav.temp[0], 2, samples, wav.file);
	std::fflush(wav.file);
}


void wav_add_data_16lr(wav_file &wav, int16_t *left, int16_t *right, int samples)
{
	if (!samples)
		return;

	// resize dynamic array - don't want it to copy if it needs to expand
	wav.temp.clear();
	wav.temp.resize(samples * 2);

	// interleave
	for (int i = 0; i < samples * 2; i++)
		wav.temp[i] = (i & 1) ? right[i / 2] : left[i / 2];

	// write and flush
	std::fwrite(&wav.temp[0], 4, samples, wav.file);
	std::fflush(wav.file);
}


void wav_add_data_32lr(wav_file &wav, int32_t *left, int32_t *right, int samples, int shift)
{
	if (!samples)
		return;

	// resize dynamic array - don't want it to copy if it needs to expand
	wav.temp.clear();
	wav.temp.resize(samples * 2);

	// interleave
	for (int i = 0; i < samples * 2; i++)
	{
		int val = (i & 1) ? right[i / 2] : left[i / 2];
		val >>= shift;
		wav.temp[i] = (val < -32768) ? -32768 : (val > 32767) ? 32767 : val;
	}

	// write and flush
	std::fwrite(&wav.temp[0], 4, samples, wav.file);
	std::fflush(wav.file);
}

} // namespace util
