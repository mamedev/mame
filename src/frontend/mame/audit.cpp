// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    audit.c

    ROM set auditing functions.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "audit.h"
#include "chd.h"
#include "drivenum.h"
#include "sound/samples.h"
#include "softlist.h"
#include "swinfo.h"

//**************************************************************************
//  CORE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  media_auditor - constructor
//-------------------------------------------------

media_auditor::media_auditor(const driver_enumerator &enumerator)
	: m_enumerator(enumerator)
	, m_validation(AUDIT_VALIDATE_FULL)
	, m_searchpath(nullptr)
{
}


//-------------------------------------------------
//  audit_media - audit the media described by the
//  currently-enumerated driver
//-------------------------------------------------

media_auditor::summary media_auditor::audit_media(const char *validation)
{
	// start fresh
	m_record_list.clear();

	// store validation for later
	m_validation = validation;

// temporary hack until romload is update: get the driver path and support it for
// all searches
const char *driverpath = m_enumerator.config().root_device().searchpath();

	std::size_t found = 0;
	std::size_t required = 0;
	std::size_t shared_found = 0;
	std::size_t shared_required = 0;

	// iterate over devices and regions
	for (device_t &device : device_iterator(m_enumerator.config().root_device()))
	{
		// determine the search path for this source and iterate through the regions
		m_searchpath = device.searchpath();

		// now iterate over regions and ROMs within
		for (const rom_entry *region = rom_first_region(device); region; region = rom_next_region(region))
		{
// temporary hack: add the driver path & region name
std::string combinedpath = util::string_format("%s;%s", device.searchpath(), driverpath);
if (device.shortname())
	combinedpath.append(";").append(device.shortname());
m_searchpath = combinedpath.c_str();

			for (const rom_entry *rom = rom_first_file(region); rom; rom = rom_next_file(rom))
			{
				char const *const name(ROM_GETNAME(rom));
				util::hash_collection const hashes(ROM_GETHASHDATA(rom));
				device_t *const shared_device(find_shared_device(device, name, hashes, ROM_GETLENGTH(rom)));

				// count the number of files with hashes
				if (!hashes.flag(util::hash_collection::FLAG_NO_DUMP) && !ROM_ISOPTIONAL(rom))
				{
					required++;
					if (shared_device)
						shared_required++;
				}

				audit_record *record = nullptr;
				if (ROMREGION_ISROMDATA(region))
					record = &audit_one_rom(rom);
				else if (ROMREGION_ISDISKDATA(region))
					record = &audit_one_disk(rom, nullptr);

				if (record)
				{
					// count the number of files that are found.
					if ((record->status() == audit_status::GOOD) || ((record->status() == audit_status::FOUND_INVALID) && !find_shared_device(device, name, record->actual_hashes(), record->actual_length())))
					{
						found++;
						if (shared_device)
							shared_found++;
					}

					record->set_shared_device(shared_device);
				}
			}
		}
	}

	// if we only find files that are in the parent & either the set has no unique files or the parent is not found, then assume we don't have the set at all
	if ((found == shared_found) && (required > 0) && ((required != shared_required) || (shared_found == 0)))
	{
		m_record_list.clear();
		return NOTFOUND;
	}

	// return a summary
	return summarize(m_enumerator.driver().name);
}


//-------------------------------------------------
//  audit_device - audit the device
//-------------------------------------------------

media_auditor::summary media_auditor::audit_device(device_t &device, const char *validation)
{
	// start fresh
	m_record_list.clear();

	// store validation for later
	m_validation = validation;
	m_searchpath = device.shortname();

	std::size_t found = 0;
	std::size_t required = 0;

	audit_regions(rom_first_region(device), nullptr, found, required);

	if ((found == 0) && (required > 0))
	{
		m_record_list.clear();
		return NOTFOUND;
	}

	// return a summary
	return summarize(device.shortname());
}


//-------------------------------------------------
//  audit_software
//-------------------------------------------------
media_auditor::summary media_auditor::audit_software(const char *list_name, const software_info *swinfo, const char *validation)
{
	// start fresh
	m_record_list.clear();

	// store validation for later
	m_validation = validation;

	std::string combinedpath(util::string_format("%s;%s%s%s", swinfo->shortname(), list_name, PATH_SEPARATOR, swinfo->shortname()));
	std::string locationtag(util::string_format("%s%%%s%%", list_name, swinfo->shortname()));
	if (!swinfo->parentname().empty())
	{
		locationtag.append(swinfo->parentname());
		combinedpath.append(util::string_format(";%s;%s%s%s", swinfo->parentname(), list_name, PATH_SEPARATOR, swinfo->parentname()));
	}
	m_searchpath = combinedpath.c_str();

	std::size_t found = 0;
	std::size_t required = 0;

	// now iterate over software parts
	for (const software_part &part : swinfo->parts())
		audit_regions(part.romdata(), locationtag.c_str(), found, required);

	if ((found == 0) && (required > 0))
	{
		m_record_list.clear();
		return NOTFOUND;
	}

	// return a summary
	return summarize(list_name);
}


//-------------------------------------------------
//  audit_samples - validate the samples for the
//  currently-enumerated driver
//-------------------------------------------------

media_auditor::summary media_auditor::audit_samples()
{
	// start fresh
	m_record_list.clear();

	std::size_t required = 0;
	std::size_t found = 0;

	// iterate over sample entries
	for (samples_device &device : samples_device_iterator(m_enumerator.config().root_device()))
	{
		// by default we just search using the driver name
		std::string searchpath(m_enumerator.driver().name);

		// add the alternate path if present
		samples_iterator iter(device);
		if (iter.altbasename() != nullptr)
			searchpath.append(";").append(iter.altbasename());

		// iterate over samples in this entry
		for (const char *samplename = iter.first(); samplename; samplename = iter.next())
		{
			required++;

			// create a new record
			audit_record &record = *m_record_list.emplace(m_record_list.end(), samplename, media_type::SAMPLE);

			// look for the files
			emu_file file(m_enumerator.options().sample_path(), OPEN_FLAG_READ | OPEN_FLAG_NO_PRELOAD);
			path_iterator path(searchpath.c_str());
			std::string curpath;
			while (path.next(curpath, samplename))
			{
				// attempt to access the file (.flac) or (.wav)
				osd_file::error filerr = file.open(curpath.c_str(), ".flac");
				if (filerr != osd_file::error::NONE)
					filerr = file.open(curpath.c_str(), ".wav");

				if (filerr == osd_file::error::NONE)
				{
					record.set_status(audit_status::GOOD, audit_substatus::GOOD);
					found++;
				}
				else
				{
					record.set_status(audit_status::NOT_FOUND, audit_substatus::NOT_FOUND);
				}
			}
		}
	}

	if ((found == 0) && (required > 0))
	{
		m_record_list.clear();
		return NOTFOUND;
	}

	// return a summary
	return summarize(m_enumerator.driver().name);
}


//-------------------------------------------------
//  summary - generate a summary, with an optional
//  string format
//-------------------------------------------------

media_auditor::summary media_auditor::summarize(const char *name, std::ostream *output) const
{
	if (m_record_list.empty())
		return NONE_NEEDED;

	// loop over records
	summary overall_status = CORRECT;
	for (audit_record const &record : m_record_list)
	{
		// skip anything that's fine
		if (record.substatus() == audit_substatus::GOOD)
			continue;

		// output the game name, file name, and length (if applicable)
		if (output)
		{
			util::stream_format(*output, "%-12s: %s", name, record.name());
			if (record.expected_length() > 0)
				util::stream_format(*output, " (%d bytes)", record.expected_length());
			*output << " - ";
		}

		// use the substatus for finer details
		summary best_new_status = INCORRECT;
		switch (record.substatus())
		{
		case audit_substatus::GOOD_NEEDS_REDUMP:
			if (output) *output << "NEEDS REDUMP\n";
			best_new_status = BEST_AVAILABLE;
			break;

		case audit_substatus::FOUND_NODUMP:
			if (output) *output << "NO GOOD DUMP KNOWN\n";
			best_new_status = BEST_AVAILABLE;
			break;

		case audit_substatus::FOUND_BAD_CHECKSUM:
			if (output)
			{
				util::stream_format(*output, "INCORRECT CHECKSUM:\n");
				util::stream_format(*output, "EXPECTED: %s\n", record.expected_hashes().macro_string());
				util::stream_format(*output, "   FOUND: %s\n", record.actual_hashes().macro_string());
			}
			break;

		case audit_substatus::FOUND_WRONG_LENGTH:
			if (output) util::stream_format(*output, "INCORRECT LENGTH: %d bytes\n", record.actual_length());
			break;

		case audit_substatus::NOT_FOUND:
			if (output)
			{
				device_t *const shared_device = record.shared_device();
				if (shared_device)
					util::stream_format(*output, "NOT FOUND (%s)\n", shared_device->shortname());
				else
					util::stream_format(*output, "NOT FOUND\n");
			}
			break;

		case audit_substatus::NOT_FOUND_NODUMP:
			if (output) *output << "NOT FOUND - NO GOOD DUMP KNOWN\n";
			best_new_status = BEST_AVAILABLE;
			break;

		case audit_substatus::NOT_FOUND_OPTIONAL:
			if (output) *output << "NOT FOUND BUT OPTIONAL\n";
			best_new_status = BEST_AVAILABLE;
			break;

		default:
			assert(false);
		}

		// downgrade the overall status if necessary
		overall_status = (std::max)(overall_status, best_new_status);
	}
	return overall_status;
}


//-------------------------------------------------
//  audit_regions - validate/count for regions
//-------------------------------------------------

void media_auditor::audit_regions(const rom_entry *region, const char *locationtag, std::size_t &found, std::size_t &required)
{
	// now iterate over regions
	for ( ; region; region = rom_next_region(region))
	{
		// now iterate over rom definitions
		for (const rom_entry *rom = rom_first_file(region); rom; rom = rom_next_file(rom))
		{
			util::hash_collection const hashes(ROM_GETHASHDATA(rom));

			// count the number of files with hashes
			if (!hashes.flag(util::hash_collection::FLAG_NO_DUMP) && !ROM_ISOPTIONAL(rom))
				required++;

			audit_record const *record = nullptr;
			if (ROMREGION_ISROMDATA(region))
				record = &audit_one_rom(rom);
			else if (ROMREGION_ISDISKDATA(region))
				record = &audit_one_disk(rom, locationtag);

			// count the number of files that are found.
			if (record && ((record->status() == audit_status::GOOD) || (record->status() == audit_status::FOUND_INVALID)))
				found++;
		}
	}
}


//-------------------------------------------------
//  audit_one_rom - validate a single ROM entry
//-------------------------------------------------

media_auditor::audit_record &media_auditor::audit_one_rom(const rom_entry *rom)
{
	// allocate and append a new record
	audit_record &record = *m_record_list.emplace(m_record_list.end(), *rom, media_type::ROM);

	// see if we have a CRC and extract it if so
	UINT32 crc = 0;
	bool const has_crc = record.expected_hashes().crc(crc);

	// find the file and checksum it, getting the file length along the way
	emu_file file(m_enumerator.options().media_path(), OPEN_FLAG_READ | OPEN_FLAG_NO_PRELOAD);
	file.set_restrict_to_mediapath(true);
	path_iterator path(m_searchpath);
	std::string curpath;
	while (path.next(curpath, record.name()))
	{
		// open the file if we can
		osd_file::error filerr;
		if (has_crc)
			filerr = file.open(curpath.c_str(), crc);
		else
			filerr = file.open(curpath.c_str());

		// if it worked, get the actual length and hashes, then stop
		if (filerr == osd_file::error::NONE)
		{
			record.set_actual(file.hashes(m_validation), file.size());
			break;
		}
	}

	// compute the final status
	compute_status(record, rom, record.actual_length() != 0);
	return record;
}


//-------------------------------------------------
//  audit_one_disk - validate a single disk entry
//-------------------------------------------------

media_auditor::audit_record &media_auditor::audit_one_disk(const rom_entry *rom, const char *locationtag)
{
	// allocate and append a new record
	audit_record &record = *m_record_list.emplace(m_record_list.end(), *rom, media_type::DISK);

	// open the disk
	chd_file source;
	chd_error err = chd_error(open_disk_image(m_enumerator.options(), &m_enumerator.driver(), rom, source, locationtag));

	// if we succeeded, get the hashes
	if (err == CHDERR_NONE)
	{
		util::hash_collection hashes;

		// if there's a SHA1 hash, add them to the output hash
		if (source.sha1() != util::sha1_t::null)
			hashes.add_sha1(source.sha1());

		// update the actual values
		record.set_actual(hashes);
	}

	// compute the final status
	compute_status(record, rom, err == CHDERR_NONE);
	return record;
}


//-------------------------------------------------
//  compute_status - compute a detailed status
//  based on the information we have
//-------------------------------------------------

void media_auditor::compute_status(audit_record &record, const rom_entry *rom, bool found)
{
	// if not found, provide more details
	if (!found)
	{
		if (record.expected_hashes().flag(util::hash_collection::FLAG_NO_DUMP))
			record.set_status(audit_status::NOT_FOUND, audit_substatus::NOT_FOUND_NODUMP);
		else if (ROM_ISOPTIONAL(rom))
			record.set_status(audit_status::NOT_FOUND, audit_substatus::NOT_FOUND_OPTIONAL);
		else
			record.set_status(audit_status::NOT_FOUND, audit_substatus::NOT_FOUND);
	}
	else
	{
		if (record.expected_length() != record.actual_length())
			record.set_status(audit_status::FOUND_INVALID, audit_substatus::FOUND_WRONG_LENGTH);
		else if (record.expected_hashes().flag(util::hash_collection::FLAG_NO_DUMP))
			record.set_status(audit_status::GOOD, audit_substatus::FOUND_NODUMP);
		else if (record.expected_hashes() != record.actual_hashes())
			record.set_status(audit_status::FOUND_INVALID, audit_substatus::FOUND_BAD_CHECKSUM);
		else if (record.expected_hashes().flag(util::hash_collection::FLAG_BAD_DUMP))
			record.set_status(audit_status::GOOD, audit_substatus::GOOD_NEEDS_REDUMP);
		else
			record.set_status(audit_status::GOOD, audit_substatus::GOOD);
	}
}


//-------------------------------------------------
//  find_shared_device - return the source that
//  shares a media entry with the same hashes
//-------------------------------------------------

device_t *media_auditor::find_shared_device(device_t &device, const char *name, const util::hash_collection &romhashes, UINT64 romlength)
{
	bool const dumped = !romhashes.flag(util::hash_collection::FLAG_NO_DUMP);

	// special case for non-root devices
	device_t *highest_device = nullptr;
	if (device.owner())
	{
		for (const rom_entry *region = rom_first_region(device); region; region = rom_next_region(region))
		{
			for (const rom_entry *rom = rom_first_file(region); rom; rom = rom_next_file(rom))
			{
				if (ROM_GETLENGTH(rom) == romlength)
				{
					util::hash_collection hashes(ROM_GETHASHDATA(rom));
					if ((dumped && hashes == romhashes) || (!dumped && ROM_GETNAME(rom) == name))
						highest_device = &device;
				}
			}
		}
	}
	else
	{
		// iterate up the parent chain
		for (auto drvindex = m_enumerator.find(m_enumerator.driver().parent); drvindex >= 0; drvindex = m_enumerator.find(m_enumerator.driver(drvindex).parent))
		{
			for (device_t &scandevice : device_iterator(m_enumerator.config(drvindex).root_device()))
			{
				for (const rom_entry *region = rom_first_region(scandevice); region; region = rom_next_region(region))
				{
					for (const rom_entry *rom = rom_first_file(region); rom; rom = rom_next_file(rom))
					{
						if (ROM_GETLENGTH(rom) == romlength)
						{
							util::hash_collection hashes(ROM_GETHASHDATA(rom));
							if ((dumped && hashes == romhashes) || (!dumped && ROM_GETNAME(rom) == name))
								highest_device = &scandevice;
						}
					}
				}
			}
		}
	}

	return highest_device;
}


//-------------------------------------------------
//  audit_record - constructor
//-------------------------------------------------

media_auditor::audit_record::audit_record(const rom_entry &media, media_type type)
	: m_type(type)
	, m_status(audit_status::UNVERIFIED)
	, m_substatus(audit_substatus::UNVERIFIED)
	, m_name(ROM_GETNAME(&media))
	, m_explength(rom_file_size(&media))
	, m_length(0)
	, m_exphashes()
	, m_hashes()
	, m_shared_device(nullptr)
{
	m_exphashes.from_internal_string(ROM_GETHASHDATA(&media));
}

media_auditor::audit_record::audit_record(const char *name, media_type type)
	: m_type(type)
	, m_status(audit_status::UNVERIFIED)
	, m_substatus(audit_substatus::UNVERIFIED)
	, m_name(name)
	, m_explength(0)
	, m_length(0)
	, m_exphashes()
	, m_hashes()
	, m_shared_device(nullptr)
{
}
