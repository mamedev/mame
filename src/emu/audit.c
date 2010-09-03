/***************************************************************************

    audit.c

    ROM set auditing functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "hash.h"
#include "audit.h"
#include "harddisk.h"
#include "sound/samples.h"



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void audit_one_rom(core_options *options, const rom_entry *rom, const char *regiontag, const game_driver *gamedrv, UINT32 validation, audit_record *record);
static void audit_one_disk(core_options *options, const rom_entry *rom, const game_driver *gamedrv, UINT32 validation, audit_record *record);
static int rom_used_by_parent(const game_driver *gamedrv, const rom_entry *romentry, const game_driver **parent);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    set_status - shortcut for setting status and
    substatus values
-------------------------------------------------*/

INLINE void set_status(audit_record *record, UINT8 status, UINT8 substatus)
{
	record->status = status;
	record->substatus = substatus;
}



/***************************************************************************
    CORE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    audit_images - validate the ROM and disk
    images for a game
-------------------------------------------------*/

int audit_images(core_options *options, const game_driver *gamedrv, UINT32 validation, audit_record **audit)
{
	machine_config config(*gamedrv);
	const rom_entry *region, *rom;
	const rom_source *source;
	audit_record *record;
	int anyfound = FALSE;
	int anyrequired = FALSE;
	int allshared = TRUE;
	int records;

	/* determine the number of records we will generate */
	records = 0;
	bool source_is_gamedrv = true;
	for (source = rom_first_source(config); source != NULL; source = rom_next_source(*source))
	{
		for (region = rom_first_region(*source); region != NULL; region = rom_next_region(region))
			for (rom = rom_first_file(region); rom != NULL; rom = rom_next_file(rom))
				if (ROMREGION_ISROMDATA(region) || ROMREGION_ISDISKDATA(region))
				{
					if (source_is_gamedrv && !ROM_ISOPTIONAL(rom) && !ROM_NOGOODDUMP(rom))
					{
						anyrequired = TRUE;

						if (allshared && !rom_used_by_parent(gamedrv, rom, NULL))
							allshared = FALSE;
					}
					records++;
				}

		source_is_gamedrv = false;
	}

	if (records > 0)
	{
		/* allocate memory for the records */
		*audit = global_alloc_array_clear(audit_record, records);
		record = *audit;

		/* iterate over ROM sources and regions */
		bool source_is_gamedrv = true;
		for (source = rom_first_source(config); source != NULL; source = rom_next_source(*source))
		{
			for (region = rom_first_region(*source); region != NULL; region = rom_next_region(region))
			{
				const char *regiontag = ROMREGION_ISLOADBYNAME(region) ? ROM_GETNAME(region) : NULL;
				for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
				{
					/* audit a file */
					if (ROMREGION_ISROMDATA(region))
					{
						audit_one_rom(options, rom, regiontag, gamedrv, validation, record);
					}

					/* audit a disk */
					else if (ROMREGION_ISDISKDATA(region))
					{
						audit_one_disk(options, rom, gamedrv, validation, record);
					}

					else
					{
						continue;
					}

					if (source_is_gamedrv && record->status != AUDIT_STATUS_NOT_FOUND && (allshared || !rom_used_by_parent(gamedrv, rom, NULL)))
						anyfound = TRUE;

					record++;
				}
			}
			source_is_gamedrv = false;
		}
	}

	/* if we found nothing, we don't have the set at all */
	if (!anyfound && anyrequired)
	{
		global_free(*audit);
		*audit = NULL;
		records = 0;
	}

	return records;
}


/*-------------------------------------------------
    audit_samples - validate the samples for a
    game
-------------------------------------------------*/

int audit_samples(core_options *options, const game_driver *gamedrv, audit_record **audit)
{
	machine_config config(*gamedrv);
	audit_record *record;
	int records = 0;
	int sampnum;

	/* count the number of sample records attached to this driver */
	const device_config_sound_interface *sound = NULL;
	for (bool gotone = config.m_devicelist.first(sound); gotone; gotone = sound->next(sound))
		if (sound->devconfig().type() == SAMPLES)
		{
			const samples_interface *intf = (const samples_interface *)sound->devconfig().static_config();

			if (intf->samplenames != NULL)
			{
				/* iterate over samples in this entry */
				for (sampnum = 0; intf->samplenames[sampnum] != NULL; sampnum++)
					if (intf->samplenames[sampnum][0] != '*')
						records++;
			}
		}

	/* if no records, just quit now */
	if (records == 0)
		goto skip;

	/* allocate memory for the records */
	*audit = global_alloc_array_clear(audit_record, records);
	record = *audit;

	/* now iterate over sample entries */
	for (bool gotone = config.m_devicelist.first(sound); gotone; gotone = sound->next(sound))
		if (sound->devconfig().type() == SAMPLES)
		{
			const samples_interface *intf = (const samples_interface *)sound->devconfig().static_config();
			const char *sharedname = NULL;

			if (intf->samplenames != NULL)
			{
				/* iterate over samples in this entry */
				for (sampnum = 0; intf->samplenames[sampnum] != NULL; sampnum++)
					if (intf->samplenames[sampnum][0] == '*')
						sharedname = &intf->samplenames[sampnum][1];
					else
					{
						file_error filerr;
						mame_file *file;

						/* attempt to access the file from the game driver name */
						astring fname(gamedrv->name, PATH_SEPARATOR, intf->samplenames[sampnum]);
						filerr = mame_fopen_options(options, SEARCHPATH_SAMPLE, fname, OPEN_FLAG_READ | OPEN_FLAG_NO_PRELOAD, &file);

						/* attempt to access the file from the shared driver name */
						if (filerr != FILERR_NONE && sharedname != NULL)
						{
							fname.cpy(sharedname).cat(PATH_SEPARATOR).cat(intf->samplenames[sampnum]);
							filerr = mame_fopen_options(options, SEARCHPATH_SAMPLE, fname, OPEN_FLAG_READ | OPEN_FLAG_NO_PRELOAD, &file);
						}

						/* fill in the record */
						record->type = AUDIT_FILE_SAMPLE;
						record->name = intf->samplenames[sampnum];
						if (filerr == FILERR_NONE)
						{
							set_status(record++, AUDIT_STATUS_GOOD, SUBSTATUS_GOOD);
							mame_fclose(file);
						}
						else
							set_status(record++, AUDIT_STATUS_NOT_FOUND, SUBSTATUS_NOT_FOUND);
					}
			}
		}

skip:
	return records;
}


/*-------------------------------------------------
    audit_summary - output a summary given a
    list of audit records
-------------------------------------------------*/

int audit_summary(const game_driver *gamedrv, int count, const audit_record *records, int output)
{
	int overall_status = CORRECT;
	int recnum;

	/* no count AND no records means not found */
	if (count == 0 && records == NULL)
		return NOTFOUND;

	/* loop over records */
	for (recnum = 0; recnum < count; recnum++)
	{
		const audit_record *record = &records[recnum];
		int best_new_status = INCORRECT;

		/* skip anything that's fine */
		if (record->substatus == SUBSTATUS_GOOD)
			continue;

		/* output the game name, file name, and length (if applicable) */
		if (output)
		{
			mame_printf_info("%-8s: %s", gamedrv->name, record->name);
			if (record->explength > 0)
				mame_printf_info(" (%d bytes)", record->explength);
			mame_printf_info(" - ");
		}

		/* use the substatus for finer details */
		switch (record->substatus)
		{
			case SUBSTATUS_GOOD_NEEDS_REDUMP:
				if (output) mame_printf_info("NEEDS REDUMP\n");
				best_new_status = BEST_AVAILABLE;
				break;

			case SUBSTATUS_FOUND_NODUMP:
				if (output) mame_printf_info("NO GOOD DUMP KNOWN\n");
				best_new_status = BEST_AVAILABLE;
				break;

			case SUBSTATUS_FOUND_BAD_CHECKSUM:
				if (output)
				{
					char hashbuf[512];

					mame_printf_info("INCORRECT CHECKSUM:\n");
					hash_data_print(record->exphash, 0, hashbuf);
					mame_printf_info("EXPECTED: %s\n", hashbuf);
					hash_data_print(record->hash, 0, hashbuf);
					mame_printf_info("   FOUND: %s\n", hashbuf);
				}
				break;

			case SUBSTATUS_FOUND_WRONG_LENGTH:
				if (output) mame_printf_info("INCORRECT LENGTH: %d bytes\n", record->length);
				break;

			case SUBSTATUS_NOT_FOUND:
				if (output) mame_printf_info("NOT FOUND\n");
				break;

			case SUBSTATUS_NOT_FOUND_NODUMP:
				if (output) mame_printf_info("NOT FOUND - NO GOOD DUMP KNOWN\n");
				best_new_status = BEST_AVAILABLE;
				break;

			case SUBSTATUS_NOT_FOUND_OPTIONAL:
				if (output) mame_printf_info("NOT FOUND BUT OPTIONAL\n");
				best_new_status = BEST_AVAILABLE;
				break;

			case SUBSTATUS_NOT_FOUND_PARENT:
				if (output) mame_printf_info("NOT FOUND (shared with parent)\n");
				break;

			case SUBSTATUS_NOT_FOUND_BIOS:
				if (output) mame_printf_info("NOT FOUND (BIOS)\n");
				break;
		}

		/* downgrade the overall status if necessary */
		overall_status = MAX(overall_status, best_new_status);
	}

	return overall_status;
}



/***************************************************************************
    UTILITIES
***************************************************************************/

/*-------------------------------------------------
    audit_one_rom - validate a single ROM entry
-------------------------------------------------*/

static void audit_one_rom(core_options *options, const rom_entry *rom, const char *regiontag, const game_driver *gamedrv, UINT32 validation, audit_record *record)
{
	const game_driver *drv;
	UINT32 crc = 0;
	UINT8 crcs[4];
	int has_crc;

	/* fill in the record basics */
	record->type = AUDIT_FILE_ROM;
	record->name = ROM_GETNAME(rom);
	record->exphash = ROM_GETHASHDATA(rom);
	record->length = 0;
	record->explength = rom_file_size(rom);

	/* see if we have a CRC and extract it if so */
	has_crc = hash_data_extract_binary_checksum(record->exphash, HASH_CRC, crcs);
	if (has_crc)
		crc = (crcs[0] << 24) | (crcs[1] << 16) | (crcs[2] << 8) | crcs[3];

	/* find the file and checksum it, getting the file length along the way */
	for (drv = gamedrv; drv != NULL; drv = driver_get_clone(drv))
	{
		file_error filerr;
		mame_file *file;

		/* open the file if we can */
		astring fname(drv->name, PATH_SEPARATOR, ROM_GETNAME(rom));
	    if (has_crc)
			filerr = mame_fopen_crc_options(options, SEARCHPATH_ROM, fname, crc, OPEN_FLAG_READ | OPEN_FLAG_NO_PRELOAD, &file);
		else
			filerr = mame_fopen_options(options, SEARCHPATH_ROM, fname, OPEN_FLAG_READ | OPEN_FLAG_NO_PRELOAD, &file);

		/* if we got it, extract the hash and length */
		if (filerr == FILERR_NONE)
		{
			hash_data_copy(record->hash, mame_fhash(file, validation));
			record->length = (UINT32)mame_fsize(file);
			mame_fclose(file);
			break;
		}
	}

	/* if not found, check the region as a backup */
	if (record->length == 0 && regiontag != NULL)
	{
		file_error filerr;
		mame_file *file;

		/* open the file if we can */
		astring fname(regiontag, PATH_SEPARATOR, ROM_GETNAME(rom));
	    if (has_crc)
			filerr = mame_fopen_crc_options(options, SEARCHPATH_ROM, fname, crc, OPEN_FLAG_READ | OPEN_FLAG_NO_PRELOAD, &file);
		else
			filerr = mame_fopen_options(options, SEARCHPATH_ROM, fname, OPEN_FLAG_READ | OPEN_FLAG_NO_PRELOAD, &file);

		/* if we got it, extract the hash and length */
		if (filerr == FILERR_NONE)
		{
			hash_data_copy(record->hash, mame_fhash(file, validation));
			record->length = (UINT32)mame_fsize(file);
			mame_fclose(file);
		}
	}

	/* if we failed to find the file, set the appropriate status */
	if (record->length == 0)
	{
		const game_driver *parent;

		/* no good dump */
		if (hash_data_has_info(record->exphash, HASH_INFO_NO_DUMP))
			set_status(record, AUDIT_STATUS_NOT_FOUND, SUBSTATUS_NOT_FOUND_NODUMP);

		/* optional ROM */
		else if (ROM_ISOPTIONAL(rom))
			set_status(record, AUDIT_STATUS_NOT_FOUND, SUBSTATUS_NOT_FOUND_OPTIONAL);

		/* not found and used by parent */
		else if (rom_used_by_parent(gamedrv, rom, &parent))
			set_status(record, AUDIT_STATUS_NOT_FOUND, (parent->flags & GAME_IS_BIOS_ROOT) ? SUBSTATUS_NOT_FOUND_BIOS : SUBSTATUS_NOT_FOUND_PARENT);

		/* just plain old not found */
		else
			set_status(record, AUDIT_STATUS_NOT_FOUND, SUBSTATUS_NOT_FOUND);
	}

	/* if we did find the file, do additional verification */
	else
	{
		/* length mismatch */
		if (record->explength != record->length)
			set_status(record, AUDIT_STATUS_FOUND_INVALID, SUBSTATUS_FOUND_WRONG_LENGTH);

		/* found but needs a dump */
		else if (hash_data_has_info(record->exphash, HASH_INFO_NO_DUMP))
			set_status(record, AUDIT_STATUS_GOOD, SUBSTATUS_FOUND_NODUMP);

		/* incorrect hash */
		else if (!hash_data_is_equal(record->exphash, record->hash, 0))
			set_status(record, AUDIT_STATUS_FOUND_INVALID, SUBSTATUS_FOUND_BAD_CHECKSUM);

		/* correct hash but needs a redump */
		else if (hash_data_has_info(record->exphash, HASH_INFO_BAD_DUMP))
			set_status(record, AUDIT_STATUS_GOOD, SUBSTATUS_GOOD_NEEDS_REDUMP);

		/* just plain old good */
		else
			set_status(record, AUDIT_STATUS_GOOD, SUBSTATUS_GOOD);
	}
}


/*-------------------------------------------------
    audit_one_disk - validate a single disk entry
-------------------------------------------------*/

static void audit_one_disk(core_options *options, const rom_entry *rom, const game_driver *gamedrv, UINT32 validation, audit_record *record)
{
	mame_file *source_file;
	chd_file *source;
	chd_error err;

	/* fill in the record basics */
	record->type = AUDIT_FILE_DISK;
	record->name = ROM_GETNAME(rom);
	record->exphash = ROM_GETHASHDATA(rom);

	/* open the disk */
	err = open_disk_image_options(options, gamedrv, rom, &source_file, &source);

	/* if we failed, report the error */
	if (err != CHDERR_NONE)
	{
		/* out of memory */
		if (err == CHDERR_OUT_OF_MEMORY)
			set_status(record, AUDIT_STATUS_ERROR, SUBSTATUS_ERROR);

		/* not found but it's not good anyway */
		else if (hash_data_has_info(record->exphash, HASH_INFO_NO_DUMP))
			set_status(record, AUDIT_STATUS_NOT_FOUND, SUBSTATUS_NOT_FOUND_NODUMP);

		/* not found but optional */
		else if (DISK_ISOPTIONAL(rom))
			set_status(record, AUDIT_STATUS_NOT_FOUND, SUBSTATUS_NOT_FOUND_OPTIONAL);

		/* not found at all */
		else
			set_status(record, AUDIT_STATUS_NOT_FOUND, SUBSTATUS_NOT_FOUND);
	}

	/* if we succeeded, validate it */
	else
	{
		static const UINT8 nullhash[HASH_BUF_SIZE] = { 0 };
		chd_header header = *chd_get_header(source);

		/* if there's an MD5 or SHA1 hash, add them to the output hash */
		if (memcmp(nullhash, header.md5, sizeof(header.md5)) != 0)
			hash_data_insert_binary_checksum(record->hash, HASH_MD5, header.md5);
		if (memcmp(nullhash, header.sha1, sizeof(header.sha1)) != 0)
			hash_data_insert_binary_checksum(record->hash, HASH_SHA1, header.sha1);

		/* found but needs a dump */
		if (hash_data_has_info(record->exphash, HASH_INFO_NO_DUMP))
			set_status(record, AUDIT_STATUS_GOOD, SUBSTATUS_FOUND_NODUMP);

		/* incorrect hash */
		else if (!hash_data_is_equal(record->exphash, record->hash, 0))
			set_status(record, AUDIT_STATUS_FOUND_INVALID, SUBSTATUS_FOUND_BAD_CHECKSUM);

		/* correct hash but needs a redump */
		else if (hash_data_has_info(record->exphash, HASH_INFO_BAD_DUMP))
			set_status(record, AUDIT_STATUS_GOOD, SUBSTATUS_GOOD_NEEDS_REDUMP);

		/* just plain good */
		else
			set_status(record, AUDIT_STATUS_GOOD, SUBSTATUS_GOOD);

		chd_close(source);
		mame_fclose(source_file);
	}
}


/*-------------------------------------------------
    rom_used_by_parent - determine if a given
    ROM is also used by the parent
-------------------------------------------------*/

static int rom_used_by_parent(const game_driver *gamedrv, const rom_entry *romentry, const game_driver **parent)
{
	const char *hash = ROM_GETHASHDATA(romentry);
	const game_driver *drv;

	/* iterate up the parent chain */
	for (drv = driver_get_clone(gamedrv); drv != NULL; drv = driver_get_clone(drv))
	{
		machine_config config(*drv);
		const rom_entry *region;
		const rom_entry *rom;

		/* see if the parent has the same ROM or not */
		for (const rom_source *source = rom_first_source(config); source != NULL; source = rom_next_source(*source))
			for (region = rom_first_region(*source); region; region = rom_next_region(region))
				for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
					if (hash_data_is_equal(ROM_GETHASHDATA(rom), hash, 0))
					{
						if (parent != NULL)
							*parent = drv;
						return TRUE;
					}
	}

	return FALSE;
}
