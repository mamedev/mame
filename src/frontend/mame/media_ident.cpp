// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    media_ident.c

    Media identify.

***************************************************************************/

#include "emu.h"
#include "drivenum.h"
#include "media_ident.h"
#include "unzip.h"
#include "jedparse.h"
#include "softlist_dev.h"


//**************************************************************************
//  MEDIA IDENTIFIER
//**************************************************************************

//-------------------------------------------------
//  media_identifier - constructor
//-------------------------------------------------

media_identifier::media_identifier(emu_options &options)
	: m_drivlist(options),
		m_total(0),
		m_matches(0),
		m_nonroms(0)
{
}


//-------------------------------------------------
//  identify - identify a directory, ZIP file,
//  or raw file
//-------------------------------------------------

void media_identifier::identify(const char *filename)
{
	// first try to open as a directory
	osd::directory::ptr directory = osd::directory::open(filename);
	if (directory)
	{
		// iterate over all files in the directory
		for (const osd::directory::entry *entry = directory->read(); entry != nullptr; entry = directory->read())
			if (entry->type == osd::directory::entry::entry_type::FILE)
			{
				std::string curfile = std::string(filename).append(PATH_SEPARATOR).append(entry->name);
				identify(curfile.c_str());
			}

		// close the directory and be done
		directory.reset();
	}

	// if that failed, and the filename ends with .zip, identify as a ZIP file
	if (core_filename_ends_with(filename, ".7z") || core_filename_ends_with(filename, ".zip"))
	{
		// first attempt to examine it as a valid _7Z file
		util::archive_file::ptr archive;
		util::archive_file::error err;
		if (core_filename_ends_with(filename, ".7z"))
			err = util::archive_file::open_7z(filename, archive);
		else
			err = util::archive_file::open_zip(filename, archive);
		if ((err == util::archive_file::error::NONE) && archive)
		{
			std::vector<std::uint8_t> data;

			// loop over entries in the .7z, skipping empty files and directories
			for (int i = archive->first_file(); i >= 0; i = archive->next_file())
			{
				const std::uint64_t length(archive->current_uncompressed_length());
				if (!archive->current_is_directory() && (length != 0) && (std::uint32_t(length) == length))
				{
					// decompress data into RAM and identify it
					try
					{
						data.resize(std::size_t(length));
						err = archive->decompress(&data[0], std::uint32_t(length));
						if (err == util::archive_file::error::NONE)
							identify_data(archive->current_name().c_str(), &data[0], length);
					}
					catch (...)
					{
						// resizing the buffer could cause a bad_alloc if archive contains large files
					}
					data.clear();
				}
			}
		}

		// clear out any cached files
		archive.reset();
		util::archive_file::cache_clear();
	}

	// otherwise, identify as a raw file
	else
		identify_file(filename);
}


//-------------------------------------------------
//  identify_file - identify a file
//-------------------------------------------------

void media_identifier::identify_file(const char *name)
{
	// CHD files need to be parsed and their hashes extracted from the header
	if (core_filename_ends_with(name, ".chd"))
	{
		// output the name
		osd_printf_info("%-20s", core_filename_extract_base(name).c_str());
		m_total++;

		// attempt to open as a CHD; fail if not
		chd_file chd;
		chd_error err = chd.open(name);
		if (err != CHDERR_NONE)
		{
			osd_printf_info("NOT A CHD\n");
			m_nonroms++;
			return;
		}

		// error on writable CHDs
		if (!chd.compressed())
		{
			osd_printf_info("is a writeable CHD\n");
			return;
		}

		// otherwise, get the hash collection for this CHD
		util::hash_collection hashes;
		if (chd.sha1() != util::sha1_t::null)
			hashes.add_sha1(chd.sha1());

		// determine whether this file exists
		int found = find_by_hash(hashes, chd.logical_bytes());
		if (found == 0)
			osd_printf_info("NO MATCH\n");
		else
			m_matches++;
	}

	// all other files have their hashes computed directly
	else
	{
		// load the file and process if it opens and has a valid length
		uint32_t length;
		void *data;
		const osd_file::error filerr = util::core_file::load(name, &data, length);
		if (filerr == osd_file::error::NONE && length > 0)
		{
			identify_data(name, reinterpret_cast<uint8_t *>(data), length);
			free(data);
		}
	}
}


//-------------------------------------------------
//  identify_data - identify a buffer full of
//  data; if it comes from a .JED file, parse the
//  fusemap into raw data first
//-------------------------------------------------

void media_identifier::identify_data(const char *name, const uint8_t *data, int length)
{
	// if this is a '.jed' file, process it into raw bits first
	std::vector<uint8_t> tempjed;
	jed_data jed;
	if (core_filename_ends_with(name, ".jed") && jed_parse(data, length, &jed) == JEDERR_NONE)
	{
		// now determine the new data length and allocate temporary memory for it
		length = jedbin_output(&jed, nullptr, 0);
		tempjed.resize(length);
		jedbin_output(&jed, &tempjed[0], length);
		data = &tempjed[0];
	}

	// compute the hash of the data
	util::hash_collection hashes;
	hashes.compute(data, length, util::hash_collection::HASH_TYPES_CRC_SHA1);

	// output the name
	m_total++;
	osd_printf_info("%-20s", core_filename_extract_base(name).c_str());

	// see if we can find a match in the ROMs
	int found = find_by_hash(hashes, length);

	// if we didn't find it, try to guess what it might be
	if (found == 0)
		osd_printf_info("NO MATCH\n");

	// if we did find it, count it as a match
	else
		m_matches++;
}


//-------------------------------------------------
//  find_by_hash - scan for a file in the list
//  of drivers by hash
//-------------------------------------------------

int media_identifier::find_by_hash(const util::hash_collection &hashes, int length)
{
	int found = 0;
	std::unordered_set<std::string> listnames;
	std::unordered_set<std::string> shortnames;

	// iterate over drivers
	m_drivlist.reset();
	while (m_drivlist.next())
	{
		// iterate over devices, regions and files within the region
		for (device_t &device : device_iterator(m_drivlist.config()->root_device()))
		{
			if (shortnames.insert(device.shortname()).second)
			{
				for (const rom_entry *region = rom_first_region(device); region != nullptr; region = rom_next_region(region))
					for (const rom_entry *rom = rom_first_file(region); rom != nullptr; rom = rom_next_file(rom))
					{
						util::hash_collection romhashes(ROM_GETHASHDATA(rom));
						if (!romhashes.flag(util::hash_collection::FLAG_NO_DUMP) && hashes == romhashes)
						{
							bool baddump = romhashes.flag(util::hash_collection::FLAG_BAD_DUMP);

							// output information about the match
							if (found)
								osd_printf_info("                    ");
							osd_printf_info("= %s%-20s  %-10s %s%s\n", baddump ? "(BAD) " : "",
								ROM_GETNAME(rom), device.shortname(), device.name(),
								device.owner() != nullptr ? " (device)" : "");
							found++;
						}
					}
			}
		}

		// next iterate over softlists
		for (software_list_device &swlistdev : software_list_device_iterator(m_drivlist.config()->root_device()))
		{
			if (listnames.insert(swlistdev.list_name()).second)
			{
				for (const software_info &swinfo : swlistdev.get_info())
					for (const software_part &part : swinfo.parts())
						for (const rom_entry *region = part.romdata().data(); region != nullptr; region = rom_next_region(region))
							for (const rom_entry *rom = rom_first_file(region); rom != nullptr; rom = rom_next_file(rom))
							{
								util::hash_collection romhashes(ROM_GETHASHDATA(rom));
								if (hashes == romhashes)
								{
									bool baddump = romhashes.flag(util::hash_collection::FLAG_BAD_DUMP);

									// output information about the match
									if (found)
										osd_printf_info("                    ");
									osd_printf_info("= %s%-20s  %s:%s %s\n", baddump ? "(BAD) " : "", ROM_GETNAME(rom), swlistdev.list_name().c_str(), swinfo.shortname().c_str(), swinfo.longname().c_str());
									found++;
								}
							}
			}
		}
	}

	return found;
}

