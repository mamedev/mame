/*
    CD-DA "Red Book" audio sound hardware handler
    Relies on the actual CD logic and reading in cdrom.c.
*/

#include "sndintrf.h"
#include "streams.h"
#include "cpuintrf.h"
#include "cdrom.h"
#include "cdda.h"

typedef struct _cdda_info cdda_info;
struct _cdda_info
{
	sound_stream *		stream;
	cdrom_file *		disc;

	INT8				audio_playing, audio_pause, audio_ended_normally;
	UINT32				audio_lba, audio_length;

	UINT8 *				audio_cache;
	UINT32				audio_samples;
	UINT32				audio_bptr;
};

#define MAX_SECTORS ( 4 )

static void get_audio_data(cdda_info *info, stream_sample_t *bufL, stream_sample_t *bufR, UINT32 samples_wanted);


/*-------------------------------------------------
    cdda_update - stream update callback
-------------------------------------------------*/

static void cdda_update(void *param, stream_sample_t **inputs, stream_sample_t **outputs, int length)
{
	cdda_info *info = param;
	get_audio_data(info, &outputs[0][0], &outputs[1][0], length);
}


/*-------------------------------------------------
    cdda_start - audio start callback
-------------------------------------------------*/

static void *cdda_start(int sndindex, int clock, const void *config)
{
	const struct CDDAinterface *intf;
	cdda_info *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	/* allocate an audio cache */
	info->audio_cache = auto_malloc( CD_MAX_SECTOR_DATA * MAX_SECTORS );

	intf = config;

	info->stream = stream_create(0, 2, 44100, info, cdda_update);

	state_save_register_item( "CDDA", sndindex, info->audio_playing );
	state_save_register_item( "CDDA", sndindex, info->audio_pause );
	state_save_register_item( "CDDA", sndindex, info->audio_ended_normally );
	state_save_register_item( "CDDA", sndindex, info->audio_lba );
	state_save_register_item( "CDDA", sndindex, info->audio_length );
	state_save_register_item_pointer( "CDDA", sndindex, info->audio_cache, CD_MAX_SECTOR_DATA * MAX_SECTORS );
	state_save_register_item( "CDDA", sndindex, info->audio_samples );
	state_save_register_item( "CDDA", sndindex, info->audio_bptr );

	return info;
}


/*-------------------------------------------------
    cdda_set_cdrom - set the CD-ROM file for the
    given CDDA stream
-------------------------------------------------*/

void cdda_set_cdrom(int num, void *file)
{
	cdda_info *info = sndti_token(SOUND_CDDA, num);
	info->disc = (cdrom_file *)file;
}


/*-------------------------------------------------
    cdda_num_from_cdrom - find the CDDA stream
    that references the given CD-ROM file
-------------------------------------------------*/

int cdda_num_from_cdrom(void *file)
{
	int index;

	for (index = 0; ; index++)
	{
		cdda_info *info;

		if (!sndti_exists(SOUND_CDDA, index))
			return -1;

		info = sndti_token(SOUND_CDDA, index);

		if (info == NULL)
			return -1;
		if (info->disc == file)
			return index;
	}
	return 0;
}


/*-------------------------------------------------
    cdda_start_audio - begin playback of a Red
    Book audio track
-------------------------------------------------*/

void cdda_start_audio(int num, UINT32 startlba, UINT32 numblocks)
{
	cdda_info *info = sndti_token(SOUND_CDDA, num);

	stream_update(info->stream);
	info->audio_playing = TRUE;
	info->audio_pause = FALSE;
	info->audio_ended_normally = FALSE;
	info->audio_lba = startlba;
	info->audio_length = numblocks;
}


/*-------------------------------------------------
    cdda_stop_audio - stop playback of a Red Book
    audio track
-------------------------------------------------*/

void cdda_stop_audio(int num)
{
	cdda_info *info = sndti_token(SOUND_CDDA, num);

	stream_update(info->stream);
	info->audio_playing = FALSE;
	info->audio_ended_normally = TRUE;
}


/*-------------------------------------------------
    cdda_pause_audio - pause/unpause playback of
    a Red Book audio track
-------------------------------------------------*/

void cdda_pause_audio(int num, int pause)
{
	cdda_info *info = sndti_token(SOUND_CDDA, num);

	stream_update(info->stream);
	info->audio_pause = pause;
}


/*-------------------------------------------------
    cdda_get_audio_lba - returns the current LBA
    (physical sector) during Red Book playback
-------------------------------------------------*/

UINT32 cdda_get_audio_lba(int num)
{
	cdda_info *info = sndti_token(SOUND_CDDA, num);

	stream_update(info->stream);
	return info->audio_lba;
}


/*-------------------------------------------------
    cdda_audio_active - returns Red Book audio
    playback status
-------------------------------------------------*/

int cdda_audio_active(int num)
{
	cdda_info *info = sndti_token(SOUND_CDDA, num);

	stream_update(info->stream);
	return info->audio_playing;
}


/*-------------------------------------------------
    cdda_audio_paused - returns if Red Book
    playback is paused
-------------------------------------------------*/

int cdda_audio_paused(int num)
{
	cdda_info *info = sndti_token(SOUND_CDDA, num);
	return info->audio_pause;
}


/*-------------------------------------------------
    cdda_audio_ended - returns if a Red Book
    track reached it's natural end
-------------------------------------------------*/

int cdda_audio_ended(int num)
{
	cdda_info *info = sndti_token(SOUND_CDDA, num);
	return info->audio_ended_normally;
}


/*-------------------------------------------------
    get_audio_data - reads Red Book data off
    the disc if playback is in progress and
    converts it to 2 16-bit 44.1 kHz streams
-------------------------------------------------*/

static void get_audio_data(cdda_info *info, stream_sample_t *bufL, stream_sample_t *bufR, UINT32 samples_wanted)
{
	int i, sectoread, remaining;
	INT16 *audio_cache = (INT16 *) info->audio_cache;

	/* if no file, audio not playing, audio paused, or out of disc data,
       just zero fill */
	if (!info->disc || !info->audio_playing || info->audio_pause || (!info->audio_length && !info->audio_samples))
	{
		if( info->disc && info->audio_playing && !info->audio_pause && !info->audio_length )
		{
			info->audio_playing = FALSE;
			info->audio_ended_normally = TRUE;
		}

		memset(bufL, 0, sizeof(stream_sample_t)*samples_wanted);
		memset(bufR, 0, sizeof(stream_sample_t)*samples_wanted);
		return;
	}

	/* if we've got enough samples, just feed 'em out */
	if (samples_wanted <= info->audio_samples)
	{
		for (i = 0; i < samples_wanted; i++)
		{
			*bufL++ = audio_cache[ info->audio_bptr++ ];
			*bufR++ = audio_cache[ info->audio_bptr++ ];
		}

		info->audio_samples -= samples_wanted;
		return;
	}

	/* we don't have enough, so first feed what we've got */
	for (i = 0; i < info->audio_samples; i++)
	{
		*bufL++ = audio_cache[ info->audio_bptr++ ];
		*bufR++ = audio_cache[ info->audio_bptr++ ];
	}

	/* remember how much left for later */
	remaining = samples_wanted - info->audio_samples;

	/* reset the buffer and get what we can from the disc */
	info->audio_samples = 0;
	if (info->audio_length >= MAX_SECTORS)
	{
		sectoread = MAX_SECTORS;
	}
	else
	{
		sectoread = info->audio_length;
	}

	for (i = 0; i < sectoread; i++)
	{
		cdrom_read_data(info->disc, info->audio_lba, &info->audio_cache[CD_MAX_SECTOR_DATA*i], CD_TRACK_AUDIO);

		info->audio_lba++;
	}

	info->audio_samples = (CD_MAX_SECTOR_DATA*sectoread)/4;
	info->audio_length -= sectoread;

	/* CD-DA data on the disc is big-endian, flip if we're not */
	#ifdef LSB_FIRST
	for( i = 0; i < info->audio_samples * 2; i++ )
	{
		audio_cache[ i ] = BIG_ENDIANIZE_INT16( audio_cache[ i ] );
	}
	#endif

	/* reset feedout ptr */
	info->audio_bptr = 0;

	/* we've got data, feed it out by calling ourselves recursively */
	get_audio_data(info, bufL, bufR, remaining);
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void cdda_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void cdda_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = cdda_set_info;			break;
		case SNDINFO_PTR_START:							info->start = cdda_start;				break;
		case SNDINFO_PTR_STOP:							/* nothing */							break;
		case SNDINFO_PTR_RESET:							/* nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "CD/DA";						break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "CD Audio";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

