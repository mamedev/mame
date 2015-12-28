// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 SuperAMS Memory Expansion Card. Uses a 74LS612 memory mapper.
    The card can be equipped with up to 1 MiB of static CMOS memory; it is
    not buffered, however.

    SAMS organizes memory in 4 KiB blocks which are mapped into the address
    space by a memory mapper. The mapper can be configured via a sequence of
    addresses at 4000, 4002, ..., 401e, which correspond to memory locations
    0000-0fff, 1000-1fff, ..., f000-ffff.

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#include "samsmem.h"
#define RAMREGION "ram"

#define SAMS_CRU_BASE 0x1e00

#define VERBOSE 1
#define LOG logerror

sams_memory_expansion_device::sams_memory_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: ti_expansion_card_device(mconfig, TI99_SAMSMEM, "SuperAMS memory expansion card", tag, owner, clock, "ti99_sams", __FILE__), m_ram(nullptr), m_map_mode(false), m_access_mapper(false)
{
}

/*
    Memory read. The SAMS card has two address areas: The memory is at locations
    0x2000-0x3fff and 0xa000-0xffff, and the mapper area is at 0x4000-0x401e
    (only even addresses).
*/
READ8Z_MEMBER(sams_memory_expansion_device::readz)
{
	int base;

	if (m_access_mapper && ((offset & 0xe000)==0x4000))
	{
		*value = m_mapper[(offset>>1)&0x000f];
	}

	if (((offset & 0xe000)==0x2000) || ((offset & 0xe000)==0xa000) || ((offset & 0xe000)==0xc000) || ((offset & 0xe000)==0xe000))
	{
		if (!m_map_mode)
		{
			// transparent mode
			*value = m_ram[offset & 0xffff];
		}
		else
		{
			base = (m_mapper[(offset & 0xf000)>>12] << 12);
			*value = m_ram[base | (offset & 0x0fff)];
		}
	}
}

WRITE8_MEMBER(sams_memory_expansion_device::write)
{
	int base;

	if (m_access_mapper && ((offset & 0xe000)==0x4000))
	{
		m_mapper[(offset>>1)&0x000f] = data;
	}

	if (((offset & 0xe000)==0x2000) || ((offset & 0xe000)==0xa000) || ((offset & 0xe000)==0xc000) || ((offset & 0xe000)==0xe000))
	{
		if (!m_map_mode)
		{
			// transparent mode
			m_ram[offset & 0xffff] = data;
		}
		else
		{
			base = (m_mapper[(offset & 0xf000)>>12] << 12);
			m_ram[base | (offset & 0x0fff)] = data;
		}
	}
}

/*
    CRU read. None here.
*/
READ8Z_MEMBER(sams_memory_expansion_device::crureadz)
{
}

/*
    CRU write. Turns on the mapper and allows to change it.
*/
WRITE8_MEMBER(sams_memory_expansion_device::cruwrite)
{
	if ((offset & 0xff00)==SAMS_CRU_BASE)
	{
		if (VERBOSE>7) LOG("cru address %04x = %02x\n", offset&0xffff, data);

		if ((offset & 0x000e)==0) m_access_mapper = (data!=0);
		if ((offset & 0x000e)==2) m_map_mode = (data!=0);
	}
}


ROM_START( sams_card )
	ROM_REGION(0x100000, RAMREGION, 0)
	ROM_FILL(0x0000, 0x100000, 0x00)
ROM_END

void sams_memory_expansion_device::device_start()
{
	if (VERBOSE>5) LOG("SuperAMS: start\n");
	m_ram = memregion(RAMREGION)->base();
}

void sams_memory_expansion_device::device_reset()
{
	if (VERBOSE>5) LOG("SuperAMS: reset\n");
	// Resetting values
	m_map_mode = false;
	m_access_mapper = false;
	for (auto & elem : m_mapper) elem = 0;
}

const rom_entry *sams_memory_expansion_device::device_rom_region() const
{
	return ROM_NAME( sams_card );
}

const device_type TI99_SAMSMEM = &device_creator<sams_memory_expansion_device>;
