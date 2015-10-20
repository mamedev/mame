// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

Marine Boy hardware memory map (preliminary)

driver by Zsolt Vasvari

MAIN CPU:

0000-7fff ROM (not all games use the entire region)
8000-87ff RAM
8c18-8c3f sprite RAM (Hoccer only)
8800-8bff video RAM  \ (contains sprite RAM in invisible regions)
9000-93ff color RAM  /

read:
a000      IN0
a800      IN1
b000      DSW
b800      IN2/watchdog reset

write:
9800      column scroll
9a00      char palette bank bit 0 (not used by Hoccer)
9c00      char palette bank bit 1 (not used by Hoccer)
a000      NMI interrupt acknowledge/enable
a001      flipy
a002      flipx
b800      ???


I/0 ports:
write
08        8910  control
09        8910  write

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/marineb.h"


void marineb_state::machine_reset()
{
	m_palette_bank = 0;
	m_column_scroll = 0;
	m_flipscreen_x = 0;
	m_flipscreen_y = 0;
	m_marineb_active_low_flipscreen = 0;
}

MACHINE_RESET_MEMBER(marineb_state,springer)
{
	marineb_state::machine_reset();

	m_marineb_active_low_flipscreen = 1;
}

void marineb_state::machine_start()
{
	save_item(NAME(m_marineb_active_low_flipscreen));
}

WRITE8_MEMBER(marineb_state::irq_mask_w)
{
	m_irq_mask = data & 1;
}

static ADDRESS_MAP_START( marineb_map, AS_PROGRAM, 8, marineb_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8bff) AM_RAM_WRITE(marineb_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x8c00, 0x8c3f) AM_RAM AM_SHARE("spriteram")  /* Hoccer only */
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(marineb_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x9800, 0x9800) AM_WRITE(marineb_column_scroll_w)
	AM_RANGE(0x9a00, 0x9a00) AM_WRITE(marineb_palette_bank_0_w)
	AM_RANGE(0x9c00, 0x9c00) AM_WRITE(marineb_palette_bank_1_w)
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("P2") AM_WRITE(irq_mask_w)
	AM_RANGE(0xa001, 0xa001) AM_WRITE(marineb_flipscreen_y_w)
	AM_RANGE(0xa002, 0xa002) AM_WRITE(marineb_flipscreen_x_w)
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("P1")
	AM_RANGE(0xb000, 0xb000) AM_READ_PORT("DSW")
	AM_RANGE(0xb800, 0xb800) AM_READ_PORT("SYSTEM") AM_WRITENOP     /* also watchdog */
ADDRESS_MAP_END


static ADDRESS_MAP_START( marineb_io_map, AS_IO, 8, marineb_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x08, 0x09) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( wanted_io_map, AS_IO, 8, marineb_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x02, 0x03) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( marineb )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x3c, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x3c, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x04, "???" )
//  PORT_DIPSETTING(    0x08, "???" )
//  PORT_DIPSETTING(    0x0c, "???" )
//  PORT_DIPSETTING(    0x10, "???" )
//  PORT_DIPSETTING(    0x14, "???" )
//  PORT_DIPSETTING(    0x18, "???" )
	PORT_DIPSETTING(    0x1c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "20000 50000" )
	PORT_DIPSETTING(    0x20, "40000 70000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( changes )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x3c, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x3c, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x04, "???" )
//  PORT_DIPSETTING(    0x08, "???" )
	PORT_DIPSETTING(    0x0c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x00, "1st Bonus Life" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x10, "40000" )
	PORT_DIPNAME( 0x20, 0x00, "2nd Bonus Life" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPSETTING(    0x20, "100000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( hoccer )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x3c, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x3c, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Unknown ) )  /* difficulty maybe? */
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( wanted )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_COIN1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("SYSTEM")    /* we use same tags as above, to simplify reads */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0xf0, 0x10, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, " A 3C/1C  B 3C/1C" )
	PORT_DIPSETTING(    0xe0, " A 3C/1C  B 1C/2C" )
	PORT_DIPSETTING(    0xf0, " A 3C/1C  B 1C/4C" )
	PORT_DIPSETTING(    0x20, " A 2C/1C  B 2C/1C" )
	PORT_DIPSETTING(    0xd0, " A 2C/1C  B 1C/1C" )
	PORT_DIPSETTING(    0x70, " A 2C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0xb0, " A 2C/1C  B 1C/5C" )
	PORT_DIPSETTING(    0xc0, " A 2C/1C  B 1C/6C" )
	PORT_DIPSETTING(    0x60, " A 1C/1C  B 4C/1C" )
	PORT_DIPSETTING(    0x50, " A 1C/1C  B 2C/1C" )
	PORT_DIPSETTING(    0x10, " A 1C/1C  B 1C/1C" )
	PORT_DIPSETTING(    0x30, " A 1C/2C  B 1C/2C" )
	PORT_DIPSETTING(    0xa0, " A 1C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0x80, " A 1C/1C  B 1C/5C" )
	PORT_DIPSETTING(    0x90, " A 1C/1C  B 1C/6C" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bcruzm12 )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_COIN1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")    /* we use same tags as above, to simplify reads */
	PORT_DIPNAME( 0x03, 0x01, "2nd Bonus Life" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x01, "60000" )
	PORT_DIPSETTING(    0x02, "80000" )
	PORT_DIPSETTING(    0x03, "100000" )
	PORT_DIPNAME( 0x0c, 0x04, "1st Bonus Life" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x08, "40000" )
	PORT_DIPSETTING(    0x0c, "50000" )
	PORT_DIPNAME( 0xf0, 0x10, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, " A 3C/1C  B 3C/1C" )
	PORT_DIPSETTING(    0xe0, " A 3C/1C  B 1C/2C" )
	PORT_DIPSETTING(    0xf0, " A 3C/1C  B 1C/4C" )
	PORT_DIPSETTING(    0x20, " A 2C/1C  B 2C/1C" )
	PORT_DIPSETTING(    0xd0, " A 2C/1C  B 1C/1C" )
	PORT_DIPSETTING(    0x70, " A 2C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0xb0, " A 2C/1C  B 1C/5C" )
	PORT_DIPSETTING(    0xc0, " A 2C/1C  B 1C/6C" )
	PORT_DIPSETTING(    0x60, " A 1C/1C  B 4C/5C" )
	PORT_DIPSETTING(    0x50, " A 1C/1C  B 2C/3C" )
	PORT_DIPSETTING(    0x10, " A 1C/1C  B 1C/1C" )
	PORT_DIPSETTING(    0x30, " A 1C/2C  B 1C/2C" )
	PORT_DIPSETTING(    0xa0, " A 1C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0x80, " A 1C/1C  B 1C/5C" )
	PORT_DIPSETTING(    0x90, " A 1C/1C  B 1C/6C" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
INPUT_PORTS_END

static const gfx_layout marineb_charlayout =
{
	8,8,    /* 8*8 characters */
	512,    /* 512 characters */
	2,      /* 2 bits per pixel */
	{ 0, 4 },   /* the two bitplanes for 4 pixels are packed into one byte */
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 }, /* bits are packed in groups of four */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8    /* every char takes 16 bytes */
};

static const gfx_layout wanted_charlayout =
{
	8,8,    /* 8*8 characters */
	1024,   /* 1024 characters */
	2,      /* 2 bits per pixel */
	{ 4, 0 },   /* the two bitplanes for 4 pixels are packed into one byte */
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 }, /* bits are packed in groups of four */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8    /* every char takes 16 bytes */
};

static const gfx_layout hopprobo_charlayout =
{
	8,8,    /* 8*8 characters */
	1024,   /* 1024 characters */
	2,      /* 2 bits per pixel */
	{ 0, 4 },   /* the two bitplanes for 4 pixels are packed into one byte */
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 }, /* bits are packed in groups of four */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8    /* every char takes 16 bytes */
};

static const gfx_layout marineb_small_spritelayout =
{
	16,16,  /* 16*16 sprites */
	64,     /* 64 sprites */
	2,      /* 2 bits per pixel */
	{ 0, 256*32*8 },    /* the two bitplanes are separated */
	{     0,     1,     2,     3,     4,     5,     6,     7,
		8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static const gfx_layout marineb_big_spritelayout =
{
	32,32,  /* 32*32 sprites */
	64,     /* 64 sprites */
	2,      /* 2 bits per pixel */
	{ 0, 256*32*8 },    /* the two bitplanes are separated */
	{      0,      1,      2,      3,      4,      5,      6,      7,
		8*8+0,  8*8+1,  8*8+2,  8*8+3,  8*8+4,  8*8+5,  8*8+6,  8*8+7,
		32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
		40*8+0, 40*8+1, 40*8+2, 40*8+3, 40*8+4, 40*8+5, 40*8+6, 40*8+7 },
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
		64*8, 65*8, 66*8, 67*8, 68*8, 69*8, 70*8, 71*8,
		80*8, 81*8, 82*8, 83*8, 84*8, 85*8, 86*8, 87*8 },
	4*32*8  /* every sprite takes 128 consecutive bytes */
};

static const gfx_layout changes_small_spritelayout =
{
	16,16,  /* 16*16 sprites */
	64,     /* 64 sprites */
	2,      /* 2 bits per pixel */
	{ 0, 4 },   /* the two bitplanes for 4 pixels are packed into one byte */
	{      0,      1,      2,      3,  8*8+0,  8*8+1,  8*8+2,  8*8+3,
		16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
		32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8    /* every sprite takes 64 consecutive bytes */
};

static const gfx_layout changes_big_spritelayout =
{
	32,32,  /* 32*3 sprites */
	16,     /* 32 sprites */
	2,      /* 2 bits per pixel */
	{ 0, 4 },   /* the two bitplanes for 4 pixels are packed into one byte */
	{      0,      1,      2,      3,  8*8+0,  8*8+1,  8*8+2,  8*8+3,
		16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3,
		64*8+0, 64*8+1, 64*8+2, 64*8+3, 72*8+0, 72*8+1, 72*8+2, 72*8+3,
		80*8+0, 80*8+1, 80*8+2, 80*8+3, 88*8+0, 88*8+1, 88*8+2, 88*8+3 },
	{   0*8,   1*8,   2*8,   3*8,   4*8,   5*8,   6*8,   7*8,
		32*8,  33*8,  34*8,  35*8,  36*8,  37*8,  38*8,  39*8,
		128*8, 129*8, 130*8, 131*8, 132*8, 133*8, 134*8, 135*8,
		160*8, 161*8, 162*8, 163*8, 164*8, 165*8, 166*8, 167*8 },
	4*64*8  /* every sprite takes 256 consecutive bytes */
};


static GFXDECODE_START( marineb )
	GFXDECODE_ENTRY( "gfx1", 0x0000, marineb_charlayout,          0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, marineb_small_spritelayout,  0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, marineb_big_spritelayout,    0, 64 )
GFXDECODE_END

static GFXDECODE_START( wanted )
	GFXDECODE_ENTRY( "gfx1", 0x0000, wanted_charlayout,           0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, marineb_small_spritelayout,  0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, marineb_big_spritelayout,    0, 64 )
GFXDECODE_END

static GFXDECODE_START( changes )
	GFXDECODE_ENTRY( "gfx1", 0x0000, marineb_charlayout,          0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, changes_small_spritelayout,  0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0x1000, changes_big_spritelayout,    0, 64 )
GFXDECODE_END

static GFXDECODE_START( hoccer )
	GFXDECODE_ENTRY( "gfx1", 0x0000, marineb_charlayout,          0, 16 )   /* no palette banks */
	GFXDECODE_ENTRY( "gfx2", 0x0000, changes_small_spritelayout,  0, 16 )   /* no palette banks */
GFXDECODE_END

static GFXDECODE_START( hopprobo )
	GFXDECODE_ENTRY( "gfx1", 0x0000, hopprobo_charlayout,         0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, marineb_small_spritelayout,  0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, marineb_big_spritelayout,    0, 64 )
GFXDECODE_END

INTERRUPT_GEN_MEMBER(marineb_state::marineb_vblank_irq)
{
	if(m_irq_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INTERRUPT_GEN_MEMBER(marineb_state::wanted_vblank_irq)
{
	if(m_irq_mask)
		device.execute().set_input_line(0, HOLD_LINE);
}


static MACHINE_CONFIG_START( marineb, marineb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 3072000)   /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(marineb_map)
	MCFG_CPU_IO_MAP(marineb_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", marineb_state,  marineb_vblank_irq)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(5000)   /* frames per second, vblank duration */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(marineb_state, screen_update_marineb)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", marineb)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(marineb_state, marineb)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay1", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( changes, marineb )

	/* basic machine hardware */

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", changes)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(marineb_state, screen_update_changes)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( springer, marineb )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(marineb_state,springer)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(marineb_state, screen_update_springer)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( hoccer, marineb )

	/* basic machine hardware */

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", hoccer)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(marineb_state, screen_update_hoccer)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( wanted, marineb )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(wanted_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", marineb_state,  wanted_vblank_irq)

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", wanted)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(marineb_state, screen_update_springer)

	/* sound hardware */
	MCFG_SOUND_REPLACE("ay1", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( hopprobo, marineb )

	/* basic machine hardware */

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", hopprobo)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(marineb_state, screen_update_hopprobo)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( bcruzm12, wanted )

	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(marineb_state,springer)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( marineb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "marineb.1",     0x0000, 0x1000, CRC(661d6540) SHA1(9ef6b58952be7ca245092916782311ee466fe3e1) )
	ROM_LOAD( "marineb.2",     0x1000, 0x1000, CRC(922da17f) SHA1(e4925ae5afe937a0306af0a4fbbc6edcd13f1926) )
	ROM_LOAD( "marineb.3",     0x2000, 0x1000, CRC(820a235b) SHA1(00bd8d9af1c88201100811518d3d795a80604711) )
	ROM_LOAD( "marineb.4",     0x3000, 0x1000, CRC(a157a283) SHA1(741e44c75636e5349ad43308076e1a8533255711) )
	ROM_LOAD( "marineb.5",     0x4000, 0x1000, CRC(9ffff9c0) SHA1(55fa22f28e56c69cee50d4054206cb7f63e24590) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "marineb.6",     0x0000, 0x2000, CRC(ee53ec2e) SHA1(a8aab0ad70a20884e30420a6956b503cf7fdfbb8) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "marineb.8",     0x0000, 0x2000, CRC(dc8bc46c) SHA1(5ac945d7632dbc24a47d4dd8816b931b5615e2cd) )
	ROM_LOAD( "marineb.7",     0x2000, 0x2000, CRC(9d2e19ab) SHA1(a64460c9222447cc63094350f910a9f73c423edd) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "marineb.1b",    0x0000, 0x0100, CRC(f32d9472) SHA1(b965eabb313cdfa0d10f8f25f659e20b0abe9a97) ) /* palette low 4 bits */
	ROM_LOAD( "marineb.1c",    0x0100, 0x0100, CRC(93c69d3e) SHA1(d13720fa4947e5058d4d699990b9a731e25e5595) ) /* palette high 4 bits */
ROM_END

ROM_START( changes )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "changes.1",     0x0000, 0x1000, CRC(56f83813) SHA1(8b97e877b1819402703a9e9e641efc0e89b84cca) )
	ROM_LOAD( "changes.2",     0x1000, 0x1000, CRC(0e627f0b) SHA1(59012c8f65b921387b381dbc5157a7a22b3d50dc) )
	ROM_LOAD( "changes.3",     0x2000, 0x1000, CRC(ff8291e9) SHA1(c07394cb384259287a6820ce0e513e32c69d768b) )
	ROM_LOAD( "changes.4",     0x3000, 0x1000, CRC(a8e9aa22) SHA1(fbccf017851eb099960ad51ef3060a16bc0107a5) )
	ROM_LOAD( "changes.5",     0x4000, 0x1000, CRC(f4198e9e) SHA1(4a9aea2b38d3bf093d9e97c884a7a9d16c203a46) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "changes.7",     0x0000, 0x2000, CRC(2204194e) SHA1(97ee40dd804158e92a2a1034f8e910f1057a7b54) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "changes.6",     0x0000, 0x2000, CRC(985c9db4) SHA1(d95a8794b96aec9133fd49b7d5724c161c5478bf) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "changes.1b",    0x0000, 0x0100, CRC(f693c153) SHA1(463426b580fa02f00baf2fff9f42d34b52bd6be4) ) /* palette low 4 bits */
	ROM_LOAD( "changes.1c",    0x0100, 0x0100, CRC(f8331705) SHA1(cbead7ed85f96219af14b6552301906f32260b69) ) /* palette high 4 bits */
ROM_END

ROM_START( changesa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "changes3.1",    0x0000, 0x1000, CRC(ff80cad7) SHA1(00a97137c0b92e8b9532c824bade89002ec5d63c) )
	ROM_LOAD( "changes.2",     0x1000, 0x1000, CRC(0e627f0b) SHA1(59012c8f65b921387b381dbc5157a7a22b3d50dc) )
	ROM_LOAD( "changes3.3",    0x2000, 0x1000, CRC(359bf7e1) SHA1(9c3cc4415ccaa0276f98224ca373922c2425bb40) )
	ROM_LOAD( "changes.4",     0x3000, 0x1000, CRC(a8e9aa22) SHA1(fbccf017851eb099960ad51ef3060a16bc0107a5) )
	ROM_LOAD( "changes3.5",    0x4000, 0x1000, CRC(c197e64a) SHA1(86b9f5f51f208bc9cdda3d13176147dbcafdc913) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "changes.7",     0x0000, 0x2000, CRC(2204194e) SHA1(97ee40dd804158e92a2a1034f8e910f1057a7b54) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "changes.6",     0x0000, 0x2000, CRC(985c9db4) SHA1(d95a8794b96aec9133fd49b7d5724c161c5478bf) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "changes.1b",    0x0000, 0x0100, CRC(f693c153) SHA1(463426b580fa02f00baf2fff9f42d34b52bd6be4) ) /* palette low 4 bits */
	ROM_LOAD( "changes.1c",    0x0100, 0x0100, CRC(f8331705) SHA1(cbead7ed85f96219af14b6552301906f32260b69) ) /* palette high 4 bits */
ROM_END

ROM_START( looper )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "changes.1",     0x0000, 0x1000, CRC(56f83813) SHA1(8b97e877b1819402703a9e9e641efc0e89b84cca) )
	ROM_LOAD( "changes.2",     0x1000, 0x1000, CRC(0e627f0b) SHA1(59012c8f65b921387b381dbc5157a7a22b3d50dc) )
	ROM_LOAD( "changes.3",     0x2000, 0x1000, CRC(ff8291e9) SHA1(c07394cb384259287a6820ce0e513e32c69d768b) )
	ROM_LOAD( "changes.4",     0x3000, 0x1000, CRC(a8e9aa22) SHA1(fbccf017851eb099960ad51ef3060a16bc0107a5) )
	ROM_LOAD( "changes.5",     0x4000, 0x1000, CRC(f4198e9e) SHA1(4a9aea2b38d3bf093d9e97c884a7a9d16c203a46) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "looper_7.bin",  0x0000, 0x2000, CRC(71a89975) SHA1(798b50af7348b20487c1c37a3e1b9b26585d50f6) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "looper_6.bin",  0x0000, 0x2000, CRC(1f3f70c2) SHA1(c3c04892961e83e3be7773fca8304651b98046cf) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "changes.1b",    0x0000, 0x0100, CRC(f693c153) SHA1(463426b580fa02f00baf2fff9f42d34b52bd6be4) ) /* palette low 4 bits */
	ROM_LOAD( "changes.1c",    0x0100, 0x0100, CRC(f8331705) SHA1(cbead7ed85f96219af14b6552301906f32260b69) ) /* palette high 4 bits */
ROM_END

ROM_START( springer )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "springer.1",    0x0000, 0x1000, CRC(0794103a) SHA1(71bb7f6bf4f41f50a39a552cc98ea111e4064acd) )
	ROM_LOAD( "springer.2",    0x1000, 0x1000, CRC(f4aecd9a) SHA1(414d5385286d899f883bd70ee7fbb5f23e424ef9) )
	ROM_LOAD( "springer.3",    0x2000, 0x1000, CRC(2f452371) SHA1(973839714322f517b51c224adf0792a3fe0a091b) )
	ROM_LOAD( "springer.4",    0x3000, 0x1000, CRC(859d1bf5) SHA1(0f90e0c22d1a0fdf61b87738cd2c4113e1e9178f) )
	ROM_LOAD( "springer.5",    0x4000, 0x1000, CRC(72adbbe3) SHA1(5e9d65eb524f7e0e8ea84eeebc698d8bdae6606c) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "springer.6",    0x0000, 0x1000, CRC(6a961833) SHA1(38f4b0ec73e404b3ec750a2f0c1e502dd75e3ee3) )
	ROM_LOAD( "springer.7",    0x1000, 0x1000, CRC(95ab8fc0) SHA1(74dad6fe1edd38b22656cf6cd9e4a57012bf0d60) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "springer.9",    0x0000, 0x1000, CRC(fa302775) SHA1(412afdc620be95e70b3b782d1a08e4a46777e710) )
							/* 0x1000-0x1fff empty for my convinience */
	ROM_LOAD( "springer.8",    0x2000, 0x1000, CRC(a54bafdc) SHA1(70f1a9ab116dc2a195aa9026ed1004101897d274) )
							/* 0x3000-0x3fff empty for my convinience */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "1b.vid",        0x0000, 0x0100, CRC(a2f935aa) SHA1(575b0b0e67aeff664bdf00f1bfd9dc68fdf88074) ) /* palette low 4 bits */
	ROM_LOAD( "1c.vid",        0x0100, 0x0100, CRC(b95421f4) SHA1(48ddf33d1094eb4343b7c54a2a221050f83b749b) ) /* palette high 4 bits */
ROM_END

ROM_START( hoccer )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hr1.cpu",       0x0000, 0x2000, CRC(12e96635) SHA1(5d330687dc117a319be355a541c9f634c6711889) )
	ROM_LOAD( "hr2.cpu",       0x2000, 0x2000, CRC(cf1fc328) SHA1(c8669330d47fe3c8f855990ff9f27549bf94c5bd) )
	ROM_LOAD( "hr3.cpu",       0x4000, 0x2000, CRC(048a0659) SHA1(7dc0ba2046f8985d4e3bfbba5284090dd4382aa1) )
	ROM_LOAD( "hr4.cpu",       0x6000, 0x2000, CRC(9a788a2c) SHA1(a49efa5bbebcb9f7cbe22a85494f24af9965b4dc) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "hr.d",          0x0000, 0x2000, CRC(d33aa980) SHA1(ba6cc0eed87a1561a584058bdcaac2c0a375e235) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "hr.c",          0x0000, 0x2000, CRC(02808294) SHA1(c4f5c6a8e3fbb156917821b4e7b1b25d23418ca3) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "hr.1b",         0x0000, 0x0100, CRC(896521d7) SHA1(54b295b4bb7357ffdb8916ff9618570867a950b2) ) /* palette low 4 bits */
	ROM_LOAD( "hr.1c",         0x0100, 0x0100, CRC(2efdd70b) SHA1(1b4fc9e52aaa4600c535b04d40aac1e0dd85fd7b) ) /* palette high 4 bits */
ROM_END

ROM_START( hoccer2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hr.1",          0x0000, 0x2000, CRC(122d159f) SHA1(cc58fc746d6afebc39f8840c1ea6a68b5d033fbb) )
	ROM_LOAD( "hr.2",          0x2000, 0x2000, CRC(48e1efc0) SHA1(6472423b129e0fc40e4f12855d70207a360ee378) )
	ROM_LOAD( "hr.3",          0x4000, 0x2000, CRC(4e67b0be) SHA1(2c2e6a7798325621d1b67ee90fc2f198731b4ab1) )
	ROM_LOAD( "hr.4",          0x6000, 0x2000, CRC(d2b44f58) SHA1(6a3666b1b5f4da5a72f6712db8d242281ea634fa) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "hr.d",          0x0000, 0x2000, CRC(d33aa980) SHA1(ba6cc0eed87a1561a584058bdcaac2c0a375e235) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "hr.c",          0x0000, 0x2000, CRC(02808294) SHA1(c4f5c6a8e3fbb156917821b4e7b1b25d23418ca3) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "hr.1b",         0x0000, 0x0100, CRC(896521d7) SHA1(54b295b4bb7357ffdb8916ff9618570867a950b2) ) /* palette low 4 bits */
	ROM_LOAD( "hr.1c",         0x0100, 0x0100, CRC(2efdd70b) SHA1(1b4fc9e52aaa4600c535b04d40aac1e0dd85fd7b) ) /* palette high 4 bits */
ROM_END

ROM_START( wanted )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "prg-1",         0x0000, 0x2000, CRC(2dd90aed) SHA1(3982d47a66f653a046a95e38648a7c5a2bfe1470) )
	ROM_LOAD( "prg-2",         0x2000, 0x2000, CRC(67ac0210) SHA1(29fd01289c9ba5a3a992ac6740badbf2e37f05ac) )
	ROM_LOAD( "prg-3",         0x4000, 0x2000, CRC(373c7d82) SHA1(e68e1fd1d5e48c709280a714d7df330fc29df03a) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "vram-1",        0x0000, 0x2000, CRC(c4226e54) SHA1(4eb59db4d9688f62ecbaee7dde4cf3117e8f942d) )
	ROM_LOAD( "vram-2",        0x2000, 0x2000, CRC(2a9b1e36) SHA1(daaf90753477f22dba8f9a9d28799c63622351a5) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "obj-a",         0x0000, 0x2000, CRC(90b60771) SHA1(67cf20fa47439a16f9ebe07aa128cb267b256704) )
	ROM_LOAD( "obj-b",         0x2000, 0x2000, CRC(e14ee689) SHA1(797985c9b5b9a2c39c98defae56d119ce41714d6) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "wanted.k7",     0x0000, 0x0100, CRC(2ba90a00) SHA1(ec545b4682be2875f4e6440a28306a36c6f1771d) )   /* palette low 4 bits */
	ROM_LOAD( "wanted.k6",     0x0100, 0x0100, CRC(a93d87cc) SHA1(73920a2a2842efc8678f280e3b30177f4ca6ea9c) )   /* palette high 4 bits */
ROM_END

/*
Battle Cruiser M12
(C) Sigma Enterprises, Inc. 1983

PCB Number: F-005A

CPU: NEC D780C (Z80)
SND 2 x AY-3-8912
XTAL: 12.000MHz
DIPS: 2 x 8 position

All roms type 2764
Both proms type MB7052 (compatible to 82s129)
RAM: 1 x 8416, 1 x AM9122, 2 x D2125, 4 x M5L2114

PCB Layout:

       NECD780C                      D-84_4  D-84_5  D-84_6  D-84_7

8416  D-84_1  D-84_2  D-84_3                  D2125    AM9122        12.000MHz
                                              D2125
                                              M5L2114
                                              M5L2114
                                              M5L2114
      SW1                                     M5L2114
      SW2
AY-3-8912    AY-3-8912                                             BCM12COL.K7
                                                                   BCM12COL.K6
*/

ROM_START( bcruzm12 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d-84_3.12c",    0x0000, 0x2000, CRC(132baa3d) SHA1(79f7a1dc49b6e45e68f4008f3ee4e383846f75d5) )
	ROM_LOAD( "d-84_2.12b",    0x2000, 0x2000, CRC(1a788d1f) SHA1(5029f93f45d328a282d56e010eee68287b6b9306) )
	ROM_LOAD( "d-84_1.12ab",   0x4000, 0x2000, CRC(9d5b3017) SHA1(bced3f39faf94ce25cba382010f2c2ed322e9d7b) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "d-84_5.17f",    0x0000, 0x2000, CRC(2e963f6a) SHA1(dcc32ab4a4fa241b6f4a211642d00f6e0438f466) )
	ROM_LOAD( "d-84_4.17ef",   0x2000, 0x2000, CRC(fe186459) SHA1(3b0ee1fe98c835271f5b67de5ca0507827e25d71) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "d-84_7.17h",    0x0000, 0x2000, CRC(a5be90ef) SHA1(6037d924296ba62999aafe665396fef142d73df2) )
	ROM_LOAD( "d-84_6.17fh",   0x2000, 0x2000, CRC(1337dc01) SHA1(c55bfc6dd15a499dd71da0acc5016035a7c51f16) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "bcm12col.7k",   0x0000, 0x0100, CRC(bf4f2671) SHA1(dde6da568ecf0121910f4b507c83fe6230b07c8d) )   /* palette low 4 bits */
	ROM_LOAD( "bcm12col.6k",   0x0100, 0x0100, CRC(59f955f6) SHA1(6d6d784971569e0af7cec8bd36659f24a652cd6a) )   /* palette high 4 bits */
ROM_END

ROM_START( hopprobo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hopper01.3k",   0x0000, 0x1000, CRC(fd7935c0) SHA1(08f1b1589203d0f5967cc780a15412ca599c5d2f) )
	ROM_LOAD( "hopper02.3l",   0x1000, 0x1000, CRC(df1a479a) SHA1(27b56498887041692536f5f71abfa3d6b9098a7d) )
	ROM_LOAD( "hopper03.3n",   0x2000, 0x1000, CRC(097ac2a7) SHA1(d7e571a1f44239cbe6080efdbe2feeff8a8653bb) )
	ROM_LOAD( "hopper04.3p",   0x3000, 0x1000, CRC(0f4f3ca8) SHA1(22976fb63a8931c7db000ccf9de51417c0d512fe) )
	ROM_LOAD( "hopper05.3r",   0x4000, 0x1000, CRC(9d77a37b) SHA1(a63419382cb86e3f762637692c646bbd6b1b664d) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "hopper06.5c",   0x0000, 0x2000, CRC(68f79bc8) SHA1(17a9891c98c34935831493e27bd0cf3239644de0) )
	ROM_LOAD( "hopper07.5d",   0x2000, 0x1000, CRC(33d82411) SHA1(b112d76c9d35a2dbd051825e7818e894e8d9259f) )
	ROM_RELOAD(                0x3000, 0x1000 )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "hopper09.6k",   0x0000, 0x2000, CRC(047921c7) SHA1(8ef4722a98be540e4b5c67965599c400511b4a52) )
	ROM_LOAD( "hopper08.6f",   0x2000, 0x2000, CRC(06d37e64) SHA1(c0923a1a40dca43b66e14d755dacf7767d62ab8b) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "7052hop.1b",    0x0000, 0x0100, CRC(94450775) SHA1(e15fcf6d1cd7cfc0d98e82bd0559b6d342aac9ed) ) /* palette low 4 bits */
	ROM_LOAD( "7052hop.1c",    0x0100, 0x0100, CRC(a76bbd51) SHA1(5c61d93ab1e9c80b30cfbc2cbd13ede32f0f4f61) ) /* palette high 4 bits */
ROM_END


/*    year  name      parent   machine   inputs */
GAME( 1982, marineb,  0,       marineb,  marineb, driver_device, 0, ROT0,   "Orca", "Marine Boy", MACHINE_SUPPORTS_SAVE )
GAME( 1982, changes,  0,       changes,  changes, driver_device, 0, ROT0,   "Orca", "Changes", MACHINE_SUPPORTS_SAVE )
GAME( 1982, changesa, changes, changes,  changes, driver_device, 0, ROT0,   "Orca (Eastern Micro Electronics, Inc. license)", "Changes (EME license)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, looper,   changes, changes,  changes, driver_device, 0, ROT0,   "Orca", "Looper", MACHINE_SUPPORTS_SAVE )
GAME( 1982, springer, 0,       springer, marineb, driver_device, 0, ROT270, "Orca", "Springer", MACHINE_SUPPORTS_SAVE )
GAME( 1983, hoccer,   0,       hoccer,   hoccer, driver_device,  0, ROT90,  "Eastern Micro Electronics, Inc.", "Hoccer (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, hoccer2,  hoccer,  hoccer,   hoccer, driver_device,  0, ROT90,  "Eastern Micro Electronics, Inc.", "Hoccer (set 2)" , MACHINE_SUPPORTS_SAVE )  /* earlier */
GAME( 1983, bcruzm12, 0,       bcruzm12, bcruzm12, driver_device,0, ROT90,  "Sigma Enterprises Inc.", "Battle Cruiser M-12", MACHINE_SUPPORTS_SAVE )
GAME( 1983, hopprobo, 0,       hopprobo, marineb, driver_device, 0, ROT90,  "Sega", "Hopper Robo", MACHINE_SUPPORTS_SAVE )
GAME( 1984, wanted,   0,       wanted,   wanted, driver_device,  0, ROT90,  "Sigma Enterprises Inc.", "Wanted", MACHINE_SUPPORTS_SAVE )
