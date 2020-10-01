// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    avhuff.h

    Audio/video compression and decompression helpers.

***************************************************************************/

#ifndef MAME_UTIL_AVHUFF_H
#define MAME_UTIL_AVHUFF_H

#pragma once

#include "osdcore.h"
#include "coretmpl.h"
#include "bitmap.h"
#include "huffman.h"
#include "flac.h"

#include <algorithm>


//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define AVHUFF_USE_FLAC     (1)


// errors
enum avhuff_error
{
	AVHERR_NONE = 0,
	AVHERR_INVALID_DATA,
	AVHERR_VIDEO_TOO_LARGE,
	AVHERR_AUDIO_TOO_LARGE,
	AVHERR_METADATA_TOO_LARGE,
	AVHERR_OUT_OF_MEMORY,
	AVHERR_COMPRESSION_ERROR,
	AVHERR_TOO_MANY_CHANNELS,
	AVHERR_INVALID_CONFIGURATION,
	AVHERR_INVALID_PARAMETER,
	AVHERR_BUFFER_TOO_SMALL
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> avhuff_encoder

// core state for the codec
class avhuff_encoder
{
public:
	// construction/destruction
	avhuff_encoder();

	// encode/decode
	avhuff_error encode_data(const uint8_t *source, uint8_t *dest, uint32_t &complength);

	// static helpers
	static uint32_t raw_data_size(const uint8_t *data);
	static uint32_t raw_data_size(uint32_t width, uint32_t height, uint8_t channels, uint32_t numsamples, uint32_t metadatasize = 0) { return 12 + channels * numsamples * 2 + width * height * 2; }
	static avhuff_error assemble_data(std::vector<uint8_t> &buffer, bitmap_yuy16 &bitmap, uint8_t channels, uint32_t numsamples, int16_t **samples, uint8_t *metadata = nullptr, uint32_t metadatasize = 0);

private:
	// delta-RLE Huffman encoder
	class deltarle_encoder
	{
	public:
		// construction/destruction
		deltarle_encoder() { }

		// histogramming
		uint16_t *rle_and_histo_bitmap(const uint8_t *source, uint32_t items_per_row, uint32_t item_advance, uint32_t row_count);

		// encoding
		void flush_rle() { m_rlecount = 0; }
		void encode_one(bitstream_out &bitbuf, uint16_t *&rleptr);
		huffman_error export_tree_rle(bitstream_out &bitbuf) { return m_encoder.export_tree_rle(bitbuf); }

	private:
		// internal state
		int                         m_rlecount = 0;
		huffman_encoder<256 + 16>   m_encoder;
		std::vector<uint16_t>       m_rlebuffer;
	};

	// internal helpers
	avhuff_error encode_audio(const uint8_t *source, int channels, int samples, uint8_t *dest, uint8_t *sizes);
	avhuff_error encode_video(const uint8_t *source, int width, int height, uint8_t *dest, uint32_t &complength);
	avhuff_error encode_video_lossless(const uint8_t *source, int width, int height, uint8_t *dest, uint32_t &complength);

	// video encoding contexts
	deltarle_encoder            m_ycontext;
	deltarle_encoder            m_cbcontext;
	deltarle_encoder            m_crcontext;

	// audio encoding contexts
	std::vector<uint8_t>              m_audiobuffer;
#if AVHUFF_USE_FLAC
	flac_encoder                m_flac_encoder;
#else
	huffman_8bit_encoder        m_audiohi_encoder;
	huffman_8bit_encoder        m_audiolo_encoder;
#endif
};


// ======================> avhuff_decoder

// core state for the codec
class avhuff_decoder
{
public:
	// decompression configuration
	class config
	{
	public:
		config()
		{
			std::fill(std::begin(audio), std::end(audio), nullptr);
		}

		bitmap_yuy16 *  video = nullptr;            // pointer to video bitmap
		uint32_t        maxsamples = 0;             // maximum number of samples per channel
		uint32_t *      actsamples = nullptr;       // actual number of samples per channel
		int16_t *       audio[16];                  // pointer to individual audio channels
		uint32_t        maxmetalength = 0;          // maximum length of metadata
		uint32_t *      actmetalength = nullptr;    // actual length of metadata
		uint8_t *       metadata = nullptr;         // pointer to metadata buffer
	};


	// construction/destruction
	avhuff_decoder();

	// configuration
	void configure(const config &cfg);

	// encode/decode
	avhuff_error decode_data(const uint8_t *source, uint32_t complength, uint8_t *dest);

private:
	// delta-RLE Huffman decoder
	class deltarle_decoder
	{
	public:
		// construction/destruction
		deltarle_decoder() { }

		// general
		void reset() { m_rlecount = m_prevdata = 0; }

		// decoding
		void flush_rle() { m_rlecount = 0; }
		uint32_t decode_one(bitstream_in &bitbuf);
		huffman_error import_tree_rle(bitstream_in &bitbuf) { return m_decoder.import_tree_rle(bitbuf); }

	private:
		// internal state
		int                         m_rlecount = 0;
		uint8_t                     m_prevdata = 0;
		huffman_decoder<256 + 16>   m_decoder;
	};


	// internal helpers
	avhuff_error decode_audio(int channels, int samples, const uint8_t *source, uint8_t **dest, uint32_t dxor, const uint8_t *sizes);
	avhuff_error decode_video(int width, int height, const uint8_t *source, uint32_t complength, uint8_t *dest, uint32_t dstride, uint32_t dxor);
	avhuff_error decode_video_lossless(int width, int height, const uint8_t *source, uint32_t complength, uint8_t *dest, uint32_t dstride, uint32_t dxor);

	// internal state
	bitmap_yuy16                m_video;
	config                      m_config;

	// video decoding contexts
	deltarle_decoder            m_ycontext;
	deltarle_decoder            m_cbcontext;
	deltarle_decoder            m_crcontext;

	// audio decoding contexts
	huffman_8bit_decoder        m_audiohi_decoder;
	huffman_8bit_decoder        m_audiolo_decoder;
#if AVHUFF_USE_FLAC
	flac_decoder                m_flac_decoder;
#endif
};

#endif // MAME_UTIL_AVHUFF_H
