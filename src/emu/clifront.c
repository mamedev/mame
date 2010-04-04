/***************************************************************************

    clifront.c

    Command-line interface frontend for MAME.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "hash.h"
#include "jedparse.h"
#include "audit.h"
#include "info.h"
#include "unzip.h"
#include "validity.h"
#include "sound/samples.h"
#include "clifront.h"

#include <new>
#include <ctype.h>

#ifdef MESS
#include "climess.h"
#endif /* MESS */



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _romident_status romident_status;
struct _romident_status
{
	int			total;				/* total files processed */
	int			matches;			/* number of matches found */
	int			nonroms;			/* number of non-ROM files found */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static int execute_simple_commands(core_options *options, const char *exename);
static int execute_commands(core_options *options, const char *exename, const game_driver *driver);
static void display_help(void);

/* informational functions */
static int info_verifyroms(core_options *options, const char *gamename);
static int info_verifysamples(core_options *options, const char *gamename);
static int info_romident(core_options *options, const char *gamename);

/* utilities */
static void romident(core_options *options, const char *filename, romident_status *status);
static void identify_file(core_options *options, const char *name, romident_status *status);
static void identify_data(core_options *options, const char *name, const UINT8 *data, int length, romident_status *status);
static void match_roms(core_options *options, const char *hash, int length, int *found);



/***************************************************************************
    COMMAND-LINE OPTIONS
***************************************************************************/

static const options_entry cli_options[] =
{
	/* core commands */
	{ NULL,                       NULL,       OPTION_HEADER,     "CORE COMMANDS" },
	{ "help;h;?",                 "0",        OPTION_COMMAND,    "show help message" },
	{ "validate;valid",           "0",        OPTION_COMMAND,    "perform driver validation on all game drivers" },

	/* configuration commands */
	{ NULL,                       NULL,       OPTION_HEADER,     "CONFIGURATION COMMANDS" },
	{ "createconfig;cc",          "0",        OPTION_COMMAND,    "create the default configuration file" },
	{ "showconfig;sc",            "0",        OPTION_COMMAND,    "display running parameters" },
	{ "showusage;su",             "0",        OPTION_COMMAND,    "show this help" },

	/* frontend commands */
	{ NULL,                       NULL,       OPTION_HEADER,     "FRONTEND COMMANDS" },
	{ "listxml;lx",               "0",        OPTION_COMMAND,    "all available info on driver in XML format" },
	{ "listfull;ll",              "0",        OPTION_COMMAND,    "short name, full name" },
	{ "listsource;ls",            "0",        OPTION_COMMAND,    "driver sourcefile" },
	{ "listclones;lc",            "0",        OPTION_COMMAND,    "show clones" },
	{ "listbrothers;lb",          "0",        OPTION_COMMAND,    "show \"brothers\", or other drivers from same sourcefile" },
	{ "listcrc",                  "0",        OPTION_COMMAND,    "CRC-32s" },
	{ "listroms",                 "0",        OPTION_COMMAND,    "list required roms for a driver" },
	{ "listsamples",              "0",        OPTION_COMMAND,    "list optional samples for a driver" },
	{ "verifyroms",               "0",        OPTION_COMMAND,    "report romsets that have problems" },
	{ "verifysamples",            "0",        OPTION_COMMAND,    "report samplesets that have problems" },
	{ "romident",                 "0",        OPTION_COMMAND,    "compare files with known MAME roms" },
	{ "listdevices;ld",           "0",        OPTION_COMMAND,    "list available devices" },
#ifdef MESS
	{ "listmedia;lm",             "0",        OPTION_COMMAND,    "list available media for the system" },
	{ "listsoftware",             "0",        OPTION_COMMAND,    "list known software for the system" },
#endif

	{ NULL }
};



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    cli_execute - execute a game via the standard
    command line interface
-------------------------------------------------*/

int cli_execute(int argc, char **argv, const options_entry *osd_options)
{
	core_options *options = NULL;
	const char *gamename_option;
	const game_driver *driver;
	int result = MAMERR_FATALERROR;
	astring gamename;
	astring exename;

	try
	{
		/* initialize the options manager and add the CLI-specific options */
		options = mame_options_init(osd_options);
		options_add_entries(options, cli_options);

		/* parse the command line first; if we fail here, we're screwed */
		if (options_parse_command_line(options, argc, argv, OPTION_PRIORITY_CMDLINE))
		{
			result = MAMERR_INVALID_CONFIG;
			goto error;
		}

		/* parse the simple commmands before we go any further */
		core_filename_extract_base(&exename, argv[0], TRUE);
		result = execute_simple_commands(options, exename);
		if (result != -1)
			goto error;

		/* find out what game we might be referring to */
		gamename_option = options_get_string(options, OPTION_GAMENAME);
		core_filename_extract_base(&gamename, gamename_option, TRUE);
		driver = driver_get_name(gamename);

		/* execute any commands specified */
		result = execute_commands(options, exename, driver);
		if (result != -1)
			goto error;

		/* if we don't have a valid driver selected, offer some suggestions */
		if (strlen(gamename_option) > 0 && driver == NULL)
		{
			const game_driver *matches[10];
			int drvnum;

			/* get the top 10 approximate matches */
			driver_list_get_approx_matches(drivers, gamename_option, ARRAY_LENGTH(matches), matches);

			/* print them out */
			fprintf(stderr, "\n\"%s\" approximately matches the following\n"
					"supported " GAMESNOUN " (best match first):\n\n", gamename_option);
			for (drvnum = 0; drvnum < ARRAY_LENGTH(matches); drvnum++)
				if (matches[drvnum] != NULL)
					fprintf(stderr, "%-18s%s\n", matches[drvnum]->name, matches[drvnum]->description);

			/* exit with an error */
			result = MAMERR_NO_SUCH_GAME;
			goto error;
		}

		/* run the game */
		result = mame_execute(options);
	}
	catch (emu_fatalerror &fatal)
	{
		fprintf(stderr, "%s\n", fatal.string());
		if (fatal.exitcode() != 0)
			result = fatal.exitcode();
	}
	catch (emu_exception &)
	{
		fprintf(stderr, "Caught unhandled emulator exception\n");
	}
	catch (std::bad_alloc &)
	{
		fprintf(stderr, "Out of memory!\n");
	}
	catch (...)
	{
		fprintf(stderr, "Caught unhandled exception\n");
	}

error:
	/* free our options and exit */
	if (options != NULL)
		options_free(options);

	/* report any unfreed memory */
	dump_unfreed_mem();
	return result;
}


/*-------------------------------------------------
    help_output - output callback for printing
    requested help information
-------------------------------------------------*/

static void help_output(const char *s)
{
	mame_printf_info("%s", s);
}


/*-------------------------------------------------
    execute_simple_commands - execute basic
    commands that don't require any context
-------------------------------------------------*/

static int execute_simple_commands(core_options *options, const char *exename)
{
	/* help? */
	if (options_get_bool(options, CLIOPTION_HELP))
	{
		display_help();
		return MAMERR_NONE;
	}

	/* showusage? */
	if (options_get_bool(options, CLIOPTION_SHOWUSAGE))
	{
		mame_printf_info("Usage: %s [%s] [options]\n\nOptions:\n", exename, GAMENOUN);
		options_output_help(options, help_output);
		return MAMERR_NONE;
	}

	/* validate? */
	if (options_get_bool(options, CLIOPTION_VALIDATE))
	{
		return mame_validitychecks(NULL);
	}

	return -1;
}


/*-------------------------------------------------
    execute_commands - execute various frontend
    commands
-------------------------------------------------*/

static int execute_commands(core_options *options, const char *exename, const game_driver *driver)
{
	static const struct
	{
		const char *option;
		int (*function)(core_options *options, const char *gamename);
	} info_commands[] =
	{
		{ CLIOPTION_LISTXML,		cli_info_listxml },
		{ CLIOPTION_LISTFULL,		cli_info_listfull },
		{ CLIOPTION_LISTSOURCE,		cli_info_listsource },
		{ CLIOPTION_LISTCLONES,		cli_info_listclones },
		{ CLIOPTION_LISTBROTHERS,	cli_info_listbrothers },
		{ CLIOPTION_LISTCRC,		cli_info_listcrc },
		{ CLIOPTION_LISTDEVICES,	cli_info_listdevices },
		{ CLIOPTION_LISTROMS,		cli_info_listroms },
		{ CLIOPTION_LISTSAMPLES,	cli_info_listsamples },
		{ CLIOPTION_VERIFYROMS,		info_verifyroms },
		{ CLIOPTION_VERIFYSAMPLES,	info_verifysamples },
#ifdef MESS
		{ CLIOPTION_LISTMEDIA,		info_listmedia },
		{ CLIOPTION_LISTSOFTWARE,	info_listsoftware },
#endif
		{ CLIOPTION_ROMIDENT,		info_romident }
	};
	int i;

	/* createconfig? */
	if (options_get_bool(options, CLIOPTION_CREATECONFIG))
	{
		file_error filerr;
		mame_file *file;

		/* parse any relevant INI files before proceeding */
		mame_parse_ini_files(options, driver);

		/* make the output filename */
		filerr = mame_fopen_options(options, NULL, CONFIGNAME ".ini", OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &file);

		/* error if unable to create the file */
		if (filerr != FILERR_NONE)
		{
			fprintf(stderr, "Unable to create file " CONFIGNAME ".ini\n");
			return MAMERR_FATALERROR;
		}

		/* output the configuration and exit cleanly */
		options_output_ini_file(options, mame_core_file(file));
		mame_fclose(file);
		return MAMERR_NONE;
	}

	/* showconfig? */
	if (options_get_bool(options, CLIOPTION_SHOWCONFIG))
	{
		/* parse any relevant INI files before proceeding */
		mame_parse_ini_files(options, driver);
		options_output_ini_stdfile(options, stdout);
		return MAMERR_NONE;
	}

	/* informational commands? */
	for (i = 0; i < ARRAY_LENGTH(info_commands); i++)
		if (options_get_bool(options, info_commands[i].option))
		{
			const char *gamename = options_get_string(options, OPTION_GAMENAME);

			/* parse any relevant INI files before proceeding */
			mame_parse_ini_files(options, driver);
			return (*info_commands[i].function)(options, (gamename[0] == 0) ? "*" : gamename);
		}

	return -1;
}


/*-------------------------------------------------
    display_help - display help to standard
    output
-------------------------------------------------*/

static void display_help(void)
{
#ifndef MESS
	mame_printf_info("M.A.M.E. v%s - Multiple Arcade Machine Emulator\n"
		   "Copyright Nicola Salmoria and the MAME Team\n\n", build_version);
	mame_printf_info("%s\n", mame_disclaimer);
	mame_printf_info("Usage:  MAME gamename [options]\n\n"
		   "        MAME -showusage    for a brief list of options\n"
		   "        MAME -showconfig   for a list of configuration options\n"
		   "        MAME -createconfig to create a " CONFIGNAME ".ini\n\n"
		   "For usage instructions, please consult the file windows.txt\n");
#else
	mess_display_help();
#endif
}



/***************************************************************************
    INFORMATIONAL FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    cli_info_listxml - output the XML data for one
    or more games
-------------------------------------------------*/

int cli_info_listxml(core_options *options, const char *gamename)
{
	print_mame_xml(stdout, drivers, gamename);
	return MAMERR_NONE;
}


/*-------------------------------------------------
    cli_info_listfull - output the name and description
    of one or more games
-------------------------------------------------*/

int cli_info_listfull(core_options *options, const char *gamename)
{
	int drvindex, count = 0;

	/* iterate over drivers */
	for (drvindex = 0; drivers[drvindex] != NULL; drvindex++)
		if ((drivers[drvindex]->flags & GAME_NO_STANDALONE) == 0 && mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			/* print the header on the first one */
			if (count == 0)
				mame_printf_info("Name:             Description:\n");

			/* output the remaining information */
			mame_printf_info("%-18s\"%s\"\n", drivers[drvindex]->name, drivers[drvindex]->description);
			count++;
		}

	/* return an error if none found */
	return (count > 0) ? MAMERR_NONE : MAMERR_NO_SUCH_GAME;
}


/*-------------------------------------------------
    cli_info_listsource - output the name and source
    filename of one or more games
-------------------------------------------------*/

int cli_info_listsource(core_options *options, const char *gamename)
{
	int drvindex, count = 0;
	astring filename;

	/* iterate over drivers */
	for (drvindex = 0; drivers[drvindex] != NULL; drvindex++)
		if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			/* output the remaining information */
			mame_printf_info("%-16s %s\n", drivers[drvindex]->name, core_filename_extract_base(&filename, drivers[drvindex]->source_file, FALSE)->cstr());
			count++;
		}

	/* return an error if none found */
	return (count > 0) ? MAMERR_NONE : MAMERR_NO_SUCH_GAME;
}


/*-------------------------------------------------
    cli_info_listclones - output the name and source
    filename of one or more games
-------------------------------------------------*/

int cli_info_listclones(core_options *options, const char *gamename)
{
	int drvindex, count = 0;

	/* iterate over drivers */
	for (drvindex = 0; drivers[drvindex] != NULL; drvindex++)
	{
		const game_driver *clone_of = driver_get_clone(drivers[drvindex]);

		/* if we are a clone, and either our name matches the gamename, or the clone's name matches, display us */
		if (clone_of != NULL && (clone_of->flags & GAME_IS_BIOS_ROOT) == 0)
			if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0 || mame_strwildcmp(gamename, clone_of->name) == 0)
			{
				/* print the header on the first one */
				if (count == 0)
					mame_printf_info("Name:            Clone of:\n");

				/* output the remaining information */
				mame_printf_info("%-16s %-8s\n", drivers[drvindex]->name, clone_of->name);
				count++;
			}
	}

	/* return an error if none found */
	return (count > 0) ? MAMERR_NONE : MAMERR_NO_SUCH_GAME;
}


/*-------------------------------------------------
    cli_info_listbrothers - output the name and
    source filename of one or more games
-------------------------------------------------*/

int cli_info_listbrothers(core_options *options, const char *gamename)
{
	UINT8 *didit = global_alloc_array_clear(UINT8, driver_list_get_count(drivers));
	int drvindex, count = 0;
	astring filename;

	/* iterate over drivers */
	for (drvindex = 0; drivers[drvindex] != NULL; drvindex++)
		if (!didit[drvindex] && mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			int matchindex;

			didit[drvindex] = TRUE;
			if (count > 0)
				mame_printf_info("\n");
			mame_printf_info("%s ... other drivers in %s:\n", drivers[drvindex]->name, core_filename_extract_base(&filename, drivers[drvindex]->source_file, FALSE)->cstr());

			/* now iterate again over drivers, finding those with the same source file */
			for (matchindex = 0; drivers[matchindex]; matchindex++)
				if (matchindex != drvindex && strcmp(drivers[drvindex]->source_file, drivers[matchindex]->source_file) == 0)
				{
					const char *matchstring = (mame_strwildcmp(gamename, drivers[matchindex]->name) == 0) ? "-> " : "   ";
					const game_driver *clone_of = driver_get_clone(drivers[matchindex]);

					if (clone_of != NULL && (clone_of->flags & GAME_IS_BIOS_ROOT) == 0)
						mame_printf_info("%s%-16s [%s]\n", matchstring, drivers[matchindex]->name, clone_of->name);
					else
						mame_printf_info("%s%s\n", matchstring, drivers[matchindex]->name);
					didit[matchindex] = TRUE;
				}

			count++;
		}

	/* return an error if none found */
	global_free(didit);
	return (count > 0) ? MAMERR_NONE : MAMERR_NO_SUCH_GAME;
}


/*-------------------------------------------------
    cli_info_listcrc - output the CRC and name of
    all ROMs referenced by MAME
-------------------------------------------------*/

int cli_info_listcrc(core_options *options, const char *gamename)
{
	int drvindex, count = 0;

	/* iterate over drivers */
	for (drvindex = 0; drivers[drvindex] != NULL; drvindex++)
		if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			machine_config *config = machine_config_alloc(drivers[drvindex]->machine_config);
			const rom_entry *region, *rom;
			const rom_source *source;

			/* iterate over sources, regions, and then ROMs within the region */
			for (source = rom_first_source(drivers[drvindex], config); source != NULL; source = rom_next_source(drivers[drvindex], config, source))
				for (region = rom_first_region(drivers[drvindex], source); region; region = rom_next_region(region))
					for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
					{
						char hashbuf[HASH_BUF_SIZE];

						/* if we have a CRC, display it */
						if (hash_data_extract_printable_checksum(ROM_GETHASHDATA(rom), HASH_CRC, hashbuf))
							mame_printf_info("%s %-12s %s\n", hashbuf, ROM_GETNAME(rom), drivers[drvindex]->description);
					}

			count++;
			machine_config_free(config);
		}

	/* return an error if none found */
	return (count > 0) ? MAMERR_NONE : MAMERR_NO_SUCH_GAME;
}


/*-------------------------------------------------
    cli_info_listroms - output the list of ROMs
    referenced by a given game or set of games
-------------------------------------------------*/

int cli_info_listroms(core_options *options, const char *gamename)
{
	int drvindex, count = 0;

	/* iterate over drivers */
	for (drvindex = 0; drivers[drvindex] != NULL; drvindex++)
		if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			machine_config *config = machine_config_alloc(drivers[drvindex]->machine_config);
			const rom_entry *region, *rom;
			const rom_source *source;

			/* print the header */
			if (count > 0)
				mame_printf_info("\n");
			mame_printf_info("This is the list of the ROMs required for driver \"%s\".\n"
					"Name            Size Checksum\n", drivers[drvindex]->name);

			/* iterate over sources, regions and then ROMs within the region */
			for (source = rom_first_source(drivers[drvindex], config); source != NULL; source = rom_next_source(drivers[drvindex], config, source))
				for (region = rom_first_region(drivers[drvindex], source); region != NULL; region = rom_next_region(region))
					for (rom = rom_first_file(region); rom != NULL; rom = rom_next_file(rom))
					{
						const char *name = ROM_GETNAME(rom);
						const char *hash = ROM_GETHASHDATA(rom);
						char hashbuf[HASH_BUF_SIZE];
						int length = -1;

						/* accumulate the total length of all chunks */
						if (ROMREGION_ISROMDATA(region))
							length = rom_file_size(rom);

						/* start with the name */
						mame_printf_info("%-12s ", name);

						/* output the length next */
						if (length >= 0)
							mame_printf_info("%7d", length);
						else
							mame_printf_info("       ");

						/* output the hash data */
						if (!hash_data_has_info(hash, HASH_INFO_NO_DUMP))
						{
							if (hash_data_has_info(hash, HASH_INFO_BAD_DUMP))
								mame_printf_info(" BAD");

							hash_data_print(hash, 0, hashbuf);
							mame_printf_info(" %s", hashbuf);
						}
						else
							mame_printf_info(" NO GOOD DUMP KNOWN");

						/* end with a CR */
						mame_printf_info("\n");
					}

			count++;
			machine_config_free(config);
		}

	return (count > 0) ? MAMERR_NONE : MAMERR_NO_SUCH_GAME;
}


/*-------------------------------------------------
    cli_info_listsamples - output the list of samples
    referenced by a given game or set of games
-------------------------------------------------*/

int cli_info_listsamples(core_options *options, const char *gamename)
{
	int count = 0;
	int drvindex;

	/* iterate over drivers */
	for (drvindex = 0; drivers[drvindex] != NULL; drvindex++)
		if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			machine_config *config = machine_config_alloc(drivers[drvindex]->machine_config);
			const device_config *devconfig;

			/* find samples interfaces */
			for (devconfig = sound_first(config); devconfig != NULL; devconfig = sound_next(devconfig))
				if (sound_get_type(devconfig) == SOUND_SAMPLES)
				{
					const char *const *samplenames = ((const samples_interface *)devconfig->static_config)->samplenames;
					int sampnum;

					/* if the list is legit, walk it and print the sample info */
					if (samplenames != NULL)
						for (sampnum = 0; samplenames[sampnum] != NULL; sampnum++)
							mame_printf_info("%s\n", samplenames[sampnum]);
				}

			count++;
			machine_config_free(config);
		}

	return (count > 0) ? MAMERR_NONE : MAMERR_NO_SUCH_GAME;
}


/*-------------------------------------------------
    cli_info_listdevices - output the list of
    devices referenced by a given game or set of
    games
-------------------------------------------------*/

int cli_info_listdevices(core_options *options, const char *gamename)
{
	int count = 0;
	int drvindex;

	/* iterate over drivers */
	for (drvindex = 0; drivers[drvindex] != NULL; drvindex++)
		if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			machine_config *config = machine_config_alloc(drivers[drvindex]->machine_config);
			const device_config *devconfig;

			if (count != 0)
				printf("\n");
			printf("Driver %s (%s):\n", drivers[drvindex]->name, drivers[drvindex]->description);

			/* iterate through devices */
			for (devconfig = config->devicelist.first(); devconfig != NULL; devconfig = devconfig->next)
			{
				switch (devconfig->devclass)
				{
					case DEVICE_CLASS_AUDIO:            printf("  Audio: ");	break;
					case DEVICE_CLASS_VIDEO:            printf("  Video: ");	break;
					case DEVICE_CLASS_CPU_CHIP:         printf("  CPU:   ");	break;
					case DEVICE_CLASS_SOUND_CHIP:       printf("  Sound: ");	break;
					case DEVICE_CLASS_TIMER:            printf("  Timer: ");	break;
					default:                            printf("  Other: ");	break;
				}
				printf("%s ('%s')", devconfig->name(), devconfig->tag());
				if (devconfig->clock >= 1000000000)
					printf(" @ %d.%02d GHz\n", devconfig->clock / 1000000000, (devconfig->clock / 10000000) % 100);
				else if (devconfig->clock >= 1000000)
					printf(" @ %d.%02d MHz\n", devconfig->clock / 1000000, (devconfig->clock / 10000) % 100);
				else if (devconfig->clock >= 1000)
					printf(" @ %d.%02d kHz\n", devconfig->clock / 1000, (devconfig->clock / 10) % 100);
				else if (devconfig->clock > 0)
					printf(" @ %d Hz\n", devconfig->clock);
				else
					printf("\n");
			}

			count++;
			machine_config_free(config);
		}

	return (count > 0) ? MAMERR_NONE : MAMERR_NO_SUCH_GAME;
}


/*-------------------------------------------------
    info_verifyroms - verify the ROM sets of
    one or more games
-------------------------------------------------*/

static int info_verifyroms(core_options *options, const char *gamename)
{
	int correct = 0;
	int incorrect = 0;
	int notfound = 0;
	int drvindex;

	/* iterate over drivers */
	for (drvindex = 0; drivers[drvindex] != NULL; drvindex++)
		if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			audit_record *audit;
			int audit_records;
			int res;

			/* audit the ROMs in this set */
			audit_records = audit_images(options, drivers[drvindex], AUDIT_VALIDATE_FAST, &audit);
			res = audit_summary(drivers[drvindex], audit_records, audit, TRUE);
			if (audit_records > 0)
				global_free(audit);

			/* if not found, count that and leave it at that */
			if (res == NOTFOUND)
				notfound++;

			/* else display information about what we discovered */
			else
			{
				const game_driver *clone_of;

				/* output the name of the driver and its clone */
				mame_printf_info("romset %s ", drivers[drvindex]->name);
				clone_of = driver_get_clone(drivers[drvindex]);
				if (clone_of != NULL)
					mame_printf_info("[%s] ", clone_of->name);

				/* switch off of the result */
				switch (res)
				{
					case INCORRECT:
						mame_printf_info("is bad\n");
						incorrect++;
						break;

					case CORRECT:
						mame_printf_info("is good\n");
						correct++;
						break;

					case BEST_AVAILABLE:
						mame_printf_info("is best available\n");
						correct++;
						break;
				}
			}
		}

	/* clear out any cached files */
	zip_file_cache_clear();

	/* if we didn't get anything at all, display a generic end message */
	if (correct + incorrect == 0)
	{
		if (notfound > 0)
			mame_printf_info("romset \"%s\" not found!\n", gamename);
		else
			mame_printf_info("romset \"%s\" not supported!\n", gamename);
		return MAMERR_NO_SUCH_GAME;
	}

	/* otherwise, print a summary */
	else
	{
		mame_printf_info("%d romsets found, %d were OK.\n", correct + incorrect, correct);
		return (incorrect > 0) ? MAMERR_MISSING_FILES : MAMERR_NONE;
	}
}


/*-------------------------------------------------
    info_verifysamples - verify the sample sets of
    one or more games
-------------------------------------------------*/

static int info_verifysamples(core_options *options, const char *gamename)
{
	int correct = 0;
	int incorrect = 0;
	int notfound = FALSE;
	int drvindex;

	/* now iterate over drivers */
	for (drvindex = 0; drivers[drvindex] != NULL; drvindex++)
		if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			audit_record *audit;
			int audit_records;
			int res;

			/* audit the samples in this set */
			audit_records = audit_samples(options, drivers[drvindex], &audit);
			res = audit_summary(drivers[drvindex], audit_records, audit, TRUE);
			if (audit_records > 0)
				global_free(audit);
			else
				continue;

			/* if not found, print a message and set the flag */
			if (res == NOTFOUND)
			{
				mame_printf_error("sampleset \"%s\" not found!\n", drivers[drvindex]->name);
				notfound = TRUE;
			}

			/* else display information about what we discovered */
			else
			{
				mame_printf_info("sampleset %s ", drivers[drvindex]->name);

				/* switch off of the result */
				switch (res)
				{
					case INCORRECT:
						mame_printf_info("is bad\n");
						incorrect++;
						break;

					case CORRECT:
						mame_printf_info("is good\n");
						correct++;
						break;

					case BEST_AVAILABLE:
						mame_printf_info("is best available\n");
						correct++;
						break;
				}
			}
		}

	/* clear out any cached files */
	zip_file_cache_clear();

	/* if we didn't get anything at all because of an unsupported set, display message */
	if (correct + incorrect == 0)
	{
		if (!notfound)
			mame_printf_error("sampleset \"%s\" not supported!\n", gamename);
		return MAMERR_NO_SUCH_GAME;
	}

	/* otherwise, print a summary */
	else
	{
		mame_printf_info("%d samplesets found, %d were OK.\n", correct + incorrect, correct);
		return (incorrect > 0) ? MAMERR_MISSING_FILES : MAMERR_NONE;
	}
}


/*-------------------------------------------------
    info_romident - identify ROMs by looking for
    matches in our internal database
-------------------------------------------------*/

static int info_romident(core_options *options, const char *gamename)
{
	romident_status status;

	/* a NULL gamename is a fatal error */
	if (gamename == NULL)
		return MAMERR_FATALERROR;

	/* do the identification */
	romident(options, gamename, &status);

	/* clear out any cached files */
	zip_file_cache_clear();

	/* return the appropriate error code */
	if (status.matches == status.total)
		return MAMERR_NONE;
	else if (status.matches == status.total - status.nonroms)
		return MAMERR_IDENT_NONROMS;
	else if (status.matches > 0)
		return MAMERR_IDENT_PARTIAL;
	else
		return MAMERR_IDENT_NONE;
}



/***************************************************************************
    UTILITIES
***************************************************************************/

/*-------------------------------------------------
    romident - identify files
-------------------------------------------------*/

static void romident(core_options *options, const char *filename, romident_status *status)
{
	osd_directory *directory;

	/* reset the status */
	memset(status, 0, sizeof(*status));

	/* first try to open as a directory */
	directory = osd_opendir(filename);
	if (directory != NULL)
	{
		const osd_directory_entry *entry;

		/* iterate over all files in the directory */
		while ((entry = osd_readdir(directory)) != NULL)
			if (entry->type == ENTTYPE_FILE)
			{
				astring curfile(filename, PATH_SEPARATOR, entry->name);
				identify_file(options, curfile, status);
			}
		osd_closedir(directory);
	}

	/* if that failed, and the filename ends with .zip, identify as a ZIP file */
	else if (core_filename_ends_with(filename, ".zip"))
	{
		/* first attempt to examine it as a valid ZIP file */
		zip_file *zip = NULL;
		zip_error ziperr = zip_file_open(filename, &zip);
		if (ziperr == ZIPERR_NONE && zip != NULL)
		{
			const zip_file_header *entry;

			/* loop over entries in the ZIP, skipping empty files and directories */
			for (entry = zip_file_first_file(zip); entry; entry = zip_file_next_file(zip))
				if (entry->uncompressed_length != 0)
				{
					UINT8 *data = global_alloc_array(UINT8, entry->uncompressed_length);
					if (data != NULL)
					{
						/* decompress data into RAM and identify it */
						ziperr = zip_file_decompress(zip, data, entry->uncompressed_length);
						if (ziperr == ZIPERR_NONE)
							identify_data(options, entry->filename, data, entry->uncompressed_length, status);
						global_free(data);
					}
				}

			/* close up */
			zip_file_close(zip);
		}
	}

	/* otherwise, identify as a raw file */
	else
		identify_file(options, filename, status);
}


/*-------------------------------------------------
    identify_file - identify a file; if it is a
    ZIP file, scan it and identify all enclosed
    files
-------------------------------------------------*/

static void identify_file(core_options *options, const char *name, romident_status *status)
{
	file_error filerr;
	osd_file *file;
	UINT64 length;

	/* open for read and process if it opens and has a valid length */
	filerr = osd_open(name, OPEN_FLAG_READ, &file, &length);
	if (filerr == FILERR_NONE && length > 0 && (UINT32)length == length)
	{
		UINT8 *data = global_alloc_array(UINT8, length);
		if (data != NULL)
		{
			UINT32 bytes;

			/* read file data into RAM and identify it */
			filerr = osd_read(file, data, 0, length, &bytes);
			if (filerr == FILERR_NONE)
				identify_data(options, name, data, bytes, status);
			global_free(data);
		}
		osd_close(file);
	}
}


/*-------------------------------------------------
    identify_data - identify a buffer full of
    data; if it comes from a .JED file, parse the
    fusemap into raw data first
-------------------------------------------------*/

static void identify_data(core_options *options, const char *name, const UINT8 *data, int length, romident_status *status)
{
	char hash[HASH_BUF_SIZE];
	UINT8 *tempjed = NULL;
	astring basename;
	int found = 0;
	jed_data jed;

	/* if this is a '.jed' file, process it into raw bits first */
	if (core_filename_ends_with(name, ".jed") && jed_parse(data, length, &jed) == JEDERR_NONE)
	{
		/* now determine the new data length and allocate temporary memory for it */
		length = jedbin_output(&jed, NULL, 0);
		tempjed = global_alloc_array(UINT8, length);
		if (tempjed == NULL)
			return;

		/* create a binary output of the JED data and use that instead */
		jedbin_output(&jed, tempjed, length);
		data = tempjed;
	}

	/* compute the hash of the data */
	hash_data_clear(hash);
	hash_compute(hash, data, length, HASH_SHA1 | HASH_CRC);

	/* output the name */
	status->total++;
	core_filename_extract_base(&basename, name, FALSE);
	mame_printf_info("%-20s", basename.cstr());

	/* see if we can find a match in the ROMs */
	match_roms(options, hash, length, &found);

	/* if we didn't find it, try to guess what it might be */
	if (found == 0)
	{
		/* if not a power of 2, assume it is a non-ROM file */
		if ((length & (length - 1)) != 0)
		{
			mame_printf_info("NOT A ROM\n");
			status->nonroms++;
		}

		/* otherwise, it's just not a match */
		else
			mame_printf_info("NO MATCH\n");
	}

	/* if we did find it, count it as a match */
	else
		status->matches++;

	/* free any temporary JED data */
	if (tempjed != NULL)
		global_free(tempjed);
}


/*-------------------------------------------------
    match_roms - scan for a matching ROM by hash
-------------------------------------------------*/

static void match_roms(core_options *options, const char *hash, int length, int *found)
{
	int drvindex;

	/* iterate over drivers */
	for (drvindex = 0; drivers[drvindex] != NULL; drvindex++)
	{
		machine_config *config = machine_config_alloc(drivers[drvindex]->machine_config);
		const rom_entry *region, *rom;
		const rom_source *source;

		/* iterate over sources, regions and files within the region */
		for (source = rom_first_source(drivers[drvindex], config); source != NULL; source = rom_next_source(drivers[drvindex], config, source))
			for (region = rom_first_region(drivers[drvindex], source); region; region = rom_next_region(region))
				for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
					if (hash_data_is_equal(hash, ROM_GETHASHDATA(rom), 0))
					{
						int baddump = hash_data_has_info(ROM_GETHASHDATA(rom), HASH_INFO_BAD_DUMP);

						/* output information about the match */
						if (*found != 0)
							mame_printf_info("                    ");
						mame_printf_info("= %s%-20s  %-10s %s\n", baddump ? "(BAD) " : "", ROM_GETNAME(rom), drivers[drvindex]->name, drivers[drvindex]->description);
						(*found)++;
					}

		machine_config_free(config);
	}

#ifdef MESS
	mess_match_roms( options, hash, length, found );
#endif
}
