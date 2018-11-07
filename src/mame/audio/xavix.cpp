// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "includes/xavix.h"

#define VERBOSE 1
#include "logmacro.h"

// 16 stereo channels?

/* 75f0, 75f1 - 2x8 bits (16 channels?) */
READ8_MEMBER(xavix_state::sound_reg16_0_r)
{
	LOG("%s: sound_reg16_0_r %02x\n", machine().describe_context(), offset);
	return m_soundreg16_0[offset];
}

WRITE8_MEMBER(xavix_state::sound_reg16_0_w)
{
	/* looks like the sound triggers

	  offset 0
	  data & 0x01 - channel 0  (registers at regbase + 0x00) eg 0x3b00 - 0x3b0f in monster truck
	  data & 0x02 - channel 1  (registers at regbase + 0x10) eg 0x3b10 - 0x3b1f in monster truck
	  data & 0x04 - channel 2  
	  data & 0x08 - channel 3
	  data & 0x10 - channel 4
	  data & 0x20 - channel 5
	  data & 0x40 - channel 6
	  data & 0x80 - channel 7

	  offset 1
	  data & 0x01 - channel 8
	  data & 0x02 - channel 9
	  data & 0x04 - channel 10
	  data & 0x08 - channel 11
	  data & 0x10 - channel 12
	  data & 0x20 - channel 13
	  data & 0x40 - channel 14 (registers at regbase + 0xf0) eg 0x3be0 - 0x3bef in monster truck
	  data & 0x80 - channel 15 (registers at regbase + 0xf0) eg 0x3bf0 - 0x3bff in monster truck
*/
	if (offset == 0)
		LOG("%s: sound_reg16_0_w %02x, %02x (%d %d %d %d %d %d %d %d - - - - - - - -)\n", machine().describe_context(), offset, data, (data & 0x01) ? 1 : 0, (data & 0x02) ? 1 : 0, (data & 0x04) ? 1 : 0, (data & 0x08) ? 1 : 0, (data & 0x10) ? 1 : 0, (data & 0x20) ? 1 : 0, (data & 0x40) ? 1 : 0, (data & 0x80) ? 1 : 0);
	else
		LOG("%s: sound_reg16_0_w %02x, %02x (- - - - - - - - %d %d %d %d %d %d %d %d)\n", machine().describe_context(), offset, data, (data & 0x01) ? 1 : 0, (data & 0x02) ? 1 : 0, (data & 0x04) ? 1 : 0, (data & 0x08) ? 1 : 0, (data & 0x10) ? 1 : 0, (data & 0x20) ? 1 : 0, (data & 0x40) ? 1 : 0, (data & 0x80) ? 1 : 0);


	for (int i = 0; i < 8; i++)
	{
		int channel_state = (data & (1<<i));
		int old_channel_state = (m_soundreg16_0[offset] & (1<<i));
		if (channel_state != old_channel_state)
		{
			if (channel_state)
			{
				int channel = (offset*8 + i);

				LOG("channel %d 0->1 ", channel);

				uint16_t memorybase = ((m_sound_regbase & 0x3f) << 8) | (channel * 0x10);

				uint16_t param1 = (m_mainram[memorybase + 0x0]<<8) | (m_mainram[memorybase + 0x1]);
				uint16_t param2 = (m_mainram[memorybase + 0x2]<<8) | (m_mainram[memorybase + 0x3]);
				uint16_t param3 = (m_mainram[memorybase + 0x4]<<8) | (m_mainram[memorybase + 0x5]);
				uint16_t param4 = (m_mainram[memorybase + 0x6]<<8) | (m_mainram[memorybase + 0x7]);
				uint16_t param5 = (m_mainram[memorybase + 0x8]<<8) | (m_mainram[memorybase + 0x9]);
				uint16_t param6 = (m_mainram[memorybase + 0xa]<<8) | (m_mainram[memorybase + 0xb]); // seems to be a start position
				uint16_t param7 = (m_mainram[memorybase + 0xc]<<8) | (m_mainram[memorybase + 0xd]); // another start position? sometimes same as param6
				uint16_t param8 = (m_mainram[memorybase + 0xe]<<8) | (m_mainram[memorybase + 0xf]); // upper 8 bits of memory address? 8 bits unused?
				LOG(" (params %04x %04x %04x %04x   %04x %04x %04x %04x)\n", param1, param2, param3, param4, param5, param6, param7, param8);

				// samples appear to be PCM, 0x80 terminated
			}
		}
	}

	m_soundreg16_0[offset] = data;

}

/* 75f0, 75f1 - 2x8 bits (16 channels?) */
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


/* 75f4, 75f5 - 2x8 bits (16 channels?) status? */
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
	// (16x16 regs, so complete 0x100 bytes of RAM eg 0x3b means the complete 0x3b00 - 0x3bff range with 0x3b00 - 0x3b0f being channel 1 etc)
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

	m_sound_irqstatus = data & 0x0f; // look like IRQ enable flags - 4 sources? channels? timers?

	LOG("%s: sound_irqstatus_w %02x\n", machine().describe_context(), data);
}



WRITE8_MEMBER(xavix_state::sound_75ff_w)
{
	m_soundregs[15] = data;
	LOG("%s: sound_75ff_w %02x\n", machine().describe_context(), data);
}

