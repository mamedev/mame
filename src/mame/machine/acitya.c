/***************************************************************************

  machine.c

Same Epos board as usual(theglobp,beastf,street heat), see theglobp.c for hardware documentation.  This is fairly easy to decrypt since it has consecutive bytes with the same algorithym.  There are 4 different algorithyms.  One consists almost entirely of text, one contains the majority of code and the remaining 2 are not used much and are therefore the most difficult.  It is however difficult to decrypt to rom.  The data for the coin sound is actually program code in a different phase. You need to move the sound tables and add a rom to get it to run without the daughterboard.

acitya contains a bug with the insurance in blackjack.  It's impossible to collect, so it's likely that acitya is earlier than bwcasino.

I don't think this game is a gambling game.  For one thing there's no real output hardware on a pacman board and the epos daughterboard doesn't contain any either.

David Widel d_widel@hotmail.com

***************************************************************************/

#include "driver.h"
#include "includes/pacman.h"

static INT8 counter=0;


static void acitya_decrypt_rom_8(void)
{
	int oldbyte,inverted_oldbyte,newbyte;
	int mem;
	UINT8 *RAM;

	RAM = memory_region(REGION_CPU1);


	for (mem=0;mem<0x4000;mem++)
	{
		oldbyte = RAM[mem];
		inverted_oldbyte = ~oldbyte;

		/*  Note: D2 is inverted and connected to D1, D5 is inverted and
            connected to D0.  The other six data bits are converted by a
            PAL10H8 driven by the counter. */
		newbyte = 0;

		/* Direct inversion */
		newbyte  = (inverted_oldbyte & 0x80) >> 2;
		newbyte |= (oldbyte & 0x40) >> 0;
		newbyte |= (inverted_oldbyte & 0x20) >> 5;
		newbyte |= (inverted_oldbyte & 0x10) >> 1;
		newbyte |= (oldbyte & 0x08) << 1;
		newbyte |= (inverted_oldbyte & 0x04) >> 1;
		newbyte |= (oldbyte & 0x02) << 6;
		newbyte |= (inverted_oldbyte & 0x01) << 2;

		RAM[mem + 0x10000] = newbyte;
	}

	return;
}


static void acitya_decrypt_rom_9(void)
{
	int oldbyte,inverted_oldbyte,newbyte;
	int mem;
	UINT8 *RAM;

	RAM = memory_region(REGION_CPU1);

	for (mem=0;mem<0x4000;mem++)
	{
		oldbyte = RAM[mem];
		inverted_oldbyte = ~oldbyte;

		/*  Note: D2 is inverted and connected to D1, D5 is inverted and
            connected to D0.  The other six data bits are converted by a
            PAL10H8 driven by the counter. */
		newbyte = 0;

		/* Direct inversion */
		newbyte  = (inverted_oldbyte & 0x80) >> 0;
		newbyte |= (oldbyte & 0x40) >> 0;
		newbyte |= (inverted_oldbyte & 0x20) >> 5;
		newbyte |= (oldbyte & 0x10) >> 1;
		newbyte |= (oldbyte & 0x08) << 1;
		newbyte |= (inverted_oldbyte & 0x04) >> 1;
		newbyte |= (inverted_oldbyte & 0x02) << 4;
		newbyte |= (inverted_oldbyte & 0x01) << 2;
		RAM[mem + 0x14000] = newbyte;
	}

	return;
}

static void acitya_decrypt_rom_A(void)
{
	int oldbyte,inverted_oldbyte,newbyte;
	int mem;
	UINT8 *RAM;

	RAM = memory_region(REGION_CPU1);

	for (mem=0;mem<0x4000;mem++)
	{
		oldbyte = RAM[mem];
		inverted_oldbyte = ~oldbyte;

		/*  Note: D2 is inverted and connected to D1, D5 is inverted and
            connected to D0.  The other six data bits are converted by a
            PAL10H8 driven by the counter. */
		newbyte = 0;

		newbyte  = (inverted_oldbyte & 0x80) >> 2;
		newbyte |= (inverted_oldbyte & 0x40) >> 2;
		newbyte |= (inverted_oldbyte & 0x20) >> 5;
		newbyte |= (inverted_oldbyte & 0x10) >> 1;

		newbyte |= (inverted_oldbyte & 0x08) >> 1;
		newbyte |= (inverted_oldbyte & 0x04) >> 1;
		newbyte |= (oldbyte & 0x02) << 6;
		newbyte |= (oldbyte & 0x01) << 6;
		RAM[mem + 0x18000] = newbyte;
	}

	return;
}

static void acitya_decrypt_rom_B(void)
{
	int oldbyte,inverted_oldbyte,newbyte;
	int mem;
	UINT8 *RAM;

	RAM = memory_region(REGION_CPU1);

	for (mem=0;mem<0x4000;mem++)
	{
		oldbyte = RAM[mem];
		inverted_oldbyte = ~oldbyte;

		/*  Note: D2 is inverted and connected to D1, D5 is inverted and
            connected to D0.  The other six data bits are converted by a
            PAL10H8 driven by the counter. */
		newbyte = 0;

		/* Direct inversion */
		newbyte  = (inverted_oldbyte & 0x80) >> 0;
		newbyte |= (inverted_oldbyte & 0x40) >> 2;
		newbyte |= (inverted_oldbyte & 0x20) >> 5;
		newbyte |= (oldbyte & 0x10) >> 1;
		newbyte |= (inverted_oldbyte & 0x08) >> 1;
		newbyte |= (inverted_oldbyte & 0x04) >> 1;
		newbyte |= (inverted_oldbyte & 0x02) << 4;
		newbyte |= (oldbyte & 0x01) << 6;


		RAM[mem + 0x1C000] = newbyte;
	}

	return;
}


READ8_HANDLER( acitya_decrypt_rom )
{
	if (offset & 0x01)
	{
		counter = counter - 1;
		if (counter < 0)
			counter = 0x0F;
	}
	else
	{
		counter = (counter + 1) & 0x0F;
	}

	switch(counter)
	{
		case 0x08:	memory_set_bank (1, 0);		break;
		case 0x09:	memory_set_bank (1, 1);		break;
		case 0x0A:	memory_set_bank (1, 2);		break;
		case 0x0B:	memory_set_bank (1, 3);		break;
		default:
			logerror("Invalid counter = %02X\n",counter);
			break;
	}

	return 0;
}


MACHINE_RESET( acitya )
{
	UINT8 *RAM = memory_region(REGION_CPU1);

	/* While the PAL supports up to 16 decryption methods, only four
        are actually used in the PAL.  Therefore, we'll take a little
        memory overhead and decrypt the ROMs using each method in advance. */
	acitya_decrypt_rom_8();
	acitya_decrypt_rom_9();
	acitya_decrypt_rom_A();
	acitya_decrypt_rom_B();

	/* The initial state of the counter is 0x0B */
	counter = 0x0B;
	memory_configure_bank(1, 0, 4, &RAM[0x10000], 0x4000);
	memory_set_bank(1, 3);

	state_save_register_global(counter);
}
