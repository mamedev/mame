// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "includes/xavix.h"

// #define VERBOSE 1
#include "logmacro.h"

// 16 channel sound?

READ8_MEMBER(xavix_state::sound_75f0_r)
{
	// something? (same as 75f1) 1 bit per channel, 8 channels
	LOG("%s: sound_75f0_r\n", machine().describe_context());
	return m_soundregs[0];
}

READ8_MEMBER(xavix_state::sound_75f1_r)
{
	// something? (same as 75f0) 1 bit per channel, 8 channels
	LOG("%s: sound_75f1_r\n", machine().describe_context());
	return m_soundregs[1];
}

READ8_MEMBER(xavix_state::sound_75f6_r)
{
	LOG("%s: sound_75f6_r\n", machine().describe_context());
	return m_soundregs[6];
}

READ8_MEMBER(xavix_state::sound_75f8_r)
{
	LOG("%s: sound_75f8_r\n", machine().describe_context());
	return m_soundregs[8];
}

READ8_MEMBER(xavix_state::sound_75f9_r)
{
	LOG("%s: sound_75f9_r\n", machine().describe_context());
	return 0x00;
}

READ8_MEMBER(xavix_state::sound_75fa_r)
{
	LOG("%s: sound_75fa_r\n", machine().describe_context());
	return m_soundregs[10];
}

READ8_MEMBER(xavix_state::sound_75fb_r)
{
	LOG("%s: sound_75fb_r\n", machine().describe_context());
	return m_soundregs[11];
}

READ8_MEMBER(xavix_state::sound_75fc_r)
{
	LOG("%s: sound_75fc_r\n", machine().describe_context());
	return m_soundregs[12];
}

READ8_MEMBER(xavix_state::sound_75fd_r)
{
	LOG("%s: sound_75fd_r\n", machine().describe_context());
	return m_soundregs[13];
}




WRITE8_MEMBER(xavix_state::sound_75f0_w)
{
	// something? (same as 75f1) 1 bit per channel, 8 channels

	// expected to return data written
	m_soundregs[0] = data;
	LOG("%s: sound_75f0_w %02x\n", machine().describe_context(), data);
}


WRITE8_MEMBER(xavix_state::sound_75f1_w)
{
	// something? (same as 75f0) 1 bit per channel, 8 channels

	// expected to return data written
	m_soundregs[1] = data;
	LOG("%s: sound_75f1_w %02x\n", machine().describe_context(), data);
}


WRITE8_MEMBER(xavix_state::sound_75f6_w)
{
	// expected to return data written
	m_soundregs[6] = data;
	LOG("%s: sound_75f6_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::sound_75f7_w)
{
	m_soundregs[7] = data;
	LOG("%s: sound_75f7_w %02x\n", machine().describe_context(), data);
}


WRITE8_MEMBER(xavix_state::sound_75f8_w)
{
	// expected to return data written
	m_soundregs[8] = data;
	LOG("%s: sound_75f8_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::sound_75f9_w)
{
	m_soundregs[9] = data;
	LOG("%s: sound_75f9_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::sound_75fa_w)
{
	// timer? frequency? reg 0

	// expected to return data written
	m_soundregs[10] = data;
	LOG("%s: sound_75fa_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::sound_75fb_w)
{
	// timer? frequency? reg 1

	// expected to return data written
	m_soundregs[11] = data;
	LOG("%s: sound_75fb_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::sound_75fc_w)
{
	// timer? frequency? reg 2

	// expected to return data written
	m_soundregs[12] = data;
	LOG("%s: sound_75fc_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::sound_75fd_w)
{
	// timer? frequency? reg 3

	// expected to return data written
	m_soundregs[13] = data;
	LOG("%s: sound_75fd_w %02x\n", machine().describe_context(), data);
}

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
	// related to sound_75fa_w ,  sound_75fb_w,  sound_75fc_w,  sound_75fd_w  ?
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

READ8_MEMBER(xavix_state::sound_75f4_r)
{
	// status? 1 bit per channel, 8 channels?

	// used with 75f0
	return 0xff;
}

READ8_MEMBER(xavix_state::sound_75f5_r)
{
	// status? 1 bit per channel, 8 channels?

	// used with 75f1
	return 0xff;
}
