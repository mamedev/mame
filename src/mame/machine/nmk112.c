/*  NMK112 - NMK custom IC for bankswitching the sample ROMs of a pair of
    OKI6295 ADPCM chips

    The address space of each OKI6295 is divided into four banks, each one
    independently controlled. The sample table at the beginning of the
    address space may be divided in four pages as well, banked together
    with the sample data.  This allows each of the four voices on the chip
    to play a sample from a different bank at the same time. */

#include "driver.h"
#include "nmk112.h"

#define MAXCHIPS 2
#define TABLESIZE 0x100
#define BANKSIZE 0x10000

/* which chips have their sample address table divided into pages */
static UINT8 page_mask;

static UINT8 current_bank[8];

void NMK112_init(UINT8 disable_page_mask)
{
	memset(current_bank, ~0, sizeof(current_bank));
	page_mask = ~disable_page_mask;
}

WRITE8_HANDLER( NMK112_okibank_w )
{
	int chip	=	(offset & 4) >> 2;
	int banknum	=	offset & 3;
	int paged	=	(page_mask & (1 << chip));

	UINT8 *rom	=	memory_region(REGION_SOUND1 + chip);
	int size			=	memory_region_length(REGION_SOUND1 + chip) - 0x40000;
	int bankaddr		=	(data * BANKSIZE) % size;

	if (current_bank[offset] == data) return;
	current_bank[offset] = data;

	/* copy the samples */
	if ((paged) && (banknum == 0))
		memcpy(rom + 0x400, rom + 0x40000 + bankaddr+0x400, BANKSIZE-0x400);
	else
		memcpy(rom + banknum * BANKSIZE, rom + 0x40000 + bankaddr, BANKSIZE);

	/* also copy the sample address table, if it is paged on this chip */
	if (paged)
	{
		rom += banknum * TABLESIZE;
		memcpy(rom, rom + 0x40000 + bankaddr, TABLESIZE);
	}
}

WRITE16_HANDLER( NMK112_okibank_lsb_w )
{
	if (ACCESSING_LSB)
	{
		NMK112_okibank_w(machine, offset, data & 0xff);
	}
}
