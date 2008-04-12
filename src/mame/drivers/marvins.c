/*
various early SNK games (1983-1985)
- Marvin's Maze
- Vanguard II
- Mad Crasher

driver by Phil Stroffolino

Known Issues:
    Mad Crasher fails the ROM test, but ROMs are verified to be good (reason's unknown)
    Mad Crasher sound effects aren't being played (fixed)


Change Log
----------

AT08XX03:
 - added shadows
 - fixed Mad Crasher bad background, sound effects and foreground priority.
   (great now I can fall under the skyway like I did at Chuck'n Cheese;)
 - fixed Vanguard2 scroll offsets
 - tuned music tempo and wavegen frequency

2008.04.04: Small note regarding DipSwitches. Locations and values have been
verified with the manual for both Marvin's Maze and Mad Crasher. Vanguard II
DIPs have been checked against jammaboards diplist, no manual available ATM.
DSW2:7 in madcrash is listed as Difficulty, but it's not clear how it affects
the gameplay difficulty

*/

#include "driver.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "snk.h"
#include "sound/ay8910.h"
#include "sound/namco.h"

#define CREDITS "Phil Stroffolino\nTim Lindquist\nCarlos A. Lozano"


/***************************************************************************
**
**  CPUA and CPUB communicate through shared RAM.
**
***************************************************************************/

extern WRITE8_HANDLER( marvins_background_ram_w );
extern WRITE8_HANDLER( marvins_foreground_ram_w );
extern WRITE8_HANDLER( marvins_text_ram_w );
extern WRITE8_HANDLER( marvins_spriteram_w );


/***************************************************************************
**
** Video Driver
**
***************************************************************************/

extern VIDEO_START( marvins );
extern VIDEO_UPDATE( marvins );
extern VIDEO_UPDATE( madcrash );

extern WRITE8_HANDLER( marvins_palette_bank_w );


/***************************************************************************
**
**  Interrupt Handling
**
**  CPUA can trigger an interrupt on CPUB, and CPUB can trigger an interrupt
**  on CPUA.  Each CPU must re-enable interrupts on itself.
**
***************************************************************************/

// see drivers\snk.c


/***************************************************************************
**
**  Sound System
**
**  The sound CPU is a slave, with communication.
**
**  Sound Chips: PSGX2 + "Wave Generater"
**
**  The Custom Wave Generator is controlled by 6 bytes
**
**  The first pair of registers (0x8002, 0x8003) appear to define frequency
**  as a fraction: RAM[0x8003]/RAM[0x8002].
**
**  (0x8004, 0x8005, 0x8006, 0x8007) are currently unmapped.  Probably they
**  control the shape of the wave being played.
**
**  snkwave_interface is currently implemented with the "namco" sound component.
**
***************************************************************************/

extern WRITE8_HANDLER( snkwave_w );

static int sound_cpu_busy;

static const struct namco_interface snkwave_interface =
{
	1,				/* number of voices */
	-1				/* memory region */
};

static void init_sound( int busy_bit )
{
	snk_sound_busy_bit = busy_bit;
	sound_cpu_busy = 0;
}

static WRITE8_HANDLER( sound_command_w )
{
	sound_cpu_busy = snk_sound_busy_bit;
	soundlatch_w(machine, 0, data);
	cpunum_set_input_line(machine, 2, 0, HOLD_LINE);
}

static READ8_HANDLER( sound_command_r )
{
	sound_cpu_busy = 0;
	return(soundlatch_r(machine,0));
}

static READ8_HANDLER( sound_nmi_ack_r )
{
	cpunum_set_input_line(machine, 2, INPUT_LINE_NMI, CLEAR_LINE);
	return 0;
}

/* this input port has one of its bits mapped to sound CPU status */
static READ8_HANDLER( marvins_port_0_r )
{
	return(input_port_read_indexed(machine, 0) | sound_cpu_busy);
}

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_BASE(&namco_wavedata)	/* silly hack - this shouldn't be here */
	AM_RANGE(0x4000, 0x4000) AM_READ(sound_command_r)
	AM_RANGE(0x8000, 0x8000) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x8001, 0x8001) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x8002, 0x8007) AM_WRITE(snkwave_w)
	AM_RANGE(0x8008, 0x8008) AM_WRITE(AY8910_control_port_1_w)
	AM_RANGE(0x8009, 0x8009) AM_WRITE(AY8910_write_port_1_w)
	AM_RANGE(0xa000, 0xa000) AM_READ(sound_nmi_ack_r)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READNOP
ADDRESS_MAP_END


/***************************************************************************
**
**  Memory Maps for CPUA, CPUB
**
**  Shared RAM is shuffled in Mad Crasher/Vanguard II compared to
**  Marvin's Maze.
**
**  A few ports are mapped differently for each game.
**
***************************************************************************/

static ADDRESS_MAP_START( marvins_cpuA_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x6000) AM_WRITE(marvins_palette_bank_w)
	AM_RANGE(0x8000, 0x8000) AM_READ(marvins_port_0_r)	/* coin input, start, sound CPU status */
	AM_RANGE(0x8100, 0x8100) AM_READ_PORT("IN1")			/* player #1 controls */
	AM_RANGE(0x8200, 0x8200) AM_READ_PORT("IN2")			/* player #2 controls */
	AM_RANGE(0x8300, 0x8300) AM_WRITE(sound_command_w)
	AM_RANGE(0x8400, 0x8400) AM_READ_PORT("DSW1")		/* dipswitch#1 */
	AM_RANGE(0x8500, 0x8500) AM_READ_PORT("DSW2")		/* dipswitch#2 */
	AM_RANGE(0x8600, 0x8600) AM_RAM						/* video attribute */
	AM_RANGE(0x8700, 0x8700) AM_READWRITE(snk_cpuB_nmi_trigger_r, snk_cpuA_nmi_ack_w)
	AM_RANGE(0xc000, 0xcfff) AM_RAM AM_BASE(&spriteram) AM_SHARE(1)
	AM_RANGE(0xd000, 0xdfff) AM_READWRITE(SMH_RAM, marvins_background_ram_w) AM_SHARE(2) AM_BASE(&spriteram_3)
	AM_RANGE(0xe000, 0xefff) AM_READWRITE(SMH_RAM, marvins_foreground_ram_w) AM_SHARE(3) AM_BASE(&spriteram_2)
	AM_RANGE(0xf000, 0xffff) AM_READWRITE(SMH_RAM, marvins_text_ram_w) AM_SHARE(4) AM_BASE(&videoram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( marvins_cpuB_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x8700, 0x8700) AM_READWRITE(snk_cpuA_nmi_trigger_r, snk_cpuB_nmi_ack_w)
	AM_RANGE(0xc000, 0xcfff) AM_RAM AM_SHARE(1)
	AM_RANGE(0xd000, 0xdfff) AM_READWRITE(SMH_RAM, marvins_background_ram_w) AM_SHARE(2)
	AM_RANGE(0xe000, 0xefff) AM_READWRITE(SMH_RAM, marvins_foreground_ram_w) AM_SHARE(3)
	AM_RANGE(0xf000, 0xffff) AM_READWRITE(SMH_RAM, marvins_text_ram_w) AM_SHARE(4)
ADDRESS_MAP_END


static ADDRESS_MAP_START( madcrash_cpuA_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8000) AM_READ(marvins_port_0_r)	/* coin input, start, sound CPU status */
	AM_RANGE(0x8100, 0x8100) AM_READ_PORT("IN1")			/* player #1 controls */
	AM_RANGE(0x8200, 0x8200) AM_READ_PORT("IN2")			/* player #2 controls */
	AM_RANGE(0x8300, 0x8300) AM_WRITE(sound_command_w)
	AM_RANGE(0x8400, 0x8400) AM_READ_PORT("DSW1")		/* dipswitch#1 */
	AM_RANGE(0x8500, 0x8500) AM_READ_PORT("DSW2")		/* dipswitch#2 */
	AM_RANGE(0x8600, 0x86ff) AM_RAM	// video attribute
	AM_RANGE(0x8700, 0x8700) AM_READWRITE(snk_cpuB_nmi_trigger_r, snk_cpuA_nmi_ack_w)
//  AM_RANGE(0xc800, 0xc800) AM_WRITE(marvins_palette_bank_w)   // palette bank switch (c8f1 for Vanguard)
	AM_RANGE(0xc800, 0xc8ff) AM_RAM
	AM_RANGE(0xc000, 0xcfff) AM_RAM AM_BASE(&spriteram) AM_SHARE(1)
	AM_RANGE(0xd000, 0xdfff) AM_READWRITE(SMH_RAM, marvins_background_ram_w) AM_SHARE(2) AM_BASE(&spriteram_3)
	AM_RANGE(0xe000, 0xefff) AM_READWRITE(SMH_RAM, marvins_foreground_ram_w) AM_SHARE(3) AM_BASE(&spriteram_2)
	AM_RANGE(0xf000, 0xffff) AM_READWRITE(SMH_RAM, marvins_text_ram_w) AM_SHARE(4) AM_BASE(&videoram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( madcrash_cpuB_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x8700, 0x8700) AM_WRITE(snk_cpuB_nmi_ack_w)	/* Vangaurd II */
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xa000) AM_WRITE(snk_cpuB_nmi_ack_w)	/* Mad Crasher */
	AM_RANGE(0xc000, 0xcfff) AM_READWRITE(SMH_RAM, marvins_foreground_ram_w) AM_SHARE(3)
	AM_RANGE(0xd000, 0xdfff) AM_READWRITE(SMH_RAM, marvins_text_ram_w) AM_SHARE(4)
	AM_RANGE(0xe000, 0xefff) AM_RAM AM_SHARE(1)
	AM_RANGE(0xf000, 0xffff) AM_READWRITE(SMH_RAM, marvins_background_ram_w) AM_SHARE(2)
ADDRESS_MAP_END


static INPUT_PORTS_START( marvins )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* sound CPU status */
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START_TAG("IN1") /* player#1 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN2") /* player#2 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME(0x04,  0x04, "Infinite Lives (Cheat)")	PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coinage ) )		PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x38, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Free_Play ) )	PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )				PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "1st Bonus Life" )		PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x01, "20000" )
	PORT_DIPSETTING(    0x02, "30000" )
	PORT_DIPSETTING(    0x03, "40000" )
	PORT_DIPSETTING(    0x04, "50000" )
	PORT_DIPSETTING(    0x05, "60000" )
	PORT_DIPSETTING(    0x06, "70000" )
	PORT_DIPSETTING(    0x07, "80000" )
	PORT_DIPNAME( 0x18, 0x08, "2nd Bonus Life" )		PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x08, "1st bonus*2" )
	PORT_DIPSETTING(    0x10, "1st bonus*3" )
	PORT_DIPSETTING(    0x18, "1st bonus*4" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( vangrd2 )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* sound CPU status */
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START_TAG("IN1") /* player#1 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN2") /* player#2 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )			PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Bonus_Life ) )		PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x38, "30000" )
	PORT_DIPSETTING(    0x30, "40000" )
	PORT_DIPSETTING(    0x28, "50000" )
	PORT_DIPSETTING(    0x20, "60000" )
	PORT_DIPSETTING(    0x18, "70000" )
	PORT_DIPSETTING(    0x10, "80000" )
	PORT_DIPSETTING(    0x08, "90000" )
	PORT_DIPSETTING(    0x00, "100000" )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Lives ) )			PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0xc0, "5" )

	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )		PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Freeze" )					PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) )			PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Language ) )			PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x10, 0x00, "Bonus Life Occurence" )		PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, "Every bonus" )
	PORT_DIPSETTING(    0x10, "Bonus only" )
	PORT_DIPNAME( 0x20, 0x20, "Infinite Lives (Cheat)")		PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Free_Play ) )		PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )		PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( madcrash )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* sound CPU status */
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START_TAG("IN1") /* player#1 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN2") /* player#2 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("DSW1")
	PORT_DIPUNUSED_DIPLOC(0x01, IP_ACTIVE_LOW, "SW1:1")	/* Listed as Unused */
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coinage ) )		PORT_DIPLOCATION("SW1:4,5,6")
//  PORT_DIPSETTING(    0x08, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "20000 60000" )
	PORT_DIPSETTING(    0x80, "40000 90000" )
	PORT_DIPSETTING(    0x40, "50000 120000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )

	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "Bonus Life Occurence" )		PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "1st, 2nd, then every 2nd" )	/* Check the "Non Bugs" page */
	PORT_DIPSETTING(    0x00, "1st and 2nd only" )
	PORT_DIPNAME( 0x06, 0x04, "Scroll Speed" )				PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, "Slow" )//DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, "Fast" )//DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, "Faster" )
	PORT_DIPNAME( 0x18, 0x10, "Game mode" )					PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "Demo Sounds Off" )
	PORT_DIPSETTING(    0x10, "Demo Sounds On" )
	PORT_DIPSETTING(    0x08, "Infinite Lives (Cheat)")
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )		PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )				/* Check the "Non Bugs" page */
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )		PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPUNKNOWN_DIPLOC(0x80, IP_ACTIVE_LOW, "SW2:8")	/* Listed as Unused, it is actually tested in many places */
INPUT_PORTS_END


/***************************************************************************
**
**  Graphics Layout
**
***************************************************************************/

static const gfx_layout sprite_layout =
{
	16,16,
	0x100,
	3,
	{ 0,0x2000*8,0x4000*8 },
	{
		7,6,5,4,3,2,1,0,
		15,14,13,12,11,10,9,8
	},
	{
		0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16
	},
	256
};

static const gfx_layout tile_layout =
{
	8,8,
	0x100,
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	256
};

static GFXDECODE_START( marvins )
	GFXDECODE_ENTRY( REGION_GFX1, 0, tile_layout,	0x080, 8  ) /* text layer */
	GFXDECODE_ENTRY( REGION_GFX2, 0, tile_layout,	0x110, 1  ) /* background */
	GFXDECODE_ENTRY( REGION_GFX3, 0, tile_layout,	0x100, 1  ) /* foreground */
	GFXDECODE_ENTRY( REGION_GFX4, 0, sprite_layout,	0x000, 16 ) /* sprites */
GFXDECODE_END


/***************************************************************************
**
**  Machine Driver
**
***************************************************************************/

static MACHINE_DRIVER_START( marvins )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 3360000)	/* 3.36 MHz */
	MDRV_CPU_PROGRAM_MAP(marvins_cpuA_map,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_CPU_ADD(Z80, 3360000)	/* 3.36 MHz */
	MDRV_CPU_PROGRAM_MAP(marvins_cpuB_map,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_CPU_ADD(Z80, 4000000)	/* 4.0 MHz */
	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(sound_map,0)
	MDRV_CPU_IO_MAP(sound_portmap,0)
	MDRV_CPU_PERIODIC_INT(nmi_line_assert, 244)	// schematics show a separate 244Hz timer

	MDRV_INTERLEAVE(100)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60.606060)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256+32, 224)
	MDRV_SCREEN_VISIBLE_AREA(0, 255+32,0, 223)

	MDRV_GFXDECODE(marvins)
	MDRV_PALETTE_LENGTH((16+2)*16)

	MDRV_VIDEO_START(marvins)
	MDRV_VIDEO_UPDATE(marvins)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 2000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD(AY8910, 2000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD(NAMCO, 8000000/256)
	MDRV_SOUND_CONFIG(snkwave_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( vangrd2 )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", Z80, 3360000)	/* 3.36 MHz */
	MDRV_CPU_PROGRAM_MAP(madcrash_cpuA_map,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_CPU_ADD_TAG("sub", Z80, 3360000)	/* 3.36 MHz */
	MDRV_CPU_PROGRAM_MAP(madcrash_cpuB_map,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_CPU_ADD(Z80, 4000000)	/* 4.0 MHz */
	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(sound_map,0)
	MDRV_CPU_IO_MAP(sound_portmap,0)
	MDRV_CPU_PERIODIC_INT(nmi_line_assert, 244)

	MDRV_INTERLEAVE(100)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60.606060)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256+32, 224)
	MDRV_SCREEN_VISIBLE_AREA(0, 255+32,0, 223)

	MDRV_GFXDECODE(marvins)
	MDRV_PALETTE_LENGTH((16+2)*16)

	MDRV_VIDEO_START(marvins)
	MDRV_VIDEO_UPDATE(madcrash)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 2000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD(AY8910, 2000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD(NAMCO, 8000000/256)
	MDRV_SOUND_CONFIG(snkwave_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( madcrash )

	/* basic machine hardware */
	MDRV_IMPORT_FROM( vangrd2 )

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_VBLANK_INT_HACK(0, 0)

	MDRV_CPU_MODIFY("sub")
	MDRV_CPU_VBLANK_INT("main", snk_irq_BA)

	MDRV_INTERLEAVE(300)

	/* video hardware */
	MDRV_SCREEN_MODIFY("main")
	MDRV_SCREEN_VISIBLE_AREA(16, 16+256-1, 0, 0+216-1)
MACHINE_DRIVER_END


/***************************************************************************
**
**  ROM Loading
**
**  note:
**      Mad Crasher doesn't pass its internal checksum
**      Also, some of the background graphics look to be incorrect.
**
***************************************************************************/

ROM_START( marvins )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for CPUA code */
	ROM_LOAD( "pa1",   0x0000, 0x2000, CRC(0008d791) SHA1(6ffb174b2d680314f74efeef83da9f3ee3e0c753) )
	ROM_LOAD( "pa2",   0x2000, 0x2000, CRC(9457003c) SHA1(05ecd5c638a12163e2a65bdfcc09875618f792e1) )
	ROM_LOAD( "pa3",   0x4000, 0x2000, CRC(54c33ecb) SHA1(cfbf9ffc125fbc51f2abef180f36781f9e748bbd) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for CPUB code */
	ROM_LOAD( "pb1",   0x0000, 0x2000, CRC(3b6941a5) SHA1(9c29870196eaed87f34456fdb06bf7b69c8f489d) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 )	/* 64k for sound code */
	ROM_LOAD( "m1",    0x0000, 0x2000, CRC(2314c696) SHA1(1b84a0c82a4dcff648752f53aa1f0abf5357c5d1) )
	ROM_LOAD( "m2",    0x2000, 0x2000, CRC(74ba5799) SHA1(c278b0e5c4134f6077d4ae7b51e3c5cba28af1a8) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "s1",    0x0000, 0x2000, CRC(327f70f3) SHA1(078dcc6b4697617d4d833ccd59c6a543b2a88d9e) )	/* characters */

	ROM_REGION( 0x2000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "b1",    0x0000, 0x2000, CRC(e528bc60) SHA1(3365ac7cbc57739054bc11e68831be87c0c1a97a) )	/* background tiles */

	ROM_REGION( 0x2000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "b2",    0x0000, 0x2000, CRC(e528bc60) SHA1(3365ac7cbc57739054bc11e68831be87c0c1a97a) )	/* foreground tiles */

	ROM_REGION( 0x6000, REGION_GFX4, ROMREGION_DISPOSE )
	ROM_LOAD( "f3",    0x0000, 0x2000, CRC(e55c9b83) SHA1(04b0d99955e4b11820015b7721ac6399a3d5a829) )	/* sprites */
	ROM_LOAD( "f2",    0x2000, 0x2000, CRC(8fc2b081) SHA1(fb345965375cb62ec1b947d6c6d071380dc0f395) )
	ROM_LOAD( "f1",    0x4000, 0x2000, CRC(0bd6b4e5) SHA1(c56747ff2135db734f1b5f6c2906de5ac8f53bbc) )

	ROM_REGION( 0x0c00, REGION_PROMS, 0 )
	ROM_LOAD( "marvmaze.j1",  0x000, 0x400, CRC(92f5b06d) SHA1(97979ffb6fb065d9c99da43173180fefb2de1886) )
	ROM_LOAD( "marvmaze.j2",  0x400, 0x400, CRC(d2b25665) SHA1(b913b8b9c5ee0a29b5a115b2432c5706979059cf) )
	ROM_LOAD( "marvmaze.j3",  0x800, 0x400, CRC(df9e6005) SHA1(8f633f664c3f8e4f6ca94bee74a68c8fda8873e3) )
ROM_END

ROM_START( madcrash )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for CPUA code */
	ROM_LOAD( "p8",    0x0000, 0x2000, CRC(ecb2fdc9) SHA1(7dd79fbbe286a9f18ed2cae45b1bfab765e549a1) )
	ROM_LOAD( "p9",    0x2000, 0x2000, CRC(0a87df26) SHA1(327710452bdc5dbb931abc853957225814f224c5) )
	ROM_LOAD( "p10",   0x4000, 0x2000, CRC(6eb8a87c) SHA1(375377df22b331175aaf1f9eb8d8ad83e8e146f6) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for CPUB code */
	ROM_LOAD( "p4",   0x0000, 0x2000, CRC(5664d699) SHA1(5bfa57a0f8d718d522003da6513a70d7ca3a87a3) )
	ROM_LOAD( "p5",   0x2000, 0x2000, CRC(dea2865a) SHA1(0807281e35159ee29fbe2d1aa087b57804f1a14f) )
	ROM_LOAD( "p6",   0x4000, 0x2000, CRC(e25a9b9c) SHA1(26853611e3898907239e15f1a00f62290889f89b) )
	ROM_LOAD( "p7",   0x6000, 0x2000, CRC(55b14a36) SHA1(7d5566a6ba285af92ddf560efda60a79f1da84c2) )
	ROM_LOAD( "p3",   0x8000, 0x2000, CRC(e3c8c2cb) SHA1(b3e39eacd2609ff0fa0f511bff0fc83e6b3970d4) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 )	/* 64k for sound code */
	ROM_LOAD( "p1",   0x0000, 0x2000, CRC(2dcd036d) SHA1(4da42ab1e502fff57f5d5787df406289538fa484) )
	ROM_LOAD( "p2",   0x2000, 0x2000, CRC(cc30ae8b) SHA1(ffedc747b9e0b616a163ff8bb1def318e522585b) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "p13",    0x0000, 0x2000, CRC(48c4ade0) SHA1(3628abb4f425b8c9d8659c8e4082735168b0f3e9) )	/* characters */

	ROM_REGION( 0x2000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "p11",    0x0000, 0x2000, CRC(67174956) SHA1(65a921176294212971c748932a9010f45e1fb499) )	/* background tiles */

	ROM_REGION( 0x2000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "p12",    0x0000, 0x2000, CRC(085094c1) SHA1(5c5599d1ed7f8a717ada54bbd28383a22e09a8fe) )	/* foreground tiles */

	ROM_REGION( 0x6000, REGION_GFX4, ROMREGION_DISPOSE )
	ROM_LOAD( "p14",    0x0000, 0x2000, CRC(07e807bc) SHA1(f651d3a5394ced8e0a1b2be3aa52b3e5a5d84c37) )	/* sprites */
	ROM_LOAD( "p15",    0x2000, 0x2000, CRC(a74149d4) SHA1(e8011a8d4d1a98a0ffe67fc28ea9fa192ca80321) )
	ROM_LOAD( "p16",    0x4000, 0x2000, CRC(6153611a) SHA1(b352f92b233761122f74830e46913cc4df800259) )

	ROM_REGION( 0x0c00, REGION_PROMS, 0 )
	ROM_LOAD( "m3-prom.j3",  0x000, 0x400, CRC(d19e8a91) SHA1(b21fbdb8ed8d0b27c3ec78cf2e115624f69c67e0) )
	ROM_LOAD( "m2-prom.j4",  0x400, 0x400, CRC(9fc325af) SHA1(a180662f168ba001376f25f5d9205cb119c1ffee) )
	ROM_LOAD( "m1-prom.j5",  0x800, 0x400, CRC(07678443) SHA1(267951886d8b031dd633dc4823d9bd862a585437) )
ROM_END

ROM_START( vangrd2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "p1.9a",  0x0000, 0x2000, CRC(bc9eeca5) SHA1(5a737e0f0aa1a3a5296d1e1fec13b34aee970609) )
	ROM_LOAD( "p3.11a", 0x2000, 0x2000, CRC(3970f69d) SHA1(b0ef7494888804ab5b4002730fb0232a7fd6797b) )
	ROM_LOAD( "p2.12a", 0x4000, 0x2000, CRC(58b08b58) SHA1(eccc85191d678a0115a113002a43203afd857a5b) )
	ROM_LOAD( "p4.14a", 0x6000, 0x2000, CRC(a95f11ea) SHA1(8007efb4ad948c8768e474fc77134f3ce52da1d2) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "p5.4a", 0x0000, 0x2000, CRC(e4dfd0ba) SHA1(12d45ff147f3ea9c9e898c3831874cd7c1a071b7) )
	ROM_LOAD( "p6.6a", 0x2000, 0x2000, CRC(894ff00d) SHA1(1c66f327d8e94dc6ac386e11fcc5eb17c9081434) )
	ROM_LOAD( "p7.7a", 0x4000, 0x2000, CRC(40b4d069) SHA1(56c464bd055125ffc2da02d70137aa5efe5cd8f6) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 )	/* 64k for sound code */
	ROM_LOAD( "p8.6a", 0x0000, 0x2000, CRC(a3daa438) SHA1(4e659ac7e3ebaf85bc3ce5c9946fcf0af23083b4) )
	ROM_LOAD( "p9.8a", 0x2000, 0x2000, CRC(9345101a) SHA1(b99ad1c2a79df50b0a60fdd43ca466f6cb38445b) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "p15.1e", 0x0000, 0x2000, CRC(85718a41) SHA1(4c9aa1f8b229410414cd67bac8cb10a14bea12f4) )	/* characters */

	ROM_REGION( 0x2000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "p13.1a", 0x0000, 0x2000, CRC(912f22c6) SHA1(5042edc80b58f77b3576b5e6eb8c6460c8a35494) )	/* background tiles */

	ROM_REGION( 0x2000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "p9",     0x0000, 0x2000, CRC(7aa0b684) SHA1(d52670ec50b1a07d6c2c537f67922063deacdeea) )	/* foreground tiles */

	ROM_REGION( 0x6000, REGION_GFX4, ROMREGION_DISPOSE )
	ROM_LOAD( "p12.1kl", 0x0000, 0x2000, CRC(8658ea6c) SHA1(d5ea9be2c1776b11abc77c944a653eeb73b27fc8) )	/* sprites */
	ROM_LOAD( "p11.3kl", 0x2000, 0x2000, CRC(620cd4ec) SHA1(a2fcc3d24d0d3c7cc601620ae7a709f46b613c0f) )
	ROM_LOAD( "p10.4kl", 0x4000, 0x2000, CRC(5bfc04c0) SHA1(4eb152fdf39cb0024f71d5bdf1bfc79c2b8c2329) )

	ROM_REGION( 0x0c00, REGION_PROMS, 0 )
	ROM_LOAD( "mb7054.3j", 0x000, 0x400, CRC(506f659a) SHA1(766f1a0dd462eba64546c514004e6542e200d7c3) )
	ROM_LOAD( "mb7054.4j", 0x400, 0x400, CRC(222133ce) SHA1(109a63c8c44608a8ad9183e7b5d269765cc5f067) )
	ROM_LOAD( "mb7054.5j", 0x800, 0x400, CRC(2e21a79b) SHA1(1956377c799e0bbd127bf4fae016adc148efe007) )
ROM_END


/*******************************************************************************************/

static DRIVER_INIT( marvins )
{
	init_sound( 0x40 );
	snk_gamegroup = 0;
}

static DRIVER_INIT( madcrash )
{
/*
    The following lines patch out the ROM test (which fails - probably
    because of bit rot, so the rest of the test mode (what little there
    is) can be explored.

    UINT8 *mem = memory_region(REGION_CPU1);
    mem[0x3a5d] = 0; mem[0x3a5e] = 0; mem[0x3a5f] = 0;
*/
	init_sound( 0x20 );
	snk_gamegroup = 1;
	snk_irq_delay = 1700;
}

static DRIVER_INIT( vangrd2 )
{
	init_sound( 0x20 );
	snk_gamegroup = 2;
}


GAME( 1983, marvins,  0, marvins,  marvins,  marvins,  ROT270, "SNK", "Marvin's Maze", GAME_NO_COCKTAIL )
GAME( 1984, madcrash, 0, madcrash, madcrash, madcrash, ROT0,   "SNK", "Mad Crasher", GAME_IMPERFECT_GRAPHICS )
GAME( 1984, vangrd2,  0, vangrd2,  vangrd2,  vangrd2,  ROT270, "SNK", "Vanguard II", GAME_NO_COCKTAIL )
