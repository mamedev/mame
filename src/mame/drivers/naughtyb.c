// license:BSD-3-Clause
// copyright-holders:Brad Oliver,Sal and John Bugliarisi,Paul Priest
/***************************************************************************

Naughty Boy driver by Sal and John Bugliarisi.
This driver is based largely on MAME's Phoenix driver, since Naughty Boy runs
on similar hardware as Phoenix. Phoenix driver provided by Brad Oliver.
Thanks to Richard Davies for his Phoenix emulator source.


Naughty Boy memory map

0000-3fff 16Kb Program ROM
4000-7fff 1Kb Work RAM (mirrored)
8000-87ff 2Kb Video RAM Charset A (lower priority, mirrored)
8800-8fff 2Kb Video RAM Charset b (higher priority, mirrored)
9000-97ff 2Kb Video Control write-only (mirrored)
9800-9fff 2Kb Video Scroll Register (mirrored)
a000-a7ff 2Kb Sound Control A (mirrored)
a800-afff 2Kb Sound Control B (mirrored)
b000-b7ff 2Kb 8bit Game Control read-only (mirrored)
b800-bfff 1Kb 8bit Dip Switch read-only (mirrored)
c000-ffff 16Kb Unused

memory mapped ports:

read-only:
b000-b7ff IN
b800-bfff DSW


Naughty Boy Switch Settings
(C)1982 Cinematronics

 --------------------------------------------------------
|Option |Factory|Descrpt| 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
 ------------------------|-------------------------------
|Lives  |       |2      |on |on |   |   |   |   |   |   |
 ------------------------ -------------------------------
|       |   X   |3      |off|on |   |   |   |   |   |   |
 ------------------------ -------------------------------
|       |       |4      |on |off|   |   |   |   |   |   |
 ------------------------ -------------------------------
|       |       |5      |off|off|   |   |   |   |   |   |
 ------------------------ -------------------------------
|Extra  |       |10000  |   |   |on |on |   |   |   |   |
 ------------------------ -------------------------------
|       |   X   |30000  |   |   |off|on |   |   |   |   |
 ------------------------ -------------------------------
|       |       |50000  |   |   |on |off|   |   |   |   |
 ------------------------ -------------------------------
|       |       |70000  |   |   |off|off|   |   |   |   |
 ------------------------ -------------------------------
|Credits|       |2c, 1p |   |   |   |   |on |on |   |   |
 ------------------------ -------------------------------
|       |   X   |1c, 1p |   |   |   |   |off|on |   |   |
 ------------------------ -------------------------------
|       |       |1c, 2p |   |   |   |   |on |off|   |   |
 ------------------------ -------------------------------
|       |       |4c, 3p |   |   |   |   |off|off|   |   |
 ------------------------ -------------------------------
|Dffclty|   X   |Easier |   |   |   |   |   |   |on |   |
 ------------------------ -------------------------------
|       |       |Harder |   |   |   |   |   |   |off|   |
 ------------------------ -------------------------------
| Type  |       |Upright|   |   |   |   |   |   |   |on |
 ------------------------ -------------------------------
|       |       |Cktail |   |   |   |   |   |   |   |off|
 ------------------------ -------------------------------

*
* Pop Flamer
*

Pop Flamer appears to run on identical hardware as Naughty Boy.
The dipswitches are even identical. Spooky.

                        1   2   3   4   5   6   7   8
-------------------------------------------------------
Number of Mr. Mouse 2 |ON |ON |   |   |   |   |   |   |
                    3 |OFF|ON |   |   |   |   |   |   |
                    4 |ON |OFF|   |   |   |   |   |   |
                    5 |OFF|OFF|   |   |   |   |   |   |
-------------------------------------------------------
Extra Mouse    10,000 |   |   |ON |ON |   |   |   |   |
               30,000 |   |   |OFF|ON |   |   |   |   |
               50,000 |   |   |ON |OFF|   |   |   |   |
               70,000 |   |   |OFF|OFF|   |   |   |   |
-------------------------------------------------------
Credit  2 coin 1 play |   |   |   |   |ON |ON |   |   |
        1 coin 1 play |   |   |   |   |OFF|ON |   |   |
        1 coin 2 play |   |   |   |   |ON |OFF|   |   |
        1 coin 3 play |   |   |   |   |OFF|OFF|   |   |
-------------------------------------------------------
Skill          Easier |   |   |   |   |   |   |ON |   |
               Harder |   |   |   |   |   |   |OFF|   |
-------------------------------------------------------
Game style      Table |   |   |   |   |   |   |   |OFF|
              Upright |   |   |   |   |   |   |   |ON |


TODO:
    * sounds are a little skanky

 ***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/tms36xx.h"
#include "includes/naughtyb.h"

#define CLOCK_XTAL 12000000

READ8_MEMBER(naughtyb_state::in0_port_r)
{
	int in0 = ioport("IN0")->read();

	if ( m_cocktail )
	{
		// cabinet == cocktail -AND- handling player 2

		in0 = ( in0 & 0x03 ) |              // start buttons
				( ioport("IN0_COCKTAIL")->read() & 0xFC );  // cocktail inputs
	}

	return in0;
}

READ8_MEMBER(naughtyb_state::dsw0_port_r)
{
	// vblank replaces the cabinet dip

	return ( ( ioport("DSW0")->read() & 0x7F ) |        // dsw0
				( ioport("FAKE")->read() & 0x80 ) );        // vblank
}

/* Pop Flamer
   1st protection relies on reading values from a device at $9000 and writing to 400A-400D (See $26A9).
   Then value stored in 400C must be xxxx1001 (rrca x 3) or else reset
   2nd protection relies on the values stored in 400A-400D matching $2690+($400E) (Starts at $460)
   If the values all match then it will jump to 0x0011 instead of 0x0009 (refresh instead of reset)
   Paul Priest: tourniquet@mameworld.net */


READ8_MEMBER(naughtyb_state::popflame_protection_r)/* Not used by bootleg/hack */
{
	static const int seed00[4] = { 0x78, 0x68, 0x48, 0x38|0x80 };
	static const int seed10[4] = { 0x68, 0x60, 0x68, 0x60|0x80 };
	UINT8 seedxx;

	seedxx = (m_r_index < 0x89) ? 1 : 0;

	m_prot_count = (m_prot_count + 1) % 4;
	if(m_popflame_prot_seed == 0x10)
		return seed10[m_prot_count] | seedxx;
	else
		return seed00[m_prot_count] | seedxx;

#if 0
	if ( space.device().safe_pc() == (0x26F2 + 0x03) )
	{
		popflame_prot_count = 0;
		return 0x01;
	} /* Must not carry when rotated left */

	if ( space.device().safe_pc() == (0x26F9 + 0x03) )
		return 0x80; /* Must carry when rotated left */

	if ( space.device().safe_pc() == (0x270F + 0x03) )
	{
		switch( popflame_prot_count++ )
		{
			case 0: return 0x78; /* x111 1xxx, matches 0x0F at $2690, stored in $400A */
			case 1: return 0x68; /* x110 1xxx, matches 0x0D at $2691, stored in $400B */
			case 2: return 0x48; /* x100 1xxx, matches 0x09 at $2692, stored in $400C */
			case 3: return 0x38; /* x011 1xxx, matches 0x07 at $2693, stored in $400D */
		}
	}
	logerror("CPU #0 PC %06x: unmapped protection read\n", space.device().safe_pc());
	return 0x00;
#endif
}

WRITE8_MEMBER(naughtyb_state::popflame_protection_w)
{
	/*
	Alternative protection check is executed at the end of stage 3, it seems some kind of pseudo "EEPROM" device:
2720: 21 98 B0      ld   hl,$B098
2723: 36 01         ld   (hl),$01
2725: 0E 40         ld   c,$40
2727: 73            ld   (hl),e ;reset write index buffer
2728: 06 40         ld   b,$40
272A: 1A            ld   a,(de) ;reads the "random" data (ROM index base = 0x3000)
272B: AB            xor  e
272C: E6 02         and  $02
272E: 77            ld   (hl),a ;puts a bit there
272F: 13            inc  de
2730: 05            dec  b
2731: C2 2A 27      jp   nz,$272A ;loops for 0x40 iterations
2734: 70            ld   (hl),b
2735: 06 10         ld   b,$10
2737: 36 04         ld   (hl),$04 ;reset the read buffer index
2739: CD 6F 27      call $276F
	276F: 36 08         ld   (hl),$08
	2771: 3A 90 90      ld   a,($9090) ;reads the protection buffer, it probably rearrange the aforementioned write bits into a bit 2-1-0 packet format
	2774: E6 07         and  $07
	2776: 85            add  a,l
	2777: 6F            ld   l,a
	2778: 36 00         ld   (hl),$00
	277A: 05            dec  b
	277B: C2 6F 27      jp   nz,$276F ;loops for 0x10 iterations
	277E: 0D            dec  c
	277F: C9            ret
273C: C2 28 27      jp   nz,$2728
273F: 7A            ld   a,d ;total n of iterations = 0x400
2740: FE 40         cp   $40
2742: C3 04 25      jp   $2504
2504: 7D            ld   a,l
2505: C8            ret  z
0CF1: FE 20         cp   $20
0CF3: C8            ret  z ;if A == 0x20 then fine, otherwise ...
0CF4: 21 10 41      ld   hl,$4110
0CF7: 25            dec  h
0CF8: 36 00         ld   (hl),$00 ; ... reset the game
0CFA: C9            ret

	For now, we use a kludge to feed what the game needs, there could be many possible combinations of this so a PCB tracing / trojan is needed
	to determine the behaviour of this.

	---x ---- enables alternative protection seed
	---- x--- increments read index buffer
	---- -x-- reset read index buffer
	---- --x- puts a bit into the write buffer
	---- ---x reset write index buffer
	*/
	if(data & 1 && ((m_popflame_prot_seed & 1) == 0)) //Note: we use the write buffer index
		m_r_index = 0;
	if(data & 8 && ((m_popflame_prot_seed & 8) == 0))
		m_r_index++;

	m_popflame_prot_seed = data & 0x10;

}



static ADDRESS_MAP_START( naughtyb_map, AS_PROGRAM, 8, naughtyb_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x8800, 0x8fff) AM_RAM AM_SHARE("videoram2")
	AM_RANGE(0x9000, 0x97ff) AM_WRITE(naughtyb_videoreg_w)
	AM_RANGE(0x9800, 0x9fff) AM_RAM AM_SHARE("scrollreg")
	AM_RANGE(0xa000, 0xa7ff) AM_DEVWRITE("naughtyb_custom", naughtyb_sound_device, control_a_w)
	AM_RANGE(0xa800, 0xafff) AM_DEVWRITE("naughtyb_custom", naughtyb_sound_device, control_b_w)
	AM_RANGE(0xb000, 0xb7ff) AM_READ(in0_port_r)    // IN0
	AM_RANGE(0xb800, 0xbfff) AM_READ(dsw0_port_r)   // DSW0
ADDRESS_MAP_END

static ADDRESS_MAP_START( popflame_map, AS_PROGRAM, 8, naughtyb_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x8800, 0x8fff) AM_RAM AM_SHARE("videoram2")
	AM_RANGE(0x9000, 0x97ff) AM_WRITE(popflame_videoreg_w)
	AM_RANGE(0x9800, 0x9fff) AM_RAM AM_SHARE("scrollreg")
	AM_RANGE(0xa000, 0xa7ff) AM_DEVWRITE("popflame_custom", popflame_sound_device, control_a_w)
	AM_RANGE(0xa800, 0xafff) AM_DEVWRITE("popflame_custom", popflame_sound_device, control_b_w)
	AM_RANGE(0xb000, 0xb7ff) AM_READ(in0_port_r)    // IN0
	AM_RANGE(0xb800, 0xbfff) AM_READ(dsw0_port_r)   // DSW0
ADDRESS_MAP_END



/***************************************************************************

  Naughty Boy doesn't have VBlank interrupts.
  Interrupts are still used by the game: but they are related to coin
  slots.

***************************************************************************/

INPUT_CHANGED_MEMBER(naughtyb_state::coin_inserted)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? ASSERT_LINE : CLEAR_LINE);
}

static INPUT_PORTS_START( naughtyb )
	PORT_START( "IN0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY

	PORT_START( "IN0_COCKTAIL" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // IPT_START1
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // IPT_START2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL

	PORT_START( "DSW0" )
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x08, "50000" )
	PORT_DIPSETTING(    0x0c, "70000" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START( "FAKE" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, naughtyb_state,coin_inserted, 0)
	// when reading DSW0, bit 7 doesn't read cabinet, but vblank
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

static INPUT_PORTS_START( trvmstr )
	PORT_START( "IN0" )
	PORT_SERVICE(0x0f, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START( "IN0_COCKTAIL" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START( "DSW0" )
	PORT_DIPNAME( 0x03, 0x00, "Screen Orientation" )
	PORT_DIPSETTING(    0x00, "0'" )
	PORT_DIPSETTING(    0x02, "90'" )
	PORT_DIPSETTING(    0x01, "180'" )
	PORT_DIPSETTING(    0x03, "270'" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Show Correct Answer" )
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x40, 0x40, "Number of Questions" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x40, "7" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START( "FAKE" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, naughtyb_state,coin_inserted, 0)
	// when reading DSW0, bit 7 doesn't read cabinet, but vblank
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	512,    /* 512 characters */
	2,      /* 2 bits per pixel */
	{ 512*8*8, 0 }, /* the two bitplanes are separated */
	{ 7, 6, 5, 4, 3, 2, 1, 0 }, /* pretty straightforward layout */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     /* every char takes 8 consecutive bytes */
};



static GFXDECODE_START( naughtyb )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout, 32*4, 32 )
GFXDECODE_END


static MACHINE_CONFIG_START( naughtyb, naughtyb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, CLOCK_XTAL / 4) /* 12 MHz clock, divided by 4. CPU is a Z80A */
	MCFG_CPU_PROGRAM_MAP(naughtyb_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(36*8, 28*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(naughtyb_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", naughtyb)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(naughtyb_state, naughtyb)

	/* sound hardware */
	/* uses the TMS3615NS for sound */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_TMS36XX_ADD("tms", 350)
	MCFG_TMS36XX_TYPE(TMS3615)
	MCFG_TMS36XX_DECAY_TIMES(0.15, 0.20, 0, 0, 0, 0)
	// NOTE: it's unknown if the TMS3615 mixes more than one voice internally.
	// A wav taken from Pop Flamer sounds like there are at least no 'odd'
	// harmonics (5 1/3' and 2 2/3')
	MCFG_SOUND_ROUTE(0, "mono", 0.60)

	MCFG_SOUND_ADD("naughtyb_custom", NAUGHTYB, 0)
	MCFG_SOUND_ROUTE(0, "mono", 0.40)
MACHINE_CONFIG_END


/* Exactly the same but for certain address writes */
static MACHINE_CONFIG_START( popflame, naughtyb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, CLOCK_XTAL / 4) /* 12 MHz clock, divided by 4. CPU is a Z80A */
	MCFG_CPU_PROGRAM_MAP(popflame_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(36*8, 28*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(naughtyb_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", naughtyb)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(naughtyb_state, naughtyb)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_TMS36XX_ADD("tms", 350)
	MCFG_TMS36XX_TYPE(TMS3615)
	MCFG_TMS36XX_DECAY_TIMES(0.15, 0.20, 0, 0, 0, 0)
	// NOTE: it's unknown if the TMS3615 mixes more than one voice internally.
	// A wav taken from Pop Flamer sounds like there are at least no 'odd'
	// harmonics (5 1/3' and 2 2/3')
	MCFG_SOUND_ROUTE(0, "mono", 0.60)

	MCFG_SOUND_ADD("popflame_custom", POPFLAME, 0)
	MCFG_SOUND_ROUTE(0, "mono", 1.0)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( naughtyb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.30",      0x0000, 0x0800, CRC(f6e1178e) SHA1(5cd428e1f085ff82d7237b3e261b33ff876fd4cb) )
	ROM_LOAD( "2.29",      0x0800, 0x0800, CRC(b803eb8c) SHA1(c21b781eb329195e36e6fd1d7467bd9b0d9cbc5b) )
	ROM_LOAD( "3.28",      0x1000, 0x0800, CRC(004d0ba7) SHA1(5c182fa6f65f7caa3459fcc5cdc3b7faa8b34769) )
	ROM_LOAD( "4.27",      0x1800, 0x0800, CRC(3c7bcac6) SHA1(ef291cd5b2f8a64999dc015e16d3ea479fefaf8f) )
	ROM_LOAD( "5.26",      0x2000, 0x0800, CRC(ea80f39b) SHA1(f05cc4ca48245053a8b35b594fb4c0c3b19304e0) )
	ROM_LOAD( "6.25",      0x2800, 0x0800, CRC(66d9f942) SHA1(756b188836e9e9d86f8be59c9505288339b91899) )
	ROM_LOAD( "7.24",      0x3000, 0x0800, CRC(00caf9be) SHA1(0599b28dfe8dd9c18564202af56ba8f272d7ac54) )
	ROM_LOAD( "8.23",      0x3800, 0x0800, CRC(17c3b6fb) SHA1(c01c8ae27f5b9be90778f7c459c5ba0dddf443ba) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "15.44",     0x0000, 0x0800, CRC(d692f9c7) SHA1(3573c518868690b140340d19f88c670026a6696d) )
	ROM_LOAD( "16.43",     0x0800, 0x0800, CRC(d3ba8b27) SHA1(0ff14b8b983ab75870fb19b64327070ccd0888d6) )
	ROM_LOAD( "13.46",     0x1000, 0x0800, CRC(c1669cd5) SHA1(9b4370ed54424e3615fa2e4d07cadae37ab8cd10) )
	ROM_LOAD( "14.45",     0x1800, 0x0800, CRC(eef2c8e5) SHA1(5077c4052342958ee26c25047704c62eed44eb89) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "11.48",     0x0000, 0x0800, CRC(75ec9710) SHA1(b41606930eff79ccf5bfcad01362251d7bab114a) )
	ROM_LOAD( "12.47",     0x0800, 0x0800, CRC(ef0706c3) SHA1(0e0b82d29d710d1244384db84344bfba2e867b2e) )
	ROM_LOAD( "9.50",      0x1000, 0x0800, CRC(8c8db764) SHA1(2641a1b8bc30896293ebd9396e304ce5eb7eb705) )
	ROM_LOAD( "10.49",     0x1800, 0x0800, CRC(c97c97b9) SHA1(5da7fb378e85b6c9d5ab6e75544f1e64fae9997a) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "6301-1.63", 0x0000, 0x0100, CRC(98ad89a1) SHA1(ddee7dcb003b66fbc7d6d6e90d499ed090c59227) ) /* palette low bits */
	ROM_LOAD( "6301-1.64", 0x0100, 0x0100, CRC(909107d4) SHA1(138ace7845424bc3ca86b0889be634943c8c2d19) ) /* palette high bits */
ROM_END

ROM_START( naughtyba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "91",        0x0000, 0x0800, CRC(42b14bc7) SHA1(a5890834105b83f6761a5ea819e94533473f0e44) )
	ROM_LOAD( "92",        0x0800, 0x0800, CRC(a24674b4) SHA1(2d93981c2f0dea190745cbc3926b012cfd561ec3) )
	ROM_LOAD( "3.28",      0x1000, 0x0800, CRC(004d0ba7) SHA1(5c182fa6f65f7caa3459fcc5cdc3b7faa8b34769) )
	ROM_LOAD( "4.27",      0x1800, 0x0800, CRC(3c7bcac6) SHA1(ef291cd5b2f8a64999dc015e16d3ea479fefaf8f) )
	ROM_LOAD( "95",        0x2000, 0x0800, CRC(e282f1b8) SHA1(9eb7b2fed75cd23f3c90e445021f23648503c96f) )
	ROM_LOAD( "96",        0x2800, 0x0800, CRC(61178ff2) SHA1(2a7fb894e7fc5ec170d00d24300f1e23307f9687) )
	ROM_LOAD( "97",        0x3000, 0x0800, CRC(3cafde88) SHA1(c77f03e81128341522d46056aad77e73c2818069) )
	ROM_LOAD( "8.23",      0x3800, 0x0800, CRC(17c3b6fb) SHA1(c01c8ae27f5b9be90778f7c459c5ba0dddf443ba) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "15.44",     0x0000, 0x0800, CRC(d692f9c7) SHA1(3573c518868690b140340d19f88c670026a6696d) )
	ROM_LOAD( "16.43",     0x0800, 0x0800, CRC(d3ba8b27) SHA1(0ff14b8b983ab75870fb19b64327070ccd0888d6) )
	ROM_LOAD( "13.46",     0x1000, 0x0800, CRC(c1669cd5) SHA1(9b4370ed54424e3615fa2e4d07cadae37ab8cd10) )
	ROM_LOAD( "14.45",     0x1800, 0x0800, CRC(eef2c8e5) SHA1(5077c4052342958ee26c25047704c62eed44eb89) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "11.48",     0x0000, 0x0800, CRC(75ec9710) SHA1(b41606930eff79ccf5bfcad01362251d7bab114a) )
	ROM_LOAD( "12.47",     0x0800, 0x0800, CRC(ef0706c3) SHA1(0e0b82d29d710d1244384db84344bfba2e867b2e) )
	ROM_LOAD( "9.50",      0x1000, 0x0800, CRC(8c8db764) SHA1(2641a1b8bc30896293ebd9396e304ce5eb7eb705) )
	ROM_LOAD( "10.49",     0x1800, 0x0800, CRC(c97c97b9) SHA1(5da7fb378e85b6c9d5ab6e75544f1e64fae9997a) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "6301-1.63", 0x0000, 0x0100, CRC(98ad89a1) SHA1(ddee7dcb003b66fbc7d6d6e90d499ed090c59227) ) /* palette low bits */
	ROM_LOAD( "6301-1.64", 0x0100, 0x0100, CRC(909107d4) SHA1(138ace7845424bc3ca86b0889be634943c8c2d19) ) /* palette high bits */
ROM_END

ROM_START( naughtybc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nb1ic30",   0x0000, 0x0800, CRC(3f482fa3) SHA1(5c670ad37be5bed12a65b8b02330525cfe5ae303) )
	ROM_LOAD( "nb2ic29",   0x0800, 0x0800, CRC(7ddea141) SHA1(8a725614b156f1fdb249c2767ddb3f04681e7a3f) )
	ROM_LOAD( "nb3ic28",   0x1000, 0x0800, CRC(8c72a069) SHA1(648df992a5b118d0c48aa20e8621172f50ee5b4c) )
	ROM_LOAD( "nb4ic27",   0x1800, 0x0800, CRC(30feae51) SHA1(fa28942a58c2292147e33747feecad9817c2c8ea) )
	ROM_LOAD( "nb5ic26",   0x2000, 0x0800, CRC(05242fd0) SHA1(3436a18c021643959bd5d475eeb0b8ac6afaec74) )
	ROM_LOAD( "nb6ic25",   0x2800, 0x0800, CRC(7a12ffea) SHA1(4a34d6fcd0b6dc9319424d4122d88744ddc473c7) )
	ROM_LOAD( "nb7ic24",   0x3000, 0x0800, CRC(9cc287df) SHA1(507c551ca8044479e588bd2a3fff600c77ea2255) )
	ROM_LOAD( "nb8ic23",   0x3800, 0x0800, CRC(4d84ff2c) SHA1(66e51116bae787c67c10f282700a94069d7b9fe0) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "15.44",     0x0000, 0x0800, CRC(d692f9c7) SHA1(3573c518868690b140340d19f88c670026a6696d) )
	ROM_LOAD( "16.43",     0x0800, 0x0800, CRC(d3ba8b27) SHA1(0ff14b8b983ab75870fb19b64327070ccd0888d6) )
	ROM_LOAD( "13.46",     0x1000, 0x0800, CRC(c1669cd5) SHA1(9b4370ed54424e3615fa2e4d07cadae37ab8cd10) )
	ROM_LOAD( "14.45",     0x1800, 0x0800, CRC(eef2c8e5) SHA1(5077c4052342958ee26c25047704c62eed44eb89) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "nb11ic48",  0x0000, 0x0800, CRC(23271a13) SHA1(ba46fe9af0f6b6ab366469b9058d95477620e05c) )
	ROM_LOAD( "12.47",     0x0800, 0x0800, CRC(ef0706c3) SHA1(0e0b82d29d710d1244384db84344bfba2e867b2e) )
	ROM_LOAD( "nb9ic50",   0x1000, 0x0800, CRC(d6949c27) SHA1(2076e76ef9f8f4c9beb3dfe863608151ffae3f3c) )
	ROM_LOAD( "10.49",     0x1800, 0x0800, CRC(c97c97b9) SHA1(5da7fb378e85b6c9d5ab6e75544f1e64fae9997a) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "6301-1.63", 0x0000, 0x0100, CRC(98ad89a1) SHA1(ddee7dcb003b66fbc7d6d6e90d499ed090c59227) ) /* palette low bits */
	ROM_LOAD( "6301-1.64", 0x0100, 0x0100, CRC(909107d4) SHA1(138ace7845424bc3ca86b0889be634943c8c2d19) ) /* palette high bits */
ROM_END

ROM_START( popflame )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic86.bin",     0x0000, 0x1000, CRC(06397a4b) SHA1(12ef8aa60033161479ba2239b61a318cbf15b3cf) )
	ROM_LOAD( "ic80.pop",     0x1000, 0x1000, CRC(b77abf3d) SHA1(8626af8fe7d10c52bea7570dd6237de60607bab6) )
	ROM_LOAD( "ic94.bin",     0x2000, 0x1000, CRC(ae5248ae) SHA1(39a7feb94d0392a0eeeb506d2f52299151521692) )
	ROM_LOAD( "ic100.pop",    0x3000, 0x1000, CRC(f9f2343b) SHA1(c019a5d838152417ec76be021d659f884928ef87) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "ic13.pop",     0x0000, 0x1000, CRC(2367131e) SHA1(d7c15536e5e51f406f9b874333386251f4d3934f) )
	ROM_LOAD( "ic3.pop",      0x1000, 0x1000, CRC(deed0a8b) SHA1(1aaa854f5c6ca3847726cb8a2f2f37f3eb4f621b) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "ic29.pop",     0x0000, 0x1000, CRC(7b54f60f) SHA1(9ec979e67313351dd1165ff6cf591aadead30770) )
	ROM_LOAD( "ic38.pop",     0x1000, 0x1000, CRC(dd2d9601) SHA1(7f495705d6b61c5416e87557a69da7e6457567ac) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "ic54",         0x0000, 0x0100, CRC(236bc771) SHA1(5c078eecdd9df2fbc791e440f96bc4c79476b211) ) /* palette low bits */
	ROM_LOAD( "ic53",         0x0100, 0x0100, CRC(6e66057f) SHA1(084d630f5e2f23e28a1f7839337ef608e086e8c4) ) /* palette high bits */
ROM_END

ROM_START( popflamea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic86.pop",     0x0000, 0x1000, CRC(5e32bbdf) SHA1(b75e3125301d05f5fb6bcef85d0028de2ee75fab) )
	ROM_LOAD( "ic80.pop",     0x1000, 0x1000, CRC(b77abf3d) SHA1(8626af8fe7d10c52bea7570dd6237de60607bab6) )
	ROM_LOAD( "ic94.pop",     0x2000, 0x1000, CRC(945a3c0f) SHA1(353fce8904d869bbf654b7be99e76cadf325b47d) )
	ROM_LOAD( "ic100.pop",    0x3000, 0x1000, CRC(f9f2343b) SHA1(c019a5d838152417ec76be021d659f884928ef87) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "ic13.pop",     0x0000, 0x1000, CRC(2367131e) SHA1(d7c15536e5e51f406f9b874333386251f4d3934f) )
	ROM_LOAD( "ic3.pop",      0x1000, 0x1000, CRC(deed0a8b) SHA1(1aaa854f5c6ca3847726cb8a2f2f37f3eb4f621b) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "ic29.pop",     0x0000, 0x1000, CRC(7b54f60f) SHA1(9ec979e67313351dd1165ff6cf591aadead30770) )
	ROM_LOAD( "ic38.pop",     0x1000, 0x1000, CRC(dd2d9601) SHA1(7f495705d6b61c5416e87557a69da7e6457567ac) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "ic54",         0x0000, 0x0100, CRC(236bc771) SHA1(5c078eecdd9df2fbc791e440f96bc4c79476b211) ) /* palette low bits */
	ROM_LOAD( "ic53",         0x0100, 0x0100, CRC(6e66057f) SHA1(084d630f5e2f23e28a1f7839337ef608e086e8c4) ) /* palette high bits */
ROM_END

ROM_START( popflameb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "popflama.30",     0x0000, 0x1000, CRC(a9bb0e8a) SHA1(1948be9545401b8163e0f2fa8e962ea66c26d9e0) )
	ROM_LOAD( "popflama.28",     0x1000, 0x1000, CRC(debe6d03) SHA1(2365c57a0a08563bea31ab150934dcfc1e6eba58) )
	ROM_LOAD( "popflama.26",     0x2000, 0x1000, CRC(09df0d4d) SHA1(ddc0227035edd11bec045c09c535ad7a375698f1) )
	ROM_LOAD( "popflama.24",     0x3000, 0x1000, CRC(f399d553) SHA1(c08c496fcb99370c344185af599e2ad57a327bc9) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "ic13.pop",     0x0000, 0x1000, CRC(2367131e) SHA1(d7c15536e5e51f406f9b874333386251f4d3934f) )
	ROM_LOAD( "ic3.pop",      0x1000, 0x1000, CRC(deed0a8b) SHA1(1aaa854f5c6ca3847726cb8a2f2f37f3eb4f621b) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "ic29.pop",     0x0000, 0x1000, CRC(7b54f60f) SHA1(9ec979e67313351dd1165ff6cf591aadead30770) )
	ROM_LOAD( "ic38.pop",     0x1000, 0x1000, CRC(dd2d9601) SHA1(7f495705d6b61c5416e87557a69da7e6457567ac) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "ic54",         0x0000, 0x0100, CRC(236bc771) SHA1(5c078eecdd9df2fbc791e440f96bc4c79476b211) ) /* palette low bits */
	ROM_LOAD( "ic53",         0x0100, 0x0100, CRC(6e66057f) SHA1(084d630f5e2f23e28a1f7839337ef608e086e8c4) ) /* palette high bits */
ROM_END

/*
CPU

Main cpu z80
Sound ic TMS3615
Osc: 12 Mhz

ROMs

pfb2-1 to pfb2-8 main program
pfb2-9 to pfb2-16 graphics
All eproms are 2716

Note

This romset comes from a bootleg pcb.It's a Naughty Boy conversion.
*/

ROM_START( popflamen )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pfb2-1.bin",   0x0000, 0x0800, CRC(88cd3faa) SHA1(5232aaadfc0a7275e19176a6e49e178c29cd463b) )
	ROM_LOAD( "pfb2-2.bin",   0x0800, 0x0800, CRC(a09892e8) SHA1(7a56878e3b01023cfa412f7ce7515e34877ff062) )
	ROM_LOAD( "pfb2-3.bin",   0x1000, 0x0800, CRC(99fca5ed) SHA1(b4ced9038681e329b5dbe9b11ad3c91c2945daff) )
	ROM_LOAD( "pfb2-4.bin",   0x1800, 0x0800, CRC(c8d254e0) SHA1(dc7776f672f8f4371a36546fe6c636c2552033d3) )
	ROM_LOAD( "pfb2-5.bin",   0x2000, 0x0800, CRC(d89710d5) SHA1(e226faf315b69462d8592867618c4bed276a5926) )
	ROM_LOAD( "pfb2-6.bin",   0x2800, 0x0800, CRC(b6cec1aa) SHA1(f676ff96eb3bde85837b6c624c0b246c4bca2f57) )
	ROM_LOAD( "pfb2-7.bin",   0x3000, 0x0800, CRC(1cf8b5c4) SHA1(f9e063196be9338377c2298956c8d04e0d3ffcdf) )
	ROM_LOAD( "pfb2-8.bin",   0x3800, 0x0800, CRC(a63feeff) SHA1(c121eb4e46e432679de42031a9649c6b3ac403ce) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pfb2-15.bin",      0x0000, 0x0800, CRC(3d8b8f6f) SHA1(1a05fb802e91a70e14295a3cef9c3c544df0dbdd) )
	ROM_LOAD( "pfb2-16.bin",      0x0800, 0x0800, CRC(75f0308b) SHA1(a38f302844a39d08ef7d117efc0d8cf94cfe4756) )
	ROM_LOAD( "pfb2-13.bin",      0x1000, 0x0800, CRC(42fc5bac) SHA1(9a5755112cd18e3e75a331c90c6a0a16874f10e6) )
	ROM_LOAD( "pfb2-14.bin",      0x1800, 0x0800, CRC(fefada6e) SHA1(bd4ad92077445e63ec4d2977598753feac279817) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "pfb2-11.bin",      0x0000, 0x0800, CRC(8ccdcc01) SHA1(3cea26ca832e95507f0736e755d765b20b0f5415) )
	ROM_LOAD( "pfb2-12.bin",      0x0800, 0x0800, CRC(49e04ddb) SHA1(860633b431b5313ada2060829d45e3b7194e6825) )
	ROM_LOAD( "pfb2-9.bin",       0x1000, 0x0800, CRC(32debf48) SHA1(4181ac1416d4ed4f13a968db31f4026c92622743) )
	ROM_LOAD( "pfb2-10.bin",      0x1800, 0x0800, CRC(7fe61ed3) SHA1(9654543089ceeec8a3d398eb591abc500dbeaf28) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "ic54",         0x0000, 0x0100, CRC(236bc771) SHA1(5c078eecdd9df2fbc791e440f96bc4c79476b211) ) /* palette low bits */
	ROM_LOAD( "ic53",         0x0100, 0x0100, CRC(6e66057f) SHA1(084d630f5e2f23e28a1f7839337ef608e086e8c4) ) /* palette high bits */
ROM_END


ROM_START( trvmstr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic30.bin",     0x0000, 0x1000, CRC(4ccd0537) SHA1(f0991581c2efeb54626dd1f8acf33a28ed1b6f80) )
	ROM_LOAD( "ic28.bin",     0x1000, 0x1000, CRC(782a2b8c) SHA1(611be829470c2fcbb301f48f5e80ad97e51ef821) )
	ROM_LOAD( "ic26.bin",     0x2000, 0x1000, CRC(1362010a) SHA1(d721e051329b823e79515a631244eb77b77c731a) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "ic44.bin",     0x0000, 0x1000, CRC(dac8cff7) SHA1(21da2b2ceb4a726d03b2e49a2df75ca66b89a197) )
	ROM_LOAD( "ic46.bin",     0x1000, 0x1000, CRC(a97ab879) SHA1(67b86d056896f10e0c055fb58c97341cf75c3d17) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "ic48.bin",     0x0000, 0x1000, CRC(79952015) SHA1(8407c2bab476a60d945d82201f01bf59ae9e0dad) )
	ROM_LOAD( "ic50.bin",     0x1000, 0x1000, CRC(f09da428) SHA1(092d0eea41c8bbd48d7a3aff54c15f85262b21ff) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "ic64.bin",     0x0000, 0x0100, CRC(e9915da8) SHA1(7c64ea76e39eaff724179d52ff5482df363fcf56) )  /* palette low & high bits */
	ROM_RELOAD(               0x0100, 0x0100 )

	ROM_REGION( 0x20000, "user1", 0 )   /* Questions roms */
	ROM_LOAD( "sport_lo.u2",  0x00000, 0x4000, CRC(24f30489) SHA1(b34ecd485bccb7b78332196e6dffd18721177ac3) )
	ROM_LOAD( "sport_hi.u1",  0x04000, 0x4000, CRC(d64a7480) SHA1(4239c7142d783cbd4242ff58d74e87d87f3535e6) )
	ROM_LOAD( "etain_lo.u4",  0x08000, 0x4000, CRC(a2af9709) SHA1(24858ab58a8a6577446215e261da877cb48c03df) )
	ROM_LOAD( "etain_hi.u3",  0x0c000, 0x4000, CRC(82a60dea) SHA1(2b03a67507c5a5c343804cf40b8b8147df070002) )
	ROM_LOAD( "sex_lo.u6",    0x10000, 0x4000, CRC(f2ecfa88) SHA1(15e9ce1be8b868a99b72426abbdf086fcf134517) )
	ROM_LOAD( "sex_hi.u5",    0x14000, 0x4000, CRC(de4a6c4b) SHA1(ba12193eabcee7e4d354678ddd780e1e338efbb1) )
	ROM_LOAD( "scien_lo.u8",  0x18000, 0x4000, CRC(01a01ff1) SHA1(d3b62ae466681ae01ab1beaf2958af94c9c4cbcb) )
	ROM_LOAD( "scien_hi.u7",  0x1c000, 0x4000, CRC(0bc68078) SHA1(910cd1a8ca68cff87c93a8ffa810d77338fc710b) )
ROM_END

ROM_START( trvmstra )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic30a.bin",    0x0000, 0x2000, CRC(4c175c45) SHA1(770e128ad30ef6ad9936cbf4da810c8b38c7b630) )
	ROM_LOAD( "ic28a.bin",    0x1000, 0x2000, CRC(3a8ca87d) SHA1(bf82ca226daa13eabf8db3cabe2c047b831188e8) )
	ROM_LOAD( "ic26a.bin",    0x2000, 0x2000, CRC(3c655400) SHA1(d536e7cf63834b0ce94fb4e597c370befa792f82) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "ic44.bin",     0x0000, 0x1000, CRC(dac8cff7) SHA1(21da2b2ceb4a726d03b2e49a2df75ca66b89a197) )
	ROM_LOAD( "ic46.bin",     0x1000, 0x1000, CRC(a97ab879) SHA1(67b86d056896f10e0c055fb58c97341cf75c3d17) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "ic48.bin",     0x0000, 0x1000, CRC(79952015) SHA1(8407c2bab476a60d945d82201f01bf59ae9e0dad) )
	ROM_LOAD( "ic50.bin",     0x1000, 0x1000, CRC(f09da428) SHA1(092d0eea41c8bbd48d7a3aff54c15f85262b21ff) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "ic64.bin",     0x0000, 0x0100, CRC(e9915da8) SHA1(7c64ea76e39eaff724179d52ff5482df363fcf56) )  /* palette low & high bits */
	ROM_RELOAD(               0x0100, 0x0100 )

	ROM_REGION( 0x20000, "user1", 0 )   /* Questions roms */
	ROM_LOAD( "enter_lo.u2",  0x00000, 0x4000, CRC(a65b8f83) SHA1(a86bef07349a00aa977270e3504cf2698c7c6333) )
	ROM_LOAD( "enter_hi.u1",  0x04000, 0x4000, CRC(caede447) SHA1(ee6d015e3e7d338926296c69eab3e07dbb64a8e6) )
	ROM_LOAD( "sports_lo.u4", 0x08000, 0x4000, CRC(d5317b26) SHA1(8d93cf9c15b25687f224e01f332f53cac3180b83) )
	ROM_LOAD( "sports_hi.u3", 0x0c000, 0x4000, CRC(9f706db2) SHA1(171b5c490bd576d33355cfd3cd4d1b0c5cb90e00) )
	ROM_LOAD( "sex2_lo.u6",   0x10000, 0x4000, CRC(b73f2e31) SHA1(4390152e053118c31ed74fe850ea7124c0e7b731) )
	ROM_LOAD( "sex2_hi.u5",   0x14000, 0x4000, CRC(bf654110) SHA1(5229f5e6973a04c53572ea94c14d79a238c0e90f) )
	ROM_LOAD( "comic_lo.u8",  0x18000, 0x4000, CRC(109bd359) SHA1(ea8cb4b0a14a3ef4932947afdfa773ecc34c2b9b) )
	ROM_LOAD( "comic_hi.u7",  0x1c000, 0x4000, CRC(8e8b5f71) SHA1(71514af2af2468a13cf5cc4237fa2590d7a16b27) )
ROM_END

ROM_START( trvmstrb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic30b.bin",   0x0000, 0x1000, CRC(d3eb4197) SHA1(5843ffc8ec82ffe9a6519180e54c82b0375cc3dc) )
	ROM_LOAD( "ic28b.bin",   0x1000, 0x1000, CRC(70322d65) SHA1(498102236390f2e15444943e0fff8a53f37db083) )
	ROM_LOAD( "ic26b.bin",   0x2000, 0x1000, CRC(31dfa9cf) SHA1(007c6ef2381ce9e707932c66a451805cec342eeb) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "ic44.bin",     0x0000, 0x1000, CRC(dac8cff7) SHA1(21da2b2ceb4a726d03b2e49a2df75ca66b89a197) )
	ROM_LOAD( "ic46.bin",     0x1000, 0x1000, CRC(a97ab879) SHA1(67b86d056896f10e0c055fb58c97341cf75c3d17) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "ic48.bin",     0x0000, 0x1000, CRC(79952015) SHA1(8407c2bab476a60d945d82201f01bf59ae9e0dad) )
	ROM_LOAD( "ic50.bin",     0x1000, 0x1000, CRC(f09da428) SHA1(092d0eea41c8bbd48d7a3aff54c15f85262b21ff) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "ic64.bin",     0x0000, 0x0100, CRC(e9915da8) SHA1(7c64ea76e39eaff724179d52ff5482df363fcf56) )  /* palette low & high bits */
	ROM_RELOAD(               0x0100, 0x0100 )

	ROM_REGION( 0x20000, "user1", 0 )   /* Questions roms */
	ROM_LOAD( "earlytv_lo.u2", 0x00000, 0x4000, CRC(dbfce45f) SHA1(5d96186c96dee810b0ef63964cb3614fd486aefa) )
	ROM_LOAD( "earlytv_hi.u1", 0x04000, 0x4000, CRC(c8f5a02d) SHA1(8a566f83f9bd39ab508085af942957a7ed941813) )
	ROM_LOAD( "sex_lo.u4",     0x08000, 0x4000, CRC(27a4ff7a) SHA1(7479c69b85ea1f4bb3d368a11cb036f707b65b6b) )
	ROM_LOAD( "sex_hi.u3",     0x0c000, 0x4000, CRC(de84fc2f) SHA1(ac5736ca6654fc649f2e8e4a996c2b0f1e86947b) )
	ROM_LOAD( "rock_lo.u6",    0x10000, 0x4000, CRC(ec1df27b) SHA1(95f32b366123742a4576aef272209c8d59cc6b0a) )
	ROM_LOAD( "rock_hi.u5",    0x14000, 0x4000, CRC(8a4eccc9) SHA1(e3a03f511a84b3321a59f9e9672da27b96931713) )
	ROM_LOAD( "lifesci_lo.u8", 0x18000, 0x4000, CRC(b58ba9eb) SHA1(4fdd62a464dec84c880e1be598a7896a957d9339) )
	ROM_LOAD( "lifesci_hi.u7", 0x1c000, 0x4000, CRC(e03dfa12) SHA1(e668ea5bccc95d8a53a1de1d96583beef6636edb) )
ROM_END

ROM_START( trvmstrc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jaleco.30",   0x0000, 0x1000, CRC(9a80c5a7) SHA1(d8094572b946527a652f9b978a17f0a8a7acbddb) )
	ROM_LOAD( "jaleco.28",   0x1000, 0x1000, CRC(70322d65) SHA1(498102236390f2e15444943e0fff8a53f37db083) )
	ROM_LOAD( "jaleco.26",   0x2000, 0x1000, CRC(3431a2ba) SHA1(d704d5420e232e9d0ea65142aab063dbc20da8c5) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "jaleco.44",     0x0000, 0x1000, CRC(dac8cff7) SHA1(21da2b2ceb4a726d03b2e49a2df75ca66b89a197) )
	ROM_LOAD( "jaleco.46",     0x1000, 0x1000, CRC(a97ab879) SHA1(67b86d056896f10e0c055fb58c97341cf75c3d17) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "jaleco.48",     0x0000, 0x1000, CRC(79952015) SHA1(8407c2bab476a60d945d82201f01bf59ae9e0dad) )
	ROM_LOAD( "jaleco.50",     0x1000, 0x1000, CRC(f09da428) SHA1(092d0eea41c8bbd48d7a3aff54c15f85262b21ff) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "jaleco.64",     0x0000, 0x0100, CRC(e9915da8) SHA1(7c64ea76e39eaff724179d52ff5482df363fcf56) )  /* palette low & high bits */
	ROM_RELOAD(               0x0100, 0x0100 )

	ROM_REGION( 0x20000, "user1", 0 )   /* Questions roms */
	ROM_LOAD( "jaleco.2", 0x00000, 0x4000, CRC(1ad4c446) SHA1(daef6e4bea69002a52bca016cf33d2726eac4e66) ) //car boats and planes
	ROM_LOAD( "jaleco.1", 0x04000, 0x4000, CRC(9c308849) SHA1(b40a5c238c0116ac92f3076fbec02b3d7d209883) )
	ROM_LOAD( "jaleco.4", 0x08000, 0x4000, CRC(38dd45cd) SHA1(e701de1509a898fe400a6284794ead6bee8ccba3) ) //movies
	ROM_LOAD( "jaleco.3", 0x0c000, 0x4000, CRC(83b5465b) SHA1(828500a008926057f10d40ac9aaaa14d2890181e) )
	ROM_LOAD( "jaleco.6", 0x10000, 0x4000, CRC(4a2263a7) SHA1(63f2f79261d508c9bba3d73d78f7dce5d348b6d4) ) //sports II
	ROM_LOAD( "jaleco.5", 0x14000, 0x4000, CRC(bd31f382) SHA1(ec04a5d4a5fc8be059abf3c21c65cd970e569d44) )
	ROM_LOAD( "jaleco.8", 0x18000, 0x4000, CRC(b73f2e31) SHA1(4390152e053118c31ed74fe850ea7124c0e7b731) ) //sex II
	ROM_LOAD( "jaleco.7", 0x1c000, 0x4000, CRC(bf654110) SHA1(5229f5e6973a04c53572ea94c14d79a238c0e90f) )
ROM_END

/* These 'Trivia Genius' roms were found on a Naughty Boy pcb, factory?-retooled somewhat to use 8 2732s instead of 16 2716s. The pcb is a real Naughty Boy PCB, with Jaleco markings. Latest chip datecodes on the PCB are from 85 (on the two proms) but the other chips are dated 81 and 82 (which makes sense if they're formerly naughty boy pcbs).
This may be a hacked/bootlegged version of trivia master, hacked to run on a naughty boy pcb, or (if ALL versions of trivia master ran on naughty boy pcbs) may be a 'second source bootleg', a bootleg of the trivia master naughty boy conversion.
*/

ROM_START( trvgns )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "trvgns.30",   0x0000, 0x1000, CRC(a17f172c) SHA1(b831673f860f6b7566e248b13b349d82379b5e72) )
	ROM_LOAD( "trvgns.28",   0x1000, 0x1000, CRC(681a1bff) SHA1(53da179185ae3bfb30502706cc623c2f4cc57128) )
	ROM_LOAD( "trvgns.26",   0x2000, 0x1000, CRC(5b4068b8) SHA1(3b424dd8e2a6fa1e4628790f60c51d44f9a535a1) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "trvgns.44",   0x0000, 0x1000, CRC(cd67f2cb) SHA1(22d9d8509fd44fbeb313f5120e692d7a30e3ca54) )
	ROM_LOAD( "trvgns.46",   0x1000, 0x1000, CRC(f4021941) SHA1(81a93b5b2bf46e2f5254a86b14e31b31b7821d4f) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trvgns.48",   0x0000, 0x1000, CRC(6d05845e) SHA1(b427075ab05aea79298211d882b523d4fad1e9ad) )
	ROM_LOAD( "trvgns.50",   0x1000, 0x1000, CRC(ac292be8) SHA1(41f95273907b27158af0631c716fdb9301852e27) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s129.ic63.bin", 0x0000, 0x0100, CRC(8ab6076a) SHA1(042df008aa4fd0a99b662333fa91d20ed17bf045) ) /* palette low bits */
	ROM_LOAD( "82s129.ic64.bin", 0x0100, 0x0100, CRC(c766c54a) SHA1(3ac001009ce1dbcb3eaacd2da2540c19259934c0) ) /* palette high bits */

	ROM_REGION( 0x20000, "user1", 0 ) /* Question roms */
	ROM_LOAD( "trvgns.u2",   0x00000, 0x4000, CRC(109bd359) SHA1(ea8cb4b0a14a3ef4932947afdfa773ecc34c2b9b) )
	ROM_LOAD( "trvgns.u1",   0x04000, 0x4000, CRC(8e8b5f71) SHA1(71514af2af2468a13cf5cc4237fa2590d7a16b27) )
	ROM_LOAD( "trvgns.u4",   0x08000, 0x4000, CRC(b73f2e31) SHA1(4390152e053118c31ed74fe850ea7124c0e7b731) )
	ROM_LOAD( "trvgns.u3",   0x0c000, 0x4000, CRC(bf654110) SHA1(5229f5e6973a04c53572ea94c14d79a238c0e90f) )
	ROM_LOAD( "trvgns.u6",   0x10000, 0x4000, CRC(4a2263a7) SHA1(63f2f79261d508c9bba3d73d78f7dce5d348b6d4) )
	ROM_LOAD( "trvgns.u5",   0x14000, 0x4000, CRC(bd31f382) SHA1(ec04a5d4a5fc8be059abf3c21c65cd970e569d44) )
	ROM_LOAD( "trvgns.u8",   0x18000, 0x4000, CRC(dbfce45f) SHA1(5d96186c96dee810b0ef63964cb3614fd486aefa) )
	ROM_LOAD( "trvgns.u7",   0x1c000, 0x4000, CRC(c8f5a02d) SHA1(8a566f83f9bd39ab508085af942957a7ed941813) )
ROM_END


DRIVER_INIT_MEMBER(naughtyb_state,popflame)
{
	/* install a handler to catch protection checks */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x9000, 0x9000, read8_delegate(FUNC(naughtyb_state::popflame_protection_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x9090, 0x9090, read8_delegate(FUNC(naughtyb_state::popflame_protection_r),this));

	m_maincpu->space(AS_PROGRAM).install_write_handler(0xb000, 0xb0ff, write8_delegate(FUNC(naughtyb_state::popflame_protection_w),this));
	
	save_item(NAME(m_popflame_prot_seed));
	save_item(NAME(m_r_index));
	save_item(NAME(m_prot_count));
}


READ8_MEMBER(naughtyb_state::trvmstr_questions_r)
{
	return memregion("user1")->base()[m_question_offset];
}

WRITE8_MEMBER(naughtyb_state::trvmstr_questions_w)
{
	switch(offset)
	{
	case 0:
		m_question_offset = (m_question_offset & 0xffff00) | data;
		break;
	case 1:
		m_question_offset = (m_question_offset & 0xff00ff) | (data << 8);
		break;
	case 2:
		m_question_offset = (m_question_offset & 0x00ffff) | (data << 16);
		break;
	}
}

DRIVER_INIT_MEMBER(naughtyb_state,trvmstr)
{
	/* install questions' handlers  */
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xc000, 0xc002, read8_delegate(FUNC(naughtyb_state::trvmstr_questions_r),this), write8_delegate(FUNC(naughtyb_state::trvmstr_questions_w),this));
	
	save_item(NAME(m_question_offset));
}


GAME( 1982, naughtyb, 0,        naughtyb, naughtyb, driver_device, 0,        ROT90, "Jaleco", "Naughty Boy", GAME_SUPPORTS_SAVE )
GAME( 1982, naughtyba,naughtyb, naughtyb, naughtyb, driver_device, 0,        ROT90, "bootleg", "Naughty Boy (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1982, naughtybc,naughtyb, naughtyb, naughtyb, driver_device, 0,        ROT90, "Jaleco (Cinematronics license)", "Naughty Boy (Cinematronics)", GAME_SUPPORTS_SAVE )
GAME( 1982, popflame, 0,        popflame, naughtyb, naughtyb_state, popflame, ROT90, "Jaleco", "Pop Flamer (protected)", GAME_SUPPORTS_SAVE )
GAME( 1982, popflamea,popflame, popflame, naughtyb, driver_device, 0,        ROT90, "Jaleco", "Pop Flamer (not protected)", GAME_SUPPORTS_SAVE )
GAME( 1982, popflameb,popflame, popflame, naughtyb, driver_device, 0,        ROT90, "Jaleco", "Pop Flamer (hack?)", GAME_SUPPORTS_SAVE )
GAME( 1982, popflamen,popflame, naughtyb, naughtyb, driver_device, 0,        ROT90, "Jaleco", "Pop Flamer (bootleg on Naughty Boy PCB)", GAME_SUPPORTS_SAVE )
GAME( 1985, trvmstr,  0,        naughtyb, trvmstr, naughtyb_state,  trvmstr,  ROT90, "Enerdyne Technologies Inc.", "Trivia Master (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1985, trvmstra, trvmstr,  naughtyb, trvmstr, naughtyb_state,  trvmstr,  ROT90, "Enerdyne Technologies Inc.", "Trivia Master (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1985, trvmstrb, trvmstr,  naughtyb, trvmstr, naughtyb_state,  trvmstr,  ROT90, "Enerdyne Technologies Inc.", "Trivia Master (set 3)", GAME_SUPPORTS_SAVE )
GAME( 1985, trvmstrc, trvmstr,  naughtyb, trvmstr, naughtyb_state,  trvmstr,  ROT90, "Enerdyne Technologies Inc.", "Trivia Master (set 4)", GAME_SUPPORTS_SAVE )
GAME( 1985, trvgns,   trvmstr,  naughtyb, trvmstr, naughtyb_state,  trvmstr,  ROT90, "bootleg", "Trivia Genius", GAME_SUPPORTS_SAVE )
