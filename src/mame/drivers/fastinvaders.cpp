// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"


class fastinvaders_state : public driver_device
{
public:
	fastinvaders_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/***************************************************************************

  Video

***************************************************************************/

void fastinvaders_state::video_start()
{
}

UINT32 fastinvaders_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


/***************************************************************************

  I/O

***************************************************************************/

static ADDRESS_MAP_START( fastinvaders_map, AS_PROGRAM, 8, fastinvaders_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
ADDRESS_MAP_END



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( fastinvaders )
INPUT_PORTS_END


/***************************************************************************

  Machine Config

***************************************************************************/

static const gfx_layout charlayout =
{
	16,16,
	RGN_FRAC(1,2),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, RGN_FRAC(1,2) + 0, RGN_FRAC(1,2) +1, RGN_FRAC(1,2) +2, RGN_FRAC(1,2) +3, RGN_FRAC(1,2) +4, RGN_FRAC(1,2) +5, RGN_FRAC(1,2) +6, RGN_FRAC(1,2) +7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8
};

static GFXDECODE_START( fastinvaders )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( fastinvaders, fastinvaders_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8085A, 10000000 ) // guess
	MCFG_CPU_PROGRAM_MAP(fastinvaders_map)
//  MCFG_CPU_IO_MAP(fastinvaders_io_map)
//  MCFG_CPU_VBLANK_INT_DRIVER("screen", fastinvaders_state, irq0_line_hold) // where is irqack?

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(fastinvaders_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", fastinvaders)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	/* sound hardware */
	// TODO
MACHINE_CONFIG_END


/***************************************************************************

  Game drivers

***************************************************************************/

// the last pair of gfx roms were mixed up in each set (each set contained 2 identical roms rather than one of each) hopefully nothing else was

ROM_START( fi6845 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "R2.1A",     0x0000, 0x0200, CRC(6180d652) SHA1(3aac67f52897059c8366f52c41464052ce860ae8) )
	ROM_LOAD( "R2.1B",     0x0200, 0x0200, CRC(f10baf3e) SHA1(4a1702c24e949d9bef990033b5507a573abd7bc3) )
	ROM_LOAD( "R2.2A",     0x0400, 0x0200, CRC(f446ef0d) SHA1(2be337c1197d14e5ffc33ea05b5262f1ea17d442) )
	ROM_LOAD( "R2.2B",     0x0600, 0x0200, CRC(b97e35a3) SHA1(0878a83c7f9f0645749fdfb1ff372d0e04833c9e) )
	ROM_LOAD( "R2.3A",     0x0800, 0x0200, CRC(988f36da) SHA1(7229f660a6a9cf9f66d0924c63772daabd09710e) )
	ROM_LOAD( "R2.3B",     0x0a00, 0x0200, CRC(be7dc34d) SHA1(e4aa1617629869c9ff5f39b656001a43020f0cb8) )
	ROM_LOAD( "R2.4A",     0x0c00, 0x0200, CRC(199cb227) SHA1(064f6005a3f1afe9ca04e93e6bc999735a12a05b) )
	ROM_LOAD( "R2.4B",     0x0e00, 0x0200, CRC(ca41218a) SHA1(01529e21c44669dc96df4331e87d45098a263772) )
	ROM_LOAD( "R2.5A",     0x1000, 0x0200, CRC(e8ecf0da) SHA1(723edaa6f069a21ab7496a24831f23b4b4d73629) )
	ROM_LOAD( "R2.5B",     0x1200, 0x0200, CRC(cb2d8029) SHA1(cac067accddfcce6014d2a425b4291ed8226d169) )
	ROM_LOAD( "R2.6A",     0x1400, 0x0200, CRC(e4d4cc96) SHA1(2dc3d7e4cbd93220285938aec31011b685563cf7) )
	ROM_LOAD( "R2.6B",     0x1600, 0x0200, CRC(0c96ba4a) SHA1(0da104472c33523d4002cab0c77ca20fc2998a2c) )
	ROM_LOAD( "R2.7A",     0x1800, 0x0200, CRC(c9207fbd) SHA1(bf388e26ee1e2073b8a641ba6fb551c24d471a70) )

	ROM_REGION( 0x0c00, "gfx1", 0 )
	ROM_LOAD( "_C2.1F",     0x0000, 0x0200, CRC(9feca88a) SHA1(14a8c46eb51eed01b7b537a9931cd092cec2019f) )
	ROM_LOAD( "_C2.1G",     0x0200, 0x0200, CRC(79fc3963) SHA1(25651d1031895a01a2a4751b355ff1200a899ac5) )
	ROM_LOAD( "_C2.1H",     0x0400, 0x0200, CRC(936171e4) SHA1(d0756b49bfd5d58a79f735d4a98a99cce7604b0e) )
	ROM_LOAD( "_C2.2F",     0x0600, 0x0200, CRC(3bb16f55) SHA1(b1cc1e2346acd0e5c84861b414b4677871079844) )
	ROM_LOAD( "_C2.2G",     0x0800, 0x0200, CRC(19828c47) SHA1(f215ce55be32b3564e1b7cc19500d38a93117051) )
	ROM_LOAD( "_C2.2H",     0x0a00, 0x0200, CRC(284ae4eb) SHA1(6e28fcd9d481d37f47728f22f6048b29266f4346) )
ROM_END

ROM_START( fi8275 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "R1.1A",     0x0000, 0x0200, CRC(fef96dfe) SHA1(f6df0cf6b5d90ea07ee890985c8cbf0f68e08550) )
	ROM_LOAD( "R1.1B",     0x0200, 0x0200, CRC(c48c6ebc) SHA1(f1f86839819b6abce9ff55c1b02bbf2c4036c51a) )
	ROM_LOAD( "R1.2A",     0x0400, 0x0200, CRC(626a740c) SHA1(3a1df1d71acc207b1b952ad5176804fce27ea97e) )
	ROM_LOAD( "R1.2B",     0x0600, 0x0200, CRC(fbe9782e) SHA1(3661bd03e029e4d2092d259f38a7dec9e763761c) )
	ROM_LOAD( "R1.3A",     0x0800, 0x0200, CRC(6e10de0d) SHA1(8d937f6f2fe1a79b62e6e75536889416ec0071e3) )
	ROM_LOAD( "R1.3B",     0x0a00, 0x0200, CRC(ee1bac50) SHA1(f723b2d1c2a1194aa2df67d48f2b669f5076d857) )
	ROM_LOAD( "R1.4A",     0x0c00, 0x0200, CRC(7faff8f1) SHA1(9275123d6513ab917506f6e9d929935ed1bef429) )
	ROM_LOAD( "R1.4B",     0x0e00, 0x0200, CRC(205ca0c1) SHA1(edf68e9c75523e1b6a485b27af60592fdfb78e04) )
	ROM_LOAD( "R1.5A",     0x1000, 0x0200, CRC(9ada6666) SHA1(f965a08c75fa87e8e3fd7595dcd98231e976e072) )
	ROM_LOAD( "R1.5B",     0x1200, 0x0200, CRC(0f617215) SHA1(b342c783335ab26c036ae77f63a2e932a590c2fa) )
	ROM_LOAD( "R1.6A",     0x1400, 0x0200, CRC(75ea69ae) SHA1(edd9bf686c169ca64373ea87ba92fab4e8c6ee4d) )
	//ROM_LOAD( "R1.6B",   0x1600, 0x0200, CRC(11111111) SHA1(1111111111111111111111111111111111111111) ) // not populated
	ROM_LOAD( "R1.7A",     0x1800, 0x0200, CRC(6e12538f) SHA1(aa08a2db2e5570b431afc967ea5fd749c4f82e33) )
	ROM_LOAD( "R1.7B",     0x1a00, 0x0200, CRC(7270d194) SHA1(7cef9c420c3c3cbc5846bd22137213a78506a8d3) )

	ROM_REGION( 0x0c00, "gfx1", 0 )
	ROM_LOAD( "_C1.1B",     0x0000, 0x0200, CRC(9feca88a) SHA1(14a8c46eb51eed01b7b537a9931cd092cec2019f) )
	ROM_LOAD( "_C1.2B",     0x0200, 0x0200, CRC(79fc3963) SHA1(25651d1031895a01a2a4751b355ff1200a899ac5) )
	ROM_LOAD( "_C1.3B",     0x0400, 0x0200, CRC(936171e4) SHA1(d0756b49bfd5d58a79f735d4a98a99cce7604b0e) )
	ROM_LOAD( "_C1.1A",     0x0600, 0x0200, CRC(3bb16f55) SHA1(b1cc1e2346acd0e5c84861b414b4677871079844) )
	ROM_LOAD( "_C1.2A",     0x0800, 0x0200, CRC(19828c47) SHA1(f215ce55be32b3564e1b7cc19500d38a93117051) )
	ROM_LOAD( "_C1.3A",     0x0a00, 0x0200, CRC(284ae4eb) SHA1(6e28fcd9d481d37f47728f22f6048b29266f4346) )
ROM_END


GAME( 1979, fi6845, 0,      fastinvaders, fastinvaders, driver_device, 0, ROT0, "Fiberglass", "Fast Invaders (6845 version)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1979, fi8275, fi6845, fastinvaders, fastinvaders, driver_device, 0, ROT0, "Fiberglass", "Fast Invaders (8275 version)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
