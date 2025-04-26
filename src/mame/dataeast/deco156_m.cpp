// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/* Deco 156 analysis

 Data East chip 156 is an encrypted ARM processor.

 The 156 Chip is used on the following games:

 dataeast/deco_mlc.cpp

 Stadium Hero 96 (stadhr96*)
 Skull Fang (skullfng*)
 Dunk Dream '95 / Hoops / Hoops '96 (ddream95, hoops95, hoops96)
 Janken Game Acchi Muite Hoi! (acchi)

 dataeast/deco32.cpp

 Night Slashers (nslasher*)

 dataeast/simpl156.cpp

 Charlie Ninja (charlien)
 Magical Drop / Magical Drop Plus 1 / Chain Reaction (magdrop, magdropp, chainrec)
 Joe & Mac Returns (joemacr*)
 Party Time / Ganbare! Gonta!! 2 (prtytime, gangonta)
 Osman / Cannon Dancer (osman, candance)

 dataeast/deco156.cpp

 Heavy Smash (hvysmsh*)
 World Cup Volley '95 (wcvol95*)

 dataeast/backfire.cpp

 Backfire! (backfire*)

*/

#include "emu.h"
#include "deco156_m.h"


static void decrypt(uint32_t *src, uint32_t *dst, int length)
{
	for (int a = 0; a < length / 4; a++)
	{
		int addr = (a & 0xff0000) | 0x92c6;

		if (a & 0x0001) addr ^= 0xce4a;
		if (a & 0x0002) addr ^= 0x4db2;
		if (a & 0x0004) addr ^= 0xef60;
		if (a & 0x0008) addr ^= 0x5737;
		if (a & 0x0010) addr ^= 0x13dc;
		if (a & 0x0020) addr ^= 0x4bd9;
		if (a & 0x0040) addr ^= 0xa209;
		if (a & 0x0080) addr ^= 0xd996;
		if (a & 0x0100) addr ^= 0xa700;
		if (a & 0x0200) addr ^= 0xeca0;
		if (a & 0x0400) addr ^= 0x7529;
		if (a & 0x0800) addr ^= 0x3100;
		if (a & 0x1000) addr ^= 0x33b4;
		if (a & 0x2000) addr ^= 0x6161;
		if (a & 0x4000) addr ^= 0x1eef;
		if (a & 0x8000) addr ^= 0xf5a5;

		uint32_t dword = src[addr];

		// note that each of the following lines affects exactly two bits

		if (a & 0x00004) dword ^= 0x04400000;
		if (a & 0x00008) dword ^= 0x40000004;
		if (a & 0x00010) dword ^= 0x00048000;
		if (a & 0x00020) dword ^= 0x00000280;
		if (a & 0x00040) dword ^= 0x00200040;
		if (a & 0x00080) dword ^= 0x09000000;
		if (a & 0x00100) dword ^= 0x00001100;
		if (a & 0x00200) dword ^= 0x20002000;
		if (a & 0x00400) dword ^= 0x00000022;
		if (a & 0x00800) dword ^= 0x000a0000;
		if (a & 0x01000) dword ^= 0x10004000;
		if (a & 0x02000) dword ^= 0x00010400;
		if (a & 0x04000) dword ^= 0x80000010;
		if (a & 0x08000) dword ^= 0x00000009;
		if (a & 0x10000) dword ^= 0x02100000;
		if (a & 0x20000) dword ^= 0x00800800;

		switch (a & 3)
		{
			case 0:
				dword = bitswap<32>(dword ^ 0xec63197a,
					 1,  4,  7, 28, 22, 18, 20,  9,
					16, 10, 30,  2, 31, 24, 19, 29,
					 6, 21, 23, 11, 12, 13,  5,  0,
					 8, 26, 27, 15, 14, 17, 25,  3);
				break;

			case 1:
				dword = bitswap<32>(dword ^ 0x58a5a55f,
					14, 23, 28, 29,  6, 24, 10,  1,
					 5, 16,  7,  2, 30,  8, 18,  3,
					31, 22, 25, 20, 17,  0, 19, 27,
					 9, 12, 21, 15, 26, 13,  4, 11);
				break;

			case 2:
				dword = bitswap<32>(dword ^ 0xe3a65f16,
					19, 30, 21,  4,  2, 18, 15,  1,
					12, 25,  8,  0, 24, 20, 17, 23,
					22, 26, 28, 16,  9, 27,  6, 11,
					31, 10,  3, 13, 14,  7, 29,  5);
				break;

			case 3:
				dword = bitswap<32>(dword ^ 0x28d93783,
					30,  6, 15,  0, 31, 18, 26, 22,
					14, 23, 19, 17, 10,  8, 11, 20,
					 1, 28,  2,  4,  9, 24, 25, 27,
					 7, 21, 13, 29,  5,  3, 16, 12);
				break;
		}

		dst[a] = dword;
	}
}


void deco156_decrypt(running_machine &machine)
{
	uint32_t *rom = (uint32_t *)machine.root_device().memregion("maincpu")->base();
	int const length = machine.root_device().memregion("maincpu")->bytes();
	std::vector<uint32_t> buf(length / 4);

	memcpy(&buf[0], rom, length);
	decrypt(&buf[0], rom, length);
}
