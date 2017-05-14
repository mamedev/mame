// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    i8243.c

    Intel 8243 Port Expander (for MCS-48)

***************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "i8243.h"

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(I8243, i8243_device, "i8243", "Intel 8243 I/O Expander")

//-------------------------------------------------
//  i8243_device - constructor
//-------------------------------------------------

i8243_device::i8243_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, I8243, tag, owner, clock)
	, m_p2out(0), m_p2(0), m_opcode(0), m_prog(0)
	, m_readhandler(*this)
	, m_writehandler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8243_device::device_start()
{
	m_readhandler.resolve_safe(0);
	m_writehandler.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8243_device::device_reset()
{
	m_p2 = 0x0f;
	m_p2out = 0x0f;
	m_prog = 1;
}


/*-------------------------------------------------
    i8243_p2_r - handle a read from port 2
-------------------------------------------------*/

READ8_MEMBER(i8243_device::p2_r)
{
	return m_p2out;
}


/*-------------------------------------------------
    i8243_p2_r - handle a write to port 2
-------------------------------------------------*/

WRITE8_MEMBER(i8243_device::p2_w)
{
	m_p2 = data & 0x0f;
}


/*-------------------------------------------------
    i8243_prog_w - handle a change in the PROG
    line state
-------------------------------------------------*/

WRITE_LINE_MEMBER(i8243_device::prog_w)
{
	/* on high->low transition state, latch opcode/port */
	if (m_prog && !state)
	{
		m_opcode = m_p2;

		/* if this is a read opcode, copy result to p2out */
		if ((m_opcode >> 2) == mcs48_cpu_device::EXPANDER_OP_READ)
		{
			if (m_readhandler.isnull())
			{
				m_p[m_opcode & 3] = m_readhandler(m_opcode & 3);
			}
			m_p2out = m_p[m_opcode & 3] & 0x0f;
		}
	}

	/* on low->high transition state, act on opcode */
	else if (!m_prog && state)
	{
		switch (m_opcode >> 2)
		{
			case mcs48_cpu_device::EXPANDER_OP_READ:
				break; // handled above

			case mcs48_cpu_device::EXPANDER_OP_WRITE:
				m_p[m_opcode & 3] = m_p2 & 0x0f;
				m_writehandler((offs_t)(m_opcode & 3), m_p[m_opcode & 3]);
				break;

			case mcs48_cpu_device::EXPANDER_OP_OR:
				m_p[m_opcode & 3] |= m_p2 & 0x0f;
				m_writehandler((offs_t)(m_opcode & 3), m_p[m_opcode & 3]);
				break;

			case mcs48_cpu_device::EXPANDER_OP_AND:
				m_p[m_opcode & 3] &= m_p2 & 0x0f;
				m_writehandler((offs_t)(m_opcode & 3), m_p[m_opcode & 3]);
				break;
		}
	}

	/* remember the state */
	m_prog = state;
}
