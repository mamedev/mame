// license:BSD-3-Clause
// copyright-holders:David Haywood
/****************************************

Libble Rabble (c) 1983 Namco
Toypop        (c) 1986 Namco

Notes:
------
- Libble Rabble Easter egg:
  - enter service mode
  - turn off the service mode switch, and turn it on again quickly to remain
    on the monitor test grid
  - Enter the following sequence using the right joystick:
    9xU 2xR 9xD 2xL
  (c) 1983 NAMCO LTD. will appear on the screen.

****************************************/

#include "emu.h"



class toypop_state : public driver_device
{
public:
	toypop_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }
};


static INPUT_PORTS_START( liblrabl )
INPUT_PORTS_END

static INPUT_PORTS_START( toypop )
INPUT_PORTS_END


static MACHINE_CONFIG_START( liblrabl, toypop_state )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( toypop, liblrabl )
MACHINE_CONFIG_END


ROM_START( liblrabl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5b.rom",   0x8000, 0x4000, CRC(da7a93c2) SHA1(fe4a02cdab66722eb7b8cf58825f899b1949a6a2) )
	ROM_LOAD( "5c.rom",   0xc000, 0x4000, CRC(6cae25dc) SHA1(de74317a7d5de1865d096c377923a764be5e6879) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "2c.rom",   0xe000, 0x2000, CRC(7c09e50a) SHA1(5f004d60bbb7355e008a9cda137b28bc2192b8ef) )

	ROM_REGION( 0x8000, "sub", 0 )
	ROM_LOAD16_BYTE( "8c.rom",   0x0000, 0x4000, CRC(a00cd959) SHA1(cc5621103c31cfbc65941615cab391db0f74e6ce) )
	ROM_LOAD16_BYTE("10c.rom",   0x0001, 0x4000, CRC(09ce209b) SHA1(2ed46d6592f8227bac8ab54963d9a300706ade47) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5p.rom",   0x0000, 0x2000, CRC(3b4937f0) SHA1(06d9de576f1c2262c34aeb91054e68c9298af688) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "9t.rom",   0x0000, 0x4000, CRC(a88e24ca) SHA1(eada133579f19de09255084dcdc386311606a335) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "lr1-3.1r", 0x0000, 0x0100, CRC(f3ec0d07) SHA1(b0aad1fb6df79f202889600f486853995352f9c2) )
	ROM_LOAD( "lr1-2.1s", 0x0100, 0x0100, CRC(2ae4f702) SHA1(838fdca9e91fea4f64a59880ac47c48973bb8fbf) )
	ROM_LOAD( "lr1-1.1t", 0x0200, 0x0100, CRC(7601f208) SHA1(572d070ca387b780030ed5de38a8970b7cc14349) )
	ROM_LOAD( "lr1-5.5l", 0x0300, 0x0100, CRC(940f5397) SHA1(825a7bd78a8a08d30bad2e4890ae6e9ad88b36b8) )
	ROM_LOAD( "lr1-6.2p", 0x0400, 0x0200, CRC(a6b7f850) SHA1(7cfde16dfd5c4d5b876b4fbe4f924f1385932a93) )

	ROM_REGION( 0x0100, "namco", 0 )
	ROM_LOAD( "lr1-4.3d", 0x0000, 0x0100, CRC(16a9166a) SHA1(847cbaf7c88616576c410177e066ae1d792ac0ba) )
ROM_END

ROM_START( toypop )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tp1-2.5b", 0x8000, 0x4000, CRC(87469620) SHA1(2ee257486c9c044386ac7d0cd4a90583eaeb3e97) )
	ROM_LOAD( "tp1-1.5c", 0xc000, 0x4000, CRC(dee2fd6e) SHA1(b2c12008d6d3e7544ba3c12a52a6abf9181842c8) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "tp1-3.2c", 0xe000, 0x2000, CRC(5f3bf6e2) SHA1(d1b3335661b9b23cb10001416c515b77b5e783e9) )

	ROM_REGION( 0x8000, "sub", 0 )
	ROM_LOAD16_BYTE( "tp1-4.8c", 0x0000, 0x4000, CRC(76997db3) SHA1(5023a2f20a5f2c9baff130f6832583493c71f883) )
	ROM_LOAD16_BYTE("tp1-5.10c", 0x0001, 0x4000, CRC(37de8786) SHA1(710365e34c05d01815844c414518f93234b6160b) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tp1-7.5p", 0x0000, 0x2000, CRC(95076f9e) SHA1(1e3d32b21f6d46591ec3921aba51f672d64a9023) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "tp1-6.9t", 0x0000, 0x4000, CRC(481ffeaf) SHA1(c51735ad3a1dbb46ad414408b54554e9223b2219) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "tp1-3.1r", 0x0000, 0x0100, CRC(cfce2fa5) SHA1(b42aa0f34d885389d2650bf7a0531b95703b8a28) )
	ROM_LOAD( "tp1-2.1s", 0x0100, 0x0100, CRC(aeaf039d) SHA1(574560526100d38635aecd71eb73499c4f57d586) )
	ROM_LOAD( "tp1-1.1t", 0x0200, 0x0100, CRC(08e7cde3) SHA1(5261aca6834d635d17f8afaa8e35848930030ba4) )
	ROM_LOAD( "tp1-4.5l", 0x0300, 0x0100, CRC(74138973) SHA1(2e21dbb1b19dd089da52e70fcb0ca91336e004e6) )
	ROM_LOAD( "tp1-5.2p", 0x0400, 0x0200, CRC(4d77fa5a) SHA1(2438910314b23ecafb553230244f3931861ad2da) )

	ROM_REGION( 0x0100, "namco", 0 )
	ROM_LOAD( "tp1-6.3d", 0x0000, 0x0100, CRC(16a9166a) SHA1(847cbaf7c88616576c410177e066ae1d792ac0ba) )
ROM_END

GAME( 1983, liblrabl, 0,     liblrabl, liblrabl, driver_device, 0,   ROT0,   "Namco", "Libble Rabble", MACHINE_IS_SKELETON )
GAME( 1986, toypop,   0,     toypop,   toypop, driver_device,   0,   ROT0,   "Namco", "Toypop", MACHINE_IS_SKELETON )
