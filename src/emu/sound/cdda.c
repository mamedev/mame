/*
    CD-DA "Red Book" audio sound hardware handler
    Relies on the actual CD logic and reading in cdrom.c.
*/

#include "emu.h"
#include "cdrom.h"
#include "cdda.h"

struct cdda_info
{
	sound_stream *		stream;
	cdrom_file *		disc;

	INT8				audio_playing, audio_pause, audio_ended_normally;
	UINT32				audio_lba, audio_length;

	UINT8 *				audio_cache;
	UINT32				audio_samples;
	UINT32				audio_bptr;
	INT16				audio_volume[2];
};

INLINE cdda_info *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == CDDA);
	return (cdda_info *)downcast<cdda_device *>(device)->token();
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
	info->audio_volume[0] = (INT16)outputs[0][0];
	info->audio_volume[1] = (INT16)outputs[1][0];
}


/*-------------------------------------------------
    DEVICE_START( cdda ) - audio start callback
-------------------------------------------------*/

static DEVICE_START( cdda )
{
	//const struct CDDAinterface *intf;
	cdda_info *info = get_safe_token(device);

	/* allocate an audio cache */
	info->audio_cache = auto_alloc_array( device->machine(), UINT8, CD_MAX_SECTOR_DATA * MAX_SECTORS );

	//intf = (const struct CDDAinterface *)device->static_config();

	info->stream = device->machine().sound().stream_alloc(*device, 0, 2, 44100, info, cdda_update);

	device->save_item( NAME(info->audio_playing) );
	device->save_item( NAME(info->audio_pause) );
	device->save_item( NAME(info->audio_ended_normally) );
	device->save_item( NAME(info->audio_lba) );
	device->save_item( NAME(info->audio_length) );
	device->save_pointer( NAME(info->audio_cache), CD_MAX_SECTOR_DATA * MAX_SECTORS );
	device->save_item( NAME(info->audio_samples) );
	device->save_item( NAME(info->audio_bptr) );
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

device_t *cdda_from_cdrom(running_machine &machine, void *file)
{
	sound_interface_iterator iter(machine.root_device());
	for (device_sound_interface *sound = iter.first(); sound != NULL; sound = iter.next())
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

	info->stream->update();
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

	info->stream->update();
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

	info->stream->update();
	info->audio_pause = pause;
}


/*-------------------------------------------------
    cdda_get_audio_lba - returns the current LBA
    (physical sector) during Red Book playback
-------------------------------------------------*/

UINT32 cdda_get_audio_lba(device_t *device)
{
	cdda_info *info = get_safe_token(device);

	info->stream->update();
	return info->audio_lba;
}


/*-------------------------------------------------
    cdda_audio_active - returns Red Book audio
    playback status
-------------------------------------------------*/

int cdda_audio_active(device_t *device)
{
	cdda_info *info = get_safe_token(device);

	info->stream->update();
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

	cdda->stream->set_output_gain(0,volume / 100.0);
	cdda->stream->set_output_gain(1,volume / 100.0);
}

/*-------------------------------------------------
    cdda_set_channel_volume - sets CD-DA volume level
    for either speaker, used for fade in/out effects
-------------------------------------------------*/

void cdda_set_channel_volume(device_t *device, int channel, int volume)
{
	cdda_info *cdda = get_safe_token(device);

	cdda->stream->set_output_gain(channel,volume / 100.0);
}


/*-------------------------------------------------
    cdda_get_channel_volume - sets CD-DA volume level
    for either speaker, used for volume control display
-------------------------------------------------*/

INT16 cdda_get_channel_volume(device_t *device, int channel)
{
	cdda_info *cdda = get_safe_token(device);

	return cdda->audio_volume[channel];
}

const device_type CDDA = &device_creator<cdda_device>;

cdda_device::cdda_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CDDA, "CD/DA", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(cdda_info);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void cdda_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdda_device::device_start()
{
	DEVICE_START_NAME( cdda )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void cdda_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


