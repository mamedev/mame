// license:BSD-3-Clause
// copyright-holders:David Haywood, Roberto Fresca
/*******************************************************************************************

  Playmark 'Super Derby' Hardware
  -------------------------------

  Driver by David Haywood.
  Additional work by Roberto Fresca.


  Supported games:

  Super Derby,                  1996, Playmark.
  Scacco Matto / Space Win,     1996, Playmark.
  Croupier (Playmark Roulette), 1997, Playmark.


********************************************************************************************

  NOTES:
  -----

  Payout / hopper controls not connected

  Roulette appears to have some kind of MCU device
  between the processor and the hopper


  Working notes: (Relating to SDERBY)

  Stephh's notes :

  - The game is playable, but :

      * it isn't possible to decrease the bet (but it might be an ingame "feature")
      * it isn't possible to insert a note
      * it isn't possible to exchange the winning points against tickets or cash

  - The settings can be modified in the "test mode", but there aren't mapped to
    input ports.


  EC notes :

  Thinks... Those three reads at the beginning - hopper, ticket and note acceptor tests?
  The system certainly performs those before printing the error.
  (There are three different error graphics in the ROM, depending on what is wrong)
  Also, hardware freezes if we try to turn the dispenser or acceptor on, because it's waiting
  for the response back from NOP?


********************************************************************************************

  TO DO :

  - figure out the reads from 0x308002.w and 0x30800e.w (see sderby_input_r read handler)
  (by default, demo sounds are OFF, so change this in the "test mode")


*******************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "includes/sderby.h"
#include "machine/nvram.h"

#include "sderby.lh"
#include "spacewin.lh"
#include "pmroulet.lh"

/***************************
*       R/W Handlers       *
***************************/

READ16_MEMBER(sderby_state::sderby_input_r)
{
	switch (offset)
	{
		case 0x00 >> 1:
			return ioport("IN0")->read();
		case 0x02 >> 1:
			return 0xffff;          // to avoid game to reset (needs more work)
	}

	logerror("sderby_input_r : offset = %x - PC = %06x\n",offset*2,space.device().safe_pc());

	return 0xffff;
}

READ16_MEMBER(sderby_state::sderbya_input_r)
{
	switch (offset)
	{
		case 0x00 >> 1:
			return ioport("IN0")->read();
		case 0x02 >> 1:
			return 0xffff;          // to avoid game to reset (needs more work)
	}

	logerror("sderby_input_r : offset = %x - PC = %06x\n",offset*2,space.device().safe_pc());

	return 0xffff;
}


READ16_MEMBER(sderby_state::roulette_input_r)
{
	switch (offset)
	{
		case 0x00 >> 1:
			return ioport("IN0")->read();
		case 0x02 >> 1:
			return ioport("IN1")->read();
		case 0x04 >> 1:
			return ioport("IN2")->read();
	}

	return 0xffff;
}


/***************************************************************

    Roulette MCU communication.
    ---------------------------

    Defeating the 'always win' protection...

    Offset: 0x70800e - 0x70800f.

    Writes to the MCU are always the same values.
    At beginning, the code writes 3 values: 0x53, 0x5a and 0x0d.
    Then, whith each placed bet the code normally writes 0x4e.
    After that, there are 2 reads expecting the MCU response.

    Most probably there is a shared RAM there for communication,
    but for now, I temporarily simulated the MCU response till
    we can get the MCU decapped.


****************************************************************/

READ16_MEMBER(sderby_state::rprot_r)
{
	logerror("rprot_r : offset = %02x\n",space.device().safe_pc());

/* This is the only mask I found that allow a normal play.
   Using other values, the game hangs waiting for response,
   or simply throw a deliberated losing number.

   If someone more skilled in 68K code can help to trace it,
   searching for an accurated response, I'll appreciate.
*/
	return machine().rand() & 0x1f;
}

WRITE16_MEMBER(sderby_state::rprot_w)
{
	logerror("rprot_w %02x\n", data);
}


/******************************
*       Outputs / Lamps       *
******************************/

WRITE16_MEMBER(sderby_state::sderby_out_w)
{
/*
  ---------------------------
  --- Super Derby Outputs ---
  ---------------------------

  0x0000 - Normal State (lamps off).
  0x0001 - Start lamp.
  0x0002 - Bet lamp.

  0x0100 - Ticket dispenser out.
  0x0800 - Unknown (always activated).
  0x1000 - Hopper out.
  0x2000 - Coin counter.
  0x4000 - Unknown.
  0x8000 - End of Race lamp.


    - Lbits -
    7654 3210
    =========
    ---- ---x  Start lamp.
    ---- --x-  Bet lamp.

    - Hbits -
    7654 3210
    =========
    ---- ---x  Ticket dispenser out.
    ---- x---  unknown (always activated).
    ---x ----  Hopper out.
    --x- ----  Coin counter.
    -x-- ----  unknown.
    x--- ----  End of Race lamp.

*/
	output_set_lamp_value(1, (data & 1));           /* Lamp 1 - START */
	output_set_lamp_value(2, (data >> 1) & 1);      /* Lamp 2 - BET */
	output_set_lamp_value(3, (data >> 15) & 1);     /* Lamp 3 - END OF RACE */

	coin_counter_w(machine(), 0, data & 0x2000);
}


WRITE16_MEMBER(sderby_state::scmatto_out_w)
{
/*
  ----------------------------------------
  --- Scacco Matto / Space Win Outputs ---
  ----------------------------------------

  0x0000 - Normal State (lamps off).
  0x0001 - Hold 1 lamp.
  0x0002 - Hold 2 lamp.
  0x0004 - Hold 3 lamp.
  0x0008 - Hold 4 lamp.
  0x0010 - Hold 5 lamp.
  0x0020 - Start lamp.
  0x0040 - Bet lamp.
  0x1000 - Hopper out.
  0x2000 - Coin counter.


    - Lbits -
    7654 3210
    =========
    ---- ---x  Hold1 lamp.
    ---- --x-  Hold2 lamp.
    ---- -x--  Hold3 lamp.
    ---- x---  Hold4 lamp.
    ---x ----  Hold5 lamp.
    --x- ----  Start lamp.
    -x-- ----  Bet lamp.

    - Hbits -
    7654 3210
    =========
    ---x ----  Hopper out.
    --x- ----  Coin counter.

*/
	output_set_lamp_value(1, (data & 1));           /* Lamp 1 - HOLD 1 */
	output_set_lamp_value(2, (data >> 1) & 1);      /* Lamp 2 - HOLD 2 */
	output_set_lamp_value(3, (data >> 2) & 1);      /* Lamp 3 - HOLD 3 */
	output_set_lamp_value(4, (data >> 3) & 1);      /* Lamp 4 - HOLD 4 */
	output_set_lamp_value(5, (data >> 4) & 1);      /* Lamp 5 - HOLD 5 */
	output_set_lamp_value(6, (data >> 5) & 1);      /* Lamp 6 - START  */
	output_set_lamp_value(7, (data >> 6) & 1);      /* Lamp 7 - BET    */

	coin_counter_w(machine(), 0, data & 0x2000);
}


WRITE16_MEMBER(sderby_state::roulette_out_w)
{
/*
  -----------------------------------
  --- Croupier (Roulette) Outputs ---
  -----------------------------------

  0x708006 - 0x708007
  ===================

  0x0000 - Normal State (lamps off).
  0x0001 - Start lamp.
  0x0002 - Bet lamp.
  0x0008 - Unknown (always activated).


    - Lbits -
    7654 3210
    =========
    ---- ---x  Start lamp.
    ---- --x-  Bet lamp.
    ---- x---  Unknown (always activated).

*/
	output_set_lamp_value(1, (data & 1));           /* Lamp 1 - START */
	output_set_lamp_value(2, (data >> 1) & 1);      /* Lamp 2 - BET   */
}


/***************************
*       Memory Maps        *
***************************/

static ADDRESS_MAP_START( sderby_map, AS_PROGRAM, 16, sderby_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x100fff) AM_RAM_WRITE(sderby_videoram_w) AM_SHARE("videoram")       /* bg */
	AM_RANGE(0x101000, 0x101fff) AM_RAM_WRITE(sderby_md_videoram_w) AM_SHARE("md_videoram") /* mid */
	AM_RANGE(0x102000, 0x103fff) AM_RAM_WRITE(sderby_fg_videoram_w) AM_SHARE("fg_videoram") /* fg */
	AM_RANGE(0x104000, 0x10400b) AM_WRITE(sderby_scroll_w)
	AM_RANGE(0x10400c, 0x10400d) AM_WRITENOP    /* ??? - check code at 0x000456 (executed once at startup) */
	AM_RANGE(0x10400e, 0x10400f) AM_WRITENOP    /* ??? - check code at 0x000524 (executed once at startup) */
	AM_RANGE(0x200000, 0x200fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x308000, 0x30800d) AM_READ(sderby_input_r)
	AM_RANGE(0x308008, 0x308009) AM_WRITE(sderby_out_w) /* output port */
	AM_RANGE(0x30800e, 0x30800f) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x380000, 0x380fff) AM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x500000, 0x500001) AM_WRITENOP    /* unknown... write 0x01 in game, and 0x00 on reset */
	AM_RANGE(0xd00000, 0xd007ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xffc000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sderbya_map, AS_PROGRAM, 16, sderby_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x100fff) AM_RAM_WRITE(sderby_videoram_w) AM_SHARE("videoram")       /* bg */
	AM_RANGE(0x101000, 0x101fff) AM_RAM_WRITE(sderby_md_videoram_w) AM_SHARE("md_videoram") /* mid */
	AM_RANGE(0x102000, 0x103fff) AM_RAM_WRITE(sderby_fg_videoram_w) AM_SHARE("fg_videoram") /* fg */
	AM_RANGE(0x104000, 0x10400b) AM_WRITE(sderby_scroll_w)
	AM_RANGE(0x10400c, 0x10400d) AM_WRITENOP    /* ??? - check code at 0x000456 (executed once at startup) */
	AM_RANGE(0x10400e, 0x10400f) AM_WRITENOP    /* ??? - check code at 0x000524 (executed once at startup) */
	AM_RANGE(0x200000, 0x200fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x308000, 0x30800d) AM_READ(sderbya_input_r)
	AM_RANGE(0x308008, 0x308009) AM_WRITE(sderby_out_w) /* output port */
	AM_RANGE(0x30800e, 0x30800f) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x380000, 0x380fff) AM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x300000, 0x300001) AM_WRITENOP    /* unknown... write 0x01 in game, and 0x00 on reset */      //MOD
	AM_RANGE(0xcf0000, 0xcf07ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xcfc000, 0xcfffff) AM_RAM                                                                     //MOD
ADDRESS_MAP_END


static ADDRESS_MAP_START( luckboom_map, AS_PROGRAM, 16, sderby_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x100fff) AM_RAM_WRITE(sderby_videoram_w) AM_SHARE("videoram")       /* bg */
	AM_RANGE(0x101000, 0x101fff) AM_RAM_WRITE(sderby_md_videoram_w) AM_SHARE("md_videoram") /* mid */
	AM_RANGE(0x102000, 0x103fff) AM_RAM_WRITE(sderby_fg_videoram_w) AM_SHARE("fg_videoram") /* fg */
	AM_RANGE(0x104000, 0x10400b) AM_WRITE(sderby_scroll_w)
	AM_RANGE(0x10400c, 0x10400d) AM_WRITENOP    /* ??? - check code at 0x000456 (executed once at startup) */
	AM_RANGE(0x10400e, 0x10400f) AM_WRITENOP    /* ??? - check code at 0x000524 (executed once at startup) */
	AM_RANGE(0x200000, 0x200fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x308000, 0x30800d) AM_READ(sderby_input_r)
	AM_RANGE(0x308008, 0x308009) AM_WRITE(sderby_out_w) /* output port */
	AM_RANGE(0x30800e, 0x30800f) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x380000, 0x380fff) AM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x500000, 0x500001) AM_WRITENOP    /* unknown... write 0x01 in game, and 0x00 on reset */
	AM_RANGE(0xe00000, 0xe007ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( spacewin_map, AS_PROGRAM, 16, sderby_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x100fff) AM_RAM_WRITE(sderby_videoram_w) AM_SHARE("videoram")       /* bg */
	AM_RANGE(0x101000, 0x101fff) AM_RAM_WRITE(sderby_md_videoram_w) AM_SHARE("md_videoram") /* mid */
	AM_RANGE(0x102000, 0x103fff) AM_RAM_WRITE(sderby_fg_videoram_w) AM_SHARE("fg_videoram") /* fg */
	AM_RANGE(0x104000, 0x10400b) AM_WRITE(sderby_scroll_w)  /* tilemaps offset control */
	AM_RANGE(0x10400c, 0x10400d) AM_WRITENOP    /* seems another video register. constantly used */
	AM_RANGE(0x10400e, 0x10400f) AM_WRITENOP    /* seems another video register. constantly used */
	AM_RANGE(0x104010, 0x105fff) AM_WRITENOP    /* unknown */
	AM_RANGE(0x300000, 0x300001) AM_WRITENOP    /* unknown... write 0x01 in game, and 0x00 on reset */
	AM_RANGE(0x308000, 0x30800d) AM_READ(sderby_input_r)
	AM_RANGE(0x308008, 0x308009) AM_WRITE(scmatto_out_w)    /* output port */
	AM_RANGE(0x30800e, 0x30800f) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x380000, 0x380fff) AM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xd00000, 0xd001ff) AM_RAM
	AM_RANGE(0x800000, 0x800fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x801000, 0x80100d) AM_WRITENOP    /* unknown */
	AM_RANGE(0x8f0000, 0x8f07ff) AM_RAM AM_SHARE("nvram")   /* 16K Dallas DS1220Y-200 NVRAM */
	AM_RANGE(0x8fc000, 0x8fffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( roulette_map, AS_PROGRAM, 16, sderby_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x440000, 0x440fff) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0x500000, 0x500fff) AM_RAM_WRITE(sderby_videoram_w) AM_SHARE("videoram")           /* bg */
	AM_RANGE(0x501000, 0x501fff) AM_RAM_WRITE(sderby_md_videoram_w) AM_SHARE("md_videoram") /* mid */
	AM_RANGE(0x502000, 0x503fff) AM_RAM_WRITE(sderby_fg_videoram_w) AM_SHARE("fg_videoram") /* fg */
	AM_RANGE(0x504000, 0x50400b) AM_RAM_WRITE(sderby_scroll_w)
	AM_RANGE(0x50400e, 0x50400f) AM_WRITENOP

	AM_RANGE(0x708000, 0x708009) AM_READ(roulette_input_r)
	AM_RANGE(0x708006, 0x708007) AM_WRITE(roulette_out_w)
	AM_RANGE(0x70800a, 0x70800b) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x70800c, 0x70800d) AM_WRITENOP    /* watchdog?? (0x0003) */
	AM_RANGE(0x70800e, 0x70800f) AM_READWRITE(rprot_r, rprot_w) /* MCU communication */
	AM_RANGE(0x780000, 0x780fff) AM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")

	AM_RANGE(0xff0000, 0xff07ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xffc000, 0xffffff) AM_RAM
ADDRESS_MAP_END



/***************************
*       Input Ports        *
***************************/

static INPUT_PORTS_START( sderby )
	PORT_START("IN0")   /* 0x308000.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_4WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_4WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_4WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Collect")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )        // Adds n credits depending on settings in service menu
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START1 )
	PORT_SERVICE_NO_TOGGLE(0x1000, IP_ACTIVE_LOW)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )      // check code at 0x00765e
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( sderbya )
	PORT_START("IN0")   /* 0x308000.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_4WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_4WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_4WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Collect")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )        // Adds n credits depending on settings in service menu
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START1 )
	PORT_SERVICE_NO_TOGGLE(0x1000, IP_ACTIVE_LOW)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )      // check code at 0x00765e
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( luckboom )
	PORT_START("IN0")   /* 0x308000.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START1 )
	PORT_SERVICE_NO_TOGGLE(0x1000, IP_ACTIVE_LOW)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( spacewin )
	PORT_START("IN0")   /* 0x308000.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x1000, IP_ACTIVE_LOW)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( pmroulet )
	PORT_START("IN0")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen") /* it must be toggled to boot anyway */
	PORT_SERVICE_NO_TOGGLE(0x0020, IP_ACTIVE_LOW)
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x000e, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00f0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) /* to cancel bets in 3-button mode */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/****************************
*     Graphics Layouts      *
****************************/

static const gfx_layout tiles8x8_layout =
{
	8, 8,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(4,5), RGN_FRAC(3,5), RGN_FRAC(2,5), RGN_FRAC(1,5), RGN_FRAC(0,5) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tiles16x16_layout =
{
	16, 16,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(4,5), RGN_FRAC(3,5), RGN_FRAC(2,5), RGN_FRAC(1,5), RGN_FRAC(0,5) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		128+0, 128+1, 128+2, 128+3, 128+4, 128+5, 128+6, 128+7
	},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		64+0*8,64+1*8,64+2*8,64+3*8,64+4*8,64+5*8,64+6*8,64+7*8
	},
	256,
};


/****************************
*      Graphics Decode      *
****************************/

static GFXDECODE_START( sderby )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout,   0x000, 256  ) /* sprites */
	GFXDECODE_ENTRY( "gfx1", 0, tiles16x16_layout, 0x000, 256  ) /* sprites */
GFXDECODE_END


/****************************
*      Machine Drivers      *
****************************/

static MACHINE_CONFIG_START( sderby, sderby_state )

	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(sderby_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sderby_state,  irq2_line_hold)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(4*8, 44*8-1, 3*8, 33*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(sderby_state, screen_update_sderby)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sderby)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(RRRRRGGGGGBBBBBx)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) /* clock frequency & pin 7 not verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( sderbya, sderby_state )

	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(sderbya_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sderby_state,  irq2_line_hold)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(4*8, 44*8-1, 3*8, 33*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(sderby_state, screen_update_sderby)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sderby)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(RRRRRGGGGGBBBBBx)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) /* clock frequency & pin 7 not verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



static MACHINE_CONFIG_START( luckboom, sderby_state )

	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(luckboom_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sderby_state,  irq4_line_hold)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(4*8, 44*8-1, 3*8, 33*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(sderby_state, screen_update_sderby)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sderby)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(RRRRRGGGGGBBBBBx)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) /* clock frequency & pin 7 not verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( spacewin, sderby_state )

	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(spacewin_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sderby_state,  irq2_line_hold)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(4*8, 44*8-1, 3*8, 33*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(sderby_state, screen_update_pmroulet)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sderby)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(RRRRRGGGGGBBBBBx)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) /* clock frequency & pin 7 not verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( pmroulet, sderby_state )

	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(roulette_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sderby_state,  irq2_line_hold)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(4*8, 44*8-1, 3*8, 33*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(sderby_state, screen_update_pmroulet)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sderby)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(RRRRRGGGGGBBBBBx)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) /* clock frequency & pin 7 not verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/****************************
*        Rom Loads          *
****************************/
/*

Super Derby
Playmark '96

mc 68k 12mhz
OKI m6295

2x GM 76c88al (8kx8 6264) near program ROMs
2x 6264 near gfx ROMs
16k nonvolatile SRAM (Dallas 1220Y)
two FPGA chips - one labelled 'Playmark 010412'
4x hm3-65728bk-5
--
21.bin 6F9F2F2B - near OKI (samples?)

22.bin A319F1E0 - program code
23.bin 1D6E2321 /

24.bin 93C917DF  - gfx
25.bin 7BA485CD
26.bin BEABE4F7
27.bin 672CE5DF
28.bin 39CA3B52

*/
ROM_START( sderby )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "22.bin", 0x00000, 0x20000, CRC(a319f1e0) SHA1(d932cc7e990aa87308dcd9ffa5af2aaea333aa9a) )
	ROM_LOAD16_BYTE( "23.bin", 0x00001, 0x20000, CRC(1d6e2321) SHA1(3bb32021cc9ee6bd6d1fd79a89159fef70f34f41) )

	ROM_REGION( 0x080000, "oki", 0 ) /* Samples */
	ROM_LOAD( "21.bin", 0x00000, 0x80000, CRC(6f9f2f2b) SHA1(9778439979bc21b3e49f0c16353488a33b93c01b) )

	ROM_REGION( 0xa0000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "24.bin", 0x00000, 0x20000, CRC(93c917df) SHA1(dc2fa5e29749ec92871c66146c0412a23f47e316) )
	ROM_LOAD( "25.bin", 0x20000, 0x20000, CRC(7ba485cd) SHA1(b0170614d713af9d1556251c76ae762de872abe6) )
	ROM_LOAD( "26.bin", 0x40000, 0x20000, CRC(beabe4f7) SHA1(a5615450fae930cb2408f201a9faa12551de0d70) )
	ROM_LOAD( "27.bin", 0x60000, 0x20000, CRC(672ce5df) SHA1(cdf3af842cbcbf53cc73d9986744dc9cfa92c71a) )
	ROM_LOAD( "28.bin", 0x80000, 0x20000, CRC(39ca3b52) SHA1(9a03e73d88a1551cd3cfe616ab71e67dced1272a) )
ROM_END

ROM_START( sderbya )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "22.u16", 0x00000, 0x20000, CRC(5baadc33) SHA1(475843d3f99f5a6aa25bdba75b251ad6be32802f) )
	ROM_LOAD16_BYTE( "23.u15", 0x00001, 0x20000, CRC(04518b8c) SHA1(97598c43c1cb0a757bca70c0a498838144b2302b) )

	ROM_REGION( 0x080000, "oki", 0 ) /* Samples */
	ROM_LOAD( "274001.u147", 0x00000, 0x80000,  CRC(6f9f2f2b) SHA1(9778439979bc21b3e49f0c16353488a33b93c01b) )

	ROM_REGION( 0x140000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "24.u141", 0x000000, 0x40000, CRC(54234f72) SHA1(c2e3e547255174daa9a6acad743c210b92ded385) )
	ROM_LOAD( "25.u142", 0x040000, 0x40000, CRC(1bd98cf7) SHA1(67143a16e2cf01868fac3d8d6d9db423f77d09b9) )
	ROM_LOAD( "26.u143", 0x080000, 0x40000, CRC(21bf2004) SHA1(7f7cbdcd89807c9e8ca64023a0060b18ef9b6536) )
	ROM_LOAD( "27.u144", 0x0c0000, 0x40000, CRC(097e0b27) SHA1(26ea6c0996a62e52241b335be64f46d7586f997a) )
	ROM_LOAD( "28.u145", 0x100000, 0x40000, CRC(580daff7) SHA1(2813faa3509d86ad1f721381fa600f28f8f51e09) )
ROM_END


/* Scacco Matto / Space Win
   Playmark, 1996.

CPU:
1x MC68000P12 (main)(u24)

Sound:
1x M6295 (sound)(u146)
1x TDA2003 (sound)(td1)
1x 358D (sound)(u155)

PLDs:
2x A1020B-PL84C (u110,u137)(not dumped)

Xtals:
1x oscillator 24.000000MHz (xl1)
1x oscillator 28.000000MHz (xl2)
1x blu resonator 1000J (sound)(y1)

ROMs:
1x M27C2001 (1)
7x M27C1001 (2,3,4,5,6,7,8)

4x PALCE22V10H (not dumped)
1x PALCE16V8H (not dumped)
1x DS1220Y-200 (non volatile ram)

Note:
1x JAMMA edge connector
1x 12 legs connector
2x 6 legs connector
1x pushbutton (cb1)
1x trimmer (volume)

Title depends on graphics type in test mode.

*/
ROM_START( spacewin )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "2.u16", 0x00000, 0x20000, CRC(2d17c2ab) SHA1(833ab39081fbc3d114103055e3a3f2ea2a28f158) )
	ROM_LOAD16_BYTE( "3.u15", 0x00001, 0x20000, CRC(fd6f93ef) SHA1(1dc35fd92a0185434b44aa3c7da47a408fb2ce27) )

	ROM_REGION( 0x080000, "oki", 0 ) /* Samples */
	ROM_LOAD( "1.u147", 0x00000, 0x40000,  CRC(eedc7090) SHA1(fc8cabf7a11a1de3ccc3b8860afd8f32669608b8) )

	ROM_REGION( 0xa0000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "4.u141", 0x000000, 0x20000, CRC(ce20c599) SHA1(93358c3a891c66ca944eb684bd47e9c25bfcb88d) )
	ROM_LOAD( "5.u142", 0x020000, 0x20000, CRC(ae4f8e06) SHA1(cb3b941c67ae5c9df005d9f8b2105dc5a114f19f) )
	ROM_LOAD( "6.u143", 0x040000, 0x20000, CRC(b99afc96) SHA1(589270dbd5022db2d032da85a9813c271fa71a90) )
	ROM_LOAD( "7.u144", 0x060000, 0x20000, CRC(30f212ad) SHA1(ecafd93b11ff15386ef02e9f6657a52baf7932b4) )
	ROM_LOAD( "8.u145", 0x080000, 0x20000, CRC(541a73fd) SHA1(fede5e2fcbb18e90cc50995d44e831c3f9b56614) )
ROM_END


/*
Croupier (PlaYMark)
-------------------

1x MC68000FN12 (U24, QFP)
1x unknown scratched 40-pin DIP IC (U39)

1x OKI M6295 (U34, near ROM 1)
1x Resonator 1000J (Y1)

2x KM6264BL-10L SRAM (U2/U6, near ROMs 2 & 3)
2x KM6264BL-10 SRAM (U36/U37, near ROMs 4, 5, 6, 7 & 8)
1x DALLAS DS1220Y-100 NONVOLATILE SRAM (U17)

2x ACTEL A1020B PL84C
4x GAL 22CV10-25LNC (U1, U77, U111, U112. Undumped)

1x 24.000000 MHz. Xtal. (XTL1)
1x 28.000000 MHz. Xtal. (XTL2)

1x Pot (TR1, near OKI M6295)
1x Push Button.
1x JAMMA connector.

ROMs:
-----

1.bin : AMD Am27C020.
checksum : 03FC0000h
CRC-32 : B7094978h

2.bin : ST M27C1001.
checksum : 00B958B6h
CRC-32 : E7941975h

3.bin : ST M27C1001.
checksum : 011DE502h
CRC-32 : 29D06A38h

4.bin : MACRONIX MX27C4000.
checksum : 01CADC6Ch
CRC-32 : EFCDDAC9h

5.bin : MACRONIX MX27C4000.
checksum : 01F8DEB8h
CRC-32 : BC75EF8Fh

6.bin : MACRONIX MX27C4000.
checksum : 0195B5C0h
CRC-32 : E47D5F55h

7.bin : MACRONIX MX27C4000.
checksum : 013483F4h
CRC-32 : 0FA6CE7Dh

8.bin : MACRONIX MX27C4000.
checksum : 0106A95Ch
CRC-32 : D4C2B7DAh

*/
ROM_START( croupier )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "2.bin", 0x00000, 0x20000, CRC(e7941975) SHA1(ea32cd51b8d87205a1d6c6a83ebf8b50e03c55fc))
	ROM_LOAD16_BYTE( "3.bin", 0x00001, 0x20000, CRC(29d06a38) SHA1(c6fdca1a31fad9abf854e521e593f3ec8018ae6d))

	ROM_REGION( 0x080000, "oki", 0 ) /* samples are ok */
	ROM_LOAD( "1.bin", 0x00000, 0x40000, CRC(6673de85) SHA1(df390cd6268efc0e743a9020f19bc0cbeb757cfa))

	ROM_REGION( 0x280000, "gfx1", 0 ) /* sprites */
	ROM_LOAD( "4.bin", 0x000000, 0x80000, CRC(efcddac9) SHA1(72435ec478b70a067d47f3daf7c224169ee5827a))
	ROM_LOAD( "5.bin", 0x080000, 0x80000, CRC(bc75ef8f) SHA1(1f3dc457e5ae143d53cfef0e1fcb4586dceefb67))
	ROM_LOAD( "6.bin", 0x100000, 0x80000, CRC(e47d5f55) SHA1(a341e24f98125265cb3986f8c7ce84eedd056b71))
	ROM_LOAD( "7.bin", 0x180000, 0x80000, CRC(0fa6ce7d) SHA1(5ba96c9c0625a131d890d9c0c0f65cb2a03fa084))
	ROM_LOAD( "8.bin", 0x200000, 0x80000, CRC(d4c2b7da) SHA1(515be861443acc5b911241dbaafa42e02f79985a))
ROM_END

ROM_START( croupiera )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "2.bin", 0x00000, 0x20000, CRC(1677a2de) SHA1(4dcbb3c1ce9b65e06ba7e0cffa00c0c8016538f5)) // sldh
	ROM_LOAD16_BYTE( "3.bin", 0x00001, 0x20000, CRC(11acaac2) SHA1(19e7bbbf4356fc9a866f9f36d0568c42d6a36c07)) // sldh

	ROM_REGION( 0x080000, "oki", 0 ) /* samples are ok */
	ROM_LOAD( "1.bin", 0x00000, 0x40000, CRC(6673de85) SHA1(df390cd6268efc0e743a9020f19bc0cbeb757cfa))

	ROM_REGION( 0x280000, "gfx1", 0 ) /* sprites */
	ROM_LOAD( "4.bin", 0x000000, 0x80000, CRC(efcddac9) SHA1(72435ec478b70a067d47f3daf7c224169ee5827a))
	ROM_LOAD( "5.bin", 0x080000, 0x80000, CRC(bc75ef8f) SHA1(1f3dc457e5ae143d53cfef0e1fcb4586dceefb67))
	ROM_LOAD( "6.bin", 0x100000, 0x80000, CRC(e47d5f55) SHA1(a341e24f98125265cb3986f8c7ce84eedd056b71))
	ROM_LOAD( "7.bin", 0x180000, 0x80000, CRC(0fa6ce7d) SHA1(5ba96c9c0625a131d890d9c0c0f65cb2a03fa084))
	ROM_LOAD( "8.bin", 0x200000, 0x80000, CRC(d4c2b7da) SHA1(515be861443acc5b911241dbaafa42e02f79985a))
ROM_END

/*
Lucky Boom (c)1996 Playmark

QTY     Type                    clock           position    function
1x      MC68000P12                              u24         16/32-bit Microprocessor - main
1x      M6295                                   u146        4-Channel Mixing ADCPM Voice Synthesis LSI - sound
1x      KA358                                   u155        Dual Operational Amplifier - sound
1x      TDA2003                                 TD1         Audio Amplifier - sound
1x      oscillator              24.000000MHz    XL1
1x      oscillator              28.000000MHz    XL2
1x      blu resonator   1000J                   Y1

ROMs
QTY     Type        position    status
1x      TMS27C020   1           dumped
2x      AM27C010    2,3         dumped
5x      TMS27C010   4-8         dumped

RAMs
QTY     Type            position
8x      HM3-65728BK-5   u42,u73,u74,u75,u80,u81,u124,u125
2x      HY62256ALP-10   u2,u6
2x      KM6264BL-7L     u36,u37
PLDs
QTY     Type                position            status
2x      A1020B-PL84C        u110,u137           read protected
4x      TIBPAL22V10ACNT     u1,u77,u111,u112    read protected
*/



ROM_START( luckboom )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "2.u16", 0x00000, 0x20000, CRC(0a20eaca) SHA1(edd325b60b3a3ce84e89f8be91b9e19f82d15317) )
	ROM_LOAD16_BYTE( "3.u15", 0x00001, 0x20000, CRC(0d3bb24c) SHA1(31826ec29650aa8b70e1b713678401113e008832) )

	ROM_REGION( 0x080000, "oki", 0 ) /* Samples */
	ROM_LOAD( "1.u147", 0x00000, 0x40000, CRC(0d42c0a3) SHA1(1b1d4c7dcbb063e8bf133063770b753947d1a017) )

	ROM_REGION( 0xa0000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "4.u141", 0x000000, 0x20000, CRC(3aeccad7) SHA1(4104dd3ae928c1a876ac9b460fe18b58ce1206f7) )
	ROM_LOAD( "5.u142", 0x020000, 0x20000, CRC(4e4f9ac6) SHA1(e9b0c48195d8b21cdab16d66423ff43e6292ef2c) )
	ROM_LOAD( "6.u143", 0x040000, 0x20000, CRC(d1b4910e) SHA1(cfcb14bcd992bceaa634b20eb2d446caa62b6d82) )
	ROM_LOAD( "7.u144", 0x060000, 0x20000, CRC(00334bad) SHA1(2d8af2a4b517ceb1ae5504b15086c8faaf934b53) )
	ROM_LOAD( "8.u145", 0x080000, 0x20000, CRC(dc12df50) SHA1(0796d6428dc3a032ea55e06e8e07245a256ed036) )
ROM_END


/******************************
*        Game Drivers         *
******************************/

/*     YEAR  NAME       PARENT    MACHINE   INPUT     INIT           ROT    COMPANY            FULLNAME                               FLAGS                                          LAYOUT  */
GAMEL( 1996, sderby,    0,        sderby,   sderby,   driver_device, 0,     ROT0, "Playmark", "Super Derby (v.07.03)",                0,                                             layout_sderby   )
GAMEL( 1996, sderbya,   sderby,   sderbya,  sderbya,  driver_device, 0,     ROT0, "Playmark", "Super Derby (v.10.04)",                0,                                             layout_sderby   )
GAMEL( 1996, spacewin,  0,        spacewin, spacewin, driver_device, 0,     ROT0, "Playmark", "Scacco Matto / Space Win",             0,                                             layout_spacewin )
GAMEL( 1997, croupier,  0,        pmroulet, pmroulet, driver_device, 0,     ROT0, "Playmark", "Croupier (Playmark Roulette v.20.05)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING, layout_pmroulet )
GAMEL( 1997, croupiera, croupier, pmroulet, pmroulet, driver_device, 0,     ROT0, "Playmark", "Croupier (Playmark Roulette v.09.04)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING, layout_pmroulet )
GAME ( 1996, luckboom,  0,        luckboom, luckboom, driver_device, 0,     ROT0, "Playmark", "Lucky Boom",                           0 )
