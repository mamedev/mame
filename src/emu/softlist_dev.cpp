// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

	softlist_dev.cpp

	Software list construction helpers.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "diimage.h"
#include "softlist_dev.h"
#include "validity.h"

#include <ctype.h>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef std::unordered_map<std::string, const software_info *> softlist_map;


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type SOFTWARE_LIST = &device_creator<software_list_device>;
false_software_list_loader false_software_list_loader::s_instance;
rom_software_list_loader rom_software_list_loader::s_instance;
image_software_list_loader image_software_list_loader::s_instance;


//**************************************************************************
//  SOFTWARE LIST LOADER
//**************************************************************************

//-------------------------------------------------
//  false_software_list_loader::load_software
//-------------------------------------------------

bool false_software_list_loader::load_software(device_image_interface &device, software_list_device &swlist, const char *swname, const rom_entry *start_entry) const
{
	return false;
}


//-------------------------------------------------
//  rom_software_list_loader::load_software
//-------------------------------------------------

bool rom_software_list_loader::load_software(device_image_interface &device, software_list_device &swlist, const char *swname, const rom_entry *start_entry) const
{
	swlist.machine().rom_load().load_software_part_region(device, swlist, swname, start_entry);
	return true;
}


//-------------------------------------------------
//  image_software_list_loader::load_software
//-------------------------------------------------

bool image_software_list_loader::load_software(device_image_interface &device, software_list_device &swlist, const char *swname, const rom_entry *start_entry) const
{
	return device.load_software(swlist, swname, start_entry);
}


//**************************************************************************
//  SOFTWARE LIST DEVICE
//**************************************************************************

//-------------------------------------------------
//  software_list_device - constructor
//-------------------------------------------------

software_list_device::software_list_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SOFTWARE_LIST, "Software list", tag, owner, clock, "software_list", __FILE__),
	m_list_type(SOFTWARE_LIST_ORIGINAL_SYSTEM),
	m_filter(nullptr),
	m_parsed(false),
	m_file(mconfig.options().hash_path(), OPEN_FLAG_READ),
	m_description("")
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

void software_list_device::find_approx_matches(const std::string &name, int matches, const software_info **list, const char *interface)
{
	// if no name, return
	if (name.empty())
		return;

	// initialize everyone's states
	std::vector<int> penalty(matches);
	for (int matchnum = 0; matchnum < matches; matchnum++)
	{
		penalty[matchnum] = 9999;
		list[matchnum] = nullptr;
	}

	// iterate over our info (will cause a parse if needed)
	for (const software_info &swinfo : get_info())
	{
		const software_part &part = swinfo.parts().front();
		if ((interface == nullptr || part.matches_interface(interface)) && is_compatible(part) == SOFTWARE_IS_COMPATIBLE)
		{
			// pick the best match between driver name and description
			int longpenalty = driver_list::penalty_compare(name.c_str(), swinfo.longname().c_str());
			int shortpenalty = driver_list::penalty_compare(name.c_str(), swinfo.shortname().c_str());
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
	osd_printf_verbose("Resetting %s\n", m_file.filename());
	m_parsed = false;
	m_description.clear();
	m_errors.clear();
	m_infolist.clear();
}


//-------------------------------------------------
//  find_by_name - find a software list by name
//  across all software list devices
//-------------------------------------------------

software_list_device *software_list_device::find_by_name(const machine_config &config, const std::string &name)
{
	// iterate over each device in the system and find a match
	for (software_list_device &swlistdev : software_list_device_iterator(config.root_device()))
		if (swlistdev.list_name() == name)
			return &swlistdev;
	return nullptr;
}


//-------------------------------------------------
//  software_display_matches - display a list of
//  possible matches in the system to the given
//  name, across all software list devices
//-------------------------------------------------

void software_list_device::display_matches(const machine_config &config, const char *interface, const std::string &name)
{
	// check if there is at least one software list
	software_list_device_iterator deviter(config.root_device());
	if (deviter.first() != nullptr)
		osd_printf_error("\n\"%s\" approximately matches the following\n"
			"supported software items (best match first):\n\n", name.c_str());

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
			if (swlistdev.list_type() == SOFTWARE_LIST_ORIGINAL_SYSTEM)
				osd_printf_error("* Software list \"%s\" (%s) matches: \n", swlistdev.list_name().c_str(), swlistdev.description());
			else
				osd_printf_error("* Compatible software list \"%s\" (%s) matches: \n", swlistdev.list_name().c_str(), swlistdev.description());

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

	bool iswild = strchr(look_for, '*') != nullptr || strchr(look_for, '?');

	// find a match (will cause a parse if needed when calling get_info)
	const auto &info_list = get_info();
	auto iter = std::find_if(
		info_list.begin(),
		info_list.end(),
		[&](const software_info &info)
	{
		const char *shortname = info.shortname().c_str();
		return (iswild && core_strwildcmp(look_for, shortname) == 0)
			|| core_stricmp(look_for, shortname) == 0;
	});

	return iter != info_list.end()
		? &*iter
		: nullptr;
}


//-------------------------------------------------
//  parse - parse our softlist file
//-------------------------------------------------

void software_list_device::parse()
{
	// skip if done
	if (m_parsed)
		return;

	// reset the errors
	m_errors.clear();

	// attempt to open the file
	osd_file::error filerr = m_file.open(m_list_name.c_str(), ".xml");
	if (filerr == osd_file::error::NONE)
	{
		// parse if no error
		std::ostringstream errs;
		softlist_parser parser(m_file, m_description, m_infolist, errs);
		m_file.close();
		m_errors = errs.str();
	}
	else
		m_errors = string_format("Error opening file: %s\n", filename());

	// indicate that we've been parsed
	m_parsed = true;
}


//-------------------------------------------------
//  is_compatible - determine if we are compatible
//  with the given software_list_device
//-------------------------------------------------

software_compatibility software_list_device::is_compatible(const software_part &swpart) const
{
	// get the softlist filter; if null, assume compatible
	if (m_filter == nullptr)
		return SOFTWARE_IS_COMPATIBLE;

	// copy the comma-delimited string and ensure it ends with a final comma
	std::string filt = std::string(m_filter).append(",");

	// get the incompatibility filter and test against it first if it exists
	const char *incompatibility = swpart.feature("incompatibility");
	if (incompatibility != nullptr)
	{
		// copy the comma-delimited string and ensure it ends with a final comma
		std::string incomp = std::string(incompatibility).append(",");

		// iterate over filter items and see if they exist in the list; if so, it's incompatible
		for (int start = 0, end = filt.find_first_of(',', start); end != -1; start = end + 1, end = filt.find_first_of(',', start))
		{
			std::string token(filt, start, end - start + 1);
			if (incomp.find(token) != -1)
				return SOFTWARE_IS_INCOMPATIBLE;
		}
	}

	// get the compatibility feature; if null, assume compatible
	const char *compatibility = swpart.feature("compatibility");
	if (compatibility == nullptr)
		return SOFTWARE_IS_COMPATIBLE;

	// copy the comma-delimited string and ensure it ends with a final comma
	std::string comp = std::string(compatibility).append(",");

	// iterate over filter items and see if they exist in the compatibility list; if so, it's compatible
	for (int start = 0, end = filt.find_first_of(',', start); end != -1; start = end + 1, end = filt.find_first_of(',', start))
	{
		std::string token(filt, start, end - start + 1);
		if (comp.find(token) != -1)
			return SOFTWARE_IS_COMPATIBLE;
	}
	return SOFTWARE_NOT_COMPATIBLE;
}


//-------------------------------------------------
//  find_mountable_image - find an image interface
//  that can automatically mount this software part
//-------------------------------------------------

device_image_interface *software_list_device::find_mountable_image(const machine_config &mconfig, const software_part &part)
{
	// if automount="no", don't bother
	const char *mount = part.feature("automount");
	if (mount != nullptr && strcmp(mount, "no") == 0)
		return nullptr;

	for (device_image_interface &image : image_interface_iterator(mconfig.root_device()))
	{
		const char *interface = image.image_interface();
		if (interface != nullptr && part.matches_interface(interface))
		{
			// mount only if not already mounted
			const char *option = mconfig.options().value(image.brief_instance_name());
			if (*option == '\0' && !image.filename())
				return &image;
		}
	}
	return nullptr;
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
	const_cast<software_list_device *>(this)->internal_validity_check(valid);
}


//-------------------------------------------------
//  internal_validity_check - internal helper to
//  check the list
//-------------------------------------------------

void software_list_device::internal_validity_check(validity_checker &valid)
{
	enum { NAME_LEN_PARENT = 8, NAME_LEN_CLONE = 16 };

	softlist_map names;
	softlist_map descriptions;
	for (const software_info &swinfo : get_info())
	{
		// first parse and output core errors if any
		if (m_errors.length() > 0)
		{
			osd_printf_error("%s: Errors parsing software list:\n%s", filename(), errors_string());
			break;
		}

		// Now check if the xml data is valid:

		// Did we lost any description?
		if (swinfo.longname().empty())
		{
			osd_printf_error("%s: %s has no description\n", filename(), swinfo.shortname().c_str());
			break;
		}

		// Did we lost any year?
		if (swinfo.year().empty())
		{
			osd_printf_error("%s: %s has no year\n", filename(), swinfo.shortname().c_str());
			break;
		}

		// Did we lost any publisher?
		if (swinfo.publisher().empty())
		{
			osd_printf_error("%s: %s has no publisher\n", filename(), swinfo.shortname().c_str());
			break;
		}

		// Did we lost the software parts?
		if (swinfo.parts().empty())
		{
			osd_printf_error("%s: %s has no part\n", filename(), swinfo.shortname().c_str());
			break;
		}

		// Second, since the xml is fine, run additional checks:

		// check for duplicate names
		if (!names.insert(std::make_pair(swinfo.shortname(), &swinfo)).second)
		{
			const software_info *match = names.find(swinfo.shortname())->second;
			osd_printf_error("%s: %s is a duplicate name (%s)\n", filename(), swinfo.shortname().c_str(), match->shortname().c_str());
		}

		// check for duplicate descriptions
		std::string longname = std::string(swinfo.longname());
		if (!descriptions.insert(std::make_pair(strmakelower(longname), &swinfo)).second)
			osd_printf_error("%s: %s is a duplicate description (%s)\n", filename(), swinfo.longname().c_str(), swinfo.shortname().c_str());

		bool is_clone = false;
		if (!swinfo.parentname().empty())
		{
			is_clone = true;
			if (swinfo.parentname() == swinfo.shortname())
			{
				osd_printf_error("%s: %s is set as a clone of itself\n", filename(), swinfo.shortname().c_str());
				break;
			}

			// make sure the parent exists
			const software_info *swinfo2 = find(swinfo.parentname().c_str());

			if (swinfo2 == nullptr)
				osd_printf_error("%s: parent '%s' software for '%s' not found\n", filename(), swinfo.parentname().c_str(), swinfo.shortname().c_str());
			else if (!swinfo2->parentname().empty())
				osd_printf_error("%s: %s is a clone of a clone\n", filename(), swinfo.shortname().c_str());
		}

		// make sure the driver name is 8 chars or less
		if ((is_clone && swinfo.shortname().length() > NAME_LEN_CLONE) || (!is_clone && swinfo.shortname().length() > NAME_LEN_PARENT))
			osd_printf_error("%s: %s %s driver name must be %d characters or less\n", filename(), swinfo.shortname().c_str(),
				is_clone ? "clone" : "parent", is_clone ? NAME_LEN_CLONE : NAME_LEN_PARENT);

		// make sure the year is only digits, '?' or '+'
		for (const char *s = swinfo.year().c_str(); *s != 0; s++)
			if (!isdigit((UINT8)*s) && *s != '?' && *s != '+')
			{
				osd_printf_error("%s: %s has an invalid year '%s'\n", filename(), swinfo.shortname().c_str(), swinfo.year().c_str());
				break;
			}

		softlist_map part_names;
		for (const software_part &part : swinfo.parts())
		{
			if (part.interface().empty())
				osd_printf_error("%s: %s has a part (%s) without interface\n", filename(), swinfo.shortname().c_str(), part.name().c_str());

			if (part.romdata().empty())
				osd_printf_error("%s: %s has a part (%s) with no data\n", filename(), swinfo.shortname().c_str(), part.name().c_str());

			if (!part_names.insert(std::make_pair(part.name(), &swinfo)).second)
				osd_printf_error("%s: %s has a part (%s) whose name is duplicate\n", filename(), swinfo.shortname().c_str(), part.name().c_str());
		}
	}

	// release all the memory
	release();
}


