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

void media_identifier::file_info::match(
		device_t const &device,
		romload::file const &rom,
		util::hash_collection const &hashes)
{
	if (hashes == m_hashes)
	{
		m_matches.emplace_back(
				device.shortname(),
				device.name(),
				rom.get_name(),
				hashes.flag(util::hash_collection::FLAG_BAD_DUMP),
				device.owner());
	}
}

void media_identifier::file_info::match(
		std::string const &list,
		software_info const &software,
		rom_entry const &rom,
		util::hash_collection const &hashes)
{
	if (hashes == m_hashes)
	{
		m_matches.emplace_back(
				util::string_format("%s:%s", list, software.shortname()),
				std::string(software.longname()),
				ROM_GETNAME(&rom),
				hashes.flag(util::hash_collection::FLAG_BAD_DUMP),
				false);
	}
}


//-------------------------------------------------
//  media_identifier - constructor
//-------------------------------------------------

media_identifier::media_identifier(emu_options &options)
	: m_drivlist(options)
	, m_total(0)
	, m_matches(0)
	, m_nonroms(0)
{
}


//-------------------------------------------------
//  identify - identify a directory, ZIP file,
//  or raw file
//-------------------------------------------------

void media_identifier::identify(const char *filename)
{
	std::vector<file_info> info;
	collect_files(info, filename);
	match_hashes(info);
	print_results(info);
}


//-------------------------------------------------
//  identify_file - identify a file
//-------------------------------------------------

void media_identifier::identify_file(const char *name)
{
	std::vector<file_info> info;
	digest_file(info, name);
	match_hashes(info);
	print_results(info);
}


//-------------------------------------------------
//  identify_data - identify a buffer full of
//  data; if it comes from a .JED file, parse the
//  fusemap into raw data first
//-------------------------------------------------

void media_identifier::identify_data(const char *name, const uint8_t *data, std::size_t length)
{
	std::vector<file_info> info;
	digest_data(info, name, data, length);
	match_hashes(info);
	print_results(info);
}


//-------------------------------------------------
//  collect_files - pre-process files for
//  identification
//-------------------------------------------------

void media_identifier::collect_files(std::vector<file_info> &info, char const *path)
{
	// first try to open as a directory
	osd::directory::ptr const directory = osd::directory::open(path);
	if (directory)
	{
		// iterate over all files in the directory
		for (osd::directory::entry const *entry = directory->read(); entry; entry = directory->read())
		{
			if (entry->type == osd::directory::entry::entry_type::FILE)
			{
				std::string const curfile = std::string(path).append(PATH_SEPARATOR).append(entry->name);
				collect_files(info, curfile.c_str());
			}
		}
	}
	else if (core_filename_ends_with(path, ".7z") || core_filename_ends_with(path, ".zip"))
	{
		// first attempt to examine it as a valid zip/7z file
		util::archive_file::ptr archive;
		util::archive_file::error err;
		if (core_filename_ends_with(path, ".7z"))
			err = util::archive_file::open_7z(path, archive);
		else
			err = util::archive_file::open_zip(path, archive);

		if ((util::archive_file::error::NONE == err) && archive)
		{
			std::vector<std::uint8_t> data;

			// loop over entries in the .7z, skipping empty files and directories
			for (int i = archive->first_file(); i >= 0; i = archive->next_file())
			{
				std::uint64_t const length(archive->current_uncompressed_length());
				if (!archive->current_is_directory() && length)
				{
					std::string const curfile = std::string(path).append(PATH_SEPARATOR).append(archive->current_name());
					if (std::uint32_t(length) == length)
					{
						// decompress data into RAM and identify it
						try
						{
							data.resize(std::size_t(length));
							err = archive->decompress(&data[0], std::uint32_t(length));
							if (util::archive_file::error::NONE == err)
								digest_data(info, curfile.c_str(), &data[0], length);
							else
								osd_printf_error("%s: error decompressing file\n", curfile.c_str());
						}
						catch (...)
						{
							// resizing the buffer could cause a bad_alloc if archive contains large files
							osd_printf_error("%s: error decompressing file\n", curfile.c_str());
						}
						data.clear();
					}
					else
					{
						osd_printf_error("%s: file too large to decompress into memory\n", curfile.c_str());
					}
				}
			}
		}
		else
		{
			osd_printf_error("%s: error opening archive\n", path);
		}

		// clear out any cached files
		util::archive_file::cache_clear();
	}
	else
	{
		// otherwise, identify as a raw file
		digest_file(info, path);
	}
}


//-------------------------------------------------
//  digest_file - calculate hashes for a single
//  file
//-------------------------------------------------

void media_identifier::digest_file(std::vector<file_info> &info, char const *path)
{
	// CHD files need to be parsed and their hashes extracted from the header
	if (core_filename_ends_with(path, ".chd"))
	{
		// attempt to open as a CHD; fail if not
		chd_file chd;
		chd_error const err = chd.open(path);
		m_total++;
		if (err != CHDERR_NONE)
		{
			osd_printf_info("%-20sNOT A CHD\n", core_filename_extract_base(path).c_str());
			m_nonroms++;
		}
		else if (!chd.compressed())
		{
			osd_printf_info("%-20sis a writeable CHD\n", core_filename_extract_base(path).c_str());
		}
		else
		{
			// otherwise, get the hash collection for this CHD
			util::hash_collection hashes;
			if (chd.sha1() != util::sha1_t::null)
				hashes.add_sha1(chd.sha1());
			info.emplace_back(path, chd.logical_bytes(), std::move(hashes), file_flavour::CHD);
		}
	}
	else
	{
		// if this is a '.jed' file, process it into raw bits first
		if (core_filename_ends_with(path, ".jed"))
		{
			// load the file and process if it opens and has a valid length
			uint32_t length;
			void *data;
			if (osd_file::error::NONE == util::core_file::load(path, &data, length))
			{
				jed_data jed;
				if (JEDERR_NONE == jed_parse(data, length, &jed))
				{
					try
					{
						// now determine the new data length and allocate temporary memory for it
						std::vector<uint8_t> tempjed(jedbin_output(&jed, nullptr, 0));
						jedbin_output(&jed, &tempjed[0], tempjed.size());
						util::hash_collection hashes;
						hashes.compute(&tempjed[0], tempjed.size(), util::hash_collection::HASH_TYPES_CRC_SHA1);
						info.emplace_back(path, tempjed.size(), std::move(hashes), file_flavour::JED);
						free(data);
						m_total++;
						return;
					}
					catch (...)
					{
					}
				}
				free(data);
			}
		}

		// load the file and process if it opens and has a valid length
		util::core_file::ptr file;
		if ((osd_file::error::NONE == util::core_file::open(path, OPEN_FLAG_READ, file)) && file)
		{
			util::hash_collection hashes;
			hashes.begin(util::hash_collection::HASH_TYPES_CRC_SHA1);
			std::uint8_t buf[1024];
			for (std::uint64_t remaining = file->size(); remaining; )
			{
				std::uint32_t const block = std::min<std::uint64_t>(remaining, sizeof(buf));
				if (file->read(buf, block) < block)
				{
					osd_printf_error("%s: error reading file\n", path);
					return;
				}
				remaining -= block;
				hashes.buffer(buf, block);
			}
			hashes.end();
			info.emplace_back(path, file->size(), std::move(hashes), file_flavour::RAW);
			m_total++;
		}
		else
		{
			osd_printf_error("%s: error opening file\n", path);
		}
	}
}


//-------------------------------------------------
//  digest_data - calculate hashes for data in
//  memory
//-------------------------------------------------

void media_identifier::digest_data(std::vector<file_info> &info, char const *name, void const *data, std::uint64_t length)
{
	util::hash_collection hashes;

	// if this is a '.jed' file, process it into raw bits first
	if (core_filename_ends_with(name, ".jed"))
	{
		jed_data jed;
		if (JEDERR_NONE == jed_parse(data, length, &jed))
		{
			try
			{
				// now determine the new data length and allocate temporary memory for it
				std::vector<uint8_t> tempjed(jedbin_output(&jed, nullptr, 0));
				jedbin_output(&jed, &tempjed[0], tempjed.size());
				hashes.compute(&tempjed[0], tempjed.size(), util::hash_collection::HASH_TYPES_CRC_SHA1);
				info.emplace_back(name, tempjed.size(), std::move(hashes), file_flavour::JED);
				m_total++;
				return;
			}
			catch (...)
			{
			}
		}
	}

	hashes.compute(reinterpret_cast<std::uint8_t const *>(data), length, util::hash_collection::HASH_TYPES_CRC_SHA1);
	info.emplace_back(name, length, std::move(hashes), file_flavour::RAW);
	m_total++;
}


//-------------------------------------------------
//  match_hashes - find known dumps that mach
//  collected hashes
//-------------------------------------------------

void media_identifier::match_hashes(std::vector<file_info> &info)
{
	std::unordered_set<std::string> listnames;

	// iterate over drivers
	m_drivlist.reset();
	while (m_drivlist.next())
	{
		// iterate over regions and files within the region
		device_t &device = m_drivlist.config()->root_device();
		for (romload::region const &region : romload::entries(device.rom_region()).get_regions())
		{
			for (romload::file const &rom : region.get_files())
			{
				util::hash_collection const romhashes(rom.get_hashdata());
				if (!romhashes.flag(util::hash_collection::FLAG_NO_DUMP))
				{
					for (file_info &file : info)
						file.match(device, rom, romhashes);
				}
			}
		}

		// next iterate over softlists
		for (software_list_device &swlistdev : software_list_device_iterator(device))
		{
			if (listnames.insert(swlistdev.list_name()).second)
			{
				for (software_info const &swinfo : swlistdev.get_info())
				{
					for (software_part const &part : swinfo.parts())
					{
						for (rom_entry const *region = part.romdata().data(); region; region = rom_next_region(region))
						{
							for (rom_entry const *rom = rom_first_file(region); rom; rom = rom_next_file(rom))
							{
								util::hash_collection romhashes(ROM_GETHASHDATA(rom));
								if (!romhashes.flag(util::hash_collection::FLAG_NO_DUMP))
								{
									for (file_info &file : info)
										file.match(swlistdev.list_name(), swinfo, *rom, romhashes);
								}
							}
						}
					}
				}
			}
		}
	}

	// iterator over devices
	machine_config config(GAME_NAME(___empty), m_drivlist.options());
	machine_config::token const tok(config.begin_configuration(config.root_device()));
	for (device_type type : registered_device_types)
	{
		// iterate over regions and files within the region
		device_t *const device = config.device_add("_tmp", type, 0);
		for (romload::region const &region : romload::entries(device->rom_region()).get_regions())
		{
			for (romload::file const &rom : region.get_files())
			{
				util::hash_collection const romhashes(rom.get_hashdata());
				if (!romhashes.flag(util::hash_collection::FLAG_NO_DUMP))
				{
					for (file_info &file : info)
						file.match(*device, rom, romhashes);
				}
			}
		}
		config.device_remove("_tmp");
	}
}


//-------------------------------------------------
//  print_results - print info on files that were
//  found to match known dumps
//-------------------------------------------------

void media_identifier::print_results(std::vector<file_info> const &info)
{
	for (file_info const &file : info)
	{
		osd_printf_info("%-20s", core_filename_extract_base(file.name()).c_str());
		if (file.matches().empty())
		{
			osd_printf_info("NO MATCH\n");
		}
		else
		{
			bool first = true;
			m_matches++;
			for (match_data const &match : file.matches())
			{
				if (!first)
					osd_printf_info("%-20s", "");
				first = false;
				osd_printf_info(
						"= %s%-20s  %-10s %s%s\n",
						match.bad() ? "(BAD) " : "",
						match.romname().c_str(),
						match.shortname().c_str(),
						match.fullname().c_str(),
						match.device() ? " (device)" : "");
			}
		}
	}
}
