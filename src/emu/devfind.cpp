// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    devfind.cpp

    Device finding template helpers.

***************************************************************************/

#include "emu.h"

#include "romload.h"
#include "validity.h"


//**************************************************************************
//  BASE FINDER CLASS
//**************************************************************************

constexpr char finder_base::DUMMY_TAG[];


//-------------------------------------------------
//  finder_base - constructor
//-------------------------------------------------

finder_base::finder_base(device_t &base, char const *tag)
	: m_next(base.register_auto_finder(*this))
	, m_base(base)
	, m_tag(tag)
	, m_resolved(false)
{
}


//-------------------------------------------------
//  ~finder_base - destructor
//-------------------------------------------------

finder_base::~finder_base()
{
}


//-------------------------------------------------
//  set_tag - set tag
//-------------------------------------------------

void finder_base::set_tag(char const *tag)
{
	assert(!m_resolved);
	m_base = m_base.get().mconfig().current_device();
	m_tag = tag;
}


//-------------------------------------------------
//  find_memregion - find memory region
//-------------------------------------------------

void *finder_base::find_memregion(u8 width, size_t &length, bool required) const
{
	// look up the region and return nullptr if not found
	memory_region *const region(m_base.get().memregion(m_tag));
	if (!region)
	{
		length = 0;
		return nullptr;
	}

	// check the width and warn if not correct
	if (region->bytewidth() != width)
	{
		if (required)
			osd_printf_warning("Region '%s' found but is width %d, not %d as requested\n", m_tag, region->bitwidth(), width * 8);
		length = 0;
		return nullptr;
	}

	// return results
	length = region->bytes() / width;
	return region->base();
}


//-------------------------------------------------
//  validate_memregion - find memory region
//-------------------------------------------------

bool finder_base::validate_memregion(bool required) const
{
	// make sure we can resolve the full path to the region
	size_t bytes_found = 0;
	std::string const region_fulltag(m_base.get().subtag(m_tag));

	// look for the region
	for (device_t const &dev : device_iterator(m_base.get().mconfig().root_device()))
	{
		for (romload::region const &region : romload::entries(dev.rom_region()).get_regions())
		{
			if (dev.subtag(region.get_tag()) == region_fulltag)
			{
				bytes_found = region.get_length();
				break;
			}
		}
		if (bytes_found != 0)
			break;
	}

	return report_missing(bytes_found != 0, "memory region", required);
}


//-------------------------------------------------
//  find_memshare - find memory share
//-------------------------------------------------

void *finder_base::find_memshare(u8 width, size_t &bytes, bool required) const
{
	// look up the share and return nullptr if not found
	memory_share *const share(m_base.get().memshare(m_tag));
	if (!share)
		return nullptr;

	// check the width and warn if not correct
	if (width != 0 && share->bitwidth() != width)
	{
		osd_printf_warning("Shared ptr '%s' found but is width %d, not %d as requested\n", m_tag, share->bitwidth(), width);
		return nullptr;
	}

	// return results
	bytes = share->bytes();
	return share->ptr();
}


//-------------------------------------------------
//  find_addrspace - find address space
//-------------------------------------------------

address_space *finder_base::find_addrspace(int spacenum, u8 width, bool required) const
{
	// look up the device and return nullptr if not found
	device_t *const device(m_base.get().subdevice(m_tag));
	if (device == nullptr)
		return nullptr;

	// check for memory interface and the specified space number
	const device_memory_interface *memory;
	if (!device->interface(memory))
	{
		if (required)
			osd_printf_warning("Device '%s' found but lacks memory interface\n", m_tag);
		return nullptr;
	}
	if (!memory->has_space(spacenum))
	{
		if (required)
			osd_printf_warning("Device '%s' found but lacks address space #%d\n", m_tag, spacenum);
		return nullptr;
	}

	// check data width
	address_space &space(memory->space(spacenum));
	if (width != 0 && width != space.data_width())
	{
		osd_printf_warning("Device '%s' found but address space #%d has the wrong data width (expected %d, found %d)\n", m_tag, spacenum, width, space.data_width());
		return nullptr;
	}

	// return result
	return &space;
}


//-------------------------------------------------
//  validate_addrspace - find address space
//-------------------------------------------------

bool finder_base::validate_addrspace(int spacenum, u8 width, bool required) const
{
	// look up the device and return false if not found
	device_t *const device(m_base.get().subdevice(m_tag));
	if (!device)
		return report_missing(false, "address space", required);

	// check for memory interface and a configuration for the designated space
	const device_memory_interface *memory = nullptr;
	const address_space_config *config = nullptr;
	if (device->interface(memory))
	{
		config = memory->space_config(spacenum);
		if (required)
		{
			if (config == nullptr)
				osd_printf_warning("Device '%s' found but lacks address space #%d\n", m_tag, spacenum);
			else if (width != 0 && width != config->data_width())
				osd_printf_warning("Device '%s' found but space #%d has the wrong data width (expected %d, found %d)\n", m_tag, spacenum, width, config->data_width());
		}
	}
	else if (required)
		osd_printf_warning("Device '%s' found but lacks memory interface\n", m_tag);

	// report result
	return report_missing(config != nullptr && (width == 0 || width == config->data_width()), "address space", required);
}


//-------------------------------------------------
//  report_missing - report missing objects and
//  return true if it's ok
//-------------------------------------------------

bool finder_base::report_missing(bool found, const char *objname, bool required) const
{
	if (required && (DUMMY_TAG == m_tag))
	{
		osd_printf_error("Tag not defined for required %s\n", objname);
		return false;
	}
	else if (found)
	{
		// just pass through in the found case
		return true;
	}
	else
	{
		// otherwise, report
		std::string const region_fulltag(m_base.get().subtag(m_tag));
		if (required)
			osd_printf_error("Required %s '%s' not found\n", objname, region_fulltag);
		else if (DUMMY_TAG != m_tag)
			osd_printf_verbose("Optional %s '%s' not found\n", objname, region_fulltag);
		return !required;
	}
}



//**************************************************************************
//  MEMORY REGION FINDER
//**************************************************************************

template <bool Required>
memory_region_finder<Required>::memory_region_finder(device_t &base, char const *tag)
	: object_finder_base<memory_region, Required>(base, tag)
{
}


template <bool Required>
bool memory_region_finder<Required>::findit(validity_checker *valid)
{
	if (valid)
		return this->validate_memregion(Required);

	assert(!this->m_resolved);
	this->m_resolved = true;
	this->m_target = this->m_base.get().memregion(this->m_tag);
	return this->report_missing("memory region");
}



//**************************************************************************
//  MEMORY BANK FINDER
//**************************************************************************

template <bool Required>
memory_bank_finder<Required>::memory_bank_finder(device_t &base, char const *tag)
	: object_finder_base<memory_bank, Required>(base, tag)
{
}


template <bool Required>
bool memory_bank_finder<Required>::findit(validity_checker *valid)
{
	if (valid)
		return true;

	assert(!this->m_resolved);
	this->m_resolved = true;
	this->m_target = this->m_base.get().membank(this->m_tag);
	return this->report_missing("memory bank");
}



//**************************************************************************
//  I/O PORT FINDER
//**************************************************************************

template <bool Required>
ioport_finder<Required>::ioport_finder(device_t &base, char const *tag)
	: object_finder_base<ioport_port, Required>(base, tag)
{
}


template <bool Required>
bool ioport_finder<Required>::findit(validity_checker *valid)
{
	if (valid)
		return finder_base::report_missing(!valid->ioport_missing(this->m_base.get().subtag(this->m_tag).c_str()), "I/O port", Required);

	assert(!this->m_resolved);
	this->m_resolved = true;
	this->m_target = this->m_base.get().ioport(this->m_tag);
	return this->report_missing("I/O port");
}



//**************************************************************************
//  ADDRESS SPACE FINDER
//**************************************************************************

template <bool Required>
address_space_finder<Required>::address_space_finder(device_t &base, char const *tag, int spacenum, u8 width)
	: object_finder_base<address_space, Required>(base, tag)
	, m_spacenum(spacenum)
	, m_data_width(width)
{
}


template <bool Required>
bool address_space_finder<Required>::findit(validity_checker *valid)
{
	if (valid)
		return this->validate_addrspace(this->m_spacenum, this->m_data_width, Required);

	assert(!this->m_resolved);
	this->m_resolved = true;
	this->m_target = this->find_addrspace(this->m_spacenum, this->m_data_width, Required);
	return this->report_missing("address space");
}



//**************************************************************************
//  MEMORY BANK CREATOR
//**************************************************************************

bool memory_bank_creator::findit(validity_checker *valid)
{
	if (valid)
		return true;

	device_t &dev = m_base.get();
	memory_manager &manager = dev.machine().memory();
	std::string const tag = dev.subtag(m_tag);
	memory_bank *const bank = manager.bank_find(tag);
	m_target = bank ? bank : manager.bank_alloc(dev, tag);
	return true;
}


void memory_bank_creator::end_configuration()
{
	m_target = nullptr;
}



//**************************************************************************
//  MEMORY SHARE CREATOR
//**************************************************************************

template <typename PointerType>
memory_share_creator<PointerType>::memory_share_creator(device_t &base, char const *tag, size_t bytes, endianness_t endianness)
	: finder_base(base, tag)
	, m_width(sizeof(PointerType) * 8)
	, m_bytes(bytes)
	, m_endianness(endianness)
{
}


template <typename PointerType>
bool memory_share_creator<PointerType>::findit(validity_checker *valid)
{
	if (valid)
		return true;

	device_t &dev = m_base.get();
	memory_manager &manager = dev.machine().memory();
	std::string const tag = dev.subtag(m_tag);
	memory_share *const share = manager.share_find(tag);
	if (share)
	{
		std::string const result = share->compare(m_width, m_bytes, m_endianness);
		if (!result.empty())
		{
			osd_printf_error("%s\n", result);
			return false;
		}
		m_target = share;
	}
	else
	{
		m_target = manager.share_alloc(dev, tag, m_width, m_bytes, m_endianness);
	}
	return true;
}


template <typename PointerType>
void memory_share_creator<PointerType>::end_configuration()
{
	m_target = nullptr;
}



//**************************************************************************
//  EXPLICIT TEMPLATE INSTANTIATIONS
//**************************************************************************

template class object_finder_base<memory_region, false>;
template class object_finder_base<memory_region, true>;
template class object_finder_base<memory_bank, false>;
template class object_finder_base<memory_bank, true>;
template class object_finder_base<ioport_port, false>;
template class object_finder_base<ioport_port, true>;
template class object_finder_base<address_space, false>;
template class object_finder_base<address_space, true>;

template class object_finder_base<u8, false>;
template class object_finder_base<u8, true>;
template class object_finder_base<u16, false>;
template class object_finder_base<u16, true>;
template class object_finder_base<u32, false>;
template class object_finder_base<u32, true>;
template class object_finder_base<u64, false>;
template class object_finder_base<u64, true>;

template class object_finder_base<s8, false>;
template class object_finder_base<s8, true>;
template class object_finder_base<s16, false>;
template class object_finder_base<s16, true>;
template class object_finder_base<s32, false>;
template class object_finder_base<s32, true>;
template class object_finder_base<s64, false>;
template class object_finder_base<s64, true>;

template class memory_region_finder<false>;
template class memory_region_finder<true>;

template class memory_bank_finder<false>;
template class memory_bank_finder<true>;

template class ioport_finder<false>;
template class ioport_finder<true>;

template class address_space_finder<false>;
template class address_space_finder<true>;

template class region_ptr_finder<u8, false>;
template class region_ptr_finder<u8, true>;
template class region_ptr_finder<u16, false>;
template class region_ptr_finder<u16, true>;
template class region_ptr_finder<u32, false>;
template class region_ptr_finder<u32, true>;
template class region_ptr_finder<u64, false>;
template class region_ptr_finder<u64, true>;

template class region_ptr_finder<s8, false>;
template class region_ptr_finder<s8, true>;
template class region_ptr_finder<s16, false>;
template class region_ptr_finder<s16, true>;
template class region_ptr_finder<s32, false>;
template class region_ptr_finder<s32, true>;
template class region_ptr_finder<s64, false>;
template class region_ptr_finder<s64, true>;

template class shared_ptr_finder<u8, false>;
template class shared_ptr_finder<u8, true>;
template class shared_ptr_finder<u16, false>;
template class shared_ptr_finder<u16, true>;
template class shared_ptr_finder<u32, false>;
template class shared_ptr_finder<u32, true>;
template class shared_ptr_finder<u64, false>;
template class shared_ptr_finder<u64, true>;

template class shared_ptr_finder<s8, false>;
template class shared_ptr_finder<s8, true>;
template class shared_ptr_finder<s16, false>;
template class shared_ptr_finder<s16, true>;
template class shared_ptr_finder<s32, false>;
template class shared_ptr_finder<s32, true>;
template class shared_ptr_finder<s64, false>;
template class shared_ptr_finder<s64, true>;

template class memory_share_creator<u8>;
template class memory_share_creator<u16>;
template class memory_share_creator<u32>;
template class memory_share_creator<u64>;

template class memory_share_creator<s8>;
template class memory_share_creator<s16>;
template class memory_share_creator<s32>;
template class memory_share_creator<s64>;
