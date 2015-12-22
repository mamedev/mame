// license:BSD-3-Clause
// copyright-holders:Couriersud
/**********************************************************************************************

     TMS6100 simulator

     Written for MAME by Couriersud

     Todo:
        - implement CS
        - implement chip addressing (0-15 mask programmed)

     TMS6100:

                 +-----------------+
       VDD       |  1           28 |  NC
       NC        |  2           27 |  NC
       DATA/ADD1 |  3           26 |  NC
       DATA/ADD2 |  4           25 |  NC
       DATA/ADD4 |  5           24 |  NC
       DATA/ADD8 |  6           23 |  NC
       CLK       |  7           22 |  NC
       NC        |  8           21 |  NC
       NC        |  9           20 |  NC
       M0        | 10           19 |  NC
       M1        | 11           18 |  NC
       NC        | 12           17 |  NC
       /CS       | 13           16 |  NC
       VSS       | 14           15 |  NC
                 +-----------------+

     TMS6125:

                 +---------+
       DATA/ADD1 | 1    16 |  NC
       DATA/ADD2 | 2    15 |  NC
       DATA/ADD4 | 3    14 |  NC
       DATA/ADD8 | 4    13 |  NC
       CLK       | 5    12 |  VDD
       NC        | 6    11 |  /CS
       NC        | 7    10 |  M1
       M0        | 8     9 |  VSS
                 +---------+

    M58819 (from radarscope schematics):

                 +-----------------+
       AD0       |  1           40 |  AD1
       GND       |  2           39 |  AD2
       -5V       |  3           38 |  AD3
       A0        |  4           37 |  AD4
       NC        |  5           36 |  AD5
       NC        |  6           35 |  AD6
       A1        |  7           34 |  AD7
       A2        |  8           33 |  AD8
       A3        |  9           32 |  AD9
       CLK       | 10           31 |  AD10
       NC        | 11           30 |  AD11
       -5V       | 12           29 |  AD12
       C0        | 13           28 |  NC
       C1        | 14           27 |  NC
       NC        | 15           26 |  I7
       NC        | 16           25 |  NC
       +5V       | 17           24 |  I6
       I0        | 18           23 |  I5
       I1        | 19           22 |  I4
       I2        | 20           21 |  I3
                 +-----------------+

    The M58819 is used as an interface to external speech eproms.
    NC pins may have a function, although they are not connected in
    radarscope.

***********************************************************************************************/

#include "tms6100.h"

#define VERBOSE     (0)

#if VERBOSE
#define LOG(x)      logerror x
#else
#define LOG(x)
#endif

#define TMS6100_READ_PENDING        0x01
#define TMS6100_NEXT_READ_IS_DUMMY  0x02

const device_type TMS6100 = &device_creator<tms6100_device>;

tms6100_device::tms6100_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	m_rom(*this, DEVICE_SELF),
	m_reverse_bits(false),
	m_4bit_read(false)
{
}

tms6100_device::tms6100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TMS6100, "TMS6100", tag, owner, clock, "tms6100", __FILE__),
	m_rom(*this, DEVICE_SELF),
	m_reverse_bits(false),
	m_4bit_read(false)
{
}

const device_type M58819 = &device_creator<m58819_device>;

m58819_device::m58819_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms6100_device(mconfig, M58819, "M58819 Memory Controller", tag, owner, clock, "m58819", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms6100_device::device_start()
{
	// zerofill
	m_addr_bits = 0;
	m_address = 0;
	m_address_latch = 0;
	m_loadptr = 0;
	m_m0 = 0;
	m_m1 = 0;
	m_state = 0;
	m_data = 0;
	m_tms_clock = 0;

	// save device variables
	save_item(NAME(m_addr_bits));
	save_item(NAME(m_address));
	save_item(NAME(m_address_latch));
	save_item(NAME(m_loadptr));
	save_item(NAME(m_m0));
	save_item(NAME(m_m1));
	save_item(NAME(m_state));
	save_item(NAME(m_data));
	save_item(NAME(m_tms_clock));
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

WRITE8_MEMBER(tms6100_device::addr_w)
{
	m_addr_bits = data & 0xf;
}

READ8_MEMBER(tms6100_device::data_r)
{
	return m_data;
}

READ_LINE_MEMBER(tms6100_device::data_line_r)
{
	return m_data;
}


// CLK line

WRITE_LINE_MEMBER(tms6100_device::romclock_w)
{
	// process on falling edge
	if (m_tms_clock && !state)
	{
		switch (m_m1 << 1 | m_m0)
		{
		case 0x00:
			// NOP in datasheet, not really ...
			if (m_state & TMS6100_READ_PENDING)
			{
				if (m_state & TMS6100_NEXT_READ_IS_DUMMY)
				{
					m_address = (m_address_latch << 3);
					m_address_latch = 0;
					m_loadptr = 0;
					m_state &= ~TMS6100_NEXT_READ_IS_DUMMY;
					LOG(("loaded address %08x\n", m_address));
				}
				else
				{
					// read bit(s) at address
					UINT8 word = m_rom[m_address >> 3];
					if (m_reverse_bits)
						word = BITSWAP8(word,0,1,2,3,4,5,6,7);
					
					if (m_4bit_read)
					{
						m_data = word >> (m_address & 7) & 0xf;
						m_address += 4;
					}
					else
					{
						m_data = word >> (m_address & 7) & 1;
						m_address++;
					}
				}
				m_state &= ~TMS6100_READ_PENDING;
			}
			break;

		case 0x01:
			// READ
			m_state |= TMS6100_READ_PENDING;
			break;

		case 0x02:
			// LOAD ADDRESS
			m_state |= TMS6100_NEXT_READ_IS_DUMMY;
			m_address_latch |= (m_addr_bits << m_loadptr);
			LOG(("loaded address latch %08x\n", m_address_latch));
			m_loadptr += 4;
			break;

		case 0x03:
			// READ AND BRANCH
			if (m_state & TMS6100_NEXT_READ_IS_DUMMY)
			{
				m_state &= ~TMS6100_NEXT_READ_IS_DUMMY; // clear - no dummy read according to datasheet
				LOG(("loaded address latch %08x\n", m_address_latch));
				m_address = m_rom[m_address_latch] | (m_rom[m_address_latch+1] << 8);
				m_address &= 0x3fff; // 14 bits
				LOG(("loaded indirect address %04x\n", m_address));
				m_address = (m_address << 3);
				m_address_latch = 0;
				m_loadptr = 0;
			}
			break;
		}
	}
	m_tms_clock = state;
}
