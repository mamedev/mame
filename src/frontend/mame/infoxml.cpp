// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Paul Priest
/***************************************************************************

    info.cpp

    Dumps the MAME internal data as an XML file.

***************************************************************************/

#include "emu.h"
#include "infoxml.h"

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
#include "xmlfile.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <future>
#include <queue>
#include <type_traits>
#include <unordered_set>
#include <utility>


#define XML_ROOT    "mame"
#define XML_TOP     "machine"


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

std::string normalize_string(const char *string);

// internal helper
void output_header(std::ostream &out, bool dtd);
void output_footer(std::ostream &out);

void output_one(std::ostream &out, driver_enumerator &drivlist, const game_driver &driver, device_type_set *devtypes);
void output_sampleof(std::ostream &out, device_t &device);
void output_bios(std::ostream &out, device_t const &device);
void output_rom(std::ostream &out, driver_enumerator *drivlist, const game_driver *driver, device_t &device);
void output_device_refs(std::ostream &out, device_t &root);
void output_sample(std::ostream &out, device_t &device);
void output_chips(std::ostream &out, device_t &device, const char *root_tag);
void output_display(std::ostream &out, device_t &device, machine_flags::type const *flags, const char *root_tag);
void output_sound(std::ostream &out, device_t &device);
void output_ioport_condition(std::ostream &out, const ioport_condition &condition, unsigned indent);
void output_input(std::ostream &out, const ioport_list &portlist);
void output_switches(std::ostream &out, const ioport_list &portlist, const char *root_tag, int type, const char *outertag, const char *loctag, const char *innertag);
void output_ports(std::ostream &out, const ioport_list &portlist);
void output_adjusters(std::ostream &out, const ioport_list &portlist);
void output_driver(std::ostream &out, game_driver const &driver, device_t::feature_type unemulated, device_t::feature_type imperfect);
void output_features(std::ostream &out, device_type type, device_t::feature_type unemulated, device_t::feature_type imperfect);
void output_images(std::ostream &out, device_t &device, const char *root_tag);
void output_slots(std::ostream &out, machine_config &config, device_t &device, const char *root_tag, device_type_set *devtypes);
void output_software_lists(std::ostream &out, device_t &root, const char *root_tag);
void output_ramoptions(std::ostream &out, device_t &root);

void output_one_device(std::ostream &out, machine_config &config, device_t &device, const char *devtag);
void output_devices(std::ostream &out, emu_options &lookup_options, device_type_set const *filter);

const char *get_merge_name(driver_enumerator &drivlist, const game_driver &driver, util::hash_collection const &romhashes);


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// DTD string describing the data
constexpr char f_dtd_string[] =
		"<!DOCTYPE __XML_ROOT__ [\n"
		"<!ELEMENT __XML_ROOT__ (__XML_TOP__+)>\n"
		"\t<!ATTLIST __XML_ROOT__ build CDATA #IMPLIED>\n"
		"\t<!ATTLIST __XML_ROOT__ debug (yes|no) \"no\">\n"
		"\t<!ATTLIST __XML_ROOT__ mameconfig CDATA #REQUIRED>\n"
		"\t<!ELEMENT __XML_TOP__ (description, year?, manufacturer?, biosset*, rom*, disk*, device_ref*, sample*, chip*, display*, sound?, input?, dipswitch*, configuration*, port*, adjuster*, driver?, feature*, device*, slot*, softwarelist*, ramoption*)>\n"
		"\t\t<!ATTLIST __XML_TOP__ name CDATA #REQUIRED>\n"
		"\t\t<!ATTLIST __XML_TOP__ sourcefile CDATA #IMPLIED>\n"
		"\t\t<!ATTLIST __XML_TOP__ isbios (yes|no) \"no\">\n"
		"\t\t<!ATTLIST __XML_TOP__ isdevice (yes|no) \"no\">\n"
		"\t\t<!ATTLIST __XML_TOP__ ismechanical (yes|no) \"no\">\n"
		"\t\t<!ATTLIST __XML_TOP__ runnable (yes|no) \"yes\">\n"
		"\t\t<!ATTLIST __XML_TOP__ cloneof CDATA #IMPLIED>\n"
		"\t\t<!ATTLIST __XML_TOP__ romof CDATA #IMPLIED>\n"
		"\t\t<!ATTLIST __XML_TOP__ sampleof CDATA #IMPLIED>\n"
		"\t\t<!ELEMENT description (#PCDATA)>\n"
		"\t\t<!ELEMENT year (#PCDATA)>\n"
		"\t\t<!ELEMENT manufacturer (#PCDATA)>\n"
		"\t\t<!ELEMENT biosset EMPTY>\n"
		"\t\t\t<!ATTLIST biosset name CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST biosset description CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST biosset default (yes|no) \"no\">\n"
		"\t\t<!ELEMENT rom EMPTY>\n"
		"\t\t\t<!ATTLIST rom name CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST rom bios CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST rom size CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST rom crc CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST rom sha1 CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST rom merge CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST rom region CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST rom offset CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST rom status (baddump|nodump|good) \"good\">\n"
		"\t\t\t<!ATTLIST rom optional (yes|no) \"no\">\n"
		"\t\t<!ELEMENT disk EMPTY>\n"
		"\t\t\t<!ATTLIST disk name CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST disk sha1 CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST disk merge CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST disk region CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST disk index CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST disk writable (yes|no) \"no\">\n"
		"\t\t\t<!ATTLIST disk status (baddump|nodump|good) \"good\">\n"
		"\t\t\t<!ATTLIST disk optional (yes|no) \"no\">\n"
		"\t\t<!ELEMENT device_ref EMPTY>\n"
		"\t\t\t<!ATTLIST device_ref name CDATA #REQUIRED>\n"
		"\t\t<!ELEMENT sample EMPTY>\n"
		"\t\t\t<!ATTLIST sample name CDATA #REQUIRED>\n"
		"\t\t<!ELEMENT chip EMPTY>\n"
		"\t\t\t<!ATTLIST chip name CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST chip tag CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST chip type (cpu|audio) #REQUIRED>\n"
		"\t\t\t<!ATTLIST chip clock CDATA #IMPLIED>\n"
		"\t\t<!ELEMENT display EMPTY>\n"
		"\t\t\t<!ATTLIST display tag CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST display type (raster|vector|lcd|svg|unknown) #REQUIRED>\n"
		"\t\t\t<!ATTLIST display rotate (0|90|180|270) #IMPLIED>\n"
		"\t\t\t<!ATTLIST display flipx (yes|no) \"no\">\n"
		"\t\t\t<!ATTLIST display width CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST display height CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST display refresh CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST display pixclock CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST display htotal CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST display hbend CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST display hbstart CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST display vtotal CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST display vbend CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST display vbstart CDATA #IMPLIED>\n"
		"\t\t<!ELEMENT sound EMPTY>\n"
		"\t\t\t<!ATTLIST sound channels CDATA #REQUIRED>\n"
		"\t\t<!ELEMENT condition EMPTY>\n"
		"\t\t\t<!ATTLIST condition tag CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST condition mask CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST condition relation (eq|ne|gt|le|lt|ge) #REQUIRED>\n"
		"\t\t\t<!ATTLIST condition value CDATA #REQUIRED>\n"
		"\t\t<!ELEMENT input (control*)>\n"
		"\t\t\t<!ATTLIST input service (yes|no) \"no\">\n"
		"\t\t\t<!ATTLIST input tilt (yes|no) \"no\">\n"
		"\t\t\t<!ATTLIST input players CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST input coins CDATA #IMPLIED>\n"
		"\t\t\t<!ELEMENT control EMPTY>\n"
		"\t\t\t\t<!ATTLIST control type CDATA #REQUIRED>\n"
		"\t\t\t\t<!ATTLIST control player CDATA #IMPLIED>\n"
		"\t\t\t\t<!ATTLIST control buttons CDATA #IMPLIED>\n"
		"\t\t\t\t<!ATTLIST control reqbuttons CDATA #IMPLIED>\n"
		"\t\t\t\t<!ATTLIST control minimum CDATA #IMPLIED>\n"
		"\t\t\t\t<!ATTLIST control maximum CDATA #IMPLIED>\n"
		"\t\t\t\t<!ATTLIST control sensitivity CDATA #IMPLIED>\n"
		"\t\t\t\t<!ATTLIST control keydelta CDATA #IMPLIED>\n"
		"\t\t\t\t<!ATTLIST control reverse (yes|no) \"no\">\n"
		"\t\t\t\t<!ATTLIST control ways CDATA #IMPLIED>\n"
		"\t\t\t\t<!ATTLIST control ways2 CDATA #IMPLIED>\n"
		"\t\t\t\t<!ATTLIST control ways3 CDATA #IMPLIED>\n"
		"\t\t<!ELEMENT dipswitch (condition?, diplocation*, dipvalue*)>\n"
		"\t\t\t<!ATTLIST dipswitch name CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST dipswitch tag CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST dipswitch mask CDATA #REQUIRED>\n"
		"\t\t\t<!ELEMENT diplocation EMPTY>\n"
		"\t\t\t\t<!ATTLIST diplocation name CDATA #REQUIRED>\n"
		"\t\t\t\t<!ATTLIST diplocation number CDATA #REQUIRED>\n"
		"\t\t\t\t<!ATTLIST diplocation inverted (yes|no) \"no\">\n"
		"\t\t\t<!ELEMENT dipvalue (condition?)>\n"
		"\t\t\t\t<!ATTLIST dipvalue name CDATA #REQUIRED>\n"
		"\t\t\t\t<!ATTLIST dipvalue value CDATA #REQUIRED>\n"
		"\t\t\t\t<!ATTLIST dipvalue default (yes|no) \"no\">\n"
		"\t\t<!ELEMENT configuration (condition?, conflocation*, confsetting*)>\n"
		"\t\t\t<!ATTLIST configuration name CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST configuration tag CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST configuration mask CDATA #REQUIRED>\n"
		"\t\t\t<!ELEMENT conflocation EMPTY>\n"
		"\t\t\t\t<!ATTLIST conflocation name CDATA #REQUIRED>\n"
		"\t\t\t\t<!ATTLIST conflocation number CDATA #REQUIRED>\n"
		"\t\t\t\t<!ATTLIST conflocation inverted (yes|no) \"no\">\n"
		"\t\t\t<!ELEMENT confsetting (condition?)>\n"
		"\t\t\t\t<!ATTLIST confsetting name CDATA #REQUIRED>\n"
		"\t\t\t\t<!ATTLIST confsetting value CDATA #REQUIRED>\n"
		"\t\t\t\t<!ATTLIST confsetting default (yes|no) \"no\">\n"
		"\t\t<!ELEMENT port (analog*)>\n"
		"\t\t\t<!ATTLIST port tag CDATA #REQUIRED>\n"
		"\t\t\t<!ELEMENT analog EMPTY>\n"
		"\t\t\t\t<!ATTLIST analog mask CDATA #REQUIRED>\n"
		"\t\t<!ELEMENT adjuster (condition?)>\n"
		"\t\t\t<!ATTLIST adjuster name CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST adjuster default CDATA #REQUIRED>\n"
		"\t\t<!ELEMENT driver EMPTY>\n"
		"\t\t\t<!ATTLIST driver status (good|imperfect|preliminary) #REQUIRED>\n"
		"\t\t\t<!ATTLIST driver emulation (good|imperfect|preliminary) #REQUIRED>\n"
		"\t\t\t<!ATTLIST driver cocktail (good|imperfect|preliminary) #IMPLIED>\n"
		"\t\t\t<!ATTLIST driver savestate (supported|unsupported) #REQUIRED>\n"
		"\t\t\t<!ATTLIST driver requiresartwork (yes|no) \"no\">\n"
		"\t\t\t<!ATTLIST driver unofficial (yes|no) \"no\">\n"
		"\t\t\t<!ATTLIST driver nosoundhardware (yes|no) \"no\">\n"
		"\t\t\t<!ATTLIST driver incomplete (yes|no) \"no\">\n"
		"\t\t<!ELEMENT feature EMPTY>\n"
		"\t\t\t<!ATTLIST feature type (protection|timing|graphics|palette|sound|capture|camera|microphone|controls|keyboard|mouse|media|disk|printer|tape|punch|drum|rom|comms|lan|wan) #REQUIRED>\n"
		"\t\t\t<!ATTLIST feature status (unemulated|imperfect) #IMPLIED>\n"
		"\t\t\t<!ATTLIST feature overall (unemulated|imperfect) #IMPLIED>\n"
		"\t\t<!ELEMENT device (instance?, extension*)>\n"
		"\t\t\t<!ATTLIST device type CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST device tag CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST device fixed_image CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST device mandatory CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST device interface CDATA #IMPLIED>\n"
		"\t\t\t<!ELEMENT instance EMPTY>\n"
		"\t\t\t\t<!ATTLIST instance name CDATA #REQUIRED>\n"
		"\t\t\t\t<!ATTLIST instance briefname CDATA #REQUIRED>\n"
		"\t\t\t<!ELEMENT extension EMPTY>\n"
		"\t\t\t\t<!ATTLIST extension name CDATA #REQUIRED>\n"
		"\t\t<!ELEMENT slot (slotoption*)>\n"
		"\t\t\t<!ATTLIST slot name CDATA #REQUIRED>\n"
		"\t\t\t<!ELEMENT slotoption EMPTY>\n"
		"\t\t\t\t<!ATTLIST slotoption name CDATA #REQUIRED>\n"
		"\t\t\t\t<!ATTLIST slotoption devname CDATA #REQUIRED>\n"
		"\t\t\t\t<!ATTLIST slotoption default (yes|no) \"no\">\n"
		"\t\t<!ELEMENT softwarelist EMPTY>\n"
		"\t\t\t<!ATTLIST softwarelist tag CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST softwarelist name CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST softwarelist status (original|compatible) #REQUIRED>\n"
		"\t\t\t<!ATTLIST softwarelist filter CDATA #IMPLIED>\n"
		"\t\t<!ELEMENT ramoption (#PCDATA)>\n"
		"\t\t\t<!ATTLIST ramoption name CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST ramoption default CDATA #IMPLIED>\n"
		"]>";


// XML feature names
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
//  INFO XML CREATOR
//**************************************************************************


//-------------------------------------------------
//  get_feature_name - get XML name for feature
//-------------------------------------------------

char const *info_xml_creator::feature_name(device_t::feature_type feature)
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
//  info_xml_creator - constructor
//-------------------------------------------------

info_xml_creator::info_xml_creator(emu_options const &options, bool dtd)
	: m_dtd(dtd)
{
}


//-------------------------------------------------
//  output - print the XML information for all
//  known machines matching a pattern
//-------------------------------------------------

void info_xml_creator::output(std::ostream &out, const std::vector<std::string> &patterns)
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
//  output - print the XML information for all
//  known (and filtered) machines
//-------------------------------------------------

void info_xml_creator::output(std::ostream &out, const std::function<bool(const char *shortname, bool &done)> &filter, bool include_devices)
{
	struct prepared_info
	{
		std::string     m_xml_snippet;
		device_type_set m_dev_set;
	};

	// prepare a driver enumerator and the queue
	driver_enumerator drivlist(m_lookup_options);
	bool drivlist_done = false;
	bool filter_done = false;
	bool header_outputted = false;

	auto output_header_if_necessary = [this, &header_outputted](std::ostream &out)
	{
		if (!header_outputted)
		{
			output_header(out, m_dtd);
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
				std::future<prepared_info> future_pi = std::async(std::launch::async, [&drivlist, &driver, &devfilter]
				{
					prepared_info result;
					std::ostringstream stream;

					output_one(stream, drivlist, driver, devfilter ? &result.m_dev_set : nullptr);
					result.m_xml_snippet = stream.str();
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

			// emit the XML
			output_header_if_necessary(out);
			out << pi.m_xml_snippet;

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
		output_header_if_necessary(out);
		output_devices(out, m_lookup_options, devfilter.get());
	}

	if (header_outputted)
		output_footer(out);
}


//**************************************************************************
//  ANONYMOUS NAMESPACE IMPLEMENTATION
//**************************************************************************

namespace
{

//-------------------------------------------------
//  normalize_string
//-------------------------------------------------

std::string normalize_string(const char *string)
{
	std::ostringstream stream;

	if (string != nullptr)
	{
		while (*string)
		{
			switch (*string)
			{
			case '\"': stream << "&quot;"; break;
			case '&': stream << "&amp;"; break;
			case '<': stream << "&lt;"; break;
			case '>': stream << "&gt;"; break;
			default:
				stream << *string;
				break;
			}
			++string;
		}
	}
	return stream.str();
}


//-------------------------------------------------
//  output_header - print the XML DTD and open
//  the root element
//-------------------------------------------------

void output_header(std::ostream &out, bool dtd)
{
	if (dtd)
	{
		// output the DTD
		out << "<?xml version=\"1.0\"?>\n";
		std::string dtd(f_dtd_string);
		strreplace(dtd, "__XML_ROOT__", XML_ROOT);
		strreplace(dtd, "__XML_TOP__", XML_TOP);

		out << dtd << "\n\n";
	}

	// top-level tag
	out << util::string_format("<%s build=\"%s\" debug=\""
#ifdef MAME_DEBUG
			"yes"
#else
			"no"
#endif
			"\" mameconfig=\"%d\">\n",
			XML_ROOT,
			normalize_string(emulator_info::get_build_version()),
			configuration_manager::CONFIG_VERSION);
}


//-------------------------------------------------
//  output_header - close the root element
//-------------------------------------------------

void output_footer(std::ostream &out)
{
	// close the top level tag
	out << util::string_format("</%s>\n", XML_ROOT);
}


//-------------------------------------------------
//  output_one - print the XML information
//  for one particular machine driver
//-------------------------------------------------

void output_one(std::ostream &out, driver_enumerator &drivlist, const game_driver &driver, device_type_set *devtypes)
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
	out << util::string_format("\t<%s name=\"%s\"", XML_TOP, normalize_string(driver.name));

	// strip away any path information from the source_file and output it
	const char *start = strrchr(driver.type.source(), '/');
	if (!start)
		start = strrchr(driver.type.source(), '\\');
	start = start ? (start + 1) : driver.type.source();
	out << util::string_format(" sourcefile=\"%s\"", normalize_string(start));

	// append bios and runnable flags
	if (driver.flags & machine_flags::IS_BIOS_ROOT)
		out << " isbios=\"yes\"";
	if (driver.flags & machine_flags::MECHANICAL)
		out << " ismechanical=\"yes\"";

	// display clone information
	int clone_of = drivlist.find(driver.parent);
	if (clone_of != -1 && !(drivlist.driver(clone_of).flags & machine_flags::IS_BIOS_ROOT))
		out << util::string_format(" cloneof=\"%s\"", normalize_string(drivlist.driver(clone_of).name));
	if (clone_of != -1)
		out << util::string_format(" romof=\"%s\"", normalize_string(drivlist.driver(clone_of).name));

	// display sample information and close the game tag
	output_sampleof(out, config.root_device());
	out << ">\n";

	// output game description
	if (driver.type.fullname() != nullptr)
		out << util::string_format("\t\t<description>%s</description>\n", normalize_string(driver.type.fullname()));

	// print the year only if is a number or another allowed character (? or +)
	if (driver.year != nullptr && strspn(driver.year, "0123456789?+") == strlen(driver.year))
		out << util::string_format("\t\t<year>%s</year>\n", normalize_string(driver.year));

	// print the manufacturer information
	if (driver.manufacturer != nullptr)
		out << util::string_format("\t\t<manufacturer>%s</manufacturer>\n", normalize_string(driver.manufacturer));

	// now print various additional information
	output_bios(out, config.root_device());
	output_rom(out, &drivlist, &driver, config.root_device());
	output_device_refs(out, config.root_device());
	output_sample(out, config.root_device());
	output_chips(out, config.root_device(), "");
	output_display(out, config.root_device(), &driver.flags, "");
	output_sound(out, config.root_device());
	output_input(out, portlist);
	output_switches(out, portlist, "", IPT_DIPSWITCH, "dipswitch", "diplocation", "dipvalue");
	output_switches(out, portlist, "", IPT_CONFIG, "configuration", "conflocation", "confsetting");
	output_ports(out, portlist);
	output_adjusters(out, portlist);
	output_driver(out, driver, overall_unemulated, overall_imperfect);
	output_features(out, driver.type, overall_unemulated, overall_imperfect);
	output_images(out, config.root_device(), "");
	output_slots(out, config, config.root_device(), "", devtypes);
	output_software_lists(out, config.root_device(), "");
	output_ramoptions(out, config.root_device());

	// close the topmost tag
	out << util::string_format("\t</%s>\n", XML_TOP);
}


//-------------------------------------------------
//  output_one_device - print the XML info for
//  a single device
//-------------------------------------------------

void output_one_device(std::ostream &out, machine_config &config, device_t &device, const char *devtag)
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
	out << util::string_format("\t<%s name=\"%s\"", XML_TOP, normalize_string(device.shortname()));
	std::string src(device.source());
	strreplace(src,"../", "");
	out << util::string_format(" sourcefile=\"%s\" isdevice=\"yes\" runnable=\"no\"", normalize_string(src.c_str()));
	output_sampleof(out, device);
	out << ">\n" << util::string_format("\t\t<description>%s</description>\n", normalize_string(device.name()));

	output_bios(out, device);
	output_rom(out, nullptr, nullptr, device);
	output_device_refs(out, device);

	if (device.type().type() != typeid(samples_device)) // ignore samples_device itself
		output_sample(out, device);

	output_chips(out, device, devtag);
	output_display(out, device, nullptr, devtag);
	if (has_speaker)
		output_sound(out, device);
	if (has_input)
		output_input(out, portlist);
	output_switches(out, portlist, devtag, IPT_DIPSWITCH, "dipswitch", "diplocation", "dipvalue");
	output_switches(out, portlist, devtag, IPT_CONFIG, "configuration", "conflocation", "confsetting");
	output_adjusters(out, portlist);
	output_features(out, device.type(), overall_unemulated, overall_imperfect);
	output_images(out, device, devtag);
	output_slots(out, config, device, devtag, nullptr);
	output_software_lists(out, device, devtag);
	out << util::string_format("\t</%s>\n", XML_TOP);
}


//-------------------------------------------------
//  output_devices - print the XML info for
//  registered device types
//-------------------------------------------------

void output_devices(std::ostream &out, emu_options &lookup_options, device_type_set const *filter)
{
	// get config for empty machine
	machine_config config(GAME_NAME(___empty), lookup_options);

	auto const action = [&config, &out] (device_type type)
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
				output_one_device(out, config, *dev, dev->tag());
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

void output_device_refs(std::ostream &out, device_t &root)
{
	for (device_t &device : device_enumerator(root))
		if (&device != &root)
			out << util::string_format("\t\t<device_ref name=\"%s\"/>\n", normalize_string(device.shortname()));
}


//------------------------------------------------
//  output_sampleof - print the 'sampleof'
//  attribute, if appropriate
//-------------------------------------------------

void output_sampleof(std::ostream &out, device_t &device)
{
	// iterate over sample devices
	for (samples_device &samples : samples_device_enumerator(device))
	{
		samples_iterator sampiter(samples);
		if (sampiter.altbasename() != nullptr)
		{
			out << util::string_format(" sampleof=\"%s\"", normalize_string(sampiter.altbasename()));

			// must stop here, as there can only be one attribute of the same name
			return;
		}
	}
}


//-------------------------------------------------
//  output_bios - print BIOS sets for a device
//-------------------------------------------------

void output_bios(std::ostream &out, device_t const &device)
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
		// output extracted name and descriptions'
		out << "\t\t<biosset";
		out << util::string_format(" name=\"%s\"", normalize_string(bios.get_name()));
		out << util::string_format(" description=\"%s\"", normalize_string(bios.get_description()));
		if (defaultname && !std::strcmp(defaultname, bios.get_name()))
			out << " default=\"yes\"";
		out << "/>\n";
	}
}


//-------------------------------------------------
//  output_rom - print the roms section of
//  the XML output
//-------------------------------------------------

void output_rom(std::ostream &out, driver_enumerator *drivlist, const game_driver *driver, device_t &device)
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

			// opening tag
			if (is_disk)
				out << "\t\t<disk";
			else
				out << "\t\t<rom";

			// add name, merge, bios, and size tags */
			char const *const name(rom->name);
			if (name && name[0])
				out << util::string_format(" name=\"%s\"", normalize_string(name));
			if (merge_name)
				out << util::string_format(" merge=\"%s\"", normalize_string(merge_name));
			if (bios_name)
				out << util::string_format(" bios=\"%s\"", normalize_string(bios_name));
			if (!is_disk)
				out << util::string_format(" size=\"%u\"", rom_file_size(rom));

			// dump checksum information only if there is a known dump
			if (!hashes.flag(util::hash_collection::FLAG_NO_DUMP))
				out << ' ' << hashes.attribute_string(); // iterate over hash function types and print m_output their values
			else
				out << " status=\"nodump\"";

			// append a region name
			out << util::string_format(" region=\"%s\"", region->name);

			if (!is_disk)
			{
				// for non-disk entries, print offset
				out << util::string_format(" offset=\"%x\"", ROM_GETOFFSET(rom));
			}
			else
			{
				// for disk entries, add the disk index
				out << util::string_format(" index=\"%x\" writable=\"%s\"", DISK_GETINDEX(rom), DISK_ISREADONLY(rom) ? "no" : "yes");
			}

			// add optional flag
			if (ROM_ISOPTIONAL(rom))
				out << " optional=\"yes\"";

			out << "/>\n";
		}
		bios_scanned = true;
	}
}


//-------------------------------------------------
//  output_sample - print a list of all
//  samples referenced by a game_driver
//-------------------------------------------------

void output_sample(std::ostream &out, device_t &device)
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
			out << util::string_format("\t\t<sample name=\"%s\"/>\n", normalize_string(samplename));
		}
	}
}


/*-------------------------------------------------
    output_chips - print a list of CPU and
    sound chips used by a game
-------------------------------------------------*/

void output_chips(std::ostream &out, device_t &device, const char *root_tag)
{
	// iterate over executable devices
	for (device_execute_interface &exec : execute_interface_enumerator(device))
	{
		if (strcmp(exec.device().tag(), device.tag()))
		{
			std::string newtag(exec.device().tag()), oldtag(":");
			newtag = newtag.substr(newtag.find(oldtag.append(root_tag)) + oldtag.length());

			out << "\t\t<chip";
			out << " type=\"cpu\"";
			out << util::string_format(" tag=\"%s\"", normalize_string(newtag.c_str()));
			out << util::string_format(" name=\"%s\"", normalize_string(exec.device().name()));
			out << util::string_format(" clock=\"%d\"", exec.device().clock());
			out << "/>\n";
		}
	}

	// iterate over sound devices
	for (device_sound_interface &sound : sound_interface_enumerator(device))
	{
		if (strcmp(sound.device().tag(), device.tag()) != 0 && sound.issound())
		{
			std::string newtag(sound.device().tag()), oldtag(":");
			newtag = newtag.substr(newtag.find(oldtag.append(root_tag)) + oldtag.length());

			out << "\t\t<chip";
			out << " type=\"audio\"";
			out << util::string_format(" tag=\"%s\"", normalize_string(newtag.c_str()));
			out << util::string_format(" name=\"%s\"", normalize_string(sound.device().name()));
			if (sound.device().clock() != 0)
				out << util::string_format(" clock=\"%d\"", sound.device().clock());
			out << "/>\n";
		}
	}
}


//-------------------------------------------------
//  output_display - print a list of all the
//  displays
//-------------------------------------------------

void output_display(std::ostream &out, device_t &device, machine_flags::type const *flags, const char *root_tag)
{
	// iterate over screens
	for (const screen_device &screendev : screen_device_enumerator(device))
	{
		if (strcmp(screendev.tag(), device.tag()))
		{
			std::string newtag(screendev.tag()), oldtag(":");
			newtag = newtag.substr(newtag.find(oldtag.append(root_tag)) + oldtag.length());

			out << util::string_format("\t\t<display tag=\"%s\"", normalize_string(newtag.c_str()));

			switch (screendev.screen_type())
			{
				case SCREEN_TYPE_RASTER:    out << " type=\"raster\"";  break;
				case SCREEN_TYPE_VECTOR:    out << " type=\"vector\"";  break;
				case SCREEN_TYPE_LCD:       out << " type=\"lcd\"";     break;
				case SCREEN_TYPE_SVG:       out << " type=\"svg\"";     break;
				default:                    out << " type=\"unknown\""; break;
			}

			// output the orientation as a string
			switch (screendev.orientation())
			{
			case ORIENTATION_FLIP_X:
				out << " rotate=\"0\" flipx=\"yes\"";
				break;
			case ORIENTATION_FLIP_Y:
				out << " rotate=\"180\" flipx=\"yes\"";
				break;
			case ORIENTATION_FLIP_X|ORIENTATION_FLIP_Y:
				out << " rotate=\"180\"";
				break;
			case ORIENTATION_SWAP_XY:
				out << " rotate=\"90\" flipx=\"yes\"";
				break;
			case ORIENTATION_SWAP_XY|ORIENTATION_FLIP_X:
				out << " rotate=\"90\"";
				break;
			case ORIENTATION_SWAP_XY|ORIENTATION_FLIP_Y:
				out << " rotate=\"270\"";
				break;
			case ORIENTATION_SWAP_XY|ORIENTATION_FLIP_X|ORIENTATION_FLIP_Y:
				out << " rotate=\"270\" flipx=\"yes\"";
				break;
			default:
				out << " rotate=\"0\"";
				break;
			}

			// output width and height only for games that are not vector
			if (screendev.screen_type() != SCREEN_TYPE_VECTOR)
			{
				const rectangle &visarea = screendev.visible_area();
				out << util::string_format(" width=\"%d\"", visarea.width());
				out << util::string_format(" height=\"%d\"", visarea.height());
			}

			// output refresh rate
			out << util::string_format(" refresh=\"%f\"", ATTOSECONDS_TO_HZ(screendev.refresh_attoseconds()));

			// output raw video parameters only for games that are not vector
			// and had raw parameters specified
			if (screendev.screen_type() != SCREEN_TYPE_VECTOR && !screendev.oldstyle_vblank_supplied())
			{
				int pixclock = screendev.width() * screendev.height() * ATTOSECONDS_TO_HZ(screendev.refresh_attoseconds());

				out << util::string_format(" pixclock=\"%d\"", pixclock);
				out << util::string_format(" htotal=\"%d\"", screendev.width());
				out << util::string_format(" hbend=\"%d\"", screendev.visible_area().min_x);
				out << util::string_format(" hbstart=\"%d\"", screendev.visible_area().max_x+1);
				out << util::string_format(" vtotal=\"%d\"", screendev.height());
				out << util::string_format(" vbend=\"%d\"", screendev.visible_area().min_y);
				out << util::string_format(" vbstart=\"%d\"", screendev.visible_area().max_y+1);
			}
			out << " />\n";
		}
	}
}


//-------------------------------------------------
//  output_sound - print a list of all the
//  speakers
//------------------------------------------------

void output_sound(std::ostream &out, device_t &device)
{
	speaker_device_enumerator spkiter(device);
	int speakers = spkiter.count();

	// if we have no sound, zero m_output the speaker count
	sound_interface_enumerator snditer(device);
	if (snditer.first() == nullptr)
		speakers = 0;

	out << util::string_format("\t\t<sound channels=\"%d\"/>\n", speakers);
}


//-------------------------------------------------
//  output_ioport_condition - print condition
//  required to use I/O port field/setting
//-------------------------------------------------

void output_ioport_condition(std::ostream &out, const ioport_condition &condition, unsigned indent)
{
	for (unsigned i = 0; indent > i; ++i)
		out << '\t';

	char const *rel(nullptr);
	switch (condition.condition())
	{
	case ioport_condition::ALWAYS:          throw false;
	case ioport_condition::EQUALS:          rel = "eq"; break;
	case ioport_condition::NOTEQUALS:       rel = "ne"; break;
	case ioport_condition::GREATERTHAN:     rel = "gt"; break;
	case ioport_condition::NOTGREATERTHAN:  rel = "le"; break;
	case ioport_condition::LESSTHAN:        rel = "lt"; break;
	case ioport_condition::NOTLESSTHAN:     rel = "ge"; break;
	}

	out << util::string_format("<condition tag=\"%s\" mask=\"%u\" relation=\"%s\" value=\"%u\"/>\n", normalize_string(condition.tag()), condition.mask(), rel, condition.value());
}

//-------------------------------------------------
//  output_input - print a summary of a game's
//  input
//-------------------------------------------------

void output_input(std::ostream &out, const ioport_list &portlist)
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
		for (ioport_field const &field : port.second->fields())
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
	out << "\t\t<input";
	out << util::string_format(" players=\"%d\"", nplayer);
	if (ncoin != 0)
		out << util::string_format(" coins=\"%d\"", ncoin);
	if (service)
		out << util::string_format(" service=\"yes\"");
	if (tilt)
		out << util::string_format(" tilt=\"yes\"");
	out << ">\n";

	// Then controller specific ones
	for (auto & elem : control_info)
		if (elem.type != nullptr)
		{
			//printf("type %s - player %d - buttons %d\n", elem.type, elem.player, elem.nbuttons);
			if (elem.analog)
			{
				out << util::string_format("\t\t\t<control type=\"%s\"", normalize_string(elem.type));
				if (nplayer > 1)
					out << util::string_format(" player=\"%d\"", elem.player);
				if (elem.nbuttons > 0)
				{
					out << util::string_format(" buttons=\"%d\"", strcmp(elem.type, "stick") ? elem.nbuttons : elem.maxbuttons);
					if (elem.reqbuttons < elem.nbuttons)
						out << util::string_format(" reqbuttons=\"%d\"", elem.reqbuttons);
				}
				if (elem.min != 0 || elem.max != 0)
					out << util::string_format(" minimum=\"%d\" maximum=\"%d\"", elem.min, elem.max);
				if (elem.sensitivity != 0)
					out << util::string_format(" sensitivity=\"%d\"", elem.sensitivity);
				if (elem.keydelta != 0)
					out << util::string_format(" keydelta=\"%d\"", elem.keydelta);
				if (elem.reverse)
					out << " reverse=\"yes\"";

				out << "/>\n";
			}
			else
			{
				if (elem.helper[1] == 0 && elem.helper[2] != 0) { elem.helper[1] = elem.helper[2]; elem.helper[2] = 0; }
				if (elem.helper[0] == 0 && elem.helper[1] != 0) { elem.helper[0] = elem.helper[1]; elem.helper[1] = 0; }
				if (elem.helper[1] == 0 && elem.helper[2] != 0) { elem.helper[1] = elem.helper[2]; elem.helper[2] = 0; }
				const char *joys = (elem.helper[2] != 0) ? "triple" : (elem.helper[1] != 0) ? "double" : "";
				out << util::string_format("\t\t\t<control type=\"%s%s\"", joys, normalize_string(elem.type));
				if (nplayer > 1)
					out << util::string_format(" player=\"%d\"", elem.player);
				if (elem.nbuttons > 0)
				{
					out << util::string_format(" buttons=\"%d\"", strcmp(elem.type, "joy") ? elem.nbuttons : elem.maxbuttons);
					if (elem.reqbuttons < elem.nbuttons)
						out << util::string_format(" reqbuttons=\"%d\"", elem.reqbuttons);
				}
				for (int lp = 0; lp < 3 && elem.helper[lp] != 0; lp++)
				{
					const char *plural = (lp==2) ? "3" : (lp==1) ? "2" : "";
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
					out << util::string_format(" ways%s=\"%s\"", plural, ways);
				}
				out << "/>\n";
			}
		}

	out << "\t\t</input>\n";
}


//-------------------------------------------------
//  output_switches - print the configurations or
//  DIP switch settings
//-------------------------------------------------

void output_switches(std::ostream &out, const ioport_list &portlist, const char *root_tag, int type, const char *outertag, const char *loctag, const char *innertag)
{
	// iterate looking for DIP switches
	for (auto &port : portlist)
		for (ioport_field const &field : port.second->fields())
			if (field.type() == type)
			{
				std::string newtag(port.second->tag()), oldtag(":");
				newtag = newtag.substr(newtag.find(oldtag.append(root_tag)) + oldtag.length());

				// output the switch name information
				std::string const normalized_field_name(normalize_string(field.name()));
				std::string const normalized_newtag(normalize_string(newtag.c_str()));
				out << util::string_format("\t\t<%s name=\"%s\" tag=\"%s\" mask=\"%u\">\n", outertag, normalized_field_name.c_str(), normalized_newtag.c_str(), field.mask());
				if (!field.condition().none())
					output_ioport_condition(out, field.condition(), 3);

				// loop over locations
				for (ioport_diplocation const &diploc : field.diplocations())
				{
					out << util::string_format("\t\t\t<%s name=\"%s\" number=\"%u\"", loctag, normalize_string(diploc.name()), diploc.number());
					if (diploc.inverted())
						out << " inverted=\"yes\"";
					out << "/>\n";
				}

				// loop over settings
				for (ioport_setting const &setting : field.settings())
				{
					out << util::string_format("\t\t\t<%s name=\"%s\" value=\"%u\"", innertag, normalize_string(setting.name()), setting.value());
					if (setting.value() == field.defvalue())
						out << " default=\"yes\"";
					if (setting.condition().none())
					{
						out << "/>\n";
					}
					else
					{
						out << ">\n";
						output_ioport_condition(out, setting.condition(), 4);
						out << util::string_format("\t\t\t</%s>\n", innertag);
					}
				}

				// terminate the switch entry
				out << util::string_format("\t\t</%s>\n", outertag);
			}
}

//-------------------------------------------------
//  output_ports - print the structure of input
//  ports in the driver
//-------------------------------------------------
void output_ports(std::ostream &out, const ioport_list &portlist)
{
	// cycle through ports
	for (auto &port : portlist)
	{
		out << util::string_format("\t\t<port tag=\"%s\">\n", normalize_string(port.second->tag()));
		for (ioport_field const &field : port.second->fields())
		{
			if (field.is_analog())
				out << util::string_format("\t\t\t<analog mask=\"%u\"/>\n", field.mask());
		}
		out << util::string_format("\t\t</port>\n");
	}

}

//-------------------------------------------------
//  output_adjusters - print the Analog
//  Adjusters for a game
//-------------------------------------------------

void output_adjusters(std::ostream &out, const ioport_list &portlist)
{
	// iterate looking for Adjusters
	for (auto &port : portlist)
		for (ioport_field const &field : port.second->fields())
			if (field.type() == IPT_ADJUSTER)
			{
				out << util::string_format("\t\t<adjuster name=\"%s\" default=\"%d\"/>\n", normalize_string(field.name()), field.defvalue());
			}
}


//-------------------------------------------------
//  output_driver - print driver status
//-------------------------------------------------

void output_driver(std::ostream &out, game_driver const &driver, device_t::feature_type unemulated, device_t::feature_type imperfect)
{
	out << "\t\t<driver";

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
		out << " status=\"preliminary\"";
	else if (imperfect)
		out << " status=\"imperfect\"";
	else
		out << " status=\"good\"";

	if (flags & machine_flags::NOT_WORKING)
		out << " emulation=\"preliminary\"";
	else
		out << " emulation=\"good\"";

	if (flags & machine_flags::NO_COCKTAIL)
		out << " cocktail=\"preliminary\"";

	if (flags & machine_flags::SUPPORTS_SAVE)
		out << " savestate=\"supported\"";
	else
		out << " savestate=\"unsupported\"";

	if (flags & machine_flags::REQUIRES_ARTWORK)
		out << " requiresartwork=\"yes\"";

	if (flags & machine_flags::UNOFFICIAL)
		out << " unofficial=\"yes\"";

	if (flags & machine_flags::NO_SOUND_HW)
		out << " nosoundhardware=\"yes\"";

	if (flags & machine_flags::IS_INCOMPLETE)
		out << " incomplete=\"yes\"";

	out << "/>\n";
}


//-------------------------------------------------
//  output_features - print emulation features of
//
//-------------------------------------------------

void output_features(std::ostream &out, device_type type, device_t::feature_type unemulated, device_t::feature_type imperfect)
{
	device_t::feature_type const flags(type.unemulated_features() | type.imperfect_features() | unemulated | imperfect);
	for (auto const &feature : f_feature_names)
	{
		if (flags & feature.first)
		{
			out << util::string_format("\t\t<feature type=\"%s\"", feature.second);
			if (type.unemulated_features() & feature.first)
			{
				out << " status=\"unemulated\"";
			}
			else
			{
				if (type.imperfect_features() & feature.first)
					out << " status=\"imperfect\"";
				if (unemulated & feature.first)
					out << " overall=\"unemulated\"";
				else if ((~type.imperfect_features() & imperfect) & feature.first)
					out << " overall=\"imperfect\"";
			}
			out << "/>\n";
		}
	}
}


//-------------------------------------------------
//  output_images - prints m_output all info on
//  image devices
//-------------------------------------------------

void output_images(std::ostream &out, device_t &device, const char *root_tag)
{
	for (const device_image_interface &imagedev : image_interface_enumerator(device))
	{
		if (strcmp(imagedev.device().tag(), device.tag()))
		{
			bool loadable = imagedev.user_loadable();
			std::string newtag(imagedev.device().tag()), oldtag(":");
			newtag = newtag.substr(newtag.find(oldtag.append(root_tag)) + oldtag.length());

			// print m_output device type
			out << util::string_format("\t\t<device type=\"%s\"", normalize_string(imagedev.image_type_name()));

			// does this device have a tag?
			if (imagedev.device().tag())
				out << util::string_format(" tag=\"%s\"", normalize_string(newtag.c_str()));

			// is this device available as media switch?
			if (!loadable)
				out << " fixed_image=\"1\"";

			// is this device mandatory?
			if (imagedev.must_be_loaded())
				out << " mandatory=\"1\"";

			if (imagedev.image_interface() && imagedev.image_interface()[0])
				out << util::string_format(" interface=\"%s\"", normalize_string(imagedev.image_interface()));

			// close the XML tag
			out << ">\n";

			if (loadable)
			{
				char const *const name = imagedev.instance_name().c_str();
				char const *const shortname = imagedev.brief_instance_name().c_str();

				out << "\t\t\t<instance";
				out << util::string_format(" name=\"%s\"", normalize_string(name));
				out << util::string_format(" briefname=\"%s\"", normalize_string(shortname));
				out << "/>\n";

				char const *extensions(imagedev.file_extensions());
				while (extensions)
				{
					char const *end(extensions);
					while (*end && (',' != *end))
						++end;
					out << util::string_format("\t\t\t<extension name=\"%s\"/>\n", normalize_string(std::string(extensions, end).c_str()));
					extensions = *end ? (end + 1) : nullptr;
				}
			}
			out << "\t\t</device>\n";
		}
	}
}


//-------------------------------------------------
//  output_slots - prints all info about slots
//-------------------------------------------------

void output_slots(std::ostream &out, machine_config &config, device_t &device, const char *root_tag, device_type_set *devtypes)
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

			// print m_output device type
			if (listed)
				out << util::string_format("\t\t<slot name=\"%s\">\n", normalize_string(newtag.c_str()));

			for (auto &option : slot.option_list())
			{
				if (devtypes || (listed && option.second->selectable()))
				{
					device_t *const dev = config.device_add("_dummy", option.second->devtype(), option.second->clock());
					if (!dev->configured())
						dev->config_complete();

					if (devtypes)
						for (device_t &subdevice : device_enumerator(*dev)) devtypes->insert(&subdevice.type());

					if (listed && option.second->selectable())
					{
						out << util::string_format("\t\t\t<slotoption name=\"%s\"", normalize_string(option.second->name()));
						out << util::string_format(" devname=\"%s\"", normalize_string(dev->shortname()));
						if (slot.default_option() != nullptr && strcmp(slot.default_option(), option.second->name())==0)
							out << " default=\"yes\"";
						out << "/>\n";
					}

					config.device_remove("_dummy");
				}
			}

			if (listed)
				out << "\t\t</slot>\n";
		}
	}
}


//-------------------------------------------------
//  output_software_lists - print the information
//  for all known software lists for this system
//-------------------------------------------------

void output_software_lists(std::ostream &out, device_t &root, const char *root_tag)
{
	for (const software_list_device &swlist : software_list_device_enumerator(root))
	{
		if (&static_cast<const device_t &>(swlist) == &root)
		{
			assert(swlist.list_name().empty());
			continue;
		}

		std::string newtag(swlist.tag()), oldtag(":");
		newtag = newtag.substr(newtag.find(oldtag.append(root_tag)) + oldtag.length());
		out << util::string_format("\t\t<softwarelist tag=\"%s\" name=\"%s\" status=\"%s\"", normalize_string(newtag.c_str()), normalize_string(swlist.list_name().c_str()), swlist.is_original() ? "original" : "compatible");

		if (swlist.filter())
			out << util::string_format(" filter=\"%s\"", normalize_string(swlist.filter()));
		out << "/>\n";
	}
}



//-------------------------------------------------
//  output_ramoptions - prints m_output all RAM
//  options for this system
//-------------------------------------------------

void output_ramoptions(std::ostream &out, device_t &root)
{
	for (const ram_device &ram : ram_device_enumerator(root, 1))
	{
		if (!std::strcmp(ram.tag(), ":" RAM_TAG))
		{
			uint32_t const defsize(ram.default_size());
			bool havedefault(false);
			for (ram_device::extra_option const &option : ram.extra_options())
			{
				if (defsize == option.second)
				{
					assert(!havedefault);
					havedefault = true;
					out << util::string_format("\t\t<ramoption name=\"%s\" default=\"yes\">%u</ramoption>\n", normalize_string(option.first.c_str()), option.second);
				}
				else
				{
					out << util::string_format("\t\t<ramoption name=\"%s\">%u</ramoption>\n", normalize_string(option.first.c_str()), option.second);
				}
			}
			if (!havedefault)
				out << util::string_format("\t\t<ramoption name=\"%s\" default=\"yes\">%u</ramoption>\n", ram.default_size_string(), defsize);
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
