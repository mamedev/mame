// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    aviio.h

    AVI movie format parsing helpers.

***************************************************************************/

#ifndef MAME_LIB_UTIL_AVIIO_H
#define MAME_LIB_UTIL_AVIIO_H

#include "bitmap.h"

#include <cstdint>
#include <memory>
#include <string>


/***************************************************************************
    MACROS
***************************************************************************/

#define AVI_FOURCC(a,b,c,d)     ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))

#define FORMAT_UYVY             AVI_FOURCC('U','Y','V','Y')
#define FORMAT_VYUY             AVI_FOURCC('V','Y','U','Y')
#define FORMAT_YUY2             AVI_FOURCC('Y','U','Y','2')
#define FORMAT_HFYU             AVI_FOURCC('H','F','Y','U')
#define FORMAT_I420             AVI_FOURCC('I','4','2','0')
#define FORMAT_DIB              AVI_FOURCC('D','I','B',' ')
#define FORMAT_RGB              AVI_FOURCC('R','G','B',' ')
#define FORMAT_RAW              AVI_FOURCC('R','A','W',' ')
#define FORMAT_UNCOMPRESSED     0x00000000


class avi_file
{
public:

	/***********************************************************************
	    CONSTANTS
	***********************************************************************/

	enum class error
	{
		NONE = 0,
		END,
		INVALID_DATA,
		NO_MEMORY,
		READ_ERROR,
		WRITE_ERROR,
		STACK_TOO_DEEP,
		UNSUPPORTED_FEATURE,
		CANT_OPEN_FILE,
		INCOMPATIBLE_AUDIO_STREAMS,
		INVALID_SAMPLERATE,
		INVALID_STREAM,
		INVALID_FRAME,
		INVALID_BITMAP,
		UNSUPPORTED_VIDEO_FORMAT,
		UNSUPPORTED_AUDIO_FORMAT,
		EXCEEDED_SOUND_BUFFER
	};

	enum class datatype
	{
		VIDEO,
		AUDIO_CHAN0,
		AUDIO_CHAN1,
		AUDIO_CHAN2,
		AUDIO_CHAN3,
		AUDIO_CHAN4,
		AUDIO_CHAN5,
		AUDIO_CHAN6,
		AUDIO_CHAN7
	};


	/***********************************************************************
	    TYPE DEFINITIONS
	***********************************************************************/

	struct movie_info
	{
		std::uint32_t   video_format;               // format of video data
		std::uint32_t   video_timescale;            // timescale for video data
		std::uint32_t   video_sampletime;           // duration of a single video sample (frame)
		std::uint32_t   video_numsamples;           // total number of video samples
		std::uint32_t   video_width;                // width of the video
		std::uint32_t   video_height;               // height of the video
		std::uint32_t   video_depth;                // depth of the video

		std::uint32_t   audio_format;               // format of audio data
		std::uint32_t   audio_timescale;            // timescale for audio data
		std::uint32_t   audio_sampletime;           // duration of a single audio sample
		std::uint32_t   audio_numsamples;           // total number of audio samples
		std::uint32_t   audio_channels;             // number of audio channels
		std::uint32_t   audio_samplebits;           // number of bits per channel
		std::uint32_t   audio_samplerate;           // sample rate of audio
	};

	typedef std::unique_ptr<avi_file> ptr;


	/***********************************************************************
	    PROTOTYPES
	***********************************************************************/

	static error open(std::string const &filename, ptr &file);
	static error create(std::string const &filename, movie_info const &info, ptr &file);
	virtual ~avi_file();

	virtual void printf_chunks() = 0;
	static const char *error_string(error err);

	virtual movie_info const &get_movie_info() const = 0;
	virtual std::uint32_t first_sample_in_frame(std::uint32_t framenum) const = 0;

	virtual error read_uncompressed_video_frame(std::uint32_t framenum, bitmap_argb32 &bitmap) = 0;
	virtual error read_video_frame(std::uint32_t framenum, bitmap_yuy16 &bitmap) = 0;
	virtual error read_sound_samples(int channel, std::uint32_t firstsample, std::uint32_t numsamples, std::int16_t *output) = 0;

	virtual error append_video_frame(bitmap_yuy16 &bitmap) = 0;
	virtual error append_video_frame(bitmap_rgb32 &bitmap) = 0;
	virtual error append_sound_samples(int channel, std::int16_t const *samples, std::uint32_t numsamples, std::uint32_t sampleskip) = 0;

protected:
	avi_file();
};

#endif // MAME_LIB_UTIL_AVIIO_H
