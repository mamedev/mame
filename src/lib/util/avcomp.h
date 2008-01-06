/***************************************************************************

    avcomp.h

    Audio/video compression and decompression helpers.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#ifndef __AVCOMP_H__

#include "osdcore.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* errors */
enum  _avcomp_error
{
	AVCERR_NONE = 0,
	AVCERR_INVALID_DATA,
	AVCERR_VIDEO_TOO_LARGE,
	AVCERR_AUDIO_TOO_LARGE,
	AVCERR_OUT_OF_MEMORY,
	AVCERR_COMPRESSION_ERROR
};
typedef enum _avcomp_error avcomp_error;

/* default decompression parameters */
#define AVCOMP_DECODE_META					(1 << 0)
#define AVCOMP_DECODE_VIDEO					(1 << 1)
#define AVCOMP_DECODE_AUDIO(x)				(1 << (2 + (x)))
#define AVCOMP_DECODE_DEFAULT				(~0)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _avcomp_state avcomp_state;



/***************************************************************************
    PROTOTYPES
***************************************************************************/

avcomp_state *avcomp_init(UINT32 maxwidth, UINT32 maxheight, UINT32 maxchannels);
void avcomp_free(avcomp_state *state);
void avcomp_decompress_config(avcomp_state *state, UINT32 decodemask, UINT8 *videobuffer, UINT32 videostride, UINT32 videoxor, UINT32 audioxor);

avcomp_error avcomp_encode_data(avcomp_state *state, const UINT8 *source, UINT8 *dest, UINT32 *complength);
avcomp_error avcomp_decode_data(avcomp_state *state, const UINT8 *source, UINT32 complength, UINT8 *dest);

#endif
