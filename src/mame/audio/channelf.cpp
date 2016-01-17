// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, Frank Palazzolo, Sean Riddle
#include "emu.h"
#include "audio/channelf.h"


#define MAX_AMPLITUDE  0x7fff

const device_type CHANNELF_SOUND = &device_creator<channelf_sound_device>;

channelf_sound_device::channelf_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CHANNELF_SOUND, "Channel F Sound", tag, owner, clock, "channelf_sound", __FILE__),
		device_sound_interface(mconfig, *this),
		m_channel(nullptr),
		m_sound_mode(0),
		m_incr(0),
		m_decay_mult(0),
		m_envelope(0),
		m_sample_counter(0),
		m_forced_ontime(0),
		m_min_ontime(0)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void channelf_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void channelf_sound_device::device_start()
{
	int rate;

	m_channel = stream_alloc(0, 1, machine().sample_rate());
	rate = machine().sample_rate();

	/*
	 * 2V = 1000Hz ~= 3579535/224/16
	 * Note 2V on the schematic is not the 2V scanline counter -
	 *      it is the 2V vertical pixel counter
	 *      1 pixel = 4 scanlines high
	 *
	 *
	 * This is a convenient way to generate the relevant frequencies,
	 * using a DDS (Direct Digital Synthesizer)
	 *
	 * Essentially, you want a counter to overflow some bit position
	 * at a fixed rate.  So, you figure out a number which you can add
	 * to the counter at every sample, so that you will achieve this
	 *
	 * In this case, we want to overflow bit 16 and the 2V rate, 1000Hz.
	 * This means we also get bit 17 = 4V, bit 18 = 8V, etc.
	 */

	/* This is the proper value to add per sample */
	m_incr = 65536.0/(rate/1000.0/2.0);

	//  added for improved sound
	/* This is the minimum forced ontime, in samples */
	m_min_ontime = rate/1000*2;  /* approx 2ms - estimated, not verified on HW */

	/* This was measured, decay envelope with half life of ~9ms */
	/* (this is decay multiplier per sample) */
	m_decay_mult = exp((-0.693/9e-3)/rate);

	/* initial conditions */
	m_envelope = 0;
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void channelf_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	UINT32 mask = 0, target = 0;
	stream_sample_t *buffer = outputs[0];
	stream_sample_t *sample = buffer;

	switch( m_sound_mode )
	{
		case 0: /* sound off */
			memset(buffer,0,sizeof(*buffer)*samples);
			return;

		case 1: /* high tone (2V) - 1000Hz */
			mask   = 0x00010000;
			target = 0x00010000;
			break;
		case 2: /* medium tone (4V) - 500Hz */
			mask   = 0x00020000;
			target = 0x00020000;
			break;
		case 3: /* low (weird) tone (32V & 8V) */
			mask   = 0x00140000;
			target = 0x00140000;
			break;
	}

	while (samples-- > 0)
	{
		if ((m_forced_ontime > 0) || ((m_sample_counter & mask) == target))   //  change made for improved sound
			*sample++ = m_envelope;
		else
			*sample++ = 0;
		m_sample_counter += m_incr;
		m_envelope *= m_decay_mult;
		if (m_forced_ontime > 0)          //  added for improved sound
			m_forced_ontime -= 1;      //  added for improved sound
	}
}

void channelf_sound_device::sound_w(int mode)
{
	if (mode == m_sound_mode)
		return;

	m_channel->update();
	m_sound_mode = mode;

	switch(mode)
	{
		case 0:
			m_envelope = 0;
			m_forced_ontime = 0;     //  added for improved sound
			break;
		case 1:
		case 2:
		case 3:
			m_envelope = MAX_AMPLITUDE;
			m_forced_ontime = m_min_ontime;   //  added for improved sound
			break;
	}
}
