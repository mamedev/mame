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
06    R random number (LFSR)

06-07 W distance for collision check
08-09 W Y pos of obj1
0a-0b W X pos of obj1
0c-0d W Y pos of obj2
0e-0f W X pos of obj2
07    R collision flags

08-1f R mirror of 00-07

10-11 W NMI timer (triggers NMI on overflow)
12    W NMI enable (bit 0)
13    W unknown

Other addresses are unknown or unused.
*/

#include "emu.h"
#include "k051733.h"

#define VERBOSE 0
#include "logmacro.h"


DEFINE_DEVICE_TYPE(K051733, k051733_device, "k051733", "Konami 051733 math chip")

k051733_device::k051733_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
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
	save_item(NAME(m_lfsr));
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

	m_lfsr = 0xff;
	m_nmi_timer = 0;
	m_nmi_cb(0);
}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void k051733_device::clock_lfsr()
{
	const int feedback = BIT(m_lfsr, 1) ^ BIT(m_lfsr, 8) ^ BIT(m_lfsr, 12);
	m_lfsr = m_lfsr << 1 | feedback;
}

u32 k051733_device::u32_sqrt(u32 op)
{
	if (op)
		return u32(sqrt(op)) & ~1;
	else
		return 0;
}

u8 k051733_device::read(offs_t offset)
{
	offset &= 0x07;

	u8 const lfsr = m_lfsr & 0xff;
	if (!machine().side_effects_disabled())
		clock_lfsr();

	s16 const op1 = (m_ram[0x00] << 8) | m_ram[0x01];
	s16 const op2 = (m_ram[0x02] << 8) | m_ram[0x03];
	u16 const op3 = (m_ram[0x04] << 8) | m_ram[0x05];

	u16 const rad = (m_ram[0x06] << 8) | m_ram[0x07];
	u16 const y1 = (m_ram[0x08] << 8) | m_ram[0x09];
	u16 const x1 = (m_ram[0x0a] << 8) | m_ram[0x0b];
	u16 const y2 = (m_ram[0x0c] << 8) | m_ram[0x0d];
	u16 const x2 = (m_ram[0x0e] << 8) | m_ram[0x0f];

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
			return u32_sqrt(op3 << 16) >> 8;
		case 0x05:
			return u32_sqrt(op3 << 16) & 0xff;

		case 0x06:
			return lfsr;

		case 0x07:
		{
			const int dx = x2 - x1;
			const int dy = y2 - y1;

			// note: Chequered Flag definitely wants all these bits to be set
			if (abs(dx) > rad || abs(dy) > rad)
				return 0xff;

			// octant angle
			if (y1 >= y2)
			{
				if (x1 >= x2)
				{
					if (dy >= dx)
						return 0x00;
					else
						return 0x06;
				}
				else
				{
					if (dy >= -dx)
						return 0x04;
					else
						return 0x07;
				}
			}
			else
			{
				if (x1 >= x2)
				{
					if (dy > -dx)
						return 0x02;
					else
						return 0x01;
				}
				else
				{
					if (dy > dx)
						return 0x03;
					else
						return 0x05;
				}
			}
		}
	}

	return 0;
}

void k051733_device::write(offs_t offset, u8 data)
{
	offset &= 0x1f;
	LOG("%s: write %02x to 051733 address %02x\n", machine().describe_context(), data, offset);

	m_ram[offset] = data;
	clock_lfsr();
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
