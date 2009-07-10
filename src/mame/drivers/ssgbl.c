#include "driver.h"
#include "megadriv.h"

/* Sunset Riders bootleg - based on Genesis / Megadrive version

 - coinage system is protected?  (by what?)

 - title raster effect is broken (bug in megadrive code, happens with normal set too)

 */

ROM_START( ssgbl )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "u1", 0x000001, 0x020000,  CRC(c59f33bd) SHA1(bd5bce7698a70ea005b79ab34bcdb056872ef980) )
	ROM_LOAD16_BYTE( "u2", 0x000000, 0x020000,  CRC(9125c054) SHA1(c73bdeb6b11c59d2b5f5968959b02697957ca894) )
	ROM_LOAD16_BYTE( "u3", 0x040001, 0x020000,  CRC(0fee0fbe) SHA1(001e0fda12707512aad537e533acf28e726e6107) )
	ROM_LOAD16_BYTE( "u4", 0x040000, 0x020000,  CRC(fc2aed41) SHA1(27eb3957f5ed26ee5276523b1df46fa7eb298e1f))
ROM_END


READ16_HANDLER( sunsetbl_r )
{
	return mame_rand(space->machine);
}

READ16_HANDLER( sunsetbl2_r )
{
	return 0x0000;//mame_rand(space->machine);
}

static DRIVER_INIT( sunsetbl )
{
	int i;
	UINT8* rom = memory_region(machine, "maincpu");

	/* todo, reduce bitswaps to single swap */
	for (i=0x00001;i<0x40000;i+=2)
	{
		rom[i] = rom[i] ^ 0xff;
		rom[i] = BITSWAP8(rom[i], 7,6,5,4,3,2,1,0 );
		rom[i] = BITSWAP8(rom[i], 1,6,5,4,3,2,7,0 );
		rom[i] = BITSWAP8(rom[i], 7,6,5,3,4,2,1,0 );
		rom[i] = BITSWAP8(rom[i], 7,6,5,2,3,4,1,0 );
		rom[i] = BITSWAP8(rom[i], 5,6,7,4,3,2,1,0 );
		rom[i] = BITSWAP8(rom[i], 7,5,6,4,3,2,1,0 );
	}

	for (i=0x40001;i<0x80000;i+=2)
	{
		rom[i] = BITSWAP8(rom[i], 7,6,5,4,3,2,1,0 );
		rom[i] = BITSWAP8(rom[i], 7,6,1,4, 3,2,5,0);
		rom[i] = BITSWAP8(rom[i], 7,6,5,4,0,2,1,3 );
		rom[i] = BITSWAP8(rom[i], 2,6,5,4,3,7,1,0 );
		rom[i] = BITSWAP8(rom[i], 7,6,5,0,3,2,1,4 );
		rom[i] = BITSWAP8(rom[i], 7,6,5,1,3,2,4,0 );

	}

	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x770070, 0x77007f, 0, 0, sunsetbl_r);
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa10006, 0xa10007, 0, 0, sunsetbl2_r); // coins??

	/* patch the startup and boot vector?! */
	rom[0x01] = 0x00;
	rom[0x00] = 0xff;
	rom[0x03] = 0xff;
	rom[0x02] = 0x00;

	rom[0x06] = 0xd2;
	rom[0x07] = 0x00;

	DRIVER_INIT_CALL(megadriv);
}

GAME( 1993, ssgbl ,  0,   megadriv,    megadriv,       sunsetbl,  ROT0,   "bootleg", "Sunset Riders (bootleg of Megadrive version)", GAME_NOT_WORKING) // playable, but coin system is broken
