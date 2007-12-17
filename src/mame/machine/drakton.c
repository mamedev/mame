/***************************************************************************************
    Drakton (c) 1984 Epos
    decryption by Pierpaolo Prazzoli
    based on decryptions of the other Epos games

    For general documentation on how the encryption works, see machine/theglobp.c
****************************************************************************************/

#include "driver.h"
#include "includes/dkong.h"

static void drakton_decrypt_rom_8(void)
{
	UINT8 oldbyte,newbyte;
	UINT8 *ROM;
	int mem;

	ROM = memory_region(REGION_CPU1);

	for (mem=0;mem<0x4000;mem++)
	{
		oldbyte = ROM[mem];

		/*  Note: D2 is inverted and connected to D1, D5 is inverted and
            connected to D0.  The other six data bits are converted by a
            PAL10H8 driven by the counter. */

		newbyte = (oldbyte & 0x02) | (~oldbyte & ~0x02);
		newbyte = BITSWAP8(newbyte,7,6,1,3,0,4,2,5);

		ROM[mem + 0x10000] = newbyte;
	}
}

static void drakton_decrypt_rom_9(void)
{
	UINT8 oldbyte,newbyte;
	UINT8 *ROM;
	int mem;

	ROM = memory_region(REGION_CPU1);

	for (mem=0;mem<0x4000;mem++)
	{
		oldbyte = ROM[mem];

		/*  Note: D2 is inverted and connected to D1, D5 is inverted and
            connected to D0.  The other six data bits are converted by a
            PAL10H8 driven by the counter. */

		newbyte = (oldbyte & 0x40) | (~oldbyte & ~0x40);
		newbyte = BITSWAP8(newbyte,7,1,4,3,0,6,2,5);

		ROM[mem + 0x14000] = newbyte;
	}
}

static void drakton_decrypt_rom_A(void)
{
	UINT8 oldbyte,newbyte;
	UINT8 *ROM;
	int mem;

	ROM = memory_region(REGION_CPU1);

	for (mem=0;mem<0x4000;mem++)
	{
		oldbyte = ROM[mem];

		/*  Note: D2 is inverted and connected to D1, D5 is inverted and
            connected to D0.  The other six data bits are converted by a
            PAL10H8 driven by the counter. */

		newbyte = (oldbyte & 0x8a) | (~oldbyte & ~0x8a);
		newbyte = BITSWAP8(newbyte,7,6,1,0,3,4,2,5);

		ROM[mem + 0x18000] = newbyte;
	}
}

static void drakton_decrypt_rom_B(void)
{
	UINT8 oldbyte,newbyte;
	UINT8 *ROM;
	int mem;

	ROM = memory_region(REGION_CPU1);

	for (mem=0;mem<0x4000;mem++)
	{
		oldbyte = ROM[mem];

		/*  Note: D2 is inverted and connected to D1, D5 is inverted and
            connected to D0.  The other six data bits are converted by a
            PAL10H8 driven by the counter. */

		newbyte = (oldbyte & 0xc8) | (~oldbyte & ~0xc8);
		newbyte = BITSWAP8(newbyte,7,1,4,0,3,6,2,5);

		ROM[mem + 0x1C000] = newbyte;
	}
}

DRIVER_INIT( drakton )
{
	/* While the PAL supports up to 16 decryption methods, only four
        are actually used in the PAL.  Therefore, we'll take a little
        memory overhead and decrypt the ROMs using each method in advance. */
	drakton_decrypt_rom_8();
	drakton_decrypt_rom_9();
	drakton_decrypt_rom_A();
	drakton_decrypt_rom_B();
}
