/*********************************************************************

    romload.c

    ROM loading functions.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "osdepend.h"
#include "driver.h"
#include "hash.h"
#include "png.h"
#include "harddisk.h"
#include "config.h"
#include "ui.h"
#include <stdarg.h>
#include <ctype.h>


//#define LOG_LOAD



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _open_chd open_chd;
struct _open_chd
{
	open_chd *			next;					/* pointer to next in the list */
	chd_file *			origchd;				/* handle to the original CHD */
	mame_file *			origfile;				/* file handle to the original CHD file */
	chd_file *			diffchd;				/* handle to the diff CHD */
	mame_file *			difffile;				/* file handle to the diff CHD file */
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* disks */
static open_chd *chd_list;
static open_chd **chd_list_tailptr;

/* system BIOS */
int system_bios;

static int total_rom_load_warnings;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void rom_exit(running_machine *machine);



/***************************************************************************
    HARD DISK HANDLING
***************************************************************************/

chd_file *get_disk_handle(int diskindex)
{
	open_chd *curdisk;
	for (curdisk = chd_list; curdisk != NULL && diskindex-- != 0; curdisk = curdisk->next) ;
	if (curdisk != NULL)
		return (curdisk->diffchd != NULL) ? curdisk->diffchd : curdisk->origchd;
	return NULL;
}



/***************************************************************************
    ROM LOADING
***************************************************************************/

/*-------------------------------------------------
    rom_first_region - return pointer to first ROM
    region
-------------------------------------------------*/

const rom_entry *rom_first_region(const game_driver *drv)
{
	return (drv->rom != NULL && !ROMENTRY_ISEND(drv->rom)) ? drv->rom : NULL;
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
    rom_first_chunk - return pointer to first ROM
    chunk
-------------------------------------------------*/

const rom_entry *rom_first_chunk(const rom_entry *romp)
{
	return (ROMENTRY_ISFILE(romp)) ? romp : NULL;
}


/*-------------------------------------------------
    rom_next_chunk - return pointer to next ROM
    chunk
-------------------------------------------------*/

const rom_entry *rom_next_chunk(const rom_entry *romp)
{
	romp++;
	return (ROMENTRY_ISCONTINUE(romp) || ROMENTRY_ISIGNORE(romp)) ? romp : NULL;
}


/*-------------------------------------------------
    debugload - log data to a file
-------------------------------------------------*/

static void CLIB_DECL ATTR_PRINTF(1,2) debugload(const char *string, ...)
{
#ifdef LOG_LOAD
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
#endif
}


/*-------------------------------------------------
    determine_bios_rom - determine system_bios
    from SystemBios structure and OPTION_BIOS
-------------------------------------------------*/

static int determine_bios_rom(const rom_entry *romp)
{
	const char *specbios = options_get_string(mame_options(), OPTION_BIOS);
	const rom_entry *rom;
	int bios_count = 0;

	/* set to default */
	int bios_no = 1;

	for (rom = romp;!ROMENTRY_ISEND(rom);rom++)
		if (ROMENTRY_ISSYSTEM_BIOS(rom))
		{
			const char *biosname = ROM_GETHASHDATA(rom);
			int bios_flags = ROM_GETBIOSFLAGS(rom);
			char bios_number[3];

			/* Allow '-bios n' to still be used */
			sprintf(bios_number, "%d", bios_flags-1);
			if (strcmp(bios_number, specbios) == 0 || strcmp(biosname, specbios) == 0)
				bios_no = bios_flags;
			bios_count++;
		}

	if (bios_count == 0)
		bios_no = 0;

	debugload("Using System BIOS: %d\n", bios_no);

	return bios_no;
}


/*-------------------------------------------------
    count_roms - counts the total number of ROMs
    that will need to be loaded
-------------------------------------------------*/

static int count_roms(const rom_entry *romp)
{
	const rom_entry *region, *rom;
	int count = 0;

	/* loop over regions, then over files */
	for (region = romp; region; region = rom_next_region(region))
		for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
		{
			int bios_flags = ROM_GETBIOSFLAGS(rom);
			if (!bios_flags || (bios_flags == system_bios)) /* alternate bios sets */
				count++;
		}

	/* return the total count */
	return count;
}


/*-------------------------------------------------
    fill_random - fills an area of memory with
    random data
-------------------------------------------------*/

static void fill_random(UINT8 *base, UINT32 length)
{
	while (length--)
		*base++ = mame_rand(Machine);
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
		sprintf(&romdata->errorbuf[strlen(romdata->errorbuf)], "OPTIONAL %s NOT FOUND\n", ROM_GETNAME(romp));
		romdata->warnings++;
	}

	/* no good dumps are okay */
	else if (ROM_NOGOODDUMP(romp))
	{
		sprintf(&romdata->errorbuf[strlen(romdata->errorbuf)], "%s NOT FOUND (NO GOOD DUMP KNOWN)\n", ROM_GETNAME(romp));
		romdata->warnings++;
	}

	/* anything else is bad */
	else
	{
		sprintf(&romdata->errorbuf[strlen(romdata->errorbuf)], "%s NOT FOUND\n", ROM_GETNAME(romp));
		romdata->errors++;
	}
}


/*-------------------------------------------------
    dump_wrong_and_correct_checksums - dump an
    error message containing the wrong and the
    correct checksums for a given ROM
-------------------------------------------------*/

static void dump_wrong_and_correct_checksums(rom_load_data* romdata, const char* hash, const char* acthash)
{
	unsigned i;
	char chksum[256];
	unsigned found_functions;
	unsigned wrong_functions;

	found_functions = hash_data_used_functions(hash) & hash_data_used_functions(acthash);

	hash_data_print(hash, found_functions, chksum);
	sprintf(&romdata->errorbuf[strlen(romdata->errorbuf)], "    EXPECTED: %s\n", chksum);

	/* We dump informations only of the functions for which MAME provided
        a correct checksum. Other functions we might have calculated are
        useless here */
	hash_data_print(acthash, found_functions, chksum);
	sprintf(&romdata->errorbuf[strlen(romdata->errorbuf)], "       FOUND: %s\n", chksum);

	/* For debugging purposes, we check if the checksums available in the
       driver are correctly specified or not. This can be done by checking
       the return value of one of the extract functions. Maybe we want to
       activate this only in debug buils, but many developers only use
       release builds, so I keep it as is for now. */
	wrong_functions = 0;
	for (i=0;i<HASH_NUM_FUNCTIONS;i++)
		if (hash_data_extract_printable_checksum(hash, 1<<i, chksum) == 2)
			wrong_functions |= 1<<i;

	if (wrong_functions)
	{
		for (i=0;i<HASH_NUM_FUNCTIONS;i++)
			if (wrong_functions & (1<<i))
			{
				sprintf(&romdata->errorbuf[strlen(romdata->errorbuf)],
					"\tInvalid %s checksum treated as 0 (check leading zeros)\n",
					hash_function_name(1<<i));

				romdata->warnings++;
			}
	}
}


/*-------------------------------------------------
    verify_length_and_hash - verify the length
    and hash signatures of a file
-------------------------------------------------*/

static void verify_length_and_hash(rom_load_data *romdata, const char *name, UINT32 explength, const char* hash)
{
	UINT32 actlength;
	const char* acthash;

	/* we've already complained if there is no file */
	if (!romdata->file)
		return;

	/* get the length and CRC from the file */
	actlength = mame_fsize(romdata->file);
	acthash = mame_fhash(romdata->file, hash_data_used_functions(hash));

	/* verify length */
	if (explength != actlength)
	{
		sprintf(&romdata->errorbuf[strlen(romdata->errorbuf)], "%s WRONG LENGTH (expected: %08x found: %08x)\n", name, explength, actlength);
		romdata->warnings++;
	}

	/* If there is no good dump known, write it */
	if (hash_data_has_info(hash, HASH_INFO_NO_DUMP))
	{
			sprintf(&romdata->errorbuf[strlen(romdata->errorbuf)], "%s NO GOOD DUMP KNOWN\n", name);
		romdata->warnings++;
	}
	/* verify checksums */
	else if (!hash_data_is_equal(hash, acthash, 0))
	{
		/* otherwise, it's just bad */
		sprintf(&romdata->errorbuf[strlen(romdata->errorbuf)], "%s WRONG CHECKSUMS:\n", name);

		dump_wrong_and_correct_checksums(romdata, hash, acthash);

		romdata->warnings++;
	}
	/* If it matches, but it is actually a bad dump, write it */
	else if (hash_data_has_info(hash, HASH_INFO_BAD_DUMP))
	{
		sprintf(&romdata->errorbuf[strlen(romdata->errorbuf)], "%s ROM NEEDS REDUMP\n",name);
		romdata->warnings++;
	}
}


/*-------------------------------------------------
    display_loading_rom_message - display
    messages about ROM loading to the user
-------------------------------------------------*/

static void display_loading_rom_message(const char *name, rom_load_data *romdata)
{
	char buffer[200];

	if (name != NULL)
		sprintf(buffer, "Loading (%d%%)", 100 * romdata->romsloaded / romdata->romstotal);
	else
		sprintf(buffer, "Loading Complete");

	ui_set_startup_text(buffer, FALSE);
}


/*-------------------------------------------------
    display_rom_load_results - display the final
    results of ROM loading
-------------------------------------------------*/

static void display_rom_load_results(rom_load_data *romdata)
{
	int region;

	/* final status display */
	display_loading_rom_message(NULL, romdata);

	/* if we had errors, they are fatal */
	if (romdata->errors != 0)
	{
		/* clean up any regions */
		for (region = 0; region < MAX_MEMORY_REGIONS; region++)
			free_memory_region(Machine, region);

		/* create the error message and exit fatally */
		strcat(romdata->errorbuf, "ERROR: required files are missing, the game cannot be run.");
		fatalerror_exitcode(MAMERR_MISSING_FILES, "%s", romdata->errorbuf);
	}

	/* if we had warnings, output them, but continue */
	if (romdata->warnings)
	{
		strcat(romdata->errorbuf, "WARNING: the game might not run correctly.");
		mame_printf_warning("%s\n", romdata->errorbuf);
	}
}


/*-------------------------------------------------
    region_post_process - post-process a region,
    byte swapping and inverting data as necessary
-------------------------------------------------*/

static void region_post_process(rom_load_data *romdata, const rom_entry *regiondata)
{
	int type = ROMREGION_GETTYPE(regiondata);
	int datawidth = ROMREGION_GETWIDTH(regiondata) / 8;
	int littleendian = ROMREGION_ISLITTLEENDIAN(regiondata);
	UINT8 *base;
	int i, j;

	debugload("+ datawidth=%d little=%d\n", datawidth, littleendian);

	/* if this is a CPU region, override with the CPU width and endianness */
	if (type >= REGION_CPU1 && type < REGION_CPU1 + MAX_CPU)
	{
		cpu_type cputype = Machine->drv->cpu[type - REGION_CPU1].type;
		if (cputype != CPU_DUMMY)
		{
			datawidth = cputype_databus_width(cputype, ADDRESS_SPACE_PROGRAM) / 8;
			littleendian = (cputype_endianness(cputype) == CPU_IS_LE);
			debugload("+ CPU region #%d: datawidth=%d little=%d\n", type - REGION_CPU1, datawidth, littleendian);
		}
	}

	/* if the region is inverted, do that now */
	if (ROMREGION_ISINVERTED(regiondata))
	{
		debugload("+ Inverting region\n");
		for (i = 0, base = romdata->regionbase; i < romdata->regionlength; i++)
			*base++ ^= 0xff;
	}

	/* swap the endianness if we need to */
#ifdef LSB_FIRST
	if (datawidth > 1 && !littleendian)
#else
	if (datawidth > 1 && littleendian)
#endif
	{
		debugload("+ Byte swapping region\n");
		for (i = 0, base = romdata->regionbase; i < romdata->regionlength; i += datawidth)
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

static int open_rom_file(rom_load_data *romdata, const rom_entry *romp)
{
	file_error filerr = FILERR_NOT_FOUND;
	const game_driver *drv;

	++romdata->romsloaded;

	/* update status display */
	display_loading_rom_message(ROM_GETNAME(romp), romdata);

	/* Attempt reading up the chain through the parents. It automatically also
       attempts any kind of load by checksum supported by the archives. */
	romdata->file = NULL;
	for (drv = Machine->gamedrv; !romdata->file && drv; drv = driver_get_clone(drv))
		if (drv->name && *drv->name)
		{
			UINT8 crcs[4];
			astring *fname;

			fname = astring_assemble_3(astring_alloc(), drv->name, PATH_SEPARATOR, ROM_GETNAME(romp));
			if (hash_data_extract_binary_checksum(ROM_GETHASHDATA(romp), HASH_CRC, crcs))
			{
				UINT32 crc = (crcs[0] << 24) | (crcs[1] << 16) | (crcs[2] << 8) | crcs[3];
				filerr = mame_fopen_crc(SEARCHPATH_ROM, astring_c(fname), crc, OPEN_FLAG_READ, &romdata->file);
			}
			else
				filerr = mame_fopen(SEARCHPATH_ROM, astring_c(fname), OPEN_FLAG_READ, &romdata->file);
			astring_free(fname);
		}

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
	if (romdata->file)
		return mame_fread(romdata->file, buffer, length);

	/* otherwise, fill with randomness */
	else
		fill_random(buffer, length);

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
	int i;

	debugload("Loading ROM data: offs=%X len=%X mask=%02X group=%d skip=%d reverse=%d\n", ROM_GETOFFSET(romp), numbytes, datamask, groupsize, skip, reversed);

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

	/* chunky reads for complex loads */
	skip += groupsize;
	while (numbytes)
	{
		int evengroupcount = (sizeof(romdata->tempbuf) / groupsize) * groupsize;
		int bytesleft = (numbytes > evengroupcount) ? evengroupcount : numbytes;
		UINT8 *bufptr = romdata->tempbuf;

		/* read as much as we can */
		debugload("  Reading %X bytes into buffer\n", bytesleft);
		if (rom_fread(romdata, romdata->tempbuf, bytesleft) != bytesleft)
			return 0;
		numbytes -= bytesleft;

		debugload("  Copying to %p\n", base);

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
	debugload("  All done\n");
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
	int srcregion = ROM_GETFLAGS(romp) >> 24;
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
	srcbase = memory_region(srcregion);
	if (!srcbase)
		fatalerror("Error in RomModule definition: COPY from an invalid region\n");

	/* make sure we find within the region space */
	if (srcoffs + numbytes > memory_region_length(srcregion))
		fatalerror("Error in RomModule definition: COPY out of source memory region space\n");

	/* fill the data */
	memcpy(base, srcbase + srcoffs, numbytes);
}


/*-------------------------------------------------
    process_rom_entries - process all ROM entries
    for a region
-------------------------------------------------*/

static void process_rom_entries(rom_load_data *romdata, const rom_entry *romp)
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
			int bios_flags = ROM_GETBIOSFLAGS(romp);
			if (!bios_flags || (bios_flags == system_bios)) /* alternate bios sets */
			{
				const rom_entry *baserom = romp;
				int explength = 0;

				/* open the file */
				debugload("Opening ROM file: %s\n", ROM_GETNAME(romp));
				if (!open_rom_file(romdata, romp))
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
						if (!ROMENTRY_ISIGNORE(&modified_romp))
							readresult = read_rom_data(romdata, &modified_romp);
					}
					while (ROMENTRY_ISCONTINUE(romp) || ROMENTRY_ISIGNORE(romp));

					/* if this was the first use of this file, verify the length and CRC */
					if (baserom)
					{
						debugload("Verifying length (%X) and checksums\n", explength);
						verify_length_and_hash(romdata, ROM_GETNAME(baserom), explength, ROM_GETHASHDATA(baserom));
						debugload("Verify finished\n");
					}

					/* reseek to the start and clear the baserom so we don't reverify */
					if (romdata->file)
						mame_fseek(romdata->file, 0, SEEK_SET);
					baserom = NULL;
					explength = 0;
				}
				while (ROMENTRY_ISRELOAD(romp));

				/* close the file */
				if (romdata->file)
				{
					debugload("Closing ROM file\n");
					mame_fclose(romdata->file);
					romdata->file = NULL;
				}
			}
			else
			{
				romp++; /* skip over file */
			}
		}
		else
		{
			romp++;	/* something else; skip */
		}
	}
}


/*-------------------------------------------------
    open_disk_image - open a DISK image, searching
    up the parent and loading by checksum
-------------------------------------------------*/

chd_error open_disk_image(const game_driver *gamedrv, const rom_entry *romp, mame_file **image_file, chd_file **image_chd)
{
	return open_disk_image_options(mame_options(), gamedrv, romp, image_file, image_chd);
}


/*-------------------------------------------------
    open_disk_image_options - open a DISK image, searching
    up the parent and loading by checksum
-------------------------------------------------*/

chd_error open_disk_image_options(core_options *options, const game_driver *gamedrv, const rom_entry *romp, mame_file **image_file, chd_file **image_chd)
{
	const game_driver *drv, *searchdrv;
	const rom_entry *region, *rom;
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
		for (region = rom_first_region(drv); region != NULL; region = rom_next_region(region))
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
	debugload("Opening differencing image file: %s\n", astring_c(fname));
	filerr = mame_fopen(SEARCHPATH_IMAGE_DIFF, astring_c(fname), OPEN_FLAG_READ | OPEN_FLAG_WRITE, diff_file);
	if (filerr != FILERR_NONE)
	{
		/* didn't work; try creating it instead */
		debugload("Creating differencing image: %s\n", astring_c(fname));
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

	debugload("Opening differencing image file: %s\n", astring_c(fname));
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

static void process_disk_entries(rom_load_data *romdata, const rom_entry *romp)
{
	/* loop until we hit the end of this region */
	while (!ROMENTRY_ISREGIONEND(romp))
	{
		/* handle files */
		if (ROMENTRY_ISFILE(romp))
		{
			char acthash[HASH_BUF_SIZE];
			open_chd chd = { 0 };
			chd_header header;
			astring *filename;
			chd_error err;

			/* make the filename of the source */
			filename = astring_assemble_2(astring_alloc(), ROM_GETNAME(romp), ".chd");

			/* first open the source drive */
			debugload("Opening disk image: %s\n", astring_c(filename));
			err = open_disk_image(Machine->gamedrv, romp, &chd.origfile, &chd.origchd);
			if (err != CHDERR_NONE)
			{
				if (err == CHDERR_UNSUPPORTED_VERSION)
					sprintf(&romdata->errorbuf[strlen(romdata->errorbuf)], "%s UNSUPPORTED CHD VERSION\n", astring_c(filename));
				else
					sprintf(&romdata->errorbuf[strlen(romdata->errorbuf)], "%s NOT FOUND\n", astring_c(filename));

				/* if this is NO_DUMP, keep going, though the system may not be able to handle it */
				if (hash_data_has_info(ROM_GETHASHDATA(romp), HASH_INFO_NO_DUMP))
					romdata->warnings++;
				else
					romdata->errors++;
				goto next;
			}

			/* get the header and extract the MD5/SHA1 */
			header = *chd_get_header(chd.origchd);
			hash_data_clear(acthash);
			hash_data_insert_binary_checksum(acthash, HASH_MD5, header.md5);
			hash_data_insert_binary_checksum(acthash, HASH_SHA1, header.sha1);

			/* verify the MD5 */
			if (!hash_data_is_equal(ROM_GETHASHDATA(romp), acthash, 0))
			{
				sprintf(&romdata->errorbuf[strlen(romdata->errorbuf)], "%s WRONG CHECKSUMS:\n", astring_c(filename));
				dump_wrong_and_correct_checksums(romdata, ROM_GETHASHDATA(romp), acthash);
				romdata->warnings++;
			}
			else if (hash_data_has_info(ROM_GETHASHDATA(romp), HASH_INFO_BAD_DUMP))
			{
				sprintf(&romdata->errorbuf[strlen(romdata->errorbuf)], "%s CHD NEEDS REDUMP\n", astring_c(filename));
				romdata->warnings++;
			}

			/* if not read-only, make the diff file */
			if (!DISK_ISREADONLY(romp))
			{
				/* try to open or create the diff */
				err = open_disk_diff(Machine->gamedrv, romp, chd.origchd, &chd.difffile, &chd.diffchd);
				if (err != CHDERR_NONE)
				{
					if (err == CHDERR_UNSUPPORTED_VERSION)
						sprintf(&romdata->errorbuf[strlen(romdata->errorbuf)], "%s UNSUPPORTED CHD VERSION\n", astring_c(filename));
					else
						sprintf(&romdata->errorbuf[strlen(romdata->errorbuf)], "%s: CAN'T OPEN DIFF FILE\n", astring_c(filename));
					romdata->errors++;
					goto next;
				}
			}

			/* we're okay, add to the list of disks */
			debugload("Assigning to handle %d\n", DISK_GETINDEX(romp));
			*chd_list_tailptr = auto_malloc(sizeof(**chd_list_tailptr));
			**chd_list_tailptr = chd;
			chd_list_tailptr = &(*chd_list_tailptr)->next;
next:
			romp++;
			astring_free(filename);
		}
	}
}


/*-------------------------------------------------
    rom_init - new, more flexible ROM
    loading system
-------------------------------------------------*/

void rom_init(running_machine *machine, const rom_entry *romp)
{
	const rom_entry *regionlist[REGION_MAX];
	const rom_entry *region;
	static rom_load_data romdata;
	int regnum;

	/* if no roms, bail */
	if (romp == NULL)
		return;

	/* make sure we get called back on the way out */
	add_exit_callback(machine, rom_exit);

	/* reset the region list */
	memset((void *)regionlist, 0, sizeof(regionlist));

	/* reset the romdata struct */
	memset(&romdata, 0, sizeof(romdata));

	/* determine the correct biosset to load based on OPTION_BIOS string */
	system_bios = determine_bios_rom(romp);

	romdata.romstotal = count_roms(romp);

	/* reset the disk list */
	chd_list = NULL;
	chd_list_tailptr = &chd_list;

	/* loop until we hit the end */
	for (region = romp, regnum = 0; region; region = rom_next_region(region), regnum++)
	{
		int regiontype = ROMREGION_GETTYPE(region);

		debugload("Processing region %02X (length=%X)\n", regiontype, ROMREGION_GETLENGTH(region));

		/* the first entry must be a region */
		assert(ROMENTRY_ISREGION(region));

		/* remember the base and length */
		romdata.regionbase = new_memory_region(machine, regiontype, ROMREGION_GETLENGTH(region), ROMREGION_GETFLAGS(region));
		romdata.regionlength = ROMREGION_GETLENGTH(region);
		debugload("Allocated %X bytes @ %p\n", romdata.regionlength, romdata.regionbase);

		/* clear the region if it's requested */
		if (ROMREGION_ISERASE(region))
			memset(romdata.regionbase, ROMREGION_GETERASEVAL(region), romdata.regionlength);

		/* or if it's sufficiently small (<= 4MB) */
		else if (romdata.regionlength <= 0x400000)
			memset(romdata.regionbase, 0, romdata.regionlength);

#ifdef MAME_DEBUG
		/* if we're debugging, fill region with random data to catch errors */
		else
			fill_random(romdata.regionbase, romdata.regionlength);
#endif

		/* now process the entries in the region */
		if (ROMREGION_ISROMDATA(region))
			process_rom_entries(&romdata, region + 1);
		else if (ROMREGION_ISDISKDATA(region))
			process_disk_entries(&romdata, region + 1);

		/* add this region to the list */
		if (regiontype < REGION_MAX)
			regionlist[regiontype] = region;
	}

	/* post-process the regions */
	for (regnum = 0; regnum < REGION_MAX; regnum++)
		if (regionlist[regnum])
		{
			debugload("Post-processing region %02X\n", regnum);
			romdata.regionlength = memory_region_length(regnum);
			romdata.regionbase = memory_region(regnum);
			region_post_process(&romdata, regionlist[regnum]);
		}

	/* display the results and exit */
	total_rom_load_warnings = romdata.warnings;

	display_rom_load_results(&romdata);
}


/*-------------------------------------------------
    rom_exit - clean up after ourselves
-------------------------------------------------*/

static void rom_exit(running_machine *machine)
{
	open_chd *curchd;
	int i;

	/* free the memory allocated for various regions */
	for (i = 0; i < MAX_MEMORY_REGIONS; i++)
		free_memory_region(machine, i);

	/* close all hard drives */
	for (curchd = chd_list; curchd != NULL; curchd = curchd->next)
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


int rom_load_warnings(void)
{
	return total_rom_load_warnings;
}
