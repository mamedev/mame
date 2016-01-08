// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    clifront.c

    Command-line interface frontend for MAME.

***************************************************************************/

#include "emu.h"
#include "chd.h"
#include "emuopts.h"
#include "jedparse.h"
#include "audit.h"
#include "info.h"
#include "unzip.h"
#include "un7z.h"
#include "validity.h"
#include "sound/samples.h"
#include "cliopts.h"
#include "clifront.h"
#include "xmlfile.h"

#include "drivenum.h"

#include "osdepend.h"
#include "softlist.h"

#include <new>
#include <ctype.h>



// media_identifier class identifies media by hash via a search in
// the driver database
class media_identifier
{
public:
	// construction/destruction
	media_identifier(cli_options &options);

	// getters
	int total() const { return m_total; }
	int matches() const { return m_matches; }
	int nonroms() const { return m_nonroms; }

	// operations
	void reset() { m_total = m_matches = m_nonroms = 0; }
	void identify(const char *name);
	void identify_file(const char *name);
	void identify_data(const char *name, const UINT8 *data, int length);
	int find_by_hash(const hash_collection &hashes, int length);

private:
	// internal state
	driver_enumerator   m_drivlist;
	int                 m_total;
	int                 m_matches;
	int                 m_nonroms;
};


//**************************************************************************
//  CLI FRONTEND
//**************************************************************************

//-------------------------------------------------
//  cli_frontend - constructor
//-------------------------------------------------

cli_frontend::cli_frontend(cli_options &options, osd_interface &osd)
	: m_options(options),
		m_osd(osd),
		m_result(MAMERR_NONE)
{
}


//-------------------------------------------------
//  ~cli_frontend - destructor
//-------------------------------------------------

cli_frontend::~cli_frontend()
{
	// nuke any device options since they will leak memory
	m_options.remove_device_options();
}


//-------------------------------------------------
//  execute - execute a game via the standard
//  command line interface
//-------------------------------------------------

int cli_frontend::execute(int argc, char **argv)
{
	// wrap the core execution in a try/catch to field all fatal errors
	m_result = MAMERR_NONE;
	try
	{
		// first parse options to be able to get software from it
		std::string option_errors;
		m_options.parse_command_line(argc, argv, option_errors);

		m_options.parse_standard_inis(option_errors);

		if (*(m_options.software_name()) != 0)
		{
			const game_driver *system = m_options.system();
			if (system == nullptr && *(m_options.system_name()) != 0)
				throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "Unknown system '%s'", m_options.system_name());

			machine_config config(*system, m_options);
			software_list_device_iterator iter(config.root_device());
			if (iter.count() == 0)
				throw emu_fatalerror(MAMERR_FATALERROR, "Error: unknown option: %s\n", m_options.software_name());

			bool found = false;
			for (software_list_device *swlistdev = iter.first(); swlistdev != nullptr; swlistdev = iter.next())
			{
				software_info *swinfo = swlistdev->find(m_options.software_name());
				if (swinfo != nullptr)
				{
					// loop through all parts
					for (software_part *swpart = swinfo->first_part(); swpart != nullptr; swpart = swpart->next())
					{
						const char *mount = swpart->feature("automount");
						if (swpart->is_compatible(*swlistdev))
						{
							if (mount == nullptr || strcmp(mount,"no") != 0)
							{
								// search for an image device with the right interface
								image_interface_iterator imgiter(config.root_device());
								for (device_image_interface *image = imgiter.first(); image != nullptr; image = imgiter.next())
								{
									const char *interface = image->image_interface();
									if (interface != nullptr)
									{
										if (swpart->matches_interface(interface))
										{
											const char *option = m_options.value(image->brief_instance_name());

											// mount only if not already mounted
											if (*option == 0)
											{
												std::string val;
												strprintf(val, "%s:%s:%s", swlistdev->list_name(), m_options.software_name(), swpart->name());

												// call this in order to set slot devices according to mounting
												m_options.parse_slot_devices(argc, argv, option_errors, image->instance_name(), val.c_str());
												break;
											}
										}
									}
								}
							}
							found = true;
						}
					}
				}

				if (found)
					break;
			}
			if (!found)
			{
				software_list_device::display_matches(config, nullptr, m_options.software_name());
				throw emu_fatalerror(MAMERR_FATALERROR, nullptr);
			}
		}

		// parse the command line, adding any system-specific options
		if (!m_options.parse_command_line(argc, argv, option_errors))
		{
			// if we failed, check for no command and a system name first; in that case error on the name
			if (*(m_options.command()) == 0 && m_options.system() == nullptr && *(m_options.system_name()) != 0)
				throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "Unknown system '%s'", m_options.system_name());

			// otherwise, error on the options
			throw emu_fatalerror(MAMERR_INVALID_CONFIG, "%s", strtrimspace(option_errors).c_str());
		}
		if (!option_errors.empty())
			osd_printf_error("Error in command line:\n%s\n", strtrimspace(option_errors).c_str());

		// determine the base name of the EXE
		std::string exename;
		core_filename_extract_base(exename, argv[0], true);

		// if we have a command, execute that
		if (*(m_options.command()) != 0)
			execute_commands(exename.c_str());

		// otherwise, check for a valid system
		else
		{
			// We need to preprocess the config files once to determine the web server's configuration
			// and file locations
			if (m_options.read_config())
			{
				m_options.revert(OPTION_PRIORITY_INI);
				m_options.parse_standard_inis(option_errors);
			}
			if (!option_errors.empty())
				osd_printf_error("Error in command line:\n%s\n", strtrimspace(option_errors).c_str());

			// if we can't find it, give an appropriate error
			const game_driver *system = m_options.system();
			if (system == nullptr && *(m_options.system_name()) != 0)
				throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "Unknown system '%s'", m_options.system_name());

			// otherwise just run the game
			machine_manager *manager = machine_manager::instance(m_options, m_osd);
			m_result = manager->execute();
			global_free(manager);
		}
	}

	// handle exceptions of various types
	catch (emu_fatalerror &fatal)
	{
		std::string str(fatal.string());
		strtrimspace(str);
		osd_printf_error("%s\n", str.c_str());
		m_result = (fatal.exitcode() != 0) ? fatal.exitcode() : MAMERR_FATALERROR;

		// if a game was specified, wasn't a wildcard, and our error indicates this was the
		// reason for failure, offer some suggestions
		if (m_result == MAMERR_NO_SUCH_GAME && *(m_options.system_name()) != 0 && strchr(m_options.system_name(), '*') == nullptr && m_options.system() == nullptr)
		{
			// get the top 16 approximate matches
			driver_enumerator drivlist(m_options);
			int matches[16];
			drivlist.find_approximate_matches(m_options.system_name(), ARRAY_LENGTH(matches), matches);

			// print them out
			osd_printf_error("\n\"%s\" approximately matches the following\n"
					"supported %s (best match first):\n\n", m_options.system_name(),emulator_info::get_gamesnoun());
			for (auto & matche : matches)
				if (matche != -1)
					osd_printf_error("%-18s%s\n", drivlist.driver(matche).name, drivlist.driver(matche).description);
		}
	}
	catch (emu_exception &)
	{
		osd_printf_error("Caught unhandled emulator exception\n");
		m_result = MAMERR_FATALERROR;
	}
	catch (add_exception &aex)
	{
		osd_printf_error("Tag '%s' already exists in tagged_list\n", aex.tag());
		m_result = MAMERR_FATALERROR;
	}
	catch (std::exception &ex)
	{
		osd_printf_error("Caught unhandled %s exception: %s\n", typeid(ex).name(), ex.what());
		m_result = MAMERR_FATALERROR;
	}
	catch (...)
	{
		osd_printf_error("Caught unhandled exception\n");
		m_result = MAMERR_FATALERROR;
	}

	_7z_file_cache_clear();

	return m_result;
}


//-------------------------------------------------
//  listxml - output the XML data for one or more
//  games
//-------------------------------------------------

void cli_frontend::listxml(const char *gamename)
{
	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// create the XML and print it to stdout
	info_xml_creator creator(drivlist);
	creator.output(stdout);
}


//-------------------------------------------------
//  listfull - output the name and description of
//  one or more games
//-------------------------------------------------

void cli_frontend::listfull(const char *gamename)
{
	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// print the header
	osd_printf_info("Name:             Description:\n");

	// iterate through drivers and output the info
	while (drivlist.next())
		if ((drivlist.driver().flags & MACHINE_NO_STANDALONE) == 0)
			osd_printf_info("%-18s\"%s\"\n", drivlist.driver().name, drivlist.driver().description);
}


//-------------------------------------------------
//  listsource - output the name and source
//  filename of one or more games
//-------------------------------------------------

void cli_frontend::listsource(const char *gamename)
{
	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// iterate through drivers and output the info
	std::string filename;
	while (drivlist.next())
		osd_printf_info("%-16s %s\n", drivlist.driver().name, core_filename_extract_base(filename, drivlist.driver().source_file).c_str());
}


//-------------------------------------------------
//  listclones - output the name and parent of all
//  clones matching the given pattern
//-------------------------------------------------

void cli_frontend::listclones(const char *gamename)
{
	// start with a filtered list of drivers
	driver_enumerator drivlist(m_options, gamename);
	int original_count = drivlist.count();

	// iterate through the remaining ones to see if their parent matches
	while (drivlist.next_excluded())
	{
		// if we have a non-bios clone and it matches, keep it
		int clone_of = drivlist.clone();
		if (clone_of != -1 && (drivlist.driver(clone_of).flags & MACHINE_IS_BIOS_ROOT) == 0)
			if (drivlist.matches(gamename, drivlist.driver(clone_of).name))
				drivlist.include();
	}

	// return an error if none found
	if (drivlist.count() == 0)
	{
		// see if we match but just weren't a clone
		if (original_count == 0)
			throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);
		else
			osd_printf_info("Found %d matches for '%s' but none were clones\n", drivlist.count(), gamename);
		return;
	}

	// print the header
	osd_printf_info("Name:            Clone of:\n");

	// iterate through drivers and output the info
	drivlist.reset();
	while (drivlist.next())
	{
		int clone_of = drivlist.clone();
		if (clone_of != -1 && (drivlist.driver(clone_of).flags & MACHINE_IS_BIOS_ROOT) == 0)
			osd_printf_info("%-16s %-8s\n", drivlist.driver().name, drivlist.driver(clone_of).name);
	}
}


//-------------------------------------------------
//  listbrothers - for each matching game, output
//  the list of other games that share the same
//  source file
//-------------------------------------------------

void cli_frontend::listbrothers(const char *gamename)
{
	// start with a filtered list of drivers; return an error if none found
	driver_enumerator initial_drivlist(m_options, gamename);
	if (initial_drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

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
			if (strcmp(drivlist.driver().source_file, initial_drivlist.driver().source_file) == 0)
				drivlist.include();
	}

	// print the header
	osd_printf_info("Source file:     Name:            Parent:\n");

	// output the entries found
	drivlist.reset();
	std::string filename;
	while (drivlist.next())
	{
		int clone_of = drivlist.clone();
		osd_printf_info("%-16s %-16s %-16s\n", core_filename_extract_base(filename, drivlist.driver().source_file).c_str(), drivlist.driver().name, (clone_of == -1 ? "" : drivlist.driver(clone_of).name));
	}
}


//-------------------------------------------------
//  listcrc - output the CRC and name of all ROMs
//  referenced by the emulator
//-------------------------------------------------

void cli_frontend::listcrc(const char *gamename)
{
	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// iterate through matches, and then through ROMs
	while (drivlist.next())
	{
		device_iterator deviter(drivlist.config().root_device());
		for (device_t *device = deviter.first(); device != nullptr; device = deviter.next())
			for (const rom_entry *region = rom_first_region(*device); region; region = rom_next_region(region))
				for (const rom_entry *rom = rom_first_file(region); rom; rom = rom_next_file(rom))
				{
					// if we have a CRC, display it
					UINT32 crc;
					if (hash_collection(ROM_GETHASHDATA(rom)).crc(crc))
						osd_printf_info("%08x %-16s \t %-8s \t %s\n", crc, ROM_GETNAME(rom), device->shortname(), device->name());
				}
	}
}


//-------------------------------------------------
//  listroms - output the list of ROMs referenced
//  by a given game or set of games
//-------------------------------------------------

void cli_frontend::listroms(const char *gamename)
{
	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// iterate through matches
	std::string tempstr;
	bool first = true;
	while (drivlist.next())
	{
		// print a header
		if (!first)
			osd_printf_info("\n");
		first = false;
		osd_printf_info("ROMs required for driver \"%s\".\n"
				"Name                    Size Checksum\n", drivlist.driver().name);

		// iterate through roms
		device_iterator deviter(drivlist.config().root_device());
		for (device_t *device = deviter.first(); device != nullptr; device = deviter.next())
			for (const rom_entry *region = rom_first_region(*device); region; region = rom_next_region(region))
				for (const rom_entry *rom = rom_first_file(region); rom; rom = rom_next_file(rom))
				{
					// accumulate the total length of all chunks
					int length = -1;
					if (ROMREGION_ISROMDATA(region))
						length = rom_file_size(rom);

					// start with the name
					const char *name = ROM_GETNAME(rom);
					osd_printf_info("%-20s ", name);

					// output the length next
					if (length >= 0)
						osd_printf_info("%7d", length);
					else
						osd_printf_info("       ");

					// output the hash data
					hash_collection hashes(ROM_GETHASHDATA(rom));
					if (!hashes.flag(hash_collection::FLAG_NO_DUMP))
					{
						if (hashes.flag(hash_collection::FLAG_BAD_DUMP))
							osd_printf_info(" BAD");
						osd_printf_info(" %s", hashes.macro_string(tempstr));
					}
					else
						osd_printf_info(" NO GOOD DUMP KNOWN");

					// end with a CR
					osd_printf_info("\n");
				}
	}
}


//-------------------------------------------------
//  listsamples - output the list of samples
//  referenced by a given game or set of games
//-------------------------------------------------

void cli_frontend::listsamples(const char *gamename)
{
	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// iterate over drivers, looking for SAMPLES devices
	bool first = true;
	while (drivlist.next())
	{
		// see if we have samples
		samples_device_iterator iter(drivlist.config().root_device());
		if (iter.count() == 0)
			continue;

		// print a header
		if (!first)
			osd_printf_info("\n");
		first = false;
		osd_printf_info("Samples required for driver \"%s\".\n", drivlist.driver().name);

		// iterate over samples devices and print the samples from each one
		for (samples_device *device = iter.first(); device != nullptr; device = iter.next())
		{
			samples_iterator sampiter(*device);
			for (const char *samplename = sampiter.first(); samplename != nullptr; samplename = sampiter.next())
				osd_printf_info("%s\n", samplename);
		}
	}
}


//-------------------------------------------------
//  listdevices - output the list of devices
//  referenced by a given game or set of games
//-------------------------------------------------

int cli_frontend::compare_devices(const void *i1, const void *i2)
{
	device_t *dev1 = *(device_t **)i1;
	device_t *dev2 = *(device_t **)i2;
	return strcmp(dev1->tag(), dev2->tag());
}

void cli_frontend::listdevices(const char *gamename)
{
	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// iterate over drivers, looking for SAMPLES devices
	bool first = true;
	while (drivlist.next())
	{
		// print a header
		if (!first)
			printf("\n");
		first = false;
		printf("Driver %s (%s):\n", drivlist.driver().name, drivlist.driver().description);

		// build a list of devices
		device_iterator iter(drivlist.config().root_device());
		std::vector<device_t *> device_list;
		for (device_t *device = iter.first(); device != nullptr; device = iter.next())
			device_list.push_back(device);

		// sort them by tag
		qsort(&device_list[0], device_list.size(), sizeof(device_list[0]), compare_devices);

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
			UINT32 clock = device->clock();
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

void cli_frontend::listslots(const char *gamename)
{
	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// print header
	printf(" SYSTEM      SLOT NAME    SLOT OPTIONS    SLOT DEVICE NAME     \n");
	printf("----------  -----------  --------------  ----------------------\n");

	// iterate over drivers
	while (drivlist.next())
	{
		// iterate
		slot_interface_iterator iter(drivlist.config().root_device());
		bool first = true;
		for (const device_slot_interface *slot = iter.first(); slot != nullptr; slot = iter.next())
		{
			if (slot->fixed()) continue;
			// output the line, up to the list of extensions
			printf("%-13s%-10s   ", first ? drivlist.driver().name : "", slot->device().tag()+1);

			bool first_option = true;

			// get the options and print them
			for (const device_slot_option *option = slot->first_option(); option != nullptr; option = option->next())
			{
				if (option->selectable())
				{
					device_t *dev = (*option->devtype())(drivlist.config(), "dummy", &drivlist.config().root_device(), 0);
					dev->config_complete();
					if (first_option) {
						printf("%-15s %s\n", option->name(),dev->name());
					} else {
						printf("%-23s   %-15s %s\n", "",option->name(),dev->name());
					}
					global_free(dev);

					first_option = false;
				}
			}
			if (first_option)
				printf("%-15s %s\n", "[none]","No options available");
			// end the line
			printf("\n");
			first = false;
		}

		// if we didn't get any at all, just print a none line
		if (first)
			printf("%-13s(none)\n", drivlist.driver().name);
	}
}


//-------------------------------------------------
//  listmedia - output the list of image devices
//  referenced by a given game or set of games
//-------------------------------------------------

void cli_frontend::listmedia(const char *gamename)
{
	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// print header
	printf(" SYSTEM      MEDIA NAME (brief)   IMAGE FILE EXTENSIONS SUPPORTED     \n");
	printf("----------  --------------------  ------------------------------------\n");

	// iterate over drivers
	while (drivlist.next())
	{
		// iterate
		image_interface_iterator iter(drivlist.config().root_device());
		bool first = true;
		for (const device_image_interface *imagedev = iter.first(); imagedev != nullptr; imagedev = iter.next())
		{
			// extract the shortname with parentheses
			std::string paren_shortname;
			strprintf(paren_shortname,"(%s)", imagedev->brief_instance_name());

			// output the line, up to the list of extensions
			printf("%-13s%-12s%-8s   ", first ? drivlist.driver().name : "", imagedev->instance_name(), paren_shortname.c_str());

			// get the extensions and print them
			std::string extensions(imagedev->file_extensions());
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
			printf("%-13s(none)\n", drivlist.driver().name);
	}
}

//-------------------------------------------------
//  verifyroms - verify the ROM sets of one or
//  more games
//-------------------------------------------------
void cli_frontend::verifyroms(const char *gamename)
{
	// determine which drivers to output;
	driver_enumerator drivlist(m_options, gamename);

	int correct = 0;
	int incorrect = 0;
	int notfound = 0;
	int matched = 0;

	// iterate over drivers
	media_auditor auditor(drivlist);
	while (drivlist.next())
	{
		matched++;

		// audit the ROMs in this set
		media_auditor::summary summary = auditor.audit_media(AUDIT_VALIDATE_FAST);

		// if not found, count that and leave it at that
		if (summary == media_auditor::NOTFOUND)
			notfound++;

		// else display information about what we discovered
		else
		{
			// output the summary of the audit
			std::string summary_string;
			auditor.summarize(drivlist.driver().name,&summary_string);
			osd_printf_info("%s", summary_string.c_str());

			// output the name of the driver and its clone
			osd_printf_info("romset %s ", drivlist.driver().name);
			int clone_of = drivlist.clone();
			if (clone_of != -1)
				osd_printf_info("[%s] ", drivlist.driver(clone_of).name);

			// switch off of the result
			switch (summary)
			{
				case media_auditor::INCORRECT:
					osd_printf_info("is bad\n");
					incorrect++;
					break;

				case media_auditor::CORRECT:
					osd_printf_info("is good\n");
					correct++;
					break;

				case media_auditor::BEST_AVAILABLE:
				case media_auditor::NONE_NEEDED:
					osd_printf_info("is best available\n");
					correct++;
					break;

				default:
					break;
			}
		}
	}

	if (!matched || strchr(gamename, '*') || strchr(gamename, '?'))
	{
		driver_enumerator dummy_drivlist(m_options);
		std::unordered_set<std::string> device_map;
		while (dummy_drivlist.next())
		{
			machine_config &config = dummy_drivlist.config();
			device_iterator iter(config.root_device());
			for (device_t *dev = iter.first(); dev != nullptr; dev = iter.next())
			{
				if (dev->owner() != nullptr && (*(dev->shortname()) != 0) && dev->rom_region() != nullptr && (device_map.insert(dev->shortname()).second)) {
					if (core_strwildcmp(gamename, dev->shortname()) == 0)
					{
						matched++;

						// audit the ROMs in this set
						media_auditor::summary summary = auditor.audit_device(dev, AUDIT_VALIDATE_FAST);

						// if not found, count that and leave it at that
						if (summary == media_auditor::NOTFOUND)
							notfound++;
						// else display information about what we discovered
						else if (summary != media_auditor::NONE_NEEDED)
						{
							// output the summary of the audit
							std::string summary_string;
							auditor.summarize(dev->shortname(),&summary_string);
							osd_printf_info("%s", summary_string.c_str());

							// display information about what we discovered
							osd_printf_info("romset %s ", dev->shortname());

							// switch off of the result
							switch (summary)
							{
								case media_auditor::INCORRECT:
									osd_printf_info("is bad\n");
									incorrect++;
									break;

								case media_auditor::CORRECT:
									osd_printf_info("is good\n");
									correct++;
									break;

								case media_auditor::BEST_AVAILABLE:
									osd_printf_info("is best available\n");
									correct++;
									break;

								default:
									break;
							}
						}
					}
				}
			}

			slot_interface_iterator slotiter(config.root_device());
			for (const device_slot_interface *slot = slotiter.first(); slot != nullptr; slot = slotiter.next())
			{
				for (const device_slot_option *option = slot->first_option(); option != nullptr; option = option->next())
				{
					std::string temptag("_");
					temptag.append(option->name());
					device_t *dev = const_cast<machine_config &>(config).device_add(&config.root_device(), temptag.c_str(), option->devtype(), 0);

					// notify this device and all its subdevices that they are now configured
					device_iterator subiter(*dev);
					for (device_t *device = subiter.first(); device != nullptr; device = subiter.next())
						if (!device->configured())
							device->config_complete();

					if (device_map.insert(dev->shortname()).second) {
						if (core_strwildcmp(gamename, dev->shortname()) == 0)
						{
							matched++;
							if (dev->rom_region() != nullptr)
							{
								// audit the ROMs in this set
								media_auditor::summary summary = auditor.audit_device(dev, AUDIT_VALIDATE_FAST);

								// if not found, count that and leave it at that
								if (summary == media_auditor::NOTFOUND)
									notfound++;

								// else display information about what we discovered
								else if(summary != media_auditor::NONE_NEEDED)
								{
									// output the summary of the audit
									std::string summary_string;
									auditor.summarize(dev->shortname(),&summary_string);
									osd_printf_info("%s", summary_string.c_str());

									// display information about what we discovered
									osd_printf_info("romset %s ", dev->shortname());

									// switch off of the result
									switch (summary)
									{
										case media_auditor::INCORRECT:
											osd_printf_info("is bad\n");
											incorrect++;
											break;

										case media_auditor::CORRECT:
											osd_printf_info("is good\n");
											correct++;
											break;

										case media_auditor::BEST_AVAILABLE:
											osd_printf_info("is best available\n");
											correct++;
											break;

										default:
											break;
									}
								}
							}
						}
					} else {
						// check for subdevices with ROMs (a few devices are missed otherwise, e.g. MPU401)
						device_iterator subiter(*dev);
						for (device_t *device = subiter.first(); device != nullptr; device = subiter.next())
						{
							device_iterator subsubiter(*device);
							for (device_t *subdev = subsubiter.first(); subdev != nullptr; subdev = subsubiter.next())
							{
								if (subdev->owner() == device && subdev->rom_region() != nullptr && subdev->shortname() != nullptr && subdev->shortname()[0] != '\0')
								{
									if (device_map.insert(subdev->shortname()).second)
									{
										if (core_strwildcmp(gamename, subdev->shortname()) == 0)
										{
											matched++;

											// audit the ROMs in this set
											media_auditor::summary summary = auditor.audit_device(subdev, AUDIT_VALIDATE_FAST);

											// if not found, count that and leave it at that
											if (summary == media_auditor::NOTFOUND)
												notfound++;

											// else display information about what we discovered
											else if (summary != media_auditor::NONE_NEEDED)
											{
												// output the summary of the audit
												std::string summary_string;
												auditor.summarize(subdev->shortname(),&summary_string);
												osd_printf_info("%s", summary_string.c_str());

												// display information about what we discovered
												osd_printf_info("romset %s ", subdev->shortname());

												// switch off of the result
												switch (summary)
												{
													case media_auditor::INCORRECT:
														osd_printf_info("is bad\n");
														incorrect++;
														break;

													case media_auditor::CORRECT:
														osd_printf_info("is good\n");
														correct++;
														break;

													case media_auditor::BEST_AVAILABLE:
														osd_printf_info("is best available\n");
														correct++;
														break;

													default:
														break;
												}
											}
										}
									}
								}
							}
						}
					}

					const_cast<machine_config &>(config).device_remove(&config.root_device(), temptag.c_str());
				}
			}
		}
	}

	// clear out any cached files
	zip_file_cache_clear();

	// return an error if none found
	if (matched == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// if we didn't get anything at all, display a generic end message
	if (matched > 0 && correct == 0 && incorrect == 0)
	{
		if (notfound > 0)
			throw emu_fatalerror(MAMERR_MISSING_FILES, "romset \"%s\" not found!\n", gamename);
		else
			throw emu_fatalerror(MAMERR_MISSING_FILES, "romset \"%s\" has no roms!\n", gamename);
	}

	// otherwise, print a summary
	else
	{
		if (incorrect > 0)
			throw emu_fatalerror(MAMERR_MISSING_FILES, "%d romsets found, %d were OK.\n", correct + incorrect, correct);
		osd_printf_info("%d romsets found, %d were OK.\n", correct, correct);
	}
}


//-------------------------------------------------
//  info_verifysamples - verify the sample sets of
//  one or more games
//-------------------------------------------------

void cli_frontend::verifysamples(const char *gamename)
{
	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);

	int correct = 0;
	int incorrect = 0;
	int notfound = 0;
	int matched = 0;

	// iterate over drivers
	media_auditor auditor(drivlist);
	while (drivlist.next())
	{
		matched++;

		// audit the samples in this set
		media_auditor::summary summary = auditor.audit_samples();

		// if not found, count that and leave it at that
		if (summary == media_auditor::NOTFOUND)
			notfound++;

		// else display information about what we discovered
		else if (summary != media_auditor::NONE_NEEDED)
		{
			// output the summary of the audit
			std::string summary_string;
			auditor.summarize(drivlist.driver().name,&summary_string);
			osd_printf_info("%s", summary_string.c_str());

			// output the name of the driver and its clone
			osd_printf_info("sampleset %s ", drivlist.driver().name);
			int clone_of = drivlist.clone();
			if (clone_of != -1)
				osd_printf_info("[%s] ", drivlist.driver(clone_of).name);

			// switch off of the result
			switch (summary)
			{
				case media_auditor::INCORRECT:
					osd_printf_info("is bad\n");
					incorrect++;
					break;

				case media_auditor::CORRECT:
					osd_printf_info("is good\n");
					correct++;
					break;

				case media_auditor::BEST_AVAILABLE:
					osd_printf_info("is best available\n");
					correct++;
					break;

				default:
					break;
			}
		}
	}

	// clear out any cached files
	zip_file_cache_clear();

	// return an error if none found
	if (matched == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// if we didn't get anything at all, display a generic end message
	if (matched > 0 && correct == 0 && incorrect == 0)
	{
		if (notfound > 0)
			throw emu_fatalerror(MAMERR_MISSING_FILES, "sampleset \"%s\" not found!\n", gamename);
		else
			throw emu_fatalerror(MAMERR_MISSING_FILES, "sampleset \"%s\" not required!\n", gamename);
	}

	// otherwise, print a summary
	else
	{
		if (incorrect > 0)
			throw emu_fatalerror(MAMERR_MISSING_FILES, "%d samplesets found, %d were OK.\n", correct + incorrect, correct);
		osd_printf_info("%d samplesets found, %d were OK.\n", correct, correct);
	}
}
#define SOFTLIST_XML_BEGIN "<?xml version=\"1.0\"?>\n" \
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
				"]>\n\n" \
				"<softwarelists>\n"

void cli_frontend::output_single_softlist(FILE *out, software_list_device &swlistdev)
{
	std::string tempstr;

	fprintf(out, "\t<softwarelist name=\"%s\" description=\"%s\">\n", swlistdev.list_name(), xml_normalize_string(swlistdev.description()));
	for (software_info *swinfo = swlistdev.first_software_info(); swinfo != nullptr; swinfo = swinfo->next())
	{
		fprintf( out, "\t\t<software name=\"%s\"", swinfo->shortname() );
		if ( swinfo->parentname() != nullptr )
			fprintf( out, " cloneof=\"%s\"", swinfo->parentname() );
		if ( swinfo->supported() == SOFTWARE_SUPPORTED_PARTIAL )
			fprintf( out, " supported=\"partial\"" );
		if ( swinfo->supported() == SOFTWARE_SUPPORTED_NO )
			fprintf( out, " supported=\"no\"" );
		fprintf( out, ">\n" );
		fprintf( out, "\t\t\t<description>%s</description>\n", xml_normalize_string(swinfo->longname()) );
		fprintf( out, "\t\t\t<year>%s</year>\n", xml_normalize_string( swinfo->year() ) );
		fprintf( out, "\t\t\t<publisher>%s</publisher>\n", xml_normalize_string( swinfo->publisher() ) );

		for (feature_list_item *flist = swinfo->other_info(); flist != nullptr; flist = flist->next())
			fprintf( out, "\t\t\t<info name=\"%s\" value=\"%s\"/>\n", flist->name(), xml_normalize_string( flist->value() ) );

		for ( software_part *part = swinfo->first_part(); part != nullptr; part = part->next() )
		{
			fprintf( out, "\t\t\t<part name=\"%s\"", part->name() );
			if ( part->interface() != nullptr )
				fprintf( out, " interface=\"%s\"", part->interface() );

			fprintf( out, ">\n");

			for (feature_list_item *flist = part->featurelist(); flist != nullptr; flist = flist->next())
				fprintf( out, "\t\t\t\t<feature name=\"%s\" value=\"%s\" />\n", flist->name(), xml_normalize_string(flist->value()) );

			/* TODO: display rom region information */
			for ( const rom_entry *region = part->romdata(); region; region = rom_next_region( region ) )
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
						if ( !hashes.flag(hash_collection::FLAG_NO_DUMP) )
							fprintf( out, " %s", hashes.attribute_string(tempstr) );
						else
							fprintf( out, " status=\"nodump\"" );

						if (is_disk)
							fprintf( out, " writeable=\"%s\"", (ROM_GETFLAGS(rom) & DISK_READONLYMASK) ? "no" : "yes");

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
/*-------------------------------------------------
    info_listsoftware - output the list of
    software supported by a given game or set of
    games
    TODO: Add all information read from the source files
    Possible improvement: use a sorted list for
        identifying duplicate lists.
-------------------------------------------------*/

void cli_frontend::listsoftware(const char *gamename)
{
	FILE *out = stdout;
	std::unordered_set<std::string> list_map;
	bool isfirst = true;

	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	while (drivlist.next())
	{
		software_list_device_iterator iter(drivlist.config().root_device());
		for (software_list_device *swlistdev = iter.first(); swlistdev != nullptr; swlistdev = iter.next())
			if (list_map.insert(swlistdev->list_name()).second)
				if (swlistdev->first_software_info() != nullptr)
				{
					if (isfirst) { fprintf(out, SOFTLIST_XML_BEGIN); isfirst = false; }
					output_single_softlist(out, *swlistdev);
				}
	}

	if (!isfirst)
		fprintf( out, "</softwarelists>\n" );
	else
		fprintf( out, "No software lists found for this system\n" );
}


/*-------------------------------------------------
    verifysoftware - verify roms from the software
    list of the specified driver(s)
-------------------------------------------------*/
void cli_frontend::verifysoftware(const char *gamename)
{
	std::unordered_set<std::string> list_map;

	int correct = 0;
	int incorrect = 0;
	int notfound = 0;
	int matched = 0;
	int nrlists = 0;

	// determine which drivers to process; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
	{
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);
	}

	media_auditor auditor(drivlist);
	while (drivlist.next())
	{
		matched++;

		software_list_device_iterator iter(drivlist.config().root_device());
		for (software_list_device *swlistdev = iter.first(); swlistdev != nullptr; swlistdev = iter.next())
			if (swlistdev->list_type() == SOFTWARE_LIST_ORIGINAL_SYSTEM)
				if (list_map.insert(swlistdev->list_name()).second)
					if (swlistdev->first_software_info() != nullptr)
					{
						nrlists++;
						for (software_info *swinfo = swlistdev->first_software_info(); swinfo != nullptr; swinfo = swinfo->next())
						{
							media_auditor::summary summary = auditor.audit_software(swlistdev->list_name(), swinfo, AUDIT_VALIDATE_FAST);

							// if not found, count that and leave it at that
							if (summary == media_auditor::NOTFOUND)
							{
								notfound++;
							}
							// else display information about what we discovered
							else if(summary != media_auditor::NONE_NEEDED)
							{
								// output the summary of the audit
								std::string summary_string;
								auditor.summarize(swinfo->shortname(), &summary_string);
								osd_printf_info("%s", summary_string.c_str());

								// display information about what we discovered
								osd_printf_info("romset %s:%s ", swlistdev->list_name(), swinfo->shortname());

								// switch off of the result
								switch (summary)
								{
									case media_auditor::INCORRECT:
										osd_printf_info("is bad\n");
										incorrect++;
										break;

									case media_auditor::CORRECT:
										osd_printf_info("is good\n");
										correct++;
										break;

									case media_auditor::BEST_AVAILABLE:
										osd_printf_info("is best available\n");
										correct++;
										break;

									default:
										break;
								}
							}
						}
					}
	}

	// clear out any cached files
	zip_file_cache_clear();

	// return an error if none found
	if (matched == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// if we didn't get anything at all, display a generic end message
	if (matched > 0 && correct == 0 && incorrect == 0)
	{
		throw emu_fatalerror(MAMERR_MISSING_FILES, "romset \"%s\" has no software entries defined!\n", gamename);
	}
	// otherwise, print a summary
	else
	{
		if (incorrect > 0)
			throw emu_fatalerror(MAMERR_MISSING_FILES, "%d romsets found in %d software lists, %d were OK.\n", correct + incorrect, nrlists, correct);
		osd_printf_info("%d romsets found in %d software lists, %d romsets were OK.\n", correct, nrlists, correct);
	}

}

/*-------------------------------------------------
    getsoftlist - retrieve software list by name
-------------------------------------------------*/

void cli_frontend::getsoftlist(const char *gamename)
{
	FILE *out = stdout;
	std::unordered_set<std::string> list_map;
	bool isfirst = TRUE;

	driver_enumerator drivlist(m_options);
	while (drivlist.next())
	{
		software_list_device_iterator iter(drivlist.config().root_device());
		for (software_list_device *swlistdev = iter.first(); swlistdev != nullptr; swlistdev = iter.next())
			if (core_strwildcmp(gamename, swlistdev->list_name()) == 0 && list_map.insert(swlistdev->list_name()).second)
				if (swlistdev->first_software_info() != nullptr)
				{
					if (isfirst) { fprintf( out, SOFTLIST_XML_BEGIN); isfirst = FALSE; }
					output_single_softlist(out, *swlistdev);
				}
	}

	if (!isfirst)
		fprintf( out, "</softwarelists>\n" );
	else
		fprintf( out, "No such software lists found\n" );
}


/*-------------------------------------------------
    verifysoftlist - verify software list by name
-------------------------------------------------*/
void cli_frontend::verifysoftlist(const char *gamename)
{
	std::unordered_set<std::string> list_map;
	int correct = 0;
	int incorrect = 0;
	int notfound = 0;
	int matched = 0;

	driver_enumerator drivlist(m_options);
	media_auditor auditor(drivlist);

	while (drivlist.next())
	{
		software_list_device_iterator iter(drivlist.config().root_device());
		for (software_list_device *swlistdev = iter.first(); swlistdev != nullptr; swlistdev = iter.next())
			if (core_strwildcmp(gamename, swlistdev->list_name()) == 0 && list_map.insert(swlistdev->list_name()).second)
				if (swlistdev->first_software_info() != nullptr)
				{
					matched++;

					// Get the actual software list contents
					for (software_info *swinfo = swlistdev->first_software_info(); swinfo != nullptr; swinfo = swinfo->next())
					{
						media_auditor::summary summary = auditor.audit_software(swlistdev->list_name(), swinfo, AUDIT_VALIDATE_FAST);

						// if not found, count that and leave it at that
						if (summary == media_auditor::NOTFOUND)
						{
							notfound++;
						}
						// else display information about what we discovered
						else if (summary != media_auditor::NONE_NEEDED)
						{
							// output the summary of the audit
							std::string summary_string;
							auditor.summarize(swinfo->shortname(), &summary_string);
							osd_printf_info("%s", summary_string.c_str());

							// display information about what we discovered
							osd_printf_info("romset %s:%s ", swlistdev->list_name(), swinfo->shortname());

							// switch off of the result
							switch (summary)
							{
								case media_auditor::INCORRECT:
									osd_printf_info("is bad\n");
									incorrect++;
									break;

								case media_auditor::CORRECT:
									osd_printf_info("is good\n");
									correct++;
									break;

								case media_auditor::BEST_AVAILABLE:
									osd_printf_info("is best available\n");
									correct++;
									break;

								default:
									break;
							}
						}
					}
				}
	}

	// clear out any cached files
	zip_file_cache_clear();

	// return an error if none found
	if (matched == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching software lists found for '%s'", gamename);

	// if we didn't get anything at all, display a generic end message
	if (matched > 0 && correct == 0 && incorrect == 0)
	{
		throw emu_fatalerror(MAMERR_MISSING_FILES, "no romsets found for software list \"%s\"!\n", gamename);
	}
	// otherwise, print a summary
	else
	{
		if (incorrect > 0)
			throw emu_fatalerror(MAMERR_MISSING_FILES, "%d romsets found in %d software lists, %d were OK.\n", correct + incorrect, matched, correct);
		osd_printf_info("%d romsets found in %d software lists, %d romsets were OK.\n", correct, matched, correct);
	}
}

//-------------------------------------------------
//  romident - identify ROMs by looking for
//  matches in our internal database
//-------------------------------------------------

void cli_frontend::romident(const char *filename)
{
	media_identifier ident(m_options);

	// identify the file, then output results
	osd_printf_info("Identifying %s....\n", filename);
	ident.identify(filename);

	// return the appropriate error code
	if (ident.matches() == ident.total())
		return;
	else if (ident.matches() == ident.total() - ident.nonroms())
		throw emu_fatalerror(MAMERR_IDENT_NONROMS, "Out of %d files, %d matched, %d are not roms.\n",ident.total(),ident.matches(),ident.nonroms());
	else if (ident.matches() > 0)
		throw emu_fatalerror(MAMERR_IDENT_PARTIAL, "Out of %d files, %d matched, %d did not match.\n",ident.total(),ident.matches(),ident.total()-ident.matches());
	else
		throw emu_fatalerror(MAMERR_IDENT_NONE, "No roms matched.\n");
}


//-------------------------------------------------
//  execute_commands - execute various frontend
//  commands
//-------------------------------------------------

void cli_frontend::execute_commands(const char *exename)
{
	// help?
	if (strcmp(m_options.command(), CLICOMMAND_HELP) == 0)
	{
		display_help();
		return;
	}

	// showusage?
	if (strcmp(m_options.command(), CLICOMMAND_SHOWUSAGE) == 0)
	{
		std::string helpstring;
		emulator_info::printf_usage(exename, emulator_info::get_gamenoun());
		osd_printf_info("\n\nOptions:\n%s", m_options.output_help(helpstring));
		return;
	}

	// validate?
	if (strcmp(m_options.command(), CLICOMMAND_VALIDATE) == 0)
	{
		validity_checker valid(m_options);
		bool result = valid.check_all();
		if (!result)
			throw emu_fatalerror(MAMERR_FAILED_VALIDITY, "Validity check failed (%d errors, %d warnings in total)\n", valid.errors(), valid.warnings());
		return;
	}

	// other commands need the INIs parsed
	std::string option_errors;
	m_options.parse_standard_inis(option_errors);
	if (!option_errors.empty())
		osd_printf_error("%s\n", option_errors.c_str());

	// createconfig?
	if (strcmp(m_options.command(), CLICOMMAND_CREATECONFIG) == 0)
	{
		// attempt to open the output file
		emu_file file(OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		if (file.open(emulator_info::get_configname(), ".ini") != FILERR_NONE)
			throw emu_fatalerror("Unable to create file %s.ini\n",emulator_info::get_configname());

		// generate the updated INI
		std::string initext;
		file.puts(m_options.output_ini(initext));
		return;
	}

	// showconfig?
	if (strcmp(m_options.command(), CLICOMMAND_SHOWCONFIG) == 0)
	{
		// print the INI text
		std::string initext;
		printf("%s\n", m_options.output_ini(initext));
		return;
	}

	// all other commands call out to one of these helpers
	static const struct
	{
		const char *option;
		void (cli_frontend::*function)(const char *gamename);
	} info_commands[] =
	{
		{ CLICOMMAND_LISTXML,       &cli_frontend::listxml },
		{ CLICOMMAND_LISTFULL,      &cli_frontend::listfull },
		{ CLICOMMAND_LISTSOURCE,    &cli_frontend::listsource },
		{ CLICOMMAND_LISTCLONES,    &cli_frontend::listclones },
		{ CLICOMMAND_LISTBROTHERS,  &cli_frontend::listbrothers },
		{ CLICOMMAND_LISTCRC,       &cli_frontend::listcrc },
		{ CLICOMMAND_LISTDEVICES,   &cli_frontend::listdevices },
		{ CLICOMMAND_LISTSLOTS,     &cli_frontend::listslots },
		{ CLICOMMAND_LISTROMS,      &cli_frontend::listroms },
		{ CLICOMMAND_LISTSAMPLES,   &cli_frontend::listsamples },
		{ CLICOMMAND_VERIFYROMS,    &cli_frontend::verifyroms },
		{ CLICOMMAND_VERIFYSAMPLES, &cli_frontend::verifysamples },
		{ CLICOMMAND_LISTMEDIA,     &cli_frontend::listmedia },
		{ CLICOMMAND_LISTSOFTWARE,  &cli_frontend::listsoftware },
		{ CLICOMMAND_VERIFYSOFTWARE,&cli_frontend::verifysoftware },
		{ CLICOMMAND_ROMIDENT,      &cli_frontend::romident },
		{ CLICOMMAND_GETSOFTLIST,   &cli_frontend::getsoftlist },
		{ CLICOMMAND_VERIFYSOFTLIST,&cli_frontend::verifysoftlist },
	};

	// find the command
	for (auto & info_command : info_commands)
		if (strcmp(m_options.command(), info_command.option) == 0)
		{
			// parse any relevant INI files before proceeding
			const char *sysname = m_options.system_name();
			(this->*info_command.function)((sysname[0] == 0) ? "*" : sysname);
			return;
		}

	if (!m_osd.execute_command(m_options.command()))
		// if we get here, we don't know what has been requested
		throw emu_fatalerror(MAMERR_INVALID_CONFIG, "Unknown command '%s' specified", m_options.command());
}


//-------------------------------------------------
//  display_help - display help to standard
//  output
//-------------------------------------------------

void cli_frontend::display_help()
{
	osd_printf_info("%s v%s\n%s\n\n", emulator_info::get_appname(),build_version,emulator_info::get_copyright_info());
	osd_printf_info("%s\n", emulator_info::get_disclaimer());
	emulator_info::printf_usage(emulator_info::get_appname(),emulator_info::get_gamenoun());
	osd_printf_info("\n\n"
			"        %s -showusage    for a brief list of options\n"
			"        %s -showconfig   for a list of configuration options\n"
			"        %s -listmedia    for a full list of supported media\n"
			"        %s -createconfig to create a %s.ini\n\n"
			"For usage instructions, please consult the files config.txt and windows.txt.\n",emulator_info::get_appname(),
			emulator_info::get_appname(),emulator_info::get_appname(),emulator_info::get_appname(),emulator_info::get_configname());
}


//-------------------------------------------------
//  display_suggestions - display 10 possible
//  matches for a given invalid gamename
//-------------------------------------------------

void cli_frontend::display_suggestions(const char *gamename)
{
}


//**************************************************************************
//  MEDIA IDENTIFIER
//**************************************************************************

//-------------------------------------------------
//  media_identifier - constructor
//-------------------------------------------------

media_identifier::media_identifier(cli_options &options)
	: m_drivlist(options),
		m_total(0),
		m_matches(0),
		m_nonroms(0)
{
}


//-------------------------------------------------
//  identify - identify a directory, ZIP file,
//  or raw file
//-------------------------------------------------

void media_identifier::identify(const char *filename)
{
	// first try to open as a directory
	osd_directory *directory = osd_opendir(filename);
	if (directory != nullptr)
	{
		// iterate over all files in the directory
		for (const osd_directory_entry *entry = osd_readdir(directory); entry != nullptr; entry = osd_readdir(directory))
			if (entry->type == ENTTYPE_FILE)
			{
				std::string curfile = std::string(filename).append(PATH_SEPARATOR).append(entry->name);
				identify(curfile.c_str());
			}

		// close the directory and be done
		osd_closedir(directory);
	}

	// if that failed, and the filename ends with .zip, identify as a ZIP file
	if (core_filename_ends_with(filename, ".7z"))
	{
		// first attempt to examine it as a valid _7Z file
		_7z_file *_7z = nullptr;
		_7z_error _7zerr = _7z_file_open(filename, &_7z);
		if (_7zerr == _7ZERR_NONE && _7z != nullptr)
		{
			// loop over entries in the .7z, skipping empty files and directories
			for (int i = 0; i < _7z->db.db.NumFiles; i++)
			{
				const CSzFileItem *f = _7z->db.db.Files + i;
				_7z->curr_file_idx = i;
				int namelen = SzArEx_GetFileNameUtf16(&_7z->db, i, nullptr);
				std::vector<UINT16> temp(namelen);
				dynamic_buffer temp2(namelen+1);
				UINT8* temp3 = &temp2[0];
				memset(temp3, 0x00, namelen);
				SzArEx_GetFileNameUtf16(&_7z->db, i, &temp[0]);
				// crude, need real UTF16->UTF8 conversion ideally
				for (int j=0;j<namelen;j++)
				{
					temp3[j] = (UINT8)temp[j];
				}

				if (!(f->IsDir) && (f->Size != 0))
				{
					// decompress data into RAM and identify it
					dynamic_buffer data(f->Size);
					_7zerr = _7z_file_decompress(_7z, &data[0], f->Size);
					if (_7zerr == _7ZERR_NONE)
						identify_data((const char*)&temp2[0], &data[0], f->Size);
				}
			}

			// close up
			_7z_file_close(_7z);
		}

		// clear out any cached files
		_7z_file_cache_clear();
	}
	else if (core_filename_ends_with(filename, ".zip"))
	{
		// first attempt to examine it as a valid ZIP file
		zip_file *zip = nullptr;
		zip_error ziperr = zip_file_open(filename, &zip);
		if (ziperr == ZIPERR_NONE && zip != nullptr)
		{
			// loop over entries in the ZIP, skipping empty files and directories
			for (const zip_file_header *entry = zip_file_first_file(zip); entry != nullptr; entry = zip_file_next_file(zip))
				if (entry->uncompressed_length != 0)
				{
					// decompress data into RAM and identify it
					dynamic_buffer data(entry->uncompressed_length);
					ziperr = zip_file_decompress(zip, &data[0], entry->uncompressed_length);
					if (ziperr == ZIPERR_NONE)
						identify_data(entry->filename, &data[0], entry->uncompressed_length);
				}

			// close up
			zip_file_close(zip);
		}

		// clear out any cached files
		zip_file_cache_clear();
	}

	// otherwise, identify as a raw file
	else
		identify_file(filename);
}


//-------------------------------------------------
//  identify_file - identify a file
//-------------------------------------------------

void media_identifier::identify_file(const char *name)
{
	// CHD files need to be parsed and their hashes extracted from the header
	if (core_filename_ends_with(name, ".chd"))
	{
		// output the name
		std::string basename;
		osd_printf_info("%-20s", core_filename_extract_base(basename, name).c_str());
		m_total++;

		// attempt to open as a CHD; fail if not
		chd_file chd;
		chd_error err = chd.open(name);
		if (err != CHDERR_NONE)
		{
			osd_printf_info("NOT A CHD\n");
			m_nonroms++;
			return;
		}

		// error on writable CHDs
		if (!chd.compressed())
		{
			osd_printf_info("is a writeable CHD\n");
			return;
		}

		// otherwise, get the hash collection for this CHD
		hash_collection hashes;
		if (chd.sha1() != sha1_t::null)
			hashes.add_sha1(chd.sha1());

		// determine whether this file exists
		int found = find_by_hash(hashes, chd.logical_bytes());
		if (found == 0)
			osd_printf_info("NO MATCH\n");
		else
			m_matches++;
	}

	// all other files have their hashes computed directly
	else
	{
		// load the file and process if it opens and has a valid length
		UINT32 length;
		void *data;
		file_error filerr = core_fload(name, &data, &length);
		if (filerr == FILERR_NONE && length > 0)
		{
			identify_data(name, reinterpret_cast<UINT8 *>(data), length);
			osd_free(data);
		}
	}
}


//-------------------------------------------------
//  identify_data - identify a buffer full of
//  data; if it comes from a .JED file, parse the
//  fusemap into raw data first
//-------------------------------------------------

void media_identifier::identify_data(const char *name, const UINT8 *data, int length)
{
	// if this is a '.jed' file, process it into raw bits first
	dynamic_buffer tempjed;
	jed_data jed;
	if (core_filename_ends_with(name, ".jed") && jed_parse(data, length, &jed) == JEDERR_NONE)
	{
		// now determine the new data length and allocate temporary memory for it
		length = jedbin_output(&jed, nullptr, 0);
		tempjed.resize(length);
		jedbin_output(&jed, &tempjed[0], length);
		data = &tempjed[0];
	}

	// compute the hash of the data
	hash_collection hashes;
	hashes.compute(data, length, hash_collection::HASH_TYPES_CRC_SHA1);

	// output the name
	m_total++;
	std::string basename;
	osd_printf_info("%-20s", core_filename_extract_base(basename, name).c_str());

	// see if we can find a match in the ROMs
	int found = find_by_hash(hashes, length);

	// if we didn't find it, try to guess what it might be
	if (found == 0)
		osd_printf_info("NO MATCH\n");

	// if we did find it, count it as a match
	else
		m_matches++;
}


//-------------------------------------------------
//  find_by_hash - scan for a file in the list
//  of drivers by hash
//-------------------------------------------------

int media_identifier::find_by_hash(const hash_collection &hashes, int length)
{
	int found = 0;
	std::unordered_set<std::string> listnames;
	std::unordered_set<std::string> shortnames;

	// iterate over drivers
	m_drivlist.reset();
	while (m_drivlist.next())
	{
		// iterate over devices, regions and files within the region */
		device_iterator deviter(m_drivlist.config().root_device());
		for (device_t *device = deviter.first(); device != nullptr; device = deviter.next())
		{
			if (shortnames.insert(device->shortname()).second)
			{
				for (const rom_entry *region = rom_first_region(*device); region != nullptr; region = rom_next_region(region))
					for (const rom_entry *rom = rom_first_file(region); rom != nullptr; rom = rom_next_file(rom))
					{
						hash_collection romhashes(ROM_GETHASHDATA(rom));
						if (!romhashes.flag(hash_collection::FLAG_NO_DUMP) && hashes == romhashes)
						{
							bool baddump = romhashes.flag(hash_collection::FLAG_BAD_DUMP);

							// output information about the match
							if (found)
								osd_printf_info("                    ");
							osd_printf_info("= %s%-20s  %-10s %s\n", baddump ? "(BAD) " : "", ROM_GETNAME(rom), m_drivlist.driver().name, m_drivlist.driver().description);
							found++;
						}
					}
			}
		}

		// next iterate over softlists
		software_list_device_iterator iter(m_drivlist.config().root_device());
		for (software_list_device *swlistdev = iter.first(); swlistdev != nullptr; swlistdev = iter.next())
		{
			if (listnames.insert(swlistdev->list_name()).second)
			{
				for (software_info *swinfo = swlistdev->first_software_info(); swinfo != nullptr; swinfo = swinfo->next())
					for (software_part *part = swinfo->first_part(); part != nullptr; part = part->next())
						for (const rom_entry *region = part->romdata(); region != nullptr; region = rom_next_region(region))
							for (const rom_entry *rom = rom_first_file(region); rom != nullptr; rom = rom_next_file(rom))
							{
								hash_collection romhashes(ROM_GETHASHDATA(rom));
								if (hashes == romhashes)
								{
									bool baddump = romhashes.flag(hash_collection::FLAG_BAD_DUMP);

									// output information about the match
									if (found)
										osd_printf_info("                    ");
									osd_printf_info("= %s%-20s  %s:%s %s\n", baddump ? "(BAD) " : "", ROM_GETNAME(rom), swlistdev->list_name(), swinfo->shortname(), swinfo->longname());
									found++;
								}
							}
			}
		}
	}

	return found;
}
