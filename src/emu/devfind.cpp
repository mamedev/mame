// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    devfind.c

    Device finding template helpers.

***************************************************************************/

#include "emu.h"


//**************************************************************************
//  EXPLICIT TEMPLATE INSTANTIATIONS
//**************************************************************************

template class object_finder_base<memory_region, false>;
template class object_finder_base<memory_region, true>;
template class object_finder_base<memory_bank, false>;
template class object_finder_base<memory_bank, true>;
template class object_finder_base<ioport_port, false>;
template class object_finder_base<ioport_port, true>;

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



//**************************************************************************
//  BASE FINDER CLASS
//**************************************************************************

constexpr char finder_base::DUMMY_TAG[];


//-------------------------------------------------
//  finder_base - constructor
//-------------------------------------------------

finder_base::finder_base(device_t &base, const char *tag)
	: m_next(base.register_auto_finder(*this))
	, m_base(base)
	, m_tag(tag)
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
			osd_printf_warning("Region '%s' found but is width %d, not %d as requested\n", m_tag, region->bitwidth(), width*8);
		length = 0;
		return nullptr;
	}

	// check the length and warn if other than specified
	size_t const length_found = region->bytes() / width;
	if (length != 0 && length != length_found)
	{
		if (required)
			osd_printf_warning("Region '%s' found but has %d bytes, not %ld as requested\n", m_tag, region->bytes(), long(length*width));
		length = 0;
		return nullptr;
	}

	// return results
	length = length_found;
	return region->base();
}


//-------------------------------------------------
//  validate_memregion - find memory region
//-------------------------------------------------

bool finder_base::validate_memregion(size_t bytes, bool required) const
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

	// check the length and warn if other than specified
	if ((bytes_found != 0) && (bytes != 0) && (bytes != bytes_found))
	{
		osd_printf_warning("Region '%s' found but has %ld bytes, not %ld as requested\n", m_tag, long(bytes_found), long(bytes));
		bytes_found = 0;
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
		if (required)
			osd_printf_warning("Shared ptr '%s' found but is width %d, not %d as requested\n", m_tag, share->bitwidth(), width);
		return nullptr;
	}

	// return results
	bytes = share->bytes();
	return share->ptr();
}


//-------------------------------------------------
//  report_missing - report missing objects and
//  return true if it's ok
//-------------------------------------------------

bool finder_base::report_missing(bool found, const char *objname, bool required) const
{
	if (required && (strcmp(m_tag, DUMMY_TAG) == 0))
	{
		osd_printf_error("Tag not defined for required %s\n", objname);
		return false;
	}

	// just pass through in the found case
	if (found)
		return true;

	// otherwise, report
	std::string const region_fulltag(m_base.get().subtag(m_tag));
	if (required)
		osd_printf_error("Required %s '%s' not found\n", objname, region_fulltag.c_str());
	else
		osd_printf_verbose("Optional %s '%s' not found\n", objname, region_fulltag.c_str());
	return !required;
}


void finder_base::printf_warning(const char *format, ...)
{
	va_list argptr;
	char buffer[1024];

	/* do the output */
	va_start(argptr, format);
	vsnprintf(buffer, 1024, format, argptr);
	osd_printf_warning("%s", buffer);
	va_end(argptr);
}
