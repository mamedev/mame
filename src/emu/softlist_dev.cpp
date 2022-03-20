// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    softlist_dev.cpp

    Software list construction helpers.

***************************************************************************/

#include "emu.h"
#include "softlist_dev.h"

#include "diimage.h"
#include "emuopts.h"
#include "fileio.h"
#include "romload.h"
#include "validity.h"

#include "corestr.h"
#include "unicode.h"

#include <cctype>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef std::unordered_map<std::string, const software_info *> softlist_map;


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(SOFTWARE_LIST, software_list_device, "software_list", "Software List")

false_software_list_loader false_software_list_loader::s_instance;
rom_software_list_loader rom_software_list_loader::s_instance;
image_software_list_loader image_software_list_loader::s_instance;


//**************************************************************************
//  SOFTWARE LIST LOADER
//**************************************************************************

//-------------------------------------------------
//  false_software_list_loader::load_software
//-------------------------------------------------

bool false_software_list_loader::load_software(device_image_interface &image, software_list_device &swlist, std::string_view swname, const rom_entry *start_entry) const
{
	return false;
}


//-------------------------------------------------
//  rom_software_list_loader::load_software
//-------------------------------------------------

bool rom_software_list_loader::load_software(device_image_interface &image, software_list_device &swlist, std::string_view swname, const rom_entry *start_entry) const
{
	swlist.machine().rom_load().load_software_part_region(image.device(), swlist, swname, start_entry);
	return true;
}


//-------------------------------------------------
//  image_software_list_loader::load_software
//-------------------------------------------------

bool image_software_list_loader::load_software(device_image_interface &image, software_list_device &swlist, std::string_view swname, const rom_entry *start_entry) const
{
	return image.load_software(swlist, swname, start_entry);
}


//**************************************************************************
//  SOFTWARE LIST DEVICE
//**************************************************************************

//-------------------------------------------------
//  software_list_device - constructor
//-------------------------------------------------

software_list_device::software_list_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SOFTWARE_LIST, tag, owner, clock),
	m_list_type(softlist_type::ORIGINAL_SYSTEM),
	m_filter(nullptr),
	m_parsed(false),
	m_description("")
{
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

void software_list_device::find_approx_matches(std::string_view name, int matches, const software_info **list, const char *interface)
{
	// if no name, return
	if (name.empty())
		return;

	// initialize everyone's states
	std::vector<double> penalty(matches);
	for (int matchnum = 0; matchnum < matches; matchnum++)
	{
		penalty[matchnum] = 2.0;
		list[matchnum] = nullptr;
	}

	// iterate over our info (will cause a parse if needed)
	std::u32string const search(ustr_from_utf8(normalize_unicode(name, unicode_normalization_form::D, true)));
	for (const software_info &swinfo : get_info())
	{
		for (const software_part &swpart : swinfo.parts())
		{
			if ((interface == nullptr || swpart.matches_interface(interface)) && is_compatible(swpart) == SOFTWARE_IS_COMPATIBLE)
			{
				// pick the best match between driver name and description
				double const longpenalty = util::edit_distance(search, ustr_from_utf8(normalize_unicode(swinfo.longname(), unicode_normalization_form::D, true)));
				double const shortpenalty = util::edit_distance(search, ustr_from_utf8(normalize_unicode(swinfo.shortname(), unicode_normalization_form::D, true)));
				double const curpenalty = (std::min)(longpenalty, shortpenalty);

				// make sure it isn't already in the table
				bool skip = false;
				for (int matchnum = 0; !skip && (matchnum < matches) && list[matchnum]; matchnum++)
				{
					if ((penalty[matchnum] == curpenalty) && (swinfo.longname() == list[matchnum]->longname()) && (swinfo.shortname() == list[matchnum]->shortname()))
						skip = true;
				}

				if (!skip)
				{
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
	}
}


//-------------------------------------------------
//  release - reset to a pre-parsed state
//-------------------------------------------------

void software_list_device::release()
{
	osd_printf_verbose("%s: Resetting %s\n", tag(), m_list_name);
	m_parsed = false;
	m_filename.clear();
	m_shortname.clear();
	m_description.clear();
	m_errors.clear();
	m_infolist.clear();
}


//-------------------------------------------------
//  find_by_name - find a software list by name
//  across all software list devices
//-------------------------------------------------

software_list_device *software_list_device::find_by_name(const machine_config &config, std::string_view name)
{
	// iterate over each device in the system and find a match
	for (software_list_device &swlistdev : software_list_device_enumerator(config.root_device()))
		if (swlistdev.list_name() == name)
			return &swlistdev;
	return nullptr;
}


//-------------------------------------------------
//  software_display_matches - display a list of
//  possible matches in the system to the given
//  name, across all software list devices
//-------------------------------------------------

void software_list_device::display_matches(const machine_config &config, const char *interface, std::string_view name)
{
	// check if there is at least one software list
	software_list_device_enumerator deviter(config.root_device());
	if (deviter.first() != nullptr)
		osd_printf_error("\n\"%s\" approximately matches the following\n"
			"supported software items (best match first):\n\n", name);

	// iterate through lists
	for (software_list_device &swlistdev : deviter)
	{
		// get the top 16 approximate matches for the selected device interface (i.e. only carts for cartslot, etc.)
		const software_info *matches[16] = { nullptr };
		swlistdev.find_approx_matches(name, std::size(matches), matches, interface);

		// if we found some, print them
		if (matches[0] != nullptr)
		{
			// different output depending on original system or compatible
			if (swlistdev.is_original())
				osd_printf_error("* Software list \"%s\" (%s) matches: \n", swlistdev.list_name(), swlistdev.description());
			else
				osd_printf_error("* Compatible software list \"%s\" (%s) matches: \n", swlistdev.list_name(), swlistdev.description());

			// print them out
			for (auto &match : matches)
			{
				if (match != nullptr)
					osd_printf_error("%-18s%s\n", match->shortname(), match->longname());
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

const software_info *software_list_device::find(const std::string &look_for)
{
	// empty search returns nothing
	if (look_for.empty())
		return nullptr;

	const bool iswild = look_for.find_first_of("*?") != std::string::npos;

	// find a match (will cause a parse if needed when calling get_info)
	const auto &info_list = get_info();
	auto iter = std::find_if(
			info_list.begin(),
			info_list.end(),
			[&look_for, iswild] (const software_info &info)
			{
				const char *shortname = info.shortname().c_str();
				return (iswild && core_strwildcmp(look_for.c_str(), shortname) == 0)
						|| core_stricmp(look_for.c_str(), shortname) == 0;
			});

	return iter != info_list.end() ? &*iter : nullptr;
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
	emu_file file(mconfig().options().hash_path(), OPEN_FLAG_READ);
	const std::error_condition filerr = file.open(m_list_name + ".xml");
	m_filename = file.filename();
	if (!filerr)
	{
		// parse if no error
		std::ostringstream errs;
		parse_software_list(file, m_filename, m_shortname, m_description, m_infolist, errs);
		file.close();
		m_errors = errs.str();
	}
	else if (std::errc::no_such_file_or_directory == filerr)
	{
		osd_printf_verbose("%s: Software list %s not found\n", tag(), m_filename);
	}
	else
	{
		m_errors = string_format("Error opening file: %s\n", m_filename);
	}

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

device_image_interface *software_list_device::find_mountable_image(const machine_config &mconfig, const software_part &part, std::function<bool(const device_image_interface &)> filter)
{
	// if automount="no", don't bother
	const char *mount = part.feature("automount");
	if (mount != nullptr && strcmp(mount, "no") == 0)
		return nullptr;

	for (device_image_interface &image : image_interface_enumerator(mconfig.root_device()))
	{
		const char *interface = image.image_interface();
		if (interface != nullptr && part.matches_interface(interface) && filter(image))
			return &image;
	}
	return nullptr;
}


//-------------------------------------------------
//  find_mountable_image - find an image interface
//  that can automatically mount this software part
//-------------------------------------------------

device_image_interface *software_list_device::find_mountable_image(const machine_config &mconfig, const software_part &part)
{
	// Multi-part softlists will distribute individual images serially (e.g. - first floppy to flop1, next one to flop2
	// etc).  Pre MAME 0.183 relied on the core doing this distribution between calls to find_mountable_image() so it
	// could check to see if the slot was empty.
	//
	// When softlists were refactored in MAME 0.183, this was changed to build a "plan" for what needs to be loaded, so
	// it was incorrect to check the image slot.  This is why an overload for find_mountable_image() was created that
	// takes an std::function.  This overload is being preserved for compatibility with existing code, but I regard the
	// continued existence of this overload as a red flag.
	return find_mountable_image(
		mconfig,
		part,
		[](const device_image_interface &image) { return !image.exists(); });
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

	// skip in case of quick validation
	if (!valid.quick())
		const_cast<software_list_device *>(this)->internal_validity_check(valid);
}


//-------------------------------------------------
//  internal_validity_check - internal helper to
//  check the list
//-------------------------------------------------

void software_list_device::internal_validity_check(validity_checker &valid)
{
	enum { NAME_LEN_LIST = 24, NAME_LEN_PARENT = 16, NAME_LEN_CLONE = 16, NAME_LEN_PART = 16  };
	auto const valid_name_char = [] (char ch) { return ((ch >= '0') && (ch <= '9')) || ((ch >= 'a') && (ch <= 'z')) || (ch == '_'); };
	auto const valid_tag_char = [] (char ch) { return ((ch >= '0') && (ch <= '9')) || ((ch >= 'a') && (ch <= 'z')) || strchr("$.:_", u8(ch)); };
	auto const valid_year_char = [] (char ch) { return isdigit(u8(ch)) || (ch == '?') || (ch == '+'); };
	auto const valid_label_char = [] (char ch) { return (ch >= ' ') && (ch <= '~') && !strchr("!$%/:\\", u8(ch)); };

	// first parse and output core errors if any
	auto const &info(get_info());
	if (!m_errors.empty())
	{
		osd_printf_error("%s: Errors parsing software list:\n%s", m_filename, m_errors);
		release();
		return;
	}

	// ignore empty shortname to work around ignoring missing software lists
	if (!m_shortname.empty())
	{
		if (m_list_name != m_shortname)
			osd_printf_error("%s: Software list name %s does not match filename %s\n", m_filename, m_shortname, m_list_name);

		if (m_shortname.length() > NAME_LEN_LIST)
			osd_printf_error("%s: %s software list name must be %d characters or less\n", m_filename, m_shortname, NAME_LEN_LIST);
	}

	// now check the software items
	softlist_map names;
	softlist_map descriptions;
	for (const software_info &swinfo : info)
	{
		std::string const &shortname(swinfo.shortname());

		if (swinfo.longname().empty())
			osd_printf_error("%s: %s has no description\n", m_filename, shortname);

		if (swinfo.year().empty())
			osd_printf_error("%s: %s has no year\n", m_filename, shortname);

		if (swinfo.publisher().empty())
			osd_printf_error("%s: %s has no publisher\n", m_filename, shortname);

		// Did we lost the software parts?
		if (swinfo.parts().empty())
			osd_printf_error("%s: %s has no parts\n", m_filename, shortname);

		// Second, since the xml is fine, run additional checks:

		// check for duplicate names
		auto const dupname(names.emplace(shortname, &swinfo));
		if (!dupname.second)
			osd_printf_error("%s: %s is a duplicate name (%s)\n", m_filename, shortname, dupname.first->second->shortname());

		// check for duplicate descriptions
		auto const dupdesc(descriptions.emplace(swinfo.longname(), &swinfo));
		if (!dupdesc.second)
			osd_printf_error("%s: %s has duplicate description '%s' (%s)\n", m_filename, shortname, swinfo.longname(), dupdesc.first->second->shortname());

		bool const is_clone(!swinfo.parentname().empty());
		if (is_clone)
		{
			if (swinfo.parentname() == shortname)
			{
				osd_printf_error("%s: %s is set as a clone of itself\n", m_filename, shortname);
			}
			else
			{
				software_info const *const parent = find(swinfo.parentname());
				if (!parent)
					osd_printf_error("%s: %s is a clone of non-existent parent %s\n", m_filename, shortname, swinfo.parentname());
				else if (!parent->parentname().empty())
					osd_printf_error("%s: %s is a clone %s which is a clone of %s\n", m_filename, shortname, swinfo.parentname(), parent->parentname());
			}
		}

		// make sure the driver name isn't too long
		if (shortname.length() > (is_clone ? NAME_LEN_CLONE : NAME_LEN_PARENT))
		{
			osd_printf_error(
					"%s: %s %s software name must be %d characters or less\n",
					m_filename,
					shortname,
					is_clone ? "clone" : "parent", is_clone ? NAME_LEN_CLONE : NAME_LEN_PARENT);
		}

		// make sure the driver name doesn't contain invalid characters
		if (std::find_if_not(shortname.begin(), shortname.end(), valid_name_char) != shortname.end())
			osd_printf_error("%s: %s contains invalid characters\n", m_filename, shortname);

		// make sure the year is only digits, '?' or '+'
		if (std::find_if_not(swinfo.year().begin(), swinfo.year().end(), valid_year_char) != swinfo.year().end())
			osd_printf_error("%s: %s has an invalid year '%s'\n", m_filename, shortname, swinfo.year());

		std::set<std::string> part_names;
		for (software_part const &part : swinfo.parts())
		{
			if (part.interface().empty())
				osd_printf_error("%s: %s part %s has no interface\n", m_filename, shortname, part.name());

			if (part.romdata().empty())
				osd_printf_error("%s: %s part %s has no data areas\n", m_filename, shortname, part.name());

			if (!part_names.emplace(part.name()).second)
				osd_printf_error("%s: %s part %s has duplicate name\n", m_filename, shortname, part.name());

			if (part.name().length() > NAME_LEN_PART)
				osd_printf_error("%s: %s part %s name must be %d characters or less\n", m_filename, shortname, part.name(), NAME_LEN_PART);

			if (std::find_if_not(part.name().begin(), part.name().end(), valid_name_char) != part.name().end())
				osd_printf_error("%s: %s part %s contains invalid characters\n", m_filename, shortname, part.name());

			// validate data areas
			// based on ROM validation code from validity.cpp but adapted to work with rom_entry and ignore unavailable features like BIOS
			if (!part.romdata().empty())
			{
				std::map<std::string, u32> data_area_map;
				char const *last_region_name = "???";
				char const *last_name = "???";
				u32 current_length = 0;
				int items_since_region = 1;
				for (rom_entry const *romp = &part.romdata().front(); romp && !ROMENTRY_ISEND(romp); ++romp)
				{
					if (ROMENTRY_ISREGION(romp)) // if this is a region, make sure it's valid, and record the length
					{
						// if we haven't seen any items since the last region, print a warning
						if (!items_since_region)
							osd_printf_verbose("%s: %s part %s has empty data area '%s' (warning)\n", m_filename, shortname, part.name(), last_region_name);

						// reset our region tracking states
						items_since_region = (ROMREGION_ISERASE(romp) || ROMREGION_ISDISKDATA(romp)) ? 1 : 0;
						last_region_name = romp->name().c_str();

						// check for a valid tag
						if (romp->name().size() < MIN_TAG_LENGTH)
							osd_printf_error("%s: %s part %s data area name '%s' is too short (must be at least %d characters)\n", m_filename, shortname, part.name(), romp->name(), MIN_TAG_LENGTH);

						if (std::find_if_not(romp->name().begin(), romp->name().end(), valid_tag_char) != romp->name().end())
							osd_printf_error("%s: %s part %s data area name '%s' contains invalid characters\n", m_filename, shortname, part.name(), romp->name());

						// attempt to add it to the map, reporting duplicates as errors
						current_length = ROMREGION_GETLENGTH(romp);
						if (!data_area_map.emplace(romp->name(), current_length).second)
							osd_printf_error("%s: %s part %s data area has duplicate name '%s'\n", m_filename, shortname, part.name(), romp->name());
					}
					else if (ROMENTRY_ISFILE(romp)) // if this is a file, make sure it is properly formatted
					{
						// track the last ROM label we found
						last_name = romp->name().c_str();

						// validate the name
						if (romp->name().length() > 127)
							osd_printf_error("%s: %s part %s ROM label '%s' exceeds maximum 127 characters\n", m_filename, shortname, part.name(), romp->name());
						if (std::find_if_not(romp->name().begin(), romp->name().end(), valid_label_char) != romp->name().end())
							osd_printf_error("%s: %s part %s ROM label '%s' contains invalid characters\n", m_filename, shortname, part.name(), romp->name());

						// make sure the hash is valid
						util::hash_collection hashes;
						if (!hashes.from_internal_string(romp->hashdata()))
							osd_printf_error("%s: %s part %s ROM '%s' has invalid hash string '%s'\n", m_filename, shortname, part.name(), romp->name(), romp->hashdata());
					}

					// for any non-region ending entries, make sure they don't extend past the end
					if (!ROMENTRY_ISREGIONEND(romp) && current_length > 0)
					{
						items_since_region++;
						if (!ROMENTRY_ISIGNORE(romp) && (ROM_GETOFFSET(romp) + ROM_GETLENGTH(romp) > current_length))
							osd_printf_error("%s: %s part %s ROM '%s' extends past the defined data area\n", m_filename, shortname, part.name(), last_name);
					}
				}

				// if we haven't seen any items since the last region, print a warning
				if (!items_since_region)
					osd_printf_verbose("%s: %s part %s has empty data area '%s' (warning)\n", m_filename, shortname, part.name(), last_region_name);
			}
		}
	}

	// discard parsed info
	release();
}
