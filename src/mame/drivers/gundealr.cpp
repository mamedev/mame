// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Gun Dealer memory map

driver by Nicola Salmoria

Yam! Yam!? runs on the same hardware but has a protection device which can
           access RAM at e000. Program writes to e000 and expects a value back
           at e001, then jumps to subroutines at e010 and e020. Also, the
           player and coin inputs appear magically at e004-e006.

0000-7fff ROM
8000-bfff ROM (banked)
c400-c7ff palette RAM
c800-cfff background video RAM
d000-dfff foreground (scrollable) video RAM.
e000-ffff work RAM

read:
c000      DSW0
c001      DSW1
c004      COIN (Gun Dealer only)
c005      IN1 (Gun Dealer only)
c006      IN0 (Gun Dealer only)

write:
c010-c011 foreground scroll x lo-hi (Yam Yam)
c012-c013 foreground scroll y lo-hi (Yam Yam)
c014      flip screen
c015      Yam Yam only, maybe reset protection device
c016      ROM bank selector
c020-c021 foreground scroll x hi-lo (Gun Dealer)
c022-c023 foreground scroll y hi-lo (Gun Dealer)

I/O:
read:
01        YM2203 read

write:
00        YM2203 control
01        YM2203 write

Interrupts:
Runs in interrupt mode 0, the interrupt vectors are 0xcf (RST 08h) and
0xd7 (RST 10h)

PCB:  DY-90010001
  CPU: Z80B
Sound: YM2203C + Y3014B DAC
  MCU: Unknown 64 pin DIL
  OSC: 12MHz, 5MHz

Clock measurements:
Z80 CPU - 12MHz/2
 YM2203 - 12MHz/8

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "includes/gundealr.h"

WRITE8_MEMBER(gundealr_state::yamyam_bankswitch_w)
{
	membank("bank1")->set_entry(data & 0x07);
}



static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, gundealr_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("DSW0")
	AM_RANGE(0xc001, 0xc001) AM_READ_PORT("DSW1")
	AM_RANGE(0xc004, 0xc004) AM_READ_PORT("IN0")
	AM_RANGE(0xc005, 0xc005) AM_READ_PORT("IN1")
	AM_RANGE(0xc006, 0xc006) AM_READ_PORT("IN2")
	AM_RANGE(0xc010, 0xc013) AM_WRITE(yamyam_fg_scroll_w)       /* Yam Yam only */
	AM_RANGE(0xc014, 0xc014) AM_WRITE(gundealr_flipscreen_w)
	AM_RANGE(0xc016, 0xc016) AM_WRITE(yamyam_bankswitch_w)
	AM_RANGE(0xc020, 0xc023) AM_WRITE(gundealr_fg_scroll_w) /* Gun Dealer only */
	AM_RANGE(0xc400, 0xc7ff) AM_RAM_WRITE(gundealr_paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(gundealr_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xd000, 0xdfff) AM_RAM_WRITE(gundealr_fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE("rambase")
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_portmap, AS_IO, 8, gundealr_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
ADDRESS_MAP_END



static INPUT_PORTS_START( gundealr )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:8") /* Listed in the manual as always OFF */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:7") /* Listed in the manual as always OFF */
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:4") /* Listed in the manual as always OFF */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:3") /* Listed in the manual as always OFF */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:8,7,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:5,4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:2,1") /* Both switch 1 & 2 are listed in the manual as always OFF */
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xc0, "3" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
INPUT_PORTS_END

static INPUT_PORTS_START( gundealt )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xc0, "3" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( yamyam )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, "Difficulty?" )       PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "Easy?" )
	PORT_DIPSETTING(    0x04, "Medium?" )
	PORT_DIPSETTING(    0x08, "Hard?" )
	PORT_DIPSETTING(    0x0c, "Hardest?" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
/*  PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) ) */
/*  PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) ) */
/*  PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) ) */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x38, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
/*  PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) ) */
/*  PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) ) */
/*  PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) ) */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(   0x80, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )   /* "TEST" */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
			16*32+0*4, 16*32+1*4, 16*32+2*4, 16*32+3*4, 16*32+4*4, 16*32+5*4, 16*32+6*4, 16*32+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	128*8
};

static GFXDECODE_START( gundealr )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 16 ) /* colors 0-255 */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 256, 16 ) /* colors 256-511 */
GFXDECODE_END




void gundealr_state::machine_start()
{
	UINT8 *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 8, &ROM[0x10000], 0x4000);

	save_item(NAME(m_flipscreen));
	save_item(NAME(m_scroll));
}

void gundealr_state::machine_reset()
{
	m_flipscreen = 0;
	m_scroll[0] = 0;
	m_scroll[1] = 0;
	m_scroll[2] = 0;
	m_scroll[3] = 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(gundealr_state::gundealr_scanline)
{
	int scanline = param;

	if(scanline == 240) // vblank-out irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE,0xd7); /* RST 10h */
	else if((scanline == 0) || (scanline == 120) ) //timer irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE,0xcf); /* RST 10h */
}

static MACHINE_CONFIG_START( gundealr, gundealr_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_12MHz/2)   /* 6 MHz verified for Yam! Yam!? */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_portmap)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", gundealr_state, gundealr_scanline, "screen", 0, 1)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(gundealr_state, screen_update_gundealr)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", gundealr)
	MCFG_PALETTE_ADD("palette", 512)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, XTAL_12MHz/8) /* 1.5Mhz verified for Yam! Yam!? */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

TIMER_DEVICE_CALLBACK_MEMBER(gundealr_state::yamyam_mcu_sim)
{
	static const UINT8 snipped_cmd03[8] = { 0x3a, 0x00, 0xc0, 0x47, 0x3a, 0x01, 0xc0, 0xc9 };
	static const UINT8 snipped_cmd05_1[5] = { 0xcd, 0x20, 0xe0, 0x7e, 0xc9 };
	static const UINT8 snipped_cmd05_2[8] = { 0xc5, 0x01, 0x00, 0x00, 0x4f, 0x09, 0xc1, 0xc9 };

	int i;

	//logerror("e000 = %02x\n", m_rambase[0x000]);
	switch(m_rambase[0x000])
	{
		case 0x03:
			m_rambase[0x001] = 0x03;
			/*
			    read dip switches
			    3a 00 c0  ld   a,($c000)
			    47        ld   b,a
			    3a 01 c0  ld   a,($c001)
			    c9        ret
			*/
			for(i=0;i<8;i++)
				m_rambase[0x010+i] = snipped_cmd03[i];

			break;
		case 0x04:
			m_rambase[0x001] = 0x04;
			break;
		case 0x05:
			m_rambase[0x001] = 0x05;
			/*
			    add a to hl
			    c5          push    bc
			    01 00 00    ld      bc,#0000
			    4f          ld      c,a
			    09          add     hl,bc
			    c1          pop     bc
			    c9          ret
			*/
			for(i=0;i<8;i++)
				m_rambase[0x020+i] = snipped_cmd05_2[i];

			/*
			    lookup data in table
			    cd 20 e0    call    #e020
			    7e          ld      a,(hl)
			    c9          ret
			*/
			for(i=0;i<5;i++)
				m_rambase[0x010+i] = snipped_cmd05_1[i];

			break;
		case 0x0a:
			m_rambase[0x001] = 0x08;
			break;
		case 0x0d:
			m_rambase[0x001] = 0x07;
			break;
	}

	m_rambase[0x004] = ioport("IN2")->read();
	m_rambase[0x005] = ioport("IN1")->read();
	m_rambase[0x006] = ioport("IN0")->read();
}

static MACHINE_CONFIG_DERIVED( yamyam, gundealr )

	MCFG_TIMER_DRIVER_ADD_PERIODIC("mcusim", gundealr_state, yamyam_mcu_sim, attotime::from_hz(6000000/60)) /* 6mhz confirmed */
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( gundealr )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for banks */
	ROM_LOAD( "1.3j",   0x00000, 0x10000, CRC(5797e830) SHA1(54bd9fbcafdf3fff55d73ecfe26d8e8df0dd55d9) ) /* 27c512; NOTE: the socket is labeled 1, but the rom has a '2' sticker on it! */
	ROM_RELOAD(               0x10000, 0x10000 )    /* banked at 0x8000-0xbfff */

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "3.6p",         0x00000, 0x10000, CRC(01f99de2) SHA1(2d9e9c50b0669811beb6fa53c0ff1b240fa939c7) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "2.6b",   0x00000, 0x20000, CRC(7874ec41) SHA1(2d2ff013cc37ce5966aa4b6c6724234655196102) ) /* NOTE: the socket is labeled 2, but the rom has a '1' sticker on it! */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s135.7l", 0x0000, 0x0100, NO_DUMP)
	ROM_LOAD( "82s129.7i", 0x0100, 0x0100, NO_DUMP)
ROM_END

ROM_START( gundealra )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for banks */
	ROM_LOAD( "gundeala.1.3j",   0x00000, 0x10000, CRC(d87e24f1) SHA1(5ac3e20e5848b9cab2a23e083d2566bfd54502d4) )
	ROM_RELOAD(               0x10000, 0x10000 )    /* banked at 0x8000-0xbfff */

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "gundeala.3.6p",   0x00000, 0x10000, CRC(836cf1a3) SHA1(ca57e7fc3e4497d249af963d1c8610e80ca65aa7) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "gundeala.2.6b",   0x00000, 0x20000, CRC(4b5fb53c) SHA1(3b73d9aeed334aece75f551f5b7f3cec0aedbfaa) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s135.7l", 0x0000, 0x0100, NO_DUMP)
	ROM_LOAD( "82s129.7i", 0x0100, 0x0100, NO_DUMP)
ROM_END

ROM_START( gundealrt )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for banks */
	ROM_LOAD( "1.3j",         0x00000, 0x10000, CRC(1d951292) SHA1(a8bd34dfaf31c7dc4f9e0ec1fd7d4e10c5b29a85) )
	ROM_RELOAD(               0x10000, 0x10000 )    /* banked at 0x8000-0xbfff */

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "3.6p",         0x00000, 0x10000, CRC(01f99de2) SHA1(2d9e9c50b0669811beb6fa53c0ff1b240fa939c7) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "2.6b",         0x00000, 0x20000, CRC(508ed0d0) SHA1(ea6b2d07e2e3d4f6c2a622a73b150ee7709b28de) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s135.7l", 0x0000, 0x0100, NO_DUMP)
	ROM_LOAD( "82s129.7i", 0x0100, 0x0100, NO_DUMP)
ROM_END

ROM_START( gundealrbl ) // bootleg with gfx customs done out in TTL logic, different proms, patched code rom
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for banks */
	ROM_LOAD( "29.2.am27c512.f10",   0x00000, 0x10000, CRC(7981751e) SHA1(3138581bcff84a11670ba54cbca608d590055b4e) ) // almost == gundealr "1.3j", 5 bytes different: (what does this change?)
	ROM_RELOAD(               0x10000, 0x10000 )    /* banked at 0x8000-0xbfff */
	// address gundealr gundealrbl
	// 009a    07       00
	// 6d4a    21       10
	// 6d52    20       11
	// 6d58    23       12
	// 6d60    22       13

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "30.3.am27c512.d16",  0x00000, 0x10000, CRC(01f99de2) SHA1(2d9e9c50b0669811beb6fa53c0ff1b240fa939c7) ) // == gundealr "3.16d"

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "22.1.d27c010.a16",   0x00000, 0x20000, CRC(7874ec41) SHA1(2d2ff013cc37ce5966aa4b6c6724234655196102) ) // == gundealr "2.6b"

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "4.82s129.7e", 0x0000, 0x0100, CRC(5c78339e) SHA1(4567c20122ae9694358b462e899f8bd2b453499f) ) // this might match the undumped 82s129.7i on the original dooyong board

	ROM_REGION( 0x0400, "pals", 0 )
	ROM_LOAD( "ep320pc.jed", 0x0000, 0x0400, NO_DUMP) // altera ep320pc on a daughterboard, undumped
ROM_END

ROM_START( yamyam ) /* DY-90010001 PCB */
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for banks */
	ROM_LOAD( "3.10f",       0x00000, 0x20000, CRC(96ae9088) SHA1(a605882dcdcf1e8cf8b0112f614e696d59acfd97) )
	ROM_RELOAD(               0x10000, 0x20000 )    /* banked at 0x8000-0xbfff */

	ROM_REGION( 0x10000, "mcu", 0 ) // unknown 64 pin MCU at J9 with internal ROM code
	ROM_LOAD( "mcu", 0x0000, 0x10000, NO_DUMP)

	ROM_REGION( 0x10000, "gfx1", 0 ) /* only gfx are different, code is the same */
	ROM_LOAD( "b2.16d",       0x00000, 0x10000, CRC(cb4f84ee) SHA1(54319ecbd74b763757eb6d17c8f7be0705ab0714) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "1.16a",       0x00000, 0x20000, CRC(b122828d) SHA1(90994ba548893a2eacdd58351cfa3952f4af926a) )
ROM_END

ROM_START( yamyamk ) /* DY-90010001 PCB */
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for banks */
	ROM_LOAD( "3.10f",       0x00000, 0x20000, CRC(96ae9088) SHA1(a605882dcdcf1e8cf8b0112f614e696d59acfd97) )
	ROM_RELOAD(               0x10000, 0x20000 )    /* banked at 0x8000-0xbfff */

	ROM_REGION( 0x10000, "mcu", 0 ) // unknown 64 pin MCU at J9 with internal ROM code
	ROM_LOAD( "mcu", 0x0000, 0x10000, NO_DUMP)

	ROM_REGION( 0x10000, "gfx1", 0 ) /* only gfx are different, code is the same */
	ROM_LOAD( "2.16d",       0x00000, 0x10000, CRC(dc9691d8) SHA1(118a05a1c94020d6739ed8c805c61b8ab003b6af) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "1.16a",       0x00000, 0x20000, CRC(b122828d) SHA1(90994ba548893a2eacdd58351cfa3952f4af926a) )
ROM_END

ROM_START( wiseguy ) /* DY-90010001 PCB */
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for banks */
	ROM_LOAD( "b3.f10",       0x00000, 0x20000, CRC(96ae9088) SHA1(a605882dcdcf1e8cf8b0112f614e696d59acfd97) )
	ROM_RELOAD(               0x10000, 0x20000 )    /* banked at 0x8000-0xbfff */

	ROM_REGION( 0x10000, "mcu", 0 ) // unknown 64 pin MCU at J9 with internal ROM code
	ROM_LOAD( "mcu", 0x0000, 0x10000, NO_DUMP)

	ROM_REGION( 0x10000, "gfx1", 0 ) /* only gfx are different, code is the same */
	ROM_LOAD( "wguyb2.16d",   0x00000, 0x10000, CRC(1c684c46) SHA1(041bc500e31b02a8bf3ce4683a67de998f938ccc) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "1.16a",       0x00000, 0x20000, CRC(b122828d) SHA1(90994ba548893a2eacdd58351cfa3952f4af926a) )
ROM_END



GAME( 1990, gundealr,  0,        gundealr, gundealr, driver_device, 0, ROT270, "Dooyong", "Gun Dealer",                MACHINE_SUPPORTS_SAVE )
GAME( 1990, gundealra, gundealr, gundealr, gundealr, driver_device, 0, ROT270, "Dooyong", "Gun Dealer (alt card set)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, gundealrt, gundealr, gundealr, gundealt, driver_device, 0, ROT270, "Dooyong (Tecmo license)", "Gun Dealer (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, gundealrbl, gundealr, gundealr, gundealr, driver_device, 0, ROT270, "bootleg", "Gun Dealer (bootleg)",                MACHINE_SUPPORTS_SAVE )

GAME( 1990, yamyam,    0,        yamyam,   yamyam, driver_device,   0, ROT0,   "Dooyong", "Yam! Yam!?",                MACHINE_SUPPORTS_SAVE )
GAME( 1990, yamyamk,   yamyam,   yamyam,   yamyam, driver_device,   0, ROT0,   "Dooyong", "Yam! Yam!? (Korea)",        MACHINE_SUPPORTS_SAVE )
GAME( 1990, wiseguy,   yamyam,   yamyam,   yamyam, driver_device,   0, ROT0,   "Dooyong", "Wise Guy",                  MACHINE_SUPPORTS_SAVE )
