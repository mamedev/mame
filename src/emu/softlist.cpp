// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    softlist.c

    Software list construction helpers.

***************************************************************************/

#include "emu.h"
#include "softlist.h"

#include "drivenum.h"
#include "emuopts.h"
#include "swinfo.h"
#include "validity.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type SOFTWARE_LIST = &device_creator<software_list_device>;



//**************************************************************************
//  SOFTWARE LIST DEVICE
//**************************************************************************

//-------------------------------------------------
//  software_list_device - constructor
//-------------------------------------------------

software_list_device::software_list_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SOFTWARE_LIST, "Software list", tag, owner, clock, "software_list", __FILE__),
		m_list_type(SOFTWARE_LIST_ORIGINAL_SYSTEM),
		m_filter(nullptr)
{
}


//-------------------------------------------------
//  static_set_type - configuration helper
//  to set the list type
//-------------------------------------------------

void software_list_device::static_set_type(device_t &device, const char *list, softlist_type list_type)
{
	software_list_device &swlistdev = downcast<software_list_device &>(device);
	swlistdev.m_list_name.assign(list);
	swlistdev.m_list_type = list_type;
}


//-------------------------------------------------
//  static_set_custom_handler - configuration
//  helper to set a custom callback
//-------------------------------------------------

void software_list_device::static_set_filter(device_t &device, const char *filter)
{
	downcast<software_list_device &>(device).m_filter = filter;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void software_list_device::device_start()
{
}


//-------------------------------------------------
//  find_approx_matches - search ourselves for
//  a list of possible matches of the given name
//  and optional interface
//-------------------------------------------------

void software_list_device::find_approx_matches(const char *name, int matches, const software_info **list, const char *interface)
{
	// if no name, return
	if (name == nullptr || name[0] == 0)
		return;

	// initialize everyone's states
	std::vector<int> penalty(matches);
	for (int matchnum = 0; matchnum < matches; matchnum++)
	{
		penalty[matchnum] = 9999;
		list[matchnum] = nullptr;
	}

	// cause a parse if needed
	parse();

	// iterate over our info
	for (const software_info &swinfo : m_parsed->get_info())
	{
		const software_part &part = swinfo.parts().front();
		if ((interface == nullptr || part.matches_interface(interface)) && part.is_compatible(m_filter) == SOFTWARE_IS_COMPATIBLE)
		{
			// pick the best match between driver name and description
			int longpenalty = driver_list::penalty_compare(name, swinfo.longname().c_str());
			int shortpenalty = driver_list::penalty_compare(name, swinfo.shortname().c_str());
			int curpenalty = std::min(longpenalty, shortpenalty);

			// insert into the sorted table of matches
			for (int matchnum = matches - 1; matchnum >= 0; matchnum--)
			{
				// stop if we're worse than the current entry
				if (curpenalty >= penalty[matchnum])
					break;

				// as long as this isn't the last entry, bump this one down
				if (matchnum < matches - 1)
				{
					penalty[matchnum + 1] = penalty[matchnum];
					list[matchnum + 1] = list[matchnum];
				}
				list[matchnum] = &swinfo;
				penalty[matchnum] = curpenalty;
			}
		}
	}
}


//-------------------------------------------------
//  release - reset to a pre-parsed state
//-------------------------------------------------

void software_list_device::release()
{
	if (m_parsed != nullptr)
	{
		osd_printf_verbose("Resetting %s\n", m_parsed->filename());
		m_parsed.release();
	}
}


//-------------------------------------------------
//  find_by_name - find a software list by name
//  across all software list devices
//-------------------------------------------------

software_list_device *software_list_device::find_by_name(const machine_config &config, const char *name)
{
	// iterate over each device in the system and find a match
	for (software_list_device &swlistdev : software_list_device_iterator(config.root_device()))
		if (strcmp(swlistdev.list_name(), name) == 0)
			return &swlistdev;
	return nullptr;
}


//-------------------------------------------------
//  software_display_matches - display a list of
//  possible matches in the system to the given
//  name, across all software list devices
//-------------------------------------------------

void software_list_device::display_matches(const machine_config &config, const char *interface, const char *name)
{
	// check if there is at least one software list
	software_list_device_iterator deviter(config.root_device());
	if (deviter.first() != nullptr)
		osd_printf_error("\n\"%s\" approximately matches the following\n"
							"supported software items (best match first):\n\n", name);

	// iterate through lists
	for (software_list_device &swlistdev : deviter)
	{
		// get the top 16 approximate matches for the selected device interface (i.e. only carts for cartslot, etc.)
		const software_info *matches[16] = { nullptr };
		swlistdev.find_approx_matches(name, ARRAY_LENGTH(matches), matches, interface);

		// if we found some, print them
		if (matches[0] != nullptr)
		{
			// different output depending on original system or compatible
			const char *description = swlistdev.parsed().description();
			if (swlistdev.list_type() == SOFTWARE_LIST_ORIGINAL_SYSTEM)
				osd_printf_error("* Software list \"%s\" (%s) matches: \n", swlistdev.list_name(), description);
			else
				osd_printf_error("* Compatible software list \"%s\" (%s) matches: \n", swlistdev.list_name(), description);

			// print them out
			for (auto &match : matches)
			{
				if (match != nullptr)
					osd_printf_error("%-18s%s\n", match->shortname().c_str(), match->longname().c_str());
			}

			osd_printf_error("\n");
		}
	}
}


//-------------------------------------------------
//  find - find an item by name in the software
//  list, using wildcards and optionally starting
//  from an intermediate point
//-------------------------------------------------

const software_info *software_list_device::find(const char *look_for)
{
	// nullptr search returns nothing
	if (look_for == nullptr)
		return nullptr;

	// cause a parse if needed
	parse();

	return m_parsed->find(look_for);
}


//-------------------------------------------------
//  parse - parse our softlist file (if needed)
//-------------------------------------------------

void software_list_device::parse()
{
	// skip if already parsed
	if (m_parsed != nullptr)
		return;

	// create a new parsed list
	m_parsed = std::make_unique<parsed_software_list>(mconfig().options().hash_path(), m_list_name.c_str());
}


//-------------------------------------------------
//  device_validity_check - validate the device
//  configuration
//-------------------------------------------------

void software_list_device::device_validity_check(validity_checker &valid) const
{
	// add to the global map whenever we check a list so we don't re-check
	// it in the future
	if (valid.already_checked(std::string("softlist/").append(m_list_name).c_str()))
		return;

	// do device validation only in case of validate command
	if (!valid.validate_all())
		return;

	// actually do the validate
	parsed_software_list parsed_list(mconfig().options().hash_path(), m_list_name.c_str());
	parsed_list.internal_validity_check(valid);
}
