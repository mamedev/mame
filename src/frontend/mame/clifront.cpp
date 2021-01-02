// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    clifront.cpp

    Command-line interface frontend for MAME.

***************************************************************************/

#include "emu.h"
#include "clifront.h"

#include "ui/moptions.h"

#include "audit.h"
#include "infoxml.h"
#include "language.h"
#include "luaengine.h"
#include "mame.h"
#include "media_ident.h"
#include "pluginopts.h"

#include "emuopts.h"
#include "mameopts.h"
#include "romload.h"
#include "softlist_dev.h"
#include "validity.h"
#include "sound/samples.h"

#include "chd.h"
#include "unzip.h"
#include "xmlfile.h"

#include "osdepend.h"

#include <algorithm>
#include <new>
#include <cctype>


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// core commands
#define CLICOMMAND_HELP                 "help"
#define CLICOMMAND_VALIDATE             "validate"

// configuration commands
#define CLICOMMAND_CREATECONFIG         "createconfig"
#define CLICOMMAND_SHOWCONFIG           "showconfig"
#define CLICOMMAND_SHOWUSAGE            "showusage"

// frontend commands
#define CLICOMMAND_LISTXML              "listxml"
#define CLICOMMAND_LISTFULL             "listfull"
#define CLICOMMAND_LISTSOURCE           "listsource"
#define CLICOMMAND_LISTCLONES           "listclones"
#define CLICOMMAND_LISTBROTHERS         "listbrothers"
#define CLICOMMAND_LISTCRC              "listcrc"
#define CLICOMMAND_LISTROMS             "listroms"
#define CLICOMMAND_LISTSAMPLES          "listsamples"
#define CLICOMMAND_VERIFYROMS           "verifyroms"
#define CLICOMMAND_VERIFYSAMPLES        "verifysamples"
#define CLICOMMAND_ROMIDENT             "romident"
#define CLICOMMAND_LISTDEVICES          "listdevices"
#define CLICOMMAND_LISTSLOTS            "listslots"
#define CLICOMMAND_LISTMEDIA            "listmedia"
#define CLICOMMAND_LISTSOFTWARE         "listsoftware"
#define CLICOMMAND_VERIFYSOFTWARE       "verifysoftware"
#define CLICOMMAND_GETSOFTLIST          "getsoftlist"
#define CLICOMMAND_VERIFYSOFTLIST       "verifysoftlist"
#define CLICOMMAND_VERSION              "version"

// command options
#define CLIOPTION_DTD                   "dtd"


namespace {
//**************************************************************************
//  COMMAND-LINE OPTIONS
//**************************************************************************

const options_entry cli_option_entries[] =
{
	/* core commands */
	{ nullptr,                              nullptr,   OPTION_HEADER,     "CORE COMMANDS" },
	{ CLICOMMAND_HELP           ";h;?",     "0",       OPTION_COMMAND,    "show help message" },
	{ CLICOMMAND_VALIDATE       ";valid",   "0",       OPTION_COMMAND,    "perform validation on system drivers and devices" },

	/* configuration commands */
	{ nullptr,                              nullptr,   OPTION_HEADER,     "CONFIGURATION COMMANDS" },
	{ CLICOMMAND_CREATECONFIG   ";cc",      "0",       OPTION_COMMAND,    "create the default configuration file" },
	{ CLICOMMAND_SHOWCONFIG     ";sc",      "0",       OPTION_COMMAND,    "display running parameters" },
	{ CLICOMMAND_SHOWUSAGE      ";su",      "0",       OPTION_COMMAND,    "show this help" },

	/* frontend commands */
	{ nullptr,                              nullptr,   OPTION_HEADER,     "FRONTEND COMMANDS" },
	{ CLICOMMAND_LISTXML        ";lx",      "0",       OPTION_COMMAND,    "all available info on driver in XML format" },
	{ CLICOMMAND_LISTFULL       ";ll",      "0",       OPTION_COMMAND,    "short name, full name" },
	{ CLICOMMAND_LISTSOURCE     ";ls",      "0",       OPTION_COMMAND,    "driver sourcefile" },
	{ CLICOMMAND_LISTCLONES     ";lc",      "0",       OPTION_COMMAND,    "show clones" },
	{ CLICOMMAND_LISTBROTHERS   ";lb",      "0",       OPTION_COMMAND,    "show \"brothers\", or other drivers from same sourcefile" },
	{ CLICOMMAND_LISTCRC,                   "0",       OPTION_COMMAND,    "CRC-32s" },
	{ CLICOMMAND_LISTROMS       ";lr",      "0",       OPTION_COMMAND,    "list required ROMs for a driver" },
	{ CLICOMMAND_LISTSAMPLES,               "0",       OPTION_COMMAND,    "list optional samples for a driver" },
	{ CLICOMMAND_VERIFYROMS,                "0",       OPTION_COMMAND,    "report romsets that have problems" },
	{ CLICOMMAND_VERIFYSAMPLES,             "0",       OPTION_COMMAND,    "report samplesets that have problems" },
	{ CLICOMMAND_ROMIDENT,                  "0",       OPTION_COMMAND,    "compare files with known MAME ROMs" },
	{ CLICOMMAND_LISTDEVICES    ";ld",      "0",       OPTION_COMMAND,    "list available devices" },
	{ CLICOMMAND_LISTSLOTS      ";lslot",   "0",       OPTION_COMMAND,    "list available slots and slot devices" },
	{ CLICOMMAND_LISTMEDIA      ";lm",      "0",       OPTION_COMMAND,    "list available media for the system" },
	{ CLICOMMAND_LISTSOFTWARE   ";lsoft",   "0",       OPTION_COMMAND,    "list known software for the system" },
	{ CLICOMMAND_VERIFYSOFTWARE ";vsoft",   "0",       OPTION_COMMAND,    "verify known software for the system" },
	{ CLICOMMAND_GETSOFTLIST    ";glist",   "0",       OPTION_COMMAND,    "retrieve software list by name" },
	{ CLICOMMAND_VERIFYSOFTLIST ";vlist",   "0",       OPTION_COMMAND,    "verify software list by name" },
	{ CLICOMMAND_VERSION,                   "0",       OPTION_COMMAND,    "get MAME version" },

	{ nullptr,                              nullptr,   OPTION_HEADER,     "FRONTEND COMMAND OPTIONS" },
	{ CLIOPTION_DTD,                        "1",       OPTION_BOOLEAN,    "include DTD in XML output" },
	{ nullptr }
};


void print_summary(
		const media_auditor &auditor, media_auditor::summary summary, bool record_none_needed,
		const char *type, const char *name, const char *parent,
		unsigned &correct, unsigned &incorrect, unsigned &notfound,
		util::ovectorstream &buffer)
{
	if (summary == media_auditor::NOTFOUND)
	{
		// if not found, count that and leave it at that
		++notfound;
	}
	else if (record_none_needed || (summary != media_auditor::NONE_NEEDED))
	{
		// output the summary of the audit
		buffer.clear();
		buffer.seekp(0);
		auditor.summarize(name, &buffer);
		buffer.put('\0');
		osd_printf_info("%s", &buffer.vec()[0]);

		// output the name of the driver and its parent
		osd_printf_info("%sset %s ", type, name);
		if (parent)
			osd_printf_info("[%s] ", parent);

		// switch off of the result
		switch (summary)
		{
		case media_auditor::INCORRECT:
			osd_printf_info("is bad\n");
			++incorrect;
			return;

		case media_auditor::CORRECT:
			osd_printf_info("is good\n");
			++correct;
			return;

		case media_auditor::BEST_AVAILABLE:
		case media_auditor::NONE_NEEDED:
			osd_printf_info("is best available\n");
			++correct;
			return;

		case media_auditor::NOTFOUND:
			osd_printf_info("not found\n");
			return;
		}
		assert(false);
		osd_printf_error("has unknown status (%u)\n", unsigned(summary));
	}
}

} // anonymous namespace


//**************************************************************************
//  CLI FRONTEND
//**************************************************************************

//-------------------------------------------------
//  cli_frontend - constructor
//-------------------------------------------------

cli_frontend::cli_frontend(emu_options &options, osd_interface &osd)
	: m_options(options)
	, m_osd(osd)
	, m_result(EMU_ERR_NONE)
{
	m_options.add_entries(cli_option_entries);
}


//-------------------------------------------------
//  ~cli_frontend - destructor
//-------------------------------------------------

cli_frontend::~cli_frontend()
{
}

void cli_frontend::start_execution(mame_machine_manager *manager, const std::vector<std::string> &args)
{
	std::ostringstream option_errors;

	// because softlist evaluation relies on hashpath being populated, we are going to go through
	// a special step to force it to be evaluated
	mame_options::populate_hashpath_from_args_and_inis(m_options, args);

	// parse the command line, adding any system-specific options
	try
	{
		m_options.parse_command_line(args, OPTION_PRIORITY_CMDLINE);
		m_osd.set_verbose(m_options.verbose());
	}
	catch (options_exception &ex)
	{
		// if we failed, check for no command and a system name first; in that case error on the name
		if (m_options.command().empty() && mame_options::system(m_options) == nullptr && !m_options.attempted_system_name().empty())
			throw emu_fatalerror(EMU_ERR_NO_SUCH_SYSTEM, "Unknown system '%s'", m_options.attempted_system_name());

		// otherwise, error on the options
		throw emu_fatalerror(EMU_ERR_INVALID_CONFIG, "%s", ex.message());
	}

	// determine the base name of the EXE
	std::string exename = core_filename_extract_base(args[0], true);

	// if we have a command, execute that
	if (!m_options.command().empty())
	{
		execute_commands(exename.c_str());
		return;
	}

	// read INI's, if appropriate
	if (m_options.read_config())
	{
		mame_options::parse_standard_inis(m_options, option_errors);
		m_osd.set_verbose(m_options.verbose());
	}

	// otherwise, check for a valid system
	load_translation(m_options);

	manager->start_http_server();

	manager->start_luaengine();

	if (option_errors.tellp() > 0)
	{
		std::string option_errors_string = option_errors.str();
		osd_printf_error("Error in command line:\n%s\n", strtrimspace(option_errors_string));
	}

	// if we can't find it, give an appropriate error
	const game_driver *system = mame_options::system(m_options);
	if (system == nullptr && *(m_options.system_name()) != 0)
		throw emu_fatalerror(EMU_ERR_NO_SUCH_SYSTEM, "Unknown system '%s'", m_options.system_name());

	// otherwise just run the game
	m_result = manager->execute();
}

//-------------------------------------------------
//  execute - execute a game via the standard
//  command line interface
//-------------------------------------------------

int cli_frontend::execute(std::vector<std::string> &args)
{
	// wrap the core execution in a try/catch to field all fatal errors
	m_result = EMU_ERR_NONE;
	mame_machine_manager *manager = mame_machine_manager::instance(m_options, m_osd);

	try
	{
		start_execution(manager, args);
	}
	// handle exceptions of various types
	catch (emu_fatalerror &fatal)
	{
		std::string str(fatal.what());
		strtrimspace(str);
		osd_printf_error("%s\n", str);
		m_result = (fatal.exitcode() != 0) ? fatal.exitcode() : EMU_ERR_FATALERROR;

		// if a game was specified, wasn't a wildcard, and our error indicates this was the
		// reason for failure, offer some suggestions
		if (m_result == EMU_ERR_NO_SUCH_SYSTEM
			&& !m_options.attempted_system_name().empty()
			&& !core_iswildstr(m_options.attempted_system_name().c_str())
			&& mame_options::system(m_options) == nullptr)
		{
			// get the top 16 approximate matches
			driver_enumerator drivlist(m_options);
			int matches[16];
			drivlist.find_approximate_matches(m_options.attempted_system_name(), ARRAY_LENGTH(matches), matches);

			// work out how wide the titles need to be
			int titlelen(0);
			for (int match : matches)
				if (0 <= match)
					titlelen = (std::max)(titlelen, int(strlen(drivlist.driver(match).type.fullname())));

			// print them out
			osd_printf_error("\n\"%s\" approximately matches the following\n"
					"supported machines (best match first):\n\n", m_options.attempted_system_name());
			for (int match : matches)
			{
				if (0 <= match)
				{
					game_driver const &drv(drivlist.driver(match));
					osd_printf_error("%-18s%-*s(%s, %s)\n", drv.name, titlelen + 2, drv.type.fullname(), drv.manufacturer, drv.year);
				}
			}
		}
	}
	catch (emu_exception &)
	{
		osd_printf_error("Caught unhandled emulator exception\n");
		m_result = EMU_ERR_FATALERROR;
	}
	catch (tag_add_exception &aex)
	{
		osd_printf_error("Tag '%s' already exists in tagged map\n", aex.tag());
		m_result = EMU_ERR_FATALERROR;
	}
	catch (std::exception &ex)
	{
		osd_printf_error("Caught unhandled %s exception: %s\n", typeid(ex).name(), ex.what());
		m_result = EMU_ERR_FATALERROR;
	}
	catch (...)
	{
		osd_printf_error("Caught unhandled exception\n");
		m_result = EMU_ERR_FATALERROR;
	}

	util::archive_file::cache_clear();
	delete manager;

	return m_result;
}


//-------------------------------------------------
//  listxml - output the XML data for one or more
//  games
//-------------------------------------------------

void cli_frontend::listxml(const std::vector<std::string> &args)
{
	// create the XML and print it to stdout
	info_xml_creator creator(m_options, m_options.bool_value(CLIOPTION_DTD));
	creator.output(std::cout, args);
}


//-------------------------------------------------
//  listfull - output the name and description of
//  one or more games
//-------------------------------------------------

void cli_frontend::listfull(const std::vector<std::string> &args)
{
	auto const list_system_name = [] (device_type type, bool first)
	{
		// print the header
		if (first)
			osd_printf_info("Name:             Description:\n");

		osd_printf_info("%-17s \"%s\"\n", type.shortname(), type.fullname());
	};
	apply_action(
			args,
			[&list_system_name] (driver_enumerator &drivlist, bool first)
			{ list_system_name(drivlist.driver().type, first); },
			[&list_system_name] (device_type type, bool first)
			{ list_system_name(type, first); });
}


//-------------------------------------------------
//  listsource - output the name and source
//  filename of one or more games
//-------------------------------------------------

void cli_frontend::listsource(const std::vector<std::string> &args)
{
	auto const list_system_source = [] (device_type type)
	{
		osd_printf_info("%-16s %s\n", type.shortname(), core_filename_extract_base(type.source()));
	};
	apply_action(
			args,
			[&list_system_source] (driver_enumerator &drivlist, bool first)
			{ list_system_source(drivlist.driver().type); },
			[&list_system_source] (device_type type, bool first)
			{ list_system_source(type); });
}


//-------------------------------------------------
//  listclones - output the name and parent of all
//  clones matching the given pattern
//-------------------------------------------------

void cli_frontend::listclones(const std::vector<std::string> &args)
{
	const char *gamename = args.empty() ? nullptr : args[0].c_str();

	// start with a filtered list of drivers
	driver_enumerator drivlist(m_options, gamename);
	int const original_count = drivlist.count();

	// iterate through the remaining ones to see if their parent matches
	while (drivlist.next_excluded())
	{
		// if we have a non-bios clone and it matches, keep it
		int const clone_of = drivlist.clone();
		if ((clone_of >= 0) && !(drivlist.driver(clone_of).flags & machine_flags::IS_BIOS_ROOT))
			if (drivlist.matches(gamename, drivlist.driver(clone_of).name))
				drivlist.include();
	}

	// return an error if none found
	if (drivlist.count() == 0)
	{
		// see if we match but just weren't a clone
		if (original_count == 0)
			throw emu_fatalerror(EMU_ERR_NO_SUCH_SYSTEM, "No matching systems found for '%s'", gamename);
		else
			osd_printf_info("Found %lu match(es) for '%s' but none were clones\n", (unsigned long)drivlist.count(), gamename); // FIXME: this never gets hit
		return;
	}

	// print the header
	osd_printf_info("Name:            Clone of:\n");

	// iterate through drivers and output the info
	drivlist.reset();
	while (drivlist.next())
	{
		int clone_of = drivlist.clone();
		if ((clone_of >= 0) && !(drivlist.driver(clone_of).flags & machine_flags::IS_BIOS_ROOT))
			osd_printf_info("%-16s %s\n", drivlist.driver().name, drivlist.driver(clone_of).name);
	}
}


//-------------------------------------------------
//  listbrothers - for each matching game, output
//  the list of other games that share the same
//  source file
//-------------------------------------------------

void cli_frontend::listbrothers(const std::vector<std::string> &args)
{
	const char *gamename = args.empty() ? nullptr : args[0].c_str();

	// start with a filtered list of drivers; return an error if none found
	driver_enumerator initial_drivlist(m_options, gamename);
	if (initial_drivlist.count() == 0)
		throw emu_fatalerror(EMU_ERR_NO_SUCH_SYSTEM, "No matching systems found for '%s'", gamename);

	// for the final list, start with an empty driver list
	driver_enumerator drivlist(m_options);
	drivlist.exclude_all();

	// scan through the initially-selected drivers
	while (initial_drivlist.next())
	{
		// if we are already marked in the final list, we don't need to do anything
		if (drivlist.included(initial_drivlist.current()))
			continue;

		// otherwise, walk excluded items in the final list and mark any that match
		drivlist.reset();
		while (drivlist.next_excluded())
			if (strcmp(drivlist.driver().type.source(), initial_drivlist.driver().type.source()) == 0)
				drivlist.include();
	}

	// print the header
	osd_printf_info("%-20s %-16s %s\n", "Source file:", "Name:", "Parent:");

	// output the entries found
	drivlist.reset();
	while (drivlist.next())
	{
		int clone_of = drivlist.clone();
		if (clone_of != -1)
			osd_printf_info("%-20s %-16s %s\n", core_filename_extract_base(drivlist.driver().type.source()), drivlist.driver().name, (clone_of == -1 ? "" : drivlist.driver(clone_of).name));
		else
			osd_printf_info("%-20s %s\n", core_filename_extract_base(drivlist.driver().type.source()), drivlist.driver().name);
	}
}


//-------------------------------------------------
//  listcrc - output the CRC and name of all ROMs
//  referenced by the emulator
//-------------------------------------------------

void cli_frontend::listcrc(const std::vector<std::string> &args)
{
	apply_device_action(
			args,
			[] (device_t &root, char const *type, bool first)
			{
				for (device_t const &device : device_enumerator(root))
				{
					for (tiny_rom_entry const *rom = device.rom_region(); rom && !ROMENTRY_ISEND(rom); ++rom)
					{
						if (ROMENTRY_ISFILE(rom))
						{
							// if we have a CRC, display it
							uint32_t crc;
							if (util::hash_collection(rom->hashdata).crc(crc))
								osd_printf_info("%08x %-32s\t%-16s\t%s\n", crc, rom->name, device.shortname(), device.name());
						}
					}
				}
			});
}


//-------------------------------------------------
//  listroms - output the list of ROMs referenced
//  by matching systems/devices
//-------------------------------------------------

void cli_frontend::listroms(const std::vector<std::string> &args)
{
	apply_device_action(
			args,
			[] (device_t &root, char const *type, bool first)
			{
				// space between items
				if (!first)
					osd_printf_info("\n");

				// iterate through ROMs
				bool hasroms = false;
				for (device_t const &device : device_enumerator(root))
				{
					for (const rom_entry *region = rom_first_region(device); region; region = rom_next_region(region))
					{
						for (const rom_entry *rom = rom_first_file(region); rom; rom = rom_next_file(rom))
						{
							// print a header
							if (!hasroms)
								osd_printf_info(
									"ROMs required for %s \"%s\".\n"
									"%-32s %10s %s\n",
									type, root.shortname(), "Name", "Size", "Checksum");
							hasroms = true;

							// accumulate the total length of all chunks
							int64_t length = -1;
							if (ROMREGION_ISROMDATA(region))
								length = rom_file_size(rom);

							// start with the name
							const char *name = ROM_GETNAME(rom);
							osd_printf_info("%-32s ", name);

							// output the length next
							if (length >= 0)
								osd_printf_info("%10u", unsigned(uint64_t(length)));
							else
								osd_printf_info("%10s", "");

							// output the hash data
							util::hash_collection hashes(ROM_GETHASHDATA(rom));
							if (!hashes.flag(util::hash_collection::FLAG_NO_DUMP))
							{
								if (hashes.flag(util::hash_collection::FLAG_BAD_DUMP))
									osd_printf_info(" BAD");
								osd_printf_info(" %s", hashes.macro_string());
							}
							else
								osd_printf_info(" NO GOOD DUMP KNOWN");

							// end with a CR
							osd_printf_info("\n");
						}
					}
				}
				if (!hasroms)
					osd_printf_info("No ROMs required for %s \"%s\".\n", type, root.shortname());
			});
}


//-------------------------------------------------
//  listsamples - output the list of samples
//  referenced by a given game or set of games
//-------------------------------------------------

void cli_frontend::listsamples(const std::vector<std::string> &args)
{
	const char *gamename = args.empty() ? nullptr : args[0].c_str();

	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(EMU_ERR_NO_SUCH_SYSTEM, "No matching systems found for '%s'", gamename);

	// iterate over drivers, looking for SAMPLES devices
	bool first = true;
	while (drivlist.next())
	{
		// see if we have samples
		samples_device_enumerator iter(drivlist.config()->root_device());
		if (iter.count() == 0)
			continue;

		// print a header
		if (!first)
			osd_printf_info("\n");
		first = false;
		osd_printf_info("Samples required for driver \"%s\".\n", drivlist.driver().name);

		// iterate over samples devices and print the samples from each one
		for (samples_device &device : iter)
		{
			samples_iterator sampiter(device);
			for (const char *samplename = sampiter.first(); samplename != nullptr; samplename = sampiter.next())
				osd_printf_info("%s\n", samplename);
		}
	}
}


//-------------------------------------------------
//  listdevices - output the list of devices
//  referenced by a given game or set of games
//-------------------------------------------------

void cli_frontend::listdevices(const std::vector<std::string> &args)
{
	const char *gamename = args.empty() ? nullptr : args[0].c_str();

	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(EMU_ERR_NO_SUCH_SYSTEM, "No matching systems found for '%s'", gamename);

	// iterate over drivers, looking for SAMPLES devices
	bool first = true;
	while (drivlist.next())
	{
		// print a header
		if (!first)
			printf("\n");
		first = false;
		printf("Driver %s (%s):\n", drivlist.driver().name, drivlist.driver().type.fullname());

		// build a list of devices
		std::vector<device_t *> device_list;
		for (device_t &device : device_enumerator(drivlist.config()->root_device()))
			device_list.push_back(&device);

		// sort them by tag
		std::sort(device_list.begin(), device_list.end(), [](device_t *dev1, device_t *dev2) {
			// end of string < ':' < '0'
			const char *tag1 = dev1->tag();
			const char *tag2 = dev2->tag();
			while (*tag1 == *tag2 && *tag1 != '\0' && *tag2 != '\0')
			{
				tag1++;
				tag2++;
			}
			return (*tag1 == ':' ? ' ' : *tag1) < (*tag2 == ':' ? ' ' : *tag2);
		});

		// dump the results
		for (auto device : device_list)
		{
			// extract the tag, stripping the leading colon
			const char *tag = device->tag();
			if (*tag == ':')
				tag++;

			// determine the depth
			int depth = 1;
			if (*tag == 0)
			{
				tag = "<root>";
				depth = 0;
			}
			else
			{
				for (const char *c = tag; *c != 0; c++)
					if (*c == ':')
					{
						tag = c + 1;
						depth++;
					}
			}
			printf("   %*s%-*s %s", depth * 2, "", 30 - depth * 2, tag, device->name());

			// add more information
			uint32_t clock = device->clock();
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
	}
}


//-------------------------------------------------
//  listslots - output the list of slot devices
//  referenced by a given game or set of games
//-------------------------------------------------

void cli_frontend::listslots(const std::vector<std::string> &args)
{
	const char *gamename = args.empty() ? nullptr : args[0].c_str();

	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(EMU_ERR_NO_SUCH_SYSTEM, "No matching systems found for '%s'", gamename);

	// print header
	printf("%-16s %-16s %-16s %s\n", "SYSTEM", "SLOT NAME", "SLOT OPTIONS", "SLOT DEVICE NAME");
	printf("%s %s %s %s\n", std::string(16,'-').c_str(), std::string(16,'-').c_str(), std::string(16,'-').c_str(), std::string(28,'-').c_str());

	// iterate over drivers
	while (drivlist.next())
	{
		// iterate
		bool first = true;
		for (const device_slot_interface &slot : slot_interface_enumerator(drivlist.config()->root_device()))
		{
			if (slot.fixed()) continue;

			// build a list of user-selectable options
			std::vector<device_slot_interface::slot_option const *> option_list;
			for (auto &option : slot.option_list())
				if (option.second->selectable())
					option_list.push_back(option.second.get());

			// sort them by name
			std::sort(option_list.begin(), option_list.end(), [](device_slot_interface::slot_option const *opt1, device_slot_interface::slot_option const *opt2) {
				return strcmp(opt1->name(), opt2->name()) < 0;
			});


			// output the line, up to the list of extensions
			printf("%-16s %-16s ", first ? drivlist.driver().name : "", slot.device().tag()+1);

			bool first_option = true;

			// get the options and print them
			for (device_slot_interface::slot_option const *opt : option_list)
			{
				if (first_option)
					printf("%-16s %s\n", opt->name(), opt->devtype().fullname());
				else
					printf("%-34s%-16s %s\n", "", opt->name(), opt->devtype().fullname());

				first_option = false;
			}
			if (first_option)
				printf("%-16s %s\n", "[none]","No options available");
			// end the line
			printf("\n");
			first = false;
		}

		// if we didn't get any at all, just print a none line
		if (first)
			printf("%-16s (none)\n", drivlist.driver().name);
	}
}


//-------------------------------------------------
//  listmedia - output the list of image devices
//  referenced by a given game or set of games
//-------------------------------------------------

void cli_frontend::listmedia(const std::vector<std::string> &args)
{
	const char *gamename = args.empty() ? nullptr : args[0].c_str();

	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(EMU_ERR_NO_SUCH_SYSTEM, "No matching systems found for '%s'", gamename);

	// print header
	printf("%-16s %-16s %-10s %s\n", "SYSTEM", "MEDIA NAME", "(brief)", "IMAGE FILE EXTENSIONS SUPPORTED");
	printf("%s %s-%s %s\n", std::string(16,'-').c_str(), std::string(16,'-').c_str(), std::string(10,'-').c_str(), std::string(31,'-').c_str());

	// iterate over drivers
	while (drivlist.next())
	{
		// iterate
		bool first = true;
		for (const device_image_interface &imagedev : image_interface_enumerator(drivlist.config()->root_device()))
		{
			if (!imagedev.user_loadable())
				continue;

			// extract the shortname with parentheses
			std::string paren_shortname = string_format("(%s)", imagedev.brief_instance_name());

			// output the line, up to the list of extensions
			printf("%-16s %-16s %-10s ", first ? drivlist.driver().name : "", imagedev.instance_name().c_str(), paren_shortname.c_str());

			// get the extensions and print them
			std::string extensions(imagedev.file_extensions());
			for (int start = 0, end = extensions.find_first_of(',');; start = end + 1, end = extensions.find_first_of(',', start))
			{
				std::string curext(extensions, start, (end == -1) ? extensions.length() - start : end - start);
				printf(".%-5s", curext.c_str());
				if (end == -1)
					break;
			}

			// end the line
			printf("\n");
			first = false;
		}

		// if we didn't get any at all, just print a none line
		if (first)
			printf("%-16s (none)\n", drivlist.driver().name);
	}
}

//-------------------------------------------------
//  verifyroms - verify the ROM sets of one or
//  more games
//-------------------------------------------------
void cli_frontend::verifyroms(const std::vector<std::string> &args)
{
	bool const iswild((1U != args.size()) || core_iswildstr(args[0].c_str()));
	std::vector<bool> matched(args.size(), false);
	unsigned matchcount = 0;
	auto const included = [&args, &matched, &matchcount] (char const *name) -> bool
	{
		if (args.empty())
		{
			++matchcount;
			return true;
		}

		bool result = false;
		auto it = matched.begin();
		for (std::string const &pat : args)
		{
			if (!core_strwildcmp(pat.c_str(), name))
			{
				++matchcount;
				result = true;
				*it = true;
			}
			++it;
		}
		return result;
	};

	unsigned correct = 0;
	unsigned incorrect = 0;
	unsigned notfound = 0;

	// iterate over drivers
	driver_enumerator drivlist(m_options);
	media_auditor auditor(drivlist);
	util::ovectorstream summary_string;
	while (drivlist.next())
	{
		if (included(drivlist.driver().name))
		{
			// audit the ROMs in this set
			media_auditor::summary summary = auditor.audit_media(AUDIT_VALIDATE_FAST);

			auto const clone_of = drivlist.clone();
			print_summary(
					auditor, summary, true,
					"rom", drivlist.driver().name, (clone_of >= 0) ? drivlist.driver(clone_of).name : nullptr,
					correct, incorrect, notfound,
					summary_string);

			// if it wasn't a wildcard, there can only be one
			if (!iswild)
				break;
		}
	}

	if (iswild || !matchcount)
	{
		machine_config config(GAME_NAME(___empty), m_options);
		machine_config::token const tok(config.begin_configuration(config.root_device()));
		for (device_type type : registered_device_types)
		{
			if (included(type.shortname()))
			{
				// audit the ROMs in this set
				device_t *const dev = config.device_add("_tmp", type, 0);
				media_auditor::summary summary = auditor.audit_device(*dev, AUDIT_VALIDATE_FAST);

				print_summary(
						auditor, summary, false,
						"rom", dev->shortname(), nullptr,
						correct, incorrect, notfound,
						summary_string);
				config.device_remove("_tmp");

				// if it wasn't a wildcard, there can only be one
				if (!iswild)
					break;
			}
		}
	}

	// clear out any cached files
	util::archive_file::cache_clear();

	// return an error if none found
	auto it = matched.begin();
	for (std::string const &pat : args)
	{
		if (!*it)
			throw emu_fatalerror(EMU_ERR_NO_SUCH_SYSTEM, "No matching systems found for '%s'", pat);

		++it;
	}

	if ((1U == args.size()) && (matchcount > 0) && (correct == 0) && (incorrect == 0))
	{
		// if we didn't get anything at all, display a generic end message
		if (notfound > 0)
			throw emu_fatalerror(EMU_ERR_MISSING_FILES, "romset \"%s\" not found!\n", args[0]);
		else
			throw emu_fatalerror(EMU_ERR_MISSING_FILES, "romset \"%s\" has no roms!\n", args[0]);
	}
	else
	{
		// otherwise, print a summary
		if (incorrect > 0)
			throw emu_fatalerror(EMU_ERR_MISSING_FILES, "%u romsets found, %u were OK.\n", correct + incorrect, correct);
		else
			osd_printf_info("%u romsets found, %u were OK.\n", correct, correct);
	}
}


//-------------------------------------------------
//  info_verifysamples - verify the sample sets of
//  one or more games
//-------------------------------------------------

void cli_frontend::verifysamples(const std::vector<std::string> &args)
{
	const char *gamename = args.empty() ? "*" : args[0].c_str();

	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);

	unsigned correct = 0;
	unsigned incorrect = 0;
	unsigned notfound = 0;
	unsigned matched = 0;

	// iterate over drivers
	media_auditor auditor(drivlist);
	util::ovectorstream summary_string;
	while (drivlist.next())
	{
		matched++;

		// audit the samples in this set
		media_auditor::summary summary = auditor.audit_samples();

		auto const clone_of = drivlist.clone();
		print_summary(
				auditor, summary, false,
				"sample", drivlist.driver().name, (clone_of >= 0) ? drivlist.driver(clone_of).name : nullptr,
				correct, incorrect, notfound,
				summary_string);
	}

	// clear out any cached files
	util::archive_file::cache_clear();

	// return an error if none found
	if (matched == 0)
		throw emu_fatalerror(EMU_ERR_NO_SUCH_SYSTEM, "No matching systems found for '%s'", gamename);

	// if we didn't get anything at all, display a generic end message
	if (matched > 0 && correct == 0 && incorrect == 0)
	{
		if (notfound > 0)
			throw emu_fatalerror(EMU_ERR_MISSING_FILES, "sampleset \"%s\" not found!\n", gamename);
		else
			throw emu_fatalerror(EMU_ERR_MISSING_FILES, "sampleset \"%s\" not required!\n", gamename);
	}

	// otherwise, print a summary
	else
	{
		if (incorrect > 0)
			throw emu_fatalerror(EMU_ERR_MISSING_FILES, "%u samplesets found, %u were OK.\n", correct + incorrect, correct);
		osd_printf_info("%u samplesets found, %u were OK.\n", correct, correct);
	}
}

const char cli_frontend::s_softlist_xml_dtd[] =
				"<?xml version=\"1.0\"?>\n" \
				"<!DOCTYPE softwarelists [\n" \
				"<!ELEMENT softwarelists (softwarelist*)>\n" \
				"\t<!ELEMENT softwarelist (software+)>\n" \
				"\t\t<!ATTLIST softwarelist name CDATA #REQUIRED>\n" \
				"\t\t<!ATTLIST softwarelist description CDATA #IMPLIED>\n" \
				"\t\t<!ELEMENT software (description, year, publisher, info*, sharedfeat*, part*)>\n" \
				"\t\t\t<!ATTLIST software name CDATA #REQUIRED>\n" \
				"\t\t\t<!ATTLIST software cloneof CDATA #IMPLIED>\n" \
				"\t\t\t<!ATTLIST software supported (yes|partial|no) \"yes\">\n" \
				"\t\t\t<!ELEMENT description (#PCDATA)>\n" \
				"\t\t\t<!ELEMENT year (#PCDATA)>\n" \
				"\t\t\t<!ELEMENT publisher (#PCDATA)>\n" \
				"\t\t\t<!ELEMENT info EMPTY>\n" \
				"\t\t\t\t<!ATTLIST info name CDATA #REQUIRED>\n" \
				"\t\t\t\t<!ATTLIST info value CDATA #IMPLIED>\n" \
				"\t\t\t<!ELEMENT sharedfeat EMPTY>\n" \
				"\t\t\t\t<!ATTLIST sharedfeat name CDATA #REQUIRED>\n" \
				"\t\t\t\t<!ATTLIST sharedfeat value CDATA #IMPLIED>\n" \
				"\t\t\t<!ELEMENT part (feature*, dataarea*, diskarea*, dipswitch*)>\n" \
				"\t\t\t\t<!ATTLIST part name CDATA #REQUIRED>\n" \
				"\t\t\t\t<!ATTLIST part interface CDATA #REQUIRED>\n" \
				"\t\t\t\t<!ELEMENT feature EMPTY>\n" \
				"\t\t\t\t\t<!ATTLIST feature name CDATA #REQUIRED>\n" \
				"\t\t\t\t\t<!ATTLIST feature value CDATA #IMPLIED>\n" \
				"\t\t\t\t<!ELEMENT dataarea (rom*)>\n" \
				"\t\t\t\t\t<!ATTLIST dataarea name CDATA #REQUIRED>\n" \
				"\t\t\t\t\t<!ATTLIST dataarea size CDATA #REQUIRED>\n" \
				"\t\t\t\t\t<!ATTLIST dataarea databits (8|16|32|64) \"8\">\n" \
				"\t\t\t\t\t<!ATTLIST dataarea endian (big|little) \"little\">\n" \
				"\t\t\t\t\t<!ELEMENT rom EMPTY>\n" \
				"\t\t\t\t\t\t<!ATTLIST rom name CDATA #IMPLIED>\n" \
				"\t\t\t\t\t\t<!ATTLIST rom size CDATA #IMPLIED>\n" \
				"\t\t\t\t\t\t<!ATTLIST rom length CDATA #IMPLIED>\n" \
				"\t\t\t\t\t\t<!ATTLIST rom crc CDATA #IMPLIED>\n" \
				"\t\t\t\t\t\t<!ATTLIST rom sha1 CDATA #IMPLIED>\n" \
				"\t\t\t\t\t\t<!ATTLIST rom offset CDATA #IMPLIED>\n" \
				"\t\t\t\t\t\t<!ATTLIST rom value CDATA #IMPLIED>\n" \
				"\t\t\t\t\t\t<!ATTLIST rom status (baddump|nodump|good) \"good\">\n" \
				"\t\t\t\t\t\t<!ATTLIST rom loadflag (load16_byte|load16_word|load16_word_swap|load32_byte|load32_word|load32_word_swap|load32_dword|load64_word|load64_word_swap|reload|fill|continue|reload_plain) #IMPLIED>\n" \
				"\t\t\t\t<!ELEMENT diskarea (disk*)>\n" \
				"\t\t\t\t\t<!ATTLIST diskarea name CDATA #REQUIRED>\n" \
				"\t\t\t\t\t<!ELEMENT disk EMPTY>\n" \
				"\t\t\t\t\t\t<!ATTLIST disk name CDATA #REQUIRED>\n" \
				"\t\t\t\t\t\t<!ATTLIST disk sha1 CDATA #IMPLIED>\n" \
				"\t\t\t\t\t\t<!ATTLIST disk status (baddump|nodump|good) \"good\">\n" \
				"\t\t\t\t\t\t<!ATTLIST disk writeable (yes|no) \"no\">\n" \
				"\t\t\t\t<!ELEMENT dipswitch (dipvalue*)>\n" \
				"\t\t\t\t\t<!ATTLIST dipswitch name CDATA #REQUIRED>\n" \
				"\t\t\t\t\t<!ATTLIST dipswitch tag CDATA #REQUIRED>\n" \
				"\t\t\t\t\t<!ATTLIST dipswitch mask CDATA #REQUIRED>\n" \
				"\t\t\t\t\t<!ELEMENT dipvalue EMPTY>\n" \
				"\t\t\t\t\t\t<!ATTLIST dipvalue name CDATA #REQUIRED>\n" \
				"\t\t\t\t\t\t<!ATTLIST dipvalue value CDATA #REQUIRED>\n" \
				"\t\t\t\t\t\t<!ATTLIST dipvalue default (yes|no) \"no\">\n" \
				"]>\n\n";

void cli_frontend::output_single_softlist(std::ostream &out, software_list_device &swlistdev)
{
	util::stream_format(out, "\t<softwarelist name=\"%s\" description=\"%s\">\n", swlistdev.list_name(), util::xml::normalize_string(swlistdev.description().c_str()));
	for (const software_info &swinfo : swlistdev.get_info())
	{
		util::stream_format(out, "\t\t<software name=\"%s\"", util::xml::normalize_string(swinfo.shortname().c_str()));
		if (!swinfo.parentname().empty())
			util::stream_format(out, " cloneof=\"%s\"", util::xml::normalize_string(swinfo.parentname().c_str()));
		if (swinfo.supported() == SOFTWARE_SUPPORTED_PARTIAL)
			out << " supported=\"partial\"";
		if (swinfo.supported() == SOFTWARE_SUPPORTED_NO)
			out << " supported=\"no\"";
		out << ">\n";
		util::stream_format(out, "\t\t\t<description>%s</description>\n", util::xml::normalize_string(swinfo.longname().c_str()));
		util::stream_format(out, "\t\t\t<year>%s</year>\n", util::xml::normalize_string(swinfo.year().c_str()));
		util::stream_format(out, "\t\t\t<publisher>%s</publisher>\n", util::xml::normalize_string(swinfo.publisher().c_str()));

		for (const feature_list_item &flist : swinfo.other_info())
			util::stream_format(out, "\t\t\t<info name=\"%s\" value=\"%s\"/>\n", flist.name().c_str(), util::xml::normalize_string(flist.value().c_str()));

		for (const software_part &part : swinfo.parts())
		{
			util::stream_format(out, "\t\t\t<part name=\"%s\"", util::xml::normalize_string(part.name().c_str()));
			if (!part.interface().empty())
				util::stream_format(out, " interface=\"%s\"", util::xml::normalize_string(part.interface().c_str()));

			out << ">\n";

			for (const feature_list_item &flist : part.featurelist())
				util::stream_format(out, "\t\t\t\t<feature name=\"%s\" value=\"%s\" />\n", flist.name().c_str(), util::xml::normalize_string(flist.value().c_str()));

			// TODO: display ROM region information
			for (const rom_entry *region = part.romdata().data(); region; region = rom_next_region(region))
			{
				int is_disk = ROMREGION_ISDISKDATA(region);

				if (!is_disk)
					util::stream_format(out, "\t\t\t\t<dataarea name=\"%s\" size=\"%d\">\n", util::xml::normalize_string(ROMREGION_GETTAG(region)), ROMREGION_GETLENGTH(region));
				else
					util::stream_format(out, "\t\t\t\t<diskarea name=\"%s\">\n", util::xml::normalize_string(ROMREGION_GETTAG(region)));

				for (const rom_entry *rom = rom_first_file(region); rom && !ROMENTRY_ISREGIONEND(rom); rom++)
				{
					if (ROMENTRY_ISFILE(rom))
					{
						if (!is_disk)
							util::stream_format(out, "\t\t\t\t\t<rom name=\"%s\" size=\"%d\"", util::xml::normalize_string(ROM_GETNAME(rom)), rom_file_size(rom));
						else
							util::stream_format(out, "\t\t\t\t\t<disk name=\"%s\"", util::xml::normalize_string(ROM_GETNAME(rom)));

						// dump checksum information only if there is a known dump
						util::hash_collection hashes(ROM_GETHASHDATA(rom));
						if (!hashes.flag(util::hash_collection::FLAG_NO_DUMP))
							util::stream_format(out, " %s", hashes.attribute_string());
						else
							out << " status=\"nodump\"";

						if (is_disk)
							util::stream_format(out, " writeable=\"%s\"", (ROM_GETFLAGS(rom) & DISK_READONLYMASK) ? "no" : "yes");

						if ((ROM_GETFLAGS(rom) & ROM_SKIPMASK) == ROM_SKIP(1))
							out << " loadflag=\"load16_byte\"";

						if ((ROM_GETFLAGS(rom) & ROM_SKIPMASK) == ROM_SKIP(3))
							out << " loadflag=\"load32_byte\"";

						if (((ROM_GETFLAGS(rom) & ROM_SKIPMASK) == ROM_SKIP(2)) && ((ROM_GETFLAGS(rom) & ROM_GROUPMASK) == ROM_GROUPWORD))
						{
							if (!(ROM_GETFLAGS(rom) & ROM_REVERSEMASK))
								out << " loadflag=\"load32_word\"";
							else
								out << " loadflag=\"load32_word_swap\"";
						}

						if (((ROM_GETFLAGS(rom) & ROM_SKIPMASK) == ROM_SKIP(6)) && ((ROM_GETFLAGS(rom) & ROM_GROUPMASK) == ROM_GROUPWORD))
						{
							if (!(ROM_GETFLAGS(rom) & ROM_REVERSEMASK))
								out << " loadflag=\"load64_word\"";
							else
								out << " loadflag=\"load64_word_swap\"";
						}

						if (((ROM_GETFLAGS(rom) & ROM_SKIPMASK) == ROM_NOSKIP) && ((ROM_GETFLAGS(rom) & ROM_GROUPMASK) == ROM_GROUPWORD))
						{
							if (!(ROM_GETFLAGS(rom) & ROM_REVERSEMASK))
								out << " loadflag=\"load32_dword\"";
							else
								out << " loadflag=\"load16_word_swap\"";
						}

						out << "/>\n";
					}
					else if (ROMENTRY_ISRELOAD(rom))
					{
						util::stream_format(out, "\t\t\t\t\t<rom size=\"%d\" offset=\"0x%x\" loadflag=\"reload\" />\n", ROM_GETLENGTH(rom), ROM_GETOFFSET(rom));
					}
					else if (ROMENTRY_ISFILL(rom))
					{
						util::stream_format(out, "\t\t\t\t\t<rom size=\"%d\" offset=\"0x%x\" loadflag=\"fill\" />\n", ROM_GETLENGTH(rom), ROM_GETOFFSET(rom));
					}
				}

				if (!is_disk)
					out << "\t\t\t\t</dataarea>\n";
				else
					out << "\t\t\t\t</diskarea>\n";
			}

			out << "\t\t\t</part>\n";
		}

		out << "\t\t</software>\n";
	}
	out << "\t</softwarelist>\n";
}


/*-------------------------------------------------
    info_listsoftware - output the list of
    software supported by a given game or set of
    games
    TODO: Add all information read from the source files
-------------------------------------------------*/

void cli_frontend::listsoftware(const std::vector<std::string> &args)
{
	std::unordered_set<std::string> list_map;
	bool firstlist(true);
	apply_device_action(
			args,
			[this, &list_map, &firstlist] (device_t &root, char const *type, bool first)
			{
				for (software_list_device &swlistdev : software_list_device_enumerator(root))
				{
					if (list_map.insert(swlistdev.list_name()).second)
					{
						if (!swlistdev.get_info().empty())
						{
							if (firstlist)
							{
								if (m_options.bool_value(CLIOPTION_DTD))
									std::cout << s_softlist_xml_dtd;
								std::cout << "<softwarelists>\n";
								firstlist = false;
							}
							output_single_softlist(std::cout, swlistdev);
						}
					}
				}
			});

	if (!firstlist)
		std::cout << "</softwarelists>\n";
	else
		fprintf(stdout, "No software lists found for this system\n"); // TODO: should this go to stderr instead?
}


/*-------------------------------------------------
    verifysoftware - verify ROMs from the software
    list of the specified driver(s)
-------------------------------------------------*/
void cli_frontend::verifysoftware(const std::vector<std::string> &args)
{
	const char *gamename = args.empty() ? "*" : args[0].c_str();

	std::unordered_set<std::string> list_map;

	unsigned correct = 0;
	unsigned incorrect = 0;
	unsigned notfound = 0;
	unsigned matched = 0;
	unsigned nrlists = 0;

	// determine which drivers to process; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(EMU_ERR_NO_SUCH_SYSTEM, "No matching systems found for '%s'", gamename);

	media_auditor auditor(drivlist);
	util::ovectorstream summary_string;
	while (drivlist.next())
	{
		matched++;

		for (software_list_device &swlistdev : software_list_device_enumerator(drivlist.config()->root_device()))
		{
			if (swlistdev.is_original())
			{
				if (list_map.insert(swlistdev.list_name()).second)
				{
					if (!swlistdev.get_info().empty())
					{
						nrlists++;
						for (const software_info &swinfo : swlistdev.get_info())
						{
							media_auditor::summary summary = auditor.audit_software(swlistdev, swinfo, AUDIT_VALIDATE_FAST);

							print_summary(
									auditor, summary, false,
									"rom", util::string_format("%s:%s", swlistdev.list_name(), swinfo.shortname()).c_str(), nullptr,
									correct, incorrect, notfound,
									summary_string);
						}
					}
				}
			}
		}
	}

	// clear out any cached files
	util::archive_file::cache_clear();

	// return an error if none found
	if (matched == 0)
		throw emu_fatalerror(EMU_ERR_NO_SUCH_SYSTEM, "No matching systems found for '%s'", gamename);

	// if we didn't get anything at all, display a generic end message
	if (matched > 0 && correct == 0 && incorrect == 0)
	{
		throw emu_fatalerror(EMU_ERR_MISSING_FILES, "romset \"%s\" has no software entries defined!\n", gamename);
	}
	// otherwise, print a summary
	else
	{
		if (incorrect > 0)
			throw emu_fatalerror(EMU_ERR_MISSING_FILES, "%u romsets found in %u software lists, %u were OK.\n", correct + incorrect, nrlists, correct);
		osd_printf_info("%u romsets found in %u software lists, %u romsets were OK.\n", correct, nrlists, correct);
	}

}


/*-------------------------------------------------
    getsoftlist - retrieve software list by name
-------------------------------------------------*/

void cli_frontend::getsoftlist(const std::vector<std::string> &args)
{
	const char *gamename = args.empty() ? "*" : args[0].c_str();

	std::unordered_set<std::string> list_map;
	bool firstlist(true);
	apply_device_action(
			std::vector<std::string>(),
			[this, gamename, &list_map, &firstlist] (device_t &root, char const *type, bool first)
			{
				for (software_list_device &swlistdev : software_list_device_enumerator(root))
				{
					if (core_strwildcmp(gamename, swlistdev.list_name().c_str()) == 0 && list_map.insert(swlistdev.list_name()).second)
					{
						if (!swlistdev.get_info().empty())
						{
							if (firstlist)
							{
								if (m_options.bool_value(CLIOPTION_DTD))
									std::cout << s_softlist_xml_dtd;
								std::cout << "<softwarelists>\n";
								firstlist = false;
							}
							output_single_softlist(std::cout, swlistdev);
						}
					}
				}
			});

	if (!firstlist)
		std::cout << "</softwarelists>\n";
	else
		fprintf(stdout, "No such software lists found\n"); // TODO: should this go to stderr instead?
}


/*-------------------------------------------------
    verifysoftlist - verify software list by name
-------------------------------------------------*/
void cli_frontend::verifysoftlist(const std::vector<std::string> &args)
{
	const char *gamename = args.empty() ? "*" : args[0].c_str();

	std::unordered_set<std::string> list_map;
	unsigned correct = 0;
	unsigned incorrect = 0;
	unsigned notfound = 0;
	unsigned matched = 0;

	driver_enumerator drivlist(m_options);
	media_auditor auditor(drivlist);
	util::ovectorstream summary_string;

	while (drivlist.next())
	{
		for (software_list_device &swlistdev : software_list_device_enumerator(drivlist.config()->root_device()))
		{
			if (core_strwildcmp(gamename, swlistdev.list_name().c_str()) == 0 && list_map.insert(swlistdev.list_name()).second)
			{
				if (!swlistdev.get_info().empty())
				{
					matched++;

					// Get the actual software list contents
					for (const software_info &swinfo : swlistdev.get_info())
					{
						media_auditor::summary summary = auditor.audit_software(swlistdev, swinfo, AUDIT_VALIDATE_FAST);

						print_summary(
								auditor, summary, false,
								"rom", util::string_format("%s:%s", swlistdev.list_name(), swinfo.shortname()).c_str(), nullptr,
								correct, incorrect, notfound,
								summary_string);
					}
				}
			}
		}
	}

	// clear out any cached files
	util::archive_file::cache_clear();

	// return an error if none found
	if (matched == 0)
		throw emu_fatalerror(EMU_ERR_NO_SUCH_SYSTEM, "No matching software lists found for '%s'", gamename);

	// if we didn't get anything at all, display a generic end message
	if (matched > 0 && correct == 0 && incorrect == 0)
	{
		throw emu_fatalerror(EMU_ERR_MISSING_FILES, "no romsets found for software list \"%s\"!\n", gamename);
	}
	// otherwise, print a summary
	else
	{
		if (incorrect > 0)
			throw emu_fatalerror(EMU_ERR_MISSING_FILES, "%u romsets found in %u software lists, %u were OK.\n", correct + incorrect, matched, correct);
		osd_printf_info("%u romsets found in %u software lists, %u romsets were OK.\n", correct, matched, correct);
	}
}


//-------------------------------------------------
//  version - emit MAME version to stdout
//-------------------------------------------------

void cli_frontend::version(const std::vector<std::string> &args)
{
	osd_printf_info("%s", emulator_info::get_build_version());
}


//-------------------------------------------------
//  romident - identify ROMs by looking for
//  matches in our internal database
//-------------------------------------------------

void cli_frontend::romident(const std::vector<std::string> &args)
{
	const char *filename = args[0].c_str();

	// create our own copy of options for the purposes of ROM identification
	// so we are not "polluted" with driver-specific slot/image options
	emu_options options;
	options.set_value(OPTION_MEDIAPATH, m_options.media_path(), OPTION_PRIORITY_DEFAULT);

	media_identifier ident(options);

	// identify the file, then output results
	osd_printf_info("Identifying %s....\n", filename);
	ident.identify(filename);

	// return the appropriate error code
	if (ident.total() == 0)
		throw emu_fatalerror(EMU_ERR_MISSING_FILES, "No files found.\n");
	else if (ident.matches() == ident.total())
		return;
	else if (ident.matches() == ident.total() - ident.nonroms())
		throw emu_fatalerror(EMU_ERR_IDENT_NONROMS, "Out of %d files, %d matched, %d are not roms.\n", ident.total(), ident.matches(), ident.nonroms());
	else if (ident.matches() > 0)
		throw emu_fatalerror(EMU_ERR_IDENT_PARTIAL, "Out of %d files, %d matched, %d did not match.\n", ident.total(), ident.matches(), ident.total() - ident.matches());
	else
		throw emu_fatalerror(EMU_ERR_IDENT_NONE, "No roms matched.\n");
}


//-------------------------------------------------
//  apply_action - apply action to matching
//  systems/devices
//-------------------------------------------------

template <typename T, typename U> void cli_frontend::apply_action(const std::vector<std::string> &args, T &&drvact, U &&devact)

{
	bool const iswild((1U != args.size()) || core_iswildstr(args[0].c_str()));
	std::vector<bool> matched(args.size(), false);
	auto const included = [&args, &matched] (char const *name) -> bool
	{
		if (args.empty())
			return true;

		bool result = false;
		auto it = matched.begin();
		for (std::string const &pat : args)
		{
			if (!core_strwildcmp(pat.c_str(), name))
			{
				result = true;
				*it = true;
			}
			++it;
		}
		return result;
	};

	// determine which drivers to output
	driver_enumerator drivlist(m_options);

	// iterate through matches
	bool first(true);
	while (drivlist.next())
	{
		if (included(drivlist.driver().name))
		{
			drvact(drivlist, first);
			first = false;

			// if it wasn't a wildcard, there can only be one
			if (!iswild)
				break;
		}
	}

	if (iswild || first)
	{
		for (device_type type : registered_device_types)
		{
			if (included(type.shortname()))
			{
				devact(type, first);
				first = false;

				// if it wasn't a wildcard, there can only be one
				if (!iswild)
					break;
			}
		}
	}

	// return an error if none found
	auto it = matched.begin();
	for (std::string const &pat : args)
	{
		if (!*it)
			throw emu_fatalerror(EMU_ERR_NO_SUCH_SYSTEM, "No matching systems found for '%s'", pat);

		++it;
	}
}


//-------------------------------------------------
//  apply_device_action - apply action to matching
//  systems/devices
//-------------------------------------------------

template <typename T> void cli_frontend::apply_device_action(const std::vector<std::string> &args, T &&action)
{
	machine_config config(GAME_NAME(___empty), m_options);
	machine_config::token const tok(config.begin_configuration(config.root_device()));
	apply_action(
			args,
			[&action] (driver_enumerator &drivlist, bool first)
			{
				action(drivlist.config()->root_device(), "driver", first);
			},
			[&action, &config] (device_type type, bool first)
			{
				device_t *const dev = config.device_add("_tmp", type, 0);
				action(*dev, "device", first);
				config.device_remove("_tmp");
			});
}


//-------------------------------------------------
//  find_command
//-------------------------------------------------

const cli_frontend::info_command_struct *cli_frontend::find_command(const std::string &s)
{
	static const info_command_struct s_info_commands[] =
	{
		{ CLICOMMAND_LISTXML,           0, -1, &cli_frontend::listxml,          "[pattern] ..." },
		{ CLICOMMAND_LISTFULL,          0, -1, &cli_frontend::listfull,         "[pattern] ..." },
		{ CLICOMMAND_LISTSOURCE,        0, -1, &cli_frontend::listsource,       "[system name]" },
		{ CLICOMMAND_LISTCLONES,        0,  1, &cli_frontend::listclones,       "[system name]" },
		{ CLICOMMAND_LISTBROTHERS,      0,  1, &cli_frontend::listbrothers,     "[system name]" },
		{ CLICOMMAND_LISTCRC,           0, -1, &cli_frontend::listcrc,          "[system name]" },
		{ CLICOMMAND_LISTDEVICES,       0,  1, &cli_frontend::listdevices,      "[system name]" },
		{ CLICOMMAND_LISTSLOTS,         0,  1, &cli_frontend::listslots,        "[system name]" },
		{ CLICOMMAND_LISTROMS,          0, -1, &cli_frontend::listroms,         "[pattern] ..." },
		{ CLICOMMAND_LISTSAMPLES,       0,  1, &cli_frontend::listsamples,      "[system name]" },
		{ CLICOMMAND_VERIFYROMS,        0, -1, &cli_frontend::verifyroms,       "[pattern] ..." },
		{ CLICOMMAND_VERIFYSAMPLES,     0,  1, &cli_frontend::verifysamples,    "[system name|*]" },
		{ CLICOMMAND_LISTMEDIA,         0,  1, &cli_frontend::listmedia,        "[system name]" },
		{ CLICOMMAND_LISTSOFTWARE,      0,  1, &cli_frontend::listsoftware,     "[system name]" },
		{ CLICOMMAND_VERIFYSOFTWARE,    0,  1, &cli_frontend::verifysoftware,   "[system name|*]" },
		{ CLICOMMAND_ROMIDENT,          1,  1, &cli_frontend::romident,         "(file or directory path)" },
		{ CLICOMMAND_GETSOFTLIST,       0,  1, &cli_frontend::getsoftlist,      "[system name|*]" },
		{ CLICOMMAND_VERIFYSOFTLIST,    0,  1, &cli_frontend::verifysoftlist,   "[system name|*]" },
		{ CLICOMMAND_VERSION,           0,  0, &cli_frontend::version,          "" }
	};

	for (const auto &info_command : s_info_commands)
	{
		if (s == info_command.option)
			return &info_command;
	}
	return nullptr;
}


//-------------------------------------------------
//  execute_commands - execute various frontend
//  commands
//-------------------------------------------------

void cli_frontend::execute_commands(const char *exename)
{
	// help?
	if (m_options.command() == CLICOMMAND_HELP)
	{
		display_help(exename);
		return;
	}

	// showusage?
	if (m_options.command() == CLICOMMAND_SHOWUSAGE)
	{
		osd_printf_info("Usage:  %s [machine] [media] [software] [options]",exename);
		osd_printf_info("\n\nOptions:\n%s", m_options.output_help());
		return;
	}

	// validate?
	if (m_options.command() == CLICOMMAND_VALIDATE)
	{
		if (m_options.command_arguments().size() > 1)
		{
			osd_printf_error("Auxiliary verb -validate takes at most 1 argument\n");
			return;
		}
		validity_checker valid(m_options, false);
		const char *sysname = m_options.command_arguments().empty() ? nullptr : m_options.command_arguments()[0].c_str();
		bool result = valid.check_all_matching(sysname);
		if (!result)
			throw emu_fatalerror(EMU_ERR_FAILED_VALIDITY, "Validity check failed (%d errors, %d warnings in total)\n", valid.errors(), valid.warnings());
		return;
	}

	// other commands need the INIs parsed
	std::ostringstream option_errors;
	mame_options::parse_standard_inis(m_options,option_errors);
	if (option_errors.tellp() > 0)
		osd_printf_error("%s\n", option_errors.str());

	// createconfig?
	if (m_options.command() == CLICOMMAND_CREATECONFIG)
	{
		// attempt to open the output file
		emu_file file(OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		if (file.open(std::string(emulator_info::get_configname()) + ".ini") != osd_file::error::NONE)
			throw emu_fatalerror("Unable to create file %s.ini\n",emulator_info::get_configname());

		// generate the updated INI
		file.puts(m_options.output_ini());

		ui_options ui_opts;
		emu_file file_ui(OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		if (file_ui.open("ui.ini") != osd_file::error::NONE)
			throw emu_fatalerror("Unable to create file ui.ini\n");

		// generate the updated INI
		file_ui.puts(ui_opts.output_ini());

		plugin_options plugin_opts;
		path_iterator iter(m_options.plugins_path());
		std::string pluginpath;
		while (iter.next(pluginpath))
		{
			osd_subst_env(pluginpath, pluginpath);
			plugin_opts.scan_directory(pluginpath, true);
		}
		emu_file file_plugin(OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		if (file_plugin.open("plugin.ini") != osd_file::error::NONE)
			throw emu_fatalerror("Unable to create file plugin.ini\n");

		// generate the updated INI
		file_plugin.puts(plugin_opts.output_ini());

		return;
	}

	// showconfig?
	if (m_options.command() == CLICOMMAND_SHOWCONFIG)
	{
		// print the INI text
		printf("%s\n", m_options.output_ini().c_str());
		return;
	}

	// all other commands call out to one of the info_commands helpers; first
	// find the command
	const auto *info_command = find_command(m_options.command());
	if (info_command)
	{
		// validate argument count
		const char *error_message = nullptr;
		if (m_options.command_arguments().size() < info_command->min_args)
			error_message = "Auxiliary verb -%s requires at least %d argument(s)\n";
		if ((info_command->max_args >= 0) && (m_options.command_arguments().size() > info_command->max_args))
			error_message = "Auxiliary verb -%s takes at most %d argument(s)\n";
		if (error_message)
		{
			osd_printf_info(error_message, info_command->option, info_command->max_args);
			osd_printf_info("\n");
			osd_printf_info("Usage:  %s -%s %s\n", exename, info_command->option, info_command->usage);
			return;
		}

		// invoke the auxiliary command!
		(this->*info_command->function)(m_options.command_arguments());
		return;
	}

	if (!m_osd.execute_command(m_options.command().c_str()))
		// if we get here, we don't know what has been requested
		throw emu_fatalerror(EMU_ERR_INVALID_CONFIG, "Unknown command '%s' specified", m_options.command());
}


//-------------------------------------------------
//  display_help - display help to standard
//  output
//-------------------------------------------------

void cli_frontend::display_help(const char *exename)
{
	osd_printf_info(
			"%3$s v%2$s\n"
			"%5$s\n"
			"\n"
			"This software reproduces, more or less faithfully, the behaviour of a wide range\n"
			"of machines. But hardware is useless without software, so images of the ROMs and\n"
			"other media which run on that hardware are also required.\n"
			"\n"
			"Usage:  %1$s [machine] [media] [software] [options]\n"
			"\n"
			"        %1$s -showusage    for a list of options\n"
			"        %1$s -showconfig   to show current configuration in %4$s.ini format\n"
			"        %1$s -listmedia    for a full list of supported media\n"
			"        %1$s -createconfig to create a %4$s.ini file\n"
			"\n"
			"For usage instructions, please visit https://docs.mamedev.org/\n",
			exename,
			build_version,
			emulator_info::get_appname(),
			emulator_info::get_configname(),
			emulator_info::get_copyright_info());
}
