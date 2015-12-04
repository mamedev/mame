// license:BSD-3-Clause
// copyright-holders:James Wallace, Philip Bennett
/***********************************************************************************************************
Barcrest MPU4 Extension driver by J.Wallace, Phil Bennett and Anonymous.

--- Board Setup ---
For the Barcrest MPU4 Video system, the GAME CARD (cartridge) contains the MPU4 video bios in the usual ROM
space (occupying 16k), an interface card to connect an additional Video board, and a 6850 serial IO to
communicate with said board.

The VIDEO BOARD is driven by a 10mhz 68000 processor, and contains a 6840PTM, 6850 serial IO
(the other end of the communications), an SAA1099 for stereo sound and SCN2674 gfx chip.

The VIDEO CARTRIDGE plugs into the video board, and contains the program ROMs for the video based game.
Like the MPU4 game card, in some cases an extra OKI sound chip is added to the video board's game card,
as well as extra RAM.
There is a protection chip similar to and replacing the MPU4 Characteriser.

No video card schematics ever left the PCB factory, but some decent scans of the board have been made,
now also available for review.

Additional: 68k HALT line is tied to the reset circuit of the MPU4.

--- Preliminary MPU4 Video Interface Card Memorymap  ---

   hex     |r/w| D D D D D D D D |
 location  |   | 7 6 5 4 3 2 1 0 | function
-----------+---+-----------------+--------------------------------------------------------------------------
 0000-07FF |R/W| D D D D D D D D | 2k RAM
-----------+---+-----------------+--------------------------------------------------------------------------
 0800      |R/W|                 | ACIA 6850
-----------+---+-----------------+--------------------------------------------------------------------------
 0900-     |R/W| D D D D D D D D | MC6840 PTM IC2


  Clock1 <--------------------------------------
     |                                          |
     V                                          |
  Output1 ---> Clock2                           |
                                                |
               Output2 --+-> Clock3             |
                         |                      |
                         |   Output3 ---> 'to audio amp' ??
                         |
                         +--------> CA1 IC3 (

IRQ line connected to CPU

-----------+---+-----------------+--------------------------------------------------------------------------
 0A00-0A03 |R/W| D D D D D D D D | PIA6821 IC3 port A Lamp Drives 1,2,3,4,6,7,8,9 (sic)(IC14)
           |   |                 |
           |   |                 |          CA1 <= output2 from PTM6840 (IC2)
           |   |                 |          CA2 => alpha data
           |   |                 |
           |   |                 |          port B Lamp Drives 10,11,12,13,14,15,16,17 (sic)(IC13)
           |   |                 |
           |   |                 |          CB2 => alpha reset (clock on Dutch systems)
           |   |                 |
-----------+---+-----------------+--------------------------------------------------------------------------
 0B00-0B03 |R/W| D D D D D D D D | PIA6821 IC4 port A = data for 7seg leds (pins 10 - 17, via IC32)
           |   |                 |
           |   |                 |             CA1 INPUT, 50 Hz input (used to generate IRQ)
           |   |                 |             CA2 OUTPUT, connected to pin2 74LS138 CE for multiplexer
           |   |                 |                        (B on LED strobe multiplexer)
           |   |                 |             IRQA connected to IRQ of CPU
           |   |                 |             port B
           |   |                 |                    PB7 = INPUT, serial port Receive data (Rx)
           |   |                 |                    PB6 = INPUT, reel A sensor
           |   |                 |                    PB5 = INPUT, reel B sensor
           |   |                 |                    PB4 = INPUT, reel C sensor
           |   |                 |                    PB3 = INPUT, reel D sensor
           |   |                 |                    PB2 = INPUT, Connected to CA1 (50Hz signal)
           |   |                 |                    PB1 = INPUT, undercurrent sense
           |   |                 |                    PB0 = INPUT, overcurrent  sense
           |   |                 |
           |   |                 |             CB1 INPUT,  used to generate IRQ on edge of serial input line
           |   |                 |             CB2 OUTPUT, enable signal for reel optics
           |   |                 |             IRQB connected to IRQ of CPU
           |   |                 |
-----------+---+-----------------+--------------------------------------------------------------------------
 0C00-0C03 |R/W| D D D D D D D D | PIA6821 IC5 port A
           |   |                 |
           |   |                 |                    PA0-PA7, INPUT AUX1 connector
           |   |                 |
           |   |                 |             CA2  OUTPUT, serial port Transmit line
           |   |                 |             CA1  not connected
           |   |                 |             IRQA connected to IRQ of CPU
           |   |                 |
           |   |                 |             port B
           |   |                 |
           |   |                 |                    PB0-PB7 INPUT, AUX2 connector
           |   |                 |
           |   |                 |             CB1  INPUT,  connected to PB7 (Aux2 connector pin 4)
           |   |                 |
           |   |                 |             CB2  OUTPUT, AY8913 chip select line
           |   |                 |             IRQB connected to IRQ of CPU
           |   |                 |
-----------+---+-----------------+--------------------------------------------------------------------------
 0D00-0D03 |R/W| D D D D D D D D | PIA6821 IC6
           |   |                 |
           |   |                 |  port A
           |   |                 |
           |   |                 |        PA0 - PA7 (INPUT/OUTPUT) data port AY8913 sound chip
           |   |                 |
           |   |                 |        CA1 INPUT,  not connected
           |   |                 |        CA2 OUTPUT, BC1 pin AY8913 sound chip
           |   |                 |        IRQA , connected to IRQ CPU
           |   |                 |
           |   |                 |  port B
           |   |                 |
           |   |                 |        PB0-PB3 OUTPUT, reel A
           |   |                 |        PB4-PB7 OUTPUT, reel B
           |   |                 |
           |   |                 |        CB1 INPUT,  not connected
           |   |                 |        CB2 OUTPUT, B01R pin AY8913 sound chip
           |   |                 |        IRQB , connected to IRQ CPU
           |   |                 |
-----------+---+-----------------+--------------------------------------------------------------------------
 0E00-0E03 |R/W| D D D D D D D D | PIA6821 IC7
           |   |                 |
           |   |                 |  port A
           |   |                 |
           |   |                 |        PA0-PA3 OUTPUT, reel C
           |   |                 |        PA4-PA7 OUTPUT, reel D
           |   |                 |        CA1     INPUT,  not connected
           |   |                 |        CA2     OUTPUT, A on LED strobe multiplexer
           |   |                 |        IRQA , connected to IRQ CPU
           |   |                 |
           |   |                 |  port B
           |   |                 |
           |   |                 |        PB0-PB6 OUTPUT mech meter 1-7 or reel E + F
           |   |                 |        PB7     Voltage drop sensor
           |   |                 |        CB1     INPUT, not connected
           |   |                 |        CB2     OUTPUT,mech meter 8
           |   |                 |        IRQB , connected to IRQ CPU
           |   |                 |
-----------+---+-----------------+--------------------------------------------------------------------------
 0F00-0F03 |R/W| D D D D D D D D | PIA6821 IC8
           |   |                 |
           |   |                 | port A
           |   |                 |
           |   |                 |        PA0-PA7 INPUT  multiplexed inputs data
           |   |                 |
           |   |                 |        CA1     INPUT, not connected
           |   |                 |        CA2    OUTPUT, C on LED strobe multiplexer
           |   |                 |        IRQA           connected to IRQ CPU
           |   |                 |
           |   |                 | port B
           |   |                 |
           |   |                 |        PB0-PB7 OUTPUT  triacs outputs connector PL6
           |   |                 |        used for slides / hoppers
           |   |                 |
           |   |                 |        CB1     INPUT, not connected
           |   |                 |        CB2    OUTPUT, pin1 alpha display PL7 (clock signal)
           |   |                 |        IRQB           connected to IRQ CPU
           |   |                 |
-----------+---+-----------------+--------------------------------------------------------------------------
 4000-40FF |R/W| D D D D D D D D | RAM ?
-----------+---+-----------------+--------------------------------------------------------------------------
 BE00-BFFF |R/W| D D D D D D D D | RAM
-----------+---+-----------------+--------------------------------------------------------------------------
 C000-FFFF | R | D D D D D D D D | ROM
-----------+---+-----------------+--------------------------------------------------------------------------
Everything here is preliminary...  the boards are quite fussy with regards their self tests
and the timing may have to be perfect for them to function correctly.  (as the comms are
timer driven, the video is capable of various raster effects etc.)

TODO:
      - Correctly implement characteriser protection for each game.
      - Get the BwB games running
        * They have a slightly different 68k memory map. The 6850 is at e00000 and the 6840 is at e01000
        They appear to hang on the handshake with the MPU4 board
      - Find out what causes the games to hang/reset in service mode
        Probably down to AVDC interrupt timing, there seem to be a number of race conditions re: masks
        that need sorting out with proper blank handling, etc. I'm using a scanline timer to drive an
        approximation of the SCN2674 scanline logic, but this is perhaps better served as a proper device.
 ***********************************************************************************************************/
#include "emu.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/nvram.h"

#include "cpu/m6809/m6809.h"
#include "sound/ay8910.h"
#include "sound/okim6376.h"
#include "sound/2413intf.h"
#include "sound/upd7759.h"
#include "machine/steppers.h"
#include "machine/roc10937.h"
#include "machine/meters.h"
#include "video/scn2674.h"

#include "cpu/m68000/m68000.h"
#include "machine/6850acia.h"
#include "sound/saa1099.h"
#include "machine/nvram.h"
#include "crmaze2p.lh"
#include "crmaze4p.lh"
#include "includes/mpu4.h"
#include "cpu/m68000/m68000.h"


struct ef9369_t
{
	UINT32 addr;
	UINT16 clut[16];    /* 13-bits - a marking bit and a 444 color */
};

struct bt471_t
{
	UINT8 address;
	UINT8 addr_cnt;
	UINT8 pixmask;
	UINT8 command;
	rgb_t color;
};


class mpu4vid_state : public mpu4_state
{
public:
	mpu4vid_state(const machine_config &mconfig, device_type type, const char *tag)
		: mpu4_state(mconfig, type, tag),
		m_videocpu(*this, "video"),
		m_scn2674(*this, "scn2674_vid"),
		m_vid_vidram(*this, "vid_vidram"),
		m_vid_mainram(*this, "vid_mainram"),
		m_acia_0(*this, "acia6850_0"),
		m_acia_1(*this, "acia6850_1"),
		m_ptm(*this, "6840ptm_68k"),
		m_trackx_port(*this, "TRACKX"),
		m_tracky_port(*this, "TRACKY"),
		m_gfxdecode(*this, "gfxdecode")
	{
	}

	required_device<m68000_base_device> m_videocpu;
	optional_device<scn2674_device> m_scn2674;
	optional_shared_ptr<UINT16> m_vid_vidram;
	optional_shared_ptr<UINT16> m_vid_mainram;
	required_device<acia6850_device> m_acia_0;
	required_device<acia6850_device> m_acia_1;
	required_device<ptm6840_device> m_ptm;
	optional_ioport m_trackx_port;
	optional_ioport m_tracky_port;
	required_device<gfxdecode_device> m_gfxdecode;

	struct ef9369_t m_pal;
	struct bt471_t m_bt471;

	//Video
	UINT8 m_m6840_irq_state;
	UINT8 m_m6850_irq_state;
	int m_gfx_index;
	INT8 m_cur[2];


	DECLARE_DRIVER_INIT(crmazea);
	DECLARE_DRIVER_INIT(v4barqst2);
	DECLARE_DRIVER_INIT(quidgrid);
	DECLARE_DRIVER_INIT(v4barqst);
	DECLARE_DRIVER_INIT(timemchn);
	DECLARE_DRIVER_INIT(crmaze2a);
	DECLARE_DRIVER_INIT(v4opt3);
	DECLARE_DRIVER_INIT(eyesdown);
	DECLARE_DRIVER_INIT(v4cmazeb);
	DECLARE_DRIVER_INIT(crmaze2);
	DECLARE_DRIVER_INIT(crmaze);
	DECLARE_DRIVER_INIT(prizeinv);
	DECLARE_DRIVER_INIT(strikeit);
	DECLARE_DRIVER_INIT(v4wize);
	DECLARE_DRIVER_INIT(turnover);
	DECLARE_DRIVER_INIT(adders);
	DECLARE_DRIVER_INIT(mating);
	DECLARE_DRIVER_INIT(crmaze3a);
	DECLARE_DRIVER_INIT(skiltrek);
	DECLARE_DRIVER_INIT(crmaze3);
	DECLARE_DRIVER_INIT(cybcas);
	DECLARE_MACHINE_START(mpu4_vid);
	DECLARE_MACHINE_RESET(mpu4_vid);
	DECLARE_VIDEO_START(mpu4_vid);
	SCN2674_DRAW_CHARACTER_MEMBER(display_pixels);
	DECLARE_WRITE_LINE_MEMBER(m6809_acia_irq);
	DECLARE_WRITE_LINE_MEMBER(m68k_acia_irq);
	DECLARE_WRITE_LINE_MEMBER(cpu1_ptm_irq);
	DECLARE_WRITE8_MEMBER(vid_o1_callback);
	DECLARE_WRITE8_MEMBER(vid_o2_callback);
	DECLARE_WRITE8_MEMBER(vid_o3_callback);
	DECLARE_READ8_MEMBER(pia_ic5_porta_track_r);
	void mpu4vid_char_cheat( int address);
	DECLARE_WRITE_LINE_MEMBER(update_mpu68_interrupts);
	DECLARE_READ16_MEMBER( mpu4_vid_vidram_r );
	DECLARE_WRITE16_MEMBER( mpu4_vid_vidram_w );
	DECLARE_WRITE8_MEMBER( ef9369_w );
	DECLARE_READ8_MEMBER( ef9369_r );
	DECLARE_WRITE8_MEMBER( bt471_w );
	DECLARE_READ8_MEMBER( bt471_r );
	DECLARE_WRITE8_MEMBER( vidcharacteriser_w );
	DECLARE_READ8_MEMBER( vidcharacteriser_r );
	DECLARE_WRITE_LINE_MEMBER(mpu_video_reset);
	DECLARE_WRITE8_MEMBER( vram_w );
	DECLARE_READ8_MEMBER( vram_r );
};

/*************************************
 *
 *  Interrupt system
 *
 *************************************/

/* The interrupt system consists of a 74148 priority encoder
   with the following interrupt priorites.  A lower number
   indicates a lower priority:

    7 - Game Card
    6 - Game Card
    5 - Game Card
    4 - Game Card
    3 - 2674 AVDC
    2 - 6850 ACIA
    1 - 6840 PTM
    0 - Unused
*/


WRITE_LINE_MEMBER(mpu4vid_state::update_mpu68_interrupts)
{
	m_videocpu->set_input_line(1, m_m6840_irq_state ? ASSERT_LINE : CLEAR_LINE);
	m_videocpu->set_input_line(2, m_m6850_irq_state ? ASSERT_LINE : CLEAR_LINE);
}

/* Communications with 6809 board */

WRITE_LINE_MEMBER(mpu4vid_state::m6809_acia_irq)
{
	m_acia_1->write_cts(state);
	m_maincpu->set_input_line(M6809_IRQ_LINE, state);
}

WRITE_LINE_MEMBER(mpu4vid_state::m68k_acia_irq)
{
	m_acia_0->write_cts(state);
	m_m6850_irq_state = state;
	update_mpu68_interrupts(1);
}


WRITE_LINE_MEMBER(mpu4vid_state::cpu1_ptm_irq)
{
	m_m6840_irq_state = state;
	update_mpu68_interrupts(1);
}


WRITE8_MEMBER(mpu4vid_state::vid_o1_callback)
{
	m_ptm->set_c2(data); /* this output is the clock for timer2 */

	m_acia_0->write_txc(data);
	m_acia_0->write_rxc(data);
	m_acia_1->write_txc(data);
	m_acia_1->write_rxc(data);
}


WRITE8_MEMBER(mpu4vid_state::vid_o2_callback)
{
	m_ptm->set_c3(data); /* this output is the clock for timer3 */
}


WRITE8_MEMBER(mpu4vid_state::vid_o3_callback)
{
	m_ptm->set_c1(data); /* this output is the clock for timer1 */
}


static const gfx_layout mpu4_vid_char_8x8_layout =
{
	8,8,
	0x1000, /* 0x1000 tiles (128k of GFX RAM, 0x20 bytes per tile) */
	4,
	{ 0,8,16,24 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32},
	8*32
};

WRITE8_MEMBER(mpu4vid_state::vram_w)
{
	m_vid_mainram[offset] = data | (m_vid_mainram[offset] & 0xff00);
}

READ8_MEMBER(mpu4vid_state::vram_r)
{
	return m_vid_mainram[offset];
}

static ADDRESS_MAP_START( mpu4_vram, AS_0, 8, mpu4vid_state )
	AM_RANGE(0x0000, 0x7fff) AM_READWRITE(vram_r, vram_w)
ADDRESS_MAP_END

SCN2674_DRAW_CHARACTER_MEMBER(mpu4vid_state::display_pixels)
{
	if(!lg)
	{
		UINT16 tile = m_vid_mainram[address & 0x7fff];
		const UINT8 *line = m_gfxdecode->gfx(m_gfx_index+0)->get_data(tile & 0xfff);
		int offset = m_gfxdecode->gfx(m_gfx_index+0)->rowbytes() * linecount;
		for(int i = 0; i < 8; i++)
			bitmap.pix32(y, x + i) = (tile >> 12) ? m_palette->pen(line[offset + i]) : m_palette->black_pen();
	}
}


READ16_MEMBER(mpu4vid_state::mpu4_vid_vidram_r )
{
	return m_vid_vidram[offset];
}


WRITE16_MEMBER(mpu4vid_state::mpu4_vid_vidram_w )
{
	COMBINE_DATA(&m_vid_vidram[offset]);
	offset <<= 1;
	m_gfxdecode->gfx(m_gfx_index+0)->mark_dirty(offset/0x20);
}


VIDEO_START_MEMBER(mpu4vid_state,mpu4_vid)
{
	m_vid_vidram.allocate(0x20000/2);

	memset(m_vid_vidram,0,0x20000);

	/* find first empty slot to decode gfx */
	for (m_gfx_index = 0; m_gfx_index < MAX_GFX_ELEMENTS; m_gfx_index++)
		if (m_gfxdecode->gfx(m_gfx_index) == nullptr)
			break;

	assert(m_gfx_index != MAX_GFX_ELEMENTS);

	/* create the char set (gfx will then be updated dynamically from RAM) */
	m_gfxdecode->set_gfx(m_gfx_index+0, global_alloc(gfx_element(m_palette, mpu4_vid_char_8x8_layout, reinterpret_cast<UINT8 *>(m_vid_vidram.target()), NATIVE_ENDIAN_VALUE_LE_BE(8,0), m_palette->entries() / 16, 0)));
}




/****************************
 *  EF9369 color palette IC
 *  (16 colors from 4096)
 ****************************/

/* Non-multiplexed mode */

WRITE8_MEMBER(mpu4vid_state::ef9369_w )
{
	struct ef9369_t &pal = m_pal;

	/* Address register */
	if (offset & 1)
	{
		pal.addr = data & 0x1f;
	}
	/* Data register */
	else
	{
		UINT32 entry = pal.addr >> 1;

		if ((pal.addr & 1) == 0)
		{
			pal.clut[entry] &= ~0x00ff;
			pal.clut[entry] |= data;
		}
		else
		{
			UINT16 col;

			pal.clut[entry] &= ~0x1f00;
			pal.clut[entry] |= (data & 0x1f) << 8;

			/* Remove the marking bit */
			col = pal.clut[entry] & 0xfff;

			/* Update the MAME palette */
			m_palette->set_pen_color(entry, pal4bit(col >> 8), pal4bit(col >> 4), pal4bit(col >> 0));
		}

			/* Address register auto-increment */
		if (++pal.addr == 32)
			pal.addr = 0;
	}
}


READ8_MEMBER(mpu4vid_state::ef9369_r )
{
	struct ef9369_t &pal = m_pal;
	if ((offset & 1) == 0)
	{
		UINT16 col = pal.clut[pal.addr >> 1];

/*      if ((pal.addr & 1) == 0)
            return col & 0xff;
        else
            return col >> 8;
            */

	return col;
	}
	else
	{
		/* Address register is write only */
		return 0xff;
	}
}

/******************************************
 *
 *  Brooktree Bt471 RAMDAC
 *  Implementation stolen from JPM
 *  Impact, may not be 100% (that has a 477)
 ******************************************/

/*
 *  0 0 0    Address register (RAM write mode)
 *  0 0 1    Color palette RAMs
 *  0 1 0    Pixel read mask register
 *  0 1 1    Address register (RAM read mode)
 *  1 0 0    Address register (overlay write mode)
 *  1 1 1    Address register (overlay read mode)
 *  1 0 1    Overlay register
 */

WRITE8_MEMBER(mpu4vid_state::bt471_w )
{
	struct bt471_t &bt471 = m_bt471;

	switch (offset)
	{
		case 0x0:
		{
			bt471.address = data;
			bt471.addr_cnt = 0;
			break;
		}
		case 0x1:
		{
			UINT8 *addr_cnt = &bt471.addr_cnt;
			rgb_t *color = &bt471.color;

			color[*addr_cnt] = data;

			if (++*addr_cnt == 3)
			{
				m_palette->set_pen_color(bt471.address, rgb_t(color[0], color[1], color[2]));
				*addr_cnt = 0;

				/* Address register increments */
				bt471.address++;
			}
			break;
		}
		case 0x2:
		{
			bt471.pixmask = data;
			break;
		}

		default:
		{
			popmessage("Bt471: Unhandled write access (offset:%x, data:%x)", offset, data);
		}
	}
}

READ8_MEMBER(mpu4vid_state::bt471_r )
{
	popmessage("Bt471: Unhandled read access (offset:%x)", offset);
	return 0;
}


/*************************************
 *
 *  Trackball interface
 *
 *************************************/

READ8_MEMBER(mpu4vid_state::pia_ic5_porta_track_r)
{
	/* The SWP trackball interface connects a standard trackball to the AUX1 port on the MPU4
	mainboard. As per usual, they've taken the cheap route here, reading and processing the
	raw quadrature signal from the encoder wheels for a 4 bit interface, rather than use any
	additional hardware to simplify matters. What makes matters worse is that there is a 45 degree rotation to take into account.
	For our purposes, two fake ports give the X and Y positions, which are then worked back into the signal levels.
	We invert the X and Y data at source due to the use of Schmitt triggers in the interface, which
	clean up the pulses and flip the active phase.*/

	LOG(("%s: IC5 PIA Read of Port A (AUX1)\n",machine().describe_context()));


	UINT8 data = m_aux1_port->read();

	INT8 dx = m_trackx_port->read();
	INT8 dy = m_tracky_port->read();

	m_cur[0] = dy + dx;
	m_cur[1] = dy - dx;

	UINT8 xa, xb, ya, yb;

	/* generate pulses for the input port (A and B are 1 unit out of phase for direction sensing)*/
	xa = ((m_cur[0] + 1) & 3) <= 1;
	xb = (m_cur[0] & 3) <= 1;
	ya = ((m_cur[1] + 1) & 3) <= 1;
	yb = (m_cur[1] & 3) <= 1;

	data |= (xa << 4); // XA
	data |= (ya << 5); // YA
	data |= (xb << 6); // XB
	data |= (yb << 7); // YB

	return data;
}


/*************************************
 *
 *  Input defintions
 *
 *************************************/

static INPUT_PORTS_START( crmaze )
	PORT_START("ORANGE1")
	PORT_BIT(0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ORANGE2")
	PORT_BIT(0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Test Button") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Cashbox Door")  PORT_CODE(KEYCODE_Q) PORT_TOGGLE

	PORT_START("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Right Yellow")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Right Red")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Left Red")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Left Yellow")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Getout Yellow")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Escape/Getout Red")/* Labelled Escape on cabinet */
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DIL1")
	PORT_DIPNAME( 0x01, 0x00, "DIL101" ) PORT_DIPLOCATION("DIL1:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL102" ) PORT_DIPLOCATION("DIL1:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL103" ) PORT_DIPLOCATION("DIL1:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL104" ) PORT_DIPLOCATION("DIL1:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL105" ) PORT_DIPLOCATION("DIL1:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL106" ) PORT_DIPLOCATION("DIL1:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL107" ) PORT_DIPLOCATION("DIL1:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "DIL108" ) PORT_DIPLOCATION("DIL1:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("DIL2")
	PORT_DIPNAME( 0x01, 0x00, "DIL201" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL202" ) PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL203" ) PORT_DIPLOCATION("DIL2:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL204" ) PORT_DIPLOCATION("DIL2:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL205" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL206" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL207" ) PORT_DIPLOCATION("DIL2:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "DIL208" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("AUX1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SPECIAL)//XA
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SPECIAL)//YA
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SPECIAL)//XB
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_SPECIAL)//YB

	PORT_START("AUX2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("20p")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("50p")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("100p")

	PORT_START("TRACKX")//FAKE
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_INVERT
	PORT_START("TRACKY")//FAKE
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_INVERT
INPUT_PORTS_END

static INPUT_PORTS_START( mating )
	PORT_START("ORANGE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ORANGE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Test Button") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Cashbox Door")  PORT_CODE(KEYCODE_Q) PORT_TOGGLE

	PORT_START("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Right Yellow")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Right Red")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Left Yellow")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Left Red")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("100p Service?")

	PORT_START("DIL1")
	PORT_DIPNAME( 0x01, 0x00, "DIL101" ) PORT_DIPLOCATION("DIL1:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL102" ) PORT_DIPLOCATION("DIL1:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL103" ) PORT_DIPLOCATION("DIL1:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL104" ) PORT_DIPLOCATION("DIL1:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL105" ) PORT_DIPLOCATION("DIL1:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL106" ) PORT_DIPLOCATION("DIL1:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL107" ) PORT_DIPLOCATION("DIL1:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "DIL108" ) PORT_DIPLOCATION("DIL1:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("DIL2")
	PORT_DIPNAME( 0x01, 0x00, "DIL201" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL202" ) PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL203" ) PORT_DIPLOCATION("DIL2:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL204" ) PORT_DIPLOCATION("DIL2:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL205" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL206" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL207" ) PORT_DIPLOCATION("DIL2:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "DIL208" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("AUX1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SPECIAL)//XA
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SPECIAL)//YA
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SPECIAL)//XB
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_SPECIAL)//YB

	PORT_START("AUX2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("20p")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("50p")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("100p")

	PORT_START("TRACKX")//FAKE
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_INVERT
	PORT_START("TRACKY")//FAKE
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_INVERT

INPUT_PORTS_END



static INPUT_PORTS_START( skiltrek )
	PORT_START("ORANGE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ORANGE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Pass")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("C")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("B")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("A")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Test Button") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Cashbox Door")  PORT_CODE(KEYCODE_Q) PORT_TOGGLE

	PORT_START("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Continue")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("C")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("B")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("A")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DIL1")
	PORT_DIPNAME( 0x01, 0x00, "DIL101" ) PORT_DIPLOCATION("DIL1:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL102" ) PORT_DIPLOCATION("DIL1:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL103" ) PORT_DIPLOCATION("DIL1:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL104" ) PORT_DIPLOCATION("DIL1:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL105" ) PORT_DIPLOCATION("DIL1:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL106" ) PORT_DIPLOCATION("DIL1:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL107" ) PORT_DIPLOCATION("DIL1:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "DIL108" ) PORT_DIPLOCATION("DIL1:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("DIL2")
	PORT_DIPNAME( 0x01, 0x00, "1 Pound for change" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL202" ) PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL203" ) PORT_DIPLOCATION("DIL2:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Attract mode inhibit" ) PORT_DIPLOCATION("DIL2:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL205" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin alarm inhibit" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL207" ) PORT_DIPLOCATION("DIL2:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "Single coin entry" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("AUX1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("AUX2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p")//PORT_IMPULSE(5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("20p")//PORT_IMPULSE(5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("50p")//PORT_IMPULSE(5)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("100p")//PORT_IMPULSE(5)
INPUT_PORTS_END

static INPUT_PORTS_START( turnover )
	PORT_START("ORANGE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ORANGE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Pass")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("C")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("B")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("A")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Test Button") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Cashbox Door")  PORT_CODE(KEYCODE_Q) PORT_TOGGLE

	PORT_START("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Continue")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("C")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("B")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("A")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_START1)

	PORT_START("DIL1")
	PORT_DIPNAME( 0x01, 0x00, "DIL101" ) PORT_DIPLOCATION("DIL1:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL102" ) PORT_DIPLOCATION("DIL1:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL103" ) PORT_DIPLOCATION("DIL1:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL104" ) PORT_DIPLOCATION("DIL1:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL105" ) PORT_DIPLOCATION("DIL1:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL106" ) PORT_DIPLOCATION("DIL1:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL107" ) PORT_DIPLOCATION("DIL1:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "DIL108" ) PORT_DIPLOCATION("DIL1:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("DIL2")
	PORT_DIPNAME( 0x01, 0x00, "1 Pound for change" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL202" ) PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL203" ) PORT_DIPLOCATION("DIL2:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Attract mode inhibit" ) PORT_DIPLOCATION("DIL2:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL205" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin alarm inhibit" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL207" ) PORT_DIPLOCATION("DIL2:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "Single coin entry" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("AUX1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("AUX2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p")//PORT_IMPULSE(5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("20p")//PORT_IMPULSE(5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("50p")//PORT_IMPULSE(5)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("100p")//PORT_IMPULSE(5)
INPUT_PORTS_END

static INPUT_PORTS_START( adders )
	PORT_START("ORANGE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ORANGE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("C")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("B")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("A")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Pass")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Test Button") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Cashbox Door")  PORT_CODE(KEYCODE_Q) PORT_TOGGLE

	PORT_START("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("C")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("B")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("A")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DIL1")
	PORT_DIPNAME( 0x01, 0x00, "DIL101" ) PORT_DIPLOCATION("DIL1:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL102" ) PORT_DIPLOCATION("DIL1:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL103" ) PORT_DIPLOCATION("DIL1:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL104" ) PORT_DIPLOCATION("DIL1:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL105" ) PORT_DIPLOCATION("DIL1:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL106" ) PORT_DIPLOCATION("DIL1:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL107" ) PORT_DIPLOCATION("DIL1:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "DIL108" ) PORT_DIPLOCATION("DIL1:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("DIL2")
	PORT_DIPNAME( 0x01, 0x00, "1 Pound for change" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL202" ) PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL203" ) PORT_DIPLOCATION("DIL2:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Attract mode inhibit" ) PORT_DIPLOCATION("DIL2:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL205" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin alarm inhibit" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL207" ) PORT_DIPLOCATION("DIL2:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "Single coin entry" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("AUX1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("AUX2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p")//PORT_IMPULSE(5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("20p")//PORT_IMPULSE(5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("50p")//PORT_IMPULSE(5)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("100p")//PORT_IMPULSE(5)
INPUT_PORTS_END

WRITE_LINE_MEMBER(mpu4vid_state::mpu_video_reset)
{
	m_ptm->reset();
	m_acia_1->reset();
}

/* machine start (called only once) */
MACHINE_START_MEMBER(mpu4vid_state,mpu4_vid)
{
	mpu4_config_common();

	m_mod_number=4; //No AY chip
	/* setup communications */
	m_link7a_connected = 1;

	/* setup 8 mechanical meters */
	MechMtr_config(machine(),8);

	/* Hook the reset line */
	m_videocpu->set_reset_callback(write_line_delegate(FUNC(mpu4vid_state::mpu_video_reset),this));
}

MACHINE_RESET_MEMBER(mpu4vid_state,mpu4_vid)
{
	m_vfd->reset(); //for debug ports only

	m_lamp_strobe    = 0;
	m_lamp_strobe2   = 0;
	m_led_strobe     = 0;

	m_IC23GC    = 0;
	m_IC23GB    = 0;
	m_IC23GA    = 0;
	m_IC23G1    = 1;
	m_IC23G2A   = 0;
	m_IC23G2B   = 0;

	m_prot_col  = 0;
	m_chr_counter    = 0;
	m_chr_value     = 0;
}

static ADDRESS_MAP_START( mpu4_68k_map, AS_PROGRAM, 16, mpu4vid_state )
	AM_RANGE(0x000000, 0x7fffff) AM_ROM
	AM_RANGE(0x800000, 0x80ffff) AM_RAM AM_SHARE("vid_mainram")
//  AM_RANGE(0x810000, 0x81ffff) AM_RAM /* ? */
	AM_RANGE(0x900000, 0x900001) AM_DEVWRITE8("saa", saa1099_device, data_w, 0x00ff)
	AM_RANGE(0x900002, 0x900003) AM_DEVWRITE8("saa", saa1099_device, control_w, 0x00ff)
	AM_RANGE(0xa00000, 0xa00003) AM_READWRITE8(ef9369_r, ef9369_w,0x00ff)
/*  AM_RANGE(0xa00004, 0xa0000f) AM_READWRITE(mpu4_vid_unmap_r, mpu4_vid_unmap_w) */


	AM_RANGE(0xb00000, 0xb0000f) AM_DEVREADWRITE8("scn2674_vid", scn2674_device, read, write,0x00ff)

	AM_RANGE(0xc00000, 0xc1ffff) AM_READWRITE(mpu4_vid_vidram_r, mpu4_vid_vidram_w) AM_SHARE("vid_vidram")
	AM_RANGE(0xff8000, 0xff8001) AM_DEVREADWRITE8("acia6850_1", acia6850_device, status_r, control_w, 0x00ff)
	AM_RANGE(0xff8002, 0xff8003) AM_DEVREADWRITE8("acia6850_1", acia6850_device, data_r, data_w, 0x00ff)
	AM_RANGE(0xff9000, 0xff900f) AM_DEVREADWRITE8("6840ptm_68k", ptm6840_device, read, write, 0x00ff)
	AM_RANGE(0xffd000, 0xffd00f) AM_READWRITE8(vidcharacteriser_r, vidcharacteriser_w,0x00ff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mpu4oki_68k_map, AS_PROGRAM, 16, mpu4vid_state )
	AM_RANGE(0x000000, 0x5fffff) AM_ROM //AM_WRITENOP
	AM_RANGE(0x600000, 0x63ffff) AM_RAM /* The Mating Game has an extra 256kB RAM on the program card */
//  AM_RANGE(0x640000, 0x7fffff) AM_NOP /* Possible bug, reads and writes here */
	AM_RANGE(0x800000, 0x80ffff) AM_RAM AM_SHARE("vid_mainram")
	AM_RANGE(0x900000, 0x900001) AM_DEVWRITE8("saa", saa1099_device, data_w, 0x00ff)
	AM_RANGE(0x900002, 0x900003) AM_DEVWRITE8("saa", saa1099_device, control_w, 0x00ff)
	AM_RANGE(0xa00000, 0xa00003) AM_READWRITE8(ef9369_r, ef9369_w,0x00ff)

	AM_RANGE(0xb00000, 0xb0000f) AM_DEVREADWRITE8("scn2674_vid", scn2674_device, read, write,0x00ff)

	AM_RANGE(0xc00000, 0xc1ffff) AM_READWRITE(mpu4_vid_vidram_r, mpu4_vid_vidram_w) AM_SHARE("vid_vidram")
	AM_RANGE(0xff8000, 0xff8001) AM_DEVREADWRITE8("acia6850_1", acia6850_device, status_r, control_w, 0x00ff)
	AM_RANGE(0xff8002, 0xff8003) AM_DEVREADWRITE8("acia6850_1", acia6850_device, data_r, data_w, 0x00ff)
	AM_RANGE(0xff9000, 0xff900f) AM_DEVREADWRITE8("6840ptm_68k", ptm6840_device, read, write, 0x00ff)
	AM_RANGE(0xffa040, 0xffa04f) AM_DEVREAD8("ptm_ic3ss", ptm6840_device, read,0x00ff)  // 6840PTM on sampled sound board
	AM_RANGE(0xffa040, 0xffa04f) AM_WRITE8(ic3ss_w,0x00ff)  // 6840PTM on sampled sound board
	AM_RANGE(0xffa060, 0xffa067) AM_DEVREADWRITE8("pia_ic4ss", pia6821_device, read, write,0x00ff)    // PIA6821 on sampled sound board
	AM_RANGE(0xffd000, 0xffd00f) AM_READWRITE8(vidcharacteriser_r, vidcharacteriser_w, 0x00ff)
//  AM_RANGE(0xfff000, 0xffffff) AM_NOP /* Possible bug, reads and writes here */
ADDRESS_MAP_END


static ADDRESS_MAP_START( bwbvid_68k_map, AS_PROGRAM, 16, mpu4vid_state )
	AM_RANGE(0x000000, 0x7fffff) AM_ROM
	AM_RANGE(0x800000, 0x80ffff) AM_RAM AM_SHARE("vid_mainram")
	AM_RANGE(0x810000, 0x81ffff) AM_RAM /* ? */
	AM_RANGE(0x900000, 0x900001) AM_DEVWRITE8("saa", saa1099_device, data_w, 0x00ff)
	AM_RANGE(0x900002, 0x900003) AM_DEVWRITE8("saa", saa1099_device, control_w, 0x00ff)
	AM_RANGE(0xa00000, 0xa00003) AM_READWRITE8(ef9369_r, ef9369_w,0x00ff)
//  AM_RANGE(0xa00000, 0xa0000f) AM_READWRITE(bt471_r,bt471_w) //Some games use this
/*  AM_RANGE(0xa00004, 0xa0000f) AM_READWRITE(mpu4_vid_unmap_r, mpu4_vid_unmap_w) */

	AM_RANGE(0xb00000, 0xb0000f) AM_DEVREADWRITE8("scn2674_vid", scn2674_device, read, write,0x00ff)
	AM_RANGE(0xc00000, 0xc1ffff) AM_READWRITE(mpu4_vid_vidram_r, mpu4_vid_vidram_w) AM_SHARE("vid_vidram")
	AM_RANGE(0xe00000, 0xe00001) AM_DEVREADWRITE8("acia6850_1", acia6850_device, status_r, control_w, 0x00ff)
	AM_RANGE(0xe00002, 0xe00003) AM_DEVREADWRITE8("acia6850_1", acia6850_device, data_r, data_w, 0x00ff)
	AM_RANGE(0xe01000, 0xe0100f) AM_DEVREADWRITE8("6840ptm_68k", ptm6840_device, read, write, 0x00ff)
	//AM_RANGE(0xa00004, 0xa0000f) AM_READWRITE(bwb_characteriser16_r, bwb_characteriser16_w)//AM_READWRITE(adpcm_r, adpcm_w)  CHR ?
ADDRESS_MAP_END

static ADDRESS_MAP_START( bwbvid5_68k_map, AS_PROGRAM, 16, mpu4vid_state )
	AM_RANGE(0x000000, 0x7fffff) AM_ROM
	AM_RANGE(0x800000, 0x80ffff) AM_RAM AM_SHARE("vid_mainram")
	AM_RANGE(0x810000, 0x81ffff) AM_RAM /* ? */
	AM_RANGE(0x900000, 0x900001) AM_DEVWRITE8("saa", saa1099_device, data_w, 0x00ff)
	AM_RANGE(0x900002, 0x900003) AM_DEVWRITE8("saa", saa1099_device, control_w, 0x00ff)
	AM_RANGE(0xa00000, 0xa00003) AM_READWRITE8(ef9369_r, ef9369_w,0x00ff)
	//AM_RANGE(0xa00000, 0xa00003) AM_READWRITE8(bt471_r,bt471_w,0x00ff) Some games use this
/*  AM_RANGE(0xa00004, 0xa0000f) AM_READWRITE(mpu4_vid_unmap_r, mpu4_vid_unmap_w) */

	AM_RANGE(0xb00000, 0xb0000f) AM_DEVREADWRITE8("scn2674_vid", scn2674_device, read, write,0x00ff)
	AM_RANGE(0xc00000, 0xc1ffff) AM_READWRITE(mpu4_vid_vidram_r, mpu4_vid_vidram_w) AM_SHARE("vid_vidram")
	AM_RANGE(0xe00000, 0xe00001) AM_DEVREADWRITE8("acia6850_1", acia6850_device, status_r, control_w, 0x00ff)
	AM_RANGE(0xe00002, 0xe00003) AM_DEVREADWRITE8("acia6850_1", acia6850_device, data_r, data_w, 0x00ff)
	AM_RANGE(0xe01000, 0xe0100f) AM_DEVREADWRITE8("6840ptm_68k", ptm6840_device, read, write, 0x00ff)
	AM_RANGE(0xe02000, 0xe02007) AM_DEVREADWRITE8("pia_ic4ss", pia6821_device, read, write, 0xff00) //Seems odd...
	AM_RANGE(0xe03000, 0xe0300f) AM_DEVREAD8("ptm_ic3ss", ptm6840_device, read,0xff00)  // 6840PTM on sampled sound board
	AM_RANGE(0xe03000, 0xe0300f) AM_WRITE8(ic3ss_w,0xff00)  // 6840PTM on sampled sound board
	AM_RANGE(0xe04000, 0xe0400f) AM_READWRITE8(bwb_characteriser_r, bwb_characteriser_w, 0x00ff)//AM_READWRITE(adpcm_r, adpcm_w)  CHR ?
ADDRESS_MAP_END



/* TODO: Fix up MPU4 map*/
static ADDRESS_MAP_START( mpu4_6809_map, AS_PROGRAM, 8, mpu4_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x0800, 0x0800) AM_DEVREADWRITE("acia6850_0", acia6850_device, status_r, control_w)
	AM_RANGE(0x0801, 0x0801) AM_DEVREADWRITE("acia6850_0", acia6850_device, data_r, data_w)
	AM_RANGE(0x0880, 0x0881) AM_NOP //Read/write here
	AM_RANGE(0x0900, 0x0907) AM_DEVREADWRITE("ptm_ic2", ptm6840_device, read, write)
	AM_RANGE(0x0a00, 0x0a03) AM_DEVREADWRITE("pia_ic3", pia6821_device, read, write)
	AM_RANGE(0x0b00, 0x0b03) AM_DEVREADWRITE("pia_ic4", pia6821_device, read, write)
	AM_RANGE(0x0c00, 0x0c03) AM_DEVREADWRITE("pia_ic5", pia6821_device, read, write)
	AM_RANGE(0x0d00, 0x0d03) AM_DEVREADWRITE("pia_ic6", pia6821_device, read, write)
	AM_RANGE(0x0e00, 0x0e03) AM_DEVREADWRITE("pia_ic7", pia6821_device, read, write)
	AM_RANGE(0x0f00, 0x0f03) AM_DEVREADWRITE("pia_ic8", pia6821_device, read, write)
	AM_RANGE(0x4000, 0x7fff) AM_RAM
	AM_RANGE(0xbe00, 0xbfff) AM_RAM
	AM_RANGE(0xc000, 0xffff) AM_ROM AM_REGION("maincpu",0)  /* 64k EPROM on board, only this region read */
ADDRESS_MAP_END




static MACHINE_CONFIG_START( mpu4_vid, mpu4vid_state )
	MCFG_CPU_ADD("maincpu", M6809, MPU4_MASTER_CLOCK/4 )
	MCFG_CPU_PROGRAM_MAP(mpu4_6809_map)

	MCFG_NVRAM_ADD_0FILL("nvram")               /* confirm */

	MCFG_FRAGMENT_ADD(mpu4_common)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE((63*8)+(17*8), (37*8)+17) // note this directly affects the scanline counters used below, and thus the timing of everything
	MCFG_SCREEN_VISIBLE_AREA(0, (63*8)+(0)-1, 0, (37*8)+0-1)

	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_UPDATE_DEVICE("scn2674_vid", scn2674_device, screen_update)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", empty)

	MCFG_SCN2674_VIDEO_ADD("scn2674_vid", 0, INPUTLINE("video", 3))
	MCFG_SCN2674_TEXT_CHARACTER_WIDTH(8)
	MCFG_SCN2674_GFX_CHARACTER_WIDTH(8)
	MCFG_SCN2674_DRAW_CHARACTER_CALLBACK_OWNER(mpu4vid_state, display_pixels)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, mpu4_vram)


	MCFG_CPU_ADD("video", M68000, VIDEO_MASTER_CLOCK )
	MCFG_CPU_PROGRAM_MAP(mpu4_68k_map)

//  MCFG_QUANTUM_TIME(attotime::from_hz(960))

	MCFG_MACHINE_START_OVERRIDE(mpu4vid_state,mpu4_vid)
	MCFG_MACHINE_RESET_OVERRIDE(mpu4vid_state,mpu4_vid)
	MCFG_VIDEO_START_OVERRIDE (mpu4vid_state,mpu4_vid)

	MCFG_PALETTE_ADD("palette", 16)

	MCFG_DEVICE_ADD("6840ptm_68k", PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(VIDEO_MASTER_CLOCK / 10) /* 68k E clock */
	MCFG_PTM6840_EXTERNAL_CLOCKS(0, 0, 0)
	MCFG_PTM6840_OUT0_CB(WRITE8(mpu4vid_state, vid_o1_callback))
	MCFG_PTM6840_OUT1_CB(WRITE8(mpu4vid_state, vid_o2_callback))
	MCFG_PTM6840_OUT2_CB(WRITE8(mpu4vid_state, vid_o3_callback))
	MCFG_PTM6840_IRQ_CB(WRITELINE(mpu4vid_state, cpu1_ptm_irq))
	/* Present on all video cards */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SAA1099_ADD("saa", 8000000)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.5)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.5)

	MCFG_DEVICE_ADD("acia6850_0", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("acia6850_1", acia6850_device, write_rxd))
	MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE("acia6850_1", acia6850_device, write_dcd))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(mpu4vid_state, m6809_acia_irq))

	MCFG_DEVICE_ADD("acia6850_1", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("acia6850_0", acia6850_device, write_rxd))
	MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE("acia6850_0", acia6850_device, write_dcd))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(mpu4vid_state, m68k_acia_irq))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( crmaze, mpu4_vid )
	MCFG_DEVICE_MODIFY("pia_ic5")
	MCFG_PIA_READPA_HANDLER(READ8(mpu4vid_state, pia_ic5_porta_track_r))
	MCFG_PIA_WRITEPA_HANDLER(NULL)
	MCFG_PIA_WRITEPB_HANDLER(NULL)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mating, crmaze )
	MCFG_CPU_MODIFY("video")
	MCFG_CPU_PROGRAM_MAP(mpu4oki_68k_map)

	MCFG_FRAGMENT_ADD(mpu4_common2)

	MCFG_SOUND_ADD("msm6376", OKIM6376, 128000) //?
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.5)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.5)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bwbvid, mpu4_vid )
	MCFG_CPU_MODIFY("video")
	MCFG_CPU_PROGRAM_MAP(bwbvid_68k_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bwbvid5, bwbvid )
	MCFG_CPU_MODIFY("video")
	MCFG_CPU_PROGRAM_MAP(bwbvid5_68k_map)

	MCFG_FRAGMENT_ADD(mpu4_common2)

	MCFG_SOUND_ADD("msm6376", OKIM6376, 128000) //?
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.5)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.5)
MACHINE_CONFIG_END

/*
Characteriser (CHR)
 The question data on the quiz games gets passed through the characterizer, the tables tested at startup are just a
 very specific test with known responses to make sure the device functions properly.  Unless there is extra encryption
 applied to just the question ROMs then the assumptions made here are wrong, because the questions don't decode.

 Perhaps the address lines for the question ROMS are scrambled somehow to make things decode, but how?

 It seems more likely that the Characterizer (PAL) acts as a challenge / response system, but various writes cause
 'latching' behavior because if you study the sequence written at startup you can see that the same write value should
 generate different responses.

 Note:
 the 'challenge' part of the startup check is always the same
*/

WRITE8_MEMBER(mpu4vid_state::vidcharacteriser_w )
{
	int x;
	int call=(data&0xff);
	LOG_CHR_FULL(("%04x Characteriser write offset %02X data %02X", space.device().safe_pcbase(),offset,data));

	if (!m_current_chr_table)
	{
		logerror("No Characteriser Table @ %04x\n", space.device().safe_pcbase());
		return;
	}

	for (x = m_prot_col; x < 64; x++)
	{
		if (call == 0)
		{
			m_prot_col = 0;
		}
		else
		{
			if  (m_current_chr_table[(x)].call == call)
			{
				m_prot_col = x;
				LOG_CHR(("Characteriser find column %02X\n",m_prot_col));
				break;
			}
		}
	}
}


READ8_MEMBER(mpu4vid_state::vidcharacteriser_r )
{
	LOG_CHR_FULL(("%04x Characteriser read offset %02X,data %02X", space.device().safe_pcbase(),offset,m_current_chr_table[m_prot_col].response));
	LOG_CHR(("Characteriser read offset %02X \n",offset));
	LOG_CHR(("Characteriser read data %02X \n",m_current_chr_table[m_prot_col].response));

	if (!m_current_chr_table)
	{
		logerror("No Characteriser Table @ %04x\n", space.device().safe_pcbase());
		return 0x00;
	}


	/* hack for 'invalid questions' error on time machine.. I guess it wants them to decode properly for startup check? */
	if (space.device().safe_pcbase()==0x283a)
	{
		return 0x00;
	}

	return m_current_chr_table[m_prot_col].response;
}



static mpu4_chr_table adders_data[64] = {
	{0x00, 0x00}, {0x1A, 0x8C}, {0x04, 0x64}, {0x10, 0x84}, {0x18, 0x84}, {0x0F, 0xC4}, {0x13, 0x84}, {0x1B, 0x84},
	{0x03, 0x9C}, {0x07, 0xF4}, {0x17, 0x04}, {0x1D, 0xCC}, {0x36, 0x24}, {0x35, 0x84}, {0x2B, 0xC4}, {0x28, 0x94},
	{0x39, 0x54}, {0x21, 0x0C}, {0x22, 0x74}, {0x25, 0x0C}, {0x2C, 0x34}, {0x29, 0x04}, {0x31, 0x84}, {0x34, 0x84},
	{0x0A, 0xC4}, {0x1F, 0x84}, {0x06, 0x9C}, {0x0E, 0xE4}, {0x1C, 0x84}, {0x12, 0x84}, {0x1E, 0x84}, {0x0D, 0xD4},
	{0x14, 0x44}, {0x0A, 0x84}, {0x19, 0xC4}, {0x15, 0x84}, {0x06, 0x9C}, {0x0F, 0xE4}, {0x08, 0x84}, {0x1B, 0x84},
	{0x1E, 0x84}, {0x04, 0x8C}, {0x01, 0x60}, {0x0C, 0x84}, {0x18, 0x84}, {0x1A, 0x84}, {0x11, 0x84}, {0x0B, 0xC4},
	{0x03, 0x9C}, {0x17, 0xF4}, {0x10, 0x04}, {0x1D, 0xCC}, {0x0E, 0x24}, {0x07, 0x9C}, {0x12, 0xF4}, {0x09, 0x04},
	{0x0D, 0x94}, {0x1F, 0x14}, {0x16, 0x44}, {0x05, 0x8C}, {0x13, 0x34}, {0x1C, 0x04}, {0x02, 0x9C}, {0x00, 0x00}
};

static mpu4_chr_table crmaze_data[64] = {
	{0x00, 0x00}, {0x1A, 0x34}, {0x04, 0x14}, {0x10, 0x0C}, {0x18, 0x54}, {0x0F, 0x04}, {0x13, 0x24}, {0x1B, 0x34},
	{0x03, 0x94}, {0x07, 0x94}, {0x17, 0x0C}, {0x1D, 0x5C}, {0x36, 0x6C}, {0x35, 0x44}, {0x2B, 0x24}, {0x28, 0x24},
	{0x39, 0x3C}, {0x21, 0x6C}, {0x22, 0xCC}, {0x25, 0x4C}, {0x2C, 0xC4}, {0x29, 0xA4}, {0x31, 0x24}, {0x34, 0x24},
	{0x0A, 0x34}, {0x1F, 0x84}, {0x06, 0xB4}, {0x0E, 0x1C}, {0x1C, 0x64}, {0x12, 0x24}, {0x1E, 0x34}, {0x0D, 0x04},
	{0x14, 0x24}, {0x0A, 0x34}, {0x19, 0x8C}, {0x15, 0xC4}, {0x06, 0xB4}, {0x0F, 0x1C}, {0x08, 0xE4}, {0x1B, 0x24},
	{0x1E, 0x34}, {0x04, 0x14}, {0x01, 0x10}, {0x0C, 0x84}, {0x18, 0x24}, {0x1A, 0x34}, {0x11, 0x04}, {0x0B, 0x24},
	{0x03, 0xB4}, {0x17, 0x04}, {0x10, 0x24}, {0x1D, 0x3C}, {0x0E, 0x74}, {0x07, 0x94}, {0x12, 0x0C}, {0x09, 0xC4},
	{0x0D, 0xA4}, {0x1F, 0x24}, {0x16, 0x24}, {0x05, 0x34}, {0x13, 0x04}, {0x1C, 0x34}, {0x02, 0x94}, {0x00, 0x00}
};

static mpu4_chr_table crmazea_data[64] = {
	{0x00, 0x00}, {0x1A, 0x0C}, {0x04, 0x90}, {0x10, 0xE0}, {0x18, 0xA4}, {0x0F, 0xAC}, {0x13, 0x78}, {0x1B, 0x5C},
	{0x03, 0xDC}, {0x07, 0xD4}, {0x17, 0xA0}, {0x1D, 0xEC}, {0x36, 0x78}, {0x35, 0x54}, {0x2B, 0x48}, {0x28, 0x50},
	{0x39, 0xC8}, {0x21, 0xF8}, {0x22, 0xDC}, {0x25, 0x94}, {0x2C, 0xE0}, {0x29, 0x24}, {0x31, 0x0C}, {0x34, 0xD8},
	{0x0A, 0x5C}, {0x1F, 0xD4}, {0x06, 0x68}, {0x0E, 0x18}, {0x1C, 0x14}, {0x12, 0xC8}, {0x1E, 0x38}, {0x0D, 0x5C},
	{0x14, 0xDC}, {0x0A, 0x5C}, {0x19, 0xDC}, {0x15, 0xD4}, {0x06, 0x68}, {0x0F, 0x18}, {0x08, 0xD4}, {0x1B, 0x60},
	{0x1E, 0x0C}, {0x04, 0x90}, {0x01, 0xE8}, {0x0C, 0xF8}, {0x18, 0xD4}, {0x1A, 0x60}, {0x11, 0x44}, {0x0B, 0x4C},
	{0x03, 0xD8}, {0x17, 0xD4}, {0x10, 0xE8}, {0x1D, 0xF8}, {0x0E, 0x9C}, {0x07, 0xD4}, {0x12, 0xE8}, {0x09, 0x30},
	{0x0D, 0x48}, {0x1F, 0xD8}, {0x16, 0xDC}, {0x05, 0x94}, {0x13, 0xE8}, {0x1C, 0x38}, {0x02, 0xDC}, {0x00, 0x00}
};

static mpu4_chr_table crmaze2_data[64] = {
	{0x00, 0x00}, {0x1A, 0x88}, {0x04, 0x54}, {0x10, 0x40}, {0x18, 0x88}, {0x0F, 0x54}, {0x13, 0x40}, {0x1B, 0x88},
	{0x03, 0x74}, {0x07, 0x28}, {0x17, 0x30}, {0x1D, 0x60}, {0x36, 0x80}, {0x35, 0x84}, {0x2B, 0xC4}, {0x28, 0xA4},
	{0x39, 0xC4}, {0x21, 0x8C}, {0x22, 0x74}, {0x25, 0x08}, {0x2C, 0x30}, {0x29, 0x00}, {0x31, 0x80}, {0x34, 0x84},
	{0x0A, 0xC4}, {0x1F, 0x84}, {0x06, 0xAC}, {0x0E, 0x5C}, {0x1C, 0x90}, {0x12, 0x44}, {0x1E, 0x88}, {0x0D, 0x74},
	{0x14, 0x00}, {0x0A, 0x80}, {0x19, 0xC4}, {0x15, 0x84}, {0x06, 0xAC}, {0x0F, 0x5C}, {0x08, 0xB0}, {0x1B, 0x24},
	{0x1E, 0x88}, {0x04, 0x54}, {0x01, 0x08}, {0x0C, 0x30}, {0x18, 0x00}, {0x1A, 0x88}, {0x11, 0x34}, {0x0B, 0x08},
	{0x03, 0x70}, {0x17, 0x00}, {0x10, 0x80}, {0x1D, 0xC4}, {0x0E, 0x84}, {0x07, 0xAC}, {0x12, 0x34}, {0x09, 0x00},
	{0x0D, 0xA0}, {0x1F, 0x84}, {0x16, 0x84}, {0x05, 0x8C}, {0x13, 0x34}, {0x1C, 0x00}, {0x02, 0xA8}, {0x00, 0x00}
};

static mpu4_chr_table crmaze3_data[64] = {
	{0x00, 0x00}, {0x1A, 0x84}, {0x04, 0x94}, {0x10, 0x3C}, {0x18, 0xEC}, {0x0F, 0x5C}, {0x13, 0xEC}, {0x1B, 0x50},
	{0x03, 0x2C}, {0x07, 0x68}, {0x17, 0x60}, {0x1D, 0xAC}, {0x36, 0x74}, {0x35, 0x00}, {0x2B, 0xAC}, {0x28, 0x58},
	{0x39, 0xEC}, {0x21, 0x7C}, {0x22, 0xEC}, {0x25, 0x58}, {0x2C, 0xE0}, {0x29, 0x90}, {0x31, 0x18}, {0x34, 0xEC},
	{0x0A, 0x54}, {0x1F, 0x28}, {0x06, 0x68}, {0x0E, 0x44}, {0x1C, 0x84}, {0x12, 0xB4}, {0x1E, 0x10}, {0x0D, 0x20},
	{0x14, 0x84}, {0x0A, 0xBC}, {0x19, 0xE8}, {0x15, 0x70}, {0x06, 0x24}, {0x0F, 0x84}, {0x08, 0xB8}, {0x1B, 0xE0},
	{0x1E, 0x94}, {0x04, 0x14}, {0x01, 0x2C}, {0x0C, 0x64}, {0x18, 0x8C}, {0x1A, 0x50}, {0x11, 0x28}, {0x0B, 0x4C},
	{0x03, 0x6C}, {0x17, 0x60}, {0x10, 0xA0}, {0x1D, 0xBC}, {0x0E, 0xCC}, {0x07, 0x78}, {0x12, 0xE8}, {0x09, 0x50},
	{0x0D, 0x20}, {0x1F, 0xAC}, {0x16, 0x74}, {0x05, 0x04}, {0x13, 0xA4}, {0x1C, 0x94}, {0x02, 0x3C}, {0x00, 0x00}
};

static mpu4_chr_table crmaze3a_data[64] = {
	{0x00, 0x00}, {0x1A, 0x0C}, {0x04, 0x60}, {0x10, 0x84}, {0x18, 0x34}, {0x0F, 0x08}, {0x13, 0xC0}, {0x1B, 0x14},
	{0x03, 0xA8}, {0x07, 0xF0}, {0x17, 0x10}, {0x1D, 0xA0}, {0x36, 0x1C}, {0x35, 0xE4}, {0x2B, 0x1C}, {0x28, 0xE4},
	{0x39, 0x34}, {0x21, 0xA8}, {0x22, 0xF8}, {0x25, 0x64}, {0x2C, 0x8C}, {0x29, 0xF0}, {0x31, 0x30}, {0x34, 0x08},
	{0x0A, 0xE8}, {0x1F, 0xF8}, {0x06, 0xE4}, {0x0E, 0x3C}, {0x1C, 0x44}, {0x12, 0x8C}, {0x1E, 0x58}, {0x0D, 0xC4},
	{0x14, 0x3C}, {0x0A, 0x6C}, {0x19, 0x68}, {0x15, 0xC0}, {0x06, 0x9C}, {0x0F, 0x64}, {0x08, 0x04}, {0x1B, 0x0C},
	{0x1E, 0x48}, {0x04, 0x60}, {0x01, 0xAC}, {0x0C, 0xF8}, {0x18, 0xE4}, {0x1A, 0x14}, {0x11, 0xA8}, {0x0B, 0x78},
	{0x03, 0xEC}, {0x17, 0xD0}, {0x10, 0xB0}, {0x1D, 0xB0}, {0x0E, 0x38}, {0x07, 0xE4}, {0x12, 0x9C}, {0x09, 0xE4},
	{0x0D, 0xBC}, {0x1F, 0xE4}, {0x16, 0x1C}, {0x05, 0x64}, {0x13, 0x8C}, {0x1C, 0x58}, {0x02, 0xEC}, {0x00, 0x00}
};

static mpu4_chr_table mating_data[64] = {
	{0x00, 0x00}, {0x1A, 0x18}, {0x04, 0xC8}, {0x10, 0xA4}, {0x18, 0x0C}, {0x0F, 0x80}, {0x13, 0x0C}, {0x1B, 0x90},
	{0x03, 0x34}, {0x07, 0x30}, {0x17, 0x00}, {0x1D, 0x58}, {0x36, 0xC8}, {0x35, 0x84}, {0x2B, 0x4C}, {0x28, 0xA0},
	{0x39, 0x4C}, {0x21, 0xC0}, {0x22, 0x3C}, {0x25, 0xC8}, {0x2C, 0xA4}, {0x29, 0x4C}, {0x31, 0x80}, {0x34, 0x0C},
	{0x0A, 0x80}, {0x1F, 0x0C}, {0x06, 0xE0}, {0x0E, 0x1C}, {0x1C, 0x88}, {0x12, 0xA4}, {0x1E, 0x0C}, {0x0D, 0xA0},
	{0x14, 0x0C}, {0x0A, 0x80}, {0x19, 0x4C}, {0x15, 0xA0}, {0x06, 0x3C}, {0x0F, 0x98}, {0x08, 0xEC}, {0x1B, 0x84},
	{0x1E, 0x0C}, {0x04, 0xC0}, {0x01, 0x1C}, {0x0C, 0xA8}, {0x18, 0x84}, {0x1A, 0x0C}, {0x11, 0xA0}, {0x0B, 0x5C},
	{0x03, 0xE8}, {0x17, 0xA4}, {0x10, 0x0C}, {0x1D, 0xD0}, {0x0E, 0x04}, {0x07, 0x38}, {0x12, 0xA8}, {0x09, 0xC4},
	{0x0D, 0x2C}, {0x1F, 0x90}, {0x16, 0x44}, {0x05, 0x18}, {0x13, 0xE8}, {0x1C, 0x84}, {0x02, 0x3C}, {0x00, 0x00}
};

static mpu4_chr_table skiltrek_data[64] = {
	{0x00, 0x00}, {0x1A, 0x1C}, {0x04, 0xCC}, {0x10, 0x64}, {0x18, 0x1C}, {0x0F, 0x4C}, {0x13, 0x64}, {0x1B, 0x1C},
	{0x03, 0xEC}, {0x07, 0xE4}, {0x17, 0x0C}, {0x1D, 0xD4}, {0x36, 0x84}, {0x35, 0x0C}, {0x2B, 0x44}, {0x28, 0x2C},
	{0x39, 0xD4}, {0x21, 0x14}, {0x22, 0x34}, {0x25, 0x14}, {0x2C, 0x24}, {0x29, 0x0C}, {0x31, 0x44}, {0x34, 0x0C},
	{0x0A, 0x44}, {0x1F, 0x1C}, {0x06, 0xEC}, {0x0E, 0x54}, {0x1C, 0x04}, {0x12, 0x0C}, {0x1E, 0x54}, {0x0D, 0x24},
	{0x14, 0x0C}, {0x0A, 0x44}, {0x19, 0x9C}, {0x15, 0xEC}, {0x06, 0xE4}, {0x0F, 0x1C}, {0x08, 0x6C}, {0x1B, 0x54},
	{0x1E, 0x04}, {0x04, 0x1C}, {0x01, 0xC8}, {0x0C, 0x64}, {0x18, 0x1C}, {0x1A, 0x4C}, {0x11, 0x64}, {0x0B, 0x1C},
	{0x03, 0xEC}, {0x17, 0x64}, {0x10, 0x0C}, {0x1D, 0xD4}, {0x0E, 0x04}, {0x07, 0x3C}, {0x12, 0x6C}, {0x09, 0x44},
	{0x0D, 0x2C}, {0x1F, 0x54}, {0x16, 0x84}, {0x05, 0x1C}, {0x13, 0xEC}, {0x1C, 0x44}, {0x02, 0x3C}, {0x00, 0x00}
};

static mpu4_chr_table timemchn_data[64] = {
	{0x00, 0x00}, {0x1A, 0x2C}, {0x04, 0x94}, {0x10, 0x14}, {0x18, 0x04}, {0x0F, 0x0C}, {0x13, 0xC4}, {0x1B, 0x0C},
	{0x03, 0xD4}, {0x07, 0x64}, {0x17, 0x0C}, {0x1D, 0xB4}, {0x36, 0x04}, {0x35, 0x0C}, {0x2B, 0x84}, {0x28, 0x5C},
	{0x39, 0xDC}, {0x21, 0x9C}, {0x22, 0xDC}, {0x25, 0x9C}, {0x2C, 0xDC}, {0x29, 0xCC}, {0x31, 0x84}, {0x34, 0x0C},
	{0x0A, 0x84}, {0x1F, 0x0C}, {0x06, 0xD4}, {0x0E, 0x04}, {0x1C, 0x2C}, {0x12, 0xC4}, {0x1E, 0x0C}, {0x0D, 0xC4},
	{0x14, 0x0C}, {0x0A, 0x84}, {0x19, 0x1C}, {0x15, 0xDC}, {0x06, 0xDC}, {0x0F, 0x8C}, {0x08, 0xD4}, {0x1B, 0x44},
	{0x1E, 0x2C}, {0x04, 0x94}, {0x01, 0x20}, {0x0C, 0x0C}, {0x18, 0xA4}, {0x1A, 0x0C}, {0x11, 0xC4}, {0x0B, 0x0C},
	{0x03, 0xD4}, {0x17, 0x14}, {0x10, 0x14}, {0x1D, 0x54}, {0x0E, 0x04}, {0x07, 0x6C}, {0x12, 0xC4}, {0x09, 0x4C},
	{0x0D, 0xC4}, {0x1F, 0x0C}, {0x16, 0xC4}, {0x05, 0x2C}, {0x13, 0xC4}, {0x1C, 0x0C}, {0x02, 0xD4}, {0x00, 0x00}
};

static mpu4_chr_table strikeit_data[64] = {
	{0x00, 0x00}, {0x1A, 0xC4}, {0x04, 0xC4}, {0x10, 0x44}, {0x18, 0xC4}, {0x0F, 0x44}, {0x13, 0x44}, {0x1B, 0xC4},
	{0x03, 0xCC}, {0x07, 0x3C}, {0x17, 0x5C}, {0x1D, 0x7C}, {0x36, 0x54}, {0x35, 0x24}, {0x2B, 0xC4}, {0x28, 0x4C},
	{0x39, 0xB4}, {0x21, 0x84}, {0x22, 0xCC}, {0x25, 0x34}, {0x2C, 0x04}, {0x29, 0x4C}, {0x31, 0x14}, {0x34, 0x24},
	{0x0A, 0xC4}, {0x1F, 0x44}, {0x06, 0xCC}, {0x0E, 0x14}, {0x1C, 0x04}, {0x12, 0x44}, {0x1E, 0xC4}, {0x0D, 0x4C},
	{0x14, 0x1C}, {0x0A, 0x54}, {0x19, 0x2C}, {0x15, 0x1C}, {0x06, 0x7C}, {0x0F, 0xD4}, {0x08, 0x0C}, {0x1B, 0x94},
	{0x1E, 0x04}, {0x04, 0xC4}, {0x01, 0xC0}, {0x0C, 0x4C}, {0x18, 0x94}, {0x1A, 0x04}, {0x11, 0x44}, {0x0B, 0x44},
	{0x03, 0xCC}, {0x17, 0x1C}, {0x10, 0x7C}, {0x1D, 0x7C}, {0x0E, 0xD4}, {0x07, 0x8C}, {0x12, 0x1C}, {0x09, 0x5C},
	{0x0D, 0x5C}, {0x1F, 0x5C}, {0x16, 0x7C}, {0x05, 0x74}, {0x13, 0x04}, {0x1C, 0xC4}, {0x02, 0xCC}, {0x00, 0x00}
};

static mpu4_chr_table turnover_data[64] = {
	{0x00, 0x00}, {0x1A, 0x1C}, {0x04, 0x6C}, {0x10, 0xA4}, {0x18, 0x0C}, {0x0F, 0x24}, {0x13, 0x0C}, {0x1B, 0x34},
	{0x03, 0x94}, {0x07, 0x94}, {0x17, 0x44}, {0x1D, 0x5C}, {0x36, 0x6C}, {0x35, 0x24}, {0x2B, 0x1C}, {0x28, 0xAC},
	{0x39, 0x64}, {0x21, 0x1C}, {0x22, 0xEC}, {0x25, 0x64}, {0x2C, 0x0C}, {0x29, 0xA4}, {0x31, 0x0C}, {0x34, 0x24},
	{0x0A, 0x1C}, {0x1F, 0xAC}, {0x06, 0xE4}, {0x0E, 0x1C}, {0x1C, 0x2C}, {0x12, 0xA4}, {0x1E, 0x0C}, {0x0D, 0xA4},
	{0x14, 0x0C}, {0x0A, 0x24}, {0x19, 0x5C}, {0x15, 0xEC}, {0x06, 0xE4}, {0x0F, 0x1C}, {0x08, 0xAC}, {0x1B, 0x24},
	{0x1E, 0x1C}, {0x04, 0x6C}, {0x01, 0x60}, {0x0C, 0x0C}, {0x18, 0x34}, {0x1A, 0x04}, {0x11, 0x0C}, {0x0B, 0x24},
	{0x03, 0x9C}, {0x17, 0xEC}, {0x10, 0xA4}, {0x1D, 0x4C}, {0x0E, 0x24}, {0x07, 0x9C}, {0x12, 0xEC}, {0x09, 0x24},
	{0x0D, 0x0C}, {0x1F, 0x34}, {0x16, 0x04}, {0x05, 0x1C}, {0x13, 0xEC}, {0x1C, 0x24}, {0x02, 0x9C}, {0x00, 0x00}
};

static mpu4_chr_table eyesdown_data[64] = {
	{0x00, 0x00}, {0x1A, 0x8C}, {0x04, 0x64}, {0x10, 0x0C}, {0x18, 0xC4}, {0x0F, 0x0C}, {0x13, 0x54}, {0x1B, 0x14},
	{0x03, 0x94}, {0x07, 0x94}, {0x17, 0x24}, {0x1D, 0xAC}, {0x36, 0x44}, {0x35, 0x0C}, {0x2B, 0x44}, {0x28, 0x1C},
	{0x39, 0x7C}, {0x21, 0x6C}, {0x22, 0x74}, {0x25, 0x84}, {0x2C, 0x3C}, {0x29, 0x4C}, {0x31, 0x44}, {0x34, 0x0C},
	{0x0A, 0x44}, {0x1F, 0x8C}, {0x06, 0x74}, {0x0E, 0x84}, {0x1C, 0x0C}, {0x12, 0x54}, {0x1E, 0x04}, {0x0D, 0x1C},
	{0x14, 0x7C}, {0x0A, 0xCC}, {0x19, 0x64}, {0x15, 0x0C}, {0x06, 0x74}, {0x0F, 0x84}, {0x08, 0x3C}, {0x1B, 0x5C},
	{0x1E, 0x4C}, {0x04, 0x64}, {0x01, 0x88}, {0x0C, 0x74}, {0x18, 0x04}, {0x1A, 0x8C}, {0x11, 0x54}, {0x0B, 0x04},
	{0x03, 0x9C}, {0x17, 0x7C}, {0x10, 0x5C}, {0x1D, 0x7C}, {0x0E, 0xCC}, {0x07, 0x74}, {0x12, 0x04}, {0x09, 0x1C},
	{0x0D, 0x5C}, {0x1F, 0x5C}, {0x16, 0x7C}, {0x05, 0x6C}, {0x13, 0x54}, {0x1C, 0x04}, {0x02, 0x9C}, {0x00, 0x00}
};

static mpu4_chr_table quidgrid_data[64] = {
	{0x00, 0x00}, {0x1A, 0x64}, {0x04, 0x64}, {0x10, 0x24}, {0x18, 0x64}, {0x0F, 0x64}, {0x13, 0x24}, {0x1B, 0x64},
	{0x03, 0x74}, {0x07, 0x54}, {0x17, 0x84}, {0x1D, 0xA4}, {0x36, 0x24}, {0x35, 0x24}, {0x2B, 0x64}, {0x28, 0x24},
	{0x39, 0xE4}, {0x21, 0x64}, {0x22, 0x74}, {0x25, 0x44}, {0x2C, 0x34}, {0x29, 0x04}, {0x31, 0x24}, {0x34, 0x24},
	{0x0A, 0x64}, {0x1F, 0x64}, {0x06, 0x74}, {0x0E, 0x44}, {0x1C, 0x64}, {0x12, 0x24}, {0x1E, 0x64}, {0x0D, 0x24},
	{0x14, 0x24}, {0x0A, 0x64}, {0x19, 0xE4}, {0x15, 0x24}, {0x06, 0x74}, {0x0F, 0x44}, {0x08, 0x34}, {0x1B, 0x14},
	{0x1E, 0x04}, {0x04, 0x64}, {0x01, 0x60}, {0x0C, 0x24}, {0x18, 0x64}, {0x1A, 0x64}, {0x11, 0x24}, {0x0B, 0x64},
	{0x03, 0x74}, {0x17, 0x04}, {0x10, 0x24}, {0x1D, 0xE4}, {0x0E, 0x64}, {0x07, 0x74}, {0x12, 0x04}, {0x09, 0x34},
	{0x0D, 0x04}, {0x1F, 0x64}, {0x16, 0x24}, {0x05, 0x64}, {0x13, 0x24}, {0x1C, 0x64}, {0x02, 0x74}, {0x00, 0x00}
};

static mpu4_chr_table blank_data[72] = {
{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},
{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},
{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},
{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},
{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},
{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},
{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},
{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},
{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},{0xff, 0xff},
};


#if 0
static const bwb_chr_table prizeinv_data1[5] = {
//This is all wrong, but without BwB Vid booting,
//I can't find the right values. These should be close though
	{0x67},{0x17},{0x0f},{0x24},{0x3c},
};
#endif

static mpu4_chr_table prizeinv_data[8] = {
{0xEF, 0x02},{0x81, 0x00},{0xCE, 0x00},{0x00, 0x2e},
{0x06, 0x20},{0xC6, 0x0f},{0xF8, 0x24},{0x8E, 0x3c},
};

DRIVER_INIT_MEMBER(mpu4vid_state,adders)
{
	m_reels = 0;//currently no hybrid games
	m_current_chr_table = adders_data;
}

DRIVER_INIT_MEMBER(mpu4vid_state,crmaze)
{
	m_reels = 0;//currently no hybrid games
	m_current_chr_table = crmaze_data;
}

DRIVER_INIT_MEMBER(mpu4vid_state,crmazea)
{
	m_reels = 0;//currently no hybrid games
	m_current_chr_table = crmazea_data;
}

DRIVER_INIT_MEMBER(mpu4vid_state,crmaze2)
{
	m_reels = 0;//currently no hybrid games
	m_current_chr_table = crmaze2_data;
}

DRIVER_INIT_MEMBER(mpu4vid_state,crmaze2a)
{
	m_reels = 0;//currently no hybrid games
}

DRIVER_INIT_MEMBER(mpu4vid_state,crmaze3)
{
	m_reels = 0;//currently no hybrid games
	m_reel_mux = FLUTTERBOX;
	m_current_chr_table = crmaze3_data;
}

DRIVER_INIT_MEMBER(mpu4vid_state,crmaze3a)
{
	m_reels = 0;//currently no hybrid games
	m_reel_mux = FLUTTERBOX;
	m_current_chr_table = crmaze3a_data;
}

DRIVER_INIT_MEMBER(mpu4vid_state,mating)
{
	m_reels = 0;//currently no hybrid games

	m_current_chr_table = mating_data;
}

DRIVER_INIT_MEMBER(mpu4vid_state,skiltrek)
{
	m_reels = 0;//currently no hybrid games
	m_current_chr_table = skiltrek_data;
}

DRIVER_INIT_MEMBER(mpu4vid_state,timemchn)
{
	m_reels = 0;//currently no hybrid games
	m_current_chr_table = timemchn_data;
}

DRIVER_INIT_MEMBER(mpu4vid_state,strikeit)
{
	m_reels = 0;//currently no hybrid games
	m_current_chr_table = strikeit_data;
}

DRIVER_INIT_MEMBER(mpu4vid_state,turnover)
{
	m_reels = 0;//currently no hybrid games
	m_current_chr_table = turnover_data;
}

DRIVER_INIT_MEMBER(mpu4vid_state,eyesdown)
{
	m_reels = 0;//currently no hybrid games
	m_current_chr_table = eyesdown_data;
}

DRIVER_INIT_MEMBER(mpu4vid_state,quidgrid)
{
	m_reels = 0;//currently no hybrid games
	m_current_chr_table = quidgrid_data;
}

DRIVER_INIT_MEMBER(mpu4vid_state,prizeinv)
{
	m_reels = 0;//currently no hybrid games
	m_current_chr_table = prizeinv_data;
}

static const bwb_chr_table cybcas_data1[5] = {
//Magic number 724A

// PAL Codes
// 0   1   2  3  4  5  6  7  8
// ??  ?? 20 0F 24 3C 36 27 09

	{0x67},{0x17},{0x0f},{0x24},{0x3c},
};

static mpu4_chr_table cybcas_data[8] = {
{0xEF, 0x02},{0x81, 0x00},{0xCE, 0x00},{0x00, 0x2e},
{0x06, 0x20},{0xC6, 0x0f},{0xF8, 0x24},{0x8E, 0x3c},
};

DRIVER_INIT_MEMBER(mpu4vid_state,cybcas)
{
	//no idea what this should be, use blues boys table for now
	m_bwb_chr_table1 = cybcas_data1;
	m_current_chr_table = cybcas_data;
}


void mpu4vid_state::mpu4vid_char_cheat( int address)
{
	UINT8* cheattable = memregion( "video" )->base()+address;
	m_current_chr_table = blank_data;
	for (int i=0;i<72;i++)
	{
		m_current_chr_table[i].response = cheattable++[0];
		m_current_chr_table[i].call = cheattable++[0];
	}
}

DRIVER_INIT_MEMBER(mpu4vid_state,v4barqst)
{
	mpu4vid_char_cheat(0x154);
}

DRIVER_INIT_MEMBER(mpu4vid_state,v4barqst2)
{
	mpu4vid_char_cheat(0x15c);
}

DRIVER_INIT_MEMBER(mpu4vid_state,v4wize)
{
	mpu4vid_char_cheat(0x16c);
}

DRIVER_INIT_MEMBER(mpu4vid_state,v4cmazeb)
{
	mpu4vid_char_cheat(0x4c6);
}

DRIVER_INIT_MEMBER(mpu4vid_state,v4opt3)
{
	mpu4vid_char_cheat(0x164);
}


#define VID_BIOS \
	ROM_LOAD("vid.p1",  0x00000, 0x10000,  CRC(e996bc18) SHA1(49798165640627eb31024319353da04380787b10))

ROM_START( v4bios )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS
ROM_END

ROM_START( v4psi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("in220mpu4.p1",  0x00000, 0x04000,  CRC(75ff4b1f) SHA1(a3adaad9a91c30fe6ff42dc2003c34a199b28807) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "in14d.p1", 0x0000, 0x004000, CRC(cb9a093a) SHA1(225ca4f191f64f6ca3ed6bc7b58819a893fdd36a) )
	ROM_LOAD( "in20d.p1", 0x0000, 0x004000, CRC(e86e62a0) SHA1(97b0d41fa688cdd86bd6a1ef65cf143a34e23fac) )
	ROM_LOAD( "in214.p1", 0x0000, 0x004000, CRC(4fb02448) SHA1(c2f2413a460012e3aadf7effbf8a33b40bc02df1) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "in2-20p1.1",  0x000000, 0x10000, CRC(f34d9001) SHA1(2bae06f4a5a5510b15b918261ecb0de9e34a6b53) )
	ROM_LOAD16_BYTE( "in2-20p1.2",  0x000001, 0x10000, CRC(1dc931b4) SHA1(c46626183edd52c7938c5edee2395aacb49e0730) )
	ROM_LOAD16_BYTE( "in2-20p1.3",  0x020000, 0x10000, CRC(107aa448) SHA1(7b3d4053aaae3b97136cddefbc9edd5e61713ff7) )
	ROM_LOAD16_BYTE( "in2-20p1.4",  0x020001, 0x10000, CRC(04933278) SHA1(97462aef782f7fe82b60f4bddcad0e6a6b50f3df) )
ROM_END

ROM_START( v4psia )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("psi12m.p1",  0x00000, 0x04000,  CRC(560b2085) SHA1(5dccede70e228d896ff11ff861c9f32b895e807d) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "psi12.p1",  0x000000, 0x10000, CRC(9a3972bd) SHA1(72c747c16b5a31a7efcf561c2f9ce151a559b1ac) )
	ROM_LOAD16_BYTE( "psi12.p2",  0x000001, 0x10000, CRC(1a5da4f4) SHA1(f926cb650e2014a771621a497d4cc228c18f2979) )
	ROM_LOAD16_BYTE( "psi12.p3",  0x020000, 0x10000, CRC(cab2e50b) SHA1(f5ba3ccef87bb7afc59e6aa38c364a492d11b0a2) )
	ROM_LOAD16_BYTE( "psi12.p4",  0x020001, 0x10000, CRC(83781f1a) SHA1(a21d2e0ce6add058b5c6efad778a14128842b71b) )
ROM_END

ROM_START( v4psib )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "psi20int", 0x0000, 0x004000, CRC(20ee94b4) SHA1(4c77be34e20a843add2bba23c092fd5bba90bc45) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "psi20p1", 0x000000, 0x010000, CRC(1795fe9c) SHA1(c93e0815d76ab4e55e7659d5c332db9d847a00b0) )
	ROM_LOAD16_BYTE( "psi20p2", 0x000001, 0x010000, CRC(c1497581) SHA1(f6b64df97d90aa13ed7b5d42608b9014326a880b) )
	ROM_LOAD16_BYTE( "psi20p3", 0x020000, 0x10000, NO_DUMP )
	ROM_LOAD16_BYTE( "psi20p4", 0x020001, 0x010000, CRC(45ff4d46) SHA1(f3d402fad950adb366e4deb67b4038c0febae004) )
ROM_END


ROM_START( v4blox )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("blxv___2.0_0",  0x00000, 0x04000, CRC(b399b85e) SHA1(d36391fee4e3126754d6a0fa5f52fe05bc676930) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "blxv___2.0_1",  0x000000, 0x10000, CRC(68134eda) SHA1(b324ae1243164c7be7f5eced7ff93760e2176a4e) )
	ROM_LOAD16_BYTE( "blxv___2.0_2",  0x000001, 0x10000, CRC(6b1f8588) SHA1(e8a443f062555f1ed228e1bfed2031927bcd7015) )
	ROM_LOAD16_BYTE( "blxv___2.0_3",  0x020000, 0x10000, CRC(c62d704b) SHA1(bea0c9519063f1601f70372ccb49fb892fbd6e76) )
	ROM_LOAD16_BYTE( "blxv___2.0_4",  0x020001, 0x10000, CRC(e431471a) SHA1(cf90dc48be3bc5e3c5a8efea5818dbc15fa442e9) )
	ROM_LOAD16_BYTE( "blxv___2.0_5",  0x040001, 0x10000, CRC(98ac6bc7) SHA1(9575014ba21fa4330138a34f53e13d30d312bc8b) )
	ROM_LOAD16_BYTE( "blxv___2.0_6",  0x040001, 0x10000, CRC(a3d92b5b) SHA1(1e7042d5eae4a19a01a3ef7d806c434886dc9f4d) )
ROM_END

ROM_START( v4bloxd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("blxv_d_2.0_0",  0x00000, 0x04000, CRC(2e0891e1) SHA1(3e8ffd0d41227a8a1e311ca0c0bde7590e06dfbd) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "blxv___2.0_1",  0x000000, 0x10000, CRC(68134eda) SHA1(b324ae1243164c7be7f5eced7ff93760e2176a4e) )
	ROM_LOAD16_BYTE( "blxv___2.0_2",  0x000001, 0x10000, CRC(6b1f8588) SHA1(e8a443f062555f1ed228e1bfed2031927bcd7015) )
	ROM_LOAD16_BYTE( "blxv___2.0_3",  0x020000, 0x10000, CRC(c62d704b) SHA1(bea0c9519063f1601f70372ccb49fb892fbd6e76) )
	ROM_LOAD16_BYTE( "blxv___2.0_4",  0x020001, 0x10000, CRC(e431471a) SHA1(cf90dc48be3bc5e3c5a8efea5818dbc15fa442e9) )
	ROM_LOAD16_BYTE( "blxv___2.0_5",  0x040001, 0x10000, CRC(98ac6bc7) SHA1(9575014ba21fa4330138a34f53e13d30d312bc8b) )
	ROM_LOAD16_BYTE( "blxv___2.0_6",  0x040001, 0x10000, CRC(a3d92b5b) SHA1(1e7042d5eae4a19a01a3ef7d806c434886dc9f4d) )
ROM_END

ROM_START( v4tetrs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("tetris22.p0",  0x00000, 0x04000, CRC(b711c7ae) SHA1(767b17ddf9021fdf79ff6c52f04a5d8ea60cf30e) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "tetv1int", 0x0000, 0x004000, CRC(98de975d) SHA1(5b4fc06aa8008d3967c68f364c47f8377a1ba9df) )


	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "tetris22.p1",  0x000000, 0x10000, CRC(e81dd182) SHA1(28b460224abf6fe24b637542ccd1c84040674555) )
	ROM_LOAD16_BYTE( "tetris22.p2",  0x000001, 0x10000, CRC(68aa4f15) SHA1(4e4511a64391fc64e5f5b7ccb46a78fd2e1d94d6) )
	ROM_LOAD16_BYTE( "tetris22.p3",  0x020000, 0x10000, CRC(b38b4763) SHA1(d28e77fdd6869cb5b5ec40ed1f300a2a947e0482) )
	ROM_LOAD16_BYTE( "tetris22.p4",  0x020001, 0x10000, CRC(1649f604) SHA1(ca4ac303391a0969d41c8f988b8e81cfcee1a21c) )
	ROM_LOAD16_BYTE( "tetris22.p5",  0x040001, 0x10000, CRC(02859676) SHA1(5293c767021a6b5253eecab0b0568aa082ea7084) )
	ROM_LOAD16_BYTE( "tetris22.p6",  0x040001, 0x10000, CRC(40d24c82) SHA1(7ac3cf148af84ad93eaf11ce3420abbe45d986e2) )
ROM_END

/* Vegas Poker Prototype dumped by HIGHWAYMAN */
ROM_START( v4vgpok )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("comms-v2.0.bin",  0x00000, 0x10000,  CRC(1717581f) SHA1(40f8cae39a2ab0c89d2bbfd8a37725aaae229c96))

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("video-1.bin",  0x000000, 0x010000,  CRC(7ba2396c) SHA1(eb24b802b984315fc2eba4f15c208e2c1925c1c8))
	ROM_LOAD16_BYTE("video-2.bin",  0x000001, 0x010000,  CRC(4f9e830b) SHA1(f17bebb289c3620bf4c88b2b358a9dab87ac214f))
	ROM_LOAD16_BYTE("video-3.bin",  0x020000, 0x010000,  CRC(607e0baa) SHA1(9f64a46ef3b9a854e939b5e7f0d1e6e925735922))
	ROM_LOAD16_BYTE("video-4.bin",  0x020001, 0x010000,  CRC(2019f5d3) SHA1(d183b3b92d03be9f9d57b5df1a621cbfe955ed93))
	ROM_LOAD16_BYTE("video-5.bin",  0x040000, 0x010000,  CRC(c029202e) SHA1(b08bb2678c2ff62a58ef67d5440c326d0fadc34e))
	ROM_LOAD16_BYTE("video-6.bin",  0x040001, 0x010000,  CRC(3287ae4e) SHA1(3b05a036de3ca7ec644bfbf04934e44e631d1e28))
	ROM_LOAD16_BYTE("video-7.bin",  0x060000, 0x010000,  CRC(231cf163) SHA1(02b28ef0e1661a82d0fba2ecc5474c79651fa9e7))
	ROM_LOAD16_BYTE("video-8.bin",  0x060001, 0x010000,  CRC(076efdc8) SHA1(bef0a1d8f0e7486ee5dc7407ce5c96854cefa5cf))

	/*characteriser chip*/
	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "ic18-palce16v8h.bin", 0x0000, 0x0117, CRC(f144738a) SHA1(8e563aaff07ad23864e3019737cbdf4490b70d8f) )
ROM_END


ROM_START( v4reno )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rr_27s__.a60",  0x00000, 0x10000,  CRC(44c9ff47) SHA1(93a3155144b233c113aa3b49bd4eb5969e400a68))

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )

	ROM_REGION( 0x800000, "altvideo", 0 ) // alt revs of the video roms?
	ROM_LOAD16_BYTE( "reno reels 5-1",    0x000000, 0x080000,  CRC(9ebd0eaf) SHA1(3d326509240fe8a83df9d2369f184838bee2b407) )
	ROM_LOAD16_BYTE( "reno reels 5-2",    0x000001, 0x080000,  CRC(1cbcd9b5) SHA1(989d64e10c67dab7d20229e5c63d24111d556138) )
	ROM_LOAD16_BYTE( "reno_71_27c040.bin",0x000000, 0x080000,  CRC(c1125c51) SHA1(a461049cd3768096c03f3a5149cdef31d0ab447e) )
	ROM_LOAD16_BYTE( "reno_72_27c040.bin",0x000001, 0x080000,  CRC(31773743) SHA1(e1245f6b35c9049b3d1478e93fb1b6cfff34733e) )
	ROM_LOAD16_BYTE( "rr8p1",             0x000000, 0x080000,  CRC(68992dd3) SHA1(75ab1cd02ac627b6191e9b61ee7c072029becaeb) )
	ROM_LOAD16_BYTE( "rr8p2",             0x000001, 0x080000,  CRC(b859020e) SHA1(811ccac82d022ceccc83f1bf6c6b4de6cc313e14) )
	ROM_LOAD16_BYTE( "rr______.8_1",      0x000000, 0x080000,  CRC(eca43ed4) SHA1(e2e4e5d3d4b659ddd74c120316b9658708e188f1) )
	ROM_LOAD16_BYTE( "rr______.8_2",      0x000001, 0x080000,  CRC(c3f25586) SHA1(7335708a7d90c7fbd0088bb6ee5ce0255b9b18ab) )

	ROM_REGION( 0x800000, "altmain", 0 ) // alt revs of MPU4 interface ROM
	ROM_LOAD("rri20s__.a_0",    0x00000, 0x10000,   CRC(0fb9686a) SHA1(a403d4424897fcdc343b277aa0caa032ed970747) )
	ROM_LOAD("rrixes__.a_0",    0x00000, 0x10000,   CRC(3f055fa1) SHA1(ee6561d6849e5150d7b7b5585e8ed8176e706aeb) )
	ROM_LOAD("rrv8ss",          0x00000, 0x10000,   CRC(a37383a5) SHA1(6c2563967546d810f2c50aa9a269bb1369014c18) )
	ROM_LOAD("rr_20ab_.a_0",    0x00000, 0x10000,   CRC(6da308aa) SHA1(c1f418592942a9f68aac9a5a6f91911b96861d48) )
	ROM_LOAD("rr_20a_p.a_0",    0x00000, 0x10000,   CRC(0dc6b163) SHA1(5a666dec859807cab6478b06f38473997fe49cd6) )
	ROM_LOAD("rr_20a__.a_0",    0x00000, 0x10000,   CRC(9b279f39) SHA1(9e9e80fdc8517a314bac15a5087d7619a84c1e00) )
	ROM_LOAD("rr_20bgp.a_0",    0x00000, 0x10000,   CRC(7175112b) SHA1(799c822a6dabcf2a7d67b2ef81273a0fba6cf3d9) )
	ROM_LOAD("rr_20bg_.a_0",    0x00000, 0x10000,   CRC(e7943f71) SHA1(490af3fc7d3506ca9c5c049a6fcffb856bf28d1e) )
	ROM_LOAD("rr_20btp.a_0",    0x00000, 0x10000,   CRC(c73e1c28) SHA1(37c5b984311439906cae2ba48aab249caeb1f2ab) )
	ROM_LOAD("rr_20bt_.a_0",    0x00000, 0x10000,   CRC(51df3272) SHA1(c9cc06556e79e09b9b3cd9816b6f7dde92dadfe7) )
	ROM_LOAD("rr_20sbp.a_0",    0x00000, 0x10000,   CRC(0ea4be35) SHA1(2e3950bcc01f4c1ce53873b552cb156a91c74e85) )
	ROM_LOAD("rr_20sb_.a_0",    0x00000, 0x10000,   CRC(9845906f) SHA1(693e480d548482c073644513803ddd4e5ed0694c) )
	ROM_LOAD("rr_20s__.a_0",    0x00000, 0x10000,   CRC(6ec107fc) SHA1(46ac2bbb19ff4d562fa2e0029e9831be0bec5def) )
	ROM_LOAD("rr_27sd_.a60",    0x00000, 0x10000,   CRC(0f6c18e6) SHA1(23f07d1ed2340e73abcf6b86581bc5dd768dbab5) )
	ROM_LOAD("rr_37sd_.a60",    0x00000, 0x10000,   CRC(807e73c8) SHA1(202d621cead9b2af8fef12ea0d07a6fce6262518) )
	ROM_LOAD("rr_37s__.a60",    0x00000, 0x10000,   CRC(cbdb9469) SHA1(bc802b4c15451feebc332944f6bc09c7fb20ea20) )
	ROM_LOAD("rr_x7sd_.a60",    0x00000, 0x10000,   CRC(3fd02f2d) SHA1(49ae60e8bdc6681482272d31eefc0098cc6c9667) )
	ROM_LOAD("rr_x7s__.a60",    0x00000, 0x10000,   CRC(7475c88c) SHA1(0425e722321d4f365f6e90de5159721ac8a9d0d2) )
	ROM_LOAD("rr_xeadp.a_0",    0x00000, 0x10000,   CRC(76df6109) SHA1(fbc76a9612a48f1b589e43e2f920459ed6c32c57) )
	ROM_LOAD("rr_xead_.a_0",    0x00000, 0x10000,   CRC(e03e4f53) SHA1(17b4bdf82393aacf74765f04fc0d9b1f683114cc) )
	ROM_LOAD("rr_xea_p.a_0",    0x00000, 0x10000,   CRC(3d7a86a8) SHA1(98bb8b2c0705219536720eef404c7bbc14a85793) )
	ROM_LOAD("rr_xea__.a_0",    0x00000, 0x10000,   CRC(ab9ba8f2) SHA1(52b77aa66980fa552d286225919fca9910f48326) )


	ROM_REGION( 0x800000, "altsnd", 0 ) // alt revs of the sound roms?
	ROM_LOAD( "reno reels sound 1a",  0x000000, 0x080000,  CRC(a8b7bba7) SHA1(5fa3512a6fdcf512fafa6261b3a99922a00d6874) ) // bad dump, only one ROM?

ROM_END



ROM_START( v4redhtp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rp_20s__.3_0",  0x00000, 0x10000,  CRC(b7d02d22) SHA1(f9da1c6dde064bc39d0c48a165dac7acde933397))

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x800000, "altvideo", 0 ) // alt revs of the video roms?
	ROM_LOAD16_BYTE( "rp_05___.2_5", 0x040000, 0x010000, CRC(cc79187b) SHA1(b2e556fd7a1667203dcb196b1dc2d89bff785675) )
	ROM_LOAD16_BYTE( "rp_05___.2_6", 0x040001, 0x010000, CRC(57d1cf7b) SHA1(c8d6f4d0e8a5a383c47300e8d56e13d62295f60f) )
	ROM_LOAD16_BYTE( "rhp1.6p5", 0x040000, 0x010000, CRC(750436a1) SHA1(006a31fc5c22969bd79dbc54e618348ad7832ac7) )
	ROM_LOAD16_BYTE( "rhp1.6p6", 0x040001, 0x010000, CRC(d78839c2) SHA1(e82b769cba4b8d50dcf5c301c03d4ca66e893f70) )
	ROM_LOAD16_BYTE( "redhotpokervideoboardp5.bin", 0x040000, 0x010000, CRC(d36189b7) SHA1(7757ce9879754d4b8a450ba1f6067c17c151c13c) )
	ROM_LOAD16_BYTE( "redhotpokervideoboardp6.bin", 0x040001, 0x010000, CRC(c89d164d) SHA1(0cf33db0f85958251624dd7bc2c3024814489040) )



	ROM_REGION( 0x800000, "altmain", 0 ) // alt revs of MPU4 interface ROM
	ROM_LOAD( "rhpmpu4.p1", 0x0000, 0x010000, CRC(e614757f) SHA1(96e825bedfb1715aa9b5d131e5b492247b17b725) )
	ROM_LOAD( "redhotpoker20_mpu4_interface.bin", 0x0000, 0x010000, CRC(cafbcc82) SHA1(662df1f6409f9e20fac07c07d08eae3ea8ba362a) )
	ROM_LOAD( "rhp20ac6", 0x0000, 0x010000, CRC(d6a842b4) SHA1(94f6cc6a9e0efa8a2eeee14f981f9d2407dfb092) )
	ROM_LOAD( "rhpint", 0x0000, 0x010000, CRC(d60e7e30) SHA1(174f69ff2e76837455c107055b06f98875033b5e) )
	ROM_LOAD( "rp_05a__.2_0", 0x0000, 0x010000, CRC(970fa9ad) SHA1(77550948aae171eae715e077cdbeffaeeadb2436) )
	ROM_LOAD( "rp_05ab_.2_0", 0x0000, 0x010000, CRC(618b3e3e) SHA1(19f7c83957f1d0b36f62607d314a310748b3c84a) )
	ROM_LOAD( "rp_05ad_.2_0", 0x0000, 0x010000, CRC(dcaa4e0c) SHA1(2a037722f5e951513c233227448c3a2e55de8ef9) )
	ROM_LOAD( "rp_05ak_.2_0", 0x0000, 0x010000, CRC(2a2ed99f) SHA1(45215a140b4e9d6b190fff0b89fcbeffc054d732) )
	ROM_LOAD( "rp_05s__.2_0", 0x0000, 0x010000, CRC(62e93168) SHA1(8287eee2d6ac4cc447ce6652de24dfe056015ef3) )
	ROM_LOAD( "rp_05sb_.2_0", 0x0000, 0x010000, CRC(946da6fb) SHA1(d406f0bea0940b6910dc923ded0c89db9c1f3c61) )
	ROM_LOAD( "rp_05sd_.2_0", 0x0000, 0x010000, CRC(294cd6c9) SHA1(4f2890c58cfa6d91ddc73d9098d8e45e79f410c3) )
	ROM_LOAD( "rp_05sk_.2_0", 0x0000, 0x010000, CRC(dfc8415a) SHA1(5d9f946dc0307d16c9d90856d4ae2b4c8c5013a6) )
	ROM_LOAD( "rp_10a__.3_0", 0x0000, 0x010000, CRC(90f58c6f) SHA1(c9b412172a6ef361407f486a8fb134ac68fe31e5) )
	ROM_LOAD( "rp_10ab_.3_0", 0x0000, 0x010000, CRC(66711bfc) SHA1(ee51215479e57646e9490b6e898ca5092172879a) )
	ROM_LOAD( "rp_10ad_.3_0", 0x0000, 0x010000, CRC(db506bce) SHA1(1f152e58a3a5251aa00f890df9a9930a72d84db6) )
	ROM_LOAD( "rp_10ak_.3_0", 0x0000, 0x010000, CRC(2dd4fc5d) SHA1(b1e908ef70c98e7467a5f7d3ec26e7e6bfa1c475) )
	ROM_LOAD( "rp_10bg_.3_0", 0x0000, 0x010000, CRC(ec462c27) SHA1(88379585055f245c8d84bd1295e7f5b6555ad8c3) )
	ROM_LOAD( "rp_10s__.3_0", 0x0000, 0x010000, CRC(651314aa) SHA1(f11cd9c9419511e179acd2883d83cb475eb4d761) )
	ROM_LOAD( "rp_10sb_.3_0", 0x0000, 0x010000, CRC(93978339) SHA1(7c943fe44103d0ac276b4478e20709d6ab0b07d5) )
	ROM_LOAD( "rp_10sd_.3_0", 0x0000, 0x010000, CRC(2eb6f30b) SHA1(5aa3161888ad50bb9c1945bff32e6d8552ab4c89) )
	ROM_LOAD( "rp_10sk_.3_0", 0x0000, 0x010000, CRC(d8326498) SHA1(89ab78991ea72f5186488714e8b7daeafa1e9497) )
	ROM_LOAD( "rp_20a__.3_0", 0x0000, 0x010000, CRC(4236b5e7) SHA1(1bd73c8fd20a176c25288becb6f07b7f0447ede3) )
	ROM_LOAD( "rp_20ab_.3_0", 0x0000, 0x010000, CRC(b4b22274) SHA1(f5f4d4f88d74454e6857118dc3eba90defd15e50) )
	ROM_LOAD( "rp_20ad_.3_0", 0x0000, 0x010000, CRC(09935246) SHA1(e82749b575f22373a3a7f24a8f68980e8040ac9d) )
	ROM_LOAD( "rp_20ak_.3_0", 0x0000, 0x010000, CRC(ff17c5d5) SHA1(1aa1ca348cd8828e616a4781704445d04800b970) )
	ROM_LOAD( "rp_20bg_.3_0", 0x0000, 0x010000, CRC(3e8515af) SHA1(3df382818b1d527ab4037a0dca24ba28227ab6a4) )
	ROM_LOAD( "rp_20sb_.3_0", 0x0000, 0x010000, CRC(4154bab1) SHA1(b3d5dcdd276e3b2fc1f5d66a870e85c5064f0f5b) )
	ROM_LOAD( "rp_20sd_.3_0", 0x0000, 0x010000, CRC(fc75ca83) SHA1(9cf7178c990151c7711c8bca68c0dd85325808ab) )
	ROM_LOAD( "rp_20sk_.3_0", 0x0000, 0x010000, CRC(0af15d10) SHA1(df1367176f8f2efcc21cf5c841125f9553ad0c8a) )
	ROM_LOAD( "rp_xca__.3_0", 0x0000, 0x010000, CRC(728a822c) SHA1(b23e4893325c2dbeb8f72114dd1e5c3f06a786a8) )
	ROM_LOAD( "rp_xcab_.3_0", 0x0000, 0x010000, CRC(840e15bf) SHA1(9eba10eeebdf1c801186ae2c0206292da28ab7c5) )
	ROM_LOAD( "rp_xcad_.3_0", 0x0000, 0x010000, CRC(392f658d) SHA1(c730a5e54faab1946554af9e6b9e08dc50bbe9d6) )
	ROM_LOAD( "rp_xcak_.3_0", 0x0000, 0x010000, CRC(cfabf21e) SHA1(9cec91ea209d250ba885552579044b5ce9d730bd) )
	ROM_LOAD( "rp_xcbg_.3_0", 0x0000, 0x010000, CRC(0e392264) SHA1(aeb5a4df61b3e67100912ede2f5e127d413b656c) )
	ROM_LOAD( "rp_xcs__.3_0", 0x0000, 0x010000, CRC(876c1ae9) SHA1(9ea1b2a28f2b87089500dd25e6dc1450be43acbc) )
	ROM_LOAD( "rp_xcsb_.3_0", 0x0000, 0x010000, CRC(71e88d7a) SHA1(81d38e598514a5a1b9bca0131232b18c3829edfb) )
	ROM_LOAD( "rp_xcsd_.3_0", 0x0000, 0x010000, CRC(ccc9fd48) SHA1(1069b945b5312eb01139b7671901212f8f65e3bf) )
	ROM_LOAD( "rp_xcsk_.3_0", 0x0000, 0x010000, CRC(3a4d6adb) SHA1(01dfead74d40af9a5a4a95781b83d078cfb92ac9) )
	ROM_LOAD( "rpi10___.3_0", 0x0000, 0x010000, CRC(046b7b3c) SHA1(71f74153dbdd52036a55fc0a217120dee84ca230) )
ROM_END

ROM_START( v4cmaze )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "crys.p1", 0x000000, 0x80000,  CRC(40fbde50) SHA1(91bd21c0aaffb9c9b89a114affbc485b12ae9bb4) )
	ROM_LOAD16_BYTE( "cry.p2",  0x000001, 0x80000,  CRC(fa7d006f) SHA1(ecc03b4d7a4089feccc53ad05313c35b33e061d7) )
	ROM_LOAD16_BYTE( "cry.p3",  0x100000, 0x80000,  CRC(e8cf8203) SHA1(e9f42e5c18b97807f51284ad2416346578ed73c4) )
	ROM_LOAD16_BYTE( "cry.p4",  0x100001, 0x80000,  CRC(7b036151) SHA1(7b0040c296059b1e1798ddedf0ecb4582d67ee70) )
	ROM_LOAD16_BYTE( "cry.p5",  0x200000, 0x80000,  CRC(48f17b20) SHA1(711c46fcfd86ded8ff7da883188d70560d20e42f) )
	ROM_LOAD16_BYTE( "cry.p6",  0x200001, 0x80000,  CRC(2b3d9a97) SHA1(7468fffd90d840d245a70475b42308f1e48c5017) )
	ROM_LOAD16_BYTE( "cry.p7",  0x300000, 0x80000,  CRC(20f73433) SHA1(593b40ac17591ac312ad41b4d3a5772626137bba) )
	ROM_LOAD16_BYTE( "cry.p8",  0x300001, 0x80000,  CRC(835da1f2) SHA1(f93e075916d370466832871410591570ad7b9f3b) )
	ROM_LOAD16_BYTE( "cry.p9",  0x400000, 0x80000,  CRC(c0e442ee) SHA1(a3877b200538642fe2bc96cfe8b33f04d8a82a98) )
	ROM_LOAD16_BYTE( "cry.p10", 0x400001, 0x80000,  CRC(500172fa) SHA1(d83a37612daa79ba8425fdb28f39b8324b5736b6) )
ROM_END

ROM_START( v4cmazedat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cryd.p1", 0x000000, 0x80000,  CRC(b245f661) SHA1(85a8baca8797fea74bc36eea077be56bde8e54a9) )
	ROM_LOAD16_BYTE( "cry.p2",  0x000001, 0x80000,  CRC(fa7d006f) SHA1(ecc03b4d7a4089feccc53ad05313c35b33e061d7) )
	ROM_LOAD16_BYTE( "cry.p3",  0x100000, 0x80000,  CRC(e8cf8203) SHA1(e9f42e5c18b97807f51284ad2416346578ed73c4) )
	ROM_LOAD16_BYTE( "cry.p4",  0x100001, 0x80000,  CRC(7b036151) SHA1(7b0040c296059b1e1798ddedf0ecb4582d67ee70) )
	ROM_LOAD16_BYTE( "cry.p5",  0x200000, 0x80000,  CRC(48f17b20) SHA1(711c46fcfd86ded8ff7da883188d70560d20e42f) )
	ROM_LOAD16_BYTE( "cry.p6",  0x200001, 0x80000,  CRC(2b3d9a97) SHA1(7468fffd90d840d245a70475b42308f1e48c5017) )
	ROM_LOAD16_BYTE( "cry.p7",  0x300000, 0x80000,  CRC(20f73433) SHA1(593b40ac17591ac312ad41b4d3a5772626137bba) )
	ROM_LOAD16_BYTE( "cry.p8",  0x300001, 0x80000,  CRC(835da1f2) SHA1(f93e075916d370466832871410591570ad7b9f3b) )
	ROM_LOAD16_BYTE( "cry.p9",  0x400000, 0x80000,  CRC(c0e442ee) SHA1(a3877b200538642fe2bc96cfe8b33f04d8a82a98) )
	ROM_LOAD16_BYTE( "cry.p10", 0x400001, 0x80000,  CRC(500172fa) SHA1(d83a37612daa79ba8425fdb28f39b8324b5736b6) )
ROM_END

ROM_START( v4cmazea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "am1z.p1",  0x000000, 0x80000,  CRC(d2fdda6d) SHA1(96c27dedc3cf1dd478e6169200844d418a276f14) )
	ROM_LOAD16_BYTE( "am1z.p2",  0x000001, 0x80000,  CRC(1637170f) SHA1(fd17a0e7794f01bf4ad7a16b185f87cb060c70ab) )
	ROM_LOAD16_BYTE( "am1g.p1",  0x100000, 0x80000,  CRC(e8cf8203) SHA1(e9f42e5c18b97807f51284ad2416346578ed73c4) )
	ROM_LOAD16_BYTE( "am1g.p2",  0x100001, 0x80000,  CRC(7b036151) SHA1(7b0040c296059b1e1798ddedf0ecb4582d67ee70) )
	ROM_LOAD16_BYTE( "am1g.p3",  0x200000, 0x80000,  CRC(48f17b20) SHA1(711c46fcfd86ded8ff7da883188d70560d20e42f) )
	ROM_LOAD16_BYTE( "am1g.p4",  0x200001, 0x80000,  CRC(2b3d9a97) SHA1(7468fffd90d840d245a70475b42308f1e48c5017) )
	ROM_LOAD16_BYTE( "am1g.p5",  0x300000, 0x80000,  CRC(68286bb1) SHA1(c307e3ad1e0fd92314216c8e554aafa949559452) )
	ROM_LOAD16_BYTE( "am1g.p6",  0x300001, 0x80000,  CRC(a6b498ad) SHA1(117e1a4ec7e2d3c7d530c5a56cbc1d24b0ddc747) )
	ROM_LOAD16_BYTE( "am1g.p7",  0x400000, 0x80000,  CRC(15882699) SHA1(b29a331e51a37554323b91330a7c2b62b33a943a) )
	ROM_LOAD16_BYTE( "am1g.p8",  0x400001, 0x80000,  CRC(6f0f855b) SHA1(ab411d1af0f88049a6c435bafd4b1fa63f5519b1) )
ROM_END

ROM_START( v4cmazeb ) /* Was in a romset called 'Mad Money!' */
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cm1.2p1", 0x000000, 0x040000, CRC(481f2fe2) SHA1(7bff9476dbd8c0812a51cd648acf11ca42789e54) )
	ROM_LOAD16_BYTE( "cm1.2p2", 0x000001, 0x040000, CRC(6e6dc121) SHA1(05b29ea935d54aa97279e0a80265412747a92a10) )
	ROM_LOAD16_BYTE( "cry.p3",  0x100000, 0x80000,  CRC(e8cf8203) SHA1(e9f42e5c18b97807f51284ad2416346578ed73c4) )
	ROM_LOAD16_BYTE( "cry.p4",  0x100001, 0x80000,  CRC(7b036151) SHA1(7b0040c296059b1e1798ddedf0ecb4582d67ee70) )
	ROM_LOAD16_BYTE( "cry.p5",  0x200000, 0x80000,  CRC(48f17b20) SHA1(711c46fcfd86ded8ff7da883188d70560d20e42f) )
	ROM_LOAD16_BYTE( "cry.p6",  0x200001, 0x80000,  CRC(2b3d9a97) SHA1(7468fffd90d840d245a70475b42308f1e48c5017) )
	ROM_LOAD16_BYTE( "cry.p7",  0x300000, 0x80000,  CRC(20f73433) SHA1(593b40ac17591ac312ad41b4d3a5772626137bba) )
	ROM_LOAD16_BYTE( "cry.p8",  0x300001, 0x80000,  CRC(835da1f2) SHA1(f93e075916d370466832871410591570ad7b9f3b) )
	ROM_LOAD16_BYTE( "cry.p9",  0x400000, 0x80000,  CRC(c0e442ee) SHA1(a3877b200538642fe2bc96cfe8b33f04d8a82a98) )
	ROM_LOAD16_BYTE( "cry.p10", 0x400001, 0x80000,  CRC(500172fa) SHA1(d83a37612daa79ba8425fdb28f39b8324b5736b6) )
ROM_END

ROM_START( v4cmazec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cm1.p1",  0x000000, 0x040000, CRC(40533c1d) SHA1(06f115b7e1008e8797767d3f1e37edb4c1ce6675) )
	ROM_LOAD16_BYTE( "cm1.p2",  0x000001, 0x040000, CRC(8b207eca) SHA1(cbd6b1b177daefe255609f7adc00effa6afc9cb5) )
	ROM_LOAD16_BYTE( "cry.p3",  0x100000, 0x80000,  CRC(e8cf8203) SHA1(e9f42e5c18b97807f51284ad2416346578ed73c4) )
	ROM_LOAD16_BYTE( "cry.p4",  0x100001, 0x80000,  CRC(7b036151) SHA1(7b0040c296059b1e1798ddedf0ecb4582d67ee70) )
	ROM_LOAD16_BYTE( "cry.p5",  0x200000, 0x80000,  CRC(48f17b20) SHA1(711c46fcfd86ded8ff7da883188d70560d20e42f) )
	ROM_LOAD16_BYTE( "cry.p6",  0x200001, 0x80000,  CRC(2b3d9a97) SHA1(7468fffd90d840d245a70475b42308f1e48c5017) )
	ROM_LOAD16_BYTE( "cry.p7",  0x300000, 0x80000,  CRC(20f73433) SHA1(593b40ac17591ac312ad41b4d3a5772626137bba) )
	ROM_LOAD16_BYTE( "cry.p8",  0x300001, 0x80000,  CRC(835da1f2) SHA1(f93e075916d370466832871410591570ad7b9f3b) )
	ROM_LOAD16_BYTE( "cry.p9",  0x400000, 0x80000,  CRC(c0e442ee) SHA1(a3877b200538642fe2bc96cfe8b33f04d8a82a98) )
	ROM_LOAD16_BYTE( "cry.p10", 0x400001, 0x80000,  CRC(500172fa) SHA1(d83a37612daa79ba8425fdb28f39b8324b5736b6) )
ROM_END

ROM_START( v4cmazed )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cmz11p1",  0x000000, 0x040000, CRC(887baf65) SHA1(096133414bff89db4e30405f72f3c994f962be08) )
	ROM_LOAD16_BYTE( "cmz11p2",  0x000001, 0x040000, CRC(f8f095a6) SHA1(6b23fe3712416bf268c6387d3b304f05206d45e1) )
	ROM_LOAD16_BYTE( "cry.p3",  0x100000, 0x80000,  CRC(e8cf8203) SHA1(e9f42e5c18b97807f51284ad2416346578ed73c4) )
	ROM_LOAD16_BYTE( "cry.p4",  0x100001, 0x80000,  CRC(7b036151) SHA1(7b0040c296059b1e1798ddedf0ecb4582d67ee70) )
	ROM_LOAD16_BYTE( "cry.p5",  0x200000, 0x80000,  CRC(48f17b20) SHA1(711c46fcfd86ded8ff7da883188d70560d20e42f) )
	ROM_LOAD16_BYTE( "cry.p6",  0x200001, 0x80000,  CRC(2b3d9a97) SHA1(7468fffd90d840d245a70475b42308f1e48c5017) )
	ROM_LOAD16_BYTE( "cry.p7",  0x300000, 0x80000,  CRC(20f73433) SHA1(593b40ac17591ac312ad41b4d3a5772626137bba) )
	ROM_LOAD16_BYTE( "cry.p8",  0x300001, 0x80000,  CRC(835da1f2) SHA1(f93e075916d370466832871410591570ad7b9f3b) )
	ROM_LOAD16_BYTE( "cry.p9",  0x400000, 0x80000,  CRC(c0e442ee) SHA1(a3877b200538642fe2bc96cfe8b33f04d8a82a98) )
	ROM_LOAD16_BYTE( "cry.p10", 0x400001, 0x80000,  CRC(500172fa) SHA1(d83a37612daa79ba8425fdb28f39b8324b5736b6) )
ROM_END



//The New Crystal Maze Featuring Ocean Zone
ROM_START( v4cmaze2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cm2s.p1", 0x000000, 0x80000,  CRC(2bcdff56) SHA1(5ae4d4960db032ec9b8d66c53bc151830d009f61) )
	ROM_LOAD16_BYTE( "cm2s.p2", 0x000001, 0x80000,  CRC(92126def) SHA1(531593dee05954000d9836018aeff9460aecbd26) )
	ROM_LOAD16_BYTE( "cm2.p3",  0x100000, 0x80000,  CRC(88324715) SHA1(c6c8de4e5aeda14232ec7b026da389774b3c7bb1) )
	ROM_LOAD16_BYTE( "cm2.p4",  0x100001, 0x80000,  CRC(8d54a81d) SHA1(37753cf8595647aaf8b8267ca177b6744de9c6d4) )
	ROM_LOAD16_BYTE( "cm2.p5",  0x200000, 0x80000,  CRC(5cf8a2bf) SHA1(2514e78e82842fa5c85d26de35637269cd08b21d) )
	ROM_LOAD16_BYTE( "cm2.p6",  0x200001, 0x80000,  CRC(cf793d2d) SHA1(579c759f57fb6bb87aa27c9d5fb684058913dedc) )
	ROM_LOAD16_BYTE( "cm2.p7",  0x300000, 0x80000,  CRC(008aa4b0) SHA1(b4cec6d11abd0e111c295533700595398ff59075) )
	ROM_LOAD16_BYTE( "cm2.p8",  0x300001, 0x80000,  CRC(bac04f5a) SHA1(130721b7abf28dea1f8162705c8bfc5a4bb78152) )
ROM_END

ROM_START( v4cmaze2d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cm2d.p1", 0x000000, 0x80000,  CRC(a150027b) SHA1(f737b0e3b8954c1589a929b7e567cb55df4a3997) )
	ROM_LOAD16_BYTE( "cm2d.p2", 0x000001, 0x80000,  CRC(84ed6bce) SHA1(7a11473e7ec277508952f7ae6cfc7ed28e1b5c99) )
	ROM_LOAD16_BYTE( "cm2.p3",  0x100000, 0x80000,  CRC(88324715) SHA1(c6c8de4e5aeda14232ec7b026da389774b3c7bb1) )
	ROM_LOAD16_BYTE( "cm2.p4",  0x100001, 0x80000,  CRC(8d54a81d) SHA1(37753cf8595647aaf8b8267ca177b6744de9c6d4) )
	ROM_LOAD16_BYTE( "cm2.p5",  0x200000, 0x80000,  CRC(5cf8a2bf) SHA1(2514e78e82842fa5c85d26de35637269cd08b21d) )
	ROM_LOAD16_BYTE( "cm2.p6",  0x200001, 0x80000,  CRC(cf793d2d) SHA1(579c759f57fb6bb87aa27c9d5fb684058913dedc) )
	ROM_LOAD16_BYTE( "cm2.p7",  0x300000, 0x80000,  CRC(008aa4b0) SHA1(b4cec6d11abd0e111c295533700595398ff59075) )
	ROM_LOAD16_BYTE( "cm2.p8",  0x300001, 0x80000,  CRC(bac04f5a) SHA1(130721b7abf28dea1f8162705c8bfc5a4bb78152) )
ROM_END

ROM_START( v4cmaze2a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "am2z.p1",  0x000000, 0x80000,  CRC(f27e02f0) SHA1(8637904c201ece4f08ad63b4fd6d06a860fa762f) )
	ROM_LOAD16_BYTE( "am2z.p2",  0x000001, 0x80000,  CRC(4d24f482) SHA1(9e3687db9d0233e56999017f3ed59ec543bce303) )
	ROM_LOAD16_BYTE( "am2g.p1",  0x100000, 0x80000,  CRC(115402db) SHA1(250f2eded1b88a1abf82febb009eadbb90936f8a) )
	ROM_LOAD16_BYTE( "am2g.p2",  0x100001, 0x80000,  CRC(5d804fbb) SHA1(8dc02eb9329f9c29d4bcc9a0315ae96085625d3e) )
	ROM_LOAD16_BYTE( "am2g.p3",  0x200000, 0x80000,  CRC(5ead0c06) SHA1(35d9aefc60e2c391e32f8119a6dc44434d91c09e) )
	ROM_LOAD16_BYTE( "am2g.p4",  0x200001, 0x80000,  CRC(de4fb542) SHA1(4bf8f8f6850fd819d91827d3c474bd488e61e5ac) )
	ROM_LOAD16_BYTE( "am2g.p5",  0x300000, 0x80000,  CRC(80b01ce2) SHA1(4a3a4bcff4bd9affd1a5eeca5781b6af05bbcc16) )
	ROM_LOAD16_BYTE( "am2g.p6",  0x300001, 0x80000,  CRC(3e134ecc) SHA1(1f8cdce62e693eb07c4620b64cc467339c0563de) )
	ROM_LOAD16_BYTE( "am2g.p7",  0x400000, 0x80000,  CRC(6eb36f1d) SHA1(08b9ec184d64bdbdfa61d3e991a3647e74a7756f) )
	ROM_LOAD16_BYTE( "am2g.p8",  0x400001, 0x80000,  CRC(dda353ef) SHA1(56a5b43f0b0bd9dbf348946a5758ebe63eadb8cf) )
ROM_END

ROM_START( v4cmaze2b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cr220.p1", 0x000000, 0x80000,  CRC(0a6ebe65) SHA1(504648c97bc9de6c3e0c9c73eb2f356632e1c562) )
	ROM_LOAD16_BYTE( "cr220.p2", 0x000001, 0x80000,  CRC(699ab8d4) SHA1(8a6516e515d111c29833f6a94845942d156cc54d) )
	ROM_LOAD16_BYTE( "cm2.p3",  0x100000, 0x80000,  CRC(88324715) SHA1(c6c8de4e5aeda14232ec7b026da389774b3c7bb1) )
	ROM_LOAD16_BYTE( "cm2.p4",  0x100001, 0x80000,  CRC(8d54a81d) SHA1(37753cf8595647aaf8b8267ca177b6744de9c6d4) )
	ROM_LOAD16_BYTE( "cm2.p5",  0x200000, 0x80000,  CRC(5cf8a2bf) SHA1(2514e78e82842fa5c85d26de35637269cd08b21d) )
	ROM_LOAD16_BYTE( "cm2.p6",  0x200001, 0x80000,  CRC(cf793d2d) SHA1(579c759f57fb6bb87aa27c9d5fb684058913dedc) )
	ROM_LOAD16_BYTE( "cm2.p7",  0x300000, 0x80000,  CRC(008aa4b0) SHA1(b4cec6d11abd0e111c295533700595398ff59075) )
	ROM_LOAD16_BYTE( "cm2.p8",  0x300001, 0x80000,  CRC(bac04f5a) SHA1(130721b7abf28dea1f8162705c8bfc5a4bb78152) )
ROM_END

ROM_START( v4cmaze2c )//Should be a set, but PROM 2 checksum error?
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cm2_7.p1", 0x000000, 0x80000,  CRC(d455d5fe) SHA1(20186a5fff458d3eba15ccb564addc0b064736e0) )
	ROM_LOAD16_BYTE( "cm2_7.p2", 0x000001, 0x80000, BAD_DUMP CRC(55fef843) SHA1(ccc187c0137af54bad8b72181a544d5b78004ed8) )
	ROM_LOAD16_BYTE( "cm2.p3",  0x100000, 0x80000,  CRC(88324715) SHA1(c6c8de4e5aeda14232ec7b026da389774b3c7bb1) )
	ROM_LOAD16_BYTE( "cm2.p4",  0x100001, 0x80000,  CRC(8d54a81d) SHA1(37753cf8595647aaf8b8267ca177b6744de9c6d4) )
	ROM_LOAD16_BYTE( "cm2.p5",  0x200000, 0x80000,  CRC(5cf8a2bf) SHA1(2514e78e82842fa5c85d26de35637269cd08b21d) )
	ROM_LOAD16_BYTE( "cm2.p6",  0x200001, 0x80000,  CRC(cf793d2d) SHA1(579c759f57fb6bb87aa27c9d5fb684058913dedc) )
	ROM_LOAD16_BYTE( "cm2.p7",  0x300000, 0x80000,  CRC(008aa4b0) SHA1(b4cec6d11abd0e111c295533700595398ff59075) )
	ROM_LOAD16_BYTE( "cm2.p8",  0x300001, 0x80000,  CRC(bac04f5a) SHA1(130721b7abf28dea1f8162705c8bfc5a4bb78152) )
ROM_END



//The Crystal Maze Team Challenge
ROM_START( v4cmaze3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cm3.p1",  0x000000, 0x80000,  CRC(2d2edee5) SHA1(0281ec97aaaaf4c7969340bd5995ac1541dbad54) )
	ROM_LOAD16_BYTE( "cm3.p2",  0x000001, 0x80000,  CRC(c223d7b9) SHA1(da9d730716a30d0e93f2a02c1efa7f19457ae010) )
	ROM_LOAD16_BYTE( "cm3.p3",  0x100000, 0x80000,  CRC(2959c77b) SHA1(8de533bfad48ad19a635dddcafa2a0825133b4de) )
	ROM_LOAD16_BYTE( "cm3.p4",  0x100001, 0x80000,  CRC(b7873e9a) SHA1(a71fac883e02d5f49aee0a20f92dbdb00640ce8d) )
	ROM_LOAD16_BYTE( "cm3.p5",  0x200000, 0x80000,  CRC(c8375070) SHA1(da2ba6591d8765f896c40d6526da8e945d02a182) )
	ROM_LOAD16_BYTE( "cm3.p6",  0x200001, 0x80000,  CRC(1ea36938) SHA1(43f62935b21232d23f662e1e124663267edb1283) )
	ROM_LOAD16_BYTE( "cm3.p7",  0x300000, 0x80000,  CRC(9de3802e) SHA1(ec792f115a0708d68046ba0beb314b7e1f1eb422) )
	ROM_LOAD16_BYTE( "cm3.p8",  0x300001, 0x80000,  CRC(1e6e60b0) SHA1(5e71714747073dd89852a84585642388ee440325) )
	ROM_LOAD16_BYTE( "cm3.p9",  0x400000, 0x80000,  CRC(bfba55a7) SHA1(22eb9b1f9fe83d3b424fd521b68e2976a1940df9) )
	ROM_LOAD16_BYTE( "cm3.p10",  0x400001, 0x80000,  CRC(07edda81) SHA1(e94525be03f30e407051992925bb0d693f3d809b) )
ROM_END

ROM_START( v4cmaze3d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cm3d.p1",  0x000000, 0x80000,  CRC(245f00fa) SHA1(ff5ae1a2ae024dfc0b4104360626e4106f4cd36f) )
	ROM_LOAD16_BYTE( "cm3d.p2",  0x000001, 0x80000,  CRC(091adbcb) SHA1(1466b036d06f6335c90426095ad0f60ea958a29d) )
	ROM_LOAD16_BYTE( "cm3.p3",  0x100000, 0x80000,  CRC(2959c77b) SHA1(8de533bfad48ad19a635dddcafa2a0825133b4de) )
	ROM_LOAD16_BYTE( "cm3.p4",  0x100001, 0x80000,  CRC(b7873e9a) SHA1(a71fac883e02d5f49aee0a20f92dbdb00640ce8d) )
	ROM_LOAD16_BYTE( "cm3.p5",  0x200000, 0x80000,  CRC(c8375070) SHA1(da2ba6591d8765f896c40d6526da8e945d02a182) )
	ROM_LOAD16_BYTE( "cm3.p6",  0x200001, 0x80000,  CRC(1ea36938) SHA1(43f62935b21232d23f662e1e124663267edb1283) )
	ROM_LOAD16_BYTE( "cm3.p7",  0x300000, 0x80000,  CRC(9de3802e) SHA1(ec792f115a0708d68046ba0beb314b7e1f1eb422) )
	ROM_LOAD16_BYTE( "cm3.p8",  0x300001, 0x80000,  CRC(1e6e60b0) SHA1(5e71714747073dd89852a84585642388ee440325) )
	ROM_LOAD16_BYTE( "cm3.p9",  0x400000, 0x80000,  CRC(bfba55a7) SHA1(22eb9b1f9fe83d3b424fd521b68e2976a1940df9) )
	ROM_LOAD16_BYTE( "cm3.p10",  0x400001, 0x80000,  CRC(07edda81) SHA1(e94525be03f30e407051992925bb0d693f3d809b) )
ROM_END

ROM_START( v4cmaze3a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "am3z.p1",  0x000000, 0x80000,  CRC(9484e656) SHA1(3c35a8ebeddea56d73ce8db4e93c51cdd9546d59) )
	ROM_LOAD16_BYTE( "am3z.p2",  0x000001, 0x80000,  CRC(1865ee80) SHA1(b3ff8e1631d811b8e88664dd84ae82231ce1f5aa) )
	ROM_LOAD16_BYTE( "am3g.p1",  0x100000, 0x80000,  CRC(49fe36af) SHA1(7c39223b07f53ff57a56c3817299734491372170) )
	ROM_LOAD16_BYTE( "am3g.p2",  0x100001, 0x80000,  CRC(b8823cbd) SHA1(206d3b1b2daff1979f97841041661f8407c35d4d) )
	ROM_LOAD16_BYTE( "am3g.p3",  0x200000, 0x80000,  CRC(b1870f17) SHA1(54c6cabb56e4daa4ccf801d5e44b2789b116d562) )
	ROM_LOAD16_BYTE( "am3g.p4",  0x200001, 0x80000,  CRC(c015d446) SHA1(669007e841afeb1084d9062d0a47c159e4c83cc9) )
	ROM_LOAD16_BYTE( "am3g.p5",  0x300000, 0x80000,  CRC(9de3802e) SHA1(ec792f115a0708d68046ba0beb314b7e1f1eb422) )
	ROM_LOAD16_BYTE( "am3g.p6",  0x300001, 0x80000,  CRC(1e6e60b0) SHA1(5e71714747073dd89852a84585642388ee440325) )
	ROM_LOAD16_BYTE( "am3g.p7",  0x400000, 0x80000,  CRC(a4611e29) SHA1(91b164eea5dbdd1129ad12d7af2dbdb3cd68bcec) )
	ROM_LOAD16_BYTE( "am3g.p8",  0x400001, 0x80000,  CRC(1a10c22e) SHA1(8533a5db3922b80b6e9f74e4e432a2b64bc24fc0) )
ROM_END

ROM_START( v4cmaze3b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cm30_8.p1", 0x000000, 0x80000,CRC(fde7333e) SHA1(a94d7b88972af599418b9893124457a11c125fcc) )
	ROM_LOAD16_BYTE( "cm30_8.p2", 0x000001, 0x80000,CRC(35ff7e31) SHA1(52fe305c63cde70de550230bad03d85b53f8e227) )
	ROM_LOAD16_BYTE( "cm3.p3",  0x100000, 0x80000,  CRC(2959c77b) SHA1(8de533bfad48ad19a635dddcafa2a0825133b4de) )
	ROM_LOAD16_BYTE( "cm3.p4",  0x100001, 0x80000,  CRC(b7873e9a) SHA1(a71fac883e02d5f49aee0a20f92dbdb00640ce8d) )
	ROM_LOAD16_BYTE( "cm3.p5",  0x200000, 0x80000,  CRC(c8375070) SHA1(da2ba6591d8765f896c40d6526da8e945d02a182) )
	ROM_LOAD16_BYTE( "cm3.p6",  0x200001, 0x80000,  CRC(1ea36938) SHA1(43f62935b21232d23f662e1e124663267edb1283) )
	ROM_LOAD16_BYTE( "cm3.p7",  0x300000, 0x80000,  CRC(9de3802e) SHA1(ec792f115a0708d68046ba0beb314b7e1f1eb422) )
	ROM_LOAD16_BYTE( "cm3.p8",  0x300001, 0x80000,  CRC(1e6e60b0) SHA1(5e71714747073dd89852a84585642388ee440325) )
	ROM_LOAD16_BYTE( "cm3.p9",  0x400000, 0x80000,  CRC(bfba55a7) SHA1(22eb9b1f9fe83d3b424fd521b68e2976a1940df9) )
	ROM_LOAD16_BYTE( "cm3.p10", 0x400001, 0x80000,  CRC(07edda81) SHA1(e94525be03f30e407051992925bb0d693f3d809b) )
ROM_END

ROM_START( v4cmaze3c )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cm30_6.p1",  0x000000, 0x80000,  CRC(68aa610a) SHA1(d57071a960402b12a13ac3af602cedade03989f8) )
	ROM_LOAD16_BYTE( "cm30_6.p2",  0x000001, 0x80000,  NO_DUMP )
	ROM_LOAD16_BYTE( "cm3.p3",  0x100000, 0x80000,  CRC(2959c77b) SHA1(8de533bfad48ad19a635dddcafa2a0825133b4de) )
	ROM_LOAD16_BYTE( "cm3.p4",  0x100001, 0x80000,  CRC(b7873e9a) SHA1(a71fac883e02d5f49aee0a20f92dbdb00640ce8d) )
	ROM_LOAD16_BYTE( "cm3.p5",  0x200000, 0x80000,  CRC(c8375070) SHA1(da2ba6591d8765f896c40d6526da8e945d02a182) )
	ROM_LOAD16_BYTE( "cm3.p6",  0x200001, 0x80000,  CRC(1ea36938) SHA1(43f62935b21232d23f662e1e124663267edb1283) )
	ROM_LOAD16_BYTE( "cm3.p7",  0x300000, 0x80000,  CRC(9de3802e) SHA1(ec792f115a0708d68046ba0beb314b7e1f1eb422) )
	ROM_LOAD16_BYTE( "cm3.p8",  0x300001, 0x80000,  CRC(1e6e60b0) SHA1(5e71714747073dd89852a84585642388ee440325) )
	ROM_LOAD16_BYTE( "cm3.p9",  0x400000, 0x80000,  CRC(bfba55a7) SHA1(22eb9b1f9fe83d3b424fd521b68e2976a1940df9) )
	ROM_LOAD16_BYTE( "cm3.p10",  0x400001, 0x80000,  CRC(07edda81) SHA1(e94525be03f30e407051992925bb0d693f3d809b) )
ROM_END


ROM_START( v4turnov )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "tos.p1",  0x000000, 0x010000,  CRC(4044dbeb) SHA1(3aa553055c56f14564b1e33e1c1975337e639f70) )
	ROM_LOAD16_BYTE( "to.p2",   0x000001, 0x010000,  CRC(4bc4659a) SHA1(0e282134c4fe4e8c1cc7b16957903179e23c7abc) )
	ROM_LOAD16_BYTE( "to.p3",   0x020000, 0x010000,  CRC(273c7c14) SHA1(71feb555a05a0ff1ec674505cab72d93c9fbdf65) )
	ROM_LOAD16_BYTE( "to.p4",   0x020001, 0x010000,  CRC(83d29546) SHA1(cef90455b9d8a92424fe1aa10f20fd075d0e3091) )
	ROM_LOAD16_BYTE( "to.p5",   0x040000, 0x010000,  CRC(dceac511) SHA1(7a6d65464e23d832943f771c4cf580aabc6f0e44) )
	ROM_LOAD16_BYTE( "to.p6",   0x040001, 0x010000,  CRC(54c6afb7) SHA1(b724b87b6f4e47d220310b38c97be2fa73dcd617) )
	ROM_LOAD16_BYTE( "to.p7",   0x060000, 0x010000,  CRC(acf19542) SHA1(ad46ffb3c2c078a8e3712eff27aa61f0d1a7c059) )
	ROM_LOAD16_BYTE( "to.p8",   0x060001, 0x010000,  CRC(a5ca385d) SHA1(8df26a33ea7f5b577761c6f9d2fa4eaed74661f8) )
	ROM_LOAD16_BYTE( "to.p9",   0x080000, 0x010000,  CRC(6e85fde3) SHA1(14868d58829e13987e66f52e1899c4385987a87b) )
	ROM_LOAD16_BYTE( "to.p10",  0x080001, 0x010000,  CRC(fadd11a2) SHA1(2b2fbb0769ef6035688d495464f3ea3bc8c7c660) )
	ROM_LOAD16_BYTE( "to.p11",  0x0a0000, 0x010000,  CRC(2d72a61a) SHA1(ce455ab6fea452f96a3ad365178e0e5a0b437867) )
	ROM_LOAD16_BYTE( "to.p12",  0x0a0001, 0x010000,  CRC(a14eedb6) SHA1(219b887a334ff28a88ed2e50f0caff4b510cd549) )
	ROM_LOAD16_BYTE( "to.p13",  0x0c0000, 0x010000,  CRC(3f66ef6b) SHA1(60be6d3f8da1f3084db15ac1bb2470e55c0271de) )
	ROM_LOAD16_BYTE( "to.p14",  0x0c0001, 0x010000,  CRC(127ba65d) SHA1(e34dcd19efd31dc712daac940277bb17694ea61a) )
	ROM_LOAD16_BYTE( "to.p15",  0x0e0000, 0x010000,  CRC(ad787e31) SHA1(314ba312adfc71e4b3b2d52355ec692c192b74eb) )
	ROM_LOAD16_BYTE( "to.p16",  0x0e0001, 0x010000,  CRC(e635c942) SHA1(08f8b5fdb738647bc0b49938da05533be42a2d60) )

	ROM_REGION( 0x800000, "altvideo", 0 )
	// seems to be an unmatched 2.2 revision rom
	ROM_LOAD16_BYTE( "tov2.2p1", 0x0000, 0x010000, CRC(460a5dd0) SHA1(42bc54b0ca206606b980dd80ccf0cbfb3210769d) )

	ROM_LOAD16_BYTE( "to3.p1", 0x0000, 0x010000, CRC(09751994) SHA1(72d4aa40f14411ef8064822de3f5a13bcc84aea3) )
	ROM_LOAD16_BYTE( "todo.p1", 0x0000, 0x010000, CRC(9111e702) SHA1(fa408e1c8fa56a96ffc3422335f105ef328a6edd) )
	ROM_LOAD16_BYTE( "too.p1", 0x0000, 0x010000, CRC(919b5207) SHA1(770be6e3b00e666c6f939167d35d43bb2d793e14) )

ROM_END

ROM_START( v4skltrk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )


	ROM_LOAD16_BYTE( "st.p1",  0x000000, 0x010000,  CRC(d9de47a5) SHA1(625bf40780203293fc34cd8cea8278b4b4a52a75) )
	ROM_LOAD16_BYTE( "st.p2",  0x000001, 0x010000,  CRC(b62575c2) SHA1(06d75e8a364750663d329650720021279e195236) )
	ROM_LOAD16_BYTE( "st.p3",  0x020000, 0x010000,  CRC(9506da76) SHA1(6ef28ab8ec1af455be8ecfab20243f0823dca7c1) )
	ROM_LOAD16_BYTE( "st.p4",  0x020001, 0x010000,  CRC(6ab447bc) SHA1(d01c209dbf4d19a6a7f878fa54ff1cb51e7dcba5) )
	ROM_LOAD16_BYTE( "st.q1",  0x040000, 0x010000,  CRC(4faca475) SHA1(69b498c543600b8e37ab0ed1863ba57845648f3c) )
	ROM_LOAD16_BYTE( "st.q2",  0x040001, 0x010000,  CRC(9f2c5938) SHA1(85527c4c0b7a1e66576d56607d89750fab082580) )
	ROM_LOAD16_BYTE( "st.q3",  0x060000, 0x010000,  CRC(6b6cb194) SHA1(aeac5dcc0827c17e758e3e821ae8a78a3a16ddce) )
	ROM_LOAD16_BYTE( "st.q4",  0x060001, 0x010000,  CRC(ec57bc17) SHA1(d9f522739dbb190fb941ca654299bbedbb8fb703) )
	ROM_LOAD16_BYTE( "st.q5",  0x080000, 0x010000,  CRC(7740a88b) SHA1(d9a683d3e0d6c1b4b59520f90f825124b7a61168) )
	ROM_LOAD16_BYTE( "st.q6",  0x080001, 0x010000,  CRC(95e97796) SHA1(f1a8de0ad02aca31f79a4fe8ba5044546163e3c4) )
	ROM_LOAD16_BYTE( "st.q7",  0x0a0000, 0x010000,  CRC(f3b8fe7f) SHA1(52d5be3f8cab419103f4727d0fb9d30f34c8f651) )
	ROM_LOAD16_BYTE( "st.q8",  0x0a0001, 0x010000,  CRC(b85e75a2) SHA1(b7b03b090c0ec6d92e9a25abb7fec0507356bdfc) )
	ROM_LOAD16_BYTE( "st.q9",  0x0c0000, 0x010000,  CRC(835f6001) SHA1(2cd9084c102d74bcb578c8ea22bbc9ea58f0ceab) )
	ROM_LOAD16_BYTE( "st.qa",  0x0c0001, 0x010000,  CRC(3fc62a0e) SHA1(0628de4b962d3fcca3757cd4e89b3005c9bfd218) )
ROM_END

ROM_START( v4skltrka )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "sws.p1",0x000000, 0x010000, CRC(a96f11ea) SHA1(5e5c548e96674f8f98c1c4eb0a590979371f6f10) )
	ROM_LOAD16_BYTE( "sw.p2", 0x000001, 0x010000, CRC(2598ba38) SHA1(30509bd2f987949bb423600e13a464edaa7f9ce5) )
	ROM_LOAD16_BYTE( "sw.p3", 0x020000, 0x010000, CRC(1290cef3) SHA1(5cf63940c164becf64902846aa4fd5bc628f9ebe) )
	ROM_LOAD16_BYTE( "sw.p4", 0x020001, 0x010000, CRC(176f23c3) SHA1(e64460818329f23e449f65781e3dc56d3a1710a3) )
	ROM_LOAD16_BYTE( "st.q1",  0x040000, 0x010000,  CRC(4faca475) SHA1(69b498c543600b8e37ab0ed1863ba57845648f3c) )
	ROM_LOAD16_BYTE( "st.q2",  0x040001, 0x010000,  CRC(9f2c5938) SHA1(85527c4c0b7a1e66576d56607d89750fab082580) )
	ROM_LOAD16_BYTE( "st.q3",  0x060000, 0x010000,  CRC(6b6cb194) SHA1(aeac5dcc0827c17e758e3e821ae8a78a3a16ddce) )
	ROM_LOAD16_BYTE( "st.q4",  0x060001, 0x010000,  CRC(ec57bc17) SHA1(d9f522739dbb190fb941ca654299bbedbb8fb703) )
	ROM_LOAD16_BYTE( "st.q5",  0x080000, 0x010000,  CRC(7740a88b) SHA1(d9a683d3e0d6c1b4b59520f90f825124b7a61168) )
	ROM_LOAD16_BYTE( "st.q6",  0x080001, 0x010000,  CRC(95e97796) SHA1(f1a8de0ad02aca31f79a4fe8ba5044546163e3c4) )
	ROM_LOAD16_BYTE( "st.q7",  0x0a0000, 0x010000,  CRC(f3b8fe7f) SHA1(52d5be3f8cab419103f4727d0fb9d30f34c8f651) )
	ROM_LOAD16_BYTE( "st.q8",  0x0a0001, 0x010000,  CRC(b85e75a2) SHA1(b7b03b090c0ec6d92e9a25abb7fec0507356bdfc) )
	ROM_LOAD16_BYTE( "st.q9",  0x0c0000, 0x010000,  CRC(835f6001) SHA1(2cd9084c102d74bcb578c8ea22bbc9ea58f0ceab) )
	ROM_LOAD16_BYTE( "st.qa",  0x0c0001, 0x010000,  CRC(3fc62a0e) SHA1(0628de4b962d3fcca3757cd4e89b3005c9bfd218) )
ROM_END


ROM_START( v4sklcsh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "scs.p1", 0x000000, 0x010000, CRC(3181524e) SHA1(e801a036b9d509d8e2e5f7c9c6ccd9d9ddf76b2f) )
	ROM_LOAD16_BYTE( "scs.p2", 0x000001, 0x010000, CRC(a7596804) SHA1(f27fc3ee7a17904ea08baa421e14d02226428c42) )
	ROM_LOAD16_BYTE( "scs.p3", 0x020000, 0x010000, CRC(b3ded2bc) SHA1(39cb15fde4aea1187668120d548bfe022835b600) )
	ROM_LOAD16_BYTE( "scs.p4", 0x020001, 0x010000, CRC(2ce2e9d1) SHA1(c492fea2e179796a360131c19a9cba5bbfe3c7fd) )

	/* missing questions */
ROM_END




ROM_START( v4time )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "tm20.p1",  0x000000, 0x010000,  CRC(6919697c) SHA1(786d7b9ab218dbf54ff839d1f83580c409c725b3) )
	ROM_LOAD16_BYTE( "tm20.p2",  0x000001, 0x010000,  CRC(d13b56e4) SHA1(623e73995da93c07b51ce0a5843dba1f853529dd) )
	ROM_LOAD16_BYTE( "tm20.p3",  0x020000, 0x010000,  CRC(efd3ae64) SHA1(9d2a3b65048e04842205751c6921d2550f38bd52) )
	ROM_LOAD16_BYTE( "tm20.p4",  0x020001, 0x010000,  CRC(602ba3fb) SHA1(7243f58df9a26adfd1a149a1e60630b187787dd0) )
	ROM_LOAD16_BYTE( "q12.p5" ,  0x040000, 0x010000,  CRC(adddd8a7) SHA1(73a8dd191eda2f4b41b79d4b55723731953b8970) )
	ROM_LOAD16_BYTE( "q11.p6" ,  0x040001, 0x010000,  CRC(e8ed736f) SHA1(e7068c550aa39a6e8f1692a16794147e996d36b4) )
	ROM_LOAD16_BYTE( "q14.p7" ,  0x060000, 0x010000,  CRC(02abb026) SHA1(42224678e5913090c91c21672661beb8e27127a8) )
	ROM_LOAD16_BYTE( "q13.p8" ,  0x060001, 0x010000,  CRC(3de147dd) SHA1(d2111d54d1604fe2da0133102bbfee706f8f542e) )
	ROM_LOAD16_BYTE( "q16.p9" ,  0x080000, 0x010000,  CRC(ce2bf15e) SHA1(29c7f2e718bce415b0b8dc6d902bf74dad6b1ef4) )
	ROM_LOAD16_BYTE( "q15.p10",  0x080001, 0x010000,  CRC(7894ac8b) SHA1(dc46bd108ac4f67a9062bb7ace91aa51f069cbc8) )
	ROM_LOAD16_BYTE( "q18.p11",  0x0a0000, 0x010000,  CRC(27de90b3) SHA1(625c98e555f7b627ea96653926b8917996a2fdb7) )
	ROM_LOAD16_BYTE( "q17.p12",  0x0a0001, 0x010000,  CRC(5cab773e) SHA1(59a235c51a975b341bdbb88e909729507408f75b) )
	ROM_LOAD16_BYTE( "q20.p13",  0x0c0000, 0x010000,  CRC(083f6c65) SHA1(291ad39ee5f8eba9da293d9206b1f6a6d852f9bd) )
	ROM_LOAD16_BYTE( "q19.p14",  0x0c0001, 0x010000,  CRC(73747644) SHA1(ae252fc95c069a3c82e155220fbfcb74dd43bf89) )

	ROM_REGION( 0x800000, "altvideo", 0 )
	ROM_LOAD( "tmd.p1", 0x0000, 0x010000, CRC(e21045e0) SHA1(c4d0e80970ec8558db777a882edc5a0c80767375) )

	ROM_REGION( 0x800000, "altquestion", 0 )
	ROM_LOAD( "tm.q1", 0x0000, 0x010000, CRC(6af4d58b) SHA1(ee547dad30cd9940f0b017caac97aeb046604f22) )
	ROM_LOAD( "tm.q2", 0x0000, 0x010000, CRC(b01b7687) SHA1(99db448d7e40c2ec16afef3c10abc8a9493f2ab4) )
	ROM_LOAD( "tm.q3", 0x0000, 0x010000, CRC(e97f14eb) SHA1(9163d11e1bc5a13d5002a13bc18b65a91e1738c7) )
	ROM_LOAD( "tm.q4", 0x0000, 0x010000, CRC(6134d918) SHA1(e4b4d6b08d94729d4dcca474d4c7bdcb267530a8) )
	ROM_LOAD( "tm.q5", 0x0000, 0x010000, CRC(6c07814b) SHA1(a97feada5cfa1bb059837b292637fbad9c7137ac) )
	ROM_LOAD( "tm.q6", 0x0000, 0x010000, CRC(5f16a536) SHA1(3435282bfb940604fb44e06dc4748e668768f286) )
	ROM_LOAD( "tm.q7", 0x0000, 0x010000, CRC(9afdce0b) SHA1(a969038d9ce2a2cff1e1a75959c05a3f03f08235) )
	ROM_LOAD( "tm.q8", 0x0000, 0x010000, CRC(f1878251) SHA1(b6a8527112bcdf21b9a0acab4d8fa507a96aaba7) )
	ROM_LOAD( "tm.q9", 0x0000, 0x010000, CRC(ace01faa) SHA1(79d6247a74e1bce0d76ea3788d0022d9e50173c4) )
	ROM_LOAD( "tm.qa", 0x0000, 0x010000, CRC(021f4523) SHA1(10884665f5700c147c7035d0c98f3889917ff015) )
ROM_END

ROM_START( v4mate )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mats.p1",  0x000000, 0x080000,  CRC(ebc7ea7e) SHA1(11015489a803ba5c8dbdafd632424bbd6080aece) )
	ROM_LOAD16_BYTE( "mats.p2",  0x000001, 0x080000,  CRC(a4c7e9b4) SHA1(30148c0257181bb88159e02d2b7cd79995ee84a7) )
	ROM_LOAD16_BYTE( "matg.p3",  0x100000, 0x080000,  CRC(571f4e8e) SHA1(51babacb5d9fb1cc9e1e56a3b2a355597d04f178) )
	ROM_LOAD16_BYTE( "matg.p4",  0x100001, 0x080000,  CRC(52d8657b) SHA1(e44e1db13c4abd4fedcd72df9dce1df594f74e44) )
	ROM_LOAD16_BYTE( "matg.p5",  0x200000, 0x080000,  CRC(9f0c9552) SHA1(8b1197f20853e18841a8f64fd5ff58cdd0bd1dbd) )
	ROM_LOAD16_BYTE( "matg.p6",  0x200001, 0x080000,  CRC(59f2b6a8) SHA1(4921cf1fc4c3bc50d2598b63726f61f68b41658c) )
	ROM_LOAD16_BYTE( "matg.p7",  0x300000, 0x080000,  CRC(64c0031a) SHA1(a519addd5d8f4696967ec84c163d28cb81ff9f32) )
	ROM_LOAD16_BYTE( "matg.p8",  0x300001, 0x080000,  CRC(22370dae) SHA1(72b1686b458750b5ee9dfe5599c308329d2c79d5) )
	ROM_LOAD16_BYTE( "matq.p9",  0x400000, 0x040000,  CRC(2d42e982) SHA1(80e476d5d65662059daa93a2fd383aecb74903c1) )
	ROM_LOAD16_BYTE( "matq.p10", 0x400001, 0x040000,  CRC(90364c3c) SHA1(6a4d2a3dd2cf9040887503888e6f38341578ad64) )

	/* Mating Game has an extra OKI sound chip */
	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "matsnd.p1",  0x000000, 0x080000,  CRC(f16df9e3) SHA1(fd9b82d73e18e635a9ea4aabd8c0b4aa2c8c6fdb) )
	ROM_LOAD( "matsnd.p2",  0x080000, 0x080000,  CRC(0c041621) SHA1(9156bf17ef6652968d9fbdc0b2bde64d3a67459c) )
	ROM_LOAD( "matsnd.p3",  0x100000, 0x080000,  CRC(c7435af9) SHA1(bd6080afaaaecca0d65e6d4125b46849aa4d1f33) )
	ROM_LOAD( "matsnd.p4",  0x180000, 0x080000,  CRC(d7e65c5b) SHA1(5575fb9f948158f2e94c986bf4bca9c9ee66a489) )
ROM_END

ROM_START( v4mated )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "matd.p1",  0x000000, 0x080000,  CRC(e660909f) SHA1(0acd990264fd7faf1f91a796d2438e8c2c7b83d1) )
	ROM_LOAD16_BYTE( "matd.p2",  0x000001, 0x080000,  CRC(a4c7e9b4) SHA1(30148c0257181bb88159e02d2b7cd79995ee84a7) )
	ROM_LOAD16_BYTE( "matg.p3",  0x100000, 0x080000,  CRC(571f4e8e) SHA1(51babacb5d9fb1cc9e1e56a3b2a355597d04f178) )
	ROM_LOAD16_BYTE( "matg.p4",  0x100001, 0x080000,  CRC(52d8657b) SHA1(e44e1db13c4abd4fedcd72df9dce1df594f74e44) )
	ROM_LOAD16_BYTE( "matg.p5",  0x200000, 0x080000,  CRC(9f0c9552) SHA1(8b1197f20853e18841a8f64fd5ff58cdd0bd1dbd) )
	ROM_LOAD16_BYTE( "matg.p6",  0x200001, 0x080000,  CRC(59f2b6a8) SHA1(4921cf1fc4c3bc50d2598b63726f61f68b41658c) )
	ROM_LOAD16_BYTE( "matg.p7",  0x300000, 0x080000,  CRC(64c0031a) SHA1(a519addd5d8f4696967ec84c163d28cb81ff9f32) )
	ROM_LOAD16_BYTE( "matg.p8",  0x300001, 0x080000,  CRC(22370dae) SHA1(72b1686b458750b5ee9dfe5599c308329d2c79d5) )
	ROM_LOAD16_BYTE( "matq.p9",  0x400000, 0x040000,  CRC(2d42e982) SHA1(80e476d5d65662059daa93a2fd383aecb74903c1) )
	ROM_LOAD16_BYTE( "matq.p10", 0x400001, 0x040000,  CRC(90364c3c) SHA1(6a4d2a3dd2cf9040887503888e6f38341578ad64) )

	/* Mating Game has an extra OKI sound chip */
	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "matsnd.p1",  0x000000, 0x080000,  CRC(f16df9e3) SHA1(fd9b82d73e18e635a9ea4aabd8c0b4aa2c8c6fdb) )
	ROM_LOAD( "matsnd.p2",  0x080000, 0x080000,  CRC(0c041621) SHA1(9156bf17ef6652968d9fbdc0b2bde64d3a67459c) )
	ROM_LOAD( "matsnd.p3",  0x100000, 0x080000,  CRC(c7435af9) SHA1(bd6080afaaaecca0d65e6d4125b46849aa4d1f33) )
	ROM_LOAD( "matsnd.p4",  0x180000, 0x080000,  CRC(d7e65c5b) SHA1(5575fb9f948158f2e94c986bf4bca9c9ee66a489) )
ROM_END

ROM_START( v4addlad )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "als_21.p1", 0x000000, 0x010000, CRC(c2c4ceea) SHA1(0c5ea4d02ba053191855547b714da9a7f61f3713) )
	ROM_LOAD16_BYTE( "al_21.p2",  0x000001, 0x010000, CRC(d5909619) SHA1(eb8d978ed28cd3afab6b3c1274d79bc6b188d240) )
	ROM_LOAD16_BYTE( "al_21.p3",  0x020000, 0x010000, CRC(5dc65d9f) SHA1(52dffb162a0e5649e056817e9db567cc7653538c) )
	ROM_LOAD16_BYTE( "al_21.p4",  0x020001, 0x010000, CRC(17c77264) SHA1(321372c1c0d25cfc5a1302ccfd5680d9bf62d31b) )
	ROM_LOAD16_BYTE( "al.q1",  0x040000, 0x10000,  CRC(b9b50b70) SHA1(1887ab00ee004e3f27902d6880fa31277a981891) )
	ROM_LOAD16_BYTE( "al.q2",  0x040001, 0x10000,  CRC(1bed86ac) SHA1(8e6563b5441ad9ddd468a3d9ae906733fed7912a) )
	ROM_LOAD16_BYTE( "al.q3",  0x060000, 0x10000,  CRC(294d8f28) SHA1(9f9aca491ba6c4dc5cfb91da867990a9610c3a28) )
	ROM_LOAD16_BYTE( "al.q4",  0x060001, 0x10000,  CRC(aa3e9fbd) SHA1(59d0868f4c8b3f56ca31a11d2e6af83b202bb735) )
	ROM_LOAD16_BYTE( "al.q5",  0x080000, 0x10000,  CRC(503f4193) SHA1(86df379c736598ba59446961bf0666e155164e1d) )
	ROM_LOAD16_BYTE( "al.q6",  0x080001, 0x10000,  CRC(9fd77e52) SHA1(d8fdebb0fd57ab9ea9797dd386168581a45ebc62) )
	ROM_LOAD16_BYTE( "al.q7",  0x0a0000, 0x10000,  CRC(cf3fa7c7) SHA1(fa1edf09c6d3a8b5737474117b0306ef64f7741c) )
	ROM_LOAD16_BYTE( "al.q8",  0x0a0001, 0x10000,  CRC(55058a63) SHA1(cf9edef5264f4301be4ee11f221ab67a5183a603) )
	ROM_LOAD16_BYTE( "al.q9",  0x0c0000, 0x10000,  CRC(22274191) SHA1(9bee5709edcd853e96408f37447c0f5324610903) )
	ROM_LOAD16_BYTE( "al.qa",  0x0c0001, 0x10000,  CRC(1fe98b4d) SHA1(533afeaea42903905f6f1206bba1a023b141bdd9) )

	ROM_REGION( 0x800000, "altvideo", 0 )
	ROM_LOAD16_BYTE( "ald_21.p1", 0x000000, 0x010000, CRC(ecc7c79c) SHA1(e03f470d0b83ed81af737a1d16a02528df733149) )

	ROM_REGION( 0x800000, "altvideo2", 0 ) // not sure what these are, they look like different hw??
	ROM_LOAD16_BYTE( "a6ppl.p0", 0x00000, 0x020000, CRC(350da2df) SHA1(a390e0c7e1e624c17f0e254e0b99ef9dbf56269d) )
	ROM_LOAD16_BYTE( "a6ppl.p1", 0x00001, 0x020000, CRC(63038aba) SHA1(8ec4e02109e872460a9598e469b59919cc5450dd) )
ROM_END

ROM_START( v4addlad20 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "al.p1",  0x000000, 0x10000,  CRC(97579eb2) SHA1(68f08ab73e0c8044de32f7eea1b3abc1fefff830) )
	ROM_LOAD16_BYTE( "al.p2",  0x000001, 0x10000,  CRC(ca877046) SHA1(7d0e1bb471b7acc89d04953ee5e0b34f64d58325) )
	ROM_LOAD16_BYTE( "al.p3",  0x020000, 0x10000,  CRC(92b5e1c4) SHA1(910126527ddb9d83f5c9c29caf2424ae4ebbc640) )
	ROM_LOAD16_BYTE( "al.p4",  0x020001, 0x10000,  CRC(5e0e3b23) SHA1(3862e7b873a1e2762032151289ce2ad886a8cc3c) )
	ROM_LOAD16_BYTE( "al.q1",  0x040000, 0x10000,  CRC(b9b50b70) SHA1(1887ab00ee004e3f27902d6880fa31277a981891) )
	ROM_LOAD16_BYTE( "al.q2",  0x040001, 0x10000,  CRC(1bed86ac) SHA1(8e6563b5441ad9ddd468a3d9ae906733fed7912a) )
	ROM_LOAD16_BYTE( "al.q3",  0x060000, 0x10000,  CRC(294d8f28) SHA1(9f9aca491ba6c4dc5cfb91da867990a9610c3a28) )
	ROM_LOAD16_BYTE( "al.q4",  0x060001, 0x10000,  CRC(aa3e9fbd) SHA1(59d0868f4c8b3f56ca31a11d2e6af83b202bb735) )
	ROM_LOAD16_BYTE( "al.q5",  0x080000, 0x10000,  CRC(503f4193) SHA1(86df379c736598ba59446961bf0666e155164e1d) )
	ROM_LOAD16_BYTE( "al.q6",  0x080001, 0x10000,  CRC(9fd77e52) SHA1(d8fdebb0fd57ab9ea9797dd386168581a45ebc62) )
	ROM_LOAD16_BYTE( "al.q7",  0x0a0000, 0x10000,  CRC(cf3fa7c7) SHA1(fa1edf09c6d3a8b5737474117b0306ef64f7741c) )
	ROM_LOAD16_BYTE( "al.q8",  0x0a0001, 0x10000,  CRC(55058a63) SHA1(cf9edef5264f4301be4ee11f221ab67a5183a603) )
	ROM_LOAD16_BYTE( "al.q9",  0x0c0000, 0x10000,  CRC(22274191) SHA1(9bee5709edcd853e96408f37447c0f5324610903) )
	ROM_LOAD16_BYTE( "al.qa",  0x0c0001, 0x10000,  CRC(1fe98b4d) SHA1(533afeaea42903905f6f1206bba1a023b141bdd9) )

ROM_END



ROM_START( v4strike )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "sils.p1", 0x000000, 0x020000,  CRC(66ed6696) SHA1(a6aa68eb212254213db5573dfb9da1e9e06a8e39) )
	ROM_LOAD16_BYTE( "sil.p2",  0x000001, 0x020000,  CRC(1afc07b7) SHA1(38777d56192b640b003d8dbf4b793cee0c81d9b2) )
	ROM_LOAD16_BYTE( "sil.p3",  0x040000, 0x020000,  CRC(40f5851c) SHA1(0e3dc0dd2a257a955a1f250556d047481ae87269) )
	ROM_LOAD16_BYTE( "sil.p4",  0x040001, 0x020000,  CRC(657e297e) SHA1(306f40376115ca40099a0010650b5edc183a2c57) )
	ROM_LOAD16_BYTE( "sil.p5",  0x080000, 0x020000,  CRC(28bced09) SHA1(7ba5013f1e0f4e921581b23c4a1d4c005a043b66) )
	ROM_LOAD16_BYTE( "sil.p6",  0x080001, 0x020000,  CRC(6f5fc296) SHA1(bd32a937581df6b5a4f08e6ef40c37a2b4278936) )

	ROM_LOAD( "strikeitlucky_questions",  0x0c0000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END

ROM_START( v4striked )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "sild.p1", 0x000000, 0x020000,  CRC(6886e7c8) SHA1(9ef0895c9a8df762d17a3ea036bf0c6c2a440582) )
	ROM_LOAD16_BYTE( "sil.p2",  0x000001, 0x020000,  CRC(1afc07b7) SHA1(38777d56192b640b003d8dbf4b793cee0c81d9b2) )
	ROM_LOAD16_BYTE( "sil.p3",  0x040000, 0x020000,  CRC(40f5851c) SHA1(0e3dc0dd2a257a955a1f250556d047481ae87269) )
	ROM_LOAD16_BYTE( "sil.p4",  0x040001, 0x020000,  CRC(657e297e) SHA1(306f40376115ca40099a0010650b5edc183a2c57) )
	ROM_LOAD16_BYTE( "sil.p5",  0x080000, 0x020000,  CRC(28bced09) SHA1(7ba5013f1e0f4e921581b23c4a1d4c005a043b66) )
	ROM_LOAD16_BYTE( "sil.p6",  0x080001, 0x020000,  CRC(6f5fc296) SHA1(bd32a937581df6b5a4f08e6ef40c37a2b4278936) )

	ROM_LOAD( "strikeitlucky_questions",  0x0c0000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END

ROM_START( v4strike2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "sil3.p1", 0x000000, 0x020000,  CRC(51c22f94) SHA1(dbdd30e67df4662d13b32a1df6cfabef2cf9471f) )
	ROM_LOAD16_BYTE( "sil.p2",  0x000001, 0x020000,  CRC(1afc07b7) SHA1(38777d56192b640b003d8dbf4b793cee0c81d9b2) )
	ROM_LOAD16_BYTE( "sil.p3",  0x040000, 0x020000,  CRC(40f5851c) SHA1(0e3dc0dd2a257a955a1f250556d047481ae87269) )
	ROM_LOAD16_BYTE( "sil.p4",  0x040001, 0x020000,  CRC(657e297e) SHA1(306f40376115ca40099a0010650b5edc183a2c57) )
	ROM_LOAD16_BYTE( "sil.p5",  0x080000, 0x020000,  CRC(28bced09) SHA1(7ba5013f1e0f4e921581b23c4a1d4c005a043b66) )
	ROM_LOAD16_BYTE( "sil.p6",  0x080001, 0x020000,  CRC(6f5fc296) SHA1(bd32a937581df6b5a4f08e6ef40c37a2b4278936) )

	ROM_LOAD( "strikeitlucky_questions",  0x0c0000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END

ROM_START( v4strike2d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "sil3d.p1", 0x000000, 0x020000,  CRC(82def1b5) SHA1(d3e90e8e4e905a3779889305bb6441907d9169f1) )
	ROM_LOAD16_BYTE( "sil.p2",  0x000001, 0x020000,  CRC(1afc07b7) SHA1(38777d56192b640b003d8dbf4b793cee0c81d9b2) )
	ROM_LOAD16_BYTE( "sil.p3",  0x040000, 0x020000,  CRC(40f5851c) SHA1(0e3dc0dd2a257a955a1f250556d047481ae87269) )
	ROM_LOAD16_BYTE( "sil.p4",  0x040001, 0x020000,  CRC(657e297e) SHA1(306f40376115ca40099a0010650b5edc183a2c57) )
	ROM_LOAD16_BYTE( "sil.p5",  0x080000, 0x020000,  CRC(28bced09) SHA1(7ba5013f1e0f4e921581b23c4a1d4c005a043b66) )
	ROM_LOAD16_BYTE( "sil.p6",  0x080001, 0x020000,  CRC(6f5fc296) SHA1(bd32a937581df6b5a4f08e6ef40c37a2b4278936) )

	ROM_LOAD( "strikeitlucky_questions",  0x0c0000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END

ROM_START( v4eyedwn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "eds.p1", 0x000000, 0x010000, CRC(bd4f4c5d) SHA1(4ad34d92d45c0b8c70af8454525dd669cab94c5c) )
	ROM_LOAD16_BYTE( "ed.p2",  0x000001, 0x010000, CRC(8804b926) SHA1(cf9d49ca090a435819f2cd2d32dec3a6767f9e10) )
	ROM_LOAD16_BYTE( "ed.p3",  0x020000, 0x010000, CRC(969d2264) SHA1(b55f6881852f81ec9fb2c57c8137c872f4714710) )
	ROM_LOAD16_BYTE( "ed.p4",  0x020001, 0x010000, CRC(80d9addd) SHA1(f6359354c928e69a90cdf6d4f514c4992d3fa64c) )

	ROM_LOAD( "eyesdown_questions",  0x040000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END

ROM_START( v4eyedwnd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "edd.p1", 0x000000, 0x010000, CRC(369a145c) SHA1(c54305690798086b4f809f38d1a6454d1d34fd0c) )
	ROM_LOAD16_BYTE( "ed.p2",  0x000001, 0x010000, CRC(8804b926) SHA1(cf9d49ca090a435819f2cd2d32dec3a6767f9e10) )
	ROM_LOAD16_BYTE( "ed.p3",  0x020000, 0x010000, CRC(969d2264) SHA1(b55f6881852f81ec9fb2c57c8137c872f4714710) )
	ROM_LOAD16_BYTE( "ed.p4",  0x020001, 0x010000, CRC(80d9addd) SHA1(f6359354c928e69a90cdf6d4f514c4992d3fa64c) )

	ROM_LOAD( "eyesdown_questions",  0x040000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END

ROM_START( v4quidgr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "qgs.p1", 0x000000, 0x010000, CRC(e6d072a9) SHA1(b2d656818c2fb12a26ef8a170152087336c00c66) )
	ROM_LOAD16_BYTE( "qg.p2",  0x000001, 0x010000, CRC(e4d74b11) SHA1(d23ccc57c54823ffaa7869ca824e049dc80f8945) )
	ROM_LOAD16_BYTE( "qg.p3",  0x020000, 0x010000, CRC(5dda7cd8) SHA1(246a801e862990aade98fa358477e53707714b42) )
	ROM_LOAD16_BYTE( "qg.p4",  0x020001, 0x010000, CRC(2106cf5d) SHA1(2073589775139ad92daef05a67afb2c70ece168c) )

	ROM_LOAD( "quidgrid_questions",  0x040000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END

ROM_START( v4quidgrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "qgd.p1", 0x000000, 0x010000, CRC(c8d37bdf) SHA1(e39a41015dd551fd62efa52faf2c5106bf7d23ac) )
	ROM_LOAD16_BYTE( "qg.p2",  0x000001, 0x010000, CRC(e4d74b11) SHA1(d23ccc57c54823ffaa7869ca824e049dc80f8945) )
	ROM_LOAD16_BYTE( "qg.p3",  0x020000, 0x010000, CRC(5dda7cd8) SHA1(246a801e862990aade98fa358477e53707714b42) )
	ROM_LOAD16_BYTE( "qg.p4",  0x020001, 0x010000, CRC(2106cf5d) SHA1(2073589775139ad92daef05a67afb2c70ece168c) )

	ROM_LOAD( "quidgrid_questions",  0x040000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END

ROM_START( v4quidgr2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "q2gs.p1", 0x000000, 0x010000, CRC(6d4b92c0) SHA1(05777e0ecc8bf9f3564d8395bb7bfa245498d132) )
	ROM_LOAD16_BYTE( "q2g.p2",  0x000001, 0x010000, CRC(9bf74ae3) SHA1(11e75acad496a9304f612714afbef90e48d1b9cb) )
	ROM_LOAD16_BYTE( "q2g.p3",  0x020000, 0x010000, CRC(78555155) SHA1(e1218fc00f08c19a0cafb203e46044efa617ac16) )
	ROM_LOAD16_BYTE( "q2g.p4",  0x020001, 0x010000, CRC(2a4295b6) SHA1(2aa0b5dbe6b934a7a4c8069c91fd6d85cae02836) )

	ROM_LOAD( "quidgrid_questions",  0x040000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END

ROM_START( v4quidgr2d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "q2gd.p1", 0x000000, 0x010000, CRC(26185449) SHA1(921d355020788d53e8ffa83af44223dbe5a2dd5e) )
	ROM_LOAD16_BYTE( "q2g.p2",  0x000001, 0x010000, CRC(9bf74ae3) SHA1(11e75acad496a9304f612714afbef90e48d1b9cb) )
	ROM_LOAD16_BYTE( "q2g.p3",  0x020000, 0x010000, CRC(78555155) SHA1(e1218fc00f08c19a0cafb203e46044efa617ac16) )
	ROM_LOAD16_BYTE( "q2g.p4",  0x020001, 0x010000, CRC(2a4295b6) SHA1(2aa0b5dbe6b934a7a4c8069c91fd6d85cae02836) )

	ROM_LOAD( "quidgrid_questions",  0x040000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END


ROM_START( v4barqst )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "bqd.p1", 0x000000, 0x010000, CRC(b574ea46) SHA1(0eb446fbf4f7fcd1b30f35631b4b521730ce26b4) )
	ROM_LOAD16_BYTE( "bq.p2", 0x000001, 0x010000, CRC(b9ce9f2e) SHA1(9407a83d1713b641dc551dd73f357d99baebbba2) )

	ROM_LOAD( "barquest_questions",  0x040000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END

ROM_START( v4barqs2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "b2qs.p1", 0x000000, 0x010000, CRC(35c1a58b) SHA1(e2d07229e7136c462c48b6c77c9d26a33deb8e34) )
	ROM_LOAD16_BYTE( "b2q.p2", 0x000001, 0x010000, CRC(28a0064a) SHA1(3751d9f3cc93994c44927ca3f72ade6bee22b20b) )

	ROM_LOAD( "barquest2_questions",  0x040000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END

ROM_START( v4wize )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "wmd.p1", 0x000000, 0x010000, CRC(45d29045) SHA1(37911346753211801f8404d42b275f49764ba5f4) )
	ROM_LOAD16_BYTE( "wm.p2",  0x000001, 0x010000, CRC(41b5fb2a) SHA1(e9ee484ec7445d58efa9bbfbd705202ef83656f2) )
	ROM_LOAD16_BYTE( "wm.p3",  0x020000, 0x010000, CRC(934da7e4) SHA1(9cac87ccadbc871577640ec0bddd5e07aef139f8) )
	ROM_LOAD16_BYTE( "wm.p4",  0x020001, 0x010000, CRC(463f6c0b) SHA1(ffee4cca73ebe7130e34118031cb16b3c42f03cb) )
	ROM_LOAD16_BYTE( "wm.p5",  0x040000, 0x010000, CRC(eaea2502) SHA1(adeda7148ee4eee98870f4aa529b5c9f36417e2e) )
	ROM_LOAD16_BYTE( "wm.p6",  0x040001, 0x010000, CRC(40a5e980) SHA1(e7bd49308b63a94a9ca0b138de0c48d2316d6aa0) )

	ROM_LOAD( "wizemove_questions",  0x080000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..


	ROM_REGION( 0x800000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "w23.p1", 0x000000, 0x010000, CRC(a8d8fb2e) SHA1(cf5462b224a7960ade867cf76079d11084f13e4b) )
	ROM_LOAD16_BYTE( "wm3.p1", 0x000000, 0x010000, CRC(0752f0f1) SHA1(2b0531312bf1d4b489394401c5d78c7f04e12aea) )
	ROM_LOAD16_BYTE( "wm3d.p1", 0x000000, 0x010000, CRC(4e3ab877) SHA1(64408d1ac1f626390ffe93e024c672ba5acb42d6) )
	ROM_LOAD16_BYTE( "wms.p1", 0x000000, 0x010000, CRC(712385c1) SHA1(075a98626eba2eae6a31b395c2a74541a31b2582) )
ROM_END

ROM_START( v4wizea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "wzm1&2p1", 0x000000, 0x010000, CRC(629e38d0) SHA1(bbc2688635c4cb9a1712a0a0d28017d1867417a4) )
	ROM_LOAD16_BYTE( "wzm1&2p2", 0x000001, 0x010000, CRC(96b9c484) SHA1(b02595ba5a5c7d673bdc7675708a4d8f6d907779) )
	ROM_LOAD16_BYTE( "wzm1&2p3", 0x020000, 0x010000, CRC(e545fac0) SHA1(4a3f9e5522bd666d5a1e9ac7878d3d78f3756762) )
	ROM_LOAD16_BYTE( "wzm1&2p4", 0x020001, 0x010000, CRC(df7d4dba) SHA1(b892cfb807421c99ba98bfd9b34d717e17345d83) )
	ROM_LOAD16_BYTE( "wzm1&2p5", 0x040000, 0x010000, NO_DUMP )
	ROM_LOAD16_BYTE( "wzm1&2p6", 0x040001, 0x010000, CRC(3eecbdf8) SHA1(9ecc4fe25e1c1e167aaa413eaf601b55e1a432fb) )

	ROM_LOAD( "wizemove_questions",  0x080000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..

ROM_END




ROM_START( v4opt3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "o3-s.p1",0x000000, 0x020000, CRC(9b825fa1) SHA1(c743bed32fa1af934f9610204a4f4062cfb85193) )
	ROM_LOAD16_BYTE( "o3-.p2", 0x000001, 0x020000, CRC(9545a845) SHA1(24c6656c137baf74c8ea5d930220bf0d0c90b3ab) )
	ROM_LOAD16_BYTE( "o3-.p3", 0x040000, 0x020000, CRC(2f3666cb) SHA1(94010eb56de7f3900fbe1c0b77a43752f9cf3ba6) )
	ROM_LOAD16_BYTE( "o3-.p4", 0x040001, 0x020000, CRC(63b6aa70) SHA1(6f0bef67437caa150ee98f0349d0f6412665e36b) )
	ROM_LOAD16_BYTE( "o3-.p5", 0x080000, 0x020000, CRC(750db328) SHA1(8442b32e2f70ba2e0849d994a1e2502634652c5e) )
	ROM_LOAD16_BYTE( "o3-.p6", 0x080001, 0x020000, CRC(1fed469e) SHA1(351a8cd687c0e11300c498e45e4b6f3a35940615) )
	ROM_LOAD16_BYTE( "o3-.p7", 0x0c0000, 0x020000, CRC(400e304f) SHA1(3abe30cf75990a5ce9066ccd4cfed680e5d4963c) )
	ROM_LOAD16_BYTE( "o3-.p8", 0x0c0001, 0x020000, CRC(0fb22311) SHA1(2329520d4ce763ba10cc81314711d28b4a5fb192) )
	ROM_LOAD16_BYTE( "o3-.p9", 0x100000, 0x020000, CRC(0d71b620) SHA1(103cf5700c876e94e2ca3b63ef71b768bd2d99a2) )
	ROM_LOAD16_BYTE( "o3-.p10",0x100001, 0x020000, CRC(239fa03b) SHA1(c4c77ded07481746b041122a8538036a65a5abe6) )
ROM_END

ROM_START( v4opt3d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "o3-d.p1",0x000000, 0x020000, CRC(09eef902) SHA1(2ce68fe08c5a1be7daaf3a380aa5f5fef8401944) )
	ROM_LOAD16_BYTE( "o3-.p2", 0x000001, 0x020000, CRC(9545a845) SHA1(24c6656c137baf74c8ea5d930220bf0d0c90b3ab) )
	ROM_LOAD16_BYTE( "o3-.p3", 0x040000, 0x020000, CRC(2f3666cb) SHA1(94010eb56de7f3900fbe1c0b77a43752f9cf3ba6) )
	ROM_LOAD16_BYTE( "o3-.p4", 0x040001, 0x020000, CRC(63b6aa70) SHA1(6f0bef67437caa150ee98f0349d0f6412665e36b) )
	ROM_LOAD16_BYTE( "o3-.p5", 0x080000, 0x020000, CRC(750db328) SHA1(8442b32e2f70ba2e0849d994a1e2502634652c5e) )
	ROM_LOAD16_BYTE( "o3-.p6", 0x080001, 0x020000, CRC(1fed469e) SHA1(351a8cd687c0e11300c498e45e4b6f3a35940615) )
	ROM_LOAD16_BYTE( "o3-.p7", 0x0c0000, 0x020000, CRC(400e304f) SHA1(3abe30cf75990a5ce9066ccd4cfed680e5d4963c) )
	ROM_LOAD16_BYTE( "o3-.p8", 0x0c0001, 0x020000, CRC(0fb22311) SHA1(2329520d4ce763ba10cc81314711d28b4a5fb192) )
	ROM_LOAD16_BYTE( "o3-.p9", 0x100000, 0x020000, CRC(0d71b620) SHA1(103cf5700c876e94e2ca3b63ef71b768bd2d99a2) )
	ROM_LOAD16_BYTE( "o3-.p10",0x100001, 0x020000, CRC(239fa03b) SHA1(c4c77ded07481746b041122a8538036a65a5abe6) )
ROM_END


/*

FRUIT FACTORY                   ID=FF_
-------------

GRAPHIC PROMS
=============

FF_GFX10.P3     4mb
FF_GFX10.P4     4MB
FF_GFX10.P5     4MB
FF_GFX10.P6     4MB
FF_GFX10.P7     4MB
FF_GFX10.P8     4MB

MPU4 PROM
=========

FFMPU416.P1 A209    27C512


GAME PROMS
==========
FF_293.P1   5F95    27C040  20p/?15
FF_293.P2   DCE6    27C040  20p/?15
FF_293D.P2  5F95    27C040  20p/?15 data
FF_293D.P2  DDE5    27C040  20p/?15 data

FF_393.P1   5F9F    27C040  25p/?15
FF_393.P2   DCE6    27C040  25p/?15
FF_393D.P2  5F9F    27C040  25p/?15 data
FF_393D.P2  DDE5    27C040  25p/?15 data

FF_493.P1   5EAA    27C040  30p/?15
FF_493.P2   DCE6    27C040  30p/?15
FF_493D.P2  5EAA    27C040  30p/?15 data
FF_493D.P2  DDE5    27C040  30p/?15 data

---------------------------------------------------------------------
 END

 */

ROM_START( v4frfact )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS // should use FFMPU416.P1  A209    27C512 .. find out which that is, if it's dumped

	ROM_REGION( 0x800000, "video", 0 )
	/* p1/p2 load the wrong way around? */
	ROM_LOAD16_BYTE( "ff_40d.p1", 0x000001, 0x080000, CRC(c91fd349) SHA1(18c36612ee9fed6f12bb3dbf08f6740cf2a463c2) )
	ROM_LOAD16_BYTE( "ff_40d.p2", 0x000000, 0x080000, CRC(e7555936) SHA1(0af7a0be17735831eb037c4bc55d35891a608b23) )

	ROM_LOAD16_BYTE( "ff_gfx10.p3", 0x100001, 0x080000, CRC(a043a1f6) SHA1(0e591f0e7ecdf8b390a20ee826705a22ed6923d5) )
	ROM_LOAD16_BYTE( "ff_gfx10.p4", 0x100000, 0x080000, CRC(58226ff5) SHA1(f2647e43da69e8aa2f78d46f3cfc553440213c36) )
	ROM_LOAD16_BYTE( "ff_gfx10.p5", 0x200001, 0x080000, CRC(9bfc6da8) SHA1(969c4a84392c28d61e87482e7f881bdfda79f879) )
	ROM_LOAD16_BYTE( "ff_gfx10.p6", 0x200000, 0x080000, CRC(240cdfd3) SHA1(8ccf199aff929813df554d957e84484a482c98c6) )
	ROM_LOAD16_BYTE( "ff_gfx10.p7", 0x300001, 0x080000, CRC(42d3bd01) SHA1(6d07875e8f251c3c9c4e7f48ae886b8069c20897) )
	ROM_LOAD16_BYTE( "ff_gfx10.p8", 0x300000, 0x080000, CRC(1951a944) SHA1(b8eca580ae43be855d93cf9f50058b2fb9e8981b) )

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision Prg ROMs, to be sorted into clones */
	// 30p/?15
	ROM_LOAD16_BYTE( "ff_493.p1", 0x000001, 0x080000, CRC(1a88e0e6) SHA1(198fffd0d98d557462485b1ca6e3460199e03924) )
	ROM_LOAD16_BYTE( "ff_493.p2", 0x000000, 0x080000, CRC(1c948e83) SHA1(571f854c33a2ef68daa8633193b49486ff92d7e2) )
	// 30p/?15 data
	ROM_LOAD16_BYTE( "ff_493d.p1", 0x000001, 0x080000, CRC(1a88e0e6) SHA1(198fffd0d98d557462485b1ca6e3460199e03924) )
	ROM_LOAD16_BYTE( "ff_493d.p2", 0x000000, 0x080000, CRC(86830e6c) SHA1(9510bf42a8c2f8ffeafd1f2b0e027a0a59d80b20) )
	// 25p/?15
	ROM_LOAD16_BYTE( "ff_393.p1", 0x000001, 0x080000, CRC(35011cab) SHA1(9e381db93dbe1f71d40b152ae6c68ea7a7b9728c) )
	ROM_LOAD16_BYTE( "ff_393.p2", 0x000000, 0x080000, CRC(1c948e83) SHA1(571f854c33a2ef68daa8633193b49486ff92d7e2) )
	// 25p/?15 data
	ROM_LOAD16_BYTE( "ff_393d.p1", 0x000001, 0x080000, CRC(35011cab) SHA1(9e381db93dbe1f71d40b152ae6c68ea7a7b9728c) )
	ROM_LOAD16_BYTE( "ff_393d.p2", 0x000000, 0x080000, CRC(86830e6c) SHA1(9510bf42a8c2f8ffeafd1f2b0e027a0a59d80b20) )
	// 20p/?15
	ROM_LOAD16_BYTE( "ff_293.p1", 0x000001, 0x080000, CRC(4787bc3d) SHA1(a1d53f1640c6d829c9fee8c72057b2801aac4cb2) )
	ROM_LOAD16_BYTE( "ff_293.p2", 0x000000, 0x080000, CRC(1c948e83) SHA1(571f854c33a2ef68daa8633193b49486ff92d7e2) )
	// 20p/?15 data
	ROM_LOAD16_BYTE( "ff_293d.p1", 0x000001, 0x080000, CRC(4787bc3d) SHA1(a1d53f1640c6d829c9fee8c72057b2801aac4cb2) )
	ROM_LOAD16_BYTE( "ff_293d.p2", 0x000000, 0x080000, CRC(86830e6c) SHA1(9510bf42a8c2f8ffeafd1f2b0e027a0a59d80b20) )
ROM_END

/* BwB */
ROM_START( v4bigfrt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	// standard/stake Key
	ROM_LOAD("bi_xca__.2_0",  0x00000, 0x10000,  CRC(bdba5ce9) SHA1(e4bce58957230183b96f9d3155575005ffb002c8))

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("bi______.2_1",  0x000000, 0x010000,  CRC(22e7e736) SHA1(478abe8dc9e29b8ceccfac80ddaa5ce2727572cd))
	ROM_LOAD16_BYTE("bi______.2_2",  0x000001, 0x010000,  CRC(9d139ae7) SHA1(fb38b3102355ee5f79f66410ce5a26f74296321a))
	ROM_LOAD16_BYTE("bi______.2_3",  0x020000, 0x010000,  CRC(9f4c424d) SHA1(f20e5e6f43ac0d481b7f2d8ea421fa6b9fa92b38))
	ROM_LOAD16_BYTE("bi______.2_4",  0x020001, 0x010000,  CRC(58f0dcb4) SHA1(79039d489f5ce3a1865fa92a2b6e8b002b63efcf))
	ROM_LOAD16_BYTE("bi______.2_5",  0x040000, 0x010000,  CRC(512c6d1a) SHA1(1c9d04e7e59a95f6975e6d2d5e5def4c1a7777e0))
	ROM_LOAD16_BYTE("bi______.2_6",  0x040001, 0x010000,  CRC(5df850ec) SHA1(5b455f4cfb19c551723f7fd5f4f95e5420f8682f))
	ROM_LOAD16_BYTE("bi______.2_7",  0x050000, 0x010000,  CRC(9ea394a2) SHA1(3b9840627f7676aa7872d2bc022406a4ced7958f))
	ROM_LOAD16_BYTE("bi______.2_8",  0x050001, 0x010000,  CRC(4aa1d37d) SHA1(3c1a3ccacdc33cd4a54b7bbf06ea4c33868705f4))

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision BIOS ROMs, to be sorted into clones (could probably do so already, but let's not complicate things)*/
	// protocol/20p/Rank Bingo
	ROM_LOAD("bi_20bt_.2_0",  0x00000, 0x10000,  CRC(0c5b21c8) SHA1(bec3a0ca87156de82024449ed5acfd9d8877f15c))
	// protocol/20p/Gala Bingo
	ROM_LOAD("bi_20bg_.2_0",  0x00000, 0x10000,  CRC(f1b5cb6a) SHA1(b26040a283d3c2642367e86f66e47d97c1ac07a4))
	// protocol and % key/20p
	ROM_LOAD("bi_20sb_.2_0",  0x00000, 0x10000,  CRC(8e646474) SHA1(8b72fba96947fb78a79997b37987eadec522cc4e))

ROM_END

ROM_START( v4bubbnk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	// Standard/20p
	ROM_LOAD("bu_20a__.4_0",  0x00000, 0x10000,  CRC(743a0c56) SHA1(6dfe6733d19b19fcb9be2472615a340237253966))

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("bu______.4_1",  0x000000, 0x010000,  CRC(1e85dd5d) SHA1(0f3c35d9f75d3495e2d0cc1fcf0e96dcbbeeacc8))
	ROM_LOAD16_BYTE("bu______.4_2",  0x000001, 0x010000,  CRC(989db70a) SHA1(e44f8900392db51cc3f0a5bf0391bde0d71b9878))
	ROM_LOAD16_BYTE("bu______.4_3",  0x020000, 0x010000,  CRC(b079fc12) SHA1(6cb736fbe07617eca3ea691e85ff17a0ae218faf))
	ROM_LOAD16_BYTE("bu______.4_4",  0x020001, 0x010000,  CRC(7e22a7cf) SHA1(00f6a303684850e0a8a168d4a6da628bc19599ea))
	ROM_LOAD16_BYTE("bu______.4_5",  0x040000, 0x010000,  CRC(225eff49) SHA1(015efc0f058172cc03f9e38794784e52c097e2bc))
	ROM_LOAD16_BYTE("bu______.4_6",  0x040001, 0x010000,  CRC(84ce8822) SHA1(5b9b24bc5e586678d415c1234b8c870776f970cb))
	ROM_LOAD16_BYTE("bu______.4_7",  0x060000, 0x010000,  CRC(a3b00c9f) SHA1(16d9f09a9c20a23521468ea332d19ec485e3606f))
	ROM_LOAD16_BYTE("bu______.4_8",  0x060001, 0x010000,  CRC(7bd03d6e) SHA1(63802bc31115410f5021cdaca2ef832ace59288d))
	ROM_LOAD16_BYTE("bu______.4_9",  0x080000, 0x010000,  CRC(18157aca) SHA1(e6666ecae620a9d8d3c9099f2d269a3cebd05d0f))
	ROM_LOAD16_BYTE("bu______.4_a",  0x080001, 0x010000,  CRC(bc76817d) SHA1(418355740310d431a2803663b5138dff4b45452f))

	ROM_REGION( 0x100000, "altrevs", 0 ) /* Alternate revision BIOS ROMs, to be sorted into clones (could probably do so already, but let's not complicate things)*/
	// Irish/20p
	ROM_LOAD("bui20a__.4_0",  0x00000, 0x10000,  CRC(154263c0) SHA1(8207ce1876be443e2023554629c0de1117544170))
	// protocol and % key/20p
	ROM_LOAD("bu_20ab_.4_0",  0x00000, 0x10000,  CRC(82be9bc5) SHA1(6bd5e08cd157c7c02259aa93ddbce19979dd85f0))
	// protocol/20p
	ROM_LOAD("bu_20ad_.4_0",  0x00000, 0x10000,  CRC(3f9febf7) SHA1(172e7e03d7b4c6fdda6bb8f3f5d93d3803f678c3))
	// % Key/20p
	ROM_LOAD("bu_20ak_.4_0",  0x00000, 0x10000,  CRC(c91b7c64) SHA1(7cb4027e43e524792e1b5e523857f5d52cc7e7d2))

ROM_END


ROM_START( v4mazbla )
	ROM_REGION( 0x10000, "maincpu", 0 )
	// standard/20p
	ROM_LOAD("maz20__1.5x0",  0x00000, 0x10000,  CRC(06f95de7) SHA1(6ee8a345b12c9993513dda686ddb8c89d847976f))

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("maz20__1.5x1",  0x000000, 0x010000,  CRC(a4be5a3f) SHA1(e510a54c7d7551dbb2c5dd450c921fd0e3649db6))
	ROM_LOAD16_BYTE("maz20__1.5x2",  0x000001, 0x010000,  CRC(261466aa) SHA1(ce61f7f1af9abed6e9fad778a270b3c4c167d401))
	ROM_LOAD16_BYTE("maz20__1.5x3",  0x020000, 0x010000,  CRC(a9f02fc6) SHA1(d892bcd3e755c623bb7f2ff4db4876c091c5ae30))
	ROM_LOAD16_BYTE("maz20__1.5x4",  0x020001, 0x010000,  CRC(b5f333b6) SHA1(b848f9d6f39e102d72992d56836cd5b8c16b5554))

	ROM_REGION( 0x100000, "altrevs", 0 )
	// protocol % key/20p/
	ROM_LOAD("maz20dy1.5x0",  0x00000, 0x10000,  CRC(f07dca74) SHA1(f5c2658911a3d8d082ca3f2289b69e767f5a80f4))

ROM_END

ROM_START( v4mazbel )
	ROM_REGION( 0x10000, "maincpu", 0 )
	// protocol % key/20p/
	ROM_LOAD("maz20dy2.5_0",  0x00000, 0x10000,  CRC(e43220df) SHA1(07b58cc40fa490564a6bf8add65a3b3c4d9b1164))

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("maz20__2.5_1",  0x000000, 0x010000,  CRC(6d3b4dbd) SHA1(79479c458395fa718f00e4c1547b186a85153848))
	ROM_LOAD16_BYTE("maz20__2.5_2",  0x000001, 0x010000,  CRC(40074cc3) SHA1(498beeed1a1f86e535474a25f3ec73c1fdb004b9))
	ROM_LOAD16_BYTE("maz20__2.5_3",  0x020000, 0x010000,  CRC(b78bc8a9) SHA1(7436cb9e67a3d72b36cbdd92d8aa3e26e6b7b40d))
	ROM_LOAD16_BYTE("maz20__2.5_4",  0x020001, 0x010000,  CRC(116a6f65) SHA1(10fa599b620f6d595b27723f7ae1c36f6ff61f7d))

ROM_END

ROM_START( v4shpwnd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	// Standard/20p/
	ROM_LOAD("sw_20___.2_0",  0x00000, 0x10000,  CRC(c1b49d81) SHA1(dcb077734beb814002046d36091a6407644c1393))

	ROM_REGION( 0x10000, "altmain", 0 )
	ROM_LOAD( "sw_10___.2_0", 0x0000, 0x010000, CRC(1377a409) SHA1(632bae8bf70270fb13caabbc491f81f8b6589ece) )
	ROM_LOAD( "sw_10_b_.2_0", 0x0000, 0x010000, CRC(e5f3339a) SHA1(67d79c2939ad0c566e05e470672d7153fdc2fbd5) )
	ROM_LOAD( "sw_10_d_.2_0", 0x0000, 0x010000, CRC(58d243a8) SHA1(1f718a153e2a42893c2dcecddf857c93872a780d) )
	ROM_LOAD( "sw_10_k_.2_0", 0x0000, 0x010000, CRC(ae56d43b) SHA1(ad73f7ad4c142d3ad629b0df46985456a2f8ebc8) )
	ROM_LOAD( "sw_20_b_.2_0", 0x0000, 0x010000, CRC(37300a12) SHA1(34abe1b28afe9c36810f9c7eba356720d9ff53de) )
	ROM_LOAD( "sw_20_d_.2_0", 0x0000, 0x010000, CRC(8a117a20) SHA1(697d97155b3c4f515988390997064ace7402959e) )
	ROM_LOAD( "sw_20_k_.2_0", 0x0000, 0x010000, CRC(7c95edb3) SHA1(4a8208c2786eed73a4ade67209a5cb66ba8c4ed8) )
	ROM_LOAD( "sw_20bg_.2_0", 0x0000, 0x010000, CRC(48e1a50c) SHA1(7f1aaa3207b86996cb1804ff82685723839e7c7c) )
	ROM_LOAD( "sw_20bt_.2_0", 0x0000, 0x010000, CRC(feaaa80f) SHA1(23ec154fe0fdb4bbb558960d700c94674bb4a144) )
	ROM_LOAD( "sw_xc___.2_0", 0x0000, 0x010000, CRC(f108aa4a) SHA1(f27456a22b781df3103cc7b7f35fb08bb547e864) )
	ROM_LOAD( "sw_xc_b_.2_0", 0x0000, 0x010000, CRC(078c3dd9) SHA1(07515a2476ae15c780da2de58de8b17f30c0ebdf) )
	ROM_LOAD( "sw_xc_d_.2_0", 0x0000, 0x010000, CRC(baad4deb) SHA1(ad3d90eefdad4e6baa10558ed5a1415ed94742b6) )
	ROM_LOAD( "sw_xc_k_.2_0", 0x0000, 0x010000, CRC(4c29da78) SHA1(0431016a0a13d97fc833e31bc2897e7abb6a5047) )


	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("sw______.2_1",  0x000000, 0x010000,  CRC(3346ee38) SHA1(29659ba8173b86bb52057f75dc874465048ef6d9))
	ROM_LOAD16_BYTE("sw______.2_2",  0x000001, 0x010000,  CRC(2c5293c8) SHA1(e0b5fa629e8dbbc6164f16cbbe19d0e42467d281))
	ROM_LOAD16_BYTE("sw______.2_3",  0x020000, 0x010000,  CRC(09144ac0) SHA1(d55658a361ebec0e054e3a9d99eba61c81fde619))
	ROM_LOAD16_BYTE("sw______.2_4",  0x020001, 0x010000,  CRC(a23db51a) SHA1(9516a9655c5f7f000475df03f05f332c5ec09959))

ROM_END

/* Nova */

ROM_START( v4cybcas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ccd_____.6_0", 0x0000, 0x010000, CRC(93aa1a3d) SHA1(dce4cc4bfc616163ca240c417d1ef9a0a19e5901) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ccd_____.6_1", 0x000001, 0x080000, CRC(58c8ae96) SHA1(9f3aea47ae014c61bb2b75664210a3609996f983) )
	ROM_LOAD16_BYTE( "ccd_____.6_2", 0x000000, 0x080000, CRC(f0556749) SHA1(2f35a9f9672219001c2a48d7f103ac99e1b21b32) )
	ROM_LOAD16_BYTE( "ccd_____.6_3", 0x100001, 0x080000, CRC(4d2f5485) SHA1(a8c9b5843384ce55a10531b82766f5498dc0d334) )
	ROM_LOAD16_BYTE( "ccd_____.6_4", 0x100000, 0x080000, CRC(87b29468) SHA1(89a3855754e8a720ca76fa16b11f791ade75fb78) )
	ROM_LOAD16_BYTE( "ccd_____.6_5", 0x200001, 0x080000, CRC(bd8be9ff) SHA1(c5d620774e476e6ec00876e513ff838af102ecc9) )
	ROM_LOAD16_BYTE( "ccd_____.6_6", 0x200000, 0x080000, CRC(faf65e25) SHA1(f50369348740ba4c878455c108ac3e8d2262ef5e) )
	ROM_LOAD16_BYTE( "ccd_____.6_7", 0x300001, 0x080000, CRC(4cd1461c) SHA1(00379212130d5e9c1c364191f67a35cc5eca8b72) )
	ROM_LOAD16_BYTE( "ccd_____.6_8", 0x300000, 0x080000, CRC(08ce378f) SHA1(035b0ff4d18c09a385002db73c54b8dac92dfa8a) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4miami )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mdd4_0.bin", 0x0000, 0x010000, CRC(0d868466) SHA1(3cea446f094ae3b4f56163ccf01cd31c15dca03f) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md41.p1", 0x000001, 0x080000, CRC(9567347f) SHA1(5188b3f359ff26c0854d5745df37575851502ae9) )
	ROM_LOAD16_BYTE( "md42.p2", 0x000000, 0x080000, CRC(e0dad693) SHA1(9768331a4a776906935b3c5501b68cb4dd1bd41f) )
	ROM_LOAD16_BYTE( "md43.p3", 0x100001, 0x080000, CRC(9bceb3f6) SHA1(5be84d5f1635f80a9fe8072c2d94012ed00d97be) )
	ROM_LOAD16_BYTE( "md44.p4", 0x100000, 0x080000, CRC(54b8fbfa) SHA1(ca2fb67972507a2eb33d2800a3b2d45d3ee49289) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "md41snd.p1", 0x000000, 0x080000, CRC(7a3ba770) SHA1(e620f521d16b39be9b41b934435812afff3993b2) )
	ROM_LOAD( "md42snd.p2", 0x080000, 0x080000, CRC(8ebc7329) SHA1(82ce25c1486a8619355f363125a26d8cdeb05d33) )
	ROM_LOAD( "md43snd.p3", 0x100000, 0x080000, CRC(14f4d838) SHA1(0508890a9884fbef0e194a38fe3afc5cf5282091) )
ROM_END


ROM_START( v4missis )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mldsf___.6_0", 0x0000, 0x010000, CRC(3f0fe40e) SHA1(6fd9ddede5f11c35d47c7ddd360b0021267e7e3f) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mld_____.6_1", 0x000000, 0x080000, CRC(44e26e6e) SHA1(ba96757b74e75d094b6cda340c78754f1b8f6a7c) )
	ROM_LOAD16_BYTE( "mld_____.6_2", 0x000001, 0x080000, CRC(e9219fff) SHA1(80c0211b98c4200ad9512e89b067018ce8600494) )
	ROM_LOAD16_BYTE( "mld_____.6_3", 0x100000, 0x080000, CRC(680bd284) SHA1(33dfd0cb396c1466c2a5cfc474756b8a2d966518) )
	ROM_LOAD16_BYTE( "mld_____.6_4", 0x100001, 0x080000, CRC(0e21022b) SHA1(30465321a976064f7841b2c3314244b744a45092) )
	ROM_LOAD16_BYTE( "mld_____.6_5", 0x200000, 0x080000, CRC(cd6c39d2) SHA1(c4d4b5c7a1f3dcfdc464fff29f10ccee932f265a) )
	ROM_LOAD16_BYTE( "mld_____.6_6", 0x200001, 0x080000, CRC(5ad997a3) SHA1(0fd75b4a9b5991fda9cc3373b86f466492c3b4bb) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mld_snd_.1_1", 0x000000, 0x080000, CRC(8c26fe12) SHA1(0532126d8e283b4567a4cdf2bb807f5471c84832) )
	ROM_LOAD( "mld_snd_.1_2", 0x080000, 0x080000, CRC(9ab841b6) SHA1(7828c6144777621859b85a3ec92760d353576527) )
	ROM_LOAD( "mld_snd_.1_3", 0x100000, 0x080000, CRC(3f068632) SHA1(5e43da287b3aa163493c1be03ebee28ef58c44a1) )
	ROM_LOAD( "mld_snd_.1_4", 0x180000, 0x080000, CRC(f78f7221) SHA1(dac88abee6a5fdf7b69c5d39345ee05c1ac47314) )
ROM_END

ROM_START( v4picdil )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pcdsf___.a_0", 0x0000, 0x010000, CRC(45a15082) SHA1(d7655722c7ad9f3b1b2663d85287ae185917d677) )
	ROM_LOAD( "pcdsf__e.a_0", 0x0000, 0x010000, CRC(0ed1c7af) SHA1(c19ff141fba7fd1f2cf0b152e3c6df61c6b27b46) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "pcd_____.a_1", 0x000001, 0x080000, CRC(0ed561d4) SHA1(899c76d2a2352988a01bece2a229deb934a00892) )
	ROM_LOAD16_BYTE( "pcd_____.a_2", 0x000000, 0x080000, CRC(bffe3863) SHA1(ea6fdcce470e8a50c35f43fd86e39524be2db3a3) )
	ROM_LOAD16_BYTE( "pcd_____.a_3", 0x100001, 0x080000, CRC(3b64a328) SHA1(a9fa9dc30b352388906e6021bc0d1ad7c3a28746) )
	ROM_LOAD16_BYTE( "pcd_____.a_4", 0x100000, 0x080000, CRC(25faba03) SHA1(572aaee3af3b915294ba057b7ceb653dd135098b) )
	ROM_LOAD16_BYTE( "pcd_____.a_5", 0x200001, 0x080000, CRC(275f3c1c) SHA1(1d0f8f7d0388d5072ae404f10b2481153979a217) )
	ROM_LOAD16_BYTE( "pcd_____.a_6", 0x200000, 0x080000, CRC(148ecba0) SHA1(2ae0f5529fa3951025539fe19f4e8fdf10f13374) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END



ROM_START( v4big40 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b4_sja__.3_0", 0x0000, 0x010000, CRC(afdd005e) SHA1(71730b55212fbfc1905caf0d2842d495741305f1) )

	ROM_REGION( 0x10000, "altmain", 0 )
	ROM_LOAD( "b4_sja__.4_0", 0x0000, 0x010000, CRC(4375e090) SHA1(923eadc4cf322e544102cede2eb7a487354981b5) )
	ROM_LOAD( "b4_sja__.6_0", 0x0000, 0x010000, CRC(f5742e6c) SHA1(42b0b22d2da690b24166fe103a3453dcfc7beacc) )
	ROM_LOAD( "b4_sja_n.6_0", 0x0000, 0x010000, CRC(be04b941) SHA1(8d0561a623df6fc447fbcbbc1934425b23df4f58) )
	ROM_LOAD( "b4_sjad_.6_0", 0x0000, 0x010000, CRC(bed1c9cd) SHA1(f4cfb305e4ec2c4c1b55356ce32745875f1641d9) )
	ROM_LOAD( "b4_sjadn.6_0", 0x0000, 0x010000, CRC(f5a15ee0) SHA1(d61fc54724d579d7a7055a1cfe20b55e4e59a653) )
	ROM_LOAD( "b4_sjmb_.6_0", 0x0000, 0x010000, CRC(7472fa9c) SHA1(3315b37b1a221e40e6122612c8ee8a27b6131947) )
	ROM_LOAD( "b4_sjs__.6_0", 0x0000, 0x010000, CRC(0092b6a9) SHA1(cead4988d2dc6b0e8724c7b1c6eb416317a9b7c4) )
	ROM_LOAD( "b4_sjs_n.6_0", 0x0000, 0x010000, CRC(4be22184) SHA1(5b779d8fe85db719b539f93ff05a6e3568968557) )
	ROM_LOAD( "b4_sjsb_.6_0", 0x0000, 0x010000, CRC(f616213a) SHA1(bcfc7fd1639ed423faa32d066c7f19877e4d1bc2) )
	ROM_LOAD( "b4_sjsbn.6_0", 0x0000, 0x010000, CRC(bd66b617) SHA1(522c0903405f1a97c140c2aaef543aff2057e6ba) )
	ROM_LOAD( "b4_sjsd_.6_0", 0x0000, 0x010000, CRC(4b375108) SHA1(77e5a59ca524550c68231cf64387e8f206f97393) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "b4______.3_1", 0x000001, 0x080000, CRC(c388e5a9) SHA1(baafe2da91891f288debd89907d36438494e876a) )
	ROM_LOAD16_BYTE( "b4______.3_2", 0x000000, 0x080000, CRC(cc3ab5c3) SHA1(a3778b462a823fd73c1a3463c53ef0537e8d5ed4) )

	ROM_REGION( 0x800000, "altvideo", 0 )
	ROM_LOAD16_BYTE( "b4______.4_1", 0x000001, 0x080000, CRC(fc33c0fc) SHA1(838a7ef4252f9f736639858fe97a4982a89be09f) )
	ROM_LOAD16_BYTE( "b4______.4_2", 0x000000, 0x080000, CRC(f2211865) SHA1(5bcb95a079f57305d3e58fae3899bceec211f44a) )
	ROM_LOAD16_BYTE( "b4______.6_1", 0x000001, 0x080000, CRC(75532ad3) SHA1(0584261faede35f6e846ee398081fac66aea8368) )
	ROM_LOAD16_BYTE( "b4______.6_2", 0x000000, 0x080000, CRC(03ef74c5) SHA1(fa5d27b0e94c8d05f1c50b28b96f7a4ae3ecda4a) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "b4__snd_.1_a", 0x0000, 0x080000, CRC(2d630b87) SHA1(e4be02a1356735c47934f8f30e1e2462bf28968c) )
ROM_END


ROM_START( v4bulblx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bv_50___.2_0", 0x0000, 0x010000, CRC(615f9772) SHA1(56bc2c78c8b50b7250906ad43211f26c2e7e17b8) )

	ROM_REGION( 0x10000, "altmain", 0 )
	ROM_LOAD( "bv_1p___.2_0", 0x0000, 0x010000, CRC(f3101261) SHA1(e2c82222e3384620f52e15350ab38e62beabe1b3) )
	ROM_LOAD( "bv_1p_d_.2_0", 0x0000, 0x010000, CRC(b8b5f5c0) SHA1(84bf9b5a3ca5ea7eae1ab97af481206fb87bd2c5) )
	ROM_LOAD( "bv_50_d_.2_0", 0x0000, 0x010000, CRC(2afa70d3) SHA1(11da4db50162054deb753ab85d11566afef2b801) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "bv_50___.2_1", 0x000000, 0x010000, CRC(bb80cd18) SHA1(7e16100c30fe8d6feb1ac206447269e9c29489a5) )
	ROM_LOAD16_BYTE( "bv_50___.2_2", 0x000001, 0x010000, CRC(2a18b8dc) SHA1(c2fccc7eae41176941ec15c421430b8bb20f72bd) )
	ROM_LOAD16_BYTE( "bv______.2_3", 0x020000, 0x010000, CRC(d249b9dd) SHA1(b624759b10ac12146e6026e52d5d96fe4f7663b3) )
	ROM_LOAD16_BYTE( "bv______.2_4", 0x020001, 0x010000, CRC(f5fffa95) SHA1(17c63a7e83feb0cafca34c30b334db082bb2c321) )
	ROM_LOAD16_BYTE( "bv______.2_5", 0x040000, 0x010000, CRC(c5775387) SHA1(b301392ae39298284ae256c819877ae287861cc8) )
	ROM_LOAD16_BYTE( "bv______.2_6", 0x040001, 0x010000, CRC(4443fddc) SHA1(fb4972620f9aa07f8bd62701f64b7902567d34db) )

	ROM_REGION( 0x800000, "altvideo", 0 )
	ROM_LOAD16_BYTE( "bv_1p___.2_1", 0x000000, 0x010000, CRC(568ea314) SHA1(fa65f2cc4734b9987a153fbf1ac4b32cd0fe4f32) )
	ROM_LOAD16_BYTE( "bv_1p___.2_2", 0x000001, 0x010000, CRC(b91f8c3b) SHA1(82e1582f75595283a4ea9f2dc39e0d5a0d5ccb57) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END


ROM_START( v4cshinf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ci_20s__.4_0", 0x0000, 0x010000, CRC(7a62c68b) SHA1(b892e023cd5ec2a508d36075ba5a96320408a4ab) )

	ROM_REGION( 0x10000, "altmain", 0 )
	ROM_LOAD( "ci_05a__.4_0", 0x0000, 0x010000, CRC(3426fb02) SHA1(84be34a727444232c595c7adaa0a1d733b5416a3) )
	ROM_LOAD( "ci_10a__.4_0", 0x0000, 0x010000, CRC(5d4767c6) SHA1(291cf48c387fc515dce6eb22b1b38aaa65aec4ac) )
	ROM_LOAD( "ci_20.10", 0x0000, 0x010000, CRC(80a4bfb3) SHA1(b3c11bd621457d190eeab423bc15895b0c7cf6da) )
	ROM_LOAD( "ci_20a__.4_0", 0x0000, 0x010000, CRC(8f845e4e) SHA1(1b8e46fed36db1b1539df3e28fcbb739079d3960) )
	ROM_LOAD( "ci_20bg_.4_0", 0x0000, 0x010000, CRC(f337fe06) SHA1(eef073d04a08347ad4814cf0d15615d45ecc8314) )
	ROM_LOAD( "ci_20bgp.4_0", 0x0000, 0x010000, CRC(65d6d05c) SHA1(3d0bd44cb692ae54ba0b2186d645124dc12dc5b3) )
	ROM_LOAD( "ci_20bt_.4_0", 0x0000, 0x010000, CRC(457cf305) SHA1(122e7f494043ebba01b5b827409d419ac54b240e) )
	ROM_LOAD( "ci_20btp.4_0", 0x0000, 0x010000, CRC(d39ddd5f) SHA1(6c97f3c19aea51efec4e510c46b98fbf3a2356a5) )
	ROM_LOAD( "ci_20sb_.4_0", 0x0000, 0x010000, CRC(8ce65118) SHA1(57207f1ef7e98eac7f31671789fb1e01e87c2aa0) )
	ROM_LOAD( "ci_20sbp.4_0", 0x0000, 0x010000, CRC(1a077f42) SHA1(141f7c3708bde43302a68df65d74d4a2e7296a05) )
	ROM_LOAD( "ci_20sd_.4_0", 0x0000, 0x010000, CRC(31c7212a) SHA1(b34aee87d4be460f47c894807ef877700b6be60f) )
	ROM_LOAD( "ci_20sk_.4_0", 0x0000, 0x010000, CRC(c743b6b9) SHA1(0ef3de1663527a770066e872699ee281f1af21eb) )
	ROM_LOAD( "ci_xea__.4_0", 0x0000, 0x010000, CRC(bf386985) SHA1(a7eec0d70bc0795f537ec0975dc251739b86c3ca) )
	ROM_LOAD( "ci_xea_p.4_0", 0x0000, 0x010000, CRC(29d947df) SHA1(b08ca18442a701eb6a6f1ffc868333e65aa4aeb2) )
	ROM_LOAD( "ci_xeab_.4_0", 0x0000, 0x010000, CRC(49bcfe16) SHA1(0feab2f177ef6bd1d1c7ae9507a247c7afab7e97) )
	ROM_LOAD( "ci_xead_.4_0", 0x0000, 0x010000, CRC(f49d8e24) SHA1(f24b41ae866b324af3605f1bc121cd445b9ecb8c) )
	ROM_LOAD( "ci_xeak_.4_0", 0x0000, 0x010000, CRC(021919b7) SHA1(d8176be8b4548a5b61af415e7b7d0d1c8742592b) )
	ROM_LOAD( "ci_xes__.4_0", 0x0000, 0x010000, CRC(4adef140) SHA1(1003e666532fd773617d4ed30380eda0089027fe) )
	ROM_LOAD( "ci_xesd_.4_0", 0x0000, 0x010000, CRC(017b16e1) SHA1(c352c7f635ece2faceb2f3ee6154c9fed86ddc6f) )
	ROM_LOAD( "ciixea__.3_0", 0x0000, 0x010000, CRC(252f43e5) SHA1(5443a53a50fb27c3e98fa4612b4ba476cc946662) )
	ROM_LOAD( "ciixea__.4_0", 0x0000, 0x010000, CRC(de400613) SHA1(b353c47b01c3918677f1a2c302171bde7386daf3) )
	ROM_LOAD( "ciixead_.3_0", 0x0000, 0x010000, CRC(6e8aa444) SHA1(63cecb83aa8e4f94b16d6c2ded8dbe8aa98d88a8) )
	ROM_LOAD( "ciixead_.4_0", 0x0000, 0x010000, CRC(95e5e1b2) SHA1(b823b1c873e209fee61f32f458d6d4f5ec7e206b) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ci______.4_1", 0x000000, 0x010000, CRC(bd1adec6) SHA1(7a60adc4b5159e672eb4dc33051192f0cf863aef) )
	ROM_LOAD16_BYTE( "ci______.4_2", 0x000001, 0x010000, CRC(e46852ff) SHA1(ad6f0a75578041b06120d1d5f3c3779c827b8f2f) )
	ROM_LOAD16_BYTE( "ci______.4_3", 0x020000, 0x010000, CRC(b63b9edf) SHA1(2a47c97f5ea51bc441fb847da6e81451dde77093) )
	ROM_LOAD16_BYTE( "ci______.4_4", 0x020001, 0x010000, CRC(7ef4daeb) SHA1(f889f4e68dc4ef736eee44ddedaa94832c6cdbc7) )
	ROM_LOAD16_BYTE( "ci______.4_5", 0x040000, 0x010000, CRC(381277b7) SHA1(6d934d400468c067809452981577c17cf425ff4f) )
	ROM_LOAD16_BYTE( "ci______.4_6", 0x040001, 0x010000, CRC(e245c830) SHA1(315b8163731abe5c65bebd94884431e99253740b) )
	ROM_LOAD16_BYTE( "ci______.4_7", 0x060000, 0x010000, CRC(169d6ac5) SHA1(7d9e63d9ab29fbace754e8c46d4e80b1ab2d422d) )
	ROM_LOAD16_BYTE( "ci______.4_8", 0x060001, 0x010000, CRC(12de2ae0) SHA1(734207fffd918f3a0890ec6a2d442efb428adbaa) )
	ROM_LOAD16_BYTE( "ci______.4_9", 0x080000, 0x010000, CRC(f1f9987f) SHA1(0a4b5fa61e237e1e209301a07af2ad1e9fedcc35) )
	ROM_LOAD16_BYTE( "ci______.4_a", 0x080001, 0x010000, CRC(4747cb48) SHA1(ac33d318f6fff67c8a2f7d47c0ee0bcddfc2af8e) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END



ROM_START( v4dbltak )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dt_sja_n.4_0", 0x0000, 0x010000, CRC(c6578f37) SHA1(e3fedb25a420bc8565fc07fa6a82ce1d584a7fd1) )

	ROM_REGION( 0x10000, "altmain", 0 )
	ROM_LOAD( "dt_sjadn.4_0", 0x0000, 0x010000, CRC(8df26896) SHA1(6e19e56ba376c063e207b217a120f95041785b0f) )
	ROM_LOAD( "dt_sjsbn.4_0", 0x0000, 0x010000, CRC(c5358061) SHA1(aaf0b20175b63bb06524edbcb0b2e8a2b3ca7df2) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "dt______.4_1", 0x000001, 0x080000, CRC(786aaee8) SHA1(309b15bbc597891407d384ded0bc246c5aa50106) )
	ROM_LOAD16_BYTE( "dt______.4_2", 0x000000, 0x080000, CRC(0b5024ae) SHA1(4385b06021d349c8ee624afc1736666f94152a85) )
	ROM_LOAD16_BYTE( "dt______.4_3", 0x100001, 0x080000, CRC(b715bff9) SHA1(be8ef30b50c235e78a03ea83eadd7541ad96f7a1) )
	ROM_LOAD16_BYTE( "dt______.4_4", 0x100000, 0x080000, CRC(f41f2566) SHA1(e18e019bb04003e0fdce3a3051da0c618bdaef3d) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END



ROM_START( v4gldrsh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_27s__.860", 0x0000, 0x010000, CRC(88dd4ff9) SHA1(8edfbb622cd542828f5184f92b357dd9094c2cb0) )

	ROM_REGION( 0x10000, "altmain", 0 )
	ROM_LOAD( "go_20a__.8_0", 0x0000, 0x010000, CRC(57332f87) SHA1(b340107d8679c0a39887e26c7ceddd0e4bbf9047) )
	ROM_LOAD( "go_20a_s.8_0", 0x0000, 0x010000, CRC(c1d201dd) SHA1(11d2f2a9022aba402667621d91d29d5a47983d9f) )
	ROM_LOAD( "go_20ab_.8_0", 0x0000, 0x010000, CRC(a1b7b814) SHA1(5c03e4d6754429b97590e644106d0c963ad46c53) )
	ROM_LOAD( "go_20bg_.8_0", 0x0000, 0x010000, CRC(2b808fcf) SHA1(30394973e0cd4f265dbcd9b4e836a67e0894ae04) )
	ROM_LOAD( "go_20bgs.8_0", 0x0000, 0x010000, CRC(bd61a195) SHA1(9775e45c4c9dfbbe655f27e4c5cf4bb86f9a8b38) )
	ROM_LOAD( "go_20bt_.8_0", 0x0000, 0x010000, CRC(9dcb82cc) SHA1(8283ddaccb4fefbb056883c18165ec52c3df494f) )
	ROM_LOAD( "go_20bts.8_0", 0x0000, 0x010000, CRC(0b2aac96) SHA1(115643b3b746ae97f1aa8a5f715b8ff5ec05bd83) )
	ROM_LOAD( "go_20s__.8_0", 0x0000, 0x010000, CRC(a2d5b742) SHA1(1f6d58794226a09609b07d1b66642fe86e808f2e) )
	ROM_LOAD( "go_20sb_.8_0", 0x0000, 0x010000, CRC(545120d1) SHA1(76bcd72c2ffb316ee8cafc6426615f297fe2e845) )
	ROM_LOAD( "go_20sbs.8_0", 0x0000, 0x010000, CRC(c2b00e8b) SHA1(71e4a926de3fe90a38407c6bbc59fdf420260cef) )
	ROM_LOAD( "go_27sd_.860", 0x0000, 0x010000, CRC(c378a858) SHA1(53f0d357daeb0b7a6bdf9a4bbf852b39bd4a337f) )
	ROM_LOAD( "go_37s__.860", 0x0000, 0x010000, CRC(07cf24d7) SHA1(305f1f46b7082a25efd32c2960db5961fba249d0) )
	ROM_LOAD( "go_37sd_.860", 0x0000, 0x010000, CRC(4c6ac376) SHA1(9d4d39654638e6adad64ab13bc7b145c93278653) )
	ROM_LOAD( "go_x7s__.860", 0x0000, 0x010000, CRC(b8617832) SHA1(8a3c8de816a39f3bd0d5ba012e65c690468e5574) )
	ROM_LOAD( "go_x7sd_.860", 0x0000, 0x010000, CRC(f3c49f93) SHA1(bdd9de7be43dce45e435f2fbc7dd770089406dec) )
	ROM_LOAD( "go_xea__.8_0", 0x0000, 0x010000, CRC(678f184c) SHA1(36738f6608fd09e78ae158d0cbcd827d3ae5bb07) )
	ROM_LOAD( "go_xea_s.8_0", 0x0000, 0x010000, CRC(f16e3616) SHA1(a4e46bc464201c8552ec6cb7253228ee15f73f74) )
	ROM_LOAD( "go_xead_.8_0", 0x0000, 0x010000, CRC(2c2affed) SHA1(893a819ec8bb99d08eaace47487f22b6edc3f363) )
	ROM_LOAD( "go_xeads.8_0", 0x0000, 0x010000, CRC(bacbd1b7) SHA1(3dee2487f91a815fa450955cae3afb0a6980921d) )
	ROM_LOAD( "goi20s__.8_0", 0x0000, 0x010000, CRC(c3add8d4) SHA1(1b9bd696818eaa57805b5efcf21b5a3a52513d87) )
	ROM_LOAD( "goixes__.8_0", 0x0000, 0x010000, CRC(f311ef1f) SHA1(22b81313e62a3cd36d97fc6e39b6a3815eb5a7f3) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END


ROM_START( v4mdice )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_37s__.8_0", 0x0000, 0x010000, CRC(bcd47ec0) SHA1(2f32ae16eada13a826cde1162a56f43079f983fb) )

	ROM_REGION( 0x10000, "altmain", 0 )
	ROM_LOAD( "md_02a__.8_0", 0x0000, 0x010000, CRC(35ed5c15) SHA1(86409c70d7ba5b7a587753abf8180e424d48cc08) )
	ROM_LOAD( "md_12a__.8_0", 0x0000, 0x010000, CRC(267d19cb) SHA1(a386abfc3296cda0f22f2f9f33902d19a99af16f) )
	ROM_LOAD( "md_16a__.8_0", 0x0000, 0x010000, CRC(cf218b9c) SHA1(78798e9fcb8260e6c27ab363bcea0d30f8e01ab5) )
	ROM_LOAD( "md_22a__.8_0", 0x0000, 0x010000, CRC(12cdd7a9) SHA1(457a10f02e3c2093f247e2332612a5200ebf3884) )
	ROM_LOAD( "md_22sd_.8_0", 0x0000, 0x010000, CRC(ac8ea8cd) SHA1(31cc328aa803e232a57572fbc9ea6ac90177ad37) )
	ROM_LOAD( "md_26a__.8_0", 0x0000, 0x010000, CRC(fb9145fe) SHA1(8173755dd82b49a730858ccec27fbc2a6b0c416a) )
	ROM_LOAD( "md_26a_s.8_0", 0x0000, 0x010000, CRC(6d706ba4) SHA1(0c1323b84a247454d4272ac5be595f453b004037) )
	ROM_LOAD( "md_26ad_.8_0", 0x0000, 0x010000, CRC(b034a25f) SHA1(70a534b07584f86b3428dbc86b2e9278989b9d0e) )
	ROM_LOAD( "md_26s__.8_0", 0x0000, 0x010000, CRC(0e77dd3b) SHA1(0989d08d6119c36841697974983397d6327d7a3e) )
	ROM_LOAD( "md_26sb_.8_0", 0x0000, 0x010000, CRC(f8f34aa8) SHA1(d5e238ebd62da2fad4d208b2e7ca71cfc3d044d9) )
	ROM_LOAD( "md_26sbs.8_0", 0x0000, 0x010000, CRC(6e1264f2) SHA1(a85e257958e94623bfc63107d3b483a19bc80f10) )
	ROM_LOAD( "md_26sd_.8_0", 0x0000, 0x010000, CRC(45d23a9a) SHA1(0a825f50a2e3ddfaad710b9dd4e3740f0b2480b6) )
	ROM_LOAD( "md_26sk_.8_0", 0x0000, 0x010000, CRC(b356ad09) SHA1(06e97be819203d183d10c56fa447b57b7fd5d594) )
	ROM_LOAD( "md_27a__.8_0", 0x0000, 0x010000, CRC(5aa2a3db) SHA1(e85a982d2ee515a53956ef457b52f3368da038c7) )
	ROM_LOAD( "md_27ad_.8_0", 0x0000, 0x010000, CRC(1107447a) SHA1(a5803004b22564611d6073bc6ead88fca4318ce9) )
	ROM_LOAD( "md_27s__.8_0", 0x0000, 0x010000, CRC(af443b1e) SHA1(e13b5880f0abf173596bbd7b029221a35bdc0c6c) )
	ROM_LOAD( "md_27sb_.8_0", 0x0000, 0x010000, CRC(59c0ac8d) SHA1(5abf26e90e0a63cfb47a3a4ee9ef0bbc56123022) )
	ROM_LOAD( "md_27sd_.8_0", 0x0000, 0x010000, CRC(e4e1dcbf) SHA1(ed0ff22f106350ec614e4ec37f14c1c51468bb66) )
	ROM_LOAD( "md_37a__.8_0", 0x0000, 0x010000, CRC(4932e605) SHA1(6db4ce36708611938ef58586af976b12b45b96fe) )
	ROM_LOAD( "md_37ad_.8_0", 0x0000, 0x010000, CRC(029701a4) SHA1(88e56394664b807812d2e47e96f8f1280beed6d9) )
	ROM_LOAD( "md_37sb_.8_0", 0x0000, 0x010000, CRC(4a50e953) SHA1(d8b1751eafa6c853f0d054e56df8c9d80e5a00e6) )
	ROM_LOAD( "md_37sd_.8_0", 0x0000, 0x010000, CRC(f7719961) SHA1(6c1a26878adf689cf3aa259356239f364848f681) )
	ROM_LOAD( "mdi25a__.8_0", 0x0000, 0x010000, CRC(c3b469d0) SHA1(00cd85bd9acd477f2ab4baf83e7bb10d763b1e93) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mdv58p1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "mdv58p2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "mdv58p3", 0x100001, 0x080000, CRC(0d907e37) SHA1(b6ad78a4a7bc877d2152907df2317621f00bdc1c) )
	ROM_LOAD16_BYTE( "mdv58p4", 0x100000, 0x080000, CRC(2e21c249) SHA1(d5192339313a8dd234cb164ca0094d9a7b64ccc2) )

	ROM_REGION( 0x800000, "altvideo", 0 )
	ROM_LOAD( "md______.8_1", 0x0000, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD( "md______.8_2", 0x0000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD( "md______.8_3", 0x0000, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD( "md______.8_4", 0x0000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )


	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END



ROM_START( v4monte )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "montecarloorbustmpu4cardrom.bin", 0x0000, 0x010000, CRC(44967b33) SHA1(92d35d1b0edcc2eef1062468722c80ef8208b437) ) // Monte Carlo Or Bust Release D (c)1996

	/* Some roms were simply in a set marked as Monte Carlo, but the 2 letter code mn) is the same */
	/* The roms were a mixture of
	    VIDEO 5 - MONTE CARLO OR BUST!
	    others contain
	    NM4 Monte Carlo on Options4

	    is the latter just an later revision, or a different game? (date suggests it isn't earlier)

	*/

	ROM_REGION( 0x10000, "altmain", 0 ) // alt MPU4 interface ROMS
	// Monte Carlo Or Bust Release B (c)1995
	ROM_LOAD( "mcob8ac", 0x0000, 0x010000, CRC(6dceb28f) SHA1(dc3daee15c25470501ab11e7b34cfef7edf302d4) )
	ROM_LOAD( "mcobarc", 0x0000, 0x010000, CRC(f95045dc) SHA1(463af12feed6dfe5f5e23d584c4eac121672918a) )
	ROM_LOAD( "mnb51020", 0x0000, 0x010000, CRC(c9ec7217) SHA1(006a70fb4050d726ae80678dc49afbd8c2c0c124) )

	// Monte Carlo Or Bust Release C (c)1995
	ROM_LOAD( "mn_", 0x0000, 0x010000, CRC(768bcc18) SHA1(d895f5207145a9d7b93b6b88fefc5be5ccf3eb72) )

	// Monte Carlo Or Bust Release D (c)1996
	ROM_LOAD( "mcob2025", 0x0000, 0x010000, CRC(0fe6ec1e) SHA1(de5b7edb40b9ab3fc9111eb83061d55ce569afdd) )
	ROM_LOAD( "mcob5vd", 0x0000, 0x010000, CRC(44967b33) SHA1(92d35d1b0edcc2eef1062468722c80ef8208b437) )
	ROM_LOAD( "mcobvd~1", 0x0000, 0x010000, CRC(e0b4bbab) SHA1(bd806a286464d36ff0cbff214ed60ccf81fd2db9) )
	ROM_LOAD( "mcovd10", 0x0000, 0x010000, CRC(3f5adbd5) SHA1(f1876afd345e8398c1dc00b63be7d5a91f57519f) )
	ROM_LOAD( "mn_xes__.d_0", 0x0000, 0x010000, CRC(25ee14a5) SHA1(2eba1b5b68a9f7ad6aacbdf6b93f6b4569164258) )
	ROM_LOAD( "mnixes__.d_0", 0x0000, 0x010000, CRC(44967b33) SHA1(92d35d1b0edcc2eef1062468722c80ef8208b437) )
	ROM_LOAD( "mn_37s__.d60", 0x0000, 0x010000, CRC(b048b0fb) SHA1(277622362d4d31b3e6386b27d1e043a88afea0ab) )

	// Monte Carlo Or Bust Release E (c)1996
	ROM_LOAD( "mn_x7s__.e60", 0x0000, 0x010000, CRC(a883207a) SHA1(948990c254eaaee7ab7c4d832290069170b2da78) )
	ROM_LOAD( "mn_xea__.e_0", 0x0000, 0x010000, CRC(776d4004) SHA1(4bc0f5688f6fb39e9398a330317f281c4c7cb1cd) )
	ROM_LOAD( "mn_xea_s.e_0", 0x0000, 0x010000, CRC(e18c6e5e) SHA1(b008129c50b0b9031f1324fb3feac5ac50695e87) )
	ROM_LOAD( "mn_xead_.e_0", 0x0000, 0x010000, CRC(3cc8a7a5) SHA1(6d00e4ff02aee210d940d53d1bc0a3c8c57a4f76) )
	ROM_LOAD( "mn_xes__.e_0", 0x0000, 0x010000, CRC(828bd8c1) SHA1(d6411579ec5f5b1a7f83d27d9e222d17e355441f) )
	ROM_LOAD( "mn_xes_s.e_0", 0x0000, 0x010000, CRC(146af69b) SHA1(ce261fe2e93cb91a46bf67fcda504b168c84fa27) )
	ROM_LOAD( "mn_xesb_.e_0", 0x0000, 0x010000, CRC(740f4f52) SHA1(cd811a90abd07d5cea466a631ffdc1329a5d3cff) )
	ROM_LOAD( "mn_xesd_.e_0", 0x0000, 0x010000, CRC(c92e3f60) SHA1(2a07c11bb9e212b1cb049f087b618ec7f7e9e8bc) )
	ROM_LOAD( "mn_xesk_.e_0", 0x0000, 0x010000, CRC(3faaa8f3) SHA1(6ddaf4ab3128ed16ad9abd9e77556710f88ec6f0) )
	ROM_LOAD( "mni20s__.e_0", 0x0000, 0x010000, CRC(d34f809c) SHA1(6c51c2b52e06d2b115737690bf22dd0182f73fc2) )
	ROM_LOAD( "mnixes__.e_0", 0x0000, 0x010000, CRC(e3f3b757) SHA1(f7a285730b7d76de4253c234be652021dc2bf860) )
	ROM_LOAD( "mn_20a__.e_0", 0x0000, 0x010000, CRC(47d177cf) SHA1(7d3e59bc399b731880320899169732b20932c39d) )
	ROM_LOAD( "mn_20sbs.e_0", 0x0000, 0x010000, CRC(d25256c3) SHA1(b359307021604e405b4bc50e38aa51f7675f73a0) )
	ROM_LOAD( "mn_27s__.e60", 0x0000, 0x010000, CRC(983f17b1) SHA1(e57104ff8fc136db11fc81fc2160d1bc6961f362) )
	ROM_LOAD( "mn_27sd_.e60", 0x0000, 0x010000, CRC(d39af010) SHA1(71ceb3ab370518b0e367dc93db9101efa4710067) )
	ROM_LOAD( "mn_20s_s.e_0", 0x0000, 0x010000, CRC(24d6c150) SHA1(4671098dc24185837ace42acf224baabb2581906) )
	ROM_LOAD( "mn_20sb_.e_0", 0x0000, 0x010000, CRC(44b37899) SHA1(9de7ba3284e6f091d1ae29f7fc861e1da3879daf) )
	ROM_LOAD( "mn_20sd_.e_0", 0x0000, 0x010000, CRC(f99208ab) SHA1(1ac02645712bd9c2d575b42ea8641cec03347507) )
	ROM_LOAD( "mn_20sk_.e_0", 0x0000, 0x010000, CRC(0f169f38) SHA1(66b95512c3f6bfdf0f6824b5a97b12ac55c56c50) )
	ROM_LOAD( "mn_xesbs.e_0", 0x0000, 0x010000, CRC(e2ee6108) SHA1(838f724c49f1f737dae02d10dd6633994c6480fc) )
	ROM_LOAD( "mn_37s__.e60", 0x0000, 0x010000, CRC(172d7c9f) SHA1(303bd7019b91c782255befea7a5e530030a6461b) )
	ROM_LOAD( "mn_37sd_.e60", 0x0000, 0x010000, CRC(5c889b3e) SHA1(c55c395fc7aa8f6c0ab4f74acf41f04017da8330) )
	ROM_LOAD( "mn_x7sd_.e60", 0x0000, 0x010000, CRC(e326c7db) SHA1(d09409b4edca673675bd3b3653de0608b463ea7c) )

	// Monte Carlo Or Bust Release F (c)1996
	ROM_LOAD( "mn_x7s__.f60", 0x0000, 0x010000, CRC(866a18ee) SHA1(b21bc50ef84e5e836da133caffb14b2ef680bdcd) )
	ROM_LOAD( "mn_x7sd_.f60", 0x0000, 0x010000, CRC(cdcfff4f) SHA1(9e442cd46684efbdf186df48a1143a2b23f796e7) )
	ROM_LOAD( "mn_20s__.f_0", 0x0000, 0x010000, CRC(9cded79e) SHA1(0702e57e04bfe860ec63122fe29a1ff920884c49) )
	ROM_LOAD( "mn_27s__.f60", 0x0000, 0x010000, CRC(b6d62f25) SHA1(1d5f8c40ae12508bb146638706946da9107bacc1) )
	ROM_LOAD( "mn_27sd_.f60", 0x0000, 0x010000, CRC(fd73c884) SHA1(32a972cf552af2d9b8c46de3d59b1adee9b4286d) )
	ROM_LOAD( "mn_37s__.f60", 0x0000, 0x010000, CRC(39c4440b) SHA1(e54c480420313f3e5d3d67ffc0b5827458f20c9c) )
	ROM_LOAD( "mn_37sd_.f60", 0x0000, 0x010000, CRC(7261a3aa) SHA1(08d289b5f2493d058db0779ae7858c3ea9ef7c85) )

	// Monte Carlo on Option4 Release 9 (c)1996
	ROM_LOAD( "mni20s__.940", 0x0000, 0x010000, CRC(13ac85ab) SHA1(f1d65613787dc1312ad68bd49dcb5a9a8bc1093c) )
	ROM_LOAD( "mnixes__.940", 0x0000, 0x010000, CRC(2310b260) SHA1(309d5e34c811866cbef634032fddc53ca0b78966) )
	ROM_LOAD( "mn_xesb_.940", 0x0000, 0x010000, CRC(b4ec4a65) SHA1(e360704a434dde21c6fb794f2b75ecd4b8cea2b5) )
	ROM_LOAD( "mn_xea__.940", 0x0000, 0x010000, CRC(b78e4533) SHA1(5084f5434c0ee727e6b15422b5fc5d33a7e732d8) )
	ROM_LOAD( "mn_xeab_.940", 0x0000, 0x010000, CRC(410ad2a0) SHA1(0f781bc89837e7def08f71589834ccbdadca6409) )
	ROM_LOAD( "mn_xes__.940", 0x0000, 0x010000, CRC(4268ddf6) SHA1(e81cc5f429ae8c3e868ac6301b60b7a0acd2d190) )
	ROM_LOAD( "mn_x7s__.9a0", 0x0000, 0x010000, CRC(6860254d) SHA1(a0247ec07f2ec13fadc543986d27b5763d799f9b) )
	ROM_LOAD( "mn_20ab_.940", 0x0000, 0x010000, CRC(71b6e56b) SHA1(b87b488398b5554b63324b3c8d47f5ed2210026d) )
	ROM_LOAD( "mn_20a__.940", 0x0000, 0x010000, CRC(873272f8) SHA1(7097987c175fc441a6ae6f1a7a0dec3ef5bf53a5) )
	ROM_LOAD( "mn_20sb_.940", 0x0000, 0x010000, CRC(84507dae) SHA1(1a2fb9514cbcb44f324574fc913a7f1f5d321e53) )
	ROM_LOAD( "mn_27s__.9a0", 0x0000, 0x010000, CRC(58dc1286) SHA1(d90fe58b612d438c082fb7dac8db82bec3faabf9) )
	ROM_LOAD( "mn_27sd_.9a0", 0x0000, 0x010000, CRC(1379f527) SHA1(e9b84b8844be7c26f2e7db731c48b0c6d2bfd689) )
	ROM_LOAD( "mn_37s__.9a0", 0x0000, 0x010000, CRC(d7ce79a8) SHA1(6daa848f79ad65bd2567afbacee2dcbb60317f29) )
	ROM_LOAD( "mn_37sd_.9a0", 0x0000, 0x010000, CRC(9c6b9e09) SHA1(1b0d35c2ba906b1e478e58d66fc5432cdf05f36b) )
	ROM_LOAD( "mn_x7sd_.9a0", 0x0000, 0x010000, CRC(23c5c2ec) SHA1(42568dd981a65c34ae7e25c201ccc90c109bf32e) )
	ROM_LOAD( "mn_20s__.940", 0x0000, 0x010000, CRC(72d4ea3d) SHA1(bf2d3279550351069ccc04064dc3b6966b11ee12) )



	// these look like some other MPU4 game called Monte Carlo...
	ROM_LOAD( "mx_05a__.2_1", 0x0000, 0x010000, CRC(a1a03e03) SHA1(bf49b516e6824a47cd9bf1408bf676f9f1e43d62) )
	ROM_LOAD( "mx_10a__.2_1", 0x0000, 0x010000, CRC(bbf21e9f) SHA1(901b14b96cdb0945f491c39707ab9d2b9a2d25dd) )
	ROM_LOAD( "mx_20__c.1_1", 0x0000, 0x010000, CRC(a753798d) SHA1(ae1f5f14a37dead66f6b2d075a5bfc019d59f806) )
	ROM_LOAD( "mx_20a_c.1_1", 0x0000, 0x010000, CRC(9ec6f5fb) SHA1(ee181a64557053349cc8bff86bba937b191cab01) )
	ROM_LOAD( "mx_20dkc.1_1", 0x0000, 0x010000, CRC(d580f742) SHA1(3c1d6aba4068d60ab53eceecf65bc920f8b5604e) )
	ROM_LOAD( "mx_25__c.3_1", 0x0000, 0x010000, CRC(11ae121d) SHA1(11e61db1c645410ac18ef429cde167a7774be5f5) )
	ROM_LOAD( "mx_25_bc.3_1", 0x0000, 0x010000, CRC(4228139c) SHA1(a448ddc034923cba58ee298fd2a4c2cdd4f84f04) )
	ROM_LOAD( "mx_25a_c.3_1", 0x0000, 0x010000, CRC(283b9e6b) SHA1(937da8bda49a7a0fa1f728770f96d10a65bfe7bc) )
	ROM_LOAD( "mxi05___.2_1", 0x0000, 0x010000, CRC(de425b55) SHA1(2aa63bbd32c766e7e2d888345115c3185dc03bff) )
	ROM_LOAD( "mxi10___.2_1", 0x0000, 0x010000, CRC(19077425) SHA1(e31da38a903345c65b083cac192555f1f4ba2e5a) )


	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcobo4p3", 0x100001, 0x080000, CRC(ebe851df) SHA1(61d37a7f91480592da6f5b6ee7ef4b6097ee5c65) )
	ROM_LOAD16_BYTE( "mcobo4p4", 0x100000, 0x080000, CRC(49b0cfd7) SHA1(51fe74371bdac3c507a04aa9faeb522640d1cdf7) )



	ROM_REGION( 0x800000, "altvideo", 0 )
	// these are bad dumps I think, they're 0x40000-0x60000 of the main roms?
	ROM_LOAD( "montecarloorbustvideoboardp1.bin", 0x0000, 0x020000, CRC(5fd2c5ee) SHA1(60194df160070754e1cce6558dedc87b7fc05044) )
	ROM_LOAD( "montecarloorbustvideoboardp2.bin", 0x0000, 0x020000, CRC(7f6747cb) SHA1(0a85c0199583c5c48012f627ec3e4c3d12e39859) )
	ROM_LOAD( "montecarloorbustvideoboardp3.bin", 0x0000, 0x020000, CRC(f3d4a37d) SHA1(e81df776bb220832b45a5f6d12e8831f17dbd10b) )
	ROM_LOAD( "montecarloorbustvideoboardp4.bin", 0x0000, 0x020000, CRC(94be9981) SHA1(fe5803102e5e301ad6659ed83d319f55aa62c33e) )

	// this seems to be an alt (incomplete? set)
	ROM_LOAD( "mn______.f_1", 0x0000, 0x080000, CRC(1a81b3fb) SHA1(bbf0fe7e48404962a2f2120734efe71dc1eed64c) ) // unmatched rom? (significant changes)
	ROM_LOAD( "mcop3vd", 0x0000, 0x080000, CRC(721e9ad1) SHA1(fb926debd57301c9c0c3ecb9bb1ac36b0b60ee40) ) // alt p3 (significant changes)
	ROM_LOAD( "mcop4vd", 0x0000, 0x080000, CRC(6eba1107) SHA1(c696b620781782c3b4045fe3550ab8e7e905661d) ) // alt p4 (significant changes

	ROM_REGION( 0x10000, "unk", 0 ) // something else?
	ROM_LOAD( "montvnd", 0x0000, 0x010000, CRC(9858bb1d) SHA1(a2d3de2cec7420cc6f7da2239bdc79d7c4b7394e) ) // this looks like a different MPU4 game? - check

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END


ROM_START( v4ovrmn3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "o3_20s__.4_0", 0x0000, 0x010000, CRC(fab0af50) SHA1(de57130763965bcea42dd17c1094a2d1c363d328) )

	ROM_REGION( 0x10000, "altmain", 0 ) // alt MPU4 interface ROMS
	ROM_LOAD( "o3_20sb_.4_0", 0x0000, 0x010000, CRC(0c3438c3) SHA1(dd0fc2db66d7bfb97f7f456c92654fc60c494e49) )
	ROM_LOAD( "o3_20sd_.4_0", 0x0000, 0x010000, CRC(b11548f1) SHA1(59585394c57a0441817d2bea424abe0bcf09f7ff) )
	ROM_LOAD( "o3_20sk_.4_0", 0x0000, 0x010000, CRC(4791df62) SHA1(c3d8ecf5f35a95aa5b2fd0a4085aa80b0b6d4447) )
	ROM_LOAD( "o3i20s__.4_0", 0x0000, 0x010000, CRC(9bc8c0c6) SHA1(19ea9614ca2a9c28e1819d1451bc3117360f143f) )

	ROM_REGION( 0x800000, "video", 0 )
	// these 2 match Bubbly Bonk ??
	ROM_LOAD16_BYTE( "o3______.4_1", 0x000000, 0x010000,  CRC(1e85dd5d) SHA1(0f3c35d9f75d3495e2d0cc1fcf0e96dcbbeeacc8)) // == bu______.4_1
	ROM_LOAD16_BYTE( "o3______.4_2", 0x000001, 0x010000,  CRC(989db70a) SHA1(e44f8900392db51cc3f0a5bf0391bde0d71b9878)) // == bu______.4_2
	ROM_LOAD16_BYTE( "o3______.4_3", 0x020000, 0x010000, CRC(e6adda98) SHA1(176df97e3f0b22531a4c7b30e951bab0bc6403ad) )
	ROM_LOAD16_BYTE( "o3______.4_4", 0x020001, 0x010000, CRC(b044faef) SHA1(e78f44fa32d9b8428e8ba9f19d95c0e32b8ec1e8) )
	ROM_LOAD16_BYTE( "o3______.4_5", 0x040000, 0x010000, CRC(5bcc553d) SHA1(dd073cccce6fca693b772f6739132382b237bc67) )
	ROM_LOAD16_BYTE( "o3______.4_6", 0x040001, 0x010000, CRC(ea616d69) SHA1(e606f5e999a93f8505f529c95fe209c6452934c3) )
	ROM_LOAD16_BYTE( "o3______.4_7", 0x060000, 0x010000, CRC(e4dc300d) SHA1(8631bd78abfebcfa75f60a891b9709d84c8124e7) )
	ROM_LOAD16_BYTE( "o3______.4_8", 0x060001, 0x010000, CRC(c51ffb5f) SHA1(78b0d8fe04d419d52b68549a61bdc51b3bbda50f) )
	ROM_LOAD16_BYTE( "o3______.4_9", 0x080000, 0x010000, CRC(6201a444) SHA1(a4a419fd94c571a85259f0f0092e1c99ef6b5797) )
	ROM_LOAD16_BYTE( "o3______.4_a", 0x080001, 0x010000, CRC(5b526937) SHA1(dd9de97ee48a157a26e8e70211819aed0a87921c) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END



ROM_START( v4pztet )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tc______.2_0", 0x0000, 0x010000, CRC(9a94ccaf) SHA1(42988a22c26f88fa07c1ab68a85f15bc3af0a71c) )

	ROM_REGION( 0x10000, "altmain", 0 ) // alt MPU4 interface ROMS
	ROM_LOAD( "tc____d_.2_0", 0x0000, 0x010000, CRC(d1312b0e) SHA1(d1ba8f49a0b30771f5ffbaaf3f8b6142965a8330) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "tc______.2_1", 0x000000, 0x010000, CRC(048f30b5) SHA1(e6cf196cabe5c9b27a40942f547d50135d9633d0) )
	ROM_LOAD16_BYTE( "tc______.2_2", 0x000001, 0x010000, CRC(546719a6) SHA1(17a0fd552f4afa9d4eaeec903f3952f61af345d2) )
	ROM_LOAD16_BYTE( "tc______.2_3", 0x020000, 0x010000, CRC(6afa8148) SHA1(f4bfe41c9b9bb363c4fc1de616d641bea22248ba) )
	ROM_LOAD16_BYTE( "tc______.2_4", 0x020001, 0x010000, CRC(f7f99e42) SHA1(ca3b26fd911b8fc277e14dcdba428cb15288c995) )
	ROM_LOAD16_BYTE( "tc______.2_5", 0x040000, 0x010000, CRC(a2a50948) SHA1(57bf6c0738363da93ec6f379a23e706d1a6fc237) )
	ROM_LOAD16_BYTE( "tc______.2_6", 0x040001, 0x010000, CRC(de2146e4) SHA1(4e65c5d59d561d052834c9a0d139c286839fcf86) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END


ROM_START( v4pzteta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tp______.2_0", 0x0000, 0x010000, CRC(17757b59) SHA1(013690047e2769c420a9422c662990a71e1bd09d) )

	ROM_REGION( 0x10000, "altmain", 0 ) // alt MPU4 interface ROMS
	ROM_LOAD( "tp____d_.2_0", 0x0000, 0x010000, CRC(5cd09cf8) SHA1(3c288169f9bd49affaaa4e1f5f0fdddf52f381a8) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "tp______.2_1", 0x000000, 0x010000, CRC(91f3a03d) SHA1(c1eb60b32052cfe188cf465233a2fa0d0ca0a649) )
	ROM_LOAD16_BYTE( "tp______.2_2", 0x000001, 0x010000, CRC(f5fc5d3d) SHA1(d22f92769c0ad2c30887c60680ff1f984013bd02) )
	ROM_LOAD16_BYTE( "tp______.2_3", 0x020000, 0x010000, CRC(ef015d5e) SHA1(d7477743cbc0cc7c2e8cdf33a2c82c0425dd7d61) )
	ROM_LOAD16_BYTE( "tp______.2_4", 0x020001, 0x010000, CRC(401b5a50) SHA1(1b9bea05d4ba69d874f0067908bcfc19a5d8c6af) )
	ROM_LOAD16_BYTE( "tp______.2_5", 0x040000, 0x010000, CRC(e35cb24b) SHA1(a1c32c195b1d61a99b784c646ad78fa59b8270c4) )
	ROM_LOAD16_BYTE( "tp______.2_6", 0x040001, 0x010000, CRC(fecd48d0) SHA1(67ee3bee7aade5ec5fce1fcfe7ef3982264f1762) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END




ROM_START( v4rhmaz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rm_10___.4_0", 0x0000, 0x010000, CRC(5e9b282e) SHA1(d40744ae4cb36e49398f2f2e91274f9a1ecbe018) )

	ROM_REGION( 0x10000, "altmain", 0 ) // alt MPU4 interface ROMS
	ROM_LOAD( "rm_10_b_.4_0", 0x0000, 0x010000, CRC(a81fbfbd) SHA1(24fda5a2eaa7291ad4869d2f3061798360c5e862) )
	ROM_LOAD( "rm_10_d_.4_0", 0x0000, 0x010000, CRC(153ecf8f) SHA1(a1d67c2298bbd008be69b7ae7b3b92669972e37f) )
	ROM_LOAD( "rm_20___.4_0", 0x0000, 0x010000, CRC(8c5811a6) SHA1(2b51776e7dc78c322173708c6426b9eafde84870) )
	ROM_LOAD( "rm_20_b_.4_0", 0x0000, 0x010000, CRC(7adc8635) SHA1(8af3c8f332ca877f82940bc2a4e7abc4d37d6f48) )
	ROM_LOAD( "rm_20_bl.4_0", 0x0000, 0x010000, CRC(244c3e2c) SHA1(c764fe5a5ec74a4c4bb8bfea1bab2e8ae95cc4e7) )
	ROM_LOAD( "rm_20_d_.4_0", 0x0000, 0x010000, CRC(c7fdf607) SHA1(eab05365451d66e23163cbed433921bcbd415022) )
	ROM_LOAD( "rm_xc___.4_0", 0x0000, 0x010000, CRC(bce4266d) SHA1(018419adc7ee4af6c183868d1b7c3ef0cab8a837) )
	ROM_LOAD( "rm_xc_b_.4_0", 0x0000, 0x010000, CRC(4a60b1fe) SHA1(8effdbcf89aa942c434ebbb5e7127c357608d83f) )
	ROM_LOAD( "rm_xc_d_.4_0", 0x0000, 0x010000, CRC(f741c1cc) SHA1(576e250319ea2269982cfd6b5ea68ae4a26f81f2) )
	ROM_LOAD( "rm_xc_k_.4_0", 0x0000, 0x010000, CRC(01c5565f) SHA1(293b7591cd73d12552db751629a60aa315433dbf) )
	ROM_LOAD( "rmixc___.4_0", 0x0000, 0x010000, CRC(dd9c49fb) SHA1(649eb1d5fd05698e60094fe7225413da4971d65b) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rm______.4_1", 0x000000, 0x010000, CRC(51af0692) SHA1(74e52d63adeaaa1f0949dad60f5d4ca1635d2261) )
	ROM_LOAD16_BYTE( "rm______.4_2", 0x000001, 0x010000, CRC(2c0971ee) SHA1(7ee22c7bf0e0dcc548537a6418d56c7088a699a0) )
	ROM_LOAD16_BYTE( "rm______.4_3", 0x020000, 0x010000, CRC(b7981930) SHA1(d4356dcbbfda7486a684d8c8d375648dd0f8e200) )
	ROM_LOAD16_BYTE( "rm______.4_4", 0x020001, 0x010000, CRC(1bc71ac8) SHA1(ac0f56d974a4aba143cb3b8cad8a65dc25b3e145) )
	ROM_LOAD16_BYTE( "rm______.4_5", 0x040000, 0x010000, CRC(71460efc) SHA1(3ae79df9d3ec83abdde059827e06e81316026380) )
	ROM_LOAD16_BYTE( "rm______.4_6", 0x040001, 0x010000, CRC(471678de) SHA1(919394768314bc8707f93875528dca33bcf7e09c) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END




ROM_START( v4sunbst )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "st_10___.4_0", 0x0000, 0x010000, CRC(153ce606) SHA1(2e3fada2d150ac46ca38d67845a97d649f7489f9) )

	ROM_REGION( 0x10000, "altmain", 0 ) // alt MPU4 interface ROMS
	ROM_LOAD( "st_10_b_.4_0", 0x0000, 0x010000, CRC(e3b87195) SHA1(f6bc0edd6c1e5396fb10adaee1d1d870262ff084) )
	ROM_LOAD( "st_10_d_.4_0", 0x0000, 0x010000, CRC(5e9901a7) SHA1(8e3b620d31e03589330d92bce7fac038a2c2484b) )
	ROM_LOAD( "st_10_k_.4_0", 0x0000, 0x010000, CRC(a81d9634) SHA1(70aded04e2d014632d832d299b0a88124334d2b6) )
	ROM_LOAD( "st_20___.4_0", 0x0000, 0x010000, CRC(c7ffdf8e) SHA1(2dd24eeff88ae62b83302404c1304eeb81b83284) )
	ROM_LOAD( "st_20_b_.4_0", 0x0000, 0x010000, CRC(317b481d) SHA1(f06e3510c0a16efa27e94f455c51e8f8cdbc23ae) )
	ROM_LOAD( "st_20_bl.4_0", 0x0000, 0x010000, CRC(6febf004) SHA1(d70a64343d6e509fa10cc2321d14dffda4fb2a4a) )
	ROM_LOAD( "st_20_d_.4_0", 0x0000, 0x010000, CRC(8c5a382f) SHA1(76e5927216c6bd12d3e3c7bc614366c12a733865) )
	ROM_LOAD( "st_20_dl.4_0", 0x0000, 0x010000, CRC(d2ca8036) SHA1(cb2db62defeab671e8407e8139feb3b1f9d91c1f) )
	ROM_LOAD( "st_20_k_.4_0", 0x0000, 0x010000, CRC(7adeafbc) SHA1(216a3344c5e9e60e4429cfb7813ff250db7eb378) )
	ROM_LOAD( "st_20bg_.4_0", 0x0000, 0x010000, CRC(4eaae703) SHA1(bdc2a0d752496708b911579867844870b799ee5b) )
	ROM_LOAD( "st_20bt_.4_0", 0x0000, 0x010000, CRC(f8e1ea00) SHA1(ce3a0bb34a966d15797e7e398d88342b488e0cb1) )
	ROM_LOAD( "st_xc___.4_0", 0x0000, 0x010000, CRC(f743e845) SHA1(af60698c8349a982b9cb5bef0a9d670cf3658a0c) )
	ROM_LOAD( "st_xc__l.4_0", 0x0000, 0x010000, CRC(a9d3505c) SHA1(07dd1b9a3c663a6315aa3e02bcaa4bc3f38cc4e0) )
	ROM_LOAD( "st_xc_b_.4_0", 0x0000, 0x010000, CRC(01c77fd6) SHA1(7562969f482e20ed4a7d66d204309c12ae1843f6) )
	ROM_LOAD( "st_xc_d_.4_0", 0x0000, 0x010000, CRC(bce60fe4) SHA1(dd2a7e35afe5ed9b07c7395e746090205f014b54) )
	ROM_LOAD( "st_xc_dl.4_0", 0x0000, 0x010000, CRC(e276b7fd) SHA1(b6c295a4f1b7bae5bab0ce75032f514945a0c637) )
	ROM_LOAD( "st_xc_k_.4_0", 0x0000, 0x010000, CRC(4a629877) SHA1(8c6e5dba3dc048a727562c53203f26d3ac8fc7ee) )
	ROM_LOAD( "sti10___.4_0", 0x0000, 0x010000, CRC(74448990) SHA1(4a6163f20f0f79c4328c6e9577b7063d91f5ae33) )
	ROM_LOAD( "sti20___.4_0", 0x0000, 0x010000, CRC(a687b018) SHA1(a1f2bd6568a01614a08b9f64abda458d7b7d3fd4) )
	ROM_LOAD( "stixc___.4_0", 0x0000, 0x010000, CRC(963b87d3) SHA1(8999b41cf214992ed8aab60c83cfdb06b7a2b312) )
	ROM_LOAD( "stixc_d_.4_0", 0x0000, 0x010000, CRC(dd9e6072) SHA1(a28e3e20863f737e7ba8db434c4563d9bfb15fca) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END


ROM_START( v4timebn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ba_20s__.1_0", 0x0000, 0x010000, CRC(55a7b969) SHA1(2809f207fcfe5357b50df1124f8bf55c6e629308) )

	ROM_REGION( 0x10000, "altmain", 0 ) // alt MPU4 interface ROMS
	ROM_LOAD( "ba_20a__.1_0", 0x0000, 0x010000, CRC(a04121ac) SHA1(7ba2f229de8bc5883e6e961d873e5fa524680201) )
	ROM_LOAD( "ba_20bg_.1_0", 0x0000, 0x010000, CRC(dcf281e4) SHA1(0145b8d558ed95b96590cdf9c2f54b6b5f26971d) )
	ROM_LOAD( "ba_20bt_.1_0", 0x0000, 0x010000, CRC(6ab98ce7) SHA1(6f56c8ba4681255c505bb52e3a76751d1e8acb0e) )
	ROM_LOAD( "ba_20sb_.1_0", 0x0000, 0x010000, CRC(a3232efa) SHA1(12ca99a3f368e54a82719382820e4778bdaccb6e) )
	ROM_LOAD( "ba_20sd_.1_0", 0x0000, 0x010000, CRC(1e025ec8) SHA1(87cb62ae81aa2735edba08d34c6aa31c78e545e8) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ba______.1_1", 0x000000, 0x080000, CRC(df853e0e) SHA1(07b0b9aa8a6dc3a70991236f8c1f88b4a6c6755f) )
	ROM_LOAD16_BYTE( "ba______.1_2", 0x000001, 0x080000, CRC(9a2ab155) SHA1(582b33f9ddbf7fb2da71ec6ad5fbbb20a03eda19) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "ba______.1_a", 0x000000, 0x080000, CRC(976d761b) SHA1(32cceff2cd9bc6ad48caac0a2d95d815a5f24aa9) )
	ROM_LOAD( "ba______.1_b", 0x080000, 0x080000, CRC(25a6c7ef) SHA1(cb614c7b2142e47862127d9fdfc7db50978f7f00) )
ROM_END


ROM_START( v4sixx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6x_20s__.3_0", 0x0000, 0x010000, CRC(452ba9aa) SHA1(4034bfdba6cb46a63a59ba56e19ae912f5633412) )

	ROM_REGION( 0x10000, "altmain", 0 ) // alt MPU4 interface ROMS
	ROM_LOAD( "6x_20a__.3_0", 0x0000, 0x010000, CRC(b0cd316f) SHA1(56e2499f041cc534571f14492b5bd8d64a7f0c7b) )
	ROM_LOAD( "6x_20ad_.3_0", 0x0000, 0x010000, CRC(fb68d6ce) SHA1(186230bd84d41cbe45f808a63f06d37edff818b1) )
	ROM_LOAD( "6x_20ak_.3_0", 0x0000, 0x010000, CRC(0dec415d) SHA1(2f9231ee5a3ab978bbeade3495038c14c13cba09) )
	ROM_LOAD( "6x_20bg_.3_0", 0x0000, 0x010000, CRC(cc7e9127) SHA1(ceeab2d61eec87977528030ea8d4b870d08adae2) )
	ROM_LOAD( "6x_20bt_.3_0", 0x0000, 0x010000, CRC(7a359c24) SHA1(2a3496ef1da06aa5a65b79e598ac4652d4520551) )
	ROM_LOAD( "6x_20sb_.3_0", 0x0000, 0x010000, CRC(b3af3e39) SHA1(2690b8e84e812898390979268ab496e5fb12f6ca) )
	ROM_LOAD( "6x_xca__.3_0", 0x0000, 0x010000, CRC(807106a4) SHA1(29895cb8450dc916bffdd3958f144b0a35644c39) )
	ROM_LOAD( "6x_xcab_.3_0", 0x0000, 0x010000, CRC(76f59137) SHA1(183dac7b11c9a7e8d5fdf44e1c6fa755121af5d3) )
	ROM_LOAD( "6x_xcad_.3_0", 0x0000, 0x010000, CRC(cbd4e105) SHA1(443ea2c4ff8afe5c43ee462bfca91ed10ad0d431) )
	ROM_LOAD( "6x_xcak_.3_0", 0x0000, 0x010000, CRC(3d507696) SHA1(9148e33eeef25bb1f4f40ef65257133f744ad503) )
	ROM_LOAD( "6xi20s__.3_0", 0x0000, 0x010000, CRC(2453c63c) SHA1(c83df6afd45d25de0f315b5a75ecd486bca13018) )
	ROM_LOAD( "6xixcs__.3_0", 0x0000, 0x010000, CRC(14eff1f7) SHA1(a9cb41d2c812e3fa51b859912130a4a1e1c0f43d) )
	ROM_LOAD( "6xixcsd_.3_0", 0x0000, 0x010000, CRC(5f4a1656) SHA1(b541ce749bcfca9ef89e320e46cdf060acbca8c3) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "6x______.3_1", 0x000000, 0x010000, CRC(870dc043) SHA1(a0bcc8dbfc3cfdd43ed1d6ed5308c43a6f4f8117) )
	ROM_LOAD16_BYTE( "6x______.3_2", 0x000001, 0x010000, CRC(f9ff2007) SHA1(87833cd36859332195c845fffb8a310eb561ae2b) )
	ROM_LOAD16_BYTE( "6x______.3_3", 0x020000, 0x010000, CRC(e8a4531d) SHA1(c816b3b270f1aeaf3ee90aa65dfeea59e8862d1a) )
	ROM_LOAD16_BYTE( "6x______.3_4", 0x020001, 0x010000, CRC(4129b8af) SHA1(30ad007f543e570021292f3eef728b0697c31bb5) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END



ROM_START( v4megbuk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mc_xc___.2_0", 0x0000, 0x010000, CRC(372ed757) SHA1(ce9d13bc546b7e64ee377627577b3ae0848e8659) )

	ROM_REGION( 0x10000, "altmain", 0 ) // alt MPU4 interface ROMS
	ROM_LOAD( "mc_xc_b_.2_0", 0x0000, 0x010000, CRC(c1aa40c4) SHA1(c1ce0bf5929ccba6d5e4ad7508e41d1df8fe10f2) )
	ROM_LOAD( "mc_xc_d_.2_0", 0x0000, 0x010000, CRC(7c8b30f6) SHA1(d851fc2dcb5cf72d0b63ce6fe47b0350faa94356) )
	ROM_LOAD( "mc_xc_k_.2_0", 0x0000, 0x010000, CRC(8a0fa765) SHA1(d687a324223ac1d32a312c8666f63772dc5d9e2b) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END



ROM_START( v4rencas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rc_20a__.3_0", 0x0000, 0x010000, CRC(9727f862) SHA1(7815bef16fa7304108d553bbf897376ce2a45ad6) )

	ROM_REGION( 0x10000, "altmain", 0 ) // alt MPU4 interface ROMS
	ROM_LOAD( "rc_20ab_.3_0", 0x0000, 0x010000, CRC(61a36ff1) SHA1(33a8bd624e995acc43757689988aee1ac79939f1) )
	ROM_LOAD( "rc_20ad_.3_0", 0x0000, 0x010000, CRC(dc821fc3) SHA1(2b7ae1b971d137e4f9dc36bf482d3f69f6737a94) )
	ROM_LOAD( "rc_20ak_.3_0", 0x0000, 0x010000, CRC(2a068850) SHA1(87d439e3e03fb5a9ba225dea37d0aaa8e2f475f9) )
	ROM_LOAD( "rc_20s__.3_0", 0x0000, 0x010000, CRC(62c160a7) SHA1(ea0979ebe0a12f58b15825e1a4074385453e839c) )
	ROM_LOAD( "rc_20sb_.3_0", 0x0000, 0x010000, CRC(9445f734) SHA1(74cff30e57a7cc856e72c1bfd8ab2d8d259a2b72) )
	ROM_LOAD( "rc_20sd_.3_0", 0x0000, 0x010000, CRC(29648706) SHA1(649aed977028a57664f181bc159663c93fe86e67) )
	ROM_LOAD( "rc_20sk_.3_0", 0x0000, 0x010000, CRC(dfe01095) SHA1(bf7c2b93bce4d5caa3c56ff948439acc99efd75b) )
	ROM_LOAD( "rci20___.3_0", 0x0000, 0x010000, CRC(03b90f31) SHA1(89c73b74751a648686480a17071b231fdfd9002d) )
	ROM_LOAD( "rci20_d_.3_0", 0x0000, 0x010000, CRC(481ce890) SHA1(2fe493728acc2aa0398fcc6559bce572ce921274) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END






/* Complete sets */
/* Standard sets are the most common setups, while Datapak releases use a BACTA datalogger (not emulated) to record more information about the game operation, for security etc.
AMLD versions do not pay out, and instead just feature highscore tables. These were mainly intended for locations unwilling to pay for gaming licenses.
The AMLD Crystal Maze versions appear to be a mixture of the original game modules and Team Challenge's scoring system. This would suggest they were all made ~1994. */

GAME(  199?,v4bios,     0,          mod2,   mpu4, driver_device,        0,          ROT0, "Barcrest","MPU4 Video Firmware",MACHINE_IS_BIOS_ROOT )

#define GAME_FLAGS MACHINE_NOT_WORKING

GAMEL( 1993,v4cmaze,    v4bios,     crmaze,     crmaze, mpu4vid_state,      crmaze,     ROT0, "Barcrest","The Crystal Maze (v1.3) (MPU4 Video)",GAME_FLAGS,layout_crmaze2p )//SWP 0.9
GAMEL( 1993,v4cmazedat, v4cmaze,    crmaze,     crmaze, mpu4vid_state,      crmaze,     ROT0, "Barcrest","The Crystal Maze (v1.3, Datapak) (MPU4 Video)",GAME_FLAGS,layout_crmaze2p )//SWP 0.9D
GAMEL( 1993,v4cmazea,   v4cmaze,    crmaze,     crmaze, mpu4vid_state,      crmazea,    ROT0, "Barcrest","The Crystal Maze (v0.1, AMLD) (MPU4 Video)",GAME_FLAGS,layout_crmaze2p )//SWP 0.9 (actually newer than the 1.1 set then??)
GAMEL( 1993,v4cmazeb,   v4cmaze,    crmaze,     crmaze, mpu4vid_state,      v4cmazeb,   ROT0, "Barcrest","The Crystal Maze (v1.2) (MPU4 Video)",GAME_FLAGS,layout_crmaze2p )//SWP 0.9
GAMEL( 1993,v4cmazec,   v4cmaze,    crmaze,     crmaze, mpu4vid_state,      v4cmazeb,   ROT0, "Barcrest","The Crystal Maze (v1.3 alt) (MPU4 Video)",GAME_FLAGS,layout_crmaze2p )//SWP 0.9
GAMEL( 1993,v4cmazed,   v4cmaze,    crmaze,     crmaze, mpu4vid_state,      v4cmazeb,   ROT0, "Barcrest","The Crystal Maze (v1.1) (MPU4 Video)",GAME_FLAGS,layout_crmaze2p )//SWP 0.6

GAMEL( 1993,v4cmaze2,   v4bios,     crmaze,     crmaze, mpu4vid_state,      crmaze2,    ROT0, "Barcrest","The New Crystal Maze Featuring Ocean Zone (v2.2) (MPU4 Video)",GAME_FLAGS,layout_crmaze4p )//SWP 1.0
GAMEL( 1993,v4cmaze2d,  v4cmaze2,   crmaze,     crmaze, mpu4vid_state,      crmaze2,    ROT0, "Barcrest","The New Crystal Maze Featuring Ocean Zone (v2.2, Datapak) (MPU4 Video)",GAME_FLAGS,layout_crmaze4p )//SWP 1.0D
GAMEL( 1993,v4cmaze2a,  v4cmaze2,   crmaze,     crmaze, mpu4vid_state,      crmaze2a,   ROT0, "Barcrest","The New Crystal Maze Featuring Ocean Zone (v0.1, AMLD) (MPU4 Video)",GAME_FLAGS,layout_crmaze4p )//SWP 1.0 /* unprotected? proto? */
GAMEL( 1993,v4cmaze2b,  v4cmaze2,   crmaze,     crmaze, mpu4vid_state,      crmaze2,    ROT0, "Barcrest","The New Crystal Maze Featuring Ocean Zone (v2.0) (MPU4 Video)",GAME_FLAGS,layout_crmaze4p )//SWP 1.0
GAMEL( 1993,v4cmaze2c,  v4cmaze2,   crmaze,     crmaze, mpu4vid_state,      crmaze2,    ROT0, "Barcrest","The New Crystal Maze Featuring Ocean Zone (v?.?) (MPU4 Video)",GAME_FLAGS,layout_crmaze4p )// bad rom?

GAMEL( 1994,v4cmaze3,   v4bios,     crmaze,     crmaze, mpu4vid_state,      crmaze3,    ROT0, "Barcrest","The Crystal Maze Team Challenge (v0.9) (MPU4 Video)",GAME_FLAGS,layout_crmaze4p )//SWP 0.7
GAMEL( 1994,v4cmaze3d,  v4cmaze3,   crmaze,     crmaze, mpu4vid_state,      crmaze3,    ROT0, "Barcrest","The Crystal Maze Team Challenge (v0.9, Datapak) (MPU4 Video)",GAME_FLAGS,layout_crmaze4p )//SWP 0.7D
GAMEL( 1994,v4cmaze3a,  v4cmaze3,   crmaze,     crmaze, mpu4vid_state,      crmaze3a,   ROT0, "Barcrest","The Crystal Maze Team Challenge (v1.2, AMLD) (MPU4 Video)",GAME_FLAGS,layout_crmaze4p )//SWP 0.7
GAMEL( 1994,v4cmaze3b,  v4cmaze3,   crmaze,     crmaze, mpu4vid_state,      v4cmazeb,   ROT0, "Barcrest","The Crystal Maze Team Challenge (v0.8) (MPU4 Video)",GAME_FLAGS,layout_crmaze4p )//SWP 0.7
GAMEL( 1994,v4cmaze3c,  v4cmaze3,   crmaze,     crmaze, mpu4vid_state,      v4cmazeb,   ROT0, "Barcrest","The Crystal Maze Team Challenge (v?.?) (MPU4 Video)",GAME_FLAGS,layout_crmaze4p )// missing one program rom

GAME(  199?,v4turnov,   v4bios,     mpu4_vid,   turnover, mpu4vid_state,    turnover,   ROT0, "Barcrest","Turnover (v2.3) (MPU4 Video)",GAME_FLAGS )

GAME(  1990,v4skltrk,   v4bios,     mpu4_vid,   skiltrek, mpu4vid_state,    skiltrek,   ROT0, "Barcrest","Skill Trek (v1.1) (MPU4 Video, set 1)",GAME_FLAGS ) // 10 pound max
GAME(  1990,v4skltrka,  v4skltrk,   mpu4_vid,   skiltrek, mpu4vid_state,    skiltrek,   ROT0, "Barcrest","Skill Trek (v1.1) (MPU4 Video, set 2)",GAME_FLAGS ) // 12 pound max
GAME(  1990,v4sklcsh,   v4bios,     mpu4_vid,   skiltrek, mpu4vid_state,    v4barqst,   ROT0, "Barcrest","Skill Cash (v1.1) (MPU4 Video)",GAME_FLAGS )

GAME(  1989,v4addlad,   v4bios,     mpu4_vid,   adders, mpu4vid_state,      adders,     ROT0, "Barcrest","Adders and Ladders (v2.1) (MPU4 Video)",GAME_FLAGS )
GAME(  1989,v4addlad20, v4addlad,   mpu4_vid,   adders, mpu4vid_state,      adders,     ROT0, "Barcrest","Adders and Ladders (v2.0) (MPU4 Video)",GAME_FLAGS )

GAME(  1989,v4time,     v4bios,     mpu4_vid,   skiltrek, mpu4vid_state,    timemchn,   ROT0, "Barcrest","Time Machine (v2.0) (MPU4 Video)",GAME_FLAGS )

//Year is a guess, based on the use of the 'Coin Man' logo
GAME(  1996?,v4mate,    v4bios,     mating,     mating, mpu4vid_state,      mating,     ROT0, "Barcrest","The Mating Game (v0.4) (MPU4 Video)",GAME_FLAGS )//SWP 0.2 /* Using crmaze controls for now, cabinet has trackball */
GAME(  1996?,v4mated,   v4mate,     mating,     mating, mpu4vid_state,      mating,     ROT0, "Barcrest","The Mating Game (v0.4, Datapak) (MPU4 Video)",GAME_FLAGS )//SWP 0.2D

/* Games below are missing question ROMs */

GAME(  199?,v4strike,   v4bios,     mpu4_vid,   mpu4, mpu4vid_state,        strikeit,   ROT0, "Barcrest","Strike it Lucky (v0.5) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4striked,  v4strike,   mpu4_vid,   mpu4, mpu4vid_state,        strikeit,   ROT0, "Barcrest","Strike it Lucky (v0.5, Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4strike2,  v4strike,   mpu4_vid,   mpu4, mpu4vid_state,        strikeit,   ROT0, "Barcrest","Strike it Lucky (v0.53) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4strike2d, v4strike,   mpu4_vid,   mpu4, mpu4vid_state,        strikeit,   ROT0, "Barcrest","Strike it Lucky (v0.53, Datapak) (MPU4 Video)",GAME_FLAGS )

GAME(  199?,v4eyedwn,   v4bios,     mpu4_vid,   mpu4, mpu4vid_state,        eyesdown,   ROT0, "Barcrest","Eyes Down (v1.3) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4eyedwnd,  v4eyedwn,   mpu4_vid,   mpu4, mpu4vid_state,        eyesdown,   ROT0, "Barcrest","Eyes Down (v1.3, Datapak) (MPU4 Video)",GAME_FLAGS )

GAME(  199?,v4quidgr,   v4bios,     mpu4_vid,   mpu4, mpu4vid_state,        quidgrid,   ROT0, "Barcrest","Ten Quid Grid (v1.2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4quidgrd,  v4quidgr,   mpu4_vid,   mpu4, mpu4vid_state,        quidgrid,   ROT0, "Barcrest","Ten Quid Grid (v1.2, Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4quidgr2,  v4quidgr,   mpu4_vid,   mpu4, mpu4vid_state,        quidgrid,   ROT0, "Barcrest","Ten Quid Grid (v2.4) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4quidgr2d, v4quidgr,   mpu4_vid,   mpu4, mpu4vid_state,        quidgrid,   ROT0, "Barcrest","Ten Quid Grid (v2.4, Datapak) (MPU4 Video)",GAME_FLAGS )

GAME(  199?,v4barqst,   v4bios,     mpu4_vid,   mpu4, mpu4vid_state,        v4barqst,   ROT0, "Barcrest","Barquest (v2.6d) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4barqs2,   v4bios,     mpu4_vid,   mpu4, mpu4vid_state,        v4barqst2,  ROT0, "Barcrest","Barquest 2 (v0.3) (MPU4 Video)",GAME_FLAGS )

GAME(  199?,v4wize,     v4bios,     mpu4_vid,   mpu4, mpu4vid_state,        v4wize,     ROT0, "Barcrest","Wize Move (v1.3d) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4wizea,    v4bios,     mpu4_vid,   mpu4, mpu4vid_state,        v4wize,     ROT0, "Barcrest","Wize Move (v1.2) (MPU4 Video)",GAME_FLAGS )

GAME(  1991,v4opt3,     v4bios,     mpu4_vid,   mpu4, mpu4vid_state,        v4opt3,     ROT0, "Barcrest","Option 3 (v1.0) (MPU4 Video)",GAME_FLAGS )
GAME(  1991,v4opt3d,    v4opt3,     mpu4_vid,   mpu4, mpu4vid_state,        v4opt3,     ROT0, "Barcrest","Option 3 (v1.0) (Datapak) (MPU4 Video)",GAME_FLAGS )




/* Games below are newer BwB games and use their own BIOS ROMs and hardware setups*/

GAME(  199?,v4vgpok,    0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Vegas Poker (prototype, release 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4psi,      0,          bwbvid,     mpu4, mpu4vid_state,        prizeinv,   ROT0, "BwB","Prize Space Invaders (v1.1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4psia,     0,          bwbvid,     mpu4, mpu4vid_state,        prizeinv,   ROT0, "BwB","Prize Space Invaders (v1.2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4psib,     0,          bwbvid,     mpu4, mpu4vid_state,        prizeinv,   ROT0, "BwB","Prize Space Invaders (v2.0?) (MPU4 Video)",GAME_FLAGS ) // bad dump
GAME(  199?,v4blox,     0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Blox (v2.0) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4bloxd,    v4blox,     bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Blox (v2.0, Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  1996,v4reno,     0,          bwbvid5,    mpu4, mpu4vid_state,        prizeinv,   ROT0, "BwB","Reno Reels (20p/10GBP Cash, release A) (MPU4 Video)",GAME_FLAGS )

GAME(  199?,v4bigfrt,   0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Big Fruits (v2.0?) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4bubbnk,   0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Bubbly Bonk (v4.0?) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4mazbel,   0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Mazooma Belle (v2.5) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4mazbla,   0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Mazooma Belle (v1.5) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4shpwnd,   0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Shop Window (v2.0) (MPU4 Video)",GAME_FLAGS )

GAME(  199?,v4redhtp,   0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Red Hot Poker (20p/10GBP Cash, release 3) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4tetrs,    0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","BwB Tetris v 2.2 (MPU4 Video)",GAME_FLAGS )

GAME(  199?,v4big40,    0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Big 40 Poker (Bwb) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4bulblx,   0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Bullion Blox (Bwb) (MPU4 Video)",GAME_FLAGS ) // is this the same game as v4blox?
GAME(  199?,v4cshinf,   0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Cash Inferno (Bwb) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4dbltak,   0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Double Take (Bwb) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4gldrsh,   0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Gold Rush (Bwb) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4mdice,    0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Miami Dice (Bwb) (MPU4 Video)",GAME_FLAGS ) // is this the same as the Nova game below?
GAME(  199?,v4monte,    0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Monte Carlo Or Bust (Bwb) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4ovrmn3,   0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Over Moon Pt3 (Bwb) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4pztet,    0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Prize Tetris (Bwb) (MPU4 Video, set 1)",GAME_FLAGS ) // is this the same as v4tetrs?
GAME(  199?,v4pzteta,   v4pztet,    bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Prize Tetris (Bwb) (MPU4 Video, set 2)",GAME_FLAGS )
GAME(  199?,v4rhmaz,    0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Red Hot Mazooma Belle (Bwb) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4sunbst,   0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Sunburst (Bwb) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4timebn,   0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Time Bandit (Bwb) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4sixx,     0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","6-X (Bwb) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4megbuk,   0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Megabucks Poker (Bwb) (MPU4 Video)",GAME_FLAGS ) // no video roms!
GAME(  199?,v4rencas,   0,          bwbvid,     mpu4, driver_device,        0,          ROT0, "BwB","Reno Casino (Bwb) (MPU4 Video)",GAME_FLAGS ) // no video roms!


/* Uncertain BIOS */
GAME(  199?,v4frfact,   v4bios,     crmaze,     crmaze, mpu4vid_state,      crmaze,     ROT0, "Bwb","Fruit Factory (Bwb) (MPU4 Video)", GAME_FLAGS )


/* Nova - is this the same video board? One of the games displays 'Resetting' but the others do nothing interesting and access strange addresses */
/* All contain BwB video in the BIOS rom tho */
GAME(  199?,v4cybcas,   0,          bwbvid5,    mpu4, mpu4vid_state,        cybcas,     ROT0, "Nova","Cyber Casino (Nova) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4miami,    0,          bwbvid5,    mpu4, driver_device,        0,          ROT0, "Nova","Miami Dice (Nova) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4missis,   0,          bwbvid5,    mpu4, driver_device,        0,          ROT0, "Nova","Mississippi Lady (Nova) (MPU4 Video)",GAME_FLAGS )
GAME(  199?,v4picdil,   0,          bwbvid5,    mpu4, driver_device,        0,          ROT0, "Nova","Piccadilly Nights (Nova) (MPU4 Video)",GAME_FLAGS )
