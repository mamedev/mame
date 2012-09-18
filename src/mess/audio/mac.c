/***************************************************************************

    mac.c

    Sound handler

****************************************************************************/


#include "emu.h"
#include "sound/asc.h"
#include "includes/mac.h"
#include "machine/ram.h"

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define MAC_MAIN_SND_BUF_OFFSET	0x0300
#define MAC_ALT_SND_BUF_OFFSET	0x5F00
#define MAC_SND_BUF_SIZE		370			/* total number of scan lines */
#define MAC_SAMPLE_RATE			( MAC_SND_BUF_SIZE * 60 /*22255*/ )	/* scan line rate, should be 22254.5 Hz */


/* intermediate buffer */
#define SND_CACHE_SIZE 128


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct mac_sound
{
	sound_stream *mac_stream;
	int sample_enable;
	UINT16 *mac_snd_buf_ptr;
	UINT8 *snd_cache;
	int snd_cache_len;
	int snd_cache_head;
	int snd_cache_tail;
	int indexx;
};


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE mac_sound *get_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == MAC_SOUND);
	return (mac_sound *) downcast<mac_sound_device *>(device)->token();
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/************************************/
/* Stream updater                   */
/************************************/

static STREAM_UPDATE( mac_sound_update )
{
	INT16 last_val = 0;
	stream_sample_t *buffer = outputs[0];
	mac_sound *token = get_token(device);
	mac_state *mac = device->machine().driver_data<mac_state>();

	if ((mac->m_model == MODEL_MAC_PORTABLE) || (mac->m_model == MODEL_MAC_PB100))
	{
		memset(buffer, 0, samples * sizeof(*buffer));
		return;
	}

	/* if we're not enabled, just fill with 0 */
	if (device->machine().sample_rate() == 0)
	{
		memset(buffer, 0, samples * sizeof(*buffer));
		return;
	}

	/* fill in the sample */
	while (samples && token->snd_cache_len)
	{
		*buffer++ = last_val = ((token->snd_cache[token->snd_cache_head] << 8) ^ 0x8000) & 0xff00;
		token->snd_cache_head++;
		token->snd_cache_head %= SND_CACHE_SIZE;
		token->snd_cache_len--;
		samples--;
	}

	while (samples--)
	{
		/* should never happen */
		*buffer++ = last_val;
	}
}



/************************************/
/* Sound handler start              */
/************************************/

static DEVICE_START(mac_sound)
{
	mac_sound *token = get_token(device);

	memset(token, 0, sizeof(*token));

	token->snd_cache = auto_alloc_array(device->machine(), UINT8, SND_CACHE_SIZE);
	token->mac_stream = device->machine().sound().stream_alloc(*device, 0, 1, MAC_SAMPLE_RATE, 0, mac_sound_update);
}



/*
    Set the sound enable flag (VIA port line)
*/
void mac_enable_sound(device_t *device, int on)
{
	mac_sound *token = get_token(device);
	token->sample_enable = on;
}



/*
    Set the current sound buffer (one VIA port line)
*/
void mac_set_sound_buffer(device_t *device, int buffer)
{
	mac_sound *token = get_token(device);
	ram_device *ram = device->machine().device<ram_device>(RAM_TAG);

	if (buffer)
		token->mac_snd_buf_ptr = (UINT16 *) (ram->pointer() + ram->size() - MAC_MAIN_SND_BUF_OFFSET);
	else
		token->mac_snd_buf_ptr = (UINT16 *) (ram->pointer() + ram->size() - MAC_ALT_SND_BUF_OFFSET);
}



/*
    Set the current sound volume (3 VIA port line)
*/
void mac_set_volume(device_t *device, int volume)
{
	mac_sound *token = get_token(device);

	token->mac_stream->update();
	volume = (100 / 7) * volume;
	token->mac_stream->set_output_gain(0, volume / 100.0);
}



/*
    Fetch one byte from sound buffer and put it to sound output (called every scanline)
*/
void mac_sh_updatebuffer(device_t *device)
{
	mac_sound *token = get_token(device);
	UINT16 *base = token->mac_snd_buf_ptr;

	token->indexx++;
	token->indexx %= 370;

	if (token->snd_cache_len >= SND_CACHE_SIZE)
	{
		/* clear buffer */
		token->mac_stream->update();
	}

	if (token->snd_cache_len >= SND_CACHE_SIZE)
		/* should never happen */
		return;

	token->snd_cache[token->snd_cache_tail] = token->sample_enable ? (base[token->indexx] >> 8) & 0xff : 0;
	token->snd_cache_tail++;
	token->snd_cache_tail %= SND_CACHE_SIZE;
	token->snd_cache_len++;
}


const device_type MAC_SOUND = &device_creator<mac_sound_device>;

mac_sound_device::mac_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MAC_SOUND, "Mac Custom", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(mac_sound);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mac_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mac_sound_device::device_start()
{
	DEVICE_START_NAME( mac_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void mac_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


