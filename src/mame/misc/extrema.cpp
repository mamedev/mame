// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
  Extrema Ukrainian video fruit machines

  The games appear to be Z80 based. ROMs are scrambled.

  Maski Show has what appears to be a HDD image in 'WHX' (WinHex Backup) format
  Other games don't seem to have such images, missing.

  ROMs are strange sizes, they appear to be cut at the end of the last block of data. Some sets contain
  ROMs padded to normal sizes.

  TODO:
  driver level: everything, just a skeleton for now;
  maski: convert image to CHD;
  all games: verify if decryption is complete;

  Some notes on the encryption:
  Every game needs a base XOR. Depending on single address bits other XORs are applied.
  In all decrypted games the sum of the address bits dependent XORs is always 0xff.
  It doesn't appear the encryption utilizes data or address line bitswaps.
  All versions of the same game appear to use the same encryption.
*/

#include "emu.h"
#include "cpu/z80/z80.h"


namespace {

class extrema_state : public driver_device
{
public:
	extrema_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void extrema(machine_config &config);

	void init_adults();
	void init_bloto();
	void init_blpearl();
	void init_exsafar();
	void init_extrmth();
	void init_extrmti();
	void init_grancan();
	void init_luckshel();
	void init_maski();
	void init_strlink();

private:
	required_device<cpu_device> m_maincpu;
	void extrema_map(address_map &map) ATTR_COLD;
	void extrema_portmap(address_map &map) ATTR_COLD;
};


void extrema_state::extrema_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0xe000, 0xefff).ram();
}

void extrema_state::extrema_portmap(address_map &map)
{
	map.global_mask(0xff);
}


static INPUT_PORTS_START( extrema )
INPUT_PORTS_END


void extrema_state::extrema(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 6000000); // guessed clock
	m_maincpu->set_addrmap(AS_PROGRAM, &extrema_state::extrema_map);
	m_maincpu->set_addrmap(AS_IO, &extrema_state::extrema_portmap);
}


#define MISSING_DISK \
	DISK_REGION( "ata:0:hdd" ) \
	DISK_IMAGE( "extrema_hdd", 0, NO_DUMP )

ROM_START( maski )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "maski show_sys_v43.09.bin", 0x0000, 0x070000, CRC(d0862569) SHA1(e5eb8fcf60ed02d5c9090841e1f859d431ff1138) )
ROM_END

ROM_START( maskia )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "maski show_sys_v43.07.bin", 0x0000, 0x070000, CRC(4f4b6b05) SHA1(e1b46bf4f40e5ba92d53b9efee426a9700597537) )

	MISSING_DISK
ROM_END

ROM_START( maskib )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "maskishow_sys_v.42.11.bin", 0x0000, 0x068000, CRC(e21b817c) SHA1(2f951c37e617d3b5bc127735069d0e13f27e002a) )

	MISSING_DISK
ROM_END

ROM_START( maskic )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "maskishow_sys_v.42.05.bin", 0x0000, 0x068000, CRC(211e6e34) SHA1(e7b5b2724de2c93c1a61bbb09646204d3e33d975) )

	ROM_REGION( 0x2000000, "drive", 0 ) // HDD? (if so convert to CHD)
	ROM_LOAD( "maski show 42.05.whx", 0x0000, 0x18bd8dd, CRC(e4c6b921) SHA1(c1349a5d5b6ca457696fbeef7027e75a96f91c37) )
ROM_END

ROM_START( adults )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "aov43_23.bin", 0x0000, 0x080000, CRC(25eb34ab) SHA1(d92391809764fc2bf4336b81b1f3f2e6d9bcd811) )

	MISSING_DISK
ROM_END

ROM_START( adultsa )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "adult only_4317rus.bin", 0x0000, 0x068000, CRC(fecb5f82) SHA1(5f90391a5307edf1957432ea14fd468f33ad6a6b) )

	MISSING_DISK
ROM_END

ROM_START( bloto )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "bloto_3016.bin", 0x0000, 0x020000, CRC(05731d21) SHA1(a12816d79d465649fd3da5ad341caf67831ada65) )

	MISSING_DISK
ROM_END

ROM_START( blotoa )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "bloto_3008.bin", 0x0000, 0x020000, CRC(dcd46ab5) SHA1(b4817d6e6db6096e8fe2dd3ebad5657c3e72fe9a) )

	MISSING_DISK
ROM_END

ROM_START( blpearl )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "blpearl_sys_45.02.bin", 0x0000, 0x058000, CRC(00e9b39d) SHA1(d2bd7175fef041106b50f52ce71361ccd127ce6c) )

	MISSING_DISK
ROM_END

ROM_START( blpearla )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "black pearl_v43.07.bin", 0x0000, 0x058000, CRC(6d839184) SHA1(94586b29b35e443c120524ff91cf353c221f30ee) )

	MISSING_DISK
ROM_END

ROM_START( blpearlb )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "black pearl_v43.04.bin", 0x0000, 0x058000, CRC(e7c20ff5) SHA1(ef6b63cfe8f1f3a50942f397c5c15e0640e39942) )

	MISSING_DISK
ROM_END

ROM_START( blpearlc )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "blpearl_sys_42.03.bin", 0x0000, 0x058000, CRC(758bde12) SHA1(adfc4193edc6e0b71b16b20083a9b0635862ef8a) )

	MISSING_DISK
ROM_END

ROM_START( blpearld )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "blpearl_sys_42.01.bin", 0x0000, 0x058000, CRC(6e3b3eda) SHA1(7cda7dd7ed487c8fe80967934fe444c80c368768) )

	MISSING_DISK
ROM_END

ROM_START( grancan )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "grand canyon_sys_43.09.bin", 0x0000, 0x068000, CRC(9d27dc85) SHA1(5dce3ab40399a32b2cb7f07fa3d0cc621f3287d9) )

	MISSING_DISK
ROM_END

ROM_START( grancana )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "grand canyon_sys_43.05.bin", 0x0000, 0x068000, CRC(bdf5426b) SHA1(3f7853482c12dd96f7ab67aac8799fdbcf2757b9) )

	MISSING_DISK
ROM_END

ROM_START( grancanb )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "grcanyon_sys_42.13.bin", 0x0000, 0x068000, CRC(46f4562d) SHA1(01d3d1d3f0d1c3d5bf8f3c87da81ab5ba8356322) )

	MISSING_DISK
ROM_END

ROM_START( grancanc )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "grcanyon_sys_42.06.bin", 0x0000, 0x068000, CRC(6591e970) SHA1(69d8d8bb074b9a30593072f13e8ab6e0722e2aa3) )

	MISSING_DISK
ROM_END

ROM_START( luckshel )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "luckyshell_v.42.25.bin", 0x0000, 0x068000, CRC(0e89e575) SHA1(60af8390106db28301a24340cd4e79ea8b8bfb5c) )

	MISSING_DISK
ROM_END

ROM_START( luckshela )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "luckyshell_v.42.10.bin", 0x0000, 0x068000, CRC(dc8d3d45) SHA1(7a6659f6ed5f0d162f394ed80c39c0cff915ab41) )

	MISSING_DISK
ROM_END

ROM_START( exsafar )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "safari_sys_v43.14.bin", 0x0000, 0x070000, CRC(61def527) SHA1(4b60f8c2c5845ece23244f41d5a71d6c6093b19c) )

	MISSING_DISK
ROM_END

ROM_START( exsafara )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "safari_sys_v43.08.bin", 0x0000, 0x070000, CRC(2a35ab64) SHA1(288624f9bedc57f5229f3d88677b0f2bf141c857) )

	MISSING_DISK
ROM_END

ROM_START( exsafarb )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "safari_sys_v.42.07.bin", 0x0000, 0x070000, CRC(d4d02370) SHA1(69ca8d074fc2a7a64012cb3db6c82e91ae8ed56f) )

	MISSING_DISK
ROM_END

ROM_START( exsafarc )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "safari_sys_v.42.04.bin", 0x0000, 0x070000, CRC(c4fbcd52) SHA1(0e40194909cd9dad49a597df2f2d793c72535544) )

	MISSING_DISK
ROM_END

ROM_START( strlink )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "strong link_43.48.bin", 0x0000, 0x058000, CRC(fed4610e) SHA1(dd43765c1db9a86b46c56db6f5de4ddf7eb7b5f4) )

	MISSING_DISK
ROM_END

ROM_START( strlinka )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "stronglink_v43.45.bin", 0x0000, 0x058000, CRC(07dcc10b) SHA1(07a99d8ca4ad088177b5593fab49b4bc37a069a4) )

	MISSING_DISK
ROM_END

ROM_START( extrmth )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "trhunt_sys_34.03.bin", 0x0000, 0x048000, CRC(003639db) SHA1(65d5b530deb424fe575ed51039b7bc4b271423e6) )

	MISSING_DISK
ROM_END

ROM_START( extrmtha )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "trhunt_sys_34.02.bin", 0x0000, 0x048000, CRC(961d7011) SHA1(44c527d498061da5d84ed134c16ee7f95ed75376) )

	MISSING_DISK
ROM_END

ROM_START( extrmti )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tresure island   sys v32.49.bin", 0x0000, 0x050000, CRC(73450ee4) SHA1(6b3583461d0aed2d4d6a53d9e18392f94bd7bbd6) )

	MISSING_DISK
ROM_END


void extrema_state::init_adults()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x80000; i++)
	{
		rom[i] ^= 0x92;
		if (BIT(i, 2))  rom[i] ^= 0x09;
		if (BIT(i, 4))  rom[i] ^= 0x24;
		if (BIT(i, 5))  rom[i] ^= 0x80;
		if (BIT(i, 9))  rom[i] ^= 0x12;
		if (BIT(i, 10)) rom[i] ^= 0x40;
	}
}

void extrema_state::init_bloto()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x20000; i++)
	{
		rom[i] ^= 0x20;
		if (BIT(i, 0)) rom[i] ^= 0xc3;
		if (BIT(i, 1)) rom[i] ^= 0x20;
		if (BIT(i, 2)) rom[i] ^= 0x10;
		if (BIT(i, 4)) rom[i] ^= 0x08;
		if (BIT(i, 8)) rom[i] ^= 0x04;
	}
}

void extrema_state::init_blpearl()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x58000; i++)
	{
		rom[i] ^= 0xb5;
		if (BIT(i, 5))  rom[i] ^= 0x01;
		if (BIT(i, 6))  rom[i] ^= 0x48;
		if (BIT(i, 10)) rom[i] ^= 0x02;
		if (BIT(i, 11)) rom[i] ^= 0x90;
		if (BIT(i, 13)) rom[i] ^= 0x24;
	}
}

void extrema_state::init_exsafar()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x70000; i++)
	{
		rom[i] ^= 0x41;
		if (BIT(i, 0))  rom[i] ^= 0x06;
		if (BIT(i, 1))  rom[i] ^= 0x01;
		if (BIT(i, 2))  rom[i] ^= 0x80;
		if (BIT(i, 4))  rom[i] ^= 0x10;
		if (BIT(i, 5))  rom[i] ^= 0x40;
		if (BIT(i, 8))  rom[i] ^= 0x08;
		if (BIT(i, 10)) rom[i] ^= 0x20;
	}
}

void extrema_state::init_extrmth()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x48000; i++)
	{
		rom[i] ^= 0x57;
		if (BIT(i, 5))  rom[i] ^= 0x14;
		if (BIT(i, 7))  rom[i] ^= 0x01;
		if (BIT(i, 10)) rom[i] ^= 0x28;
		if (BIT(i, 11)) rom[i] ^= 0x02;
		if (BIT(i, 13)) rom[i] ^= 0x40;
		if (BIT(i, 14)) rom[i] ^= 0x80;
	}
}

void extrema_state::init_extrmti()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x50000; i++)
	{
		rom[i] ^= 0x2d;
		if (BIT(i, 2))  rom[i] ^= 0x10;
		if (BIT(i, 4))  rom[i] ^= 0x40;
		if (BIT(i, 5))  rom[i] ^= 0x08;
		if (BIT(i, 6))  rom[i] ^= 0x02;
		if (BIT(i, 9))  rom[i] ^= 0x20;
		if (BIT(i, 10)) rom[i] ^= 0x80;
		if (BIT(i, 11)) rom[i] ^= 0x04;
		if (BIT(i, 13)) rom[i] ^= 0x01;
	}
}

void extrema_state::init_grancan()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x68000; i++)
	{
		rom[i] ^= 0x25;
		if (BIT(i, 2))  rom[i] ^= 0x12;
		if (BIT(i, 4))  rom[i] ^= 0x48;
		if (BIT(i, 5))  rom[i] ^= 0x01;
		if (BIT(i, 9))  rom[i] ^= 0x24;
		if (BIT(i, 10)) rom[i] ^= 0x80;
	}
}

void extrema_state::init_luckshel()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x68000; i++)
	{
		rom[i] ^= 0x18;
		if (BIT(i, 0))  rom[i] ^= 0xe0;
		if (BIT(i, 1))  rom[i] ^= 0x10;
		if (BIT(i, 3))  rom[i] ^= 0x08;
		if (BIT(i, 6))  rom[i] ^= 0x04;
		if (BIT(i, 8))  rom[i] ^= 0x01;
		if (BIT(i, 12)) rom[i] ^= 0x02;
	}
}

void extrema_state::init_maski()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x70000; i++)
	{
		rom[i] ^= 0xb6;
		if (BIT(i, 5))  rom[i] ^= 0x20;
		if (BIT(i, 6))  rom[i] ^= 0x09;
		if (BIT(i, 10)) rom[i] ^= 0x40;
		if (BIT(i, 11)) rom[i] ^= 0x12;
		if (BIT(i, 13)) rom[i] ^= 0x84;
	}
}

void extrema_state::init_strlink()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x58000; i++)
	{
		rom[i] ^= 0xa2;
		if (BIT(i, 1))  rom[i] ^= 0x02;
		if (BIT(i, 2))  rom[i] ^= 0x01;
		if (BIT(i, 4))  rom[i] ^= 0x08;
		if (BIT(i, 5))  rom[i] ^= 0xa0;
		if (BIT(i, 8))  rom[i] ^= 0x04;
		if (BIT(i, 10)) rom[i] ^= 0x50;
	}
}

} // Anonymous namespace


GAME( 2003, maski,     0,        extrema, extrema, extrema_state, init_maski,    ROT0, "Extrema", "Maski Show (Ukraine V. 43.10)",                MACHINE_IS_SKELETON )
GAME( 2003, maskia,    maski,    extrema, extrema, extrema_state, init_maski,    ROT0, "Extrema", "Maski Show (Ukraine V. 43.07)",                MACHINE_IS_SKELETON )
GAME( 2003, maskib,    maski,    extrema, extrema, extrema_state, init_maski,    ROT0, "Extrema", "Maski Show (Ukraine V. 42.11)",                MACHINE_IS_SKELETON )
GAME( 2003, maskic,    maski,    extrema, extrema, extrema_state, init_maski,    ROT0, "Extrema", "Maski Show (Ukraine V. 42.05)",                MACHINE_IS_SKELETON )
GAME( 2003, adults,    0,        extrema, extrema, extrema_state, init_adults,   ROT0, "Extrema", "Adults Only (Ukraine, V. 43.23)",              MACHINE_IS_SKELETON )
GAME( 2003, adultsa,   adults,   extrema, extrema, extrema_state, init_adults,   ROT0, "Extrema", "Adults Only (Ukraine, V. 43.17)",              MACHINE_IS_SKELETON )
GAME( 200?, bloto,     0,        extrema, extrema, extrema_state, init_bloto,    ROT0, "Extrema", "Blits Loto (Ukraine, V. 30.16)",               MACHINE_IS_SKELETON )
GAME( 200?, blotoa,    bloto,    extrema, extrema, extrema_state, init_bloto,    ROT0, "Extrema", "Blits Loto (Ukraine, V. 30.08)",               MACHINE_IS_SKELETON )
GAME( 2003, blpearl,   0,        extrema, extrema, extrema_state, init_blpearl,  ROT0, "Extrema", "Black Pearl (Ukraine, V. 45.02)",              MACHINE_IS_SKELETON )
GAME( 2003, blpearla,  blpearl,  extrema, extrema, extrema_state, init_blpearl,  ROT0, "Extrema", "Black Pearl (Ukraine, V. 43.07)",              MACHINE_IS_SKELETON )
GAME( 2003, blpearlb,  blpearl,  extrema, extrema, extrema_state, init_blpearl,  ROT0, "Extrema", "Black Pearl (Ukraine, V. 43.04)",              MACHINE_IS_SKELETON )
GAME( 2003, blpearlc,  blpearl,  extrema, extrema, extrema_state, init_blpearl,  ROT0, "Extrema", "Black Pearl (Ukraine, V. 42.03)",              MACHINE_IS_SKELETON )
GAME( 2003, blpearld,  blpearl,  extrema, extrema, extrema_state, init_blpearl,  ROT0, "Extrema", "Black Pearl (Ukraine, V. 42.01)",              MACHINE_IS_SKELETON )
GAME( 2003, grancan,   0,        extrema, extrema, extrema_state, init_grancan,  ROT0, "Extrema", "Grand Canyon (Ukraine, V. 43.09)",             MACHINE_IS_SKELETON )
GAME( 2003, grancana,  grancan,  extrema, extrema, extrema_state, init_grancan,  ROT0, "Extrema", "Grand Canyon (Ukraine, V. 43.05)",             MACHINE_IS_SKELETON )
GAME( 2003, grancanb,  grancan,  extrema, extrema, extrema_state, init_grancan,  ROT0, "Extrema", "Grand Canyon (Ukraine, V. 42.13)",             MACHINE_IS_SKELETON )
GAME( 2003, grancanc,  grancan,  extrema, extrema, extrema_state, init_grancan,  ROT0, "Extrema", "Grand Canyon (Ukraine, V. 42.06)",             MACHINE_IS_SKELETON )
GAME( 2003, luckshel,  0,        extrema, extrema, extrema_state, init_luckshel, ROT0, "Extrema", "Lucky Shell (Ukraine, V. 42.25)",              MACHINE_IS_SKELETON )
GAME( 2003, luckshela, luckshel, extrema, extrema, extrema_state, init_luckshel, ROT0, "Extrema", "Lucky Shell (Ukraine, V. 42.10)",              MACHINE_IS_SKELETON )
GAME( 2003, exsafar,   0,        extrema, extrema, extrema_state, init_exsafar,  ROT0, "Extrema", "Safari (Extrema, Ukraine, V. 43.14)",          MACHINE_IS_SKELETON )
GAME( 2003, exsafara,  exsafar,  extrema, extrema, extrema_state, init_exsafar,  ROT0, "Extrema", "Safari (Extrema, Ukraine, V. 43.08)",          MACHINE_IS_SKELETON )
GAME( 2003, exsafarb,  exsafar,  extrema, extrema, extrema_state, init_exsafar,  ROT0, "Extrema", "Safari (Extrema, Ukraine, V. 42.07)",          MACHINE_IS_SKELETON )
GAME( 2003, exsafarc,  exsafar,  extrema, extrema, extrema_state, init_exsafar,  ROT0, "Extrema", "Safari (Extrema, Ukraine, V. 42.04)",          MACHINE_IS_SKELETON )
GAME( 2004, strlink,   0,        extrema, extrema, extrema_state, init_strlink,  ROT0, "Extrema", "Strong Link (Ukraine, V. 43.48)",              MACHINE_IS_SKELETON )
GAME( 2004, strlinka,  strlink,  extrema, extrema, extrema_state, init_strlink,  ROT0, "Extrema", "Strong Link (Ukraine, V. 43.45)",              MACHINE_IS_SKELETON )
GAME( 200?, extrmth,   0,        extrema, extrema, extrema_state, init_extrmth,  ROT0, "Extrema", "Treasure Hunt (Extrema, Ukraine, V. 34.03)",   MACHINE_IS_SKELETON ) // other string has it as 3.4.03
GAME( 200?, extrmtha,  extrmth,  extrema, extrema, extrema_state, init_extrmth,  ROT0, "Extrema", "Treasure Hunt (Extrema, Ukraine, V. 34.02)",   MACHINE_IS_SKELETON ) // other string has it as 3.4.02
GAME( 200?, extrmti,   0,        extrema, extrema, extrema_state, init_extrmti,  ROT0, "Extrema", "Treasure Island (Extrema, Ukraine, V. 32.49)", MACHINE_IS_SKELETON ) // other string has it as 3.2.49
