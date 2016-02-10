// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
/******************************************************************

    Double Dragon 3                     Technos Japan Corp 1990
    The Combatribes                     Technos Japan Corp 1990
    WWF WrestleFest                     Technos Japan Corp 1991

    Notes:

    Both games have original and bootleg versions supported.
    Double Dragon 3 bootleg has some misplaced graphics, but I
    think this is how the real thing would look.
    Combatribes has sprite lag but it seems to be caused by poor
    programming and I think the original does the same.



Double Dragon 3 PCB Layout
--------------------------

TA-0030-P1-03 (early version with EPROMs)
TA-0030-P1-04 (later version with MASKROMs)
|----------------------------------------------|
|VOL M51516 YM3012 YM2151     3.579545MHz IC15 |
|     MB3615 1.056MHz   Z80               IC14|-|
|     MB3615  M6295      IC43                 | |
|                                6116     IC13| |
|           IC74         6116 |-------|   IC12| |
|           IC73         28MHz|TECHNOS|       | |
|                        6116 |TJ-003 |   IC11| |
|J                            |       |   IC10|-|
|A                       6116 |-------|        |
|M                       30.IC38          IC9  |
|M  DSW1                      |-------|   IC8 |-|
|A                            |TECHNOS|       | |
|   DSW2                      |TJ-002 |   IC7 | |
|             6116            |       |   IC6 | |
|             6116            |-------|       | |
|      IC79 IC80                6264      IC5 | |
|CN4          6264              6264      IC4 |-|
|    68000    6264        20MHz                |
|----------------------------------------------|
Notes:
6264 - 8k x8 SRAM
6116 - 2k x8 SRAM
68000 clock - 10.000MHz [20/2]
Z80 clock - 3.579545MHz
M6295 clock - 1.056MHz. Pin 7 HIGH
YM2151 clock - 3.579545MHz
VSync - 57.4446Hz
HSync - 15.6250kHz
CN4 - connector for extra controls

IC4 to IC15 not populated
IC79 - 1M EPROM (27C010)
IC80 - 2M EPROM (27C020)
IC73/74 - 2M EPROM (27C020)
IC43 - 512k EPROM (27C512)
30.IC38 - MB7114 / 82S129 Bipolar PROM


ROM Board (only used on earlier version -03 main board)
---------
TA-0028-P2-10
|------------------------|
|IC33                    |
|IC34                   |-|
|IC35    IC19     IC3   | |
|IC36    IC20     IC4   | |
|IC37    IC21     IC5   | |
|        IC22     IC6   | |
|IC38    IC23     IC7   | |
|IC39             IC8   |-|
|IC40    IC24            |
|IC41    IC25            |
|IC42    IC26           |-|
|IC43    IC27           | |
|        IC28           | |
|IC44                   | |
|IC45    IC29     IC13  | |
|IC46    IC30     IC14  | |
|IC47    IC31     IC15  |-|
|IC48    IC32     IC16   |
|------------------------|
Notes:

ROMs (All ROMs are 27C010 EPROM. - means not populated)
      Label   Location
      ----------------
      25      IC3
      26      IC4
      27      IC5
      28      IC6
      30A9-0  IC7
      -       IC8
      30      IC13
      31      IC14
      -       IC15
      -       IC16
      16      IC19
      17      IC20
      18      IC21
      19      IC22
      30A10-0 IC23
      -       IC24
      21      IC25
      22      IC26
      -       IC27
      -       IC28
      23      IC29
      24      IC30
      -       IC31
      -       IC32
      4       IC33
      5       IC34
      6       IC35
      7       IC36
      30A11-0 IC37
      -       IC38
      9       IC39
      10      IC40
      11      IC41
      12      IC42
      30A12-0 IC43
      -       IC44
      14      IC45
      15      IC46
      -       IC47
      -       IC48






 Wrestlefest
 Technos 1991

 TA-0031
                                                         68000-12
  31J0_IC1  6264 6264 31A14-2 31A13-2 6264 6264 31A12-0  24MHz
  31J1_IC2
                   TJ-002          TJ-004
                                   6264                   SW1
                   28MHz                                  SW2
                                                          SW3
                                             61C16-35
                        61C16-35             61C16-35
  31J2_IC8
  31J3_IC9
  31J4_IC10
  31J5_IC11
  31J6_IC12
  31J7_IC13
  31J8_IC14    TJ-003                31A11-2  M6295   31J10_IC73
  31J9_IC15    61C16-35 61C16-35     Z80      YM2151


  Clock Crystals:

  X1 - 28.000 MHz
  X2 - 3.579545 MHz (for Z80)
  X3 - 24.000 MHz (for 68000)
  X4 - 1.056 MHz (not used, initially intended for OKI6295?)


  The mask roms at IC1 and IC2 have the same pinouts as a MX27C4100 or M27C400
  except pin 1 is not A17 but instead not used (not connected).




******************************************************************/

/*

    TODO:

    - coin counters/lockouts

*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "includes/ddragon3.h"


/*************************************
 *
 *  Read/Write Handlers
 *
 *************************************/

WRITE8_MEMBER(ddragon3_state::oki_bankswitch_w)
{
	m_oki->set_bank_base((data & 1) * 0x40000);
}

WRITE16_MEMBER(wwfwfest_state::wwfwfest_soundwrite)
{
	soundlatch_byte_w(space,1,data & 0xff);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE );
}


WRITE16_MEMBER(ddragon3_state::ddragon3_io_w)
{
	COMBINE_DATA(&m_io_reg[offset]);

	switch (offset)
	{
		case 0:
			m_vreg = m_io_reg[0];
			break;

		case 1: /* soundlatch_byte_w */
			soundlatch_byte_w(space, 1, m_io_reg[1] & 0xff);
			m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE );
		break;

		case 2:
			/*  this gets written to on startup and at the end of IRQ6
			**  possibly trigger IRQ on sound CPU
			*/
			m_maincpu->set_input_line(6, CLEAR_LINE);
			break;

		case 3:
			/*  this gets written to on startup,
			**  and at the end of IRQ5 (input port read) */
			m_maincpu->set_input_line(5, CLEAR_LINE);
			break;

		case 4:
			/* this gets written to at the end of IRQ6 only */
			m_maincpu->set_input_line(6, CLEAR_LINE);
			break;

		default:
			logerror("OUTPUT 1400[%02x] %08x, pc=%06x \n", offset, (unsigned)data, space.device().safe_pc() );
			break;
	}
}


WRITE16_MEMBER(wwfwfest_state::wwfwfest_irq_ack_w)
{
	if (offset == 0)
		m_maincpu->set_input_line(3, CLEAR_LINE);

	else
		m_maincpu->set_input_line(2, CLEAR_LINE);
}

WRITE16_MEMBER(wwfwfest_state::wwfwfest_flipscreen_w)
{
	flip_screen_set(data&1);
}

/*- Palette Reads/Writes - A5 and A6 are not connected */

READ16_MEMBER(wwfwfest_state::wwfwfest_paletteram_r)
{
	offset = (offset & 0x000f) | (offset & 0x7fc0) >> 2;
	return m_paletteram[offset];
}

WRITE16_MEMBER(wwfwfest_state::wwfwfest_paletteram_w)
{
	offset = (offset & 0x000f) | (offset & 0x7fc0) >> 2;
	m_palette->write(space, offset, data, mem_mask);
}

/*- Priority Control -*/


WRITE8_MEMBER(wwfwfest_state::wwfwfest_priority_w)
{
	m_pri = data;
}




/* DIPs are spread across the other input ports */
CUSTOM_INPUT_MEMBER(wwfwfest_state::dsw_3f_r)
{
	const char *tag = (const char *)param;
	return ioport(tag)->read() & 0x3f;
}

CUSTOM_INPUT_MEMBER(wwfwfest_state::dsw_c0_r)
{
	const char *tag = (const char *)param;
	return (ioport(tag)->read() & 0xc0) >> 6;
}



/*************************************
 *
 *  Memory Maps
 *
 *************************************/

static ADDRESS_MAP_START( ddragon3_map, AS_PROGRAM, 16, ddragon3_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x080fff) AM_RAM_WRITE(ddragon3_fg_videoram_w) AM_SHARE("fg_videoram") /* Foreground (32x32 Tiles - 4 by per tile) */
	AM_RANGE(0x082000, 0x0827ff) AM_RAM_WRITE(ddragon3_bg_videoram_w) AM_SHARE("bg_videoram") /* Background (32x32 Tiles - 2 by per tile) */
	AM_RANGE(0x0c0000, 0x0c000f) AM_WRITE(ddragon3_scroll_w)
	AM_RANGE(0x100000, 0x100001) AM_READ_PORT("P1_P2")
	AM_RANGE(0x100002, 0x100003) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x100004, 0x100005) AM_READ_PORT("DSW")
	AM_RANGE(0x100006, 0x100007) AM_READ_PORT("P3")
	AM_RANGE(0x100000, 0x10000f) AM_WRITE(ddragon3_io_w)
	AM_RANGE(0x140000, 0x1405ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette") /* Palette RAM */
	AM_RANGE(0x180000, 0x180fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x1c0000, 0x1c3fff) AM_RAM /* working RAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( dd3b_map, AS_PROGRAM, 16, ddragon3_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x080fff) AM_RAM_WRITE(ddragon3_fg_videoram_w) AM_SHARE("fg_videoram") /* Foreground (32x32 Tiles - 4 by per tile) */
	AM_RANGE(0x081000, 0x081fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x082000, 0x0827ff) AM_RAM_WRITE(ddragon3_bg_videoram_w) AM_SHARE("bg_videoram") /* Background (32x32 Tiles - 2 by per tile) */
	AM_RANGE(0x0c0000, 0x0c000f) AM_WRITE(ddragon3_scroll_w)
	AM_RANGE(0x100000, 0x1005ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette") /* Palette RAM */
	AM_RANGE(0x140000, 0x14000f) AM_WRITE(ddragon3_io_w)
	AM_RANGE(0x180000, 0x180001) AM_READ_PORT("IN0")
	AM_RANGE(0x180002, 0x180003) AM_READ_PORT("IN1")
	AM_RANGE(0x180004, 0x180005) AM_READ_PORT("IN2")
	AM_RANGE(0x180006, 0x180007) AM_READ_PORT("IN3")
	AM_RANGE(0x1c0000, 0x1c3fff) AM_RAM /* working RAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( ctribe_map, AS_PROGRAM, 16, ddragon3_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x080fff) AM_RAM_WRITE(ddragon3_fg_videoram_w) AM_SHARE("fg_videoram") /* Foreground (32x32 Tiles - 4 by per tile) */
	AM_RANGE(0x081000, 0x081fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x082000, 0x0827ff) AM_RAM_WRITE(ddragon3_bg_videoram_w) AM_SHARE("bg_videoram") /* Background (32x32 Tiles - 2 by per tile) */
	AM_RANGE(0x082800, 0x082fff) AM_RAM
	AM_RANGE(0x0c0000, 0x0c000f) AM_READWRITE(ddragon3_scroll_r, ddragon3_scroll_w)
	AM_RANGE(0x100000, 0x1005ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette") /* Palette RAM */
	AM_RANGE(0x140000, 0x14000f) AM_WRITE(ddragon3_io_w)
	AM_RANGE(0x180000, 0x180001) AM_READ_PORT("IN0")
	AM_RANGE(0x180002, 0x180003) AM_READ_PORT("IN1")
	AM_RANGE(0x180004, 0x180005) AM_READ_PORT("IN2")
	AM_RANGE(0x180006, 0x180007) AM_READ_PORT("IN3")
	AM_RANGE(0x1c0000, 0x1c3fff) AM_RAM /* working RAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, wwfwfest_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x0c0000, 0x0c1fff) AM_RAM_WRITE(wwfwfest_fg0_videoram_w) AM_SHARE("fg0_videoram") /* FG0 Ram - 4 bytes per tile */
	AM_RANGE(0x0c2000, 0x0c3fff) AM_RAM AM_SHARE("spriteram")                       /* SPR Ram */
	AM_RANGE(0x080000, 0x080fff) AM_RAM_WRITE(ddragon3_fg_videoram_w) AM_SHARE("fg_videoram") /* BG0 Ram - 4 bytes per tile */
	AM_RANGE(0x082000, 0x082fff) AM_RAM_WRITE(ddragon3_bg_videoram_w) AM_SHARE("bg_videoram") /* BG1 Ram - 2 bytes per tile */
	AM_RANGE(0x100000, 0x100007) AM_READWRITE(ddragon3_scroll_r, ddragon3_scroll_w)
	AM_RANGE(0x10000a, 0x10000b) AM_WRITE(wwfwfest_flipscreen_w)
	AM_RANGE(0x140000, 0x140003) AM_WRITE(wwfwfest_irq_ack_w)
	AM_RANGE(0x14000c, 0x14000d) AM_WRITE(wwfwfest_soundwrite)
	AM_RANGE(0x140010, 0x140011) AM_WRITE8(wwfwfest_priority_w, 0x00ff)
	AM_RANGE(0x140020, 0x140021) AM_READ_PORT("P1")
	AM_RANGE(0x140022, 0x140023) AM_READ_PORT("P2")
	AM_RANGE(0x140024, 0x140025) AM_READ_PORT("P3")
	AM_RANGE(0x140026, 0x140027) AM_READ_PORT("P4")
	AM_RANGE(0x180000, 0x18ffff) AM_READWRITE(wwfwfest_paletteram_r,wwfwfest_paletteram_w) AM_SHARE("palette")
	AM_RANGE(0x1c0000, 0x1c3fff) AM_RAM /* Work Ram */
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, ddragon3_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xc800, 0xc801) AM_DEVREADWRITE("ym2151", ym2151_device, read, write)
	AM_RANGE(0xd800, 0xd800) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xe800, 0xe800) AM_WRITE(oki_bankswitch_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( ctribe_sound_map, AS_PROGRAM, 8, ddragon3_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8801) AM_DEVREADWRITE("ym2151", ym2151_device, status_r, write)
	AM_RANGE(0x9800, 0x9800) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END

/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

static INPUT_PORTS_START( ddragon3 )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )  // punch
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )  // jump
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )  // kick
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00f8, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW1:4" )
	PORT_DIPNAME( 0x0010, 0x0010, "Continue Discount" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW1:8" )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Player Vs. Player Damage" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW2:4" )
	PORT_SERVICE_DIPLOC( 0x1000, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPNAME( 0x2000, 0x2000, "Stage Clear Energy" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, "50" )
	PORT_DIPNAME( 0x4000, 0x4000, "Starting Energy" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, "200" )
	PORT_DIPSETTING(      0x4000, "230" )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Players ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, "2" )
	PORT_DIPSETTING(      0x0000, "3" )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( ctribe )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )  // punch
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )  // jump
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )  // Unused in game but work on input test
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW1:8" )
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED_DIPLOC( 0x0400, 0x0400, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW1:4" )
	PORT_DIPNAME( 0x1000, 0x1000, "Continue Discount" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START3 )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Timer Speed" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Fast" )
	PORT_DIPNAME( 0x0800, 0x0800, "FBI Logo" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x1000, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPNAME( 0x2000, 0x2000, "Stage Clear Energy" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, "0" )         PORT_CONDITION("IN3", 0x0100, EQUALS, 0x0100)
	PORT_DIPSETTING(      0x0000, "50" )        PORT_CONDITION("IN3", 0x0100, EQUALS, 0x0100)
	PORT_DIPSETTING(      0x2000, "100" )       PORT_CONDITION("IN3", 0x0100, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0000, "150" )       PORT_CONDITION("IN3", 0x0100, EQUALS, 0x0000)
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, "More Stage Clear Energy" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Players ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( ddragon3b )
	PORT_INCLUDE( ctribe )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x0400, 0x0400, "Player Vs. Player Damage" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Stage Clear Energy" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, "50" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW2:4" )

	PORT_MODIFY("IN3")
	PORT_DIPNAME( 0x0100, 0x0100, "Starting Energy" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, "200" )
	PORT_DIPSETTING(      0x0100, "230" )
INPUT_PORTS_END

static INPUT_PORTS_START( ctribeb )
	PORT_INCLUDE( ctribe )

	PORT_MODIFY("IN2")
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW2:4" )
INPUT_PORTS_END



static INPUT_PORTS_START( wwfwfest )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x3000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, wwfwfest_state,dsw_c0_r, "DSW2")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x3f00, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, wwfwfest_state,dsw_3f_r, "DSW2")

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x3f00, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, wwfwfest_state,dsw_3f_r, "DSW1")

	PORT_START("P4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0300, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, wwfwfest_state,dsw_c0_r, "DSW1")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	/* Nb:  There are actually 3 dips on the board, 2 * 8, and 1 *4 */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C )  )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C )  )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C )  )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C )  )
	PORT_DIPNAME( 0x04, 0x04, "Buy In Price"  ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "1 Coin" )
	PORT_DIPSETTING(    0x00, "As start price" )
	PORT_DIPNAME( 0x08, 0x08, "Regain Power Price"  ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "1 Coin" )
	PORT_DIPSETTING(    0x00, "As start price" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Continue_Price )  ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "1 Coin" )
	PORT_DIPSETTING(    0x00, "As start price" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen )  ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "FBI Logo" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Players ) ) PORT_DIPLOCATION("SW2:3,4") /* Nothing listed for seting ON/ON IE:0x00 */
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPNAME( 0x60, 0x60, "Clear Stage Power Up" ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x20, "12" )
	PORT_DIPSETTING(    0x60, "24" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPNAME( 0x80, 0x80, "Championship Game" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, "4th" )
	PORT_DIPSETTING(    0x80, "5th" )
INPUT_PORTS_END

static INPUT_PORTS_START( wwfwfesta )
	PORT_INCLUDE(wwfwfest)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x80, 0x00, "FBI Logo" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics Layouts
 *
 *************************************/


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 2, 4, 6 },
	{ 1, 0, 8*8+1, 8*8+0, 16*8+1, 16*8+0, 24*8+1, 24*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	32*8
};

static const gfx_layout wwf_tile_layout =
{
	16,16,  /* 16*16 tiles */
	RGN_FRAC(1,2),   /* 8192 tiles */
	4,  /* 4 bits per pixel */
	{ 8, 0, RGN_FRAC(1,2)+8 , RGN_FRAC(1,2)+0 },    /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			16*8, 16*9, 16*10, 16*11, 16*12, 16*13, 16*14, 16*15 },
	64*8    /* every tile takes 64 consecutive bytes */
};

static const gfx_layout wwf_sprite_layout = {
	16,16,  /* 16*16 tiles */
	RGN_FRAC(1,4),
	4,  /* 4 bits per pixel */
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4) , RGN_FRAC(3,4) }, /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    /* every tile takes 32 consecutive bytes */
};

/*************************************
 *
 *  Graphics Decode Info
 *
 *************************************/

static GFXDECODE_START( ddragon3 )
	GFXDECODE_ENTRY( "gfx1", 0, wwf_tile_layout,   512, 32 )
	GFXDECODE_ENTRY( "gfx1", 0, wwf_tile_layout,   256, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, wwf_sprite_layout,  0, 16 )
GFXDECODE_END

static GFXDECODE_START( wwfwfest )
	GFXDECODE_ENTRY( "gfx3", 0, wwf_tile_layout,     0x0c00, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, wwf_tile_layout,     0x1000, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, wwf_sprite_layout,   0x0400, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0x0000, 16 )
GFXDECODE_END



/*************************************
 *
 *  Interrupt Generators
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(ddragon3_state::ddragon3_scanline)
{
	int scanline = param;

	/* An interrupt is generated every 16 scanlines */
	if (scanline % 16 == 0)
	{
		if (scanline > 0)
			m_screen->update_partial(scanline - 1);
		m_maincpu->set_input_line(raster_level, ASSERT_LINE);
	}

	/* Vblank is raised on scanline 248 */
	if (scanline == 248)
	{
		m_screen->update_partial(scanline - 1);
		m_maincpu->set_input_line(vblank_level, ASSERT_LINE);
	}
}

/*************************************
 *
 *  Machine driver
 *
 *************************************/

void ddragon3_state::machine_start()
{
	save_item(NAME(m_vreg));
	save_item(NAME(m_bg_scrollx));
	save_item(NAME(m_bg_scrolly));
	save_item(NAME(m_fg_scrollx));
	save_item(NAME(m_fg_scrolly));
	save_item(NAME(m_bg_tilebase));
	save_item(NAME(m_io_reg));
}

void ddragon3_state::machine_reset()
{
	int i;

	m_vreg = 0;
	m_bg_scrollx = 0;
	m_bg_scrolly = 0;
	m_fg_scrollx = 0;
	m_fg_scrolly = 0;
	m_bg_tilebase = 0;

	for (i = 0; i < 8; i++)
		m_io_reg[i] = 0;
}

static MACHINE_CONFIG_START( ddragon3, ddragon3_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_20MHz / 2)
	MCFG_CPU_PROGRAM_MAP(ddragon3_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", ddragon3_state, ddragon3_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_3_579545MHz)
	MCFG_CPU_PROGRAM_MAP(sound_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_28MHz / 4, 448, 0, 320, 272, 8, 248)   /* HTOTAL and VTOTAL are guessed */
	MCFG_SCREEN_UPDATE_DRIVER(ddragon3_state, screen_update_ddragon3)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ddragon3)
	MCFG_PALETTE_ADD("palette", 768)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ym2151", XTAL_3_579545MHz)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)

	MCFG_OKIM6295_ADD("oki", XTAL_1_056MHz, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ddragon3b, ddragon3 )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(dd3b_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ctribe, ddragon3 )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(ctribe_map)

	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_PROGRAM_MAP(ctribe_sound_map)

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(ddragon3_state, screen_update_ctribe)

	MCFG_SOUND_MODIFY("ym2151")
	MCFG_SOUND_ROUTES_RESET()
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.20)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.20)

	MCFG_SOUND_MODIFY("oki")
	MCFG_SOUND_ROUTES_RESET()
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.80)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.80)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( wwfwfest, wwfwfest_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz / 2)  /* 24 crystal, 12 rated chip */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", ddragon3_state, ddragon3_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_3_579545MHz)
	MCFG_CPU_PROGRAM_MAP(sound_map)

	/* video hardware */
	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_28MHz / 4, 448, 0, 320, 272, 8, 248)   /* HTOTAL and VTOTAL are guessed */
	MCFG_SCREEN_UPDATE_DRIVER(wwfwfest_state, screen_update_wwfwfest)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", wwfwfest)
	MCFG_PALETTE_ADD("palette", 8192)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ym2151", XTAL_3_579545MHz)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "mono", 0.45)
	MCFG_SOUND_ROUTE(1, "mono", 0.45)

	MCFG_OKIM6295_ADD("oki", 1024188, OKIM6295_PIN7_HIGH) /* Verified - Pin 7 tied to +5VDC */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.90)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( wwfwfstb, wwfwfest )
	MCFG_VIDEO_START_OVERRIDE(wwfwfest_state,wwfwfstb)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( ddragon3 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 64k for cpu code */
	ROM_LOAD16_BYTE( "30a14-0.ic78", 0x00001, 0x40000, CRC(f42fe016) SHA1(11511aa43caa12b36a795bfaefee824821282523) )
	ROM_LOAD16_BYTE( "30a15-0.ic79", 0x00000, 0x20000, CRC(ad50e92c) SHA1(facac5bbe11716d076a40eacbb67f7caab7a4a27) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound cpu code */
	ROM_LOAD( "30a13-0.ic43", 0x00000, 0x10000, CRC(1e974d9b) SHA1(8e54ff747efe587a2e971c15e729445c4e232f0f) )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* Background */
	ROM_LOAD16_BYTE( "30j-7.ic4",    0x000001, 0x40000, CRC(89d58d32) SHA1(54cfc154024e014f537c7ae0c2275ece50413bc5) )
	ROM_LOAD16_BYTE( "30j-6.ic5",    0x000000, 0x40000, CRC(9bf1538e) SHA1(c7cb96c6b1ac73ec52f46b2a6687bfcfd375ab44) )
	ROM_LOAD16_BYTE( "30j-5.ic6",    0x080001, 0x40000, CRC(8f671a62) SHA1(b5dba61ad6ed39440bb98f7b2dc1111779d6c4a1) )
	ROM_LOAD16_BYTE( "30j-4.ic7",    0x080000, 0x40000, CRC(0f74ea1c) SHA1(6bd8dd89bd22b29038cf502a898336e95e50a9cc) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* Sprites */
	ROM_LOAD( "30j-3.ic9",    0x000000, 0x80000, CRC(b3151871) SHA1(a647b4d9bddd6b8715a1d24641391a2e2d0f8867) )
	ROM_LOAD( "30a12-0.ic8",  0x080000, 0x10000, CRC(20d64bea) SHA1(c2bd86bc5310f13f158ca2f93cfc57e5dbf01f7e) )
	ROM_LOAD( "30j-2.ic11",   0x100000, 0x80000, CRC(41c6fb08) SHA1(9fb6105bdc9ff8eeaacf378d208cf6d32a09401b) )
	ROM_LOAD( "30a11-0.ic10", 0x180000, 0x10000, CRC(785d71b0) SHA1(e3f63f6984589d4d6ec6200ae33ce12610d27774) )
	ROM_LOAD( "30j-1.ic13",   0x200000, 0x80000, CRC(67a6f114) SHA1(7d0f3cd6376128ddfcd13f2ec683ec270e95c19c) )
	ROM_LOAD( "30a10-0.ic12", 0x280000, 0x10000, CRC(15e43d12) SHA1(b51cbd0c4c38b802e60616e11795b1ac43bfcb01) )
	ROM_LOAD( "30j-0.ic15",   0x300000, 0x80000, CRC(f15dafbe) SHA1(68049c4542e1c7119bbf1be1fa44e3eea9c11b6e) )
	ROM_LOAD( "30a9-0.ic14",  0x380000, 0x10000, CRC(5a47e7a4) SHA1(74b9dff6e3d5fe22ea505dc439121ff64889769c) )

	ROM_REGION( 0x080000, "oki", 0 )    /* ADPCM Samples */
	ROM_LOAD( "30j-8.ic73",   0x000000, 0x80000, CRC(c3ad40f3) SHA1(6f3f5fc5b1050fc9a366e02e8e507183a624494d) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.ic38", 0x000000, 0x00100, CRC(113c7443) SHA1(7b0b13e9f0c219f6d436aeec06494734d1f4a599) )
ROM_END

ROM_START( ddragon3j )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 64k for cpu code */
	ROM_LOAD16_BYTE( "30j15.ic78", 0x00001, 0x40000, CRC(40618cbc) SHA1(cb05498003a45d773983501d3a0f1584a25dcdd3) )
	ROM_LOAD16_BYTE( "30j14.ic79", 0x00000, 0x20000, CRC(96827e80) SHA1(499c0c67d55ff4816ad8832d1d8836eb7432bd13) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound cpu code */
	ROM_LOAD( "30j13.ic43",   0x00000, 0x10000, CRC(1e974d9b) SHA1(8e54ff747efe587a2e971c15e729445c4e232f0f) )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* Background */
	ROM_LOAD16_BYTE( "30j-7.ic4",    0x000001, 0x40000, CRC(89d58d32) SHA1(54cfc154024e014f537c7ae0c2275ece50413bc5) )
	ROM_LOAD16_BYTE( "30j-6.ic5",    0x000000, 0x40000, CRC(9bf1538e) SHA1(c7cb96c6b1ac73ec52f46b2a6687bfcfd375ab44) )
	ROM_LOAD16_BYTE( "30j-5.ic6",    0x080001, 0x40000, CRC(8f671a62) SHA1(b5dba61ad6ed39440bb98f7b2dc1111779d6c4a1) )
	ROM_LOAD16_BYTE( "30j-4.ic7",    0x080000, 0x40000, CRC(0f74ea1c) SHA1(6bd8dd89bd22b29038cf502a898336e95e50a9cc) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* Sprites */
	ROM_LOAD( "30j-3.ic9",    0x000000, 0x80000, CRC(b3151871) SHA1(a647b4d9bddd6b8715a1d24641391a2e2d0f8867) )
	ROM_LOAD( "30j12-0.ic8",  0x080000, 0x10000, CRC(1e9290d7) SHA1(77e660d2dc9a0e2c4c8ceb3e47b7ce674bceb34a) )
	ROM_LOAD( "30j-2.ic11",   0x100000, 0x80000, CRC(41c6fb08) SHA1(9fb6105bdc9ff8eeaacf378d208cf6d32a09401b) )
	ROM_LOAD( "30j11-0.ic10", 0x180000, 0x10000, CRC(99195b2a) SHA1(d1c0e1855aed22f169717f94d78bc326e68e3064) )
	ROM_LOAD( "30j-1.ic13",   0x200000, 0x80000, CRC(67a6f114) SHA1(7d0f3cd6376128ddfcd13f2ec683ec270e95c19c) )
	ROM_LOAD( "30j10-0.ic12", 0x280000, 0x10000, CRC(e3879b5d) SHA1(fc87aedb0f4964a8d261d86121fe8544b330bed9) )
	ROM_LOAD( "30j-0.ic15",   0x300000, 0x80000, CRC(f15dafbe) SHA1(68049c4542e1c7119bbf1be1fa44e3eea9c11b6e) )
	ROM_LOAD( "30j9-0.ic14",  0x380000, 0x10000, CRC(2759ae84) SHA1(02c70958259f56174ce2ba2db56040dad72be02b) )

	ROM_REGION( 0x080000, "oki", 0 )    /* ADPCM Samples */
	ROM_LOAD( "30j-8.ic73",   0x000000, 0x80000, CRC(c3ad40f3) SHA1(6f3f5fc5b1050fc9a366e02e8e507183a624494d) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.ic38", 0x000000, 0x00100, CRC(113c7443) SHA1(7b0b13e9f0c219f6d436aeec06494734d1f4a599) )
ROM_END

ROM_START( ddragon3p )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 64k for cpu code */
	ROM_LOAD16_BYTE( "30a14-0.ic80", 0x00001, 0x40000, CRC(f42fe016) SHA1(11511aa43caa12b36a795bfaefee824821282523) )
	ROM_LOAD16_BYTE( "30a15-0.ic79", 0x00000, 0x20000, CRC(ad50e92c) SHA1(facac5bbe11716d076a40eacbb67f7caab7a4a27) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound cpu code */
	ROM_LOAD( "30a13-0.ic43",   0x00000, 0x10000, CRC(1e974d9b) SHA1(8e54ff747efe587a2e971c15e729445c4e232f0f) )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* Background */
	ROM_LOAD16_BYTE( "14.ic45",      0x000001, 0x20000, CRC(b036a27b) SHA1(c13589c3882bb86f14a3b0143f2d9a4474350ddd) )
	ROM_LOAD16_BYTE( "15.ic46",      0x040001, 0x20000, CRC(24d0bf41) SHA1(2e9c26c8078d17323af6ba378c7ceaed9045d3f7) )
	ROM_LOAD16_BYTE( "30.ic13",      0x000000, 0x20000, CRC(72fe2b16) SHA1(92f02381c0216cf5cfede6813e4dcb814a040091) )
	ROM_LOAD16_BYTE( "31.ic14",      0x040000, 0x20000, CRC(ab48a0c8) SHA1(b908f601a621697ad3b5067d26b6fb1713c4af39) )
	ROM_LOAD16_BYTE( "23.ic29",      0x080001, 0x20000, CRC(0768fedd) SHA1(757c4378f53b4b8cc024b4c5a74d19ab653e886e) )
	ROM_LOAD16_BYTE( "24.ic30",      0x0c0001, 0x20000, CRC(ec9db18a) SHA1(7e4085ba4c0e20ec00f392a2bf9cdb81be53b97f) )
	ROM_LOAD16_BYTE( "21.ic25",      0x080000, 0x20000, CRC(902744b9) SHA1(eea623ce013bc270b1611982dd2f9388b205dbb3) )
	ROM_LOAD16_BYTE( "22.ic26",      0x0c0000, 0x20000, CRC(5b142d4d) SHA1(88e22e102efa35449c0d9f6139eb0718528a9d72) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* Sprites */
	ROM_LOAD( "9.ic39",       0x000000, 0x20000, CRC(726c49b7) SHA1(dbafad47bb6b717c409fdc5d81c413f1282f2bbb) )
	ROM_LOAD( "10.ic40",      0x020000, 0x20000, CRC(37a1c335) SHA1(de70ba51788b601591c3aff71cb94aae349b272d) )
	ROM_LOAD( "11.ic41",      0x040000, 0x20000, CRC(2bcfe63c) SHA1(678ef0e7cc38e4df1e1d1e3f5cba6601aa520ec6) )
	ROM_LOAD( "12.ic42",      0x060000, 0x20000, CRC(b864cf17) SHA1(39a5155f40ba500bf201acca6f7d230cb0ea8309) )
	ROM_LOAD( "30a12-0.ic43", 0x080000, 0x20000, CRC(91da004c) SHA1(d61c8545e622de6872ce1f1487dd0342fd81572c) )

	ROM_LOAD( "4.ic33",       0x100000, 0x20000, CRC(8c71eb06) SHA1(e47acf9e2d5eeec0cff9654210a43c690a45d447) )
	ROM_LOAD( "5.ic34",       0x120000, 0x20000, CRC(3e134be9) SHA1(0a75b56353bed2743f7ce8f3f74379fc9f0d3cb9) )
	ROM_LOAD( "6.ic35",       0x140000, 0x20000, CRC(b4115ef0) SHA1(d90943f75051c7590a0effcc30fa813890c9ad11) )
	ROM_LOAD( "7.ic36",       0x160000, 0x20000, CRC(4639333d) SHA1(8e3c982d6fa38cbec42e8de780f165547b5b0271) )
	ROM_LOAD( "30a11-0.ic37", 0x180000, 0x20000, CRC(5f419232) SHA1(86a883d7f0dfdfcc34c90e54f0b65b23b5822c16) )

	ROM_LOAD( "16.ic19",      0x200000, 0x20000, CRC(04420cc8) SHA1(ed148c52374bbd0d29c12070ea1499333fc04449) )
	ROM_LOAD( "17.ic20",      0x220000, 0x20000, CRC(33f97b2f) SHA1(40dc5357caa17ed6673588422332966ee97752b7) )
	ROM_LOAD( "18.ic21",      0x240000, 0x20000, CRC(0f9a8f2a) SHA1(d7e46d32067d3f8b3bacbf96ea313645a9a48410) )
	ROM_LOAD( "19.ic22",      0x260000, 0x20000, CRC(15c91772) SHA1(8578b6c501e3af64863bd6b28ef59c6884dfe028) )
	ROM_LOAD( "30a10-0.ic23", 0x280000, 0x20000, CRC(12f641ba) SHA1(1e197a584bcc0b2f3b97e1a8ec61864b279ab951) )

	ROM_LOAD( "25.ic3",       0x300000, 0x20000, CRC(894734b3) SHA1(46fa174a303e85f439254976252835626c4b2ddc) )
	ROM_LOAD( "26.ic4",       0x320000, 0x20000, CRC(cd504584) SHA1(674481b524853dbfcb7d173d58250b1be8464313) )
	ROM_LOAD( "27.ic5",       0x340000, 0x20000, CRC(38e8a9ad) SHA1(1c66acde8f72fa7c6415a7aadc2dbf4300446c88) )
	ROM_LOAD( "28.ic6",       0x360000, 0x20000, CRC(80c1cb74) SHA1(5558fa36b238cff1bee9df921e77d7de2062bf15) )
	ROM_LOAD( "30a9-0.ic7",   0x380000, 0x20000, CRC(9199a77b) SHA1(35b9a2a707ffd7dd2cfc2bea0c78f02f3639d1bd) )

	ROM_REGION( 0x080000, "oki", 0 )    /* ADPCM Samples */
	ROM_LOAD( "2.ic73",   0x000000, 0x40000, CRC(3af21dbe) SHA1(295d0b7f33c55ef37a71382a22edd8fc97fa5353) )
	ROM_LOAD( "3.ic74",   0x040000, 0x40000, CRC(c28b53cd) SHA1(93d29669ec899fd5852f61b1d91d0a90cc30e192) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "30.ic38", 0x000000, 0x00100, CRC(113c7443) SHA1(7b0b13e9f0c219f6d436aeec06494734d1f4a599) )
ROM_END

ROM_START( ddragon3b )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 64k for cpu code */
	ROM_LOAD16_BYTE( "dd3.01",   0x00001, 0x20000, CRC(68321d8b) SHA1(bd34d361e8ef18ef2b7e8bfe438b1b098c3151b5) )
	ROM_LOAD16_BYTE( "dd3.03",   0x00000, 0x20000, CRC(bc05763b) SHA1(49f661fdc98bd43a6622945e9aa8d8e7a7dc1ce6) )
	ROM_LOAD16_BYTE( "dd3.02",   0x40001, 0x20000, CRC(38d9ae75) SHA1(d42e1d9c704c66bad94e14d14f5e0b7209cc938e) )
	/* No EVEN rom! */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound cpu code */
	ROM_LOAD( "dd3.06",    0x00000, 0x10000, CRC(1e974d9b) SHA1(8e54ff747efe587a2e971c15e729445c4e232f0f) )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* Background */
	ROM_LOAD16_BYTE( "dd3.f",   0x000001, 0x40000, CRC(89d58d32) SHA1(54cfc154024e014f537c7ae0c2275ece50413bc5) )
	ROM_LOAD16_BYTE( "dd3.e",   0x000000, 0x40000, CRC(9bf1538e) SHA1(c7cb96c6b1ac73ec52f46b2a6687bfcfd375ab44) )
	ROM_LOAD16_BYTE( "dd3.b",   0x080001, 0x40000, CRC(8f671a62) SHA1(b5dba61ad6ed39440bb98f7b2dc1111779d6c4a1) )
	ROM_LOAD16_BYTE( "dd3.a",   0x080000, 0x40000, CRC(0f74ea1c) SHA1(6bd8dd89bd22b29038cf502a898336e95e50a9cc) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* Sprites */
	ROM_LOAD( "dd3.3e",   0x000000, 0x20000, CRC(726c49b7) SHA1(dbafad47bb6b717c409fdc5d81c413f1282f2bbb) ) //4a
	ROM_LOAD( "dd3.3d",   0x020000, 0x20000, CRC(37a1c335) SHA1(de70ba51788b601591c3aff71cb94aae349b272d) ) //3a
	ROM_LOAD( "dd3.3c",   0x040000, 0x20000, CRC(2bcfe63c) SHA1(678ef0e7cc38e4df1e1d1e3f5cba6601aa520ec6) ) //2a
	ROM_LOAD( "dd3.3b",   0x060000, 0x20000, CRC(b864cf17) SHA1(39a5155f40ba500bf201acca6f7d230cb0ea8309) ) //1a
	ROM_LOAD( "dd3.3a",   0x080000, 0x10000, CRC(20d64bea) SHA1(c2bd86bc5310f13f158ca2f93cfc57e5dbf01f7e) ) //5a

	ROM_LOAD( "dd3.2e",   0x100000, 0x20000, CRC(8c71eb06) SHA1(e47acf9e2d5eeec0cff9654210a43c690a45d447) ) //4b
	ROM_LOAD( "dd3.2d",   0x120000, 0x20000, CRC(3e134be9) SHA1(0a75b56353bed2743f7ce8f3f74379fc9f0d3cb9) ) //3b
	ROM_LOAD( "dd3.2c",   0x140000, 0x20000, CRC(b4115ef0) SHA1(d90943f75051c7590a0effcc30fa813890c9ad11) ) //2b
	ROM_LOAD( "dd3.2b",   0x160000, 0x20000, CRC(4639333d) SHA1(8e3c982d6fa38cbec42e8de780f165547b5b0271) ) //1b
	ROM_LOAD( "dd3.2a",   0x180000, 0x10000, CRC(785d71b0) SHA1(e3f63f6984589d4d6ec6200ae33ce12610d27774) ) //5b

	ROM_LOAD( "dd3.1e",   0x200000, 0x20000, CRC(04420cc8) SHA1(ed148c52374bbd0d29c12070ea1499333fc04449) ) //4c
	ROM_LOAD( "dd3.1d",   0x220000, 0x20000, CRC(33f97b2f) SHA1(40dc5357caa17ed6673588422332966ee97752b7) ) //3c
	ROM_LOAD( "dd3.1c",   0x240000, 0x20000, CRC(0f9a8f2a) SHA1(d7e46d32067d3f8b3bacbf96ea313645a9a48410) ) //2c
	ROM_LOAD( "dd3.1b",   0x260000, 0x20000, CRC(15c91772) SHA1(8578b6c501e3af64863bd6b28ef59c6884dfe028) ) //1c
	ROM_LOAD( "dd3.1a",   0x280000, 0x10000, CRC(15e43d12) SHA1(b51cbd0c4c38b802e60616e11795b1ac43bfcb01) ) //5c

	ROM_LOAD( "dd3.0e",   0x300000, 0x20000, CRC(894734b3) SHA1(46fa174a303e85f439254976252835626c4b2ddc) ) //4d
	ROM_LOAD( "dd3.0d",   0x320000, 0x20000, CRC(cd504584) SHA1(674481b524853dbfcb7d173d58250b1be8464313) ) //3d
	ROM_LOAD( "dd3.0c",   0x340000, 0x20000, CRC(38e8a9ad) SHA1(1c66acde8f72fa7c6415a7aadc2dbf4300446c88) ) //2d
	ROM_LOAD( "dd3.0b",   0x360000, 0x20000, CRC(80c1cb74) SHA1(5558fa36b238cff1bee9df921e77d7de2062bf15) ) //1d
	ROM_LOAD( "dd3.0a",   0x380000, 0x10000, CRC(5a47e7a4) SHA1(74b9dff6e3d5fe22ea505dc439121ff64889769c) ) //5d

	ROM_REGION( 0x080000, "oki", 0 )    /* ADPCM Samples */
	ROM_LOAD( "dd3.j7",   0x000000, 0x40000, CRC(3af21dbe) SHA1(295d0b7f33c55ef37a71382a22edd8fc97fa5353) )
	ROM_LOAD( "dd3.j8",   0x040000, 0x40000, CRC(c28b53cd) SHA1(93d29669ec899fd5852f61b1d91d0a90cc30e192) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.38", 0x0000, 0x0100, CRC(113c7443) SHA1(7b0b13e9f0c219f6d436aeec06494734d1f4a599) )
ROM_END

ROM_START( ctribe )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 64k for cpu code */
	ROM_LOAD16_BYTE( "28a16-2.ic26", 0x00001, 0x20000, CRC(c46b2e63) SHA1(86ace715dca48c78a46da1d102de47e5f948a86c) )
	ROM_LOAD16_BYTE( "28a15-2.ic25", 0x00000, 0x20000, CRC(3221c755) SHA1(0f6fe5cd6947f6547585eedb7fc5e6af8544b1f7) )
	ROM_LOAD16_BYTE( "28j17-0.104", 0x40001, 0x10000, CRC(8c2c6dbd) SHA1(b99b9be6e0bdc8340fedd258819c4df587926a84) )
	/* No EVEN rom! */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound cpu code */
	ROM_LOAD( "28a10-0.ic89", 0x00000, 0x8000, CRC(4346de13) SHA1(67c6de90ba31a325f03e64d28c9391a315ee359c) )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* Background */
	ROM_LOAD16_BYTE( "28j7-0.ic11",  0x000001, 0x40000, CRC(a8b773f1) SHA1(999e41dfeb3fb937da769c4a33bb29bf4076dc63) )
	ROM_LOAD16_BYTE( "28j6-0.ic13",  0x000000, 0x40000, CRC(617530fc) SHA1(b9155ed0ae1437bf4d0b7a95e769bc05a820ecec) )
	ROM_LOAD16_BYTE( "28j5-0.ic12",  0x080001, 0x40000, CRC(cef0a821) SHA1(c7a35048d5ebf3f09abf9d27f91d12adc03befeb) )
	ROM_LOAD16_BYTE( "28j4-0.ic14",  0x080000, 0x40000, CRC(b84fda09) SHA1(3ae0c0ec6c398dea17e248b017ea3e2f6c3571e1) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* Sprites */
	ROM_LOAD( "28j3-0.ic77",  0x000000, 0x80000, CRC(1ac2a461) SHA1(17436f5dcf29041ca5f470dfae538e4fc12153cc) )
	ROM_LOAD( "28a14-0.ic60", 0x080000, 0x10000, CRC(972faddb) SHA1(f2b211e8f8301667e6c9a3ce9612e39b16e66a67) )
	ROM_LOAD( "28j2-0.ic78",  0x100000, 0x80000, CRC(8c796707) SHA1(7417ad0413083876ed65a8612845ccb0d2717530) )
	ROM_LOAD( "28a13-0.ic61", 0x180000, 0x10000, CRC(eb3ab374) SHA1(db66cb7976c111fa76a3a211e96ad1d7b78ce0ad) )
	ROM_LOAD( "28j1-0.ic97",  0x200000, 0x80000, CRC(1c9badbd) SHA1(d28f6d684d88448eaa3feae0bba2f5a836d89bd7) )
	ROM_LOAD( "28a12-0.ic85", 0x280000, 0x10000, CRC(c602ac97) SHA1(44440739636b684c6dcac837f59664120c9ba5f3) )
	ROM_LOAD( "28j0-0.ic98",  0x300000, 0x80000, CRC(ba73c49e) SHA1(830099027ede1f7c56bb0bf3cdef3018b92e0b87) )
	ROM_LOAD( "28a11-0.ic86", 0x380000, 0x10000, CRC(4da1d8e5) SHA1(568e9e8d00f1b1ca27c28df5fc0ffc74ad91da7e) )

	ROM_REGION( 0x040000, "oki", 0 ) /* ADPCM Samples */
	ROM_LOAD( "28j9-0.ic83", 0x00000, 0x20000, CRC(f92a7f4a) SHA1(3717ef64876be9ada378b449749918ce9072073a) )
	ROM_LOAD( "28j8-0.ic82", 0x20000, 0x20000, CRC(1a3a0b39) SHA1(8847530027cf4be03ffbc6d78dee97b459d03a04) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "28.ic44", 0x0000, 0x0100, CRC(964329ef) SHA1(f26846571a16d27b726f689049deb0188103aadb) )
ROM_END

ROM_START( ctribe1 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 64k for cpu code */
	ROM_LOAD16_BYTE( "1_28a16-2.ic26", 0x00001, 0x20000, CRC(f00f8443) SHA1(3c099b6bea9956cc60ce4a9a5d790ac2bf7d77bd) )
	ROM_LOAD16_BYTE( "1_28a15-2.ic25", 0x00000, 0x20000, CRC(dd70079f) SHA1(321b523fefec2a962d0afa20b33428e7caea8958) )
	ROM_LOAD16_BYTE( "28j17-0.104", 0x40001, 0x10000, CRC(8c2c6dbd) SHA1(b99b9be6e0bdc8340fedd258819c4df587926a84) )
	/* No EVEN rom! */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound cpu code */
	ROM_LOAD( "28a10-0.ic89", 0x00000, 0x8000, CRC(4346de13) SHA1(67c6de90ba31a325f03e64d28c9391a315ee359c) )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* Background */
	ROM_LOAD16_BYTE( "28j7-0.ic11",  0x000001, 0x40000, CRC(a8b773f1) SHA1(999e41dfeb3fb937da769c4a33bb29bf4076dc63) )
	ROM_LOAD16_BYTE( "28j6-0.ic13",  0x000000, 0x40000, CRC(617530fc) SHA1(b9155ed0ae1437bf4d0b7a95e769bc05a820ecec) )
	ROM_LOAD16_BYTE( "28j5-0.ic12",  0x080001, 0x40000, CRC(cef0a821) SHA1(c7a35048d5ebf3f09abf9d27f91d12adc03befeb) )
	ROM_LOAD16_BYTE( "28j4-0.ic14",  0x080000, 0x40000, CRC(b84fda09) SHA1(3ae0c0ec6c398dea17e248b017ea3e2f6c3571e1) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* Sprites */
	ROM_LOAD( "28j3-0.ic77",  0x000000, 0x80000, CRC(1ac2a461) SHA1(17436f5dcf29041ca5f470dfae538e4fc12153cc) )
	ROM_LOAD( "28a14-0.ic60", 0x080000, 0x10000, CRC(972faddb) SHA1(f2b211e8f8301667e6c9a3ce9612e39b16e66a67) )
	ROM_LOAD( "28j2-0.ic78",  0x100000, 0x80000, CRC(8c796707) SHA1(7417ad0413083876ed65a8612845ccb0d2717530) )
	ROM_LOAD( "28a13-0.ic61", 0x180000, 0x10000, CRC(eb3ab374) SHA1(db66cb7976c111fa76a3a211e96ad1d7b78ce0ad) )
	ROM_LOAD( "28j1-0.ic97",  0x200000, 0x80000, CRC(1c9badbd) SHA1(d28f6d684d88448eaa3feae0bba2f5a836d89bd7) )
	ROM_LOAD( "28a12-0.ic85", 0x280000, 0x10000, CRC(c602ac97) SHA1(44440739636b684c6dcac837f59664120c9ba5f3) )
	ROM_LOAD( "28j0-0.ic98",  0x300000, 0x80000, CRC(ba73c49e) SHA1(830099027ede1f7c56bb0bf3cdef3018b92e0b87) )
	ROM_LOAD( "28a11-0.ic86", 0x380000, 0x10000, CRC(4da1d8e5) SHA1(568e9e8d00f1b1ca27c28df5fc0ffc74ad91da7e) )

	ROM_REGION( 0x040000, "oki", 0 ) /* ADPCM Samples */
	ROM_LOAD( "28j9-0.ic83", 0x00000, 0x20000, CRC(f92a7f4a) SHA1(3717ef64876be9ada378b449749918ce9072073a) )
	ROM_LOAD( "28j8-0.ic82", 0x20000, 0x20000, CRC(1a3a0b39) SHA1(8847530027cf4be03ffbc6d78dee97b459d03a04) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "28.ic44", 0x0000, 0x0100, CRC(964329ef) SHA1(f26846571a16d27b726f689049deb0188103aadb) )
ROM_END

ROM_START( ctribeo ) // only main program code differs from ctribe1 set
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 64k for cpu code */
	ROM_LOAD16_BYTE( "28a16-1.ic26", 0x00001, 0x20000, CRC(d108f36f) SHA1(af53fa441b9ddfc639abb573864a9b351633c6b7) )
	ROM_LOAD16_BYTE( "28a15-1.ic25", 0x00000, 0x20000, CRC(3f5693a3) SHA1(2d4516506ad1d68ac45242fe149bba1b4b53eb45) )
	ROM_LOAD16_BYTE( "28j17-0.104", 0x40001, 0x10000, CRC(8c2c6dbd) SHA1(b99b9be6e0bdc8340fedd258819c4df587926a84) )
	/* No EVEN rom! */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound cpu code */
	ROM_LOAD( "28a10-0.ic89", 0x00000, 0x8000, CRC(4346de13) SHA1(67c6de90ba31a325f03e64d28c9391a315ee359c) )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* Background */
	ROM_LOAD16_BYTE( "28j7-0.ic11",  0x000001, 0x40000, CRC(a8b773f1) SHA1(999e41dfeb3fb937da769c4a33bb29bf4076dc63) )
	ROM_LOAD16_BYTE( "28j6-0.ic13",  0x000000, 0x40000, CRC(617530fc) SHA1(b9155ed0ae1437bf4d0b7a95e769bc05a820ecec) )
	ROM_LOAD16_BYTE( "28j5-0.ic12",  0x080001, 0x40000, CRC(cef0a821) SHA1(c7a35048d5ebf3f09abf9d27f91d12adc03befeb) )
	ROM_LOAD16_BYTE( "28j4-0.ic14",  0x080000, 0x40000, CRC(b84fda09) SHA1(3ae0c0ec6c398dea17e248b017ea3e2f6c3571e1) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* Sprites */
	ROM_LOAD( "28j3-0.ic77",  0x000000, 0x80000, CRC(1ac2a461) SHA1(17436f5dcf29041ca5f470dfae538e4fc12153cc) )
	ROM_LOAD( "28a14-0.ic60", 0x080000, 0x10000, CRC(972faddb) SHA1(f2b211e8f8301667e6c9a3ce9612e39b16e66a67) )
	ROM_LOAD( "28j2-0.ic78",  0x100000, 0x80000, CRC(8c796707) SHA1(7417ad0413083876ed65a8612845ccb0d2717530) )
	ROM_LOAD( "28a13-0.ic61", 0x180000, 0x10000, CRC(eb3ab374) SHA1(db66cb7976c111fa76a3a211e96ad1d7b78ce0ad) )
	ROM_LOAD( "28j1-0.ic97",  0x200000, 0x80000, CRC(1c9badbd) SHA1(d28f6d684d88448eaa3feae0bba2f5a836d89bd7) )
	ROM_LOAD( "28a12-0.ic85", 0x280000, 0x10000, CRC(c602ac97) SHA1(44440739636b684c6dcac837f59664120c9ba5f3) )
	ROM_LOAD( "28j0-0.ic98",  0x300000, 0x80000, CRC(ba73c49e) SHA1(830099027ede1f7c56bb0bf3cdef3018b92e0b87) )
	ROM_LOAD( "28a11-0.ic86", 0x380000, 0x10000, CRC(4da1d8e5) SHA1(568e9e8d00f1b1ca27c28df5fc0ffc74ad91da7e) )

	ROM_REGION( 0x040000, "oki", 0 ) /* ADPCM Samples */
	ROM_LOAD( "28j9-0.ic83", 0x00000, 0x20000, CRC(f92a7f4a) SHA1(3717ef64876be9ada378b449749918ce9072073a) )
	ROM_LOAD( "28j8-0.ic82", 0x20000, 0x20000, CRC(1a3a0b39) SHA1(8847530027cf4be03ffbc6d78dee97b459d03a04) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "28.ic44", 0x0000, 0x0100, CRC(964329ef) SHA1(f26846571a16d27b726f689049deb0188103aadb) )
ROM_END





ROM_START( ctribej )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 64k for cpu code */
	ROM_LOAD16_BYTE( "28j16-02.26", 0x00001, 0x20000, CRC(658b8568) SHA1(899682f6ab28b184654c51c1169216974043f1b9) )
	ROM_LOAD16_BYTE( "28j15-12.25", 0x00000, 0x20000, CRC(50aac7e7) SHA1(af77107f325f9b45a92c544328d3073ed1db5465) )
	ROM_LOAD16_BYTE( "28j17-0.104", 0x40001, 0x10000, CRC(8c2c6dbd) SHA1(b99b9be6e0bdc8340fedd258819c4df587926a84) )
	/* No EVEN rom! */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound cpu code */
	ROM_LOAD( "28j10-0.89", 0x00000, 0x8000, CRC(4346de13) SHA1(67c6de90ba31a325f03e64d28c9391a315ee359c) )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* Background */
	ROM_LOAD16_BYTE( "28j7-0.ic11",  0x000001, 0x40000, CRC(a8b773f1) SHA1(999e41dfeb3fb937da769c4a33bb29bf4076dc63) )
	ROM_LOAD16_BYTE( "28j6-0.ic13",  0x000000, 0x40000, CRC(617530fc) SHA1(b9155ed0ae1437bf4d0b7a95e769bc05a820ecec) )
	ROM_LOAD16_BYTE( "28j5-0.ic12",  0x080001, 0x40000, CRC(cef0a821) SHA1(c7a35048d5ebf3f09abf9d27f91d12adc03befeb) )
	ROM_LOAD16_BYTE( "28j4-0.ic14",  0x080000, 0x40000, CRC(b84fda09) SHA1(3ae0c0ec6c398dea17e248b017ea3e2f6c3571e1) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* Sprites */
	ROM_LOAD( "28j3-0.ic77",  0x000000, 0x80000, CRC(1ac2a461) SHA1(17436f5dcf29041ca5f470dfae538e4fc12153cc) )
	ROM_LOAD( "28j14-0.60",   0x080000, 0x10000, CRC(6869050a) SHA1(34bdab383f2b0c5327306d419f65ce7974e1b7ba) )
	ROM_LOAD( "28j2-0.ic78",  0x100000, 0x80000, CRC(8c796707) SHA1(7417ad0413083876ed65a8612845ccb0d2717530) )
	ROM_LOAD( "28j13-0.61",   0x180000, 0x10000, CRC(8b8addea) SHA1(87bc1c843a51d232d339735a85946dde799a9ac5) )
	ROM_LOAD( "28j1-0.ic97",  0x200000, 0x80000, CRC(1c9badbd) SHA1(d28f6d684d88448eaa3feae0bba2f5a836d89bd7) )
	ROM_LOAD( "28j12-0.85",   0x280000, 0x10000, CRC(422b041c) SHA1(fa19dce6ee84b5a2ad729963073abe8919cb1689) )
	ROM_LOAD( "28j0-0.ic98",  0x300000, 0x80000, CRC(ba73c49e) SHA1(830099027ede1f7c56bb0bf3cdef3018b92e0b87) )
	ROM_LOAD( "28j11-0.86",   0x380000, 0x10000, CRC(4a391c5b) SHA1(520aa49808a3e7faf32efbb6b0ec22f90e5b4890) )

	ROM_REGION( 0x040000, "oki", 0 ) /* ADPCM Samples */
	ROM_LOAD( "28j9-0.ic83", 0x00000, 0x20000, CRC(f92a7f4a) SHA1(3717ef64876be9ada378b449749918ce9072073a) )
	ROM_LOAD( "28j8-0.ic82", 0x20000, 0x20000, CRC(1a3a0b39) SHA1(8847530027cf4be03ffbc6d78dee97b459d03a04) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "28.ic44", 0x0000, 0x0100, CRC(964329ef) SHA1(f26846571a16d27b726f689049deb0188103aadb) )
ROM_END

ROM_START( ctribeb )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 64k for cpu code */
	ROM_LOAD16_BYTE( "ct_ep1.rom", 0x00001, 0x20000, CRC(9cfa997f) SHA1(ee49b4b9e9cd29616f244fdf3912ef743e2404ce) )
	ROM_LOAD16_BYTE( "ct_ep3.rom", 0x00000, 0x20000, CRC(2ece8681) SHA1(17ee2ceb893e2eb08fa4cabcdebcec02bee16cda) )
	ROM_LOAD16_BYTE( "ct_ep2.rom", 0x40001, 0x10000, CRC(8c2c6dbd) SHA1(b99b9be6e0bdc8340fedd258819c4df587926a84) )
	/* No EVEN rom! */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound cpu code */
	ROM_LOAD( "ct_ep4.rom",   0x00000, 0x8000, CRC(4346de13) SHA1(67c6de90ba31a325f03e64d28c9391a315ee359c) )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* Background */
	ROM_LOAD16_BYTE( "ct_mr7.rom",  0x000001, 0x40000, CRC(a8b773f1) SHA1(999e41dfeb3fb937da769c4a33bb29bf4076dc63) )
	ROM_LOAD16_BYTE( "ct_mr6.rom",  0x000000, 0x40000, CRC(617530fc) SHA1(b9155ed0ae1437bf4d0b7a95e769bc05a820ecec) )
	ROM_LOAD16_BYTE( "ct_mr5.rom",  0x080001, 0x40000, CRC(cef0a821) SHA1(c7a35048d5ebf3f09abf9d27f91d12adc03befeb) )
	ROM_LOAD16_BYTE( "ct_mr4.rom",  0x080000, 0x40000, CRC(b84fda09) SHA1(3ae0c0ec6c398dea17e248b017ea3e2f6c3571e1) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* Sprites */
	ROM_LOAD( "ct_mr3.rom",  0x000000, 0x80000, CRC(1ac2a461) SHA1(17436f5dcf29041ca5f470dfae538e4fc12153cc) )
	ROM_LOAD( "ct_ep5.rom",  0x080000, 0x10000, CRC(972faddb) SHA1(f2b211e8f8301667e6c9a3ce9612e39b16e66a67) )
	ROM_LOAD( "ct_mr2.rom",  0x100000, 0x80000, CRC(8c796707) SHA1(7417ad0413083876ed65a8612845ccb0d2717530) )
	ROM_LOAD( "ct_ep6.rom",  0x180000, 0x10000, CRC(eb3ab374) SHA1(db66cb7976c111fa76a3a211e96ad1d7b78ce0ad) )
	ROM_LOAD( "ct_mr1.rom",  0x200000, 0x80000, CRC(1c9badbd) SHA1(d28f6d684d88448eaa3feae0bba2f5a836d89bd7) )
	ROM_LOAD( "ct_ep7.rom",  0x280000, 0x10000, CRC(c602ac97) SHA1(44440739636b684c6dcac837f59664120c9ba5f3) )
	ROM_LOAD( "ct_mr0.rom",  0x300000, 0x80000, CRC(ba73c49e) SHA1(830099027ede1f7c56bb0bf3cdef3018b92e0b87) )
	ROM_LOAD( "ct_ep8.rom",  0x380000, 0x10000, CRC(4da1d8e5) SHA1(568e9e8d00f1b1ca27c28df5fc0ffc74ad91da7e) )

	ROM_REGION( 0x040000, "oki", 0 )    /* ADPCM Samples */
	ROM_LOAD( "ct_mr8.rom",   0x020000, 0x20000, CRC(9963a6be) SHA1(b09b8f52b7fe5ceac34bc7d70c235d60d808fcbf) )
	ROM_CONTINUE(             0x000000, 0x20000 )
ROM_END

ROM_START( ctribeb2 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 64k for cpu code */
	ROM_LOAD16_BYTE( "1.bin",  0x00001, 0x20000, CRC(9cfa997f) SHA1(ee49b4b9e9cd29616f244fdf3912ef743e2404ce) )
	ROM_LOAD16_BYTE( "3.bin",  0x00000, 0x20000, CRC(2ece8681) SHA1(17ee2ceb893e2eb08fa4cabcdebcec02bee16cda) )
	ROM_LOAD16_BYTE( "2.bin",  0x40001, 0x10000, CRC(8c2c6dbd) SHA1(b99b9be6e0bdc8340fedd258819c4df587926a84) )
	/* No EVEN rom! */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound cpu code */
	ROM_LOAD( "6.bin",   0x00000, 0x10000, CRC(0101df2d) SHA1(35e1efa4a11c0f9d9db5ee057926e5de29c3a4c1) )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* Background */
	ROM_LOAD16_BYTE( "7.bin",   0x000001, 0x40000, CRC(a8b773f1) SHA1(999e41dfeb3fb937da769c4a33bb29bf4076dc63) )
	ROM_LOAD16_BYTE( "8.bin",   0x000000, 0x40000, CRC(617530fc) SHA1(b9155ed0ae1437bf4d0b7a95e769bc05a820ecec) )
	ROM_LOAD16_BYTE( "11.bin",  0x080001, 0x40000, CRC(cef0a821) SHA1(c7a35048d5ebf3f09abf9d27f91d12adc03befeb) )
	ROM_LOAD16_BYTE( "12.bin",  0x080000, 0x40000, CRC(b84fda09) SHA1(3ae0c0ec6c398dea17e248b017ea3e2f6c3571e1) )
	// a second copy of the 2nd half of the above roms? did the bootleg pull the data for one of the layers from here instead?
	ROM_LOAD16_BYTE( "9.bin",   0x040001, 0x20000, CRC(2719d7ce) SHA1(35275d32b584c477033037bc041a3687ecca412d) )
	ROM_LOAD16_BYTE( "10.bin",  0x040000, 0x20000, CRC(753a4f53) SHA1(c76a449ef29dde671196cda1f128b0b2d4839a97) )
	ROM_LOAD16_BYTE( "13.bin",  0x0c0001, 0x20000, CRC(59e01fe1) SHA1(67f5a4e9c9e9ebc6218b7c2ede0e5ff51682ee2f) )
	ROM_LOAD16_BYTE( "14.bin",  0x0c0000, 0x20000, CRC(a69ab4f3) SHA1(bc99c6a587c972cb5c9e719c53ef921a28f1498e) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* Sprites */
	ROM_LOAD( "34.bin",  0x000000, 0x20000, CRC(5b498f0e) SHA1(9d93ca1f44e1f04eb3b66db6027130683c9431a2) )
	ROM_LOAD( "33.bin",  0x020000, 0x20000, CRC(14d79049) SHA1(250c87aa238fc794cd0f07e3388b97137cc1228f) )
	ROM_LOAD( "32.bin",  0x040000, 0x20000, CRC(9631ea23) SHA1(de0a6595731d185ea18959bd04e3ac9e4261a8f8) )
	ROM_LOAD( "31.bin",  0x060000, 0x20000, CRC(0ca8d3b9) SHA1(c48e7c456acb9deb7f01c19ab9a66360af09a13d) )
	ROM_LOAD( "30.bin",  0x080000, 0x10000, CRC(972faddb) SHA1(f2b211e8f8301667e6c9a3ce9612e39b16e66a67) )
	ROM_LOAD( "29.bin",  0x100000, 0x20000, CRC(479ae8ea) SHA1(6bfbab5ed7ae7275be0c177c7e39cff19210c0ab) )
	ROM_LOAD( "28.bin",  0x120000, 0x20000, CRC(95598bbf) SHA1(2a155bbb09dc4efca6e1a6b847829f7ec09d323e) )
	ROM_LOAD( "27.bin",  0x140000, 0x20000, CRC(4a3d006d) SHA1(d32fa627e3a4683101aff233bfc8a016c78a1702) )
	ROM_LOAD( "26.bin",  0x160000, 0x20000, CRC(aa34a3cb) SHA1(9bcf5db3a89a300468e9300a9fc5fd33d1ba60de) )
	ROM_LOAD( "25.bin",  0x180000, 0x10000, CRC(eb3ab374) SHA1(db66cb7976c111fa76a3a211e96ad1d7b78ce0ad) )
	ROM_LOAD( "24.bin",  0x200000, 0x20000, CRC(d60bbff0) SHA1(b5e978b1d58f4d0ff05e51b728bb3ec06eea7d08) )
	ROM_LOAD( "23.bin",  0x220000, 0x20000, CRC(d9595c47) SHA1(323e572a022d5297c727dc8b1717269c5b9134b7) )
	ROM_LOAD( "22.bin",  0x240000, 0x20000, CRC(5a19a911) SHA1(d7ce955d3127b57ee560379055d71bacf3a9d6a1) )
	ROM_LOAD( "21.bin",  0x260000, 0x20000, CRC(071360f9) SHA1(8269115484f3ceb69c3b2a215f684dd31a366989) )
	ROM_LOAD( "20.bin",  0x280000, 0x10000, CRC(c602ac97) SHA1(44440739636b684c6dcac837f59664120c9ba5f3) )
	ROM_LOAD( "19.bin",  0x300000, 0x20000, CRC(8d22736a) SHA1(d17f92544efebbdf89ca9fd11ab7c16d2636f175) )
	ROM_LOAD( "18.bin",  0x320000, 0x20000, CRC(0f157822) SHA1(a1e16e4036b0c68c3f58cb0520f8120fe60b0dfa) )
	ROM_LOAD( "17.bin",  0x340000, 0x20000, CRC(7f48c824) SHA1(af6b48dbcf88cfcaecb8cec5dd9eb4c25f4bf9a8) )
	ROM_LOAD( "16.bin",  0x360000, 0x20000, CRC(cd1e9bd5) SHA1(1f956ab476b2c403c0dbf9e6169cfe2c51bb45ac) )
	ROM_LOAD( "15.bin",  0x380000, 0x10000, CRC(4da1d8e5) SHA1(568e9e8d00f1b1ca27c28df5fc0ffc74ad91da7e) )

	ROM_REGION( 0x040000, "oki", 0 )    /* ADPCM Samples */
	ROM_LOAD( "5.bin",   0x000000, 0x20000, CRC(f92a7f4a) SHA1(3717ef64876be9ada378b449749918ce9072073a) )
	ROM_LOAD( "4.bin",   0x020000, 0x20000, CRC(1a3a0b39) SHA1(8847530027cf4be03ffbc6d78dee97b459d03a04) )
ROM_END


ROM_START( wwfwfest )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* Main CPU  (68000) */
	ROM_LOAD16_BYTE( "31a13-2.ic19", 0x00001, 0x40000, CRC(7175bca7) SHA1(992b47a787b5bc2a5a381ec78b8dfaf7d42c614b) )
	ROM_LOAD16_BYTE( "31a14-2.ic18", 0x00000, 0x40000, CRC(5d06bfd1) SHA1(39a93da662158aa5a9953dcabfcb47c2fc196dc7) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU (Z80)  */
	ROM_LOAD( "31a11-2.ic42", 0x00000, 0x10000, CRC(5ddebfea) SHA1(30073963e965250d94f0dc3bd261a054850adf95) )

	ROM_REGION( 0x80000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "31j10.ic73",   0x00000, 0x80000, CRC(6c522edb) SHA1(8005d59c94160638ba2ea7caf4e991fff03003d5) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* FG0 Tiles (8x8) */
	ROM_LOAD( "31a12-0.ic33", 0x00000, 0x20000, CRC(d0803e20) SHA1(b68758e9a5522396f831a3972571f8aed54c64de) )

	ROM_REGION( 0x800000, "gfx2", 0 ) /* SPR Tiles (16x16), 27080 Mask ROM's */
	ROM_LOAD( "31j3.ic9",     0x000000, 0x100000, CRC(e395cf1d) SHA1(241f98145e295993c9b6a44dc087a9b61fbc9a6f) ) /* Tiles 0 */
	ROM_LOAD( "31j2.ic8",     0x100000, 0x100000, CRC(b5a97465) SHA1(08d82c29a5c02b83fdbd0bad649b74eb35ab7e54) ) /* Tiles 1 */
	ROM_LOAD( "31j5.ic11",    0x200000, 0x100000, CRC(2ce545e8) SHA1(82173e58a8476a6fe9d2c990fce1f71af117a0ea) ) /* Tiles 0 */
	ROM_LOAD( "31j4.ic10",    0x300000, 0x100000, CRC(00edb66a) SHA1(926606d1923936b6e75391b1ab03b369d9822d13) ) /* Tiles 1 */
	ROM_LOAD( "31j6.ic12",    0x400000, 0x100000, CRC(79956cf8) SHA1(52207263620a6b6dde66d3f8749b772577899ea5) ) /* Tiles 0 */
	ROM_LOAD( "31j7.ic13",    0x500000, 0x100000, CRC(74d774c3) SHA1(a723ac5d481bf91b12e17652fbb2d869c886dec0) ) /* Tiles 1 */
	ROM_LOAD( "31j9.ic15",    0x600000, 0x100000, CRC(dd387289) SHA1(2cad42d4e7cd1a49346f844058ae18c38bc686a8) ) /* Tiles 0 */
	ROM_LOAD( "31j8.ic14",    0x700000, 0x100000, CRC(44abe127) SHA1(c723e1dea117534e976d2d383e634faf073cd57b) ) /* Tiles 1 */

	ROM_REGION( 0x80000, "gfx3", 0 ) /* BG0 / BG1 Tiles (16x16) */
	ROM_LOAD( "31j0.ic1",     0x40000, 0x40000, CRC(8a12b450) SHA1(2e15c949efcda8bb6f11afe3ff07ba1dee9c771c) ) /* 0,1 */
	ROM_LOAD( "31j1.ic2",     0x00000, 0x40000, CRC(82ed7155) SHA1(b338e1150ffe3277c11d4d6e801a7d3bd7c58492) ) /* 2,3 */
ROM_END

ROM_START( wwfwfesta )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* Main CPU  (68000) */
	ROM_LOAD16_BYTE( "wf_18.rom", 0x00000, 0x40000, CRC(933ea1a0) SHA1(61da142cfa7abd3b77ab21979c061a078c0d0c63) )
	ROM_LOAD16_BYTE( "wf_19.rom", 0x00001, 0x40000, CRC(bd02e3c4) SHA1(7ae63e48caf9919ce7b63b4c5aa9474ba8c336da) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU (Z80)  */
	ROM_LOAD( "31a11-2.ic42", 0x00000, 0x10000, CRC(5ddebfea) SHA1(30073963e965250d94f0dc3bd261a054850adf95) )

	ROM_REGION( 0x80000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "31j10.ic73",   0x00000, 0x80000, CRC(6c522edb) SHA1(8005d59c94160638ba2ea7caf4e991fff03003d5) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* FG0 Tiles (8x8) */
	ROM_LOAD( "31e12-0.ic33", 0x00000, 0x20000, CRC(06f22615) SHA1(2e9418e372da85ea597977d912d8b35753655f4e) )

	ROM_REGION( 0x800000, "gfx2", 0 ) /* SPR Tiles (16x16), 27080 Mask ROM's */
	ROM_LOAD( "31j3.ic9",     0x000000, 0x100000, CRC(e395cf1d) SHA1(241f98145e295993c9b6a44dc087a9b61fbc9a6f) ) /* Tiles 0 */
	ROM_LOAD( "31j2.ic8",     0x100000, 0x100000, CRC(b5a97465) SHA1(08d82c29a5c02b83fdbd0bad649b74eb35ab7e54) ) /* Tiles 1 */
	ROM_LOAD( "31j5.ic11",    0x200000, 0x100000, CRC(2ce545e8) SHA1(82173e58a8476a6fe9d2c990fce1f71af117a0ea) ) /* Tiles 0 */
	ROM_LOAD( "31j4.ic10",    0x300000, 0x100000, CRC(00edb66a) SHA1(926606d1923936b6e75391b1ab03b369d9822d13) ) /* Tiles 1 */
	ROM_LOAD( "31j6.ic12",    0x400000, 0x100000, CRC(79956cf8) SHA1(52207263620a6b6dde66d3f8749b772577899ea5) ) /* Tiles 0 */
	ROM_LOAD( "31j7.ic13",    0x500000, 0x100000, CRC(74d774c3) SHA1(a723ac5d481bf91b12e17652fbb2d869c886dec0) ) /* Tiles 1 */
	ROM_LOAD( "31j9.ic15",    0x600000, 0x100000, CRC(dd387289) SHA1(2cad42d4e7cd1a49346f844058ae18c38bc686a8) ) /* Tiles 0 */
	ROM_LOAD( "31j8.ic14",    0x700000, 0x100000, CRC(44abe127) SHA1(c723e1dea117534e976d2d383e634faf073cd57b) ) /* Tiles 1 */

	ROM_REGION( 0x80000, "gfx3", 0 ) /* BG0 / BG1 Tiles (16x16) */
	ROM_LOAD( "31j0.ic1",     0x40000, 0x40000, CRC(8a12b450) SHA1(2e15c949efcda8bb6f11afe3ff07ba1dee9c771c) ) /* 0,1 */
	ROM_LOAD( "31j1.ic2",     0x00000, 0x40000, CRC(82ed7155) SHA1(b338e1150ffe3277c11d4d6e801a7d3bd7c58492) ) /* 2,3 */
ROM_END

ROM_START( wwfwfestb )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* Main CPU  (68000) */
	ROM_LOAD16_BYTE( "3",      0x00000, 0x40000, CRC(ea73369c) SHA1(be614a342f9014251810fa30ec56fec03f7c8ef3) )
	ROM_LOAD16_BYTE( "2",      0x00001, 0x40000, CRC(632bb3a4) SHA1(9c04fed5aeefc683810cfbd9b3318e155ed9813f) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU (Z80)  */
	ROM_LOAD( "1",             0x00000, 0x10000, CRC(d9e8cda2) SHA1(754c73cd341d51ffd35cdb62155a3f061416c9ba) )

	ROM_REGION( 0x80000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "wf_73a.rom",    0x00000, 0x80000, CRC(6c522edb) SHA1(8005d59c94160638ba2ea7caf4e991fff03003d5) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* FG0 Tiles (8x8) */
	ROM_LOAD( "4",             0x00000, 0x20000, CRC(520ef575) SHA1(99a5e9b94e9234851c6b504d58939ad84e0d6589) )

	ROM_REGION( 0x800000, "gfx2", 0 ) /* SPR Tiles (16x16) */
	ROM_LOAD( "wf_09.rom",    0x000000, 0x100000, CRC(e395cf1d) SHA1(241f98145e295993c9b6a44dc087a9b61fbc9a6f) ) /* Tiles 0 */
	ROM_LOAD( "wf_08.rom",    0x100000, 0x100000, CRC(b5a97465) SHA1(08d82c29a5c02b83fdbd0bad649b74eb35ab7e54) ) /* Tiles 1 */
	ROM_LOAD( "wf_11.rom",    0x200000, 0x100000, CRC(2ce545e8) SHA1(82173e58a8476a6fe9d2c990fce1f71af117a0ea) ) /* Tiles 0 */
	ROM_LOAD( "wf_10.rom",    0x300000, 0x100000, CRC(00edb66a) SHA1(926606d1923936b6e75391b1ab03b369d9822d13) ) /* Tiles 1 */
	ROM_LOAD( "wf_12.rom",    0x400000, 0x100000, CRC(79956cf8) SHA1(52207263620a6b6dde66d3f8749b772577899ea5) ) /* Tiles 0 */
	ROM_LOAD( "wf_13.rom",    0x500000, 0x100000, CRC(74d774c3) SHA1(a723ac5d481bf91b12e17652fbb2d869c886dec0) ) /* Tiles 1 */
	ROM_LOAD( "wf_15.rom",    0x600000, 0x100000, CRC(dd387289) SHA1(2cad42d4e7cd1a49346f844058ae18c38bc686a8) ) /* Tiles 0 */
	ROM_LOAD( "wf_14.rom",    0x700000, 0x100000, CRC(44abe127) SHA1(c723e1dea117534e976d2d383e634faf073cd57b) ) /* Tiles 1 */

	ROM_REGION( 0x80000, "gfx3", 0 ) /* BG0 / BG1 Tiles (16x16) */
	ROM_LOAD16_BYTE( "5",     0x40000, 0x20000, CRC(35e4d6eb) SHA1(d2a12bde268bc0734e6806ff5302b8c3dcc17280) ) /* 0 */
	ROM_LOAD16_BYTE( "6",     0x40001, 0x20000, CRC(a054a5b2) SHA1(d6ed5d5a20acb7cdbaee8e3f520873650529c0ae) ) /* 1 */
	ROM_LOAD16_BYTE( "7",     0x00000, 0x20000, CRC(101f0136) SHA1(2ccd641e49cdd3f5243ebe8c52c492842d62f5b8) ) /* 2 */
	ROM_LOAD16_BYTE( "8",     0x00001, 0x20000, CRC(7b2ecba7) SHA1(1ed2451132448930ac4afcdc67ca14e3e922863e) ) /* 3 */
ROM_END

ROM_START( wwfwfestj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* Main CPU  (68000) */
	ROM_LOAD16_BYTE( "31j13-0.ic19", 0x00001, 0x40000, CRC(2147780d) SHA1(9a7a5db06117f3780e084d3f0c7b642ff8a9db55) )
	ROM_LOAD16_BYTE( "31j14-0.ic18", 0x00000, 0x40000, CRC(d76fc747) SHA1(5f6819bc61756d1df4ac0776ac420a59c438cf8a) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU (Z80)  */
	ROM_LOAD( "31a11-2.ic42", 0x00000, 0x10000, CRC(5ddebfea) SHA1(30073963e965250d94f0dc3bd261a054850adf95) )

	ROM_REGION( 0x80000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "31j10.ic73",   0x00000, 0x80000, CRC(6c522edb) SHA1(8005d59c94160638ba2ea7caf4e991fff03003d5) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* FG0 Tiles (8x8) */
	ROM_LOAD( "31j12-0.ic33", 0x00000, 0x20000, CRC(f4821fe0) SHA1(e5faa9860e9d4e75393b64ca85a8bfc4852fd4fd) )

	ROM_REGION( 0x800000, "gfx2", 0 ) /* SPR Tiles (16x16), 27080 Mask ROM's */
	ROM_LOAD( "31j3.ic9",     0x000000, 0x100000, CRC(e395cf1d) SHA1(241f98145e295993c9b6a44dc087a9b61fbc9a6f) ) /* Tiles 0 */
	ROM_LOAD( "31j2.ic8",     0x100000, 0x100000, CRC(b5a97465) SHA1(08d82c29a5c02b83fdbd0bad649b74eb35ab7e54) ) /* Tiles 1 */
	ROM_LOAD( "31j5.ic11",    0x200000, 0x100000, CRC(2ce545e8) SHA1(82173e58a8476a6fe9d2c990fce1f71af117a0ea) ) /* Tiles 0 */
	ROM_LOAD( "31j4.ic10",    0x300000, 0x100000, CRC(00edb66a) SHA1(926606d1923936b6e75391b1ab03b369d9822d13) ) /* Tiles 1 */
	ROM_LOAD( "31j6.ic12",    0x400000, 0x100000, CRC(79956cf8) SHA1(52207263620a6b6dde66d3f8749b772577899ea5) ) /* Tiles 0 */
	ROM_LOAD( "31j7.ic13",    0x500000, 0x100000, CRC(74d774c3) SHA1(a723ac5d481bf91b12e17652fbb2d869c886dec0) ) /* Tiles 1 */
	ROM_LOAD( "31j9.ic15",    0x600000, 0x100000, CRC(dd387289) SHA1(2cad42d4e7cd1a49346f844058ae18c38bc686a8) ) /* Tiles 0 */
	ROM_LOAD( "31j8.ic14",    0x700000, 0x100000, CRC(44abe127) SHA1(c723e1dea117534e976d2d383e634faf073cd57b) ) /* Tiles 1 */

	ROM_REGION( 0x80000, "gfx3", 0 ) /* BG0 / BG1 Tiles (16x16) */
	ROM_LOAD( "31j0.ic1",     0x40000, 0x40000, CRC(8a12b450) SHA1(2e15c949efcda8bb6f11afe3ff07ba1dee9c771c) ) /* 0,1 */
	ROM_LOAD( "31j1.ic2",     0x00000, 0x40000, CRC(82ed7155) SHA1(b338e1150ffe3277c11d4d6e801a7d3bd7c58492) ) /* 2,3 */
ROM_END

ROM_START( wwfwfestk )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* Main CPU  (68000) */
	ROM_LOAD16_BYTE( "31e13-0.ic19", 0x00001, 0x40000, CRC(774a26a7) SHA1(30e00bff9027a0ae971f8820ca6c3e4cdea82994) )
	ROM_LOAD16_BYTE( "31e14-0.ic18", 0x00000, 0x40000, CRC(05bbb807) SHA1(1dc2ddd9ae498468a97e002f78e7f3a331d802d1) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU (Z80)  */
	ROM_LOAD( "31a11-2.ic42", 0x00000, 0x10000, CRC(5ddebfea) SHA1(30073963e965250d94f0dc3bd261a054850adf95) )

	ROM_REGION( 0x80000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "31j10.ic73",   0x00000, 0x80000, CRC(6c522edb) SHA1(8005d59c94160638ba2ea7caf4e991fff03003d5) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* FG0 Tiles (8x8) */
	ROM_LOAD( "31e12-0.ic33", 0x00000, 0x20000, CRC(06f22615) SHA1(2e9418e372da85ea597977d912d8b35753655f4e) )

	ROM_REGION( 0x800000, "gfx2", 0 ) /* SPR Tiles (16x16), 27080 Mask ROM's */
	ROM_LOAD( "31j3.ic9",     0x000000, 0x100000, CRC(e395cf1d) SHA1(241f98145e295993c9b6a44dc087a9b61fbc9a6f) ) /* Tiles 0 */
	ROM_LOAD( "31j2.ic8",     0x100000, 0x100000, CRC(b5a97465) SHA1(08d82c29a5c02b83fdbd0bad649b74eb35ab7e54) ) /* Tiles 1 */
	ROM_LOAD( "31j5.ic11",    0x200000, 0x100000, CRC(2ce545e8) SHA1(82173e58a8476a6fe9d2c990fce1f71af117a0ea) ) /* Tiles 0 */
	ROM_LOAD( "31j4.ic10",    0x300000, 0x100000, CRC(00edb66a) SHA1(926606d1923936b6e75391b1ab03b369d9822d13) ) /* Tiles 1 */
	ROM_LOAD( "31j6.ic12",    0x400000, 0x100000, CRC(79956cf8) SHA1(52207263620a6b6dde66d3f8749b772577899ea5) ) /* Tiles 0 */
	ROM_LOAD( "31j7.ic13",    0x500000, 0x100000, CRC(74d774c3) SHA1(a723ac5d481bf91b12e17652fbb2d869c886dec0) ) /* Tiles 1 */
	ROM_LOAD( "31j9.ic15",    0x600000, 0x100000, CRC(dd387289) SHA1(2cad42d4e7cd1a49346f844058ae18c38bc686a8) ) /* Tiles 0 */
	ROM_LOAD( "31j8.ic14",    0x700000, 0x100000, CRC(44abe127) SHA1(c723e1dea117534e976d2d383e634faf073cd57b) ) /* Tiles 1 */

	ROM_REGION( 0x80000, "gfx3", 0 ) /* BG0 / BG1 Tiles (16x16) */
	ROM_LOAD( "31j0.ic1",     0x40000, 0x40000, CRC(8a12b450) SHA1(2e15c949efcda8bb6f11afe3ff07ba1dee9c771c) ) /* 0,1 */
	ROM_LOAD( "31j1.ic2",     0x00000, 0x40000, CRC(82ed7155) SHA1(b338e1150ffe3277c11d4d6e801a7d3bd7c58492) ) /* 2,3 */
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1990, ddragon3, 0,        ddragon3, ddragon3, driver_device, 0, ROT0, "Technos Japan", "Double Dragon 3 - The Rosetta Stone (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, ddragon3j,ddragon3, ddragon3, ddragon3, driver_device, 0, ROT0, "Technos Japan", "Double Dragon 3 - The Rosetta Stone (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, ddragon3p,ddragon3, ddragon3, ddragon3, driver_device, 0, ROT0, "Technos Japan", "Double Dragon 3 - The Rosetta Stone (prototype)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, ddragon3b,ddragon3, ddragon3b,ddragon3b, driver_device,0, ROT0, "bootleg", "Double Dragon 3 - The Rosetta Stone (bootleg)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, ctribe,   0,        ctribe,   ctribe, driver_device,   0, ROT0, "Technos Japan", "The Combatribes (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, ctribe1,  ctribe,   ctribe,   ctribe, driver_device,   0, ROT0, "Technos Japan", "The Combatribes (US set 1?)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, ctribeo,  ctribe,   ctribe,   ctribe, driver_device,   0, ROT0, "Technos Japan", "The Combatribes (US, older)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, ctribej,  ctribe,   ctribe,   ctribe, driver_device,   0, ROT0, "Technos Japan", "The Combatribes (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, ctribeb,  ctribe,   ctribe,   ctribeb, driver_device,  0, ROT0, "bootleg", "The Combatribes (bootleg set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, ctribeb2, ctribe,   ctribe,   ctribeb, driver_device,  0, ROT0, "bootleg", "The Combatribes (bootleg set 2)", MACHINE_SUPPORTS_SAVE )

GAME( 1991, wwfwfest,  0,        wwfwfest, wwfwfest,  driver_device, 0, ROT0, "Technos Japan",                 "WWF WrestleFest (US set 1)",   MACHINE_SUPPORTS_SAVE )
GAME( 1991, wwfwfesta, wwfwfest, wwfwfest, wwfwfest,  driver_device, 0, ROT0, "Technos Japan (Tecmo license)", "WWF WrestleFest (US Tecmo)",   MACHINE_SUPPORTS_SAVE )
GAME( 1991, wwfwfestb, wwfwfest, wwfwfstb, wwfwfest,  driver_device, 0, ROT0, "bootleg",                       "WWF WrestleFest (US bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, wwfwfestj, wwfwfest, wwfwfest, wwfwfesta, driver_device, 0, ROT0, "Technos Japan (Tecmo license)", "WWF WrestleFest (Japan)",      MACHINE_SUPPORTS_SAVE )
GAME( 1991, wwfwfestk, wwfwfest, wwfwfest, wwfwfesta, driver_device, 0, ROT0, "Technos Japan (Tecmo license)", "WWF WrestleFest (Korea)",      MACHINE_SUPPORTS_SAVE )
