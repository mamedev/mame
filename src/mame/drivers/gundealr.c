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

***************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "deprecat.h"
#include "sound/2203intf.h"


extern UINT8 *gundealr_bg_videoram,*gundealr_fg_videoram;

static UINT8 *rambase;

WRITE8_HANDLER( gundealr_paletteram_w );
WRITE8_HANDLER( gundealr_bg_videoram_w );
WRITE8_HANDLER( gundealr_fg_videoram_w );
WRITE8_HANDLER( gundealr_fg_scroll_w );
WRITE8_HANDLER( yamyam_fg_scroll_w );
WRITE8_HANDLER( gundealr_flipscreen_w );
VIDEO_UPDATE( gundealr );
VIDEO_START( gundealr );


static int input_ports_hack;

static INTERRUPT_GEN( yamyam_interrupt )
{
	if (cpu_getiloops(device) == 0)
	{
		if (input_ports_hack)
		{
			rambase[0x004] = input_port_read(device->machine, "IN2");
			rambase[0x005] = input_port_read(device->machine, "IN1");
			rambase[0x006] = input_port_read(device->machine, "IN0");
		}
		cpu_set_input_line_and_vector(device, 0, HOLD_LINE, 0xd7);	/* RST 10h vblank */
	}
	else if ((cpu_getiloops(device) & 1) == 1)
		cpu_set_input_line_and_vector(device, 0, HOLD_LINE, 0xcf);	/* RST 08h sound (hand tuned) */
}

static WRITE8_HANDLER( yamyam_bankswitch_w )
{
 	int bankaddress;
	UINT8 *RAM = memory_region(space->machine, "maincpu");

	bankaddress = 0x10000 + (data & 0x07) * 0x4000;
	memory_set_bankptr(space->machine, 1,&RAM[bankaddress]);
}

static WRITE8_HANDLER( yamyam_protection_w )
{
logerror("e000 = %02x\n",rambase[0x000]);
	rambase[0x000] = data;
	if (data == 0x03) rambase[0x001] = 0x03;
	if (data == 0x04) rambase[0x001] = 0x04;
	if (data == 0x05) rambase[0x001] = 0x05;
	if (data == 0x0a) rambase[0x001] = 0x08;
	if (data == 0x0d) rambase[0x001] = 0x07;

	if (data == 0x03)
	{
		/*
        read dip switches
        3a 00 c0  ld   a,($c000)
        47        ld   b,a
        3a 01 c0  ld   a,($c001)
        c9        ret
        */
		rambase[0x010] = 0x3a;
		rambase[0x011] = 0x00;
		rambase[0x012] = 0xc0;
		rambase[0x013] = 0x47;
		rambase[0x014] = 0x3a;
		rambase[0x015] = 0x01;
		rambase[0x016] = 0xc0;
		rambase[0x017] = 0xc9;
	}
	if (data == 0x05)
	{
		/*
        add a to hl
        c5        push    bc
        010000    ld      bc,#0000
        4f        ld      c,a
        09        add     hl,bc
        c1        pop     bc
        c9        ret
        */
		rambase[0x020] = 0xc5;
		rambase[0x021] = 0x01;
		rambase[0x022] = 0x00;
		rambase[0x023] = 0x00;
		rambase[0x024] = 0x4f;
		rambase[0x025] = 0x09;
		rambase[0x026] = 0xc1;
		rambase[0x027] = 0xc9;
		/*
        lookup data in table
        cd20e0    call    #e020
        7e        ld      a,(hl)
        c9        ret
        */
		rambase[0x010] = 0xcd;
		rambase[0x011] = 0x20;
		rambase[0x012] = 0xe0;
		rambase[0x013] = 0x7e;
		rambase[0x014] = 0xc9;
	}
}



static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK(1)
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("DSW0")	/* DSW0 */
	AM_RANGE(0xc001, 0xc001) AM_READ_PORT("DSW1")	/* DSW1 */
	AM_RANGE(0xc004, 0xc004) AM_READ_PORT("IN0")	/* COIN (Gun Dealer only) */
	AM_RANGE(0xc005, 0xc005) AM_READ_PORT("IN1")	/* IN1 (Gun Dealer only) */
	AM_RANGE(0xc006, 0xc006) AM_READ_PORT("IN2")	/* IN0 (Gun Dealer only) */
	AM_RANGE(0xc010, 0xc013) AM_WRITE(yamyam_fg_scroll_w)		/* Yam Yam only */
	AM_RANGE(0xc014, 0xc014) AM_WRITE(gundealr_flipscreen_w)
	AM_RANGE(0xc016, 0xc016) AM_WRITE(yamyam_bankswitch_w)
	AM_RANGE(0xc020, 0xc023) AM_WRITE(gundealr_fg_scroll_w)	/* Gun Dealer only */
	AM_RANGE(0xc400, 0xc7ff) AM_RAM_WRITE(gundealr_paletteram_w) AM_BASE(&paletteram)
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(gundealr_bg_videoram_w) AM_BASE(&gundealr_bg_videoram)
	AM_RANGE(0xd000, 0xdfff) AM_RAM_WRITE(gundealr_fg_videoram_w) AM_BASE(&gundealr_fg_videoram)
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_BASE(&rambase)
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ym", ym2203_r, ym2203_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( gundealr )
	PORT_START("DSW0")	/* DSW0 */
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

	PORT_START("DSW1")	/* DSW1 */
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

	PORT_START("IN0")	/* COIN */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")	/* Player 2 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */

	PORT_START("IN2")	/* Player 1 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
INPUT_PORTS_END

static INPUT_PORTS_START( gundealt )
	PORT_START("DSW0")	/* DSW0 */
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

	PORT_START("DSW1")	/* DSW1 */
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

	PORT_START("IN0")	/* Player 1 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */

	PORT_START("IN1")	/* Player 2 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */

	PORT_START("IN2")	/* COIN */
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
	PORT_START("DSW0")	/* DSW0 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, "Difficulty?" )
	PORT_DIPSETTING(    0x00, "Easy?" )
	PORT_DIPSETTING(    0x04, "Medium?" )
	PORT_DIPSETTING(    0x08, "Hard?" )
	PORT_DIPSETTING(    0x0c, "Hardest?" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")	/* DSW1 */
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
/*  PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) ) */
/*  PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) ) */
/*  PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) ) */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
/*  PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) ) */
/*  PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) ) */
/*  PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) ) */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("IN0")	/* COIN */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )	/* "TEST" */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")	/* Player 2 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */

	PORT_START("IN2")	/* Player 1 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
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
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 16 )	/* colors 0-255 */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 256, 16 )	/* colors 256-511 */
GFXDECODE_END




static MACHINE_DRIVER_START( gundealr )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, 8000000)	/* 8 MHz ??? */
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_IO_MAP(main_portmap)
	MDRV_CPU_VBLANK_INT_HACK(yamyam_interrupt,4)	/* ? */

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(gundealr)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(gundealr)
	MDRV_VIDEO_UPDATE(gundealr)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym", YM2203, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( gundealr )
	ROM_REGION( 0x30000, "maincpu", 0 )	/* 64k for code + 128k for banks */
	ROM_LOAD( "gundealr.1",   0x00000, 0x10000, CRC(5797e830) SHA1(54bd9fbcafdf3fff55d73ecfe26d8e8df0dd55d9) )
	ROM_RELOAD(               0x10000, 0x10000 )	/* banked at 0x8000-0xbfff */

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "3.6p",         0x00000, 0x10000, CRC(01f99de2) SHA1(2d9e9c50b0669811beb6fa53c0ff1b240fa939c7) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "gundealr.2",   0x00000, 0x20000, CRC(7874ec41) SHA1(2d2ff013cc37ce5966aa4b6c6724234655196102) )
ROM_END

ROM_START( gundeala )
	ROM_REGION( 0x30000, "maincpu", 0 )	/* 64k for code + 128k for banks */
	ROM_LOAD( "gundeala.1",   0x00000, 0x10000, CRC(d87e24f1) SHA1(5ac3e20e5848b9cab2a23e083d2566bfd54502d4) )
	ROM_RELOAD(               0x10000, 0x10000 )	/* banked at 0x8000-0xbfff */

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "gundeala.3",   0x00000, 0x10000, CRC(836cf1a3) SHA1(ca57e7fc3e4497d249af963d1c8610e80ca65aa7) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "gundeala.2",   0x00000, 0x20000, CRC(4b5fb53c) SHA1(3b73d9aeed334aece75f551f5b7f3cec0aedbfaa) )
ROM_END

ROM_START( gundealt )
	ROM_REGION( 0x30000, "maincpu", 0 )	/* 64k for code + 128k for banks */
	ROM_LOAD( "1.3j",         0x00000, 0x10000, CRC(1d951292) SHA1(a8bd34dfaf31c7dc4f9e0ec1fd7d4e10c5b29a85) )
	ROM_RELOAD(               0x10000, 0x10000 )	/* banked at 0x8000-0xbfff */

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "3.6p",         0x00000, 0x10000, CRC(01f99de2) SHA1(2d9e9c50b0669811beb6fa53c0ff1b240fa939c7) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "2.6b",         0x00000, 0x20000, CRC(508ed0d0) SHA1(ea6b2d07e2e3d4f6c2a622a73b150ee7709b28de) )
ROM_END

ROM_START( yamyam )
	ROM_REGION( 0x30000, "maincpu", 0 )	/* 64k for code + 128k for banks */
	ROM_LOAD( "b3.f10",       0x00000, 0x20000, CRC(96ae9088) SHA1(a605882dcdcf1e8cf8b0112f614e696d59acfd97) )
	ROM_RELOAD(               0x10000, 0x20000 )	/* banked at 0x8000-0xbfff */

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "b2.d16",       0x00000, 0x10000, CRC(cb4f84ee) SHA1(54319ecbd74b763757eb6d17c8f7be0705ab0714) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "b1.a16",       0x00000, 0x20000, CRC(b122828d) SHA1(90994ba548893a2eacdd58351cfa3952f4af926a) )
ROM_END

/* only gfx are different, code is the same */
ROM_START( wiseguy )
	ROM_REGION( 0x30000, "maincpu", 0 )	/* 64k for code + 128k for banks */
	ROM_LOAD( "b3.f10",       0x00000, 0x20000, CRC(96ae9088) SHA1(a605882dcdcf1e8cf8b0112f614e696d59acfd97) )
	ROM_RELOAD(               0x10000, 0x20000 )	/* banked at 0x8000-0xbfff */

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "wguyb2.bin",   0x00000, 0x10000, CRC(1c684c46) SHA1(041bc500e31b02a8bf3ce4683a67de998f938ccc) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "b1.a16",       0x00000, 0x20000, CRC(b122828d) SHA1(90994ba548893a2eacdd58351cfa3952f4af926a) )
ROM_END



static DRIVER_INIT( gundealr )
{
	input_ports_hack = 0;
}

static DRIVER_INIT( yamyam )
{
	input_ports_hack = 1;
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xe000, 0xe000, 0, 0, yamyam_protection_w);
}



GAME( 1990, gundealr, 0,        gundealr, gundealr, gundealr, ROT270, "Dooyong", "Gun Dealer (set 1)", 0 )
GAME( 19??, gundeala, gundealr, gundealr, gundealr, gundealr, ROT270, "Dooyong", "Gun Dealer (set 2)", 0 )
GAME( 1990, gundealt, gundealr, gundealr, gundealt, gundealr, ROT270, "Tecmo", "Gun Dealer (Tecmo)", 0 )
GAME( 1990, yamyam,   0,        gundealr, yamyam,   yamyam,   ROT0,   "Dooyong", "Yam! Yam!?", 0 )
GAME( 1990, wiseguy,  yamyam,   gundealr, yamyam,   yamyam,   ROT0,   "Dooyong", "Wise Guy", 0 )
