// license:BSD-3-Clause
// copyright-holders:James Wallace
/***************************************************************************

    Bellfruit dotmatrix driver, (under heavy construction !!!)

    19-08-2005: Re-Animator
    25-08-2005: Fixed feedback through sc2 input line

    A.G.E Code Copyright J. Wallace and the AGEMAME Development Team.
    Visit http://www.mameworld.net/agemame/ for more information.


Standard dm01 memorymap

    hex    |r/w| D D D D D D D D |
 location  |   | 7 6 5 4 3 2 1 0 | function
-----------+---+-----------------+-----------------------------------------
0000-1FFF  |R/W| D D D D D D D D | RAM (8k)
-----------+---+-----------------+-----------------------------------------
2000       |R/W| D D D D D D D D | control register
-----------+---+-----------------+-----------------------------------------
2800       |R/W| D D D D D D D D | mux
-----------+---+-----------------+-----------------------------------------
3000       |R/W| D D D D D D D D | communication with mainboard
-----------+---+-----------------+-----------------------------------------
3800       |R/W| D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
4000-FFFF  | W | D D D D D D D D | ROM (48k)
-----------+---+-----------------+-----------------------------------------

  TODO: - find out clockspeed of CPU
        - sometimes screen isn't cleared, is the a clear bit missing?

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "bfm_dm01.h"

// local vars /////////////////////////////////////////////////////////////

#define DM_MAXLINES    21


#ifdef MAME_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

const device_type BF_DM01 = &device_creator<bfmdm01_device>;

bfmdm01_device::bfmdm01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, BF_DM01, "BFM Dotmatrix 01", tag, owner, clock, "bfm_dm01", __FILE__),
	m_data_avail(0),
	m_control(0),
	m_xcounter(0),
	m_busy(0),
	m_comdata(0),
	m_busy_cb(*this)
{
	for (auto & elem : m_segbuffer)
	elem = 0;

	for (auto & elem : m_scanline)
	elem = 0;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bfmdm01_device::device_start()
{
	m_busy_cb.resolve_safe();

	save_item(NAME(m_data_avail));
	save_item(NAME(m_control));
	save_item(NAME(m_xcounter));
	save_item(NAME(m_busy));
	save_item(NAME(m_comdata));

	for (int i = 0; i < 65; i++)
	save_item(NAME(m_segbuffer), i);

	for (int i = 0; i < DM_BYTESPERROW; i++)
	save_item(NAME(m_scanline), i);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bfmdm01_device::device_reset()
{
	m_busy     = 0;
	m_control  = 0;
	m_xcounter = 0;
	m_data_avail = 0;

	m_busy_cb(m_busy);
}

///////////////////////////////////////////////////////////////////////////

int bfmdm01_device::read_data(void)
{
	int data = m_comdata;

	m_data_avail = 0;

	return data;
}

///////////////////////////////////////////////////////////////////////////

READ8_MEMBER( bfmdm01_device::control_r )
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER( bfmdm01_device::control_w )
{
	int changed = m_control ^ data;

	m_control = data;

	if ( changed & 2 )
	{   // reset horizontal counter
		if ( !(data & 2) )
		{
			//int offset = 0;

			m_xcounter = 0;
		}
	}

	if ( changed & 8 )
	{ // bit 3 changed = BUSY line
		if ( data & 8 )   m_busy = 0;
		else              m_busy = 1;

		m_busy_cb(m_busy);
	}
}

///////////////////////////////////////////////////////////////////////////

READ8_MEMBER( bfmdm01_device::mux_r )
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER( bfmdm01_device::mux_w )
{
	if ( m_xcounter < DM_BYTESPERROW )
	{
		m_scanline[m_xcounter] = data;
		m_xcounter++;
	}
	if ( m_xcounter == 9 )
	{
		int row = ((0xFF^data) & 0x7C) >> 2;    // 7C = 000001111100
		m_scanline[8] &= 0x80;//filter all other bits
		if ( (row >= 0)  && (row < DM_MAXLINES) )
		{
			int p = 0;

			while ( p < (DM_BYTESPERROW) )
			{
				UINT8 d = m_scanline[p];

				for (int bitpos=0; bitpos <8; bitpos++)
				{
					if (((p*8)+bitpos) <65)
					{
						if (d & 1<<(7-bitpos)) m_segbuffer[(p*8)+bitpos]=1;
						else m_segbuffer[(p*8)+bitpos]=0;
					}
				}
				p++;
			}

			for (int pos=0;pos<65;pos++)
			{
				output_set_indexed_value("dotmatrix", pos +(65*row), m_segbuffer[(pos)]);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////

READ8_MEMBER( bfmdm01_device::comm_r )
{
	int result = 0;

	if ( m_data_avail )
	{
		result = read_data();

		#ifdef UNUSED_FUNCTION
		if ( m_data_avail() )
		{
			cpu_set_irq_line(1, M6809_IRQ_LINE, ASSERT_LINE );  // trigger IRQ
		}
		#endif
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER( bfmdm01_device::comm_w )
{
}
///////////////////////////////////////////////////////////////////////////

READ8_MEMBER( bfmdm01_device::unknown_r )
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER( bfmdm01_device::unknown_w )
{
	space.machine().device("matrix")->execute().set_input_line(INPUT_LINE_NMI, CLEAR_LINE ); //?
}

///////////////////////////////////////////////////////////////////////////

ADDRESS_MAP_START( bfm_dm01_memmap, AS_PROGRAM, 8, bfmdm01_device )
	AM_RANGE(0x0000, 0x1fff) AM_RAM                             // 8k RAM
	AM_RANGE(0x2000, 0x2000) AM_DEVREADWRITE("dm01", bfmdm01_device, control_r, control_w)  // control reg
	AM_RANGE(0x2800, 0x2800) AM_DEVREADWRITE("dm01", bfmdm01_device, mux_r, mux_w)           // mux
	AM_RANGE(0x3000, 0x3000) AM_DEVREADWRITE("dm01", bfmdm01_device, comm_r, comm_w)     //
	AM_RANGE(0x3800, 0x3800) AM_DEVREADWRITE("dm01", bfmdm01_device, unknown_r, unknown_w)   // ???
	AM_RANGE(0x4000, 0xFfff) AM_ROM                             // 48k  ROM
ADDRESS_MAP_END

///////////////////////////////////////////////////////////////////////////

void bfmdm01_device::writedata(UINT8 data)
{
	m_comdata = data;
	m_data_avail = 1;

	//pulse IRQ line
	machine().device("matrix")->execute().set_input_line(M6809_IRQ_LINE, HOLD_LINE ); // trigger IRQ
}

///////////////////////////////////////////////////////////////////////////
int bfmdm01_device::busy(void)
{
	return m_data_avail;
}
