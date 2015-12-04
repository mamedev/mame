// license:???
// copyright-holders:hap, insideoutboy
/* Clarue Flower sound driver.
Initial version was based on the Wiping sound driver, which was based on the old namco.c sound driver.

TODO:
- timing (see main driver file), but also of samplerate and effects counter
- what do the unknown bits in soundregs do?
- Are channel effects correct? It's currently mostly guesswork, the pitch effects sound pretty convincing though.
  Considering that the game sound hardware isn't complicated (no dedicated soundchip) these bits are possibly
  for something way simpler, such as a length counter. PCB sound recordings would be useful!

*/

#include "emu.h"
#include "includes/flower.h"

#define FLOWER_VERBOSE      0       // show register writes

#define MIXER_SAMPLERATE    48000   /* ? (native freq is probably in the MHz range) */
#define MIXER_DEFGAIN       48


const device_type FLOWER = &device_creator<flower_sound_device>;

flower_sound_device::flower_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, FLOWER, "Flower Audio Custom", tag, owner, clock, "flower_sound", __FILE__),
		device_sound_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void flower_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void flower_sound_device::device_start()
{
	flower_sound_channel *voice;

	m_effect_timer = timer_alloc(TIMER_CLOCK_EFFECT);
	m_stream = machine().sound().stream_alloc(*this, 0, 1, MIXER_SAMPLERATE);
	m_mixer_buffer = auto_alloc_array(machine(), short, MIXER_SAMPLERATE);
	make_mixer_table(8, MIXER_DEFGAIN);

	/* extract globals from the interface */
	m_last_channel = m_channel_list + 8;

	m_sample_rom = machine().root_device().memregion("sound1")->base();
	m_volume_rom = machine().root_device().memregion("sound2")->base();

	/* register for savestates */
	for (int i = 0; i < 8; i++)
	{
		voice = &m_channel_list[i];

		save_item(NAME(voice->freq), i+1);
		save_item(NAME(voice->pos), i+1);
		save_item(NAME(voice->volume), i+1);
		save_item(NAME(voice->voltab), i+1);
		save_item(NAME(voice->effect), i+1);
		save_item(NAME(voice->ecount), i+1);
		save_item(NAME(voice->oneshot), i+1);
		save_item(NAME(voice->active), i+1);
		save_item(NAME(voice->start), i+1);
	}
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void flower_sound_device::device_reset()
{
	flower_sound_channel *voice;
	attotime period;

	/* reset effect timer, period is unknown/guessed */
	period = attotime::from_hz(MIXER_SAMPLERATE / 256);
	m_effect_timer->adjust(period, 0, period);

	/* reset all the voices */
	for (auto & elem : m_channel_list)
	{
		voice = &elem;

		voice->freq = 0;
		voice->pos = 0;
		voice->volume = 0;
		voice->voltab = 0;
		voice->effect = 0;
		voice->ecount = 0;
		voice->oneshot = 1;
		voice->active = 0;
		voice->start = 0;
	}

	memset(m_soundregs1, 0, 0x40);
	memset(m_soundregs2, 0, 0x40);
}

/* build a table to divide by the number of voices; gain is specified as gain*16 */
void flower_sound_device::make_mixer_table(int voices, int gain)
{
	int count = voices * 128;

	/* allocate memory */
	m_mixer_table = auto_alloc_array(machine(), INT16, 256 * voices);

	/* find the middle of the table */
	m_mixer_lookup = m_mixer_table + (128 * voices);

	/* fill in the table - 16 bit case */
	for (int i = 0; i < count; i++)
	{
		int val = i * gain * 16 / voices;
		if (val > 32767) val = 32767;
		m_mixer_lookup[ i] = val;
		m_mixer_lookup[-i] =-val;
	}
}


/* clock sound channel effect counters */
void flower_sound_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_CLOCK_EFFECT:
		flower_sound_channel *voice;
		m_stream->update();

		for (voice = m_channel_list; voice < m_last_channel; voice++)
			voice->ecount += (voice->ecount < (1<<22));
		break;

		default:
			assert_always(FALSE, "Unknown id in flower_sound_device::device_timer");
	}
}

/********************************************************************************/

#if FLOWER_VERBOSE
static int r_numwrites[2][8] = {{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}};
void flower_sound_device::show_soundregs()
{
	int set,reg,chan;
	char text[0x100];
	char message[0x1000] = {0};
	UINT8 *base = m_soundregs1;

	for (set=0;set<2;set++)
	{
		for (reg=0;reg<8;reg++)
		{
			sprintf(text,"R%d%d:",set+1,reg);
			strcat(message,text);

			for (chan=0;chan<8;chan++)
			{
				sprintf(text," %02X",base[reg + 8*chan]);
				strcat(message,text);
			}
			sprintf(text," - %07d\n",r_numwrites[set][reg]);
			strcat(message,text);
		}
		strcat(message,"\n");
		base = m_soundregs2;
	}
	popmessage("%s",message);
}
#endif // FLOWER_VERBOSE


/* register functions (preliminary):
offset: cccrrr      c=channel, r=register

set 1:
R  76543210
0  xxxxxxxx         frequency (which nibble?)
1  xxxxxxxx         *
2  xxxxxxxx         *
3  xxxxxxxx         *
4  ...x....         one-shot sample
5  ...x....         ??? same as R4?
6  ........         unused
7  xxxx....         volume

set 2:
R  76543210
0  ....xxxx         start address
1  ....xxxx         *
2  ....xxxx         *
3  ....xxxx         *
4  xxxx             assume it's channel pitch/volume effects
       xxxx         start address
5  x...             ???
       xxxx         start address
6  ........         unused
7  ......xx         volume table + start trigger

*/

WRITE8_MEMBER( flower_sound_device::sound1_w )
{
	flower_sound_channel *voice = &m_channel_list[offset >> 3 & 7];
	int c = offset & 0xf8;
	UINT8 *base1 = m_soundregs1;
//  UINT8 *base2 = m_soundregs2;

	m_stream->update();
	base1[offset] = data;
#if FLOWER_VERBOSE
	r_numwrites[0][offset & 7]++;
	show_soundregs();
#endif

	// recompute voice parameters
	voice->freq = (base1[c+2] & 0xf) << 12 | (base1[c+3] & 0xf) << 8 | (base1[c+0] & 0xf) << 4 | (base1[c+1] & 0xf);
	voice->volume = base1[c+7] >> 4;
}

WRITE8_MEMBER( flower_sound_device::sound2_w )
{
	flower_sound_channel *voice = &m_channel_list[offset >> 3 & 7];
	int i, c = offset & 0xf8;
	UINT8 *base1 = m_soundregs1;
	UINT8 *base2 = m_soundregs2;

	m_stream->update();
	base2[offset] = data;
#if FLOWER_VERBOSE
	r_numwrites[1][offset & 7]++;
	show_soundregs();
#endif

	// reg 7 is start trigger!
	if ((offset & 7) != 7)
		return;

	voice->voltab = (base2[c+7] & 3) << 4;
	voice->oneshot = (~base1[c+4] & 0x10) >> 4;
	voice->effect = base2[c+4] >> 4;
	voice->ecount = 0;
	voice->pos = 0;
	voice->active = 1;

	// full start address is 6 nibbles
	voice->start = 0;
	for (i = 5; i >= 0; i--)
		voice->start = (voice->start << 4) | (base2[c+i] & 0xf);
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void flower_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *buffer = outputs[0];
	flower_sound_channel *voice;
	short *mix;
	int i;

	/* zap the contents of the mixer buffer */
	memset(m_mixer_buffer, 0, samples * sizeof(short));

	/* loop over each voice and add its contribution */
	for (voice = m_channel_list; voice < m_last_channel; voice++)
	{
		int f = voice->freq;
		int v = voice->volume;

		if (!voice->active)
			continue;

		// effects
		// bit 0: volume slide down?
		if (voice->effect & 1 && !voice->oneshot)
		{
			// note: one-shot samples are fixed volume
			v -= (voice->ecount >> 4);
			if (v < 0) v = 0;
		}
		// bit 1: used often, but hard to figure out what for
		// bit 2: probably pitch slide
		if (voice->effect & 4)
		{
			f -= (voice->ecount << 7);
			if (f < 0) f = 0;
		}
		// bit 3: not used much, maybe pitch slide the other way?

		v |= voice->voltab;
		mix = m_mixer_buffer;

		for (i = 0; i < samples; i++)
		{
			// add sample
			if (voice->oneshot)
			{
				UINT8 sample = m_sample_rom[(voice->start + voice->pos) >> 7 & 0x7fff];
				if (sample == 0xff)
				{
					voice->active = 0;
					break;
				}
				else
					*mix++ += m_volume_rom[v << 8 | sample] - 0x80;
			}
			else
			{
				UINT8 sample = m_sample_rom[(voice->start >> 7 & 0x7e00) | (voice->pos >> 7 & 0x1ff)];
				*mix++ += m_volume_rom[v << 8 | sample] - 0x80;
			}

			// update counter
			voice->pos += f;
		}
	}

	/* mix it down */
	mix = m_mixer_buffer;
	for (i = 0; i < samples; i++)
		*buffer++ = m_mixer_lookup[*mix++];
}
