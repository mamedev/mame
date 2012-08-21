/*
    spchroms.c - This is an emulator for "typical" speech ROMs from TI, as used by TI99/4(a).

    In order to support its speech processor, TI designed some ROMs with a 1-bit data bus
    and 4-bit address bus (multiplexed 5 times to provide a 18-bit address).
    A fairly complete description of such a ROM (tms6100) is found in the tms5220 datasheet.

    One notable thing is that the address is a byte address (*NOT* a bit address).

    This file is designed to be interfaced with the tms5220 core.
    Interfacing it with the tms5110 would make sense, too.
*/

#include "emu.h"
#include "spchroms.h"

static UINT8 *speechrom_data = NULL;	/* pointer to speech ROM data */
static unsigned long speechROMlen = 0;	/* length of data pointed by speechrom_data, from 0 to 2^18 */
static unsigned long speechROMaddr;		/* 18 bit pointer in ROM */
#define TMS5220_ADDRESS_MASK 0x3FFFFUL	/* 18-bit mask for tms5220 address */
static int load_pointer = 0;			/* which 4-bit nibble will be affected by load address */
static int ROM_bits_count;				/* current bit position in ROM */

/*
    set the speech ROMs
*/
void spchroms_config(running_machine &machine, const spchroms_interface *intf)
{
	if (intf->memory_region == NULL)
	{	/* no speech ROM */
		speechrom_data = NULL;
		speechROMlen = 0;
	}
	else
	{	/* speech ROM */
		speechrom_data = machine.root_device().memregion(intf->memory_region)->base();
		/* take region length */
		speechROMlen = machine.root_device().memregion(intf->memory_region)->bytes();
	}
}

/*
    Read 'count' bits serially from speech ROM
*/
int spchroms_read(device_t *device, int count)
{
	int val;

	if (load_pointer)
	{	/* first read after load address is ignored */
		load_pointer = 0;
		count--;
	}

	if (speechROMaddr < speechROMlen)
		if (count < ROM_bits_count)
		{
			ROM_bits_count -= count;
			val = (speechrom_data[speechROMaddr] >> ROM_bits_count) & (0xFF >> (8 - count));
		}
		else
		{
			val = ((int) speechrom_data[speechROMaddr]) << 8;

			speechROMaddr = (speechROMaddr + 1) & TMS5220_ADDRESS_MASK;

			if (speechROMaddr < speechROMlen)
				val |= speechrom_data[speechROMaddr];

			ROM_bits_count += 8 - count;

			val = (val >> ROM_bits_count) & (0xFF >> (8 - count));
		}
	else
		val = 0;

	return val;
}

/*
    Write an address nibble to speech ROM
*/
void spchroms_load_address(device_t *device, int data)
{
	/* tms5220 data sheet says that if we load only one 4-bit nibble, it won't work.
      This code does not care about this. */
	speechROMaddr = ( (speechROMaddr & ~(0xf << load_pointer))
		| (((unsigned long) (data & 0xf)) << load_pointer) ) & TMS5220_ADDRESS_MASK;
	load_pointer += 4;
	ROM_bits_count = 8;
}

/*
    Perform a read and branch command
*/
void spchroms_read_and_branch(device_t *device)
{
	/* tms5220 data sheet says that if more than one speech ROM (tms6100) is present,
      there is a bus contention.  This code does not care about this. */
	if (speechROMaddr < speechROMlen-1)
		speechROMaddr = (speechROMaddr & 0x3c000UL)
			| (((((unsigned long) speechrom_data[speechROMaddr]) << 8)
			| speechrom_data[speechROMaddr+1]) & 0x3fffUL);
	else if (speechROMaddr == speechROMlen-1)
		speechROMaddr = (speechROMaddr & 0x3c000UL)
			| ((((unsigned long) speechrom_data[speechROMaddr]) << 8) & 0x3fffUL);
	else
		speechROMaddr = (speechROMaddr & 0x3c000UL);

	ROM_bits_count = 8;
}
