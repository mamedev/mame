/*

 Data East Custom Chip 102 decryption
 (encrypted 68000)

  by Nicola Salmoria

*/

#include "driver.h"
#include "cpu/m68000/m68000.h"

extern void deco102_decrypt(int region, int address_xor, int data_select_xor, int opcode_select_xor);

static UINT16 decrypt(UINT16 data, int address, int select_xor)
{
	static UINT16 xors[16] =
	{
		0xb52c,0x2458,0x139a,0xc998,0xce8e,0x5144,0x0429,0xaad4,0xa331,0x3645,0x69a3,0xac64,0x1a53,0x5083,0x4dea,0xd237
	};
	static UINT8 bitswaps[16][16] =
	{
		{ 12,8,13,11,14,10,15,9, 3,2,1,0,4,5,6,7 }, { 10,11,14,12,15,13,8,9, 6,7,5,3,0,4,2,1 },
		{ 14,13,15,9,8,12,11,10, 7,4,1,5,6,0,3,2 }, { 15,14,8,9,10,11,13,12, 1,2,7,3,4,6,0,5 },
		{ 10,9,13,14,15,8,12,11, 5,2,1,0,3,4,7,6 }, { 8,9,15,14,10,11,13,12, 0,6,5,4,1,2,3,7 },
		{ 14,8,15,9,10,11,13,12, 4,5,3,0,2,7,6,1 }, { 13,11,12,10,15,9,14,8, 6,0,7,5,1,4,3,2 },
		{ 12,11,13,10,9,8,14,15, 0,2,4,6,7,5,3,1 }, { 15,13,9,8,10,11,12,14, 2,1,0,7,6,5,4,3 },
		{ 13,8,9,10,11,12,15,14, 6,0,1,2,3,7,4,5 }, { 12,11,10,8,9,13,14,15, 6,5,4,0,7,1,2,3 },
		{ 12,15,8,13,9,11,14,10, 6,5,4,3,2,1,0,7 }, { 11,12,13,14,15,8,9,10, 4,5,7,1,6,3,2,0 },
		{ 13,8,12,14,11,15,10,9, 7,6,5,4,3,2,1,0 }, { 15,14,13,12,11,10,9,8, 0,6,7,4,3,2,1,5 }
	};
	int j, xor;
	UINT8 *bs;

	// calculate bitswap to use
	j = ((address ^ select_xor) & 0xf0) >> 4;
	if (address & 0x20000) j ^= 4;
	bs = bitswaps[j];

	// calculate xor to use
	j = (address ^ select_xor) & 0x0f;
	if (address & 0x40000) j ^= 2;	// boogwing
	xor = xors[j];

	// decrypt
	return xor ^ BITSWAP16(data,
				bs[0],bs[1],bs[2],bs[3],bs[4],bs[5],bs[6],bs[7],
				bs[8],bs[9],bs[10],bs[11],bs[12],bs[13],bs[14],bs[15]);
}

void deco102_decrypt(int region, int address_xor, int data_select_xor, int opcode_select_xor)
{
	int i;
	UINT16 *rom = (UINT16 *)memory_region(region);
	int size = memory_region_length(region);
	UINT16 *opcodes = auto_malloc(size);
	UINT16 *buf = malloc_or_die(size);

		memcpy(buf,rom,size);

		memory_set_decrypted_region(0, 0, size - 1, opcodes);
		m68k_set_encrypted_opcode_range(0,0,size);

		for (i = 0;i < size/2;i++)
		{
			int src;

			// calculate address of encrypted word in ROM
			src = i & 0xf0000;
			if (i & 0x0001) src ^= 0xbe0b;
			if (i & 0x0002) src ^= 0x5699;
			if (i & 0x0004) src ^= 0x1322;
			if (i & 0x0008) src ^= 0x0004;
			if (i & 0x0010) src ^= 0x08a0;
			if (i & 0x0020) src ^= 0x0089;
			if (i & 0x0040) src ^= 0x0408;
			if (i & 0x0080) src ^= 0x1212;
			if (i & 0x0100) src ^= 0x08e0;
			if (i & 0x0200) src ^= 0x5499;
			if (i & 0x0400) src ^= 0x9a8b;
			if (i & 0x0800) src ^= 0x1222;
			if (i & 0x1000) src ^= 0x1200;
			if (i & 0x2000) src ^= 0x0008;
			if (i & 0x4000) src ^= 0x1210;
			if (i & 0x8000) src ^= 0x00e0;
			src ^= address_xor;

			rom[i]     = decrypt(buf[src], i, data_select_xor);
			opcodes[i] = decrypt(buf[src], i, opcode_select_xor);
		}

		free(buf);
}
