// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    addrmap.c

    Macros and helper functions for handling address map definitions.

***************************************************************************/

#include "emu.h"
#include "validity.h"


//**************************************************************************
//  PARAMETERS
//**************************************************************************

#define DETECT_OVERLAPPING_MEMORY   (0)



//**************************************************************************
//  ADDRESS MAP ENTRY
//**************************************************************************

//-------------------------------------------------
//  address_map_entry - constructor
//-------------------------------------------------

address_map_entry::address_map_entry(device_t &device, address_map &map, offs_t start, offs_t end)
	: m_next(nullptr),
		m_map(map),
		m_devbase(device),
		m_addrstart(start),
		m_addrend(end),
		m_addrmirror(0),
		m_addrmask(0),
		m_addrselect(0),
		m_share(nullptr),
		m_region(nullptr),
		m_rgnoffs(0),
		m_submap_bits(0),
		m_memory(nullptr),
		m_bytestart(0),
		m_byteend(0),
		m_bytemirror(0),
		m_bytemask(0)
{
	if (map.m_globalmask != 0 && (start & ~map.m_globalmask) != 0)
	{
		osd_printf_warning("AS_%d map entry start %08X lies outside global address mask %08X\n", map.m_spacenum, start, map.m_globalmask);
		m_addrstart &= map.m_globalmask;
	}

	if (map.m_globalmask != 0 && (end & ~map.m_globalmask) != 0)
	{
		osd_printf_warning("AS_%d map entry end %08X lies outside global address mask %08X\n", map.m_spacenum, end, map.m_globalmask);
		m_addrend &= map.m_globalmask;
	}
}


//-------------------------------------------------
//  set_mask - set the mask value
//-------------------------------------------------

address_map_entry &address_map_entry::mask(offs_t _mask)
{
	m_addrmask = _mask;
	if (m_map.m_globalmask != 0)
		m_addrmask &= m_map.m_globalmask;
	return *this;
}


//-------------------------------------------------
//  set_submap - set up a handler for
//  retrieve a submap from a device
//-------------------------------------------------

address_map_entry &address_map_entry::set_submap(const char *tag, address_map_delegate func, int bits, u64 mask)
{
	if(!bits)
		bits = m_map.m_databits;

	assert(unitmask_is_appropriate(bits, mask, func.name()));

	m_read.m_type = AMH_DEVICE_SUBMAP;
	m_read.m_tag = tag;
	m_read.m_mask = mask;
	m_write.m_type = AMH_DEVICE_SUBMAP;
	m_write.m_tag = tag;
	m_write.m_mask = mask;
	m_submap_delegate = func;
	m_submap_bits = bits;
	return *this;
}


//-------------------------------------------------
//  set_handler - handler setters for
//  8-bit read/write handlers
//-------------------------------------------------

address_map_entry &address_map_entry::set_handler(read8_delegate func, u64 unitmask)
{
	assert(!func.isnull());
	assert(unitmask_is_appropriate(8, unitmask, func.name()));
	m_read.m_type = AMH_DEVICE_DELEGATE;
	m_read.m_bits = 8;
	m_read.m_mask = unitmask;
	m_read.m_name = func.name();
	m_rproto8 = func;
	return *this;
}


address_map_entry &address_map_entry::set_handler(write8_delegate func, u64 unitmask)
{
	assert(!func.isnull());
	assert(unitmask_is_appropriate(8, unitmask, func.name()));
	m_write.m_type = AMH_DEVICE_DELEGATE;
	m_write.m_bits = 8;
	m_write.m_mask = unitmask;
	m_write.m_name = func.name();
	m_wproto8 = func;
	return *this;
}


address_map_entry &address_map_entry::set_handler(read8_delegate rfunc, write8_delegate wfunc, u64 unitmask)
{
	set_handler(rfunc, unitmask);
	set_handler(wfunc, unitmask);
	return *this;
}


//-------------------------------------------------
//  set_handler - handler setters for
//  16-bit read/write handlers
//-------------------------------------------------

address_map_entry &address_map_entry::set_handler(read16_delegate func, u64 unitmask)
{
	assert(!func.isnull());
	assert(unitmask_is_appropriate(16, unitmask, func.name()));
	m_read.m_type = AMH_DEVICE_DELEGATE;
	m_read.m_bits = 16;
	m_read.m_mask = unitmask;
	m_read.m_name = func.name();
	m_rproto16 = func;
	return *this;
}


address_map_entry &address_map_entry::set_handler(write16_delegate func, u64 unitmask)
{
	assert(!func.isnull());
	assert(unitmask_is_appropriate(16, unitmask, func.name()));
	m_write.m_type = AMH_DEVICE_DELEGATE;
	m_write.m_bits = 16;
	m_write.m_mask = unitmask;
	m_write.m_name = func.name();
	m_wproto16 = func;
	return *this;
}


address_map_entry &address_map_entry::set_handler(read16_delegate rfunc, write16_delegate wfunc, u64 unitmask)
{
	set_handler(rfunc, unitmask);
	set_handler(wfunc, unitmask);
	return *this;
}


//-------------------------------------------------
//  set_handler - handler setters for
//  32-bit read/write handlers
//-------------------------------------------------

address_map_entry &address_map_entry::set_handler(read32_delegate func, u64 unitmask)
{
	assert(!func.isnull());
	assert(unitmask_is_appropriate(32, unitmask, func.name()));
	m_read.m_type = AMH_DEVICE_DELEGATE;
	m_read.m_bits = 32;
	m_read.m_mask = unitmask;
	m_read.m_name = func.name();
	m_rproto32 = func;
	return *this;
}


address_map_entry &address_map_entry::set_handler(write32_delegate func, u64 unitmask)
{
	assert(!func.isnull());
	assert(unitmask_is_appropriate(32, unitmask, func.name()));
	m_write.m_type = AMH_DEVICE_DELEGATE;
	m_write.m_bits = 32;
	m_write.m_mask = unitmask;
	m_write.m_name = func.name();
	m_wproto32 = func;
	return *this;
}


address_map_entry &address_map_entry::set_handler(read32_delegate rfunc, write32_delegate wfunc, u64 unitmask)
{
	set_handler(rfunc, unitmask);
	set_handler(wfunc, unitmask);
	return *this;
}


//-------------------------------------------------
//  set_handler - handler setters for
//  64-bit read/write handlers
//-------------------------------------------------

address_map_entry &address_map_entry::set_handler(read64_delegate func, u64 unitmask)
{
	assert(!func.isnull());
	assert(unitmask_is_appropriate(64, unitmask, func.name()));
	m_read.m_type = AMH_DEVICE_DELEGATE;
	m_read.m_bits = 64;
	m_read.m_mask = 0;
	m_read.m_name = func.name();
	m_rproto64 = func;
	return *this;
}


address_map_entry &address_map_entry::set_handler(write64_delegate func, u64 unitmask)
{
	assert(!func.isnull());
	assert(unitmask_is_appropriate(64, unitmask, func.name()));
	m_write.m_type = AMH_DEVICE_DELEGATE;
	m_write.m_bits = 64;
	m_write.m_mask = 0;
	m_write.m_name = func.name();
	m_wproto64 = func;
	return *this;
}


address_map_entry &address_map_entry::set_handler(read64_delegate rfunc, write64_delegate wfunc, u64 unitmask)
{
	set_handler(rfunc, unitmask);
	set_handler(wfunc, unitmask);
	return *this;
}


//-------------------------------------------------
//  set_handler - handler setter for setoffset
//-------------------------------------------------

address_map_entry &address_map_entry::set_handler(setoffset_delegate func)
{
	assert(!func.isnull());
	m_setoffsethd.m_type = AMH_DEVICE_DELEGATE;
	m_setoffsethd.m_bits = 0;
	m_setoffsethd.m_mask = 0;
	m_setoffsethd.m_name = func.name();
	m_soproto = func;
	return *this;
}

//-------------------------------------------------
//  unitmask_is_appropriate - verify that the
//  provided unitmask is valid and expected
//-------------------------------------------------

bool address_map_entry::unitmask_is_appropriate(u8 width, u64 unitmask, const char *string)
{
	// if no mask, this must match the default width of the map
	if (unitmask == 0)
	{
		if (m_map.m_databits != width)
			throw emu_fatalerror("Handler %s is a %d-bit handler but was specified in a %d-bit address map", string, width, m_map.m_databits);
		return true;
	}

	// if we have a mask, we must be smaller than the default width of the map
	if (m_map.m_databits < width)
		throw emu_fatalerror("Handler %s is a %d-bit handler and is too wide to be used in a %d-bit address map", string, width, m_map.m_databits);

	// if map is narrower than 64 bits, check the mask width as well
	// disabled because it breaks f.i. isa8_device::install_device, and the fix is nonobvious
	if (false && m_map.m_databits < 64 && (unitmask >> m_map.m_databits) != 0)
		throw emu_fatalerror("Handler %s specified a mask of %08X%08X, too wide to be used in a %d-bit address map", string, (u32)(unitmask >> 32), (u32)unitmask, m_map.m_databits);

	// the mask must represent whole units of width
	u32 basemask = (width == 8) ? 0xff : (width == 16) ? 0xffff : 0xffffffff;
	u64 singlemask = basemask;
	int count = 0;
	while (singlemask != 0)
	{
		if ((unitmask & singlemask) == singlemask)
			count++;
		else if ((unitmask & singlemask) != 0)
			throw emu_fatalerror("Handler %s specified a mask of %08X%08X; needs to be in even chunks of %X", string, (u32)(unitmask >> 32), (u32)unitmask, basemask);
		singlemask <<= width;
	}

	// subunit count must be a power of 2
	if (count != 1 && count != 2 && count != 4 && count != 8)
		throw emu_fatalerror("Handler %s specifies %d subunits with a mask of %08X%08X; needs to be a power of 2", string, count, (u32)(unitmask >> 32), (u32)unitmask);

	// the mask must be symmetrical
	u64 unitmask_bh = unitmask >> 8 & 0x00ff00ff00ff00ffU;
	u64 unitmask_bl = unitmask & 0x00ff00ff00ff00ffU;
	u64 unitmask_wh = unitmask >> 16 & 0x0000ffff0000ffffU;
	u64 unitmask_wl = unitmask & 0x0000ffff0000ffffU;
	u64 unitmask_dh = unitmask >> 32 & 0x00000000ffffffffU;
	u64 unitmask_dl = unitmask & 0x00000000ffffffffU;
	if ((unitmask_bh != 0 && unitmask_bl != 0 && unitmask_bh != unitmask_bl)
		|| (unitmask_wh != 0 && unitmask_wl != 0 && unitmask_wh != unitmask_wl)
		|| (unitmask_dh != 0 && unitmask_dl != 0 && unitmask_dh != unitmask_dl))
		throw emu_fatalerror("Handler %s specified an asymmetrical mask of %08X%08X", string, (u32)(unitmask >> 32), (u32)unitmask);

	return true;
}


//**************************************************************************
//  ADDRESS MAP
//**************************************************************************

//-------------------------------------------------
//  address_map - constructor
//-------------------------------------------------

address_map::address_map(device_t &device, address_spacenum spacenum)
	: m_spacenum(spacenum),
		m_device(&device),
		m_databits(0xff),
		m_unmapval(0),
		m_globalmask(0)
{
	// get our memory interface
	const device_memory_interface *memintf;
	if (!m_device->interface(memintf))
		throw emu_fatalerror("No memory interface defined for device '%s'\n", m_device->tag());

	// and then the configuration for the current address space
	const address_space_config *spaceconfig = memintf->space_config(spacenum);
	if (spaceconfig == nullptr)
		throw emu_fatalerror("No memory address space configuration found for device '%s', space %d\n", m_device->tag(), spacenum);

	// construct the internal device map (first so it takes priority)
	if (spaceconfig->m_internal_map != nullptr)
		(*spaceconfig->m_internal_map)(*this);
	if (!spaceconfig->m_internal_map_delegate.isnull())
		spaceconfig->m_internal_map_delegate(*this);

	// append the map provided by the owner
	if (memintf->address_map(spacenum) != nullptr)
	{
		m_device = device.owner();
		(*memintf->address_map(spacenum))(*this);
		m_device = &device;
	}
	else
	{
		// if the owner didn't provide a map, use the default device map
		if (spaceconfig->m_default_map != nullptr)
			(*spaceconfig->m_default_map)(*this);
		if (!spaceconfig->m_default_map_delegate.isnull())
			spaceconfig->m_default_map_delegate(*this);
	}
}



//-------------------------------------------------
//  address_map - constructor in the submap case
//-------------------------------------------------

address_map::address_map(device_t &device, address_map_entry *entry)
	: m_spacenum(AS_PROGRAM),
		m_device(&device),
		m_databits(0xff),
		m_unmapval(0),
		m_globalmask(0)
{
	// Retrieve the submap
	entry->m_submap_delegate.late_bind(*m_device);
	entry->m_submap_delegate(*this);
}



//----------------------------------------------------------
//  address_map - constructor dynamic device mapping case
//----------------------------------------------------------

address_map::address_map(const address_space &space, offs_t start, offs_t end, int bits, u64 unitmask, device_t &device, address_map_delegate submap_delegate)
	: m_spacenum(space.spacenum()),
		m_device(&device),
		m_databits(space.data_width()),
		m_unmapval(space.unmap()),
		m_globalmask(space.bytemask())
{
	range(start, end).set_submap(DEVICE_SELF, submap_delegate, bits, unitmask);
}


//-------------------------------------------------
//  ~address_map - destructor
//-------------------------------------------------

address_map::~address_map()
{
}


//-------------------------------------------------
//  configure - either configure the space and
//  databits, or verify they match previously-set
//  values
//-------------------------------------------------

void address_map::configure(address_spacenum spacenum, u8 databits)
{
	if (spacenum != m_spacenum)
		osd_printf_error("Space %d configured as address space %d\n", m_spacenum, spacenum);

	if (m_databits == 0xff)
		m_databits = databits;
	else if (databits != m_databits)
		osd_printf_error("Space %d configured with %d data bits when %d expected\n", m_spacenum, databits, m_databits);
}


//-------------------------------------------------
//  append - append an entry to the end of the
//  list
//-------------------------------------------------

void address_map::global_mask(offs_t mask)
{
//  if (m_entrylist != nullptr)
//      throw emu_fatalerror("AM_GLOBALMASK must be specified before any entries");
	m_globalmask = mask;
}



//-------------------------------------------------
//  add - add a new entry
//-------------------------------------------------

address_map_entry &address_map::range(offs_t start, offs_t end)
{
	address_map_entry *ptr = global_alloc(address_map_entry(*m_device, *this, start, end));
	m_entrylist.append(*ptr);
	return *ptr;
}


//-------------------------------------------------
//  uplift_submaps - propagate in the device submaps
//-------------------------------------------------

void address_map::uplift_submaps(running_machine &machine, device_t &owner, endianness_t endian)
{
	address_map_entry *prev = nullptr;
	address_map_entry *entry = m_entrylist.first();
	while (entry)
	{
		if (entry->m_read.m_type == AMH_DEVICE_SUBMAP)
		{
			std::string tag = owner.subtag(entry->m_read.m_tag);
			device_t *mapdevice = machine.device(tag.c_str());
			if (mapdevice == nullptr) {
				throw emu_fatalerror("Attempted to submap a non-existent device '%s' in space %d of device '%s'\n", tag.c_str(), m_spacenum, m_device->basetag());
			}
			// Grab the submap
			address_map submap(*mapdevice, entry);

			// Recursively uplift it if needed
			submap.uplift_submaps(machine, *mapdevice, endian);

			// Compute the unit repartition characteristics
			int entry_bits = entry->m_submap_bits;
			if (!entry_bits)
				entry_bits = m_databits;

			if (submap.m_databits != entry_bits)
				throw emu_fatalerror("AM_DEVICE wants a %d bits large address map and got a %d bits large one instead.\n", entry_bits, submap.m_databits);

			int entry_bytes = entry_bits / 8;
			int databytes = m_databits / 8;

			offs_t mirror_address_mask = (databytes - 1) & ~(entry_bytes - 1);

			u64 entry_mask = (2ULL << (entry_bits-1)) - 1;

			int slot_offset[8];
			int slot_count = 0;
			int max_slot_count = m_databits / entry_bits;
			int slot_xor_mask = endian == ENDIANNESS_LITTLE ? 0 : max_slot_count - 1;

			u64 global_mask = entry->m_read.m_mask;
			// zero means all
			if (!global_mask)
				global_mask = ~global_mask;

			// mask consistency has already been checked in
			// unitmask_is_appropriate, so one bit is enough
			for (int slot=0; slot < max_slot_count; slot++)
				if (global_mask & (1ULL << ((slot ^ slot_xor_mask) * entry_bits)))
					slot_offset[slot_count++] = (slot ^ slot_xor_mask) * entry_bits;

			// Merge in all the map contents in order
			while (submap.m_entrylist.count())
			{
				address_map_entry *subentry = submap.m_entrylist.detach_head();

				// Remap start and end

				unsigned int start_offset = subentry->m_addrstart / entry_bytes;
				unsigned int start_slot = start_offset % slot_count;
				subentry->m_addrstart = entry->m_addrstart + (start_offset / slot_count) * databytes;

				// Drop the entry if it ends up outside the range
				if (subentry->m_addrstart > entry->m_addrend)
				{
					global_free(subentry);
					continue;
				}

				unsigned int end_offset = subentry->m_addrend / entry_bytes;
				unsigned int end_slot = end_offset % slot_count;
				subentry->m_addrend = entry->m_addrstart + (end_offset / slot_count) * databytes + databytes - 1;

				// Clip the entry to the end of the range
				if (subentry->m_addrend > entry->m_addrend || subentry->m_addrend < entry->m_addrstart)
					subentry->m_addrend = entry->m_addrend;

				// Detect special unhandled case (range straddling
				// slots, requiring splitting in multiple entries and
				// unimplemented offset-add subunit handler)
				if (subentry->m_addrstart + databytes - 1 != subentry->m_addrend &&
					(start_slot != 0 || end_slot != slot_count - 1))
					throw emu_fatalerror("uplift_submaps unhandled case: range straddling slots.\n");

				if (entry->m_addrmask || subentry->m_addrmask)
					throw emu_fatalerror("uplift_submaps unhandled case: address masks.\n");

				if (entry->m_addrselect || subentry->m_addrselect)
					throw emu_fatalerror("uplift_submaps unhandled case: select masks.\n");

				if (subentry->m_addrmirror & mirror_address_mask)
					throw emu_fatalerror("uplift_submaps unhandled case: address mirror bit within subentry.\n");

				subentry->m_addrmirror |= entry->m_addrmirror;

				// Twiddle the unitmask on the data accessors that need it
				for (int data_entry = 0; data_entry < 3; data_entry++)
				{
					map_handler_data &mdata = (data_entry==0)? subentry->m_read : ((data_entry==1)? subentry->m_write : subentry->m_setoffsethd);

					if (mdata.m_type == AMH_NONE)
						continue;

					if (mdata.m_type != AMH_DEVICE_DELEGATE && mdata.m_type != AMH_NOP)
						throw emu_fatalerror("Only normal read/write methods are accepted in device submaps.\n");

					if (mdata.m_bits == 0 && entry_bits != m_databits)
						mdata.m_bits = entry_bits;

					u64 mask = 0;
					if (mdata.m_bits != m_databits)
					{
						u64 unitmask = mdata.m_mask ? mdata.m_mask : entry_mask;
						for (int slot = start_slot; slot <= end_slot; slot++)
							mask |= unitmask << slot_offset[slot];
					}
					mdata.m_mask = mask;
				}

				// Insert the entry in the map
				m_entrylist.insert_after(*subentry, prev);
				prev = subentry;
			}

			address_map_entry *to_delete = entry;
			entry = entry->next();
			m_entrylist.remove(*to_delete);
		}
		else
		{
			prev = entry;
			entry = entry->next();
		}
	}
}


//-------------------------------------------------
//  map_validity_check - perform validity checks on
//  one of the device's address maps
//-------------------------------------------------

void address_map::map_validity_check(validity_checker &valid, address_spacenum spacenum) const
{
	// it's safe to assume here that the device has a memory interface and a config for this space
	const address_space_config &spaceconfig = *m_device->memory().space_config(spacenum);
	int datawidth = spaceconfig.m_databus_width;
	int alignunit = datawidth / 8;

	bool detected_overlap = DETECT_OVERLAPPING_MEMORY ? false : true;

	// if this is an empty map, just ignore it
	if (m_entrylist.first() == nullptr)
		return;

	// validate the global map parameters
	if (m_spacenum != spacenum)
		osd_printf_error("Space %d has address space %d handlers!\n", spacenum, m_spacenum);
	if (m_databits != datawidth)
		osd_printf_error("Wrong memory handlers provided for %s space! (width = %d, memory = %08x)\n", spaceconfig.m_name, datawidth, m_databits);

	// loop over entries and look for errors
	for (address_map_entry &entry : m_entrylist)
	{
		u32 bytestart = spaceconfig.addr2byte(entry.m_addrstart);
		u32 byteend = spaceconfig.addr2byte_end(entry.m_addrend);

		// look for overlapping entries
		if (!detected_overlap)
		{
			for (address_map_entry &scan : m_entrylist)
			{
				if (&scan == &entry)
					break;
				if (entry.m_addrstart <= scan.m_addrend && entry.m_addrend >= scan.m_addrstart &&
					((entry.m_read.m_type != AMH_NONE && scan.m_read.m_type != AMH_NONE) ||
						(entry.m_write.m_type != AMH_NONE && scan.m_write.m_type != AMH_NONE)))
				{
					osd_printf_warning("%s space has overlapping memory (%X-%X,%d,%d) vs (%X-%X,%d,%d)\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_read.m_type, entry.m_write.m_type, scan.m_addrstart, scan.m_addrend, scan.m_read.m_type, scan.m_write.m_type);
					detected_overlap = true;
					break;
				}
			}
		}

		// look for inverted start/end pairs
		if (byteend < bytestart)
			osd_printf_error("Wrong %s memory read handler start = %08x > end = %08x\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend);

		// look for misaligned entries
		if ((bytestart & (alignunit - 1)) != 0 || (byteend & (alignunit - 1)) != (alignunit - 1))
			osd_printf_error("Wrong %s memory read handler start = %08x, end = %08x ALIGN = %d\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, alignunit);

		// verify mask/mirror/select
		offs_t set_bits = entry.m_addrstart | entry.m_addrend;
		offs_t changing_bits = entry.m_addrstart ^ entry.m_addrend;
		changing_bits |= changing_bits >> 1;
		changing_bits |= changing_bits >> 2;
		changing_bits |= changing_bits >> 4;
		changing_bits |= changing_bits >> 8;
		changing_bits |= changing_bits >> 16;

		if (entry.m_addrmask & ~changing_bits)
			osd_printf_error("In %s memory range %x-%x, mask %x is trying to unmask an unchanging address bit (%x)\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmask & ~changing_bits);
		if (entry.m_addrmirror & changing_bits)
			osd_printf_error("In %s memory range %x-%x, mirror %x touches a changing address bit (%x)\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_addrmirror, entry.m_addrmirror & changing_bits);
		if (entry.m_addrselect & changing_bits)
			osd_printf_error("In %s memory range %x-%x, select %x touches a changing address bit (%x)\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_addrselect, entry.m_addrselect & changing_bits);
		if (entry.m_addrmirror & set_bits)
			osd_printf_error("In %s memory range %x-%x, mirror %x touches a set address bit (%x)\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_addrmirror, entry.m_addrmirror & set_bits);
		if (entry.m_addrselect & set_bits)
			osd_printf_error("In %s memory range %x-%x, select %x touches a set address bit (%x)\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_addrselect, entry.m_addrselect & set_bits);
		if (entry.m_addrmirror & entry.m_addrselect)
			osd_printf_error("In %s memory range %x-%x, mirror %x touches a select bit (%x)\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_addrmirror, entry.m_addrmirror & entry.m_addrselect);

		// if this is a program space, auto-assign implicit ROM entries
		if (entry.m_read.m_type == AMH_ROM && entry.m_region == nullptr)
		{
			entry.m_region = m_device->tag();
			entry.m_rgnoffs = entry.m_addrstart;
		}

		// if this entry references a memory region, validate it
		if (entry.m_region != nullptr && entry.m_share == nullptr)
		{
			// address map entries that reference regions but are NOPs are pointless
			if (entry.m_read.m_type == AMH_NONE && entry.m_write.m_type == AMH_NONE)
				osd_printf_error("%s space references memory region %s, but is AM_NOP\n", spaceconfig.m_name, entry.m_region);

			// make sure we can resolve the full path to the region
			bool found = false;
			std::string entry_region = entry.m_devbase.subtag(entry.m_region);

			// look for the region
			for (device_t &dev : device_iterator(m_device->mconfig().root_device()))
				for (const rom_entry *romp = rom_first_region(dev); romp != nullptr && !found; romp = rom_next_region(romp))
				{
					if (rom_region_name(dev, romp) == entry_region)
					{
						// verify the address range is within the region's bounds
						offs_t length = ROMREGION_GETLENGTH(romp);
						if (entry.m_rgnoffs + (byteend - bytestart + 1) > length)
							osd_printf_error("%s space memory map entry %X-%X extends beyond region '%s' size (%X)\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_region, length);
						found = true;
					}
				}

			// error if not found
			if (!found)
				osd_printf_error("%s space memory map entry %X-%X references non-existant region '%s'\n", spaceconfig.m_name, entry.m_addrstart, entry.m_addrend, entry.m_region);
		}

		// make sure all devices exist
		if (entry.m_read.m_type == AMH_DEVICE_DELEGATE)
		{
			// extract the device tag from the proto-delegate
			const char *devtag = nullptr;
			switch (entry.m_read.m_bits)
			{
				case 8: devtag = entry.m_rproto8.device_name(); break;
				case 16: devtag = entry.m_rproto16.device_name(); break;
				case 32: devtag = entry.m_rproto32.device_name(); break;
				case 64: devtag = entry.m_rproto64.device_name(); break;
			}
			if (entry.m_devbase.subdevice(devtag) == nullptr)
				osd_printf_error("%s space memory map entry reads from nonexistent device '%s'\n", spaceconfig.m_name,
					devtag != nullptr ? devtag : "<unspecified>");
		}
		if (entry.m_write.m_type == AMH_DEVICE_DELEGATE)
		{
			// extract the device tag from the proto-delegate
			const char *devtag = nullptr;
			switch (entry.m_write.m_bits)
			{
				case 8: devtag = entry.m_wproto8.device_name(); break;
				case 16: devtag = entry.m_wproto16.device_name(); break;
				case 32: devtag = entry.m_wproto32.device_name(); break;
				case 64: devtag = entry.m_wproto64.device_name(); break;
			}
			if (entry.m_devbase.subdevice(devtag) == nullptr)
				osd_printf_error("%s space memory map entry writes to nonexistent device '%s'\n", spaceconfig.m_name,
					devtag != nullptr ? devtag : "<unspecified>");
		}
		if (entry.m_setoffsethd.m_type == AMH_DEVICE_DELEGATE)
		{
			// extract the device tag from the proto-delegate
			const char *devtag = entry.m_soproto.device_name();
			if (entry.m_devbase.subdevice(devtag) == nullptr)
				osd_printf_error("%s space memory map entry references nonexistent device '%s'\n", spaceconfig.m_name,
					devtag != nullptr ? devtag : "<unspecified>");
		}

		// make sure ports exist
//      if ((entry.m_read.m_type == AMH_PORT && entry.m_read.m_tag != nullptr && portlist.find(entry.m_read.m_tag) == nullptr) ||
//          (entry.m_write.m_type == AMH_PORT && entry.m_write.m_tag != nullptr && portlist.find(entry.m_write.m_tag) == nullptr))
//          osd_printf_error("%s space memory map entry references nonexistent port tag '%s'\n", spaceconfig.m_name, entry.m_read.m_tag);

		// validate bank and share tags
		if (entry.m_read.m_type == AMH_BANK)
			valid.validate_tag(entry.m_read.m_tag);
		if (entry.m_write.m_type == AMH_BANK)
			valid.validate_tag(entry.m_write.m_tag);
		if (entry.m_share != nullptr)
			valid.validate_tag(entry.m_share);
	}
}
