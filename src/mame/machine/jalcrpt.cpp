// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
#include "emu.h"
#include "machine/jalcrpt.h"


void phantasm_rom_decode(running_machine &machine, const char *region)
{
	UINT16  *RAM    =   (UINT16 *) machine.root_device().memregion(region)->base();
	int i,      size    =   machine.root_device().memregion(region)->bytes();
	if (size > 0x40000) size = 0x40000;

	for (i = 0 ; i < size/2 ; i++)
	{
		UINT16 x,y;

		x = RAM[i];

// [0] def0 189a bc56 7234
// [1] fdb9 7531 eca8 6420
// [2] 0123 4567 ba98 fedc
#define BITSWAP_0   BITSWAP16(x,0xd,0xe,0xf,0x0,0x1,0x8,0x9,0xa,0xb,0xc,0x5,0x6,0x7,0x2,0x3,0x4)
#define BITSWAP_1   BITSWAP16(x,0xf,0xd,0xb,0x9,0x7,0x5,0x3,0x1,0xe,0xc,0xa,0x8,0x6,0x4,0x2,0x0)
#define BITSWAP_2   BITSWAP16(x,0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0xb,0xa,0x9,0x8,0xf,0xe,0xd,0xc)

		if      (i < 0x08000/2) { if ( (i | (0x248/2)) != i ) {y = BITSWAP_0;} else {y = BITSWAP_1;} }
		else if (i < 0x10000/2) { y = BITSWAP_2; }
		else if (i < 0x18000/2) { if ( (i | (0x248/2)) != i ) {y = BITSWAP_0;} else {y = BITSWAP_1;} }
		else if (i < 0x20000/2) { y = BITSWAP_1; }
		else                    { y = BITSWAP_2; }

#undef  BITSWAP_0
#undef  BITSWAP_1
#undef  BITSWAP_2

		RAM[i] = y;
	}

}

void astyanax_rom_decode(running_machine &machine, const char *region)
{
	UINT16  *RAM    =   (UINT16 *) machine.root_device().memregion(region)->base();
	int i,      size    =   machine.root_device().memregion(region)->bytes();
	if (size > 0x40000) size = 0x40000;

	for (i = 0 ; i < size/2 ; i++)
	{
		UINT16 x,y;

		x = RAM[i];

// [0] def0 a981 65cb 7234
// [1] fdb9 7531 8ace 0246
// [2] 4567 0123 ba98 fedc

#define BITSWAP_0   BITSWAP16(x,0xd,0xe,0xf,0x0,0xa,0x9,0x8,0x1,0x6,0x5,0xc,0xb,0x7,0x2,0x3,0x4)
#define BITSWAP_1   BITSWAP16(x,0xf,0xd,0xb,0x9,0x7,0x5,0x3,0x1,0x8,0xa,0xc,0xe,0x0,0x2,0x4,0x6)
#define BITSWAP_2   BITSWAP16(x,0x4,0x5,0x6,0x7,0x0,0x1,0x2,0x3,0xb,0xa,0x9,0x8,0xf,0xe,0xd,0xc)

		if      (i < 0x08000/2) { if ( (i | (0x248/2)) != i ) {y = BITSWAP_0;} else {y = BITSWAP_1;} }
		else if (i < 0x10000/2) { y = BITSWAP_2; }
		else if (i < 0x18000/2) { if ( (i | (0x248/2)) != i ) {y = BITSWAP_0;} else {y = BITSWAP_1;} }
		else if (i < 0x20000/2) { y = BITSWAP_1; }
		else                    { y = BITSWAP_2; }

#undef  BITSWAP_0
#undef  BITSWAP_1
#undef  BITSWAP_2

		RAM[i] = y;
	}
}

void rodland_rom_decode(running_machine &machine, const char *region)
{
	UINT16  *RAM    =   (UINT16 *) machine.root_device().memregion(region)->base();
	int i,      size    =   machine.root_device().memregion(region)->bytes();
	if (size > 0x40000) size = 0x40000;

	for (i = 0 ; i < size/2 ; i++)
	{
		UINT16 x,y;

		x = RAM[i];

// [0] d0a9 6ebf 5c72 3814  [1] 4567 0123 ba98 fedc
// [2] fdb9 ce07 5318 a246  [3] 4512 ed3b a967 08fc
#define BITSWAP_0   BITSWAP16(x,0xd,0x0,0xa,0x9,0x6,0xe,0xb,0xf,0x5,0xc,0x7,0x2,0x3,0x8,0x1,0x4);
#define BITSWAP_1   BITSWAP16(x,0x4,0x5,0x6,0x7,0x0,0x1,0x2,0x3,0xb,0xa,0x9,0x8,0xf,0xe,0xd,0xc);
#define BITSWAP_2   BITSWAP16(x,0xf,0xd,0xb,0x9,0xc,0xe,0x0,0x7,0x5,0x3,0x1,0x8,0xa,0x2,0x4,0x6);
#define BITSWAP_3   BITSWAP16(x,0x4,0x5,0x1,0x2,0xe,0xd,0x3,0xb,0xa,0x9,0x6,0x7,0x0,0x8,0xf,0xc);

		if      (i < 0x08000/2) {   if ( (i | (0x248/2)) != i ) {y = BITSWAP_0;} else {y = BITSWAP_1;} }
		else if (i < 0x10000/2) {   if ( (i | (0x248/2)) != i ) {y = BITSWAP_2;} else {y = BITSWAP_3;} }
		else if (i < 0x18000/2) {   if ( (i | (0x248/2)) != i ) {y = BITSWAP_0;} else {y = BITSWAP_1;} }
		else if (i < 0x20000/2) { y = BITSWAP_1; }
		else                    { y = BITSWAP_3; }

#undef  BITSWAP_0
#undef  BITSWAP_1
#undef  BITSWAP_2
#undef  BITSWAP_3

		RAM[i] = y;
	}
}

/********** DECRYPT **********/

/* 4 known types */

/* SS91022-10: desertwr, gratiaa, tp2m32, gametngk */

/* SS92046_01: bbbxing, f1superb, tetrisp, hayaosi2 */

/* SS92047-01: gratia, kirarast */

/* SS92048-01: p47aces, 47pie2, 47pie2o */

void ms32_rearrange_sprites(running_machine &machine, const char *region)
{
	/* sprites are not encrypted, but we need to move the data around to handle them as 256x256 tiles */
	int i;
	UINT8 *source_data;
	int source_size;

	source_data = machine.root_device().memregion       ( region )->base();
	source_size = machine.root_device().memregion( region )->bytes();

	dynamic_buffer result_data(source_size);

	for(i=0; i<source_size; i++)
	{
		int j = (i & ~0x07f8) | ((i & 0x00f8) << 3) | ((i & 0x0700) >> 5);

		result_data[i] = source_data[j];
	}

	memcpy (source_data, &result_data[0], source_size);
}


void decrypt_ms32_tx(running_machine &machine, int addr_xor,int data_xor, const char *region)
{
	int i;
	UINT8 *source_data;
	int source_size;

	source_data = machine.root_device().memregion       ( region )->base();
	source_size = machine.root_device().memregion( region )->bytes();

	dynamic_buffer result_data(source_size);

	addr_xor ^= 0x1005d;

	for(i=0; i<source_size; i++)
	{
		int j;

		/* two groups of cascading XORs for the address */
		j = 0;
		i ^= addr_xor;

		if (BIT(i,18)) j ^= 0x40000;    // 18
		if (BIT(i,17)) j ^= 0x60000;    // 17
		if (BIT(i, 7)) j ^= 0x70000;    // 16
		if (BIT(i, 3)) j ^= 0x78000;    // 15
		if (BIT(i,14)) j ^= 0x7c000;    // 14
		if (BIT(i,13)) j ^= 0x7e000;    // 13
		if (BIT(i, 0)) j ^= 0x7f000;    // 12
		if (BIT(i,11)) j ^= 0x7f800;    // 11
		if (BIT(i,10)) j ^= 0x7fc00;    // 10

		if (BIT(i, 9)) j ^= 0x00200;    //  9
		if (BIT(i, 8)) j ^= 0x00300;    //  8
		if (BIT(i,16)) j ^= 0x00380;    //  7
		if (BIT(i, 6)) j ^= 0x003c0;    //  6
		if (BIT(i,12)) j ^= 0x003e0;    //  5
		if (BIT(i, 4)) j ^= 0x003f0;    //  4
		if (BIT(i,15)) j ^= 0x003f8;    //  3
		if (BIT(i, 2)) j ^= 0x003fc;    //  2
		if (BIT(i, 1)) j ^= 0x003fe;    //  1
		if (BIT(i, 5)) j ^= 0x003ff;    //  0

		i ^= addr_xor;

		/* simple XOR for the data */
		result_data[i] = source_data[j] ^ (i & 0xff) ^ data_xor;
	}

	memcpy (source_data, &result_data[0], source_size);
}

void decrypt_ms32_bg(running_machine &machine, int addr_xor,int data_xor, const char *region)
{
	int i;
	UINT8 *source_data;
	int source_size;

	source_data = machine.root_device().memregion       ( region )->base();
	source_size = machine.root_device().memregion( region )->bytes();

	dynamic_buffer result_data(source_size);

	addr_xor ^= 0xc1c5b;

	for(i=0; i<source_size; i++)
	{
		int j;

		/* two groups of cascading XORs for the address */
		j = (i & ~0xfffff); /* top bits are not affected */
		i ^= addr_xor;

		if (BIT(i,19)) j ^= 0x80000;    // 19
		if (BIT(i, 8)) j ^= 0xc0000;    // 18
		if (BIT(i,17)) j ^= 0xe0000;    // 17
		if (BIT(i, 2)) j ^= 0xf0000;    // 16
		if (BIT(i,15)) j ^= 0xf8000;    // 15
		if (BIT(i,14)) j ^= 0xfc000;    // 14
		if (BIT(i,13)) j ^= 0xfe000;    // 13
		if (BIT(i,12)) j ^= 0xff000;    // 12
		if (BIT(i, 1)) j ^= 0xff800;    // 11
		if (BIT(i,10)) j ^= 0xffc00;    // 10

		if (BIT(i, 9)) j ^= 0x00200;    //  9
		if (BIT(i, 3)) j ^= 0x00300;    //  8
		if (BIT(i, 7)) j ^= 0x00380;    //  7
		if (BIT(i, 6)) j ^= 0x003c0;    //  6
		if (BIT(i, 5)) j ^= 0x003e0;    //  5
		if (BIT(i, 4)) j ^= 0x003f0;    //  4
		if (BIT(i,18)) j ^= 0x003f8;    //  3
		if (BIT(i,16)) j ^= 0x003fc;    //  2
		if (BIT(i,11)) j ^= 0x003fe;    //  1
		if (BIT(i, 0)) j ^= 0x003ff;    //  0

		i ^= addr_xor;

		/* simple XOR for the data */
		result_data[i] = source_data[j] ^ (i & 0xff) ^ data_xor;
	}

	memcpy (source_data, &result_data[0], source_size);
}
