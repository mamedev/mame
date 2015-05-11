// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Raphael Nabet, R. Belmont
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

#define MAC_MAIN_SND_BUF_OFFSET 0x0300
#define MAC_ALT_SND_BUF_OFFSET  0x5F00
#define MAC_SND_BUF_SIZE        370         /* total number of scan lines */
#define MAC_SAMPLE_RATE         ( MAC_SND_BUF_SIZE * 60 /*22255*/ ) /* scan line rate, should be 22254.5 Hz */


/* intermediate buffer */
#define SND_CACHE_SIZE 128




const device_type MAC_SOUND = &device_creator<mac_sound_device>;

mac_sound_device::mac_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, MAC_SOUND, "Mac Audio Custom", tag, owner, clock, "mac_sound", __FILE__),
						device_sound_interface(mconfig, *this),
						m_sample_enable(0),
						m_mac_snd_buf_ptr(NULL),
						m_snd_cache_len(0),
						m_snd_cache_head(0),
						m_snd_cache_tail(0),
						m_indexx(0)
{
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
	mac_state *mac = machine().driver_data<mac_state>();

	m_snd_cache = auto_alloc_array_clear(machine(), UINT8, SND_CACHE_SIZE);
	m_mac_stream = machine().sound().stream_alloc(*this, 0, 1, MAC_SAMPLE_RATE);

	m_ram = machine().device<ram_device>(RAM_TAG);
	m_mac_model = mac->m_model;

	save_pointer(NAME(m_snd_cache), SND_CACHE_SIZE);
	save_item(NAME(m_sample_enable));
	save_item(NAME(m_snd_cache_len));
	save_item(NAME(m_snd_cache_head));
	save_item(NAME(m_snd_cache_tail));
	save_item(NAME(m_indexx));
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void mac_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	INT16 last_val = 0;
	stream_sample_t *buffer = outputs[0];

	if ((m_mac_model == MODEL_MAC_PORTABLE) || (m_mac_model == MODEL_MAC_PB100))
	{
		memset(buffer, 0, samples * sizeof(*buffer));
		return;
	}

	/* if we're not enabled, just fill with 0 */
	if (machine().sample_rate() == 0)
	{
		memset(buffer, 0, samples * sizeof(*buffer));
		return;
	}

	/* fill in the sample */
	while (samples && m_snd_cache_len)
	{
		*buffer++ = last_val = ((m_snd_cache[m_snd_cache_head] << 8) ^ 0x8000) & 0xff00;
		m_snd_cache_head++;
		m_snd_cache_head %= SND_CACHE_SIZE;
		m_snd_cache_len--;
		samples--;
	}

	while (samples--)
	{
		/* should never happen */
		*buffer++ = last_val;
	}
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/


// Set the sound enable flag (VIA port line)
void mac_sound_device::enable_sound(int on)
{
	m_sample_enable = on;
}


// Set the current sound buffer (one VIA port line)
void mac_sound_device::set_sound_buffer(int buffer)
{
	if (buffer)
		m_mac_snd_buf_ptr = (UINT16 *) (m_ram->pointer() + m_ram->size() - MAC_MAIN_SND_BUF_OFFSET);
	else
		m_mac_snd_buf_ptr = (UINT16 *) (m_ram->pointer() + m_ram->size() - MAC_ALT_SND_BUF_OFFSET);
}



// Set the current sound volume (3 VIA port line)
void mac_sound_device::set_volume(int volume)
{
	m_mac_stream->update();
	volume = (100 / 7) * volume;
	m_mac_stream->set_output_gain(0, volume / 100.0);
}



// Fetch one byte from sound buffer and put it to sound output (called every scanline)
void mac_sound_device::sh_updatebuffer()
{
	UINT16 *base = m_mac_snd_buf_ptr;

	m_indexx++;
	m_indexx %= 370;

	if (m_snd_cache_len >= SND_CACHE_SIZE)
	{
		/* clear buffer */
		m_mac_stream->update();
	}

	if (m_snd_cache_len >= SND_CACHE_SIZE)
		/* should never happen */
		return;

	m_snd_cache[m_snd_cache_tail] = m_sample_enable ? (base[m_indexx] >> 8) & 0xff : 0;
	m_snd_cache_tail++;
	m_snd_cache_tail %= SND_CACHE_SIZE;
	m_snd_cache_len++;
}
