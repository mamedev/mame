// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    aviio.h

    AVI movie format parsing helpers.

***************************************************************************/

#ifndef __AVIIO_H__
#define __AVIIO_H__

#include "osdcore.h"
#include "bitmap.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

enum avi_error
{
	AVIERR_NONE = 0,
	AVIERR_END,
	AVIERR_INVALID_DATA,
	AVIERR_NO_MEMORY,
	AVIERR_READ_ERROR,
	AVIERR_WRITE_ERROR,
	AVIERR_STACK_TOO_DEEP,
	AVIERR_UNSUPPORTED_FEATURE,
	AVIERR_CANT_OPEN_FILE,
	AVIERR_INCOMPATIBLE_AUDIO_STREAMS,
	AVIERR_INVALID_SAMPLERATE,
	AVIERR_INVALID_STREAM,
	AVIERR_INVALID_FRAME,
	AVIERR_INVALID_BITMAP,
	AVIERR_UNSUPPORTED_VIDEO_FORMAT,
	AVIERR_UNSUPPORTED_AUDIO_FORMAT,
	AVIERR_EXCEEDED_SOUND_BUFFER
};


enum avi_datatype
{
	AVIDATA_VIDEO,
	AVIDATA_AUDIO_CHAN0,
	AVIDATA_AUDIO_CHAN1,
	AVIDATA_AUDIO_CHAN2,
	AVIDATA_AUDIO_CHAN3,
	AVIDATA_AUDIO_CHAN4,
	AVIDATA_AUDIO_CHAN5,
	AVIDATA_AUDIO_CHAN6,
	AVIDATA_AUDIO_CHAN7
};



/***************************************************************************
    MACROS
***************************************************************************/

#define AVI_FOURCC(a,b,c,d)     ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))

#define FORMAT_UYVY             AVI_FOURCC('U','Y','V','Y')
#define FORMAT_VYUY             AVI_FOURCC('V','Y','U','Y')
#define FORMAT_YUY2             AVI_FOURCC('Y','U','Y','2')
#define FORMAT_HFYU             AVI_FOURCC('H','F','Y','U')



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct avi_file;


struct avi_movie_info
{
	UINT32          video_format;               /* format of video data */
	UINT32          video_timescale;            /* timescale for video data */
	UINT32          video_sampletime;           /* duration of a single video sample (frame) */
	UINT32          video_numsamples;           /* total number of video samples */
	UINT32          video_width;                /* width of the video */
	UINT32          video_height;               /* height of the video */
	UINT32          video_depth;                /* depth of the video */

	UINT32          audio_format;               /* format of audio data */
	UINT32          audio_timescale;            /* timescale for audio data */
	UINT32          audio_sampletime;           /* duration of a single audio sample */
	UINT32          audio_numsamples;           /* total number of audio samples */
	UINT32          audio_channels;             /* number of audio channels */
	UINT32          audio_samplebits;           /* number of bits per channel */
	UINT32          audio_samplerate;           /* sample rate of audio */
};



/***************************************************************************
    PROTOTYPES
***************************************************************************/

avi_error avi_open(const char *filename, avi_file **file);
avi_error avi_create(const char *filename, const avi_movie_info *info, avi_file **file);
avi_error avi_close(avi_file *file);

void avi_printf_chunks(avi_file *file);
const char *avi_error_string(avi_error err);

const avi_movie_info *avi_get_movie_info(avi_file *file);
UINT32 avi_first_sample_in_frame(avi_file *file, UINT32 framenum);

avi_error avi_read_video_frame(avi_file *file, UINT32 framenum, bitmap_yuy16 &bitmap);
avi_error avi_read_sound_samples(avi_file *file, int channel, UINT32 firstsample, UINT32 numsamples, INT16 *output);

avi_error avi_append_video_frame(avi_file *file, bitmap_yuy16 &bitmap);
avi_error avi_append_video_frame(avi_file *file, bitmap_rgb32 &bitmap);
avi_error avi_append_sound_samples(avi_file *file, int channel, const INT16 *samples, UINT32 numsamples, UINT32 sampleskip);

#endif
