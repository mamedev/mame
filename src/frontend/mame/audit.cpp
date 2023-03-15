// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    audit.cpp

    ROM set auditing functions.

***************************************************************************/

#include "emu.h"
#include "audit.h"

#include "sound/samples.h"

#include "emuopts.h"
#include "drivenum.h"
#include "fileio.h"
#include "romload.h"
#include "softlist_dev.h"

#include "chd.h"
#include "path.h"

#include <algorithm>

//#define VERBOSE 1
#define LOG_OUTPUT_FUNC osd_printf_verbose
#include "logmacro.h"


namespace {

struct parent_rom
{
	parent_rom(device_type t, rom_entry const *r) : type(t), name(r->name()), hashes(r->hashdata()), length(rom_file_size(r)) { }

	std::reference_wrapper<std::remove_reference_t<device_type> >   type;
	std::string                                                     name;
	util::hash_collection                                           hashes;
	uint64_t                                                        length;
};


class parent_rom_vector : public std::vector<parent_rom>
{
public:
	using std::vector<parent_rom>::vector;

	void remove_redundant_parents()
	{
		while (!empty())
		{
			// find where the next parent starts
			auto const last(
					std::find_if(
						std::next(cbegin()),
						cend(),
						[this] (parent_rom const &r) { return &front().type.get() != &r.type.get(); }));

			// examine dumped ROMs in this generation
			for (auto i = cbegin(); last != i; ++i)
			{
				if (!i->hashes.flag(util::hash_collection::FLAG_NO_DUMP))
				{
					auto const match(
							std::find_if(
								last,
								cend(),
								[&i] (parent_rom const &r) { return (i->length == r.length) && (i->hashes == r.hashes); }));
					if (cend() == match)
						return;
				}
			}
			erase(cbegin(), last);
		}
	}

	std::add_pointer_t<device_type> find_shared_device(device_t &current, std::string_view name, util::hash_collection const &hashes, uint64_t length) const
	{
		// if we're examining a child device, it will always have a perfect match
		if (current.owner())
			return &current.type();

		// scan backwards through parents for a matching definition
		bool const dumped(!hashes.flag(util::hash_collection::FLAG_NO_DUMP));
		std::add_pointer_t<device_type> best(nullptr);
		for (const_reverse_iterator it = crbegin(); crend() != it; ++it)
		{
			if (it->length == length)
			{
				if (dumped)
				{
					if (it->hashes == hashes)
						return &it->type.get();
				}
				else if (it->name == name)
				{
					if (it->hashes.flag(util::hash_collection::FLAG_NO_DUMP))
						return &it->type.get();
					else if (!best)
						best = &it->type.get();
				}
			}
		}
		return best;
	}

	std::pair<std::add_pointer_t<device_type>, bool> actual_matches_shared(device_t &current, media_auditor::audit_record const &record)
	{
		// no result if no matching file was found
		if ((record.status() != media_auditor::audit_status::GOOD) && (record.status() != media_auditor::audit_status::FOUND_INVALID))
			return std::make_pair(nullptr, false);

		// if we're examining a child device, scan it first
		bool matches_device_undumped(false);
		if (current.owner())
		{
			for (const rom_entry *region = rom_first_region(current); region; region = rom_next_region(region))
			{
				for (const rom_entry *rom = rom_first_file(region); rom; rom = rom_next_file(rom))
				{
					if (rom_file_size(rom) == record.actual_length())
					{
						util::hash_collection const hashes(rom->hashdata());
						if (hashes == record.actual_hashes())
							return std::make_pair(&current.type(), empty());
						else if (hashes.flag(util::hash_collection::FLAG_NO_DUMP) && (rom->name() == record.name()))
							matches_device_undumped = true;
					}
				}
			}
		}

		// look for a matching parent ROM
		std::add_pointer_t<device_type> closest_bad(nullptr);
		for (const_reverse_iterator it = crbegin(); crend() != it; ++it)
		{
			if (it->length == record.actual_length())
			{
				if (it->hashes == record.actual_hashes())
					return std::make_pair(&it->type.get(), it->type.get() == front().type.get());
				else if (it->hashes.flag(util::hash_collection::FLAG_NO_DUMP) && (it->name == record.name()))
					closest_bad = &it->type.get();
			}
		}

		// fall back to the nearest bad dump
		if (closest_bad)
			return std::make_pair(closest_bad, front().type.get() == *closest_bad);
		else if (matches_device_undumped)
			return std::make_pair(&current.type(), empty());
		else
			return std::make_pair(nullptr, false);
	}
};

} // anonymous namespace



//**************************************************************************
//  CORE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  media_auditor - constructor
//-------------------------------------------------

media_auditor::media_auditor(const driver_enumerator &enumerator)
	: m_enumerator(enumerator)
	, m_validation(AUDIT_VALIDATE_FULL)
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

	// first walk the parent chain for required ROMs
	parent_rom_vector parentroms;
	for (auto drvindex = m_enumerator.find(m_enumerator.driver().parent); 0 <= drvindex; drvindex = m_enumerator.find(m_enumerator.driver(drvindex).parent))
	{
		game_driver const &parent(m_enumerator.driver(drvindex));
		LOG("Checking parent %s for ROM files\n", parent.type.shortname());
		std::vector<rom_entry> const roms(rom_build_entries(parent.rom));
		for (rom_entry const *region = rom_first_region(&roms.front()); region; region = rom_next_region(region))
		{
			for (rom_entry const *rom = rom_first_file(region); rom; rom = rom_next_file(rom))
			{
				LOG("Adding parent ROM %s\n", rom->name());
				parentroms.emplace_back(parent.type, rom);
			}
		}
	}
	parentroms.remove_redundant_parents();

	// count ROMs required/found
	std::size_t found(0);
	std::size_t required(0);
	std::size_t shared_found(0);
	std::size_t shared_required(0);
	std::size_t parent_found(0);

	// iterate over devices and regions
	std::vector<std::string> searchpath;
	for (device_t &device : device_enumerator(m_enumerator.config()->root_device()))
	{
		searchpath.clear();

		// now iterate over regions and ROMs within
		for (const rom_entry *region = rom_first_region(device); region; region = rom_next_region(region))
		{
			for (const rom_entry *rom = rom_first_file(region); rom; rom = rom_next_file(rom))
			{
				if (searchpath.empty())
				{
					LOG("Audit media for device %s(%s)\n", device.shortname(), device.tag());
					searchpath = device.searchpath();
				}

				// look for a matching parent or device ROM
				std::string const &name(rom->name());
				util::hash_collection const hashes(rom->hashdata());
				bool const dumped(!hashes.flag(util::hash_collection::FLAG_NO_DUMP));
				std::add_pointer_t<device_type> const shared_device(parentroms.find_shared_device(device, name, hashes, rom_file_size(rom)));
				if (shared_device)
					LOG("File '%s' %s%sdumped shared with %s\n", name, ROM_ISOPTIONAL(rom) ? "optional " : "", dumped ? "" : "un", shared_device->shortname());
				else
					LOG("File '%s' %s%sdumped\n", name, ROM_ISOPTIONAL(rom) ? "optional " : "", dumped ? "" : "un");

				// count the number of files with hashes
				if (dumped && !ROM_ISOPTIONAL(rom))
				{
					required++;
					if (shared_device)
						shared_required++;
				}

				audit_record *record(nullptr);
				if (ROMREGION_ISROMDATA(region))
					record = &audit_one_rom(searchpath, rom);
				else if (ROMREGION_ISDISKDATA(region))
					record = &audit_one_disk(rom, device);

				if (record)
				{
					// see if the actual content found belongs to a parent
					auto const matchesshared(parentroms.actual_matches_shared(device, *record));
					if (matchesshared.first)
						LOG("Actual ROM file shared with %sparent %s\n", matchesshared.second ? "immediate " : "", matchesshared.first->shortname());

					// count the number of files that are found.
					if ((record->status() == audit_status::GOOD) || ((record->status() == audit_status::FOUND_INVALID) && !matchesshared.first))
					{
						found++;
						if (shared_device)
							shared_found++;
						if (matchesshared.second)
							parent_found++;
					}

					record->set_shared_device(shared_device);
				}
			}
		}

		if (!searchpath.empty())
			LOG("Total required=%u (shared=%u) found=%u (shared=%u parent=%u)\n", required, shared_required, found, shared_found, parent_found);
	}

	// if we only find files that are in the parent & either the set has no unique files or the parent is not found, then assume we don't have the set at all
	if ((found == shared_found) && required && ((required != shared_required) || !parent_found))
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

	std::size_t found = 0;
	std::size_t required = 0;

	std::vector<std::string> searchpath;
	audit_regions(
			[this, &device, &searchpath] (rom_entry const *region, rom_entry const *rom) -> audit_record const *
			{
				if (ROMREGION_ISROMDATA(region))
				{
					if (searchpath.empty())
						searchpath = device.searchpath();
					return &audit_one_rom(searchpath, rom);
				}
				else if (ROMREGION_ISDISKDATA(region))
				{
					return &audit_one_disk(rom, device);
				}
				else
				{
					return nullptr;
				}
			},
			rom_first_region(device),
			found,
			required);

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
media_auditor::summary media_auditor::audit_software(software_list_device &swlist, const software_info &swinfo, const char *validation)
{
	// start fresh
	m_record_list.clear();

	// store validation for later
	m_validation = validation;

	std::size_t found = 0;
	std::size_t required = 0;

	// now iterate over software parts
	std::vector<std::string> searchpath;
	auto const do_audit =
			[this, &swlist, &swinfo, &searchpath] (rom_entry const *region, rom_entry const *rom) -> audit_record const *
			{
				if (ROMREGION_ISROMDATA(region))
				{
					if (searchpath.empty())
						searchpath = rom_load_manager::get_software_searchpath(swlist, swinfo);
					return &audit_one_rom(searchpath, rom);
				}
				else if (ROMREGION_ISDISKDATA(region))
				{
					return &audit_one_disk(rom, swlist, swinfo);
				}
				else
				{
					return nullptr;
				}
			};
	for (const software_part &part : swinfo.parts())
		audit_regions(do_audit, part.romdata().data(), found, required);

	if ((found == 0) && (required > 0))
	{
		m_record_list.clear();
		return NOTFOUND;
	}

	// return a summary
	return summarize(swlist.list_name().c_str());
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
	for (samples_device &device : samples_device_enumerator(m_enumerator.config()->root_device()))
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
			path_iterator path(searchpath);
			std::string curpath;
			while (path.next(curpath))
			{
				util::path_append(curpath, samplename);

				// attempt to access the file (.flac) or (.wav)
				std::error_condition filerr = file.open(curpath + ".flac");
				if (filerr)
					filerr = file.open(curpath + ".wav");

				if (!filerr)
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
			if (name)
				util::stream_format(*output, "%-12s: %s", name, record.name());
			else
				util::stream_format(*output, "%s", record.name());
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
				std::add_pointer_t<device_type> const shared_device = record.shared_device();
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

template <typename T>
void media_auditor::audit_regions(T do_audit, const rom_entry *region, std::size_t &found, std::size_t &required)
{
	// now iterate over regions
	std::vector<std::string> searchpath;
	for ( ; region; region = rom_next_region(region))
	{
		// now iterate over rom definitions
		for (rom_entry const *rom = rom_first_file(region); rom; rom = rom_next_file(rom))
		{
			// count the number of files with hashes
			util::hash_collection const hashes(rom->hashdata());
			if (!hashes.flag(util::hash_collection::FLAG_NO_DUMP) && !ROM_ISOPTIONAL(rom))
				required++;

			audit_record const *const record = do_audit(region, rom);

			// count the number of files that are found.
			if (record && ((record->status() == audit_status::GOOD) || (record->status() == audit_status::FOUND_INVALID)))
				found++;
		}
	}
}


//-------------------------------------------------
//  audit_one_rom - validate a single ROM entry
//-------------------------------------------------

media_auditor::audit_record &media_auditor::audit_one_rom(const std::vector<std::string> &searchpath, const rom_entry *rom)
{
	// allocate and append a new record
	audit_record &record = *m_record_list.emplace(m_record_list.end(), *rom, media_type::ROM);

	// see if we have a CRC and extract it if so
	uint32_t crc = 0;
	bool const has_crc = record.expected_hashes().crc(crc);

	// find the file and checksum it, getting the file length along the way
	emu_file file(m_enumerator.options().media_path(), searchpath, OPEN_FLAG_READ | OPEN_FLAG_NO_PRELOAD);
	file.set_restrict_to_mediapath(1);

	// open the file if we can
	std::error_condition filerr;
	if (has_crc)
		filerr = file.open(record.name(), crc);
	else
		filerr = file.open(record.name());

	// if it worked, get the actual length and hashes, then stop
	if (!filerr)
		record.set_actual(file.hashes(m_validation), file.size());

	// compute the final status
	compute_status(record, rom, record.actual_length() != 0);
	return record;
}


//-------------------------------------------------
//  audit_one_disk - validate a single disk entry
//-------------------------------------------------

template <typename... T>
media_auditor::audit_record &media_auditor::audit_one_disk(const rom_entry *rom, T &&... args)
{
	// allocate and append a new record
	audit_record &record = *m_record_list.emplace(m_record_list.end(), *rom, media_type::DISK);

	// open the disk
	chd_file source;
	const std::error_condition err = rom_load_manager::open_disk_image(m_enumerator.options(), std::forward<T>(args)..., rom, source);

	// if we succeeded, get the hashes
	if (!err)
	{
		util::hash_collection hashes;

		// if there's a SHA1 hash, add them to the output hash
		if (source.sha1() != util::sha1_t::null)
			hashes.add_sha1(source.sha1());

		// update the actual values
		record.set_actual(hashes);
	}

	// compute the final status
	compute_status(record, rom, !err);
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
//  audit_record - constructor
//-------------------------------------------------

media_auditor::audit_record::audit_record(const rom_entry &media, media_type type)
	: m_type(type)
	, m_status(audit_status::UNVERIFIED)
	, m_substatus(audit_substatus::UNVERIFIED)
	, m_name(media.name())
	, m_explength(rom_file_size(&media))
	, m_length(0)
	, m_exphashes(media.hashdata())
	, m_hashes()
	, m_shared_device(nullptr)
{
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
