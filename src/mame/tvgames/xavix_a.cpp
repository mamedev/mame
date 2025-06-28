// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "xavix.h"

// #define VERBOSE 1
#include "logmacro.h"

// 16 stereo voices?

// xavix_sound_device

DEFINE_DEVICE_TYPE(XAVIX_SOUND, xavix_sound_device, "xavix_sound", "XaviX Sound")

xavix_sound_device::xavix_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XAVIX_SOUND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_readregs_cb(*this, 0xff)
	, m_readsamples_cb(*this, 0x80)
{
}

void xavix_sound_device::device_start()
{
	m_stream = stream_alloc(0, 2, 163840);
}

void xavix_sound_device::device_reset()
{
	for (int v = 0; v < 16; v++)
	{
		m_voice[v].enabled[0] = false;
		m_voice[v].enabled[1] = false;
		m_voice[v].position[0] = 0x00;
		m_voice[v].position[1] = 0x00;
		m_voice[v].bank = 0x00;
	}
}


void xavix_sound_device::sound_stream_update(sound_stream &stream)
{
	int outpos = 0;
	// loop while we still have samples to generate
	int samples = stream.samples();
	while (samples-- != 0)
	{
		for (int channel = 0; channel < 2; channel++)
		{
			for (int v = 0; v < 16; v++)
			{
				if (m_voice[v].enabled[channel] == true)
				{
					// 2 is looping? 3 is single shot? 0/1 are something else?
					if (m_voice[v].type == 2 || m_voice[v].type == 3)
					{
						uint32_t pos = (m_voice[v].bank << 16) | (m_voice[v].position[channel] >> 14);
						int8_t sample = m_readsamples_cb(pos);

						if ((uint8_t)sample == 0x80) // would both channels stop / loop or just one?, Yellow submarine indicates just one, but might be running in some interleaved mode?
						{
							//if (m_voice[v].type == 3)
							{
								m_voice[v].enabled[channel] = false;
								break;
							}
							/* need envelopes or some of these loop forever!
							else if (m_voice[v].type == 2)
							{
							    m_voice[v].position[channel] = m_voice[v].startposition[channel];
							    // presumably don't want to play 0x80 byte, so read in a new one
							    pos = (m_voice[v].bank << 16) | (m_voice[v].position[channel] >> 14);
							    sample = m_readsamples_cb(pos);
							}
							*/
						}

						stream.add_int(channel, outpos, sample * (m_voice[v].vol + 1), 32768);
						m_voice[v].position[channel] += m_voice[v].rate;
					}
					else
					{
						logerror("unsupported voice type %01x", m_voice[v].type);
						m_voice[v].enabled[channel] = false;
					}
				}
			}
		}
		outpos++;
	}
}

bool xavix_sound_device::is_voice_enabled(int voice)
{
	m_stream->update();

/*
    if ((m_voice[voice].enabled[0] == true) || (m_voice[voice].enabled[1] == true))
        return true;
    else
        return false;
*/
	if ((m_voice[voice].enabled[0] == true) && (m_voice[voice].enabled[1] == true))
		return true;
	else
		return false;
}


void xavix_sound_device::enable_voice(int voice, bool update_only)
{
	m_stream->update();
	int voicemembase = voice * 0x10;

	uint16_t freq_mode = (m_readregs_cb(voicemembase + 0x1) << 8) | (m_readregs_cb(voicemembase + 0x0)); // sample rate maybe?
	uint16_t sampleaddrleft = (m_readregs_cb(voicemembase + 0x3) << 8) | (m_readregs_cb(voicemembase + 0x2)); // seems to be a start position
	uint16_t sampleaddrright = (m_readregs_cb(voicemembase + 0x5) << 8) | (m_readregs_cb(voicemembase + 0x4)); // another start position? sometimes same as envaddrleft
	uint8_t unused1 = (m_readregs_cb(voicemembase + 0x7)); // data gets written but doesn't look like it's used by the chip?
	uint8_t sampleaddrbank = (m_readregs_cb(voicemembase + 0x6)); // upper 8 bits of memory address? 8 bits unused?

	// these don't seem to be populated as often, maybe some kind of effect / envelope filter?
	uint8_t envfreq = (m_readregs_cb(voicemembase + 0x9));
	uint8_t envmode_unk = (m_readregs_cb(voicemembase + 0x8));
	uint16_t envaddrleft = (m_readregs_cb(voicemembase + 0xb) << 8) | (m_readregs_cb(voicemembase + 0xa)); // seems to be a start position, lower byte is direct value, not address in some modes??
	uint16_t envaddrright = (m_readregs_cb(voicemembase + 0xd) << 8) | (m_readregs_cb(voicemembase + 0xc)); // another start position? sometimes same as envaddrleft
	uint8_t unused2 = (m_readregs_cb(voicemembase + 0xf)); // data gets written but doesn't look like it's used by the chip?
	uint8_t envaddrbank = (m_readregs_cb(voicemembase + 0xe)); // upper 8 bits of memory address? 8 bits unused (or not unused?, get populated with increasing values sometimes?)

	uint32_t sampleaddrleft_full = (sampleaddrleft | sampleaddrbank << 16) & 0x00ffffff; // definitely addresses based on rad_snow
	uint32_t sampleaddrright_full = (sampleaddrright | sampleaddrbank << 16) & 0x00ffffff;

	uint32_t envaddrleft_full = (envaddrleft | envaddrbank << 16) & 0x00ffffff; // still looks like addresses, sometimes pointing at RAM
	uint32_t envaddrright_full = (envaddrright | envaddrbank << 16) & 0x00ffffff;

	uint8_t envmode = (envmode_unk >> 4)&3; // upper bits not used?
	uint8_t envunk = envmode_unk & 0x0f;

	if (update_only) LOG("(UPDATE ONLY) ");

	LOG("voice %01x (params %04x %04x %04x %02x %02x     %02x %02x  %04x %04x %02x %02x)\n", voice, freq_mode, sampleaddrleft, sampleaddrright, unused1, sampleaddrbank, envfreq, envmode_unk, envaddrleft, envaddrright, unused2, envaddrbank);

	if (envmode == 0)
	{
		// mode 0 doesn't seem to use a second pair of addresses for the envelope but instead a fixed value in the lower part of the address registers?, usually used with non-looping samples too? upper bytes of address end up being leftovers from previous sounds
		LOG("voice %01x (possible meanings mode %01x rate %04x sampleaddrleft_full %08x sampleaddrright_full %08x envvalue_left %02x envvalue_right %02x envfreq %02x envmode_unk [%01x, %01x])\n", voice, freq_mode & 0x3, freq_mode >> 2, sampleaddrleft_full, sampleaddrright_full, envaddrleft_full & 0xff, envaddrright_full & 0xff, envfreq, envmode, envunk);
	}
	else
	{
		// envelopes usually point to 8-byte sequences of values?
		// when written from sound_updateenv_w (update only) then mode is usually 0x3 (key off?)
		// mode 1 is used for most samples (key on?)
		// mode 2 is used on monster truck

		LOG("voice %01x (possible meanings mode %01x rate %04x sampleaddrleft_full %08x sampleaddrright_full %08x envaddrleft_full %08x envaddrright_full %08x envfreq %02x envmode_unk [%01x, %01x])\n", voice, freq_mode & 0x3, freq_mode >> 2, sampleaddrleft_full, sampleaddrright_full, envaddrleft_full, envaddrright_full, envfreq, envmode, envunk);

		LOG("  (ENV1 ");
		for (int i = 0; i < 8; i++)
		{
			uint8_t env = m_readsamples_cb(envaddrleft_full+i);
			LOG("%02x ", env);
		}
		LOG(")  ");

		LOG("  (ENV2 ");
		for (int i = 0; i < 8; i++)
		{
			uint8_t env = m_readsamples_cb(envaddrright_full+i);
			LOG("%02x ", env);
		}
		LOG(")  \n");

	}

	if (envmode_unk & 0xc0)
	{
		LOG("   (unexpected bits set in envmode_unk %02x)\n", envmode_unk & 0xc0);
	}

	if (!update_only)
	{
		m_voice[voice].enabled[0] = true;
		m_voice[voice].enabled[1] = true;

		m_voice[voice].bank = sampleaddrbank;
		m_voice[voice].position[0] = sampleaddrleft << 14;
		m_voice[voice].position[1] = sampleaddrright << 14;
		m_voice[voice].type = freq_mode & 0x3;
		m_voice[voice].rate = freq_mode >> 2;
		m_voice[voice].vol = envunk;

		m_voice[voice].startposition[0] = m_voice[voice].position[0]; // for looping
		m_voice[voice].startposition[1] = m_voice[voice].position[1];
	}

	// 0320 (800) == 8000hz
	// 4000 (16384) == 163840hz ? = 163.840kHz

	// samples appear to be PCM, 0x80 terminated, looks like there's a way of specifying mono (interleave channels?) or stereo, maybe in master regs?
}

void xavix_sound_device::disable_voice(int voice)
{
	m_voice[voice].enabled[0] = false;
	m_voice[voice].enabled[1] = false;
}


// xavix_state support

uint8_t xavix_state::sound_regram_read_cb(offs_t offset)
{
	// 0x00 would be zero page memory, and problematic for many reasons, assume it just doesn't work like that
	if ((m_sound_regbase & 0x3f) != 0x00)
	{
		uint16_t memorybase = (m_sound_regbase & 0x3f) << 8;

		return m_mainram[memorybase + offset];
	}

	return 0x00;
}


/* 75f0, 75f1 - 2x8 bits (16 voices?) */
uint8_t xavix_state::sound_startstop_r(offs_t offset)
{
	LOG("%s: sound_startstop_r %02x\n", machine().describe_context(), offset);
	return m_soundreg16_0[offset];
}

void xavix_state::sound_startstop_w(offs_t offset, uint8_t data)
{
	/* looks like the sound triggers

	  offset 0
	  data & 0x01 - voice 0  (registers at regbase + 0x00) eg 0x3b00 - 0x3b0f in monster truck
	  data & 0x02 - voice 1  (registers at regbase + 0x10) eg 0x3b10 - 0x3b1f in monster truck
	  data & 0x04 - voice 2
	  data & 0x08 - voice 3
	  data & 0x10 - voice 4
	  data & 0x20 - voice 5
	  data & 0x40 - voice 6
	  data & 0x80 - voice 7

	  offset 1
	  data & 0x01 - voice 8
	  data & 0x02 - voice 9
	  data & 0x04 - voice 10
	  data & 0x08 - voice 11
	  data & 0x10 - voice 12
	  data & 0x20 - voice 13
	  data & 0x40 - voice 14 (registers at regbase + 0xf0) eg 0x3be0 - 0x3bef in monster truck
	  data & 0x80 - voice 15 (registers at regbase + 0xf0) eg 0x3bf0 - 0x3bff in monster truck
*/
	if (offset == 0)
		LOG("%s: sound_startstop_w %02x, %02x (%d %d %d %d %d %d %d %d - - - - - - - -)\n", machine().describe_context(), offset, data, (data & 0x01) ? 1 : 0, (data & 0x02) ? 1 : 0, (data & 0x04) ? 1 : 0, (data & 0x08) ? 1 : 0, (data & 0x10) ? 1 : 0, (data & 0x20) ? 1 : 0, (data & 0x40) ? 1 : 0, (data & 0x80) ? 1 : 0);
	else
		LOG("%s: sound_startstop_w %02x, %02x (- - - - - - - - %d %d %d %d %d %d %d %d)\n", machine().describe_context(), offset, data, (data & 0x01) ? 1 : 0, (data & 0x02) ? 1 : 0, (data & 0x04) ? 1 : 0, (data & 0x08) ? 1 : 0, (data & 0x10) ? 1 : 0, (data & 0x20) ? 1 : 0, (data & 0x40) ? 1 : 0, (data & 0x80) ? 1 : 0);


	for (int i = 0; i < 8; i++)
	{
		const int voice_state = (data & (1 << i));
		const int old_voice_state = (m_soundreg16_0[offset] & (1 << i));
		if (voice_state != old_voice_state)
		{
			const int voice = (offset * 8 + i);

			if (voice_state)
			{
				m_sound->enable_voice(voice, false);
			}
			else
			{
				m_sound->disable_voice(voice);
			}
		}
	}

	m_soundreg16_0[offset] = data;
}

/* 75f0, 75f1 - 2x8 bits (16 voices?) */
uint8_t xavix_state::sound_updateenv_r(offs_t offset)
{
	LOG("%s: sound_updateenv_r %02x\n", machine().describe_context(), offset);
	return m_soundreg16_1[offset];
}

void xavix_state::sound_updateenv_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
		LOG("%s: sound_updateenv_w %02x, %02x (%d %d %d %d %d %d %d %d - - - - - - - -)\n", machine().describe_context(), offset, data, (data & 0x01) ? 1 : 0, (data & 0x02) ? 1 : 0, (data & 0x04) ? 1 : 0, (data & 0x08) ? 1 : 0, (data & 0x10) ? 1 : 0, (data & 0x20) ? 1 : 0, (data & 0x40) ? 1 : 0, (data & 0x80) ? 1 : 0);
	else
		LOG("%s: sound_updateenv_w %02x, %02x (- - - - - - - - %d %d %d %d %d %d %d %d)\n", machine().describe_context(), offset, data, (data & 0x01) ? 1 : 0, (data & 0x02) ? 1 : 0, (data & 0x04) ? 1 : 0, (data & 0x08) ? 1 : 0, (data & 0x10) ? 1 : 0, (data & 0x20) ? 1 : 0, (data & 0x40) ? 1 : 0, (data & 0x80) ? 1 : 0);

	// used to update envelopes without restarting sound? for key-off events?
	for (int i = 0; i < 8; i++)
	{
		const int voice_state = (data & (1 << i));
		const int old_voice_state = (m_soundreg16_1[offset] & (1 << i));
		if (voice_state != old_voice_state)
		{
			const int voice = (offset * 8 + i);

			if (voice_state)
			{
				m_sound->enable_voice(voice, true);
			}
			else
			{
				m_sound->disable_voice(voice);
			}
		}
	}

	m_soundreg16_1[offset] = data;
}


/* 75f4, 75f5 - 2x8 bits (16 voices?) status? */
uint8_t xavix_state::sound_sta16_r(offs_t offset)
{
	uint8_t ret = 0x00;

	for (int i = 0; i < 8; i++)
	{
		const int voice = (offset * 8 + i);
		const bool enabled = m_sound->is_voice_enabled(voice);
		ret |= enabled ? 1 << i : 0;
	}

	return ret;
}

/* 75f6 - master volume control? */
uint8_t xavix_state::sound_volume_r()
{
	LOG("%s: sound_volume_r\n", machine().describe_context());
	return m_mastervol;
}

void xavix_state::sound_volume_w(uint8_t data)
{
	m_mastervol = data;
	LOG("%s: sound_volume_w %02x\n", machine().describe_context(), data);
}

/* 75f7 - main register base*/

void xavix_state::sound_regbase_w(uint8_t data)
{
	// this is the upper 6 bits of the RAM address where the actual sound register sets are
	// (16x16 regs, so complete 0x100 bytes of RAM eg 0x3b means the complete 0x3b00 - 0x3bff range with 0x3b00 - 0x3b0f being voice 1 etc)
	m_sound_regbase = data;
	LOG("%s: sound_regbase_w %02x (sound regs are at 0x%02x00 to 0x%02xff)\n", machine().describe_context(), data, m_sound_regbase & 0x3f, m_sound_regbase & 0x3f);
}

/* 75f8, 75f9 - misc unknown sound regs*/

uint8_t xavix_state::sound_75f8_r()
{
	LOG("%s: sound_75f8_r\n", machine().describe_context());
	return m_unk_snd75f8;
}

void xavix_state::sound_75f8_w(uint8_t data)
{
	m_unk_snd75f8 = data;
	LOG("%s: sound_75f8_w %02x\n", machine().describe_context(), data);
}

uint8_t xavix_state::sound_75f9_r()
{
	LOG("%s: sound_75f9_r\n", machine().describe_context());
	return m_unk_snd75f9;
}

void xavix_state::sound_75f9_w(uint8_t data)
{
	m_unk_snd75f9 = data;
	LOG("%s: sound_75f9_w %02x\n", machine().describe_context().c_str(), data);
}

/* 75fa, 75fb, 75fc, 75fd - timers?? generate interrupts?? */

uint8_t xavix_state::sound_timer0_r()
{
	LOG("%s: sound_timer0_r\n", machine().describe_context());
	return m_sndtimer[0];
}

void xavix_state::sound_timer0_w(uint8_t data)
{
	m_sndtimer[0] = data;
	LOG("%s: sound_timer0_w %02x\n", machine().describe_context(), data);
}

uint8_t xavix_state::sound_timer1_r()
{
	LOG("%s: sound_timer1_r\n", machine().describe_context());
	return m_sndtimer[1];
}

void xavix_state::sound_timer1_w(uint8_t data)
{
	m_sndtimer[1] = data;
	LOG("%s: sound_timer1_w %02x\n", machine().describe_context(), data);
}

uint8_t xavix_state::sound_timer2_r()
{
	LOG("%s: sound_timer2_r\n", machine().describe_context());
	return m_sndtimer[2];
}

void xavix_state::sound_timer2_w(uint8_t data)
{
	m_sndtimer[2] = data;
	LOG("%s: sound_timer2_w %02x\n", machine().describe_context(), data);
}

uint8_t xavix_state::sound_timer3_r()
{
	LOG("%s: sound_timer3_r\n", machine().describe_context());
	return m_sndtimer[3];
}

void xavix_state::sound_timer3_w(uint8_t data) // this one is used by ekara (uk carts)  , enabled with 08 in 75fe
{
	m_sndtimer[3] = data;
	LOG("%s: sound_timer3_w %02x\n", machine().describe_context(), data);
}

/* 75fe - some kind of IRQ status / Timer Status? */

uint8_t xavix_state::sound_irqstatus_r()
{
	// rad_rh checks this after doing something that looks like an irq ack
	// the UK ekara sets check the upper bit to see if the interrupt is from the sound timer (rather than checking interrupt source register)
	// and decrease a counter that controls the tempo (the US / Japan sets don't enable the sound timer at all)
	return m_sound_irqstatus;
}

void xavix_state::sound_irqstatus_w(uint8_t data)
{
	// these look like irq ack bits, 4 sources?
	// related to sound_timer0_w ,  sound_timer1_w,  sound_timer2_w,  sound_timer3_w  ?

	for (int t = 0; t < 4; t++)
	{
		int bit = (1 << t) << 4;

		if (data & bit)
		{
			m_sound_irqstatus &= ~data & bit;
		}
	}

	// check if all interrupts have been cleared to see if the line should be lowered
	if (m_sound_irqstatus & 0xf0)
		m_irqsource |= 0x80;
	else
		m_irqsource &= ~0x80;

	update_irqs();


	for (int t = 0; t < 4; t++)
	{
		int bit = 1 << t;

		if ((m_sound_irqstatus & bit) != (data & bit))
		{
			if (data & bit)
			{
				/* period should be based on m_sndtimer[t] at least, maybe also some other regs?

				   rad_crdn : sound_timer0_w 06
				   ddrfammt, popira etc. : sound_timer3_w 80
				   so higher value definitely needs to be faster? (unless there's another multiplier elsewhere)

				   11 is too fast (popira checked on various tracks, finish before getting to 100% then jump to 100%) where is this multiplier coming from? clock divided?
				   10 seems close to correct for ddrfammt, popira, might need fine tuning.  seems too slow for rad_crdn / rad_bass?
				   tweaked to 10.3f stay in time with the first song in https://www.youtube.com/watch?v=3x1C9bhC2rc

				   the usual clock divided by 2 would be 10.738636 but that's too high
				*/
				attotime period = attotime::from_hz(10.3f * (m_sndtimer[t]));
				m_sound_timer[t]->adjust(period, t, period);
			}
			else
			{
				m_sound_timer[t]->adjust(attotime::never, t);
			}
		}
	}

	// see if we're enabling any timers (should probably check if they're already running so we don't end up restarting them)
	m_sound_irqstatus |= data & 0x0f; // look like IRQ enable flags - 4 sources? voices? timers?


	LOG("%s: sound_irqstatus_w %02x\n", machine().describe_context(), data);
}



void xavix_state::sound_75ff_w(uint8_t data)
{
	m_unk_snd75ff = data;
	LOG("%s: sound_75ff_w %02x\n", machine().describe_context(), data);
}

// used by ekara (UK cartridges), rad_bass, rad_crdn
TIMER_CALLBACK_MEMBER(xavix_state::sound_timer_done)
{
	// param = timer number 0,1,2 or 3
	int bit = (1 << param) << 4;
	m_sound_irqstatus |= bit;

	// if any of the sound timers are causing an interrupt...
	if (m_sound_irqstatus & 0xf0)
		m_irqsource |= 0x80;
	else
		m_irqsource &= ~0x80;

	update_irqs();
}
