// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

Express Raider - (c) 1986 Data East Corporation / Data East USA

Ernesto Corvi
ernesto@imagina.com

Memory Map:
Main CPU: ( DECO CPU-16 )
0000-05ff RAM
0600-07ff Sprites
0800-0bff Videoram
0c00-0fff Colorram
1800-1800 DSW 0
1801-1801 Controls
1802-1802 Coins
1803-1803 DSW 1
2100-2100 Sound latch write
2800-2801 Protection
3800-3800 VBlank ( bootleg 2 only )
4000-ffff ROM
ffc0-ffc0 VBlank ( bootleg 3 only )

Sound Cpu: ( 6809 )
0000-1fff RAM
2000-2001 YM2203
4000-4001 YM3526
6000-6000 Sound latch read
8000-ffff ROM

NOTES:
The main 6502 cpu is a custom one. The differences with a regular 6502 is as follows:
- Extra opcode ( $4b00 ), which i think reads an external port. VBlank irq is on bit 1 ( 0x02 ).
- Reset, IRQ and NMI vectors are moved.

Also, there was some protection circuitry which is now emulated.

(Note (15/jun/09): CPU is actually a DECO CPU-16, used mostly by the liberate.c games -AS)

The bootleg version patched the rom to get rid of the extra opcode ( bootlegs
used a regular 6502 ), the vectors hardcoded in place, and also had the
protection cracked.

The background tiles had a very ugly encoding. It was so ugly that our
decode gfx routine will not be able to decode it without some little help.
So thats why exprraid_gfx_expand() is there. Many thanks to Phil
Stroffolino, who figured out the encoding.

NOTES ON THE BOOTLEGS:

1st bootleg set expects to read vblank status from 0x3800, country warning
sign has been defaced by the bootleggers

2nd bootleg set expects to read vblank status from 0xFFC0, country warning
sign is intact, however Credit is spelt incorrectly.


Stephh's notes (based on the games M6502 code and some tests) :

1) 'exprrada'

  - "@ 1986 DATA EAST CORPORATION" + no code to display the Warning screen (World)
  - Same way to code number of enemies in "shoot" stages as in 'exprraidu'
    (code at 5ce4) and same ingame bug :

      5CF4: AD 03 18      lda  $1803
      5CF7: 49 FF         eor  #$FF
      5CF9: 4A            lsr  a
      5CFA: 4A            lsr  a
      5CFB: 4A            lsr  a
      5CFC: 29 06         and  #$06

    Correct code shall be :

      5CFC: 29 03         and  #$03

    You'll notice by looking at the tables that there are sometimes
    more enemies than in 'exprraidu'.
  - Time for each wagon on "shoot" stage is determined by the level
    (see code at 0x6834 where location 0x0e is level number-1).
    This time is also supposed to be determined by "Difficulty"
    settings (DSW1 bits 3 and 4).
    There is however an ingame bug that reads DSW1 bits 4 and 5 :

      683B: AD 03 18      lda  $1803
      683E: 49 FF         eor  #$FF
      6840: 4A            lsr  a
      6841: 4A            lsr  a
      6842: 4A            lsr  a
      6843: 4A            lsr  a
      6844: 29 03         and  #$03

    So Time is also determined by "Demo Sound" setting because of
    extra "lsr a" instruction at 0x6843 !
    Correct code shall be :

      6843: EA            nop

    Fortunately, table at 0x685f is filled with 0x30 so you'll
    always have 30 seconds to "clear" the wagon (which is more
    than the time you have in 'exprraid').
    For the locomotive, time is always set to 0x20 = 20 seconds
    (which is again more than the time you have in 'exprraid').
  - "Bonus lives" routine starts at 0xe49b.
  - Coinage related stuff starts at 0xe78e.
    Coinage tables :
      * 0xe7dc : COIN1 - 0xe7e4 : COIN2 (Mode 1)
      * 0xe7ec : COIN1 - 0xe7f4 : COIN2 (Mode 2)
  - At the beginning of each level, you have text in lower case
    which doesn't give you any hints to pass the level nor advice.
  - In this version, you always have 5 wagons for the "shoot" stages.
  - Continue play is always available but score is reset to 0.

2) 'exprraidu'

  - "@ 1986 DATA EAST USA, INC." (US)
  - Number of enemies on "shoot" stages is determined by the level
    (see code at 0x5d21 where location 0x0e is level number-1).
    Note that time tables are coded backwards (locomotive first,
    then 5th wagon, then 4th wagon ... up to 1st wagon).
    This number of enemies is also supposed to be determined
    by "Difficulty" settings (DSW1 bits 3 and 4).
    There is however an ingame bug that reads DSW1 bits 4 and 5 :

      5D2F: AD 03 18      lda  $1803
      5D32: 49 FF         eor  #$FF
      5D34: 4A            lsr  a
      5D35: 4A            lsr  a
      5D36: 4A            lsr  a
      5D37: 29 06         and  #$06

    So number of enemies is also determined by "Demo Sound" setting !
    Correct code shall be :

      5D37: 29 03         and  #$03

  - Time for each wagon on "shoot" stage is determined by the level
    (see code at 0x6873 where location 0x0e is level number-1).
    Note that time tables are coded backwards (locomotive first,
    then 5th wagon, then 4th wagon ... up to 1st wagon).
  - In the US manual, "bonus lives" settings are told be either
    "Every 50000" or "50000/80000".
    However, when you look at code at 0xe4a1, you'll notice that
    settings shall be "50000 only" and "50000/80000".
  - "Coin Mode" as well "Mode 2 Coinage" settings (DSW0 bits 0 to 4)
    are undocumented in the US manual.
    "Coin Mode" is tested though via code at 0xe7c5.
    Coinage tables :
      * 0xe7e2 : COIN1 - 0xe7ea : COIN2 (Mode 1)
      * 0xe7f2 : COIN1 - 0xe7fa : COIN2 (Mode 2)
  - "Force Coinage" (DSW1 bit 6) setting is undocumented in the US manual.
    It is tested though via code at 0xe794.
    When this Dip Switch is set to "On", pressing COIN1 or COIN2 always
    adds 1 credit regardless of the "Coinage" and "Coin Mode" settings.
  - At the beginning of each level, you have text in upper case
    which gives you some hints to pass the level or some advice.
  - In this version, due to extra code at 0xfd80, you only have 4 wagons
    for the "shoot" stages instead of 5.
  - Continue play is always available and score is NOT reset to 0.

3) 'wexpress'

  - "@ 1986 DATA EAST CORPORATION" + extra code to display the Warning screen (Japan)
  - This version is heavily based on 'exprrad'
    so all comments also fit for this set. The main difference is
    The other difference is that you can NOT continue a game.
  - "Bonus lives" routine starts at 0xe4e5.
  - Coinage related stuff starts at 0xe7d8.
  - Coinage tables :
      * 0xe826 : COIN1 - 0xe82e : COIN2 (Mode 1)
      * 0xe836 : COIN1 - 0xe83e : COIN2 (Mode 2)

4) 'wexpressb1'

  - "@ 1986 DATA EAST CORPORATION" + no code to display the Warning screen (World)
  - This version is based on 'exprrad' so all comments also fit
    for this set. The main difference is that reads from 0x2800
    and 0x2801 (protection) are either discarded (jumps are noped
    or patched) or changed to read what shall be the correct value
    (reads from 0x2801 occur almost all the time).
    So IMO this set looks like a World bootleg .

5) 'wexpressb2'

  - "@ 1986 DATA EAST CORPORATION" + extra code to display the Warning screen (Japan)
  - Modified Warning screen
  - This version is based on 'wexpress'
    so all comments also fit for this set. The main difference is
    the way protection is bypassed (in a different way than 'wexpressb1'
    as reads from 0x2801 only occur when a life is lost).

6) 'wexpressb3'

  - "@ 1986 DATA EAST CORPORATION" + extra code to display the Warning screen (Japan)
  - Original Warning screen
  - "CREDIT" misspelled to "CRDDIT".
  - This version is based on 'wexpress'
    so all comments also fit for this set. The main difference is
    the way protection is bypassed (in a different way than 'wexpressb1'
    but also in a different way than 'wexpressb2' as reads from 0x2801
    occur when you lose a life but also on "shoot" stages).


***************************************************************************/

#include "emu.h"
#include "cpu/m6502/deco16.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6809/m6809.h"
#include "sound/2203intf.h"
#include "sound/3526intf.h"
#include "includes/exprraid.h"


/*****************************************************************************************/
/* Emulate DECO 291 protection (for original express raider, code is cracked on the bootleg)*/
/*****************************************************************************************/

READ8_MEMBER(exprraid_state::exprraid_prot_data_r)
{
	return m_prot_value;
}

READ8_MEMBER(exprraid_state::exprraid_prot_status_r)
{
	/*
	    76543210
	    .......x    ?
	    ......x.    Device data available
	    .....x..    CPU data available (cleared by device)
	*/

	return 0x02;
}

WRITE8_MEMBER(exprraid_state::exprraid_prot_data_w)
{
	switch (data)
	{
		case 0x20:
			// Written when CPU times out waiting for status
			break;

		case 0x60:
			// ?
			break;

		case 0x80:
			++m_prot_value;
			break;

		case 0x90:
			m_prot_value = 0;
			break;

		default:
			logerror("Unknown protection write: %x at PC:%x\n", data, space.device().safe_pc());
	}
}

READ8_MEMBER(exprraid_state::sound_cpu_command_r)
{
	m_slave->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	return soundlatch_byte_r(space, 0);
}

WRITE8_MEMBER(exprraid_state::sound_cpu_command_w)
{
	soundlatch_byte_w(space, 0, data);
	m_slave->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

WRITE8_MEMBER(exprraid_state::exprraid_int_clear_w)
{
	m_maincpu->set_input_line(DECO16_IRQ_LINE, CLEAR_LINE);
}

READ8_MEMBER(exprraid_state::vblank_r)
{
	return ioport("IN0")->read();
}


static ADDRESS_MAP_START( master_map, AS_PROGRAM, 8, exprraid_state )
	AM_RANGE(0x0000, 0x05ff) AM_RAM AM_SHARE("main_ram")
	AM_RANGE(0x0600, 0x07ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x0800, 0x0bff) AM_RAM_WRITE(exprraid_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x0c00, 0x0fff) AM_RAM_WRITE(exprraid_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x1800, 0x1800) AM_READ_PORT("DSW0")   /* DSW 0 */
	AM_RANGE(0x1801, 0x1801) AM_READ_PORT("IN1")    /* Controls */
	AM_RANGE(0x1802, 0x1802) AM_READ_PORT("IN2")    /* Coins */
	AM_RANGE(0x1803, 0x1803) AM_READ_PORT("DSW1")   /* DSW 1 */
	AM_RANGE(0x2000, 0x2000) AM_WRITE(exprraid_int_clear_w)
	AM_RANGE(0x2001, 0x2001) AM_WRITE(sound_cpu_command_w)
	AM_RANGE(0x2002, 0x2002) AM_WRITE(exprraid_flipscreen_w)
	AM_RANGE(0x2003, 0x2003) AM_WRITENOP // DMA SWAP - Allow writes to video and sprite RAM
	AM_RANGE(0x2800, 0x2800) AM_READ(exprraid_prot_data_r)
	AM_RANGE(0x2801, 0x2801) AM_READ(exprraid_prot_status_r)
	AM_RANGE(0x2800, 0x2803) AM_WRITE(exprraid_bgselect_w)
	AM_RANGE(0x2804, 0x2804) AM_WRITE(exprraid_scrolly_w)
	AM_RANGE(0x2805, 0x2806) AM_WRITE(exprraid_scrollx_w)
	AM_RANGE(0x2807, 0x2807) AM_WRITE(exprraid_prot_data_w)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( master_io_map, AS_IO, 8, exprraid_state )
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN0")
ADDRESS_MAP_END

static ADDRESS_MAP_START( slave_map, AS_PROGRAM, 8, exprraid_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_DEVREADWRITE("ym1", ym2203_device, read, write)
	AM_RANGE(0x4000, 0x4001) AM_DEVREADWRITE("ym2", ym3526_device, read, write)
	AM_RANGE(0x6000, 0x6000) AM_READ(sound_cpu_command_r)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


INPUT_CHANGED_MEMBER(exprraid_state::coin_inserted_deco16)
{
	if (oldval && !newval)
		m_maincpu->set_input_line(DECO16_IRQ_LINE, ASSERT_LINE);
}

INPUT_CHANGED_MEMBER(exprraid_state::coin_inserted_nmi)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, oldval ? ASSERT_LINE : CLEAR_LINE);
}

static INPUT_PORTS_START( exprraid )
	PORT_START("IN0")   /* 0x3800 */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW0")  /* 0x1800 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSW0",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSW0",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSW0",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSW0",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) ) PORT_CONDITION("DSW0",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) ) PORT_CONDITION("DSW0",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) ) PORT_CONDITION("DSW0",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) ) PORT_CONDITION("DSW0",0x10,EQUALS,0x00)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) ) PORT_CONDITION("DSW0",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) ) PORT_CONDITION("DSW0",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSW0",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSW0",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSW0",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSW0",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSW0",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) ) PORT_CONDITION("DSW0",0x10,EQUALS,0x10)
	PORT_DIPNAME( 0x10, 0x10, "Coin Mode" )                 PORT_DIPLOCATION("SW1:5")     /* see notes */
	PORT_DIPSETTING(    0x10, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW1:8" )

	PORT_START("IN1")   /* 0x1801 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")   /* 0x1802 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, exprraid_state, coin_inserted_deco16, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, exprraid_state, coin_inserted_deco16, 0)

	PORT_START("DSW1")  /* 0x1803 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:3")     /* see notes */
	PORT_DIPSETTING(    0x00, "50k 80k" )
	PORT_DIPSETTING(    0x04, "50k only" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:4,5")   /* see notes */
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:6")     /* see notes */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, exprraid_state, coin_inserted_deco16, 0)
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( exprboot )
	PORT_INCLUDE( exprraid )
	PORT_MODIFY("IN2")  /* 0x1802 */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, exprraid_state, coin_inserted_nmi, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, exprraid_state, coin_inserted_nmi, 0)
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	1024,   /* 1024 characters */
	2,  /* 2 bits per pixel */
	{ 0, 4 },   /* the bitplanes are packed in the same byte */
	{ (0x2000*8)+0, (0x2000*8)+1, (0x2000*8)+2, (0x2000*8)+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	2048,   /* 2048 sprites */
	3,  /* 3 bits per pixel */
	{ 2*2048*32*8, 2048*32*8, 0 },  /* the bitplanes are separated */
	{ 128+0, 128+1, 128+2, 128+3, 128+4, 128+5, 128+6, 128+7, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    /* every char takes 32 consecutive bytes */
};

static const gfx_layout tile1 =
{
	16,16,  /* 16*16 tiles */
	128,    /* 128 tiles */
	3,  /* 3 bits per pixel */
	{ 4, 0x10000*8+0, 0x10000*8+4 },
	{ 0, 1, 2, 3, 1024*32*2,1024*32*2+1,1024*32*2+2,1024*32*2+3,
		128+0,128+1,128+2,128+3,128+1024*32*2,128+1024*32*2+1,128+1024*32*2+2,128+1024*32*2+3 }, /* BOGUS */
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,
		64+0*8,64+1*8,64+2*8,64+3*8,64+4*8,64+5*8,64+6*8,64+7*8 },
	32*8
};

static const gfx_layout tile2 =
{
	16,16,  /* 16*16 tiles */
	128,    /* 128 tiles */
	3,  /* 3 bits per pixel */
	{ 0, 0x11000*8+0, 0x11000*8+4  },
	{ 0, 1, 2, 3, 1024*32*2,1024*32*2+1,1024*32*2+2,1024*32*2+3,
		128+0,128+1,128+2,128+3,128+1024*32*2,128+1024*32*2+1,128+1024*32*2+2,128+1024*32*2+3 }, /* BOGUS */
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,
		64+0*8,64+1*8,64+2*8,64+3*8,64+4*8,64+5*8,64+6*8,64+7*8 },
	32*8
};


static GFXDECODE_START( exprraid )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout,   128, 2 ) /* characters */
	GFXDECODE_ENTRY( "gfx2", 0x00000, spritelayout,  64, 8 ) /* sprites */
	GFXDECODE_ENTRY( "gfx3", 0x00000, tile1,          0, 4 ) /* background tiles */
	GFXDECODE_ENTRY( "gfx3", 0x00000, tile2,          0, 4 )
	GFXDECODE_ENTRY( "gfx3", 0x04000, tile1,          0, 4 )
	GFXDECODE_ENTRY( "gfx3", 0x04000, tile2,          0, 4 )
	GFXDECODE_ENTRY( "gfx3", 0x08000, tile1,          0, 4 )
	GFXDECODE_ENTRY( "gfx3", 0x08000, tile2,          0, 4 )
	GFXDECODE_ENTRY( "gfx3", 0x0c000, tile1,          0, 4 )
	GFXDECODE_ENTRY( "gfx3", 0x0c000, tile2,          0, 4 )
GFXDECODE_END


/* handler called by the 3812 emulator when the internal timers cause an IRQ */
WRITE_LINE_MEMBER(exprraid_state::irqhandler)
{
	m_slave->set_input_line_and_vector(0, state, 0xff);
}

void exprraid_state::machine_start()
{
	save_item(NAME(m_prot_value));
	save_item(NAME(m_bg_index));
}

void exprraid_state::machine_reset()
{
	m_bg_index[0] = 0;
	m_bg_index[1] = 0;
	m_bg_index[2] = 0;
	m_bg_index[3] = 0;
}


static MACHINE_CONFIG_START( exprraid, exprraid_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", DECO16, XTAL_12MHz / 8)
	MCFG_CPU_PROGRAM_MAP(master_map)
	MCFG_CPU_IO_MAP(master_io_map)

	MCFG_CPU_ADD("slave", M6809, XTAL_12MHz / 8)
	MCFG_CPU_PROGRAM_MAP(slave_map)
	/* IRQs are caused by the YM3526 */

	MCFG_QUANTUM_TIME(attotime::from_hz(12000))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
//  MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
//  MCFG_SCREEN_SIZE(32*8, 32*8)
//  MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_RAW_PARAMS(XTAL_12MHz/2, 384, 0, 256, 262, 8, 256-8) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(exprraid_state, screen_update_exprraid)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", exprraid)
	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 256)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, XTAL_12MHz / 8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("ym2", YM3526, XTAL_12MHz / 4)
	MCFG_YM3526_IRQ_HANDLER(WRITELINE(exprraid_state, irqhandler))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( exprboot, exprraid )

	MCFG_CPU_REPLACE("maincpu", M6502, 1500000)        /* 1.5 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(master_map)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( exprraid )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cz01-2e.16b", 0x4000, 0x4000, CRC(a0ae6756) SHA1(7f7ec1efddbb62e9d201c6013bca8ab72c3f75f6) )
	ROM_LOAD( "cz00-4e.15a", 0x8000, 0x8000, CRC(910f6ccc) SHA1(1dbf164a7add9335d90ee07b6db9a162a28e407b) )

	ROM_REGION( 0x10000, "slave", 0 )   /* 64k for the sub cpu */
	ROM_LOAD( "cz02-1.2a", 0x8000, 0x8000, CRC(552e6112) SHA1(f8412a63cab0aa47321d602f69bf534426c6aa5d) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "cz07.5b", 0x00000, 0x4000, CRC(686bac23) SHA1(b6c96ed40e90a8ba32c2e78a65f9589d387b0254) )    /* characters */

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "cz09.16h", 0x00000, 0x8000, CRC(1ed250d1) SHA1(c98b0440e4319308e683e857bbfeb6a150c76ff3) )   /* sprites */
	ROM_LOAD( "cz08.14h", 0x08000, 0x8000, CRC(2293fc61) SHA1(bf81db375f5424396559dcf0e04d34a52f6a020a) )
	ROM_LOAD( "cz13.16k", 0x10000, 0x8000, CRC(7c3bfd00) SHA1(87b48e09aaeacf78f3260df893b0922e25d10a5d) )
	ROM_LOAD( "cz12.14k", 0x18000, 0x8000, CRC(ea2294c8) SHA1(bc996351921e68e6237cee2d29fee882931ce0ea) )
	ROM_LOAD( "cz11.13k", 0x20000, 0x8000, CRC(b7418335) SHA1(e9d08ee651b9221c371e2629a757bceca7b6192b) )
	ROM_LOAD( "cz10.11k", 0x28000, 0x8000, CRC(2f611978) SHA1(fb60be573184d2af1dfdd543e68eeec53f2788f2) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "cz04.8e", 0x00000, 0x8000, CRC(643a1bd3) SHA1(b23631d96cb413808f65f3ebe8fe6539b6140606) )    /* tiles */
	/* Save 0x08000-0x0ffff to expand the previous so we can decode the thing */
	ROM_LOAD( "cz05.8f", 0x10000, 0x8000, CRC(c44570bf) SHA1(3e9b8b6b36c7f5ae016dba3987ea19a29bd5ee5b) )    /* tiles */
	ROM_LOAD( "cz06.8h", 0x18000, 0x8000, CRC(b9bb448b) SHA1(84974b1f3a5b58cd427d874f805a6dd9244c1101) )    /* tiles */

	ROM_REGION( 0x8000, "gfx4", 0 )     /* background tilemaps */
	ROM_LOAD( "cz03.12d", 0x0000, 0x8000, CRC(6ce11971) SHA1(16bfa69b3ad02253e81c8110c9b840be03952790) )

	ROM_REGION( 0x0400, "proms", 0 ) /* All 4 proms are Fujitsu MB7114 or compatible */
	ROM_LOAD( "cy-17.5b", 0x0000, 0x0100, CRC(da31dfbc) SHA1(ac476440864f538918f7bef2e1db82fd19195f89) ) /* red */
	ROM_LOAD( "cy-16.6b", 0x0100, 0x0100, CRC(51f25b4c) SHA1(bfcca57613fbb22919e00db1f6a8c7ca50faa60b) ) /* green */
	ROM_LOAD( "cy-15.7b", 0x0200, 0x0100, CRC(a6168d7f) SHA1(0c7b31adcd764ce2631c3fb5c1a968b01f65e741) ) /* blue */
	ROM_LOAD( "cy-14.9b", 0x0300, 0x0100, CRC(52aad300) SHA1(ff09772b930afa87e28d0628ef85a589a3d149c9) ) /* ??? */

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "pal16r4a.5c", 0x0000, 0x0104, CRC(d66aaa87) SHA1(dc29b473238ed6a9de2076c79644b613a9ba6924) )
	ROM_LOAD( "pal16r4a.5e", 0x0200, 0x0104, CRC(9a8766a7) SHA1(5f84ad9e633daeb14531ef527827ef3d9b269437) )
ROM_END

ROM_START( exprraidu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cz01-5a.16b", 0x4000, 0x4000, CRC(dc8f9fba) SHA1(cae6af54fc0081d606b6884e8873aed356a37ba9) )
	ROM_LOAD( "cz00-5.15a",  0x8000, 0x8000, CRC(a81290bc) SHA1(ddb0acda6124427bee691f9926c41fda27ed816e) )

	ROM_REGION( 0x10000, "slave", 0 )   /* 64k for the sub cpu */
	ROM_LOAD( "cz02-1.2a", 0x8000, 0x8000, CRC(552e6112) SHA1(f8412a63cab0aa47321d602f69bf534426c6aa5d) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "cz07.5b", 0x00000, 0x4000, CRC(686bac23) SHA1(b6c96ed40e90a8ba32c2e78a65f9589d387b0254) )    /* characters */

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "cz09.16h", 0x00000, 0x8000, CRC(1ed250d1) SHA1(c98b0440e4319308e683e857bbfeb6a150c76ff3) )   /* sprites */
	ROM_LOAD( "cz08.14h", 0x08000, 0x8000, CRC(2293fc61) SHA1(bf81db375f5424396559dcf0e04d34a52f6a020a) )
	ROM_LOAD( "cz13.16k", 0x10000, 0x8000, CRC(7c3bfd00) SHA1(87b48e09aaeacf78f3260df893b0922e25d10a5d) )
	ROM_LOAD( "cz12.14k", 0x18000, 0x8000, CRC(ea2294c8) SHA1(bc996351921e68e6237cee2d29fee882931ce0ea) )
	ROM_LOAD( "cz11.13k", 0x20000, 0x8000, CRC(b7418335) SHA1(e9d08ee651b9221c371e2629a757bceca7b6192b) )
	ROM_LOAD( "cz10.11k", 0x28000, 0x8000, CRC(2f611978) SHA1(fb60be573184d2af1dfdd543e68eeec53f2788f2) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "cz04.8e", 0x00000, 0x8000, CRC(643a1bd3) SHA1(b23631d96cb413808f65f3ebe8fe6539b6140606) )    /* tiles */
	/* Save 0x08000-0x0ffff to expand the previous so we can decode the thing */
	ROM_LOAD( "cz05.8f", 0x10000, 0x8000, CRC(c44570bf) SHA1(3e9b8b6b36c7f5ae016dba3987ea19a29bd5ee5b) )    /* tiles */
	ROM_LOAD( "cz06.8h", 0x18000, 0x8000, CRC(b9bb448b) SHA1(84974b1f3a5b58cd427d874f805a6dd9244c1101) )    /* tiles */

	ROM_REGION( 0x8000, "gfx4", 0 )     /* background tilemaps */
	ROM_LOAD( "cz03.12d", 0x0000, 0x8000, CRC(6ce11971) SHA1(16bfa69b3ad02253e81c8110c9b840be03952790) )

	ROM_REGION( 0x0400, "proms", 0 ) /* All 4 proms are Fujitsu MB7114 or compatible */
	ROM_LOAD( "cy-17.5b", 0x0000, 0x0100, CRC(da31dfbc) SHA1(ac476440864f538918f7bef2e1db82fd19195f89) ) /* red */
	ROM_LOAD( "cy-16.6b", 0x0100, 0x0100, CRC(51f25b4c) SHA1(bfcca57613fbb22919e00db1f6a8c7ca50faa60b) ) /* green */
	ROM_LOAD( "cy-15.7b", 0x0200, 0x0100, CRC(a6168d7f) SHA1(0c7b31adcd764ce2631c3fb5c1a968b01f65e741) ) /* blue */
	ROM_LOAD( "cy-14.9b", 0x0300, 0x0100, CRC(52aad300) SHA1(ff09772b930afa87e28d0628ef85a589a3d149c9) ) /* ??? */

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "pal16r4a.5c", 0x0000, 0x0104, CRC(d66aaa87) SHA1(dc29b473238ed6a9de2076c79644b613a9ba6924) )
	ROM_LOAD( "pal16r4a.5e", 0x0200, 0x0104, CRC(9a8766a7) SHA1(5f84ad9e633daeb14531ef527827ef3d9b269437) )
ROM_END

ROM_START( exprraidi ) /* PCB manufactured in Italy by Gecas under Data East license (custom ICs are all DECO original) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cz01-2e.16b",  0x4000, 0x4000, CRC(a0ae6756) SHA1(7f7ec1efddbb62e9d201c6013bca8ab72c3f75f6) )
	ROM_LOAD( "exraidi6.15a", 0x8000, 0x8000, CRC(a3d98118) SHA1(d35f0fcabef045afcec5119f95ae6da2cae547db) )

	ROM_REGION( 0x10000, "slave", 0 )   /* 64k for the sub cpu */
	ROM_LOAD( "cz02-1.2a", 0x8000, 0x8000, CRC(552e6112) SHA1(f8412a63cab0aa47321d602f69bf534426c6aa5d) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "cz07.5b", 0x00000, 0x4000, CRC(686bac23) SHA1(b6c96ed40e90a8ba32c2e78a65f9589d387b0254) )    /* characters */

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "cz09.16h", 0x00000, 0x8000, CRC(1ed250d1) SHA1(c98b0440e4319308e683e857bbfeb6a150c76ff3) )   /* sprites */
	ROM_LOAD( "cz08.14h", 0x08000, 0x8000, CRC(2293fc61) SHA1(bf81db375f5424396559dcf0e04d34a52f6a020a) )
	ROM_LOAD( "cz13.16k", 0x10000, 0x8000, CRC(7c3bfd00) SHA1(87b48e09aaeacf78f3260df893b0922e25d10a5d) )
	ROM_LOAD( "cz12.14k", 0x18000, 0x8000, CRC(ea2294c8) SHA1(bc996351921e68e6237cee2d29fee882931ce0ea) )
	ROM_LOAD( "cz11.13k", 0x20000, 0x8000, CRC(b7418335) SHA1(e9d08ee651b9221c371e2629a757bceca7b6192b) )
	ROM_LOAD( "cz10.11k", 0x28000, 0x8000, CRC(2f611978) SHA1(fb60be573184d2af1dfdd543e68eeec53f2788f2) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "cz04.8e", 0x00000, 0x8000, CRC(643a1bd3) SHA1(b23631d96cb413808f65f3ebe8fe6539b6140606) )    /* tiles */
	/* Save 0x08000-0x0ffff to expand the previous so we can decode the thing */
	ROM_LOAD( "cz05.8f", 0x10000, 0x8000, CRC(c44570bf) SHA1(3e9b8b6b36c7f5ae016dba3987ea19a29bd5ee5b) )    /* tiles */
	ROM_LOAD( "cz06.8h", 0x18000, 0x8000, CRC(b9bb448b) SHA1(84974b1f3a5b58cd427d874f805a6dd9244c1101) )    /* tiles */

	ROM_REGION( 0x8000, "gfx4", 0 )     /* background tilemaps */
	ROM_LOAD( "cz03.12d", 0x0000, 0x8000, CRC(6ce11971) SHA1(16bfa69b3ad02253e81c8110c9b840be03952790) )

	ROM_REGION( 0x0400, "proms", 0 ) /* All 4 proms are Fujitsu MB7114 or compatible */
	ROM_LOAD( "cy-17.5b", 0x0000, 0x0100, CRC(da31dfbc) SHA1(ac476440864f538918f7bef2e1db82fd19195f89) ) /* red */
	ROM_LOAD( "cy-16.6b", 0x0100, 0x0100, CRC(51f25b4c) SHA1(bfcca57613fbb22919e00db1f6a8c7ca50faa60b) ) /* green */
	ROM_LOAD( "cy-15.7b", 0x0200, 0x0100, CRC(a6168d7f) SHA1(0c7b31adcd764ce2631c3fb5c1a968b01f65e741) ) /* blue */
	ROM_LOAD( "cy-14.9b", 0x0300, 0x0100, CRC(52aad300) SHA1(ff09772b930afa87e28d0628ef85a589a3d149c9) ) /* ??? */

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "pal16r4a.5c", 0x0000, 0x0104, CRC(d66aaa87) SHA1(dc29b473238ed6a9de2076c79644b613a9ba6924) )
	ROM_LOAD( "pal16r4a.5e", 0x0200, 0x0104, CRC(9a8766a7) SHA1(5f84ad9e633daeb14531ef527827ef3d9b269437) )
ROM_END

ROM_START( wexpress )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cy01-2.16b", 0x4000, 0x4000, CRC(a0ae6756) SHA1(7f7ec1efddbb62e9d201c6013bca8ab72c3f75f6) )
	ROM_LOAD( "cy00-4.15a", 0x8000, 0x8000, CRC(c66d4dd3) SHA1(3c354e7379b3c3e709039ee2f3dbad7edddfc517) )

	ROM_REGION( 0x10000, "slave", 0 )   /* 64k for the sub cpu */
	ROM_LOAD( "cy02-1.2a", 0x8000, 0x8000, CRC(552e6112) SHA1(f8412a63cab0aa47321d602f69bf534426c6aa5d) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "cz07.5b", 0x00000, 0x4000, CRC(686bac23) SHA1(b6c96ed40e90a8ba32c2e78a65f9589d387b0254) )    /* characters */

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "cz09.16h", 0x00000, 0x8000, CRC(1ed250d1) SHA1(c98b0440e4319308e683e857bbfeb6a150c76ff3) )   /* sprites */
	ROM_LOAD( "cz08.14h", 0x08000, 0x8000, CRC(2293fc61) SHA1(bf81db375f5424396559dcf0e04d34a52f6a020a) )
	ROM_LOAD( "cz13.16k", 0x10000, 0x8000, CRC(7c3bfd00) SHA1(87b48e09aaeacf78f3260df893b0922e25d10a5d) )
	ROM_LOAD( "cz12.14k", 0x18000, 0x8000, CRC(ea2294c8) SHA1(bc996351921e68e6237cee2d29fee882931ce0ea) )
	ROM_LOAD( "cz11.13k", 0x20000, 0x8000, CRC(b7418335) SHA1(e9d08ee651b9221c371e2629a757bceca7b6192b) )
	ROM_LOAD( "cz10.11k", 0x28000, 0x8000, CRC(2f611978) SHA1(fb60be573184d2af1dfdd543e68eeec53f2788f2) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "cy04.8e", 0x00000, 0x8000, CRC(f2e93ff0) SHA1(2e631966e1fa0b2699aa782b589d36801072ba03) )    /* tiles */
	/* Save 0x08000-0x0ffff to expand the previous so we can decode the thing */
	ROM_LOAD( "cy05.8f", 0x10000, 0x8000, CRC(c44570bf) SHA1(3e9b8b6b36c7f5ae016dba3987ea19a29bd5ee5b) )    /* tiles */
	ROM_LOAD( "cy06.8h", 0x18000, 0x8000, CRC(c3a56de5) SHA1(aefc516c6c69b12291c0bda03729910181a91a17) )    /* tiles */

	ROM_REGION( 0x8000, "gfx4", 0 )     /* background tilemaps */
	ROM_LOAD( "cy03.12d", 0x0000, 0x8000, CRC(242e3e64) SHA1(4fa8e93ef055bfdbe3bd619c53bf2448e1b832f0) )

	ROM_REGION( 0x0400, "proms", 0 ) /* All 4 proms are Fujitsu MB7114 or compatible */
	ROM_LOAD( "cy-17.5b", 0x0000, 0x0100, CRC(da31dfbc) SHA1(ac476440864f538918f7bef2e1db82fd19195f89) ) /* red */
	ROM_LOAD( "cy-16.6b", 0x0100, 0x0100, CRC(51f25b4c) SHA1(bfcca57613fbb22919e00db1f6a8c7ca50faa60b) ) /* green */
	ROM_LOAD( "cy-15.7b", 0x0200, 0x0100, CRC(a6168d7f) SHA1(0c7b31adcd764ce2631c3fb5c1a968b01f65e741) ) /* blue */
	ROM_LOAD( "cy-14.9b", 0x0300, 0x0100, CRC(52aad300) SHA1(ff09772b930afa87e28d0628ef85a589a3d149c9) ) /* ??? */

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "pal16r4a.5c",   0x0000, 0x0104, CRC(d66aaa87) SHA1(dc29b473238ed6a9de2076c79644b613a9ba6924) )
	ROM_LOAD( "pal16r4a.5e",   0x0200, 0x0104, CRC(9a8766a7) SHA1(5f84ad9e633daeb14531ef527827ef3d9b269437) )
ROM_END

ROM_START( wexpressb1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2.16b", 0x4000, 0x4000, CRC(ea5e5a8f) SHA1(fa92bcb6b97c2966cd330b309eba73f9c059f14e) )
	ROM_LOAD( "1.15a", 0x8000, 0x8000, CRC(a7daae12) SHA1(a97f4bc05a3ec096d8c717bdf096f4b0e59dc2c2) )

	ROM_REGION( 0x10000, "slave", 0 )   /* 64k for the sub cpu */
	ROM_LOAD( "cy02-1.2a", 0x8000, 0x8000, CRC(552e6112) SHA1(f8412a63cab0aa47321d602f69bf534426c6aa5d) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "cz07.5b", 0x00000, 0x4000, CRC(686bac23) SHA1(b6c96ed40e90a8ba32c2e78a65f9589d387b0254) )    /* characters */

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "cz09.16h", 0x00000, 0x8000, CRC(1ed250d1) SHA1(c98b0440e4319308e683e857bbfeb6a150c76ff3) )   /* sprites */
	ROM_LOAD( "cz08.14h", 0x08000, 0x8000, CRC(2293fc61) SHA1(bf81db375f5424396559dcf0e04d34a52f6a020a) )
	ROM_LOAD( "cz13.16k", 0x10000, 0x8000, CRC(7c3bfd00) SHA1(87b48e09aaeacf78f3260df893b0922e25d10a5d) )
	ROM_LOAD( "cz12.14k", 0x18000, 0x8000, CRC(ea2294c8) SHA1(bc996351921e68e6237cee2d29fee882931ce0ea) )
	ROM_LOAD( "cz11.13k", 0x20000, 0x8000, CRC(b7418335) SHA1(e9d08ee651b9221c371e2629a757bceca7b6192b) )
	ROM_LOAD( "cz10.11k", 0x28000, 0x8000, CRC(2f611978) SHA1(fb60be573184d2af1dfdd543e68eeec53f2788f2) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "cy04.8e", 0x00000, 0x8000, CRC(f2e93ff0) SHA1(2e631966e1fa0b2699aa782b589d36801072ba03) )    /* tiles */
	/* Save 0x08000-0x0ffff to expand the previous so we can decode the thing */
	ROM_LOAD( "cy05.8f", 0x10000, 0x8000, CRC(c44570bf) SHA1(3e9b8b6b36c7f5ae016dba3987ea19a29bd5ee5b) )    /* tiles */
	ROM_LOAD( "cy06.8h", 0x18000, 0x8000, CRC(c3a56de5) SHA1(aefc516c6c69b12291c0bda03729910181a91a17) )    /* tiles */

	ROM_REGION( 0x8000, "gfx4", 0 )     /* background tilemaps */
	ROM_LOAD( "cy03.12d", 0x0000, 0x8000, CRC(242e3e64) SHA1(4fa8e93ef055bfdbe3bd619c53bf2448e1b832f0) )

	ROM_REGION( 0x0400, "proms", 0 ) /* All 4 proms are Fujitsu MB7114 or compatible */
	ROM_LOAD( "cy-17.5b", 0x0000, 0x0100, CRC(da31dfbc) SHA1(ac476440864f538918f7bef2e1db82fd19195f89) ) /* red */
	ROM_LOAD( "cy-16.6b", 0x0100, 0x0100, CRC(51f25b4c) SHA1(bfcca57613fbb22919e00db1f6a8c7ca50faa60b) ) /* green */
	ROM_LOAD( "cy-15.7b", 0x0200, 0x0100, CRC(a6168d7f) SHA1(0c7b31adcd764ce2631c3fb5c1a968b01f65e741) ) /* blue */
	ROM_LOAD( "cy-14.9b", 0x0300, 0x0100, CRC(52aad300) SHA1(ff09772b930afa87e28d0628ef85a589a3d149c9) ) /* ??? */

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "pal16r4a.5c",   0x0000, 0x0104, CRC(d66aaa87) SHA1(dc29b473238ed6a9de2076c79644b613a9ba6924) )
	ROM_LOAD( "pal16r4a.5e",   0x0200, 0x0104, CRC(9a8766a7) SHA1(5f84ad9e633daeb14531ef527827ef3d9b269437) )
ROM_END

ROM_START( wexpressb2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wexpress.3", 0x4000, 0x4000, CRC(b4dd0fa4) SHA1(8d17eb28ae92486c67859871ea2bef8f50f39dbd) )
	ROM_LOAD( "wexpress.1", 0x8000, 0x8000, CRC(e8466596) SHA1(dbbd3b84d0f017292595fc19f7412b984851221a) )

	ROM_REGION( 0x10000, "slave", 0 )   /* 64k for the sub cpu */
	ROM_LOAD( "cy02-1.2a", 0x8000, 0x8000, CRC(552e6112) SHA1(f8412a63cab0aa47321d602f69bf534426c6aa5d) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "cz07.5b", 0x00000, 0x4000, CRC(686bac23) SHA1(b6c96ed40e90a8ba32c2e78a65f9589d387b0254) )    /* characters */

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "cz09.16h", 0x00000, 0x8000, CRC(1ed250d1) SHA1(c98b0440e4319308e683e857bbfeb6a150c76ff3) )   /* sprites */
	ROM_LOAD( "cz08.14h", 0x08000, 0x8000, CRC(2293fc61) SHA1(bf81db375f5424396559dcf0e04d34a52f6a020a) )
	ROM_LOAD( "cz13.16k", 0x10000, 0x8000, CRC(7c3bfd00) SHA1(87b48e09aaeacf78f3260df893b0922e25d10a5d) )
	ROM_LOAD( "cz12.14k", 0x18000, 0x8000, CRC(ea2294c8) SHA1(bc996351921e68e6237cee2d29fee882931ce0ea) )
	ROM_LOAD( "cz11.13k", 0x20000, 0x8000, CRC(b7418335) SHA1(e9d08ee651b9221c371e2629a757bceca7b6192b) )
	ROM_LOAD( "cz10.11k", 0x28000, 0x8000, CRC(2f611978) SHA1(fb60be573184d2af1dfdd543e68eeec53f2788f2) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "cy04.8e", 0x00000, 0x8000, CRC(f2e93ff0) SHA1(2e631966e1fa0b2699aa782b589d36801072ba03) )    /* tiles */
	/* Save 0x08000-0x0ffff to expand the previous so we can decode the thing */
	ROM_LOAD( "cy05.8f", 0x10000, 0x8000, CRC(c44570bf) SHA1(3e9b8b6b36c7f5ae016dba3987ea19a29bd5ee5b) )    /* tiles */
	ROM_LOAD( "cy06.8h", 0x18000, 0x8000, CRC(c3a56de5) SHA1(aefc516c6c69b12291c0bda03729910181a91a17) )    /* tiles */

	ROM_REGION( 0x8000, "gfx4", 0 )     /* background tilemaps */
	ROM_LOAD( "cy03.12d", 0x0000, 0x8000, CRC(242e3e64) SHA1(4fa8e93ef055bfdbe3bd619c53bf2448e1b832f0) )

	ROM_REGION( 0x0400, "proms", 0 ) /* All 4 proms are Fujitsu MB7114 or compatible */
	ROM_LOAD( "cy-17.5b", 0x0000, 0x0100, CRC(da31dfbc) SHA1(ac476440864f538918f7bef2e1db82fd19195f89) ) /* red */
	ROM_LOAD( "cy-16.6b", 0x0100, 0x0100, CRC(51f25b4c) SHA1(bfcca57613fbb22919e00db1f6a8c7ca50faa60b) ) /* green */
	ROM_LOAD( "cy-15.7b", 0x0200, 0x0100, CRC(a6168d7f) SHA1(0c7b31adcd764ce2631c3fb5c1a968b01f65e741) ) /* blue */
	ROM_LOAD( "cy-14.9b", 0x0300, 0x0100, CRC(52aad300) SHA1(ff09772b930afa87e28d0628ef85a589a3d149c9) ) /* ??? */
ROM_END

ROM_START( wexpressb3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "s2.16b", 0x4000, 0x4000, CRC(40d70fcb) SHA1(1327d39f872a39e020972952e5756ca59c55f9d0) )
	ROM_LOAD( "s1.15a", 0x8000, 0x8000, CRC(7c573824) SHA1(f5e4d4f0866c08c88d012a77e8aa2e74a779f986) )

	ROM_REGION( 0x10000, "slave", 0 )   /* 64k for the sub cpu */
	ROM_LOAD( "cy02-1.2a", 0x8000, 0x8000, CRC(552e6112) SHA1(f8412a63cab0aa47321d602f69bf534426c6aa5d) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "cz07.5b", 0x00000, 0x4000, CRC(686bac23) SHA1(b6c96ed40e90a8ba32c2e78a65f9589d387b0254) )    /* characters */

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "cz09.16h", 0x00000, 0x8000, CRC(1ed250d1) SHA1(c98b0440e4319308e683e857bbfeb6a150c76ff3) )   /* sprites */
	ROM_LOAD( "cz08.14h", 0x08000, 0x8000, CRC(2293fc61) SHA1(bf81db375f5424396559dcf0e04d34a52f6a020a) )
	ROM_LOAD( "cz13.16k", 0x10000, 0x8000, CRC(7c3bfd00) SHA1(87b48e09aaeacf78f3260df893b0922e25d10a5d) )
	ROM_LOAD( "cz12.14k", 0x18000, 0x8000, CRC(ea2294c8) SHA1(bc996351921e68e6237cee2d29fee882931ce0ea) )
	ROM_LOAD( "cz11.13k", 0x20000, 0x8000, CRC(b7418335) SHA1(e9d08ee651b9221c371e2629a757bceca7b6192b) )
	ROM_LOAD( "cz10.11k", 0x28000, 0x8000, CRC(2f611978) SHA1(fb60be573184d2af1dfdd543e68eeec53f2788f2) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "cy04.8e", 0x00000, 0x8000, CRC(f2e93ff0) SHA1(2e631966e1fa0b2699aa782b589d36801072ba03) )    /* tiles */
	/* Save 0x08000-0x0ffff to expand the previous so we can decode the thing */
	ROM_LOAD( "cy05.8f", 0x10000, 0x8000, CRC(c44570bf) SHA1(3e9b8b6b36c7f5ae016dba3987ea19a29bd5ee5b) )    /* tiles */
	ROM_LOAD( "cy06.8h", 0x18000, 0x8000, CRC(c3a56de5) SHA1(aefc516c6c69b12291c0bda03729910181a91a17) )    /* tiles */

	ROM_REGION( 0x8000, "gfx4", 0 )     /* background tilemaps */
	ROM_LOAD( "3.12d", 0x0000, 0x8000, CRC(242e3e64) SHA1(4fa8e93ef055bfdbe3bd619c53bf2448e1b832f0) )

	ROM_REGION( 0x0400, "proms", 0 ) /* Proms Weren't Present In This Set, Using the One from the Other */
	ROM_LOAD( "cy-17.5b", 0x0000, 0x0100, CRC(da31dfbc) SHA1(ac476440864f538918f7bef2e1db82fd19195f89) ) /* red */
	ROM_LOAD( "cy-16.6b", 0x0100, 0x0100, CRC(51f25b4c) SHA1(bfcca57613fbb22919e00db1f6a8c7ca50faa60b) ) /* green */
	ROM_LOAD( "cy-15.7b", 0x0200, 0x0100, CRC(a6168d7f) SHA1(0c7b31adcd764ce2631c3fb5c1a968b01f65e741) ) /* blue */
	ROM_LOAD( "cy-14.9b", 0x0300, 0x0100, CRC(52aad300) SHA1(ff09772b930afa87e28d0628ef85a589a3d149c9) ) /* ??? */
ROM_END


void exprraid_state::exprraid_gfx_expand()
{
	/* Expand the background rom so we can use regular decode routines */

	UINT8   *gfx = memregion("gfx3")->base();
	int offs = 0x10000 - 0x1000;
	int i;


	for ( i = 0x8000 - 0x1000; i >= 0; i-= 0x1000 )
	{
		memcpy(&(gfx[offs]), &(gfx[i]), 0x1000);

		offs -= 0x1000;

		memcpy(&(gfx[offs]), &(gfx[i]), 0x1000);

		offs -= 0x1000;
	}
}

DRIVER_INIT_MEMBER(exprraid_state,wexpressb)
{
	UINT8 *rom = memregion("maincpu")->base();

	/* HACK: this set uses M6502 irq vectors but DECO CPU-16 opcodes??? */
	rom[0xfff7] = rom[0xfffa];
	rom[0xfff6] = rom[0xfffb];

	rom[0xfff1] = rom[0xfffc];
	rom[0xfff0] = rom[0xfffd];

	rom[0xfff3] = rom[0xfffe];
	rom[0xfff2] = rom[0xffff];

	exprraid_gfx_expand();
}

DRIVER_INIT_MEMBER(exprraid_state,exprraid)
{
	exprraid_gfx_expand();
}

DRIVER_INIT_MEMBER(exprraid_state,wexpressb2)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x3800, 0x3800, read8_delegate(FUNC(exprraid_state::vblank_r),this));
	exprraid_gfx_expand();
}

DRIVER_INIT_MEMBER(exprraid_state,wexpressb3)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xFFC0, 0xFFC0, read8_delegate(FUNC(exprraid_state::vblank_r),this));
	exprraid_gfx_expand();
}


GAME( 1986, exprraid,  0,        exprraid, exprraid, exprraid_state, exprraid,  ROT0, "Data East Corporation", "Express Raider (World, Rev 4)",   MACHINE_SUPPORTS_SAVE )
GAME( 1986, exprraidu, exprraid, exprraid, exprraid, exprraid_state, exprraid,  ROT0, "Data East USA",         "Express Raider (US, rev 5)",      MACHINE_SUPPORTS_SAVE )
GAME( 1986, exprraidi, exprraid, exprraid, exprraid, exprraid_state, exprraid,  ROT0, "Data East Corporation", "Express Raider (Italy)",          MACHINE_SUPPORTS_SAVE )
GAME( 1986, wexpress,  exprraid, exprraid, exprraid, exprraid_state, exprraid,  ROT0, "Data East Corporation", "Western Express (Japan, rev 4)",  MACHINE_SUPPORTS_SAVE )
GAME( 1986, wexpressb1,exprraid, exprraid, exprraid, exprraid_state, wexpressb, ROT0, "bootleg",               "Western Express (bootleg set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, wexpressb2,exprraid, exprboot, exprboot, exprraid_state, wexpressb2,ROT0, "bootleg",               "Western Express (bootleg set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, wexpressb3,exprraid, exprboot, exprboot, exprraid_state, wexpressb3,ROT0, "bootleg",               "Western Express (bootleg set 3)", MACHINE_SUPPORTS_SAVE )
