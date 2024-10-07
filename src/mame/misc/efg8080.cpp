// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

EFG Sanremo i8080 arcade hardware

Fool Race: Italian imitation of Head On, by EFG Sanremo, it's not a bootleg.
The game title is Fool Race, confirmed by one of the programmers.
It's on much cheaper hardware than the original: 8080 instead of Z80,
and less RAM needed with the gfx tiles being on ROM.

Sounds:
- Port 0,20 dot
- Port 1,40 skid
- Port 1,80 crash
- Port 1,10 could be car motor sound

Other outs:
- Port 0,FF when coins inserted
- Port 2,0 ; 7,0 ; 8,0 unknown
- Port 3,1 while game is running
- Port 1,0 i assume is silence during attract mode

Black Hole: It is a remake of Universal's Space Panic. Same PCB as Fool Race,
with upgraded graphics capability.

TODO:
- Sound is unknown, probably simple and discrete
- other unknown writes
- dipswitch settings

******************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


namespace {

class efg_state : public driver_device
{
public:
	efg_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_video_ram(*this, "video_ram")
	{ }

	void foolrace(machine_config &config);
	void blackhol(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<u8> m_video_ram;

	tilemap_t *m_tilemap = nullptr;

	void video_ram_w(offs_t offset, u8 data);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TILE_GET_INFO_MEMBER(get_tile_info);
	void foolrace_map(address_map &map) ATTR_COLD;
	void foolrace_io(address_map &map) ATTR_COLD;
	void blackhol_map(address_map &map) ATTR_COLD;
	void blackhol_io(address_map &map) ATTR_COLD;
};



/******************************************************************************
    Video
******************************************************************************/

TILE_GET_INFO_MEMBER(efg_state::get_tile_info)
{
	u8 code = m_video_ram[tile_index];
	tileinfo.set(0, code, 0, 0);
}

void efg_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(efg_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

u32 efg_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

void efg_state::video_ram_w(offs_t offset, u8 data)
{
	m_video_ram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}



/******************************************************************************
    Address Maps
******************************************************************************/

void efg_state::foolrace_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().mirror(0x4000);
	map(0xe000, 0xe3ff).ram().w(FUNC(efg_state::video_ram_w)).share("video_ram");
	map(0xff00, 0xffff).ram();
}

void efg_state::foolrace_io(address_map &map)
{
	map(0x01, 0x01).portr("IN0");
	map(0x04, 0x04).portr("DSW");
}

void efg_state::blackhol_map(address_map &map)
{
	foolrace_map(map);
	map(0xfc00, 0xffff).ram(); // 2*2114
}

void efg_state::blackhol_io(address_map &map)
{
	foolrace_io(map);
	map(0x02, 0x02).portr("IN1");
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( foolrace )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // must be low?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "DSW:!1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DSW:!2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "DSW:!3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "DSW:!4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "DSW:!5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "DSW:!6" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("DSW:!7,!8")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0xc0, "4" )
	PORT_DIPSETTING(    0x40, "5" )
INPUT_PORTS_END

static INPUT_PORTS_START( blackhol )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("DSW:!1,!2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPNAME( 0x04, 0x00, "Oxygen" ) PORT_DIPLOCATION("DSW:!3")
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x04, "Fast" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "DSW:!4" )
	PORT_DIPNAME( 0x70, 0x50, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DSW:!5,!6,!7")
	PORT_DIPSETTING(    0x10, "A 2C/1C, B 1C/1C" ) // A 2C/1C only applies to 1st coin
	PORT_DIPSETTING(    0x00, "A 1C/1C, B 1C/2C" )
	PORT_DIPSETTING(    0x30, "A 2C/1C, B 1C/3C" ) // "
	PORT_DIPSETTING(    0x20, "A 1C/1C, B 1C/6C" )
	PORT_DIPSETTING(    0x50, "A 1C/1C, B 1C/1C" )
	PORT_DIPSETTING(    0x40, "A 1C/2C, B 1C/2C" )
	PORT_DIPSETTING(    0x70, "A 1C/1C, B 1C/3C" )
	PORT_DIPSETTING(    0x60, "A 1C/2C, B 1C/6C" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "DSW:!8" )
INPUT_PORTS_END



/******************************************************************************
    GFX Layouts
******************************************************************************/

static const gfx_layout charlayout_3bpp =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( gfx_foolrace )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x1, 0, 1 )
GFXDECODE_END

static GFXDECODE_START( gfx_blackhol )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_3bpp, 0, 1 )
GFXDECODE_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void efg_state::foolrace(machine_config &config)
{
	/* basic machine hardware */
	I8080A(config, m_maincpu, XTAL(20'000'000) / 10); // divider guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &efg_state::foolrace_map);
	m_maincpu->set_addrmap(AS_IO, &efg_state::foolrace_io);
	m_maincpu->set_vblank_int("screen", FUNC(efg_state::irq0_line_hold)); // where is irqack?

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 28*8-1);
	screen.set_screen_update(FUNC(efg_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_foolrace);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	// TODO
}

void efg_state::blackhol(machine_config &config)
{
	foolrace(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &efg_state::blackhol_map);
	m_maincpu->set_addrmap(AS_IO, &efg_state::blackhol_io);

	GFXDECODE(config.replace(), m_gfxdecode, "palette", gfx_blackhol);
	PALETTE(config.replace(), "palette", palette_device::BGR_3BIT);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( foolrace )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.1", 0x0000, 0x0400, CRC(11586f44) SHA1(95426bbae19e152c103ac589e62e5f7c803a9bd0) )
	ROM_LOAD( "2.2", 0x0400, 0x0400, CRC(c3449b99) SHA1(68f0af22c9f3ca971ac7fd5909bb7991d3a0474a) )
	ROM_LOAD( "3.3", 0x0800, 0x0400, CRC(9c80b99e) SHA1(4443151df7b2833a7534451fbebf89650266c01e) )
	ROM_LOAD( "4.4", 0x0c00, 0x0400, CRC(ed5ecc4e) SHA1(2f30e3090ff303c4198aa94f97d571ccc3b2b42e) )
	ROM_LOAD( "5.5", 0x2000, 0x0400, CRC(13cdb6da) SHA1(c58c262e7e880ef199d22d538bfb865eb03e0386) )
	ROM_LOAD( "6.6", 0x2400, 0x0400, CRC(e498d21b) SHA1(6f7beb44ce69f448540f594b231a9d9f673916dc) )
	ROM_LOAD( "7.7", 0x2800, 0x0400, CRC(ce2ef8d9) SHA1(87cdddf78b05078338de1711ba7ee17f7faa76c5) )
	ROM_LOAD( "8.8", 0x2c00, 0x0400, CRC(85f216e0) SHA1(629a512b25d17a23be4ca92f43c29e6b969d690f) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "10.10", 0x0000, 0x0400, CRC(198f4671) SHA1(129b4575c4148b4aef16c0dd047f4d62fa6a3b17) )
	ROM_LOAD( "9.9",   0x0400, 0x0400, CRC(2b4d3afe) SHA1(f5f49c6b1b9b44f8922825cbbc563549c8eab97b) )
ROM_END

ROM_START( blackhol )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.1", 0x0000, 0x0800, CRC(2a3d9d68) SHA1(5f7d9c81de706609d6e1e1ed931104d9cc0748bc) )
	ROM_LOAD( "2.2", 0x0800, 0x0800, CRC(8c680f5b) SHA1(c9f77927cfd1189594b3acd0607193dbdd85fa93) )
	ROM_LOAD( "3.3", 0x1000, 0x0800, CRC(57d9f35e) SHA1(b9aca604a3e49cf06673e3ebd48fc67ae94ac406) )
	ROM_LOAD( "4.4", 0x1800, 0x0800, CRC(0c9a1ec7) SHA1(733a2f1c72d0ff81eb479c9fb6b0247ad316315e) )
	ROM_LOAD( "5.5", 0x2000, 0x0800, CRC(b4bfe5ce) SHA1(54ad49f7bd73cd534ced194daa393d9401eb87b6) )
	ROM_LOAD( "6.6", 0x2800, 0x0800, CRC(14c185ea) SHA1(29a9606661b08fa3f3ffd598a728df7e4cda6c20) )

	ROM_REGION( 0x1800, "gfx1", 0 ) // on a daughterboard
	ROM_LOAD( "9",  0x0000, 0x0800, CRC(bc38b467) SHA1(a79196a913e1dd1e17299a7d2a32c1bfee599892) )
	ROM_LOAD( "10", 0x0800, 0x0800, CRC(3374c6b2) SHA1(0e212cb490a3c3e12f78684ac8019dfd0ecbae66) )
	ROM_LOAD( "11", 0x1000, 0x0800, CRC(354fd3d2) SHA1(1d93095ed45845e018a0f8fcfa4878b23d7c4b7a) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR   NAME      PARENT  MACHINE   INPUT     CLASS      INIT        SCREEN  COMPANY, FULLNAME, FLAGS
GAME( 1979?, foolrace, 0,      foolrace, foolrace, efg_state, empty_init, ROT0,   "EFG Sanremo", "Fool Race", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE ) // imitation of Sega's Head On
GAME( 1981?, blackhol, 0,      blackhol, blackhol, efg_state, empty_init, ROT270, "EFG Sanremo", "Black Hole (EFG Sanremo)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE ) // imitation of Universal's Space Panic
