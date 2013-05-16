/*
    spchroms.c - This is an emulator for "typical" speech ROMs from TI, as used by TI99/4(a).

    In order to support its speech processor, TI designed some ROMs with a 1-bit data bus
    and 4-bit address bus (multiplexed 5 times to provide a 18-bit address).
    A fairly complete description of such a ROM (tms6100) is found in the tms5220 datasheet.

    One notable thing is that the address is a byte address (*NOT* a bit address).

    This file is designed to be interfaced with the tms5220 core.
    Interfacing it with the tms5110 would make sense, too.

	TODO:
        Create seperate devices for TMS6100 & TMS6125
        Implement the serial protocol
*/

#include "emu.h"
#include "spchrom.h"

#define TMS5220_ADDRESS_MASK 0x3FFFFUL  /* 18-bit mask for tms5220 address */

// device type definition
const device_type SPEECHROM = &device_creator<speechrom_device>;

speechrom_device::speechrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SPEECHROM, "SPEECHROM", tag, owner, clock),
	m_speechROMaddr(0),
	m_load_pointer(0),
	m_ROM_bits_count(0)
{
}

/*
    Read 'count' bits serially from speech ROM
*/
int speechrom_device::read(int count)
{
	int val;

	if (m_load_pointer)
	{   /* first read after load address is ignored */
		m_load_pointer = 0;
		count--;
	}

	if (m_speechROMaddr < m_speechROMlen)
		if (count < m_ROM_bits_count)
		{
			m_ROM_bits_count -= count;
			val = (m_speechrom_data[m_speechROMaddr] >> m_ROM_bits_count) & (0xFF >> (8 - count));
		}
		else
		{
			val = ((int) m_speechrom_data[m_speechROMaddr]) << 8;

			m_speechROMaddr = (m_speechROMaddr + 1) & TMS5220_ADDRESS_MASK;

			if (m_speechROMaddr < m_speechROMlen)
				val |= m_speechrom_data[m_speechROMaddr];

			m_ROM_bits_count += 8 - count;

			val = (val >> m_ROM_bits_count) & (0xFF >> (8 - count));
		}
	else
		val = 0;

	return val;
}

/*
    Write an address nibble to speech ROM
*/
void speechrom_device::load_address(int data)
{
	/* tms5220 data sheet says that if we load only one 4-bit nibble, it won't work.
	  This code does not care about this. */
	m_speechROMaddr = ( (m_speechROMaddr & ~(0xf << m_load_pointer))
		| (((unsigned long) (data & 0xf)) << m_load_pointer) ) & TMS5220_ADDRESS_MASK;
	m_load_pointer += 4;
	m_ROM_bits_count = 8;
}

/*
    Perform a read and branch command
*/
void speechrom_device::read_and_branch()
{
	/* tms5220 data sheet says that if more than one speech ROM (tms6100) is present,
	  there is a bus contention.  This code does not care about this. */
	if (m_speechROMaddr < m_speechROMlen-1)
		m_speechROMaddr = (m_speechROMaddr & 0x3c000UL)
			| (((((unsigned long) m_speechrom_data[m_speechROMaddr]) << 8)
			| m_speechrom_data[m_speechROMaddr+1]) & 0x3fffUL);
	else if (m_speechROMaddr == m_speechROMlen-1)
		m_speechROMaddr = (m_speechROMaddr & 0x3c000UL)
			| ((((unsigned long) m_speechrom_data[m_speechROMaddr]) << 8) & 0x3fffUL);
	else
		m_speechROMaddr = (m_speechROMaddr & 0x3c000UL);

	m_ROM_bits_count = 8;
}

void speechrom_device::device_start()
{
	memory_region *region = memregion(tag());
	if (region == NULL)
	{
		throw emu_fatalerror("No region for device '%s'\n", tag());
	}

	m_speechrom_data = region->base();
	m_speechROMlen = region->bytes();

	save_item(NAME(m_speechROMaddr));
	save_item(NAME(m_load_pointer));
	save_item(NAME(m_ROM_bits_count));
}
