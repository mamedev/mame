/***************************************************************************

    avcomp.h

    Audio/video compression and decompression helpers.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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
