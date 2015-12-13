// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Nicola Salmoria
// thanks-to:Richard Bush
/***************************************************************************

   cchip.c

This file contains routines to interface with the Taito Controller Chip
(or "Command Chip") version 1. It's currently used by Superman.
[Further cchip emulation is in machine/rainbow.c, machine/volfied.c,
drivers/opwolf.c and drivers/taito_f2.c]

According to Richard Bush, the C-Chip is an encrypted Z80 which communicates
with the main board as a protection feature.


Superman (revised SJ 060601)
--------

In Superman, the C-chip's main purpose is to handle player inputs and
coins and pass commands along to the sound chip.

The 68k queries the c-chip, which passes back $100 bytes of 68k code which
are then executed in RAM. To get around this, we hack in our own code to
communicate with the sound board, since we are familiar with the interface
as it's used in Rastan and Super Space Invaders '91.

It is believed that the NOPs in the 68k code are there to supply the
necessary cycles to the cchip to switch banks.

This code requires that the player & coin inputs be in input ports 2-4.

***************************************************************************/

#include "emu.h"
#include "includes/taito_x.h"

/* This code for sound communication is a hack, it will not be
   identical to the code derived from the real c-chip */

static const UINT8 superman_code[40] =
{
	0x48, 0xe7, 0x80, 0x80,             /* MOVEM.L  D0/A0,-(A7)   ( Preserve Regs ) */
	0x20, 0x6d, 0x1c, 0x40,             /* MOVEA.L  ($1C40,A5),A0 ( Load sound pointer in A0 ) */
	0x30, 0x2f, 0x00, 0x0c,             /* MOVE.W   ($0C,A7),D0   ( Fetch sound number ) */
	0x10, 0x80,                         /* MOVE.B   D0,(A0)       ( Store it on sound pointer ) */
	0x52, 0x88,                         /* ADDQ.W   #1,A0         ( Increment sound pointer ) */
	0x20, 0x3c, 0x00, 0xf0, 0x1c, 0x40, /* MOVE.L   #$F01C40,D0   ( Load top of buffer in D0 ) */
	0xb1, 0xc0,                         /* CMPA.L   D0,A0         ( Are we there yet? ) */
	0x66, 0x04,                         /* BNE.S    *+$6          ( No, we arent, skip next line ) */
	0x41, 0xed, 0x1c, 0x20,             /* LEA      ($1C20,A5),A0 ( Point to the start of the buffer ) */
	0x2b, 0x48, 0x1c, 0x40,             /* MOVE.L   A0,($1C40,A5) ( Store new sound pointer ) */
	0x4c, 0xdf, 0x01, 0x01,             /* MOVEM.L  (A7)+, D0/A0  ( Restore Regs ) */
	0x4e, 0x75                          /* RTS                    ( Return ) */
};

/*************************************
 *
 * Writes to C-Chip - Important Bits
 *
 *************************************/

WRITE16_MEMBER( taitox_state::cchip1_ctrl_w )
{
	/* value 2 is written here */
}

WRITE16_MEMBER( taitox_state::cchip1_bank_w )
{
	m_current_bank = data & 7;
}

WRITE16_MEMBER( taitox_state::cchip1_ram_w )
{
	if (m_current_bank == 0 && offset == 0x03)
	{
		m_cc_port = data;

		coin_lockout_w(space.machine(), 1, data & 0x08);
		coin_lockout_w(space.machine(), 0, data & 0x04);
		coin_counter_w(space.machine(), 1, data & 0x02);
		coin_counter_w(space.machine(), 0, data & 0x01);
	}
	else
	{
		logerror("cchip1_w pc: %06x bank %02x offset %04x: %02x\n",space.device().safe_pc(),m_current_bank,offset,data);
	}
}


/*************************************
 *
 * Reads from C-Chip
 *
 *************************************/

READ16_MEMBER( taitox_state::cchip1_ctrl_r )
{
	/*
	    Bit 2 = Error signal
	    Bit 0 = Ready signal
	*/
	return 0x01; /* Return 0x05 for C-Chip error */
}

READ16_MEMBER( taitox_state::cchip1_ram_r )
{
	/* Check for input ports */
	if (m_current_bank == 0)
	{
		switch (offset)
		{
		case 0x00: return space.machine().root_device().ioport("IN0")->read();    /* Player 1 controls + START1 */
		case 0x01: return space.machine().root_device().ioport("IN1")->read();    /* Player 2 controls + START2 */
		case 0x02: return space.machine().root_device().ioport("IN2")->read();    /* COINn + SERVICE1 + TILT */
		case 0x03: return m_cc_port;
		}
	}

	/* Other non-standard offsets */

	if (m_current_bank == 1 && offset <= 0xff)
	{
		if (offset < 40)    /* our hack code is only 40 bytes long */
			return superman_code[offset];
		else    /* so pad with zeros */
			return 0;
	}

	if (m_current_bank == 2)
	{
		switch (offset)
		{
			case 0x000: return 0x47;
			case 0x001: return 0x57;
			case 0x002: return 0x4b;
		}
	}

	logerror("cchip1_r bank: %02x offset: %04x\n",m_current_bank,offset);
	return 0;
}
