// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Paul Priest,Aaron Giles
/*********************************************************************

    romload.c

    ROM loading functions.
*********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "drivenum.h"
#include "softlist.h"
#include "ui/ui.h"


#define LOG_LOAD 0
#define LOG(x) do { if (LOG_LOAD) debugload x; } while(0)


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define TEMPBUFFER_MAX_SIZE     (1024 * 1024 * 1024)

/***************************************************************************
    HELPERS (also used by diimage.cpp)
 ***************************************************************************/

static file_error common_process_file(emu_options &options, const char *location, const char *ext, const rom_entry *romp, emu_file &image_file)
{
	file_error filerr;

	if (location != nullptr && strcmp(location, "") != 0)
		filerr = image_file.open(location, PATH_SEPARATOR, ROM_GETNAME(romp), ext);
	else
		filerr = image_file.open(ROM_GETNAME(romp), ext);

	return filerr;
}

std::unique_ptr<emu_file> common_process_file(emu_options &options, const char *location, bool has_crc, UINT32 crc, const rom_entry *romp, file_error &filerr)
{
	auto image_file = std::make_unique<emu_file>(options.media_path(), OPEN_FLAG_READ);

	if (has_crc)
		filerr = image_file->open(location, PATH_SEPARATOR, ROM_GETNAME(romp), crc);
	else
		filerr = image_file->open(location, PATH_SEPARATOR, ROM_GETNAME(romp));

	if (filerr != FILERR_NONE)
	{
		image_file = nullptr;
	}
	return image_file;
}

/***************************************************************************
    ROM LOADING
***************************************************************************/

/*-------------------------------------------------
    rom_first_region - return pointer to first ROM
    region
-------------------------------------------------*/

const rom_entry *rom_first_region(const device_t &device)
{
	const rom_entry *romp = device.rom_region();
	while (romp && ROMENTRY_ISPARAMETER(romp))
		romp++;
	return (romp != nullptr && !ROMENTRY_ISEND(romp)) ? romp : nullptr;
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
	const rom_entry *romp = device.rom_region();
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
    rom_region_name - return the appropriate name
    for a rom region
-------------------------------------------------*/

std::string rom_region_name(const device_t &device, const rom_entry *romp)
{
	return device.subtag(ROM_GETNAME(romp));
}


/*-------------------------------------------------
    rom_parameter_name - return the appropriate name
    for a per-game parameter
-------------------------------------------------*/

std::string rom_parameter_name(const device_t &device, const rom_entry *romp)
{
	return device.subtag(romp->_name);
}


/*-------------------------------------------------
    rom_parameter_name - return the value for a
    per-game parameter
-------------------------------------------------*/

std::string rom_parameter_value(const rom_entry *romp)
{
	return std::string(romp->_hashdata);
}


/*-------------------------------------------------
    rom_file_size - return the expected size of a
    file given the ROM description
-------------------------------------------------*/

UINT32 rom_file_size(const rom_entry *romp)
{
	UINT32 maxlength = 0;

	/* loop until we run out of reloads */
	do
	{
		UINT32 curlength;

		/* loop until we run out of continues/ignores */
		curlength = ROM_GETLENGTH(romp++);
		while (ROMENTRY_ISCONTINUE(romp) || ROMENTRY_ISIGNORE(romp))
			curlength += ROM_GETLENGTH(romp++);

		/* track the maximum length */
		maxlength = MAX(maxlength, curlength);
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

chd_file *rom_load_manager::get_disk_handle(const char *region)
{
	for (auto &curdisk : m_chd_list)
		if (strcmp(curdisk->region(), region) == 0)
			return &curdisk->chd();
	return nullptr;
}


/*-------------------------------------------------
    set_disk_handle - set a pointer to the CHD
    file associated with the given region
-------------------------------------------------*/

int rom_load_manager::set_disk_handle(const char *region, const char *fullpath)
{
	auto chd = std::make_unique<open_chd>(region);
	auto err = chd->orig_chd().open(fullpath);
	if (err == CHDERR_NONE)
		m_chd_list.push_back(std::move(chd));
	return err;
}

/*-------------------------------------------------
    determine_bios_rom - determine system_bios
    from SystemBios structure and OPTION_BIOS
-------------------------------------------------*/

void rom_load_manager::determine_bios_rom(device_t *device, const char *specbios)
{
	const char *defaultname = nullptr;
	const rom_entry *rom;
	int default_no = 1;
	int bios_count = 0;

	device->set_system_bios(0);

	/* first determine the default BIOS name */
	for (rom = device->rom_region(); !ROMENTRY_ISEND(rom); rom++)
		if (ROMENTRY_ISDEFAULT_BIOS(rom))
			defaultname = ROM_GETNAME(rom);

	/* look for a BIOS with a matching name */
	for (rom = device->rom_region(); !ROMENTRY_ISEND(rom); rom++)
		if (ROMENTRY_ISSYSTEM_BIOS(rom))
		{
			const char *biosname = ROM_GETNAME(rom);
			int bios_flags = ROM_GETBIOSFLAGS(rom);
			char bios_number[20];

			/* Allow '-bios n' to still be used */
			sprintf(bios_number, "%d", bios_flags - 1);
			if (core_stricmp(bios_number, specbios) == 0 || core_stricmp(biosname, specbios) == 0)
				device->set_system_bios(bios_flags);
			if (defaultname != nullptr && core_stricmp(biosname, defaultname) == 0)
				default_no = bios_flags;
			bios_count++;
		}

	/* if none found, use the default */
	if (device->system_bios() == 0 && bios_count > 0)
	{
		/* if we got neither an empty string nor 'default' then warn the user */
		if (specbios[0] != 0 && strcmp(specbios, "default") != 0)
		{
			strcatprintf(m_errorstring, "%s: invalid bios, reverting to default\n", specbios);
			m_warnings++;
		}

		/* set to default */
		device->set_system_bios(default_no);
	}
	device->set_default_bios(default_no);
	LOG(("For \"%s\" using System BIOS: %d\n", device->tag(), device->system_bios()));
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
	device_iterator deviter(machine().config().root_device());
	for (device_t *device = deviter.first(); device != nullptr; device = deviter.next())
		for (region = rom_first_region(*device); region != nullptr; region = rom_next_region(region))
			for (rom = rom_first_file(region); rom != nullptr; rom = rom_next_file(rom))
				if (ROM_GETBIOSFLAGS(rom) == 0 || ROM_GETBIOSFLAGS(rom) == device->system_bios())
				{
					m_romstotal++;
					m_romstotalsize += rom_file_size(rom);
				}
}


/*-------------------------------------------------
    fill_random - fills an area of memory with
    random data
-------------------------------------------------*/

void rom_load_manager::fill_random(UINT8 *base, UINT32 length)
{
	while (length--)
		*base++ = machine().rand();
}


/*-------------------------------------------------
    handle_missing_file - handles error generation
    for missing files
-------------------------------------------------*/

void rom_load_manager::handle_missing_file(const rom_entry *romp, std::string tried_file_names, chd_error chderr)
{
	if(tried_file_names.length() != 0)
		tried_file_names = " (tried in " + tried_file_names + ")";

	std::string name(ROM_GETNAME(romp));

	bool is_chd = (chderr != CHDERR_NONE);
	if (is_chd)
		name += ".chd";

	bool is_chd_error = (is_chd && chderr != CHDERR_FILE_NOT_FOUND);
	if (is_chd_error)
		strcatprintf(m_errorstring, "%s CHD ERROR: %s\n", name.c_str(), chd_file::error_string(chderr));

	/* optional files are okay */
	if (ROM_ISOPTIONAL(romp))
	{
		if (!is_chd_error)
			strcatprintf(m_errorstring, "OPTIONAL %s NOT FOUND%s\n", name.c_str(), tried_file_names.c_str());
		m_warnings++;
	}

	/* no good dumps are okay */
	else if (hash_collection(ROM_GETHASHDATA(romp)).flag(hash_collection::FLAG_NO_DUMP))
	{
		if (!is_chd_error)
			strcatprintf(m_errorstring, "%s NOT FOUND (NO GOOD DUMP KNOWN)%s\n", name.c_str(), tried_file_names.c_str());
		m_knownbad++;
	}

	/* anything else is bad */
	else
	{
		if (!is_chd_error)
			strcatprintf(m_errorstring, "%s NOT FOUND%s\n", name.c_str(), tried_file_names.c_str());
		m_errors++;
	}
}


/*-------------------------------------------------
    dump_wrong_and_correct_checksums - dump an
    error message containing the wrong and the
    correct checksums for a given ROM
-------------------------------------------------*/

void rom_load_manager::dump_wrong_and_correct_checksums(const hash_collection &hashes, const hash_collection &acthashes)
{
	strcatprintf(m_errorstring, "    EXPECTED: %s\n", hashes.macro_string().c_str());
	strcatprintf(m_errorstring, "       FOUND: %s\n", acthashes.macro_string().c_str());
}


/*-------------------------------------------------
    verify_length_and_hash - verify the length
    and hash signatures of a file
-------------------------------------------------*/

void rom_load_manager::verify_length_and_hash(const char *name, UINT32 explength, const hash_collection &hashes)
{
	/* we've already complained if there is no file */
	if (m_file == nullptr)
		return;

	/* verify length */
	UINT32 actlength = m_file->size();
	if (explength != actlength)
	{
		strcatprintf(m_errorstring, "%s WRONG LENGTH (expected: %08x found: %08x)\n", name, explength, actlength);
		m_warnings++;
	}

	/* If there is no good dump known, write it */
	hash_collection &acthashes = m_file->hashes(hashes.hash_types().c_str());
	if (hashes.flag(hash_collection::FLAG_NO_DUMP))
	{
		strcatprintf(m_errorstring, "%s NO GOOD DUMP KNOWN\n", name);
		m_knownbad++;
	}
	/* verify checksums */
	else if (hashes != acthashes)
	{
		/* otherwise, it's just bad */
		strcatprintf(m_errorstring, "%s WRONG CHECKSUMS:\n", name);
		dump_wrong_and_correct_checksums(hashes, acthashes);
		m_warnings++;
	}
	/* If it matches, but it is actually a bad dump, write it */
	else if (hashes.flag(hash_collection::FLAG_BAD_DUMP))
	{
		strcatprintf(m_errorstring, "%s ROM NEEDS REDUMP\n",name);
		m_knownbad++;
	}
}


/*-------------------------------------------------
    display_loading_rom_message - display
    messages about ROM loading to the user
-------------------------------------------------*/

void rom_load_manager::display_loading_rom_message(const char *name, bool from_list)
{
	char buffer[200];

	if (name != nullptr)
		sprintf(buffer, "Loading %s (%d%%)", from_list ? "Software" : emulator_info::get_capstartgamenoun(), (UINT32)(100 * (UINT64)m_romsloadedsize / (UINT64)m_romstotalsize));
	else
		sprintf(buffer, "Loading Complete");

	if (!machine().ui().is_menu_active())
		machine().ui().set_startup_text(buffer, false);
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
		osd_printf_error("%s", m_errorstring.c_str());
		fatalerror_exitcode(machine(), MAMERR_MISSING_FILES, "Required files are missing, the %s cannot be run.",emulator_info::get_gamenoun());
	}

	/* if we had warnings, output them, but continue */
	if ((m_warnings) || (m_knownbad))
	{
		m_errorstring.append("WARNING: the ");
		m_errorstring.append(emulator_info::get_gamenoun());
		m_errorstring.append(" might not run correctly.");
		osd_printf_warning("%s\n", m_errorstring.c_str());
	}
}


/*-------------------------------------------------
    region_post_process - post-process a region,
    byte swapping and inverting data as necessary
-------------------------------------------------*/

void rom_load_manager::region_post_process(const char *rgntag, bool invert)
{
	memory_region *region = machine().root_device().memregion(rgntag);
	UINT8 *base;
	int i, j;

	// do nothing if no region
	if (region == nullptr)
		return;

	LOG(("+ datawidth=%dbit endian=%s\n", region->bitwidth(),
			region->endianness() == ENDIANNESS_LITTLE ? "little" : "big"));

	/* if the region is inverted, do that now */
	if (invert)
	{
		LOG(("+ Inverting region\n"));
		for (i = 0, base = region->base(); i < region->bytes(); i++)
			*base++ ^= 0xff;
	}

	/* swap the endianness if we need to */
	if (region->bytewidth() > 1 && region->endianness() != ENDIANNESS_NATIVE)
	{
		LOG(("+ Byte swapping region\n"));
		int datawidth = region->bytewidth();
		for (i = 0, base = region->base(); i < region->bytes(); i += datawidth)
		{
			UINT8 temp[8];
			memcpy(temp, base, datawidth);
			for (j = datawidth - 1; j >= 0; j--)
				*base++ = temp[j];
		}
	}
}


/*-------------------------------------------------
    open_rom_file - open a ROM file, searching
    up the parent and loading by checksum
-------------------------------------------------*/

int rom_load_manager::open_rom_file(const char *regiontag, const rom_entry *romp, std::string &tried_file_names, bool from_list)
{
	file_error filerr = FILERR_NOT_FOUND;
	UINT32 romsize = rom_file_size(romp);
	tried_file_names = "";

	/* update status display */
	display_loading_rom_message(ROM_GETNAME(romp), from_list);

	/* extract CRC to use for searching */
	UINT32 crc = 0;
	bool has_crc = hash_collection(ROM_GETHASHDATA(romp)).crc(crc);

	/* attempt reading up the chain through the parents. It automatically also
	 attempts any kind of load by checksum supported by the archives. */
	m_file = nullptr;
	for (int drv = driver_list::find(machine().system()); m_file == nullptr && drv != -1; drv = driver_list::clone(drv)) {
		if (tried_file_names.length() != 0)
			tried_file_names += " ";
		tried_file_names += driver_list::driver(drv).name;
		m_file = common_process_file(machine().options(), driver_list::driver(drv).name, has_crc, crc, romp, filerr);
	}

	/* if the region is load by name, load the ROM from there */
	if (m_file == nullptr && regiontag != nullptr)
	{
		// check if we are dealing with softwarelists. if so, locationtag
		// is actually a concatenation of: listname + setname + parentname
		// separated by '%' (parentname being present only for clones)
		std::string tag1(regiontag), tag2, tag3, tag4, tag5;
		bool is_list = FALSE;
		bool has_parent = FALSE;

		int separator1 = tag1.find_first_of('%');
		if (separator1 != -1)
		{
			is_list = TRUE;

			// we are loading through softlists, split the listname from the regiontag
			tag4.assign(tag1.substr(separator1 + 1, tag1.length() - separator1 + 1));
			tag1.erase(separator1, tag1.length() - separator1);
			tag1.append(PATH_SEPARATOR);

			// check if we are loading a clone (if this is the case also tag1 have a separator '%')
			int separator2 = tag4.find_first_of('%');
			if (separator2 != -1)
			{
				has_parent = TRUE;

				// we are loading a clone through softlists, split the setname from the parentname
				tag5.assign(tag4.substr(separator2 + 1, tag4.length() - separator2 + 1));
				tag4.erase(separator2, tag4.length() - separator2);
			}

			// prepare locations where we have to load from: list/parentname & list/clonename
			std::string swlist(tag1);
			tag2.assign(swlist.append(tag4));
			if (has_parent)
			{
				swlist.assign(tag1);
				tag3.assign(swlist.append(tag5));
			}
		}

		if (tag5.find_first_of('%') != -1)
			fatalerror("We do not support clones of clones!\n");

		// try to load from the available location(s):
		// - if we are not using lists, we have regiontag only;
		// - if we are using lists, we have: list/clonename, list/parentname, clonename, parentname
		if (!is_list)
		{
			tried_file_names += " " + tag1;
			m_file = common_process_file(machine().options(), tag1.c_str(), has_crc, crc, romp, filerr);
		}
		else
		{
			// try to load from list/setname
			if ((m_file == nullptr) && (tag2.c_str() != nullptr))
			{
				tried_file_names += " " + tag2;
				m_file = common_process_file(machine().options(), tag2.c_str(), has_crc, crc, romp, filerr);
			}
			// try to load from list/parentname
			if ((m_file == nullptr) && has_parent && (tag3.c_str() != nullptr))
			{
				tried_file_names += " " + tag3;
				m_file = common_process_file(machine().options(), tag3.c_str(), has_crc, crc, romp, filerr);
			}
			// try to load from setname
			if ((m_file == nullptr) && (tag4.c_str() != nullptr))
			{
				tried_file_names += " " + tag4;
				m_file = common_process_file(machine().options(), tag4.c_str(), has_crc, crc, romp, filerr);
			}
			// try to load from parentname
			if ((m_file == nullptr) && has_parent && (tag5.c_str() != nullptr))
			{
				tried_file_names += " " + tag5;
				m_file = common_process_file(machine().options(), tag5.c_str(), has_crc, crc, romp, filerr);
			}
		}
	}

	/* update counters */
	m_romsloaded++;
	m_romsloadedsize += romsize;

	/* return the result */
	return (filerr == FILERR_NONE);
}


/*-------------------------------------------------
    rom_fread - cheesy fread that fills with
    random data for a NULL file
-------------------------------------------------*/

int rom_load_manager::rom_fread(UINT8 *buffer, int length, const rom_entry *parent_region)
{
	/* files just pass through */
	if (m_file != nullptr)
		return m_file->read(buffer, length);

	/* otherwise, fill with randomness unless it was already specifically erased */
	else if (!ROMREGION_ISERASE(parent_region))
		fill_random(buffer, length);

	return length;
}


/*-------------------------------------------------
    read_rom_data - read ROM data for a single
    entry
-------------------------------------------------*/

int rom_load_manager::read_rom_data(const rom_entry *parent_region, const rom_entry *romp)
{
	int datashift = ROM_GETBITSHIFT(romp);
	int datamask = ((1 << ROM_GETBITWIDTH(romp)) - 1) << datashift;
	int numbytes = ROM_GETLENGTH(romp);
	int groupsize = ROM_GETGROUPSIZE(romp);
	int skip = ROM_GETSKIPCOUNT(romp);
	int reversed = ROM_ISREVERSED(romp);
	int numgroups = (numbytes + groupsize - 1) / groupsize;
	UINT8 *base = m_region->base() + ROM_GETOFFSET(romp);
	UINT32 tempbufsize;
	int i;

	LOG(("Loading ROM data: offs=%X len=%X mask=%02X group=%d skip=%d reverse=%d\n", ROM_GETOFFSET(romp), numbytes, datamask, groupsize, skip, reversed));

	/* make sure the length was an even multiple of the group size */
	if (numbytes % groupsize != 0)
		osd_printf_warning("Warning in RomModule definition: %s length not an even multiple of group size\n", ROM_GETNAME(romp));

	/* make sure we only fill within the region space */
	if (ROM_GETOFFSET(romp) + numgroups * groupsize + (numgroups - 1) * skip > m_region->bytes())
		fatalerror("Error in RomModule definition: %s out of memory region space\n", ROM_GETNAME(romp));

	/* make sure the length was valid */
	if (numbytes == 0)
		fatalerror("Error in RomModule definition: %s has an invalid length\n", ROM_GETNAME(romp));

	/* special case for simple loads */
	if (datamask == 0xff && (groupsize == 1 || !reversed) && skip == 0)
		return rom_fread(base, numbytes, parent_region);

	/* use a temporary buffer for complex loads */
	tempbufsize = MIN(TEMPBUFFER_MAX_SIZE, numbytes);
	dynamic_buffer tempbuf(tempbufsize);

	/* chunky reads for complex loads */
	skip += groupsize;
	while (numbytes > 0)
	{
		int evengroupcount = (tempbufsize / groupsize) * groupsize;
		int bytesleft = (numbytes > evengroupcount) ? evengroupcount : numbytes;
		UINT8 *bufptr = &tempbuf[0];

		/* read as much as we can */
		LOG(("  Reading %X bytes into buffer\n", bytesleft));
		if (rom_fread(bufptr, bytesleft, parent_region) != bytesleft)
			return 0;
		numbytes -= bytesleft;

		LOG(("  Copying to %p\n", base));

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

	LOG(("  All done\n"));
	return ROM_GETLENGTH(romp);
}


/*-------------------------------------------------
    fill_rom_data - fill a region of ROM space
-------------------------------------------------*/

void rom_load_manager::fill_rom_data(const rom_entry *romp)
{
	UINT32 numbytes = ROM_GETLENGTH(romp);
	UINT8 *base = m_region->base() + ROM_GETOFFSET(romp);

	/* make sure we fill within the region space */
	if (ROM_GETOFFSET(romp) + numbytes > m_region->bytes())
		fatalerror("Error in RomModule definition: FILL out of memory region space\n");

	/* make sure the length was valid */
	if (numbytes == 0)
		fatalerror("Error in RomModule definition: FILL has an invalid length\n");

	/* fill the data (filling value is stored in place of the hashdata) */
	memset(base, (FPTR)ROM_GETHASHDATA(romp) & 0xff, numbytes);
}


/*-------------------------------------------------
    copy_rom_data - copy a region of ROM space
-------------------------------------------------*/

void rom_load_manager::copy_rom_data(const rom_entry *romp)
{
	UINT8 *base = m_region->base() + ROM_GETOFFSET(romp);
	const char *srcrgntag = ROM_GETNAME(romp);
	UINT32 numbytes = ROM_GETLENGTH(romp);
	UINT32 srcoffs = (FPTR)ROM_GETHASHDATA(romp);  /* srcoffset in place of hashdata */

	/* make sure we copy within the region space */
	if (ROM_GETOFFSET(romp) + numbytes > m_region->bytes())
		fatalerror("Error in RomModule definition: COPY out of target memory region space\n");

	/* make sure the length was valid */
	if (numbytes == 0)
		fatalerror("Error in RomModule definition: COPY has an invalid length\n");

	/* make sure the source was valid */
	memory_region *region = machine().root_device().memregion(srcrgntag);
	if (region == nullptr)
		fatalerror("Error in RomModule definition: COPY from an invalid region\n");

	/* make sure we find within the region space */
	if (srcoffs + numbytes > region->bytes())
		fatalerror("Error in RomModule definition: COPY out of source memory region space\n");

	/* fill the data */
	memcpy(base, region->base() + srcoffs, numbytes);
}


/*-------------------------------------------------
    process_rom_entries - process all ROM entries
    for a region
-------------------------------------------------*/

void rom_load_manager::process_rom_entries(const char *regiontag, const rom_entry *parent_region, const rom_entry *romp, device_t *device, bool from_list)
{
	UINT32 lastflags = 0;

	/* loop until we hit the end of this region */
	while (!ROMENTRY_ISREGIONEND(romp))
	{
		/* if this is a continue entry, it's invalid */
		if (ROMENTRY_ISCONTINUE(romp))
			fatalerror("Error in RomModule definition: ROM_CONTINUE not preceded by ROM_LOAD\n");

		/* if this is an ignore entry, it's invalid */
		if (ROMENTRY_ISIGNORE(romp))
			fatalerror("Error in RomModule definition: ROM_IGNORE not preceded by ROM_LOAD\n");

		/* if this is a reload entry, it's invalid */
		if (ROMENTRY_ISRELOAD(romp))
			fatalerror("Error in RomModule definition: ROM_RELOAD not preceded by ROM_LOAD\n");

		/* handle fills */
		if (ROMENTRY_ISFILL(romp))
			fill_rom_data(romp++);

		/* handle copies */
		else if (ROMENTRY_ISCOPY(romp))
			copy_rom_data(romp++);

		/* handle files */
		else if (ROMENTRY_ISFILE(romp))
		{
			int irrelevantbios = (ROM_GETBIOSFLAGS(romp) != 0 && ROM_GETBIOSFLAGS(romp) != device->system_bios());
			const rom_entry *baserom = romp;
			int explength = 0;

			/* open the file if it is a non-BIOS or matches the current BIOS */
			LOG(("Opening ROM file: %s\n", ROM_GETNAME(romp)));
			std::string tried_file_names;
			if (!irrelevantbios && !open_rom_file(regiontag, romp, tried_file_names, from_list))
				handle_missing_file(romp, tried_file_names, CHDERR_NONE);

			/* loop until we run out of reloads */
			do
			{
				/* loop until we run out of continues/ignores */
				do
				{
					rom_entry modified_romp = *romp++;
					//int readresult;

					/* handle flag inheritance */
					if (!ROM_INHERITSFLAGS(&modified_romp))
						lastflags = modified_romp._flags;
					else
						modified_romp._flags = (modified_romp._flags & ~ROM_INHERITEDFLAGS) | lastflags;

					explength += ROM_GETLENGTH(&modified_romp);

					/* attempt to read using the modified entry */
					if (!ROMENTRY_ISIGNORE(&modified_romp) && !irrelevantbios)
						/*readresult = */read_rom_data(parent_region, &modified_romp);
				}
				while (ROMENTRY_ISCONTINUE(romp) || ROMENTRY_ISIGNORE(romp));

				/* if this was the first use of this file, verify the length and CRC */
				if (baserom)
				{
					LOG(("Verifying length (%X) and checksums\n", explength));
					verify_length_and_hash(ROM_GETNAME(baserom), explength, hash_collection(ROM_GETHASHDATA(baserom)));
					LOG(("Verify finished\n"));
				}

				/* reseek to the start and clear the baserom so we don't reverify */
				if (m_file != nullptr)
					m_file->seek(0, SEEK_SET);
				baserom = nullptr;
				explength = 0;
			}
			while (ROMENTRY_ISRELOAD(romp));

			/* close the file */
			if (m_file != nullptr)
			{
				LOG(("Closing ROM file\n"));
				m_file = nullptr;
			}
		}
		else
		{
			romp++; /* something else; skip */
		}
	}
}


/*-------------------------------------------------
    open_disk_image - open a disk image,
    searching up the parent and loading by
    checksum
-------------------------------------------------*/

int open_disk_image(emu_options &options, const game_driver *gamedrv, const rom_entry *romp, chd_file &image_chd, const char *locationtag)
{
	emu_file image_file(options.media_path(), OPEN_FLAG_READ);
	const rom_entry *region, *rom;
	file_error filerr;
	chd_error err;

	/* attempt to open the properly named file, scanning up through parent directories */
	filerr = FILERR_NOT_FOUND;
	for (int searchdrv = driver_list::find(*gamedrv); searchdrv != -1 && filerr != FILERR_NONE; searchdrv = driver_list::clone(searchdrv))
		filerr = common_process_file(options, driver_list::driver(searchdrv).name, ".chd", romp, image_file);

	if (filerr != FILERR_NONE)
		filerr = common_process_file(options, nullptr, ".chd", romp, image_file);

	/* look for the disk in the locationtag too */
	if (filerr != FILERR_NONE && locationtag != nullptr)
	{
		// check if we are dealing with softwarelists. if so, locationtag
		// is actually a concatenation of: listname + setname + parentname
		// separated by '%' (parentname being present only for clones)
		std::string tag1(locationtag), tag2, tag3, tag4, tag5;
		bool is_list = FALSE;
		bool has_parent = FALSE;

		int separator1 = tag1.find_first_of('%');
		if (separator1 != -1)
		{
			is_list = TRUE;

			// we are loading through softlists, split the listname from the regiontag
			tag4.assign(tag1.substr(separator1 + 1, tag1.length() - separator1 + 1));
			tag1.erase(separator1, tag1.length() - separator1);
			tag1.append(PATH_SEPARATOR);

			// check if we are loading a clone (if this is the case also tag1 have a separator '%')
			int separator2 = tag4.find_first_of('%');
			if (separator2 != -1)
			{
				has_parent = TRUE;

				// we are loading a clone through softlists, split the setname from the parentname
				tag5.assign(tag4.substr(separator2 + 1, tag4.length() - separator2 + 1));
				tag4.erase(separator2, tag4.length() - separator2);
			}

			// prepare locations where we have to load from: list/parentname (if any) & list/clonename
			std::string swlist(tag1);
			tag2.assign(swlist.append(tag4));
			if (has_parent)
			{
				swlist.assign(tag1);
				tag3.assign(swlist.append(tag5));
			}
		}

		if (tag5.find_first_of('%') != -1)
			fatalerror("We do not support clones of clones!\n");

		// try to load from the available location(s):
		// - if we are not using lists, we have locationtag only;
		// - if we are using lists, we have: list/clonename, list/parentname, clonename, parentname
		if (!is_list)
			filerr = common_process_file(options, locationtag, ".chd", romp, image_file);
		else
		{
			// try to load from list/setname
			if ((filerr != FILERR_NONE) && (tag2.c_str() != nullptr))
				filerr = common_process_file(options, tag2.c_str(), ".chd", romp, image_file);
			// try to load from list/parentname (if any)
			if ((filerr != FILERR_NONE) && has_parent && (tag3.c_str() != nullptr))
				filerr = common_process_file(options, tag3.c_str(), ".chd", romp, image_file);
			// try to load from setname
			if ((filerr != FILERR_NONE) && (tag4.c_str() != nullptr))
				filerr = common_process_file(options, tag4.c_str(), ".chd", romp, image_file);
			// try to load from parentname (if any)
			if ((filerr != FILERR_NONE) && has_parent && (tag5.c_str() != nullptr))
				filerr = common_process_file(options, tag5.c_str(), ".chd", romp, image_file);
			// only for CHD we also try to load from list/
			if ((filerr != FILERR_NONE) && (tag1.c_str() != nullptr))
			{
				tag1.erase(tag1.length() - 1, 1);    // remove the PATH_SEPARATOR
				filerr = common_process_file(options, tag1.c_str(), ".chd", romp, image_file);
			}
		}
	}

	/* did the file open succeed? */
	if (filerr == FILERR_NONE)
	{
		std::string fullpath(image_file.fullpath());
		image_file.close();

		/* try to open the CHD */
		err = image_chd.open(fullpath.c_str());
		if (err == CHDERR_NONE)
			return err;
	}
	else
		err = CHDERR_FILE_NOT_FOUND;

	/* otherwise, look at our parents for a CHD with an identical checksum */
	/* and try to open that */
	hash_collection romphashes(ROM_GETHASHDATA(romp));
	for (int drv = driver_list::find(*gamedrv); drv != -1; drv = driver_list::clone(drv))
	{
		machine_config config(driver_list::driver(drv), options);
		device_iterator deviter(config.root_device());
		for (device_t *device = deviter.first(); device != nullptr; device = deviter.next())
			for (region = rom_first_region(*device); region != nullptr; region = rom_next_region(region))
				if (ROMREGION_ISDISKDATA(region))
					for (rom = rom_first_file(region); rom != nullptr; rom = rom_next_file(rom))

						/* look for a differing name but with the same hash data */
						if (strcmp(ROM_GETNAME(romp), ROM_GETNAME(rom)) != 0 &&
							romphashes == hash_collection(ROM_GETHASHDATA(rom)))
						{
							/* attempt to open the properly named file, scanning up through parent directories */
							filerr = FILERR_NOT_FOUND;
							for (int searchdrv = drv; searchdrv != -1 && filerr != FILERR_NONE; searchdrv = driver_list::clone(searchdrv))
								filerr = common_process_file(options, driver_list::driver(searchdrv).name, ".chd", rom, image_file);

							if (filerr != FILERR_NONE)
								filerr = common_process_file(options, nullptr, ".chd", rom, image_file);

							/* did the file open succeed? */
							if (filerr == FILERR_NONE)
							{
								std::string fullpath(image_file.fullpath());
								image_file.close();

								/* try to open the CHD */
								err = image_chd.open(fullpath.c_str());
								if (err == CHDERR_NONE)
									return err;
							}
						}
	}
	return err;
}


/*-------------------------------------------------
    open_disk_diff - open a DISK diff file
-------------------------------------------------*/

chd_error rom_load_manager::open_disk_diff(emu_options &options, const rom_entry *romp, chd_file &source, chd_file &diff_chd)
{
	std::string fname = std::string(ROM_GETNAME(romp)).append(".dif");

	/* try to open the diff */
	LOG(("Opening differencing image file: %s\n", fname.c_str()));
	emu_file diff_file(options.diff_directory(), OPEN_FLAG_READ | OPEN_FLAG_WRITE);
	file_error filerr = diff_file.open(fname.c_str());
	if (filerr == FILERR_NONE)
	{
		std::string fullpath(diff_file.fullpath());
		diff_file.close();

		LOG(("Opening differencing image file: %s\n", fullpath.c_str()));
		return diff_chd.open(fullpath.c_str(), true, &source);
	}

	/* didn't work; try creating it instead */
	LOG(("Creating differencing image: %s\n", fname.c_str()));
	diff_file.set_openflags(OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	filerr = diff_file.open(fname.c_str());
	if (filerr == FILERR_NONE)
	{
		std::string fullpath(diff_file.fullpath());
		diff_file.close();

		/* create the CHD */
		LOG(("Creating differencing image file: %s\n", fullpath.c_str()));
		chd_codec_type compression[4] = { CHD_CODEC_NONE };
		chd_error err = diff_chd.create(fullpath.c_str(), source.logical_bytes(), source.hunk_bytes(), compression, source);
		if (err != CHDERR_NONE)
			return err;

		return diff_chd.clone_all_metadata(source);
	}

	return CHDERR_FILE_NOT_FOUND;
}


/*-------------------------------------------------
    process_disk_entries - process all disk entries
    for a region
-------------------------------------------------*/

void rom_load_manager::process_disk_entries(const char *regiontag, const rom_entry *parent_region, const rom_entry *romp, const char *locationtag)
{
	/* loop until we hit the end of this region */
	for ( ; !ROMENTRY_ISREGIONEND(romp); romp++)
	{
		/* handle files */
		if (ROMENTRY_ISFILE(romp))
		{
			auto chd = std::make_unique<open_chd>(regiontag);

			hash_collection hashes(ROM_GETHASHDATA(romp));
			chd_error err;

			/* make the filename of the source */
			std::string filename = std::string(ROM_GETNAME(romp)).append(".chd");

			/* first open the source drive */
			LOG(("Opening disk image: %s\n", filename.c_str()));
			err = chd_error(open_disk_image(machine().options(), &machine().system(), romp, chd->orig_chd(), locationtag));
			if (err != CHDERR_NONE)
			{
				handle_missing_file(romp, std::string(), err);
				chd = nullptr;
				continue;
			}

			/* get the header and extract the SHA1 */
			hash_collection acthashes;
			acthashes.add_sha1(chd->orig_chd().sha1());

			/* verify the hash */
			if (hashes != acthashes)
			{
				strcatprintf(m_errorstring, "%s WRONG CHECKSUMS:\n", filename.c_str());
				dump_wrong_and_correct_checksums(hashes, acthashes);
				m_warnings++;
			}
			else if (hashes.flag(hash_collection::FLAG_BAD_DUMP))
			{
				strcatprintf(m_errorstring, "%s CHD NEEDS REDUMP\n", filename.c_str());
				m_knownbad++;
			}

			/* if not read-only, make the diff file */
			if (!DISK_ISREADONLY(romp))
			{
				/* try to open or create the diff */
				err = open_disk_diff(machine().options(), romp, chd->orig_chd(), chd->diff_chd());
				if (err != CHDERR_NONE)
				{
					strcatprintf(m_errorstring, "%s DIFF CHD ERROR: %s\n", filename.c_str(), chd_file::error_string(err));
					m_errors++;
					chd = nullptr;
					continue;
				}
			}

			/* we're okay, add to the list of disks */
			LOG(("Assigning to handle %d\n", DISK_GETINDEX(romp)));
			m_chd_list.push_back(std::move(chd));
		}
	}
}


/*-------------------------------------------------
    normalize_flags_for_device - modify the region
    flags for the given device
-------------------------------------------------*/

void rom_load_manager::normalize_flags_for_device(running_machine &machine, const char *rgntag, UINT8 &width, endianness_t &endian)
{
	device_t *device = machine.device(rgntag);
	device_memory_interface *memory;
	if (device->interface(memory))
	{
		const address_space_config *spaceconfig = memory->space_config();
		if (spaceconfig != nullptr)
		{
			int buswidth;

			/* set the endianness */
			if (spaceconfig->m_endianness == ENDIANNESS_LITTLE)
				endian = ENDIANNESS_LITTLE;
			else
				endian = ENDIANNESS_BIG;

			/* set the width */
			buswidth = spaceconfig->m_databus_width;
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

    This is used by MESS when loading a piece of
    software. The code should be merged with
    process_region_list or updated to use a slight
    more general process_region_list.
-------------------------------------------------*/

void rom_load_manager::load_software_part_region(device_t &device, software_list_device &swlist, const char *swname, const rom_entry *start_region)
{
	std::string locationtag(swlist.list_name()), breakstr("%");
	const rom_entry *region;
	std::string regiontag;

	m_errorstring.clear();
	m_softwarningstring.clear();

	m_romstotal = 0;
	m_romstotalsize = 0;
	m_romsloadedsize = 0;

	software_info *swinfo = swlist.find(swname);
	if (swinfo != nullptr)
	{
		UINT32 supported = swinfo->supported();
		if (supported == SOFTWARE_SUPPORTED_PARTIAL)
		{
			strcatprintf(m_errorstring, "WARNING: support for software %s (in list %s) is only partial\n", swname, swlist.list_name());
			strcatprintf(m_softwarningstring, "Support for software %s (in list %s) is only partial\n", swname, swlist.list_name());
		}
		if (supported == SOFTWARE_SUPPORTED_NO)
		{
			strcatprintf(m_errorstring, "WARNING: support for software %s (in list %s) is only preliminary\n", swname, swlist.list_name());
			strcatprintf(m_softwarningstring, "Support for software %s (in list %s) is only preliminary\n", swname, swlist.list_name());
		}

		// attempt reading up the chain through the parents and create a locationtag std::string in the format
		// " swlist % clonename % parentname "
		// open_rom_file contains the code to split the elements and to create paths to load from

		locationtag.append(breakstr);

		while (swinfo != nullptr)
		{
			locationtag.append(swinfo->shortname()).append(breakstr);
			const char *parentname = swinfo->parentname();
			swinfo = (parentname != nullptr) ? swlist.find(parentname) : nullptr;
		}
		// strip the final '%'
		locationtag.erase(locationtag.length() - 1, 1);
	}


	/* loop until we hit the end */
	for (region = start_region; region != nullptr; region = rom_next_region(region))
	{
		UINT32 regionlength = ROMREGION_GETLENGTH(region);

		regiontag = device.subtag(ROMREGION_GETTAG(region));
		LOG(("Processing region \"%s\" (length=%X)\n", regiontag.c_str(), regionlength));

		/* the first entry must be a region */
		assert(ROMENTRY_ISREGION(region));

		/* if this is a device region, override with the device width and endianness */
		endianness_t endianness = ROMREGION_ISBIGENDIAN(region) ? ENDIANNESS_BIG : ENDIANNESS_LITTLE;
		UINT8 width = ROMREGION_GETWIDTH(region) / 8;
		memory_region *memregion = machine().root_device().memregion(regiontag.c_str());
		if (memregion != nullptr)
		{
			if (machine().device(regiontag.c_str()) != nullptr)
				normalize_flags_for_device(machine(), regiontag.c_str(), width, endianness);

			/* clear old region (todo: should be moved to an image unload function) */
			machine().memory().region_free(memregion->name());
		}

		/* remember the base and length */
		m_region = machine().memory().region_alloc(regiontag.c_str(), regionlength, width, endianness);
		LOG(("Allocated %X bytes @ %p\n", m_region->bytes(), m_region->base()));

		/* clear the region if it's requested */
		if (ROMREGION_ISERASE(region))
			memset(m_region->base(), ROMREGION_GETERASEVAL(region), m_region->bytes());

		/* or if it's sufficiently small (<= 4MB) */
		else if (m_region->bytes() <= 0x400000)
			memset(m_region->base(), 0, m_region->bytes());

#ifdef MAME_DEBUG
		/* if we're debugging, fill region with random data to catch errors */
		else
			fill_random(m_region->base(), m_region->bytes());
#endif

		/* update total number of roms */
		for (const rom_entry *rom = rom_first_file(region); rom != nullptr; rom = rom_next_file(rom))
		{
			m_romstotal++;
			m_romstotalsize += rom_file_size(rom);
		}

		/* now process the entries in the region */
		if (ROMREGION_ISROMDATA(region))
			process_rom_entries(locationtag.c_str(), region, region + 1, &device, TRUE);
		else if (ROMREGION_ISDISKDATA(region))
			process_disk_entries(regiontag.c_str(), region, region + 1, locationtag.c_str());
	}

	/* now go back and post-process all the regions */
	for (region = start_region; region != nullptr; region = rom_next_region(region))
	{
		regiontag = device.subtag(ROMREGION_GETTAG(region));
		region_post_process(regiontag.c_str(), ROMREGION_ISINVERTED(region));
	}

	/* display the results and exit */
	display_rom_load_results(TRUE);
}


/*-------------------------------------------------
    process_region_list - process a region list
-------------------------------------------------*/

void rom_load_manager::process_region_list()
{
	std::string regiontag;

	/* loop until we hit the end */
	device_iterator deviter(machine().root_device());
	for (device_t *device = deviter.first(); device != nullptr; device = deviter.next())
		for (const rom_entry *region = rom_first_region(*device); region != nullptr; region = rom_next_region(region))
		{
			UINT32 regionlength = ROMREGION_GETLENGTH(region);

			regiontag = rom_region_name(*device, region);
			LOG(("Processing region \"%s\" (length=%X)\n", regiontag.c_str(), regionlength));

			/* the first entry must be a region */
			assert(ROMENTRY_ISREGION(region));

			if (ROMREGION_ISROMDATA(region))
			{
				/* if this is a device region, override with the device width and endianness */
				UINT8 width = ROMREGION_GETWIDTH(region) / 8;
				endianness_t endianness = ROMREGION_ISBIGENDIAN(region) ? ENDIANNESS_BIG : ENDIANNESS_LITTLE;
				if (machine().device(regiontag.c_str()) != nullptr)
					normalize_flags_for_device(machine(), regiontag.c_str(), width, endianness);

				/* remember the base and length */
				m_region = machine().memory().region_alloc(regiontag.c_str(), regionlength, width, endianness);
				LOG(("Allocated %X bytes @ %p\n", m_region->bytes(), m_region->base()));

				/* clear the region if it's requested */
				if (ROMREGION_ISERASE(region))
					memset(m_region->base(), ROMREGION_GETERASEVAL(region), m_region->bytes());

				/* or if it's sufficiently small (<= 4MB) */
				else if (m_region->bytes() <= 0x400000)
					memset(m_region->base(), 0, m_region->bytes());

#ifdef MAME_DEBUG
				/* if we're debugging, fill region with random data to catch errors */
				else
					fill_random(m_region->base(), m_region->bytes());
#endif

				/* now process the entries in the region */
				process_rom_entries(device->shortname(), region, region + 1, device, FALSE);
			}
			else if (ROMREGION_ISDISKDATA(region))
				process_disk_entries(regiontag.c_str(), region, region + 1, nullptr);
		}

	/* now go back and post-process all the regions */
	for (device_t *device = deviter.first(); device != nullptr; device = deviter.next())
		for (const rom_entry *region = rom_first_region(*device); region != nullptr; region = rom_next_region(region))
		{
			regiontag = rom_region_name(*device, region);
			region_post_process(regiontag.c_str(), ROMREGION_ISINVERTED(region));
		}

	/* and finally register all per-game parameters */
	for (device_t *device = deviter.first(); device != nullptr; device = deviter.next())
		for (const rom_entry *param = rom_first_parameter(*device); param != nullptr; param = rom_next_parameter(param))
		{
			regiontag = rom_parameter_name(*device, param);
			machine().parameters().add(regiontag, rom_parameter_value(param));
		}
}


/*-------------------------------------------------
    rom_init - load the ROMs and open the disk
    images associated with the given machine
-------------------------------------------------*/

rom_load_manager::rom_load_manager(running_machine &machine)
	: m_machine(machine)
{
	/* figure out which BIOS we are using */
	device_iterator deviter(machine.config().root_device());
	for (device_t *device = deviter.first(); device != nullptr; device = deviter.next()) {
		if (device->rom_region()) {
			std::string specbios;
			if (device->owner() == nullptr) {
				specbios.assign(machine.options().bios());
			} else {
				specbios = machine.options().sub_value(device->owner()->tag()+1,"bios");
				if (specbios.empty()) {
					specbios = device->default_bios_tag();
				}
			}
			determine_bios_rom(device, specbios.c_str());
		}
	}

	/* count the total number of ROMs */
	count_roms();

	/* reset the disk list */
	m_chd_list.clear();

	/* process the ROM entries we were passed */
	process_region_list();

	/* display the results and exit */
	display_rom_load_results(FALSE);
}
