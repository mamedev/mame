// license:BSD-3-Clause
// copyright-holders:hap, Charles MacDonald
/**********************************************************************

    Sega 315-5296 I/O chip

    Sega 100-pin QFP, with 8 bidirectional I/O ports, and 3 output pins.
    It also has chip select(/FMCS) and clock(CKOT) for a peripheral device.
    Commonly used from the late 80s up until Sega Model 2.

    The I/O chip has 64 addresses:
    $00-0F : I/O ports, security, configuration registers
    $10-1F : Unused (no effect when read or written)
    $20-3F : Unused (enables /FMCS output, eg. to YM2151 /CS)

    On System 16 derivatives, the unused locations return the 68000 prefetch
    value off the bus when read.


    TODO:
    - complete emulation of CNT register

**********************************************************************/

#include "emu.h"
#include "315_5296.h"


DEFINE_DEVICE_TYPE(SEGA_315_5296, sega_315_5296_device, "315_5296", "Sega 315-5296 I/O")

//-------------------------------------------------
//  sega_315_5296_device - constructor
//-------------------------------------------------

sega_315_5296_device::sega_315_5296_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SEGA_315_5296, tag, owner, clock),
	m_in_port_cb(*this),
	m_out_port_cb(*this),
	m_out_cnt_cb(*this),
	m_dir_override(0xff)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega_315_5296_device::device_start()
{
	// resolve callbacks
	m_in_port_cb.resolve_all_safe(0xff);
	m_out_port_cb.resolve_all_safe();
	m_out_cnt_cb.resolve_all_safe();

	// register for savestates
	save_item(NAME(m_output_latch));
	save_item(NAME(m_cnt));
	save_item(NAME(m_dir));
	save_item(NAME(m_dir_override));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sega_315_5296_device::device_reset()
{
	// all ports are set to input
	m_dir = 0;

	// clear output ports
	memset(m_output_latch, 0, sizeof(m_output_latch));
	m_cnt = 0;

	for (int i = 0; i < 8; i++)
		m_out_port_cb[i]((offs_t)i, 0);
	for (auto & elem : m_out_cnt_cb)
		elem(0);
}


//-------------------------------------------------

uint8_t sega_315_5296_device::read(offs_t offset)
{
	offset &= 0x3f;

	switch (offset)
	{
		// port A to H
		case 0x0: case 0x1: case 0x2: case 0x3: case 0x4: case 0x5: case 0x6: case 0x7:
			// if the port is configured as an output, return the last thing written
			if (m_dir & m_dir_override & 1 << offset)
				return m_output_latch[offset];

			// otherwise, return an input port
			return m_in_port_cb[offset](offset);

		// 'SEGA' protection
		case 0x8:
			return 'S';
		case 0x9:
			return 'E';
		case 0xa:
			return 'G';
		case 0xb:
			return 'A';

		// CNT register & mirror
		case 0xc: case 0xe:
			return m_cnt;

		// port direction register & mirror
		case 0xd: case 0xf:
			return m_dir;

		default:
			break;
	}

	return 0xff;
}


void sega_315_5296_device::write(offs_t offset, uint8_t data)
{
	offset &= 0x3f;

	switch (offset)
	{
		// port A to H
		case 0x0: case 0x1: case 0x2: case 0x3: case 0x4: case 0x5: case 0x6: case 0x7:
			// if the port is configured as an output, write it
			if (m_dir & 1 << offset)
				m_out_port_cb[offset](offset, data);

			m_output_latch[offset] = data;
			break;

		// CNT register
		case 0xe:
			// d0-2: CNT0-2 output pins
			// note: When CNT2 is configured as clock output, bit 2 of this register has
			// no effect on the output level of CNT2.
			for (int i = 0; i < 3; i++)
				m_out_cnt_cb[i](data >> i & 1);

			// d3: CNT2 output mode (1= Clock output, 0= Programmable output)
			// d4,5: CNT2 clock divider (0= CLK/4, 1= CLK/8, 2= CLK/16, 3= CLK/2)
			// d6,7: CKOT clock divider (0= CLK/4, 1= CLK/8, 2= CLK/16, 3= CLK/2)
			// TODO..
			m_cnt = data;
			break;

		// port direction register
		case 0xf:
			// refresh output ports if the direction changed
			for (int i = 0; i < 8; i++)
			{
				if ((m_dir ^ data) & (1 << i))
				{
					logerror("Port %c configured for output\n", 'A' + i);
					m_out_port_cb[i]((offs_t)i, (data & 1 << i) ? m_output_latch[i] : 0);
				}
			}

			m_dir = data;
			break;

		default:
			break;
	}
}
