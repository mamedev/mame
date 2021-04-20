// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Paul Priest
/***************************************************************************

    info.cpp

    Dumps the MAME internal data as an Protobuf file.

***************************************************************************/

#include "emu.h"
#include "infoprotobuf.h"

#include "mameopts.h"

#include "machine/ram.h"
#include "sound/samples.h"

#include "config.h"
#include "drivenum.h"
#include "romload.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "corestr.h"
#include "proto/info.pb.h"
#include <google/protobuf/util/delimited_message_util.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <future>
#include <queue>
#include <type_traits>
#include <unordered_set>
#include <utility>


namespace {

//**************************************************************************
//  ANONYMOUS NAMESPACE PROTOTYPES
//**************************************************************************

class device_type_compare
{
public:
	bool operator()(const std::add_pointer_t<device_type> &lhs, const std::add_pointer_t<device_type> &rhs) const;
};

typedef std::set<std::add_pointer_t<device_type>, device_type_compare> device_type_set;

// internal helper
void output_header(infoprotobuf::mame* mame);

void output_one(infoprotobuf::machine *machine, driver_enumerator &drivlist, const game_driver &driver, device_type_set *devtypes);
void output_sampleof(infoprotobuf::machine *machine, device_t &device);
void output_bios(infoprotobuf::machine *machine, device_t const &device);
void output_rom(infoprotobuf::machine *machine, driver_enumerator *drivlist, const game_driver *driver, device_t &device);
void output_device_refs(infoprotobuf::machine *machine, device_t &root);
void output_sample(infoprotobuf::machine *machine, device_t &device);
void output_chips(infoprotobuf::machine *machine, device_t &device, const char *root_tag);
void output_display(infoprotobuf::machine *machine, device_t &device, machine_flags::type const *flags, const char *root_tag);
void output_sound(infoprotobuf::machine *machine, device_t &device);
void output_ioport_condition(infoprotobuf::condition *cond, const ioport_condition &condition);
void output_input(infoprotobuf::machine *machine, const ioport_list &portlist);
void output_switches(infoprotobuf::machine *machine, const ioport_list &portlist, const char *root_tag, int type, const char *outertag, const char *loctag, const char *innertag);
void output_ports(infoprotobuf::machine *machine, const ioport_list &portlist);
void output_adjusters(infoprotobuf::machine *machine, const ioport_list &portlist);
void output_driver(infoprotobuf::machine *machine, game_driver const &driver, device_t::feature_type unemulated, device_t::feature_type imperfect);
void output_features(infoprotobuf::machine *machine, device_type type, device_t::feature_type unemulated, device_t::feature_type imperfect);
void output_images(infoprotobuf::machine *machine, device_t &device, const char *root_tag);
void output_slots(infoprotobuf::machine *machine, machine_config &config, device_t &device, const char *root_tag, device_type_set *devtypes);
void output_software_lists(infoprotobuf::machine *machine, device_t &root, const char *root_tag);
void output_ramoptions(infoprotobuf::machine *machine, device_t &root);

void output_one_device(infoprotobuf::machine *machine, machine_config &config, device_t &device, const char *devtag);
void output_devices(infoprotobuf::mame *mame, emu_options &lookup_options, device_type_set const *filter);

const char *get_merge_name(driver_enumerator &drivlist, const game_driver &driver, util::hash_collection const &romhashes);


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// Protobuf feature names
constexpr std::pair<device_t::feature_type, char const *> f_feature_names[] = {
		{ device_t::feature::PROTECTION,    "protection"    },
		{ device_t::feature::TIMING,        "timing"        },
		{ device_t::feature::GRAPHICS,      "graphics"      },
		{ device_t::feature::PALETTE,       "palette"       },
		{ device_t::feature::SOUND,         "sound"         },
		{ device_t::feature::CAPTURE,       "capture"       },
		{ device_t::feature::CAMERA,        "camera"        },
		{ device_t::feature::MICROPHONE,    "microphone"    },
		{ device_t::feature::CONTROLS,      "controls"      },
		{ device_t::feature::KEYBOARD,      "keyboard"      },
		{ device_t::feature::MOUSE,         "mouse"         },
		{ device_t::feature::MEDIA,         "media"         },
		{ device_t::feature::DISK,          "disk"          },
		{ device_t::feature::PRINTER,       "printer"       },
		{ device_t::feature::TAPE,          "tape"          },
		{ device_t::feature::PUNCH,         "punch"         },
		{ device_t::feature::DRUM,          "drum"          },
		{ device_t::feature::ROM,           "rom"           },
		{ device_t::feature::COMMS,         "comms"         },
		{ device_t::feature::LAN,           "lan"           },
		{ device_t::feature::WAN,           "wan"           } };

} // anonymous namespace


//**************************************************************************
//  INFO PROTOBUF CREATOR
//**************************************************************************


//-------------------------------------------------
//  get_feature_name - get Protobuf name for feature
//-------------------------------------------------

char const *info_protobuf_creator::feature_name(device_t::feature_type feature)
{
	auto const found = std::lower_bound(
			std::begin(f_feature_names),
			std::end(f_feature_names),
			std::underlying_type_t<device_t::feature_type>(feature),
			[] (auto const &a, auto const &b)
			{
				return std::underlying_type_t<device_t::feature_type>(a.first) < b;
			});
	return ((std::end(f_feature_names) != found) && (found->first == feature)) ? found->second : nullptr;
}


//-------------------------------------------------
//  info_protobuf_creator - constructor
//-------------------------------------------------

info_protobuf_creator::info_protobuf_creator(emu_options const &options, bool dtd)
{
}


//-------------------------------------------------
//  output - print the Protobuf information for all
//  known machines matching a pattern
//-------------------------------------------------

void info_protobuf_creator::output(std::ostream& out, const std::vector<std::string> &patterns)
{
	if (patterns.empty())
	{
		// no patterns specified - show everything
		output(out);
	}
	else
	{
		// patterns specified - we have to filter output
		std::vector<bool> matched(patterns.size(), false);
		size_t exact_matches = 0;
		const auto filter = [&patterns, &matched, &exact_matches](const char *shortname, bool &done) -> bool
		{
			bool result = false;
			auto it = matched.begin();
			for (const std::string &pat : patterns)
			{
				if (!core_strwildcmp(pat.c_str(), shortname))
				{
					// this driver matches the pattern - tell the caller
					result = true;

					// did we see this particular pattern before?  if not, track that we have
					if (!*it)
					{
						*it = true;
						if (!core_iswildstr(pat.c_str()))
						{
							exact_matches++;

							// stop looking if we found everything specified
							if (exact_matches == patterns.size())
								done = true;
						}
					}
				}
				it++;
			}
			return result;
		};
		output(out, filter);

		// throw an error if there were unmatched patterns
		auto iter = std::find(matched.begin(), matched.end(), false);
		if (iter != matched.end())
		{
			int index = iter - matched.begin();
			throw emu_fatalerror(EMU_ERR_NO_SUCH_SYSTEM, "No matching machines found for '%s'", patterns[index]);
		}
	}
}


//-------------------------------------------------
//  output - print the Protobuf information for all
//  known (and filtered) machines
//-------------------------------------------------

void info_protobuf_creator::output(std::ostream& out, const std::function<bool(const char *shortname, bool &done)> &filter, bool include_devices)
{
	struct prepared_info
	{
		infoprotobuf::machine *m_pb_machine;
		device_type_set        m_dev_set;
	};

	// prepare a driver enumerator and the queue
	driver_enumerator drivlist(m_lookup_options);
	bool drivlist_done = false;
	bool filter_done = false;
	bool header_outputted = false;
	infoprotobuf::mame* mame = new infoprotobuf::mame();

	auto output_header_if_necessary = [this, &header_outputted](infoprotobuf::mame* mame)
	{
		if (!header_outputted)
		{
			output_header(mame);
			header_outputted = true;
		}
	};

	// only keep a device set when we're asked to track it
	std::unique_ptr<device_type_set> devfilter;
	if (include_devices && filter)
		devfilter = std::make_unique<device_type_set>();

	// prepare a queue of futures
	std::queue<std::future<prepared_info>> queue;

	// try enumerating drivers and outputting them
	while (!queue.empty() || (!drivlist_done && !filter_done))
	{
		// try populating the queue
		while (queue.size() < 20 && !drivlist_done && !filter_done)
		{
			if (!drivlist.next())
			{
				// at this point we are done enumerating through drivlist and it is no
				// longer safe to call next(), so record that we're done
				drivlist_done = true;
			}
			else if (!filter || filter(drivlist.driver().name, filter_done))
			{
				const game_driver &driver(drivlist.driver());
				infoprotobuf::machine *machine = mame->add_machine();
				std::future<prepared_info> future_pi = std::async(std::launch::async, [&drivlist, &driver, &devfilter, machine]
				{
					prepared_info result;
					std::ostringstream stream;
					output_one(machine, drivlist, driver, devfilter ? &result.m_dev_set : nullptr);
					result.m_pb_machine = machine;
					return result;
				});
				queue.push(std::move(future_pi));
			}
		}

		// now that we have the queue populated, try grabbing one (assuming that it is not empty)
		if (!queue.empty())
		{
			// wait for the future to complete and get the info
			prepared_info pi = queue.front().get();
			queue.pop();

			// emit the Protobuf
			output_header_if_necessary(mame);
			google::protobuf::util::SerializeDelimitedToOstream(*pi.m_pb_machine, &out);

			// merge devices into devfilter, if appropriate
			if (devfilter)
			{
				for (const auto &x : pi.m_dev_set)
					devfilter->insert(x);
			}
		}
	}

	// iterate through the device types if not everything matches a driver
	if (devfilter && !filter_done)
	{
		for (device_type type : registered_device_types)
		{
			if (!filter || filter(type.shortname(), filter_done))
				devfilter->insert(&type);

			if (filter_done)
				break;
		}
	}

	// output devices (both devices with roms and slot devices)
	if (include_devices && (!devfilter || !devfilter->empty()))
	{
		output_header_if_necessary(mame);

		output_devices(mame, m_lookup_options, devfilter.get());
	}
}


//**************************************************************************
//  ANONYMOUS NAMESPACE IMPLEMENTATION
//**************************************************************************

namespace
{

//-------------------------------------------------
//  output_header - set up the root element
//-------------------------------------------------

void output_header(infoprotobuf::mame *mame)
{
	mame->set_build(emulator_info::get_build_version());
#ifdef MAME_DEBUG
	mame->set_debug(true);
#else
	mame->set_debug(false);
#endif
	mame->set_mameconfig(CONFIG_VERSION);
}

//-------------------------------------------------
//  output_one - print the Protobuf information
//  for one particular machine driver
//-------------------------------------------------

void output_one(infoprotobuf::machine *machine, driver_enumerator &drivlist, const game_driver &driver, device_type_set *devtypes)
{
	machine_config config(driver, drivlist.options());
	device_enumerator iter(config.root_device());

	// allocate input ports and build overall emulation status
	ioport_list portlist;
	std::string errors;
	device_t::feature_type overall_unemulated(driver.type.unemulated_features());
	device_t::feature_type overall_imperfect(driver.type.imperfect_features());
	for (device_t &device : iter)
	{
		portlist.append(device, errors);
		overall_unemulated |= device.type().unemulated_features();
		overall_imperfect |= device.type().imperfect_features();

		if (devtypes && device.owner())
			devtypes->insert(&device.type());
	}

	// renumber player numbers for controller ports
	int player_offset = 0;
	// but treat keyboard count separately from players' number
	int kbd_offset = 0;
	for (device_t &device : iter)
	{
		int nplayers = 0;
		bool new_kbd = false;
		for (auto &port : portlist)
			if (&port.second->device() == &device)
				for (ioport_field &field : port.second->fields())
					if (field.type() >= IPT_START && field.type() < IPT_ANALOG_LAST)
					{
						if (field.type() == IPT_KEYBOARD)
						{
							if (!new_kbd)
								new_kbd = true;
							field.set_player(field.player() + kbd_offset);
						}
						else
						{
							nplayers = std::max(nplayers, field.player() + 1);
							field.set_player(field.player() + player_offset);
						}
					}
		player_offset += nplayers;
		if (new_kbd) kbd_offset++;
	}

	// print the header and the machine name
	machine->set_name(driver.name);

	// strip away any path information from the source_file and output it
	const char *start = strrchr(driver.type.source(), '/');
	if (!start)
		start = strrchr(driver.type.source(), '\\');
	start = start ? (start + 1) : driver.type.source();
	machine->set_sourcefile(start);

	// append bios and runnable flags
	if (driver.flags & machine_flags::IS_BIOS_ROOT)
		machine->set_isbios(true);
	if (driver.flags & machine_flags::MECHANICAL)
		machine->set_ismechanical(true);

	// display clone information
	int clone_of = drivlist.find(driver.parent);
	if (clone_of != -1 && !(drivlist.driver(clone_of).flags & machine_flags::IS_BIOS_ROOT))
		machine->set_cloneof(drivlist.driver(clone_of).name);
	if (clone_of != -1)
		machine->set_romof(drivlist.driver(clone_of).name);

	// display sample information and close the game tag
	output_sampleof(machine, config.root_device());

	// output game description
	if (driver.type.fullname() != nullptr)
		machine->set_description(driver.type.fullname());

	// print the year only if is a number or another allowed character (? or +)
	if (driver.year != nullptr && strspn(driver.year, "0123456789?+") == strlen(driver.year))
		machine->set_year(driver.year);

	// print the manufacturer information
	if (driver.manufacturer != nullptr)
		machine->set_manufacturer(driver.manufacturer);

	// now print various additional information
	output_bios(machine, config.root_device());
	output_rom(machine, &drivlist, &driver, config.root_device());
	output_device_refs(machine, config.root_device());
	output_sample(machine, config.root_device());
	output_chips(machine, config.root_device(), "");
	output_display(machine, config.root_device(), &driver.flags, "");
	output_sound(machine, config.root_device());
	output_input(machine, portlist);
	output_switches(machine, portlist, "", IPT_DIPSWITCH, "dipswitch", "diplocation", "dipvalue");
	output_switches(machine, portlist, "", IPT_CONFIG, "configuration", "conflocation", "confsetting");
	output_ports(machine, portlist);
	output_adjusters(machine, portlist);
	output_driver(machine, driver, overall_unemulated, overall_imperfect);
	output_features(machine, driver.type, overall_unemulated, overall_imperfect);
	output_images(machine, config.root_device(), "");
	output_slots(machine, config, config.root_device(), "", devtypes);
	output_software_lists(machine, config.root_device(), "");
	output_ramoptions(machine, config.root_device());
}


//-------------------------------------------------
//  output_one_device - print the Protobuf info for
//  a single device
//-------------------------------------------------

void output_one_device(infoprotobuf::machine* machine, machine_config &config, device_t &device, const char *devtag)
{
	bool has_speaker = false, has_input = false;
	// check if the device adds speakers to the system
	sound_interface_enumerator snditer(device);
	if (snditer.first() != nullptr)
		has_speaker = true;

	// generate input list and build overall emulation status
	ioport_list portlist;
	std::string errors;
	device_t::feature_type overall_unemulated(device.type().unemulated_features());
	device_t::feature_type overall_imperfect(device.type().imperfect_features());
	for (device_t &dev : device_enumerator(device))
	{
		portlist.append(dev, errors);
		overall_unemulated |= dev.type().unemulated_features();
		overall_imperfect |= dev.type().imperfect_features();
	}

	// check if the device adds player inputs (other than dsw and configs) to the system
	for (auto &port : portlist)
		for (ioport_field const &field : port.second->fields())
			if (field.type() >= IPT_START1 && field.type() < IPT_UI_FIRST)
			{
				has_input = true;
				break;
			}

	// start to output info
	machine->set_name(device.shortname());
	std::string src(device.source());
	strreplace(src,"../", "");
	machine->set_sourcefile(src);
	machine->set_isdevice(true);
	machine->set_runnable(false);
	output_sampleof(machine, device);
	machine->set_description(device.name());

	output_bios(machine, device);
	output_rom(machine, nullptr, nullptr, device);
	output_device_refs(machine, device);

	if (device.type().type() != typeid(samples_device)) // ignore samples_device itself
		output_sample(machine, device);

	output_chips(machine, device, devtag);
	output_display(machine, device, nullptr, devtag);
	if (has_speaker)
		output_sound(machine, device);
	if (has_input)
		output_input(machine, portlist);
	output_switches(machine, portlist, devtag, IPT_DIPSWITCH, "dipswitch", "diplocation", "dipvalue");
	output_switches(machine, portlist, devtag, IPT_CONFIG, "configuration", "conflocation", "confsetting");
	output_adjusters(machine, portlist);
	output_features(machine, device.type(), overall_unemulated, overall_imperfect);
	output_images(machine, device, devtag);
	output_slots(machine, config, device, devtag, nullptr);
	output_software_lists(machine, device, devtag);
}


//-------------------------------------------------
//  output_devices - print the Protobuf info for
//  registered device types
//-------------------------------------------------

void output_devices(infoprotobuf::mame *mame, emu_options &lookup_options, device_type_set const *filter)
{
	// get config for empty machine
	machine_config config(GAME_NAME(___empty), lookup_options);

	auto const action = [&config, mame] (device_type type)
			{
				// add it at the root of the machine config
				device_t *dev;
				{
					machine_config::token const tok(config.begin_configuration(config.root_device()));
					dev = config.device_add("_tmp", type, 0);
				}

				// notify this device and all its subdevices that they are now configured
				for (device_t &device : device_enumerator(*dev))
					if (!device.configured())
						device.config_complete();

				// print details and remove it
				infoprotobuf::machine* machine = mame->add_machine();
				output_one_device(machine, config, *dev, dev->tag());
				machine_config::token const tok(config.begin_configuration(config.root_device()));
				config.device_remove("_tmp");
			};

	// run through devices
	if (filter)
	{
		for (std::add_pointer_t<device_type> type : *filter) action(*type);
	}
	else
	{
		for (device_type type : registered_device_types) action(type);
	}
}


//------------------------------------------------
//  output_device_refs - when a machine uses a
//  subdevice, print a reference
//-------------------------------------------------

void output_device_refs(infoprotobuf::machine *machine, device_t &root)
{
	for (device_t &device : device_enumerator(root))
		if (&device != &root) {
			machine->add_device_ref()->set_name(device.shortname());
		}
}


//------------------------------------------------
//  output_sampleof - print the 'sampleof'
//  attribute, if appropriate
//-------------------------------------------------

void output_sampleof(infoprotobuf::machine *machine, device_t &device)
{
	// iterate over sample devices
	for (samples_device &samples : samples_device_enumerator(device))
	{
		samples_iterator sampiter(samples);
		if (sampiter.altbasename() != nullptr)
		{
			machine->set_sampleof(sampiter.altbasename());

			// must stop here, as there can only be one attribute of the same name
			return;
		}
	}
}


//-------------------------------------------------
//  output_bios - print BIOS sets for a device
//-------------------------------------------------

void output_bios(infoprotobuf::machine* machine, device_t const &device)
{
	// first determine the default BIOS name
	char const *defaultname(nullptr);
	for (tiny_rom_entry const *rom = device.rom_region(); rom && !ROMENTRY_ISEND(rom); ++rom)
	{
		if (ROMENTRY_ISDEFAULT_BIOS(rom))
			defaultname = rom->name;
	}

	// iterate over ROM entries and look for BIOSes
	for (romload::system_bios const &bios : romload::entries(device.rom_region()).get_system_bioses())
	{
		infoprotobuf::biosset *biosset = machine->add_biosset();
		biosset->set_name(bios.get_name());
		biosset->set_description(bios.get_description());
		if (defaultname && !std::strcmp(defaultname, bios.get_name()))
			biosset->set_default_(true);
	}
}


//-------------------------------------------------
//  output_rom - print the roms section of
//  the Protobuf output
//-------------------------------------------------

void output_rom(infoprotobuf::machine *machine, driver_enumerator *drivlist, const game_driver *driver, device_t &device)
{
	enum class type { BIOS, NORMAL, DISK };
	std::map<u32, char const *> biosnames;
	bool bios_scanned(false);
	auto const get_biosname =
			[&biosnames, &bios_scanned] (tiny_rom_entry const *rom) -> char const *
			{
				u32 const biosflags(ROM_GETBIOSFLAGS(rom));
				std::map<u32, char const *>::const_iterator const found(biosnames.find(biosflags));
				if (biosnames.end() != found)
					return found->second;

				char const *result(nullptr);
				if (!bios_scanned)
				{
					for (++rom; !ROMENTRY_ISEND(rom); ++rom)
					{
						if (ROMENTRY_ISSYSTEM_BIOS(rom))
						{
							u32 const biosno(ROM_GETBIOSFLAGS(rom));
							biosnames.emplace(biosno, rom->name);
							if (biosflags == biosno)
								result = rom->name;
						}
					}
					bios_scanned = true;
				}
				return result;
			};
	auto const rom_file_size = // FIXME: need a common way to do this without the cost of allocating rom_entry
			[] (tiny_rom_entry const *romp) -> u32
			{
				u32 maxlength = 0;

				// loop until we run out of reloads
				do
				{
					// loop until we run out of continues/ignores */
					u32 curlength(ROM_GETLENGTH(romp++));
					while (ROMENTRY_ISCONTINUE(romp) || ROMENTRY_ISIGNORE(romp))
						curlength += ROM_GETLENGTH(romp++);

					// track the maximum length
					maxlength = (std::max)(maxlength, curlength);
				}
				while (ROMENTRY_ISRELOAD(romp));

				return maxlength;
			};

	// iterate over 3 different ROM "types": BIOS, ROMs, DISKs
	bool const do_merge_name = drivlist && dynamic_cast<driver_device *>(&device);
	for (type pass : { type::BIOS, type::NORMAL, type::DISK })
	{
		tiny_rom_entry const *region(nullptr);
		for (tiny_rom_entry const *rom = device.rom_region(); rom && !ROMENTRY_ISEND(rom); ++rom)
		{
			if (ROMENTRY_ISREGION(rom))
				region = rom;
			else if (ROMENTRY_ISSYSTEM_BIOS(rom))
				biosnames.emplace(ROM_GETBIOSFLAGS(rom), rom->name);

			if (!ROMENTRY_ISFILE(rom))
				continue;

			// only list disks on the disk pass
			bool const is_disk = ROMREGION_ISDISKDATA(region);
			if ((type::DISK == pass) != is_disk)
				continue;

			// BIOS ROMs only apply to bioses
			// FIXME: disk images associated with a system BIOS will never be listed
			u32 const biosno(ROM_GETBIOSFLAGS(rom));
			if ((type::BIOS == pass) != bool(biosno))
				continue;
			char const *const bios_name((!is_disk && biosno) ? get_biosname(rom) : nullptr);

			// if we have a valid ROM and we are a clone, see if we can find the parent ROM
			util::hash_collection const hashes(rom->hashdata);
			char const *const merge_name((do_merge_name && !hashes.flag(util::hash_collection::FLAG_NO_DUMP)) ? get_merge_name(*drivlist, *driver, hashes) : nullptr);

			char const* const name(rom->name);
			// opening tag
			if (is_disk) {
				infoprotobuf::disk *disk = machine->add_disk();
				// add name, merge, bios, and size tags */
				if (name && name[0])
					disk->set_name(name);
				if (merge_name)
					disk->set_merge(merge_name);

				// append a region name
				disk->set_region(region->name);

				// dump checksum information only if there is a known dump
				if (!hashes.flag(util::hash_collection::FLAG_NO_DUMP)) {
					util::sha1_t sha1;
					if (hashes.sha1(sha1)) {
						disk->set_sha1(sha1.as_string());
					}

					if (hashes.flag(util::hash_collection::FLAG_BAD_DUMP))
						disk->set_status(infoprotobuf::status::baddump);
					else if (hashes.flag(util::hash_collection::FLAG_NO_DUMP))
						disk->set_status(infoprotobuf::status::nodump);
					else
						disk->set_status(infoprotobuf::status::good);
				}
				else {
					disk->set_status(infoprotobuf::status::nodump);
				}

				// add optional flag
				if (ROM_ISOPTIONAL(rom))
					disk->set_optional(true);

				// for disk entries, add the disk index
				// TODO: Should we just store as an int?
				disk->set_index(util::string_format("%x", DISK_GETINDEX(rom)));
				disk->set_writable(DISK_ISREADONLY(rom));
			}
			else {
				infoprotobuf::rom *rompb = machine->add_rom();

				if (name && name[0])
					rompb->set_name(name);
				if (merge_name)
					rompb->set_merge(merge_name);
				if (bios_name)
					rompb->set_bios(bios_name);

				rompb->set_size(rom_file_size(rom));

				// append a region name
				rompb->set_region(region->name);

				// for non-disk entries, print offset
				// TODO: Should we just store as an int?
				rompb->set_offset(util::string_format("%x", ROM_GETOFFSET(rom)));

				// dump checksum information only if there is a known dump
				if (!hashes.flag(util::hash_collection::FLAG_NO_DUMP)) {
					uint32_t crc = 0;
					if (hashes.crc(crc)) {
						rompb->set_crc(crc);
					}
					util::sha1_t sha1;
					if (hashes.sha1(sha1)) {
						rompb->set_sha1(sha1.as_string());
					}

					if (hashes.flag(util::hash_collection::FLAG_BAD_DUMP))
						rompb->set_status(infoprotobuf::status::baddump);
					else if (hashes.flag(util::hash_collection::FLAG_NO_DUMP))
						rompb->set_status(infoprotobuf::status::nodump);
					else
						rompb->set_status(infoprotobuf::status::good);
				}
				else {
					rompb->set_status(infoprotobuf::status::nodump);
				}

				// add optional flag
				if (ROM_ISOPTIONAL(rom))
					rompb->set_optional(true);
			}
		}
		bios_scanned = true;
	}
}


//-------------------------------------------------
//  output_sample - print a list of all
//  samples referenced by a game_driver
//-------------------------------------------------

void output_sample(infoprotobuf::machine *machine, device_t &device)
{
	// iterate over sample devices
	for (samples_device &samples : samples_device_enumerator(device))
	{
		samples_iterator iter(samples);
		std::unordered_set<std::string> already_printed;
		for (const char *samplename = iter.first(); samplename != nullptr; samplename = iter.next())
		{
			// filter out duplicates
			if (!already_printed.insert(samplename).second)
				continue;

			// output the sample name
			machine->add_sample()->set_name(samplename);
		}
	}
}


/*-------------------------------------------------
    output_chips - print a list of CPU and
    sound chips used by a game
-------------------------------------------------*/

void output_chips(infoprotobuf::machine *machine, device_t &device, const char *root_tag)
{
	// iterate over executable devices
	for (device_execute_interface &exec : execute_interface_enumerator(device))
	{
		if (strcmp(exec.device().tag(), device.tag()))
		{
			std::string newtag(exec.device().tag()), oldtag(":");
			newtag = newtag.substr(newtag.find(oldtag.append(root_tag)) + oldtag.length());

			infoprotobuf::chip *chip = machine->add_chip();
			chip->set_type(infoprotobuf::chip_type::cpu);
			chip->set_tag(newtag);
			chip->set_name(exec.device().name());
			chip->set_clock(exec.device().clock());
		}
	}

	// iterate over sound devices
	for (device_sound_interface &sound : sound_interface_enumerator(device))
	{
		if (strcmp(sound.device().tag(), device.tag()) != 0 && sound.issound())
		{
			std::string newtag(sound.device().tag()), oldtag(":");
			newtag = newtag.substr(newtag.find(oldtag.append(root_tag)) + oldtag.length());

			infoprotobuf::chip *chip = machine->add_chip();
			chip->set_type(infoprotobuf::chip_type::audio);
			chip->set_tag(newtag);
			chip->set_name(sound.device().name());
			chip->set_clock(sound.device().clock());
		}
	}
}


//-------------------------------------------------
//  output_display - print a list of all the
//  displays
//-------------------------------------------------

void output_display(infoprotobuf::machine *machine, device_t &device, machine_flags::type const *flags, const char *root_tag)
{
	// iterate over screens
	for (const screen_device &screendev : screen_device_enumerator(device))
	{
		if (strcmp(screendev.tag(), device.tag()))
		{
			std::string newtag(screendev.tag()), oldtag(":");
			newtag = newtag.substr(newtag.find(oldtag.append(root_tag)) + oldtag.length());

			infoprotobuf::display *display = machine->add_display();
			display->set_tag(newtag);

			switch (screendev.screen_type())
			{
			case SCREEN_TYPE_RASTER:		display->set_type(infoprotobuf::rastar);  break;
				case SCREEN_TYPE_VECTOR:    display->set_type(infoprotobuf::vector);  break;
				case SCREEN_TYPE_LCD:       display->set_type(infoprotobuf::lcd);     break;
				case SCREEN_TYPE_SVG:       display->set_type(infoprotobuf::svg);     break;
				default:                    display->set_type(infoprotobuf::unknown); break;
			}

			// output the orientation as a string
			switch (screendev.orientation())
			{
			case ORIENTATION_FLIP_X:
				display->set_flipx(true);
				display->set_rotate(infoprotobuf::deg0);
				break;
			case ORIENTATION_FLIP_Y:
				display->set_flipx(true);
				display->set_rotate(infoprotobuf::deg180);
				break;
			case ORIENTATION_FLIP_X|ORIENTATION_FLIP_Y:
				display->set_rotate(infoprotobuf::deg180);
				break;
			case ORIENTATION_SWAP_XY:
				display->set_flipx(true);
				display->set_rotate(infoprotobuf::deg90);
				break;
			case ORIENTATION_SWAP_XY|ORIENTATION_FLIP_X:
				display->set_rotate(infoprotobuf::deg90);
				break;
			case ORIENTATION_SWAP_XY|ORIENTATION_FLIP_Y:
				display->set_rotate(infoprotobuf::deg270);
				break;
			case ORIENTATION_SWAP_XY|ORIENTATION_FLIP_X|ORIENTATION_FLIP_Y:
				display->set_flipx(true);
				display->set_rotate(infoprotobuf::deg270);
				break;
			default:
				display->set_rotate(infoprotobuf::deg0);
				break;
			}

			// output width and height only for games that are not vector
			if (screendev.screen_type() != SCREEN_TYPE_VECTOR)
			{
				const rectangle &visarea = screendev.visible_area();
				display->set_width(visarea.width());
				display->set_height(visarea.height());
			}

			// output refresh rate
			display->set_refresh(ATTOSECONDS_TO_HZ(screendev.refresh_attoseconds()));

			// output raw video parameters only for games that are not vector
			// and had raw parameters specified
			if (screendev.screen_type() != SCREEN_TYPE_VECTOR && !screendev.oldstyle_vblank_supplied())
			{
				int pixclock = screendev.width() * screendev.height() * ATTOSECONDS_TO_HZ(screendev.refresh_attoseconds());

				display->set_pixclock(pixclock);

				display->set_htotal(screendev.width());
				display->set_hbend(screendev.visible_area().min_x);
				display->set_hbstart(screendev.visible_area().max_x+1);
				display->set_vtotal(screendev.height());
				display->set_vbend(screendev.visible_area().min_y);
				display->set_vbstart(screendev.visible_area().max_y+1);
			}
		}
	}
}


//-------------------------------------------------
//  output_sound - print a list of all the
//  speakers
//------------------------------------------------

void output_sound(infoprotobuf::machine *machine, device_t &device)
{
	speaker_device_enumerator spkiter(device);
	int speakers = spkiter.count();

	// if we have no sound, zero m_output the speaker count
	sound_interface_enumerator snditer(device);
	if (snditer.first() == nullptr)
		speakers = 0;

	machine->mutable_sound()->set_channels(speakers);
}


//-------------------------------------------------
//  output_ioport_condition - print condition
//  required to use I/O port field/setting
//-------------------------------------------------

void output_ioport_condition(infoprotobuf::condition *cond, const ioport_condition &condition)
{
	switch (condition.condition())
	{
	case ioport_condition::ALWAYS:          throw false;
	case ioport_condition::EQUALS:
		cond->set_relation(infoprotobuf::condition_condition_relation::condition_condition_relation_eq);
		break;
	case ioport_condition::NOTEQUALS:
		cond->set_relation(infoprotobuf::condition_condition_relation::condition_condition_relation_ne);
		break;
	case ioport_condition::GREATERTHAN:
		cond->set_relation(infoprotobuf::condition_condition_relation::condition_condition_relation_gt);
		break;
	case ioport_condition::NOTGREATERTHAN:
		cond->set_relation(infoprotobuf::condition_condition_relation::condition_condition_relation_le);
		break;
	case ioport_condition::LESSTHAN:
		cond->set_relation(infoprotobuf::condition_condition_relation::condition_condition_relation_lt);
		break;
	case ioport_condition::NOTLESSTHAN:
		cond->set_relation(infoprotobuf::condition_condition_relation::condition_condition_relation_ge);
		break;
	}

	cond->set_tag(condition.tag());
	cond->set_mask(condition.mask());
	cond->set_value(condition.value());
}

//-------------------------------------------------
//  output_input - print a summary of a game's
//  input
//-------------------------------------------------

void output_input(infoprotobuf::machine *machine, const ioport_list &portlist)
{
	// enumerated list of control types
	// NOTE: the order is chosen so that 'spare' button inputs are assigned to the
	// most-likely-correct input device when info is output (you can think of it as
	// a sort of likelihood order of having buttons)
	enum
	{
		CTRL_DIGITAL_BUTTONS,
		CTRL_DIGITAL_JOYSTICK,
		CTRL_ANALOG_JOYSTICK,
		CTRL_ANALOG_LIGHTGUN,
		CTRL_ANALOG_DIAL,
		CTRL_ANALOG_POSITIONAL,
		CTRL_ANALOG_TRACKBALL,
		CTRL_ANALOG_MOUSE,
		CTRL_ANALOG_PADDLE,
		CTRL_ANALOG_PEDAL,
		CTRL_DIGITAL_KEYPAD,
		CTRL_DIGITAL_KEYBOARD,
		CTRL_DIGITAL_MAHJONG,
		CTRL_DIGITAL_HANAFUDA,
		CTRL_DIGITAL_GAMBLING,
		CTRL_COUNT
	};

	enum
	{
		CTRL_P1,
		CTRL_P2,
		CTRL_P3,
		CTRL_P4,
		CTRL_P5,
		CTRL_P6,
		CTRL_P7,
		CTRL_P8,
		CTRL_P9,
		CTRL_P10,
		CTRL_PCOUNT
	};

	// directions
	const uint8_t DIR_UP = 0x01;
	const uint8_t DIR_DOWN = 0x02;
	const uint8_t DIR_LEFT = 0x04;
	const uint8_t DIR_RIGHT = 0x08;

	// initialize the list of control types
	struct
	{
		const char *    type;           // general type of input
		int             player;         // player which the input belongs to
		int             nbuttons;       // total number of buttons
		int             reqbuttons;     // total number of non-optional buttons
		int             maxbuttons;     // max index of buttons (using IPT_BUTTONn) [probably to be removed soonish]
		int             ways;           // directions for joystick
		bool            analog;         // is analog input?
		uint8_t           helper[3];      // for dual joysticks [possibly to be removed soonish]
		int32_t           min;            // analog minimum value
		int32_t           max;            // analog maximum value
		int32_t           sensitivity;    // default analog sensitivity
		int32_t           keydelta;       // default analog keydelta
		bool            reverse;        // default analog reverse setting
	} control_info[CTRL_COUNT * CTRL_PCOUNT];

	memset(&control_info, 0, sizeof(control_info));

	// tracking info as we iterate
	int nplayer = 0;
	int ncoin = 0;
	bool service = false;
	bool tilt = false;

	// iterate over the ports
	for (auto &port : portlist)
	{
		int ctrl_type = CTRL_DIGITAL_BUTTONS;
		bool ctrl_analog = false;
		for (ioport_field &field : port.second->fields())
		{
			// track the highest player number
			if (nplayer < field.player() + 1)
				nplayer = field.player() + 1;

			// switch off of the type
			switch (field.type())
			{
			// map joysticks
			case IPT_JOYSTICK_UP:
				ctrl_type = CTRL_DIGITAL_JOYSTICK;
				control_info[field.player() * CTRL_COUNT + ctrl_type].type = "joy";
				control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
				control_info[field.player() * CTRL_COUNT + ctrl_type].ways = field.way();
				control_info[field.player() * CTRL_COUNT + ctrl_type].helper[0] |= DIR_UP;
				break;
			case IPT_JOYSTICK_DOWN:
				ctrl_type = CTRL_DIGITAL_JOYSTICK;
				control_info[field.player() * CTRL_COUNT + ctrl_type].type = "joy";
				control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
				control_info[field.player() * CTRL_COUNT + ctrl_type].ways = field.way();
				control_info[field.player() * CTRL_COUNT + ctrl_type].helper[0] |= DIR_DOWN;
				break;
			case IPT_JOYSTICK_LEFT:
				ctrl_type = CTRL_DIGITAL_JOYSTICK;
				control_info[field.player() * CTRL_COUNT + ctrl_type].type = "joy";
				control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
				control_info[field.player() * CTRL_COUNT + ctrl_type].ways = field.way();
				control_info[field.player() * CTRL_COUNT + ctrl_type].helper[0] |= DIR_LEFT;
				break;
			case IPT_JOYSTICK_RIGHT:
				ctrl_type = CTRL_DIGITAL_JOYSTICK;
				control_info[field.player() * CTRL_COUNT + ctrl_type].type = "joy";
				control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
				control_info[field.player() * CTRL_COUNT + ctrl_type].ways = field.way();
				control_info[field.player() * CTRL_COUNT + ctrl_type].helper[0] |= DIR_RIGHT;
				break;

			case IPT_JOYSTICKLEFT_UP:
				ctrl_type = CTRL_DIGITAL_JOYSTICK;
				control_info[field.player() * CTRL_COUNT + ctrl_type].type = "joy";
				control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
				control_info[field.player() * CTRL_COUNT + ctrl_type].ways = field.way();
				control_info[field.player() * CTRL_COUNT + ctrl_type].helper[1] |= DIR_UP;
				break;
			case IPT_JOYSTICKLEFT_DOWN:
				ctrl_type = CTRL_DIGITAL_JOYSTICK;
				control_info[field.player() * CTRL_COUNT + ctrl_type].type = "joy";
				control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
				control_info[field.player() * CTRL_COUNT + ctrl_type].ways = field.way();
				control_info[field.player() * CTRL_COUNT + ctrl_type].helper[1] |= DIR_DOWN;
				break;
			case IPT_JOYSTICKLEFT_LEFT:
				ctrl_type = CTRL_DIGITAL_JOYSTICK;
				control_info[field.player() * CTRL_COUNT + ctrl_type].type = "joy";
				control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
				control_info[field.player() * CTRL_COUNT + ctrl_type].ways = field.way();
				control_info[field.player() * CTRL_COUNT + ctrl_type].helper[1] |= DIR_LEFT;
				break;
			case IPT_JOYSTICKLEFT_RIGHT:
				ctrl_type = CTRL_DIGITAL_JOYSTICK;
				control_info[field.player() * CTRL_COUNT + ctrl_type].type = "joy";
				control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
				control_info[field.player() * CTRL_COUNT + ctrl_type].ways = field.way();
				control_info[field.player() * CTRL_COUNT + ctrl_type].helper[1] |= DIR_RIGHT;
				break;

			case IPT_JOYSTICKRIGHT_UP:
				ctrl_type = CTRL_DIGITAL_JOYSTICK;
				control_info[field.player() * CTRL_COUNT + ctrl_type].type = "joy";
				control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
				control_info[field.player() * CTRL_COUNT + ctrl_type].ways = field.way();
				control_info[field.player() * CTRL_COUNT + ctrl_type].helper[2] |= DIR_UP;
				break;
			case IPT_JOYSTICKRIGHT_DOWN:
				ctrl_type = CTRL_DIGITAL_JOYSTICK;
				control_info[field.player() * CTRL_COUNT + ctrl_type].type = "joy";
				control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
				control_info[field.player() * CTRL_COUNT + ctrl_type].ways = field.way();
				control_info[field.player() * CTRL_COUNT + ctrl_type].helper[2] |= DIR_DOWN;
				break;
			case IPT_JOYSTICKRIGHT_LEFT:
				ctrl_type = CTRL_DIGITAL_JOYSTICK;
				control_info[field.player() * CTRL_COUNT + ctrl_type].type = "joy";
				control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
				control_info[field.player() * CTRL_COUNT + ctrl_type].ways = field.way();
				control_info[field.player() * CTRL_COUNT + ctrl_type].helper[2] |= DIR_LEFT;
				break;
			case IPT_JOYSTICKRIGHT_RIGHT:
				ctrl_type = CTRL_DIGITAL_JOYSTICK;
				control_info[field.player() * CTRL_COUNT + ctrl_type].type = "joy";
				control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
				control_info[field.player() * CTRL_COUNT + ctrl_type].ways = field.way();
				control_info[field.player() * CTRL_COUNT + ctrl_type].helper[2] |= DIR_RIGHT;
				break;

			// map analog inputs
			case IPT_AD_STICK_X:
			case IPT_AD_STICK_Y:
			case IPT_AD_STICK_Z:
				ctrl_analog = true;
				ctrl_type = CTRL_ANALOG_JOYSTICK;
				control_info[field.player() * CTRL_COUNT + ctrl_type].type = "stick";
				control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
				control_info[field.player() * CTRL_COUNT + ctrl_type].analog = true;
				break;

			case IPT_PADDLE:
			case IPT_PADDLE_V:
				ctrl_analog = true;
				ctrl_type = CTRL_ANALOG_PADDLE;
				control_info[field.player() * CTRL_COUNT + ctrl_type].type = "paddle";
				control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
				control_info[field.player() * CTRL_COUNT + ctrl_type].analog = true;
				break;

			case IPT_PEDAL:
			case IPT_PEDAL2:
			case IPT_PEDAL3:
				ctrl_analog = true;
				ctrl_type = CTRL_ANALOG_PEDAL;
				control_info[field.player() * CTRL_COUNT + ctrl_type].type = "pedal";
				control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
				control_info[field.player() * CTRL_COUNT + ctrl_type].analog = true;
				break;

			case IPT_LIGHTGUN_X:
			case IPT_LIGHTGUN_Y:
				ctrl_analog = true;
				ctrl_type = CTRL_ANALOG_LIGHTGUN;
				control_info[field.player() * CTRL_COUNT + ctrl_type].type = "lightgun";
				control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
				control_info[field.player() * CTRL_COUNT + ctrl_type].analog = true;
				break;

			case IPT_POSITIONAL:
			case IPT_POSITIONAL_V:
				ctrl_analog = true;
				ctrl_type = CTRL_ANALOG_POSITIONAL;
				control_info[field.player() * CTRL_COUNT + ctrl_type].type = "positional";
				control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
				control_info[field.player() * CTRL_COUNT + ctrl_type].analog = true;
				break;

			case IPT_DIAL:
			case IPT_DIAL_V:
				ctrl_analog = true;
				ctrl_type = CTRL_ANALOG_DIAL;
				control_info[field.player() * CTRL_COUNT + ctrl_type].type = "dial";
				control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
				control_info[field.player() * CTRL_COUNT + ctrl_type].analog = true;
				break;

			case IPT_TRACKBALL_X:
			case IPT_TRACKBALL_Y:
				ctrl_analog = true;
				ctrl_type = CTRL_ANALOG_TRACKBALL;
				control_info[field.player() * CTRL_COUNT + ctrl_type].type = "trackball";
				control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
				control_info[field.player() * CTRL_COUNT + ctrl_type].analog = true;
				break;

			case IPT_MOUSE_X:
			case IPT_MOUSE_Y:
				ctrl_analog = true;
				ctrl_type = CTRL_ANALOG_MOUSE;
				control_info[field.player() * CTRL_COUNT + ctrl_type].type = "mouse";
				control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
				control_info[field.player() * CTRL_COUNT + ctrl_type].analog = true;
				break;

			// map buttons
			case IPT_BUTTON1:
			case IPT_BUTTON2:
			case IPT_BUTTON3:
			case IPT_BUTTON4:
			case IPT_BUTTON5:
			case IPT_BUTTON6:
			case IPT_BUTTON7:
			case IPT_BUTTON8:
			case IPT_BUTTON9:
			case IPT_BUTTON10:
			case IPT_BUTTON11:
			case IPT_BUTTON12:
			case IPT_BUTTON13:
			case IPT_BUTTON14:
			case IPT_BUTTON15:
			case IPT_BUTTON16:
				ctrl_analog = false;
				if (control_info[field.player() * CTRL_COUNT + ctrl_type].type == nullptr)
				{
					control_info[field.player() * CTRL_COUNT + ctrl_type].type = "only_buttons";
					control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
					control_info[field.player() * CTRL_COUNT + ctrl_type].analog = false;
				}
				control_info[field.player() * CTRL_COUNT + ctrl_type].maxbuttons = std::max(control_info[field.player() * CTRL_COUNT + ctrl_type].maxbuttons, field.type() - IPT_BUTTON1 + 1);
				control_info[field.player() * CTRL_COUNT + ctrl_type].nbuttons++;
				if (!field.optional())
					control_info[field.player() * CTRL_COUNT + ctrl_type].reqbuttons++;
				break;

			// track maximum coin index
			case IPT_COIN1:
			case IPT_COIN2:
			case IPT_COIN3:
			case IPT_COIN4:
			case IPT_COIN5:
			case IPT_COIN6:
			case IPT_COIN7:
			case IPT_COIN8:
			case IPT_COIN9:
			case IPT_COIN10:
			case IPT_COIN11:
			case IPT_COIN12:
				ncoin = std::max(ncoin, field.type() - IPT_COIN1 + 1);
				break;

			// track presence of keypads and keyboards
			case IPT_KEYPAD:
				ctrl_type = CTRL_DIGITAL_KEYPAD;
				control_info[field.player() * CTRL_COUNT + ctrl_type].type = "keypad";
				control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
				control_info[field.player() * CTRL_COUNT + ctrl_type].nbuttons++;
				if (!field.optional())
					control_info[field.player() * CTRL_COUNT + ctrl_type].reqbuttons++;
				break;

			case IPT_KEYBOARD:
				ctrl_type = CTRL_DIGITAL_KEYBOARD;
				control_info[field.player() * CTRL_COUNT + ctrl_type].type = "keyboard";
				control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
				control_info[field.player() * CTRL_COUNT + ctrl_type].nbuttons++;
				if (!field.optional())
					control_info[field.player() * CTRL_COUNT + ctrl_type].reqbuttons++;
				break;

			// additional types
			case IPT_SERVICE:
				service = true;
				break;

			case IPT_TILT:
				tilt = true;
				break;

			default:
				if (field.type() > IPT_MAHJONG_FIRST && field.type() < IPT_MAHJONG_LAST)
				{
					ctrl_type = CTRL_DIGITAL_MAHJONG;
					control_info[field.player() * CTRL_COUNT + ctrl_type].type = "mahjong";
					control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
					control_info[field.player() * CTRL_COUNT + ctrl_type].nbuttons++;
					if (!field.optional())
						control_info[field.player() * CTRL_COUNT + ctrl_type].reqbuttons++;
				}
				else if (field.type() > IPT_HANAFUDA_FIRST && field.type() < IPT_HANAFUDA_LAST)
				{
					ctrl_type = CTRL_DIGITAL_HANAFUDA;
					control_info[field.player() * CTRL_COUNT + ctrl_type].type = "hanafuda";
					control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
					control_info[field.player() * CTRL_COUNT + ctrl_type].nbuttons++;
					if (!field.optional())
						control_info[field.player() * CTRL_COUNT + ctrl_type].reqbuttons++;
				}
				else if (field.type() > IPT_GAMBLING_FIRST && field.type() < IPT_GAMBLING_LAST)
				{
					ctrl_type = CTRL_DIGITAL_GAMBLING;
					control_info[field.player() * CTRL_COUNT + ctrl_type].type = "gambling";
					control_info[field.player() * CTRL_COUNT + ctrl_type].player = field.player() + 1;
					control_info[field.player() * CTRL_COUNT + ctrl_type].nbuttons++;
					if (!field.optional())
						control_info[field.player() * CTRL_COUNT + ctrl_type].reqbuttons++;
				}
				break;
			}

			if (ctrl_analog)
			{
				// get the analog stats
				if (field.minval() != 0)
					control_info[field.player() * CTRL_COUNT + ctrl_type].min = field.minval();
				if (field.maxval() != 0)
					control_info[field.player() * CTRL_COUNT + ctrl_type].max = field.maxval();
				if (field.sensitivity() != 0)
					control_info[field.player() * CTRL_COUNT + ctrl_type].sensitivity = field.sensitivity();
				if (field.delta() != 0)
					control_info[field.player() * CTRL_COUNT + ctrl_type].keydelta = field.delta();
				if (field.analog_reverse() != 0)
					control_info[field.player() * CTRL_COUNT + ctrl_type].reverse = true;
			}
		}
	}

	// Clean-up those entries, if any, where buttons were defined in a separate port than the actual controller they belong to.
	// This is quite often the case, especially for arcades where controls can be easily mapped to separate input ports on PCB.
	// If such situation would only happen for joystick, it would be possible to work it around by initializing differently
	// ctrl_type above, but it is quite common among analog inputs as well (for instance, this is the tipical situation
	// for lightguns) and therefore we really need this separate loop.
	for (int i = 0; i < CTRL_PCOUNT; i++)
	{
		bool fix_done = false;
		for (int j = 1; j < CTRL_COUNT; j++)
			if (control_info[i * CTRL_COUNT].type != nullptr && control_info[i * CTRL_COUNT + j].type != nullptr && !fix_done)
			{
				control_info[i * CTRL_COUNT + j].nbuttons += control_info[i * CTRL_COUNT].nbuttons;
				control_info[i * CTRL_COUNT + j].reqbuttons += control_info[i * CTRL_COUNT].reqbuttons;
				control_info[i * CTRL_COUNT + j].maxbuttons = std::max(control_info[i * CTRL_COUNT + j].maxbuttons, control_info[i * CTRL_COUNT].maxbuttons);

				memset(&control_info[i * CTRL_COUNT], 0, sizeof(control_info[0]));
				fix_done = true;
			}
	}

	// Output the input info
	// First basic info
	infoprotobuf::input *input = machine->mutable_input();
	input->set_players(nplayer);
	input->set_coins(ncoin);
	input->set_service(service);
	input->set_tilt(tilt);

	// Then controller specific ones
	for (auto & elem : control_info)
		if (elem.type != nullptr)
		{
			infoprotobuf::input::Control *control = input->add_control();
			//printf("type %s - player %d - buttons %d\n", elem.type, elem.player, elem.nbuttons);
			if (elem.analog)
			{
				control->set_type(elem.type);
				if (nplayer > 1)
					control->set_player(elem.player);
				if (elem.nbuttons > 0)
				{
					control->set_buttons(strcmp(elem.type, "stick") ? elem.nbuttons : elem.maxbuttons);
					if (elem.reqbuttons < elem.nbuttons)
						control->set_reqbuttons(elem.reqbuttons);
				}
				if (elem.min != 0 || elem.max != 0) {
					control->set_maximum(elem.max);
					control->set_minimum(elem.min);
				}
				if (elem.sensitivity != 0)
					control->set_sensitivity(elem.sensitivity);
				if (elem.keydelta != 0)
					control->set_keydelta(elem.keydelta);
				if (elem.reverse)
					control->set_reverse(true);

			}
			else
			{
				if (elem.helper[1] == 0 && elem.helper[2] != 0) { elem.helper[1] = elem.helper[2]; elem.helper[2] = 0; }
				if (elem.helper[0] == 0 && elem.helper[1] != 0) { elem.helper[0] = elem.helper[1]; elem.helper[1] = 0; }
				if (elem.helper[1] == 0 && elem.helper[2] != 0) { elem.helper[1] = elem.helper[2]; elem.helper[2] = 0; }
				const char *joys = (elem.helper[2] != 0) ? "triple" : (elem.helper[1] != 0) ? "double" : "";
				control->set_type(util::string_format("%s%s", joys, elem.type));
				if (nplayer > 1)
					control->set_player(elem.player);
				if (elem.nbuttons > 0)
				{
					control->set_buttons(strcmp(elem.type, "joy") ? elem.nbuttons : elem.maxbuttons);
					if (elem.reqbuttons < elem.nbuttons)
						control->set_reqbuttons(elem.reqbuttons);
				}
				for (int lp = 0; lp < 3 && elem.helper[lp] != 0; lp++)
				{
					const char *ways;
					std::string helper;
					switch (elem.helper[lp] & (DIR_UP | DIR_DOWN | DIR_LEFT | DIR_RIGHT))
					{
						case DIR_UP | DIR_DOWN | DIR_LEFT | DIR_RIGHT:
							helper = string_format("%d", (elem.ways == 0) ? 8 : elem.ways);
							ways = helper.c_str();
							break;
						case DIR_LEFT | DIR_RIGHT:
							ways = "2";
							break;
						case DIR_UP | DIR_DOWN:
							ways = "vertical2";
							break;
						case DIR_UP:
						case DIR_DOWN:
						case DIR_LEFT:
						case DIR_RIGHT:
							ways = "1";
							break;
						case DIR_UP | DIR_DOWN | DIR_LEFT:
						case DIR_UP | DIR_DOWN | DIR_RIGHT:
						case DIR_UP | DIR_LEFT | DIR_RIGHT:
						case DIR_DOWN | DIR_LEFT | DIR_RIGHT:
							ways = (elem.ways == 4) ? "3 (half4)" : "5 (half8)";
							break;
						default:
							ways = "strange2";
							break;
					}
					if (lp == 0)
						control->set_ways(ways);
					if (lp == 1)
						control->set_ways2(ways);
					if (lp == 2)
						control->set_ways3(ways);
				}
			}
		}
}


//-------------------------------------------------
//  output_switches - print the configurations or
//  DIP switch settings
//-------------------------------------------------

void output_switches(infoprotobuf::machine *machine, const ioport_list &portlist, const char *root_tag, int type, const char *outertag, const char *loctag, const char *innertag)
{
	// iterate looking for DIP switches
	for (auto &port : portlist)
		for (ioport_field const &field : port.second->fields())
			if (field.type() == type)
			{
				std::string newtag(port.second->tag()), oldtag(":");
				newtag = newtag.substr(newtag.find(oldtag.append(root_tag)) + oldtag.length());

				infoprotobuf::dipswitch* dipswitch = machine->add_dipswitch();
				dipswitch->set_name(field.name());
				dipswitch->set_tag(newtag);
				dipswitch->set_mask(field.mask());

				if (!field.condition().none())
					output_ioport_condition(dipswitch->mutable_condition(), field.condition());

				// loop over locations
				for (ioport_diplocation const &diploc : field.diplocations())
				{
					infoprotobuf::dipswitch::DipLocation *diplocation = dipswitch->add_diplocation();
					diplocation->set_name(diploc.name());
					diplocation->set_number(diploc.number());
					if (diploc.inverted())
						diplocation->set_inverted(true);
				}
				// loop over settings
				for (ioport_setting const &setting : field.settings())
				{
					infoprotobuf::dipswitch::DipValue* dipvalue = dipswitch->add_dipvalue();
					dipvalue->set_name(setting.name());
					dipvalue->set_value(setting.value());
					if (setting.value() == field.defvalue())
						dipvalue->set_default_(true);
					if (!setting.condition().none())
						output_ioport_condition(dipswitch->mutable_condition(), setting.condition());
				}
			}
}

//-------------------------------------------------
//  output_ports - print the structure of input
//  ports in the driver
//-------------------------------------------------
void output_ports(infoprotobuf::machine *machine, const ioport_list &portlist)
{
	// cycle through ports
	for (auto &port : portlist)
	{
		infoprotobuf::port *portpb = machine->add_port();
		portpb->set_tag(port.second->tag());
		for (ioport_field const &field : port.second->fields())
		{
			if (field.is_analog())
				portpb->add_analog()->set_mask(field.mask());
		}
	}

}

//-------------------------------------------------
//  output_adjusters - print the Analog
//  Adjusters for a game
//-------------------------------------------------

void output_adjusters(infoprotobuf::machine *machine, const ioport_list &portlist)
{
	// iterate looking for Adjusters
	for (auto &port : portlist)
		for (ioport_field const &field : port.second->fields())
			if (field.type() == IPT_ADJUSTER)
			{
				infoprotobuf::adjuster* adjuster = machine->add_adjuster();
				adjuster->set_name(field.name());
				adjuster->set_default_(field.defvalue());
			}
}


//-------------------------------------------------
//  output_driver - print driver status
//-------------------------------------------------

void output_driver(infoprotobuf::machine *machine, game_driver const &driver, device_t::feature_type unemulated, device_t::feature_type imperfect)
{
	infoprotobuf::driver* driverpb = machine->add_driver();

	/*
	The status entry is an hint for frontend authors to select working
	and not working games without the need to know all the other status
	entries.  Games marked as status=good are perfectly emulated, games
	marked as status=imperfect are emulated with only some minor issues,
	games marked as status=preliminary don't work or have major
	emulation problems.
	*/

	u32 const flags = driver.flags;
	bool const machine_preliminary(flags & (machine_flags::NOT_WORKING | machine_flags::MECHANICAL));
	bool const unemulated_preliminary(unemulated & (device_t::feature::PALETTE | device_t::feature::GRAPHICS | device_t::feature::SOUND | device_t::feature::KEYBOARD));
	bool const imperfect_preliminary((unemulated | imperfect) & device_t::feature::PROTECTION);

	if (machine_preliminary || unemulated_preliminary || imperfect_preliminary)
		driverpb->set_status(infoprotobuf::driver_driver_status::driver_driver_status_preliminary);
	else if (imperfect)
		driverpb->set_status(infoprotobuf::driver_driver_status::driver_driver_status_imperfect);
	else
		driverpb->set_status(infoprotobuf::driver_driver_status::driver_driver_status_good);

	if (flags & machine_flags::NOT_WORKING)
		driverpb->set_emulation(infoprotobuf::driver_driver_status::driver_driver_status_preliminary);
	else
		driverpb->set_emulation(infoprotobuf::driver_driver_status::driver_driver_status_good);

	if (flags & machine_flags::NO_COCKTAIL)
		driverpb->set_cocktail(infoprotobuf::driver_driver_status::driver_driver_status_preliminary);

	if (flags & machine_flags::SUPPORTS_SAVE)
		driverpb->set_savestate(infoprotobuf::driver_Supported::driver_Supported_supported);
	else
		driverpb->set_savestate(infoprotobuf::driver_Supported::driver_Supported_unsupported);

	if (flags & machine_flags::REQUIRES_ARTWORK)
		driverpb->set_requireartwork(true);

	if (flags & machine_flags::UNOFFICIAL)
		driverpb->set_unofficial(true);

	if (flags & machine_flags::NO_SOUND_HW)
		driverpb->set_nosoundhardware(true);

	if (flags & machine_flags::IS_INCOMPLETE)
		driverpb->set_incomplete(true);
}


//-------------------------------------------------
//  output_features - print emulation features of
//
//-------------------------------------------------

void output_features(infoprotobuf::machine *machine, device_type type, device_t::feature_type unemulated, device_t::feature_type imperfect)
{
	device_t::feature_type const flags(type.unemulated_features() | type.imperfect_features() | unemulated | imperfect);
	for (auto const &feature : f_feature_names)
	{
		if (flags & feature.first)
		{
			infoprotobuf::feature* featurepb = machine->add_feature();
			featurepb->set_type(feature.second);
			if (type.unemulated_features() & feature.first)
			{
				featurepb->set_status(infoprotobuf::feature_feature_status::feature_feature_status_unemulated);
			}
			else
			{
				if (type.imperfect_features() & feature.first)
					featurepb->set_status(infoprotobuf::feature_feature_status::feature_feature_status_imperfect);
				if (unemulated & feature.first)
					featurepb->set_status(infoprotobuf::feature_feature_status::feature_feature_status_unemulated);
				else if ((~type.imperfect_features() & imperfect) & feature.first)
					featurepb->set_overall(infoprotobuf::feature_feature_status::feature_feature_status_imperfect);
			}
		}
	}
}


//-------------------------------------------------
//  output_images - prints m_output all info on
//  image devices
//-------------------------------------------------

void output_images(infoprotobuf::machine *machine, device_t &device, const char *root_tag)
{
	for (const device_image_interface &imagedev : image_interface_enumerator(device))
	{
		if (strcmp(imagedev.device().tag(), device.tag()))
		{
			bool loadable = imagedev.user_loadable();
			std::string newtag(imagedev.device().tag()), oldtag(":");
			newtag = newtag.substr(newtag.find(oldtag.append(root_tag)) + oldtag.length());

			infoprotobuf::device *device = machine->add_device();
			// print m_output device type
			device->set_type(imagedev.image_type_name());

			// does this device have a tag?
			if (imagedev.device().tag())
				device->set_tag(newtag);

			// is this device available as media switch?
			if (!loadable)
				device->set_fixed_image("1");

			// is this device mandatory?
			if (imagedev.must_be_loaded())
				device->set_mandatory("1");

			if (imagedev.image_interface() && imagedev.image_interface()[0])
				device->set_interface(imagedev.image_interface());

			if (loadable)
			{
				char const *const name = imagedev.instance_name().c_str();
				char const *const shortname = imagedev.brief_instance_name().c_str();

				// create device instance
				infoprotobuf::device::DeviceInstance* instance = device->mutable_device_instance();
				instance->set_name(name);
				instance->set_briefname(shortname);

				char const *extensions(imagedev.file_extensions());
				while (extensions)
				{
					char const *end(extensions);
					while (*end && (',' != *end))
						++end;

					device->add_extension()->set_name(std::string(extensions, end));
					extensions = *end ? (end + 1) : nullptr;
				}
			}
		}
	}
}


//-------------------------------------------------
//  output_slots - prints all info about slots
//-------------------------------------------------

void output_slots(infoprotobuf::machine *machine, machine_config &config, device_t &device, const char *root_tag, device_type_set *devtypes)
{
	for (device_slot_interface &slot : slot_interface_enumerator(device))
	{
		// shall we list fixed slots as non-configurable?
		bool const listed(!slot.fixed() && strcmp(slot.device().tag(), device.tag()));

		if (devtypes || listed)
		{
			machine_config::token const tok(config.begin_configuration(slot.device()));
			std::string newtag(slot.device().tag()), oldtag(":");
			newtag = newtag.substr(newtag.find(oldtag.append(root_tag)) + oldtag.length());

			infoprotobuf::slot *slotpb = machine->add_slot();
			// print m_output device type
			if (listed)
				slotpb->set_name(newtag);

			for (auto &option : slot.option_list())
			{
				if (devtypes || (listed && option.second->selectable()))
				{
					infoprotobuf::slot::SlotOption *slotOption = slotpb->add_slotoption();
					device_t *const dev = config.device_add("_dummy", option.second->devtype(), option.second->clock());
					if (!dev->configured())
						dev->config_complete();

					if (devtypes)
						for (device_t &subdevice : device_enumerator(*dev)) devtypes->insert(&subdevice.type());

					if (listed && option.second->selectable())
					{
						slotOption->set_name(option.second->name());
						slotOption->set_devname(dev->shortname());
						if (slot.default_option() != nullptr && strcmp(slot.default_option(), option.second->name()) == 0)
							slotOption->set_default_(true);
					}

					config.device_remove("_dummy");
				}
			}
		}
	}
}


//-------------------------------------------------
//  output_software_lists - print the information
//  for all known software lists for this system
//-------------------------------------------------

void output_software_lists(infoprotobuf::machine *machine, device_t &root, const char *root_tag)
{
	for (const software_list_device &swlist : software_list_device_enumerator(root))
	{
		if (&static_cast<const device_t &>(swlist) == &root)
		{
			assert(swlist.list_name().empty());
			continue;
		}

		infoprotobuf::softwarelist* softwarelist = machine->add_softwarelist();
		std::string newtag(swlist.tag()), oldtag(":");
		newtag = newtag.substr(newtag.find(oldtag.append(root_tag)) + oldtag.length());
		softwarelist->set_tag(newtag);
		softwarelist->set_name(swlist.list_name());
		softwarelist->set_status(swlist.is_original()
			? infoprotobuf::softwarelist_Status::softwarelist_Status_original
			: infoprotobuf::softwarelist_Status::softwarelist_Status_compatibile);

		if (swlist.filter())
			softwarelist->set_filter(swlist.filter());
	}
}



//-------------------------------------------------
//  output_ramoptions - prints m_output all RAM
//  options for this system
//-------------------------------------------------

void output_ramoptions(infoprotobuf::machine *machine, device_t &root)
{
	for (const ram_device &ram : ram_device_enumerator(root, 1))
	{
		if (!std::strcmp(ram.tag(), ":" RAM_TAG))
		{
			uint32_t const defsize(ram.default_size());
			bool havedefault(false);
			for (ram_device::extra_option const &option : ram.extra_options())
			{
				infoprotobuf::ramoption* ramoption = machine->add_ramoption();
				if (defsize == option.second)
				{
					assert(!havedefault);
					havedefault = true;
					ramoption->set_default_(true);
				}

				ramoption->set_name(option.first);
				ramoption->set_value(option.second);
			}
			if (!havedefault) {
				infoprotobuf::ramoption* ramoption = machine->add_ramoption();
				ramoption->set_name(ram.default_size_string());
				ramoption->set_value(defsize);
				ramoption->set_default_(true);
			}
			break;
		}
	}
}


//-------------------------------------------------
//  get_merge_name - get the rom name from a
//  parent set
//-------------------------------------------------

const char *get_merge_name(driver_enumerator &drivlist, const game_driver &driver, util::hash_collection const &romhashes)
{
	// walk the parent chain
	for (int clone_of = drivlist.find(driver.parent); 0 <= clone_of; clone_of = drivlist.find(drivlist.driver(clone_of).parent))
	{
		// look in the parent's ROMs
		for (romload::region const &pregion : romload::entries(drivlist.driver(clone_of).rom).get_regions())
		{
			for (romload::file const &prom : pregion.get_files())
			{
				// stop when we find a match
				util::hash_collection const phashes(prom.get_hashdata());
				if (!phashes.flag(util::hash_collection::FLAG_NO_DUMP) && (romhashes == phashes))
					return prom.get_name();
			}
		}
	}

	return nullptr;
}


//-------------------------------------------------
//  device_type_compare::operator()
//-------------------------------------------------

bool device_type_compare::operator()(const std::add_pointer_t<device_type> &lhs, const std::add_pointer_t<device_type> &rhs) const
{
	return strcmp(lhs->shortname(), rhs->shortname()) < 0;
}

}
