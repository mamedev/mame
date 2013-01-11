/***************************************************************************

    TI-99/8 Address decoder and mapper

    This component implements the address decoder and mapper logic from the
    TI-99/8 console.

    The TI-99/8 defines a "logical address map" with 64 KiB (according to the
    16 address bits) and a "physical address map" with 16 MiB (according to the
    24 address bits of the mapper). Note that the mapper only uses 16 outgoing
    address lines and multiplexes the address bytes.

    Note: The TI-99/8's internal codename was "Armadillo"

    Initial setting of mapper (as defined in the power-up routine, TI-99/4A mode)

    0   00ff0000 -> Unmapped; logical address 0000...0fff = ROM0
    1   00ff0000 -> Unmapped; logical address 1000...1fff = ROM0
    2   00000800 -> DRAM; 2000 = 000800, 2fff = 0017ff
    3   00001800 -> DRAM; 3000 = 001800, 3fff = 0027ff
    4   00ff4000 -> DSR space (internal / ioport)
    5   00ff5000 -> DSR space (internal / ioport)
    6   00ff6000 -> Cartridge space (6000..6fff)
    7   00ff7000 -> Cartridge space (7000..7fff)
    8   00ff0000 -> Unmapped; device ports (VDP) and SRAM
    9   00ff0000 -> Unmapped; device ports (Speech, GROM)
    A   00002800 -> DRAM; a000 = 002800, afff = 0037ff
    B   00003800 -> DRAM; b000 = 003800, bfff = 0047ff
    C   00004800 -> DRAM; c000 = 004800, cfff = 0057ff
    D   00005800 -> DRAM; d000 = 005800, dfff = 0067ff
    E   00006800 -> DRAM; e000 = 006800, efff = 0077ff
    F   00007800 -> DRAM; f000 = 007800, ffff = 0087ff

    Informations taken from
    [1] ARMADILLO PRODUCT SPECIFICATIONS
    [2] TI-99/8 Graphics Programming Language interpreter

    Michael Zapf, October 2010
    February 2012: Rewritten as class

***************************************************************************/

#include "mapper8.h"

#define VERBOSE 0
#define LOG logerror

// Pseudo devices which are not implemented by a proper device. We use these
// constants in the read/write functions.
#define MAP8_SRAM (void*)1L
#define MAP8_ROM0 (void*)2L
#define MAP8_ROM1 (void*)3L
#define MAP8_ROM1A (void*)4L
#define MAP8_DRAM (void*)5L
#define MAP8_INTS (void*)6L

ti998_mapper_device::ti998_mapper_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: bus8z_device(mconfig, MAPPER8, "TI-99/8 Memory mapper", tag, owner, clock)
{
}

/***************************************************************************
    CRU access
***************************************************************************/

#define MAPPER_CRU_BASE 0x2700

void ti998_mapper_device::crureadz(offs_t offset, UINT8 *value)
{
	if (VERBOSE>8) LOG("mapper8: read CRU %04x ignored\n", offset);
	// Nothing here.
}

/*
    CRU handling. We handle the internal device at CRU address 0x2700 via
    this mapper component.
*/
void ti998_mapper_device::cruwrite(offs_t offset, UINT8 data)
{
	if ((offset & 0xff00)==MAPPER_CRU_BASE)
	{
		int bit = (offset & 0xff)>>1;
		switch (bit)
		{
		case 0:
			// Turn on/off the internal DSR
			m_dsr_selected = (data!=0);
			if (VERBOSE>7) LOG("mapper8: DSR select = %d\n", data);
			break;
		case 1:
			if (VERBOSE>2) LOG("mapper8: System reset by CRU request\n");
			machine().schedule_soft_reset();
			break;
		}
	}
}

void ti998_mapper_device::CRUS_set(bool state)
{
	if (VERBOSE>7) LOG("mapper8: set CRUS=%d\n", state);
	m_CRUS = state;
}

/*
    Note that PTGEN is negative logic. We invert these semantics here.
*/
void ti998_mapper_device::PTGE_set(bool state)
{
	if (VERBOSE>7) LOG("mapper8: set PTGEN=%d\n", state? 1:0);
	m_PTGE = state;
}

/***************************************************************************
    Access by address map
***************************************************************************/

/*
    This method is called via the address map.
*/
READ8_MEMBER( ti998_mapper_device::readm )
{
	UINT8 value = 0;
	bool found = false;
	if (VERBOSE>5) LOG("mapper8: read from %04x\n", offset);
	found = search_logically_addressed_r(space, offset, &value, mem_mask);
	m_waitcount = 2;

	if (!found)
	{
		// In that case, the address decoder could not find a suitable device.
		// This means the logical address is transformed by the mapper.
		// NOTE: Use "+", not OR. The offset is not a prefix.
		UINT32  pas_address = m_pas_offset[(offset & 0xf000)>>12] + (offset & 0xfff);

		// So now let's do the same as above with physical addresses
		search_physically_addressed_r(space, pas_address, &value, mem_mask);

		// The PAS area requires one more wait state, as the address bus
		// is multiplexed
		m_waitcount = 3;
	}

	// Insert wait states and let CPU enter wait state
	m_ready(CLEAR_LINE);

	return value;
}

WRITE8_MEMBER( ti998_mapper_device::writem )
{
	bool found = false;

	// Look for components responding to the logical address
	found = search_logically_addressed_w(space, offset, data, mem_mask);
	m_waitcount = 2;

	if (!found)
	{
		// In that case, the address decoder could not find a suitable device.
		// This means the logical address is transformed by the mapper.
		// NOTE: Use "+", not OR. The offset is not a prefix.
		UINT32 pas_address = m_pas_offset[(offset & 0xf000)>>12] + (offset & 0xfff);

		// So now let's do the same as above with physical addresses
		search_physically_addressed_w(space, pas_address, data, mem_mask);

		// The PAS area requires one more wait state, as the address bus
		// is multiplexed
		m_waitcount = 3;
	}

	// Insert wait states and let CPU enter wait state
	m_ready(CLEAR_LINE);
}

/***************************************************************************
    Indirect calls (mapper calls itself)
***************************************************************************/
/*
    This method is called by the mapper itself for
    f870 (NATIVE): mapper: ignore
    8810 (TI99EM): mapper: ignore
    ff4000 (PHYSIC): DSR
*/
READ8Z_MEMBER( ti998_mapper_device::readz )
{
	if ((offset & 0xffe000)==0xff4000)
	{
		if (m_dsr_selected)
		{
			//  Starts at 0x4000 in the image
			*value = m_rom[0x4000 | (offset & 0x1fff)];
			if (VERBOSE>7) LOG("mapper8: (DSR)  %04x -> %02x\n", offset, *value);
		}
	}
	else
	{
		if (((offset & 0xfff0)==0xf870 && m_CRUS==false)||(((offset & 0xfff0)==0x8810 && m_CRUS==true)))
		{
			if (VERBOSE>4) LOG("mapper8: read access to mapper ignored: %04x\n", offset);
		}
	}
}

/*
    This method is called by the mapper itself for
    ff4000 (PHYSIC): DSR. ignore

*/
WRITE8_MEMBER( ti998_mapper_device::write )
{
	if ((offset & 0xffe000)==0xff4000)
	{
		if (VERBOSE>4) LOG("mapper8: Write access to DSR space %06x ignored\n", offset);
	}
	else
	{
		if (((offset & 0xfff0)==0xf870 && m_CRUS==false)||(((offset & 0xfff0)==0x8810 && m_CRUS==true)))
		{
			mapwrite(offset, data);
		}
	}
}

/*
    Reconfigure mapper. Writing to this address copies the values in the
    SRAM into the mapper and vice versa.
    Format:
    0000 bbbl; bbb=bank, l=load
*/
void ti998_mapper_device::mapwrite(int offset, UINT8 data)
{
	if ((data & 0xf0)==0x00)
	{
		int bankindx = (data & 0x0e)>>1;
		if (data & 1)
		{
			if (VERBOSE>7) LOG("mapper8: load mapper from SRAM, bank %d\n", bankindx);
			// Load from SRAM
			// In reality the CPU is put on HOLD during this transfer
			for (int i=0; i < 16; i++)
			{
				int ptr = (bankindx << 6);
				m_pas_offset[i] =   (m_sram[(i<<2) + ptr] << 24) | (m_sram[(i<<2)+ ptr+1] << 16)
				| (m_sram[(i<<2) + ptr+2] << 8) | (m_sram[(i<<2) + ptr+3]);
			}
		}
		else
		{
			if (VERBOSE>7) LOG("mapper8: store mapper to SRAM, bank %d\n", bankindx);
			// Store in SRAM
			for (int i=0; i < 16; i++)
			{
				int ptr = (bankindx << 6);
				m_sram[(i<<2) + ptr]    =  (m_pas_offset[i] >> 24)& 0xff;
				m_sram[(i<<2) + ptr +1] =  (m_pas_offset[i] >> 16)& 0xff;
				m_sram[(i<<2) + ptr +2] =  (m_pas_offset[i] >> 8)& 0xff;
				m_sram[(i<<2) + ptr +3] =  (m_pas_offset[i])& 0xff;
			}
		}
	}
}

/***************************************************************************
    Lookup methods.
***************************************************************************/

bool ti998_mapper_device::search_logically_addressed_r(address_space& space, offs_t offset, UINT8 *value, UINT8 mem_mask )
{
	bool found = false;
	log_addressed_device *dev = m_logcomp.first();

	if (VERBOSE>8) LOG("mapper8: offset=%04x; CRUS=%d, PTGEN=%d\n", offset, m_CRUS? 1:0, m_PTGE? 0:1);
	while (dev != NULL)
	{
		if (VERBOSE>5) LOG("mapper8: checking node=%s\n", dev->m_config->name);
		// Check the mode
		if (((dev->m_config->mode == NATIVE) && (m_CRUS==false))
			|| ((dev->m_config->mode == TI99EM) && (m_CRUS==true))
			|| ((dev->m_config->mode == PATGEN) && (m_PTGE==true)))
		{
			if ((offset & dev->m_config->address_mask)==dev->m_config->select_pattern)
			{
				if (dev->m_device == MAP8_SRAM)
				{
					*value = m_sram[offset & ~dev->m_config->address_mask];
					if (VERBOSE>7) LOG("mapper8: (SRAM) %04x -> %02x\n", offset, *value);
				}
				else
				{
					if (dev->m_device == MAP8_ROM0)
					{
						// Starts at 0000
						*value = m_rom[offset & ~dev->m_config->address_mask];
						if (VERBOSE>7) LOG("mapper8: (ROM)  %04x -> %02x\n", offset, *value);
					}
					else
					{
						// device
						bus8z_device *bdev = static_cast<bus8z_device*>(dev->m_device);
						bdev->readz(space, offset, value, mem_mask);
						if (VERBOSE>7) LOG("mapper8: (dev %s)  %04x -> %02x\n", dev->m_config->name, offset, *value);
					}
				}
				found = true;
				if (dev->m_config->stop==STOP) break;
			}
		}
		dev = dev->m_next;
	}
	return found;
}

bool ti998_mapper_device::search_logically_addressed_w(address_space& space, offs_t offset, UINT8 data, UINT8 mem_mask )
{
	bool found = false;
	log_addressed_device *dev = m_logcomp.first();

	while (dev != NULL)
	{
		// Check the mode
		if (((dev->m_config->mode == NATIVE) && (m_CRUS==false))
			|| ((dev->m_config->mode == TI99EM) && (m_CRUS==true))
			|| ((dev->m_config->mode == PATGEN) && (m_PTGE==true)))
		{

			if ((offset & dev->m_config->address_mask)==(dev->m_config->select_pattern | dev->m_config->write_select))
			{
				if (dev->m_device == MAP8_SRAM)
				{
					m_sram[offset & ~dev->m_config->address_mask] = data;
					if (VERBOSE>7) LOG("mapper8: (SRAM) %04x <- %02x\n", offset, data);
				}
				else
				{
					if (dev->m_device == MAP8_ROM0)
					{
						if (VERBOSE>7) LOG("mapper8: (ROM)  %04x <- %02x (ignored)\n", offset, data);
					}
					else
					{
						// device
						bus8z_device *bdev = static_cast<bus8z_device*>(dev->m_device);
						bdev->write(space, offset, data, mem_mask);
						if (VERBOSE>7) LOG("mapper8: (dev %s)  %04x <- %02x\n", dev->m_config->name, offset, data);
					}
				}
				found = true;
				if (dev->m_config->stop==STOP) break;
			}
		}
		dev = dev->m_next;
	}
	return found;
}


void ti998_mapper_device::search_physically_addressed_r( address_space& space, offs_t pas_address, UINT8 *value, UINT8 mem_mask )
{
	phys_addressed_device *dev = m_physcomp.first();

	while (dev != NULL)
	{
		if ((pas_address & dev->m_config->address_mask)==dev->m_config->select_pattern)
		{
			if (dev->m_device == MAP8_DRAM)
			{
				*value = m_dram[pas_address & ~dev->m_config->address_mask];
				if (VERBOSE>3) LOG("mapper8: (DRAM) %06x -> %02x\n", pas_address, *value);
			}
			else
			{
				if (dev->m_device == MAP8_ROM1)
				{
					// Starts at 2000 in the image, 8K
					*value = m_rom[0x2000 | (pas_address & 0x1fff)];
					if (VERBOSE>3) LOG("mapper8: (ROM) %06x -> %02x\n", pas_address, *value);
				}
				else
				{
					if (dev->m_device == MAP8_ROM1A)
					{
						// Starts at 6000 in the image, 8K
						*value = m_rom[0x6000 | (pas_address & 0x1fff)];
						if (VERBOSE>3) LOG("mapper8: (ROM)  %06x -> %02x\n", pas_address, *value);
					}
					else
					{
						if (dev->m_device == MAP8_INTS)
						{
							// Interrupt sense
							if (VERBOSE>1) LOG("ti99_8: ILSENSE not implemented.\n");
						}
						else
						{
							// devices
							bus8z_device *bdev = static_cast<bus8z_device*>(dev->m_device);
							bdev->readz(space, pas_address, value, mem_mask);
							if (VERBOSE>7) LOG("mapper8: (dev %s)  %06x -> %02x\n", dev->m_config->name, pas_address, *value);
						}
					}
				}
			}
			if (dev->m_config->stop==STOP) break;
		}
		dev = dev->m_next;
	}
}

void ti998_mapper_device::search_physically_addressed_w( address_space& space, offs_t pas_address, UINT8 data, UINT8 mem_mask )
{
	phys_addressed_device *dev = m_physcomp.first();

	while (dev != NULL)
	{
		if ((pas_address & dev->m_config->address_mask)==(dev->m_config->select_pattern | dev->m_config->write_select))
		{
			if (dev->m_device == MAP8_DRAM)
			{
				m_dram[pas_address & ~dev->m_config->address_mask] = data;
				if (VERBOSE>3) LOG("mapper8: (DRAM) %06x <- %02x\n", pas_address, data);
			}
			else
			{
				if (dev->m_device == MAP8_ROM1 || dev->m_device == MAP8_ROM1A)
				{
					if (VERBOSE>7) LOG("mapper8: (ROM)  %06x <- %02x (ignored)\n", pas_address, data);
				}
				else
				{
					if (dev->m_device == MAP8_INTS)
					{
						// Interrupt sense
						if (VERBOSE>1) LOG("ti99_8: write to ilsense ignored\n");
					}
					else
					{
						// devices
						bus8z_device *bdev = static_cast<bus8z_device*>(dev->m_device);
						if (VERBOSE>7) LOG("mapper8: (dev %s)  %06x <- %02x\n", dev->m_config->name, pas_address, data);
						bdev->write(space, pas_address, data, mem_mask);
					}
				}
			}
			if (dev->m_config->stop==STOP) break;
		}
		dev = dev->m_next;
	}
}

/*
    The mapper is connected to the clock line in order to operate
    the wait state counter.
*/
void ti998_mapper_device::clock_in(int clock)
{
	if (clock==ASSERT_LINE && m_waitcount!=0)
	{
		m_waitcount--;
		if (m_waitcount==0) m_ready(ASSERT_LINE);
	}
}


/***************************************************************************
    DEVICE LIFECYCLE FUNCTIONS
***************************************************************************/
// String values of the pseudo constants, used in the configuration.
static const char *const pseudodev[6] = { SRAMNAME, ROM0NAME, ROM1NAME, ROM1ANAME, DRAMNAME, INTSNAME };

/*
    We need to do all of the configuration in device_start since we don't have all
    required links earlier.

    Note that device_reset is too late; the initial context switch occurs earlier.
*/
void ti998_mapper_device::device_start()
{
	if (VERBOSE>5) LOG("ti99_8: Starting mapper\n");

	const mapper8_config *conf = reinterpret_cast<const mapper8_config *>(static_config());

	const mapper8_list_entry *entry = conf->devlist;
	m_ready.resolve(conf->ready, *this);

	m_sram = machine().root_device().memregion(SRAM_TAG)->base();
	m_dram = machine().root_device().memregion(DRAM_TAG)->base();
	m_rom  = machine().root_device().memregion("maincpu")->base();

	// Clear the lists
	m_logcomp.reset();
	m_physcomp.reset();

	// Now building the list of active devices at this mapper.
	// Coyping partly from datamux.c.
	if ( entry != NULL )
	{
		bool done = false;
		for (int i=0; !done; i++)
		{
			if (entry[i].name == NULL)
			{
				done = true;
			}
			else
			{
				// Here we replace the device names by their pointers and the pseudo device
				// names by their constants MAP8_SRAM ... MAP8_INTS. */

				// To make thing efficient, we abuse the pointers with special
				// values for the ROM and RAM. (Otherwise we would have to
				// build complete devices for ROM and RAM.)
				void *dev = NULL;
				for (long long j=1; (j < 7) && (dev == NULL); j++)
				{
					// Pseudo devices are enumerated as 1 ... 6 (see MAP8_SRAM etc.)
					if (strcmp(entry[i].name, pseudodev[j-1])==0)
						dev = (void *)j;
				}
				if (dev==NULL)
				{
					// This entry points to a "real" device, not to a special constant
					dev = machine().device(entry[i].name);
				}
				if (dev != NULL)
				{
					if (entry[i].mode != PHYSIC)
					{
						log_addressed_device *ad = new log_addressed_device((device_t*)dev, entry[i]);
						m_logcomp.append(*ad);
						if (VERBOSE>6) LOG("mapper8: Device %s mounted into logical address space.\n", entry[i].name);
					}
					else
					{
						phys_addressed_device *ad = new phys_addressed_device((device_t*)dev, entry[i]);
						m_physcomp.append(*ad);
						if (VERBOSE>6) LOG("mapper8: Device %s mounted into physical address space.\n", entry[i].name);
					}
				}
				else
				{
					if (VERBOSE>1) LOG("mapper8: Device %s not found.\n", entry[i].name);
				}
			}
		}
	}
	if (VERBOSE>4) LOG("Mapper logical device count = %d\n", m_logcomp.count());
	if (VERBOSE>4) LOG("Mapper physical device count = %d\n", m_physcomp.count());

	m_dsr_selected = false;
	m_CRUS = true;
	m_PTGE = false;

	// Clean mapper
	for (int i=0; i < 16; i++) m_pas_offset[i] = 0;
}

void ti998_mapper_device::device_reset()
{
	m_dsr_selected = false;
	m_CRUS = true;
	m_PTGE = false;

	// Clean mapper
	for (int i=0; i < 16; i++) m_pas_offset[i] = 0;

	m_ready(ASSERT_LINE);
}

const device_type MAPPER8 = &device_creator<ti998_mapper_device>;
