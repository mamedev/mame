// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Acho A. Tang, R. Belmont
/*
Konami 051733
------
Sort of a protection device, used for collision detection, and for
arithmetical operations.
It is passed a few parameters, and returns the result.

Memory map(preliminary):
------------------------
00-01 W operand 1
02-03 W operand 2
04-05 W operand 3

00-01 R operand 1 / operand 2
02-03 R operand 1 % operand 2
04-05 R sqrt(operand 3 << 16)
06    R random number

06-07 W distance for collision check
08-09 W Y pos of obj1
0a-0b W X pos of obj1
0c-0d W Y pos of obj2
0e-0f W X pos of obj2

07    R collision (0xff = no, 0x00 = yes)
0a-0b R unknown (chequered flag), might just read back X pos
0e-0f R unknown (chequered flag), might just read back X pos

10-1f R mirror of 00-0f

10-11 W NMI timer (triggers NMI on overflow)
12    W NMI enable (bit 0)
13    W unknown

Other addresses are unknown or unused.

Fast Lane:
----------
$9def:
This routine is called only after a collision.
(R) 0x0006: unknown. Only bits 0-3 are used.

Blades of Steel:
----------------
$ac2f:
(R) 0x2f86: unknown. Only uses bit 0.

$a5de:
writes to 0x2f84-0x2f85, waits a little, and then reads from 0x2f84.

$7af3:
(R) 0x2f86: unknown. Only uses bit 0.

Devastators:
------------
$6ce8:
reads from 0x0006, and only uses bit 1.
*/

#include "emu.h"
#include "k051733.h"

#define VERBOSE 0
#include "logmacro.h"


DEFINE_DEVICE_TYPE(K051733, k051733_device, "k051733", "Konami 051733 math chip")

k051733_device::k051733_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, K051733, tag, owner, clock),
	m_nmi_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k051733_device::device_start()
{
	m_nmi_clock = 0;

	save_item(NAME(m_ram));
	save_item(NAME(m_nmi_clock));
	save_item(NAME(m_nmi_timer));

	m_nmi_clear = timer_alloc(FUNC(k051733_device::nmi_clear), this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k051733_device::device_reset()
{
	memset(m_ram, 0, sizeof(m_ram));

	m_nmi_timer = 0;
	m_nmi_cb(0);
}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void k051733_device::write(offs_t offset, uint8_t data)
{
	offset &= 0x1f;
	LOG("%s: write %02x to 051733 address %02x\n", machine().describe_context(), data, offset);

	m_ram[offset] = data;
}


inline uint32_t k051733_device::uint_sqrt(uint32_t op)
{
	uint32_t i = 0x8000;
	uint32_t step = 0x4000;

	while (step)
	{
		if (i * i == op)
			return i;
		else if (i * i > op)
			i -= step;
		else
			i += step;
		step >>= 1;
	}
	return i & ~1;
}

uint8_t k051733_device::read(offs_t offset)
{
	offset &= 0x0f;

	uint16_t const op1 = (m_ram[0x00] << 8) | m_ram[0x01];
	uint16_t const op2 = (m_ram[0x02] << 8) | m_ram[0x03];
	uint16_t const op3 = (m_ram[0x04] << 8) | m_ram[0x05];

	uint16_t const rad = (m_ram[0x06] << 8) | m_ram[0x07];
	uint16_t const yobj1c = (m_ram[0x08] << 8) | m_ram[0x09];
	uint16_t const xobj1c = (m_ram[0x0a] << 8) | m_ram[0x0b];
	uint16_t const yobj2c = (m_ram[0x0c] << 8) | m_ram[0x0d];
	uint16_t const xobj2c = (m_ram[0x0e] << 8) | m_ram[0x0f];

	switch (offset)
	{
		case 0x00:
			if (op2)
				return (op1 / op2) >> 8;
			else
				return 0;
		case 0x01:
			if (op2)
				return (op1 / op2) & 0xff;
			else
				return 0;

		case 0x02:
			if (op2)
				return (op1 % op2) >> 8;
			else
				return op1 >> 8;
		case 0x03:
			if (op2)
				return (op1 % op2) & 0xff;
			else
				return op1 & 0xff;

		case 0x04:
			return uint_sqrt(op3 << 16) >> 8;
		case 0x05:
			return uint_sqrt(op3 << 16) & 0xff;

		case 0x06:
			return machine().rand() & 0xff;

		case 0x07: // note: Chequered Flag definitely wants all these bits to be enabled
			if (xobj1c + rad < xobj2c)
				return 0xff;
			else if (xobj2c + rad < xobj1c)
				return 0xff;
			else if (yobj1c + rad < yobj2c)
				return 0xff;
			else if (yobj2c + rad < yobj1c)
				return 0xff;
			else
				return 0;

		case 0x0e: // best guess
			return (xobj2c - xobj1c) >> 8;
		case 0x0f:
			return (xobj2c - xobj1c) & 0xff;

		default:
			return m_ram[offset];
	}
}


/*****************************************************************************
    INTERRUPTS
*****************************************************************************/

void k051733_device::nmiclock_w(int state)
{
	if (!m_nmi_clock && state && !m_nmi_timer--)
	{
		if (BIT(m_ram[0x12], 0))
		{
			// pulse NMI for 4 clocks
			m_nmi_cb(1);
			m_nmi_clear->adjust(attotime::from_ticks(4, clock()));
		}

		m_nmi_timer = (m_ram[0x10] << 8) | m_ram[0x11];
	}

	m_nmi_clock = state;
}
