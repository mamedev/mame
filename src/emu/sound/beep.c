/***************************************************************************

    beep.c

    This is used for computers/systems which can only output a constant tone.
    This tone can be turned on and off.
    e.g. PCW and PCW16 computer systems
    KT - 25-Jun-2000

    Sound handler

****************************************************************************/

#include "emu.h"
#include "sound/beep.h"


#define BEEP_RATE			48000

typedef struct _beep_state beep_state;
struct _beep_state
{
	sound_stream *stream;	/* stream number */
	int enable; 			/* enable beep */
	int frequency;			/* set frequency - this can be changed using the appropiate function */
	int incr;				/* initial wave state */
	INT16 signal;			/* current signal */
};


INLINE beep_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == BEEP);
	return (beep_state *)downcast<beep_device *>(device)->token();
}



/*************************************
 *
 *  Stream updater
 *
 *************************************/

static STREAM_UPDATE( beep_sound_update )
{
	beep_state *bs = (beep_state *) param;
	stream_sample_t *buffer = outputs[0];
	INT16 signal = bs->signal;
	int clock = 0, rate = BEEP_RATE / 2;

    /* get progress through wave */
	int incr = bs->incr;

	if (bs->frequency > 0)
		clock = bs->frequency;

	/* if we're not enabled, just fill with 0 */
	if ( !bs->enable || clock == 0 )
	{
		memset( buffer, 0, samples * sizeof(*buffer) );
		return;
	}

	/* fill in the sample */
	while( samples-- > 0 )
	{
		*buffer++ = signal;
		incr -= clock;
		while( incr < 0 )
		{
			incr += rate;
			signal = -signal;
		}
	}

	/* store progress through wave */
	bs->incr = incr;
	bs->signal = signal;
}



/*************************************
 *
 *  Sound handler start
 *
 *************************************/

static DEVICE_START( beep )
{
	beep_state *pBeep = get_safe_token(device);

	pBeep->stream = device->machine().sound().stream_alloc(*device, 0, 1, BEEP_RATE, pBeep, beep_sound_update );
	pBeep->enable = 0;
	pBeep->frequency = 3250;
	pBeep->incr = 0;
	pBeep->signal = 0x07fff;
}



/*************************************
 *
 *  changing state to on from off will restart tone
 *
 *************************************/

void beep_set_state(device_t *device, int on)
{
	beep_state *info = get_safe_token(device);

	/* only update if new state is not the same as old state */
	if (info->enable == on)
		return;

	info->stream->update();

	info->enable = on;
	/* restart wave from beginning */
	info->incr = 0;
	info->signal = 0x07fff;
}



/*************************************
 *
 *  setting new frequency starts from beginning
 *
 *************************************/

void beep_set_frequency(device_t *device,int frequency)
{
	beep_state *info = get_safe_token(device);

	if (info->frequency == frequency)
		return;

	info->stream->update();
	info->frequency = frequency;
	info->signal = 0x07fff;
	info->incr = 0;
}



/*************************************
 *
 *  change a channel volume
 *
 *************************************/

void beep_set_volume(device_t *device, int volume)
{
	beep_state *info = get_safe_token(device);

	info->stream->update();

	volume = 100 * volume / 7;

	downcast<beep_device *>(device)->set_output_gain(0, volume);
}

const device_type BEEP = &device_creator<beep_device>;

beep_device::beep_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, BEEP, "Beep", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(beep_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void beep_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void beep_device::device_start()
{
	DEVICE_START_NAME( beep )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void beep_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


