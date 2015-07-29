// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Royal Gum

Unknown CPU (either Z80 or Z180) - or at least rgum.u47 looks z80 related
rgum.u5 is for a 6502?

Big Black Box in the middle of the PCB (for encryption, or containing roms?)

The ppi at 3000-3003 seems to be a dual port communication thing with the z80.

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6502/m65c02.h"
#include "video/mc6845.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"


class rgum_state : public driver_device
{
public:
	rgum_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vram(*this, "vram"),
		m_cram(*this, "cram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	required_shared_ptr<UINT8> m_vram;
	required_shared_ptr<UINT8> m_cram;
	UINT8 m_hbeat;
	DECLARE_CUSTOM_INPUT_MEMBER(rgum_heartbeat_r);
	virtual void video_start();
	UINT32 screen_update_royalgum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


void rgum_state::video_start()
{
}

UINT32 rgum_state::screen_update_royalgum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y,count;
	gfx_element *gfx = m_gfxdecode->gfx(0);

	count = 0;

	for(y=0;y<32;y++)
	{
		for(x=0;x<66;x++)
		{
			int tile = m_vram[count] | ((m_cram[count] & 0xf) <<8);

			gfx->opaque(bitmap,cliprect,tile,0,0,0,x*8,y*8);

			count++;
		}
	}

	return 0;
}

static ADDRESS_MAP_START( rgum_map, AS_PROGRAM, 8, rgum_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM //not all of it?

	AM_RANGE(0x0800, 0x0800) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x0801, 0x0801) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)

	AM_RANGE(0x2000, 0x2000) AM_DEVWRITE("aysnd", ay8910_device, data_w)
	AM_RANGE(0x2002, 0x2002) AM_DEVREADWRITE("aysnd", ay8910_device, data_r, address_w)

	AM_RANGE(0x2801, 0x2801) AM_READNOP //read but value discarded?
	AM_RANGE(0x2803, 0x2803) AM_READNOP

	AM_RANGE(0x3000, 0x3003) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)

	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0x5000, 0x57ff) AM_RAM AM_SHARE("cram")

	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


CUSTOM_INPUT_MEMBER(rgum_state::rgum_heartbeat_r)
{
	m_hbeat ^= 1;

	return m_hbeat;
}


static INPUT_PORTS_START( rgum )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0" )
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
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, rgum_state,rgum_heartbeat_r, NULL)
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
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

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
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
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN ) //communication port with the z80?

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
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

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
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

INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0, 1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( rgum )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END


static MACHINE_CONFIG_START( rgum, rgum_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M65C02,24000000/16)      /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(rgum_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", rgum_state,  nmi_line_pulse)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(rgum_state, screen_update_royalgum)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_MC6845_ADD("crtc", MC6845, "screen", 24000000/16)   /* unknown clock & type, hand tuned to get ~50 fps (?) */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN0"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN1"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN2"))

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", rgum)
	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 24000000/16) /* guessed to use the same xtal as the crtc */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END




ROM_START( rgum )
	ROM_REGION( 0x20000, "z80cpu", 0 )
	ROM_LOAD( "rgum.u47", 0x00000, 0x20000, CRC(fe410eb9) SHA1(25180ba336269279f251be5483c210a581d27197) ) // encrypted.. 2nd half empty

	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rgum.u5", 0x00000, 0x10000, CRC(9d2d1681) SHA1(1c1da0d970ea2cf58f7961417ab6986cc667da5c) ) // plaintext in here, but firt half is empty

	ROM_REGION( 0x10000, "unk", 0 )
	ROM_LOAD( "rgum.u6", 0x00000, 0x2000, CRC(15a34117) SHA1(c7e0aef4007abfaaa533feb026148ba03230b79f) ) // near the data rom, mostly empty

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "rgum.u16", 0x00000, 0x8000, CRC(2a2c8d78) SHA1(2ce335b900dccbc34ad8ae7ae02ec7c75ffcd559) ) // first half empty
	ROM_CONTINUE(0x00000,0x8000)
	ROM_LOAD( "rgum.u17", 0x08000,  0x8000, CRC(fae4e41a) SHA1(421aac2b567040c3a56e01aa70880c94450eaf76) ) // first half empty
	ROM_CONTINUE(0x08000,0x8000)
	ROM_LOAD( "rgum.u18", 0x10000, 0x8000, CRC(79b17da7) SHA1(31e1845261b0152df56135c212e55c4048b7496f) ) // first half empty
	ROM_CONTINUE(0x10000,0x8000)
ROM_END


GAME( 199?, rgum, 0, rgum, rgum, driver_device, 0, ROT0, "<unknown>",         "Royal Gum (Italy)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
