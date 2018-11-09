// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "includes/xavix.h"

#define VERBOSE 1
#include "logmacro.h"

// 16 stereo voices?

// xavix_sound_device

DEFINE_DEVICE_TYPE(XAVIX_SOUND, xavix_sound_device, "xavix_sound", "XaviX Sound")

xavix_sound_device::xavix_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XAVIX_SOUND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_readregs_cb(*this)
	, m_readsamples_cb(*this)
{
}

void xavix_sound_device::device_start()
{
	m_readregs_cb.resolve_safe(0xff);
	m_readsamples_cb.resolve_safe(0x80);

	m_stream = stream_alloc(0, 1, 8000);
}

void xavix_sound_device::device_reset()
{
	for (int v = 0; v < 16; v++)
	{
		m_voice[v].enabled = false;
		m_voice[v].position = 0x00;
		m_voice[v].bank = 0x00;
	}
}


void xavix_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// reset the output stream
	memset(outputs[0], 0, samples * sizeof(*outputs[0]));

	int outpos = 0;
	// loop while we still have samples to generate
	while (samples-- != 0)
	{
		for (int v = 0; v < 16; v++)
		{
			if (m_voice[v].enabled == true)
			{
				int8_t sample = m_readsamples_cb((m_voice[v].bank << 16) | m_voice[v].position);

				if ((uint8_t)sample == 0x80)
				{
					m_voice[v].enabled = false;
					break;
				}

				outputs[0][outpos] += sample * 0x100;
				m_voice[v].position++;
			}
		}
		outpos++;
	}
}

void xavix_sound_device::enable_voice(int voice)
{
	LOG("voice %d 0->1 ", voice);
	int voicemembase = voice * 0x10;

	uint16_t param1 = (m_readregs_cb(voicemembase + 0x1) << 8) | (m_readregs_cb(voicemembase + 0x0)); // sample rate maybe?
	uint16_t param2 = (m_readregs_cb(voicemembase + 0x3) << 8) | (m_readregs_cb(voicemembase + 0x2)); // seems to be a start position
	uint16_t param3 = (m_readregs_cb(voicemembase + 0x5) << 8) | (m_readregs_cb(voicemembase + 0x4)); // another start position? sometimes same as param6
	uint8_t param4a = (m_readregs_cb(voicemembase + 0x7));
	uint8_t param4b = (m_readregs_cb(voicemembase + 0x6)); // upper 8 bits of memory address? 8 bits unused?

	// these don't seem to be populated as often, maybe some kind of effect / envelope filter?
	uint8_t param5a = (m_readregs_cb(voicemembase + 0x9));
	uint8_t param5b = (m_readregs_cb(voicemembase + 0x8));
	uint16_t param6 = (m_readregs_cb(voicemembase + 0xb) << 8) | (m_readregs_cb(voicemembase + 0xa)); // seems to be a start position
	uint16_t param7 = (m_readregs_cb(voicemembase + 0xd) << 8) | (m_readregs_cb(voicemembase + 0xc)); // another start position? sometimes same as param6
	uint8_t param8a = (m_readregs_cb(voicemembase + 0xf));
	uint8_t param8b = (m_readregs_cb(voicemembase + 0xe)); // upper 8 bits of memory address? 8 bits unused (or not unused?, get populated with increasing values sometimes?)
	LOG(" (params %04x %04x %04x %02x %02x     %02x %02x  %04x %04x %02x %02x)\n", param1, param2, param3, param4a, param4b, param5a, param5b, param6, param7, param8a, param8b);

	uint32_t address1 = (param2 | param4b << 16) & 0x00ffffff; // definitely addresses based on rad_snow
	uint32_t address2 = (param3 | param4b << 16) & 0x00ffffff;

	uint32_t address3 = (param6 | param8b << 16) & 0x00ffffff; // still looks like addresses, sometimes pointing at RAM
	uint32_t address4 = (param7 | param8b << 16) & 0x00ffffff;


	LOG(" (possible meanings mode %01x rate %04x address1 %08x address2 %08x address3 %08x address4 %08x)\n", param1 & 0x3, param1 >> 2, address1, address2, address3, address4);

	m_voice[voice].enabled = true;

	m_voice[voice].bank = param4b;
	m_voice[voice].position = param2; // param3

	// samples appear to be PCM, 0x80 terminated
}



// xavix_state support

READ8_MEMBER(xavix_state::sound_regram_read_cb)
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
READ8_MEMBER(xavix_state::sound_reg16_0_r)
{
	LOG("%s: sound_reg16_0_r %02x\n", machine().describe_context(), offset);
	return m_soundreg16_0[offset];
}

WRITE8_MEMBER(xavix_state::sound_reg16_0_w)
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
		LOG("%s: sound_reg16_0_w %02x, %02x (%d %d %d %d %d %d %d %d - - - - - - - -)\n", machine().describe_context(), offset, data, (data & 0x01) ? 1 : 0, (data & 0x02) ? 1 : 0, (data & 0x04) ? 1 : 0, (data & 0x08) ? 1 : 0, (data & 0x10) ? 1 : 0, (data & 0x20) ? 1 : 0, (data & 0x40) ? 1 : 0, (data & 0x80) ? 1 : 0);
	else
		LOG("%s: sound_reg16_0_w %02x, %02x (- - - - - - - - %d %d %d %d %d %d %d %d)\n", machine().describe_context(), offset, data, (data & 0x01) ? 1 : 0, (data & 0x02) ? 1 : 0, (data & 0x04) ? 1 : 0, (data & 0x08) ? 1 : 0, (data & 0x10) ? 1 : 0, (data & 0x20) ? 1 : 0, (data & 0x40) ? 1 : 0, (data & 0x80) ? 1 : 0);


	for (int i = 0; i < 8; i++)
	{
		int voice_state = (data & (1 << i));
		int old_voice_state = (m_soundreg16_0[offset] & (1 << i));
		if (voice_state != old_voice_state)
		{
			if (voice_state)
			{
				int voice = (offset * 8 + i);

				m_sound->enable_voice(voice);
			}
		}
	}

	m_soundreg16_0[offset] = data;

}

/* 75f0, 75f1 - 2x8 bits (16 voices?) */
READ8_MEMBER(xavix_state::sound_reg16_1_r)
{
	LOG("%s: sound_reg16_1_r %02x\n", machine().describe_context(), offset);
	return m_soundreg16_1[offset];
}

WRITE8_MEMBER(xavix_state::sound_reg16_1_w)
{
	m_soundreg16_1[offset] = data;

	if (offset == 0)
		LOG("%s: sound_reg16_1_w %02x, %02x (%d %d %d %d %d %d %d %d - - - - - - - -)\n", machine().describe_context(), offset, data, (data & 0x01) ? 1 : 0, (data & 0x02) ? 1 : 0, (data & 0x04) ? 1 : 0, (data & 0x08) ? 1 : 0, (data & 0x10) ? 1 : 0, (data & 0x20) ? 1 : 0, (data & 0x40) ? 1 : 0, (data & 0x80) ? 1 : 0);
	else
		LOG("%s: sound_reg16_1_w %02x, %02x (- - - - - - - - %d %d %d %d %d %d %d %d)\n", machine().describe_context(), offset, data, (data & 0x01) ? 1 : 0, (data & 0x02) ? 1 : 0, (data & 0x04) ? 1 : 0, (data & 0x08) ? 1 : 0, (data & 0x10) ? 1 : 0, (data & 0x20) ? 1 : 0, (data & 0x40) ? 1 : 0, (data & 0x80) ? 1 : 0);
}


/* 75f4, 75f5 - 2x8 bits (16 voices?) status? */
READ8_MEMBER(xavix_state::sound_sta16_r)
{
	// used with 75f0/75f1
	return machine().rand();
}

/* 75f6 - master volume control? */
READ8_MEMBER(xavix_state::sound_volume_r)
{
	LOG("%s: sound_volume_r\n", machine().describe_context());
	return m_soundregs[6];
}

WRITE8_MEMBER(xavix_state::sound_volume_w)
{
	m_soundregs[6] = data;
	LOG("%s: sound_volume_w %02x\n", machine().describe_context(), data);
}

/* 75f7 - main register base*/

WRITE8_MEMBER(xavix_state::sound_regbase_w)
{
	// this is the upper 6 bits of the RAM address where the actual sound register sets are
	// (16x16 regs, so complete 0x100 bytes of RAM eg 0x3b means the complete 0x3b00 - 0x3bff range with 0x3b00 - 0x3b0f being voice 1 etc)
	m_sound_regbase = data;
	LOG("%s: sound_regbase_w %02x (sound regs are at 0x%02x00 to 0x%02xff)\n", machine().describe_context(), data, m_sound_regbase & 0x3f, m_sound_regbase & 0x3f);
}

/* 75f8, 75f9 - misc unknown sound regs*/

READ8_MEMBER(xavix_state::sound_75f8_r)
{
	LOG("%s: sound_75f8_r\n", machine().describe_context());
	return m_soundregs[8];
}

WRITE8_MEMBER(xavix_state::sound_75f8_w)
{
	m_soundregs[8] = data;
	LOG("%s: sound_75f8_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(xavix_state::sound_75f9_r)
{
	LOG("%s: sound_75f9_r\n", machine().describe_context());
	return 0x00;
}

WRITE8_MEMBER(xavix_state::sound_75f9_w)
{
	m_soundregs[9] = data;
	LOG("%s: sound_75f9_w %02x\n", machine().describe_context(), data);
}

/* 75fa, 75fb, 75fc, 75fd - timers?? generate interrupts?? */

READ8_MEMBER(xavix_state::sound_timer0_r)
{
	LOG("%s: sound_timer0_r\n", machine().describe_context());
	return m_soundregs[10];
}

WRITE8_MEMBER(xavix_state::sound_timer0_w)
{
	m_soundregs[10] = data;
	LOG("%s: sound_timer0_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(xavix_state::sound_timer1_r)
{
	LOG("%s: sound_timer1_r\n", machine().describe_context());
	return m_soundregs[11];
}

WRITE8_MEMBER(xavix_state::sound_timer1_w)
{
	m_soundregs[11] = data;
	LOG("%s: sound_timer1_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(xavix_state::sound_timer2_r)
{
	LOG("%s: sound_timer2_r\n", machine().describe_context());
	return m_soundregs[12];
}

WRITE8_MEMBER(xavix_state::sound_timer2_w)
{
	m_soundregs[12] = data;
	LOG("%s: sound_timer2_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(xavix_state::sound_timer3_r)
{
	LOG("%s: sound_timer3_r\n", machine().describe_context());
	return m_soundregs[13];
}

WRITE8_MEMBER(xavix_state::sound_timer3_w)
{
	m_soundregs[13] = data;
	LOG("%s: sound_timer3_w %02x\n", machine().describe_context(), data);
}

/* 75fe - some kind of IRQ status / Timer Status? */

READ8_MEMBER(xavix_state::sound_irqstatus_r)
{
	// rad_rh checks this after doing something that looks like an irq ack
	// rad_bass does the same, but returning the wrong status bits causes it to corrupt memory and crash in certain situations, see code around 0037D5
	if (m_sound_irqstatus & 0x08) // hack for rad_rh
		return 0xf0 | m_sound_irqstatus;

	return m_sound_irqstatus; // otherwise, keep rad_bass happy
}

WRITE8_MEMBER(xavix_state::sound_irqstatus_w)
{
	// these look like irq ack bits, 4 sources?
	// related to sound_timer0_w ,  sound_timer1_w,  sound_timer2_w,  sound_timer3_w  ?
	if (data & 0xf0)
	{
		m_sound_irqstatus &= ~data & 0xf0;
	}

	m_sound_irqstatus = data & 0x0f; // look like IRQ enable flags - 4 sources? voices? timers?

	LOG("%s: sound_irqstatus_w %02x\n", machine().describe_context(), data);
}



WRITE8_MEMBER(xavix_state::sound_75ff_w)
{
	m_soundregs[15] = data;
	LOG("%s: sound_75ff_w %02x\n", machine().describe_context(), data);
}

