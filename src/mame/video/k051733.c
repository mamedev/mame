// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
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
02-03 R operand 1 % operand 2?
04-05 R sqrt(operand 3<<16)
06    R unknown - return value written to 13?

06-07 W distance for collision check
08-09 W Y pos of obj1
0a-0b W X pos of obj1
0c-0d W Y pos of obj2
0e-0f W X pos of obj2
13    W unknown

07    R collision (0x80 = no, 0x00 = yes)
0a-0b R unknown (chequered flag), might just read back X pos
0e-0f R unknown (chequered flag), might just read back X pos

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
#include "konami_helper.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

const device_type K051733 = &device_creator<k051733_device>;

k051733_device::k051733_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K051733, "K051733 Protection", tag, owner, clock, "k051733", __FILE__),
	//m_ram[0x20],
	m_rng(0)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void k051733_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k051733_device::device_start()
{
	save_item(NAME(m_ram));
	save_item(NAME(m_rng));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k051733_device::device_reset()
{
	int i;

	for (i = 0; i < 0x20; i++)
		m_ram[i] = 0;

	m_rng = 0;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

WRITE8_MEMBER( k051733_device::write )
{
	//logerror("%04x: write %02x to 051733 address %02x\n", space.device().safe_pc(), data, offset);

	m_ram[offset] = data;
}


static int k051733_int_sqrt( UINT32 op )
{
	UINT32 i = 0x8000;
	UINT32 step = 0x4000;

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
	return i;
}

READ8_MEMBER( k051733_device::read )
{
	int op1 = (m_ram[0x00] << 8) | m_ram[0x01];
	int op2 = (m_ram[0x02] << 8) | m_ram[0x03];
	int op3 = (m_ram[0x04] << 8) | m_ram[0x05];

	int rad = (m_ram[0x06] << 8) | m_ram[0x07];
	int yobj1c = (m_ram[0x08] << 8) | m_ram[0x09];
	int xobj1c = (m_ram[0x0a] << 8) | m_ram[0x0b];
	int yobj2c = (m_ram[0x0c] << 8) | m_ram[0x0d];
	int xobj2c = (m_ram[0x0e] << 8) | m_ram[0x0f];

	switch (offset)
	{
		case 0x00:
			if (op2)
				return (op1 / op2) >> 8;
			else
				return 0xff;
		case 0x01:
			if (op2)
				return (op1 / op2) & 0xff;
			else
				return 0xff;

		/* this is completely unverified */
		case 0x02:
			if (op2)
				return (op1 % op2) >> 8;
			else
				return 0xff;
		case 0x03:
			if (op2)
				return (op1 % op2) & 0xff;
			else
				return 0xff;

		case 0x04:
			return k051733_int_sqrt(op3 << 16) >> 8;

		case 0x05:
			return k051733_int_sqrt(op3 << 16) & 0xff;

		case 0x06:
			m_rng += m_ram[0x13];
			return m_rng; //RNG read, used by Chequered Flag for differentiate cars, implementation is a raw guess

		case 0x07:{ /* note: Chequered Flag definitely wants all these bits to be enabled */
			if (xobj1c + rad < xobj2c)
				return 0xff;

			if (xobj2c + rad < xobj1c)
				return 0xff;

			if (yobj1c + rad < yobj2c)
				return 0xff;

			if (yobj2c + rad < yobj1c)
				return 0xff;

			return 0;
		}
		case 0x0e: /* best guess */
			return (xobj2c - xobj1c) >> 8;
		case 0x0f:
			return (xobj2c - xobj1c) & 0xff;
		default:
			return m_ram[offset];
	}
}
