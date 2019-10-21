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

    According to a software distribution disk from the South West 99ers group,
    the predecessor of this card was the Asgard Expanded Memory System (AEMS).
    Although some documentation and software was available for it, it was never
    built. Instead, a simpler memory card called the Asgard Memory System (AMS)
    was built. The South West 99ers group built a better version of this card
    called the Super AMS. Any documentation and software containing a reference
    to the AEMS are applicable to either AMS or SAMS.

    Michael Zapf

*****************************************************************************/

#include "emu.h"
#include "samsmem.h"

DEFINE_DEVICE_TYPE_NS(TI99_SAMSMEM, bus::ti99::peb, sams_memory_expansion_device, "ti99_sams", "SuperAMS memory expansion card")

namespace bus { namespace ti99 { namespace peb {

#define SAMS_CRU_BASE 0x1e00

sams_memory_expansion_device::sams_memory_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TI99_SAMSMEM, tag, owner, clock),
	device_ti99_peribox_card_interface(mconfig, *this),
	m_ram(*this, RAM_TAG),
	m_crulatch(*this, "crulatch"),
	m_map_mode(false), m_access_mapper(false)
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
			*value = m_ram->pointer()[offset & 0xffff];
		}
		else
		{
			base = (m_mapper[(offset & 0xf000)>>12] << 12);
			*value = m_ram->pointer()[base | (offset & 0x0fff)];
		}
	}
}

void sams_memory_expansion_device::write(offs_t offset, uint8_t data)
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
			m_ram->pointer()[offset & 0xffff] = data;
		}
		else
		{
			base = (m_mapper[(offset & 0xf000)>>12] << 12);
			m_ram->pointer()[base | (offset & 0x0fff)] = data;
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
void sams_memory_expansion_device::cruwrite(offs_t offset, uint8_t data)
{
	if ((offset & 0xff00)==SAMS_CRU_BASE)
		m_crulatch->write_bit((offset & 0x000e) >> 1, data);
}

WRITE_LINE_MEMBER(sams_memory_expansion_device::access_mapper_w)
{
	m_access_mapper = state;
}

WRITE_LINE_MEMBER(sams_memory_expansion_device::map_mode_w)
{
	m_map_mode = state;
}

void sams_memory_expansion_device::device_add_mconfig(machine_config &config)
{
	RAM(config, RAM_TAG).set_default_size("1M").set_default_value(0);

	LS259(config, m_crulatch); // U8
	m_crulatch->q_out_cb<0>().set(FUNC(sams_memory_expansion_device::access_mapper_w));
	m_crulatch->q_out_cb<1>().set(FUNC(sams_memory_expansion_device::map_mode_w));
}

void sams_memory_expansion_device::device_start()
{
	save_item(NAME(m_mapper));
	save_item(NAME(m_map_mode));
	save_item(NAME(m_access_mapper));
}

void sams_memory_expansion_device::device_reset()
{
	// Resetting values
	for (auto & elem : m_mapper) elem = 0;
}

} } } // end namespace bus::ti99::peb
