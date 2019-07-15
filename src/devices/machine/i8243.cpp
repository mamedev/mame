// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    i8243.c

    Intel 8243 Input/Output Expander (for MCS-48)

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
	, m_p{0, 0, 0, 0}
	, m_p2out(0x0f), m_p2(0x0f), m_opcode(0), m_prog(1), m_cs(0)
	, m_readhandler{{*this}, {*this}, {*this}, {*this}}
	, m_writehandler{{*this}, {*this}, {*this}, {*this}}
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8243_device::device_start()
{
	for (auto &cb : m_readhandler)
		cb.resolve();
	for (auto &cb : m_writehandler)
		cb.resolve();

	save_item(NAME(m_p));
	save_item(NAME(m_p2out));
	save_item(NAME(m_p2));
	save_item(NAME(m_opcode));
	save_item(NAME(m_prog));
	save_item(NAME(m_cs));
}


//-------------------------------------------------
//  p2_r - handle a read from the MCU port
//-------------------------------------------------

uint8_t i8243_device::p2_r()
{
	return m_p2out;
}


//-------------------------------------------------
//  p2_w - handle a write to the MCU port
//-------------------------------------------------

void i8243_device::p2_w(uint8_t data)
{
	m_p2 = data & 0x0f;
}


//-------------------------------------------------
//  output_update - helper for port writes
//-------------------------------------------------

void i8243_device::output_update(int which)
{
	if (m_writehandler[which].isnull())
		logerror("%s: Unconfigured write to P%d (%01X)\n", machine().describe_context(), which + 4, m_p[which]);
	else
		m_writehandler[which](m_p[which]);
}


//-------------------------------------------------
//  prog_w - handle a change in the PROG line
//  state
//-------------------------------------------------

WRITE_LINE_MEMBER(i8243_device::prog_w)
{
	/* on high->low transition state, latch opcode/port */
	if (m_prog && !state && !m_cs)
	{
		m_opcode = m_p2;

		/* if this is a read opcode, copy result to p2out */
		if ((m_opcode >> 2) == mcs48_cpu_device::EXPANDER_OP_READ)
		{
			int which = m_opcode & 3;
			if (m_readhandler[which].isnull())
				logerror("%s: Unconfigured read from P%d\n", machine().describe_context(), which + 4);
			else
				m_p[which] = m_readhandler[which]();
			m_p2out = m_p[which] & 0x0f;
		}
		else
			m_p2out = 0x0f;
	}

	/* on low->high transition state, act on opcode */
	else if (!m_prog && state && !m_cs)
	{
		switch (m_opcode >> 2)
		{
			case mcs48_cpu_device::EXPANDER_OP_READ:
				m_p2out = 0x0f; // release expander bus
				break;

			case mcs48_cpu_device::EXPANDER_OP_WRITE:
				m_p[m_opcode & 3] = m_p2 & 0x0f;
				output_update(m_opcode & 3);
				break;

			case mcs48_cpu_device::EXPANDER_OP_OR:
				m_p[m_opcode & 3] |= m_p2 & 0x0f;
				output_update(m_opcode & 3);
				break;

			case mcs48_cpu_device::EXPANDER_OP_AND:
				m_p[m_opcode & 3] &= m_p2 & 0x0f;
				output_update(m_opcode & 3);
				break;
		}
	}

	/* remember the state */
	m_prog = state;
}


//-------------------------------------------------
//  cs_w - handle chip select line (active low)
//-------------------------------------------------

WRITE_LINE_MEMBER(i8243_device::cs_w)
{
	m_cs = state;
	if (m_cs)
		m_p2out = 0x0f;
}
