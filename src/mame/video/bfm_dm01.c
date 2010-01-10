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

  TODO: - find out clockspeed of CPU
        - make a non-hacky Artwork representation of dot matrix
  Layout notes: the dot matrix is set to screen 0.
  Multiple matrices and matrix/VFD combos are not yet supported.

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "video/awpvid.h"
#include "bfm_dm01.h"
// local prototypes ///////////////////////////////////////////////////////

extern void Scorpion2_SetSwitchState(int strobe, int data, int state);
extern int  Scorpion2_GetSwitchState(int strobe, int data);

// local vars /////////////////////////////////////////////////////////////

#define DM_BYTESPERROW 9
#define DM_MAXLINES    21

#define FEEDBACK_STROBE 4
#define FEEDBACK_DATA   4

static int   control;
static int   xcounter;
static int   busy;
static int   data_avail;

static UINT8 scanline[DM_BYTESPERROW];

static UINT8 comdata;

//static int   read_index;
//static int   write_index;

static bitmap_t *dm_bitmap;

#ifdef MAME_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

///////////////////////////////////////////////////////////////////////////

static int read_data(void)
{
	int data = comdata;

	data_avail = 0;

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
	int changed = control ^ data;

	control = data;

	if ( changed & 2 )
	{	// reset horizontal counter
		if ( !(data & 2) )
		{
			//int offset = 0;

			xcounter = 0;
		}
	}

	if ( changed & 8 )
	{ // bit 3 changed = BUSY line
		if ( data & 8 )	  busy = 0;
		else			  busy = 1;

		Scorpion2_SetSwitchState(FEEDBACK_STROBE,FEEDBACK_DATA, busy?0:1);
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
	int x = xcounter;

	if ( x < DM_BYTESPERROW )
	{
		scanline[xcounter++] = data;
	}

	if ( x == 8 )
	{
    	int off = ((0xFF^data) & 0x7C) >> 2;	// 7C = 000001111100

		scanline[x] &= 0x80;
		if ( (off >= 0)  && (off < DM_MAXLINES) )
		{
			int p;

			x = 0;
			p = 0;

			while ( p < DM_BYTESPERROW )
			{
				UINT8 d = scanline[p++];
				LOG(("%d",d));
				*BITMAP_ADDR16(dm_bitmap, off, x++) = (d & 0x80) ? 0 : 1;
				*BITMAP_ADDR16(dm_bitmap, off, x++) = (d & 0x40) ? 0 : 1;
				*BITMAP_ADDR16(dm_bitmap, off, x++) = (d & 0x20) ? 0 : 1;
				*BITMAP_ADDR16(dm_bitmap, off, x++) = (d & 0x10) ? 0 : 1;
				*BITMAP_ADDR16(dm_bitmap, off, x++) = (d & 0x08) ? 0 : 1;
				*BITMAP_ADDR16(dm_bitmap, off, x++) = (d & 0x04) ? 0 : 1;
				*BITMAP_ADDR16(dm_bitmap, off, x++) = (d & 0x02) ? 0 : 1;
				*BITMAP_ADDR16(dm_bitmap, off, x++) = (d & 0x01) ? 0 : 1;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( comm_r )
{
	int result = 0;

	if ( data_avail )
	{
		result = read_data();

		#ifdef UNUSED_FUNCTION
		if ( data_avail() )
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
	cputag_set_input_line(space->machine, "matrix", INPUT_LINE_NMI, CLEAR_LINE ); //?
}

///////////////////////////////////////////////////////////////////////////

ADDRESS_MAP_START( bfm_dm01_memmap, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM								// 8k RAM
	AM_RANGE(0x2000, 0x2000) AM_READWRITE(control_r, control_w)	// control reg
	AM_RANGE(0x2800, 0x2800) AM_READWRITE(mux_r,mux_w)			// mux
	AM_RANGE(0x3000, 0x3000) AM_READWRITE(comm_r,comm_w)		//
	AM_RANGE(0x3800, 0x3800) AM_READWRITE(unknown_r,unknown_w)	// ???
	AM_RANGE(0x4000, 0xFfff) AM_ROM								// 48k  ROM
ADDRESS_MAP_END

///////////////////////////////////////////////////////////////////////////

void BFM_dm01_writedata(running_machine *machine, UINT8 data)
{
	comdata = data;
	data_avail = 1;

  //pulse IRQ line
	cputag_set_input_line(machine, "matrix", M6809_IRQ_LINE, HOLD_LINE ); // trigger IRQ
}

///////////////////////////////////////////////////////////////////////////

INTERRUPT_GEN( bfm_dm01_vbl )
{
	cpu_set_input_line(device, INPUT_LINE_NMI, ASSERT_LINE );
}

///////////////////////////////////////////////////////////////////////////

VIDEO_START( bfm_dm01 )
{
	dm_bitmap = auto_bitmap_alloc(machine, DM_BYTESPERROW*8, DM_MAXLINES,video_screen_get_format(machine->primary_screen));
}

///////////////////////////////////////////////////////////////////////////

// Note that we assume square pixels, which isn't strictly true (the elements are round).

VIDEO_UPDATE( bfm_dm01 )
{
	const rectangle *visarea = video_screen_get_visible_area(screen);

	copybitmap(bitmap, dm_bitmap, 0,0,0,0, visarea);

	LOG(("Busy=%X",data_avail));
	LOG(("%X",Scorpion2_GetSwitchState(FEEDBACK_STROBE,FEEDBACK_DATA)));

	return 0;
}

int BFM_dm01_busy(void)
{
	return data_avail;
}

void BFM_dm01_reset(void)
{
	busy     = 0;
	control  = 0;
	xcounter = 0;
	data_avail = 0;

	Scorpion2_SetSwitchState(FEEDBACK_STROBE,FEEDBACK_DATA, busy?0:1);
}

PALETTE_INIT( bfm_dm01 )
{
	palette_set_color(machine, 0, MAKE_RGB(0xFF,0x00,0x00));
	palette_set_color(machine, 1, MAKE_RGB(0x40,0x00,0x00));
}


