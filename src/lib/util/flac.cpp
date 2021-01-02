// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    flac.c

    FLAC compression wrappers

***************************************************************************/

#include "flac.h"

#include "osdcomm.h"

#include <cassert>
#include <new>


//**************************************************************************
//  FLAC ENCODER
//**************************************************************************

//-------------------------------------------------
//  flac_encoder - constructors
//-------------------------------------------------

flac_encoder::flac_encoder()
{
	init_common();
}


flac_encoder::flac_encoder(void *buffer, uint32_t buflength)
{
	init_common();
	reset(buffer, buflength);
}


flac_encoder::flac_encoder(util::core_file &file)
{
	init_common();
	reset(file);
}


//-------------------------------------------------
//  ~flac_encoder - destructor
//-------------------------------------------------

flac_encoder::~flac_encoder()
{
	// delete the encoder
	FLAC__stream_encoder_delete(m_encoder);
}


//-------------------------------------------------
//  reset - reset state with the original
//  parameters
//-------------------------------------------------

bool flac_encoder::reset()
{
	// configure the output
	m_compressed_offset = 0;
	m_ignore_bytes = m_strip_metadata ? 4 : 0;
	m_found_audio = !m_strip_metadata;

	// configure the encoder in a standard way
	// note we do this on each reset; if we don't, results are NOT consistent!
	FLAC__stream_encoder_set_verify(m_encoder, false);
//  FLAC__stream_encoder_set_do_md5(m_encoder, false);
	FLAC__stream_encoder_set_compression_level(m_encoder, 8);
	FLAC__stream_encoder_set_channels(m_encoder, m_channels);
	FLAC__stream_encoder_set_bits_per_sample(m_encoder, 16);
	FLAC__stream_encoder_set_sample_rate(m_encoder, m_sample_rate);
	FLAC__stream_encoder_set_total_samples_estimate(m_encoder, 0);
	FLAC__stream_encoder_set_streamable_subset(m_encoder, false);
	FLAC__stream_encoder_set_blocksize(m_encoder, m_block_size);

	// re-start processing
	return (FLAC__stream_encoder_init_stream(m_encoder, write_callback_static, nullptr, nullptr, nullptr, this) == FLAC__STREAM_ENCODER_INIT_STATUS_OK);
}


//-------------------------------------------------
//  reset - reset state with new memory parameters
//-------------------------------------------------

bool flac_encoder::reset(void *buffer, uint32_t buflength)
{
	// configure the output
	m_compressed_start = reinterpret_cast<FLAC__byte *>(buffer);
	m_compressed_length = buflength;
	m_file = nullptr;
	return reset();
}


//-------------------------------------------------
//  reset - reset state with new file parameters
//-------------------------------------------------

bool flac_encoder::reset(util::core_file &file)
{
	// configure the output
	m_compressed_start = nullptr;
	m_compressed_length = 0;
	m_file = &file;
	return reset();
}


//-------------------------------------------------
//  encode_interleaved - encode a buffer with
//  interleaved samples
//-------------------------------------------------

bool flac_encoder::encode_interleaved(const int16_t *samples, uint32_t samples_per_channel, bool swap_endian)
{
	int shift = swap_endian ? 8 : 0;

	// loop over source samples
	int num_channels = FLAC__stream_encoder_get_channels(m_encoder);
	uint32_t srcindex = 0;
	while (samples_per_channel != 0)
	{
		// process in batches of 2k samples
		FLAC__int32 converted_buffer[2048];
		FLAC__int32 *dest = converted_buffer;
		uint32_t cur_samples = (std::min<size_t>)(ARRAY_LENGTH(converted_buffer) / num_channels, samples_per_channel);

		// convert a buffers' worth
		for (uint32_t sampnum = 0; sampnum < cur_samples; sampnum++)
			for (int channel = 0; channel < num_channels; channel++, srcindex++)
				*dest++ = int16_t((uint16_t(samples[srcindex]) << shift) | (uint16_t(samples[srcindex]) >> shift));

		// process this batch
		if (!FLAC__stream_encoder_process_interleaved(m_encoder, converted_buffer, cur_samples))
			return false;
		samples_per_channel -= cur_samples;
	}
	return true;
}


//-------------------------------------------------
//  encode - encode a buffer with individual
//  sample streams
//-------------------------------------------------

bool flac_encoder::encode(int16_t *const *samples, uint32_t samples_per_channel, bool swap_endian)
{
	int shift = swap_endian ? 8 : 0;

	// loop over source samples
	int num_channels = FLAC__stream_encoder_get_channels(m_encoder);
	uint32_t srcindex = 0;
	while (samples_per_channel != 0)
	{
		// process in batches of 2k samples
		FLAC__int32 converted_buffer[2048];
		FLAC__int32 *dest = converted_buffer;
		uint32_t cur_samples = (std::min<size_t>)(ARRAY_LENGTH(converted_buffer) / num_channels, samples_per_channel);

		// convert a buffers' worth
		for (uint32_t sampnum = 0; sampnum < cur_samples; sampnum++, srcindex++)
			for (int channel = 0; channel < num_channels; channel++)
				*dest++ = int16_t((uint16_t(samples[channel][srcindex]) << shift) | (uint16_t(samples[channel][srcindex]) >> shift));

		// process this batch
		if (!FLAC__stream_encoder_process_interleaved(m_encoder, converted_buffer, cur_samples))
			return false;
		samples_per_channel -= cur_samples;
	}
	return true;
}


//-------------------------------------------------
//  finish - complete encoding and flush the
//  stream
//-------------------------------------------------

uint32_t flac_encoder::finish()
{
	// process the data and return the amount written
	FLAC__stream_encoder_finish(m_encoder);
	return (m_file != nullptr) ? m_file->tell() : m_compressed_offset;
}


//-------------------------------------------------
//  init_common - common initialization
//-------------------------------------------------

void flac_encoder::init_common()
{
	// allocate the encoder
	m_encoder = FLAC__stream_encoder_new();
	if (m_encoder == nullptr)
		throw std::bad_alloc();

	// initialize default state
	m_file = nullptr;
	m_compressed_offset = 0;
	m_compressed_start = nullptr;
	m_compressed_length = 0;
	m_sample_rate = 44100;
	m_channels = 2;
	m_block_size = 0;
	m_strip_metadata = false;
	m_ignore_bytes = 0;
	m_found_audio = false;
}


//-------------------------------------------------
//  write_callback - handle writes to the
//  output stream
//-------------------------------------------------

FLAC__StreamEncoderWriteStatus flac_encoder::write_callback_static(const FLAC__StreamEncoder *encoder, const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned current_frame, void *client_data)
{
	return reinterpret_cast<flac_encoder *>(client_data)->write_callback(buffer, bytes, samples, current_frame);
}

FLAC__StreamEncoderWriteStatus flac_encoder::write_callback(const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned current_frame)
{
	// loop over output data
	size_t offset = 0;
	while (offset < bytes)
	{
		// if we're ignoring, continue to do so
		if (m_ignore_bytes != 0)
		{
			size_t ignore = std::min(bytes - offset, size_t(m_ignore_bytes));
			offset += ignore;
			m_ignore_bytes -= ignore;
		}

		// if we haven't hit the end of metadata, process a new piece
		else if (!m_found_audio)
		{
			assert(bytes - offset >= 4);
			m_found_audio = ((buffer[offset] & 0x80) != 0);
			m_ignore_bytes = (buffer[offset + 1] << 16) | (buffer[offset + 2] << 8) | buffer[offset + 3];
			offset += 4;
		}

		// otherwise process as audio data and copy to the output
		else
		{
			int count = bytes - offset;
			if (m_file != nullptr)
				m_file->write(buffer, count);
			else
			{
				if (m_compressed_offset + count <= m_compressed_length)
					memcpy(m_compressed_start + m_compressed_offset, buffer, count);
				m_compressed_offset += count;
			}
			offset += count;
		}
	}
	return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
}



//**************************************************************************
//  FLAC DECODER
//**************************************************************************

//-------------------------------------------------
//  flac_decoder - constructor
//-------------------------------------------------

flac_decoder::flac_decoder()
	: m_decoder(FLAC__stream_decoder_new()),
		m_file(nullptr),
		m_sample_rate(0),
		m_channels(0),
		m_bits_per_sample(0),
		m_compressed_offset(0),
		m_compressed_start(nullptr),
		m_compressed_length(0),
		m_compressed2_start(nullptr),
		m_compressed2_length(0),
		m_uncompressed_offset(0),
		m_uncompressed_length(0),
		m_uncompressed_swap(false)
{
}


//-------------------------------------------------
//  flac_decoder - constructor
//-------------------------------------------------

flac_decoder::flac_decoder(const void *buffer, uint32_t length, const void *buffer2, uint32_t length2)
	: m_decoder(FLAC__stream_decoder_new()),
		m_file(nullptr),
		m_compressed_offset(0),
		m_compressed_start(reinterpret_cast<const FLAC__byte *>(buffer)),
		m_compressed_length(length),
		m_compressed2_start(reinterpret_cast<const FLAC__byte *>(buffer2)),
		m_compressed2_length(length2)
{
	reset();
}


//-------------------------------------------------
//  flac_decoder - constructor
//-------------------------------------------------

flac_decoder::flac_decoder(util::core_file &file)
	: m_decoder(FLAC__stream_decoder_new()),
		m_file(&file),
		m_compressed_offset(0),
		m_compressed_start(nullptr),
		m_compressed_length(0),
		m_compressed2_start(nullptr),
		m_compressed2_length(0)
{
	reset();
}


//-------------------------------------------------
//  flac_decoder - destructor
//-------------------------------------------------

flac_decoder::~flac_decoder()
{
	FLAC__stream_decoder_delete(m_decoder);
}


//-------------------------------------------------
//  reset - reset state with the original
//  parameters
//-------------------------------------------------

bool flac_decoder::reset()
{
	m_compressed_offset = 0;
	if (FLAC__stream_decoder_init_stream(m_decoder,
				&flac_decoder::read_callback_static,
				nullptr,
				&flac_decoder::tell_callback_static,
				nullptr,
				nullptr,
				&flac_decoder::write_callback_static,
				&flac_decoder::metadata_callback_static,
				&flac_decoder::error_callback_static, this) != FLAC__STREAM_DECODER_INIT_STATUS_OK)
		return false;
	return FLAC__stream_decoder_process_until_end_of_metadata(m_decoder);
}


//-------------------------------------------------
//  reset - reset state with new memory parameters
//-------------------------------------------------

bool flac_decoder::reset(const void *buffer, uint32_t length, const void *buffer2, uint32_t length2)
{
	m_file = nullptr;
	m_compressed_start = reinterpret_cast<const FLAC__byte *>(buffer);
	m_compressed_length = length;
	m_compressed2_start = reinterpret_cast<const FLAC__byte *>(buffer2);
	m_compressed2_length = length2;
	return reset();
}


//-------------------------------------------------
//  reset - reset state with new memory parameters
//  and a custom-generated header
//-------------------------------------------------

bool flac_decoder::reset(uint32_t sample_rate, uint8_t num_channels, uint32_t block_size, const void *buffer, uint32_t length)
{
	// modify the template header with our parameters
	static const uint8_t s_header_template[0x2a] =
	{
		0x66, 0x4C, 0x61, 0x43,                         // +00: 'fLaC' stream header
		0x80,                                           // +04: metadata block type 0 (STREAMINFO),
														//      flagged as last block
		0x00, 0x00, 0x22,                               // +05: metadata block length = 0x22
		0x00, 0x00,                                     // +08: minimum block size
		0x00, 0x00,                                     // +0A: maximum block size
		0x00, 0x00, 0x00,                               // +0C: minimum frame size (0 == unknown)
		0x00, 0x00, 0x00,                               // +0F: maximum frame size (0 == unknown)
		0x0A, 0xC4, 0x42, 0xF0, 0x00, 0x00, 0x00, 0x00, // +12: sample rate (0x0ac44 == 44100),
														//      numchannels (2), sample bits (16),
														//      samples in stream (0 == unknown)
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // +1A: MD5 signature (0 == none)
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  //
														// +2A: start of stream data
	};
	memcpy(m_custom_header, s_header_template, sizeof(s_header_template));
	m_custom_header[0x08] = m_custom_header[0x0a] = block_size >> 8;
	m_custom_header[0x09] = m_custom_header[0x0b] = block_size & 0xff;
	m_custom_header[0x12] = sample_rate >> 12;
	m_custom_header[0x13] = sample_rate >> 4;
	m_custom_header[0x14] = (sample_rate << 4) | ((num_channels - 1) << 1);

	// configure the header ahead of the provided buffer
	m_file = nullptr;
	m_compressed_start = reinterpret_cast<const FLAC__byte *>(m_custom_header);
	m_compressed_length = sizeof(m_custom_header);
	m_compressed2_start = reinterpret_cast<const FLAC__byte *>(buffer);
	m_compressed2_length = length;
	return reset();
}


//-------------------------------------------------
//  reset - reset state with new file parameter
//-------------------------------------------------

bool flac_decoder::reset(util::core_file &file)
{
	m_file = &file;
	m_compressed_start = nullptr;
	m_compressed_length = 0;
	m_compressed2_start = nullptr;
	m_compressed2_length = 0;
	return reset();
}


//-------------------------------------------------
//  decode_interleaved - decode to an interleaved
//  sound stream
//-------------------------------------------------

bool flac_decoder::decode_interleaved(int16_t *samples, uint32_t num_samples, bool swap_endian)
{
	// configure the uncompressed buffer
	memset(m_uncompressed_start, 0, sizeof(m_uncompressed_start));
	m_uncompressed_start[0] = samples;
	m_uncompressed_offset = 0;
	m_uncompressed_length = num_samples;
	m_uncompressed_swap = swap_endian;

	// loop until we get everything we want
	while (m_uncompressed_offset < m_uncompressed_length)
		if (!FLAC__stream_decoder_process_single(m_decoder))
			return false;
	return true;
}


//-------------------------------------------------
//  decode - decode to an multiple independent
//  data streams
//-------------------------------------------------

bool flac_decoder::decode(int16_t **samples, uint32_t num_samples, bool swap_endian)
{
	// make sure we don't have too many channels
	int chans = channels();
	if (chans > ARRAY_LENGTH(m_uncompressed_start))
		return false;

	// configure the uncompressed buffer
	memset(m_uncompressed_start, 0, sizeof(m_uncompressed_start));
	for (int curchan = 0; curchan < chans; curchan++)
		m_uncompressed_start[curchan] = samples[curchan];
	m_uncompressed_offset = 0;
	m_uncompressed_length = num_samples;
	m_uncompressed_swap = swap_endian;

	// loop until we get everything we want
	while (m_uncompressed_offset < m_uncompressed_length)
		if (!FLAC__stream_decoder_process_single(m_decoder))
			return false;
	return true;
}


//-------------------------------------------------
//  finish - finish up the decode
//-------------------------------------------------

uint32_t flac_decoder::finish()
{
	// get the final decoding position and move forward
	FLAC__uint64 position = 0;
	FLAC__stream_decoder_get_decode_position(m_decoder, &position);
	FLAC__stream_decoder_finish(m_decoder);

	// adjust position if we provided the header
	if (position == 0)
		return 0;
	if (m_compressed_start == reinterpret_cast<const FLAC__byte *>(m_custom_header))
		position -= m_compressed_length;
	return position;
}


//-------------------------------------------------
//  read_callback - handle reads from the input
//  stream
//-------------------------------------------------

FLAC__StreamDecoderReadStatus flac_decoder::read_callback_static(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data)
{
	return reinterpret_cast<flac_decoder *>(client_data)->read_callback(buffer, bytes);
}

FLAC__StreamDecoderReadStatus flac_decoder::read_callback(FLAC__byte buffer[], size_t *bytes)
{
	uint32_t expected = *bytes;

	// if a file, just read
	if (m_file != nullptr)
		*bytes = m_file->read(buffer, expected);

	// otherwise, copy from memory
	else
	{
		// copy from primary buffer first
		uint32_t outputpos = 0;
		if (outputpos < *bytes && m_compressed_offset < m_compressed_length)
		{
			uint32_t bytes_to_copy = (std::min<size_t>)(*bytes - outputpos, m_compressed_length - m_compressed_offset);
			memcpy(&buffer[outputpos], m_compressed_start + m_compressed_offset, bytes_to_copy);
			outputpos += bytes_to_copy;
			m_compressed_offset += bytes_to_copy;
		}

		// once we're out of that, copy from the secondary buffer
		if (outputpos < *bytes && m_compressed_offset < m_compressed_length + m_compressed2_length)
		{
			uint32_t bytes_to_copy = (std::min<size_t>)(*bytes - outputpos, m_compressed2_length - (m_compressed_offset - m_compressed_length));
			memcpy(&buffer[outputpos], m_compressed2_start + m_compressed_offset - m_compressed_length, bytes_to_copy);
			outputpos += bytes_to_copy;
			m_compressed_offset += bytes_to_copy;
		}
		*bytes = outputpos;
	}

	// return based on whether we ran out of data
	return (*bytes < expected) ? FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM : FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}


//-------------------------------------------------
//  metadata_callback - handle STREAMINFO metadata
//-------------------------------------------------

void flac_decoder::metadata_callback_static(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
	// ignore all but STREAMINFO metadata
	if (metadata->type != FLAC__METADATA_TYPE_STREAMINFO)
		return;

	// parse out the data we care about
	auto *fldecoder = reinterpret_cast<flac_decoder *>(client_data);
	fldecoder->m_sample_rate = metadata->data.stream_info.sample_rate;
	fldecoder->m_bits_per_sample = metadata->data.stream_info.bits_per_sample;
	fldecoder->m_channels = metadata->data.stream_info.channels;
}


//-------------------------------------------------
//  tell_callback - handle requests to find out
//  where in the input stream we are
//-------------------------------------------------

FLAC__StreamDecoderTellStatus flac_decoder::tell_callback_static(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data)
{
	*absolute_byte_offset = reinterpret_cast<flac_decoder *>(client_data)->m_compressed_offset;
	return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}


//-------------------------------------------------
//  write_callback - handle writes to the output
//  stream
//-------------------------------------------------

FLAC__StreamDecoderWriteStatus flac_decoder::write_callback_static(const FLAC__StreamDecoder *decoder, const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
{
	return reinterpret_cast<flac_decoder *>(client_data)->write_callback(frame, buffer);
}

FLAC__StreamDecoderWriteStatus flac_decoder::write_callback(const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[])
{
	assert(frame->header.channels == channels());

	// interleaved case
	int shift = m_uncompressed_swap ? 8 : 0;
	int blocksize = frame->header.blocksize;
	if (m_uncompressed_start[1] == nullptr)
	{
		int16_t *dest = m_uncompressed_start[0] + m_uncompressed_offset * frame->header.channels;
		for (int sampnum = 0; sampnum < blocksize && m_uncompressed_offset < m_uncompressed_length; sampnum++, m_uncompressed_offset++)
			for (int chan = 0; chan < frame->header.channels; chan++)
				*dest++ = int16_t((uint16_t(buffer[chan][sampnum]) << shift) | (uint16_t(buffer[chan][sampnum]) >> shift));
	}

	// non-interleaved case
	else
	{
		for (int sampnum = 0; sampnum < blocksize && m_uncompressed_offset < m_uncompressed_length; sampnum++, m_uncompressed_offset++)
			for (int chan = 0; chan < frame->header.channels; chan++)
				if (m_uncompressed_start[chan] != nullptr)
					m_uncompressed_start[chan][m_uncompressed_offset] = int16_t((uint16_t(buffer[chan][sampnum]) << shift) | (uint16_t(buffer[chan][sampnum]) >> shift));
	}
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

/**
 * @fn  void flac_decoder::error_callback_static(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
 *
 * @brief   -------------------------------------------------
 *            error_callback - handle errors (ignore them)
 *          -------------------------------------------------.
 *
 * @param   decoder             The decoder.
 * @param   status              The status.
 * @param [in,out]  client_data If non-null, information describing the client.
 */

void flac_decoder::error_callback_static(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
}
