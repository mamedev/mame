/**************************************************************************************

  Wonderswan sound emulation

  Wilbert Pol

  Sound emulation is very preliminary and far from complete

**************************************************************************************/

#include "includes/wswan.h"


struct CHAN {
	UINT16	freq;			/* frequency */
	UINT32	period;			/* period */
	UINT32	pos;			/* position */
	UINT8	vol_left;		/* volume left */
	UINT8	vol_right;		/* volume right */
	UINT8	on;			/* on/off */
	INT8	signal;			/* signal */
};

typedef struct SND wswan_sound_state;
struct SND {
	sound_stream *channel;
	struct CHAN audio1;		/* Audio channel 1 */
	struct CHAN audio2;		/* Audio channel 2 */
	struct CHAN audio3;		/* Audio channel 3 */
	struct CHAN audio4;		/* Audio channel 4 */
	INT8	sweep_step;		/* Sweep step */
	UINT32	sweep_time;		/* Sweep time */
	UINT32	sweep_count;		/* Sweep counter */
	UINT8	noise_type;		/* Noise generator type */
	UINT8	noise_reset;		/* Noise reset */
	UINT8	noise_enable;		/* Noise enable */
	UINT16	sample_address;		/* Sample address */
	UINT8	audio2_voice;		/* Audio 2 voice */
	UINT8	audio3_sweep;		/* Audio 3 sweep */
	UINT8	audio4_noise;		/* Audio 4 noise */
	UINT8	mono;			/* mono */
	UINT8	voice_data;		/* voice data */
	UINT8	output_volume;		/* output volume */
	UINT8	external_stereo;	/* external stereo */
	UINT8	external_speaker;	/* external speaker */
	UINT16	noise_shift;		/* Noise counter shift register */
	UINT8	master_volume;		/* Master volume */
};


INLINE wswan_sound_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == WSWAN);
	return (wswan_sound_state *)downcast<wswan_sound_device *>(device)->token();
}

static void wswan_ch_set_freq( running_machine &machine, struct CHAN *ch, UINT16 freq )
{
	freq &= 0x7ff;	// docs say freq is 11bits and a few games (Morita Shougi, World Stadium + others) write 0x800 causing a divide by 0 crash
	ch->freq = freq;
	ch->period = machine.sample_rate() / (3072000 / ((2048 - freq) << 5));
}

WRITE8_DEVICE_HANDLER( wswan_sound_port_w )
{
	wswan_sound_state *state = get_safe_token(device);

	state->channel->update();

	switch( offset ) {
	case 0x80:				/* Audio 1 freq (lo) */
		wswan_ch_set_freq(device->machine(), &state->audio1, (state->audio1.freq & 0xff00) | data);
		break;
	case 0x81:				/* Audio 1 freq (hi) */
		wswan_ch_set_freq(device->machine(), &state->audio1, (data << 8 ) | (state->audio1.freq & 0x00ff));
		break;
	case 0x82:				/* Audio 2 freq (lo) */
		wswan_ch_set_freq(device->machine(), &state->audio2, (state->audio2.freq & 0xff00) | data);
		break;
	case 0x83:				/* Audio 2 freq (hi) */
		wswan_ch_set_freq(device->machine(), &state->audio2, (data << 8 ) | (state->audio2.freq & 0x00ff));
		break;
	case 0x84:				/* Audio 3 freq (lo) */
		wswan_ch_set_freq(device->machine(), &state->audio3, (state->audio3.freq & 0xff00) | data);
		break;
	case 0x85:				/* Audio 3 freq (hi) */
		wswan_ch_set_freq(device->machine(), &state->audio3, (data << 8) | (state->audio3.freq & 0x00ff));
		break;
	case 0x86:				/* Audio 4 freq (lo) */
		wswan_ch_set_freq(device->machine(), &state->audio4, (state->audio4.freq & 0xff00) | data);
		break;
	case 0x87:				/* Audio 4 freq (hi) */
		wswan_ch_set_freq(device->machine(), &state->audio4, (data << 8) | (state->audio4.freq & 0x00ff));
		break;
	case 0x88:				/* Audio 1 volume */
		state->audio1.vol_left = ( data & 0xF0 ) >> 4;
		state->audio1.vol_right = data & 0x0F;
		break;
	case 0x89:				/* Audio 2 volume */
		state->voice_data = data;
		state->audio2.vol_left = ( data & 0xF0 ) >> 4;
		state->audio2.vol_right = data & 0x0F;
		break;
	case 0x8A:				/* Audio 3 volume */
		state->audio3.vol_left = ( data & 0xF0 ) >> 4;
		state->audio3.vol_right = data & 0x0F;
		break;
	case 0x8B:				/* Audio 4 volume */
		state->audio4.vol_left = ( data & 0xF0 ) >> 4;
		state->audio4.vol_right = data & 0x0F;
		break;
	case 0x8C:				/* Sweep step */
		state->sweep_step = (INT8)data;
		break;
	case 0x8D:				/* Sweep time */
		state->sweep_time = device->machine().sample_rate() / ( 3072000 / ( 8192 * (data + 1) ) );
		break;
	case 0x8E:				/* Noise control */
		state->noise_type = data & 0x07;
		state->noise_reset = ( data & 0x08 ) >> 3;
		state->noise_enable = ( data & 0x10 ) >> 4;
		break;
	case 0x8F:				/* Sample location */
		state->sample_address = data << 6;
		break;
	case 0x90:				/* Audio control */
		state->audio1.on = data & 0x01;
		state->audio2.on = ( data & 0x02 ) >> 1;
		state->audio3.on = ( data & 0x04 ) >> 2;
		state->audio4.on = ( data & 0x08 ) >> 3;
		state->audio2_voice = ( data & 0x20 ) >> 5;
		state->audio3_sweep = ( data & 0x40 ) >> 6;
		state->audio4_noise = ( data & 0x80 ) >> 7;
		break;
	case 0x91:				/* Audio output */
		state->mono = data & 0x01;
		state->output_volume = ( data & 0x06 ) >> 1;
		state->external_stereo = ( data & 0x08 ) >> 3;
		state->external_speaker = 1;
		break;
	case 0x92:				/* Noise counter shift register (lo) */
		state->noise_shift = ( state->noise_shift & 0xFF00 ) | data;
		break;
	case 0x93:				/* Noise counter shift register (hi) */
		state->noise_shift = ( data << 8 ) | ( state->noise_shift & 0x00FF );
		break;
	case 0x94:				/* Master volume */
		state->master_volume = data;
		break;
	}
}

static STREAM_UPDATE( wswan_sh_update )
{
	wswan_sound_state *state = get_safe_token(device);
	stream_sample_t sample, left, right;

	while( samples-- > 0 )
	{
		left = right = 0;

		if ( state->audio1.on ) {
			sample = state->audio1.signal;
			state->audio1.pos++;
			if ( state->audio1.pos >= state->audio1.period / 2 ) {
				state->audio1.pos = 0;
				state->audio1.signal = -state->audio1.signal;
			}
			left += state->audio1.vol_left * sample;
			right += state->audio1.vol_right * sample;
		}

		if ( state->audio2.on ) {
			if ( state->audio2_voice ) {
				left += (state->voice_data - 128)*(state->master_volume & 0x0f);
				right += (state->voice_data - 128)*(state->master_volume & 0x0f);
			} else {
				sample = state->audio2.signal;
				state->audio2.pos++;
				if ( state->audio2.pos >= state->audio2.period / 2 ) {
					state->audio2.pos = 0;
					state->audio2.signal = -state->audio2.signal;
				}
				left += state->audio2.vol_left * sample;
				right += state->audio2.vol_right * sample;
			}
		}

		if ( state->audio3.on ) {
			sample = state->audio3.signal;
			state->audio3.pos++;
			if ( state->audio3.pos >= state->audio3.period / 2 ) {
				state->audio3.pos = 0;
				state->audio3.signal = -state->audio3.signal;
			}
			if ( state->audio3_sweep && state->sweep_time ) {
				state->sweep_count++;
				if ( state->sweep_count >= state->sweep_time ) {
					state->sweep_count = 0;
					state->audio3.freq += state->sweep_step;
					state->audio3.period = device->machine().sample_rate() / (3072000  / ((2048 - (state->audio3.freq & 0x7ff)) << 5));
				}
			}
			left += state->audio3.vol_left * sample;
			right += state->audio3.vol_right * sample;
		}

		if ( state->audio4.on ) {
			if ( state->audio4_noise ) {
				sample = 0;
			} else {
				sample = state->audio4.signal;
				state->audio4.pos++;
				if ( state->audio4.pos >= state->audio4.period / 2 ) {
					state->audio4.pos = 0;
					state->audio4.signal = -state->audio4.signal;
				}
			}
			left += state->audio4.vol_left * sample;
			right += state->audio4.vol_right * sample;
		}

		left <<= 5;
		right <<= 5;

		*(outputs[0]++) = left;
		*(outputs[1]++) = right;
	}
}

static DEVICE_START(wswan_sound)
{
	wswan_sound_state *state = get_safe_token(device);
	state->channel = device->machine().sound().stream_alloc(*device, 0, 2, device->machine().sample_rate(), 0, wswan_sh_update);

	state->audio1.on = 0;
	state->audio1.signal = 16;
	state->audio1.pos = 0;
	state->audio2.on = 0;
	state->audio2.signal = 16;
	state->audio2.pos = 0;
	state->audio3.on = 0;
	state->audio3.signal = 16;
	state->audio3.pos = 0;
	state->audio4.on = 0;
	state->audio4.signal = 16;
	state->audio4.pos = 0;
}


const device_type WSWAN = &device_creator<wswan_sound_device>;

wswan_sound_device::wswan_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, WSWAN, "WonderSwan Custom", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(wswan_sound_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void wswan_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wswan_sound_device::device_start()
{
	DEVICE_START_NAME( wswan_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void wswan_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


