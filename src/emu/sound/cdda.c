/*
    CD-DA "Red Book" audio sound hardware handler
    Relies on the actual CD logic and reading in cdrom.c.
*/

#include "emu.h"
#include "streams.h"
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

INLINE cdda_info *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == CDDA);
	return (cdda_info *)downcast<legacy_device_base *>(device)->token();
}

#define MAX_SECTORS ( 4 )

static void get_audio_data(cdda_info *info, stream_sample_t *bufL, stream_sample_t *bufR, UINT32 samples_wanted);


/*-------------------------------------------------
    cdda_update - stream update callback
-------------------------------------------------*/

static STREAM_UPDATE( cdda_update )
{
	cdda_info *info = (cdda_info *)param;
	get_audio_data(info, &outputs[0][0], &outputs[1][0], samples);
}


/*-------------------------------------------------
    DEVICE_START( cdda ) - audio start callback
-------------------------------------------------*/

static DEVICE_START( cdda )
{
	//const struct CDDAinterface *intf;
	cdda_info *info = get_safe_token(device);

	/* allocate an audio cache */
	info->audio_cache = auto_alloc_array( device->machine, UINT8, CD_MAX_SECTOR_DATA * MAX_SECTORS );

	//intf = (const struct CDDAinterface *)device->baseconfig().static_config();

	info->stream = stream_create(device, 0, 2, 44100, info, cdda_update);

	state_save_register_device_item( device, 0, info->audio_playing );
	state_save_register_device_item( device, 0, info->audio_pause );
	state_save_register_device_item( device, 0, info->audio_ended_normally );
	state_save_register_device_item( device, 0, info->audio_lba );
	state_save_register_device_item( device, 0, info->audio_length );
	state_save_register_device_item_pointer( device, 0, info->audio_cache, CD_MAX_SECTOR_DATA * MAX_SECTORS );
	state_save_register_device_item( device, 0, info->audio_samples );
	state_save_register_device_item( device, 0, info->audio_bptr );
}


/*-------------------------------------------------
    cdda_set_cdrom - set the CD-ROM file for the
    given CDDA stream
-------------------------------------------------*/

void cdda_set_cdrom(device_t *device, void *file)
{
	cdda_info *info = get_safe_token(device);
	info->disc = (cdrom_file *)file;
}


/*-------------------------------------------------
    cdda_from_cdrom - find the CDDA stream
    that references the given CD-ROM file
-------------------------------------------------*/

device_t *cdda_from_cdrom(running_machine *machine, void *file)
{
	device_sound_interface *sound = NULL;

	for (bool gotone = machine->m_devicelist.first(sound); gotone; gotone = sound->next(sound))
		if (sound->device().type() == CDDA)
		{
			cdda_info *info = get_safe_token(*sound);
			if (info->disc == file)
				return *sound;
		}

	return NULL;
}


/*-------------------------------------------------
    cdda_start_audio - begin playback of a Red
    Book audio track
-------------------------------------------------*/

void cdda_start_audio(device_t *device, UINT32 startlba, UINT32 numblocks)
{
	cdda_info *info = get_safe_token(device);

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

void cdda_stop_audio(device_t *device)
{
	cdda_info *info = get_safe_token(device);

	stream_update(info->stream);
	info->audio_playing = FALSE;
	info->audio_ended_normally = TRUE;
}


/*-------------------------------------------------
    cdda_pause_audio - pause/unpause playback of
    a Red Book audio track
-------------------------------------------------*/

void cdda_pause_audio(device_t *device, int pause)
{
	cdda_info *info = get_safe_token(device);

	stream_update(info->stream);
	info->audio_pause = pause;
}


/*-------------------------------------------------
    cdda_get_audio_lba - returns the current LBA
    (physical sector) during Red Book playback
-------------------------------------------------*/

UINT32 cdda_get_audio_lba(device_t *device)
{
	cdda_info *info = get_safe_token(device);

	stream_update(info->stream);
	return info->audio_lba;
}


/*-------------------------------------------------
    cdda_audio_active - returns Red Book audio
    playback status
-------------------------------------------------*/

int cdda_audio_active(device_t *device)
{
	cdda_info *info = get_safe_token(device);

	stream_update(info->stream);
	return info->audio_playing;
}


/*-------------------------------------------------
    cdda_audio_paused - returns if Red Book
    playback is paused
-------------------------------------------------*/

int cdda_audio_paused(device_t *device)
{
	cdda_info *info = get_safe_token(device);
	return info->audio_pause;
}


/*-------------------------------------------------
    cdda_audio_ended - returns if a Red Book
    track reached it's natural end
-------------------------------------------------*/

int cdda_audio_ended(device_t *device)
{
	cdda_info *info = get_safe_token(device);
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
	if (ENDIANNESS_NATIVE == ENDIANNESS_LITTLE)
	{
		for( i = 0; i < info->audio_samples * 2; i++ )
		{
			audio_cache[ i ] = BIG_ENDIANIZE_INT16( audio_cache[ i ] );
		}
	}

	/* reset feedout ptr */
	info->audio_bptr = 0;

	/* we've got data, feed it out by calling ourselves recursively */
	get_audio_data(info, bufL, bufR, remaining);
}

/*-------------------------------------------------
    cdda_set_volume - sets CD-DA volume level
    for both speakers, used for fade in/out effects
-------------------------------------------------*/

void cdda_set_volume(device_t *device,int volume)
{
	cdda_info *cdda = get_safe_token(device);

	stream_set_output_gain(cdda->stream,0,volume / 100.0);
	stream_set_output_gain(cdda->stream,1,volume / 100.0);
}

/*-------------------------------------------------
    cdda_set_channel_volume - sets CD-DA volume level
    for either speaker, used for fade in/out effects
-------------------------------------------------*/

void cdda_set_channel_volume(device_t *device, int channel, int volume)
{
	cdda_info *cdda = get_safe_token(device);

	if(channel == 0)
		stream_set_output_gain(cdda->stream,0,volume / 100.0);
	if(channel == 1)
		stream_set_output_gain(cdda->stream,1,volume / 100.0);
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( cdda )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(cdda_info);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( cdda );		break;
		case DEVINFO_FCT_STOP:							/* nothing */								break;
		case DEVINFO_FCT_RESET:							/* nothing */								break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "CD/DA");					break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "CD Audio");				break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");						break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(CDDA, cdda);
