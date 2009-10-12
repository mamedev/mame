/***************************************************************************

    avcomp.c

    Audio/video compression and decompression helpers.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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

#include "avcomp.h"
#include "huffman.h"
#include "chd.h"

#include <math.h>



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_CHANNELS	4



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct _avcomp_state
{
	/* video parameters */
	UINT32				maxwidth, maxheight;

	/* audio parameters */
	UINT32				maxchannels;

	/* intermediate data */
	UINT8 *				audiodata;

	/* huffman contexts */
	huffman_context *	ycontext;
	huffman_context *	cbcontext;
	huffman_context *	crcontext;
	huffman_context *	audiohicontext;
	huffman_context *	audiolocontext;

	/* configuration data */
	av_codec_compress_config compress;
	av_codec_decompress_config decompress;
};



/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* encoding helpers */
static avcomp_error encode_audio(avcomp_state *state, int channels, int samples, const UINT8 **source, int sourcexor, UINT8 *dest, UINT8 *sizes);
static avcomp_error encode_video(avcomp_state *state, int width, int height, const UINT8 *source, UINT32 sstride, UINT32 sxor, UINT8 *dest, UINT32 *complength);
static avcomp_error encode_video_lossless(avcomp_state *state, int width, int height, const UINT8 *source, UINT32 sstride, UINT32 sxor, UINT8 *dest, UINT32 *complength);

/* decoding helpers */
static avcomp_error decode_audio(avcomp_state *state, int channels, int samples, const UINT8 *source, UINT8 **dest, UINT32 dxor, const UINT8 *sizes);
static avcomp_error decode_video(avcomp_state *state, int width, int height, const UINT8 *source, UINT32 complength, UINT8 *dest, UINT32 dstride, UINT32 dxor);
static avcomp_error decode_video_lossless(avcomp_state *state, int width, int height, const UINT8 *source, UINT32 complength, UINT8 *dest, UINT32 deststride, UINT32 destxor);



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

	/* error if out of range */
	if (maxchannels > MAX_CHANNELS)
		return NULL;

	/* allocate memory for state block */
	state = (avcomp_state *)malloc(sizeof(*state));
	if (state == NULL)
		return NULL;

	/* clear the buffers */
	memset(state, 0, sizeof(*state));

	/* compute the core info */
	state->maxwidth = maxwidth;
	state->maxheight = maxheight;
	state->maxchannels = maxchannels;

	/* now allocate data buffers */
	state->audiodata = (UINT8 *)malloc(65536 * state->maxchannels * 2);
	if (state->audiodata == NULL)
		goto cleanup;

	/* create huffman contexts */
	hufferr = huffman_create_context(&state->ycontext, 16);
	if (hufferr != HUFFERR_NONE)
		goto cleanup;
	hufferr = huffman_create_context(&state->cbcontext, 16);
	if (hufferr != HUFFERR_NONE)
		goto cleanup;
	hufferr = huffman_create_context(&state->crcontext, 16);
	if (hufferr != HUFFERR_NONE)
		goto cleanup;
	hufferr = huffman_create_context(&state->audiohicontext, 16);
	if (hufferr != HUFFERR_NONE)
		goto cleanup;
	hufferr = huffman_create_context(&state->audiolocontext, 16);
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
	if (state->audiodata != NULL)
		free(state->audiodata);

	/* free the contexts */
	if (state->ycontext != NULL)
		huffman_free_context(state->ycontext);
	if (state->cbcontext != NULL)
		huffman_free_context(state->cbcontext);
	if (state->crcontext != NULL)
		huffman_free_context(state->crcontext);
	if (state->audiohicontext != NULL)
		huffman_free_context(state->audiohicontext);
	if (state->audiolocontext != NULL)
		huffman_free_context(state->audiolocontext);

	free(state);
}


/*-------------------------------------------------
    avcomp_config_compress - configure compression
    parameters
-------------------------------------------------*/

void avcomp_config_compress(avcomp_state *state, const av_codec_compress_config *config)
{
	state->compress = *config;
}


/*-------------------------------------------------
    avcomp_config_decompress - configure
    decompression parameters
-------------------------------------------------*/

void avcomp_config_decompress(avcomp_state *state, const av_codec_decompress_config *config)
{
	state->decompress = *config;
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
	const UINT8 *metastart, *videostart, *audiostart[MAX_CHANNELS];
	UINT32 metasize, channels, samples, width, height;
	UINT32 audioxor, videoxor, videostride;
	avcomp_error err;
	UINT32 dstoffs;
	int chnum;

	/* extract data from source if present */
	if (source != NULL)
	{
		/* validate the header */
		if (source[0] != 'c' || source[1] != 'h' || source[2] != 'a' || source[3] != 'v')
			return AVCERR_INVALID_DATA;

		/* extract info from the header */
		metasize = source[4];
		channels = source[5];
		samples = (source[6] << 8) + source[7];
		width = (source[8] << 8) + source[9];
		height = (source[10] << 8) + source[11];

		/* determine the start of each piece of data */
		source += 12;
		metastart = source;
		source += metasize;
		for (chnum = 0; chnum < channels; chnum++)
		{
			audiostart[chnum] = source;
			source += 2 * samples;
		}
		videostart = source;

		/* data is assumed to be big-endian already */
		audioxor = videoxor = 0;
		videostride = 2 * width;
	}

	/* otherwise, extract from the state */
	else
	{
		UINT16 betest = 0;

		/* extract metadata information */
		metastart = state->compress.metadata;
		metasize = state->compress.metalength;
		if ((metastart == NULL && metasize != 0) || (metastart != NULL && metasize == 0))
			return AVCERR_INVALID_CONFIGURATION;

		/* extract audio information */
		channels = state->compress.channels;
		samples = state->compress.samples;
		for (chnum = 0; chnum < channels; chnum++)
			audiostart[chnum] = (const UINT8 *)state->compress.audio[chnum];

		/* extract video information */
		videostart = NULL;
		videostride = width = height = 0;
		if (state->compress.video != NULL)
		{
			videostart = (const UINT8 *)state->compress.video->base;
			videostride = state->compress.video->rowpixels * 2;
			width = state->compress.video->width;
			height = state->compress.video->height;
		}

		/* data is assumed to be native-endian */
		*(UINT8 *)&betest = 1;
		audioxor = videoxor = (betest == 1) ? 1 : 0;
	}

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
	dest[6] = height >> 8;
	dest[7] = height;

	/* starting offsets */
	dstoffs = 10 + 2 * channels;

	/* copy the metadata first */
	if (metasize > 0)
	{
		memcpy(dest + dstoffs, metastart, metasize);
		dstoffs += metasize;
	}

	/* encode the audio channels */
	if (channels > 0)
	{
		/* encode the audio */
		err = encode_audio(state, channels, samples, audiostart, audioxor, dest + dstoffs, &dest[8]);
		if (err != AVCERR_NONE)
			return err;

		/* advance the pointers past the data */
		dstoffs += (dest[8] << 8) + dest[9];
		for (chnum = 0; chnum < channels; chnum++)
			dstoffs += (dest[10 + 2 * chnum] << 8) + dest[11 + 2 * chnum];
	}

	/* encode the video data */
	if (width > 0 && height > 0)
	{
		UINT32 vidlength = 0;

		/* encode the video */
		err = encode_video(state, width, height, videostart, videostride, videoxor, dest + dstoffs, &vidlength);
		if (err != AVCERR_NONE)
			return err;

		/* advance the pointers past the data */
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
	UINT8 *metastart, *videostart, *audiostart[MAX_CHANNELS];
	UINT32 metasize, channels, samples, width, height;
	UINT32 audioxor, videoxor, videostride;
	UINT32 srcoffs, totalsize;
	avcomp_error err;
	int chnum;

	/* extract info from the header */
	if (complength < 8)
		return AVCERR_INVALID_DATA;
	metasize = source[0];
	channels = source[1];
	samples = (source[2] << 8) + source[3];
	width = (source[4] << 8) + source[5];
	height = (source[6] << 8) + source[7];

	/* validate the info from the header */
	if (width > state->maxwidth || height > state->maxheight)
		return AVCERR_VIDEO_TOO_LARGE;
	if (channels > state->maxchannels)
		return AVCERR_AUDIO_TOO_LARGE;

	/* validate that the sizes make sense */
	if (complength < 10 + 2 * channels)
		return AVCERR_INVALID_DATA;
	totalsize = 10 + 2 * channels;
	totalsize += (source[8] << 8) | source[9];
	for (chnum = 0; chnum < channels; chnum++)
		totalsize += (source[10 + 2 * chnum] << 8) | source[11 + 2 * chnum];
	if (totalsize >= complength)
		return AVCERR_INVALID_DATA;

	/* starting offsets */
	srcoffs = 10 + 2 * channels;

	/* if we are decoding raw, set up the output parameters */
	if (dest != NULL)
	{
		/* create a header */
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

		/* determine the start of each piece of data */
		dest += 12;
		metastart = dest;
		dest += metasize;
		for (chnum = 0; chnum < channels; chnum++)
		{
			audiostart[chnum] = dest;
			dest += 2 * samples;
		}
		videostart = dest;

		/* data is assumed to be big-endian already */
		audioxor = videoxor = 0;
		videostride = 2 * width;
	}

	/* otherwise, extract from the state */
	else
	{
		UINT16 betest = 0;

		/* determine the start of each piece of data */
		metastart = state->decompress.metadata;
		for (chnum = 0; chnum < channels; chnum++)
			audiostart[chnum] = (UINT8 *)state->decompress.audio[chnum];
		videostart = (state->decompress.video != NULL) ? (UINT8 *)state->decompress.video->base : NULL;
		videostride = (state->decompress.video != NULL) ? state->decompress.video->rowpixels * 2 : 0;

		/* data is assumed to be native-endian */
		*(UINT8 *)&betest = 1;
		audioxor = videoxor = (betest == 1) ? 1 : 0;

		/* verify against sizes */
		if (state->decompress.video != NULL && (state->decompress.video->width < width || state->decompress.video->height < height))
			return AVCERR_VIDEO_TOO_LARGE;
		for (chnum = 0; chnum < channels; chnum++)
			if (state->decompress.audio[chnum] != NULL && state->decompress.maxsamples < samples)
				return AVCERR_AUDIO_TOO_LARGE;
		if (state->decompress.metadata != NULL && state->decompress.maxmetalength < metasize)
			return AVCERR_METADATA_TOO_LARGE;

		/* set the output values */
		if (state->decompress.actsamples != NULL)
			*state->decompress.actsamples = samples;
		if (state->decompress.actmetalength != NULL)
			*state->decompress.actmetalength = metasize;
	}

	/* copy the metadata first */
	if (metasize > 0)
	{
		if (metastart != NULL)
			memcpy(metastart, source + srcoffs, metasize);
		srcoffs += metasize;
	}

	/* decode the audio channels */
	if (channels > 0)
	{
		/* decode the audio */
		err = decode_audio(state, channels, samples, source + srcoffs, audiostart, audioxor, &source[8]);
		if (err != AVCERR_NONE)
			return err;

		/* advance the pointers past the data */
		srcoffs += (source[8] << 8) + source[9];
		for (chnum = 0; chnum < channels; chnum++)
			srcoffs += (source[10 + 2 * chnum] << 8) + source[11 + 2 * chnum];
	}

	/* decode the video data */
	if (width > 0 && height > 0 && videostart != NULL)
	{
		/* decode the video */
		err = decode_video(state, width, height, source + srcoffs, complength - srcoffs, videostart, videostride, videoxor);
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

static avcomp_error encode_audio(avcomp_state *state, int channels, int samples, const UINT8 **source, int sourcexor, UINT8 *dest, UINT8 *sizes)
{
	UINT32 size, huffsize, totalsize;
	huffman_context *contexts[2];
	huffman_error hufferr;
	UINT8 *output = dest;
	int chnum, sampnum;
	UINT8 *deltabuf;

	/* iterate over channels to compute deltas */
	deltabuf = state->audiodata;
	for (chnum = 0; chnum < channels; chnum++)
	{
		const UINT8 *srcdata = source[chnum];
		INT16 prevsample = 0;

		/* extract audio data into hi and lo deltas stored in big-endian order */
		for (sampnum = 0; sampnum < samples; sampnum++)
		{
			INT16 newsample = (srcdata[0 ^ sourcexor] << 8) | srcdata[1 ^ sourcexor];
			INT16 delta = newsample - prevsample;
			prevsample = newsample;
			*deltabuf++ = delta >> 8;
			*deltabuf++ = delta;
			srcdata += 2;
		}
	}

	/* compute the trees */
	contexts[0] = state->audiohicontext;
	contexts[1] = state->audiolocontext;
	hufferr = huffman_compute_tree_interleaved(2, contexts, state->audiodata, samples * 2, channels, samples * 2, 0);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_COMPRESSION_ERROR;

	/* export them to the output */
	hufferr = huffman_export_tree(state->audiohicontext, output, 256, &size);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_COMPRESSION_ERROR;
	output += size;
	hufferr = huffman_export_tree(state->audiolocontext, output, 256, &size);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_COMPRESSION_ERROR;
	output += size;

	/* note the size of the two trees */
	huffsize = output - dest;
	sizes[0] = huffsize >> 8;
	sizes[1] = huffsize;

	/* iterate over channels */
	totalsize = huffsize;
	for (chnum = 0; chnum < channels; chnum++)
	{
		const UINT8 *input = state->audiodata + chnum * samples * 2;

		/* encode the data */
		hufferr = huffman_encode_data_interleaved(2, contexts, input, samples * 2, 1, 0, 0, output, samples * 2, &size);
		if (hufferr != HUFFERR_NONE)
			return AVCERR_COMPRESSION_ERROR;
		output += size;

		/* store the size of this stream */
		totalsize += size;
		if (totalsize >= channels * samples * 2)
			break;
		sizes[chnum * 2 + 2] = size >> 8;
		sizes[chnum * 2 + 3] = size;
	}

	/* if we ran out of room, throw it all away and just store raw */
	if (chnum < channels)
	{
		memcpy(dest, state->audiodata, channels * samples * 2);
		size = samples * 2;
		sizes[0] = sizes[1] = 0;
		for (chnum = 0; chnum < channels; chnum++)
		{
			sizes[chnum * 2 + 2] = size >> 8;
			sizes[chnum * 2 + 3] = size;
		}
	}

	return AVCERR_NONE;
}


/*-------------------------------------------------
    encode_video - encode raw video data
    to the destination
-------------------------------------------------*/

static avcomp_error encode_video(avcomp_state *state, int width, int height, const UINT8 *source, UINT32 sstride, UINT32 sxor, UINT8 *dest, UINT32 *complength)
{
	/* only lossless supported at this time */
	return encode_video_lossless(state, width, height, source, sstride, sxor, dest, complength);
}


/*-------------------------------------------------
    encode_video_lossless - do a lossless video
    encoding using deltas and huffman encoding
-------------------------------------------------*/

static avcomp_error encode_video_lossless(avcomp_state *state, int width, int height, const UINT8 *source, UINT32 sstride, UINT32 sxor, UINT8 *dest, UINT32 *complength)
{
	UINT32 srcbytes = width * height * 2;
	huffman_context *contexts[4];
	huffman_error hufferr;
	UINT32 outbytes;
	UINT8 *output;

	/* set up the output; first byte is 0x80 to indicate lossless encoding */
	output = dest;
	*output++ = 0x80;

	/* now encode to the destination using two trees, one for the Y and one for the Cr/Cb */
	contexts[0] = state->ycontext;
	contexts[1] = state->cbcontext;
	contexts[2] = state->ycontext;
	contexts[3] = state->crcontext;

	/* compute the histograms for the data */
	hufferr = huffman_deltarle_compute_tree_interleaved(4, contexts, source, width * 2, height, sstride, sxor);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_COMPRESSION_ERROR;

	/* export the trees to the data stream */
	hufferr = huffman_deltarle_export_tree(state->ycontext, output, 256, &outbytes);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_COMPRESSION_ERROR;
	output += outbytes;
	hufferr = huffman_deltarle_export_tree(state->cbcontext, output, 256, &outbytes);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_COMPRESSION_ERROR;
	output += outbytes;
	hufferr = huffman_deltarle_export_tree(state->crcontext, output, 256, &outbytes);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_COMPRESSION_ERROR;
	output += outbytes;

	/* encode the data using the trees */
	hufferr = huffman_deltarle_encode_data_interleaved(4, contexts, source, width * 2, height, sstride, sxor, output, srcbytes, &outbytes);
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

static avcomp_error decode_audio(avcomp_state *state, int channels, int samples, const UINT8 *source, UINT8 **dest, UINT32 dxor, const UINT8 *sizes)
{
	huffman_context *contexts[2];
	UINT32 actsize, huffsize;
	huffman_error hufferr;
	int chnum, sampnum;
	UINT16 size;

	/* if no huffman length, just copy the data */
	size = (sizes[0] << 8) | sizes[1];
	if (size == 0)
	{
		/* loop over channels */
		for (chnum = 0; chnum < channels; chnum++)
		{
			UINT8 *curdest = dest[chnum];

			/* extract the size of this channel */
			size = (sizes[chnum * 2 + 2] << 8) | sizes[chnum * 2 + 3];

			/* extract data from the deltas */
			if (dest[chnum] != NULL)
			{
				INT16 prevsample = 0;
				for (sampnum = 0; sampnum < samples; sampnum++)
				{
					INT16 delta = (source[0] << 8) | source[1];
					INT16 newsample = prevsample + delta;
					prevsample = newsample;

					curdest[0 ^ dxor] = newsample >> 8;
					curdest[1 ^ dxor] = newsample;
					source += 2;
					curdest += 2;
				}
			}
			else
				source += size;
		}
		return AVCERR_NONE;
	}

	/* extract the huffman trees */
	hufferr = huffman_import_tree(state->audiohicontext, source, size, &actsize);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_INVALID_DATA;
	source += actsize;
	huffsize = actsize;

	hufferr = huffman_import_tree(state->audiolocontext, source, size, &actsize);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_INVALID_DATA;
	source += actsize;
	huffsize += actsize;
	if (huffsize != size)
		return AVCERR_INVALID_DATA;

	/* set up the contexts */
	contexts[0] = state->audiohicontext;
	contexts[1] = state->audiolocontext;

	/* now loop over channels and decode their data */
	for (chnum = 0; chnum < channels; chnum++)
	{
		/* extract the size of this channel */
		size = (sizes[chnum * 2 + 2] << 8) | sizes[chnum * 2 + 3];

		/* decode the data */
		if (dest[chnum] != NULL)
		{
			UINT8 *deltabuf = state->audiodata + chnum * samples * 2;
			hufferr = huffman_decode_data_interleaved(2, contexts, source, size, deltabuf, samples * 2, 1, 0, 0, &actsize);
			if (hufferr != HUFFERR_NONE || actsize != size)
				return AVCERR_INVALID_DATA;
		}

		/* advance */
		source += size;
	}

	/* reassemble audio from the deltas */
	for (chnum = 0; chnum < channels; chnum++)
		if (dest[chnum] != NULL)
		{
			UINT8 *deltabuf = state->audiodata + chnum * samples * 2;
			UINT8 *curdest = dest[chnum];
			INT16 prevsample = 0;

			for (sampnum = 0; sampnum < samples; sampnum++)
			{
				INT16 delta = (deltabuf[0] << 8) | deltabuf[1];
				INT16 newsample = prevsample + delta;
				prevsample = newsample;

				curdest[0 ^ dxor] = newsample >> 8;
				curdest[1 ^ dxor] = newsample;
				deltabuf += 2;
				curdest += 2;
			}
		}

	return AVCERR_NONE;
}


/*-------------------------------------------------
    decode_video - decode video from a
    compressed data stream
-------------------------------------------------*/

static avcomp_error decode_video(avcomp_state *state, int width, int height, const UINT8 *source, UINT32 complength, UINT8 *dest, UINT32 dstride, UINT32 dxor)
{
	/* if the high bit of the first byte is set, we decode losslessly */
	if (source[0] & 0x80)
		return decode_video_lossless(state, width, height, source, complength, dest, dstride, dxor);
	else
		return AVCERR_INVALID_DATA;
}


/*-------------------------------------------------
    decode_video_lossless - do a lossless video
    decoding using deltas and huffman encoding
-------------------------------------------------*/

static avcomp_error decode_video_lossless(avcomp_state *state, int width, int height, const UINT8 *source, UINT32 complength, UINT8 *dest, UINT32 deststride, UINT32 destxor)
{
	const UINT8 *sourceend = source + complength;
	huffman_context *contexts[4];
	huffman_error hufferr;
	UINT32 actsize;

	/* skip the first byte */
	source++;

	/* import the tables */
	hufferr = huffman_deltarle_import_tree(state->ycontext, source, sourceend - source, &actsize);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_INVALID_DATA;
	source += actsize;
	hufferr = huffman_deltarle_import_tree(state->cbcontext, source, sourceend - source, &actsize);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_INVALID_DATA;
	source += actsize;
	hufferr = huffman_deltarle_import_tree(state->crcontext, source, sourceend - source, &actsize);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_INVALID_DATA;
	source += actsize;

	/* set up the decoding contexts */
	contexts[0] = state->ycontext;
	contexts[1] = state->cbcontext;
	contexts[2] = state->ycontext;
	contexts[3] = state->crcontext;

	/* decode to the destination */
	hufferr = huffman_deltarle_decode_data_interleaved(4, contexts, source, sourceend - source, dest, width * 2, height, deststride, destxor, &actsize);
	if (hufferr != HUFFERR_NONE)
		return AVCERR_INVALID_DATA;
	if (actsize != sourceend - source)
		return AVCERR_INVALID_DATA;

	return AVCERR_NONE;
}
