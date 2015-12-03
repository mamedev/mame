// license:GPL-2.0+
// copyright-holders:Kevin Thacker, Barry Rodewald
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

Amstrad hardware consists of:

- General Instruments AY-3-8912 (audio and keyboard scanning)
- Intel 8255PPI (keyboard, access to AY-3-8912, cassette etc)
- Z80A CPU
- 765 FDC (disc drive interface)
- "Gate Array" (custom chip by Amstrad controlling colour, mode,
rom/ram selection


On the Amstrad, any part of the 64k memory can be access by the video
hardware (GA and CRTC - the CRTC specifies the memory address to access,
and the GA fetches 2 bytes of data for each 1us cycle.

The Z80 must also access the same ram.

To maintain the screen display, the Z80 is halted on each memory access.

The result is that timing for opcodes, appears to fall into a nice pattern,
where the time for each opcode can be measured in NOP cycles. NOP cycles is
the name I give to the time taken for one NOP command to execute.

This happens to be 1us.

From measurement, there are 64 NOPs per line, with 312 lines per screen.
This gives a total of 19968 NOPs per frame.

***************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/mc146818.h"
#include "machine/upd765.h"
#include "bus/centronics/ctronics.h"
#include "bus/cpc/cpc_rom.h"
#include "bus/cpc/mface2.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "includes/amstrad.h"
#include "sound/ay8910.h"
#include "machine/ram.h"

#define MANUFACTURER_NAME 0x07
#define TV_REFRESH_RATE 0x10

enum {
	SYSTEM_CPC,
	SYSTEM_ALESTE,
	SYSTEM_PLUS,
	SYSTEM_GX4000
};


/* originally from drivers/amstrad.c */
// &ff,&77,&b3,&51,&a8,&d4,&62,&39,&9c,&46,&2b,&15,&8a,&cd,&ee
// This is the sequence for unlocking the ASIC in the CPC+/GX4000
// These are outed to port &bc00, after syncing the lock by outing a non-zero value then a zero to &bc00
// To lock the ASIC again, repeat the sequence without the last &ee
static const UINT8 asic_unlock_seq[15] =
{
	0xff, 0x77, 0xb3, 0x51, 0xa8, 0xd4, 0x62, 0x39, 0x9c, 0x46, 0x2b, 0x15, 0x8a, 0xcd, 0xee
};





/* Ram configuration */

/* pointers to current ram configuration selected for banks */

/* the hardware allows selection of 256 ROMs. Rom 0 is usually BASIC and Rom 7 is AMSDOS */
/* With the CPC hardware, if a expansion ROM is not connected, BASIC rom will be selected instead */
/* data present on input of ppi, and data written to ppi output */
#define amstrad_ppi_PortA 0
#define amstrad_ppi_PortB 1
#define amstrad_ppi_PortC 2



/*------------------
  - Ram Management -
  ------------------*/
/* There are 8 different ram configurations which work on the currently selected 64k logical block.
   The following tables show the possible ram configurations :*/
static const int RamConfigurations[8 * 4] =
{
	0, 1, 2, 3,                        /* config 0 */
	0, 1, 2, 7,                        /* config 1 */
	4, 5, 6, 7,                        /* config 2 */
	0, 3, 2, 7,                        /* config 3 */
	0, 4, 2, 3,                        /* config 4 */
	0, 5, 2, 3,                        /* config 5 */
	0, 6, 2, 3,                        /* config 6 */
	0, 7, 2, 3                       /* config 7 */
};



//static int amstrad_CRTC_CR = 0;        /* CR = Cursor Enabled */

/* this contains the colours in machine.pens form.*/
/* this is updated from the eventlist and reflects the current state
of the render colours - these may be different to the current colour palette values */
/* colours can be changed at any time and will take effect immediatly */



/* The Amstrad CPC has a fixed palette of 27 colours generated from 3 levels of Red, Green and Blue.
The hardware allows selection of 32 colours, but these extra colours are copies of existing colours.*/

static const rgb_t amstrad_palette[32] =
{
	rgb_t(0x060, 0x060, 0x060),             /* white */
	rgb_t(0x060, 0x060, 0x060),             /* white */
	rgb_t(0x000, 0x0ff, 0x060),             /* sea green */
	rgb_t(0x0ff, 0x0ff, 0x060),             /* pastel yellow */
	rgb_t(0x000, 0x000, 0x060),             /* blue */
	rgb_t(0x0ff, 0x000, 0x060),             /* purple */
	rgb_t(0x000, 0x060, 0x060),             /* cyan */
	rgb_t(0x0ff, 0x060, 0x060),             /* pink */
	rgb_t(0x0ff, 0x000, 0x060),             /* purple */
	rgb_t(0x0ff, 0x0ff, 0x060),             /* pastel yellow */
	rgb_t(0x0ff, 0x0ff, 0x000),             /* bright yellow */
	rgb_t(0x0ff, 0x0ff, 0x0ff),             /* bright white */
	rgb_t(0x0ff, 0x000, 0x000),             /* bright red */
	rgb_t(0x0ff, 0x000, 0x0ff),             /* bright magenta */
	rgb_t(0x0ff, 0x060, 0x000),             /* orange */
	rgb_t(0x0ff, 0x060, 0x0ff),             /* pastel magenta */
	rgb_t(0x000, 0x000, 0x060),             /* blue */
	rgb_t(0x000, 0x0ff, 0x060),             /* sea green */
	rgb_t(0x000, 0x0ff, 0x000),             /* bright green */
	rgb_t(0x000, 0x0ff, 0x0ff),             /* bright cyan */
	rgb_t(0x000, 0x000, 0x000),             /* black */
	rgb_t(0x000, 0x000, 0x0ff),             /* bright blue */
	rgb_t(0x000, 0x060, 0x000),             /* green */
	rgb_t(0x000, 0x060, 0x0ff),             /* sky blue */
	rgb_t(0x060, 0x000, 0x060),             /* magenta */
	rgb_t(0x060, 0x0ff, 0x060),             /* pastel green */
	rgb_t(0x060, 0x0ff, 0x060),             /* lime */
	rgb_t(0x060, 0x0ff, 0x0ff),             /* pastel cyan */
	rgb_t(0x060, 0x000, 0x000),             /* Red */
	rgb_t(0x060, 0x000, 0x0ff),             /* mauve */
	rgb_t(0x060, 0x060, 0x000),             /* yellow */
	rgb_t(0x060, 0x060, 0x0ff)              /* pastel blue */
};


/* the green brightness is equal to the firmware colour index */
static const rgb_t amstrad_green_palette[32] =
{
	rgb_t(0x000, 0x07F, 0x000),        /*13*/
	rgb_t(0x000, 0x07F, 0x000),        /*13*/
	rgb_t(0x000, 0x0BA, 0x000),        /*19*/
	rgb_t(0x000, 0x0F5, 0x000),        /*25*/
	rgb_t(0x000, 0x009, 0x000),        /*1*/
	rgb_t(0x000, 0x044, 0x000),        /*7*/
	rgb_t(0x000, 0x062, 0x000),        /*10*/
	rgb_t(0x000, 0x09C, 0x000),        /*16*/
	rgb_t(0x000, 0x044, 0x000),        /*7*/
	rgb_t(0x000, 0x0F5, 0x000),        /*25*/
	rgb_t(0x000, 0x0EB, 0x000),        /*24*/
	rgb_t(0x000, 0x0FF, 0x000),        /*26*/
	rgb_t(0x000, 0x03A, 0x000),        /*6*/
	rgb_t(0x000, 0x04E, 0x000),        /*8*/
	rgb_t(0x000, 0x093, 0x000),        /*15*/
	rgb_t(0x000, 0x0A6, 0x000),        /*17*/
	rgb_t(0x000, 0x009, 0x000),        /*1*/
	rgb_t(0x000, 0x0BA, 0x000),        /*19*/
	rgb_t(0x000, 0x0B0, 0x000),        /*18*/
	rgb_t(0x000, 0x0C4, 0x000),        /*20*/
	rgb_t(0x000, 0x000, 0x000),        /*0*/
	rgb_t(0x000, 0x013, 0x000),        /*2*/
	rgb_t(0x000, 0x058, 0x000),        /*9*/
	rgb_t(0x000, 0x06B, 0x000),        /*11*/
	rgb_t(0x000, 0x027, 0x000),        /*4*/
	rgb_t(0x000, 0x0D7, 0x000),        /*22*/
	rgb_t(0x000, 0x0CD, 0x000),        /*21*/
	rgb_t(0x000, 0x0E1, 0x000),        /*23*/
	rgb_t(0x000, 0x01D, 0x000),        /*3*/
	rgb_t(0x000, 0x031, 0x000),        /*5*/
	rgb_t(0x000, 0x075, 0x000),        /*12*/
	rgb_t(0x000, 0x089, 0x000)         /*14*/
};


/*******************************************************************
  Prototypes
*******************************************************************/

/* Initialise the palette */
PALETTE_INIT_MEMBER(amstrad_state,amstrad_cpc)
{
	palette.set_pen_colors(0, amstrad_palette, ARRAY_LENGTH(amstrad_palette));
}


PALETTE_INIT_MEMBER(amstrad_state,amstrad_cpc_green)
{
	palette.set_pen_colors(0, amstrad_green_palette, ARRAY_LENGTH(amstrad_green_palette));
}


/* Some games set the 8255 to mode 1 and expect a strobe signal */
/* on PC2. Apparently PC2 is always low on the CPC. ?!? */
TIMER_CALLBACK_MEMBER(amstrad_state::amstrad_pc2_low)
{
	m_ppi->pc2_w(0);
}


void amstrad_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_PC2_LOW:
		amstrad_pc2_low(ptr, param);
		break;
	case TIMER_VIDEO_UPDATE:
		amstrad_video_update_timer(ptr, param);
		break;
	case TIMER_SET_RESOLUTION:
		cb_set_resolution(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in amstrad_state::device_timer");
	}
}


/*************************************************************************/
/* KC Compact

The palette is defined by a colour rom. The only rom dump that exists (from the KC-Club webpage)
is 2K, which seems correct. In this rom the same 32 bytes of data is repeated throughout the rom.

When a I/O write is made to "Gate Array" to select the colour, Bit 7 and 6 are used by the
"Gate Array" to define the function, bit 7 = 0, bit 6 = 1. In the  Amstrad CPC, bits 4..0
define the hardware colour number, but in the KC Compact, it seems bits 5..0
define the hardware colour number allowing 64 colours to be chosen.

It is possible therefore that the colour rom could be reprogrammed, so that other colour
selections could be chosen allowing 64 different colours to be used. But this has not been tested
and co

colour rom byte:

Bit Function
7 not used
6 not used
5,4 Green value
3,2 Red value
1,0 Blue value

Green value, Red value, Blue value: 0 = 0%, 01/10 = 50%, 11 = 100%.
The 01 case is not used, it is unknown if this produces a different amount of colour.
*/

unsigned char amstrad_state::kccomp_get_colour_element(int colour_value)
{
	switch (colour_value)
	{
		case 0:
			return 0x00;
		case 1:
			return 0x60;
		case 2:
			return 0x60;
		case 3:
			return 0xff;
	}

	return 0xff;
}


/* the colour rom has the same 32 bytes repeated, but it might be possible to put a new rom in
with different data and be able to select the other entries - not tested on a real kc compact yet
and not supported by this driver */
PALETTE_INIT_MEMBER(amstrad_state,kccomp)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	color_prom = color_prom+0x018000;

	for (i=0; i<32; i++)
	{
		palette.set_pen_color(i,
			kccomp_get_colour_element((color_prom[i]>>2) & 0x03),
			kccomp_get_colour_element((color_prom[i]>>4) & 0x03),
			kccomp_get_colour_element((color_prom[i]>>0) & 0x03));
	}
}


/********************************************
Amstrad Plus

The Amstrad Plus has a 4096 colour palette
*********************************************/

PALETTE_INIT_MEMBER(amstrad_state,amstrad_plus)
{
	int i;

	palette.set_pen_colors(0, amstrad_palette, ARRAY_LENGTH(amstrad_palette) / 3);
	for ( i = 0; i < 0x1000; i++ )
	{
		int r, g, b;

		g = ( i >> 8 ) & 0x0f;
		r = ( i >> 4 ) & 0x0f;
		b = i & 0x0f;

		r = ( r << 4 ) | ( r );
		g = ( g << 4 ) | ( g );
		b = ( b << 4 ) | ( b );

		palette.set_pen_color(i, r, g, b);
	}
}


PALETTE_INIT_MEMBER(amstrad_state,aleste)
{
	int i;

	/* CPC Colour data is stored in the colour ROM (RFCOLDAT.BIN) at 0x140-0x17f */
	unsigned char* pal = memregion("user4")->base();

	for(i=0; i<32; i++)
	{
		int r,g,b;

		b = (pal[0x140+i] >> 4) & 0x03;
		g = (pal[0x140+i] >> 2) & 0x03;
		r = pal[0x140+i] & 0x03;

		r = (r << 6);
		g = (g << 6);
		b = (b << 6);

		palette.set_pen_color(i, r, g, b);
	}

	/* MSX colour palette is 6-bit RGB */
	for(i=0; i<64; i++)
	{
		int r,g,b;

		r = (i >> 4) & 0x03;
		g = (i >> 2) & 0x03;
		b = i & 0x03;

		r = (r << 6);
		g = (g << 6);
		b = (b << 6);

		palette.set_pen_color(i+32, r, g, b);
	}
}


void amstrad_state::amstrad_init_lookups()
{
	int i;

	for ( i = 0; i < 256; i++ )
	{
		m_mode0_lookup[i] = ( ( i & 0x80 ) >> 7 ) | ( ( i & 0x20 ) >> 3 ) | ( ( i & 0x08 ) >> 2 ) | ( ( i & 0x02 ) << 2 );
		m_mode1_lookup[i] = ( ( i & 0x80 ) >> 7 ) | ( ( i & 0x08 ) >> 2 );
		m_mode2_lookup[i] = ( ( i & 0x80 ) >> 7 );
	}
}


/* Set the new screen mode (0,1,2,4) from the GateArray */
void amstrad_state::amstrad_vh_update_mode()
{
	if ( m_system_type == SYSTEM_PLUS || m_system_type == SYSTEM_GX4000 )
	{
		/* select a cpc plus mode */
		switch ( m_gate_array.mrer & 0x03 )
		{
		case 0:     /* Mode 0: 160x200, 16 colours */
			m_gate_array.mode_lookup = m_mode0_lookup;
			m_gate_array.max_colour_ticks = 4;
			m_gate_array.ticks_increment = 1;
			break;

		case 1:     /* Mode 1: 320x200, 4 colous */
			m_gate_array.mode_lookup = m_mode1_lookup;
			m_gate_array.max_colour_ticks = 2;
			m_gate_array.ticks_increment = 1;
			break;

		case 2:     /* Mode 2: 640x200, 2 colours */
			m_gate_array.mode_lookup = m_mode2_lookup;
			m_gate_array.max_colour_ticks = 1;
			m_gate_array.ticks_increment = 1;
			break;

		case 3:     /* Mode 3: 160x200, 4 colours */
			m_gate_array.mode_lookup = m_mode0_lookup;
			m_gate_array.max_colour_ticks = 4;
			m_gate_array.ticks_increment = 1;
			break;
		}
	}
	else
	{
		if ( m_aleste_mode & 0x02 )
		{
			/* select an aleste mode */
			switch ( m_gate_array.mrer & 0x03 )
			{
			case 0:     /* Aleste Mode 0 (= Amstrad CPC mode 2): 640x200, 2 colours */
				m_gate_array.mode_lookup = m_mode2_lookup;
				m_gate_array.max_colour_ticks = 1;
				m_gate_array.ticks_increment = 1;
				break;

			case 1:     /* Aleste mode 1 (= Amstrad CPC mode 1): 320x200, 4 colours */
				m_gate_array.mode_lookup = m_mode1_lookup;
				m_gate_array.max_colour_ticks = 2;
				m_gate_array.ticks_increment = 1;
				break;

			case 2:     /* Aleste mode 2: 4 colours */
				m_gate_array.mode_lookup = m_mode1_lookup;
				m_gate_array.max_colour_ticks = 1;
				m_gate_array.ticks_increment = 2;
				break;

			case 3:     /* Aleste mode 3: 16 colours */
				m_gate_array.mode_lookup = m_mode0_lookup;
				m_gate_array.max_colour_ticks = 2;
				m_gate_array.ticks_increment = 2;
				break;
			}
		}
		else
		{
			/* select an original cpc mode */
			switch ( m_gate_array.mrer & 0x03 )
			{
			case 0:     /* Mode 0: 160x200, 16 colours */
				m_gate_array.mode_lookup = m_mode0_lookup;
				m_gate_array.max_colour_ticks = 4;
				m_gate_array.ticks_increment = 1;
				break;

			case 1:     /* Mode 1: 320x200, 4 colous */
				m_gate_array.mode_lookup = m_mode1_lookup;
				m_gate_array.max_colour_ticks = 2;
				m_gate_array.ticks_increment = 1;
				break;

			case 2:     /* Mode 2: 640x200, 2 colours */
				m_gate_array.mode_lookup = m_mode2_lookup;
				m_gate_array.max_colour_ticks = 1;
				m_gate_array.ticks_increment = 1;
				break;

			case 3:     /* Mode 3: 160x200, 4 colours */
				m_gate_array.mode_lookup = m_mode0_lookup;
				m_gate_array.max_colour_ticks = 4;
				m_gate_array.ticks_increment = 1;
				break;
			}
		}
	}
}


/*
DMA commands

0RDDh   LOAD R,D    Load 8 bit data D to PSG register R (0<=R<=15)
1NNNh   PAUSE N     Pause for N prescaled ticks (0<N<=4095)
2NNNh   REPEAT N    Set loop counter to N for this stream (0<N<=4095), and mark next instruction as loop start.
3xxxh   (reserved)  Do not use
4000h   NOP     No operation (64us idle)
4001h   LOOP    If loop counter non zero, loop back to the first instruction after REPEAT instruction and decrement loop counter.
4010h   INT     Interrupt the CPU
4020h   STOP    Stop processing the sound list.
*/

void amstrad_state::amstrad_plus_dma_parse(int channel)
{
	unsigned short command;

	if( m_asic.dma_addr[channel] & 0x01)
		m_asic.dma_addr[channel]++;  // align to even address

	if ( m_asic.dma_pause[channel] != 0 )
	{  // do nothing, this channel is paused
		m_asic.dma_prescaler[channel]--;
		if ( m_asic.dma_prescaler[channel] == 0 )
		{
			m_asic.dma_pause[channel]--;
			m_asic.dma_prescaler[channel] = m_asic.ram[0x2c02 + (4*channel)] + 1;
		}
		return;
	}
	command = (m_ram->pointer()[m_asic.dma_addr[channel]+1] << 8) + m_ram->pointer()[m_asic.dma_addr[channel]];
//  logerror("DMA #%i: address %04x: command %04x\n",channel,m_asic.dma_addr[channel],command);
	switch (command & 0xf000)
	{
	case 0x0000:  // Load PSG register
		{
			m_ay->address_w(generic_space(), 0, (command & 0x0f00) >> 8);
			m_ay->data_w(generic_space(), 0, command & 0x00ff);
			m_ay->address_w(generic_space(), 0, m_prev_reg);
		}
		logerror("DMA %i: LOAD %i, %i\n",channel,(command & 0x0f00) >> 8, command & 0x00ff);
		break;
	case 0x1000:  // Pause for n HSYNCs (0 - 4095)
		m_asic.dma_pause[channel] = (command & 0x0fff) - 1;
		logerror("DMA %i: PAUSE %i\n",channel,command & 0x0fff);
		break;
	case 0x2000:  // Beginning of repeat loop
		m_asic.dma_repeat[channel] = m_asic.dma_addr[channel];
		m_asic.dma_loopcount[channel] = (command & 0x0fff);
		logerror("DMA %i: REPEAT %i\n",channel,command & 0x0fff);
		break;
	case 0x4000:  // Control functions
		if (command & 0x01) // Loop back to last Repeat instruction
		{
			if (m_asic.dma_loopcount[channel] > 0)
			{
				m_asic.dma_addr[channel] = m_asic.dma_repeat[channel];
				logerror("DMA %i: LOOP (%i left)\n",channel,m_asic.dma_loopcount[channel]);
				m_asic.dma_loopcount[channel]--;
			}
			else
				logerror("DMA %i: LOOP (end)\n",channel);
		}
		if (command & 0x10) // Cause interrupt
		{
			m_plus_irq_cause = channel * 2;
			m_asic.ram[0x2c0f] |= (0x40 >> channel);
			m_maincpu->set_input_line(0, ASSERT_LINE);
			logerror("DMA %i: INT\n",channel);
		}
		if (command & 0x20)  // Stop processing on this channel
		{
			m_asic.dma_status &= ~(0x01 << channel);
			logerror("DMA %i: STOP\n",channel);
		}
		break;
	default:
		logerror("DMA: Unknown DMA command - %04x - at address &%04x\n",command,m_asic.dma_addr[channel]);
	}
	m_asic.dma_addr[channel] += 2;  // point to next DMA instruction
}


void amstrad_state::amstrad_plus_handle_dma()
{
	if ( m_asic.dma_status & 0x01 )  // DMA channel 0
	{
		amstrad_plus_dma_parse( 0 );
	}

	if ( m_asic.dma_status & 0x02 )  // DMA channel 1
	{
		amstrad_plus_dma_parse( 1 );
	}

	if ( m_asic.dma_status & 0x04 )  // DMA channel 2
	{
		amstrad_plus_dma_parse( 2 );
	}
}

TIMER_CALLBACK_MEMBER(amstrad_state::amstrad_video_update_timer)
{
	if(param == 1)
	{
		amstrad_plus_update_video();
	}
	else
	{
		amstrad_update_video();
	}
}

/* Set the new colour from the GateArray */
void amstrad_state::amstrad_vh_update_colour(int PenIndex, UINT16 hw_colour_index)
{
	if ( m_system_type == SYSTEM_PLUS || m_system_type == SYSTEM_GX4000 )
	{
		int val;

		timer_set(attotime::from_usec(0), TIMER_VIDEO_UPDATE, 1);

		/* CPC+/GX4000 - normal palette changes through the Gate Array also makes the corresponding change in the ASIC palette */
		val = (amstrad_palette[hw_colour_index] & 0xf00000) >> 16; /* red */
		val |= (amstrad_palette[hw_colour_index] & 0x0000f0) >> 4; /* blue */
		m_asic.ram[0x2400+PenIndex*2] = val;
		val = (amstrad_palette[hw_colour_index] & 0x00f000) >> 12; /* green */
		m_asic.ram[0x2401+PenIndex*2] = val;
	}
	else
	{
		timer_set(attotime::from_usec(0), TIMER_VIDEO_UPDATE, 0);
	}
	m_GateArray_render_colours[PenIndex] = hw_colour_index;
}


void amstrad_state::aleste_vh_update_colour(int PenIndex, UINT16 hw_colour_index)
{
	timer_set(attotime::from_usec(0), TIMER_VIDEO_UPDATE, 0);
	m_GateArray_render_colours[PenIndex] = hw_colour_index+32;
}


void amstrad_state::amstrad_gate_array_get_video_data()
{
	if ( m_aleste_mode & 0x02 )
	{
		m_gate_array.address = ( ( m_gate_array.ma & 0x2000 ) << 2 ) | ( ( m_gate_array.ra & 0x06 ) << 11 ) | ( ( m_gate_array.ra & 0x01 ) << 14 ) | ( ( m_gate_array.ma & 0x7ff ) << 1 );
	}
	else
	{
		m_gate_array.address = ( ( m_gate_array.ma & 0x3000 ) << 2 ) | ( ( m_gate_array.ra & 0x07 ) << 11 ) | ( ( m_gate_array.ma & 0x3ff ) << 1 );
	}
	m_gate_array.data = m_ram->pointer()[ m_gate_array.address ];
	m_gate_array.colour = m_GateArray_render_colours[ m_gate_array.mode_lookup[m_gate_array.data] ];
	m_gate_array.colour_ticks = m_gate_array.max_colour_ticks;
	m_gate_array.ticks = 0;
}


void amstrad_state::amstrad_update_video()
{
	attotime now = machine().time();


	if ( m_gate_array.draw_p )
	{
		UINT32 cycles_passed = (now - m_gate_array.last_draw_time ).as_ticks(XTAL_16MHz);

		while( cycles_passed )
		{
			if ( ! m_gate_array.de || ( ( m_aleste_mode & 0x02 ) && ! ( m_aleste_mode & 0x08 ) ) )
			{
				*m_gate_array.draw_p = m_GateArray_render_colours[ 16 ];
			}
			else
			{
				*m_gate_array.draw_p = m_gate_array.colour;
				m_gate_array.colour_ticks--;
				if ( ! m_gate_array.colour_ticks )
				{
					m_gate_array.data <<= 1;
					m_gate_array.colour = m_GateArray_render_colours[ m_gate_array.mode_lookup[m_gate_array.data] ];
					m_gate_array.colour_ticks = m_gate_array.max_colour_ticks;
				}
				m_gate_array.ticks += m_gate_array.ticks_increment;
				switch( m_gate_array.ticks)
				{
				case 8:
					m_gate_array.data = m_ram->pointer()[ m_gate_array.address + 1 ];
					m_gate_array.colour = m_GateArray_render_colours[ m_gate_array.mode_lookup[m_gate_array.data] ];
					break;
				case 16:
					m_gate_array.ma += 1;                        /* If we were synced with the 6845 mc6845_get_ma should return this value */
					amstrad_gate_array_get_video_data();
					break;
				}
			}
			m_gate_array.draw_p++;
			cycles_passed--;
			m_gate_array.line_ticks++;
			if ( m_gate_array.line_ticks > m_gate_array.bitmap->width() )
			{
				m_gate_array.draw_p = NULL;
				cycles_passed = 0;
			}
		}
	}

	m_gate_array.last_draw_time = now;
}


void amstrad_state::amstrad_plus_gate_array_get_video_data()
{
	UINT16 caddr;
	UINT16 ma = ( m_gate_array.ma - m_asic.split_ma_started ) + m_asic.split_ma_base;
	UINT16 ra = m_gate_array.ra + ( ( m_asic.ram[0x2804] >> 4 ) & 0x07 );

	if ( ra > 7 )
	{
		ma += m_asic.horiz_disp;
	}

	m_gate_array.address = ( ( ma & 0x3000 ) << 2 ) | ( ( ra & 0x07 ) << 11 ) | ( ( ma & 0x3ff ) << 1 );
	m_gate_array.data = m_ram->pointer()[ m_gate_array.address ];
	if((m_asic.ram[0x2804] & 0x80) && m_asic.hsync_first_tick)
		caddr = 0x2420;
	else
		caddr = 0x2400 + m_gate_array.mode_lookup[m_gate_array.data] * 2;
	m_gate_array.colour = m_asic.ram[caddr] + ( m_asic.ram[caddr+1] << 8 );
	m_gate_array.colour_ticks = m_gate_array.max_colour_ticks;
	m_gate_array.ticks = 0;
}


void amstrad_state::amstrad_plus_update_video()
{
	attotime now = machine().time();

	if ( m_gate_array.draw_p )
	{
		UINT32 cycles_passed = (now - m_gate_array.last_draw_time ).as_ticks(XTAL_16MHz);

		while( cycles_passed )
		{
			if ( ! m_gate_array.de )
			{
				*m_gate_array.draw_p = m_asic.ram[0x2420] + ( m_asic.ram[0x2421] << 8 );
			}
			else
			{
				*m_gate_array.draw_p = m_gate_array.colour;

				if ( m_asic.hscroll )
				{
					m_asic.hscroll--;
					if ( m_asic.hscroll == 0 )
						amstrad_plus_gate_array_get_video_data();
				}
				else
				{
					m_gate_array.colour_ticks--;
					if ( ! m_gate_array.colour_ticks )
					{
						UINT16 caddr;

						m_gate_array.data <<= 1;
						if((m_asic.ram[0x2804] & 0x80) && m_asic.hsync_first_tick)
							caddr = 0x2420;
						else
							caddr = 0x2400 + m_gate_array.mode_lookup[m_gate_array.data] * 2;
						m_gate_array.colour = m_asic.ram[caddr] + ( m_asic.ram[caddr+1] << 8 );
						m_gate_array.colour_ticks = m_gate_array.max_colour_ticks;
					}
					m_gate_array.ticks += m_gate_array.ticks_increment;
					switch( m_gate_array.ticks)
					{
					case 8:
						{
							UINT16 caddr;

							m_gate_array.data = m_ram->pointer()[ m_gate_array.address + 1 ];
							if((m_asic.ram[0x2804] & 0x80) && m_asic.hsync_first_tick)
								caddr = 0x2420;
							else
								caddr = 0x2400 + m_gate_array.mode_lookup[m_gate_array.data] * 2;
							m_gate_array.colour = m_asic.ram[caddr] + ( m_asic.ram[caddr+1] << 8 );
						}
						break;
					case 16:
						m_gate_array.ma += 1;                        /* If we were synced with the 6845 mc6845_get_ma should return this value */
						amstrad_plus_gate_array_get_video_data();
						break;
					}
				}
			}
			m_gate_array.draw_p++;
			cycles_passed--;
			m_gate_array.line_ticks++;
			m_asic.hsync_tick_count++;
			if(m_asic.hsync_tick_count > 16)
				m_asic.hsync_first_tick = false;
			if ( m_gate_array.line_ticks >= m_gate_array.bitmap->width() )
			{
				m_gate_array.draw_p = NULL;
				cycles_passed = 0;
			}
		}
	}

	m_gate_array.last_draw_time = now;
}


void amstrad_state::amstrad_plus_update_video_sprites()
{
	UINT16  *p;
	int i;
	UINT16 horiz;

	if ( m_gate_array.y < 0 )
		return;

	if(m_asic.h_end < m_asic.h_start)
		horiz = 0;
	else
		horiz = m_asic.h_start;

	p = &m_gate_array.bitmap->pix16(m_gate_array.y, horiz );
	for ( i = 15 * 8; i >= 0; i -= 8 )
	{
		UINT8   xmag = ( m_asic.ram[ 0x2000 + i + 4 ] >> 2 ) & 0x03;
		UINT8   ymag = m_asic.ram[ 0x2000 + i + 4 ] & 0x03;

		/* Check if sprite is enabled */
		if ( xmag && ymag )
		{
			INT16   spr_x = m_asic.ram[ 0x2000 + i ] + ( m_asic.ram[ 0x2001 + i ] << 8 );
			INT16   spr_y = m_asic.ram[ 0x2002 + i ] + ( m_asic.ram[ 0x2003 + i ] << 8 );

			xmag -= 1;
			ymag -= 1;

			/* Check if sprite would appear on this scanline */
//if(i == 0) printf("%i <= %i, %i < %i, %i < %i, %i > 0\n", spr_y,m_asic.vpos,m_asic.vpos,spr_y+(16<<ymag),spr_x,(m_asic.h_end - horiz),spr_x+(16<<xmag));
			if ( spr_y <= m_asic.vpos && m_asic.vpos < spr_y + ( 16 << ymag ) && spr_x < (m_asic.h_end - horiz) && spr_x + ( 16 << xmag ) > 0 )
			{
				UINT16  spr_addr = i * 32 + ( ( ( m_asic.vpos - spr_y ) >> ymag ) * 16 );
				int     j, k;
				for ( j = 0; j < 16; j++ )
				{
					for ( k = 0; k < ( 1 << xmag ); k++ )
					{
						INT16 x = spr_x + ( j << xmag ) + k;
						if ( x >= 0 && x < (m_asic.h_end - horiz))
						{
							UINT8   spr_col = ( m_asic.ram[ spr_addr + j ] & 0x0f ) * 2;
							if ( spr_col )
								p[x] = m_asic.ram[ 0x2420 + spr_col ] + ( m_asic.ram[ 0x2421 + spr_col ] << 8 );
						}
					}
				}
			}
		}
	}
}


WRITE_LINE_MEMBER(amstrad_state::amstrad_hsync_changed)
{
	amstrad_update_video();

	/* The gate array reacts to de-assertion of the hsycnc 6845 line */
	if ( m_gate_array.hsync && !state )
	{
		m_gate_array.hsync_counter++;
		/* Advance to next drawing line */
		m_gate_array.y++;
		m_gate_array.line_ticks = 0;
		if ( m_gate_array.y >= 0 && m_gate_array.y < m_gate_array.bitmap->height() )
		{
			m_gate_array.draw_p = &m_gate_array.bitmap->pix16(m_gate_array.y);
		}
		else
		{
			m_gate_array.draw_p = NULL;
		}

		if ( m_gate_array.hsync_after_vsync_counter != 0 )  // counters still operate regardless of PRI state
		{
			m_gate_array.hsync_after_vsync_counter--;

			if (m_gate_array.hsync_after_vsync_counter == 0)
			{
				if (m_gate_array.hsync_counter >= 32)
				{
					m_maincpu->set_input_line(0, ASSERT_LINE);
				}
				m_gate_array.hsync_counter = 0;
			}
		}

		if ( m_gate_array.hsync_counter >= 52 )
		{
			m_gate_array.hsync_counter = 0;
			m_maincpu->set_input_line(0, ASSERT_LINE);
		}
	}
	m_gate_array.hsync = state ? 1 : 0;
}


WRITE_LINE_MEMBER(amstrad_state::amstrad_plus_hsync_changed)
{
	amstrad_plus_update_video();

	if ( m_gate_array.hsync && !state )
	{
		m_gate_array.hsync_counter++;
		/* Advance to next drawing line */
		m_gate_array.y++;
		m_gate_array.line_ticks = 0;
		if ( m_gate_array.y >= 0 && m_gate_array.y < m_gate_array.bitmap->height() )
		{
			m_gate_array.draw_p = &m_gate_array.bitmap->pix16(m_gate_array.y);
		}
		else
		{
			m_gate_array.draw_p = NULL;
		}

		if ( m_gate_array.hsync_after_vsync_counter != 0 )  // counters still operate regardless of PRI state
		{
			m_gate_array.hsync_after_vsync_counter--;

			if (m_gate_array.hsync_after_vsync_counter == 0)
			{
				if (m_gate_array.hsync_counter >= 32)
				{
					if( m_asic.pri == 0 || m_asic.enabled == 0)
					{
						m_maincpu->set_input_line(0, ASSERT_LINE);
					}
				}
				m_gate_array.hsync_counter = 0;
			}
		}

		if ( m_gate_array.hsync_counter >= 52 )
		{
			m_gate_array.hsync_counter = 0;
			if ( m_asic.pri == 0 || m_asic.enabled == 0 )
			{
				m_maincpu->set_input_line(0, ASSERT_LINE);
			}
		}

		if ( m_asic.enabled )
		{
			// CPC+/GX4000 Programmable Raster Interrupt (disabled if &6800 in ASIC RAM is 0)
			if ( m_asic.pri != 0 )
			{
				if ( m_asic.pri == m_asic.vpos )
				{
					logerror("PRI: triggered, scanline %d\n",m_asic.pri);
					m_maincpu->set_input_line(0, ASSERT_LINE);
					m_plus_irq_cause = 0x06;  // raster interrupt vector
					m_gate_array.hsync_counter &= ~0x20;  // ASIC PRI resets the MSB of the raster counter
				}
			}
			// CPC+/GX4000 Split screen registers  (disabled if &6801 in ASIC RAM is 0)
			if(m_asic.ram[0x2801] != 0)
			{
				if ( m_asic.ram[0x2801] == m_asic.vpos - 1 )    // split occurs here (hopefully)
				{
					logerror("SSCR: Split screen occurred at scanline %d\n",m_asic.ram[0x2801]);
				}
			}
			// CPC+/GX4000 DMA channels
			amstrad_plus_handle_dma();  // a DMA command is handled at the leading edge of HSYNC (every 64us)
			m_asic.vpos++;
		}
	}
	m_gate_array.hsync = state ? 1 : 0;
}


WRITE_LINE_MEMBER(amstrad_state::amstrad_vsync_changed)
{
	amstrad_update_video();

	if ( ! m_gate_array.vsync && state )
	{
		/* Reset the amstrad_CRTC_HS_After_VS_Counter */
		m_gate_array.hsync_after_vsync_counter = 3;

		/* Start of new frame */
		m_gate_array.y = -1;
		m_asic.de_start = 0;
	}

	m_gate_array.vsync = state ? 1 : 0;

	/* Schedule a write to PC2 */
	timer_set(attotime::zero, TIMER_PC2_LOW);
}


WRITE_LINE_MEMBER(amstrad_state::amstrad_plus_vsync_changed)
{
	amstrad_plus_update_video();

	if ( ! m_gate_array.vsync && state )
	{
		/* Reset the amstrad_CRTC_HS_After_VS_Counter */
		m_gate_array.hsync_after_vsync_counter = 3;

		/* Start of new frame */
		m_gate_array.y = -1;
		m_asic.de_start = 0;
	}

	m_gate_array.vsync = state ? 1 : 0;

	/* Schedule a write to PC2 */
	timer_set(attotime::zero, TIMER_PC2_LOW);
}


WRITE_LINE_MEMBER(amstrad_state::amstrad_de_changed)
{
	amstrad_update_video();

	if ( ! m_gate_array.de && state )
	{
		/* DE became active, store the starting MA and RA signals */
		mc6845_device *mc6845 = m_crtc;

		if(m_asic.de_start == 0)
			m_asic.vpos = 1;

		m_gate_array.ma = mc6845->get_ma();
		m_gate_array.ra = mc6845->get_ra();
//logerror("y = %d; ma = %02x; ra = %02x, address = %04x\n", m_gate_array.y, m_gate_array.ma, m_gate_array.ra, ( ( m_gate_array.ma & 0x3000 ) << 2 ) | ( ( m_gate_array.ra & 0x07 ) << 11 ) | ( ( m_gate_array.ma & 0x3ff ) << 1 ) );
		amstrad_gate_array_get_video_data();
		m_asic.de_start = 1;
	}

	m_gate_array.de = state ? 1 : 0;
}


WRITE_LINE_MEMBER(amstrad_state::amstrad_plus_de_changed)
{
	amstrad_plus_update_video();

	if ( ! m_gate_array.de && state )
	{
		/* DE became active, store the starting MA and RA signals */
		m_gate_array.ma = m_crtc->get_ma();
		m_gate_array.ra = m_crtc->get_ra();

		m_asic.hsync_first_tick = true;
		m_asic.hsync_tick_count = 0;
		m_asic.h_start = m_gate_array.line_ticks;
		if(m_asic.de_start == 0)
			m_asic.vpos = 0;
		m_asic.de_start = 1;
		m_gate_array.colour = m_asic.ram[0x2420] + ( m_asic.ram[0x2421] << 8 );
		m_asic.hscroll = m_asic.ram[0x2804] & 0x0f;
		if ( m_asic.hscroll == 0 )
			amstrad_plus_gate_array_get_video_data();

		/* Start of screen */
		if ( m_asic.vpos == 0 )
		{
			m_asic.split_ma_base = 0x0000;
			m_asic.split_ma_started = 0x0000;
		}
		/* Start of split screen section */
		else if ( m_asic.enabled && m_asic.ram[0x2801] != 0 && m_asic.ram[0x2801] == m_asic.vpos - 1 )
		{
			m_asic.split_ma_started = m_gate_array.ma;
			m_asic.split_ma_base = ( m_asic.ram[0x2802] << 8 ) | m_asic.ram[0x2803];
		}
	}

	if ( m_gate_array.de && ! state )
	{
		m_asic.h_end = m_gate_array.line_ticks;
		amstrad_plus_update_video_sprites();
	}

	m_gate_array.de = state ? 1 : 0;
}


VIDEO_START_MEMBER(amstrad_state,amstrad)
{
	amstrad_init_lookups();

	m_gate_array.bitmap = auto_bitmap_ind16_alloc( machine(), m_screen->width(), m_screen->height() );
	m_gate_array.hsync_after_vsync_counter = 3;
}


UINT32 amstrad_state::screen_update_amstrad(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap( bitmap, *m_gate_array.bitmap, 0, 0, 0, 0, cliprect );
	return 0;
}


/* traverses the daisy-chain of expansion devices, looking for the specified device */
static device_t* get_expansion_device(running_machine &machine, const char* tag)
{
	amstrad_state *state = machine.driver_data<amstrad_state>();
	cpc_expansion_slot_device* exp_port = state->m_exp;

	while(exp_port != NULL)
	{
		device_t* temp;

		// first, check if this expansion port has the device we want attached
		temp = exp_port->subdevice(tag);
		if(temp != NULL)
			return temp;

		// if it's not what we're looking for, then check the expansion port on this expansion device. if it exists.
		temp = dynamic_cast<device_t*>(exp_port->get_card_device());
		if(temp == NULL)
			return NULL; // no device attached
		exp_port = temp->subdevice<cpc_expansion_slot_device>("exp");
		if(exp_port == NULL)
			return NULL;  // we're at the end of the chain
	}
	return NULL;
}

WRITE_LINE_MEMBER(amstrad_state::cpc_romdis)
{
	m_gate_array.romdis = state;
	amstrad_rethinkMemory();
}


/*--------------------------
  - Ram and Rom management -
  --------------------------*/
/*-----------------
  - Set Lower Rom -
  -----------------*/
void amstrad_state::amstrad_setLowerRom()
{
	UINT8 *bank_base;

	/* b2 : "1" Lower rom area disable or "0" Lower rom area enable */
	if ( m_system_type == SYSTEM_CPC || m_system_type == SYSTEM_ALESTE )
	{
		if ((m_gate_array.mrer & (1<<2)) == 0 && m_gate_array.romdis == 0)
		{
			bank_base = &m_region_maincpu->base()[0x010000];
		}
		else
		{
			if(m_aleste_mode & 0x04)
				bank_base = m_Aleste_RamBanks[0];
			else
				bank_base = m_AmstradCPC_RamBanks[0];
		}
		m_bank1->set_base(bank_base);
		m_bank2->set_base(bank_base+0x02000);
		if ((m_gate_array.mrer & (1<<2)) == 0 && m_gate_array.romdis == 0)
		{
			if (m_exp)
				m_exp->set_mapping(MAP_LOWER);
		}
	}
	else  // CPC+/GX4000
	{
		//address_space &space = state->m_maincpu->space(AS_PROGRAM);

/*      if ( m_asic.enabled && ( m_asic.rmr2 & 0x18 ) == 0x18 )
        {
            space.install_read_handler(0x4000, 0x5fff, read8_delegate(FUNC(amstrad_state::amstrad_plus_asic_4000_r),this));
            space.install_read_handler(0x6000, 0x7fff, read8_delegate(FUNC(amstrad_state::amstrad_plus_asic_6000_r),this));
            space.install_write_handler(0x4000, 0x5fff, write8_delegate(FUNC(amstrad_state::amstrad_plus_asic_4000_w),this));
            space.install_write_handler(0x6000, 0x7fff, write8_delegate(FUNC(amstrad_state::amstrad_plus_asic_6000_w),this));
        }
        else
        {
            space.install_read_bank(0x4000, 0x5fff, "bank3");
            space.install_read_bank(0x6000, 0x7fff, "bank4");
            space.install_write_bank(0x4000, 0x5fff, "bank11");
            space.install_write_bank(0x6000, 0x7fff, "bank12");
        }
*/
		if(m_AmstradCPC_RamBanks[0] != NULL)
		{
			m_bank1->set_base(m_AmstradCPC_RamBanks[0]);
			m_bank2->set_base(m_AmstradCPC_RamBanks[0]+0x2000);
			m_bank3->set_base(m_AmstradCPC_RamBanks[1]);
			m_bank4->set_base(m_AmstradCPC_RamBanks[1]+0x2000);
			m_bank5->set_base(m_AmstradCPC_RamBanks[2]);
			m_bank6->set_base(m_AmstradCPC_RamBanks[2]+0x2000);
		}

		if ( (m_gate_array.mrer & (1<<2)) == 0 && m_gate_array.romdis == 0)
		{  // ASIC secondary lower ROM selection (bit 5: 1 = enabled)
			if ( m_asic.enabled )
			{
//              logerror("L-ROM: Lower ROM enabled, cart bank %i\n", m_asic.rmr2 & 0x07 );
				bank_base = &m_region_cart->base()[0x4000 * (m_asic.rmr2 & 0x07)];
				switch( m_asic.rmr2 & 0x18 )
				{
				case 0x00:
//                  logerror("L-ROM: located at &0000\n");
					m_bank1->set_base(bank_base);
					m_bank2->set_base(bank_base+0x02000);
					break;
				case 0x08:
//                  logerror("L-ROM: located at &4000\n");
					m_bank3->set_base(bank_base);
					m_bank4->set_base(bank_base+0x02000);
					break;
				case 0x10:
//                  logerror("L-ROM: located at &8000\n");
					m_bank5->set_base(bank_base);
					m_bank6->set_base(bank_base+0x02000);
					break;
				case 0x18:
//                  logerror("L-ROM: located at &0000, ASIC registers enabled\n");
					m_bank1->set_base(bank_base);
					m_bank2->set_base(bank_base+0x02000);
					break;
				}
			}
			else
			{
				m_bank1->set_base(m_region_cart->base());
				m_bank2->set_base(m_region_cart->base() + 0x2000);
				if (m_exp)
					m_exp->set_mapping(MAP_LOWER);
			}
		}
	}
}


/*-----------------
  - Set Upper Rom -
  -----------------*/
void amstrad_state::amstrad_setUpperRom()
{
	UINT8 *bank_base = NULL;

	/* b3 : "1" Upper rom area disable or "0" Upper rom area enable */
	if ( ! ( m_gate_array.mrer & 0x08 ) && m_gate_array.romdis == 0)
	{
		bank_base = m_Amstrad_ROM_Table[ m_gate_array.upper_bank ];
	}
	else
	{
		if(m_aleste_mode & 0x04)
			bank_base = m_Aleste_RamBanks[3];
		else
			bank_base = m_AmstradCPC_RamBanks[3];
	}

	if (bank_base)
	{
		m_bank7->set_base(bank_base);
		m_bank8->set_base(bank_base+0x2000);
	}
	if ( ! ( m_gate_array.mrer & 0x08 ) && m_gate_array.romdis == 0)
	{
		if (m_exp)
			m_exp->set_mapping(MAP_UPPER);
	}

}


/*--------------------------
  - Ram and Rom management -
  --------------------------*/

/* simplified ram configuration - e.g. only correct for 128k machines

RAM Expansion Bits
                             7 6 5 4  3  2  1  0
CPC6128                      1 1 - -  -  s2 s1 s0
Dk'tronics 256K Silicon Disk 1 1 1 b1 b0 b2 -  -

"-" - this bit is ignored. The value of this bit is not important.
"0" - this bit must be set to "0"
"1" - this bit must be set to "1"
"b0,b1,b2" - this bit is used to define the logical 64k block that the ram configuration uses
"s0,s1,s2" - this bit is used to define the ram configuration

The CPC6128 has a 64k ram expansion built-in, giving 128K of RAM in this system.
In the CPC464,CPC664 and KC compact if a ram expansion is not present, then writing to this port has no effect and the ram will be in the same arrangement as if configuration 0 had been selected.
*/
void amstrad_state::AmstradCPC_GA_SetRamConfiguration()
{
	int ConfigurationIndex = m_GateArray_RamConfiguration & 0x07;
	int BankIndex,i;
	unsigned char *BankAddr;
	UINT8 banknum = (m_GateArray_RamConfiguration & 0x38) >> 3;

/* if b5 = 0 */
	if(m_ram->size() > 65536)
	{
		for (i=0;i<4;i++)
		{
			BankIndex = RamConfigurations[(ConfigurationIndex << 2) + i];
			if(BankIndex > 3)
			{
				UINT8 maxbank = ((m_ram->size()-65536) / 65536);
				BankAddr = m_ram->pointer() + (BankIndex << 14) + ((banknum%maxbank)*0x10000);
			}
			else
				BankAddr = m_ram->pointer() + (BankIndex << 14);
			m_Aleste_RamBanks[i] = BankAddr;
			m_AmstradCPC_RamBanks[i] = BankAddr;
		}
	}
	else
	{
		// set normal 64k RAM mapping
		for (i=0;i<4;i++)
		{
			BankAddr = m_ram->pointer() + (i << 14);
			m_Aleste_RamBanks[i] = BankAddr;
			m_AmstradCPC_RamBanks[i] = BankAddr;
		}
	}
	amstrad_rethinkMemory();
}


/* ASIC RAM */
/*
    ASIC RAM Layout.  Always is mapped to &4000-&7fff

    Hardware sprites: 16 sprites, 16x16, basic zooming, 15 colour (12-bit palette)
    Pixel data is 0 - 15 for sprite pen number, low 4 bits
    &4000 - &400f Pixel data for first line of first sprite
    &4010 - &401f Pixel data for second line of first sprite
      ...     ...
    &40f0 - &40ff Pixel data for last (16th) line of first sprite
      ...     ...
    &4100 - &41ff Pixel data for second sprite
    &4200 - &42ff Third sprite
      ...     ...
    &4f00 - &4fff Pixel data for last (16th) sprite

    &6000 - &607f Sprite properties (8 bytes each)
                  +0 Sprite X position LSB
                  +1 Sprite X position MSB
                  +2 Sprite Y position LSB (scanline)
                  +3 Sprite Y position MSB
                  +4 Sprite zoom - bits 3,2 X magnification, bits 1,0 Y magnification
                      Magnification: 00 = not displayed, 01 = x1, 10 = x2, 11 = x4

    Palette: LSB first, presumably GGGGBBBBxxxxRRRR
    &6400 - &641f Pen palette (12bpp, 2 bytes each, 16 total)
    &6420 - &6421 Border palette  (12bpp, 2 bytes)
    &6422 - &643f Hardware sprite palette (12bpp, 2 bytes each, 15 total)

    Programmable Raster Interrupt:
    &6800  Scanline for IRQ to be triggered after
           If 0, raster interrupts and DMA interrupts occur
           Otherwise, the PRI interrupt is triggered only

    Hardware split screen:
    &6801  Scanline for split to occur at
    &6802  LSB of screen address for split (like reg 12 of the 6845)
    &6803  MSB of the above (like reg 13 of the 6845)

    Soft Scroll Control Register:
    &6804  bits 3-0 - horizontal delay in mode 2 pixels (shifts display to the right)
           bits 6-4 - added to the 3 LSBs for the scanline address (shifts display up)
           bit 7    - extends the border by two bytes (16 mode 2 pixels), masking the bad data from the horizontal scroll

    Analogue paddle ports:
    &6808 - &680f  Analogue input, read-only, 6 bit

    PSG DMA channels:
    &6c00  DMA channel 0 address LSB
    &6c01  DMA channel 0 address MSB
    &6c02  DMA channel 0 prescaler
    &6c03  unused
    &6c04-7  DMA channel 1
    &6c08-b  DMA channel 2
    &6c0f  Control and Status register
             bit 7 - raster interrupt
             bit 6 - DMA channel 0 interrupt
             bit 5 - DMA channel 1 interrupt
             bit 4 - DMA channel 2 interrupt
             bit 3 - unused (write 0)
             bit 2 - DMA channel 2 enable
             bit 1 - DMA channel 1 enable
             bit 0 - DMA channel 0 enable

 */

WRITE8_MEMBER(amstrad_state::amstrad_plus_asic_4000_w)
{
//  logerror("ASIC: Write to register at &%04x\n",offset+0x4000);
	if ( m_asic.enabled && ( m_asic.rmr2 & 0x18 ) == 0x18 )
		m_asic.ram[offset] = data & 0x0f;
	else
	{
		UINT8* RAM = (UINT8*)m_bank11->base();
		RAM[offset] = data;
	}
}


WRITE8_MEMBER(amstrad_state::amstrad_plus_asic_6000_w)
{
	if ( m_asic.enabled && ( m_asic.rmr2 & 0x18 ) == 0x18 )
	{
		m_asic.ram[offset+0x2000] = data;
		if(offset >= 0x0400 && offset < 0x440 && ( offset & 0x01 ) ) // ASIC palette
		{
			m_asic.ram[ offset + 0x2000 ] = data & 0x0f;
		}
		if(offset == 0x0800)  // Programmable raster interrupt
		{
			// logerror("ASIC: Wrote %02x to PRI\n",data);
			m_asic.pri = data;
		}
		if(offset >= 0x0801 && offset <= 0x0803)  // Split screen registers
		{
			logerror("ASIC: Split screen at line %i, address &%04x\n",m_asic.ram[0x2801],m_asic.ram[0x2803] + (m_asic.ram[0x2802] << 8));
		}
		if(offset == 0x0804)  // Soft scroll register
		{
		}
		if(offset == 0x0805)  // Interrupt vector register (for IM 2, used by Pang)
		{
			// high 5 bits go to interrupt vector
			int vector;

			if ( m_asic.enabled )
			{
				vector = (data & 0xf8) + (m_plus_irq_cause);
				m_maincpu->set_input_line_vector(0, vector);
				logerror("ASIC: IM 2 vector write %02x, data = &%02x\n",vector,data);
			}
			m_asic.dma_clear = data & 0x01;
		}
		// DMA channels
		switch(offset)
		{
		case 0x0c00:
		case 0x0c01:
			m_asic.dma_addr[0] = (m_asic.ram[0x2c01] << 8) + m_asic.ram[0x2c00];
			m_asic.dma_status &= ~0x01;
			logerror("ASIC: DMA 0 address set to &%04x\n",m_asic.dma_addr[0]);
			break;
		case 0x0c04:
		case 0x0c05:
			m_asic.dma_addr[1] = (m_asic.ram[0x2c05] << 8) + m_asic.ram[0x2c04];
			m_asic.dma_status &= ~0x02;
			logerror("ASIC: DMA 1 address set to &%04x\n",m_asic.dma_addr[1]);
			break;
		case 0x0c08:
		case 0x0c09:
			m_asic.dma_addr[2] = (m_asic.ram[0x2c09] << 8) + m_asic.ram[0x2c08];
			m_asic.dma_status &= ~0x04;
			logerror("ASIC: DMA 2 address set to &%04x\n",m_asic.dma_addr[2]);
			break;
		case 0x0c02:
			m_asic.dma_prescaler[0] = data + 1;
			logerror("ASIC: DMA 0 pause prescaler set to %i\n",data);
			break;
		case 0x0c06:
			m_asic.dma_prescaler[1] = data + 1;
			logerror("ASIC: DMA 1 pause prescaler set to %i\n",data);
			break;
		case 0x0c0a:
			m_asic.dma_prescaler[2] = data + 1;
			logerror("ASIC: DMA 2 pause prescaler set to %i\n",data);
			break;
		case 0x0c0f:
			m_asic.dma_status = data;
			logerror("ASIC: DMA status write - %02x\n",data);
			if(data & 0x40)
			{
				logerror("ASIC: DMA 0 IRQ acknowledge\n");
				m_maincpu->set_input_line(0, CLEAR_LINE);
				m_plus_irq_cause = 0x06;
				m_asic.ram[0x2c0f] &= ~0x40;
			}
			if(data & 0x20)
			{
				logerror("ASIC: DMA 1 IRQ acknowledge\n");
				m_maincpu->set_input_line(0, CLEAR_LINE);
				m_plus_irq_cause = 0x06;
				m_asic.ram[0x2c0f] &= ~0x20;
			}
			if(data & 0x10)
			{
				logerror("ASIC: DMA 2 IRQ acknowledge\n");
				m_maincpu->set_input_line(0, CLEAR_LINE);
				m_plus_irq_cause = 0x06;
				m_asic.ram[0x2c0f] &= ~0x10;
			}
			m_asic.ram[0x2c0f] = (m_asic.ram[0x2c0f] & 0xf8) | (data & 0x07);
			break;
		}
	}
	else
	{
		UINT8* RAM = (UINT8*)m_bank12->base();
		RAM[offset] = data;
	}
}


READ8_MEMBER(amstrad_state::amstrad_plus_asic_4000_r)
{
//  logerror("RAM: read from &%04x\n",offset+0x4000);
	if ( m_asic.enabled && ( m_asic.rmr2 & 0x18 ) == 0x18 )
		return m_asic.ram[offset];
	else
	{
		UINT8* RAM = (UINT8*)m_bank3->base();
		return RAM[offset];
	}
}


READ8_MEMBER(amstrad_state::amstrad_plus_asic_6000_r)
{
//  logerror("RAM: read from &%04x\n",offset+0x6000);
	if ( m_asic.enabled && ( m_asic.rmr2 & 0x18 ) == 0x18 )
	{
		// Analogue ports
		if(offset >= 0x0808 && offset < 0x080c)
		{
			return (m_io_analog[offset & 3]->read() & 0x3f);
		}
		if(offset == 0x080c || offset == 0x080e)
		{
			return 0x3f;
		}
		if(offset == 0x080d || offset == 0x080f)
		{
			return 0x00;
		}
	#if 0
		if(offset == 0x0c0f)  // DMA status and control
		{
			int result = 0;
			if(m_plus_irq_cause == 0x00)
				result |= 0x40;
			if(m_plus_irq_cause == 0x02)
				result |= 0x20;
			if(m_plus_irq_cause == 0x04)
				result |= 0x10;
			if(m_plus_irq_cause == 0x06)
				result |= 0x80;
			return result;
		}
	#endif
		return m_asic.ram[offset+0x2000];
	}
	else
	{
		UINT8* RAM = (UINT8*)m_bank4->base();
		return RAM[offset];
	}
}


/* used for loading snapshot only ! */
void amstrad_state::AmstradCPC_PALWrite(int data)
{
	if ((data & 0x0c0)==0x0c0)
	{
		m_GateArray_RamConfiguration = data;
		AmstradCPC_GA_SetRamConfiguration();
	}
}


/* -------------------
   -  the Gate Array -
   -------------------
The gate array is controlled by I/O. The recommended I/O port address is &7Fxx.
The gate array is selected when bit 15 of the I/O port address is set to "0" and bit 14 of the I/O port address is set to "1".
The values of the other bits are ignored.
However, to avoid conflict with other devices in the system, these bits should be set to "1".

The function to be performed is selected by writing data to the Gate-Array, bit 7 and 6 of the data define the function selected (see table below).
It is not possible to read from the Gate-Array.

Bit 7 Bit 6 Function
0     0     Select pen
0     1     Select colour for selected pen
1     0     Select screen mode, rom configuration and interrupt control
1     1     Ram Memory Management (note 1)

Note 1 : This function is not available in the Gate-Array, but is performed by a device at the same I/O port address location. In the CPC464,CPC664 and KC compact, this function is performed in a memory-expansion (e.g. Dk'Tronics 64K Ram Expansion), if this expansion is not present then the function is not available. In the CPC6128, this function is performed by a PAL located on the main PCB, or a memory-expansion. In the 464+ and 6128+ this function is performed by the ASIC or a memory expansion. Please read the document on Ram Management for more information.*/


void amstrad_state::amstrad_GateArray_write(UINT8 dataToGateArray)
{
/* Get Bit 7 and 6 of the dataToGateArray = Gate Array function selected */
	switch ((dataToGateArray & 0xc0)>>6)
	{
/* Pen selection
   -------------
Bit Value Function        Bit Value Function
5   x     not used        5   x     not used
4   1     Select border   4   0     Select pen
3   x     | ignored       3   x     | Pen Number
2   x     |               2   x       |
1   x     |               1   x     |
0   x     |               0   x     |
*/
	case 0x00:
		/* Select Border Number, get b4 */
		/* if b4 = 0 : Select Pen Number, get b3-b0 */
		m_gate_array.pen_selected = ( dataToGateArray & 0x10 ) ? 0x10 : ( dataToGateArray & 0x0f );
		break;

/* Colour selection
   ----------------
Even though there is provision for 32 colours, only 27 are possible.
The remaining colours are duplicates of those already in the colour palette.

Bit Value Function
5   x     not used
4   x     | Colour number
3   x     |
2   x     |
1   x     |
0   x     |*/
	case 0x01:
		amstrad_vh_update_colour( m_gate_array.pen_selected, (dataToGateArray & 0x1F));
		break;

/* Select screen mode and rom configuration
   ----------------------------------------
Bit Value Function
5   x     not used
4   x     Interrupt generation control
3   1     Upper rom area disable or 0 Upper rom area enable
2   1     Lower rom area disable or 0 Lower rom area enable
1   x     | Mode selection
0   x     |

Screen mode selection : The settings for bits 1 and 0 and the corresponding screen mode are given in the table below.
-----------------------
b1 b0 Screen mode
0  0  Mode 0, 160x200 resolution, 16 colours
0  1  Mode 1, 320x200 resolution, 4 colours
1  0  Mode 2, 640x200 resolution, 2 colours
1  1  Mode 3, 160x200 resolution, 4 colours (note 1)

This mode is not official. From the combinations possible, we can see that 4 modes can be defined, although the Amstrad only has 3. Mode 3 is similar to mode 0, because it has the same resolution, but it is limited to only 4 colours.
Mode changing is synchronised with HSYNC. If the mode is changed, it will take effect from the next HSYNC.

Rom configuration selection :
-----------------------------
Bit 2 is used to enable or disable the lower rom area. The lower rom area occupies memory addressess &0000-&3fff and is used to access the operating system rom. When the lower rom area is is enabled, reading from &0000-&3FFF will return data in the rom. When a value is written to &0000-&3FFF, it will be written to the ram underneath the rom. When it is disabled, data read from &0000-&3FFF will return the data in the ram.
Similarly, bit 3 controls enabling or disabling of the upper rom area. The upper rom area occupies memory addressess &C000-&FFFF and is BASIC or any expansion roms which may be plugged into a rom board/box. See the document on upper rom selection for more details. When the upper rom area enabled, reading from &c000-&ffff, will return data in the rom. When data is written to &c000-&FFFF, it will be written to the ram at the same address as the rom. When the upper rom area is disabled, and data is read from &c000-&ffff the data returned will be the data in the ram.

Bit 4 controls the interrupt generation. It can be used to delay interrupts.*/
	case 0x02:
		/* If bit 5 is enabled on a CPC Plus/GX4000 when the ASIC is unlocked, sets the lower ROM position and cart bank
		   b5 = 1, b4b3 = RAM position for lower ROM area and if the ASIC registers are visible at &4000,
		   b2b1b0 = cartridge bank to read from lower ROM area (0-7 only) */
		if ( m_system_type == SYSTEM_PLUS || m_system_type == SYSTEM_GX4000 )
		{
			if ( m_asic.enabled && (dataToGateArray & 0x20) )
			{
				m_asic.rmr2 = dataToGateArray;
			}
			else
			{
				m_gate_array.mrer = dataToGateArray;
			}
		}
		else
		{
			m_gate_array.mrer = dataToGateArray;
		}

		/* If bit 4 of the "Select screen mode and rom configuration" register of the Gate-Array is set to "1"
		 then the interrupt request is cleared and the 6-bit counter is reset to "0".  */
		if ( m_gate_array.mrer & 0x10 )
		{
			m_gate_array.hsync_counter = 0;
			m_maincpu->set_input_line(0, CLEAR_LINE);
		}

		/* b3b2 != 0 then change the state of upper or lower rom area and rethink memory */
		if (m_exp)
		{
			if((dataToGateArray & 0x0c) != 0x0c)
				m_exp->romen_w(0);  // active low
			else
				m_exp->romen_w(1);
		}
		amstrad_setLowerRom();
		amstrad_setUpperRom();

		/* b1b0 mode */
		amstrad_vh_update_mode();

		break;

/* Ram Memory Management
     ---------------------
This function is not available in the Gate-Array, but is performed by a device at the same I/O port address location.
In the CPC464,CPC664 and KC compact, this function is performed in a memory-expansion (e.g. Dk'Tronics 64K Ram Expansion), if this expansion is not present then the function is not available.
In the CPC6128, this function is performed by a PAL located on the main PCB, or a memory-expansion.
In the 464+ and 6128+ this function is performed by the ASIC or a memory expansion.
*/
	case 0x03:
		m_GateArray_RamConfiguration = dataToGateArray;
		break;

	default:
		break;
	}
}


WRITE8_MEMBER(amstrad_state::aleste_msx_mapper)
{
	int page = (offset & 0x0300) >> 8;
	int ramptr = (data & 0x3f) * 0x4000;
	int rampage = data & 0x3f;
	int function = (data & 0xc0) >> 6;
	UINT8 *ram = m_ram->pointer();

	// It is assumed that functions are all mapped to each port &7cff-&7fff, and b8 and b9 are only used for RAM bank location
	switch(function)
	{
	case 0:  // Pen select (same as Gate Array?)
		amstrad_GateArray_write(data);
		break;
	case 1:  // Colour select (6-bit palette)
		aleste_vh_update_colour( m_gate_array.pen_selected, data & 0x3f );
		break;
	case 2:  // Screen mode, Upper/Lower ROM select
		amstrad_GateArray_write(data);
		break;
	case 3: // RAM banks
		switch(page)
		{
		case 0:  /* 0x0000 - 0x3fff */
			m_bank1->set_base(ram+ramptr);
			m_bank2->set_base(ram+ramptr+0x2000);
			m_bank9->set_base(ram+ramptr);
			m_bank10->set_base(ram+ramptr+0x2000);
			m_Aleste_RamBanks[0] = ram+ramptr;
			m_aleste_active_page[0] = data;
			logerror("RAM: RAM location 0x%06x (page %02x) mapped to 0x0000\n",ramptr,rampage);
			break;
		case 1:  /* 0x4000 - 0x7fff */
			m_bank3->set_base(ram+ramptr);
			m_bank4->set_base(ram+ramptr+0x2000);
			m_bank11->set_base(ram+ramptr);
			m_bank12->set_base(ram+ramptr+0x2000);
			m_Aleste_RamBanks[1] = ram+ramptr;
			m_aleste_active_page[1] = data;
			logerror("RAM: RAM location 0x%06x (page %02x) mapped to 0x4000\n",ramptr,rampage);
			break;
		case 2:  /* 0x8000 - 0xbfff */
			m_bank5->set_base(ram+ramptr);
			m_bank6->set_base(ram+ramptr+0x2000);
			m_bank13->set_base(ram+ramptr);
			m_bank14->set_base(ram+ramptr+0x2000);
			m_Aleste_RamBanks[2] = ram+ramptr;
			m_aleste_active_page[2] = data;
			logerror("RAM: RAM location 0x%06x (page %02x) mapped to 0x8000\n",ramptr,rampage);
			break;
		case 3:  /* 0xc000 - 0xffff */
			m_bank7->set_base(ram+ramptr);
			m_bank8->set_base(ram+ramptr+0x2000);
			m_bank15->set_base(ram+ramptr);
			m_bank16->set_base(ram+ramptr+0x2000);
			m_Aleste_RamBanks[3] = ram+ramptr;
			m_aleste_active_page[3] = data;
			logerror("RAM: RAM location 0x%06x (page %02x) mapped to 0xc000\n",ramptr,rampage);
			break;
		}
		break;
	}
}


/* CRTC Differences
   ----------------
The following tables list the functions that can be accessed for each type:

Type 0
b1 b0 Function Read/Write
0  0  Select internal 6845 register Write Only
0  1  Write to selected internal 6845 register Write Only
1  0  - -
1  1  Read from selected internal 6845 register Read only

Type 1
b1 b0 Function Read/Write
0  0  Select internal 6845 register Write Only
0  1  Write to selected internal 6845 register Write Only
1  0  Read Status Register Read Only
1  1  Read from selected internal 6845 register Read only

Type 2
b1 b0 Function Read/Write
0  0  Select internal 6845 register Write Only
0  1  Write to selected internal 6845 register Write Only
1  0  - -
1  1  Read from selected internal 6845 register Read only

Type 3 and 4
b1 b0 Function Read/Write
0  0  Select internal 6845 register Write Only
0  1  Write to selected internal 6845 register Write Only
1  0  Read from selected internal 6845 register Read Only
1  1  Read from selected internal 6845 register Read only
*/

/* I/O port allocation
   -------------------

Many thanks to Mark Rison for providing the original information. Thankyou to Richard Wilson for his discoveries concerning RAM management I/O decoding.

This document will explain the decoding of the I/O ports. The port address is not decoded fully which means a hardware device can be accessed through more than one address, in addition, using some addressess can access more than one element of the hardware at the same time. The CPC IN/OUT design differs from the norm in that port numbers are defined using 16 bits, as opposed to the traditional 8 bits.

IN r,(C)/OUT (C),r instructions: Bits b15-b8 come from the B register, bits b7-b0 come from "r"
IN A,(n)/OUT (n),A instructions: Bits b15-b8 come from the A register, bits b7-b0 come from "n"
Listed below are the internal hardware devices and the bit fields to which they respond. In the table:

"-" means this bit is ignored,
"0" means the bit must be set to "0" for the hardware device to respond,
"1" means the bit must be set to "1" for the hardware device to respond.
"r1" and "r0" mean a bit used to define a register

Hardware device       Read/Write Port bits
                                 b15 b14 b13 b12 b11 b10 b9  b8  b7  b6  b5  b4  b3  b2  b1  b0
Gate-Array            Write Only 0   1   -   -   -   -   -   -   -   -   -   -   -   -   -   -
RAM Configuration     Write Only 0   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -
CRTC                  Read/Write -   0   -   -   -   -   r1  r0  -   -   -   -   -   -   -   -
ROM select            Write only -   -   0   -   -   -   -   -   -   -   -   -   -   -   -   -
Printer port          Write only -   -   -   0   -   -   -   -   -   -   -   -   -   -   -   -
8255 PPI              Read/Write -   -   -   -   0   -   r1  r0  -   -   -   -   -   -   -   -
Expansion Peripherals Read/Write -   -   -   -   -   0   -   -   -   -   -   -   -   -   -   -

*/

READ8_MEMBER(amstrad_state::amstrad_cpc_io_r)
{
	unsigned char data = 0xFF;
	unsigned int r1r0 = (unsigned int)((offset & 0x0300) >> 8);
//  m6845_personality_t crtc_type;
	int page;

//  crtc_type = read_safe(ioport("crtc"), 0);
//  m6845_set_personality(crtc_type);

	if(m_aleste_mode & 0x04)
	{
		if ((offset & (1<<15)) == 0)  // Aleste Mapper is readable?
		{
			page = (offset & 0x0300) >> 8;
			data = m_aleste_active_page[page];
			return data;
		}
	}

	if ( m_system_type == SYSTEM_PLUS || m_system_type == SYSTEM_GX4000 )  // Plus systems return 0x78 (464+) or 0x79 (6128+) when attempting to read the gate array (and any other unreadable space too?)
		data = 0x79;

	/* if b14 = 0 : CRTC Read selected */
	if ((offset & (1<<14)) == 0)
	{
		switch(r1r0)
		{
		case 0x02:
			// CRTC Type 1 (UM6845R) only!!
			//data = m_crtc->status_r( space, 0 );
			if ( m_system_type == SYSTEM_PLUS || m_system_type == SYSTEM_GX4000 )  // All Plus systems are Type 3 (AMS40489)
				data = m_crtc->register_r( space, 0 );
			else
				data = 0xff;  // Type 0 (HD6845S/UM6845) and Type 2 (MC6845) return 0xff
#if 0
			/* CRTC Type 1 : Read Status Register
			   CRTC Type 3 or 4 : Read from selected internal 6845 register */
			switch(crtc_type) {
			case M6845_PERSONALITY_UM6845R:
				data = amstrad_CRTC_CR; /* Read Status Register */
				break;
			case M6845_PERSONALITY_AMS40489:
			case M6845_PERSONALITY_PREASIC:
				data = m6845_register_r(0);
				break;
			default:
				break;
			}
#endif
			break;
		case 0x03:
			/* All CRTC type : Read from selected internal 6845 register Read only */
			data = m_crtc->register_r( space, 0 );
			break;
		}
	}

/* if b11 = 0 : 8255 PPI Read selected - bits 9 and 8 then define the PPI function access as shown below:

b9 b8 | PPI Function Read/Write status
0  0  | Port A data  Read/Write
0  1  | Port B data  Read/Write
1  0  | Port C data  Read/Write
1  1  | Control      Write Only
*/
	if ((offset & (1<<11)) == 0)
	{
		if (r1r0 < 0x03 )
			data = m_ppi->read(space, r1r0);
		if ( m_system_type == SYSTEM_PLUS || m_system_type == SYSTEM_GX4000 )  // Plus systems return the data written to port C (I/O status is ignored)
			if(r1r0 == 0x02)
				data = m_last_write;
	}

/* if b10 = 0 : Expansion Peripherals Read selected

bits b7-b5 are used to select an expansion peripheral. Again, this is done by resetting the peripheral's bit:
Bit | Device
b7  | FDC
b6  | Reserved (was it ever used?)
b5  | Serial port

In the case of the FDC, bits b8 and b0 are used to select the specific mode of operation; all the other bits (b9,b4-b1) are ignored:
b8 b0 Function Read/Write state
0 0 FDD motor control Write Only
0 1 not used N/A
1 0 Main Status register of FDC (MSR) Read Only
1 1 Data register of FDC Read/Write

If b10 is reset but none of b7-b5 are reset, user expansion peripherals are selected.
The exception is the case where none of b7-b0 are reset (i.e. port &FBFF), which causes the expansion peripherals to reset.
 */
	if ( m_system_type != SYSTEM_GX4000 )
	{
		if(m_fdc)  // if FDC is present (it isn't on a 464)
		{
			if ( ( offset & (1<<10) ) == 0 )
			{
				if ( ( offset & (1<<10) ) == 0 )
				{
					int b8b0 = ( ( offset & (1<<8) ) >> (8 - 1) ) | ( offset & 0x01 );

					switch (b8b0)
					{
					case 0x02:
						data = m_fdc->msr_r(space, 0);
						break;
					case 0x03:
						data = m_fdc->fifo_r(space, 0);
						break;
					default:
						break;
					}
				}
			}
		}
	}
	return data;
}


// Handler for checking the ASIC unlocking sequence
void amstrad_state::amstrad_plus_seqcheck(int data)
{
	if(data == 0 && m_prev_data != 0)
	{
		m_asic.seqptr = 0;  // non-zero value followed by zero will sync the locking mechanism
	}
	if(data == asic_unlock_seq[m_asic.seqptr])
	{
		m_asic.seqptr++;
		if ( m_asic.seqptr == 14 && m_asic.enabled )
		{
			logerror("SYS: ASIC locked\n");
			m_asic.enabled = 0;
		}
		if ( m_asic.seqptr >= 15 )  // end of sequence
		{
			logerror("SYS: ASIC unlocked\n");
			m_asic.enabled = 1;
		}
	}
	m_prev_data = data;
}

WRITE8_MEMBER(amstrad_state::rom_select)
{
	m_gate_array.upper_bank = data;
	// expansion devices know the selected ROM by monitoring I/O writes to DFxx
	// there are no signals related to which ROM is selected
	cpc_expansion_slot_device* exp_port = m_exp;
	while(exp_port != NULL)
	{
		device_cpc_expansion_card_interface* temp;
		device_t* temp_dev;

		temp = dynamic_cast<device_cpc_expansion_card_interface*>(exp_port->get_card_device());
		temp_dev = dynamic_cast<device_t*>(exp_port->get_card_device());
		if(temp != NULL)
		{
			temp->set_rom_bank(data);
		}
		exp_port = temp_dev->subdevice<cpc_expansion_slot_device>("exp");
	}

	amstrad_rethinkMemory();
}

/* Offset handler for write */
WRITE8_MEMBER(amstrad_state::amstrad_cpc_io_w)
{
	cpc_multiface2_device* mface2;

	if ((offset & (1<<15)) == 0)
	{
		if(m_aleste_mode & 0x04) // Aleste mode
		{
			aleste_msx_mapper(space, offset, data);
		}
		else
		{
			/* if b15 = 0 and b14 = 1 : Gate-Array Write Selected*/
			if ((offset & (1<<14)) != 0)
				amstrad_GateArray_write(data);

			/* if b15 = 0 : RAM Configuration Write Selected*/
			AmstradCPC_GA_SetRamConfiguration();
		}
	}

	/*The Gate-Array and CRTC can't be selected simultaneously, which would otherwise cause potential display corruption.*/
	/* if b14 = 0 : CRTC Write Selected*/
	if ((offset & (1<<14)) == 0)
	{
		switch ((offset & 0x0300) >> 8) // r1r0
		{
		case 0x00:      /* Select internal 6845 register Write Only */
			m_crtc->address_w( space, 0, data );
			if ( m_system_type == SYSTEM_PLUS || m_system_type == SYSTEM_GX4000 )
				amstrad_plus_seqcheck(data);

			/* printer port d7 */
			if (data == 0x0c && m_system_type == SYSTEM_PLUS)
				m_printer_bit8_selected = TRUE;

			m_asic.addr_6845 = data;
			break;
		case 0x01:      /* Write to selected internal 6845 register Write Only */
			if ( m_system_type == SYSTEM_PLUS || m_system_type == SYSTEM_GX4000 )
				timer_set(attotime::from_usec(0), TIMER_VIDEO_UPDATE, 1);
			else
				timer_set(attotime::from_usec(0), TIMER_VIDEO_UPDATE, 0);
			m_crtc->register_w( space, 0, data );

			/* printer port bit 8 */
			if (m_printer_bit8_selected && m_system_type == SYSTEM_PLUS)
			{
				m_centronics->write_data7(BIT(data, 3));
				m_printer_bit8_selected = FALSE;
			}

			if ( m_asic.addr_6845 == 0x01 )
				m_asic.horiz_disp = data;

			break;
		default:
			break;
		}
	}

	/* ROM select before GateArrayWrite ?*/
	/* b13 = 0 : ROM select Write Selected*/
	if ((offset & (1<<13)) == 0)
	{
		rom_select(space,0,data);
	}

	/* b12 = 0 : Printer port Write Selected*/
	if(m_system_type != SYSTEM_GX4000)
	{
		if ((offset & (1<<12)) == 0)
		{
			/* CPC has a 7-bit data port, bit 8 is the STROBE signal */
			m_centronics->write_data0(BIT(data, 0));
			m_centronics->write_data1(BIT(data, 1));
			m_centronics->write_data2(BIT(data, 2));
			m_centronics->write_data3(BIT(data, 3));
			m_centronics->write_data4(BIT(data, 4));
			m_centronics->write_data5(BIT(data, 5));
			m_centronics->write_data6(BIT(data, 6));
			m_centronics->write_strobe(BIT(data, 7));
		}
	}

	/* if b11 = 0 : 8255 PPI Write selected - bits 9 and 8 then define the PPI function access as shown below:
	b9 b8 | PPI Function Read/Write status
	0  0  | Port A data  Read/Write
	0  1  | Port B data  Read/Write
	1  0  | Port C data  Read/Write
	1  1  | Control      Write Only
	*/
	if ((offset & (1<<11)) == 0)
	{
		unsigned int idx = ((offset & 0x0300) >> 8);

		m_ppi->write(space, idx, data);
		if(idx == 0x02)
			m_last_write = data;
	}

	/* if b10 = 0 : Expansion Peripherals Write selected */
	if ((offset & (1<<10)) == 0)
	{
/* bits b7-b5 are used to select an expansion peripheral. This is done by resetting the peripheral's bit:
Bit | Device
b7  | FDC
b6  | Reserved (was it ever used?)
b5  | Serial port

In the case of the FDC, bits b8 and b0 are used to select the specific mode of operation;
all the other bits (b9,b4-b1) are ignored:
b8 b0 Function Read/Write state
0 0 FDD motor control Write Only
0 1 not used N/A
1 0 Main Status register of FDC (MSR) Read Only
1 1 Data register of FDC Read/Write

If b10 is reset but none of b7-b5 are reset, user expansion peripherals are selected.
The exception is the case where none of b7-b0 are reset (i.e. port &FBFF), which causes the expansion peripherals to reset.
*/
		if(m_system_type != SYSTEM_GX4000)
		{
			if(m_fdc)  // if FDC is present (it isn't on a 464)
			{
				if ((offset & (1<<7)) == 0)
				{
					unsigned int b8b0 = ((offset & 0x0100) >> (8 - 1)) | (offset & 0x01);

					switch (b8b0)
					{
					case 0x00:
					case 0x01:
						{
							/* FDC Motor Control - Bit 0 defines the state of the FDD motor:
							 * "1" the FDD motor will be active.
							 * "0" the FDD motor will be in-active.*/
							floppy_image_device *floppy;
							floppy = machine().device<floppy_connector>(":upd765:0")->get_device();
							if(floppy)
								floppy->mon_w(!BIT(data, 0));
							floppy = machine().device<floppy_connector>(":upd765:1")->get_device();
							if(floppy)
								floppy->mon_w(!BIT(data, 0));
							break;
						}

					case 0x03: /* Write Data register of FDC */
						m_fdc->fifo_w(space, 0,data);
						break;

					default:
						break;
					}
				}
			}
		}
	}

	/*  Aleste Extend Port:
	    D0 - VRAM Bank 0/1 (64K)
	    D1 - MODE 0-Norm ,1-High resolution
	    D2 - MAPMOD 0-Amstrad ,1-Yamaha mapper
	    D3 - MAP Page 0/1 (256K)
	    D4 - 580WI53
	    D5 - 0-AY8910 ,1-512WI1 */
	if(offset == 0xfabf)
	{
		m_aleste_mode = data;
		logerror("EXTEND: Port &FABF write 0x%02x\n",data);
		m_crtc->set_clock( ( m_aleste_mode & 0x02 ) ? ( XTAL_16MHz / 8 ) : ( XTAL_16MHz / 16 ) );
	}

	mface2 = dynamic_cast<cpc_multiface2_device*>(get_expansion_device(machine(),"multiface2"));
	if(mface2 != NULL)
	{
		if(mface2->multiface_io_write(offset, data) != 0)
		{
			amstrad_rethinkMemory();
		}
	}
}


/* load CPCEMU style snapshots */
void amstrad_state::amstrad_handle_snapshot(unsigned char *pSnapshot)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int RegData;
	int i;

	/* init Z80 */
	RegData = (pSnapshot[0x011] & 0x0ff) | ((pSnapshot[0x012] & 0x0ff)<<8);
	m_maincpu->set_state_int(Z80_AF, RegData);

	RegData = (pSnapshot[0x013] & 0x0ff) | ((pSnapshot[0x014] & 0x0ff)<<8);
	m_maincpu->set_state_int(Z80_BC, RegData);

	RegData = (pSnapshot[0x015] & 0x0ff) | ((pSnapshot[0x016] & 0x0ff)<<8);
	m_maincpu->set_state_int(Z80_DE, RegData);

	RegData = (pSnapshot[0x017] & 0x0ff) | ((pSnapshot[0x018] & 0x0ff)<<8);
	m_maincpu->set_state_int(Z80_HL, RegData);

	RegData = (pSnapshot[0x019] & 0x0ff) ;
	m_maincpu->set_state_int(Z80_R, RegData);

	RegData = (pSnapshot[0x01a] & 0x0ff);
	m_maincpu->set_state_int(Z80_I, RegData);

	if ((pSnapshot[0x01b] & 1)==1)
	{
		m_maincpu->set_state_int(Z80_IFF1, (UINT64)1);
	}
	else
	{
		m_maincpu->set_state_int(Z80_IFF1, (UINT64)0);
	}

	if ((pSnapshot[0x01c] & 1)==1)
	{
		m_maincpu->set_state_int(Z80_IFF2, (UINT64)1);
	}
	else
	{
		m_maincpu->set_state_int(Z80_IFF2, (UINT64)0);
	}

	RegData = (pSnapshot[0x01d] & 0x0ff) | ((pSnapshot[0x01e] & 0x0ff)<<8);
	m_maincpu->set_state_int(Z80_IX, RegData);

	RegData = (pSnapshot[0x01f] & 0x0ff) | ((pSnapshot[0x020] & 0x0ff)<<8);
	m_maincpu->set_state_int(Z80_IY, RegData);

	RegData = (pSnapshot[0x021] & 0x0ff) | ((pSnapshot[0x022] & 0x0ff)<<8);
	m_maincpu->set_state_int(Z80_SP, RegData);
	m_maincpu->set_state_int(STATE_GENSP, RegData);

	RegData = (pSnapshot[0x023] & 0x0ff) | ((pSnapshot[0x024] & 0x0ff)<<8);

	m_maincpu->set_state_int(Z80_PC, RegData);
//  m_maincpu->set_state_int(REG_SP, RegData);

	RegData = (pSnapshot[0x025] & 0x0ff);
	m_maincpu->set_state_int(Z80_IM, RegData);

	RegData = (pSnapshot[0x026] & 0x0ff) | ((pSnapshot[0x027] & 0x0ff)<<8);
	m_maincpu->set_state_int(Z80_AF2, RegData);

	RegData = (pSnapshot[0x028] & 0x0ff) | ((pSnapshot[0x029] & 0x0ff)<<8);
	m_maincpu->set_state_int(Z80_BC2, RegData);

	RegData = (pSnapshot[0x02a] & 0x0ff) | ((pSnapshot[0x02b] & 0x0ff)<<8);
	m_maincpu->set_state_int(Z80_DE2, RegData);

	RegData = (pSnapshot[0x02c] & 0x0ff) | ((pSnapshot[0x02d] & 0x0ff)<<8);
	m_maincpu->set_state_int(Z80_HL2, RegData);

	/* init GA */
	for (i=0; i<17; i++)
	{
		amstrad_GateArray_write(i);

		amstrad_GateArray_write(((pSnapshot[0x02f + i] & 0x01f) | 0x040));
	}

	amstrad_GateArray_write(pSnapshot[0x02e] & 0x01f);

	amstrad_GateArray_write(((pSnapshot[0x040] & 0x03f) | 0x080));

	AmstradCPC_PALWrite(((pSnapshot[0x041] & 0x03f) | 0x0c0));

	/* init CRTC */
	for (i=0; i<18; i++)
	{
		m_crtc->address_w( space, 0, i );
		m_crtc->register_w( space, 0, pSnapshot[0x043+i] & 0xff );
	}

	m_crtc->address_w( space, 0, i );

	/* upper rom selection */
	m_gate_array.upper_bank = pSnapshot[0x055];

	/* PPI */
	m_ppi->write(space, 3, pSnapshot[0x059] & 0x0ff);

	m_ppi->write(space, 0, pSnapshot[0x056] & 0x0ff);
	m_ppi->write(space, 1, pSnapshot[0x057] & 0x0ff);
	m_ppi->write(space, 2, pSnapshot[0x058] & 0x0ff);

	/* PSG */
	for (i=0; i<16; i++)
	{
		m_ay->address_w(space, 0, i);
		m_ay->data_w(space, 0, pSnapshot[0x05b + i] & 0x0ff);
	}

	m_ay->address_w(space, 0, pSnapshot[0x05a]);

	{
		int MemSize;
		int MemorySize;

		MemSize = (pSnapshot[0x06b] & 0x0ff) | ((pSnapshot[0x06c] & 0x0ff)<<8);

		if (MemSize==128)
		{
			MemorySize = 128*1024;
		}
		else
		{
			MemorySize = 64*1024;
		}

		memcpy(m_ram->pointer(), &pSnapshot[0x0100], MemorySize);
	}
	amstrad_rethinkMemory();
}


/* sets up for a machine reset
All hardware is reset and the firmware is completely initialized
Once all tables and jumpblocks have been set up,
control is passed to the default entry in rom 0*/
void amstrad_state::amstrad_reset_machine()
{
	m_last_write = 0xff;
	
	/* enable lower rom (OS rom) */
	amstrad_GateArray_write(0x089);

	/* set ram config 0 */
	amstrad_GateArray_write(0x0c0);

	// Get manufacturer name and TV refresh rate from PCB link (dipswitch for mess emulation)
	m_ppi_port_inputs[amstrad_ppi_PortB] = (((m_io_solder_links->read()&MANUFACTURER_NAME)<<1) | (m_io_solder_links->read()&TV_REFRESH_RATE));

	if ( m_system_type == SYSTEM_PLUS || m_system_type == SYSTEM_GX4000 )
	{
		memset(m_asic.ram,0,16384);  // clear ASIC RAM
	}
}


/*------------------
  - Rethink Memory -
  ------------------*/
void amstrad_state::amstrad_rethinkMemory()
{
	cpc_multiface2_device* mface2;
	/* the following is used for banked memory read/writes and for setting up
	 * opcode and opcode argument reads */

	/* bank 0 - 0x0000..0x03fff */
	amstrad_setLowerRom();

	/* bank 1 - 0x04000..0x07fff */
	if ( m_system_type == SYSTEM_CPC || m_system_type == SYSTEM_ALESTE || m_asic.enabled == 0 )
	{
		if(m_aleste_mode & 0x04)
		{
			m_bank3->set_base(m_Aleste_RamBanks[1]);
			m_bank4->set_base(m_Aleste_RamBanks[1]+0x2000);
			/* bank 2 - 0x08000..0x0bfff */
			m_bank5->set_base(m_Aleste_RamBanks[2]);
			m_bank6->set_base(m_Aleste_RamBanks[2]+0x2000);
		}
		else
		{
			m_bank3->set_base(m_AmstradCPC_RamBanks[1]);
			m_bank4->set_base(m_AmstradCPC_RamBanks[1]+0x2000);
			/* bank 2 - 0x08000..0x0bfff */
			m_bank5->set_base(m_AmstradCPC_RamBanks[2]);
			m_bank6->set_base(m_AmstradCPC_RamBanks[2]+0x2000);
		}
	}
	else
	{
		amstrad_setLowerRom();
	}

	/* bank 3 - 0x0c000..0x0ffff */
	amstrad_setUpperRom();

	/* other banks */
	if(m_aleste_mode & 0x04)
	{
		m_bank9->set_base(m_Aleste_RamBanks[0]);
		m_bank10->set_base(m_Aleste_RamBanks[0]+0x2000);
		m_bank11->set_base(m_Aleste_RamBanks[1]);
		m_bank12->set_base(m_Aleste_RamBanks[1]+0x2000);
		m_bank13->set_base(m_Aleste_RamBanks[2]);
		m_bank14->set_base(m_Aleste_RamBanks[2]+0x2000);
		m_bank15->set_base(m_Aleste_RamBanks[3]);
		m_bank16->set_base(m_Aleste_RamBanks[3]+0x2000);
	}
	else
	{
		m_bank9->set_base(m_AmstradCPC_RamBanks[0]);
		m_bank10->set_base(m_AmstradCPC_RamBanks[0]+0x2000);
		m_bank11->set_base(m_AmstradCPC_RamBanks[1]);
		m_bank12->set_base(m_AmstradCPC_RamBanks[1]+0x2000);
		m_bank13->set_base(m_AmstradCPC_RamBanks[2]);
		m_bank14->set_base(m_AmstradCPC_RamBanks[2]+0x2000);
		m_bank15->set_base(m_AmstradCPC_RamBanks[3]);
		m_bank16->set_base(m_AmstradCPC_RamBanks[3]+0x2000);
	}

	/* multiface hardware enabled? */
	mface2 = dynamic_cast<cpc_multiface2_device*>(get_expansion_device(machine(),"multiface2"));
	if(mface2 != NULL)
	{
		if (mface2->multiface_hardware_enabled())
		{
			if((m_gate_array.mrer & 0x04) == 0)
			{
				mface2->multiface_rethink_memory();
			}
		}
	}

	/* mappings for other expansion devices */
	if (m_exp)
		m_exp->set_mapping(MAP_OTHER);
}


void amstrad_state::kccomp_reset_machine()
{
	/* enable lower rom (OS rom) */
	amstrad_GateArray_write(0x089);

	/* set ram config 0 */
	amstrad_GateArray_write(0x0c0);
}


void amstrad_state::screen_eof_amstrad(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		cpc_multiface2_device* mface2 = dynamic_cast<cpc_multiface2_device*>(get_expansion_device(machine(),"multiface2"));

		if(mface2 != NULL)
		{
			mface2->check_button_state();
		}
	}
}


/* ---------------------------------------
   - 27.05.2004 - PSG function selection -
   ---------------------------------------
The databus of the PSG is connected to PPI Port A.
Data is read from/written to the PSG through this port.

The PSG function, defined by the BC1,BC2 and BDIR signals, is controlled by bit 7 and bit 6 of PPI Port C.

PSG function selection:
-----------------------
Function

BDIR = PPI Port C Bit 7 and BC1 = PPI Port C Bit 6

PPI Port C Bit | PSG Function
BDIR BC1       |
0    0         | Inactive
0    1         | Read from selected PSG register. When function is set, the PSG will make the register data available to PPI Port A
1    0         | Write to selected PSG register. When set, the PSG will take the data at PPI Port A and write it into the selected PSG register
1    1         | Select PSG register. When set, the PSG will take the data at PPI Port A and select a register
*/
/* PSG function selected */
void amstrad_state::update_psg()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	if(m_aleste_mode & 0x20)  // RTC selected
	{
		switch(m_aleste_rtc_function)
		{
		case 0x02:  // AS
			m_rtc->write(space, 0,m_ppi_port_outputs[amstrad_ppi_PortA]);
			break;
		case 0x04:  // DS write
			m_rtc->write(space, 1,m_ppi_port_outputs[amstrad_ppi_PortA]);
			break;
		case 0x05:  // DS read
			m_ppi_port_inputs[amstrad_ppi_PortA] = m_rtc->read(space, 1);
			break;
		}
		return;
	}
	switch (m_Psg_FunctionSelected)
	{
	case 0:
		{/* Inactive */
		} break;
	case 1:
		{/* b6 = 1 ? : Read from selected PSG register and make the register data available to PPI Port A */
			m_ppi_port_inputs[amstrad_ppi_PortA] = m_ay->data_r(space, 0);
		}
		break;
	case 2:
		{/* b7 = 1 ? : Write to selected PSG register and write data to PPI Port A */
			m_ay->data_w(space, 0, m_ppi_port_outputs[amstrad_ppi_PortA]);
		}
		break;
	case 3:
		{/* b6 and b7 = 1 ? : The register will now be selected and the user can read from or write to it.  The register will remain selected until another is chosen.*/
			m_ay->address_w(space, 0, m_ppi_port_outputs[amstrad_ppi_PortA]);
			m_prev_reg = m_ppi_port_outputs[amstrad_ppi_PortA];
		}
		break;
	default:
		{
		} break;
	}
}


/* Read/Write 8255 PPI port A (connected to AY-3-8912 databus) */
READ8_MEMBER(amstrad_state::amstrad_ppi_porta_r)
{
	update_psg();
	return m_ppi_port_inputs[amstrad_ppi_PortA];
}


WRITE8_MEMBER(amstrad_state::amstrad_ppi_porta_w)
{
	m_ppi_port_outputs[amstrad_ppi_PortA] = data;
	update_psg();
}


/* - Read PPI Port B -
   -------------------
Bit Description
7   Cassette read data
6   Parallel/Printer port ready signal ("1" = not ready, "0" = Ready)
5   /EXP signal on expansion port (note 6)
4   50/60 Hz (link on PCB. For this MESS driver I have used the dipswitch feature) (note 5)
3   | PCB links to define manufacturer name. For this MESS driver I have used the dipswitch feature. (note 1) (note 4)
2   | (note 2)
1   | (note 3)
0   VSYNC State from 6845. "1" = VSYNC active, "0" = VSYNC inactive

Note:

1 On CPC464,CPC664,CPC6128 and GX4000 this is LK3 on the PCB. On the CPC464+ and CPC6128+ this is LK103 on the PCB. On the KC compact this is "1".
2 On CPC464,CPC664,CPC6128 and GX4000 this is LK2 on the PCB. On the CPC464+ and CPC6128+ this is LK102 on the PCB. On the KC compact this is "0".
3 On CPC464,CPC664,CPC6128 and GX4000 this is LK1 on the PCB. On the CPC464+ and CPC6128+ this is LK101 on the PCB. On the KC compact this is /TEST signal from the expansion port.
4 On the CPC464,CPC664,CPC6128,CPC464+,CPC6128+ and GX4000 bits 3,2 and 1 define the manufacturer name. See below to see the options available. The manufacturer name is defined on the PCB and cannot be changed through software.
5 On the CPC464,CPC664,CPC6128,CPC464+,CPC6128+ and GX4000 bit 4 defines the Screen refresh frequency. "1" = 50 Hz, "0" = 60 Hz. This is defined on the PCB and cannot be changed with software. On the KC compact bit 4 is "1"
6 This bit is connected to /EXP signal on the expansion port.
  On the KC Compact this bit is used to define bit 7 of the printer data.
  On the CPC, it is possible to use this bit to define bit 7 of the printer data, so a 8-bit printer port is made, with a hardware modification,
  On the CPC this can be used by a expansion device to report it's presence. "1" = device connected, "0" = device not connected. This is not always used by all expansion devices.
*/

WRITE_LINE_MEMBER(amstrad_state::write_centronics_busy)
{
	m_centronics_busy = state;
}

READ8_MEMBER(amstrad_state::amstrad_ppi_portb_r)
{
	int data = 0;
/* Set b7 with cassette tape input */
	if(m_system_type != SYSTEM_GX4000)
	{
		if (m_cassette->input() > 0.03)
		{
			data |= (1<<7);
		}
	}
/* Set b6 with Parallel/Printer port ready */
	if(m_system_type != SYSTEM_GX4000)
	{
		data |= m_centronics_busy << 6;
	}
/* Set b4-b1 50Hz/60Hz state and manufacturer name defined by links on PCB */
	data |= (m_ppi_port_inputs[amstrad_ppi_PortB] & 0x1e);

/*  Set b0 with VSync state from the CRTC */
	data |= m_gate_array.vsync;

	if(m_aleste_mode & 0x04)
	{
		if(m_fdc->get_irq() == 0)
			data &= ~0x02;
		else
			data |= 0x02;
	}

//logerror("amstrad_ppi_portb_r\n");
	/* Schedule a write to PC2 */
	timer_set(attotime::zero, TIMER_PC2_LOW);

	return data;
}


/* 26-May-2005 - PPI Port C
   -----------------------
Bit Description  Usage
7   PSG BDIR     | PSG function selection
6   PSG BC1      |
5                Cassette Write data
4                Cassette Motor Control set bit to "1" for motor on, or "0" for motor off
3                | Keyboard line Select keyboard line to be scanned (0-15)
2                |
1                |
0                |*/

/* previous_ppi_portc_w value */

WRITE8_MEMBER(amstrad_state::amstrad_ppi_portc_w)
{
	int changed_data;

	m_previous_ppi_portc_w = m_ppi_port_outputs[amstrad_ppi_PortC];
/* Write the data to Port C */
	changed_data = m_previous_ppi_portc_w^data;

	m_ppi_port_outputs[amstrad_ppi_PortC] = data;

/* get b7 and b6 (PSG Function Selected */
	m_Psg_FunctionSelected = ((data & 0xC0)>>6);

	/* MC146818 function */
	m_aleste_rtc_function = data & 0x07;

	/* Perform PSG function */
	update_psg();

	/* b5 Cassette Write data */
	if(m_system_type != SYSTEM_GX4000)
	{
		if ((changed_data & 0x20) != 0)
		{
			m_cassette->output(((data & 0x20) ? -1.0 : +1.0));
		}
	}

	/* b4 Cassette Motor Control */
	if(m_system_type != SYSTEM_GX4000)
	{
		if ((changed_data & 0x10) != 0)
		{
			m_cassette->change_state(
					((data & 0x10) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED),
					CASSETTE_MASK_MOTOR);
		}
	}
}


/* Additional notes for the AY-3-8912 in the CPC design
Port A of the AY-3-8912 is connected to the keyboard.
The data for a selected keyboard line can be read through Port A, as long as it is defined as input.
The operating system and I believe all programs assume this port has been defined as input. (NWC has found a bug in the Multiface 2 software. The Multiface does not reprogram the input/output state of the AY-3-8912's registers, therefore if port A is programmed as output, the keyboard will be unresponsive and it will not be possible to use the Multiface functions.)
When port B is defined as input (bit 7 of register 7 is set to "0"), a read of this port will return &FF.
*/

/* read PSG port A */
READ8_MEMBER(amstrad_state::amstrad_psg_porta_read)
{
	/* Read CPC Keyboard
	If keyboard matrix line 11-15 are selected, the byte is always &ff.
	After testing on a real CPC, it is found that these never change, they always return &FF. */

	if ( ( m_ppi_port_outputs[amstrad_ppi_PortC] & 0x0F ) > 10)
	{
		return 0xFF;
	}
	else
	{
		if(m_aleste_mode == 0x08 && ( m_ppi_port_outputs[amstrad_ppi_PortC] & 0x0F ) == 10)
			return 0xff;

		if (m_io_kbrow[m_ppi_port_outputs[amstrad_ppi_PortC] & 0x0F])
		{
			if(m_system_type != SYSTEM_GX4000)
			{
				if(m_io_ctrltype && (m_io_ctrltype->read() == 1) && (m_ppi_port_outputs[amstrad_ppi_PortC] & 0x0F) == 9)
				{
					return m_amx_mouse_data;
				}
				if(m_io_ctrltype && (m_io_ctrltype->read() == 2) && (m_ppi_port_outputs[amstrad_ppi_PortC] & 0x0F) == 9)
				{
					return (m_io_kbrow[m_ppi_port_outputs[amstrad_ppi_PortC] & 0x0F] ? m_io_kbrow[m_ppi_port_outputs[amstrad_ppi_PortC] & 0x0F]->read() & 0x80 : 0) | 0x7f;
				}
			}

			return m_io_kbrow[m_ppi_port_outputs[amstrad_ppi_PortC] & 0x0F] ? m_io_kbrow[m_ppi_port_outputs[amstrad_ppi_PortC] & 0x0F]->read() : 0;
		}
		return 0xFF;
	}
}


/* called when cpu acknowledges int */
/* reset top bit of interrupt line counter */
/* this ensures that the next interrupt is no closer than 32 lines */
IRQ_CALLBACK_MEMBER(amstrad_state::amstrad_cpu_acknowledge_int)
{
	// DMA interrupts can be automatically cleared if bit 0 of &6805 is set to 0
	if( m_asic.enabled && m_plus_irq_cause != 0x06 && m_asic.dma_clear & 0x01)
	{
		logerror("IRQ: Not cleared, IRQ was called by DMA [%i]\n",m_plus_irq_cause);
		m_asic.ram[0x2c0f] &= ~0x80;  // not a raster interrupt, so this bit is reset
		return (m_asic.ram[0x2805] & 0xf8) | m_plus_irq_cause;
	}
	m_maincpu->set_input_line(0, CLEAR_LINE);
	m_gate_array.hsync_counter &= 0x1F;
	if ( m_asic.enabled )
	{
		if(m_plus_irq_cause == 6)  // bit 7 is set "if last interrupt acknowledge cycle was caused by a raster interrupt"
			m_asic.ram[0x2c0f] |= 0x80;
		else
		{
			m_asic.ram[0x2c0f] &= ~0x80;
			m_asic.ram[0x2c0f] &= (0x40 >> m_plus_irq_cause/2);
		}
		return (m_asic.ram[0x2805] & 0xf8) | m_plus_irq_cause;
	}
	if(m_system_type != SYSTEM_GX4000)
		{
			// update AMX mouse inputs (normally done every 1/300th of a second)
			if(m_io_ctrltype && m_io_ctrltype->read() == 1)
			{
				static UINT8 prev_x,prev_y;
				UINT8 data_x, data_y;

				m_amx_mouse_data = 0x0f;
				data_x = m_io_mouse1 ? m_io_mouse1->read() : 0;
				data_y = m_io_mouse2 ? m_io_mouse2->read() : 0;

				if(data_x > prev_x)
					m_amx_mouse_data &= ~0x08;
				if(data_x < prev_x)
					m_amx_mouse_data &= ~0x04;
				if(data_y > prev_y)
					m_amx_mouse_data &= ~0x02;
				if(data_y < prev_y)
					m_amx_mouse_data &= ~0x01;
				m_amx_mouse_data |= ((m_io_mouse3 ? m_io_mouse3->read() : 0) << 4);
				prev_x = data_x;
				prev_y = data_y;

				m_amx_mouse_data |= ((m_io_kbrow[9] ? m_io_kbrow[9]->read() : 0) & 0x80);  // DEL key
			}
		}
	return 0xFF;
}


/* the following timings have been measured! */
static const UINT8 amstrad_cycle_table_op[256] = {
		4, 12,  8,  8,  4,  4,  8,  4,  4, 12,  8,  8,  4,  4,  8,  4,
	12, 12,  8,  8,  4,  4,  8,  4, 12, 12,  8,  8,  4,  4,  8,  4,
		8, 12, 20,  8,  4,  4,  8,  4,  8, 12, 20,  8,  4,  4,  8,  4,
		8, 12, 16,  8, 12, 12, 12,  4,  8, 12, 16,  8,  4,  4,  8,  4,
		4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
		4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
		4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
		8,  8,  8,  8,  8,  8,  4,  8,  4,  4,  4,  4,  4,  4,  8,  4,
		4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
		4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
		4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
		4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
		8, 12, 12, 12, 12, 16,  8, 16,  8, 12, 12,  4, 12, 20,  8, 16,
		8, 12, 12, 12, 12, 16,  8, 16,  8,  4, 12, 12, 12,  4,  8, 16,
		8, 12, 12, 24, 12, 16,  8, 16,  8,  4, 12,  4, 12,  4,  8, 16,
		8, 12, 12,  4, 12, 16,  8, 16,  8,  8, 12,  4, 12,  4,  8, 16
};

static const UINT8 amstrad_cycle_table_cb[256]=
{
		4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
		4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
		4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
		4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
		4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
		4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
		4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
		4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
		4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
		4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
		4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
		4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
		4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
		4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
		4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
		4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4
};

static const UINT8 amstrad_cycle_table_ed[256]=
{
		4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
		4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
		4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
		4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 20,  4, 12,  4,  8, 12, 12, 12, 20,  4, 12,  4,  8,
	12, 12, 12, 20,  4, 12,  4,  8, 12, 12, 12, 20,  4, 12,  4,  8,
	12, 12, 12, 20,  4, 12,  4, 16, 12, 12, 12, 20,  4, 12,  4, 16,
	12, 12, 12, 20,  4, 12,  4,  4, 12, 12, 12, 20,  4, 12,  4,  4,
		4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
		4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	16, 12, 16, 16,  4,  4,  4,  4, 16, 12, 16, 16,  4,  4,  4,  4,
	16, 12, 16, 16,  4,  4,  4,  4, 16, 12, 16, 16,  4,  4,  4,  4,
		4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
		4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
		4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
		4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4
};

static const UINT8 amstrad_cycle_table_xy[256]=
{
		4, 12,  8,  8,  4,  4,  8,  4,  4, 12,  8,  8,  4,  4,  8,  4,
	12, 12,  8,  8,  4,  4,  8,  4, 12, 12,  8,  8,  4,  4,  8,  4,
		8, 12, 20,  8,  4,  4,  8,  4,  8, 12, 20,  8,  4,  4,  8,  4,
		8, 12, 16,  8, 20, 20, 20,  4,  8, 12, 16,  8,  4,  4,  8,  4,
		4,  4,  4,  4,  4,  4, 16,  4,  4,  4,  4,  4,  4,  4, 16,  4,
		4,  4,  4,  4,  4,  4, 16,  4,  4,  4,  4,  4,  4,  4, 16,  4,
		4,  4,  4,  4,  4,  4, 16,  4,  4,  4,  4,  4,  4,  4, 16,  4,
	16, 16, 16, 16, 16, 16,  4, 16,  4,  4,  4,  4,  4,  4, 16,  4,
		4,  4,  4,  4,  4,  4, 16,  4,  4,  4,  4,  4,  4,  4, 16,  4,
		4,  4,  4,  4,  4,  4, 16,  4,  4,  4,  4,  4,  4,  4, 16,  4,
		4,  4,  4,  4,  4,  4, 16,  4,  4,  4,  4,  4,  4,  4, 16,  4,
		4,  4,  4,  4,  4,  4, 16,  4,  4,  4,  4,  4,  4,  4, 16,  4,
		8, 12, 12, 12, 12, 16,  8, 16,  8, 12, 12,  4, 12, 20,  8, 16,
		8, 12, 12, 12, 12, 16,  8, 16,  8,  4, 12, 12, 12,  4,  8, 16,
		8, 12, 12, 24, 12, 16,  8, 16,  8,  4, 12,  4, 12,  4,  8, 16,
		8, 12, 12,  4, 12, 16,  8, 16,  8,  8, 12,  4, 12,  4,  8, 16
};

static const UINT8 amstrad_cycle_table_xycb[256]=
{
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20
};

static const UINT8 amstrad_cycle_table_ex[256]=
{
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		4,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,
		4,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		4,  8,  4,  4,  0,  0,  0,  0,  4,  8,  4,  4,  0,  0,  0,  0,
		8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0,
		8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0,
		8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0,
		8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0
};

#define NEXT_ROM_SLOT   m_rom_count++; \
						if(slot3 && m_rom_count == 3) m_rom_count++; \
						if(slot7 && m_rom_count == 7) m_rom_count++;

void amstrad_state::enumerate_roms()
{
	UINT8 m_rom_count = 1;
	device_t* romexp;
	rom_image_device* romimage;
	UINT8 *rom = m_region_maincpu->base();
	char str[20];
	int i;
	bool slot3 = false,slot7 = false;

	if (m_system_type == SYSTEM_PLUS || m_system_type == SYSTEM_GX4000)
	{
		UINT8 *crt = m_region_cart->base();
		int bank_mask = (m_cart->get_rom_size() / 0x4000) - 1;

		/* ROMs are stored on the inserted cartridge in the Plus/GX4000 */
		for(i=0; i<128; i++)  // fill ROM table
			m_Amstrad_ROM_Table[i] = &crt[0x4000];
		for(i=128;i<160;i++)
			m_Amstrad_ROM_Table[i] = &crt[((i - 128) & bank_mask) * 0x4000];
		m_Amstrad_ROM_Table[7] = &crt[0xc000];
		slot7 = true;
	}
	else
	{
		/* slot 0 is always BASIC, as is any unused slot */
		for(i=0; i<256; i++)
			m_Amstrad_ROM_Table[i] = &rom[0x014000];
		/* AMSDOS ROM -- TODO: exclude from 464 unless a DDI-1 device is connected */
		m_Amstrad_ROM_Table[7] = &rom[0x018000];
		slot7 = true;
	}

	/* MSX-DOS BIOS - Aleste MSX emulation */
	if(m_system_type == SYSTEM_ALESTE)
	{
		m_Amstrad_ROM_Table[3] = &rom[0x01c000];
		slot3 = true;
	}

	/* enumerate expansion ROMs */

	/* find any expansion devices that have a 'exp_rom' region */
	cpc_expansion_slot_device* exp_port = m_exp;

	while(exp_port != NULL)
	{
		device_t* temp;

		temp = dynamic_cast<device_t*>(exp_port->get_card_device());
		if(temp != NULL)
		{
			if(temp->memregion("exp_rom")->base() != NULL)
			{
				int num = temp->memregion("exp_rom")->bytes() / 0x4000;
				for(i=0;i<num;i++)
				{
					m_Amstrad_ROM_Table[m_rom_count] = temp->memregion("exp_rom")->base()+0x4000*i;
					NEXT_ROM_SLOT
				}
			}
		}
		exp_port = temp->subdevice<cpc_expansion_slot_device>("exp");
	}


	/* add ROMs from ROMbox expansion */
	romexp = get_expansion_device(machine(),"rom");
	if(romexp)
	{
		for(i=0;i<8;i++)
		{
			sprintf(str,"rom%i",i+1);
			romimage = romexp->subdevice<rom_image_device>(str);
			if(romimage->base() != NULL)
			{
				m_Amstrad_ROM_Table[m_rom_count] = romimage->base();
				NEXT_ROM_SLOT
			}
		}
	}

}

void amstrad_state::amstrad_common_init()
{
//  address_space &space = m_maincpu->space(AS_PROGRAM);

	m_aleste_mode = 0;

	m_gate_array.mrer = 0;
	m_gate_array.vsync = 0;
	m_gate_array.hsync = 0;
	m_GateArray_RamConfiguration = 0;
	m_gate_array.hsync_counter = 2;
	AmstradCPC_GA_SetRamConfiguration();
/*  space.install_read_bank(0x0000, 0x1fff, "bank1");
    space.install_read_bank(0x2000, 0x3fff, "bank2");

    space.install_read_bank(0x4000, 0x5fff, "bank3");
    space.install_read_bank(0x6000, 0x7fff, "bank4");

    space.install_read_bank(0x8000, 0x9fff, "bank5");
    space.install_read_bank(0xa000, 0xbfff, "bank6");

    space.install_read_bank(0xc000, 0xdfff, "bank7");
    space.install_read_bank(0xe000, 0xffff, "bank8");

    space.install_write_bank(0x0000, 0x1fff, "bank9");
    space.install_write_bank(0x2000, 0x3fff, "bank10");

    space.install_write_bank(0x4000, 0x5fff, "bank11");
    space.install_write_bank(0x6000, 0x7fff, "bank12");

    space.install_write_bank(0x8000, 0x9fff, "bank13");
    space.install_write_bank(0xa000, 0xbfff, "bank14");

    space.install_write_bank(0xc000, 0xdfff, "bank15");
    space.install_write_bank(0xe000, 0xffff, "bank16");
*/
	enumerate_roms();

	m_maincpu->reset();
	if ( m_system_type == SYSTEM_CPC || m_system_type == SYSTEM_ALESTE )
		m_maincpu->set_input_line_vector(0, 0xff);
	else
		m_maincpu->set_input_line_vector(0, 0x00);

	/* The opcode timing in the Amstrad is different to the opcode
	timing in the core for the Z80 CPU.

	The Amstrad hardware issues a HALT for each memory fetch.
	This has the effect of stretching the timing for Z80 opcodes,
	so that they are all multiple of 4 T states long. All opcode
	timings are a multiple of 1us in length. */

	/* Using the cool code Juergen has provided, I will override
	the timing tables with the values for the amstrad */
	m_maincpu->z80_set_cycle_tables(
		(const UINT8*)amstrad_cycle_table_op,
		(const UINT8*)amstrad_cycle_table_cb,
		(const UINT8*)amstrad_cycle_table_ed,
		(const UINT8*)amstrad_cycle_table_xy,
		(const UINT8*)amstrad_cycle_table_xycb,
		(const UINT8*)amstrad_cycle_table_ex);

	/* Juergen is a cool dude! */
}

TIMER_CALLBACK_MEMBER(amstrad_state::cb_set_resolution)
{
	rectangle visarea;
	attoseconds_t refresh;
	int height;

	if ( m_io_solder_links->read() & 0x10 )
	{
		/* PAL */
		visarea.set(0, 64 + 640 + 64 - 1, 34, 34 + 15 + 242 + 15 - 1);
		height = 312;
	}
	else
	{
		/* NTSC */
		visarea.set(0, 64 + 640 + 64 - 1, 16, 16 + 15 + 200 + 15 - 1);
		height = 262;
	}
	refresh = HZ_TO_ATTOSECONDS( XTAL_16MHz ) * 1024 * height;
	m_screen->configure( 1024, height, visarea, refresh );
}


MACHINE_START_MEMBER(amstrad_state,amstrad)
{
	m_system_type = SYSTEM_CPC;
	m_centronics->write_data7(0);
}


MACHINE_RESET_MEMBER(amstrad_state,amstrad)
{
	amstrad_common_init();
	amstrad_reset_machine();
//  amstrad_init_palette(machine());

	m_gate_array.de = 0;
	m_gate_array.draw_p = NULL;
	m_gate_array.hsync = 0;
	m_gate_array.vsync = 0;

	timer_set(attotime::zero, TIMER_SET_RESOLUTION);
}


MACHINE_START_MEMBER(amstrad_state,plus)
{
	m_asic.ram = m_region_user1->base();  // 16kB RAM for ASIC, memory-mapped registers.
	m_system_type = SYSTEM_PLUS;
	m_centronics->write_data7(0);

	std::string region_tag;
	m_region_cart = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	if (!m_region_cart) // this should never happen, since we make carts mandatory!
		m_region_cart = memregion("maincpu");
}


MACHINE_RESET_MEMBER(amstrad_state,plus)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	m_asic.enabled = 0;
	m_asic.seqptr = 0;
	m_asic.pri = 0;  // disable PRI
	m_asic.dma_status = 0;  // disable all DMA channels
	m_asic.dma_addr[0] = 0;
	m_asic.dma_prescaler[0] = 0;
	m_asic.dma_addr[1] = 0;
	m_asic.dma_prescaler[1] = 0;
	m_asic.dma_addr[2] = 0;
	m_asic.dma_prescaler[2] = 0;
	m_asic.dma_clear = 1;  // by default, DMA interrupts must be cleared by writing to the DSCR (&6c0f)
	m_plus_irq_cause = 6;

	amstrad_common_init();
	amstrad_reset_machine();
	m_asic.ram[0x2805] = 0x01;  // interrupt vector is undefined at startup, except that bit 0 is always 1.
	AmstradCPC_GA_SetRamConfiguration();
	amstrad_GateArray_write(0x081); // Epyx World of Sports requires upper ROM to be enabled by default

	space.install_read_handler(0x4000, 0x5fff, read8_delegate(FUNC(amstrad_state::amstrad_plus_asic_4000_r),this));
	space.install_read_handler(0x6000, 0x7fff, read8_delegate(FUNC(amstrad_state::amstrad_plus_asic_6000_r),this));
	space.install_write_handler(0x4000, 0x5fff, write8_delegate(FUNC(amstrad_state::amstrad_plus_asic_4000_w),this));
	space.install_write_handler(0x6000, 0x7fff, write8_delegate(FUNC(amstrad_state::amstrad_plus_asic_6000_w),this));

	//  multiface_init();
	timer_set(attotime::zero, TIMER_SET_RESOLUTION);
}

MACHINE_START_MEMBER(amstrad_state,gx4000)
{
	m_asic.ram = m_region_user1->base();  // 16kB RAM for ASIC, memory-mapped registers.
	m_system_type = SYSTEM_GX4000;

	std::string region_tag;
	m_region_cart = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	if (!m_region_cart) // this should never happen, since we make carts mandatory!
		m_region_cart = memregion("maincpu");
}

MACHINE_RESET_MEMBER(amstrad_state,gx4000)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	m_asic.enabled = 0;
	m_asic.seqptr = 0;
	m_asic.pri = 0;  // disable PRI
	m_asic.dma_status = 0;  // disable all DMA channels
	m_asic.dma_addr[0] = 0;
	m_asic.dma_prescaler[0] = 0;
	m_asic.dma_addr[1] = 0;
	m_asic.dma_prescaler[1] = 0;
	m_asic.dma_addr[2] = 0;
	m_asic.dma_prescaler[2] = 0;
	m_asic.dma_clear = 1;  // by default, DMA interrupts must be cleared by writing to the DSCR (&6c0f)
	m_plus_irq_cause = 6;

	amstrad_common_init();
	amstrad_reset_machine();
	m_asic.ram[0x2805] = 0x01;  // interrupt vector is undefined at startup, except that bit 0 is always 1.
	AmstradCPC_GA_SetRamConfiguration();
	amstrad_GateArray_write(0x081); // Epyx World of Sports requires upper ROM to be enabled by default
	//  multiface_init();
	space.install_read_handler(0x4000, 0x5fff, read8_delegate(FUNC(amstrad_state::amstrad_plus_asic_4000_r),this));
	space.install_read_handler(0x6000, 0x7fff, read8_delegate(FUNC(amstrad_state::amstrad_plus_asic_6000_r),this));
	space.install_write_handler(0x4000, 0x5fff, write8_delegate(FUNC(amstrad_state::amstrad_plus_asic_4000_w),this));
	space.install_write_handler(0x6000, 0x7fff, write8_delegate(FUNC(amstrad_state::amstrad_plus_asic_6000_w),this));

	timer_set(attotime::zero, TIMER_SET_RESOLUTION);
}

MACHINE_START_MEMBER(amstrad_state,kccomp)
{
	m_system_type = SYSTEM_CPC;
	m_centronics->write_data7(0);

	m_gate_array.de = 0;
	m_gate_array.draw_p = NULL;
	m_gate_array.hsync = 0;
	m_gate_array.vsync = 0;

	timer_set(attotime::zero, TIMER_SET_RESOLUTION);
}


MACHINE_RESET_MEMBER(amstrad_state,kccomp)
{
	amstrad_common_init();
	kccomp_reset_machine();

	/* bit 1 = /TEST. When 0, KC compact will enter data transfer
	sequence, where another system using the expansion port signals
	DATA2,DATA1, /STROBE and DATA7 can transfer 256 bytes of program.
	When the program has been transfered, it will be executed. This
	is not supported in the driver */
	/* bit 3,4 are tied to +5V, bit 2 is tied to 0V */
	m_ppi_port_inputs[amstrad_ppi_PortB] = (1<<4) | (1<<3) | 2;
}


MACHINE_START_MEMBER(amstrad_state,aleste)
{
	m_system_type = SYSTEM_ALESTE;
	m_centronics->write_data7(0);
}

MACHINE_RESET_MEMBER(amstrad_state,aleste)
{
	amstrad_common_init();
	amstrad_reset_machine();

	timer_set(attotime::zero, TIMER_SET_RESOLUTION);
}


/* load snapshot */
SNAPSHOT_LOAD_MEMBER( amstrad_state,amstrad)
{
	dynamic_buffer snapshot;

	/* get file size */
	if (snapshot_size < 8)
		return IMAGE_INIT_FAIL;

	snapshot.resize(snapshot_size);

	/* read whole file */
	image.fread(&snapshot[0], snapshot_size);

	if (memcmp(&snapshot[0], "MV - SNA", 8))
	{
		return IMAGE_INIT_FAIL;
	}

	amstrad_handle_snapshot(&snapshot[0]);
	return IMAGE_INIT_PASS;
}


DEVICE_IMAGE_LOAD_MEMBER(amstrad_state, amstrad_plus_cartridge)
{
	UINT32 size = m_cart->common_get_size("rom");
	unsigned char header[12];
	bool is_cpr = FALSE;
	logerror("IMG: loading CPC+ cartridge file\n");

	// check for .CPR header
	if (image.software_entry() == NULL)
	{
		image.fread(header, 12);
		if (strncmp((char *)header, "RIFF", 4) != 0)
		{
			// not a CPR file, so rewind the image at start
			image.fseek(0, SEEK_SET);
		}
		else
		{
			is_cpr = TRUE;
			size -= 12;
		}
	}

	// alloc ROM
	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);

	// actually load the cart into ROM
	if (image.software_entry() != NULL)
	{
		logerror("IMG: raw CPC+ cartridge from softlist\n");
		memcpy(m_cart->get_rom_base(), image.get_software_region("rom"), size);
	}
	else if (!is_cpr)
	{
		// not an RIFF format file, assume raw binary (*.bin)
		logerror("IMG: raw CPC+ cartridge file\n");
		if (size % 0x4000)
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Attempt to load a raw binary with some block smaller than 16kB in size");
			return IMAGE_INIT_FAIL;
		}
		else
			image.fread(m_cart->get_rom_base(), size);
	}
	else
	{
		// load CPC Plus / GX4000 cartridge image
		// Format is RIFF: RIFF header chunk contains "AMS!"
		// Chunks should be 16k, but may vary
		// Chunks labeled 'cb00' represent Cartridge block 0, and is loaded to &0000-&3fff
		//                'cb01' represent Cartridge block 1, and is loaded to &4000-&7fff
		//                ... and so on.

		UINT32 offset = 0;
		UINT8 *crt = m_cart->get_rom_base();
		dynamic_buffer temp_copy;
		temp_copy.resize(size);
		image.fread(&temp_copy[0], size);

		// RIFF chunk bits
		char chunkid[4];              // chunk ID (4 character code - cb00, cb01, cb02... upto cb31 (max 512kB), other chunks are ignored)
		char chunklen[4];             // chunk length (always little-endian)
		int chunksize;                // chunk length, calcaulated from the above
		int ramblock;                 // 16k RAM block chunk is to be loaded into
		unsigned int bytes_to_read;   // total bytes to read, as mame_feof doesn't react to EOF without trying to go past it.

		// Is RIFF format (*.cpr)
		if (strncmp((char*)(header + 8), "AMS!", 4) != 0)
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Not an Amstrad CPC cartridge image (despite RIFF header)");
			return IMAGE_INIT_FAIL;
		}

		bytes_to_read = header[4] + (header[5] << 8) + (header[6] << 16)+ (header[7] << 24);
		bytes_to_read -= 4;  // account for AMS! header
		logerror("CPR: Data to read: %i bytes\n", bytes_to_read);
		// read some chunks
		while (bytes_to_read > 0)
		{
			memcpy(chunkid, &temp_copy[offset], 4);
			bytes_to_read -= 4;
			offset += 4;

			memcpy(chunklen, &temp_copy[offset], 4);
			bytes_to_read -= 4;
			offset += 4;

			// calculate little-endian value, just to be sure
			chunksize = chunklen[0] + (chunklen[1] << 8) + (chunklen[2] << 16) + (chunklen[3] << 24);

			if (strncmp(chunkid, "cb", 2) == 0)
			{
				// load chunk into RAM
				// find out what block this is
				ramblock = (chunkid[2] - 0x30) * 10;
				ramblock += chunkid[3] - 0x30;
				logerror("CPR: Loading chunk into RAM block %i ['%4s']\n", ramblock, chunkid);

				// load block into ROM area (max 512K)
				if (ramblock >= 0 && ramblock < 32)
				{
					if (chunksize > 0x4000)
						chunksize = 0x4000;

					memcpy(crt + 0x4000 * ramblock, &temp_copy[offset], chunksize);
					bytes_to_read -= chunksize;
					offset += chunksize;
					logerror("CPR: Loaded %i-byte chunk into RAM block %i\n", chunksize, ramblock);
				}
			}
			else
			{
				logerror("CPR: Unknown chunk '%4s', skipping %i bytes\n", chunkid, chunksize);
				if (chunksize != 0)
				{
					bytes_to_read -= chunksize;
					offset += chunksize;
				}
			}
		}
	}

	return IMAGE_INIT_PASS;
}
