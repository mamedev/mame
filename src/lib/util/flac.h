// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    flac.h

    FLAC compression wrappers

***************************************************************************/

#pragma once

#ifndef __FLAC_H__
#define __FLAC_H__

#include "osdcore.h"
#include "corefile.h"

#ifdef FLAC__NO_DLL
#include "libflac/include/FLAC/all.h"
#else
#include <FLAC/all.h>
#endif


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> flac_encoder

class flac_encoder
{
public:
	// construction/destruction
	flac_encoder();
	flac_encoder(void *buffer, UINT32 buflength);
	flac_encoder(core_file &file);
	~flac_encoder();

	// configuration
	void set_sample_rate(UINT32 sample_rate) { m_sample_rate = sample_rate; }
	void set_num_channels(UINT8 num_channels) { m_channels = num_channels; }
	void set_block_size(UINT32 block_size) { m_block_size = block_size; }
	void set_strip_metadata(bool strip) { m_strip_metadata = strip; }

	// getters (valid after reset)
	FLAC__StreamEncoderState state() const { return FLAC__stream_encoder_get_state(m_encoder); }
	const char *state_string() const { return FLAC__stream_encoder_get_resolved_state_string(m_encoder); }

	// reset
	bool reset();
	bool reset(void *buffer, UINT32 buflength);
	bool reset(core_file &file);

	// encode a buffer
	bool encode_interleaved(const INT16 *samples, UINT32 samples_per_channel, bool swap_endian = false);
	bool encode(INT16 *const *samples, UINT32 samples_per_channel, bool swap_endian = false);

	// finish up
	UINT32 finish();

private:
	// internal helpers
	void init_common();
	static FLAC__StreamEncoderWriteStatus write_callback_static(const FLAC__StreamEncoder *encoder, const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned current_frame, void *client_data);
	FLAC__StreamEncoderWriteStatus write_callback(const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned current_frame);

	// internal state
	FLAC__StreamEncoder *   m_encoder;              // actual encoder
	core_file *             m_file;                 // output file
	UINT32                  m_compressed_offset;    // current offset with the compressed stream
	FLAC__byte *            m_compressed_start;     // start of compressed data
	UINT32                  m_compressed_length;    // length of the compressed stream

	// parameters
	UINT32                  m_sample_rate;          // sample rate
	UINT8                   m_channels;             // number of channels
	UINT32                  m_block_size;           // block size

	// header stripping
	bool                    m_strip_metadata;       // strip the metadata?
	UINT32                  m_ignore_bytes;         // how many bytes to ignore when writing
	bool                    m_found_audio;          // have we hit the audio yet?
};


// ======================> flac_decoder

class flac_decoder
{
public:
	// construction/destruction
	flac_decoder();
	flac_decoder(const void *buffer, UINT32 length, const void *buffer2 = nullptr, UINT32 length2 = 0);
	flac_decoder(core_file &file);
	~flac_decoder();

	// getters (valid after reset)
	UINT32 sample_rate() const { return m_sample_rate; }
	UINT8 channels() const { return m_channels; }
	UINT8 bits_per_sample() const { return m_bits_per_sample; }
	UINT32 total_samples() const { return FLAC__stream_decoder_get_total_samples(m_decoder); }
	FLAC__StreamDecoderState state() const { return FLAC__stream_decoder_get_state(m_decoder); }
	const char *state_string() const { return FLAC__stream_decoder_get_resolved_state_string(m_decoder); }

	// reset
	bool reset();
	bool reset(const void *buffer, UINT32 length, const void *buffer2 = nullptr, UINT32 length2 = 0);
	bool reset(UINT32 sample_rate, UINT8 num_channels, UINT32 block_size, const void *buffer, UINT32 length);
	bool reset(core_file &file);

	// decode to a buffer; num_samples must be a multiple of the block size
	bool decode_interleaved(INT16 *samples, UINT32 num_samples, bool swap_endian = false);
	bool decode(INT16 **samples, UINT32 num_samples, bool swap_endian = false);

	// finish up
	UINT32 finish();

private:
	// internal helpers
	static FLAC__StreamDecoderReadStatus read_callback_static(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data);
	FLAC__StreamDecoderReadStatus read_callback(FLAC__byte buffer[], size_t *bytes);
	static void metadata_callback_static(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data);
	static FLAC__StreamDecoderTellStatus tell_callback_static(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data);
	static FLAC__StreamDecoderWriteStatus write_callback_static(const FLAC__StreamDecoder *decoder, const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data);
	FLAC__StreamDecoderWriteStatus write_callback(const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[]);
	static void error_callback_static(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data);

	// output state
	FLAC__StreamDecoder *   m_decoder;              // actual encoder
	core_file *             m_file;                 // output file
	UINT32                  m_sample_rate;          // decoded sample rate
	UINT8                   m_channels;             // decoded number of channels
	UINT8                   m_bits_per_sample;      // decoded bits per sample
	UINT32                  m_compressed_offset;    // current offset in compressed data
	const FLAC__byte *      m_compressed_start;     // start of compressed data
	UINT32                  m_compressed_length;    // length of compressed data
	const FLAC__byte *      m_compressed2_start;    // start of compressed data
	UINT32                  m_compressed2_length;   // length of compressed data
	INT16 *                 m_uncompressed_start[8];// pointer to start of uncompressed data (up to 8 streams)
	UINT32                  m_uncompressed_offset;  // current position in uncompressed data
	UINT32                  m_uncompressed_length;  // length of uncompressed data
	bool                    m_uncompressed_swap;    // swap uncompressed sample data
	UINT8                   m_custom_header[0x2a];  // custom header
};


#endif // __FLAC_H__
