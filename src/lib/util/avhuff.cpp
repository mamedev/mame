// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    avhuff.c

    Audio/video compression and decompression helpers.

****************************************************************************

    Each frame is compressed as a unit. The raw data is of the form:
    (all multibyte values are stored in big-endian format)

        +00 = 'chav' (4 bytes) - fixed header data to identify the format
        +04 = metasize (1 byte) - size of metadata in bytes (max=255 bytes)
        +05 = channels (1 byte) - number of audio channels
        +06 = samples (2 bytes) - number of samples per audio stream
        +08 = width (2 bytes) - width of video data
        +0A = height (2 bytes) - height of video data
        +0C = <metadata> - as raw bytes
              <audio stream 0> - as signed 16-bit samples
              <audio stream 1> - as signed 16-bit samples
              ...
              <video data> - as a raw array of 8-bit YUY data in (Cb,Y,Cr,Y) order

    When compressed, the data is stored as follows:
    (all multibyte values are stored in big-endian format)

        +00 = metasize (1 byte) - size of metadata in bytes
        +01 = channels (1 byte) - number of audio channels
        +02 = samples (2 bytes) - number of samples per audio stream
        +04 = width (2 bytes) - width of video data
        +06 = height (2 bytes) - height of video data
        +08 = audio huffman size (2 bytes) - size of audio huffman tables
                (0x0000 => uncompressed deltas are used)
        +0A = str0size (2 bytes) - compressed size of stream 0
        +0C = str1size (2 bytes) - compressed size of stream 1
              ...
              <metadata> - as raw data
              <audio huffman table> - Huffman table for audio decoding
              <audio stream 0 data> - Huffman-compressed deltas
              <audio stream 1 data> - Huffman-compressed deltas
              <...>
              <video huffman tables> - Huffman tables for video decoding
              <video data> - compressed data

****************************************************************************

    Attempted techniques that have not been worthwhile:

    * Attempted to use integer DCTs from the IJG code; even the "slow"
      variants produce a lot of error and thus kill our compression ratio,
      since our compression is based on error not bitrate.

    * Tried various other predictors for the lossless video encoding, but
      none tended to give any significant gain over predicting the
      previous pixel.

***************************************************************************/

#include <assert.h>

#include "avhuff.h"
#include "huffman.h"
#include "chd.h"

#include <math.h>
#include <stdlib.h>
#include <new>



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  code_to_rlecount - number of RLE repetitions
//  encoded in a given byte
//-------------------------------------------------

inline int code_to_rlecount(int code)
{
	if (code == 0x00)
		return 1;
	if (code <= 0x107)
		return 8 + (code - 0x100);
	return 16 << (code - 0x108);
}


//-------------------------------------------------
//  rlecount_to_byte - return a byte encoding
//  the maximum RLE count less than or equal to
//  the provided amount
//-------------------------------------------------

inline int rlecount_to_code(int rlecount)
{
	if (rlecount >= 2048)
		return 0x10f;
	if (rlecount >= 1024)
		return 0x10e;
	if (rlecount >= 512)
		return 0x10d;
	if (rlecount >= 256)
		return 0x10c;
	if (rlecount >= 128)
		return 0x10b;
	if (rlecount >= 64)
		return 0x10a;
	if (rlecount >= 32)
		return 0x109;
	if (rlecount >= 16)
		return 0x108;
	if (rlecount >= 8)
		return 0x100 + (rlecount - 8);
	return 0x00;
}


//-------------------------------------------------
//  encode_one - encode data
//-------------------------------------------------

inline void avhuff_encoder::deltarle_encoder::encode_one(bitstream_out &bitbuf, UINT16 *&rleptr)
{
	// return RLE data if we still have some
	if (m_rlecount != 0)
	{
		m_rlecount--;
		return;
	}

	// fetch the data and process
	UINT16 data = *rleptr++;
	m_encoder.encode_one(bitbuf, data);
	if (data >= 0x100)
		m_rlecount = code_to_rlecount(data) - 1;
}


//-------------------------------------------------
//  decode_one - decode data
//-------------------------------------------------
inline UINT32 avhuff_decoder::deltarle_decoder::decode_one(bitstream_in &bitbuf)
{
	// return RLE data if we still have some
	if (m_rlecount != 0)
	{
		m_rlecount--;
		return m_prevdata;
	}

	// fetch the data and process
	int data = m_decoder.decode_one(bitbuf);
	if (data < 0x100)
	{
		m_prevdata += UINT8(data);
		return m_prevdata;
	}
	else
	{
		m_rlecount = code_to_rlecount(data);
		m_rlecount--;
		return m_prevdata;
	}
}



//**************************************************************************
//  AVHUFF ENCODER
//**************************************************************************

/**
 * @fn  avhuff_encoder::avhuff_encoder()
 *
 * @brief   -------------------------------------------------
 *            avhuff_encoder - constructor
 *          -------------------------------------------------.
 */

avhuff_encoder::avhuff_encoder()
{
m_flac_encoder.set_sample_rate(48000);
m_flac_encoder.set_num_channels(1);
m_flac_encoder.set_strip_metadata(true);
}

/**
 * @fn  avhuff_error avhuff_encoder::encode_data(const UINT8 *source, UINT8 *dest, UINT32 &complength)
 *
 * @brief   -------------------------------------------------
 *            encode_data - encode a block of data into a compressed data stream
 *          -------------------------------------------------.
 *
 * @param   source              Source for the.
 * @param [in,out]  dest        If non-null, destination for the.
 * @param [in,out]  complength  The complength.
 *
 * @return  An avhuff_error.
 */

avhuff_error avhuff_encoder::encode_data(const UINT8 *source, UINT8 *dest, UINT32 &complength)
{
	// validate the header
	if (source[0] != 'c' || source[1] != 'h' || source[2] != 'a' || source[3] != 'v')
		return AVHERR_INVALID_DATA;

	// extract info from the header
	UINT32 metasize = source[4];
	UINT32 channels = source[5];
	UINT32 samples = (source[6] << 8) + source[7];
	UINT32 width = (source[8] << 8) + source[9];
	UINT32 height = (source[10] << 8) + source[11];
	source += 12;

	// write the basics to the new header
	dest[0] = metasize;
	dest[1] = channels;
	dest[2] = samples >> 8;
	dest[3] = samples;
	dest[4] = width >> 8;
	dest[5] = width;
	dest[6] = height >> 8;
	dest[7] = height;

	// starting offsets
	UINT32 dstoffs = 10 + 2 * channels;

	// copy the metadata first
	if (metasize > 0)
	{
		memcpy(dest + dstoffs, source, metasize);
		source += metasize;
		dstoffs += metasize;
	}

	// encode the audio channels
	if (channels > 0)
	{
		// encode the audio
		avhuff_error err = encode_audio(source, channels, samples, dest + dstoffs, &dest[8]);
		source += channels * samples * 2;
		if (err != AVHERR_NONE)
			return err;

		// advance the pointers past the data
		UINT16 treesize = (dest[8] << 8) + dest[9];
		if (treesize != 0xffff)
			dstoffs += treesize;
		for (int chnum = 0; chnum < channels; chnum++)
			dstoffs += (dest[10 + 2 * chnum] << 8) + dest[11 + 2 * chnum];
	}
	else
	{
		dest[8] = 0;
		dest[9] = 0;
	}

	// encode the video data
	if (width > 0 && height > 0)
	{
		// encode the video
		UINT32 vidlength = 0;
		avhuff_error err = encode_video(source, width, height, dest + dstoffs, vidlength);
		if (err != AVHERR_NONE)
			return err;

		// advance the pointers past the data
		dstoffs += vidlength;
	}

	// set the total compression
	complength = dstoffs;
	return AVHERR_NONE;
}

/**
 * @fn  UINT32 avhuff_encoder::raw_data_size(const UINT8 *data)
 *
 * @brief   -------------------------------------------------
 *            raw_data_size - return the raw data size of a raw stream based on the header
 *          -------------------------------------------------.
 *
 * @param   data    The data.
 *
 * @return  An UINT32.
 */

UINT32 avhuff_encoder::raw_data_size(const UINT8 *data)
{
	// make sure we have a correct header
	int size = 0;
	if (data[0] == 'c' && data[1] == 'h' && data[2] == 'a' && data[3] == 'v')
	{
		// add in header size plus metadata length
		size = 12 + data[4];

		// add in channels * samples
		size += 2 * data[5] * ((data[6] << 8) + data[7]);

		// add in 2 * width * height
		size += 2 * ((data[8] << 8) + data[9]) * (((data[10] << 8) + data[11]) & 0x7fff);
	}
	return size;
}

/**
 * @fn  avhuff_error avhuff_encoder::assemble_data(dynamic_buffer &buffer, bitmap_yuy16 &bitmap, UINT8 channels, UINT32 numsamples, INT16 **samples, UINT8 *metadata, UINT32 metadatasize)
 *
 * @brief   -------------------------------------------------
 *            assemble_data - assemble a datastream from raw bits
 *          -------------------------------------------------.
 *
 * @param [in,out]  buffer      The buffer.
 * @param [in,out]  bitmap      The bitmap.
 * @param   channels            The channels.
 * @param   numsamples          The numsamples.
 * @param [in,out]  samples     If non-null, the samples.
 * @param [in,out]  metadata    If non-null, the metadata.
 * @param   metadatasize        The metadatasize.
 *
 * @return  An avhuff_error.
 */

avhuff_error avhuff_encoder::assemble_data(dynamic_buffer &buffer, bitmap_yuy16 &bitmap, UINT8 channels, UINT32 numsamples, INT16 **samples, UINT8 *metadata, UINT32 metadatasize)
{
	// sanity check the inputs
	if (metadatasize > 255)
		return AVHERR_METADATA_TOO_LARGE;
	if (numsamples > 65535)
		return AVHERR_AUDIO_TOO_LARGE;
	if (bitmap.width() > 65535 || bitmap.height() > 65535)
		return AVHERR_VIDEO_TOO_LARGE;

	// fill in the header
	buffer.resize(12 + metadatasize + numsamples * channels * 2 + bitmap.width() * bitmap.height() * 2);
	UINT8 *dest = &buffer[0];
	*dest++ = 'c';
	*dest++ = 'h';
	*dest++ = 'a';
	*dest++ = 'v';
	*dest++ = metadatasize;
	*dest++ = channels;
	*dest++ = numsamples >> 8;
	*dest++ = numsamples & 0xff;
	*dest++ = bitmap.width() >> 8;
	*dest++ = bitmap.width() & 0xff;
	*dest++ = bitmap.height() >> 8;
	*dest++ = bitmap.height() & 0xff;

	// copy the metadata
	if (metadatasize > 0)
		memcpy(dest, metadata, metadatasize);
	dest += metadatasize;

	// copy the audio streams
	for (UINT8 curchan = 0; curchan < channels; curchan++)
		for (UINT32 cursamp = 0; cursamp < numsamples; cursamp++)
		{
			*dest++ = samples[curchan][cursamp] >> 8;
			*dest++ = samples[curchan][cursamp] & 0xff;
		}

	// copy the video data
	for (INT32 y = 0; y < bitmap.height(); y++)
	{
		UINT16 *src = &bitmap.pix(y);
		for (INT32 x = 0; x < bitmap.width(); x++)
		{
			*dest++ = src[x] >> 8;
			*dest++ = src[x] & 0xff;
		}
	}
	return AVHERR_NONE;
}

/**
 * @fn  avhuff_error avhuff_encoder::encode_audio(const UINT8 *source, int channels, int samples, UINT8 *dest, UINT8 *sizes)
 *
 * @brief   -------------------------------------------------
 *            encode_audio - encode raw audio data to the destination
 *          -------------------------------------------------.
 *
 * @param   source          Source for the.
 * @param   channels        The channels.
 * @param   samples         The samples.
 * @param [in,out]  dest    If non-null, destination for the.
 * @param [in,out]  sizes   If non-null, the sizes.
 *
 * @return  An avhuff_error.
 */

avhuff_error avhuff_encoder::encode_audio(const UINT8 *source, int channels, int samples, UINT8 *dest, UINT8 *sizes)
{
#if AVHUFF_USE_FLAC

	// input data is big-endian; determine our platform endianness
	UINT16 be_test = 0;
	*(UINT8 *)&be_test = 1;
	bool swap_endian = (be_test == 1);

	// set huffman tree size to 0xffff to indicate FLAC
	sizes[0] = 0xff;
	sizes[1] = 0xff;

	// set the block size for this round and iterate over channels
	m_flac_encoder.set_block_size(samples);
	for (int chnum = 0; chnum < channels; chnum++)
	{
		// encode the data
		m_flac_encoder.reset(dest, samples * 2);
		if (!m_flac_encoder.encode_interleaved(reinterpret_cast<const INT16 *>(source) + chnum * samples, samples, swap_endian))
			return AVHERR_COMPRESSION_ERROR;

		// set the size for this channel
		UINT32 cursize = m_flac_encoder.finish();
		sizes[chnum * 2 + 2] = cursize >> 8;
		sizes[chnum * 2 + 3] = cursize;
		dest += cursize;
	}

#else

	// expand the delta buffer if needed
	m_audiobuffer.resize(channels * samples * 2);
	UINT8 *deltabuf = m_audiobuffer;

	// iterate over channels to compute deltas
	m_audiohi_encoder.histo_reset();
	m_audiolo_encoder.histo_reset();
	for (int chnum = 0; chnum < channels; chnum++)
	{
		// extract audio data into hi and lo deltas stored in big-endian order
		INT16 prevsample = 0;
		for (int sampnum = 0; sampnum < samples; sampnum++)
		{
			INT16 newsample = (source[0] << 8) | source[1];
			source += 2;

			INT16 delta = newsample - prevsample;
			prevsample = newsample;
			m_audiohi_encoder.histo_one(*deltabuf++ = delta >> 8);
			m_audiolo_encoder.histo_one(*deltabuf++ = delta);
		}
	}

	// compute the trees
	huffman_error hufferr = m_audiohi_encoder.compute_tree_from_histo();
	if (hufferr != HUFFERR_NONE)
		return AVHERR_COMPRESSION_ERROR;
	hufferr = m_audiolo_encoder.compute_tree_from_histo();
	if (hufferr != HUFFERR_NONE)
		return AVHERR_COMPRESSION_ERROR;

	// export the trees to the output
	bitstream_out bitbuf(dest, 2 * channels * samples);
	hufferr = m_audiohi_encoder.export_tree_rle(bitbuf);
	if (hufferr != HUFFERR_NONE)
		return AVHERR_COMPRESSION_ERROR;
	bitbuf.flush();
	hufferr = m_audiolo_encoder.export_tree_rle(bitbuf);
	if (hufferr != HUFFERR_NONE)
		return AVHERR_COMPRESSION_ERROR;

	// note the size of the two trees
	UINT32 huffsize = bitbuf.flush();
	sizes[0] = huffsize >> 8;
	sizes[1] = huffsize;

	// iterate over channels
	UINT32 totalsize = huffsize;
	int chnum;
	for (chnum = 0; chnum < channels; chnum++)
	{
		// encode the data
		const UINT8 *input = m_audiobuffer + chnum * samples * 2;
		for (int sampnum = 0; sampnum < samples; sampnum++)
		{
			m_audiohi_encoder.encode_one(bitbuf, *input++);
			m_audiolo_encoder.encode_one(bitbuf, *input++);
		}

		// store the size of this stream
		UINT32 cursize = bitbuf.flush() - totalsize;
		totalsize += cursize;
		if (totalsize >= channels * samples * 2)
			break;
		sizes[chnum * 2 + 2] = cursize >> 8;
		sizes[chnum * 2 + 3] = cursize;
	}

	// if we ran out of room, throw it all away and just store raw
	if (chnum < channels)
	{
		memcpy(dest, m_audiobuffer, channels * samples * 2);
		UINT32 size = samples * 2;
		sizes[0] = sizes[1] = 0;
		for (chnum = 0; chnum < channels; chnum++)
		{
			sizes[chnum * 2 + 2] = size >> 8;
			sizes[chnum * 2 + 3] = size;
		}
	}

#endif

	return AVHERR_NONE;
}

/**
 * @fn  avhuff_error avhuff_encoder::encode_video(const UINT8 *source, int width, int height, UINT8 *dest, UINT32 &complength)
 *
 * @brief   -------------------------------------------------
 *            encode_video - encode raw video data to the destination
 *          -------------------------------------------------.
 *
 * @param   source              Source for the.
 * @param   width               The width.
 * @param   height              The height.
 * @param [in,out]  dest        If non-null, destination for the.
 * @param [in,out]  complength  The complength.
 *
 * @return  An avhuff_error.
 */

avhuff_error avhuff_encoder::encode_video(const UINT8 *source, int width, int height, UINT8 *dest, UINT32 &complength)
{
	// only lossless supported at this time
	return encode_video_lossless(source, width, height, dest, complength);
}

/**
 * @fn  avhuff_error avhuff_encoder::encode_video_lossless(const UINT8 *source, int width, int height, UINT8 *dest, UINT32 &complength)
 *
 * @brief   -------------------------------------------------
 *            encode_video_lossless - do a lossless video encoding using deltas and huffman
 *            encoding
 *          -------------------------------------------------.
 *
 * @param   source              Source for the.
 * @param   width               The width.
 * @param   height              The height.
 * @param [in,out]  dest        If non-null, destination for the.
 * @param [in,out]  complength  The complength.
 *
 * @return  An avhuff_error.
 */

avhuff_error avhuff_encoder::encode_video_lossless(const UINT8 *source, int width, int height, UINT8 *dest, UINT32 &complength)
{
	// set up the output; first byte is 0x80 to indicate lossless encoding
	bitstream_out bitbuf(dest, width * height * 2);
	bitbuf.write(0x80, 8);

	// compute the histograms for the data
	UINT16 *yrle = m_ycontext.rle_and_histo_bitmap(source + 0, width, 2, height);
	UINT16 *cbrle = m_cbcontext.rle_and_histo_bitmap(source + 1, width / 2, 4, height);
	UINT16 *crrle = m_crcontext.rle_and_histo_bitmap(source + 3, width / 2, 4, height);

	// export the trees to the data stream
	huffman_error hufferr = m_ycontext.export_tree_rle(bitbuf);
	if (hufferr != HUFFERR_NONE)
		return AVHERR_COMPRESSION_ERROR;
	bitbuf.flush();
	hufferr = m_cbcontext.export_tree_rle(bitbuf);
	if (hufferr != HUFFERR_NONE)
		return AVHERR_COMPRESSION_ERROR;
	bitbuf.flush();
	hufferr = m_crcontext.export_tree_rle(bitbuf);
	if (hufferr != HUFFERR_NONE)
		return AVHERR_COMPRESSION_ERROR;
	bitbuf.flush();

	// encode the data using the trees
	for (UINT32 sy = 0; sy < height; sy++)
	{
		m_ycontext.flush_rle();
		m_cbcontext.flush_rle();
		m_crcontext.flush_rle();
		for (UINT32 sx = 0; sx < width / 2; sx++)
		{
			m_ycontext.encode_one(bitbuf, yrle);
			m_cbcontext.encode_one(bitbuf, cbrle);
			m_ycontext.encode_one(bitbuf, yrle);
			m_crcontext.encode_one(bitbuf, crrle);
		}
	}

	// set the final length
	complength = bitbuf.flush();
	return AVHERR_NONE;
}



//**************************************************************************
//  DELTA-RLE ENCODER
//**************************************************************************

/**
 * @fn  UINT16 *avhuff_encoder::deltarle_encoder::rle_and_histo_bitmap(const UINT8 *source, UINT32 items_per_row, UINT32 item_advance, UINT32 row_count)
 *
 * @brief   -------------------------------------------------
 *            rle_and_histo_bitmap - RLE compress and histogram a bitmap's worth of data
 *          -------------------------------------------------.
 *
 * @param   source          Source for the.
 * @param   items_per_row   The items per row.
 * @param   item_advance    The item advance.
 * @param   row_count       Number of rows.
 *
 * @return  null if it fails, else an UINT16*.
 */

UINT16 *avhuff_encoder::deltarle_encoder::rle_and_histo_bitmap(const UINT8 *source, UINT32 items_per_row, UINT32 item_advance, UINT32 row_count)
{
	// resize our RLE buffer
	m_rlebuffer.resize(items_per_row * row_count);
	UINT16 *dest = &m_rlebuffer[0];

	// iterate over rows
	m_encoder.histo_reset();
	UINT8 prevdata = 0;
	for (UINT32 row = 0; row < row_count; row++)
	{
		const UINT8 *end = source + items_per_row * item_advance;
		for ( ; source < end; source += item_advance)
		{
			// fetch current data
			UINT8 curdelta = *source - prevdata;
			prevdata = *source;

			// 0 deltas scan forward for a count
			if (curdelta == 0)
			{
				int zerocount = 1;

				// count the number of consecutive values
				const UINT8 *scandata;
				for (scandata = source + item_advance; scandata < end; scandata += item_advance)
					if (*scandata == prevdata)
						zerocount++;
					else
						break;

				// if we hit the end of a row, maximize the count
				if (scandata >= end && zerocount >= 8)
					zerocount = 100000;

				// encode the maximal count we can
				int rlecode = rlecount_to_code(zerocount);
				m_encoder.histo_one(*dest++ = rlecode);

				// advance past the run
				source += (code_to_rlecount(rlecode) - 1) * item_advance;
			}

			// otherwise, encode the actual data
			else
				m_encoder.histo_one(*dest++ = curdelta);
		}

		// advance to the next row
		source = end;
	}

	// compute the tree for our histogram
	m_encoder.compute_tree_from_histo();
	return &m_rlebuffer[0];
}



//**************************************************************************
//  AVHUFF DECODER
//**************************************************************************

/**
 * @fn  avhuff_decoder::avhuff_decoder()
 *
 * @brief   -------------------------------------------------
 *            avhuff_decoder - constructor
 *          -------------------------------------------------.
 */

avhuff_decoder::avhuff_decoder()
{
}

/**
 * @fn  void avhuff_decoder::configure(const avhuff_decompress_config &config)
 *
 * @brief   -------------------------------------------------
 *            configure - configure decompression parameters
 *          -------------------------------------------------.
 *
 * @param   config  The configuration.
 */

void avhuff_decoder::configure(const avhuff_decompress_config &config)
{
	m_config.video.wrap(config.video, config.video.cliprect());
	m_config.maxsamples = config.maxsamples;
	m_config.actsamples = config.actsamples;
	memcpy(m_config.audio, config.audio, sizeof(m_config.audio));
	m_config.maxmetalength = config.maxmetalength;
	m_config.actmetalength = config.actmetalength;
	m_config.metadata = config.metadata;
}

/**
 * @fn  avhuff_error avhuff_decoder::decode_data(const UINT8 *source, UINT32 complength, UINT8 *dest)
 *
 * @brief   -------------------------------------------------
 *            decode_data - decode both audio and video from a raw data stream
 *          -------------------------------------------------.
 *
 * @param   source          Source for the.
 * @param   complength      The complength.
 * @param [in,out]  dest    If non-null, destination for the.
 *
 * @return  An avhuff_error.
 */

avhuff_error avhuff_decoder::decode_data(const UINT8 *source, UINT32 complength, UINT8 *dest)
{
	// extract info from the header
	if (complength < 8)
		return AVHERR_INVALID_DATA;
	UINT32 metasize = source[0];
	UINT32 channels = source[1];
	UINT32 samples = (source[2] << 8) + source[3];
	UINT32 width = (source[4] << 8) + source[5];
	UINT32 height = (source[6] << 8) + source[7];

	// validate that the sizes make sense
	if (complength < 10 + 2 * channels)
		return AVHERR_INVALID_DATA;
	UINT32 totalsize = 10 + 2 * channels;
	UINT32 treesize = (source[8] << 8) | source[9];
	if (treesize != 0xffff)
		totalsize += treesize;
	for (int chnum = 0; chnum < channels; chnum++)
		totalsize += (source[10 + 2 * chnum] << 8) | source[11 + 2 * chnum];
	if (totalsize >= complength)
		return AVHERR_INVALID_DATA;

	// starting offsets
	UINT32 srcoffs = 10 + 2 * channels;

	// if we are decoding raw, set up the output parameters
	UINT8 *metastart, *videostart, *audiostart[16];
	UINT32 audioxor, videoxor, videostride;
	if (dest != nullptr)
	{
		// create a header
		dest[0] = 'c';
		dest[1] = 'h';
		dest[2] = 'a';
		dest[3] = 'v';
		dest[4] = metasize;
		dest[5] = channels;
		dest[6] = samples >> 8;
		dest[7] = samples;
		dest[8] = width >> 8;
		dest[9] = width;
		dest[10] = height >> 8;
		dest[11] = height;
		dest += 12;

		// determine the start of each piece of data
		metastart = dest;
		dest += metasize;
		for (int chnum = 0; chnum < channels; chnum++)
		{
			audiostart[chnum] = dest;
			dest += 2 * samples;
		}
		videostart = dest;

		// data is assumed to be big-endian already
		audioxor = videoxor = 0;
		videostride = 2 * width;
	}

	// otherwise, extract from the state
	else
	{
		// determine the start of each piece of data
		metastart = m_config.metadata;
		for (int chnum = 0; chnum < channels; chnum++)
			audiostart[chnum] = (UINT8 *)m_config.audio[chnum];
		videostart = (m_config.video.valid()) ? reinterpret_cast<UINT8 *>(&m_config.video.pix(0)) : nullptr;
		videostride = (m_config.video.valid()) ? m_config.video.rowpixels() * 2 : 0;

		// data is assumed to be native-endian
		UINT16 betest = 0;
		*(UINT8 *)&betest = 1;
		audioxor = videoxor = (betest == 1) ? 1 : 0;

		// verify against sizes
		if (m_config.video.valid() && (m_config.video.width() < width || m_config.video.height() < height))
			return AVHERR_VIDEO_TOO_LARGE;
		for (int chnum = 0; chnum < channels; chnum++)
			if (m_config.audio[chnum] != nullptr && m_config.maxsamples < samples)
				return AVHERR_AUDIO_TOO_LARGE;
		if (m_config.metadata != nullptr && m_config.maxmetalength < metasize)
			return AVHERR_METADATA_TOO_LARGE;

		// set the output values
		if (m_config.actsamples != nullptr)
			*m_config.actsamples = samples;
		if (m_config.actmetalength != nullptr)
			*m_config.actmetalength = metasize;
	}

	// copy the metadata first
	if (metasize > 0)
	{
		if (metastart != nullptr)
			memcpy(metastart, source + srcoffs, metasize);
		srcoffs += metasize;
	}

	// decode the audio channels
	if (channels > 0)
	{
		// decode the audio
		avhuff_error err = decode_audio(channels, samples, source + srcoffs, audiostart, audioxor, &source[8]);
		if (err != AVHERR_NONE)
			return err;

		// advance the pointers past the data
		treesize = (source[8] << 8) + source[9];
		if (treesize != 0xffff)
			srcoffs += treesize;
		for (int chnum = 0; chnum < channels; chnum++)
			srcoffs += (source[10 + 2 * chnum] << 8) + source[11 + 2 * chnum];
	}

	// decode the video data
	if (width > 0 && height > 0 && videostart != nullptr)
	{
		// decode the video
		avhuff_error err = decode_video(width, height, source + srcoffs, complength - srcoffs, videostart, videostride, videoxor);
		if (err != AVHERR_NONE)
			return err;
	}
	return AVHERR_NONE;
}

/**
 * @fn  avhuff_error avhuff_decoder::decode_audio(int channels, int samples, const UINT8 *source, UINT8 **dest, UINT32 dxor, const UINT8 *sizes)
 *
 * @brief   -------------------------------------------------
 *            decode_audio - decode audio from a compressed data stream
 *          -------------------------------------------------.
 *
 * @exception   CHDERR_DECOMPRESSION_ERROR  Thrown when a chderr decompression error error
 *                                          condition occurs.
 *
 * @param   channels        The channels.
 * @param   samples         The samples.
 * @param   source          Source for the.
 * @param [in,out]  dest    If non-null, destination for the.
 * @param   dxor            The dxor.
 * @param   sizes           The sizes.
 *
 * @return  An avhuff_error.
 */

avhuff_error avhuff_decoder::decode_audio(int channels, int samples, const UINT8 *source, UINT8 **dest, UINT32 dxor, const UINT8 *sizes)
{
	// extract the huffman trees
	UINT16 treesize = (sizes[0] << 8) | sizes[1];

#if AVHUFF_USE_FLAC

	// if the tree size is 0xffff, the streams are FLAC-encoded
	if (treesize == 0xffff)
	{
		// output data is big-endian; determine our platform endianness
		UINT16 be_test = 0;
		*(UINT8 *)&be_test = 1;
		bool swap_endian = (be_test == 1);
		if (dxor != 0)
			swap_endian = !swap_endian;

		// loop over channels
		for (int chnum = 0; chnum < channels; chnum++)
		{
			// extract the size of this channel
			UINT16 size = (sizes[chnum * 2 + 2] << 8) | sizes[chnum * 2 + 3];

			// only process if the data is requested
			UINT8 *curdest = dest[chnum];
			if (curdest != nullptr)
			{
				// reset and decode
				if (!m_flac_decoder.reset(48000, 1, samples, source, size))
					throw CHDERR_DECOMPRESSION_ERROR;
				if (!m_flac_decoder.decode_interleaved(reinterpret_cast<INT16 *>(curdest), samples, swap_endian))
					throw CHDERR_DECOMPRESSION_ERROR;

				// finish up
				m_flac_decoder.finish();
			}

			// advance to the next channel's data
			source += size;
		}
		return AVHERR_NONE;
	}

#endif

	// if we have a non-zero tree size, extract the trees
	if (treesize != 0)
	{
		bitstream_in bitbuf(source, treesize);
		huffman_error hufferr = m_audiohi_decoder.import_tree_rle(bitbuf);
		if (hufferr != HUFFERR_NONE)
			return AVHERR_INVALID_DATA;
		bitbuf.flush();
		hufferr = m_audiolo_decoder.import_tree_rle(bitbuf);
		if (hufferr != HUFFERR_NONE)
			return AVHERR_INVALID_DATA;
		if (bitbuf.flush() != treesize)
			return AVHERR_INVALID_DATA;
		source += treesize;
	}

	// loop over channels
	for (int chnum = 0; chnum < channels; chnum++)
	{
		// extract the size of this channel
		UINT16 size = (sizes[chnum * 2 + 2] << 8) | sizes[chnum * 2 + 3];

		// only process if the data is requested
		UINT8 *curdest = dest[chnum];
		if (curdest != nullptr)
		{
			INT16 prevsample = 0;

			// if no huffman length, just copy the data
			if (treesize == 0)
			{
				const UINT8 *cursource = source;
				for (int sampnum = 0; sampnum < samples; sampnum++)
				{
					INT16 delta = (cursource[0] << 8) | cursource[1];
					cursource += 2;

					INT16 newsample = prevsample + delta;
					prevsample = newsample;

					curdest[0 ^ dxor] = newsample >> 8;
					curdest[1 ^ dxor] = newsample;
					curdest += 2;
				}
			}

			// otherwise, Huffman-decode the data
			else
			{
				bitstream_in bitbuf(source, size);
				for (int sampnum = 0; sampnum < samples; sampnum++)
				{
					INT16 delta = m_audiohi_decoder.decode_one(bitbuf) << 8;
					delta |= m_audiolo_decoder.decode_one(bitbuf);

					INT16 newsample = prevsample + delta;
					prevsample = newsample;

					curdest[0 ^ dxor] = newsample >> 8;
					curdest[1 ^ dxor] = newsample;
					curdest += 2;
				}
				if (bitbuf.overflow())
					return AVHERR_INVALID_DATA;
			}
		}

		// advance to the next channel's data
		source += size;
	}
	return AVHERR_NONE;
}

/**
 * @fn  avhuff_error avhuff_decoder::decode_video(int width, int height, const UINT8 *source, UINT32 complength, UINT8 *dest, UINT32 dstride, UINT32 dxor)
 *
 * @brief   -------------------------------------------------
 *            decode_video - decode video from a compressed data stream
 *          -------------------------------------------------.
 *
 * @param   width           The width.
 * @param   height          The height.
 * @param   source          Source for the.
 * @param   complength      The complength.
 * @param [in,out]  dest    If non-null, destination for the.
 * @param   dstride         The dstride.
 * @param   dxor            The dxor.
 *
 * @return  An avhuff_error.
 */

avhuff_error avhuff_decoder::decode_video(int width, int height, const UINT8 *source, UINT32 complength, UINT8 *dest, UINT32 dstride, UINT32 dxor)
{
	// if the high bit of the first byte is set, we decode losslessly
	if (source[0] & 0x80)
		return decode_video_lossless(width, height, source, complength, dest, dstride, dxor);
	else
		return AVHERR_INVALID_DATA;
}

/**
 * @fn  avhuff_error avhuff_decoder::decode_video_lossless(int width, int height, const UINT8 *source, UINT32 complength, UINT8 *dest, UINT32 dstride, UINT32 dxor)
 *
 * @brief   -------------------------------------------------
 *            decode_video_lossless - do a lossless video decoding using deltas and huffman
 *            encoding
 *          -------------------------------------------------.
 *
 * @param   width           The width.
 * @param   height          The height.
 * @param   source          Source for the.
 * @param   complength      The complength.
 * @param [in,out]  dest    If non-null, destination for the.
 * @param   dstride         The dstride.
 * @param   dxor            The dxor.
 *
 * @return  An avhuff_error.
 */

avhuff_error avhuff_decoder::decode_video_lossless(int width, int height, const UINT8 *source, UINT32 complength, UINT8 *dest, UINT32 dstride, UINT32 dxor)
{
	// skip the first byte
	bitstream_in bitbuf(source, complength);
	bitbuf.read(8);

	// import the tables
	huffman_error hufferr = m_ycontext.import_tree_rle(bitbuf);
	if (hufferr != HUFFERR_NONE)
		return AVHERR_INVALID_DATA;
	bitbuf.flush();
	hufferr = m_cbcontext.import_tree_rle(bitbuf);
	if (hufferr != HUFFERR_NONE)
		return AVHERR_INVALID_DATA;
	bitbuf.flush();
	hufferr = m_crcontext.import_tree_rle(bitbuf);
	if (hufferr != HUFFERR_NONE)
		return AVHERR_INVALID_DATA;
	bitbuf.flush();

	// decode to the destination
	m_ycontext.reset();
	m_cbcontext.reset();
	m_crcontext.reset();
	for (UINT32 dy = 0; dy < height; dy++)
	{
		UINT8 *row = dest + dy * dstride;
		for (UINT32 dx = 0; dx < width / 2; dx++)
		{
			row[0 ^ dxor] = m_ycontext.decode_one(bitbuf);
			row[1 ^ dxor] = m_cbcontext.decode_one(bitbuf);
			row[2 ^ dxor] = m_ycontext.decode_one(bitbuf);
			row[3 ^ dxor] = m_crcontext.decode_one(bitbuf);
			row += 4;
		}
		m_ycontext.flush_rle();
		m_cbcontext.flush_rle();
		m_crcontext.flush_rle();
	}

	// check for errors if we overflowed or decoded too little data
	if (bitbuf.overflow() || bitbuf.flush() != complength)
		return AVHERR_INVALID_DATA;
	return AVHERR_NONE;
}
