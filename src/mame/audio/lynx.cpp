// license:GPL-2.0+
// copyright-holders:Peter Trauner
/******************************************************************************
 PeT mess@utanet.at 2000,2001
******************************************************************************/

#include "emu.h"
#include "audio/lynx.h"


/* accordingly to atari's reference manual
   there were no stereo lynx produced (the manual knows only production until mid 1991)
   the howard/developement board might have stereo support
   the revised lynx 2 hardware might have stereo support at least at the stereo jacks

   some games support stereo
*/


/*
AUDIO_A EQU $FD20
AUDIO_B EQU $FD28
AUDIO_C EQU $FD30
AUDIO_D EQU $FD38

VOLUME_CNTRL    EQU 0
FEEDBACK_ENABLE EQU 1   ; enables 11/10/5..0
OUTPUT_VALUE    EQU 2
SHIFTER_L   EQU 3
AUD_BAKUP   EQU 4
AUD_CNTRL1  EQU 5
AUD_COUNT   EQU 6
AUD_CNTRL2  EQU 7

; AUD_CNTRL1
FEEDBACK_7  EQU %10000000
AUD_RESETDONE   EQU %01000000
AUD_INTEGRATE   EQU %00100000
AUD_RELOAD  EQU %00010000
AUD_CNTEN   EQU %00001000
AUD_LINK    EQU %00000111
; link timers (0->2->4 / 1->3->5->7->Aud0->Aud1->Aud2->Aud3->1
AUD_64us    EQU %00000110
AUD_32us    EQU %00000101
AUD_16us    EQU %00000100
AUD_8us EQU %00000011
AUD_4us EQU %00000010
AUD_2us EQU %00000001
AUD_1us EQU %00000000

; AUD_CNTRL2 (read only)
; B7..B4    ; shifter bits 11..8
; B3    ; who knows
; B2    ; last clock state (0->1 causes count)
; B1    ; borrow in (1 causes count)
; B0    ; borrow out (count EQU 0 and borrow in)

ATTEN_A EQU $FD40
ATTEN_B EQU $FD41
ATTEN_C EQU $FD42
ATTEN_D EQU $FD43
; B7..B4 attenuation left ear (0 silent ..15/16 volume)
; B3..B0       "     right ear

MPAN    EQU $FD44
; B7..B4 left ear
; B3..B0 right ear
; B7/B3 EQU Audio D
; a 1 enables attenuation for channel and side


MSTEREO EQU $FD50   ; a 1 disables audio connection
AUD_D_LEFT  EQU %10000000
AUD_C_LEFT  EQU %01000000
AUD_B_LEFT  EQU %00100000
AUD_A_LEFT  EQU %00010000
AUD_D_RIGHT EQU %00001000
AUD_C_RIGHT EQU %00000100
AUD_B_RIGHT EQU %00000010
AUD_A_RIGHT EQU %00000001

 */

#define LYNX_AUDIO_CHANNELS 4


// device type definition
const device_type LYNX_SND = &device_creator<lynx_sound_device>;
const device_type LYNX2_SND = &device_creator<lynx2_sound_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  lynx_sound_device - constructor
//-------------------------------------------------

lynx_sound_device::lynx_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, LYNX_SND, "Mikey", tag, owner, clock, "lynx_sound", __FILE__),
						device_sound_interface(mconfig, *this)
{
	m_timer_delegate = lynx_sound_timer_delegate();
}

lynx_sound_device::lynx_sound_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_sound_interface(mconfig, *this)
{
}


lynx2_sound_device::lynx2_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: lynx_sound_device(mconfig, LYNX2_SND, "Mikey (Lynx II)", tag, owner, clock, "lynx2_sound", __FILE__)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void lynx_sound_device::set_timer_delegate(device_t &device, lynx_sound_timer_delegate cb)
{
	lynx_sound_device &dev = downcast<lynx_sound_device &>(device);
	dev.m_timer_delegate = cb;
}

void lynx_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void lynx_sound_device::register_save()
{
	save_item(NAME(m_attenuation_enable));
	save_item(NAME(m_master_enable));
	for (int chan = 0; chan < LYNX_AUDIO_CHANNELS; chan++)
	{
		save_item(NAME(m_audio[chan].reg.volume), chan);
		save_item(NAME(m_audio[chan].reg.feedback), chan);
		save_item(NAME(m_audio[chan].reg.output), chan);
		save_item(NAME(m_audio[chan].reg.shifter), chan);
		save_item(NAME(m_audio[chan].reg.bakup), chan);
		save_item(NAME(m_audio[chan].reg.control1), chan);
		save_item(NAME(m_audio[chan].reg.counter), chan);
		save_item(NAME(m_audio[chan].reg.control2), chan);
		save_item(NAME(m_audio[chan].attenuation), chan);
		save_item(NAME(m_audio[chan].mask), chan);
		save_item(NAME(m_audio[chan].shifter), chan);
		save_item(NAME(m_audio[chan].ticks), chan);
		save_item(NAME(m_audio[chan].count), chan);
	}
}

void lynx_sound_device::init()
{
	m_shift_mask = make_unique_clear<int[]>(512);
	m_shift_xor = make_unique_clear<int[]>(4096);

	for (int i = 0; i < 512; i++)
	{
		m_shift_mask[i] = 0;
		if (i & 1) m_shift_mask[i] |= 1;
		if (i & 2) m_shift_mask[i] |= 2;
		if (i & 4) m_shift_mask[i] |= 4;
		if (i & 8) m_shift_mask[i] |= 8;
		if (i & 0x10) m_shift_mask[i] |= 0x10;
		if (i & 0x20) m_shift_mask[i] |= 0x20;
		if (i & 0x40) m_shift_mask[i] |= 0x400;
		if (i & 0x80) m_shift_mask[i] |= 0x800;
		if (i & 0x100) m_shift_mask[i] |= 0x80;
	}

	for (int i = 0; i < 4096; i++)
	{
		m_shift_xor[i] = 1;
		for (int j = 4096/2; j > 0; j >>= 1)
		{
			if (i & j)
				m_shift_xor[i] ^= 1;
		}
	}
}

void lynx_sound_device::device_start()
{
	m_mixer_channel = machine().sound().stream_alloc(*this, 0, 1, machine().sample_rate());
	m_usec_per_sample = 1000000 / machine().sample_rate();
	m_timer_delegate.bind_relative_to(*owner());
	init();
	register_save();
}


void lynx2_sound_device::device_start()
{
	m_mixer_channel = machine().sound().stream_alloc(*this, 0, 2, machine().sample_rate());
	m_usec_per_sample = 1000000 / machine().sample_rate();
	m_timer_delegate.bind_relative_to(*owner());
	init();
	register_save();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void lynx_sound_device::device_reset()
{
	for (auto & elem : m_audio)
	{
		reset_channel(&elem);
	}
}


void lynx_sound_device::reset_channel(LYNX_AUDIO *channel)
{
	channel->reg.volume = 0;
	channel->reg.feedback = 0;
	channel->reg.output = 0;
	channel->reg.shifter = 0;
	channel->reg.bakup = 0;
	channel->reg.control1 = 0;
	channel->reg.counter = 0;
	channel->reg.control2 = 0;

	channel->attenuation = 0;
	channel->mask = 0;
	channel->shifter = 0;
	channel->ticks = 0;
	channel->count = 0;
}

void lynx_sound_device::count_down(int nr)
{
	LYNX_AUDIO *channel = &m_audio[nr];
	if (channel->reg.control1 & 8 && (channel->reg.control1 & 7) != 7)
		return;
	if (nr == 0)
		m_mixer_channel->update();
	//if ((channel->reg.control1 & 0x0f) == 0x0f) //count down if linking enabled and count enabled
		channel->count--;
}

void lynx_sound_device::shift(int chan_nr)
{
	INT16 out_temp;
	LYNX_AUDIO *channel;

	assert(chan_nr < 4);

	channel = &m_audio[chan_nr];
	//channel->shifter = ((channel->shifter<<1)&0xffe) | (m_shift_xor[ channel->shifter & channel->mask ]&1);

	// alternative method (functionally the same as above)
	UINT8 xor_out = 0;
	for (int bit = 0; bit < 12; bit++)
	{
		if ((channel->mask >> bit) & 1)
			xor_out ^= (channel->shifter >> bit) & 1;
	}
	channel->shifter = ((channel->shifter << 1) & 0xffe) | (xor_out ^ 1); // output of xor is inverted


	if (channel->reg.control1 & 0x20) // integrate mode enabled
	{
		if (channel->shifter & 1)
			out_temp = channel->reg.output + channel->reg.volume;
		else
			out_temp = channel->reg.output - channel->reg.volume;

		// clipping
		if (out_temp > 127) out_temp = 127;
		if (out_temp < -128) out_temp = -128;
		channel->reg.output = (INT16)out_temp;
	}

	switch (chan_nr)
	{
		case 0: count_down(1); break;
		case 1: count_down(2); break;
		case 2: count_down(3); break;
		case 3:
			if (!m_timer_delegate.isnull())
				m_timer_delegate();
			break;
	}
}

void lynx_sound_device::execute(int chan_nr)
{
	LYNX_AUDIO *channel;

	assert(chan_nr < 4);

	channel = &m_audio[chan_nr];

	if (channel->reg.control1 & 8) // count enable
	{
		channel->ticks += m_usec_per_sample;
		if ((channel->reg.control1 & 7) == 7) // link
		{
			if (channel->count < 0) // counter finished
			{
				//channel->count+=channel->reg.counter; // reload (wrong?)
				if (channel->reg.control1 & 0x10)
					channel->count = channel->reg.bakup;
				shift(chan_nr);
			}
		}
		else
		{
			int t = 1 << (channel->reg.control1 & 7); // microseconds per count
			for (;;)
			{
				for (; (channel->ticks >= t) && (channel->count >= 0); channel->ticks -= t) // at least one sampled worth of time left, timer not expired
					channel->count--;

				if (channel->ticks < t)
					break;

				if (channel->count < 0)
				{
					shift(chan_nr);
					if (channel->reg.control1 & 0x10)
						channel->count = channel->reg.bakup;
					else
						break;
				}
			}
		}

		if (!(channel->reg.control1 & 0x20)) // normal mode
		{
			channel->reg.output = (channel->shifter & 1) ? channel->reg.volume : -channel->reg.volume;
		}
	}
	else
	{
		channel->ticks = 0;
		channel->count = 0;
	}
}

READ8_MEMBER(lynx_sound_device::read)
{
	UINT8 value = 0;
	LYNX_AUDIO *channel = &m_audio[(offset >> 3) & 3];

	m_mixer_channel->update();

	if (offset < 0x40)
	{
		switch (offset & 7)
		{
			case 0:
				value = channel->reg.volume;
				break;
			case 1:
				value = channel->reg.feedback;
				break;
			case 2:
				value = channel->reg.output;
				break;
			case 3:
				// current shifter state (lower 8 bits)
				value = channel->shifter & 0xff;
				break;
			case 4:
				value = channel->reg.bakup;
				break;
			case 5:
				value = channel->reg.control1;
				break;
			case 6:
				//current timer value
				if (channel->count >=0)
					value = channel->count;
				else
					value = 0;
				break;
			case 7:
				// current shifter state (upper 4 bits), status bits
				value = (channel->shifter >> 4) & 0xf0;
				value |= channel->reg.control2 & 0x0f;
				break;
		}
	}
	else
	{
		switch (offset) // Lynx II stereo control registers
		{
			case 0x40: case 0x41: case 0x42: case 0x43:
				value = m_audio[offset & 3].attenuation;
				break;
			case 0x44:
				value = m_attenuation_enable;
				break;
			case 0x50:
				value = m_master_enable;
				break;
		}

	}
	return value;
}

WRITE8_MEMBER(lynx_sound_device::write)
{
	//logerror("audio write %.2x %.2x\n", offset, data);
	LYNX_AUDIO *channel = &m_audio[(offset >> 3) & 3];

	m_mixer_channel->update();

	if (offset < 0x40)
	{
		switch (offset & 0x07)
		{
			// Volume control (signed)
			case 0:
				channel->reg.volume = data;
				//logerror("write to volume %d\n", data);
				break;
			// Shift register feedback enable bits 0-5, 11,10
			case 1:
				channel->reg.feedback = data;
				channel->mask &= 0x80;
				channel->mask |= (data & 0x3f) | ((data & 0xc0)<<4);
				break;
			// Output value
			case 2:
				channel->reg.output = data;
				//logerror("write to output %d\n", data);
				break;
			// Lower 8 bits of shift register
			case 3:
				channel->shifter &= 0xf00;
				channel->shifter |= data;
				break;
			// Audio timer backup value
			case 4:
				channel->reg.bakup = data;
				break;
			// Audio control bits
			case 5:
				channel->mask &= ~0x80;
				channel->mask |= (data&0x80);
				channel->reg.control1 = data;
				break;
			// Current count
			case 6:
				channel->count=data;
				break;
			// Upper 4 bits of shift register and audio status bits
			case 7:
				channel->shifter&=0xff;
				channel->shifter|=(data&0xf0)<<4;
				channel->reg.control2 = data;
				break;
		}
	}
	else
	{
		switch (offset) // Stereo Registers
		{
			case 0x40: case 0x41: case 0x42: case 0x43:
				m_audio[offset&3].attenuation = data;
				break;
			case 0x44:
				m_attenuation_enable = data;
				break;
			case 0x50:
				m_master_enable = data;
				break;
		}
	}
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void lynx_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	int v;
	stream_sample_t *buffer = outputs[0];

	for (int i = 0; i < samples; i++, buffer++)
	{
		*buffer = 0;
		for (int channel = 0; channel < LYNX_AUDIO_CHANNELS; channel++)
		{
			execute(channel);
			v = m_audio[channel].reg.output;
			*buffer += v * 15; // where does the *15 come from?
		}
	}
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void lynx2_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *left=outputs[0], *right=outputs[1];
	int v;

	for (int i = 0; i < samples; i++, left++, right++)
	{
		*left = 0;
		*right= 0;
		for (int channel = 0; channel < LYNX_AUDIO_CHANNELS; channel++)
		{
			execute(channel);
			v = m_audio[channel].reg.output;
			if (!(m_master_enable & (0x10 << channel)))
			{
				if (m_attenuation_enable & (0x10 << channel))
					*left += v * (m_audio[channel].attenuation >> 4);
				else
					*left += v * 15;
			}
			if (!(m_master_enable & (1 << channel)))
			{
				if (m_attenuation_enable & (1 << channel))
					*right += v * (m_audio[channel].attenuation & 0xf);
				else
					*right += v * 15;
			}
		}
	}
}
