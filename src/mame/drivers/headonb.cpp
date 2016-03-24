// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Italian bootleg of Head On, by EFG Sanremo (late 70s to early 80s).
  The game title is unknown. Someone wrote FOOL RACE on a piece of tape
  on the pcb, but that's not really credible.

  It's on much cheaper hardware than the original: 8080 instead of Z80,
  and less RAM needed with the gfx tiles being on ROM.

TODO:
  - Sound is unknown, probably simple and discrete
  - wrong coin handling, it writes to port $01 to reset coin status?
  - other unknown writes
  - dipswitch settings

Sounds:
  - Port 0,20 dot (can use invaders hit sound)
  - Port 1,40 skid (can use invaders shoot sound)
  - Port 1,80 crash
  - Port 1,10 could be car motor sound
Other outs:
  - Port 0,FF when coins inserted
  - Port 2,0 ; 7,0 ; 8,0 unknown
  - Port 3,1 while game is running
  - Port 1,0 i assume is silence during attract mode

***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"


class headonb_state : public driver_device
{
public:
	headonb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_video_ram(*this, "video_ram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<UINT8> m_video_ram;

	tilemap_t *m_tilemap;

	DECLARE_WRITE8_MEMBER(video_ram_w);

	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TILE_GET_INFO_MEMBER(get_tile_info);
};


/***************************************************************************

  Video

***************************************************************************/

TILE_GET_INFO_MEMBER(headonb_state::get_tile_info)
{
	UINT8 code = m_video_ram[tile_index];
	SET_TILE_INFO_MEMBER(0, code, 0, 0);
}

void headonb_state::video_start()
{
	m_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(headonb_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

UINT32 headonb_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/***************************************************************************

  I/O

***************************************************************************/

WRITE8_MEMBER(headonb_state::video_ram_w)
{
	m_video_ram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

static ADDRESS_MAP_START( headonb_map, AS_PROGRAM, 8, headonb_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_MIRROR(0x4000)
	AM_RANGE(0xe000, 0xe3ff) AM_RAM_WRITE(video_ram_w) AM_SHARE("video_ram")
	AM_RANGE(0xff00, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( headonb_io_map, AS_IO, 8, headonb_state )
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN0")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("IN1")
ADDRESS_MAP_END


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( headonb )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // must be low?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) // wrong

	PORT_START("IN1")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )
INPUT_PORTS_END


/***************************************************************************

  Machine Config

***************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( headonb )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( headonb, headonb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080A, XTAL_20MHz / 10) // divider guessed
	MCFG_CPU_PROGRAM_MAP(headonb_map)
	MCFG_CPU_IO_MAP(headonb_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", headonb_state, irq0_line_hold) // where is irqack?

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(headonb_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", headonb)
	MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* sound hardware */
	// TODO
MACHINE_CONFIG_END


/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( headonb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",  0x0000, 0x0400, CRC(11586f44) SHA1(95426bbae19e152c103ac589e62e5f7c803a9bd0) ) // sldh
	ROM_LOAD( "2.bin",  0x0400, 0x0400, CRC(c3449b99) SHA1(68f0af22c9f3ca971ac7fd5909bb7991d3a0474a) ) // sldh
	ROM_LOAD( "3.bin",  0x0800, 0x0400, CRC(9c80b99e) SHA1(4443151df7b2833a7534451fbebf89650266c01e) ) // sldh
	ROM_LOAD( "4.bin",  0x0c00, 0x0400, CRC(ed5ecc4e) SHA1(2f30e3090ff303c4198aa94f97d571ccc3b2b42e) ) // sldh
	ROM_LOAD( "5.bin",  0x2000, 0x0400, CRC(13cdb6da) SHA1(c58c262e7e880ef199d22d538bfb865eb03e0386) ) // sldh
	ROM_LOAD( "6.bin",  0x2400, 0x0400, CRC(e498d21b) SHA1(6f7beb44ce69f448540f594b231a9d9f673916dc) ) // sldh
	ROM_LOAD( "7.bin",  0x2800, 0x0400, CRC(ce2ef8d9) SHA1(87cdddf78b05078338de1711ba7ee17f7faa76c5) )
	ROM_LOAD( "8.bin",  0x2c00, 0x0400, CRC(85f216e0) SHA1(629a512b25d17a23be4ca92f43c29e6b969d690f) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "10.bin", 0x0000, 0x0400, CRC(198f4671) SHA1(129b4575c4148b4aef16c0dd047f4d62fa6a3b17) )
	ROM_LOAD( "9.bin",  0x0400, 0x0400, CRC(2b4d3afe) SHA1(f5f49c6b1b9b44f8922825cbbc563549c8eab97b) )
ROM_END


GAME( 1979, headonb, headon, headonb, headonb, driver_device, 0, ROT0, "bootleg (EFG Sanremo)", "Head On (bootleg on dedicated hardware)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
