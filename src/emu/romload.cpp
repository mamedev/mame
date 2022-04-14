// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Paul Priest,Aaron Giles,Vas Crabb
/*********************************************************************

    romload.cpp

    ROM loading functions.

*********************************************************************/

#include "emu.h"
#include "romload.h"

#include "drivenum.h"
#include "emuopts.h"
#include "fileio.h"
#include "softlist_dev.h"
#include "ui/uimain.h"

#include "corestr.h"

#include <algorithm>
#include <set>


#define LOG_LOAD 0
#define LOG(...) do { if (LOG_LOAD) debugload(__VA_ARGS__); } while(0)


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define TEMPBUFFER_MAX_SIZE     (1024 * 1024 * 1024)

/***************************************************************************
    HELPERS
****************************************************************************/

namespace {

auto next_parent_system(game_driver const &system)
{
	return
			[sys = &system, roms = std::vector<rom_entry>()] () mutable -> rom_entry const *
			{
				if (!sys)
					return nullptr;
				int const parent(driver_list::find(sys->parent));
				if (0 > parent)
				{
					sys = nullptr;
					return nullptr;
				}
				else
				{
					sys = &driver_list::driver(parent);
					roms = rom_build_entries(sys->rom);
					return &roms[0];
				}
			};
}


auto next_parent_software(std::vector<software_info const *> const &parents)
{
	auto part(parents.front()->parts().end());
	return
			[&parents, current = parents.cbegin(), part, end = part] () mutable -> const rom_entry *
			{
				if (part == end)
				{
					if (parents.end() == current)
						return nullptr;
					part = (*current)->parts().cbegin();
					end = (*current)->parts().cend();
					do { ++current; } while ((parents.cend() != current) && (*current)->parts().empty());
				}
				return &(*part++).romdata()[0];
			};
}


std::vector<std::string> make_software_searchpath(software_list_device &swlist, software_info const &swinfo, std::vector<software_info const *> &parents)
{
	std::vector<std::string> result;

	// search <rompath>/<list>/<software> following parents
	for (software_info const *i = &swinfo; i; )
	{
		if (std::find(parents.begin(), parents.end(), i) != parents.end())
			break;
		parents.emplace_back(i);
		result.emplace_back(util::string_format("%s" PATH_SEPARATOR "%s", swlist.list_name(), i->shortname()));
		i = i->parentname().empty() ? nullptr : swlist.find(i->parentname());
	}

	// search <rompath>/<software> following parents
	for (software_info const *i : parents)
		result.emplace_back(i->shortname());

	return result;
}


std::error_condition do_open_disk(const emu_options &options, std::initializer_list<std::reference_wrapper<const std::vector<std::string> > > searchpath, const rom_entry *romp, chd_file &chd, std::function<const rom_entry * ()> next_parent)
{
	// hashes are fixed, but we might need to try multiple filenames
	std::set<std::string> tried;
	const util::hash_collection hashes(romp->hashdata());
	std::string filename, fullpath;
	const rom_entry *parent(nullptr);
	std::error_condition result(std::errc::no_such_file_or_directory);
	while (romp && result)
	{
		filename = romp->name() + ".chd";
		if (tried.insert(filename).second)
		{
			// piggyback on emu_file to find the disk image file
			std::unique_ptr<emu_file> imgfile;
			for (const std::vector<std::string> &paths : searchpath)
			{
				imgfile.reset(new emu_file(options.media_path(), paths, OPEN_FLAG_READ));
				imgfile->set_restrict_to_mediapath(1);
				const std::error_condition filerr(imgfile->open(filename, OPEN_FLAG_READ));
				if (!filerr)
					break;
				else
					imgfile.reset();
			}

			// if we couldn't open a candidate file, report an error; otherwise reopen it as a CHD
			if (imgfile)
			{
				fullpath = imgfile->fullpath();
				imgfile.reset();
				result = chd.open(fullpath);
			}
		}

		// walk the parents looking for a CHD with the same hashes but a different name
		if (result)
		{
			while (romp)
			{
				// find a file in a disk region
				if (parent)
					romp = rom_next_file(romp);
				while (!parent || !romp)
				{
					if (!parent)
					{
						parent = next_parent();
						if (!parent)
						{
							romp = nullptr;
							break;
						}
						while (ROMENTRY_ISPARAMETER(parent) || ROMENTRY_ISSYSTEM_BIOS(parent) || ROMENTRY_ISDEFAULT_BIOS(parent))
							++parent;
						if (ROMENTRY_ISEND(parent))
							parent = nullptr;
					}
					else
					{
						parent = rom_next_region(parent);
					}
					while (parent && !ROMREGION_ISDISKDATA(parent))
						parent = rom_next_region(parent);
					if (parent)
						romp = rom_first_file(parent);
				}

				// try it if it matches the hashes
				if (romp && (util::hash_collection(romp->hashdata()) == hashes))
					break;
			}
		}
	}
	return result;
}

} // anonymous namespace


/***************************************************************************
    ROM LOADING
***************************************************************************/

/*-------------------------------------------------
    rom_first_region - return pointer to first ROM
    region
-------------------------------------------------*/

const rom_entry *rom_first_region(const device_t &device)
{
	return rom_first_region(&device.rom_region_vector().front());
}

const rom_entry *rom_first_region(const rom_entry *romp)
{
	while (ROMENTRY_ISPARAMETER(romp) || ROMENTRY_ISSYSTEM_BIOS(romp) || ROMENTRY_ISDEFAULT_BIOS(romp))
		romp++;
	return !ROMENTRY_ISEND(romp) ? romp : nullptr;
}


/*-------------------------------------------------
    rom_next_region - return pointer to next ROM
    region
-------------------------------------------------*/

const rom_entry *rom_next_region(const rom_entry *romp)
{
	romp++;
	while (!ROMENTRY_ISREGIONEND(romp))
		romp++;
	while (ROMENTRY_ISPARAMETER(romp))
		romp++;
	return ROMENTRY_ISEND(romp) ? nullptr : romp;
}


/*-------------------------------------------------
    rom_first_file - return pointer to first ROM
    file
-------------------------------------------------*/

const rom_entry *rom_first_file(const rom_entry *romp)
{
	romp++;
	while (!ROMENTRY_ISFILE(romp) && !ROMENTRY_ISREGIONEND(romp))
		romp++;
	return ROMENTRY_ISREGIONEND(romp) ? nullptr : romp;
}


/*-------------------------------------------------
    rom_next_file - return pointer to next ROM
    file
-------------------------------------------------*/

const rom_entry *rom_next_file(const rom_entry *romp)
{
	romp++;
	while (!ROMENTRY_ISFILE(romp) && !ROMENTRY_ISREGIONEND(romp))
		romp++;
	return ROMENTRY_ISREGIONEND(romp) ? nullptr : romp;
}


/*-------------------------------------------------
    rom_first_parameter - return pointer to the first
    per-game parameter
-------------------------------------------------*/

const rom_entry *rom_first_parameter(const device_t &device)
{
	const rom_entry *romp = &device.rom_region_vector().front();
	while (romp && !ROMENTRY_ISEND(romp) && !ROMENTRY_ISPARAMETER(romp))
		romp++;
	return (romp != nullptr && !ROMENTRY_ISEND(romp)) ? romp : nullptr;
}


/*-------------------------------------------------
    rom_next_parameter - return pointer to the next
    per-game parameter
-------------------------------------------------*/

const rom_entry *rom_next_parameter(const rom_entry *romp)
{
	romp++;
	while (!ROMENTRY_ISREGIONEND(romp) && !ROMENTRY_ISPARAMETER(romp))
		romp++;
	return ROMENTRY_ISEND(romp) ? nullptr : romp;
}


/*-------------------------------------------------
    rom_file_size - return the expected size of a
    file given the ROM description
-------------------------------------------------*/

u32 rom_file_size(const rom_entry *romp)
{
	u32 maxlength = 0;

	/* loop until we run out of reloads */
	do
	{
		/* loop until we run out of continues/ignores */
		u32 curlength = ROM_GETLENGTH(romp++);
		while (ROMENTRY_ISCONTINUE(romp) || ROMENTRY_ISIGNORE(romp))
			curlength += ROM_GETLENGTH(romp++);

		/* track the maximum length */
		maxlength = std::max(maxlength, curlength);
	}
	while (ROMENTRY_ISRELOAD(romp));

	return maxlength;
}


/*-------------------------------------------------
    debugload - log data to a file
-------------------------------------------------*/

static void CLIB_DECL ATTR_PRINTF(1,2) debugload(const char *string, ...)
{
	static int opened;
	va_list arg;
	FILE *f;

	f = fopen("romload.log", opened++ ? "a" : "w");
	if (f)
	{
		va_start(arg, string);
		vfprintf(f, string, arg);
		va_end(arg);
		fclose(f);
	}
}


/***************************************************************************
    HARD DISK HANDLING
***************************************************************************/

/*-------------------------------------------------
    get_disk_handle - return a pointer to the
    CHD file associated with the given region
-------------------------------------------------*/

chd_file *rom_load_manager::get_disk_handle(std::string_view region)
{
	for (auto &curdisk : m_chd_list)
		if (curdisk->region() == region)
			return &curdisk->chd();
	return nullptr;
}


/*-------------------------------------------------
    set_disk_handle - set a pointer to the CHD
    file associated with the given region
-------------------------------------------------*/

std::error_condition rom_load_manager::set_disk_handle(std::string_view region, const char *fullpath)
{
	auto chd = std::make_unique<open_chd>(region);
	auto err = chd->orig_chd().open(fullpath);
	if (!err)
		m_chd_list.push_back(std::move(chd));
	return err;
}

/*-------------------------------------------------
    determine_bios_rom - determine system_bios
    from SystemBios structure and OPTION_BIOS
-------------------------------------------------*/

void rom_load_manager::determine_bios_rom(device_t &device, const char *specbios)
{
	// default is applied by the device at config complete time
	if (specbios && *specbios && core_stricmp(specbios, "default"))
	{
		bool found(false);
		for (const rom_entry &rom : device.rom_region_vector())
		{
			if (ROMENTRY_ISSYSTEM_BIOS(&rom))
			{
				char const *const biosname = ROM_GETNAME(&rom);
				int const bios_flags = ROM_GETBIOSFLAGS(&rom);
				char bios_number[20];

				// Allow '-bios n' to still be used
				sprintf(bios_number, "%d", bios_flags - 1);
				if (!core_stricmp(bios_number, specbios) || !core_stricmp(biosname, specbios))
				{
					found = true;
					device.set_system_bios(bios_flags);
					break;
				}
			}
		}

		// if we got neither an empty string nor 'default' then warn the user
		if (!found)
		{
			m_errorstring.append(util::string_format("%s: invalid BIOS \"%s\", reverting to default\n", device.tag(), specbios));
			m_warnings++;
		}
	}

	// log final result
	LOG("For \"%s\" using System BIOS: %d\n", device.tag(), device.system_bios());
}


/*-------------------------------------------------
    count_roms - counts the total number of ROMs
    that will need to be loaded
-------------------------------------------------*/

void rom_load_manager::count_roms()
{
	const rom_entry *region, *rom;

	/* start with 0 */
	m_romstotal = 0;
	m_romstotalsize = 0;

	/* loop over regions, then over files */
	for (device_t &device : device_enumerator(machine().config().root_device()))
		for (region = rom_first_region(device); region != nullptr; region = rom_next_region(region))
			for (rom = rom_first_file(region); rom != nullptr; rom = rom_next_file(rom))
				if (ROM_GETBIOSFLAGS(rom) == 0 || ROM_GETBIOSFLAGS(rom) == device.system_bios())
				{
					m_romstotal++;
					m_romstotalsize += rom_file_size(rom);
				}
}


/*-------------------------------------------------
    fill_random - fills an area of memory with
    random data
-------------------------------------------------*/

void rom_load_manager::fill_random(u8 *base, u32 length)
{
	while (length--)
		*base++ = machine().rand();
}


/*-------------------------------------------------
    handle_missing_file - handles error generation
    for missing files
-------------------------------------------------*/

void rom_load_manager::handle_missing_file(const rom_entry *romp, const std::vector<std::string> &tried_file_names, std::error_condition chderr)
{
	std::string tried;
	if (!tried_file_names.empty())
	{
		tried = " (tried in";
		for (const std::string &path : tried_file_names)
		{
			tried += ' ';
			tried += path;
		}
		tried += ')';
	}

	const bool is_chd(chderr);
	const std::string name(is_chd ? romp->name() + ".chd" : romp->name());

	const bool is_chd_error(is_chd && chderr != std::errc::no_such_file_or_directory);
	if (is_chd_error)
		m_errorstring.append(string_format("%s CHD ERROR: %s\n", name, chderr.message()));

	if (ROM_ISOPTIONAL(romp))
	{
		// optional files are okay
		if (!is_chd_error)
			m_errorstring.append(string_format("OPTIONAL %s NOT FOUND%s\n", name, tried));
		m_warnings++;
	}
	else if (util::hash_collection(romp->hashdata()).flag(util::hash_collection::FLAG_NO_DUMP))
	{
		// no good dumps are okay
		if (!is_chd_error)
			m_errorstring.append(string_format("%s NOT FOUND (NO GOOD DUMP KNOWN)%s\n", name, tried));
		m_knownbad++;
	}
	else
	{
		// anything else is bad
		if (!is_chd_error)
			m_errorstring.append(string_format("%s NOT FOUND%s\n", name, tried));
		m_errors++;
	}
}


/*-------------------------------------------------
    dump_wrong_and_correct_checksums - dump an
    error message containing the wrong and the
    correct checksums for a given ROM
-------------------------------------------------*/

void rom_load_manager::dump_wrong_and_correct_checksums(const util::hash_collection &hashes, const util::hash_collection &acthashes)
{
	m_errorstring.append(string_format("    EXPECTED: %s\n", hashes.macro_string()));
	m_errorstring.append(string_format("       FOUND: %s\n", acthashes.macro_string()));
}


/*-------------------------------------------------
    verify_length_and_hash - verify the length
    and hash signatures of a file
-------------------------------------------------*/

void rom_load_manager::verify_length_and_hash(emu_file *file, std::string_view name, u32 explength, const util::hash_collection &hashes)
{
	// we've already complained if there is no file
	if (!file)
		return;

	// verify length
	u64 const actlength = file->size();
	if (explength != actlength)
	{
		m_errorstring.append(string_format("%s WRONG LENGTH (expected: %08x found: %08x)\n", name, explength, actlength));
		m_warnings++;
	}

	if (hashes.flag(util::hash_collection::FLAG_NO_DUMP))
	{
		// If there is no good dump known, write it
		m_errorstring.append(string_format("%s NO GOOD DUMP KNOWN\n", name));
		m_knownbad++;
	}
	else
	{
		// verify checksums
		util::hash_collection const &acthashes = file->hashes(hashes.hash_types());
		if (hashes != acthashes)
		{
			// otherwise, it's just bad
			util::hash_collection const &all_acthashes = (acthashes.hash_types() == util::hash_collection::HASH_TYPES_ALL)
					? acthashes
					: file->hashes(util::hash_collection::HASH_TYPES_ALL);
			m_errorstring.append(string_format("%s WRONG CHECKSUMS:\n", name));
			dump_wrong_and_correct_checksums(hashes, all_acthashes);
			m_warnings++;
		}
		else if (hashes.flag(util::hash_collection::FLAG_BAD_DUMP))
		{
			// If it matches, but it is actually a bad dump, write it
			m_errorstring.append(string_format("%s ROM NEEDS REDUMP\n", name));
			m_knownbad++;
		}
	}
}


/*-------------------------------------------------
    display_loading_rom_message - display
    messages about ROM loading to the user
-------------------------------------------------*/

void rom_load_manager::display_loading_rom_message(const char *name, bool from_list)
{
	std::string buffer;
	if (name)
		buffer = util::string_format("%s (%d%%)", from_list ? "Loading Software" : "Loading Machine", u32(100 * m_romsloadedsize / m_romstotalsize));
	else
		buffer = "Loading Complete";

	if (!machine().ui().is_menu_active())
		machine().ui().set_startup_text(buffer.c_str(), false);
}


/*-------------------------------------------------
    display_rom_load_results - display the final
    results of ROM loading
-------------------------------------------------*/

void rom_load_manager::display_rom_load_results(bool from_list)
{
	/* final status display */
	display_loading_rom_message(nullptr, from_list);

	/* if we had errors, they are fatal */
	if (m_errors != 0)
	{
		/* create the error message and exit fatally */
		osd_printf_error("%s", m_errorstring);
		throw emu_fatalerror(EMU_ERR_MISSING_FILES, "Required files are missing, the machine cannot be run.");
	}

	/* if we had warnings, output them, but continue */
	if ((m_warnings) || (m_knownbad))
	{
		m_errorstring.append("WARNING: the machine might not run correctly.");
		osd_printf_warning("%s\n", m_errorstring);
	}
}


/*-------------------------------------------------
    region_post_process - post-process a region,
    byte swapping and inverting data as necessary
-------------------------------------------------*/

void rom_load_manager::region_post_process(memory_region *region, bool invert)
{
	// do nothing if no region
	if (region == nullptr)
		return;

	LOG("+ datawidth=%dbit endian=%s\n", region->bitwidth(),
			region->endianness() == ENDIANNESS_LITTLE ? "little" : "big");

	/* if the region is inverted, do that now */
	if (invert)
	{
		LOG("+ Inverting region\n");
		u8 *base = region->base();
		for (int i = 0; i < region->bytes(); i++)
			*base++ ^= 0xff;
	}

	/* swap the endianness if we need to */
	if (region->bytewidth() > 1 && region->endianness() != ENDIANNESS_NATIVE)
	{
		LOG("+ Byte swapping region\n");
		int datawidth = region->bytewidth();
		u8 *base = region->base();
		for (int i = 0; i < region->bytes(); i += datawidth)
		{
			u8 temp[8];
			memcpy(temp, base, datawidth);
			for (int j = datawidth - 1; j >= 0; j--)
				*base++ = temp[j];
		}
	}
}


/*-------------------------------------------------
    open_rom_file - open a ROM file, searching
    up the parent and loading by checksum
-------------------------------------------------*/

std::unique_ptr<emu_file> rom_load_manager::open_rom_file(std::initializer_list<std::reference_wrapper<const std::vector<std::string> > > searchpath, const rom_entry *romp, std::vector<std::string> &tried_file_names, bool from_list)
{
	std::error_condition filerr = std::errc::no_such_file_or_directory;
	u32 const romsize = rom_file_size(romp);
	tried_file_names.clear();

	// update status display
	display_loading_rom_message(ROM_GETNAME(romp), from_list);

	// extract CRC to use for searching
	u32 crc = 0;
	bool const has_crc = util::hash_collection(romp->hashdata()).crc(crc);

	// attempt reading up the chain through the parents
	// it also automatically attempts any kind of load by checksum supported by the archives.
	std::unique_ptr<emu_file> result;
	for (const std::vector<std::string> &paths : searchpath)
	{
		result = open_rom_file(paths, tried_file_names, has_crc, crc, ROM_GETNAME(romp), filerr);
		if (result)
			break;
	}

	// update counters
	m_romsloaded++;
	m_romsloadedsize += romsize;

	// return the result
	if (filerr)
		return nullptr;
	else
		return result;
}


std::unique_ptr<emu_file> rom_load_manager::open_rom_file(const std::vector<std::string> &paths, std::vector<std::string> &tried, bool has_crc, u32 crc, std::string_view name, std::error_condition &filerr)
{
	// record the set names we search
	tried.insert(tried.end(), paths.begin(), paths.end());

	// attempt to open the file
	std::unique_ptr<emu_file> result(new emu_file(machine().options().media_path(), paths, OPEN_FLAG_READ));
	result->set_restrict_to_mediapath(1);
	if (has_crc)
		filerr = result->open(name, crc);
	else
		filerr = result->open(name);

	// don't return anything if unsuccessful
	if (filerr)
		return nullptr;
	else
		return result;
}


/*-------------------------------------------------
    rom_fread - cheesy fread that fills with
    random data for a nullptr file
-------------------------------------------------*/

int rom_load_manager::rom_fread(emu_file *file, u8 *buffer, int length, const rom_entry *parent_region)
{
	if (file) // files just pass through
		return file->read(buffer, length);

	if (!ROMREGION_ISERASE(parent_region)) // otherwise, fill with randomness unless it was already specifically erased
		fill_random(buffer, length);

	return length;
}


/*-------------------------------------------------
    read_rom_data - read ROM data for a single
    entry
-------------------------------------------------*/

int rom_load_manager::read_rom_data(emu_file *file, const rom_entry *parent_region, const rom_entry *romp)
{
	int datashift = ROM_GETBITSHIFT(romp);
	int datamask = ((1 << ROM_GETBITWIDTH(romp)) - 1) << datashift;
	int numbytes = ROM_GETLENGTH(romp);
	int groupsize = ROM_GETGROUPSIZE(romp);
	int skip = ROM_GETSKIPCOUNT(romp);
	int reversed = ROM_ISREVERSED(romp);
	int numgroups = (numbytes + groupsize - 1) / groupsize;
	u8 *base = m_region->base() + ROM_GETOFFSET(romp);
	u32 tempbufsize;
	int i;

	LOG("Loading ROM data: offs=%X len=%X mask=%02X group=%d skip=%d reverse=%d\n", ROM_GETOFFSET(romp), numbytes, datamask, groupsize, skip, reversed);

	/* make sure the length was an even multiple of the group size */
	if (numbytes % groupsize != 0)
		osd_printf_warning("Warning in RomModule definition: %s length not an even multiple of group size\n", romp->name());

	/* make sure we only fill within the region space */
	if (ROM_GETOFFSET(romp) + numgroups * groupsize + (numgroups - 1) * skip > m_region->bytes())
		throw emu_fatalerror("Error in RomModule definition: %s out of memory region space\n", romp->name());

	/* make sure the length was valid */
	if (numbytes == 0)
		throw emu_fatalerror("Error in RomModule definition: %s has an invalid length\n", romp->name());

	/* special case for simple loads */
	if (datamask == 0xff && (groupsize == 1 || !reversed) && skip == 0)
		return rom_fread(file, base, numbytes, parent_region);

	/* use a temporary buffer for complex loads */
	tempbufsize = std::min(TEMPBUFFER_MAX_SIZE, numbytes);
	std::vector<u8> tempbuf(tempbufsize);

	/* chunky reads for complex loads */
	skip += groupsize;
	while (numbytes > 0)
	{
		int evengroupcount = (tempbufsize / groupsize) * groupsize;
		int bytesleft = (numbytes > evengroupcount) ? evengroupcount : numbytes;
		u8 *bufptr = &tempbuf[0];

		/* read as much as we can */
		LOG("  Reading %X bytes into buffer\n", bytesleft);
		if (rom_fread(file, bufptr, bytesleft, parent_region) != bytesleft)
			return 0;
		numbytes -= bytesleft;

		LOG("  Copying to %p\n", base);

		/* unmasked cases */
		if (datamask == 0xff)
		{
			/* non-grouped data */
			if (groupsize == 1)
				for (i = 0; i < bytesleft; i++, base += skip)
					*base = *bufptr++;

			/* grouped data -- non-reversed case */
			else if (!reversed)
				while (bytesleft)
				{
					for (i = 0; i < groupsize && bytesleft; i++, bytesleft--)
						base[i] = *bufptr++;
					base += skip;
				}

			/* grouped data -- reversed case */
			else
				while (bytesleft)
				{
					for (i = groupsize - 1; i >= 0 && bytesleft; i--, bytesleft--)
						base[i] = *bufptr++;
					base += skip;
				}
		}

		/* masked cases */
		else
		{
			/* non-grouped data */
			if (groupsize == 1)
				for (i = 0; i < bytesleft; i++, base += skip)
					*base = (*base & ~datamask) | ((*bufptr++ << datashift) & datamask);

			/* grouped data -- non-reversed case */
			else if (!reversed)
				while (bytesleft)
				{
					for (i = 0; i < groupsize && bytesleft; i++, bytesleft--)
						base[i] = (base[i] & ~datamask) | ((*bufptr++ << datashift) & datamask);
					base += skip;
				}

			/* grouped data -- reversed case */
			else
				while (bytesleft)
				{
					for (i = groupsize - 1; i >= 0 && bytesleft; i--, bytesleft--)
						base[i] = (base[i] & ~datamask) | ((*bufptr++ << datashift) & datamask);
					base += skip;
				}
		}
	}

	LOG("  All done\n");
	return ROM_GETLENGTH(romp);
}


/*-------------------------------------------------
    fill_rom_data - fill a region of ROM space
-------------------------------------------------*/

void rom_load_manager::fill_rom_data(const rom_entry *romp)
{
	u32 numbytes = ROM_GETLENGTH(romp);
	int skip = ROM_GETSKIPCOUNT(romp);
	u8 *base = m_region->base() + ROM_GETOFFSET(romp);

	// make sure we fill within the region space
	if (ROM_GETOFFSET(romp) + numbytes > m_region->bytes())
		throw emu_fatalerror("Error in RomModule definition: FILL out of memory region space\n");

	// make sure the length was valid
	if (numbytes == 0)
		throw emu_fatalerror("Error in RomModule definition: FILL has an invalid length\n");

	// for fill bytes, the byte that gets filled is the first byte of the hashdata string
	u8 fill_byte = u8(strtol(romp->hashdata().c_str(), nullptr, 0));

	// fill the data (filling value is stored in place of the hashdata)
	if(skip != 0)
	{
		for (int i = 0; i < numbytes; i+= skip + 1)
			base[i] = fill_byte;
	}
	else
		memset(base, fill_byte, numbytes);
}


/*-------------------------------------------------
    copy_rom_data - copy a region of ROM space
-------------------------------------------------*/

void rom_load_manager::copy_rom_data(const rom_entry *romp)
{
	u8 *base = m_region->base() + ROM_GETOFFSET(romp);
	const std::string &srcrgntag = romp->name();
	u32 numbytes = ROM_GETLENGTH(romp);
	u32 srcoffs = u32(strtol(romp->hashdata().c_str(), nullptr, 0));  /* srcoffset in place of hashdata */

	/* make sure we copy within the region space */
	if (ROM_GETOFFSET(romp) + numbytes > m_region->bytes())
		throw emu_fatalerror("Error in RomModule definition: COPY out of target memory region space\n");

	/* make sure the length was valid */
	if (numbytes == 0)
		throw emu_fatalerror("Error in RomModule definition: COPY has an invalid length\n");

	/* make sure the source was valid */
	memory_region *region = machine().root_device().memregion(srcrgntag);
	if (region == nullptr)
		throw emu_fatalerror("Error in RomModule definition: COPY from an invalid region\n");

	/* make sure we find within the region space */
	if (srcoffs + numbytes > region->bytes())
		throw emu_fatalerror("Error in RomModule definition: COPY out of source memory region space\n");

	/* fill the data */
	memcpy(base, region->base() + srcoffs, numbytes);
}


/*-------------------------------------------------
    process_rom_entries - process all ROM entries
    for a region
-------------------------------------------------*/

void rom_load_manager::process_rom_entries(std::initializer_list<std::reference_wrapper<const std::vector<std::string> > > searchpath, u8 bios, const rom_entry *parent_region, const rom_entry *romp, bool from_list)
{
	u32 lastflags = 0;
	std::vector<std::string> tried_file_names;

	// loop until we hit the end of this region
	while (!ROMENTRY_ISREGIONEND(romp))
	{
		tried_file_names.clear();

		if (ROMENTRY_ISCONTINUE(romp))
			throw emu_fatalerror("Error in RomModule definition: ROM_CONTINUE not preceded by ROM_LOAD\n");

		if (ROMENTRY_ISIGNORE(romp))
			throw emu_fatalerror("Error in RomModule definition: ROM_IGNORE not preceded by ROM_LOAD\n");

		if (ROMENTRY_ISRELOAD(romp))
			throw emu_fatalerror("Error in RomModule definition: ROM_RELOAD not preceded by ROM_LOAD\n");

		if (ROMENTRY_ISFILL(romp))
		{
			if (!ROM_GETBIOSFLAGS(romp) || (ROM_GETBIOSFLAGS(romp) == bios))
				fill_rom_data(romp);

			romp++;
		}
		else if (ROMENTRY_ISCOPY(romp))
		{
			copy_rom_data(romp++);
		}
		else if (ROMENTRY_ISFILE(romp))
		{
			// handle files
			bool const irrelevantbios = (ROM_GETBIOSFLAGS(romp) != 0) && (ROM_GETBIOSFLAGS(romp) != bios);
			rom_entry const *baserom = romp;
			int explength = 0;

			// open the file if it is a non-BIOS or matches the current BIOS
			LOG("Opening ROM file: %s\n", ROM_GETNAME(romp));
			std::unique_ptr<emu_file> file;
			if (!irrelevantbios)
			{
				file = open_rom_file(searchpath, romp, tried_file_names, from_list);
				if (!file)
					handle_missing_file(romp, tried_file_names, std::error_condition());
			}

			// loop until we run out of reloads
			do
			{
				// loop until we run out of continues/ignores
				do
				{
					rom_entry modified_romp = *romp++;
					//int readresult;

					// handle flag inheritance
					if (!ROM_INHERITSFLAGS(&modified_romp))
						lastflags = modified_romp.get_flags();
					else
						modified_romp.set_flags((modified_romp.get_flags() & ~ROM_INHERITEDFLAGS) | lastflags);

					explength += ROM_GETLENGTH(&modified_romp);

					// attempt to read using the modified entry
					if (!ROMENTRY_ISIGNORE(&modified_romp) && !irrelevantbios)
						/*readresult = */read_rom_data(file.get(), parent_region, &modified_romp);
				}
				while (ROMENTRY_ISCONTINUE(romp) || ROMENTRY_ISIGNORE(romp));

				// if this was the first use of this file, verify the length and CRC
				if (baserom)
				{
					LOG("Verifying length (%X) and checksums\n", explength);
					verify_length_and_hash(file.get(), baserom->name(), explength, util::hash_collection(baserom->hashdata()));
					LOG("Verify finished\n");
				}

				// re-seek to the start and clear the baserom so we don't reverify
				if (file)
					file->seek(0, SEEK_SET);
				baserom = nullptr;
				explength = 0;
			}
			while (ROMENTRY_ISRELOAD(romp));

			// close the file
			if (file)
			{
				LOG("Closing ROM file\n");
				file.reset();
			}
		}
		else
		{
			romp++; // something else - skip
		}
	}
}


/*-------------------------------------------------
    open_disk_diff - open a DISK diff file
-------------------------------------------------*/

std::error_condition rom_load_manager::open_disk_diff(emu_options &options, const rom_entry *romp, chd_file &source, chd_file &diff_chd)
{
	// TODO: use system name and/or software list name in the path - the current setup doesn't scale
	std::string fname = romp->name() + ".dif";

	// try to open the diff
	LOG("Opening differencing image file: %s\n", fname.c_str());
	emu_file diff_file(options.diff_directory(), OPEN_FLAG_READ | OPEN_FLAG_WRITE);
	std::error_condition filerr = diff_file.open(fname);
	if (!filerr)
	{
		std::string fullpath(diff_file.fullpath());
		diff_file.close();

		LOG("Opening differencing image file: %s\n", fullpath.c_str());
		return diff_chd.open(fullpath, true, &source);
	}

	// didn't work; try creating it instead
	LOG("Creating differencing image: %s\n", fname.c_str());
	diff_file.set_openflags(OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	filerr = diff_file.open(fname);
	if (!filerr)
	{
		std::string fullpath(diff_file.fullpath());
		diff_file.close();

		// create the CHD
		LOG("Creating differencing image file: %s\n", fullpath.c_str());
		chd_codec_type compression[4] = { CHD_CODEC_NONE };
		std::error_condition err = diff_chd.create(fullpath, source.logical_bytes(), source.hunk_bytes(), compression, source);
		if (err)
			return err;

		return diff_chd.clone_all_metadata(source);
	}

	return std::errc::no_such_file_or_directory;
}


/*-------------------------------------------------
    process_disk_entries - process all disk entries
    for a region
-------------------------------------------------*/

void rom_load_manager::process_disk_entries(std::initializer_list<std::reference_wrapper<const std::vector<std::string> > > searchpath, std::string_view regiontag, const rom_entry *romp, std::function<const rom_entry * ()> next_parent)
{
	/* remove existing disk entries for this region */
	m_chd_list.erase(std::remove_if(m_chd_list.begin(), m_chd_list.end(),
			[regiontag] (std::unique_ptr<open_chd> &chd) { return chd->region() == regiontag; }), m_chd_list.end());

	/* loop until we hit the end of this region */
	for ( ; !ROMENTRY_ISREGIONEND(romp); romp++)
	{
		/* handle files */
		if (ROMENTRY_ISFILE(romp))
		{
			auto chd = std::make_unique<open_chd>(regiontag);
			std::error_condition err;

			/* make the filename of the source */
			const std::string filename = romp->name() + ".chd";

			/* first open the source drive */
			// FIXME: we've lost the ability to search parents here
			LOG("Opening disk image: %s\n", filename.c_str());
			err = do_open_disk(machine().options(), searchpath, romp, chd->orig_chd(), next_parent);
			if (err)
			{
				std::vector<std::string> tried;
				for (auto const &paths : searchpath)
				{
					for (std::string const &path : paths.get())
						tried.emplace_back(path);
				}
				handle_missing_file(romp, tried, err);
				chd = nullptr;
				continue;
			}

			/* get the header and extract the SHA1 */
			util::hash_collection acthashes;
			acthashes.add_sha1(chd->orig_chd().sha1());

			/* verify the hash */
			const util::hash_collection hashes(romp->hashdata());
			if (hashes != acthashes)
			{
				m_errorstring.append(string_format("%s WRONG CHECKSUMS:\n", filename));
				dump_wrong_and_correct_checksums(hashes, acthashes);
				m_warnings++;
			}
			else if (hashes.flag(util::hash_collection::FLAG_BAD_DUMP))
			{
				m_errorstring.append(string_format("%s CHD NEEDS REDUMP\n", filename));
				m_knownbad++;
			}

			/* if not read-only, make the diff file */
			if (!DISK_ISREADONLY(romp))
			{
				/* try to open or create the diff */
				err = open_disk_diff(machine().options(), romp, chd->orig_chd(), chd->diff_chd());
				if (err)
				{
					m_errorstring.append(string_format("%s DIFF CHD ERROR: %s\n", filename, err.message()));
					m_errors++;
					chd = nullptr;
					continue;
				}
			}

			/* we're okay, add to the list of disks */
			LOG("Assigning to handle %d\n", DISK_GETINDEX(romp));
			m_chd_list.push_back(std::move(chd));
		}
	}
}


/*-------------------------------------------------
    get_software_searchpath - get search path
    for a software list item
-------------------------------------------------*/

std::vector<std::string> rom_load_manager::get_software_searchpath(software_list_device &swlist, const software_info &swinfo)
{
	std::vector<software_info const *> parents;
	return make_software_searchpath(swlist, swinfo, parents);
}


/*-------------------------------------------------
    open_disk_image - open a disk image for a
    device
-------------------------------------------------*/

std::error_condition rom_load_manager::open_disk_image(const emu_options &options, const device_t &device, const rom_entry *romp, chd_file &image_chd)
{
	const std::vector<std::string> searchpath(device.searchpath());

	driver_device const *const driver(dynamic_cast<driver_device const *>(&device));
	std::function<const rom_entry * ()> next_parent;
	if (driver)
		next_parent = next_parent_system(driver->system());
	else
		next_parent = [] () { return nullptr; };
	return do_open_disk(options, { searchpath }, romp, image_chd, std::move(next_parent));
}


/*-------------------------------------------------
    open_disk_image - open a disk image for a
    software item
-------------------------------------------------*/

std::error_condition rom_load_manager::open_disk_image(const emu_options &options, software_list_device &swlist, const software_info &swinfo, const rom_entry *romp, chd_file &image_chd)
{
	std::vector<software_info const *> parents;
	std::vector<std::string> searchpath = make_software_searchpath(swlist, swinfo, parents);
	searchpath.emplace_back(swlist.list_name()); // look for loose disk images in software list directory
	return do_open_disk(options, { searchpath }, romp, image_chd, next_parent_software(parents));
}


/*-------------------------------------------------
    normalize_flags_for_device - modify the region
    flags for the given device
-------------------------------------------------*/

void rom_load_manager::normalize_flags_for_device(std::string_view rgntag, u8 &width, endianness_t &endian)
{
	device_t *device = machine().root_device().subdevice(rgntag);
	device_memory_interface *memory;
	if (device != nullptr && device->interface(memory))
	{
		const address_space_config *spaceconfig = memory->space_config();
		if (spaceconfig != nullptr)
		{
			int buswidth;

			/* set the endianness */
			if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
				endian = ENDIANNESS_LITTLE;
			else
				endian = ENDIANNESS_BIG;

			/* set the width */
			buswidth = spaceconfig->data_width();
			if (buswidth <= 8)
				width = 1;
			else if (buswidth <= 16)
				width = 2;
			else if (buswidth <= 32)
				width = 4;
			else
				width = 8;
		}
	}
}


/*-------------------------------------------------
    load_software_part_region - load a software part

    This is used by MAME when loading a piece of
    software. The code should be merged with
    process_region_list or updated to use a slight
    more general process_region_list.
-------------------------------------------------*/

void rom_load_manager::load_software_part_region(device_t &device, software_list_device &swlist, std::string_view swname, const rom_entry *start_region)
{
	m_errorstring.clear();
	m_softwarningstring.clear();

	m_romstotal = 0;
	m_romstotalsize = 0;
	m_romsloadedsize = 0;

	std::vector<const software_info *> parents;
	std::vector<std::string> swsearch, disksearch, devsearch;
	const software_info *const swinfo = swlist.find(std::string(swname));
	if (swinfo)
	{
		// display a warning for unsupported software
		// TODO: list supported clones like we do for machines?
		if (swinfo->supported() == software_support::PARTIALLY_SUPPORTED)
		{
			m_errorstring.append(string_format("WARNING: support for software %s (in list %s) is only partial\n", swname, swlist.list_name()));
			m_softwarningstring.append(string_format("Support for software %s (in list %s) is only partial\n", swname, swlist.list_name()));
		}
		else if (swinfo->supported() == software_support::UNSUPPORTED)
		{
			m_errorstring.append(string_format("WARNING: support for software %s (in list %s) is only preliminary\n", swname, swlist.list_name()));
			m_softwarningstring.append(string_format("Support for software %s (in list %s) is only preliminary\n", swname, swlist.list_name()));
		}

		// walk the chain of parents and add them to the search path
		swsearch = make_software_searchpath(swlist, *swinfo, parents);
	}
	else
	{
		swsearch.emplace_back(util::string_format("%s" PATH_SEPARATOR "%s", swlist.list_name(), swname));
		swsearch.emplace_back(swname);
	}

	// this is convenient for CD-only lists so you don't need an extra level of directories containing one file each
	disksearch.emplace_back(swlist.list_name());

	// for historical reasons, add the search path for the software list device's owner
	const device_t *const listowner = swlist.owner();
	if (listowner)
		devsearch = listowner->searchpath();

	// loop until we hit the end
	std::function<const rom_entry * ()> next_parent;
	for (const rom_entry *region = start_region; region != nullptr; region = rom_next_region(region))
	{
		u32 regionlength = ROMREGION_GETLENGTH(region);

		std::string regiontag = device.subtag(region->name());
		LOG("Processing region \"%s\" (length=%X)\n", regiontag.c_str(), regionlength);

		// the first entry must be a region
		assert(ROMENTRY_ISREGION(region));

		// if this is a device region, override with the device width and endianness
		endianness_t endianness = ROMREGION_ISBIGENDIAN(region) ? ENDIANNESS_BIG : ENDIANNESS_LITTLE;
		u8 width = ROMREGION_GETWIDTH(region) / 8;
		memory_region *memregion = machine().root_device().memregion(regiontag);
		if (memregion != nullptr)
		{
			normalize_flags_for_device(regiontag, width, endianness);

			// clear old region (TODO: should be moved to an image unload function)
			machine().memory().region_free(memregion->name());
		}

		// remember the base and length
		m_region = machine().memory().region_alloc(regiontag, regionlength, width, endianness);
		LOG("Allocated %X bytes @ %p\n", m_region->bytes(), m_region->base());

		if (ROMREGION_ISERASE(region)) // clear the region if it's requested
			memset(m_region->base(), ROMREGION_GETERASEVAL(region), m_region->bytes());
		else if (m_region->bytes() <= 0x400000) // or if it's sufficiently small (<= 4MB)
			memset(m_region->base(), 0, m_region->bytes());
#ifdef MAME_DEBUG
		else // if we're debugging, fill region with random data to catch errors
			fill_random(m_region->base(), m_region->bytes());
#endif

		// update total number of roms
		for (const rom_entry *rom = rom_first_file(region); rom != nullptr; rom = rom_next_file(rom))
		{
			m_romstotal++;
			m_romstotalsize += rom_file_size(rom);
		}

		// now process the entries in the region
		if (ROMREGION_ISROMDATA(region))
		{
			if (devsearch.empty())
				process_rom_entries({ swsearch }, 0U, region, region + 1, true);
			else
				process_rom_entries({ swsearch, devsearch }, 0U, region, region + 1, true);
		}
		else if (ROMREGION_ISDISKDATA(region))
		{
			if (!next_parent)
			{
				if (!parents.empty())
					next_parent = next_parent_software(parents);
				else
					next_parent = [] () { return nullptr; };
			}
			if (devsearch.empty())
				process_disk_entries({ swsearch, disksearch }, regiontag, region + 1, next_parent);
			else
				process_disk_entries({ swsearch, disksearch, devsearch }, regiontag, region + 1, next_parent);
		}
	}

	// now go back and post-process all the regions
	for (const rom_entry *region = start_region; region != nullptr; region = rom_next_region(region))
		region_post_process(device.memregion(region->name()), ROMREGION_ISINVERTED(region));

	// display the results and exit
	display_rom_load_results(true);
}


/*-------------------------------------------------
    process_region_list - process a region list
-------------------------------------------------*/

void rom_load_manager::process_region_list()
{
	// loop until we hit the end
	device_enumerator deviter(machine().root_device());
	std::vector<std::string> searchpath;
	for (device_t &device : deviter)
	{
		searchpath.clear();
		std::function<const rom_entry * ()> next_parent;
		for (const rom_entry *region = rom_first_region(device); region != nullptr; region = rom_next_region(region))
		{
			u32 regionlength = ROMREGION_GETLENGTH(region);

			std::string regiontag = device.subtag(region->name());
			LOG("Processing region \"%s\" (length=%X)\n", regiontag.c_str(), regionlength);

			// the first entry must be a region
			assert(ROMENTRY_ISREGION(region));

			if (ROMREGION_ISROMDATA(region))
			{
				// if this is a device region, override with the device width and endianness
				u8 width = ROMREGION_GETWIDTH(region) / 8;
				endianness_t endianness = ROMREGION_ISBIGENDIAN(region) ? ENDIANNESS_BIG : ENDIANNESS_LITTLE;
				normalize_flags_for_device(regiontag, width, endianness);

				// remember the base and length
				m_region = machine().memory().region_alloc(regiontag, regionlength, width, endianness);
				LOG("Allocated %X bytes @ %p\n", m_region->bytes(), m_region->base());

				if (ROMREGION_ISERASE(region)) // clear the region if it's requested
					memset(m_region->base(), ROMREGION_GETERASEVAL(region), m_region->bytes());
				else if (m_region->bytes() <= 0x400000) // or if it's sufficiently small (<= 4MB)
					memset(m_region->base(), 0, m_region->bytes());
#ifdef MAME_DEBUG
				else // if we're debugging, fill region with random data to catch errors
					fill_random(m_region->base(), m_region->bytes());
#endif

				// now process the entries in the region
				if (searchpath.empty())
					searchpath = device.searchpath();
				assert(!searchpath.empty());
				process_rom_entries({ searchpath }, device.system_bios(), region, region + 1, false);
			}
			else if (ROMREGION_ISDISKDATA(region))
			{
				if (searchpath.empty())
					searchpath = device.searchpath();
				assert(!searchpath.empty());
				if (!next_parent)
				{
					driver_device const *const driver(dynamic_cast<driver_device const *>(&device));
					if (driver)
						next_parent = next_parent_system(driver->system());
					else
						next_parent = [] () { return nullptr; };
				}
				process_disk_entries({ searchpath }, regiontag, region + 1, next_parent);
			}
		}
	}

	// now go back and post-process all the regions
	for (device_t &device : deviter)
		for (const rom_entry *region = rom_first_region(device); region != nullptr; region = rom_next_region(region))
			region_post_process(device.memregion(region->name()), ROMREGION_ISINVERTED(region));

	// and finally register all per-game parameters
	for (device_t &device : deviter)
	{
		for (const rom_entry *param = rom_first_parameter(device); param != nullptr; param = rom_next_parameter(param))
		{
			std::string regiontag = device.subtag(param->name());
			machine().parameters().add(regiontag, param->hashdata());
		}
	}
}


/*-------------------------------------------------
    rom_init - load the ROMs and open the disk
    images associated with the given machine
-------------------------------------------------*/

rom_load_manager::rom_load_manager(running_machine &machine)
	: m_machine(machine)
	, m_warnings(0)
	, m_knownbad(0)
	, m_errors(0)
	, m_romsloaded(0)
	, m_romstotal(0)
	, m_romsloadedsize(0)
	, m_romstotalsize(0)
	, m_chd_list()
	, m_region(nullptr)
	, m_errorstring()
	, m_softwarningstring()
{
	// figure out which BIOS we are using
	std::map<std::string_view, std::string> card_bios;
	for (device_t &device : device_enumerator(machine.config().root_device()))
	{
		device_slot_interface const *const slot(dynamic_cast<device_slot_interface *>(&device));
		if (slot)
		{
			device_t const *const card(slot->get_card_device());
			slot_option const &slot_opt(machine.options().slot_option(slot->slot_name()));
			if (card && !slot_opt.bios().empty())
				card_bios.emplace(card->tag(), slot_opt.bios());
		}

		if (device.rom_region())
		{
			std::string specbios;
			if (!device.owner())
			{
				specbios = machine.options().bios();
			}
			else
			{
				auto const found(card_bios.find(device.tag()));
				if (card_bios.end() != found)
				{
					specbios = std::move(found->second);
					card_bios.erase(found);
				}
			}
			determine_bios_rom(device, specbios.c_str());
		}
	}

	// count the total number of ROMs
	count_roms();

	// reset the disk list
	m_chd_list.clear();

	// process the ROM entries we were passed
	process_region_list();

	// display the results and exit
	display_rom_load_results(false);
}


// -------------------------------------------------
// rom_build_entries - builds a rom_entry vector
// from a tiny_rom_entry array
// -------------------------------------------------

std::vector<rom_entry> rom_build_entries(const tiny_rom_entry *tinyentries)
{
	std::vector<rom_entry> result;
	if (tinyentries)
	{
		int i = 0;
		do
		{
			result.emplace_back(tinyentries[i]);
		}
		while (!ROMENTRY_ISEND(tinyentries[i++]));
	}
	else
	{
		tiny_rom_entry const end_entry = { nullptr, nullptr, 0, 0, ROMENTRYTYPE_END };
		result.emplace_back(end_entry);
	}
	return result;
}
