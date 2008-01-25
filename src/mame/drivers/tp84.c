/***************************************************************************

Time Pilot 84  (c) 1984 Konami

driver by Marc Lafontaine

TODO:
- the slave CPU multiplexes sprites. We are cheating now, and reading them
  from somewhere else.


The schematics are available on the net.

There is 3 CPU for this game.
 Two 68A09E for the game.
 A Z80A for the sound

As I understand it, the second 6809 is for displaying
 the sprites. If we do not emulate him, all work well, except
 that the player cannot die.
 Address 57ff must read 0 to pass the RAM test if the second CPU
 is not emulated.


---- Master 6809 ------

Write
 2000-27ff MAFR Watch dog ?
 2800      COL0 a register that index the colors Proms
 3000      reset IRQ
 3001      OUT2  Coin Counter 2
 3002      OUT1  Coin Counter 1
 3003      MUT
 3004      HREV  Flip Screen X
 3005      VREV  Flip Screen Y
 3006      -
 3007      GMED
 3800      SON   Sound on
 3A00      SDA   Sound data
 3C00      SHF0 SHF1 J2 J3 J4 J5 J6 J7  background Y position
 3E00      L0 - L7                      background X position

Read:
 2800      in0  Buttons 1
 2820      in1  Buttons 2
 2840      in2  Buttons 3
 2860      in3  Dip switches 1
 3000      in4  Dip switches 2
 3800      in5  Dip switches 3 (not used)

Read/Write
 4000-47ff Char ram, 2 pages
 4800-4fff Background character ram, 2 pages
 5000-57ff Ram (Common for the Master and Slave 6809)  0x5000-0x517f sprites data
 6000-ffff Rom (only from $8000 to $ffff is used in this game)


------ Slave 6809 --------
 0000-1fff SAFR Watch dog ?
 2000      seem to be the beam position (if always 0, no player collision is detected)
 4000      enable or reset IRQ
 6000-67ff DRA
 8000-87ff Ram (Common for the Master and Slave 6809)
 E000-ffff Rom


------ Sound CPU (Z80) -----
There are 3 or 4 76489AN chips driven by the Z80

0000-1fff Rom program (A6)
2000-3fff Rom Program (A4) (not used or missing?)
4000-43ff Ram
6000-7fff Sound data in
8000-9fff Timer
A000-Bfff Filters
C000      Store Data that will go to one of the 76489AN
C001      76489 #1 trigger
C002      76489 #2 (optional) trigger
C003      76489 #3 trigger
C004      76489 #4 trigger

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "sound/sn76496.h"
#include "sound/flt_rc.h"


extern UINT8 *tp84_videoram2, *tp84_colorram2;

extern WRITE8_HANDLER( tp84_videoram_w );
extern WRITE8_HANDLER( tp84_colorram_w );
extern WRITE8_HANDLER( tp84_videoram2_w );
extern WRITE8_HANDLER( tp84_colorram2_w );
extern WRITE8_HANDLER( tp84_scroll_x_w );
extern WRITE8_HANDLER( tp84_scroll_y_w );
extern WRITE8_HANDLER( tp84_flipscreen_x_w );
extern WRITE8_HANDLER( tp84_flipscreen_y_w );
extern WRITE8_HANDLER( tp84_col0_w );
extern READ8_HANDLER( tp84_scanline_r );

extern PALETTE_INIT( tp84 );
extern VIDEO_START( tp84 );
extern VIDEO_UPDATE( tp84 );

extern INTERRUPT_GEN( tp84_6809_interrupt );


static UINT8 *sharedram;

static READ8_HANDLER( sharedram_r )
{
	return sharedram[offset];
}

static WRITE8_HANDLER( sharedram_w )
{
	sharedram[offset] = data;
}



static READ8_HANDLER( tp84_sh_timer_r )
{
	/* main xtal 14.318MHz, divided by 4 to get the CPU clock, further */
	/* divided by 2048 to get this timer */
	/* (divide by (2048/2), and not 1024, because the CPU cycle counter is */
	/* incremented every other state change of the clock) */
	return (activecpu_gettotalcycles() / (2048/2)) & 0x0f;
}

static WRITE8_HANDLER( tp84_filter_w )
{
	int C;

	/* 76489 #0 */
	C = 0;
	if (offset & 0x008) C +=  47000;	/*  47000pF = 0.047uF */
	if (offset & 0x010) C += 470000;	/* 470000pF = 0.47uF */
	filter_rc_set_RC(0,FLT_RC_LOWPASS,1000,2200,1000,CAP_P(C));

	/* 76489 #1 (optional) */
	C = 0;
	if (offset & 0x020) C +=  47000;	/*  47000pF = 0.047uF */
	if (offset & 0x040) C += 470000;	/* 470000pF = 0.47uF */
//  filter_rc_set_RC(1,1000,2200,1000,C);

	/* 76489 #2 */
	C = 0;
	if (offset & 0x080) C += 470000;	/* 470000pF = 0.47uF */
	filter_rc_set_RC(1,FLT_RC_LOWPASS,1000,2200,1000,CAP_P(C));

	/* 76489 #3 */
	C = 0;
	if (offset & 0x100) C += 470000;	/* 470000pF = 0.47uF */
	filter_rc_set_RC(2,FLT_RC_LOWPASS,1000,2200,1000,CAP_P(C));
}

static WRITE8_HANDLER( tp84_sh_irqtrigger_w )
{
	cpunum_set_input_line_and_vector(Machine, 2,0,HOLD_LINE,0xff);
}



/* CPU 1 read addresses */
static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x2800, 0x2800) AM_READ(input_port_0_r)
	AM_RANGE(0x2820, 0x2820) AM_READ(input_port_1_r)
	AM_RANGE(0x2840, 0x2840) AM_READ(input_port_2_r)
	AM_RANGE(0x2860, 0x2860) AM_READ(input_port_3_r)
	AM_RANGE(0x3000, 0x3000) AM_READ(input_port_4_r)
	AM_RANGE(0x4000, 0x4fff) AM_READ(MRA8_RAM)
	AM_RANGE(0x5000, 0x57ff) AM_READ(sharedram_r)
	AM_RANGE(0x8000, 0xffff) AM_READ(MRA8_ROM)
ADDRESS_MAP_END

/* CPU 1 write addresses */
static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x2000, 0x2000) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x2800, 0x2800) AM_WRITE(tp84_col0_w)
	AM_RANGE(0x3000, 0x3000) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x3004, 0x3004) AM_WRITE(tp84_flipscreen_x_w)
	AM_RANGE(0x3005, 0x3005) AM_WRITE(tp84_flipscreen_y_w)
	AM_RANGE(0x3800, 0x3800) AM_WRITE(tp84_sh_irqtrigger_w)
	AM_RANGE(0x3a00, 0x3a00) AM_WRITE(soundlatch_w)
	AM_RANGE(0x3c00, 0x3c00) AM_WRITE(tp84_scroll_x_w)
	AM_RANGE(0x3e00, 0x3e00) AM_WRITE(tp84_scroll_y_w)
	AM_RANGE(0x4000, 0x43ff) AM_WRITE(tp84_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x4400, 0x47ff) AM_WRITE(tp84_videoram2_w) AM_BASE(&tp84_videoram2)
	AM_RANGE(0x4800, 0x4bff) AM_WRITE(tp84_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x4c00, 0x4fff) AM_WRITE(tp84_colorram2_w) AM_BASE(&tp84_colorram2)
	AM_RANGE(0x5000, 0x57ff) AM_WRITE(sharedram_w) AM_BASE(&sharedram)
	AM_RANGE(0x8000, 0xffff) AM_WRITE(MWA8_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tp84b_cpu1_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_WRITE(tp84_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x0400, 0x07ff) AM_RAM AM_WRITE(tp84_videoram2_w) AM_BASE(&tp84_videoram2)
	AM_RANGE(0x0800, 0x0bff) AM_RAM AM_WRITE(tp84_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x0c00, 0x0fff) AM_RAM AM_WRITE(tp84_colorram2_w) AM_BASE(&tp84_colorram2)
	AM_RANGE(0x1000, 0x17ff) AM_READWRITE(sharedram_r, sharedram_w) AM_BASE(&sharedram)
	AM_RANGE(0x1800, 0x1800) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x1a00, 0x1a00) AM_READWRITE(input_port_0_r, tp84_col0_w)
	AM_RANGE(0x1a20, 0x1a20) AM_READ(input_port_1_r)
	AM_RANGE(0x1a40, 0x1a40) AM_READ(input_port_2_r)
	AM_RANGE(0x1a60, 0x1a60) AM_READ(input_port_3_r)
	AM_RANGE(0x1c00, 0x1c00) AM_READWRITE(input_port_4_r, MWA8_NOP)
	AM_RANGE(0x1c04, 0x1c04) AM_WRITE(tp84_flipscreen_x_w)
	AM_RANGE(0x1c05, 0x1c05) AM_WRITE(tp84_flipscreen_y_w)
	AM_RANGE(0x1e00, 0x1e00) AM_WRITE(tp84_sh_irqtrigger_w)
	AM_RANGE(0x1e80, 0x1e80) AM_WRITE(soundlatch_w)
	AM_RANGE(0x1f00, 0x1f00) AM_WRITE(tp84_scroll_x_w)
	AM_RANGE(0x1f80, 0x1f80) AM_WRITE(tp84_scroll_y_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


/* CPU 2 read addresses */
static ADDRESS_MAP_START( readmem_cpu2, ADDRESS_SPACE_PROGRAM, 8 )
//  AM_RANGE(0x0000, 0x0000) AM_READ(MRA8_RAM)
	AM_RANGE(0x2000, 0x2000) AM_READ(tp84_scanline_r) /* beam position */
	AM_RANGE(0x6000, 0x67ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x8000, 0x87ff) AM_READ(sharedram_r)
	AM_RANGE(0xe000, 0xffff) AM_READ(MRA8_ROM)
ADDRESS_MAP_END

/* CPU 2 write addresses */
static ADDRESS_MAP_START( writemem_cpu2, ADDRESS_SPACE_PROGRAM, 8 )
//  AM_RANGE(0x0000, 0x0000) AM_WRITE(MWA8_RAM) /* Watch dog ?*/
	AM_RANGE(0x4000, 0x4000) AM_WRITE(interrupt_enable_w) /* IRQ enable */
	AM_RANGE(0x6000, 0x679f) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x67a0, 0x67ff) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)	/* REAL (multiplexed) */
	AM_RANGE(0x8000, 0x87ff) AM_WRITE(sharedram_w)
	AM_RANGE(0xe000, 0xffff) AM_WRITE(MWA8_ROM)
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x4000, 0x43ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x6000, 0x6000) AM_READ(soundlatch_r)
	AM_RANGE(0x8000, 0x8000) AM_READ(tp84_sh_timer_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x4000, 0x43ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xa000, 0xa1ff) AM_WRITE(tp84_filter_w)
	AM_RANGE(0xc000, 0xc000) AM_WRITE(MWA8_NOP)
	AM_RANGE(0xc001, 0xc001) AM_WRITE(SN76496_0_w)
	AM_RANGE(0xc003, 0xc003) AM_WRITE(SN76496_1_w)
	AM_RANGE(0xc004, 0xc004) AM_WRITE(SN76496_2_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( tp84 )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSW0 */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "Invalid" )

	PORT_START      /* DSW1 */
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x18, "10000 and every 50000" )
	PORT_DIPSETTING(    0x10, "20000 and every 60000" )
	PORT_DIPSETTING(    0x08, "30000 and every 70000" )
	PORT_DIPSETTING(    0x00, "40000 and every 80000" )
	PORT_DIPNAME( 0x60, 0x20, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tp84a )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSW0 */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "Invalid" )

	PORT_START      /* DSW1 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x18, "10K 50K+" )
	PORT_DIPSETTING(    0x10, "20K 60K+" )
	PORT_DIPSETTING(    0x08, "30K 70K+" )
	PORT_DIPSETTING(    0x00, "40K 80K+" )
	PORT_DIPNAME( 0x60, 0x20, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{  0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};
static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4 ,0 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8
};

static GFXDECODE_START( tp84 )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,        0, 64*8 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, spritelayout, 64*4*8, 16*8 )
GFXDECODE_END



static MACHINE_DRIVER_START( tp84 )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("cpu1",M6809, XTAL_18_432MHz/12) /* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)

	MDRV_CPU_ADD(M6809, XTAL_18_432MHz/12)	/* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(readmem_cpu2,writemem_cpu2)
	MDRV_CPU_VBLANK_INT(tp84_6809_interrupt,256)

	MDRV_CPU_ADD(Z80,XTAL_14_31818MHz/4) /* verified on pcb */
	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(sound_readmem,sound_writemem)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_INTERLEAVE(100)	/* 100 CPU slices per frame - an high value to ensure proper */
							/* synchronization of the CPUs */

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(tp84)
	MDRV_PALETTE_LENGTH(256)
	MDRV_COLORTABLE_LENGTH(4096)

	MDRV_PALETTE_INIT(tp84)
	MDRV_VIDEO_START(tp84)
	MDRV_VIDEO_UPDATE(tp84)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(SN76489A, XTAL_14_31818MHz/8) /* verified on pcb */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "filter1", 0.75)

	MDRV_SOUND_ADD(SN76489A, XTAL_14_31818MHz/8) /* verified on pcb */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "filter2", 0.75)

	MDRV_SOUND_ADD(SN76489A, XTAL_14_31818MHz/8) /* verified on pcb */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "filter3", 0.75)

	MDRV_SOUND_ADD_TAG("filter1", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_SOUND_ADD_TAG("filter2", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_SOUND_ADD_TAG("filter3", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( tp84b )
	MDRV_IMPORT_FROM(tp84)
	MDRV_CPU_MODIFY("cpu1")
	MDRV_CPU_PROGRAM_MAP(tp84b_cpu1_map,0)
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tp84 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "tp84_7j.bin",  0x8000, 0x2000, CRC(605f61c7) SHA1(6848ef35ec7f92cccefb0fb2de42c4b0e9ec476f) )
	ROM_LOAD( "tp84_8j.bin",  0xa000, 0x2000, CRC(4b4629a4) SHA1(f3bb1ee66c9e47d050370ac9ca74f3020cb9cfa3) )
	ROM_LOAD( "tp84_9j.bin",  0xc000, 0x2000, CRC(dbd5333b) SHA1(65dee1fd4c940a5423d57cb55a7f2ad89c59c5c6) )
	ROM_LOAD( "tp84_10j.bin", 0xe000, 0x2000, CRC(a45237c4) SHA1(896e31c59aedf1c7e73e6f30fbe78cc020b457ab) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for the second CPU */
	ROM_LOAD( "tp84_10d.bin", 0xe000, 0x2000, CRC(36462ff1) SHA1(118a1b46ee01a583e6cf39af59b073321c76dbff) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 )	/* 64k for code of sound cpu Z80 */
	ROM_LOAD( "tp84s_6a.bin", 0x0000, 0x2000, CRC(c44414da) SHA1(981289f5bdf7dc1348f4ca547ac933ef503b6588) )

	ROM_REGION( 0x4000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "tp84_2j.bin",  0x0000, 0x2000, CRC(05c7508f) SHA1(1a3c7cd47ad34e37a7b0f3014e10c055cbb2b559) ) /* chars */
	ROM_LOAD( "tp84_1j.bin",  0x2000, 0x2000, CRC(498d90b7) SHA1(6975f3a1603b14132aab58329195a4845a6e28bb) )

	ROM_REGION( 0x8000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "tp84_12a.bin", 0x0000, 0x2000, CRC(cd682f30) SHA1(6f48d3efc53d63171ec655e64b225412de1374e4) ) /* sprites */
	ROM_LOAD( "tp84_13a.bin", 0x2000, 0x2000, CRC(888d4bd6) SHA1(7e2dde080bb614709561431a81b0490b2aaa42a9) )
	ROM_LOAD( "tp84_14a.bin", 0x4000, 0x2000, CRC(9a220b39) SHA1(792aaa4daedc8eb807d5a66d87da4641739b1660) )
	ROM_LOAD( "tp84_15a.bin", 0x6000, 0x2000, CRC(fac98397) SHA1(d90f99b19ab3cddfdfd37a273fb437be098088bc) )

	ROM_REGION( 0x0500, REGION_PROMS, 0 )
	ROM_LOAD( "tp84_2c.bin",  0x0000, 0x0100, CRC(d737eaba) SHA1(e39026f87f5b995cf4a38b5d3d3fee7561762ae6) ) /* palette red component */
	ROM_LOAD( "tp84_2d.bin",  0x0100, 0x0100, CRC(2f6a9a2a) SHA1(f09d8b92c7f9bf046cdd815c5282d0510e61b6e0) ) /* palette green component */
	ROM_LOAD( "tp84_1e.bin",  0x0200, 0x0100, CRC(2e21329b) SHA1(9ba8af294dbd6f3a5d039c74a56e0605a913c037) ) /* palette blue component */
	ROM_LOAD( "tp84_1f.bin",  0x0300, 0x0100, CRC(61d2d398) SHA1(3f74ad733b07b6a31cf9d4956d171eb9253dd6bf) ) /* char lookup table */
	ROM_LOAD( "tp84_16c.bin", 0x0400, 0x0100, CRC(13c4e198) SHA1(42ab23206be99e840bd9c52cefa175c12fac8e5b) ) /* sprite lookup table */
ROM_END

ROM_START( tp84a )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "tp84_7j.bin",  0x8000, 0x2000, CRC(605f61c7) SHA1(6848ef35ec7f92cccefb0fb2de42c4b0e9ec476f) )
	ROM_LOAD( "f05",          0xa000, 0x2000, CRC(e97d5093) SHA1(c76c119574d19d2ac10e6987150744542803ef5b) )
	ROM_LOAD( "tp84_9j.bin",  0xc000, 0x2000, CRC(dbd5333b) SHA1(65dee1fd4c940a5423d57cb55a7f2ad89c59c5c6) )
	ROM_LOAD( "f07",          0xe000, 0x2000, CRC(8fbdb4ef) SHA1(e615c4d9964ab00f6776147c54925b4b6100b360) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for the second CPU */
	ROM_LOAD( "tp84_10d.bin", 0xe000, 0x2000, CRC(36462ff1) SHA1(118a1b46ee01a583e6cf39af59b073321c76dbff) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 )	/* 64k for code of sound cpu Z80 */
	ROM_LOAD( "tp84s_6a.bin", 0x0000, 0x2000, CRC(c44414da) SHA1(981289f5bdf7dc1348f4ca547ac933ef503b6588) )

	ROM_REGION( 0x4000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "tp84_2j.bin",  0x0000, 0x2000, CRC(05c7508f) SHA1(1a3c7cd47ad34e37a7b0f3014e10c055cbb2b559) ) /* chars */
	ROM_LOAD( "tp84_1j.bin",  0x2000, 0x2000, CRC(498d90b7) SHA1(6975f3a1603b14132aab58329195a4845a6e28bb) )

	ROM_REGION( 0x8000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "tp84_12a.bin", 0x0000, 0x2000, CRC(cd682f30) SHA1(6f48d3efc53d63171ec655e64b225412de1374e4) ) /* sprites */
	ROM_LOAD( "tp84_13a.bin", 0x2000, 0x2000, CRC(888d4bd6) SHA1(7e2dde080bb614709561431a81b0490b2aaa42a9) )
	ROM_LOAD( "tp84_14a.bin", 0x4000, 0x2000, CRC(9a220b39) SHA1(792aaa4daedc8eb807d5a66d87da4641739b1660) )
	ROM_LOAD( "tp84_15a.bin", 0x6000, 0x2000, CRC(fac98397) SHA1(d90f99b19ab3cddfdfd37a273fb437be098088bc) )

	ROM_REGION( 0x0500, REGION_PROMS, 0 )
	ROM_LOAD( "tp84_2c.bin",  0x0000, 0x0100, CRC(d737eaba) SHA1(e39026f87f5b995cf4a38b5d3d3fee7561762ae6) ) /* palette red component */
	ROM_LOAD( "tp84_2d.bin",  0x0100, 0x0100, CRC(2f6a9a2a) SHA1(f09d8b92c7f9bf046cdd815c5282d0510e61b6e0) ) /* palette green component */
	ROM_LOAD( "tp84_1e.bin",  0x0200, 0x0100, CRC(2e21329b) SHA1(9ba8af294dbd6f3a5d039c74a56e0605a913c037) ) /* palette blue component */
	ROM_LOAD( "tp84_1f.bin",  0x0300, 0x0100, CRC(61d2d398) SHA1(3f74ad733b07b6a31cf9d4956d171eb9253dd6bf) ) /* char lookup table */
	ROM_LOAD( "tp84_16c.bin", 0x0400, 0x0100, CRC(13c4e198) SHA1(42ab23206be99e840bd9c52cefa175c12fac8e5b) ) /* sprite lookup table */
ROM_END

ROM_START( tp84b )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	/* 0x6000 - 0x7fff space for diagnostic rom */
	ROM_LOAD( "388j05.8j",   0x8000, 0x4000, CRC(a59e2fda) SHA1(7d776d5d3fcfbe81d42580cfe93614dc4618a440) )
	ROM_LOAD( "388j07.10j",  0xc000, 0x4000, CRC(d25d18e6) SHA1(043f515cc66f6af004be81d6a6b5a92b553107ff) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for the second CPU */
	ROM_LOAD( "388j08.10d", 0xe000, 0x2000, CRC(2aea6b42) SHA1(58c3b4852f22a766f440b98904b73c00a31eae01) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 )	/* 64k for code of sound cpu Z80 */
	ROM_LOAD( "388j13.6a", 0x0000, 0x2000, CRC(c44414da) SHA1(981289f5bdf7dc1348f4ca547ac933ef503b6588) )

	ROM_REGION( 0x4000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "388j02.2j",  0x0000, 0x4000, CRC(e1225f53) SHA1(59d07dc4faafc82999e9716f0bba1cb7350c03e3) ) /* chars */

	ROM_REGION( 0x8000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "388j09.12a", 0x0000, 0x4000, CRC(aec90936) SHA1(3420c24bbedb140cb20fdaf51acbe9493830b64a) ) /* sprites */
	ROM_LOAD( "388j11.14a", 0x4000, 0x4000, CRC(29257f03) SHA1(ebbb980bd226e8ada7e517e92487a32bfbc82f91) )

	ROM_REGION( 0x0500, REGION_PROMS, 0 )
	ROM_LOAD( "388j14.2c",  0x0000, 0x0100, CRC(d737eaba) SHA1(e39026f87f5b995cf4a38b5d3d3fee7561762ae6) ) /* palette red component */
	ROM_LOAD( "388j15.2d",  0x0100, 0x0100, CRC(2f6a9a2a) SHA1(f09d8b92c7f9bf046cdd815c5282d0510e61b6e0) ) /* palette green component */
	ROM_LOAD( "388j16.1e",  0x0200, 0x0100, CRC(2e21329b) SHA1(9ba8af294dbd6f3a5d039c74a56e0605a913c037) ) /* palette blue component */
	ROM_LOAD( "388j18.1f",  0x0300, 0x0100, CRC(61d2d398) SHA1(3f74ad733b07b6a31cf9d4956d171eb9253dd6bf) ) /* char lookup table */
	ROM_LOAD( "388j17.16c", 0x0400, 0x0100, CRC(13c4e198) SHA1(42ab23206be99e840bd9c52cefa175c12fac8e5b) ) /* sprite lookup table */
ROM_END


GAME( 1984, tp84,  0,    tp84,  tp84, 0, ROT90, "Konami", "Time Pilot '84 (set 1)", 0 )
GAME( 1984, tp84a, tp84, tp84,  tp84a,0, ROT90, "Konami", "Time Pilot '84 (set 2)", 0 )
GAME( 1984, tp84b, tp84, tp84b, tp84, 0, ROT90, "Konami", "Time Pilot '84 (set 3)", 0 )
