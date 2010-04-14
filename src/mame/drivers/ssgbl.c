#include "emu.h"
#include "includes/megadriv.h"

/* Sunset Riders bootleg - based on Genesis / Megadrive version

 - title raster effect is broken (bug in megadrive code, happens with normal set too)

 */

ROM_START( ssgbl )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "u1", 0x000001, 0x020000,  CRC(c59f33bd) SHA1(bd5bce7698a70ea005b79ab34bcdb056872ef980) )
	ROM_LOAD16_BYTE( "u2", 0x000000, 0x020000,  CRC(9125c054) SHA1(c73bdeb6b11c59d2b5f5968959b02697957ca894) )
	ROM_LOAD16_BYTE( "u3", 0x040001, 0x020000,  CRC(0fee0fbe) SHA1(001e0fda12707512aad537e533acf28e726e6107) )
	ROM_LOAD16_BYTE( "u4", 0x040000, 0x020000,  CRC(fc2aed41) SHA1(27eb3957f5ed26ee5276523b1df46fa7eb298e1f))
ROM_END


static READ16_HANDLER( ssgbl_dsw_r )
{
	static const char *const dswname[3] = { "DSWA", "DSWB", "DSWC" };
	return input_port_read(space->machine, dswname[offset]);
}

static DRIVER_INIT( ssgbl )
{
	int x;
	UINT8* rom = memory_region(machine, "maincpu");

	/* todo, reduce bitswaps to single swap */
	for (x=0x00001;x<0x40000;x+=2)
	{
		rom[x] = rom[x] ^ 0xff;
		rom[x] = BITSWAP8(rom[x], 7,6,5,4,3,2,1,0 );
		rom[x] = BITSWAP8(rom[x], 1,6,5,4,3,2,7,0 );
		rom[x] = BITSWAP8(rom[x], 7,6,5,3,4,2,1,0 );
		rom[x] = BITSWAP8(rom[x], 7,6,5,2,3,4,1,0 );
		rom[x] = BITSWAP8(rom[x], 5,6,7,4,3,2,1,0 );
		rom[x] = BITSWAP8(rom[x], 7,5,6,4,3,2,1,0 );
	}

	for (x=0x40001;x<0x80000;x+=2)
	{
		rom[x] = BITSWAP8(rom[x], 7,6,5,4,3,2,1,0 );
		rom[x] = BITSWAP8(rom[x], 7,6,1,4,3,2,5,0 );
		rom[x] = BITSWAP8(rom[x], 7,6,5,4,0,2,1,3 );
		rom[x] = BITSWAP8(rom[x], 2,6,5,4,3,7,1,0 );
		rom[x] = BITSWAP8(rom[x], 7,6,5,0,3,2,1,4 );
		rom[x] = BITSWAP8(rom[x], 7,6,5,1,3,2,4,0 );

	}

	// boot vectors don't seem to be valid, so they are patched...
	rom[0x01] = 0x01;
	rom[0x00] = 0x00;
	rom[0x03] = 0x00;
	rom[0x02] = 0x00;

	rom[0x06] = 0xd2;
	rom[0x07] = 0x00;

	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x770070, 0x770075, 0, 0, ssgbl_dsw_r );

	DRIVER_INIT_CALL(megadriv);
}

GAME( 1993, ssgbl,    0, megadriv, ssgbl,    ssgbl,    ROT0, "bootleg", "Sunset Riders (bootleg of Megadrive version)", 0)
