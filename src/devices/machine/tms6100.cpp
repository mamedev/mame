// license:BSD-3-Clause
// copyright-holders:hap, Couriersud
/**********************************************************************************************

    Texas Instruments TMS6100 Voice Synthesis Memory (VSM)
    
    References:
    - TMS 6100 Voice Synthesis Memory Data Manual
    - TMS 6125 Voice Synthesis Memory Data Manual
    - Speak & Spell patent US4189779 for low-level documentation
    - 1982 Mitsubishi Data Book (M58819S section)

    TODO:
    - implement clock pin(CLK) properly, xtal/timer
    - command processing timing is not accurate, on the real chip it will take a few microseconds
    - current implementation does not regard multi-chip configuration and pretends it is 1 chip,
      this will work fine under normal circumstances since CS would be disabled on invalid address
    - implement chip addressing (0-15 mask programmed, see above)
    - M58819S pins SL(PROM expansion input), POC(reset)

***********************************************************************************************/

#include "tms6100.h"


// device definitions

const device_type TMS6100 = &device_creator<tms6100_device>;

tms6100_device::tms6100_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	m_rom(*this, DEVICE_SELF),
	m_reverse_bits(false),
	m_4bit_mode(false)
{
}

tms6100_device::tms6100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TMS6100, "TMS6100", tag, owner, clock, "tms6100", __FILE__),
	m_rom(*this, DEVICE_SELF),
	m_reverse_bits(false),
	m_4bit_mode(false)
{
}

const device_type M58819 = &device_creator<m58819_device>;

m58819_device::m58819_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms6100_device(mconfig, M58819, "M58819S", tag, owner, clock, "m58819s", __FILE__)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms6100_device::device_start()
{
	m_rommask = m_rom.bytes() - 1;

	// zerofill
	m_address = 0;
	m_sa = 0;
	m_count = 0;
	m_prev_cmd = 0;
	m_prev_m = 0;

	m_add = 0;
	m_data = 0;
	m_m0 = 0;
	m_m1 = 0;
	m_cs = 1;
	m_clk = 0;
	m_rck = 0;

	// register for savestates
	save_item(NAME(m_address));
	save_item(NAME(m_sa));
	save_item(NAME(m_count));
	save_item(NAME(m_prev_cmd));
	save_item(NAME(m_prev_m));
	save_item(NAME(m_add));
	save_item(NAME(m_data));
	save_item(NAME(m_m0));
	save_item(NAME(m_m1));
	save_item(NAME(m_cs));
	save_item(NAME(m_clk));
	save_item(NAME(m_rck));
}

void m58819_device::device_start()
{
	tms6100_device::device_start();
	m_reverse_bits = true; // m58819 'vsm-emulator' chip expects ROM bit order backwards
}


// external i/o

WRITE_LINE_MEMBER(tms6100_device::m0_w)
{
	m_m0 = (state) ? 1 : 0;
}

WRITE_LINE_MEMBER(tms6100_device::m1_w)
{
	m_m1 = (state) ? 1 : 0;
}

WRITE_LINE_MEMBER(tms6100_device::cs_w)
{
	// chip select pin
	m_cs = (state) ? 1 : 0;
}

WRITE_LINE_MEMBER(tms6100_device::rck_w)
{
	// gate/mask for clk
	m_rck = (state) ? 1 : 0;
}

WRITE8_MEMBER(tms6100_device::add_w)
{
	m_add = data & 0xf;
}

READ8_MEMBER(tms6100_device::data_r)
{
	return m_data & 0xf;
}

READ_LINE_MEMBER(tms6100_device::data_line_r)
{
	// DATA/ADD8
	return (m_data & 8) ? 1 : 0;
}

WRITE_LINE_MEMBER(tms6100_device::clk_w)
{
	// process on falling edge
	if (m_clk && !m_rck && !state)
	{
		if (m_cs)
		{
			// new command enabled on rising edge of m0/m1
			UINT8 m = m_m1 << 1 | m_m0;
			if ((m & ~m_prev_m & 1) || (m & ~m_prev_m & 2))
				handle_command(m);
			
			m_prev_m = m;
		}
	}
	
	m_clk = (state) ? 1 : 0;
}


// m0/m1 commands

void tms6100_device::handle_command(UINT8 cmd)
{
	enum
	{
		M_NOP = 0, M_TB, M_LA, M_RB
	};

	switch (cmd)
	{
		// TB: transfer bit (read)
		case M_TB:
			if (m_prev_cmd == M_LA)
			{
				// dummy read after LA
				m_count = 0;
			}
			else
			{
				// load new data from rom
				if (m_count == 0)
				{
					m_sa = m_rom[m_address & m_rommask];

					// M58819S reads serial data reversed
					if (m_reverse_bits)
						m_sa = BITSWAP8(m_sa,0,1,2,3,4,5,6,7);
				}
				else
				{
					// or shift(rotate) right
					m_sa = (m_sa >> 1) | (m_sa << 7 & 0x80);
				}
				
				// output to DATA pin(s)
				if (!m_4bit_mode)
				{
					// 1-bit mode: SA0 to ADD8/DATA
					m_data = m_sa << 3 & 8;
				}
				else
				{
					// 4-bit mode: SA0-3 or SA3-6(!) to DATA
					if (m_count & 1)
						m_data = m_sa >> 3 & 0xf;
					else
						m_data = m_sa & 0xf;
				}
				
				// 8 bits in 1-bit mode, otherwise 2 nybbles
				m_count = (m_count + 1) & (m_4bit_mode ? 1 : 7);
				
				// TB8
				if (m_count == 0)
					m_address++; // CS bits too
			}
			break;
		
		// LA: load address
		case M_LA:
			if (m_prev_cmd == M_TB)
			{
				// start LA after TB
				m_address = (m_address & ~0xf) | m_add;
				m_count = 0;
			}
			else if (m_prev_cmd == M_LA)
			{
				// load consecutive address bits (including CS bits)
				// the 8-step counter PLA is shared between LA and TB
				if (m_count < 4)
				{
					const UINT8 shift = 4 * (m_count+1);
					m_address = (m_address & ~(0xf << shift)) | (m_add << shift);
				}
				
				m_count = (m_count + 1) & 7;
			}
			break;

		// RB: read and branch
		case M_RB:
			// process RB after LA or TB8
			if (m_prev_cmd == M_LA || (m_prev_cmd == M_TB && m_count == 0))
			{
				m_count = 0;

				// load new address bits (14 bits on TMS6100)
				UINT16 rb = m_rom[m_address & m_rommask];
				m_address++;
				rb |= (m_rom[m_address & m_rommask] << 8);
				m_address = (m_address & ~0x3fff) | (rb & 0x3fff);
			}
			break;
		
		default:
			break;
	}
	
	m_prev_cmd = cmd;
}
