/***************************************************************************

    clifront.c

    Command-line interface frontend for MAME.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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
#include "xmlfile.h"

#include <new>
#include <ctype.h>
#ifdef MESS
#include "mess.h"
#endif


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct romident_status
{
	int			total;				/* total files processed */
	int			matches;			/* number of matches found */
	int			nonroms;			/* number of non-ROM files found */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void execute_commands(cli_options &options, const char *exename);
static void display_help(void);

/* informational functions */
static void info_verifyroms(emu_options &options, const char *gamename);
static void info_verifysamples(emu_options &options, const char *gamename);
static void info_romident(emu_options &options, const char *gamename);
static void info_listmedia(emu_options &options, const char *gamename);
static void info_listsoftware(emu_options &options, const char *gamename);

/* utilities */
static void romident(emu_options &options, const char *filename, romident_status *status);
static void identify_file(emu_options &options, const char *name, romident_status *status);
static void identify_data(emu_options &options, const char *name, const UINT8 *data, int length, romident_status *status);
static void match_roms(emu_options &options, const hash_collection &hashes, int length, int *found);
static void display_suggestions(const char *gamename);



//**************************************************************************
//  COMMAND-LINE OPTIONS
//**************************************************************************

const options_entry cli_options::s_option_entries[] =
{
	/* core commands */
	{ NULL,                            NULL,       OPTION_HEADER,     "CORE COMMANDS" },
	{ CLICOMMAND_HELP ";h;?",           "0",       OPTION_COMMAND,    "show help message" },
	{ CLICOMMAND_VALIDATE ";valid",     "0",       OPTION_COMMAND,    "perform driver validation on all game drivers" },

	/* configuration commands */
	{ NULL,                            NULL,       OPTION_HEADER,     "CONFIGURATION COMMANDS" },
	{ CLICOMMAND_CREATECONFIG ";cc",    "0",       OPTION_COMMAND,    "create the default configuration file" },
	{ CLICOMMAND_SHOWCONFIG ";sc",      "0",       OPTION_COMMAND,    "display running parameters" },
	{ CLICOMMAND_SHOWUSAGE ";su",       "0",       OPTION_COMMAND,    "show this help" },

	/* frontend commands */
	{ NULL,                            NULL,       OPTION_HEADER,     "FRONTEND COMMANDS" },
	{ CLICOMMAND_LISTXML ";lx",         "0",       OPTION_COMMAND,    "all available info on driver in XML format" },
	{ CLICOMMAND_LISTFULL ";ll",        "0",       OPTION_COMMAND,    "short name, full name" },
	{ CLICOMMAND_LISTSOURCE ";ls",      "0",       OPTION_COMMAND,    "driver sourcefile" },
	{ CLICOMMAND_LISTCLONES ";lc",      "0",       OPTION_COMMAND,    "show clones" },
	{ CLICOMMAND_LISTBROTHERS ";lb",    "0",       OPTION_COMMAND,    "show \"brothers\", or other drivers from same sourcefile" },
	{ CLICOMMAND_LISTCRC,               "0",       OPTION_COMMAND,    "CRC-32s" },
	{ CLICOMMAND_LISTROMS,              "0",       OPTION_COMMAND,    "list required roms for a driver" },
	{ CLICOMMAND_LISTSAMPLES,           "0",       OPTION_COMMAND,    "list optional samples for a driver" },
	{ CLICOMMAND_VERIFYROMS,            "0",       OPTION_COMMAND,    "report romsets that have problems" },
	{ CLICOMMAND_VERIFYSAMPLES,         "0",       OPTION_COMMAND,    "report samplesets that have problems" },
	{ CLICOMMAND_ROMIDENT,              "0",       OPTION_COMMAND,    "compare files with known MAME roms" },
	{ CLICOMMAND_LISTDEVICES ";ld",     "0",       OPTION_COMMAND,    "list available devices" },
	{ CLICOMMAND_LISTMEDIA ";lm",       "0",       OPTION_COMMAND,    "list available media for the system" },
	{ CLICOMMAND_LISTSOFTWARE ";lsoft", "0",       OPTION_COMMAND,    "list known software for the system" },

	{ NULL }
};



//**************************************************************************
//  CLI OPTIONS
//**************************************************************************

//-------------------------------------------------
//  cli_options - constructor
//-------------------------------------------------

cli_options::cli_options()
{
	add_entries(s_option_entries);
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

static void display_suggestions(const char *gamename)
{
	const game_driver *matches[10];
	int drvnum;

	/* get the top 10 approximate matches */
	driver_list_get_approx_matches(drivers, gamename, ARRAY_LENGTH(matches), matches);

	/* print them out */
	fprintf(stderr, "\n\"%s\" approximately matches the following\n"
			"supported " GAMESNOUN " (best match first):\n\n", gamename);
	for (drvnum = 0; drvnum < ARRAY_LENGTH(matches); drvnum++)
		if (matches[drvnum] != NULL)
			fprintf(stderr, "%-18s%s\n", matches[drvnum]->name, matches[drvnum]->description);
}


//-------------------------------------------------
//  cli_execute - execute a game via the standard
//  command line interface
//-------------------------------------------------

int cli_execute(cli_options &options, osd_interface &osd, int argc, char **argv)
{
	// wrap the core execution in a try/catch to field all fatal errors
	int result = MAMERR_NONE;
	try
	{
		// parse the command line, adding any system-specific options
		astring option_errors;
		if (!options.parse_command_line(argc, argv, option_errors))
			throw emu_fatalerror(MAMERR_INVALID_CONFIG, "%s", option_errors.trimspace().cstr());
		if (option_errors)
			printf("Error in command line:\n%s\n", option_errors.trimspace().cstr());

		// determine the base name of the EXE
		astring exename;
		core_filename_extract_base(&exename, argv[0], TRUE);

		// if we have a command, execute that
		if (strlen(options.command()) != 0)
			execute_commands(options, exename);

		// otherwise, check for a valid system
		else
		{
			// if we can't find it, give an appropriate error
			const game_driver *system = options.system();
			if (system == NULL && strlen(options.system_name()) > 0)
				throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "Unknown system '%s'", options.system_name());

			// otherwise just run the game
			result = mame_execute(options, osd);
		}
	}

	// handle exceptions of various types
	catch (emu_fatalerror &fatal)
	{
		astring string(fatal.string());
		fprintf(stderr, "%s\n", string.trimspace().cstr());
		result = (fatal.exitcode() != 0) ? fatal.exitcode() : MAMERR_FATALERROR;

		// if a game was specified, wasn't a wildcard, and our error indicates this was the
		// reason for failure, offer some suggestions
		if (result == MAMERR_NO_SUCH_GAME && strlen(options.system_name()) > 0 && strchr(options.system_name(), '*') == NULL && options.system() == NULL)
			display_suggestions(options.system_name());
	}
	catch (emu_exception &)
	{
		fprintf(stderr, "Caught unhandled emulator exception\n");
		result = MAMERR_FATALERROR;
	}
	catch (std::bad_alloc &)
	{
		fprintf(stderr, "Out of memory!\n");
		result = MAMERR_FATALERROR;
	}

	// handle any other exceptions
	catch (...)
	{
		fprintf(stderr, "Caught unhandled exception\n");
		result = MAMERR_FATALERROR;
	}

	// report any unfreed memory on clean exits
	if (result == MAMERR_NONE)
		dump_unfreed_mem();
	return result;
}


//-------------------------------------------------
//  execute_commands - execute various frontend
//  commands
//-------------------------------------------------

static void execute_commands(cli_options &options, const char *exename)
{
	// help?
	if (strcmp(options.command(), CLICOMMAND_HELP) == 0)
	{
		display_help();
		return;
	}

	// showusage?
	if (strcmp(options.command(), CLICOMMAND_SHOWUSAGE) == 0)
	{
		astring helpstring;
		mame_printf_info("Usage: %s [%s] [options]\n\nOptions:\n%s", exename, GAMENOUN, options.output_help(helpstring));
		return;
	}

	// validate?
	if (strcmp(options.command(), CLICOMMAND_VALIDATE) == 0)
	{
		validate_drivers(options);
		return;
	}

	// other commands need the INIs parsed
	astring option_errors;
	options.parse_standard_inis(option_errors);
	if (option_errors)
		printf("%s\n", option_errors.cstr());

	// createconfig?
	if (strcmp(options.command(), CLICOMMAND_CREATECONFIG) == 0)
	{
		// attempt to open the output file
		emu_file file(OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		if (file.open(CONFIGNAME ".ini") != FILERR_NONE)
			throw emu_fatalerror("Unable to create file " CONFIGNAME ".ini\n");

		// generate the updated INI
		astring initext;
		file.puts(options.output_ini(initext));
		return;
	}

	// showconfig?
	if (strcmp(options.command(), CLICOMMAND_SHOWCONFIG) == 0)
	{
		// print the INI text
		astring initext;
		printf("%s\n", options.output_ini(initext));
		return;
	}

	// all other commands call out to one of these helpers
	static const struct
	{
		const char *option;
		void (*function)(emu_options &options, const char *gamename);
	} info_commands[] =
	{
		{ CLICOMMAND_LISTXML,		cli_info_listxml },
		{ CLICOMMAND_LISTFULL,		cli_info_listfull },
		{ CLICOMMAND_LISTSOURCE,	cli_info_listsource },
		{ CLICOMMAND_LISTCLONES,	cli_info_listclones },
		{ CLICOMMAND_LISTBROTHERS,	cli_info_listbrothers },
		{ CLICOMMAND_LISTCRC,		cli_info_listcrc },
		{ CLICOMMAND_LISTDEVICES,	cli_info_listdevices },
		{ CLICOMMAND_LISTROMS,		cli_info_listroms },
		{ CLICOMMAND_LISTSAMPLES,	cli_info_listsamples },
		{ CLICOMMAND_VERIFYROMS,	info_verifyroms },
		{ CLICOMMAND_VERIFYSAMPLES,	info_verifysamples },
		{ CLICOMMAND_LISTMEDIA,		info_listmedia },
		{ CLICOMMAND_LISTSOFTWARE,	info_listsoftware },
		{ CLICOMMAND_ROMIDENT,		info_romident }
	};

	// find the command
	for (int cmdindex = 0; cmdindex < ARRAY_LENGTH(info_commands); cmdindex++)
		if (strcmp(options.command(), info_commands[cmdindex].option) == 0)
		{
			// parse any relevant INI files before proceeding
			const char *sysname = options.system_name();
			(*info_commands[cmdindex].function)(options, (sysname[0] == 0) ? "*" : sysname);
			return;
		}

	// if we get here, we don't know what has been requested
	throw emu_fatalerror(MAMERR_INVALID_CONFIG, "Unknown command '%s' specified", options.command());
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

void cli_info_listxml(emu_options &options, const char *gamename)
{
	print_mame_xml(stdout, drivers, gamename, options);
}


/*-------------------------------------------------
    cli_info_listfull - output the name and
    description of one or more games
-------------------------------------------------*/

void cli_info_listfull(emu_options &options, const char *gamename)
{
	int count = 0;

	/* iterate over drivers */
	for (int drvindex = 0; drivers[drvindex] != NULL; drvindex++)
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
	if (count == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);
}


/*-------------------------------------------------
    cli_info_listsource - output the name and source
    filename of one or more games
-------------------------------------------------*/

void cli_info_listsource(emu_options &options, const char *gamename)
{
	int count = 0;
	astring filename;

	/* iterate over drivers */
	for (int drvindex = 0; drivers[drvindex] != NULL; drvindex++)
		if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			/* output the remaining information */
			mame_printf_info("%-16s %s\n", drivers[drvindex]->name, core_filename_extract_base(&filename, drivers[drvindex]->source_file, FALSE)->cstr());
			count++;
		}

	/* return an error if none found */
	if (count == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);
}


/*-------------------------------------------------
    cli_info_listclones - output the name and source
    filename of one or more games
-------------------------------------------------*/

void cli_info_listclones(emu_options &options, const char *gamename)
{
	int count = 0, drvcnt = 0;

	for (int drvindex = 0; drivers[drvindex] != NULL; drvindex++)
		if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
			drvcnt++;

	if (drvcnt == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	/* iterate over drivers */
	for (int drvindex = 0; drivers[drvindex] != NULL; drvindex++)
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
}


/*-------------------------------------------------
    cli_info_listbrothers - output the name and
    source filename of one or more games
-------------------------------------------------*/

void cli_info_listbrothers(emu_options &options, const char *gamename)
{
	UINT8 *didit = global_alloc_array_clear(UINT8, driver_list_get_count(drivers));
	int count = 0;
	astring filename;

	/* iterate over drivers */
	for (int drvindex = 0; drivers[drvindex] != NULL; drvindex++)
		if (!didit[drvindex] && mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			didit[drvindex] = TRUE;
			if (count > 0)
				mame_printf_info("\n");
			mame_printf_info("%s ... other drivers in %s:\n", drivers[drvindex]->name, core_filename_extract_base(&filename, drivers[drvindex]->source_file, FALSE)->cstr());

			/* now iterate again over drivers, finding those with the same source file */
			for (int matchindex = 0; drivers[matchindex]; matchindex++)
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
	if (count == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);
}


/*-------------------------------------------------
    cli_info_listcrc - output the CRC and name of
    all ROMs referenced by MAME
-------------------------------------------------*/

void cli_info_listcrc(emu_options &options, const char *gamename)
{
	int count = 0;

	/* iterate over drivers */
	for (int drvindex = 0; drivers[drvindex] != NULL; drvindex++)
		if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			machine_config config(*drivers[drvindex], options);
			const rom_entry *region, *rom;
			const rom_source *source;

			/* iterate over sources, regions, and then ROMs within the region */
			for (source = rom_first_source(config); source != NULL; source = rom_next_source(*source))
				for (region = rom_first_region(*source); region; region = rom_next_region(region))
					for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
					{
						/* if we have a CRC, display it */
						UINT32 crc;
						if (hash_collection(ROM_GETHASHDATA(rom)).crc(crc))
							mame_printf_info("%08x %-12s %s\n", crc, ROM_GETNAME(rom), drivers[drvindex]->description);
					}

			count++;
		}

	/* return an error if none found */
	if (count == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);
}


/*-------------------------------------------------
    cli_info_listroms - output the list of ROMs
    referenced by a given game or set of games
-------------------------------------------------*/

void cli_info_listroms(emu_options &options, const char *gamename)
{
	int count = 0;
	astring tempstr;

	/* iterate over drivers */
	for (int drvindex = 0; drivers[drvindex] != NULL; drvindex++)
		if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			machine_config config(*drivers[drvindex], options);

			/* print the header */
			if (count > 0)
				mame_printf_info("\n");
			mame_printf_info("This is the list of the ROMs required for driver \"%s\".\n"
					"Name                    Size Checksum\n", drivers[drvindex]->name);

			/* iterate over sources, regions and then ROMs within the region */
			for (const rom_source *source = rom_first_source(config); source != NULL; source = rom_next_source(*source))
				for (const rom_entry *region = rom_first_region(*source); region != NULL; region = rom_next_region(region))
					for (const rom_entry *rom = rom_first_file(region); rom != NULL; rom = rom_next_file(rom))
					{
						/* accumulate the total length of all chunks */
						int length = -1;
						if (ROMREGION_ISROMDATA(region))
							length = rom_file_size(rom);

						/* start with the name */
						const char *name = ROM_GETNAME(rom);
						mame_printf_info("%-20s ", name);

						/* output the length next */
						if (length >= 0)
							mame_printf_info("%7d", length);
						else
							mame_printf_info("       ");

						/* output the hash data */
						hash_collection hashes(ROM_GETHASHDATA(rom));
						if (!hashes.flag(hash_collection::FLAG_NO_DUMP))
						{
							if (hashes.flag(hash_collection::FLAG_BAD_DUMP))
								mame_printf_info(" BAD");
							mame_printf_info(" %s", hashes.macro_string(tempstr));
						}
						else
							mame_printf_info(" NO GOOD DUMP KNOWN");

						/* end with a CR */
						mame_printf_info("\n");
					}

			count++;
		}

	if (count == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);
}


/*-------------------------------------------------
    cli_info_listsamples - output the list of samples
    referenced by a given game or set of games
-------------------------------------------------*/

void cli_info_listsamples(emu_options &options, const char *gamename)
{
	int count = 0;

	/* iterate over drivers */
	for (int drvindex = 0; drivers[drvindex] != NULL; drvindex++)
		if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			machine_config config(*drivers[drvindex], options);
			const device_config_sound_interface *sound = NULL;

			/* find samples interfaces */
			for (bool gotone = config.m_devicelist.first(sound); gotone; gotone = sound->next(sound))
				if (sound->devconfig().type() == SAMPLES)
				{
					const char *const *samplenames = ((const samples_interface *)sound->devconfig().static_config())->samplenames;
					int sampnum;

					/* if the list is legit, walk it and print the sample info */
					if (samplenames != NULL)
						for (sampnum = 0; samplenames[sampnum] != NULL; sampnum++)
							mame_printf_info("%s\n", samplenames[sampnum]);
				}

			count++;
		}

	if (count == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);
}


/*-------------------------------------------------
    cli_info_listdevices - output the list of
    devices referenced by a given game or set of
    games
-------------------------------------------------*/

void cli_info_listdevices(emu_options &options, const char *gamename)
{
	int count = 0;

	/* iterate over drivers */
	for (int drvindex = 0; drivers[drvindex] != NULL; drvindex++)
		if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			machine_config config(*drivers[drvindex], options);

			if (count != 0)
				printf("\n");
			printf("Driver %s (%s):\n", drivers[drvindex]->name, drivers[drvindex]->description);

			/* iterate through devices */
			for (const device_config *devconfig = config.m_devicelist.first(); devconfig != NULL; devconfig = devconfig->next())
			{
				printf("   %s ('%s')", devconfig->name(), devconfig->tag());

				UINT32 clock = devconfig->clock();
				if (clock >= 1000000000)
					printf(" @ %d.%02d GHz\n", clock / 1000000000, (clock / 10000000) % 100);
				else if (clock >= 1000000)
					printf(" @ %d.%02d MHz\n", clock / 1000000, (clock / 10000) % 100);
				else if (clock >= 1000)
					printf(" @ %d.%02d kHz\n", clock / 1000, (clock / 10) % 100);
				else if (clock > 0)
					printf(" @ %d Hz\n", clock);
				else
					printf("\n");
			}

			count++;
		}

	if (count == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);
}


/*-------------------------------------------------
    info_verifyroms - verify the ROM sets of
    one or more games
-------------------------------------------------*/

static void info_verifyroms(emu_options &options, const char *gamename)
{
	int correct = 0;
	int incorrect = 0;
	int notfound = 0;

	/* iterate over drivers */
	for (int drvindex = 0; drivers[drvindex] != NULL; drvindex++)
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
			throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "romset \"%s\" not found!\n", gamename);
		else
			throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "romset \"%s\" not supported!\n", gamename);
	}

	/* otherwise, print a summary */
	else
	{
		if (incorrect > 0)
			throw emu_fatalerror(MAMERR_MISSING_FILES, "%d romsets found, %d were OK.\n", correct + incorrect, correct);
		mame_printf_info("%d romsets found, %d were OK.\n", correct, correct);
	}
}

/*-------------------------------------------------
    info_listsoftware - output the list of
    software supported by a given game or set of
    games
    TODO: Add all information read from the source files
    Possible improvement: use a sorted list for
        identifying duplicate lists.
-------------------------------------------------*/

static void info_listsoftware(emu_options &options, const char *gamename)
{
	FILE *out = stdout;
	int nr_lists = 0;
	char ** lists = NULL;
	int list_idx = 0;

	/* First determine the maximum number of lists we might encounter */
	for ( int drvindex = 0; drivers[drvindex] != NULL; drvindex++ )
	{
		if ( mame_strwildcmp( gamename, drivers[drvindex]->name ) == 0 )
		{
			/* allocate the machine config */
			machine_config config(*drivers[drvindex], options);

			for (const device_config *dev = config.m_devicelist.first(SOFTWARE_LIST); dev != NULL; dev = dev->typenext())
			{
				software_list_config *swlist = (software_list_config *)downcast<const legacy_device_config_base *>(dev)->inline_config();

				for ( int i = 0; i < DEVINFO_STR_SWLIST_MAX - DEVINFO_STR_SWLIST_0; i++ )
				{
					if ( swlist->list_name[i] && *swlist->list_name[i]  && (swlist->list_type == SOFTWARE_LIST_ORIGINAL_SYSTEM))
						nr_lists++;
				}
			}
		}
	}

	lists = global_alloc_array( char *, nr_lists );

	if (nr_lists)
	{
		fprintf( out,
				"<?xml version=\"1.0\"?>\n"
				"<!DOCTYPE softwarelist [\n"
				"<!ELEMENT softwarelists (softwarelist*)>\n"
				"\t<!ELEMENT softwarelist (software+)>\n"
				"\t\t<!ATTLIST softwarelist name CDATA #REQUIRED>\n"
				"\t\t<!ATTLIST softwarelist description CDATA #IMPLIED>\n"
				"\t\t<!ELEMENT software (description, year?, publisher, info*, sharedfeat*, part*)>\n"
				"\t\t\t<!ATTLIST software name CDATA #REQUIRED>\n"
				"\t\t\t<!ATTLIST software cloneof CDATA #IMPLIED>\n"
				"\t\t\t<!ATTLIST software supported (yes|partial|no) \"yes\">\n"
				"\t\t\t<!ELEMENT description (#PCDATA)>\n"
				"\t\t\t<!ELEMENT year (#PCDATA)>\n"
				"\t\t\t<!ELEMENT publisher (#PCDATA)>\n"
				// we still do not store the info strings internally, so there is no output here
				// TODO: add parsing info in softlist.c and then add output here!
				"\t\t\t<!ELEMENT info EMPTY>\n"
				"\t\t\t\t<!ATTLIST info name CDATA #REQUIRED>\n"
				"\t\t\t\t<!ATTLIST info value CDATA #IMPLIED>\n"
				// shared features get stored in the part->feature below and are output there
				// this means that we don't output any <sharedfeat> and that -lsoft output will 
				// be different from the list in hash/ when the list uses sharedfeat. But this 
				// is by design: sharedfeat is only available to simplify the life to list creators, 
				// to e.g. avoid manually adding the same feature to each disk of a 9 floppies game!
				"\t\t\t<!ELEMENT sharedfeat EMPTY>\n"
				"\t\t\t\t<!ATTLIST sharedfeat name CDATA #REQUIRED>\n"
				"\t\t\t\t<!ATTLIST sharedfeat value CDATA #IMPLIED>\n"
				"\t\t\t<!ELEMENT part (feature*, dataarea*, diskarea*, dipswitch*)>\n"
				"\t\t\t\t<!ATTLIST part name CDATA #REQUIRED>\n"
				"\t\t\t\t<!ATTLIST part interface CDATA #REQUIRED>\n"
				"\t\t\t\t<!ELEMENT feature EMPTY>\n"
				"\t\t\t\t\t<!ATTLIST feature name CDATA #REQUIRED>\n"
				"\t\t\t\t\t<!ATTLIST feature value CDATA #IMPLIED>\n"
				"\t\t\t\t<!ELEMENT dataarea (rom*)>\n"
				"\t\t\t\t\t<!ATTLIST dataarea name CDATA #REQUIRED>\n"
				"\t\t\t\t\t<!ATTLIST dataarea size CDATA #REQUIRED>\n"
				"\t\t\t\t\t<!ATTLIST dataarea databits (8|16|32|64) \"8\">\n"
				"\t\t\t\t\t<!ATTLIST dataarea endian (big|little) \"little\">\n"
				"\t\t\t\t\t<!ELEMENT rom EMPTY>\n"
				"\t\t\t\t\t\t<!ATTLIST rom name CDATA #IMPLIED>\n"
				"\t\t\t\t\t\t<!ATTLIST rom size CDATA #REQUIRED>\n"
				"\t\t\t\t\t\t<!ATTLIST rom crc CDATA #IMPLIED>\n"
				"\t\t\t\t\t\t<!ATTLIST rom md5 CDATA #IMPLIED>\n"
				"\t\t\t\t\t\t<!ATTLIST rom sha1 CDATA #IMPLIED>\n"
				"\t\t\t\t\t\t<!ATTLIST rom offset CDATA #IMPLIED>\n"
				"\t\t\t\t\t\t<!ATTLIST rom value CDATA #IMPLIED>\n"
				"\t\t\t\t\t\t<!ATTLIST rom status (baddump|nodump|good) \"good\">\n"
				"\t\t\t\t\t\t<!ATTLIST rom loadflag (load16_byte|load16_word|load16_word_swap|load32_byte|load32_word|load32_word_swap|load32_dword|load64_word|load64_word_swap|reload|fill|continue) #IMPLIED>\n"
				"\t\t\t\t<!ELEMENT diskarea (disk*)>\n"
				"\t\t\t\t\t<!ATTLIST diskarea name CDATA #REQUIRED>\n"
				"\t\t\t\t\t<!ELEMENT disk EMPTY>\n"
				"\t\t\t\t\t\t<!ATTLIST disk name CDATA #REQUIRED>\n"
				"\t\t\t\t\t\t<!ATTLIST disk md5 CDATA #IMPLIED>\n"
				"\t\t\t\t\t\t<!ATTLIST disk sha1 CDATA #IMPLIED>\n"
				"\t\t\t\t\t\t<!ATTLIST disk status (baddump|nodump|good) \"good\">\n"
				"\t\t\t\t\t\t<!ATTLIST disk writeable (yes|no) \"no\">\n"
				// we still do not store the dipswitch values internally, so there is no output here
				// TODO: add parsing dipsw in softlist.c and then add output here!
				"\t\t\t\t<!ELEMENT dipswitch (dipvalue*)>\n"
				"\t\t\t\t\t<!ATTLIST dipswitch name CDATA #REQUIRED>\n"
				"\t\t\t\t\t<!ATTLIST dipswitch tag CDATA #REQUIRED>\n"
				"\t\t\t\t\t<!ATTLIST dipswitch mask CDATA #REQUIRED>\n"
				"\t\t\t\t\t<!ELEMENT dipvalue EMPTY>\n"
				"\t\t\t\t\t\t<!ATTLIST dipvalue name CDATA #REQUIRED>\n"
				"\t\t\t\t\t\t<!ATTLIST dipvalue value CDATA #REQUIRED>\n"
				"\t\t\t\t\t\t<!ATTLIST dipvalue default (yes|no) \"no\">\n"
				"]>\n\n"
				"<softwarelists>\n"
				);
	}

	for ( int drvindex = 0; drivers[drvindex] != NULL; drvindex++ )
	{
		if ( mame_strwildcmp( gamename, drivers[drvindex]->name ) == 0 )
		{
			/* allocate the machine config */
			machine_config config(*drivers[drvindex], options);

			for (const device_config *dev = config.m_devicelist.first(SOFTWARE_LIST); dev != NULL; dev = dev->typenext())
			{
				software_list_config *swlist = (software_list_config *)downcast<const legacy_device_config_base *>(dev)->inline_config();

				for ( int i = 0; i < DEVINFO_STR_SWLIST_MAX - DEVINFO_STR_SWLIST_0; i++ )
				{
					if ( swlist->list_name[i] && *swlist->list_name[i] && (swlist->list_type == SOFTWARE_LIST_ORIGINAL_SYSTEM))
					{
						software_list *list = software_list_open( options, swlist->list_name[i], FALSE, NULL );

						if ( list )
						{
							/* Verify if we have encountered this list before */
							bool seen_before = false;
							for ( int l = 0; l < list_idx && !seen_before; l++ )
							{
								if ( ! strcmp( swlist->list_name[i], lists[l] ) )
								{
									seen_before = true;
								}
							}

							if ( ! seen_before )
							{
								lists[list_idx] = core_strdup( swlist->list_name[i] );
								list_idx++;
								software_list_parse( list, NULL, NULL );

								fprintf(out, "\t<softwarelist name=\"%s\" description=\"%s\">\n", swlist->list_name[i], xml_normalize_string(software_list_get_description(list)) );

								for ( software_info *swinfo = software_list_find( list, "*", NULL ); swinfo != NULL; swinfo = software_list_find( list, "*", swinfo ) )
								{
									fprintf( out, "\t\t<software name=\"%s\"", swinfo->shortname );
									if ( swinfo->parentname != NULL )
										fprintf( out, " cloneof=\"%s\"", swinfo->parentname );
									if ( swinfo->supported == SOFTWARE_SUPPORTED_PARTIAL )
										fprintf( out, " supported=\"partial\"" );
									if ( swinfo->supported == SOFTWARE_SUPPORTED_NO )
										fprintf( out, " supported=\"no\"" );
									fprintf( out, ">\n" );
									fprintf( out, "\t\t\t<description>%s</description>\n", xml_normalize_string(swinfo->longname) );
									fprintf( out, "\t\t\t<year>%s</year>\n", xml_normalize_string( swinfo->year ) );
									fprintf( out, "\t\t\t<publisher>%s</publisher>\n", xml_normalize_string( swinfo->publisher ) );

									for ( software_part *part = software_find_part( swinfo, NULL, NULL ); part != NULL; part = software_part_next( part ) )
									{
										fprintf( out, "\t\t\t<part name=\"%s\"", part->name );
										if ( part->interface_ )
											fprintf( out, " interface=\"%s\"", part->interface_ );

										fprintf( out, ">\n");

										if ( part->featurelist )
										{
											feature_list *list = part->featurelist;

											while( list )
											{
												fprintf( out, "\t\t\t\t<feature name=\"%s\" value=\"%s\" />\n", list->name, list->value );
												list = list->next;
											}
										}

										/* TODO: display rom region information */
										for ( const rom_entry *region = part->romdata; region; region = rom_next_region( region ) )
										{
											int is_disk = ROMREGION_ISDISKDATA(region);

											if (!is_disk)
												fprintf( out, "\t\t\t\t<dataarea name=\"%s\" size=\"%d\">\n", ROMREGION_GETTAG(region), ROMREGION_GETLENGTH(region) );
											else
												fprintf( out, "\t\t\t\t<diskarea name=\"%s\">\n", ROMREGION_GETTAG(region) );

											for ( const rom_entry *rom = rom_first_file( region ); rom && !ROMENTRY_ISREGIONEND(rom); rom++ )
											{
												if ( ROMENTRY_ISFILE(rom) )
												{
													if (!is_disk)
														fprintf( out, "\t\t\t\t\t<rom name=\"%s\" size=\"%d\"", xml_normalize_string(ROM_GETNAME(rom)), rom_file_size(rom) );
													else
														fprintf( out, "\t\t\t\t\t<disk name=\"%s\"", xml_normalize_string(ROM_GETNAME(rom)) );

													/* dump checksum information only if there is a known dump */
													hash_collection hashes(ROM_GETHASHDATA(rom));
													if (!hashes.flag(hash_collection::FLAG_NO_DUMP))
													{
														astring tempstr;
														for (hash_base *hash = hashes.first(); hash != NULL; hash = hash->next())
															fprintf(out, " %s=\"%s\"", hash->name(), hash->string(tempstr));
													}

													if (!is_disk)
														fprintf( out, " offset=\"0x%x\"", ROM_GETOFFSET(rom) );

													if ( hashes.flag(hash_collection::FLAG_BAD_DUMP) )
														fprintf( out, " status=\"baddump\"" );
													if ( hashes.flag(hash_collection::FLAG_NO_DUMP) )
														fprintf( out, " status=\"nodump\"" );

													if (is_disk)
														fprintf( out, " writable=\"%s\"", (ROM_GETFLAGS(rom) & DISK_READONLYMASK) ? "no" : "yes");

													if ((ROM_GETFLAGS(rom) & ROM_SKIPMASK) == ROM_SKIP(1))
														fprintf( out, " loadflag=\"load16_byte\"" );

													if ((ROM_GETFLAGS(rom) & ROM_SKIPMASK) == ROM_SKIP(3))
														fprintf( out, " loadflag=\"load32_byte\"" );

													if (((ROM_GETFLAGS(rom) & ROM_SKIPMASK) == ROM_SKIP(2)) && ((ROM_GETFLAGS(rom) & ROM_GROUPMASK) == ROM_GROUPWORD))
													{
														if (!(ROM_GETFLAGS(rom) & ROM_REVERSEMASK))
															fprintf( out, " loadflag=\"load32_word\"" );
														else
															fprintf( out, " loadflag=\"load32_word_swap\"" );
													}

													if (((ROM_GETFLAGS(rom) & ROM_SKIPMASK) == ROM_SKIP(6)) && ((ROM_GETFLAGS(rom) & ROM_GROUPMASK) == ROM_GROUPWORD))
													{
														if (!(ROM_GETFLAGS(rom) & ROM_REVERSEMASK))
															fprintf( out, " loadflag=\"load64_word\"" );
														else
															fprintf( out, " loadflag=\"load64_word_swap\"" );
													}

													if (((ROM_GETFLAGS(rom) & ROM_SKIPMASK) == ROM_NOSKIP) && ((ROM_GETFLAGS(rom) & ROM_GROUPMASK) == ROM_GROUPWORD))
													{
														if (!(ROM_GETFLAGS(rom) & ROM_REVERSEMASK))
															fprintf( out, " loadflag=\"load32_dword\"" );
														else
															fprintf( out, " loadflag=\"load16_word_swap\"" );
													}

													fprintf( out, "/>\n" );
												}
												else if ( ROMENTRY_ISRELOAD(rom) )
												{
													fprintf( out, "\t\t\t\t\t<rom size=\"%d\" offset=\"0x%x\" loadflag=\"reload\" />\n", ROM_GETLENGTH(rom), ROM_GETOFFSET(rom) );
												}
												else if ( ROMENTRY_ISCONTINUE(rom) )
												{
													fprintf( out, "\t\t\t\t\t<rom size=\"%d\" offset=\"0x%x\" loadflag=\"continue\" />\n", ROM_GETLENGTH(rom), ROM_GETOFFSET(rom) );
												}
												else if ( ROMENTRY_ISFILL(rom) )
												{
													fprintf( out, "\t\t\t\t\t<rom size=\"%d\" offset=\"0x%x\" loadflag=\"fill\" />\n", ROM_GETLENGTH(rom), ROM_GETOFFSET(rom) );
												}
											}

											if (!is_disk)
												fprintf( out, "\t\t\t\t</dataarea>\n" );
											else
												fprintf( out, "\t\t\t\t</diskarea>\n" );
										}

										fprintf( out, "\t\t\t</part>\n" );
									}

									fprintf( out, "\t\t</software>\n" );
								}

								fprintf(out, "\t</softwarelist>\n" );
							}

							software_list_close( list );
						}
					}
				}
			}
		}
	}

	if (nr_lists)
		fprintf( out, "</softwarelists>\n" );
	else
		fprintf( out, "No software lists found for this system\n" );

	global_free( lists );
}


/*-------------------------------------------------
    softlist_match_roms - scan for a matching
    software ROM by hash
-------------------------------------------------*/
void softlist_match_roms(machine_config &config, const hash_collection &hashes, int length, int *found)
{
	for (const device_config *dev = config.m_devicelist.first(SOFTWARE_LIST); dev != NULL; dev = dev->typenext())
	{
		software_list_config *swlist = (software_list_config *)downcast<const legacy_device_config_base *>(dev)->inline_config();

		for ( int i = 0; i < DEVINFO_STR_SWLIST_MAX - DEVINFO_STR_SWLIST_0; i++ )
		{
			if ( swlist->list_name[i] )
			{
				software_list *list = software_list_open( config.options(), swlist->list_name[i], FALSE, NULL );

				for ( software_info *swinfo = software_list_find( list, "*", NULL ); swinfo != NULL; swinfo = software_list_find( list, "*", swinfo ) )
				{
					for ( software_part *part = software_find_part( swinfo, NULL, NULL ); part != NULL; part = software_part_next( part ) )
					{
						for ( const rom_entry *region = part->romdata; region != NULL; region = rom_next_region(region) )
						{
							for ( const rom_entry *rom = rom_first_file(region); rom != NULL; rom = rom_next_file(rom) )
							{
								hash_collection romhashes(ROM_GETHASHDATA(rom));
								if ( hashes == romhashes )
								{
									bool baddump = romhashes.flag(hash_collection::FLAG_BAD_DUMP);

									/* output information about the match */
									if (*found != 0)
										mame_printf_info("                    ");
									mame_printf_info("= %s%-20s  %s:%s %s\n", baddump ? "(BAD) " : "", ROM_GETNAME(rom), swlist->list_name[i], swinfo->shortname, swinfo->longname);
									(*found)++;
								}
							}
						}
					}
				}

				software_list_close( list );
			}
		}
	}
}

/*-------------------------------------------------
    info_listmedia - output the list of image
    devices referenced by a given game or set of
    games
-------------------------------------------------*/

static void info_listmedia(emu_options &options, const char *gamename)
{
	int count = 0, devcount;
	const device_config_image_interface *dev = NULL;
	const char *src;
	const char *driver_name;
	const char *name;
	const char *shortname;
	char paren_shortname[16];

	printf(" SYSTEM      MEDIA NAME (brief)   IMAGE FILE EXTENSIONS SUPPORTED     \n");
	printf("----------  --------------------  ------------------------------------\n");

	/* iterate over drivers */
	for (int drvindex = 0; drivers[drvindex] != NULL; drvindex++)
		if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			/* allocate the machine config */
			machine_config config(*drivers[drvindex], options);

			driver_name = drivers[drvindex]->name;

			devcount = 0;

			for (bool gotone = config.m_devicelist.first(dev); gotone; gotone = dev->next(dev))
			{
				src = downcast<const legacy_image_device_config_base *>(dev)->file_extensions();
				name = downcast<const legacy_image_device_config_base *>(dev)->instance_name();
				shortname = downcast<const legacy_image_device_config_base *>(dev)->brief_instance_name();

				sprintf(paren_shortname, "(%s)", shortname);

				printf("%-13s%-12s%-8s   ", driver_name, name, paren_shortname);
				driver_name = " ";

				astring extensions(src);
				char *ext = strtok((char*)extensions.cstr(),",");
				while (ext != NULL)
				{
					printf(".%-5s",ext);
					ext = strtok (NULL, ",");
					devcount++;
				}
				printf("\n");
			}
			if (!devcount)
				printf("%-13s(none)\n",driver_name);

			count++;
		}

	if (!count)
		printf("There are no Computers or Consoles named %s\n", gamename);

	if (count == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);
}


/*-------------------------------------------------
    info_verifysamples - verify the sample sets of
    one or more games
-------------------------------------------------*/

static void info_verifysamples(emu_options &options, const char *gamename)
{
	int correct = 0;
	int incorrect = 0;
	int notfound = FALSE;

	/* now iterate over drivers */
	for (int drvindex = 0; drivers[drvindex] != NULL; drvindex++)
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
			throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);
		else
			throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No sample sets found for '%s'", gamename);
	}

	/* otherwise, print a summary */
	else
	{
		if (incorrect > 0)
			throw emu_fatalerror(MAMERR_MISSING_FILES, "%d sample sets found, %d were OK.\n", correct + incorrect, correct);
		mame_printf_info("%d sample sets found, %d were OK.\n", correct, correct);
	}
}


/*-------------------------------------------------
    info_romident - identify ROMs by looking for
    matches in our internal database
-------------------------------------------------*/

static void info_romident(emu_options &options, const char *gamename)
{
	romident_status status;

	/* do the identification */
	romident(options, gamename, &status);

	/* clear out any cached files */
	zip_file_cache_clear();

	/* return the appropriate error code */
	if (status.matches == status.total)
		return;
	else if (status.matches == status.total - status.nonroms)
		throw emu_fatalerror(MAMERR_IDENT_NONROMS, "");
	else if (status.matches > 0)
		throw emu_fatalerror(MAMERR_IDENT_PARTIAL, "");
	else
		throw emu_fatalerror(MAMERR_IDENT_NONE, "");
}



/***************************************************************************
    UTILITIES
***************************************************************************/

/*-------------------------------------------------
    romident - identify files
-------------------------------------------------*/

static void romident(emu_options &options, const char *filename, romident_status *status)
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

static void identify_file(emu_options &options, const char *name, romident_status *status)
{
	file_error filerr;
	osd_file *file;
	UINT64 length;

	if (core_filename_ends_with(name, ".chd"))
	{
		chd_file *chd;
		chd_error err;
		astring basename;
		int found = 0;

		core_filename_extract_base(&basename, name, FALSE);
		mame_printf_info("%-20s", basename.cstr());

		status->total++;

		err = chd_open(name, CHD_OPEN_READ, NULL, &chd);
		if (err != CHDERR_NONE)
		{
			mame_printf_info("NOT A CHD\n");
			status->nonroms++;
		}
		else
		{
			chd_header header;

			header = *chd_get_header(chd);
			if (header.flags & CHDFLAGS_IS_WRITEABLE)
			{
				mame_printf_info("is a writable CHD\n");
			}
			else
			{
				static const UINT8 nullhash[20] = { 0 };
				hash_collection hashes;

				/* if there's an MD5 or SHA1 hash, add them to the output hash */
				if (memcmp(nullhash, header.md5, sizeof(header.md5)) != 0)
					hashes.add_from_buffer(hash_collection::HASH_MD5, header.md5, sizeof(header.md5));
				if (memcmp(nullhash, header.sha1, sizeof(header.sha1)) != 0)
					hashes.add_from_buffer(hash_collection::HASH_SHA1, header.sha1, sizeof(header.sha1));

				length = header.logicalbytes;

				match_roms(options, hashes, length, &found);

				if (found == 0)
				{
					mame_printf_info("NO MATCH\n");
				}

				/* if we did find it, count it as a match */
				else
					status->matches++;
			}

			chd_close(chd);
		}
	}
	else
	{
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
}


/*-------------------------------------------------
    identify_data - identify a buffer full of
    data; if it comes from a .JED file, parse the
    fusemap into raw data first
-------------------------------------------------*/

static void identify_data(emu_options &options, const char *name, const UINT8 *data, int length, romident_status *status)
{
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
	hash_collection hashes;
	hashes.compute(data, length, hash_collection::HASH_TYPES_CRC_SHA1);

	/* output the name */
	status->total++;
	core_filename_extract_base(&basename, name, FALSE);
	mame_printf_info("%-20s", basename.cstr());

	/* see if we can find a match in the ROMs */
	match_roms(options, hashes, length, &found);

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
	global_free(tempjed);
}


/*-------------------------------------------------
    match_roms - scan for a matching ROM by hash
-------------------------------------------------*/

static void match_roms(emu_options &options, const hash_collection &hashes, int length, int *found)
{
	/* iterate over drivers */
	for (int drvindex = 0; drivers[drvindex] != NULL; drvindex++)
	{
		machine_config config(*drivers[drvindex], options);

		/* iterate over sources, regions and files within the region */
		for (const rom_source *source = rom_first_source(config); source != NULL; source = rom_next_source(*source))
			for (const rom_entry *region = rom_first_region(*source); region; region = rom_next_region(region))
				for (const rom_entry *rom = rom_first_file(region); rom; rom = rom_next_file(rom))
				{
					hash_collection romhashes(ROM_GETHASHDATA(rom));
					if (!romhashes.flag(hash_collection::FLAG_NO_DUMP) && hashes == romhashes)
					{
						bool baddump = romhashes.flag(hash_collection::FLAG_BAD_DUMP);

						/* output information about the match */
						if (*found != 0)
							mame_printf_info("                    ");
						mame_printf_info("= %s%-20s  %-10s %s\n", baddump ? "(BAD) " : "", ROM_GETNAME(rom), drivers[drvindex]->name, drivers[drvindex]->description);
						(*found)++;
					}
				}

		// also check any softlists
		softlist_match_roms( config, hashes, length, found );
	}
}
