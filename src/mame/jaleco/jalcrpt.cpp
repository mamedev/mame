// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
#include "emu.h"
#include "jalcrpt.h"

/********** DECRYPT **********/

/* 4 known types */

/* SS91022-10: desertwr, gratiaa, tp2m32, gametngk */

/* SS92046_01: bbbxing, f1superb, tetrisp, hayaosi2 */

/* SS92047-01: gratia, kirarast */

/* SS92048-01: p47aces, 47pie2, 47pie2o */


void decrypt_ms32_tx(running_machine &machine, int addr_xor,int data_xor, const char *region)
{
	int i;
	uint8_t *source_data;
	int source_size;

	source_data = machine.root_device().memregion( region )->base();
	source_size = machine.root_device().memregion( region )->bytes();

	std::vector<uint8_t> result_data(source_size);

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
	uint8_t *source_data;
	int source_size;

	source_data = machine.root_device().memregion( region )->base();
	source_size = machine.root_device().memregion( region )->bytes();

	std::vector<uint8_t> result_data(source_size);

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
