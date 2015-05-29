// license:BSD-3-Clause
// copyright-holders:Ville Linde, hap, Nicola Salmoria

#include "emu.h"
#include "machine/seibuspi.h"

/**************************************************************************

Tile encryption
---------------

The tile graphics encryption uses the same algorithm in all games, with
different keys for the SPI, RISE10 and RISE11 custom chips.

- Take 24 bits of gfx data (used to decrypt 4 pixels at 6 bpp) and perform
  a bit permutation on them (the permutation is the same in all games).
- Take the low 12 bits of the tile code and add a 24-bit number (KEY1) to it.
- Add the two 24-bit numbers resulting from the above steps, but with a
  catch: while performing the sum, some bits generate carry as usual, other
  bits don't, depending on a 24-bit key (KEY2). Note that the carry generated
  by bit 23 (if enabled) wraps around to bit 0.
- XOR the result with a 24-bit number (KEY3).

The decryption is actually programmable; the games write the key to the
custom IC on startup!! (writes to 000414)

**************************************************************************/

// add two numbers generating carry from one bit to the next only if
// the corresponding bit in carry_mask is 1
static UINT32 partial_carry_sum(UINT32 add1,UINT32 add2,UINT32 carry_mask,int bits)
{
	int i,res,carry;

	res = 0;
	carry = 0;
	for (i = 0;i < bits;i++)
	{
		int bit = BIT(add1,i) + BIT(add2,i) + carry;

		res += (bit & 1) << i;

		// generate carry only if the corresponding bit in carry_mask is 1
		if (BIT(carry_mask,i))
			carry = bit >> 1;
		else
			carry = 0;
	}

	// wrap around carry from top bit to bit 0
	if (carry)
		res ^=1;

	return res;
}

UINT32 partial_carry_sum32(UINT32 add1,UINT32 add2,UINT32 carry_mask)
{
	return partial_carry_sum(add1,add2,carry_mask,32);
}

static UINT32 partial_carry_sum24(UINT32 add1,UINT32 add2,UINT32 carry_mask)
{
	return partial_carry_sum(add1,add2,carry_mask,24);
}

static UINT32 partial_carry_sum16(UINT32 add1,UINT32 add2,UINT32 carry_mask)
{
	return partial_carry_sum(add1,add2,carry_mask,16);
}



static UINT32 decrypt_tile(UINT32 val, int tileno, UINT32 key1, UINT32 key2, UINT32 key3)
{
	val = BITSWAP24(val, 18,19,9,5, 10,17,16,20, 21,22,6,11, 15,14,4,23, 0,1,7,8, 13,12,3,2);

	return partial_carry_sum24( val, tileno + key1, key2 ) ^ key3;
}

static void decrypt_text(UINT8 *rom, UINT32 key1, UINT32 key2, UINT32 key3)
{
	int i;
	for(i=0; i<0x10000; i++)
	{
		UINT32 w;

		w = (rom[(i*3) + 0] << 16) | (rom[(i*3) + 1] << 8) | (rom[(i*3) +2]);

		w = decrypt_tile(w, i >> 4, key1, key2, key3);

		rom[(i*3) + 0] = (w >> 16) & 0xff;
		rom[(i*3) + 1] = (w >> 8) & 0xff;
		rom[(i*3) + 2] = w & 0xff;
	}
}

static void decrypt_bg(UINT8 *rom, int size, UINT32 key1, UINT32 key2, UINT32 key3)
{
	int i,j;
	for(j=0; j<size; j+=0xc0000)
	{
		for(i=0; i<0x40000; i++)
		{
			UINT32 w;

			w = (rom[j + (i*3) + 0] << 16) | (rom[j + (i*3) + 1] << 8) | (rom[j + (i*3) + 2]);

			w = decrypt_tile(w, i >> 6, key1, key2, key3);

			rom[j + (i*3) + 0] = (w >> 16) & 0xff;
			rom[j + (i*3) + 1] = (w >> 8) & 0xff;
			rom[j + (i*3) + 2] = w & 0xff;
		}
	}
}

/******************************************************************************************
cpu #0 (PC=0033B2EB): unmapped program memory dword write to 00000414 = 00000000 & 0000FFFF
cpu #0 (PC=0033B2EB): unmapped program memory dword write to 00000414 = 0000DF5B & 0000FFFF
cpu #0 (PC=0033B2EB): unmapped program memory dword write to 00000414 = 000078CF & 0000FFFF
cpu #0 (PC=0033B2EB): unmapped program memory dword write to 00000414 = 00001377 & 0000FFFF
cpu #0 (PC=0033B2EB): unmapped program memory dword write to 00000414 = 00002538 & 0000FFFF
cpu #0 (PC=0033B2EB): unmapped program memory dword write to 00000414 = 00004535 & 0000FFFF
cpu #0 (PC=0033B2EB): unmapped program memory dword write to 00000414 = 06DC0000 & FFFF0000
******************************************************************************************/

void seibuspi_text_decrypt(UINT8 *rom)
{
	decrypt_text( rom, 0x5a3845, 0x77cf5b, 0x1378df);
}

void seibuspi_bg_decrypt(UINT8 *rom, int size)
{
	decrypt_bg( rom, size, 0x5a3845, 0x77cf5b, 0x1378df);
}

/******************************************************************************************
cpu #0 (PC=002A097D): unmapped program memory dword write to 00000414 = 00000001 & 0000FFFF
cpu #0 (PC=002A097D): unmapped program memory dword write to 00000414 = 0000DCF8 & 0000FFFF
cpu #0 (PC=002A097D): unmapped program memory dword write to 00000414 = 00007AE2 & 0000FFFF
cpu #0 (PC=002A097D): unmapped program memory dword write to 00000414 = 0000154D & 0000FFFF
cpu #0 (PC=002A097D): unmapped program memory dword write to 00000414 = 00001731 & 0000FFFF
cpu #0 (PC=002A097D): unmapped program memory dword write to 00000414 = 0000466B & 0000FFFF
cpu #0 (PC=002A097D): unmapped program memory dword write to 00000414 = 3EDC0000 & FFFF0000
******************************************************************************************/

/* RISE10 (Raiden Fighters 2) */
void seibuspi_rise10_text_decrypt(UINT8 *rom)
{
	decrypt_text( rom, 0x823146, 0x4de2f8, 0x157adc);
}

void seibuspi_rise10_bg_decrypt(UINT8 *rom, int size)
{
	decrypt_bg( rom, size, 0x823146, 0x4de2f8, 0x157adc);
}

/******************************************************************************************
cpu #0 (PC=002C40F9): unmapped program memory dword write to 00000414 = 00000001 & 0000FFFF
cpu #0 (PC=002C40F9): unmapped program memory dword write to 00000414 = 00006630 & 0000FFFF
cpu #0 (PC=002C40F9): unmapped program memory dword write to 00000414 = 0000B685 & 0000FFFF
cpu #0 (PC=002C40F9): unmapped program memory dword write to 00000414 = 0000CCFE & 0000FFFF
cpu #0 (PC=002C40F9): unmapped program memory dword write to 00000414 = 000032A7 & 0000FFFF
cpu #0 (PC=002C40F9): unmapped program memory dword write to 00000414 = 0000547C & 0000FFFF
cpu #0 (PC=002C40F9): unmapped program memory dword write to 00000414 = 3EDC0000 & FFFF0000
******************************************************************************************/

/* RISE11 (Raiden Fighters Jet) */
void seibuspi_rise11_text_decrypt(UINT8 *rom)
{
	decrypt_text( rom, 0xaea754, 0xfe8530, 0xccb666);
}

void seibuspi_rise11_bg_decrypt(UINT8 *rom, int size)
{
	decrypt_bg( rom, size, 0xaea754, 0xfe8530, 0xccb666);
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

static void sprite_reorder(UINT8 *buffer)
{
	int j;
	UINT8 temp[64];
	for( j=0; j < 16; j++ ) {
		temp[2*(j*2)+0] = buffer[2*j+0];
		temp[2*(j*2)+1] = buffer[2*j+1];
		temp[2*(j*2)+2] = buffer[2*j+32];
		temp[2*(j*2)+3] = buffer[2*j+33];
	}
	memcpy(buffer, temp, 64);
}

void seibuspi_rise10_sprite_decrypt(UINT8 *rom, int size)
{
	int i;

	for (i = 0; i < size/2; i++)
	{
		UINT32 plane54,plane3210;

		plane54 = rom[0*size+2*i] + (rom[0*size+2*i+1] << 8);
		plane3210 = BITSWAP32(
				rom[2*size+2*i] + (rom[2*size+2*i+1] << 8) + (rom[1*size+2*i] << 16) + (rom[1*size+2*i+1] << 24),
				23,13,24,4,16,12,25,30,
				3,5,29,17,14,22,2,11,
				27,6,15,21,1,28,10,20,
				7,31,26,0,18,9,19,8);

		plane54   = partial_carry_sum16( plane54, 0xabcb, 0x55aa ) ^ 0x6699;
		plane3210 = partial_carry_sum32( plane3210, 0x654321d9 ^ 0x42, 0x1d463748 ) ^ 0x0ca352a9;

		rom[0*size+2*i]   = plane54   >>  8;
		rom[0*size+2*i+1] = plane54   >>  0;
		rom[1*size+2*i]   = plane3210 >> 24;
		rom[1*size+2*i+1] = plane3210 >> 16;
		rom[2*size+2*i]   = plane3210 >>  8;
		rom[2*size+2*i+1] = plane3210 >>  0;
	}

	for (i = 0; i < size/2; i += 32)
	{
		sprite_reorder(&rom[0*size+2*i]);
		sprite_reorder(&rom[1*size+2*i]);
		sprite_reorder(&rom[2*size+2*i]);
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


static void seibuspi_rise11_sprite_decrypt(UINT8 *rom, int size,
	UINT32 k1, UINT32 k2, UINT32 k3, UINT32 k4, UINT32 k5, int feversoc_kludge)
{
	int i;

	for (i = 0; i < size/2; i++)
	{
		UINT16 b1,b2,b3;
		UINT32 plane543,plane210;

		b1 = rom[0*size+2*i] + (rom[0*size+2*i+1] << 8);
		b2 = rom[1*size+2*i] + (rom[1*size+2*i+1] << 8);
		b3 = rom[2*size+2*i] + (rom[2*size+2*i+1] << 8);

		plane543 =  (BIT(b2,11)<< 0) |
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

		plane210 =  (BIT(b1,14)<< 0) |
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

		plane543 = partial_carry_sum32( plane543, k1, k2 ) ^ k3;
		plane210 = partial_carry_sum24( plane210,  i, k4 ) ^ k5;
		if (feversoc_kludge)
			plane210 = partial_carry_sum24( plane210,  1, 0x000001 );

		rom[0*size+2*i]   = plane543 >> 16;
		rom[0*size+2*i+1] = plane543 >>  8;
		rom[1*size+2*i]   = plane543 >>  0;
		rom[1*size+2*i+1] = plane210 >> 16;
		rom[2*size+2*i]   = plane210 >>  8;
		rom[2*size+2*i+1] = plane210 >>  0;
	}

	for (i = 0; i < size/2; i += 32)
	{
		sprite_reorder(&rom[0*size+2*i]);
		sprite_reorder(&rom[1*size+2*i]);
		sprite_reorder(&rom[2*size+2*i]);
	}
}


void seibuspi_rise11_sprite_decrypt_rfjet(UINT8 *rom, int size)
{
	seibuspi_rise11_sprite_decrypt(rom, size, 0xabcb64, 0x55aadd, 0xab6a4c, 0xd6375b, 0x8bf23b, 0);
}


void seibuspi_rise11_sprite_decrypt_feversoc(UINT8 *rom, int size)
{
	seibuspi_rise11_sprite_decrypt(rom, size, 0x9df5b2, 0x9ae999, 0x4a32e9, 0x968bd5, 0x1d97ac, 1);
}
