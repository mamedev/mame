/*--------------------------------------- NOTICE ------------------------------------

 this file is not a key component of MAME and the sets included here are not
 available in a normal build.

 The sets included in this driver file are intended solely to aid in the deveopment
 of the Genesis emulation in MAME.  The Megadrive / Genesis form the basis of a number
 of official Sega Arcade PCBs as well as a selection of bootlegs; the arcade titles
 however do not provide a sufficient test-bed to ensure high quality emulation.

 If changes are made to the Genesis emulation they should be regression tested against
 a full range of titles.  By including accsesible support for a range of sets such
 bug-fixing and testing becomes easier.

 Many console based arcade bootlegs are also encrypted in some way, so being able
 to reference a standard console version is useful.  Likewise the official Data East
 Arcade release of High Seas Havoc is currently unemulated due to encryption.

----------------------------------------- NOTICE ----------------------------------*/

/* 32x games */

#include "driver.h"
#include "megadriv.h"


ROM_START( 32x_bios )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "32x_g_bios.bin", 0x000000,  0x000100, CRC(5c12eae8) SHA1(dbebd76a448447cb6e524ac3cb0fd19fc065d944) )

	ROM_REGION( 0x400000, "mastersh2", 0 ) /* SH2 Code */
	ROM_LOAD( "32x_m_bios.bin", 0x000000,  0x000800, CRC(dd9c46b8) SHA1(1e5b0b2441a4979b6966d942b20cc76c413b8c5e) )

	ROM_REGION( 0x400000, "slavesh2", 0 ) /* SH2 Code */
	ROM_LOAD( "32x_s_bios.bin", 0x000000,  0x000400, CRC(bfda1fe5) SHA1(4103668c1bbd66c5e24558e73d4f3f92061a109a) )
ROM_END

ROM_START( 32x_knuk )
	ROM_REGION16_BE( 0x400000, "user1", 0 ) /* 68000 Code */
	ROM_LOAD( "32x_knuk.bin", 0x000000,  0x300000, CRC(d0b0b842) SHA1(0c2fff7bc79ed26507c08ac47464c3af19f7ced7) )

	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_COPY("user1",0,0,0x400000)
	ROM_LOAD( "32x_g_bios.bin", 0x000000,  0x000100, CRC(5c12eae8) SHA1(dbebd76a448447cb6e524ac3cb0fd19fc065d944) )

	ROM_REGION( 0x400000, "mastersh2", 0 ) /* SH2 Code */
	ROM_LOAD( "32x_m_bios.bin", 0x000000,  0x000800, CRC(dd9c46b8) SHA1(1e5b0b2441a4979b6966d942b20cc76c413b8c5e) )

	ROM_REGION( 0x400000, "slavesh2", 0 ) /* SH2 Code */
	ROM_LOAD( "32x_s_bios.bin", 0x000000,  0x000400, CRC(bfda1fe5) SHA1(4103668c1bbd66c5e24558e73d4f3f92061a109a) )
ROM_END


GAME( 1900, 32x_bios,    0,        _32x,        megadriv,    0,    ROT0,   "Unsorted", "32X Bios", GAME_NOT_WORKING|GAME_IS_BIOS_ROOT )
GAME( 1900, 32x_knuk,    32x_bios, _32x,        megadriv,    0, ROT0,   "Unsorted", "Knuckles Chaotix", GAME_NOT_WORKING )



