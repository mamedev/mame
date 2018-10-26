// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "includes/xavix.h"

// #define VERBOSE 1
#include "logmacro.h"

READ8_MEMBER(xavix_state::sound_75f0_r)
{
	LOG("%s: sound_75f0_r\n", machine().describe_context());
	return m_soundregs[0];
}

READ8_MEMBER(xavix_state::sound_75f1_r)
{
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
	// expected to return data written
	m_soundregs[0] = data;
	LOG("%s: sound_75f0_w %02x\n", machine().describe_context(), data);
}


WRITE8_MEMBER(xavix_state::sound_75f1_w)
{
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
	// expected to return data written
	m_soundregs[10] = data;
	LOG("%s: sound_75fa_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::sound_75fb_w)
{
	// expected to return data written
	m_soundregs[11] = data;
	LOG("%s: sound_75fb_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::sound_75fc_w)
{
	// expected to return data written
	m_soundregs[12] = data;
	LOG("%s: sound_75fc_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::sound_75fd_w)
{
	// expected to return data written
	m_soundregs[13] = data;
	LOG("%s: sound_75fd_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::sound_75fe_w)
{
	m_soundregs[14] = data;
	LOG("%s: sound_75fe_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::sound_75ff_w)
{
	m_soundregs[15] = data;
	LOG("%s: sound_75ff_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(xavix_state::sound_75f4_r)
{
	// used with 75f0
	return 0xff;
}

READ8_MEMBER(xavix_state::sound_75f5_r)
{
	// used with 75f1
	return 0xff;
}
