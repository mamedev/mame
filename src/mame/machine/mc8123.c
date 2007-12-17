/***************************************************************************

NEC MC-8123 encryption emulation

Decryption tables provided by David Widel
Decryption algorithm by Nicola Salmoria

The NEC MC-8123 is a Z80 with built-in encryption.
It contains 0x2000 bytes of battery-backed RAM, when the battery dies the
game stops working. It's possible that the CPU would work as a normal Z80
in that case, this hasn't been verified.

When the CPU reads a byte from memory, it uses 12 bits of the address and
the information of whether the byte is an opcode or not to select a byte in
the internal RAM. The byte controls how the decryption works.

A special value in the internal RAM instructs the CPU to not apply the decryption
algorithm at all. This is necessary for the CPU to be able to use RAM areas
normally.

In theory, the size of the key would make the encryption quite secure.

In practice, the contents of the internal RAM were generated using a linear
congruential generator, so the whole key can be generated starting from a single
24-bit seed.

The LCG is the same that was used to generate the FD1094 keys.
Note that the algorithm skips the special value that would instruct the CPU to
not decrypt at that address.

void generate_key(UINT8 *key, int seed, int upper_bound)
{
    int i;

    for (i = 0; i < upper_bound; ++i)
    {
        UINT8 byteval;

        do
        {
            seed = seed * 0x29;
            seed += seed << 16;

            byteval = (~seed >> 16) & 0xff;
        } while (byteval == 0xff);

        key[i] = byteval;
    }

    for (; i < 0x2000; ++i)
        key[i] = 0xff;
}


The known games that use this CPU are:

CPU #    Game                      Notes              Seed   Upper Limit
-------- ------------------------- ------------------ ------ -----
317-0014 DakkoChan Jansoh                             206850  1C00
317-0029 Block Gal                 NEC MC-8123B 651   091755  1800?
317-0030 Perfect Billiard                             9451EC  1C00
317-0042 Opa Opa                                      B31FD0  1C00
317-0043 Wonder Boy Monster Land                      640708  1C00
317-0054 Shinobi (sound CPU)       NEC MC-8123B 652   088992  1800
317-0057 Fantasy Zone 2                               ADFACE    1800
317-0064 Ufo Senshi Yohko Chan                        880603  1C00
317-0066 Altered Beast (sound CPU)                    ED8600  1800
317-5012 Ganbare Chinsan Ooshoubu  NEC MC-8123A       804B54  1C00
317-5??? Ninja Kid II (sound CPU)                     27998D  1800

***************************************************************************/

#include "driver.h"
#include "mc8123.h"



static int decrypt_type0(int val,int param,int swap)
{
	int a = val;

	if (swap == 1) a = BITSWAP8(a,5,4,3,6,7,1,2,0);
	if (swap == 2) a = BITSWAP8(a,0,5,3,2,4,7,6,1);
	if (swap == 3) a = BITSWAP8(a,0,1,7,5,3,6,2,4);

	val = 0x1c;
	if (BIT(a,0)) val ^= 0x23;
	if (BIT(a,1)) val ^= 0x08;
	if (BIT(a,2)) val ^= 0x04;
	if (BIT(a,3)) val ^= 0x20;
	if (BIT(a,4)) val ^= 0x01;
	if (BIT(a,6)) val ^= 0x10;
	if (BIT(a,5))
	{
		val ^= 0x73;

		if (param & 0x40)
		{
			val ^= 0xb3;
			if (param & 0x01) val ^= 0x40;
			if (param & 0x04) val ^= 0x40;
			if (param & 0x08) val ^= 0x40;
		}
		if (~param & 0x02)
		{
			val ^= 0xb3;
			if (param & 0x01) val ^= 0x40;
			if (param & 0x04) val ^= 0x40;
			if (param & 0x08) val ^= 0x40;
		}
		if (param & 0x01)
		{
			val ^= 0xb3;
			if (param & 0x01) val ^= 0x40;
			if (param & 0x04) val ^= 0x40;
			if (param & 0x08) val ^= 0x40;
		}
	}

	if (BIT(a,7))
	{
		val ^= 0x80;

		if (param & 0x01) val ^= 0x40;
		if (param & 0x02) val ^= 0x25;
		if (param & 0x04) val ^= 0x40;
		if (param & 0x08) val ^= 0x40;
		if (param & 0x40) val ^= 0x25;
		if (param & 0x80) val ^= 0x25;
	}

	if (param & 0x01) val ^= 0xe3;
	if (param & 0x02) val ^= 0x23;
	if (param & 0x04) val ^= 0xc0;
	if (param & 0x08) val ^= 0xc0;
	if (param & 0x40) val ^= 0x23;

	if (param & 0x01)
		val ^= 0x21;
	else
		val = BITSWAP8(val,7,6,5,3,2,1,4,0);

	return val;
}

static int decrypt_type1(int val,int param,int swap)
{
	if (BIT(~param,2) ^ BIT(param,0))
	{
		int a = val;

		if (swap == 0) a = BITSWAP8(a,7,4,2,0,5,3,6,1);
		if (swap == 1) a = BITSWAP8(a,2,6,0,7,4,3,5,1);
		if (swap == 2) a = BITSWAP8(a,0,2,3,5,1,4,6,7);

		if (BIT(~param,6) ^ BIT(param,1) ^ BIT(param,0))
			a = BITSWAP8(a,7,6,5,4,0,2,3,1);

		val = 0x4b;
		if (BIT(a,0)) val ^= 0x20;
		if (BIT(a,1)) val ^= 0xd4;
		if (BIT(a,2)) val ^= 0x08;
		if (BIT(a,3)) val ^= 0x03;
		if (BIT(a,4)) val ^= 0x01;
		if (BIT(a,5)) val ^= 0x48;
		if (BIT(a,6)) val ^= 0xd8;
		if (BIT(a,7)) val ^= 0x5e;

		if (param & 0x01) val ^= 0x48;
		if (param & 0x02) val ^= 0x84;
		if (param & 0x04) val ^= 0x48;
		if (param & 0x08) val ^= 0x48;
		if (param & 0x40) val ^= 0x84;
		if (param & 0x80) val ^= 0x84;

		if (param & 0x01) val = BITSWAP8(val,7,6,1,4,3,2,5,0);

		return val;
	}
	else
	{
		int a = val;

		if (swap == 1) a = BITSWAP8(a,3,4,7,6,5,1,2,0);
		if (swap == 2) a = BITSWAP8(a,3,5,0,1,7,2,6,4);
		if (swap == 3) a = BITSWAP8(a,4,2,0,5,3,6,7,1);

		val = 0x51;
		if (BIT(a,0)) val ^= 0x42;
		if (BIT(a,1)) val ^= 0x84;
		if (BIT(a,2)) val ^= 0x70;
		if (BIT(a,3)) val ^= 0xa4;
		if (BIT(a,4)) val ^= 0x22;
		if (BIT(a,5)) val ^= 0xcf;
		if (BIT(a,6)) val ^= 0x04;
		if (BIT(a,7)) val ^= 0x47;

		if (BIT(a,7) && BIT(a,6))
			val ^= 0xf4;

		if (param & 0x01) val ^= 0x96;
		if (param & 0x02) val ^= 0xdb;
		if (param & 0x04) val ^= 0x18;
		if (param & 0x08) val ^= 0x18;
		if (param & 0x40) val ^= 0xdb;
		if (param & 0x80) val ^= 0x12;

		return val;
	}
}

static int decrypt_type2(int val,int param,int swap)
{
	if (BIT(~param,2) ^ BIT(param,0))
	{
		int a = val;

		if (swap == 1) a = BITSWAP8(a,2,4,7,0,5,1,3,6);
		if (swap == 2) a = BITSWAP8(a,2,3,0,4,5,7,6,1);
		if (swap == 3) a = BITSWAP8(a,0,3,2,7,5,1,6,4);

		val = 0xea;
		if (BIT(a,0)) val ^= 0xa0;
		if (BIT(a,1)) val ^= 0x80;
		if (BIT(a,2)) val ^= 0x18;
		if (BIT(a,3)) val ^= 0x07;
		if (BIT(a,4)) val ^= 0x41;
		if (BIT(a,5)) val ^= 0x04;
		if (BIT(a,6)) val ^= 0x10;
		if (BIT(a,7)) val ^= 0x60;

		if (!BIT(a,5) && ((~param & 0x08) || !BIT(a,6)))
		{
			if (BIT(a,0)) val ^= 0x20;
			if (BIT(a,7)) val ^= 0x21;
			if (BIT(a,1)) val ^= 0xe0;
			if (BIT(a,4)) val ^= 0xe1;
		}

		if (param & 0x01) val ^= 0x1f;
		if (param & 0x02) val ^= 0x1f;
		if (param & 0x40) val ^= 0x1f;

		if (BIT(param,6) ^ BIT(param,1) ^ BIT(param,7))
		{
			if (param & 0x01)
				val = BITSWAP8(val,7,6,5,2,1,3,4,0);
			else
				val = BITSWAP8(val,7,6,5,3,4,1,2,0);
		}
		else
		{
			if (~param & 0x01)
				val = BITSWAP8(val,7,6,5,1,2,4,3,0);
		}

		return val;
	}
	else
	{
		// only 0x20 possible encryptions for this method - all others have 0x40

		int a = val;

		if (swap == 1) a = BITSWAP8(a,3,4,7,5,1,6,0,2);
		if (swap == 2) a = BITSWAP8(a,2,1,6,4,5,7,3,0);
		if (swap == 3) a = BITSWAP8(a,7,0,4,3,2,1,5,6);

		val = 0x0e;
		if (BIT(a,0)) val ^= 0x33;
		if (BIT(a,1)) val ^= 0x8e;
		if (BIT(a,2)) val ^= 0x01;
		if (BIT(a,3)) val ^= 0xe0;
		if (BIT(a,4)) val ^= 0x5b;
		if (BIT(a,5)) val ^= 0xea;
		if (BIT(a,6)) val ^= 0x58;
		if (BIT(a,7)) val ^= 0x0e;

		if (BIT(a,1) && BIT(a,5))
			val ^= 0xa8;

		if (BIT(a,6))
		{
			if (BIT(a,0) ^ BIT(a,4) ^ BIT(a,1))
				val ^= 0x11;
		}
		if (BIT(a,1))
		{
			if (!BIT(a,5))
			{
				if (BIT(a,0) ^ BIT(a,4))
					val ^= 0x11;
			}
		}

		if (param & 0x01) val ^= 0xca;
		if (param & 0x02) val ^= 0xa4;
		if (param & 0x08) val ^= 0x80;
		if (param & 0x40) val ^= 0xa4;
		if (param & 0x80) val ^= 0x4a;

		return val;
	}
}

static int decrypt_type3(int val,int param,int swap)
{
	if (BIT(~param,2) ^ BIT(param,0))
	{
		int a = val;

		if (swap == 0) a = BITSWAP8(a,2,4,7,0,5,1,3,6);
		if (swap == 2) a = BITSWAP8(a,0,3,2,7,5,1,6,4);
		if (swap == 3) a = BITSWAP8(a,2,3,0,4,5,7,6,1);

		if (BIT(param,6) ^ BIT(param,1) ^ BIT(param,7))
			a = BITSWAP8(a,7,6,5,4,2,3,1,0);

		if (BIT(a,4))
			a = BITSWAP8(a,7,6,5,4,2,3,1,0);

		val = 0x78;
		if (BIT(a,2)) val ^= 0x26;
		if (BIT(a,3)) val ^= 0x80;
		if (BIT(a,5)) val ^= 0x10;
		if (param & 0x01)
		{
			if (BIT(a,0)) val ^= 0x04;
			if (BIT(a,1)) val ^= 0x01;
			if (BIT(a,4)) val ^= 0x5d;
			if (BIT(a,6)) val ^= 0x43;
			if (BIT(a,7)) val ^= 0xf6;
		}
		else
		{
			if (BIT(a,0)) val ^= 0x43;
			if (BIT(a,1)) val ^= 0x40;
			if (BIT(a,4)) val ^= 0x1f;
			if (BIT(a,6)) val ^= 0x01;
			if (BIT(a,7)) val ^= 0xb2;
		}

		if (param & 0x01) val ^= 0x81;
		if (param & 0x02) val ^= 0x98;
		if (param & 0x08) val ^= 0x10;
		if (param & 0x40) val ^= 0x98;
		if (param & 0x80) val ^= 0x18;

		return val;
	}
	else
	{
		int a = val;

		if (swap == 0) a = BITSWAP8(a,3,4,7,5,1,6,0,2);
		if (swap == 2) a = BITSWAP8(a,7,0,4,3,2,1,5,6);
		if (swap == 3) a = BITSWAP8(a,2,1,6,4,5,7,3,0);

		if (BIT(param,6) ^ BIT(param,1) ^ BIT(param,7))
		{
			val = 0xaa;
			if (param & 0x01) val ^= 0x04;
			if (param & 0x80) val ^= 0x04;

			if (BIT(a,0)) val ^= 0xd6;
			if (BIT(a,1)) val ^= 0x30;
			if (BIT(a,2)) val ^= 0x11;
			if (BIT(a,3)) val ^= 0x04;
			if (BIT(a,4)) val ^= 0x08;
			if (BIT(a,5)) val ^= 0x42;
			if (BIT(a,6)) val ^= 0xc6;
			if (BIT(a,7)) val ^= 0x03;
		}
		else
		{
			val = 0xac;
			if (param & 0x01) val ^= 0x01;
			if (param & 0x80) val ^= 0x01;

			if (BIT(a,0)) val ^= 0x54;
			if (BIT(a,1)) val ^= 0x08;
			if (BIT(a,2)) val ^= 0x82;
			if (BIT(a,3)) val ^= 0x01;
			if (BIT(a,4)) val ^= 0x30;
			if (BIT(a,5)) val ^= 0x40;
			if (BIT(a,6)) val ^= 0x52;
			if (BIT(a,7)) val ^= 0x86;
		}

		if (param & 0x01) val ^= 0x08;
		if (param & 0x08) val ^= 0x80;

		if (BIT(a,2) ^ BIT(a,7))
		{
			if (BIT(a,4) ^ BIT(a,1))
				val ^= 0x38;

			if (BIT(a,0) ^ BIT(a,5) ^ BIT(a,6))
				val ^= 0x02;
		}

		return val;
	}
}


static int decrypt(int val,int param,int opcode)
{
	int type = 0;
	int swap = 0;

	param ^= 0xff;

	// no encryption
	if (param == 0x00)
		return val;

	type ^= BIT(param,0) << 0;
	type ^= BIT(param,1) << 0;
	type ^= BIT(param,2) << 0;
	type ^= BIT(param,4) << 0;
	type ^= BIT(param,4) << 1;
	type ^= BIT(param,5) << 1;

	swap ^= BIT(param,0) << 0;
	swap ^= BIT(param,1) << 0;
	swap ^= BIT(param,2) << 1;
	swap ^= BIT(param,3) << 1;

	if (!opcode)
	{
		param ^= 1<<0;
		param ^= 1<<1;
		param ^= 1<<3;
		param ^= 1<<7;
	}

	switch (type)
	{
		default:
		case 0: return decrypt_type0(val,param,swap);

		case 1: return decrypt_type1(val,param,swap);

		case 2: return decrypt_type2(val,param,swap);

		case 3: return decrypt_type3(val,param,swap);
	}
}


static UINT8 mc8123_decrypt(offs_t addr,UINT8 val,const UINT8 *key,int opcode)
{
	int tbl_num;

	/* pick the translation table from bits fd57 of the address */
	tbl_num = (addr & 7) + ((addr & 0x10)>>1) + ((addr & 0x40)>>2) + ((addr & 0x100)>>3) + ((addr & 0xc00)>>4) + ((addr & 0xf000)>>4) ;

	return decrypt(val,key[tbl_num + (opcode ? 0 : 0x1000)],opcode);
}


void mc8123_decrypt_rom(int cpunum, const UINT8* key, int banknum, int numbanks)
{
	UINT8 *decrypted1 = auto_malloc(numbanks == 1 ? 0xc000 : 0x8000);
	UINT8 *decrypted2 = numbanks > 1 ? auto_malloc(0x4000 * numbanks) : decrypted1 + 0x8000;
	UINT8 *rom = memory_region(REGION_CPU1 + cpunum);
	int A, bank;

	memory_set_decrypted_region(cpunum, 0x0000, 0x7fff, decrypted1);
	if (numbanks > 1)
		memory_configure_bank_decrypted(banknum, 0, numbanks, decrypted2, 0x4000);

	for (A = 0x0000;A < 0x8000;A++)
	{
		UINT8 src = rom[A];

		/* decode the opcodes */
		decrypted1[A] = mc8123_decrypt(A,src,key,1);

		/* decode the data */
		rom[A] = mc8123_decrypt(A,src,key,0);
	}

	for (bank = 0; bank < numbanks; ++bank)
	{
		for (A = 0x8000;A < 0xc000;A++)
		{
			UINT8 src = rom[0x8000 + 0x4000*bank + A];

			/* decode the opcodes */
			decrypted2[0x4000 * bank + (A-0x8000)] = mc8123_decrypt(A,src,key,1);

			/* decode the data */
			rom[0x8000 + 0x4000*bank + A] = mc8123_decrypt(A,src,key,0);
		}
	}
}
