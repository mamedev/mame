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
//  find_memory - find memory
//-------------------------------------------------

void *finder_base::find_memory(UINT8 width, size_t &bytes, bool required)
{
	// look up the share and return NULL if not found
	memory_share *share = m_base.memshare(m_tag);
	if (share == NULL)
		return NULL;

	// check the width and warn if not correct
	if (width != 0 && share->width() != width)
	{
		if (required)
			mame_printf_warning("Shared ptr '%s' found but is width %d, not %d as requested\n", m_tag, share->width(), width);
		return NULL;
	}

	// return results
	bytes = share->bytes();
	return share->ptr();
}


//-------------------------------------------------
//  report_missing - report missing objects and
//  return true if it's ok
//-------------------------------------------------

bool finder_base::report_missing(bool found, const char *objname, bool required)
{
	if (required && strcmp(m_tag, FINDER_DUMMY_TAG)==0)
	{
		mame_printf_error("Tag not defined for required device\n");
		return false;
	}

	// just pass through in the found case
	if (found)
		return true;

	// otherwise, report
	if (required)
		mame_printf_error("Required %s '%s' not found\n", objname, m_tag);
	else
		mame_printf_verbose("Optional %s '%s' not found\n", objname, m_tag);
	return !required;
}
