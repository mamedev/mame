/*******************************************************************************
 Shadow Force (c)1993 Technos
 Preliminary Driver by David Haywood
 Based on the Various Other Technos Games
********************************************************************************

Stephh's notes :

  - As for some other M68000 Technos games (or games running on similar hardware
    such as 'mugsmash'), the Inputs and the Dip Switches are mangled, so you need
    a specific read handler so end-uers can see them in a "standard" order.
    IMO you should set the USE_SHADFRCE_FAKE_INPUT_PORTS macro to 1.
  - Why do "Coin 2" and "Service 1" buttons only work in "test mode" ?

 To Do:

 Graphic Glitches
  Spurious sprite at the top right of the title screen. Visible area too large?
 Other Interrupt?

*******************************************************************************

-- Read Me --

Shadow Force (c)1993 Technos
TA-0032-P1-23 (Japan version is TA-0032-P1-24)

CPU: TMP68HC000P-16
Sound: Z80, YM2151, YM3012, OKI6295, MB3615 (x2)
Custom: Actel A1010A-1 (TJ32A 92.11.30), TJ-002, TJ-004, TJ-005

X1: 28 MHz
X2: 3.579545 MHz
X4: 13.4952 MHz

ROMs:

Program:
32A12-01.34 = 27C2001
32A14-0.33  = 27C2001
32A13-01.26 = 27C2001
32A15-0.14  = 27C2001

Char:
32A11-0.55 = 27C010

Backgrounds:
32J1-0.4 = 8meg mask
32J2-0.5 = 8meg mask
32J3-0.6 = 8meg mask

Sprites:
32J4-0.12 = 16meg mask
32J5-0.13 = 16meg mask
32J6-0.24 = 16meg mask
32J7-0.25 = 16meg mask
32J8-0.32 = 16meg mask

Sound CPU:
32J10.42 = 27C512

Samples:
32J9-0.76 = 27C040

*******************************************************************************/


/*
68k interrupts
lev 1 : 0x64 : 0004 fd00 - ?
lev 2 : 0x68 : 0000 1f32 - vblank?
lev 3 : 0x6c : 0000 10f4 - ?
lev 4 : 0x70 : 0000 11d0 - just rte
lev 5 : 0x74 : 0000 11d0 - just rte
lev 6 : 0x78 : 0000 11d0 - just rte
lev 7 : 0x7c : 0000 11d0 - just rte
*/

#include "driver.h"
#include "deprecat.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"

UINT16 *shadfrce_fgvideoram, *shadfrce_bg0videoram,  *shadfrce_bg1videoram,   *shadfrce_spvideoram;
/* UINT16 *shadfrce_videoregs; */

/* in video */
WRITE16_HANDLER ( shadfrce_bg0scrollx_w );
WRITE16_HANDLER ( shadfrce_bg1scrollx_w );
WRITE16_HANDLER ( shadfrce_bg0scrolly_w );
WRITE16_HANDLER ( shadfrce_bg1scrolly_w );
VIDEO_START( shadfrce );
VIDEO_EOF(shadfrce);
VIDEO_UPDATE( shadfrce );
WRITE16_HANDLER( shadfrce_fgvideoram_w );
WRITE16_HANDLER( shadfrce_bg0videoram_w );
WRITE16_HANDLER( shadfrce_bg1videoram_w );


static WRITE16_HANDLER( shadfrce_flip_screen )
{
	flip_screen_set(data & 0x01);
}


/* Ports mapping :

    $1d0020.w : 0123456789ABCDEF
                x---------------    right     (player 1)
                -x--------------    left      (player 1)
                --x-------------    up        (player 1)
                ---x------------    down      (player 1)
                ----x-----------    button 1  (player 1)
                -----x----------    button 2  (player 1)
                ------x---------    button 3  (player 1)
                -------x--------    start     (player 1)
                --------x-------    coin 1
                ---------x------    coin 2                       *
                ----------x-----    service 1                    *
                -----------x----    unused
                ------------x---    DIP2-7
                -------------x--    DIP2-8
                --------------x-    unused
                ---------------x    unused

    $1d0022.w : 0123456789ABCDEF
                x---------------    right     (player 2)
                -x--------------    left      (player 2)
                --x-------------    up        (player 2)
                ---x------------    down      (player 2)
                ----x-----------    button 1  (player 2)
                -----x----------    button 2  (player 2)
                ------x---------    button 3  (player 2)
                -------x--------    start     (player 2)
                --------x-------    DIP2-1    ("Difficulty")
                ---------x------    DIP2-2    ("Difficulty")
                ----------x-----    DIP2-3    ("Stage Clear Energy Regain")
                -----------x----    DIP2-4    ("Stage Clear Energy Regain")
                ------------x---    DIP2-5
                -------------x--    DIP2-6
                --------------x-    unused
                ---------------x    unused

    $1d0024.w : 0123456789ABCDEF
                x---------------    button 4  (player 1)
                -x--------------    button 5  (player 1)
                --x-------------    button 6  (player 1)
                ---x------------    button 4  (player 2)
                ----x-----------    button 5  (player 2)
                -----x----------    button 6  (player 2)
                ------x---------    unused
                -------x--------    unused
                --------x-------    DIP1-1
                ---------x------    DIP1-2    ("Coin(s) for Credit(s)")
                ----------x-----    DIP1-3    ("Coin(s) for Credit(s)")
                -----------x----    DIP1-4    (DEF_STR( Continue_Price ))
                ------------x---    DIP1-5    ("Free Play")
                -------------x--    DIP1-6    ("Flip Screen")
                --------------x-    unused
                ---------------x    unused

    $1d0026.w : 0123456789ABCDEF
                x---------------    unused
                -x--------------    unused
                --x-------------    unused
                ---x------------    unused
                ----x-----------    unused
                -----x----------    unused
                ------x---------    unused
                -------x--------    unused
                --------x-------    DIP 1-7   ("Demo Sound")
                ---------x------    DIP 1-8   ("Test Mode")
                ----------x-----    vblank?
                -----------x----    unused
                ------------x---    unknown
                -------------x--    unused
                --------------x-    unused
                ---------------x    unused

    * only available when you are in "test mode"

*/

#define USE_SHADFRCE_FAKE_INPUT_PORTS	1

#if USE_SHADFRCE_FAKE_INPUT_PORTS
static READ16_HANDLER( shadfrce_input_ports_r )
{
	UINT16 data = 0xffff;

	switch (offset)
	{
		case 0 :
			data = (input_port_read(machine, "P1") & 0xff) | ((input_port_read(machine, "DSW2") & 0xc0) << 6) | ((input_port_read(machine, "SYSTEM") & 0x0f) << 8);
			break;
		case 1 :
			data = (input_port_read(machine, "P2") & 0xff) | ((input_port_read(machine, "DSW2") & 0x3f) << 8);
			break;
		case 2 :
			data = (input_port_read(machine, "EXTRA") & 0xff) | ((input_port_read(machine, "DSW1") & 0x3f) << 8);
			break;
		case 3 :
			data = (input_port_read(machine, "OTHER") & 0xff) | ((input_port_read(machine, "DSW1") & 0xc0) << 2) | ((input_port_read(machine, "MISC") & 0x3c) << 8);
			break;
	}

	return (data);
}
#endif

static WRITE16_HANDLER ( shadfrce_sound_brt_w )
{
	if (ACCESSING_BITS_8_15)
	{
		soundlatch_w(machine,1,data >> 8);
		cpunum_set_input_line(machine, 1, INPUT_LINE_NMI, PULSE_LINE );
	}
	else
	{
		int i;
		double brt = (data & 0xff) / 255.0;

		for (i = 0; i < 0x4000; i++)
			palette_set_brightness(machine,i,brt);
	}
}

/* Memory Maps - ToDo, Work Out Unknowns */

static ADDRESS_MAP_START( shadfrce_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_READ(SMH_ROM)
	AM_RANGE(0x100000, 0x100fff) AM_READ(SMH_RAM)
	AM_RANGE(0x101000, 0x101fff) AM_READ(SMH_RAM)
	AM_RANGE(0x102000, 0x1027ff) AM_READ(SMH_RAM)
	AM_RANGE(0x102800, 0x103fff) AM_READ(SMH_RAM)
	AM_RANGE(0x140000, 0x141fff) AM_READ(SMH_RAM)
	AM_RANGE(0x142000, 0x143fff) AM_READ(SMH_RAM)
	AM_RANGE(0x180000, 0x187fff) AM_READ(SMH_RAM)
	/* inputs, dipswitches etc. */
	AM_RANGE(0x1c000a, 0x1c000b) AM_READ(SMH_NOP) /* ?? */
	AM_RANGE(0x1d000c, 0x1d000d) AM_READ(SMH_NOP) /* ?? */
#if USE_SHADFRCE_FAKE_INPUT_PORTS
	AM_RANGE(0x1d0020, 0x1d0027) AM_READ(shadfrce_input_ports_r)
#else
	AM_RANGE(0x1d0020, 0x1d0021) AM_READ_PORT("IN0")
	AM_RANGE(0x1d0022, 0x1d0023) AM_READ_PORT("IN1")
	AM_RANGE(0x1d0024, 0x1d0025) AM_READ_PORT("IN2")
	AM_RANGE(0x1d0026, 0x1d0027) AM_READ_PORT("IN3")
#endif
	AM_RANGE(0x1F0000, 0x1FFFFF) AM_READ(SMH_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( shadfrce_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x100000, 0x100fff) AM_WRITE(shadfrce_bg0videoram_w) AM_BASE(&shadfrce_bg0videoram) /* video */
	AM_RANGE(0x101000, 0x101fff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x102000, 0x1027ff) AM_WRITE(shadfrce_bg1videoram_w) AM_BASE(&shadfrce_bg1videoram) /* bg 2 */
	AM_RANGE(0x102800, 0x103fff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x140000, 0x141fff) AM_WRITE(shadfrce_fgvideoram_w) AM_BASE(&shadfrce_fgvideoram)
	AM_RANGE(0x142000, 0x143fff) AM_WRITE(SMH_RAM) AM_BASE(&shadfrce_spvideoram) AM_SIZE(&spriteram_size) /* sprites */
	AM_RANGE(0x180000, 0x187fff) AM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE(&paletteram16) /* ?? */
	/* probably video registers etc. */
//  AM_RANGE(0x1c0000, 0x1d000d) AM_WRITE(SMH_RAM) AM_BASE(&shadfrce_videoregs)
	AM_RANGE(0x1c0000, 0x1c0001) AM_WRITE(shadfrce_bg0scrollx_w) /* SCROLL X */
	AM_RANGE(0x1c0002, 0x1c0003) AM_WRITE(shadfrce_bg0scrolly_w) /* SCROLL Y */
	AM_RANGE(0x1c0004, 0x1c0005) AM_WRITE(shadfrce_bg1scrollx_w) /* SCROLL X */
	AM_RANGE(0x1c0006, 0x1c0007) AM_WRITE(shadfrce_bg1scrolly_w) /* SCROLL Y */
	AM_RANGE(0x1c0008, 0x1c0009) AM_WRITE(SMH_NOP) /* ?? */
	AM_RANGE(0x1c000a, 0x1c000b) AM_WRITE(shadfrce_flip_screen)
	AM_RANGE(0x1c000c, 0x1c000d) AM_WRITE(SMH_NOP) /* ?? */
	AM_RANGE(0x1d0000, 0x1d0001) AM_WRITE(SMH_NOP) /* ?? */
	AM_RANGE(0x1d0002, 0x1d0003) AM_WRITE(SMH_NOP) /* ?? */
	AM_RANGE(0x1d0006, 0x1d0007) AM_WRITE(SMH_NOP) /* ?? */
	AM_RANGE(0x1d0008, 0x1d0009) AM_WRITE(SMH_NOP) /* ?? */
	AM_RANGE(0x1d000c, 0x1d000d) AM_WRITE(shadfrce_sound_brt_w)	/* sound command + screen brightness */
	AM_RANGE(0x1d0010, 0x1d0011) AM_WRITE(SMH_NOP) /* ?? */
	AM_RANGE(0x1d0012, 0x1d0013) AM_WRITE(SMH_NOP) /* ?? */
	AM_RANGE(0x1d0014, 0x1d0015) AM_WRITE(SMH_NOP) /* ?? */
	AM_RANGE(0x1d0016, 0x1d0017) AM_WRITE(SMH_NOP) /* ?? */
	AM_RANGE(0x1f0000, 0x1fffff) AM_WRITE(SMH_RAM)
ADDRESS_MAP_END

/* and the sound cpu */

static WRITE8_HANDLER( oki_bankswitch_w )
{
	okim6295_set_bank_base(0, (data & 1) * 0x40000);
}

static ADDRESS_MAP_START( readmem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_READ(SMH_ROM)
	AM_RANGE(0xc000, 0xc7ff) AM_READ(SMH_RAM)
	AM_RANGE(0xc801, 0xc801) AM_READ(YM2151_status_port_0_r)
	AM_RANGE(0xd800, 0xd800) AM_READ(okim6295_status_0_r)
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch_r)
	AM_RANGE(0xf000, 0xffff) AM_READ(SMH_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_WRITE(SMH_ROM)
	AM_RANGE(0xc000, 0xc7ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0xc800, 0xc800) AM_WRITE(YM2151_register_port_0_w)
	AM_RANGE(0xc801, 0xc801) AM_WRITE(YM2151_data_port_0_w)
	AM_RANGE(0xd800, 0xd800) AM_WRITE(okim6295_data_0_w)
	AM_RANGE(0xe800, 0xe800) AM_WRITE(oki_bankswitch_w)
	AM_RANGE(0xf000, 0xffff) AM_WRITE(SMH_RAM)
ADDRESS_MAP_END


/* Input Ports */

/* Similar to MUGSMASH_PLAYER_INPUT in drivers/mugsmash.c */
#define SHADFRCE_PLAYER_INPUT( player, start ) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(player) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(player) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(player) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, start )


#if USE_SHADFRCE_FAKE_INPUT_PORTS
static INPUT_PORTS_START( shadfrce )
	PORT_START("P1")	/* Fake IN0 (player 1 inputs) */
	SHADFRCE_PLAYER_INPUT( 1, IPT_START1 )

	PORT_START("P2")	/* Fake IN1 (player 2 inputs) */
	SHADFRCE_PLAYER_INPUT( 2, IPT_START2 )

	PORT_START("EXTRA")	/* Fake IN2 (players 1 & 2 extra inputs */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("OTHER")	/* Fake IN3 (other extra inputs ?) */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")	/* Fake IN4 (system inputs) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )                   /* only in "test mode" ? */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )                /* only in "test mode" ? */
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MISC")	/* Fake IN5 (misc) */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_VBLANK )                  /* guess */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )                 /* must be ACTIVE_LOW or 'shadfrcj' jumps to the end (code at 0x04902e) */
	PORT_BIT( 0xeb, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")	/* Fake IN6 (DIP1) */
	PORT_DIPNAME( 0x01, 0x01, "Unused DIP 1-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Continue_Price ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW2")	/* Fake IN7 (DIP2) */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )                  /* "Advanced" */
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )               /* "Expert" */
	PORT_DIPNAME( 0x0c, 0x0c, "Stage Clear Energy Regain" )
	PORT_DIPSETTING(    0x04, "50%" )
	PORT_DIPSETTING(    0x0c, "25%" )
	PORT_DIPSETTING(    0x08, "10%" )
	PORT_DIPSETTING(    0x00, "0%" )
	PORT_DIPNAME( 0x10, 0x10, "Unused DIP 2-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unused DIP 2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unused DIP 2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unused DIP 2-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END
#else
static INPUT_PORTS_START( shadfrce )
	PORT_START("IN0")	/* IN0 - $1d0020.w */
	SHADFRCE_PLAYER_INPUT( 1, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )                 /* only in "test mode" ? */
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SERVICE1 )              /* only in "test mode" ? */
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x1000, 0x1000, "Unused DIP 2-7" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Unused DIP 2-8" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")	/* IN1 - $1d0022.w */
	SHADFRCE_PLAYER_INPUT( 2, IPT_START2 )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )              /* "Advanced" */
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )           /* "Expert" */
	PORT_DIPNAME( 0x0c00, 0x0c00, "Stage Clear Energy Regain" )
	PORT_DIPSETTING(      0x0400, "50%" )
	PORT_DIPSETTING(      0x0c00, "25%" )
	PORT_DIPSETTING(      0x0800, "10%" )
	PORT_DIPSETTING(      0x0000, "0%" )
	PORT_DIPNAME( 0x1000, 0x1000, "Unused DIP 2-5" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Unused DIP 2-6" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")	/* IN2 - $1d0024.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, "Unused DIP 1-1" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0600, 0x0600, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Continue_Price ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")	/* IN3 - $1d0026.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_SERVICE( 0x0200, IP_ACTIVE_LOW )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_VBLANK )                /* guess */
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )               /* must be ACTIVE_LOW or 'shadfrcj' jumps to the end (code at 0x04902e) */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END
#endif

/* Graphic Decoding */

static const gfx_layout fg8x8x4_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 2, 4, 6 },
	{ 1, 0, 8*8+1, 8*8+0, 16*8+1, 16*8+0, 24*8+1, 24*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	32*8
};

static const gfx_layout sp16x16x5_layout =
{
	16,16,
	RGN_FRAC(1,5),
	5,
	{ 0x800000*8, 0x600000*8, 0x400000*8, 0x200000*8, 0x000000*8 },
	{ 0,1,2,3,4,5,6,7,16*8+0,16*8+1,16*8+2,16*8+3,16*8+4,16*8+5,16*8+6,16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	16*16
};

static const gfx_layout bg16x16x6_layout =
{
	16,16,
	RGN_FRAC(1,3),
	6,
	{ 0x000000*8+8, 0x000000*8+0, 0x100000*8+8, 0x100000*8+0, 0x200000*8+8, 0x200000*8+0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,16*16+0,16*16+1,16*16+2,16*16+3,16*16+4,16*16+5,16*16+6,16*16+7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16 },
	64*8
};

static GFXDECODE_START( shadfrce )
	GFXDECODE_ENTRY( "gfx1", 0, fg8x8x4_layout,   0x0000, 256 )
	GFXDECODE_ENTRY( "gfx2", 0, sp16x16x5_layout, 0x1000, 128 )
	GFXDECODE_ENTRY( "gfx3", 0, bg16x16x6_layout, 0x2000, 128 )
GFXDECODE_END

/* Machine Driver Bits */

static void irq_handler(running_machine *machine, int irq)
{
	cpunum_set_input_line(machine, 1, 0, irq ? ASSERT_LINE : CLEAR_LINE );
}

static const ym2151_interface ym2151_config =
{
	irq_handler
};

static INTERRUPT_GEN( shadfrce_interrupt ) {
	if( cpu_getiloops() == 0 )
		cpunum_set_input_line(machine, 0, 3, HOLD_LINE);
	else
		cpunum_set_input_line(machine, 0, 2, HOLD_LINE);
}


static MACHINE_DRIVER_START( shadfrce )
	MDRV_CPU_ADD("main", M68000, 28000000/2) /* ? Guess - CPU is rated for 16MHz */
	MDRV_CPU_PROGRAM_MAP(shadfrce_readmem,shadfrce_writemem)
	MDRV_CPU_VBLANK_INT_HACK(shadfrce_interrupt,2)

	MDRV_CPU_ADD("audio", Z80, 3579545)
	MDRV_CPU_PROGRAM_MAP(readmem_sound,writemem_sound)

	MDRV_GFXDECODE(shadfrce)


	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x4000)

	MDRV_VIDEO_START(shadfrce)
	MDRV_VIDEO_EOF(shadfrce)
	MDRV_VIDEO_UPDATE(shadfrce)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD("ym", YM2151, 3579545)
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "left", 0.50)
	MDRV_SOUND_ROUTE(1, "right", 0.50)

	MDRV_SOUND_ADD("oki", OKIM6295, 1584000)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.50)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.50)
MACHINE_DRIVER_END

/* Rom Defs. */

ROM_START( shadfrce )
	ROM_REGION( 0x100000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "32a12-01.34", 0x00001, 0x40000, CRC(04501198) SHA1(50f981c13f9ed19d681d494376018ba86464ea13) )
	ROM_LOAD16_BYTE( "32a13-01.26", 0x00000, 0x40000, CRC(b8f8a05c) SHA1(bd9d4218a7cf57b56aec1f7e710e02af8471f9d7) )
	ROM_LOAD16_BYTE( "32a14-0.33",  0x80001, 0x40000, CRC(08279be9) SHA1(1833526b23feddb58b21874070ad2bf3b6be8dca) )
	ROM_LOAD16_BYTE( "32a15-0.14",  0x80000, 0x40000, CRC(bfcadfea) SHA1(1caa9fc30d8622ce4c7221039c446e99cc8f5346) )

	ROM_REGION( 0x10000, "audio", 0 ) /* Z80 Code */
	ROM_LOAD( "32j10-0.42",  0x00000, 0x10000, CRC(65daf475) SHA1(7144332b2d17af8645e22e1926b33113db0d20e2) )

	ROM_REGION( 0x020000, "gfx1", ROMREGION_DISPOSE ) /* Chars */
	ROM_LOAD( "32a11-0.55",  0x00000, 0x20000, CRC(cfaf5e77) SHA1(eab76e085f695c74cc868aaf95f04ff2acf66ee9) )

	ROM_REGION( 0xa00000, "gfx2", ROMREGION_DISPOSE ) /* Sprite Tiles */
	ROM_LOAD( "32j4-0.12",  0x000000, 0x200000, CRC(1ebea5b6) SHA1(35bd49dda9ad75326d45ffb10c87d83fc4f1b7a8) )
	ROM_LOAD( "32j5-0.13",  0x200000, 0x200000, CRC(600026b5) SHA1(5641246300d7e20dcff1eae004647faaee6cd1c6) )
	ROM_LOAD( "32j6-0.24",  0x400000, 0x200000, CRC(6cde8ebe) SHA1(750933798235951fe24b2e667c33f692612c0aa0) )
	ROM_LOAD( "32j7-0.25",  0x600000, 0x200000, CRC(bcb37922) SHA1(f3eee73c8b9f4873a7f1cc42e334e7502eaee3c8) )
	ROM_LOAD( "32j8-0.32",  0x800000, 0x200000, CRC(201bebf6) SHA1(c89d2895ea5b19daea1f88542419f4e10f437c73) )

	ROM_REGION( 0x300000, "gfx3", ROMREGION_DISPOSE ) /* BG Tiles */
	ROM_LOAD( "32j1-0.4",  0x000000, 0x100000, CRC(f1cca740) SHA1(339079b95ca137e66b4f032ad67a0adf58cca100) )
	ROM_LOAD( "32j2-0.5",  0x100000, 0x100000, CRC(5fac3e01) SHA1(20c30f4c76e303285ae37e596afe86aa4812c3b9) )
	ROM_LOAD( "32j3-0.6",  0x200000, 0x100000, CRC(d297925e) SHA1(5bc4d37bf0dc54114884c816b94a64ef1ccfeda5) )

	ROM_REGION( 0x080000, "oki", 0 ) /* Samples */
	ROM_LOAD( "32j9-0.76",  0x000000, 0x080000, CRC(16001e81) SHA1(67928d2024f963aee91f1498b6f4c76101d2f3b8) )
ROM_END

ROM_START( shadfrcj )
	ROM_REGION( 0x100000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "32j12-01.ic34", 0x00001, 0x40000, CRC(38fdbe1d) SHA1(476d8ef2c0d2a8c568ce44631f93f8c730f91b08) )
	ROM_LOAD16_BYTE( "32j13-01.ic26", 0x00000, 0x40000, CRC(6e1df6f1) SHA1(c165553fe967b437413dd7ddc87a267548dd0ca9) )
	ROM_LOAD16_BYTE( "32j14-01.ic33", 0x80001, 0x40000, CRC(89e3fb60) SHA1(90de38558d63215a0079079030e8b1097599c9e5) )
	ROM_LOAD16_BYTE( "32j15-01.ic14", 0x80000, 0x40000, CRC(3dc3a84a) SHA1(166ad91b93192d94e3f6d2fe6dde02f59d334f75) )

	ROM_REGION( 0x10000, "audio", 0 ) /* Z80 Code */
	ROM_LOAD( "32j10-0.42",  0x00000, 0x10000, CRC(65daf475) SHA1(7144332b2d17af8645e22e1926b33113db0d20e2) )

	ROM_REGION( 0x020000, "gfx1", ROMREGION_DISPOSE ) /* Chars */
	ROM_LOAD( "32j11-0.ic55",  0x00000, 0x20000, CRC(7252d993) SHA1(43f7de381841039aa290486aafb98e2cf3b8579b) )

	ROM_REGION( 0xa00000, "gfx2", ROMREGION_DISPOSE ) /* Sprite Tiles */
	ROM_LOAD( "32j4-0.12",  0x000000, 0x200000, CRC(1ebea5b6) SHA1(35bd49dda9ad75326d45ffb10c87d83fc4f1b7a8) )
	ROM_LOAD( "32j5-0.13",  0x200000, 0x200000, CRC(600026b5) SHA1(5641246300d7e20dcff1eae004647faaee6cd1c6) )
	ROM_LOAD( "32j6-0.24",  0x400000, 0x200000, CRC(6cde8ebe) SHA1(750933798235951fe24b2e667c33f692612c0aa0) )
	ROM_LOAD( "32j7-0.25",  0x600000, 0x200000, CRC(bcb37922) SHA1(f3eee73c8b9f4873a7f1cc42e334e7502eaee3c8) )
	ROM_LOAD( "32j8-0.32",  0x800000, 0x200000, CRC(201bebf6) SHA1(c89d2895ea5b19daea1f88542419f4e10f437c73) )

	ROM_REGION( 0x300000, "gfx3", ROMREGION_DISPOSE ) /* BG Tiles */
	ROM_LOAD( "32j1-0.4",  0x000000, 0x100000, CRC(f1cca740) SHA1(339079b95ca137e66b4f032ad67a0adf58cca100) )
	ROM_LOAD( "32j2-0.5",  0x100000, 0x100000, CRC(5fac3e01) SHA1(20c30f4c76e303285ae37e596afe86aa4812c3b9) )
	ROM_LOAD( "32j3-0.6",  0x200000, 0x100000, CRC(d297925e) SHA1(5bc4d37bf0dc54114884c816b94a64ef1ccfeda5) )

	ROM_REGION( 0x080000, "oki", 0 ) /* Samples */
	ROM_LOAD( "32j9-0.76",  0x000000, 0x080000, CRC(16001e81) SHA1(67928d2024f963aee91f1498b6f4c76101d2f3b8) )
ROM_END


GAME( 1993, shadfrce, 0,        shadfrce, shadfrce, 0, ROT0, "Technos Japan", "Shadow Force (US Version 2)", GAME_NO_COCKTAIL )
GAME( 1993, shadfrcj, shadfrce, shadfrce, shadfrce, 0, ROT0, "Technos Japan", "Shadow Force (Japan Version 2)", GAME_NO_COCKTAIL )
