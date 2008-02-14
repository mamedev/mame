/*
To do:
* verify Cb,Y,Cr,Y ordering
* swap the ordering(?)
* add backchannel support for samples as well?
*/
/***************************************************************************

    avcomp.c

    Audio/video compression and decompression helpers.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Each frame is compressed as a unit. The raw data is of the form:
    (all multibyte values are stored in big-endian format)

        'chav' (4 bytes) - fixed header data to identify the format
        metasize (1 byte) - size of metadata in bytes (max=255 bytes)
        channels (1 byte) - number of audio channels
        samples (2 bytes) - number of samples per audio stream
        width (2 bytes) - width of video data
        height (2 bytes) - height of video data (high bit set means interlaced)
        <metadata> - as raw bytes
        <audio stream 0> - as signed 16-bit samples
        <audio stream 1> - as signed 16-bit samples
        ...
        <video data> - as a raw array of 8-bit YUY data in (Cb,Y,Cr,Y) order

    When compressed, the data is stored as follows:
    (all multibyte values are stored in big-endian format)

        metasize (1 byte) - size of metadata in bytes
        channels (1 byte) - number of audio channels
        samples (2 bytes) - number of samples per audio stream
        width (2 bytes) - width of video data
        height (2 bytes) - height of video data (high bit set means interlaced)
        audhuffsize (1 byte) - size of the audio Huffman table
        str0size (2 bytes) - compressed size of stream 0
        str1size (2 bytes) - compressed size of stream 1
        ...
        <metadata> - as raw data
        <audio huffman table> - Huffman table for audio decoding
        <audio stream 0 data> - Huffman-compressed deltas
        <audio stream 1 data> - Huffman-compressed deltas
        <...>
        <video huffman table> - Huffman table for video decoding
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

#include "avcomp.h"
#include "huffman.h"
#include "chd.h"

#include <math.h>



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct _avcomp_state
{
	/* decompression parameters */
	UINT32				decodemask;
	UINT8 *				videobuffer;
	UINT32				videostride;
	UINT32				videoxor;
	UINT32				audioxor;

	/* video parameters */
	UINT32				maxwidth, maxheight;

	/* audio parameters */
	UINT32				maxchannels;

	/* intermediate data */
	UINT8 *				deltadata;
	UINT8 *				audiodata;

	/* huffman contexts */
	huffman_context *	ycontext;
	huffman_context *	ccontext;
	huffman_context *	audiocontext;
};



/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* encoding helpers */
static avcomp_error encode_audio(avcomp_state *state, int channels, int samples, const UINT8 *source, UINT8 *dest, UINT8 *sizes);
static avcomp_error encode_video(avcomp_state *state, int width, int height, int interlaced, const UINT8 *source, UINT8 *dest, UINT32 *complength);
static avcomp_error encode_video_lossless(avcomp_state *state, int width, int height, int interlaced, const UINT8 *source, UINT8 *dest, UINT32 *complength);

/* decoding helpers */
static avcomp_error decode_audio(avcomp_state *state, int channels, int samples, const UINT8 *source, UINT8 *dest, const UINT8 *sizes, UINT32 destxor);
static avcomp_error decode_video(avcomp_state *state, int width, int height, int interlaced, const UINT8 *source, UINT32 complength, UINT8 *dest);
static avcomp_error decode_video_lossless(avcomp_state *state, int width, int height, int interlaced, const UINT8 *source, UINT32 complength, UINT8 *dest, UINT32 deststride, UINT32 destxor);



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    avcomp_init - allocate and initialize a
    new state block for compression or
    decompression
-------------------------------------------------*/

avcomp_state *avcomp_init(UINT32 maxwidth, UINT32 maxheight, UINT32 maxchannels)
{
	huffman_error hufferr;
	avcomp_state *state;

	/* allocate memory for state block */
	state = malloc(sizeof(*state));
	if (state == NULL)
		return NULL;

	/* clear the buffers */
	memset(state, 0, sizeof(*state));

	/* fill in sensible decompression defaults */
	state->decodemask = AVCOMP_DECODE_DEFAULT;

	/* compute the core info */
	state->maxwidth = maxwidth;
	state->maxheight = maxheight;
	state->maxchannels = maxchannels;

	/* now allocate data buffers */
	state->deltadata = malloc(state->maxwidth * state->maxheight * 2);
	state->audiodata = malloc(65536 * state->maxchannels * 2);
	if (state->deltadata == NULL || state->audiodata == NULL)
		goto cleanup;

	/* create huffman contexts */
	hufferr = huffman_create_context(&state->ycontext, 12);
	if (hufferr != HUFFERR_NONE)
		goto cleanup;
	hufferr = huffman_create_context(&state->ccontext, 12);
	if (hufferr != HUFFERR_NONE)
		goto cleanup;
	hufferr = huffman_create_context(&state->audiocontext, 12);
	if (hufferr != HUFFERR_NONE)
		goto cleanup;

	return state;

cleanup:
	avcomp_free(state);
	return NULL;
}


/*-------------------------------------------------
    avcomp_free - free a state block
-------------------------------------------------*/

void avcomp_free(avcomp_state *state)
{
	/* free the data buffers */
	if (state->deltadata != NULL)
		free(state->deltadata);
	if (state->audiodata != NULL)
		free(state->audiodata);

	/* free the contexts */
	if (state->ycontext != NULL)
		huffman_free_context(state->ycontext);
	if (state->ccontext != NULL)
		huffman_free_context(state->ccontext);
	if (state->audiocontext != NULL)
		huffman_free_context(state->audiocontext);

	free(state);
}


/*-------------------------------------------------
    avcomp_decompress_config - configure compression
    parameters
-------------------------------------------------*/

void avcomp_decompress_config(avcomp_state *state, UINT32 decodemask, UINT8 *videobuffer, UINT32 videostride, UINT32 videoxor, UINT32 audioxor)
{
	state->decodemask = decodemask;
	state->videobuffer = videobuffer;
	state->videostride = videostride;
	state->videoxor = videoxor;
	state->audioxor = audioxor;
}



/***************************************************************************
    ENCODING/DECODING FRONTENDS
***************************************************************************/

/*-------------------------------------------------
    avcomp_encode_data - encode a block of data
    into a compressed data stream
-------------------------------------------------*/

avcomp_error avcomp_encode_data(avcomp_state *state, const UINT8 *source, UINT8 *dest, UINT32 *complength)
{
	UINT32 metasize, channels, samples, width, height;
	UINT32 srcoffs, dstoffs;
	avcomp_error err;
	int interlaced;

	/* validate the header */
	if (source[0] != 'c' || source[1] != 'h' || source[2] != 'a' || source[3] != 'v')
		return AVCERR_INVALID_DATA;

	/* extract info from the header */
	metasize = source[4];
	channels = source[5];
	samples = (source[6] << 8) + source[7];
	width = (source[8] << 8) + source[9];
	height = (source[10] << 8) + source[11];
	interlaced = (height >> 15) & 1;
	height &= 0x7fff;

	/* validate the info from the header */
	if (width > state->maxwidth || height > state->maxheight)
		return AVCERR_VIDEO_TOO_LARGE;
	if (channels > state->maxchannels)
		return AVCERR_AUDIO_TOO_LARGE;

	/* write the basics to the new header */
	dest[0] = metasize;
	dest[1] = channels;
	dest[2] = samples >> 8;
	dest[3] = samples;
	dest[4] = width >> 8;
	dest[5] = width;
	dest[6] = (interlaced << 7) | (height >> 8);
	dest[7] = height;

	/* starting offsets */
	srcoffs = 12;
	dstoffs = 9 + 2 * channels;

	/* copy the metadata first */
	if (metasize > 0)
	{
		memcpy(dest + dstoffs, source + srcoffs, metasize);
		srcoffs += metasize;
		dstoffs += metasize;
	}

	/* encode the audio channels */
	if (channels > 0)
	{
		int chnum;

		/* encode the audio */
		err = encode_audio(state, channels, samples, source + srcoffs, dest + dstoffs, &dest[8]);
		if (err != AVCERR_NONE)
			return err;

		/* advance the pointers past the data */
		srcoffs += channels * samples * 2;
		dstoffs += dest[8];
		for (chnum = 0; chnum < channels; chnum++)
			dstoffs += (dest[9 + 2 * chnum] << 8) + dest[10 + 2 * chnum];
	}

	/* encode the video data */
	if (width > 0 && height > 0)
	{
		UINT32 vidlength;

		/* encode the video */
		err = encode_video(state, width, height, interlaced, source + srcoffs, dest + dstoffs, &vidlength);
		if (err != AVCERR_NONE)
			return err;

		/* advance the pointers past the data */
		srcoffs += width * height * 2;
		dstoffs += vidlength;
	}

	/* set the total compression */
	*complength = dstoffs;
	return AVCERR_NONE;
}


/*-------------------------------------------------
    avcomp_decode_data - decode both
    audio and video from a raw data stream
-------------------------------------------------*/

avcomp_error avcomp_decode_data(avcomp_state *state, const UINT8 *source, UINT32 complength, UINT8 *dest)
{
	UINT32 metasize, channels, samples, width, height;
	UINT32 srcoffs, dstoffs, totalsize;
	avcomp_error err;
	int interlaced;
	int chnum;

	/* extract info from the header */
	if (complength < 8)
		return AVCERR_INVALID_DATA;
	metasize = source[0];
	channels = source[1];
	samples = (source[2] << 8) + source[3];
	width = (source[4] << 8) + source[5];
	height = (source[6] << 8) + source[7];
	interlaced = height >> 15;
	height &= 0x7fff;

	/* validate the info from the header */
	if (width > state->maxwidth || height > state->maxheight)
		return AVCERR_VIDEO_TOO_LARGE;
	if (channels > state->maxchannels)
		return AVCERR_AUDIO_TOO_LARGE;

	/* validate that the sizes make sense */
	if (complength < 9 + 2 * channels)
		return AVCERR_INVALID_DATA;
	totalsize = 9 + 2 * channels + source[8];
	for (chnum = 0; chnum < channels; chnum++)
		totalsize += (source[9 + 2 * chnum] << 8) | source[10 + 2 * chnum];
	if (totalsize >= complength)
		return AVCERR_INVALID_DATA;

	/* starting offsets */
	srcoffs = 9 + 2 * channels;
	dstoffs = 12;

	/* write the basics to the new header */
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
	dest[10] = (interlaced << 7) | (height >> 8);
	dest[11] = height;

	/* copy the metadata first */
	if (metasize > 0)
	{
		if ((state->decodemask & AVCOMP_DECODE_META) != 0)
			memcpy(dest + dstoffs, source + srcoffs, metasize);
		srcoffs += metasize;
		dstoffs += metasize;
	}

	/* decode the audio channels */
	if (channels > 0)
	{
		/* decode the audio */
		err = decode_audio(state, channels, samples, source + srcoffs, dest + dstoffs, &source[8], state->audioxor);
		if (err != AVCERR_NONE)
			return err;

		/* advance the pointers past the data */
		srcoffs += source[8];
		for (chnum = 0; chnum < channels; chnum++)
			srcoffs += (source[9 + 2 * chnum] << 8) + source[10 + 2 * chnum];
		dstoffs += channels * samples * 2;
	}

	/* decode the video data */
	if (width > 0 && height > 0 && (state->decodemask & AVCOMP_DECODE_VIDEO) != 0)
	{
		/* decode the video */
		err = decode_video(state, width, height, interlaced, source + srcoffs, complength - srcoffs, dest + dstoffs);
		if (err != AVCERR_NONE)
			return err;
	}
	return AVCERR_NONE;
}



/***************************************************************************
    ENCODING HELPERS
***************************************************************************/

/*-------------------------------------------------
    encode_audio - encode raw audio data
    to the destination
-------------------------------------------------*/

static avcomp_error encode_audio(avcomp_state *state, int channels, int samples, const UINT8 *source, UINT8 *dest, UINT8 *sizes)
{
	UINT32 size, bytesleft;
	huffman_error hufferr;
	int chnum, sampnum;
	const UINT8 *input;
	UINT8 *output;

	/* get input/output pointers */
	input = source;
	output = state->audiodata;

	/* extract audio deltas */
	for (chnum = 0; chnum < channels; chnum++)
	{
		INT16 prev = 0;

		for (sampnum = 0; sampnum < samples; sampnum++)
		{
			INT16 cursample, delta;

			/* read current sample (big endian) */
			cursample = *input++ << 8;
			cursample |= *input++;

			/* compute delta against previous sample in this channel */
			delta = cursample - prev;
			prev = cursample;

			/* store the delta (big endian) */
			*output++ = delta >> 8;
			*output++ = delta & 0xff;
		}
	}

	/* reset our output pointer and huffman encode */
	input = state->audiodata;
	output = dest;
	bytesleft = channels * samples * 2;

	/* compute a single huffman tree for all the audio data */
	hufferr = huffman_compute_tree(state->audiocontext, input, bytesleft, 1);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_COMPRESSION_ERROR;
	hufferr = huffman_export_tree(state->audiocontext, output, bytesleft, &size);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_COMPRESSION_ERROR;
	output += size;
	*sizes++ = size;
	bytesleft -= size;

	/* encode each stream separately */
	for (chnum = 0; chnum < channels; chnum++)
	{
		/* encode the data */
		hufferr = huffman_encode_data(state->audiocontext, input, samples * 2, output, bytesleft, &size);
		if (hufferr != HUFFERR_NONE)
			return AVCERR_COMPRESSION_ERROR;
		input += samples * 2;
		output += size;
		bytesleft = (bytesleft > size) ? (bytesleft - size) : 0;

		/* store the size of this stream */
		*sizes++ = size >> 8;
		*sizes++ = size;
	}

	/* if we didn't compress enough, set the huffman table size to 0 and memcpy the data */
	if (bytesleft == 0)
	{
		/* back up to the beginning */
		input = source;
		output = dest;
		sizes -= 1 + 2 * channels;

		/* huffman table size of 0 */
		*sizes++ = 0;

		/* write the full size of each stream and then copy it */
		size = samples * 2;
		for (chnum = 0; chnum < channels; chnum++)
		{
			/* copy the data */
			memcpy(output, input, size);
			input += size;
			output += size;

			/* store the size of this stream */
			*sizes++ = size >> 8;
			*sizes++ = size;
		}
	}

	return AVCERR_NONE;
}


/*-------------------------------------------------
    encode_video - encode raw video data
    to the destination
-------------------------------------------------*/

static avcomp_error encode_video(avcomp_state *state, int width, int height, int interlaced, const UINT8 *source, UINT8 *dest, UINT32 *complength)
{
	/* only lossless supported at this time */
	return encode_video_lossless(state, width, height, interlaced, source, dest, complength);
}


/*-------------------------------------------------
    encode_video_lossless - do a lossless video
    encoding using deltas and huffman encoding
-------------------------------------------------*/

static avcomp_error encode_video_lossless(avcomp_state *state, int width, int height, int interlaced, const UINT8 *source, UINT8 *dest, UINT32 *complength)
{
	UINT32 srcbytes = width * height * 2;
	int rowbytes = width * 2;
	huffman_error hufferr;
	UINT32 outbytes;
	UINT8 *output;
	int x, y;

	/* loop over rows */
	output = state->deltadata;
	for (y = 0; y < height; y++)
	{
		const UINT8 *src = &source[rowbytes * y];
		UINT8 lasty = (y == 0) ? 0x80 : src[-rowbytes + 1];
		UINT8 lastcb = (y == 0) ? 0x80 : src[-rowbytes + 0];
		UINT8 lastcr = (y == 0) ? 0x80 : src[-rowbytes + 2];

		/* loop over columns */
		for (x = 0; x < rowbytes; x += 4)
		{
			*output++ = *src - lastcb;
			lastcb = *src++;
			*output++ = *src - lasty;
			lasty = *src++;
			*output++ = *src - lastcr;
			lastcr = *src++;
			*output++ = *src - lasty;
			lasty = *src++;
		}
	}

	/* set up the output; first byte is 0x80 to indicate lossless encoding */
	output = dest;
	*output++ = 0x80;

	/* now encode to the destination using two trees, one for the Y and one for the Cr/Cb */

	/* compute the histograms for the data */
	hufferr = huffman_compute_tree(state->ycontext, state->deltadata + 0, srcbytes, 2);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_COMPRESSION_ERROR;
	hufferr = huffman_compute_tree(state->ccontext, state->deltadata + 1, srcbytes, 2);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_COMPRESSION_ERROR;

	/* export the two trees to the data stream */
	hufferr = huffman_export_tree(state->ycontext, output, 256, &outbytes);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_COMPRESSION_ERROR;
	output += outbytes;
	hufferr = huffman_export_tree(state->ccontext, output, 256, &outbytes);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_COMPRESSION_ERROR;
	output += outbytes;

	/* encode the data using the two trees */
	hufferr = huffman_encode_data_interleaved_2(state->ycontext, state->ccontext, state->deltadata, srcbytes, output, srcbytes, &outbytes);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_COMPRESSION_ERROR;
	output += outbytes;

	/* set the final length */
	*complength = output - dest;
	return AVCERR_NONE;
}



/***************************************************************************
    DECODING HELPERS
***************************************************************************/

/*-------------------------------------------------
    decode_audio - decode audio from a
    compressed data stream
-------------------------------------------------*/

static avcomp_error decode_audio(avcomp_state *state, int channels, int samples, const UINT8 *source, UINT8 *dest, const UINT8 *sizes, UINT32 destxor)
{
	huffman_error hufferr;
	int chnum, sampnum;
	const UINT8 *input;
	UINT32 actsize;
	UINT8 *output;
	UINT16 size;

	/* set up input/output pointers */
	input = source;
	output = dest;

	/* if no huffman length, just copy the data */
	size = *sizes++;
	if (size == 0)
	{
		/* loop over channels */
		for (chnum = 0; chnum < channels; chnum++)
		{
			/* extract the size of this channel */
			size = *sizes++ << 8;
			size |= *sizes++;

			/* copy the data */
			if ((state->decodemask & AVCOMP_DECODE_AUDIO(chnum)) != 0)
			{
				for (sampnum = 0; sampnum < samples; sampnum++)
				{
					output[0 ^ destxor] = input[0];
					output[1 ^ destxor] = input[1];
					output += 2;
					input += 2;
				}
			}
			else
			{
				input += size;
				output += size;
			}
		}
		return AVCERR_NONE;
	}

	/* extract the huffman tree */
	input = source;
	hufferr = huffman_import_tree(state->audiocontext, input, size, &actsize);
	if (hufferr != HUFFERR_NONE || actsize != size)
		return AVCERR_INVALID_DATA;
	input += actsize;

	/* now loop over channels and decode their data */
	for (chnum = 0; chnum < channels; chnum++)
	{
		/* extract the size of this channel */
		size = *sizes++ << 8;
		size |= *sizes++;

		/* decode the data */
		if ((state->decodemask & AVCOMP_DECODE_AUDIO(chnum)) != 0)
		{
			hufferr = huffman_decode_data(state->audiocontext, input, size, output, samples * 2, &actsize);
			if (hufferr != HUFFERR_NONE || actsize != size)
				return AVCERR_INVALID_DATA;
		}

		/* advance */
		input += size;
		output += samples * 2;
	}

	/* reset input/output pointers */
	output = dest;

	/* reassemble audio from the deltas */
	for (chnum = 0; chnum < channels; chnum++)
	{
		INT16 prev = 0;

		if ((state->decodemask & AVCOMP_DECODE_AUDIO(chnum)) != 0)
		{
			for (sampnum = 0; sampnum < samples; sampnum++)
			{
				INT16 cursample;

				/* read current sample (big endian) */
				cursample = output[0] << 8;
				cursample |= output[1];

				/* compute delta against previous sample in this channel */
				cursample += prev;
				prev = cursample;

				/* store the delta (big endian) */
				output[0 ^ destxor] = cursample >> 8;
				output[1 ^ destxor] = cursample;
				output += 2;
			}
		}
		else
			output += samples * 2;
	}
	return AVCERR_NONE;
}


/*-------------------------------------------------
    decode_video - decode video from a
    compressed data stream
-------------------------------------------------*/

static avcomp_error decode_video(avcomp_state *state, int width, int height, int interlaced, const UINT8 *source, UINT32 complength, UINT8 *dest)
{
	UINT32 targetstride, targetxor;
	UINT8 *target;

	/* if we have a target buffer, use that instead */
	if (state->videobuffer != NULL)
	{
		target = state->videobuffer;
		targetstride = state->videostride;
		targetxor = state->videoxor;
	}
	else
	{
		target = dest;
		targetstride = width * 2;
		targetxor = 0;
	}

	/* if the high bit of the first byte is set, we decode losslessly */
	if (source[0] & 0x80)
		return decode_video_lossless(state, width, height, interlaced, source, complength, target, targetstride, targetxor);
	else
		return AVCERR_INVALID_DATA;
}


/*-------------------------------------------------
    decode_video_lossless - do a lossless video
    decoding using deltas and huffman encoding
-------------------------------------------------*/

static avcomp_error decode_video_lossless(avcomp_state *state, int width, int height, int interlaced, const UINT8 *source, UINT32 complength, UINT8 *dest, UINT32 deststride, UINT32 destxor)
{
	const UINT8 *sourceend = source + complength;
	const huffman_lookup_value *table1, *table2;
	huffman_error hufferr;
	UINT32 bitbuf = 0;
	UINT32 actsize;
	int sbits = 0;
	int x, y;

	/* skip the first byte */
	source++;

	/* decode the table and data */
	hufferr = huffman_import_tree(state->ycontext, source, sourceend - source, &actsize);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_INVALID_DATA;
	source += actsize;
	hufferr = huffman_import_tree(state->ccontext, source, sourceend - source, &actsize);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_INVALID_DATA;
	source += actsize;

	/* get the lookup tables */
	hufferr = huffman_get_lookup_table(state->ycontext, &table1);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_OUT_OF_MEMORY;
	hufferr = huffman_get_lookup_table(state->ccontext, &table2);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_OUT_OF_MEMORY;

	/* loop over rows */
	for (y = 0; y < height; y++)
	{
		UINT8 *dst = &dest[y * deststride];
		UINT8 lasty = (y == 0) ? 0x80 : (dst - deststride)[1 ^ destxor];
		UINT8 lastcb = (y == 0) ? 0x80 : (dst - deststride)[0 ^ destxor];
		UINT8 lastcr = (y == 0) ? 0x80 : (dst - deststride)[2 ^ destxor];

		/* loop over columns */
		for (x = 0; x < width * 2; x += 4)
		{
			huffman_lookup_value lookup;

			/* keep the buffer full */
			while (sbits <= 24)
			{
				bitbuf |= *source++ << (24 - sbits);
				sbits += 8;
			}

			/* do the Cb component */
			lookup = table1[bitbuf >> 20];
			dst[(x + 0) ^ destxor] = lastcb += lookup >> 8;
			lookup &= 0x1f;
			bitbuf <<= lookup;
			sbits -= lookup;

			/* do the Y component */
			lookup = table2[bitbuf >> 20];
			dst[(x + 1) ^ destxor] = lasty += lookup >> 8;
			lookup &= 0x1f;
			bitbuf <<= lookup;
			sbits -= lookup;

			/* keep the buffer full */
			while (sbits <= 24)
			{
				bitbuf |= *source++ << (24 - sbits);
				sbits += 8;
			}

			/* do the Cr component */
			lookup = table1[bitbuf >> 20];
			dst[(x + 2) ^ destxor] = lastcr += lookup >> 8;
			lookup &= 0x1f;
			bitbuf <<= lookup;
			sbits -= lookup;

			/* do the Y component */
			lookup = table2[bitbuf >> 20];
			dst[(x + 3) ^ destxor] = lasty += lookup >> 8;
			lookup &= 0x1f;
			bitbuf <<= lookup;
			sbits -= lookup;
		}
	}

	return AVCERR_NONE;
}
