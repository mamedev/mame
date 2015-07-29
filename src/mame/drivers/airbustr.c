// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                                Air Buster
                            (C) 1990  Kaneko

                    driver by Luca Elia (l.elia@tin.it)

CPU   : Z-80 x 3
SOUND : YM2203C     M6295
OSC.  : 12.000MHz   16.000MHz

                    Interesting routines (main cpu)
                    -------------------------------

fd-fe   address of int: 0x38    (must alternate? see e600/1)
ff-100  address of int: 0x16
66      print "sub cpu caused nmi" and die!

7       after tests

1497    print string following call: (char)*,0. program continues after 0.
        base addr = c300 + HL & 08ff, DE=xy , BC=dxdy
        +0<-(e61b)  +100<-D +200<-E +300<-char  +400<-(e61c)

1642    A<- buttons status (bits 0&1)

                    Interesting locations (main cpu)
                    --------------------------------

2907    table of routines (f150 index = 1-3f, copied to e612)
        e60e<-address of routine, f150<-0. e60e is used until f150!=0

    1:  2bf4    service mode                                next

    2:  2d33    3:  16bd        4:  2dcb        5:  2fcf
    6:  3262    7:  32b8

    8:  335d>   print gfx/color test page                   next
    9:  33c0>   handle the above page

    a:  29c6        b:  2a24        c:  16ce

    d:  3e7e>   *
    e:  3ec5>   print "Sub Cpu / Ram Error"; **
    f:  3e17>   print "Coin error"; **
    10: 3528>   print (c) notice, not shown                 next
    11: 3730>   show (c) notice, wait 0x100 calls           next

    12:     9658    13: 97c3        14: a9fa        15: aa6e
    16-19:  2985    1a: 9c2e        1b: 9ffa        1c: 29c6

    1d: 2988>   *

    1e: a2c4        1f: a31a        20: a32f        21: a344
    22: a359        23: a36e        24: a383        25: a398
    26: a3ad        27: a3c2        28: a3d7        29: a3f1
    2a: a40e        2b: a4e5        2c: a69d        2d: adb8
    2e: ade9        2f: 2b8b        30: a823

    31: 3d17>   print "warm up, wait few mins. secs left: 00"   next
    32: 3dc0>   pause (e624 counter).e626                       next

    33: 96b4        34: 97ad

    35-3f:  3e7e>   *

*   Print "Command Error [$xx]" where xx is last routine index (e612)
**  ld (0000h),A (??) ; loop

3cd7    hiscores table (0x40 bytes, copied to e160)
        Try entering TERU as your name :)

7fff    country code: 0 <-> Japan; else World

e615    rank:   0-easy  1-normal    2-hard  3-hardest
e624    sound code during sound test

-- Shared RAM --

f148<-  sound code (copied from e624)
f14a->  read on nmi routine. main cpu writes the value and writes to port 02
f150<-  index of table of routines at 2907

----------------





                    Interesting routines (sub cpu)
                    -------------------------------

491     copy palette data   d000<-f200(a0)  d200<-f300(a0)  d400<-f400(200)

61c     f150<-A     f151<-A (routine index of main cpu!)
        if dsw1-2 active, it does nothing (?!)

c8c     c000-c7ff<-0    c800-cfff<-0    f600<-f200(400)
1750    copies 10 lines of 20 bytes from 289e to f800

22cd    copy 0x100 bytes
22cf    copy 0x0FF bytes
238d    copy 0x0A0 bytes

                    Interesting locations (sub cpu)
                    --------------------------------

fd-fe   address of int: 0x36e   (same as 38)
ff-100  address of int: 0x4b0

-- Shared RAM --

f000    credits

f001/d<-IN 24 (Service)
f00e<-  bank
f002<-  dsw1 (cpl'd)
f003<-  dsw2 (cpl'd)
f004<-  IN 20 (cpl'd) (player 1)
f005<-  IN 22 (cpl'd) (player 2)
f006<-  start lives: dsw-2 & 0x30 index; values: 3,4,5,7        (5da table)
f007    current lives p1
f008    current lives p2

f009<-  coin/credit 1: dsw-1 & 0x30 index; values: 11,12,21,23  (5de table)
f00a<-  coin 1
f00b<-  coin/credit 2: dsw-1 & 0xc0 index; values: 11,12,21,23  (5e2 table)
f00c<-  coin 2

f00f    ?? outa (28h)
f010    written by sub cpu, bit 4 read by main cpu.
        bit 0   p1 playing
        bit 1   p2 playing

f014    index (1-f) of routine called during int 36e (table at c3f)
    1:  62b         2:  66a         3:  6ad         4:  79f
    5:  7e0         6:  81b         7:  8a7         8:  8e9
    9:  b02         a:  0           b:  0           c:  bfc
    d:  c0d         e:  a6f         f:  ac3

f015    index of the routine to call, usually the one selected by f014
        but sometimes written directly.

Scroll registers: ports 04/06/08/0a/0c, written in sequence (101f)
port 06 <- f100 + f140  x       port 04 <- f104 + f142  y
port 0a <- f120 + f140  x       port 08 <- f124 + f142  y
port 0c <- f14c = bit 0/1/2/3 = port 6/4/a/8 val < FF

f148->  sound code (from main cpu)
f14c    scroll regs high bits

----------------








                    Interesting routines (sound cpu)
                    -------------------------------

50a     6295
521     6295
a96     2203 reg<-B     val<-A

                    Interesting locations (sound cpu)
                    ---------------------------------

c715
c716    pending sound command
c760    rom bank


                                To Do
                                -----

- Is the sub cpu / sound cpu communication status port (0e) correct ?
- Main cpu: port  01 ? boot sub/sound cpu?
- Sub  cpu: port 0x38 ? irq ack?
- incomplete DSW's
- Spriteram low 0x300 bytes (priority?)

*/

/*
**
**              Main cpu data
**
*/

/*  Runs in IM 2    fd-fe   address of int: 0x38
                    ff-100  address of int: 0x16    */

/*
**
**              Sub cpu data
**
**
*/

/*  Runs in IM 2    fd-fe   address of int: 0x36e   (same as 0x38)
                    ff-100  address of int: 0x4b0   (only writes to port 38h)   */
/*
   Sub cpu and Sound cpu communicate bidirectionally:

       Sub   cpu writes to soundlatch,  reads from soundlatch2
       Sound cpu writes to soundlatch2, reads from soundlatch

   Each latch raises a flag when it's been written.
   The flag is cleared when the latch is read.

Code at 505: waits for bit 1 to go low, writes command, waits for bit
0 to go low, reads back value. Code at 3b2 waits bit 2 to go high
(called during int fd)

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "sound/okim6295.h"
#include "includes/airbustr.h"

/* Read/Write Handlers */
READ8_MEMBER(airbustr_state::devram_r)
{
	// There's an MCU here, possibly
	switch (offset)
	{
		/* Reading efe0 probably resets a watchdog mechanism
		   that would reset the main cpu. We avoid this and patch
		   the rom instead (main cpu has to be reset once at startup) */
		case 0xfe0:
			return watchdog_reset_r(space, 0);

		/* Reading a word at eff2 probably yelds the product
		   of the words written to eff0 and eff2 */
		case 0xff2:
		case 0xff3:
		{
			int x = (m_devram[0xff0] + m_devram[0xff1] * 256) * (m_devram[0xff2] + m_devram[0xff3] * 256);
			if (offset == 0xff2)
				return (x & 0x00ff) >> 0;
			else
				return (x & 0xff00) >> 8;
		}

		/* Reading eff4, F0 times must yield at most 80-1 consecutive
		   equal values */
		case 0xff4:
			return machine().rand();

		default:
			return m_devram[offset];
	}
}

WRITE8_MEMBER(airbustr_state::master_nmi_trigger_w)
{
	m_slave->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE8_MEMBER(airbustr_state::master_bankswitch_w)
{
	membank("bank1")->set_entry(data & 0x07);
}

WRITE8_MEMBER(airbustr_state::slave_bankswitch_w)
{
	membank("bank2")->set_entry(data & 0x07);

	flip_screen_set(data & 0x10);

	// used at the end of levels, after defeating the boss, to leave trails
	m_pandora->set_clear_bitmap(data & 0x20);
}

WRITE8_MEMBER(airbustr_state::sound_bankswitch_w)
{
	membank("bank3")->set_entry(data & 0x07);
}

READ8_MEMBER(airbustr_state::soundcommand_status_r)
{
	// bits: 2 <-> ?    1 <-> soundlatch full   0 <-> soundlatch2 empty
	return 4 + m_soundlatch_status * 2 + (1 - m_soundlatch2_status);
}

READ8_MEMBER(airbustr_state::soundcommand_r)
{
	m_soundlatch_status = 0;    // soundlatch has been read
	return soundlatch_byte_r(space, 0);
}

READ8_MEMBER(airbustr_state::soundcommand2_r)
{
	m_soundlatch2_status = 0;   // soundlatch2 has been read
	return soundlatch2_byte_r(space, 0);
}

WRITE8_MEMBER(airbustr_state::soundcommand_w)
{
	soundlatch_byte_w(space, 0, data);
	m_soundlatch_status = 1;    // soundlatch has been written
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE); // cause a nmi to sub cpu
}

WRITE8_MEMBER(airbustr_state::soundcommand2_w)
{
	soundlatch2_byte_w(space, 0, data);
	m_soundlatch2_status = 1;   // soundlatch2 has been written
}





WRITE8_MEMBER(airbustr_state::airbustr_coin_counter_w)
{
	coin_counter_w(machine(), 0, data & 1);
	coin_counter_w(machine(), 1, data & 2);
	coin_lockout_w(machine(), 0, ~data & 4);
	coin_lockout_w(machine(), 1, ~data & 8);
}

/* Memory Maps */
static ADDRESS_MAP_START( master_map, AS_PROGRAM, 8, airbustr_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xcfff) AM_DEVREADWRITE("pandora", kaneko_pandora_device, spriteram_r, spriteram_w)
	AM_RANGE(0xd000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xefff) AM_RAM AM_SHARE("devram") // shared with protection device
	AM_RANGE(0xf000, 0xffff) AM_RAM AM_SHARE("share1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( master_io_map, AS_IO, 8, airbustr_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(master_bankswitch_w)
	AM_RANGE(0x01, 0x01) AM_WRITENOP // ???
	AM_RANGE(0x02, 0x02) AM_WRITE(master_nmi_trigger_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( slave_map, AS_PROGRAM, 8, airbustr_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank2")
	AM_RANGE(0xc000, 0xc3ff) AM_RAM_WRITE(airbustr_videoram2_w) AM_SHARE("videoram2")
	AM_RANGE(0xc400, 0xc7ff) AM_RAM_WRITE(airbustr_colorram2_w) AM_SHARE("colorram2")
	AM_RANGE(0xc800, 0xcbff) AM_RAM_WRITE(airbustr_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xcc00, 0xcfff) AM_RAM_WRITE(airbustr_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xd000, 0xd5ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xd600, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_RAM AM_SHARE("share1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( slave_io_map, AS_IO, 8, airbustr_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(slave_bankswitch_w)
	AM_RANGE(0x02, 0x02) AM_READWRITE(soundcommand2_r, soundcommand_w)
	AM_RANGE(0x04, 0x0c) AM_WRITE(airbustr_scrollregs_w)
	AM_RANGE(0x0e, 0x0e) AM_READ(soundcommand_status_r)
	AM_RANGE(0x20, 0x20) AM_READ_PORT("P1")
	AM_RANGE(0x22, 0x22) AM_READ_PORT("P2")
	AM_RANGE(0x24, 0x24) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x28, 0x28) AM_WRITE(airbustr_coin_counter_w)
	AM_RANGE(0x38, 0x38) AM_WRITENOP // irq ack / irq mask
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, airbustr_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank3")
	AM_RANGE(0xc000, 0xdfff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io_map, AS_IO, 8, airbustr_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(sound_bankswitch_w)
	AM_RANGE(0x02, 0x03) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
	AM_RANGE(0x04, 0x04) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x06, 0x06) AM_READWRITE(soundcommand_r, soundcommand2_w)
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( airbustr )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )        // used

	PORT_START("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x08, 0x08, "Coin Mode" )             PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "Mode 1" )            //     routine at 0x056d: 11 21 12 16 (bit 3 active)
	PORT_DIPSETTING(    0x00, "Mode 2" )            //     11 21 13 14 (bit 3 not active)
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW1", 0x08, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW1", 0x08, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW1", 0x08, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )    PORT_CONDITION("DSW1", 0x08, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )    PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )    PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW1", 0x08, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW1", 0x08, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW1", 0x08, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )    PORT_CONDITION("DSW1", 0x08, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )    PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )    PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( airbustrj )
	PORT_INCLUDE(airbustr)

	PORT_MODIFY("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:5,6") // routine at 0x0546 : 11 12 21 23
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
INPUT_PORTS_END

/* Graphics Layout */

static const gfx_layout tile_gfxlayout =
{
	16, 16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{  1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4,
		1*4+32*8, 0*4+32*8, 3*4+32*8, 2*4+32*8, 5*4+32*8, 4*4+32*8, 7*4+32*8, 6*4+32*8 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		0*32+64*8, 1*32+64*8, 2*32+64*8, 3*32+64*8, 4*32+64*8, 5*32+64*8, 6*32+64*8, 7*32+64*8 },
	16*16*4
};

static const gfx_layout sprite_gfxlayout =
{
	16, 16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
		0*4+32*8, 1*4+32*8, 2*4+32*8, 3*4+32*8, 4*4+32*8, 5*4+32*8, 6*4+32*8, 7*4+32*8 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		0*32+64*8, 1*32+64*8, 2*32+64*8, 3*32+64*8, 4*32+64*8, 5*32+64*8, 6*32+64*8, 7*32+64*8 },
	16*16*4
};

/* Graphics Decode Information */

static GFXDECODE_START( airbustr )
	GFXDECODE_ENTRY( "gfx1", 0, tile_gfxlayout,   0, 32 ) // tiles
	GFXDECODE_ENTRY( "gfx2", 0, sprite_gfxlayout, 512, 16 ) // sprites
GFXDECODE_END


/* Interrupt Generators */

/* Main Z80 uses IM2 */
TIMER_DEVICE_CALLBACK_MEMBER(airbustr_state::airbustr_scanline)
{
	int scanline = param;

	if(scanline == 240) // vblank-out irq
		m_master->set_input_line_and_vector(0, HOLD_LINE, 0xff);

	/* Pandora "sprite end dma" irq? TODO: timing is likely off */
	if(scanline == 64)
		m_master->set_input_line_and_vector(0, HOLD_LINE, 0xfd);
}

/* Sub Z80 uses IM2 too, but 0xff irq routine just contains an irq ack in it */
INTERRUPT_GEN_MEMBER(airbustr_state::slave_interrupt)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0xfd);
}

/* Machine Initialization */

void airbustr_state::machine_start()
{
	UINT8 *MASTER = memregion("master")->base();
	UINT8 *SLAVE = memregion("slave")->base();
	UINT8 *AUDIO = memregion("audiocpu")->base();

	membank("bank1")->configure_entries(0, 3, &MASTER[0x00000], 0x4000);
	membank("bank1")->configure_entries(3, 5, &MASTER[0x10000], 0x4000);
	membank("bank2")->configure_entries(0, 3, &SLAVE[0x00000], 0x4000);
	membank("bank2")->configure_entries(3, 5, &SLAVE[0x10000], 0x4000);
	membank("bank3")->configure_entries(0, 3, &AUDIO[0x00000], 0x4000);
	membank("bank3")->configure_entries(3, 5, &AUDIO[0x10000], 0x4000);

	save_item(NAME(m_soundlatch_status));
	save_item(NAME(m_soundlatch2_status));
	save_item(NAME(m_bg_scrollx));
	save_item(NAME(m_bg_scrolly));
	save_item(NAME(m_fg_scrollx));
	save_item(NAME(m_fg_scrolly));
	save_item(NAME(m_highbits));
}

void airbustr_state::machine_reset()
{
	m_soundlatch_status = m_soundlatch2_status = 0;
	m_bg_scrollx = 0;
	m_bg_scrolly = 0;
	m_fg_scrollx = 0;
	m_fg_scrolly = 0;
	m_highbits = 0;

	membank("bank1")->set_entry(0x02);
	membank("bank2")->set_entry(0x02);
	membank("bank3")->set_entry(0x02);
}

/* Machine Driver */

static MACHINE_CONFIG_START( airbustr, airbustr_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("master", Z80, XTAL_12MHz/2)   /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(master_map)
	MCFG_CPU_IO_MAP(master_io_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", airbustr_state, airbustr_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("slave", Z80, XTAL_12MHz/2)    /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(slave_map)
	MCFG_CPU_IO_MAP(slave_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", airbustr_state,  slave_interrupt) /* nmi signal from master cpu */

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_12MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", airbustr_state,  irq0_line_hold)       // nmi are caused by sub cpu writing a sound command

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))  // Palette RAM is filled by sub cpu with data supplied by main cpu
							// Maybe a high value is safer in order to avoid glitches
	MCFG_WATCHDOG_TIME_INIT(attotime::from_seconds(3))  /* a guess, and certainly wrong */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(57.4)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(airbustr_state, screen_update_airbustr)
	MCFG_SCREEN_VBLANK_DRIVER(airbustr_state, screen_eof_airbustr)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", airbustr)
	MCFG_PALETTE_ADD("palette", 768)
	MCFG_PALETTE_FORMAT(xGGGGGRRRRRBBBBB)

	MCFG_DEVICE_ADD("pandora", KANEKO_PANDORA, 0)
	MCFG_KANEKO_PANDORA_GFX_REGION(1)
	MCFG_KANEKO_PANDORA_GFXDECODE("gfxdecode")
	MCFG_KANEKO_PANDORA_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, XTAL_12MHz/4)   /* verified on pcb */
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW1"))       // DSW-1 connected to port A
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW2"))       // DSW-2 connected to port B
	MCFG_SOUND_ROUTE(0, "mono", 0.25)
	MCFG_SOUND_ROUTE(1, "mono", 0.25)
	MCFG_SOUND_ROUTE(2, "mono", 0.25)
	MCFG_SOUND_ROUTE(3, "mono", 0.50)

	MCFG_OKIM6295_ADD("oki", XTAL_12MHz/4, OKIM6295_PIN7_LOW)   /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( airbustrb, airbustr )
	MCFG_WATCHDOG_TIME_INIT(attotime::from_seconds(0)) // no protection device or watchdog
MACHINE_CONFIG_END


/* ROMs */

ROM_START( airbustr )
	ROM_REGION( 0x24000, "master", 0 )
	ROM_LOAD( "pr12.h19",   0x00000, 0x0c000, CRC(91362eb2) SHA1(cd85acfa6542af68dd1cad46f9426a95cfc9432e) )
	ROM_CONTINUE(           0x10000, 0x14000 )

	ROM_REGION( 0x24000, "slave", 0 )
	ROM_LOAD( "pr13.l15",   0x00000, 0x0c000, CRC(13b2257b) SHA1(325efa54e757a1f08caf81801930d61ea4e7b6d4) )
	ROM_CONTINUE(           0x10000, 0x14000 )

	ROM_REGION( 0x24000, "audiocpu", 0 )
	ROM_LOAD( "pr-21.bin",  0x00000, 0x0c000, CRC(6e0a5df0) SHA1(616b7c7aaf52a9a55b63c60717c1866940635cd4) )
	ROM_CONTINUE(           0x10000, 0x14000 )

	ROM_REGION( 0x1000, "mcu", 0 ) //MCU is a 80c51 like DJ Boy / Heavy Unit?
	ROM_LOAD( "i80c51", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "pr-000.bin", 0x00000, 0x80000, CRC(8ca68f0d) SHA1(d60389e7e63e9850bcddecb486558de1414f1276) ) // scrolling layers

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "pr-001.bin", 0x00000, 0x80000, CRC(7e6cb377) SHA1(005290f9f53a0c3a6a9d04486b16b7fd52cc94b6) ) // sprites
	ROM_LOAD( "pr-02.bin",  0x80000, 0x10000, CRC(6bbd5e46) SHA1(26563737f3f91ee0a056d35ce42217bb57d8a081) )

	ROM_REGION( 0x40000, "oki", 0 ) /* OKI-M6295 samples */
	ROM_LOAD( "pr-200.bin", 0x00000, 0x40000, CRC(a4dd3390) SHA1(2d72b46b4979857f6b66489bebda9f48799f59cf) )
ROM_END

ROM_START( airbustrj )
	ROM_REGION( 0x24000, "master", 0 )
	ROM_LOAD( "pr-14j.bin", 0x00000, 0x0c000, CRC(6b9805bd) SHA1(db6df33cf17316a4b81d7731dca9fe8bbf81f014) )
	ROM_CONTINUE(           0x10000, 0x14000 )

	ROM_REGION( 0x24000, "slave", 0 )
	ROM_LOAD( "pr-11j.bin", 0x00000, 0x0c000, CRC(85464124) SHA1(8cce8dfdede48032c40d5f155fd58061866668de) )
	ROM_CONTINUE(           0x10000, 0x14000 )

	ROM_REGION( 0x24000, "audiocpu", 0 )
	ROM_LOAD( "pr-21.bin",  0x00000, 0x0c000, CRC(6e0a5df0) SHA1(616b7c7aaf52a9a55b63c60717c1866940635cd4) )
	ROM_CONTINUE(           0x10000, 0x14000 )

	ROM_REGION( 0x1000, "mcu", 0 ) //MCU is a 80c51 like DJ Boy / Heavy Unit?
	ROM_LOAD( "i80c51", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "pr-000.bin", 0x00000, 0x80000, CRC(8ca68f0d) SHA1(d60389e7e63e9850bcddecb486558de1414f1276) ) // scrolling layers

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "pr-001.bin", 0x000000, 0x80000, CRC(7e6cb377) SHA1(005290f9f53a0c3a6a9d04486b16b7fd52cc94b6) ) // sprites
	ROM_LOAD( "pr-02.bin",  0x080000, 0x10000, CRC(6bbd5e46) SHA1(26563737f3f91ee0a056d35ce42217bb57d8a081) )

	ROM_REGION( 0x40000, "oki", 0 ) /* OKI-M6295 samples */
	ROM_LOAD( "pr-200.bin", 0x00000, 0x40000, CRC(a4dd3390) SHA1(2d72b46b4979857f6b66489bebda9f48799f59cf) )
ROM_END

/*

Differences with the original (when running on the bootleg hardware):

no title screen
long attract modes of every level
slow downs with corrupted screen (you can see the screen being redrawn!) when there are many sprites

the board has 2 oscillators (12 and 16 mhz).  Rom 1 and 2 are program roms. 3 and 4 for sound.
Rom 5 is on a piggyback daughterboard with a z80 and a PAL

*/

ROM_START( airbustrb )
	ROM_REGION( 0x24000, "master", 0 )
	ROM_LOAD( "5.bin",   0x00000, 0x0c000, CRC(9e4216a2) SHA1(46572da4df5a67b10cc3ee21bdc0ec4bcecaaf93) )
	ROM_CONTINUE(           0x10000, 0x14000 )

	ROM_REGION( 0x24000, "slave", 0 )
	ROM_LOAD( "1.bin",   0x00000, 0x0c000, CRC(85464124) SHA1(8cce8dfdede48032c40d5f155fd58061866668de) )
	ROM_CONTINUE(           0x10000, 0x14000 )

	ROM_REGION( 0x24000, "audiocpu", 0 )
	ROM_LOAD( "2.bin",  0x00000, 0x0c000, CRC(6e0a5df0) SHA1(616b7c7aaf52a9a55b63c60717c1866940635cd4) )
	ROM_CONTINUE(           0x10000, 0x14000 )

	ROM_REGION( 0x80000, "gfx1", 0 )
	/* Same content as airbusj, pr-001.bin, different sized roms / interleave */
	ROM_LOAD16_BYTE( "7.bin", 0x00000, 0x20000, CRC(2e3bf0a2) SHA1(84cabc753e5fd1164f0a8a9a9dee7d339a5607c5) )
	ROM_LOAD16_BYTE( "9.bin", 0x00001, 0x20000, CRC(2c23c646) SHA1(41c0f8788c9715918b4138f076415f8640adc483) )
	ROM_LOAD16_BYTE( "6.bin", 0x40000, 0x20000, CRC(0d6cd470) SHA1(329286bc6c9d1eccc74735d1c155a0f5f98f1444) )
	ROM_LOAD16_BYTE( "8.bin", 0x40001, 0x20000, CRC(b3372e51) SHA1(aa8dcbb84c829994ae04ceecbef795ac53e72493) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	/* Same content as airbusj, pr-001.bin, different sized roms */
	ROM_LOAD( "13.bin", 0x00000, 0x20000, CRC(75dee86d) SHA1(fe342fed5bb84ee6f35d3f91987141c559e94d5a) )
	ROM_LOAD( "12.bin", 0x20000, 0x20000, CRC(c98a8333) SHA1(3a990460e232ee07a9297fcffdb02451406f5bf1) )
	ROM_LOAD( "11.bin", 0x40000, 0x20000, CRC(4e9baebd) SHA1(6cf878a3fb3d344e3f5f4d031fbde6f14b653636) )
	ROM_LOAD( "10.bin", 0x60000, 0x20000, CRC(63dc8cd8) SHA1(4b466a8ede4211fa3f51572b223eba8766990d7a) )

	ROM_LOAD( "14.bin", 0x80000, 0x10000, CRC(6bbd5e46) SHA1(26563737f3f91ee0a056d35ce42217bb57d8a081) )

	ROM_REGION( 0x40000, "oki", 0 ) /* OKI-M6295 samples */
	/* Same content as airbusj, pr-200.bin, different sized roms */
	ROM_LOAD( "4.bin", 0x00000, 0x20000, CRC(21d9bfe3) SHA1(4a69458cd2a6309e389c9e7593ae29d3ef0f8daf) )
	ROM_LOAD( "3.bin", 0x20000, 0x20000, CRC(58cd19e2) SHA1(479f22241bf29f7af67d9679fc6c20f10004fdd8) )
ROM_END

/* Driver Initialization */

DRIVER_INIT_MEMBER(airbustr_state,airbustr)
{
	m_master->space(AS_PROGRAM).install_read_handler(0xe000, 0xefff, read8_delegate(FUNC(airbustr_state::devram_r),this)); // protection device lives here
}


/* Game Drivers */

GAME( 1990, airbustr,   0,        airbustr, airbustr, airbustr_state, airbustr, ROT0, "Kaneko (Namco license)", "Air Buster: Trouble Specialty Raid Unit (World)", MACHINE_SUPPORTS_SAVE ) // 891220
GAME( 1990, airbustrj,  airbustr, airbustr, airbustrj, airbustr_state,airbustr, ROT0, "Kaneko (Namco license)", "Air Buster: Trouble Specialty Raid Unit (Japan)", MACHINE_SUPPORTS_SAVE)    // 891229
GAME( 1990, airbustrb,  airbustr, airbustrb,airbustrj, driver_device,0,        ROT0, "bootleg", "Air Buster: Trouble Specialty Raid Unit (bootleg)", MACHINE_SUPPORTS_SAVE)    // based on Japan set (891229)
