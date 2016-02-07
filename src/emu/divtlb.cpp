// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    divtlb.c

    Device generic virtual TLB interface.

***************************************************************************/

#include "emu.h"
#include "divtlb.h"
#include "validity.h"



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define PRINTF_TLB          (0)



//**************************************************************************
//  DEVICE VTLB INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_vtlb_interface - constructor
//-------------------------------------------------

device_vtlb_interface::device_vtlb_interface(const machine_config &mconfig, device_t &device, address_spacenum space)
	: device_interface(device, "vtlb"),
		m_space(space),
		m_dynamic(0),
		m_fixed(0),
		m_dynindex(0),
		m_pageshift(0),
		m_addrwidth(0)
{
}


//-------------------------------------------------
//  device_vtlb_interface - destructor
//-------------------------------------------------

device_vtlb_interface::~device_vtlb_interface()
{
}


//-------------------------------------------------
//  interface_validity_check - validation for a
//  device after the configuration has been
//  constructed
//-------------------------------------------------

void device_vtlb_interface::interface_validity_check(validity_checker &valid) const
{
	const device_memory_interface *intf;
	if (!device().interface(intf))
		osd_printf_error("Device does not have memory interface\n");
	else
	{
		// validate CPU information
		const address_space_config *spaceconfig = intf->space_config(m_space);
		if (spaceconfig == nullptr)
			osd_printf_error("No memory address space configuration found for space %d\n", m_space);
		else if ((1 << spaceconfig->m_page_shift) <= VTLB_FLAGS_MASK || spaceconfig->m_logaddr_width <= spaceconfig->m_page_shift)
			osd_printf_error("Invalid page shift %d for VTLB\n", spaceconfig->m_page_shift);
	}
}


//-------------------------------------------------
//  interface_pre_start - work to be done prior to
//  actually starting a device
//-------------------------------------------------

void device_vtlb_interface::interface_pre_start()
{
	// fill in CPU information
	const address_space_config *spaceconfig = device().memory().space_config(m_space);
	m_pageshift = spaceconfig->m_page_shift;
	m_addrwidth = spaceconfig->m_logaddr_width;

	// allocate the entry array
	m_live.resize(m_fixed + m_dynamic);
	memset(&m_live[0], 0, m_live.size()*sizeof(m_live[0]));

	// allocate the lookup table
	m_table.resize((size_t) 1 << (m_addrwidth - m_pageshift));
	memset(&m_table[0], 0, m_table.size()*sizeof(m_table[0]));

	// allocate the fixed page count array
	if (m_fixed > 0)
	{
		m_fixedpages.resize(m_fixed);
		memset(&m_fixedpages[0], 0, m_fixed*sizeof(m_fixedpages[0]));
	}
}


//-------------------------------------------------
//  interface_post_start - work to be done after
//  actually starting a device
//-------------------------------------------------

void device_vtlb_interface::interface_post_start()
{
	device().save_item(NAME(m_live));
	device().save_item(NAME(m_table));
	if (m_fixed > 0)
		device().save_item(NAME(m_fixedpages));
}


//-------------------------------------------------
//  interface_pre_reset - work to be done prior to
//  actually resetting a device
//-------------------------------------------------

void device_vtlb_interface::interface_pre_reset()
{
	vtlb_flush_dynamic();
}


//**************************************************************************
//  FILLING
//**************************************************************************

//-------------------------------------------------
//  vtlb_fill - called by the CPU core in
//  response to an unmapped access
//-------------------------------------------------

int device_vtlb_interface::vtlb_fill(offs_t address, int intention)
{
	offs_t tableindex = address >> m_pageshift;
	vtlb_entry entry = m_table[tableindex];
	offs_t taddress;

#if PRINTF_TLB
	osd_printf_debug("vtlb_fill: %08X(%X) ... ", address, intention);
#endif

	// should not be called here if the entry is in the table already
//  assert((entry & (1 << intention)) == 0);

	// if we have no dynamic entries, we always fail
	if (m_dynamic == 0)
	{
#if PRINTF_TLB
		osd_printf_debug("failed: no dynamic entries\n");
#endif
		return FALSE;
	}

	// ask the CPU core to translate for us
	taddress = address;
	if (!device().memory().translate(m_space, intention, taddress))
	{
#if PRINTF_TLB
		osd_printf_debug("failed: no translation\n");
#endif
		return FALSE;
	}

	// if this is the first successful translation for this address, allocate a new entry
	if ((entry & VTLB_FLAGS_MASK) == 0)
	{
		int liveindex = m_dynindex++ % m_dynamic;

		// if an entry already exists at this index, free it
		if (m_live[liveindex] != 0)
			m_table[m_live[liveindex] - 1] = 0;

		// claim this new entry
		m_live[liveindex] = tableindex + 1;

		// form a new blank entry
		entry = (taddress >> m_pageshift) << m_pageshift;
		entry |= VTLB_FLAG_VALID;

#if PRINTF_TLB
		osd_printf_debug("success (%08X), new entry\n", taddress);
#endif
	}

	// otherwise, ensure that different intentions do not produce different addresses
	else
	{
		assert((entry >> m_pageshift) == (taddress >> m_pageshift));
		assert(entry & VTLB_FLAG_VALID);

#if PRINTF_TLB
		osd_printf_debug("success (%08X), existing entry\n", taddress);
#endif
	}

	// add the intention to the list of valid intentions and store
	entry |= 1 << (intention & (TRANSLATE_TYPE_MASK | TRANSLATE_USER_MASK));
	m_table[tableindex] = entry;
	return TRUE;
}


//-------------------------------------------------
//  vtlb_load - load a fixed VTLB entry
//-------------------------------------------------

void device_vtlb_interface::vtlb_load(int entrynum, int numpages, offs_t address, vtlb_entry value)
{
	offs_t tableindex = address >> m_pageshift;
	int liveindex = m_dynamic + entrynum;
	int pagenum;

	// must be in range
	assert(entrynum >= 0 && entrynum < m_fixed);

#if PRINTF_TLB
	osd_printf_debug("vtlb_load %d for %d pages at %08X == %08X\n", entrynum, numpages, address, value);
#endif

	// if an entry already exists at this index, free it
	if (m_live[liveindex] != 0)
	{
		int pagecount = m_fixedpages[entrynum];
		int oldtableindex = m_live[liveindex] - 1;
		for (pagenum = 0; pagenum < pagecount; pagenum++)
			m_table[oldtableindex + pagenum] = 0;
	}

	// claim this new entry
	m_live[liveindex] = tableindex + 1;

	// store the raw value, making sure the "fixed" flag is set
	value |= VTLB_FLAG_FIXED;
	m_fixedpages[entrynum] = numpages;
	for (pagenum = 0; pagenum < numpages; pagenum++)
		m_table[tableindex + pagenum] = value + (pagenum << m_pageshift);
}

//-------------------------------------------------
//  vtlb_dynload - load a dynamic VTLB entry
//-------------------------------------------------

void device_vtlb_interface::vtlb_dynload(UINT32 index, offs_t address, vtlb_entry value)
{
	vtlb_entry entry = m_table[index];

	if (m_dynamic == 0)
	{
#if PRINTF_TLB
		osd_printf_debug("failed: no dynamic entries\n");
#endif
		return;
	}

	int liveindex = m_dynindex++ % m_dynamic;
	// is entry already live?
	if (!(entry & VTLB_FLAG_VALID))
	{
		// if an entry already exists at this index, free it
		if (m_live[liveindex] != 0)
			m_table[m_live[liveindex] - 1] = 0;

		// claim this new entry
		m_live[liveindex] = index + 1;
	}
	// form a new blank entry
	entry = (address >> m_pageshift) << m_pageshift;
	entry |= VTLB_FLAG_VALID | value;

#if PRINTF_TLB
	osd_printf_debug("success (%08X), new entry\n", address);
#endif
	m_table[index] = entry;
}

//**************************************************************************
//  FLUSHING
//**************************************************************************

//-------------------------------------------------
//  vtlb_flush_dynamic - flush all knowledge
//  from the dynamic part of the VTLB
//-------------------------------------------------

void device_vtlb_interface::vtlb_flush_dynamic()
{
#if PRINTF_TLB
	osd_printf_debug("vtlb_flush_dynamic\n");
#endif

	// loop over live entries and release them from the table
	for (int liveindex = 0; liveindex < m_dynamic; liveindex++)
		if (m_live[liveindex] != 0)
		{
			offs_t tableindex = m_live[liveindex] - 1;
			m_table[tableindex] = 0;
			m_live[liveindex] = 0;
		}
}


//-------------------------------------------------
//  vtlb_flush_address - flush knowledge of a
//  particular address from the VTLB
//-------------------------------------------------

void device_vtlb_interface::vtlb_flush_address(offs_t address)
{
	offs_t tableindex = address >> m_pageshift;

#if PRINTF_TLB
	osd_printf_debug("vtlb_flush_address %08X\n", address);
#endif

	// free the entry in the table; for speed, we leave the entry in the live array
	m_table[tableindex] = 0;
}



//**************************************************************************
//  ACCESSORS
//**************************************************************************

//-------------------------------------------------
//  vtlb_table - return a pointer to the base of
//  the linear VTLB lookup table
//-------------------------------------------------

const vtlb_entry *device_vtlb_interface::vtlb_table() const
{
	return &m_table[0];
}
