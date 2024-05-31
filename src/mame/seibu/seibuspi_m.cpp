// license:BSD-3-Clause
// copyright-holders:Ville Linde, Nicola Salmoria

#include "emu.h"
#include "seibuspi_m.h"

// add two numbers generating carry from one bit to the next only if
// the corresponding bit in carry_mask is 1
static u32 partial_carry_sum(u32 add1,u32 add2,u32 carry_mask,int bits)
{
	int res = 0;
	int carry = 0;
	for (int i = 0; i < bits; i++)
	{
		const int bit = BIT(add1, i) + BIT(add2, i) + carry;

		res += (bit & 1) << i;

		// generate carry only if the corresponding bit in carry_mask is 1
		if (BIT(carry_mask, i))
			carry = bit >> 1;
		else
			carry = 0;
	}

	// wrap around carry from top bit to bit 0
	if (carry)
		res ^= 1;

	return res;
}

u32 partial_carry_sum32(u32 add1, u32 add2, u32 carry_mask)
{
	return partial_carry_sum(add1, add2, carry_mask, 32);
}

u32 partial_carry_sum24(u32 add1, u32 add2, u32 carry_mask)
{
	return partial_carry_sum(add1, add2, carry_mask, 24);
}

static u32 partial_carry_sum16(u32 add1, u32 add2, u32 carry_mask)
{
	return partial_carry_sum(add1, add2, carry_mask, 16);
}



#if 0
key tables: table[n] is the column of bit n
cpu #0 (PC=0033C2F5): unmapped program memory dword write to 00000530 = 0000xxxx & FFFFFFFF (256 times, see key_table[])

cpu #0 (PC=0033C315): unmapped program memory dword write to 00000524 = 843A0000 & FFFFFFFF
cpu #0 (PC=0033C315): unmapped program memory dword write to 00000524 = 3A590000 & FFFFFFFF
cpu #0 (PC=0033C315): unmapped program memory dword write to 00000524 = C8E20000 & FFFFFFFF
cpu #0 (PC=0033C315): unmapped program memory dword write to 00000524 = 28D40000 & FFFFFFFF
cpu #0 (PC=0033C315): unmapped program memory dword write to 00000524 = 9F840000 & FFFFFFFF
cpu #0 (PC=0033C315): unmapped program memory dword write to 00000524 = 9CAC0000 & FFFFFFFF
#endif


/*
This is the main key as written by the game.
There are 256 values; the one to use is key_table[(address >> 8) & 0xff].

Bits 0-3 select the permutation on 16 bits of the source data.
Bits 4-14 are added to the source data, with partial carry.
Bit 15 is still unknown.
*/
static const u16 key_table[256]=
{
	0x3ad7,0x54b1,0x2d41,0x8ca0,0xa69b,0x9018,0x9db9,0x6559,
	0xe9a7,0xb087,0x8a5e,0x821c,0xaafc,0x2ae7,0x557b,0xcd80,
	0xcfee,0x653e,0x9b31,0x7ab5,0x8b2a,0xbda8,0x707a,0x3c83,
	0xcbb7,0x7157,0x8226,0x5c4a,0x8bf2,0x6397,0x13e2,0x3102,
	0x8093,0x44cd,0x5f2d,0x7639,0xa7a4,0x9974,0x5263,0x8318,
	0xb78c,0xa120,0xafb4,0x615f,0x6e0b,0x1d7d,0x8c29,0x4466,
	0x3f35,0x794e,0xaea6,0x601c,0xe478,0xcf6e,0x4ee3,0xa009,
	0x4b99,0x51d3,0x3474,0x3e4d,0xe5b7,0x9088,0xb5c0,0xba9f,
	0x5646,0xa0af,0x970b,0xb14f,0x8216,0x2386,0x496d,0x9245,
	0x7e4c,0xad5f,0x89d9,0xb801,0xdf64,0x8ca8,0xe019,0xde9b,
	0x6836,0x70e2,0x7dcd,0x7ac1,0x98ef,0x71aa,0x7d6f,0x70bd,
	0x9e14,0x75b6,0x8153,0xab6c,0x1f85,0x79cd,0xb2a1,0x934a,
	0x6f74,0x37d7,0xa05a,0x6563,0x1972,0x2dcd,0x7e59,0x6a60,
	0x5163,0x84c4,0xc451,0x8d80,0x4287,0x57e8,0xacc9,0x539d,
	0xbe71,0xdb7c,0x9424,0xb224,0xcc0f,0xe3dd,0xb79c,0x461e,
	0x96a9,0x4c7c,0x5443,0x6b2b,0x3cdc,0xbee8,0x2602,0x3282,
	0x7f9c,0x59c3,0xc69a,0x39f4,0x5138,0xb7ca,0x6ca7,0x62e7,
	0xc455,0x56cf,0x8a9a,0x695c,0x5af2,0xdebf,0x4dbb,0xdaec,
	0xb564,0xc89c,0x7d2d,0x6dc3,0xa15a,0x6584,0xb8ea,0xb7ac,
	0x88d8,0xc5aa,0x98c5,0xc506,0xc13c,0x7f59,0xab65,0x8fc8,
	0x3a3c,0xd5f6,0x554d,0x5682,0x8ce7,0x40fc,0x8fd7,0x535c,
	0x6aa0,0x52fe,0x8834,0x5316,0x6c27,0x80a9,0x9e6f,0x2c08,
	0x4092,0xc7c1,0xc468,0x9520,0xbc4d,0xb621,0x3cdb,0xdce8,
	0x481f,0xd0bd,0x3a57,0x807e,0x3025,0x5aa0,0x5e49,0xa29b,
	0xd2d6,0x7bee,0x97f0,0xe28e,0x2fff,0x48e4,0x6367,0x933f,
	0x57c5,0x28d4,0x68a0,0xd22e,0x39a6,0x9d2b,0x7a64,0x7e72,
	0x5379,0xe86c,0x7554,0x8fbb,0xc06a,0x9533,0x7eec,0x4d52,
	0xa800,0x5d35,0xa47d,0xe515,0x8d19,0x703b,0x5a2e,0x627c,
	0x7cea,0x1b2c,0x5a05,0x8598,0x9e00,0xcf01,0x62d9,0x7a10,
	0x1f42,0x87ce,0x575d,0x6e23,0x86ef,0x93c2,0x3d1a,0x89aa,
	0xe199,0xba1d,0x1b72,0x4513,0x5131,0xc23c,0xba9f,0xa069,
	0xfbfb,0xda92,0x42b2,0x3a48,0xdb96,0x5fad,0xba96,0xc6eb,
};


#if 0
	{   // table[15]    NOT USED!
		0,0,0,1,1,1,1,0,1,1,1,1,1,0,0,1,1,0,1,0,1,1,0,0,1,0,1,0,1,0,0,0,
		1,0,0,0,1,1,0,1,1,1,1,0,0,0,1,0,0,0,1,0,1,1,0,1,0,0,0,0,1,1,1,1,
		0,1,1,1,1,0,0,1,0,1,1,1,1,1,1,1,0,0,0,0,1,0,0,0,1,0,1,1,0,0,1,1,
		0,0,1,0,0,0,0,0,0,1,1,1,0,0,1,0,1,1,1,1,1,1,1,0,1,0,0,0,0,1,0,0,
		0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,1,1,0,0,1,0,1,1,1,1,1,1,1,0,1,1,
		0,1,0,0,1,0,1,0,0,0,1,0,0,1,1,0,0,1,1,1,1,1,0,1,0,1,0,1,0,0,0,1,
		1,0,1,1,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,1,1,0,0,1,0,1,1,1,0,0,0,
		0,0,0,1,1,1,0,0,0,1,0,0,1,1,0,1,1,1,0,0,0,1,1,1,1,1,0,0,1,0,1,1,
	},
#endif


static const u8 spi_bitswap[16][16] =
{
	{ 15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, },
	{  7, 6, 5,14, 0,15, 4, 3, 2, 8, 9,10,11,12,13, 1, },
	{  9,15,14,13,12, 0, 1, 2,10, 8, 7, 6, 5, 4, 3,11, },
	{  5, 4, 3, 2, 9,14,13,12,11, 6, 7, 8, 1,15, 0,10, },
	{ 12,11, 0, 7, 8, 5, 6,15,10,13,14, 1, 2, 3, 4, 9, },
	{ 14, 0, 1, 2, 3, 9, 8, 7,15, 5, 6,13,12,11,10, 4, },
	{ 13,12,11,10, 2, 7, 8, 9, 0,14,15, 3, 4, 5, 6, 1, },
	{  2, 9,10,11,12, 7, 6, 5,14, 3, 4, 0,15, 1, 8,13, },
	{  8, 7, 4, 3, 2,13,12,11, 0, 9,10,14,15, 6, 5, 1, },
	{  3, 2,10,11,12, 5,14, 0, 1, 4,15, 6, 7, 8, 9,13, },
	{  2,10, 6, 5, 4,14,13,12,11, 1, 0,15, 9, 8, 7, 3, },
	{ 12,11, 8, 1,15, 3, 2, 9,10,13,14, 4, 5, 6, 7, 0, },
	{  8, 7, 0,11,12, 5, 6,15,14, 9,10, 1, 2, 3, 4,13, },
	{  3, 2, 1, 0,14, 9, 8, 7, 6, 4,15,13,12,11,10, 5, },
	{  2,10,11,12,13, 7, 8, 9,15, 1, 0, 3, 4, 5, 6,14, },
	{ 12,11,10, 9, 2, 7, 6, 5, 4,13,14, 0,15, 1, 8, 3, },
};


static int key(int table, int addr)
{
	const int xorbit = 8 + ((table & 0xc) >> 2);
	return BIT(key_table[addr & 0xff] >> 4, table) ^ BIT(addr, xorbit);
}


void seibuspi_sprite_decrypt(u8 *src, int rom_size)
{
	for (int i = 0; i < rom_size/2; i++)
	{
		const int addr = i >> 8;

		const u16 y1 = src[2 * i + 0 * rom_size + 0] + (src[2 * i + 0 * rom_size + 1] << 8);
		const u16 y2 = src[2 * i + 1 * rom_size + 0] + (src[2 * i + 1 * rom_size + 1] << 8);
		u16 y3       = src[2 * i + 2 * rom_size + 0] + (src[2 * i + 2 * rom_size + 1] << 8);


		/* first of all, permutate 16 of the 48 bits */
		const u8 *bs = spi_bitswap[key_table[addr & 0xff] & 0xf];
		y3 = bitswap<16>(y3, bs[0],bs[1],bs[2],bs[3],bs[4],bs[5],bs[6],bs[7],
							bs[8],bs[9],bs[10],bs[11],bs[12],bs[13],bs[14],bs[15]);


		// planes 4 & 5, interleaved
		u32 s1 = (BIT(y1, 4) <<  0) |
				 (BIT(y3, 7) <<  1) |
				 (BIT(y3, 6) <<  2) |
				 (BIT(y2,12) <<  3) |
				 (BIT(y2, 3) <<  4) |
				 (BIT(y1,10) <<  5) |
				 (BIT(y1, 1) <<  6) |
				 (BIT(y3,14) <<  7) |
				 (BIT(y3, 2) <<  8) |
				 (BIT(y2, 9) <<  9) |
				 (BIT(y2, 0) << 10) |
				 (BIT(y1, 7) << 11) |
				 (BIT(y3,12) << 12) |
				 (BIT(y2,15) << 13) |
				 (BIT(y2, 6) << 14) |
				 (BIT(y1,13) << 15);

		u32 add1 = (BIT(addr,11) <<  0) |
				   (BIT(addr,10) <<  1) |
				   (key(10,addr) <<  2) |
				   (key( 5,addr) <<  3) |
				   (key( 4,addr) <<  4) |
				   (BIT(addr,11) <<  5) |
				   (BIT(addr,11) <<  6) |
				   (key( 7,addr) <<  7) |
				   (key( 6,addr) <<  8) |
				   (key( 1,addr) <<  9) |
				   (key( 0,addr) << 10) |
				   (BIT(addr,11) << 11) |
				   (key( 9,addr) << 12) |
				   (key( 8,addr) << 13) |
				   (key( 3,addr) << 14) |
				   (key( 2,addr) << 15);

		// planes 0-3, interleaved
		u32 s2 = (BIT(y1, 5) <<  0) |
				 (BIT(y3, 0) <<  1) |
				 (BIT(y3, 5) <<  2) |
				 (BIT(y2,13) <<  3) |
				 (BIT(y2, 4) <<  4) |
				 (BIT(y1,11) <<  5) |
				 (BIT(y1, 2) <<  6) |
				 (BIT(y3, 9) <<  7) |
				 (BIT(y3, 3) <<  8) |
				 (BIT(y2, 8) <<  9) |
				 (BIT(y1,15) << 10) |
				 (BIT(y1, 6) << 11) |
				 (BIT(y3,11) << 12) |
				 (BIT(y2,14) << 13) |
				 (BIT(y2, 5) << 14) |
				 (BIT(y1,12) << 15) |
				 (BIT(y1, 3) << 16) |
				 (BIT(y3, 8) << 17) |
				 (BIT(y3,15) << 18) |
				 (BIT(y2,11) << 19) |
				 (BIT(y2, 2) << 20) |
				 (BIT(y1, 9) << 21) |
				 (BIT(y1, 0) << 22) |
				 (BIT(y3,10) << 23) |
				 (BIT(y3, 1) << 24) |
				 (BIT(y2,10) << 25) |
				 (BIT(y2, 1) << 26) |
				 (BIT(y1, 8) << 27) |
				 (BIT(y3,13) << 28) |
				 (BIT(y3, 4) << 29) |
				 (BIT(y2, 7) << 30) |
				 (BIT(y1,14) << 31);

		u32 add2 = (key( 0,addr) <<  0) |
				   (key( 1,addr) <<  1) |
				   (key( 2,addr) <<  2) |
				   (key( 3,addr) <<  3) |
				   (key( 4,addr) <<  4) |
				   (key( 5,addr) <<  5) |
				   (key( 6,addr) <<  6) |
				   (key( 7,addr) <<  7) |
				   (key( 8,addr) <<  8) |
				   (key( 9,addr) <<  9) |
				   (key(10,addr) << 10) |
				   (BIT(addr,10) << 11) |
				   (BIT(addr,11) << 12) |
				   (BIT(addr,11) << 13) |
				   (BIT(addr,11) << 14) |
				   (BIT(addr,11) << 15) |
				   (BIT(addr,11) << 16) |
				   (key( 7,addr) << 17) |
				   (BIT(addr,11) << 18) |
				   (key( 6,addr) << 19) |
				   (BIT(addr,11) << 20) |
				   (key( 5,addr) << 21) |
				   (BIT(addr,11) << 22) |
				   (key( 4,addr) << 23) |
				   (BIT(addr,10) << 24) |
				   (key( 3,addr) << 25) |
				   (key(10,addr) << 26) |
				   (key( 2,addr) << 27) |
				   (key( 9,addr) << 28) |
				   (key( 1,addr) << 29) |
				   (key( 8,addr) << 30) |
				   (key( 0,addr) << 31);

		s1 = partial_carry_sum(s1, add1, 0x3a59, 16) ^ 0x843a;
		s2 = partial_carry_sum(s2, add2, 0x28d49cac, 32) ^ 0xc8e29f84;


		// reorder the bits in the order MAME expects to decode the graphics
		u8 plane0 = 0, plane1 = 0, plane2 = 0, plane3 = 0, plane4 = 0, plane5 = 0;
		for (int j = 0; j < 8; j++)
		{
			plane5 |= (BIT(s1, 2 * j + 1) << j);
			plane4 |= (BIT(s1, 2 * j + 0) << j);
			plane3 |= (BIT(s2, 4 * j + 3) << j);
			plane2 |= (BIT(s2, 4 * j + 2) << j);
			plane1 |= (BIT(s2, 4 * j + 1) << j);
			plane0 |= (BIT(s2, 4 * j + 0) << j);
		}

		src[2 * i + 0 * rom_size + 0] = plane5;
		src[2 * i + 0 * rom_size + 1] = plane4;
		src[2 * i + 1 * rom_size + 0] = plane3;
		src[2 * i + 1 * rom_size + 1] = plane2;
		src[2 * i + 2 * rom_size + 0] = plane1;
		src[2 * i + 2 * rom_size + 1] = plane0;
	}
}




/******************************************************************************************

rdft2

CPU 'main' (PC=002A0709): unmapped program memory dword write to 0000054C = 00000000 & 0000FFFF
CPU 'main' (PC=002A0709): unmapped program memory dword write to 0000054C = 0000ABCB & 0000FFFF // okok
CPU 'main' (PC=002A0709): unmapped program memory dword write to 0000054C = 0000ABCB & 0000FFFF // duplicate
CPU 'main' (PC=002A0709): unmapped program memory dword write to 0000054C = 00006543 & 0000FFFF // okok
CPU 'main' (PC=002A0709): unmapped program memory dword write to 0000054C = 000021D9 & 0000FFFF // ok??
CPU 'main' (PC=002A0709): unmapped program memory dword write to 0000054C = 00006655 & 0000FFFF // okok
CPU 'main' (PC=002A0709): unmapped program memory dword write to 0000054C = 000099AA & 0000FFFF // okok
CPU 'main' (PC=002A0709): unmapped program memory dword write to 0000054C = 00006655 & 0000FFFF // duplicate
CPU 'main' (PC=002A0709): unmapped program memory dword write to 0000054C = 000099AA & 0000FFFF // duplicate
CPU 'main' (PC=002A0709): unmapped program memory dword write to 0000054C = 00000C1D & 0000FFFF // okok
CPU 'main' (PC=002A0709): unmapped program memory dword write to 0000054C = 0000A346 & 0000FFFF // okok
CPU 'main' (PC=002A0709): unmapped program memory dword write to 0000054C = 00005237 & 0000FFFF // okok
CPU 'main' (PC=002A0709): unmapped program memory dword write to 0000054C = 0000A948 & 0000FFFF // okok

        plane54   = partial_carry_sum16( plane54, 0xabcb, 0x55aa ) ^ 0x6699;
        plane3210 = partial_carry_sum32( plane3210, 0x654321d9 ^ 0x42, 0x1d463748 ) ^ 0x0ca352a9;

******************************************************************************************/

static void sprite_reorder(u8 *buffer)
{
	u8 temp[64];
	for (int j = 0; j < 16; j++)
	{
		temp[2 * (j * 2) + 0] = buffer[2 * j + 0];
		temp[2 * (j * 2) + 1] = buffer[2 * j + 1];
		temp[2 * (j * 2) + 2] = buffer[2 * j + 32];
		temp[2 * (j * 2) + 3] = buffer[2 * j + 33];
	}
	memcpy(buffer, temp, 64);
}

void seibuspi_rise10_sprite_decrypt(u8 *rom, int size)
{
	for (int i = 0; i < size / 2; i++)
	{
		u32 plane54 = rom[0 * size + 2 * i] + (rom[0 * size + 2 * i + 1] << 8);
		u32 plane3210 = bitswap<32>(
				rom[2 * size + 2 * i] + (rom[2 * size + 2 * i + 1] << 8) + (rom[1 * size + 2 * i] << 16) + (rom[1 * size + 2 * i + 1] << 24),
				23,13,24,4,16,12,25,30,
				3,5,29,17,14,22,2,11,
				27,6,15,21,1,28,10,20,
				7,31,26,0,18,9,19,8);

		plane54   = partial_carry_sum16(plane54, 0xabcb, 0x55aa) ^ 0x6699;
		plane3210 = partial_carry_sum32(plane3210, 0x654321d9 ^ 0x42, 0x1d463748) ^ 0x0ca352a9;

		rom[0 * size + 2 * i]     = plane54   >>  8;
		rom[0 * size + 2 * i + 1] = plane54   >>  0;
		rom[1 * size + 2 * i]     = plane3210 >> 24;
		rom[1 * size + 2 * i + 1] = plane3210 >> 16;
		rom[2 * size + 2 * i]     = plane3210 >>  8;
		rom[2 * size + 2 * i + 1] = plane3210 >>  0;
	}

	for (int i = 0; i < size / 2; i += 32)
	{
		sprite_reorder(&rom[0 * size + 2 * i]);
		sprite_reorder(&rom[1 * size + 2 * i]);
		sprite_reorder(&rom[2 * size + 2 * i]);
	}
}




/******************************************************************************************

rfjet

cpu #0 (PC=002C40F9): unmapped program memory dword write to 0000054C = 00000000 & 0000FFFF
cpu #0 (PC=002C40F9): unmapped program memory dword write to 0000054C = 00000000 & 0000FFFF
cpu #0 (PC=002C40F9): unmapped program memory dword write to 0000054C = 000055AB & 0000FFFF // okok
cpu #0 (PC=002C40F9): unmapped program memory dword write to 0000054C = 00000000 & 0000FFFF
cpu #0 (PC=002C40F9): unmapped program memory dword write to 0000054C = 0000AA6A & 0000FFFF // okok
cpu #0 (PC=002C40F9): unmapped program memory dword write to 0000054C = 0000ABCB & 0000FFFF // okok
cpu #0 (PC=002C40F9): unmapped program memory dword write to 0000054C = 0000DD4C & 0000FFFF // okok
cpu #0 (PC=002C40F9): unmapped program memory dword write to 0000054C = 00006543 & 0000FFFF // no??
cpu #0 (PC=002C40F9): unmapped program memory dword write to 0000054C = 0000D68B & 0000FFFF // okok
cpu #0 (PC=002C40F9): unmapped program memory dword write to 0000054C = 000037F2 & 0000FFFF // okok
cpu #0 (PC=002C40F9): unmapped program memory dword write to 0000054C = 000021D9 & 0000FFFF // ????
cpu #0 (PC=002C40F9): unmapped program memory dword write to 0000054C = 00005B3B & 0000FFFF // okok
cpu #0 (PC=002C40F9): unmapped program memory dword write to 0000054C = 00000300 & 0000FFFF

        plane543 = partial_carry_sum32( plane543, 0xabcb64, 0x55aadd ) ^ 0xab6a4c;
        plane210 = partial_carry_sum24( plane210,        i, 0xd6375b ) ^ 0x8bf23b;


feversoc

CPU 'main' (PC=00021C74): unmapped program memory dword write to 0601004C = D5A90000 & FFFF0000
CPU 'main' (PC=00021C74): unmapped program memory dword write to 0601004C = 4EB50000 & FFFF0000
CPU 'main' (PC=00021C74): unmapped program memory dword write to 0601004C = 9A4A0000 & FFFF0000 // okok
CPU 'main' (PC=00021C74): unmapped program memory dword write to 0601004C = ADB30000 & FFFF0000
CPU 'main' (PC=00021C74): unmapped program memory dword write to 0601004C = E9320000 & FFFF0000 // okok
CPU 'main' (PC=00021C74): unmapped program memory dword write to 0601004C = 9DF50000 & FFFF0000 // okok
CPU 'main' (PC=00021C74): unmapped program memory dword write to 0601004C = 99E90000 & FFFF0000 // okok
CPU 'main' (PC=00021C74): unmapped program memory dword write to 0601004C = B2590000 & FFFF0000 // ok??
CPU 'main' (PC=00021C74): unmapped program memory dword write to 0601004C = 961D0000 & FFFF0000 // okok
CPU 'main' (PC=00021C74): unmapped program memory dword write to 0601004C = 8B970000 & FFFF0000 // okok
CPU 'main' (PC=00021C74): unmapped program memory dword write to 0601004C = EAAE0000 & FFFF0000 // ????
CPU 'main' (PC=00021C74): unmapped program memory dword write to 0601004C = D5AC0000 & FFFF0000 // okok
CPU 'main' (PC=00021C74): unmapped program memory dword write to 0601004C = 03000000 & FFFF0000

        plane543 = partial_carry_sum32( plane543, 0x9df5b2, 0x9ae999 ) ^ 0x4a32e9;
        plane210 = partial_carry_sum24( plane210,        i, 0x968bd5 ) ^ 0x1d97ac;
        plane210 = partial_carry_sum24( plane210,        1, 0x000001 );

******************************************************************************************/


static void seibuspi_rise11_sprite_decrypt(u8 *rom, int size,
	u32 k1, u32 k2, u32 k3, u32 k4, u32 k5, int feversoc_kludge)
{
	for (int i = 0; i < size/2; i++)
	{
		const u16 b1 = rom[0 * size + 2 * i] + (rom[0 * size + 2 * i + 1] << 8);
		const u16 b2 = rom[1 * size + 2 * i] + (rom[1 * size + 2 * i + 1] << 8);
		const u16 b3 = rom[2 * size + 2 * i] + (rom[2 * size + 2 * i + 1] << 8);

		u32 plane543 = (BIT(b2,11)<< 0) |
					   (BIT(b1, 6)<< 1) |
					   (BIT(b3,12)<< 2) |
					   (BIT(b3, 3)<< 3) |
					   (BIT(b2,12)<< 4) |
					   (BIT(b3,14)<< 5) |
					   (BIT(b3, 4)<< 6) |
					   (BIT(b1,11)<< 7) |
					   (BIT(b1,12)<< 8) |
					   (BIT(b1, 2)<< 9) |
					   (BIT(b2, 5)<<10) |
					   (BIT(b1, 9)<<11) |
					   (BIT(b3, 1)<<12) |
					   (BIT(b2, 2)<<13) |
					   (BIT(b2,10)<<14) |
					   (BIT(b3, 5)<<15) |
					   (BIT(b1, 3)<<16) |
					   (BIT(b2, 7)<<17) |
					   (BIT(b1,15)<<18) |
					   (BIT(b3, 9)<<19) |
					   (BIT(b2,13)<<20) |
					   (BIT(b1, 4)<<21) |
					   (BIT(b3, 2)<<22) |
					   (BIT(b2, 0)<<23);

		u32 plane210 = (BIT(b1,14)<< 0) |
					   (BIT(b1, 1)<< 1) |
					   (BIT(b1,13)<< 2) |
					   (BIT(b3, 0)<< 3) |
					   (BIT(b1, 7)<< 4) |
					   (BIT(b2,14)<< 5) |
					   (BIT(b2, 4)<< 6) |
					   (BIT(b2, 9)<< 7) |
					   (BIT(b3, 8)<< 8) |
					   (BIT(b2, 1)<< 9) |
					   (BIT(b3, 7)<<10) |
					   (BIT(b2, 6)<<11) |
					   (BIT(b1, 0)<<12) |
					   (BIT(b3,11)<<13) |
					   (BIT(b2, 8)<<14) |
					   (BIT(b3,13)<<15) |
					   (BIT(b1, 8)<<16) |
					   (BIT(b3,10)<<17) |
					   (BIT(b3, 6)<<18) |
					   (BIT(b1,10)<<19) |
					   (BIT(b2,15)<<20) |
					   (BIT(b2, 3)<<21) |
					   (BIT(b1, 5)<<22) |
					   (BIT(b3,15)<<23);

		plane543 = partial_carry_sum32(plane543, k1, k2) ^ k3;
		plane210 = partial_carry_sum24(plane210,  i, k4) ^ k5;
		if (feversoc_kludge)
			plane210 = partial_carry_sum24(plane210,  1, 0x000001);

		rom[0 * size + 2 * i]     = plane543 >> 16;
		rom[0 * size + 2 * i + 1] = plane543 >>  8;
		rom[1 * size + 2 * i]     = plane543 >>  0;
		rom[1 * size + 2 * i + 1] = plane210 >> 16;
		rom[2 * size + 2 * i]     = plane210 >>  8;
		rom[2 * size + 2 * i + 1] = plane210 >>  0;
	}

	for (int i = 0; i < size / 2; i += 32)
	{
		sprite_reorder(&rom[0 * size + 2 * i]);
		sprite_reorder(&rom[1 * size + 2 * i]);
		sprite_reorder(&rom[2 * size + 2 * i]);
	}
}


void seibuspi_rise11_sprite_decrypt_rfjet(u8 *rom, int size)
{
	seibuspi_rise11_sprite_decrypt(rom, size, 0xabcb64, 0x55aadd, 0xab6a4c, 0xd6375b, 0x8bf23b, 0);
}


void seibuspi_rise11_sprite_decrypt_feversoc(u8 *rom, int size)
{
	seibuspi_rise11_sprite_decrypt(rom, size, 0x9df5b2, 0x9ae999, 0x4a32e9, 0x968bd5, 0x1d97ac, 1);
}
