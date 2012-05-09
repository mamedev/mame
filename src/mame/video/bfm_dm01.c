/***************************************************************************

    Bellfruit dotmatrix driver, (under heavy construction !!!)

    19-08-2005: Re-Animator
    25-08-2005: Fixed feedback through sc2 input line

    A.G.E Code Copyright J. Wallace and the AGEMAME Development Team.
    Visit http://www.mameworld.net/agemame/ for more information.

    M.A.M.E Core Copyright Nicola Salmoria and the MAME Team,
    used under license from http://mamedev.org

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

  NOTE: The board uses only one dot of its last set of eight in a row, as the
        rest are used for the row counter. Because of the way we do dots, we show
		the blank ones that are most likely hidden by bezels on the unit.
  TODO: - find out clockspeed of CPU
        - sometimes screen isn't cleared, is the a clear bit missing?

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "bfm_dm01.h"

// local vars /////////////////////////////////////////////////////////////

#define DM_BYTESPERROW 9
#define DM_MAXLINES    21


#ifdef MAME_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

typedef struct _dm01
{
	const bfmdm01_interface *intf;
	int		 data_avail,
		        control,	/* motor phase */
			  xcounter,
				  busy;

UINT8 scanline[DM_BYTESPERROW],
		comdata;

} bfmdm01;

static bfmdm01 dm01;
///////////////////////////////////////////////////////////////////////////

void BFM_dm01_config(running_machine &machine, const bfmdm01_interface *intf)
{
	assert_always(machine.phase() == MACHINE_PHASE_INIT, "Can only call BFM_dm01_config at init time!");
	assert_always(intf, "BFM_dm01_config called with an invalid interface!");
	dm01.intf = intf;
	BFM_dm01_reset(machine);

}

///////////////////////////////////////////////////////////////////////////

static int read_data(void)
{
	int data = dm01.comdata;

	dm01.data_avail = 0;

	return data;
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( control_r )
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( control_w )
{
	int changed = dm01.control ^ data;

	dm01.control = data;

	if ( changed & 2 )
	{	// reset horizontal counter
		if ( !(data & 2) )
		{
			//int offset = 0;

			dm01.xcounter = 0;
		}
	}

	if ( changed & 8 )
	{ // bit 3 changed = BUSY line
		if ( data & 8 )	  dm01.busy = 0;
		else			  dm01.busy = 1;

		dm01.intf->busy_func(space->machine(),dm01.busy);
	}
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( mux_r )
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( mux_w )
{

	if ( dm01.xcounter < DM_BYTESPERROW )
	{
		dm01.scanline[dm01.xcounter] = data;
		dm01.xcounter++;
	}
	if ( dm01.xcounter == 9 )
	{
    	int row = ((0xFF^data) & 0x7C) >> 2;	// 7C = 000001111100
		dm01.scanline[8] &= 0x80;//filter all other bits
		if ( (row >= 0)  && (row < DM_MAXLINES) )
		{
			int p,dots;

			p = 0;
			dots = 0;

			while ( p < (DM_BYTESPERROW) )
			{

				UINT8 d = dm01.scanline[p];
				if (d & 0x80) dots |= 0x01;
				else          dots &=~0x01;
				if (d & 0x40) dots |= 0x02;
				else          dots &=~0x02;
				if (d & 0x20) dots |= 0x04;
				else          dots &=~0x04;
				if (d & 0x10) dots |= 0x08;
				else          dots &=~0x08;
				if (d & 0x08) dots |= 0x10;
				else          dots &=~0x10;
				if (d & 0x04) dots |= 0x20;
				else          dots &=~0x20;
				if (d & 0x02) dots |= 0x40;
				else          dots &=~0x40;
				if (d & 0x01) dots |= 0x80;
				else          dots &=~0x80;
				output_set_indexed_value("dotmatrix", p +(9*row), dots);
				p++;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( comm_r )
{
	int result = 0;

	if ( dm01.data_avail )
	{
		result = read_data();

		#ifdef UNUSED_FUNCTION
		if ( dm01.data_avail() )
		{
			cpu_set_irq_line(1, M6809_IRQ_LINE, ASSERT_LINE );	// trigger IRQ
		}
		#endif
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( comm_w )
{
}
///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( unknown_r )
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( unknown_w )
{
	cputag_set_input_line(space->machine(), "matrix", INPUT_LINE_NMI, CLEAR_LINE ); //?
}

///////////////////////////////////////////////////////////////////////////

ADDRESS_MAP_START( bfm_dm01_memmap, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0000, 0x1fff) AM_RAM								// 8k RAM
	AM_RANGE(0x2000, 0x2000) AM_READWRITE_LEGACY(control_r, control_w)	// control reg
	AM_RANGE(0x2800, 0x2800) AM_READWRITE_LEGACY(mux_r,mux_w)			// mux
	AM_RANGE(0x3000, 0x3000) AM_READWRITE_LEGACY(comm_r,comm_w)		//
	AM_RANGE(0x3800, 0x3800) AM_READWRITE_LEGACY(unknown_r,unknown_w)	// ???
	AM_RANGE(0x4000, 0xFfff) AM_ROM								// 48k  ROM
ADDRESS_MAP_END

///////////////////////////////////////////////////////////////////////////

void BFM_dm01_writedata(running_machine &machine, UINT8 data)
{
	dm01.comdata = data;
	dm01.data_avail = 1;

  //pulse IRQ line
	cputag_set_input_line(machine, "matrix", M6809_IRQ_LINE, HOLD_LINE ); // trigger IRQ
}

///////////////////////////////////////////////////////////////////////////

INTERRUPT_GEN( bfm_dm01_vbl )
{
	device_set_input_line(device, INPUT_LINE_NMI, ASSERT_LINE );
}

///////////////////////////////////////////////////////////////////////////
int BFM_dm01_busy(void)
{
	return dm01.data_avail;
}

void BFM_dm01_reset(running_machine &machine)
{
	dm01.busy     = 0;
	dm01.control  = 0;
	dm01.xcounter = 0;
	dm01.data_avail = 0;

	dm01.intf->busy_func(machine,dm01.busy);
}
