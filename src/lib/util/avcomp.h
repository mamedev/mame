/***************************************************************************

    avcomp.h

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

***************************************************************************/

#ifndef __AVCOMP_H__
#define __AVCOMP_H__

#include "osdcore.h"
#include "bitmap.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* errors */
enum _avcomp_error
{
	AVCERR_NONE = 0,
	AVCERR_INVALID_DATA,
	AVCERR_VIDEO_TOO_LARGE,
	AVCERR_AUDIO_TOO_LARGE,
	AVCERR_METADATA_TOO_LARGE,
	AVCERR_OUT_OF_MEMORY,
	AVCERR_COMPRESSION_ERROR,
	AVCERR_TOO_MANY_CHANNELS,
	AVCERR_INVALID_CONFIGURATION
};
typedef enum _avcomp_error avcomp_error;

/* default decompression parameters */
#define AVCOMP_ENABLE_META					(1 << 0)
#define AVCOMP_ENABLE_VIDEO					(1 << 1)
#define AVCOMP_ENABLE_AUDIO(x)				(1 << (2 + (x)))
#define AVCOMP_ENABLE_DEFAULT				(~0)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* compression configuration */
typedef struct _av_codec_compress_config av_codec_compress_config;
struct _av_codec_compress_config
{
	bitmap_t *	video;						/* pointer to video bitmap */
	UINT32		channels;					/* number of channels */
	UINT32		samples;					/* number of samples per channel */
	INT16 *		audio[16];					/* pointer to individual audio channels */
	UINT32		metalength;					/* length of metadata */
	UINT8 *		metadata;					/* pointer to metadata buffer */
};


/* decompression configuration */
typedef struct _av_codec_decompress_config av_codec_decompress_config;
struct _av_codec_decompress_config
{
	bitmap_t *	video;						/* pointer to video bitmap */
	UINT32		maxsamples;					/* maximum number of samples per channel */
	UINT32 *	actsamples;					/* actual number of samples per channel */
	INT16 *		audio[16];					/* pointer to individual audio channels */
	UINT32		maxmetalength;				/* maximum length of metadata */
	UINT32 *	actmetalength;				/* actual length of metadata */
	UINT8 *		metadata;					/* pointer to metadata buffer */
};


/* opaque state */
typedef struct _avcomp_state avcomp_state;



/***************************************************************************
    PROTOTYPES
***************************************************************************/

avcomp_state *avcomp_init(UINT32 maxwidth, UINT32 maxheight, UINT32 maxchannels);
void avcomp_free(avcomp_state *state);

void avcomp_config_compress(avcomp_state *state, const av_codec_compress_config *config);
void avcomp_config_decompress(avcomp_state *state, const av_codec_decompress_config *config);

avcomp_error avcomp_encode_data(avcomp_state *state, const UINT8 *source, UINT8 *dest, UINT32 *complength);
avcomp_error avcomp_decode_data(avcomp_state *state, const UINT8 *source, UINT32 complength, UINT8 *dest);

#endif
