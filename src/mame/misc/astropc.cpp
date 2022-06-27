// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Astro Russian Fruit Machines on SVGA "Pallas" hardware (PC based).

Some of the games have HDD/CD/Flash images I think, but they're in a format I don't understand.
We need to figure this out and convert them to CHDs (I think.. unless they're flash)

Known games

Title            Dumped  Notes
Fairy Tales          NO  https://www.astrocorp.com.tw/eng/game_1_1.php?gid=30
Arabian Night        NO  https://www.astrocorp.com.tw/eng/game_1_1.php?gid=31
Black Beard         YES  https://www.astrocorp.com.tw/eng/game_1_1.php?gid=32
Dragon Slayer       YES  https://www.astrocorp.com.tw/eng/game_1_1.php?gid=33
Flying Age           NO  https://www.astrocorp.com.tw/eng/game_1_1.php?gid=34
Halloween Party      NO  https://www.astrocorp.com.tw/eng/game_1_1.php?gid=35
Olympian Games      YES  https://www.astrocorp.com.tw/eng/game_1_1.php?gid=36
The Circus           NO  https://www.astrocorp.com.tw/eng/game_1_1.php?gid=37
Treasure Hunting     NO  https://www.astrocorp.com.tw/eng/game_1_1.php?gid=38
World War II         NO  https://www.astrocorp.com.tw/eng/game_1_1.php?gid=39
Ra's Scepter        YES
Hawaii              YES

Note: It's "Hawaii" the same game as "Treasure Hunting"?

*/


#include "emu.h"
#include "cpu/i386/i386.h"

class astropc_state : public driver_device
{
public:
	astropc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	void astropc(machine_config &config);

	void init_astropc();

private:
	void astropc_io(address_map &map);
	void astropc_map(address_map &map);

	// devices
	required_device<cpu_device> m_maincpu;
};

void astropc_state::astropc_map(address_map &map)
{
	map(0x000c0000, 0x000fffff).rom().region("bios", 0);
	map(0xfffc0000, 0xffffffff).rom().region("bios", 0);
}

void astropc_state::astropc_io(address_map &map)
{
}


static INPUT_PORTS_START( astropc )
INPUT_PORTS_END



void astropc_state::astropc(machine_config &config)
{
	/* basic machine hardware */
	I486(config, m_maincpu, 40000000 ); // ??
	m_maincpu->set_addrmap(AS_PROGRAM, &astropc_state::astropc_map);
	m_maincpu->set_addrmap(AS_IO, &astropc_state::astropc_io);
}


ROM_START( blackbd )
	ROM_REGION32_LE(0x40000, "bios", 0) /* motherboard bios */
	ROM_LOAD( "bios.002", 0x0000, 0x040000, CRC(45333544) SHA1(bcc03b4f77b2e447192a1e5ed9f4cc09d0289714) )

	ROM_REGION(0x20000, "rom", 0)
	ROM_LOAD16_BYTE( "b43-chip1.512", 0x0000, 0x010000, CRC(f17e4a65) SHA1(a375715c3d2c1bee01e022eb7b39f9b08207de13) )
	ROM_LOAD16_BYTE( "b43-chip2.512", 0x0001, 0x010000, CRC(612fffe0) SHA1(c762700bdb87c777d8ece9c11addf93850aae6db) )

	ROM_REGION(0x8000000, "drive", 0) // possibly a drive image???
	ROM_LOAD( "black beard ru.04.43.a.dd", 0x0000, 0x3e20000, CRC(14430270) SHA1(1df178bf3b00a60448b82953696ff205adf3dc66) )

	ROM_REGION(0x8000000, "others", 0)
	ROM_LOAD( "93c46-2.046", 0x0000, 0x000080, CRC(08c9dea5) SHA1(647eda3f6ca8b8863417e9a64b87a99843ce3820) )
	ROM_LOAD( "93c46.046", 0x0000, 0x000080, CRC(52428b49) SHA1(e170e83193c97dd0016551e8d0ce56cc48d3afc4) )
ROM_END


ROM_START( blackbda )
	ROM_REGION32_LE(0x40000, "bios", 0) /* motherboard bios */  // wasn't in this set!
	ROM_LOAD( "bios.002", 0x0000, 0x040000, CRC(45333544) SHA1(bcc03b4f77b2e447192a1e5ed9f4cc09d0289714) )

	ROM_REGION(0x20000, "rom", 0)
	ROM_LOAD16_BYTE( "bbird1.512", 0x00000, 0x010000, CRC(0782cd0b) SHA1(976273880dc0357b4c9e432e44c9f82bac55f5e5) )
	ROM_LOAD16_BYTE( "bbird2.512", 0x00001, 0x010000, CRC(9cf9f0a7) SHA1(f8a9c851ca7ea859f90f9ad6afd4cf1178eae039) )

	ROM_REGION(0x8000000, "drive", 0) // possibly a drive image???
	// all 3 in a zip marked 'paragon' ?
	ROM_LOAD( "img_d1.pbf", 0x0000, 0x004c45, CRC(a0dba309) SHA1(4d9dd245cc973fb70aff90cac5a94701b6b6ccd3) )
	ROM_LOAD( "img_d100.p00", 0x0000, 0x36a50ac, CRC(e0353f4f) SHA1(a930ac9e272d5474264490dc3a3223670d00b61d) )
	ROM_LOAD( "img_d100.pfm", 0x0000, 0x000618, CRC(b3738001) SHA1(8266fd53357779d934cd1cb1b5a27b8e9e0dcce2) )
ROM_END

ROM_START( blackbdb )
	ROM_REGION32_LE(0x40000, "bios", 0) /* motherboard bios */  // wasn't in this set!
	ROM_LOAD( "bios.002", 0x0000, 0x040000, CRC(45333544) SHA1(bcc03b4f77b2e447192a1e5ed9f4cc09d0289714) )

	ROM_REGION(0x20000, "rom", 0)
	ROM_LOAD16_BYTE( "u1_bbru44.bin", 0x00000, 0x010000, CRC(bd973bc1) SHA1(1f1997a3c1c70ccec01c1cb44c127356b6412457) )
	ROM_LOAD16_BYTE( "u2_bbru44.bin", 0x00001, 0x010000, CRC(4a6a6a18) SHA1(959ad5a369b27e73e9e879471784ccab1001d114) )

	ROM_REGION(0x8000000, "drive", 0) // possibly a drive image???
	ROM_LOAD( "blackbeard  ru_04b.img", 0x0000, 0x3e20000, CRC(cadbaa2b) SHA1(15033bffedd173622d50ac0adf99e257c207748c) )
ROM_END

ROM_START( dslayrr )
	ROM_REGION32_LE(0x40000, "bios", 0) /* motherboard bios */  // wasn't in this set!
	ROM_LOAD( "bios.002", 0x0000, 0x040000, CRC(45333544) SHA1(bcc03b4f77b2e447192a1e5ed9f4cc09d0289714) )

	ROM_REGION(0x20000, "rom", 0)
	ROM_LOAD16_BYTE( "ds ru 15.21.b-1.bin", 0x00000, 0x010000, CRC(d33fdb7b) SHA1(1bf16716dd534ca8e89063184cd7ccaed732c43b) )
	ROM_LOAD16_BYTE( "ds ru 15.21.b-2.bin", 0x00001, 0x010000, CRC(50065dd5) SHA1(44f60467e90d0bdb82227ce25261d22f921b9903) )

	ROM_REGION(0x8000000, "drive", 0) // possibly a drive image???
	ROM_LOAD( "dragon slayer ru 15.21.b.whx", 0x0000, 0x7c80181, CRC(7af9ed2e) SHA1(082463eb44e8ca144e2e934ba5820ab248599033) )
ROM_END

ROM_START( dslayrra )
	ROM_REGION32_LE(0x40000, "bios", 0) /* motherboard bios */  // wasn't in this set!
	ROM_LOAD( "bios.002", 0x0000, 0x040000, CRC(45333544) SHA1(bcc03b4f77b2e447192a1e5ed9f4cc09d0289714) )

	ROM_REGION(0x20000, "rom", 0)
	ROM_LOAD16_BYTE( "u1-0f0b.rf", 0x00000, 0x010000, CRC(d33fdb7b) SHA1(1bf16716dd534ca8e89063184cd7ccaed732c43b) )
	ROM_LOAD16_BYTE( "u2-f500.rf", 0x00001, 0x010000, CRC(50065dd5) SHA1(44f60467e90d0bdb82227ce25261d22f921b9903) )

	ROM_REGION(0x8000000, "drive", 0) // possibly a drive image???
	ROM_LOAD( "ds.16b", 0x0000, 0x7a80000, CRC(19b229c6) SHA1(eb419dbfdec0ad03c422fdc54e77a5df37442026) )
ROM_END

ROM_START( hawaii )
	ROM_REGION32_LE(0x40000, "bios", 0) /* motherboard bios */  // wasn't in this set!
	ROM_LOAD( "bios.002", 0x0000, 0x040000, CRC(45333544) SHA1(bcc03b4f77b2e447192a1e5ed9f4cc09d0289714) )

	ROM_REGION(0x20000, "rom", 0)
	ROM_LOAD16_BYTE( "rom1.512", 0x00000, 0x010000, CRC(b963d590) SHA1(c6d38cf1865efd8619a9eec07410db1e16e7276d) )
	ROM_LOAD16_BYTE( "rom2.512", 0x00001, 0x010000, CRC(db670705) SHA1(6a76d5114847f54cf98e4c016bb5d47a4b7e1ef8) )

	ROM_REGION(0x8000000, "drive", 0) // possibly a drive image???
	ROM_LOAD( "hawaii   v1.01a .whx", 0x0000, 0x3c0017b, CRC(f033f963) SHA1(5f50aaf3ddbde176388612ea1a4c0040533f2109) )
ROM_END

ROM_START( oligam )
	ROM_REGION32_LE(0x40000, "bios", 0) /* motherboard bios */  // wasn't in this set!
	ROM_LOAD( "bios.002", 0x0000, 0x040000, CRC(45333544) SHA1(bcc03b4f77b2e447192a1e5ed9f4cc09d0289714) )

	ROM_REGION(0x20000, "rom", 0)
	ROM_LOAD16_BYTE( "rom1.512", 0x00000, 0x010000, CRC(41cefe2a) SHA1(5f03f93e92555a76284f97366a9761106901506f) )
	ROM_LOAD16_BYTE( "rom2.512", 0x00001, 0x010000, CRC(caa4fe49) SHA1(6b92af831a210b0c5264dde30fb13611bf2e366c) )

	ROM_REGION(0x8000000, "drive", 0) // possibly a drive image???
	ROM_LOAD( "olympian games ru.04.39.a.dd", 0x0000, 0x3e90000, CRC(ba452de5) SHA1(371e6157bcd5a1ed48b4a75f4962244157610912) )
ROM_END

ROM_START( rasce )
	ROM_REGION32_LE(0x40000, "bios", 0) /* motherboard bios */  // wasn't in this set!
	ROM_LOAD( "bios.002", 0x0000, 0x040000, CRC(45333544) SHA1(bcc03b4f77b2e447192a1e5ed9f4cc09d0289714) )

	ROM_REGION(0x20000, "rom", 0)
	ROM_LOAD16_BYTE( "u1-49c1.rf", 0x00000, 0x010000, CRC(2da68b6a) SHA1(3ad5833841d06495bae3fcca561f23124602864e) )
	ROM_LOAD16_BYTE( "u2-3f00.rf", 0x00001, 0x010000, CRC(b1dabdc9) SHA1(9d88be2a9851497c03143232d7c22da0ff297d05) )

	ROM_REGION(0x8000000, "drive", 0) // possibly a drive image???
	ROM_LOAD( "rs.06.03r", 0x0000, 0x7a80000, CRC(66132c3d) SHA1(4a73bab9518548950e11aebc6edf67f64d0d7798) )
ROM_END


void astropc_state::init_astropc()
{
}

GAME( 2002, blackbd,  0,       astropc, astropc, astropc_state, init_astropc, ROT0, "Astro", "Black Beard (Russia, set 1)", MACHINE_IS_SKELETON )
GAME( 2002, blackbda, blackbd, astropc, astropc, astropc_state, init_astropc, ROT0, "Astro", "Black Beard (Russia, set 2)", MACHINE_IS_SKELETON )
GAME( 2002, blackbdb, blackbd, astropc, astropc, astropc_state, init_astropc, ROT0, "Astro", "Black Beard (Russia, set 3)", MACHINE_IS_SKELETON )

GAME( 2002, dslayrr,  0,       astropc, astropc, astropc_state, init_astropc, ROT0, "Astro", "Dragon Slayer (Russia, set 1)", MACHINE_IS_SKELETON )
GAME( 2002, dslayrra, dslayrr, astropc, astropc, astropc_state, init_astropc, ROT0, "Astro", "Dragon Slayer (Russia, set 2)", MACHINE_IS_SKELETON )

GAME( 2002, hawaii,   0,       astropc, astropc, astropc_state, init_astropc, ROT0, "Astro", "Hawaii (Russia)", MACHINE_IS_SKELETON )

GAME( 2002, oligam,   0,       astropc, astropc, astropc_state, init_astropc, ROT0, "Astro", "Olympian Games (Russia)", MACHINE_IS_SKELETON )

GAME( 2002, rasce,    0,       astropc, astropc, astropc_state, init_astropc, ROT0, "Astro", "Ra's Scepter (Russia)", MACHINE_IS_SKELETON )
