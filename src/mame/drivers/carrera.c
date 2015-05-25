// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

This is a simple 'Pairs' game called
Carrera by BS Electronics



PCB Layout
----------

|----------------------------------------------|
|            22.1184MHz  Z80                   |
|                                     ROM.IC1  |
|_  BATTERY              ROM.IC22              |
  |                                            |
 _|                      6116         ROM.IC2  |
|                                              |
|                                              |
|J  AY-3-8910                         ROM.IC3  |
|A                       DSW1(8)               |
|M                                             |
|M                       DSW2(8)      ROM.IC4  |
|A                                             |
|                        DSW3(8)               |
|                                     ROM.IC5  |
|_  PROM.IC39            DSW4(8)               |
  |                                    6116    |
 _|                                            |
|                   HD6845             6116    |
|----------------------------------------------|
Notes:
      Z80 @ 3.6864MHz [22.1184/6]
      AY-3-8910 @ 1.8432MHz [22.1184/12]


Emulation Notes:
 Corrupt Tile on the first R in Carrera? (unlikely to be a bug, HW is very simple..)
 There is also a 'Bomberman' title in the GFX roms, unused from what I can see.

TODO:
- Are colors 100% correct? Needs a reference to be sure.
- There are reel gfxs on the roms (near the end), left-over or there's a way to enable it?
  Update: if you trigger a normal irq 0 instead of a NMI the game will change into a proper 8 liner game without inputs. Investigate on it...

*/

#define MASTER_CLOCK    XTAL_22_1184MHz

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"


class carrera_state : public driver_device
{
public:
	carrera_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_tileram(*this, "tileram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	required_shared_ptr<UINT8> m_tileram;
	DECLARE_READ8_MEMBER(unknown_r);
	DECLARE_PALETTE_INIT(carrera);
	UINT32 screen_update_carrera(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


static ADDRESS_MAP_START( carrera_map, AS_PROGRAM, 8, carrera_state )
	AM_RANGE(0x0000, 0x4fff) AM_ROM
	AM_RANGE(0xe000, 0xe7ff) AM_RAM
	AM_RANGE(0xe800, 0xe800) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xe801, 0xe801) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0xf000, 0xffff) AM_RAM AM_SHARE("tileram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, carrera_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("IN3")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("IN4")
	AM_RANGE(0x05, 0x05) AM_READ_PORT("IN5")
	AM_RANGE(0x06, 0x06) AM_WRITENOP // ?
	AM_RANGE(0x08, 0x09) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( carrera )
	PORT_START("IN0")   /* Port 0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	/* unused / unknown inputs, not dips */
	PORT_DIPNAME( 0x20, 0x20, "0" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")   /* Port 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	/* unused / unknown inputs, not dips */
	PORT_DIPNAME( 0x04, 0x04, "1" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	/* I suspect the 4 below are the 4xDSWs */
	PORT_START("IN2")   /* Port 2 */
	PORT_DIPNAME( 0x01, 0x01, "2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN3")   /* Port 3 */
	PORT_DIPNAME( 0x01, 0x01, "3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN4")   /* Port 4 */
	PORT_DIPNAME( 0x01, 0x01, "4" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN5")   /* Port 5 */
	PORT_DIPNAME( 0x01, 0x01, "5" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Playing Graphics" )
	PORT_DIPSETTING(    0x08, "Bricks" )
	PORT_DIPSETTING(    0x00, "Fruits" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Debug?" ) // displays numbers over the game area
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(0,5), RGN_FRAC(1,5),RGN_FRAC(2,5),RGN_FRAC(3,5),RGN_FRAC(4,5) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( carrera )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 1 )
GFXDECODE_END

UINT32 carrera_state::screen_update_carrera(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y;
	int count = 0;

	for (y=0;y<32;y++)
	{
		for (x=0;x<64;x++)
		{
			int tile = m_tileram[count&0x7ff] | m_tileram[(count&0x7ff)+0x800]<<8;

			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect,tile,0,0,0,x*8,y*8);
			count++;
		}
	}
	return 0;
}

READ8_MEMBER(carrera_state::unknown_r)
{
	return machine().rand();
}

PALETTE_INIT_MEMBER(carrera_state, carrera)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int br_bit0, br_bit1, bit0, bit1, r, g, b;
	int i;

	for (i = 0; i < 0x20; ++i)
	{
		br_bit0 = (color_prom[0] >> 6) & 0x01;
		br_bit1 = (color_prom[0] >> 7) & 0x01;

		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 3) & 0x01;
		b = 0x0e * br_bit0 + 0x1f * br_bit1 + 0x43 * bit0 + 0x8f * bit1;
		bit0 = (color_prom[0] >> 1) & 0x01;
		bit1 = (color_prom[0] >> 4) & 0x01;
		g = 0x0e * br_bit0 + 0x1f * br_bit1 + 0x43 * bit0 + 0x8f * bit1;
		bit0 = (color_prom[0] >> 2) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		r = 0x0e * br_bit0 + 0x1f * br_bit1 + 0x43 * bit0 + 0x8f * bit1;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}


static MACHINE_CONFIG_START( carrera, carrera_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK / 6)
	MCFG_CPU_PROGRAM_MAP(carrera_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", carrera_state,  nmi_line_pulse)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(carrera_state, screen_update_carrera)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_MC6845_ADD("crtc", MC6845, "screen", MASTER_CLOCK / 16)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", carrera)
	MCFG_PALETTE_ADD("palette", 32)
	MCFG_PALETTE_INIT_OWNER(carrera_state, carrera)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, MASTER_CLOCK/12)
	/* these are set as input, but I have no idea which input port it uses is for the AY */
	MCFG_AY8910_PORT_A_READ_CB(READ8(carrera_state, unknown_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(carrera_state, unknown_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


ROM_START( carrera )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27512.ic22", 0x00000, 0x10000, CRC(2385b9c8) SHA1(12d4397779e074096fbb23b114985f104366b79c) )

	ROM_REGION( 0x50000, "gfx1", 0 )
	ROM_LOAD( "27512.ic1", 0x00000, 0x10000, CRC(a16e914e) SHA1(09f2271f193a7bffd62ef6e428ecbf9aa1154860) )
	ROM_LOAD( "27512.ic2", 0x10000, 0x10000, CRC(147036a5) SHA1(34b4818fe61c5b13220b0a2001987b68b655b2cb) )
	ROM_LOAD( "27512.ic3", 0x20000, 0x10000, CRC(920eee0e) SHA1(85e6d5292b751c57c64d17858bd00292356599e3) )
	ROM_LOAD( "27512.ic4", 0x30000, 0x10000, CRC(97433f36) SHA1(39f3c6b76ad540693682832aba6e4fc400ca3753) )
	ROM_LOAD( "27512.ic5", 0x40000, 0x10000, CRC(ffa75920) SHA1(aa5619f5aabcdfa250bb24bcad101a8c512a1776) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "82s123.ic39", 0x00, 0x20, CRC(af16359f) SHA1(1ff5c9d7807e52be09c0ded56fb68a47e41b3fcf) )
ROM_END


GAME( 19??, carrera, 0, carrera, carrera, driver_device,0, ROT0, "BS Electronics", "Carrera (Version 6.7)", 0 )
