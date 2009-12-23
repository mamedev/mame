/*********************************************************************

    romload.c

    ROM loading functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "osdepend.h"
#include "driver.h"
#include "hash.h"
#include "png.h"
#include "harddisk.h"
#include "config.h"
#include "ui.h"


#define LOG_LOAD 0
#define LOG(x) do { if (LOG_LOAD) debugload x; } while(0)


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define TEMPBUFFER_MAX_SIZE		(1024 * 1024 * 1024)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _open_chd open_chd;
struct _open_chd
{
	open_chd *			next;					/* pointer to next in the list */
	const char *		region;					/* disk region we came from */
	chd_file *			origchd;				/* handle to the original CHD */
	mame_file *			origfile;				/* file handle to the original CHD file */
	chd_file *			diffchd;				/* handle to the diff CHD */
	mame_file *			difffile;				/* file handle to the diff CHD file */
};


typedef struct _romload_private rom_load_data;
struct _romload_private
{
	running_machine *machine;			/* machine object where needed */
	int				system_bios;		/* the system BIOS we wish to load */

	int				warnings;			/* warning count during processing */
	int				errors;				/* error count during processing */

	int				romsloaded;			/* current ROMs loaded count */
	int				romstotal;			/* total number of ROMs to read */
	UINT32			romsloadedsize;		/* total size of ROMs loaded so far */
	UINT32			romstotalsize;		/* total size of ROMs to read */

	mame_file *		file;				/* current file */
	open_chd *		chd_list;			/* disks */
	open_chd **		chd_list_tailptr;

	UINT8 *			regionbase;			/* base of current region */
	UINT32			regionlength;		/* length of current region */

	astring *		errorstring;		/* error string */
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void rom_exit(running_machine *machine);



/***************************************************************************
    HARD DISK HANDLING
***************************************************************************/

/*-------------------------------------------------
    get_disk_handle - return a pointer to the
    CHD file associated with the given region
-------------------------------------------------*/

chd_file *get_disk_handle(running_machine *machine, const char *region)
{
	open_chd *curdisk;

	for (curdisk = machine->romload_data->chd_list; curdisk != NULL; curdisk = curdisk->next)
		if (strcmp(curdisk->region, region) == 0)
			return (curdisk->diffchd != NULL) ? curdisk->diffchd : curdisk->origchd;
	return NULL;
}


/*-------------------------------------------------
    add_disk_handle - add a disk to the to the
    list of CHD files
-------------------------------------------------*/

static void add_disk_handle(running_machine *machine, open_chd *chd)
{
	romload_private *romload_data = machine->romload_data;

	*romload_data->chd_list_tailptr = auto_alloc(machine, open_chd);
	**romload_data->chd_list_tailptr = *chd;
	romload_data->chd_list_tailptr = &(*romload_data->chd_list_tailptr)->next;
}


/*-------------------------------------------------
    set_disk_handle - set a pointer to the CHD
    file associated with the given region
-------------------------------------------------*/

void set_disk_handle(running_machine *machine, const char *region, mame_file *file, chd_file *chdfile)
{
	open_chd chd = { 0 };

	/* note the region we are in */
	chd.region = region;
	chd.origchd = chdfile;
	chd.origfile = file;

	/* we're okay, add to the list of disks */
	add_disk_handle(machine, &chd);
}



/***************************************************************************
    ROM LOADING
***************************************************************************/

/*-------------------------------------------------
    rom_source_is_gamedrv - return TRUE if the
    given rom_source refers to the game driver
    itself
-------------------------------------------------*/

int rom_source_is_gamedrv(const game_driver *drv, const rom_source *source)
{
	return ((const game_driver *)source == drv);
}


/*-------------------------------------------------
    rom_first_source - return pointer to first ROM
    source
-------------------------------------------------*/

const rom_source *rom_first_source(const game_driver *drv, const machine_config *config)
{
	const device_config *device;

	/* if the driver has a ROM pointer, that's what we want */
	if (drv->rom != NULL)
		return (rom_source *)drv;

	/* otherwise, look through devices */
	if (config != NULL)
		for (device = config->devicelist.head; device != NULL; device = device->next)
		{
			const rom_entry *devromp = (const rom_entry *)device_get_info_ptr(device, DEVINFO_PTR_ROM_REGION);
			if (devromp != NULL)
				return (rom_source *)device;
		}
	return NULL;
}


/*-------------------------------------------------
    rom_next_source - return pointer to next ROM
    source
-------------------------------------------------*/

const rom_source *rom_next_source(const game_driver *drv, const machine_config *config, const rom_source *previous)
{
	const device_config *device;

	/* if the previous was the driver, we want the first device */
	if (rom_source_is_gamedrv(drv, previous))
		device = (config != NULL) ? config->devicelist.head : NULL;
	else
		device = ((const device_config *)previous)->next;

	/* look for further devices with ROM definitions */
	for ( ; device != NULL; device = device->next)
	{
		const rom_entry *devromp = (const rom_entry *)device_get_info_ptr(device, DEVINFO_PTR_ROM_REGION);
		if (devromp != NULL)
			return (rom_source *)device;
	}
	return NULL;
}


/*-------------------------------------------------
    rom_first_region - return pointer to first ROM
    region
-------------------------------------------------*/

const rom_entry *rom_first_region(const game_driver *drv, const rom_source *source)
{
	const rom_entry *romp;

	if (source == NULL || rom_source_is_gamedrv(drv, source))
		romp = drv->rom;
	else
		romp = (const rom_entry *)device_get_info_ptr((const device_config *)source, DEVINFO_PTR_ROM_REGION);

	return (romp != NULL && !ROMENTRY_ISEND(romp)) ? romp : NULL;
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
	return ROMENTRY_ISEND(romp) ? NULL : romp;
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
	return ROMENTRY_ISREGIONEND(romp) ? NULL : romp;
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
	return ROMENTRY_ISREGIONEND(romp) ? NULL : romp;
}


/*-------------------------------------------------
    rom_region_name - return the appropriate name
    for a rom region
-------------------------------------------------*/

astring *rom_region_name(astring *result, const game_driver *drv, const rom_source *source, const rom_entry *romp)
{
	if (rom_source_is_gamedrv(drv, source))
		astring_cpyc(result, ROMREGION_GETTAG(romp));
	else
	{
		const device_config *device = (const device_config *)source;
		astring_printf(result, "%s:%s", device->tag, ROMREGION_GETTAG(romp));
	}
	return result;
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


/*-------------------------------------------------
    determine_bios_rom - determine system_bios
    from SystemBios structure and OPTION_BIOS
-------------------------------------------------*/

static void determine_bios_rom(rom_load_data *romdata)
{
	const char *specbios = options_get_string(mame_options(), OPTION_BIOS);
	const char *defaultname = NULL;
	const rom_entry *rom;
	int default_no = 1;
	int bios_count = 0;

	romdata->system_bios = 0;

	/* first determine the default BIOS name */
	for (rom = romdata->machine->gamedrv->rom; !ROMENTRY_ISEND(rom); rom++)
		if (ROMENTRY_ISDEFAULT_BIOS(rom))
			defaultname = ROM_GETNAME(rom);

	/* look for a BIOS with a matching name */
	for (rom = romdata->machine->gamedrv->rom; !ROMENTRY_ISEND(rom); rom++)
		if (ROMENTRY_ISSYSTEM_BIOS(rom))
		{
			const char *biosname = ROM_GETNAME(rom);
			int bios_flags = ROM_GETBIOSFLAGS(rom);
			char bios_number[20];

			/* Allow '-bios n' to still be used */
			sprintf(bios_number, "%d", bios_flags - 1);
			if (strcmp(bios_number, specbios) == 0 || strcmp(biosname, specbios) == 0)
				romdata->system_bios = bios_flags;
			if (defaultname != NULL && strcmp(biosname, defaultname) == 0)
				default_no = bios_flags;
			bios_count++;
		}

	/* if none found, use the default */
	if (romdata->system_bios == 0 && bios_count > 0)
	{
		/* if we got neither an empty string nor 'default' then warn the user */
		if (specbios[0] != 0 && strcmp(specbios, "default") != 0 && romdata != NULL)
		{
			astring_catprintf(romdata->errorstring, "%s: invalid bios\n", specbios);
			romdata->warnings++;
		}

		/* set to default */
		romdata->system_bios = default_no;
	}

	LOG(("Using System BIOS: %d\n", romdata->system_bios));
}


/*-------------------------------------------------
    count_roms - counts the total number of ROMs
    that will need to be loaded
-------------------------------------------------*/

static void count_roms(rom_load_data *romdata)
{
	const rom_entry *region, *rom;
	const rom_source *source;

	/* start with 0 */
	romdata->romstotal = 0;
	romdata->romstotalsize = 0;

	/* loop over regions, then over files */
	for (source = rom_first_source(romdata->machine->gamedrv, romdata->machine->config); source != NULL; source = rom_next_source(romdata->machine->gamedrv, romdata->machine->config, source))
		for (region = rom_first_region(romdata->machine->gamedrv, source); region != NULL; region = rom_next_region(region))
			for (rom = rom_first_file(region); rom != NULL; rom = rom_next_file(rom))
				if (ROM_GETBIOSFLAGS(rom) == 0 || ROM_GETBIOSFLAGS(rom) == romdata->system_bios)
				{
					romdata->romstotal++;
					romdata->romstotalsize += rom_file_size(rom);
				}
}


/*-------------------------------------------------
    fill_random - fills an area of memory with
    random data
-------------------------------------------------*/

static void fill_random(running_machine *machine, UINT8 *base, UINT32 length)
{
	while (length--)
		*base++ = mame_rand(machine);
}


/*-------------------------------------------------
    handle_missing_file - handles error generation
    for missing files
-------------------------------------------------*/

static void handle_missing_file(rom_load_data *romdata, const rom_entry *romp)
{
	/* optional files are okay */
	if (ROM_ISOPTIONAL(romp))
	{
		astring_catprintf(romdata->errorstring, "OPTIONAL %s NOT FOUND\n", ROM_GETNAME(romp));
		romdata->warnings++;
	}

	/* no good dumps are okay */
	else if (ROM_NOGOODDUMP(romp))
	{
		astring_catprintf(romdata->errorstring, "%s NOT FOUND (NO GOOD DUMP KNOWN)\n", ROM_GETNAME(romp));
		romdata->warnings++;
	}

	/* anything else is bad */
	else
	{
		astring_catprintf(romdata->errorstring, "%s NOT FOUND\n", ROM_GETNAME(romp));
		romdata->errors++;
	}
}


/*-------------------------------------------------
    dump_wrong_and_correct_checksums - dump an
    error message containing the wrong and the
    correct checksums for a given ROM
-------------------------------------------------*/

static void dump_wrong_and_correct_checksums(rom_load_data *romdata, const char *hash, const char *acthash)
{
	unsigned i;
	char chksum[256];
	unsigned found_functions;
	unsigned wrong_functions;

	found_functions = hash_data_used_functions(hash) & hash_data_used_functions(acthash);

	hash_data_print(hash, found_functions, chksum);
	astring_catprintf(romdata->errorstring, "    EXPECTED: %s\n", chksum);

	/* We dump informations only of the functions for which MAME provided
        a correct checksum. Other functions we might have calculated are
        useless here */
	hash_data_print(acthash, found_functions, chksum);
	astring_catprintf(romdata->errorstring, "       FOUND: %s\n", chksum);

	/* For debugging purposes, we check if the checksums available in the
       driver are correctly specified or not. This can be done by checking
       the return value of one of the extract functions. Maybe we want to
       activate this only in debug buils, but many developers only use
       release builds, so I keep it as is for now. */
	wrong_functions = 0;
	for (i = 0; i < HASH_NUM_FUNCTIONS; i++)
		if (hash_data_extract_printable_checksum(hash, 1 << i, chksum) == 2)
			wrong_functions |= 1 << i;

	if (wrong_functions)
	{
		for (i = 0; i < HASH_NUM_FUNCTIONS; i++)
			if (wrong_functions & (1 << i))
			{
				astring_catprintf(romdata->errorstring,
					"\tInvalid %s checksum treated as 0 (check leading zeros)\n",
					hash_function_name(1 << i));

				romdata->warnings++;
			}
	}
}


/*-------------------------------------------------
    verify_length_and_hash - verify the length
    and hash signatures of a file
-------------------------------------------------*/

static void verify_length_and_hash(rom_load_data *romdata, const char *name, UINT32 explength, const char *hash)
{
	UINT32 actlength;
	const char* acthash;

	/* we've already complained if there is no file */
	if (romdata->file == NULL)
		return;

	/* get the length and CRC from the file */
	actlength = mame_fsize(romdata->file);
	acthash = mame_fhash(romdata->file, hash_data_used_functions(hash));

	/* verify length */
	if (explength != actlength)
	{
		astring_catprintf(romdata->errorstring, "%s WRONG LENGTH (expected: %08x found: %08x)\n", name, explength, actlength);
		romdata->warnings++;
	}

	/* If there is no good dump known, write it */
	if (hash_data_has_info(hash, HASH_INFO_NO_DUMP))
	{
			astring_catprintf(romdata->errorstring, "%s NO GOOD DUMP KNOWN\n", name);
		romdata->warnings++;
	}
	/* verify checksums */
	else if (!hash_data_is_equal(hash, acthash, 0))
	{
		/* otherwise, it's just bad */
		astring_catprintf(romdata->errorstring, "%s WRONG CHECKSUMS:\n", name);

		dump_wrong_and_correct_checksums(romdata, hash, acthash);

		romdata->warnings++;
	}
	/* If it matches, but it is actually a bad dump, write it */
	else if (hash_data_has_info(hash, HASH_INFO_BAD_DUMP))
	{
		astring_catprintf(romdata->errorstring, "%s ROM NEEDS REDUMP\n",name);
		romdata->warnings++;
	}
}


/*-------------------------------------------------
    display_loading_rom_message - display
    messages about ROM loading to the user
-------------------------------------------------*/

static void display_loading_rom_message(rom_load_data *romdata, const char *name)
{
	char buffer[200];

	if (name != NULL)
		sprintf(buffer, "Loading (%d%%)", (UINT32)(100 * (UINT64)romdata->romsloadedsize / (UINT64)romdata->romstotalsize));
	else
		sprintf(buffer, "Loading Complete");

	ui_set_startup_text(romdata->machine, buffer, FALSE);
}


/*-------------------------------------------------
    display_rom_load_results - display the final
    results of ROM loading
-------------------------------------------------*/

static void display_rom_load_results(rom_load_data *romdata)
{
	/* final status display */
	display_loading_rom_message(romdata, NULL);

	/* if we had errors, they are fatal */
	if (romdata->errors != 0)
	{
		const char *rgntag, *nextrgntag;

		/* clean up any regions */
		for (rgntag = memory_region_next(romdata->machine, NULL); rgntag != NULL; rgntag = nextrgntag)
		{
			nextrgntag = memory_region_next(romdata->machine, rgntag);
			memory_region_free(romdata->machine, rgntag);
		}

		/* create the error message and exit fatally */
		mame_printf_error("%s", astring_c(romdata->errorstring));
		astring_free(romdata->errorstring);
		fatalerror_exitcode(romdata->machine, MAMERR_MISSING_FILES, "ERROR: required files are missing, the "GAMENOUN" cannot be run.");
	}

	/* if we had warnings, output them, but continue */
	if (romdata->warnings)
	{
		astring_catc(romdata->errorstring, "WARNING: the "GAMENOUN" might not run correctly.");
		mame_printf_warning("%s\n", astring_c(romdata->errorstring));
	}
}


/*-------------------------------------------------
    region_post_process - post-process a region,
    byte swapping and inverting data as necessary
-------------------------------------------------*/

static void region_post_process(rom_load_data *romdata, const char *rgntag)
{
	UINT32 regionlength = memory_region_length(romdata->machine, rgntag);
	UINT32 regionflags = memory_region_flags(romdata->machine, rgntag);
	UINT8 *regionbase = memory_region(romdata->machine, rgntag);
	int endianness = ((regionflags & ROMREGION_ENDIANMASK) == ROMREGION_LE) ? ENDIANNESS_LITTLE : ENDIANNESS_BIG;
	int datawidth = 1 << ((regionflags & ROMREGION_WIDTHMASK) >> 8);
	UINT8 *base;
	int i, j;

	LOG(("+ datawidth=%d little=%d\n", datawidth, endianness == ENDIANNESS_LITTLE));

	/* if the region is inverted, do that now */
	if (regionflags & ROMREGION_INVERTMASK)
	{
		LOG(("+ Inverting region\n"));
		for (i = 0, base = regionbase; i < regionlength; i++)
			*base++ ^= 0xff;
	}

	/* swap the endianness if we need to */
	if (datawidth > 1 && endianness != ENDIANNESS_NATIVE)
	{
		LOG(("+ Byte swapping region\n"));
		for (i = 0, base = regionbase; i < regionlength; i += datawidth)
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

static int open_rom_file(rom_load_data *romdata, const char *regiontag, const rom_entry *romp)
{
	file_error filerr = FILERR_NOT_FOUND;
	UINT32 romsize = rom_file_size(romp);
	const game_driver *drv;
	int has_crc = FALSE;
	UINT8 crcbytes[4];
	UINT32 crc = 0;

	/* update status display */
	display_loading_rom_message(romdata, ROM_GETNAME(romp));

	/* extract CRC to use for searching */
	has_crc = hash_data_extract_binary_checksum(ROM_GETHASHDATA(romp), HASH_CRC, crcbytes);
	if (has_crc)
		crc = (crcbytes[0] << 24) | (crcbytes[1] << 16) | (crcbytes[2] << 8) | crcbytes[3];

	/* attempt reading up the chain through the parents. It automatically also
       attempts any kind of load by checksum supported by the archives. */
	romdata->file = NULL;
	for (drv = romdata->machine->gamedrv; romdata->file == NULL && drv != NULL; drv = driver_get_clone(drv))
		if (drv->name != NULL && *drv->name != 0)
		{
			astring *fname = astring_assemble_3(astring_alloc(), drv->name, PATH_SEPARATOR, ROM_GETNAME(romp));
			if (has_crc)
				filerr = mame_fopen_crc(SEARCHPATH_ROM, astring_c(fname), crc, OPEN_FLAG_READ, &romdata->file);
			else
				filerr = mame_fopen(SEARCHPATH_ROM, astring_c(fname), OPEN_FLAG_READ, &romdata->file);
			astring_free(fname);
		}

	/* if the region is load by name, load the ROM from there */
	if (romdata->file == NULL && regiontag != NULL)
	{
		astring *fname = astring_assemble_3(astring_alloc(), regiontag, PATH_SEPARATOR, ROM_GETNAME(romp));
		if (has_crc)
			filerr = mame_fopen_crc(SEARCHPATH_ROM, astring_c(fname), crc, OPEN_FLAG_READ, &romdata->file);
		else
			filerr = mame_fopen(SEARCHPATH_ROM, astring_c(fname), OPEN_FLAG_READ, &romdata->file);
		astring_free(fname);
	}

	/* update counters */
	romdata->romsloaded++;
	romdata->romsloadedsize += romsize;

	/* return the result */
	return (filerr == FILERR_NONE);
}


/*-------------------------------------------------
    rom_fread - cheesy fread that fills with
    random data for a NULL file
-------------------------------------------------*/

static int rom_fread(rom_load_data *romdata, UINT8 *buffer, int length)
{
	/* files just pass through */
	if (romdata->file != NULL)
		return mame_fread(romdata->file, buffer, length);

	/* otherwise, fill with randomness */
	else
		fill_random(romdata->machine, buffer, length);

	return length;
}


/*-------------------------------------------------
    read_rom_data - read ROM data for a single
    entry
-------------------------------------------------*/

static int read_rom_data(rom_load_data *romdata, const rom_entry *romp)
{
	int datashift = ROM_GETBITSHIFT(romp);
	int datamask = ((1 << ROM_GETBITWIDTH(romp)) - 1) << datashift;
	int numbytes = ROM_GETLENGTH(romp);
	int groupsize = ROM_GETGROUPSIZE(romp);
	int skip = ROM_GETSKIPCOUNT(romp);
	int reversed = ROM_ISREVERSED(romp);
	int numgroups = (numbytes + groupsize - 1) / groupsize;
	UINT8 *base = romdata->regionbase + ROM_GETOFFSET(romp);
	UINT32 tempbufsize;
	UINT8 *tempbuf;
	int i;

	LOG(("Loading ROM data: offs=%X len=%X mask=%02X group=%d skip=%d reverse=%d\n", ROM_GETOFFSET(romp), numbytes, datamask, groupsize, skip, reversed));

	/* make sure the length was an even multiple of the group size */
	if (numbytes % groupsize != 0)
		fatalerror("Error in RomModule definition: %s length not an even multiple of group size\n", ROM_GETNAME(romp));

	/* make sure we only fill within the region space */
	if (ROM_GETOFFSET(romp) + numgroups * groupsize + (numgroups - 1) * skip > romdata->regionlength)
		fatalerror("Error in RomModule definition: %s out of memory region space\n", ROM_GETNAME(romp));

	/* make sure the length was valid */
	if (numbytes == 0)
		fatalerror("Error in RomModule definition: %s has an invalid length\n", ROM_GETNAME(romp));

	/* special case for simple loads */
	if (datamask == 0xff && (groupsize == 1 || !reversed) && skip == 0)
		return rom_fread(romdata, base, numbytes);

	/* use a temporary buffer for complex loads */
	tempbufsize = MIN(TEMPBUFFER_MAX_SIZE, numbytes);
	tempbuf = alloc_array_or_die(UINT8, tempbufsize);

	/* chunky reads for complex loads */
	skip += groupsize;
	while (numbytes > 0)
	{
		int evengroupcount = (tempbufsize / groupsize) * groupsize;
		int bytesleft = (numbytes > evengroupcount) ? evengroupcount : numbytes;
		UINT8 *bufptr = tempbuf;

		/* read as much as we can */
		LOG(("  Reading %X bytes into buffer\n", bytesleft));
		if (rom_fread(romdata, bufptr, bytesleft) != bytesleft)
		{
			free(tempbuf);
			return 0;
		}
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
	free(tempbuf);

	LOG(("  All done\n"));
	return ROM_GETLENGTH(romp);
}


/*-------------------------------------------------
    fill_rom_data - fill a region of ROM space
-------------------------------------------------*/

static void fill_rom_data(rom_load_data *romdata, const rom_entry *romp)
{
	UINT32 numbytes = ROM_GETLENGTH(romp);
	UINT8 *base = romdata->regionbase + ROM_GETOFFSET(romp);

	/* make sure we fill within the region space */
	if (ROM_GETOFFSET(romp) + numbytes > romdata->regionlength)
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

static void copy_rom_data(rom_load_data *romdata, const rom_entry *romp)
{
	UINT8 *base = romdata->regionbase + ROM_GETOFFSET(romp);
	const char *srcrgntag = ROM_GETNAME(romp);
	UINT32 numbytes = ROM_GETLENGTH(romp);
	UINT32 srcoffs = (FPTR)ROM_GETHASHDATA(romp);  /* srcoffset in place of hashdata */
	UINT8 *srcbase;

	/* make sure we copy within the region space */
	if (ROM_GETOFFSET(romp) + numbytes > romdata->regionlength)
		fatalerror("Error in RomModule definition: COPY out of target memory region space\n");

	/* make sure the length was valid */
	if (numbytes == 0)
		fatalerror("Error in RomModule definition: COPY has an invalid length\n");

	/* make sure the source was valid */
	srcbase = memory_region(romdata->machine, srcrgntag);
	if (srcbase == NULL)
		fatalerror("Error in RomModule definition: COPY from an invalid region\n");

	/* make sure we find within the region space */
	if (srcoffs + numbytes > memory_region_length(romdata->machine, srcrgntag))
		fatalerror("Error in RomModule definition: COPY out of source memory region space\n");

	/* fill the data */
	memcpy(base, srcbase + srcoffs, numbytes);
}


/*-------------------------------------------------
    process_rom_entries - process all ROM entries
    for a region
-------------------------------------------------*/

static void process_rom_entries(rom_load_data *romdata, const char *regiontag, const rom_entry *romp)
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
			fill_rom_data(romdata, romp++);

		/* handle copies */
		else if (ROMENTRY_ISCOPY(romp))
			copy_rom_data(romdata, romp++);

		/* handle files */
		else if (ROMENTRY_ISFILE(romp))
		{
			int irrelevantbios = (ROM_GETBIOSFLAGS(romp) != 0 && ROM_GETBIOSFLAGS(romp) != romdata->system_bios);
			const rom_entry *baserom = romp;
			int explength = 0;

			/* open the file if it is a non-BIOS or matches the current BIOS */
			LOG(("Opening ROM file: %s\n", ROM_GETNAME(romp)));
			if (!irrelevantbios && !open_rom_file(romdata, regiontag, romp))
				handle_missing_file(romdata, romp);

			/* loop until we run out of reloads */
			do
			{
				/* loop until we run out of continues/ignores */
				do
				{
					rom_entry modified_romp = *romp++;
					int readresult;

					/* handle flag inheritance */
					if (!ROM_INHERITSFLAGS(&modified_romp))
						lastflags = modified_romp._flags;
					else
						modified_romp._flags = (modified_romp._flags & ~ROM_INHERITEDFLAGS) | lastflags;

					explength += ROM_GETLENGTH(&modified_romp);

					/* attempt to read using the modified entry */
					if (!ROMENTRY_ISIGNORE(&modified_romp) && !irrelevantbios)
						readresult = read_rom_data(romdata, &modified_romp);
				}
				while (ROMENTRY_ISCONTINUE(romp) || ROMENTRY_ISIGNORE(romp));

				/* if this was the first use of this file, verify the length and CRC */
				if (baserom)
				{
					LOG(("Verifying length (%X) and checksums\n", explength));
					verify_length_and_hash(romdata, ROM_GETNAME(baserom), explength, ROM_GETHASHDATA(baserom));
					LOG(("Verify finished\n"));
				}

				/* reseek to the start and clear the baserom so we don't reverify */
				if (romdata->file != NULL)
					mame_fseek(romdata->file, 0, SEEK_SET);
				baserom = NULL;
				explength = 0;
			}
			while (ROMENTRY_ISRELOAD(romp));

			/* close the file */
			if (romdata->file != NULL)
			{
				LOG(("Closing ROM file\n"));
				mame_fclose(romdata->file);
				romdata->file = NULL;
			}
		}
		else
		{
			romp++;	/* something else; skip */
		}
	}
}


/*-------------------------------------------------
    open_disk_image - open a disk image, searching
    up the parent and loading by checksum
-------------------------------------------------*/

chd_error open_disk_image(const game_driver *gamedrv, const rom_entry *romp, mame_file **image_file, chd_file **image_chd)
{
	return open_disk_image_options(mame_options(), gamedrv, romp, image_file, image_chd);
}


/*-------------------------------------------------
    open_disk_image_options - open a disk image,
    searching up the parent and loading by
    checksum
-------------------------------------------------*/

chd_error open_disk_image_options(core_options *options, const game_driver *gamedrv, const rom_entry *romp, mame_file **image_file, chd_file **image_chd)
{
	const game_driver *drv, *searchdrv;
	const rom_entry *region, *rom;
	const rom_source *source;
	file_error filerr;
	chd_error err;

	*image_file = NULL;
	*image_chd = NULL;

	/* attempt to open the properly named file, scanning up through parent directories */
	filerr = FILERR_NOT_FOUND;
	for (searchdrv = gamedrv; searchdrv != NULL && filerr != FILERR_NONE; searchdrv = driver_get_clone(searchdrv))
	{
		astring *fname = astring_assemble_4(astring_alloc(), searchdrv->name, PATH_SEPARATOR, ROM_GETNAME(romp), ".chd");
		filerr = mame_fopen_options(options, SEARCHPATH_IMAGE, astring_c(fname), OPEN_FLAG_READ, image_file);
		astring_free(fname);
	}

	if (filerr != FILERR_NONE)
	{
		astring *fname = astring_assemble_2(astring_alloc(), ROM_GETNAME(romp), ".chd");
		filerr = mame_fopen_options(options, SEARCHPATH_IMAGE, astring_c(fname), OPEN_FLAG_READ, image_file);
		astring_free(fname);
	}

	/* did the file open succeed? */
	if (filerr == FILERR_NONE)
	{
		/* try to open the CHD */
		err = chd_open_file(mame_core_file(*image_file), CHD_OPEN_READ, NULL, image_chd);
		if (err == CHDERR_NONE)
			return err;

		/* close the file on failure */
		mame_fclose(*image_file);
		*image_file = NULL;
	}
	else
		err = CHDERR_FILE_NOT_FOUND;

	/* otherwise, look at our parents for a CHD with an identical checksum */
	/* and try to open that */
	for (drv = gamedrv; drv != NULL; drv = driver_get_clone(drv))
		for (source = rom_first_source(drv, NULL); source != NULL; source = rom_next_source(drv, NULL, source))
			for (region = rom_first_region(drv, source); region != NULL; region = rom_next_region(region))
				if (ROMREGION_ISDISKDATA(region))
					for (rom = rom_first_file(region); rom != NULL; rom = rom_next_file(rom))

						/* look for a differing name but with the same hash data */
						if (strcmp(ROM_GETNAME(romp), ROM_GETNAME(rom)) != 0 &&
							hash_data_is_equal(ROM_GETHASHDATA(romp), ROM_GETHASHDATA(rom), 0))
						{
							/* attempt to open the properly named file, scanning up through parent directories */
							filerr = FILERR_NOT_FOUND;
							for (searchdrv = drv; searchdrv != NULL && filerr != FILERR_NONE; searchdrv = driver_get_clone(searchdrv))
							{
								astring *fname = astring_assemble_4(astring_alloc(), searchdrv->name, PATH_SEPARATOR, ROM_GETNAME(rom), ".chd");
								filerr = mame_fopen_options(options, SEARCHPATH_IMAGE, astring_c(fname), OPEN_FLAG_READ, image_file);
								astring_free(fname);
							}

							if (filerr != FILERR_NONE)
							{
								astring *fname = astring_assemble_2(astring_alloc(), ROM_GETNAME(rom), ".chd");
								filerr = mame_fopen_options(options, SEARCHPATH_IMAGE, astring_c(fname), OPEN_FLAG_READ, image_file);
								astring_free(fname);
							}

							/* did the file open succeed? */
							if (filerr == FILERR_NONE)
							{
								/* try to open the CHD */
								err = chd_open_file(mame_core_file(*image_file), CHD_OPEN_READ, NULL, image_chd);
								if (err == CHDERR_NONE)
									return err;

								/* close the file on failure */
								mame_fclose(*image_file);
								*image_file = NULL;
							}
						}

	return err;
}


/*-------------------------------------------------
    open_disk_diff - open a DISK diff file
-------------------------------------------------*/

static chd_error open_disk_diff(const game_driver *drv, const rom_entry *romp, chd_file *source, mame_file **diff_file, chd_file **diff_chd)
{
	astring *fname = astring_assemble_2(astring_alloc(), ROM_GETNAME(romp), ".dif");
	file_error filerr;
	chd_error err;

	*diff_file = NULL;
	*diff_chd = NULL;

	/* try to open the diff */
	LOG(("Opening differencing image file: %s\n", astring_c(fname)));
	filerr = mame_fopen(SEARCHPATH_IMAGE_DIFF, astring_c(fname), OPEN_FLAG_READ | OPEN_FLAG_WRITE, diff_file);
	if (filerr != FILERR_NONE)
	{
		/* didn't work; try creating it instead */
		LOG(("Creating differencing image: %s\n", astring_c(fname)));
		filerr = mame_fopen(SEARCHPATH_IMAGE_DIFF, astring_c(fname), OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, diff_file);
		if (filerr != FILERR_NONE)
		{
			err = CHDERR_FILE_NOT_FOUND;
			goto done;
		}

		/* create the CHD */
		err = chd_create_file(mame_core_file(*diff_file), 0, 0, CHDCOMPRESSION_NONE, source);
		if (err != CHDERR_NONE)
			goto done;
	}

	LOG(("Opening differencing image file: %s\n", astring_c(fname)));
	err = chd_open_file(mame_core_file(*diff_file), CHD_OPEN_READWRITE, source, diff_chd);
	if (err != CHDERR_NONE)
		goto done;

done:
	astring_free(fname);
	if ((err != CHDERR_NONE) && (*diff_file != NULL))
	{
		mame_fclose(*diff_file);
		*diff_file = NULL;
	}
	return err;
}


/*-------------------------------------------------
    process_disk_entries - process all disk entries
    for a region
-------------------------------------------------*/

static void process_disk_entries(rom_load_data *romdata, const char *regiontag, const rom_entry *romp)
{
	astring *filename = astring_alloc();

	/* loop until we hit the end of this region */
	for ( ; !ROMENTRY_ISREGIONEND(romp); romp++)
	{
		/* handle files */
		if (ROMENTRY_ISFILE(romp))
		{
			char acthash[HASH_BUF_SIZE];
			open_chd chd = { 0 };
			chd_header header;
			chd_error err;

			/* note the region we are in */
			chd.region = regiontag;

			/* make the filename of the source */
			filename = astring_assemble_2(filename, ROM_GETNAME(romp), ".chd");

			/* first open the source drive */
			LOG(("Opening disk image: %s\n", astring_c(filename)));
			err = open_disk_image(romdata->machine->gamedrv, romp, &chd.origfile, &chd.origchd);
			if (err != CHDERR_NONE)
			{
				if (err == CHDERR_FILE_NOT_FOUND)
					astring_catprintf(romdata->errorstring, "%s NOT FOUND\n", astring_c(filename));
				else
					astring_catprintf(romdata->errorstring, "%s CHD ERROR: %s\n", astring_c(filename), chd_error_string(err));

				/* if this is NO_DUMP, keep going, though the system may not be able to handle it */
				if (hash_data_has_info(ROM_GETHASHDATA(romp), HASH_INFO_NO_DUMP) || DISK_ISOPTIONAL(romp))
					romdata->warnings++;
				else
					romdata->errors++;
				continue;
			}

			/* get the header and extract the MD5/SHA1 */
			header = *chd_get_header(chd.origchd);
			hash_data_clear(acthash);
			hash_data_insert_binary_checksum(acthash, HASH_SHA1, header.sha1);

			/* verify the hash */
			if (!hash_data_is_equal(ROM_GETHASHDATA(romp), acthash, 0))
			{
				astring_catprintf(romdata->errorstring, "%s WRONG CHECKSUMS:\n", astring_c(filename));
				dump_wrong_and_correct_checksums(romdata, ROM_GETHASHDATA(romp), acthash);
				romdata->warnings++;
			}
			else if (hash_data_has_info(ROM_GETHASHDATA(romp), HASH_INFO_BAD_DUMP))
			{
				astring_catprintf(romdata->errorstring, "%s CHD NEEDS REDUMP\n", astring_c(filename));
				romdata->warnings++;
			}

			/* if not read-only, make the diff file */
			if (!DISK_ISREADONLY(romp))
			{
				/* try to open or create the diff */
				err = open_disk_diff(romdata->machine->gamedrv, romp, chd.origchd, &chd.difffile, &chd.diffchd);
				if (err != CHDERR_NONE)
				{
					astring_catprintf(romdata->errorstring, "%s DIFF CHD ERROR: %s\n", astring_c(filename), chd_error_string(err));
					romdata->errors++;
					continue;
				}
			}

			/* we're okay, add to the list of disks */
			LOG(("Assigning to handle %d\n", DISK_GETINDEX(romp)));
			add_disk_handle(romdata->machine, &chd);
		}
	}
	astring_free(filename);
}


/*-------------------------------------------------
    normalize_flags_for_cpu - modify the region
    flags for the given CPU index
-------------------------------------------------*/

static UINT32 normalize_flags_for_cpu(running_machine *machine, UINT32 startflags, const char *rgntag)
{
	const device_config *device = cputag_get_cpu(machine, rgntag);
	if (device != NULL && cpu_get_databus_width(device, ADDRESS_SPACE_PROGRAM) != 0)
	{
		int buswidth;

		/* set the endianness */
		startflags &= ~ROMREGION_ENDIANMASK;
		if (cpu_get_endianness(device) == ENDIANNESS_LITTLE)
			startflags |= ROMREGION_LE;
		else
			startflags |= ROMREGION_BE;

		/* set the width */
		startflags &= ~ROMREGION_WIDTHMASK;
		buswidth = cpu_get_databus_width(device, ADDRESS_SPACE_PROGRAM);
		if (buswidth <= 8)
			startflags |= ROMREGION_8BIT;
		else if (buswidth <= 16)
			startflags |= ROMREGION_16BIT;
		else if (buswidth <= 32)
			startflags |= ROMREGION_32BIT;
		else
			startflags |= ROMREGION_64BIT;
	}
	return startflags;
}


/*-------------------------------------------------
    process_region_list - process a region list
-------------------------------------------------*/

static void process_region_list(rom_load_data *romdata)
{
	astring *regiontag = astring_alloc();
	const rom_source *source;
	const rom_entry *region;

	/* loop until we hit the end */
	for (source = rom_first_source(romdata->machine->gamedrv, romdata->machine->config); source != NULL; source = rom_next_source(romdata->machine->gamedrv, romdata->machine->config, source))
		for (region = rom_first_region(romdata->machine->gamedrv, source); region != NULL; region = rom_next_region(region))
		{
			UINT32 regionlength = ROMREGION_GETLENGTH(region);
			UINT32 regionflags = ROMREGION_GETFLAGS(region);

			rom_region_name(regiontag, romdata->machine->gamedrv, source, region);
			LOG(("Processing region \"%s\" (length=%X)\n", astring_c(regiontag), regionlength));

			/* the first entry must be a region */
			assert(ROMENTRY_ISREGION(region));

			/* if this is a CPU region, override with the CPU width and endianness */
			if (cputag_get_cpu(romdata->machine, astring_c(regiontag)) != NULL)
				regionflags = normalize_flags_for_cpu(romdata->machine, regionflags, astring_c(regiontag));

			/* remember the base and length */
			romdata->regionbase = memory_region_alloc(romdata->machine, astring_c(regiontag), regionlength, regionflags);
			romdata->regionlength = regionlength;
			LOG(("Allocated %X bytes @ %p\n", romdata->regionlength, romdata->regionbase));

			/* clear the region if it's requested */
			if (ROMREGION_ISERASE(region))
				memset(romdata->regionbase, ROMREGION_GETERASEVAL(region), romdata->regionlength);

			/* or if it's sufficiently small (<= 4MB) */
			else if (romdata->regionlength <= 0x400000)
				memset(romdata->regionbase, 0, romdata->regionlength);

#ifdef MAME_DEBUG
			/* if we're debugging, fill region with random data to catch errors */
			else
				fill_random(romdata->machine, romdata->regionbase, romdata->regionlength);
#endif

			/* now process the entries in the region */
			if (ROMREGION_ISROMDATA(region))
				process_rom_entries(romdata, ROMREGION_ISLOADBYNAME(region) ? ROMREGION_GETTAG(region) : NULL, region + 1);
			else if (ROMREGION_ISDISKDATA(region))
				process_disk_entries(romdata, ROMREGION_GETTAG(region), region + 1);
		}

	/* now go back and post-process all the regions */
	for (source = rom_first_source(romdata->machine->gamedrv, romdata->machine->config); source != NULL; source = rom_next_source(romdata->machine->gamedrv, romdata->machine->config, source))
		for (region = rom_first_region(romdata->machine->gamedrv, source); region != NULL; region = rom_next_region(region))
			region_post_process(romdata, ROMREGION_GETTAG(region));

	astring_free(regiontag);
}


/*-------------------------------------------------
    rom_init - load the ROMs and open the disk
    images associated with the given machine
-------------------------------------------------*/

void rom_init(running_machine *machine)
{
	rom_load_data *romdata;

	/* allocate private data */
	machine->romload_data = romdata = auto_alloc_clear(machine, romload_private);

	/* make sure we get called back on the way out */
	add_exit_callback(machine, rom_exit);

	/* reset the romdata struct */
	romdata->machine = machine;
	romdata->errorstring = astring_alloc();

	/* figure out which BIOS we are using */
	determine_bios_rom(romdata);

	/* count the total number of ROMs */
	count_roms(romdata);

	/* reset the disk list */
	romdata->chd_list = NULL;
	romdata->chd_list_tailptr = &machine->romload_data->chd_list;

	/* process the ROM entries we were passed */
	process_region_list(romdata);

	/* display the results and exit */
	display_rom_load_results(romdata);
	astring_free(romdata->errorstring);
}


/*-------------------------------------------------
    rom_exit - clean up after ourselves
-------------------------------------------------*/

static void rom_exit(running_machine *machine)
{
	const char *rgntag, *nextrgntag;
	open_chd *curchd;

	/* free the memory allocated for various regions */
	for (rgntag = memory_region_next(machine, NULL); rgntag != NULL; rgntag = nextrgntag)
	{
		nextrgntag = memory_region_next(machine, rgntag);
		memory_region_free(machine, rgntag);
	}

	/* close all hard drives */
	for (curchd = machine->romload_data->chd_list; curchd != NULL; curchd = curchd->next)
	{
		if (curchd->diffchd != NULL)
			chd_close(curchd->diffchd);
		if (curchd->difffile != NULL)
			mame_fclose(curchd->difffile);
		if (curchd->origchd != NULL)
			chd_close(curchd->origchd);
		if (curchd->origfile != NULL)
			mame_fclose(curchd->origfile);
	}
}


/*-------------------------------------------------
    rom_load_warnings - return the number of
    warnings we generated
-------------------------------------------------*/

int rom_load_warnings(running_machine *machine)
{
	return machine->romload_data->warnings;
}
