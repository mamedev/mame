// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    devfind.c

    Device finding template helpers.

***************************************************************************/

#include "emu.h"


//**************************************************************************
//  BASE FINDER CLASS
//**************************************************************************

//-------------------------------------------------
//  finder_base - constructor
//-------------------------------------------------

finder_base::finder_base(device_t &base, const char *tag)
	: m_next(base.register_auto_finder(*this)),
		m_base(base),
		m_tag(tag)
{
}


//-------------------------------------------------
//  ~finder_base - destructor
//-------------------------------------------------

finder_base::~finder_base()
{
}


//-------------------------------------------------
//  find_memregion - find memory region
//-------------------------------------------------

void *finder_base::find_memregion(UINT8 width, size_t &length, bool required) const
{
	// look up the region and return NULL if not found
	memory_region *region = m_base.memregion(m_tag);
	if (region == nullptr)
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
	size_t length_found = region->bytes() / width;
	if (length != 0 && length != length_found)
	{
		if (required)
			osd_printf_warning("Region '%s' found but has %d bytes, not %d as requested\n", m_tag, region->bytes(), (int)length*width);
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
	std::string region_fulltag = m_base.subtag(m_tag);

	// look for the region
	device_iterator deviter(m_base.mconfig().root_device());
	for (device_t *dev = deviter.first(); dev != nullptr && bytes_found == 0; dev = deviter.next())
		for (const rom_entry *romp = rom_first_region(*dev); romp != nullptr; romp = rom_next_region(romp))
		{
			if (rom_region_name(*dev, romp) == region_fulltag)
			{
				bytes_found = ROMREGION_GETLENGTH(romp);
				break;
			}
		}

	if (bytes_found != 0)
	{
		// check the length and warn if other than specified
		if (bytes != 0 && bytes != bytes_found)
		{
			osd_printf_warning("Region '%s' found but has %d bytes, not %d as requested\n", m_tag, (int)bytes_found, (int)bytes);
			bytes_found = 0;
		}
	}
	return report_missing(bytes_found != 0, "memory region", required);
}


//-------------------------------------------------
//  find_memshare - find memory share
//-------------------------------------------------

void *finder_base::find_memshare(UINT8 width, size_t &bytes, bool required) const
{
	// look up the share and return NULL if not found
	memory_share *share = m_base.memshare(m_tag);
	if (share == nullptr)
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
	if (required && strcmp(m_tag, FINDER_DUMMY_TAG)==0)
	{
		osd_printf_error("Tag not defined for required device\n");
		return false;
	}

	// just pass through in the found case
	if (found)
		return true;

	// otherwise, report
	if (required)
		osd_printf_error("Required %s '%s' not found\n", objname, m_tag);
	else
		osd_printf_verbose("Optional %s '%s' not found\n", objname, m_tag);
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
