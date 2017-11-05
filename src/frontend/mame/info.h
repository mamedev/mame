// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    info.h

    Dumps the MAME internal data as an XML file.

***************************************************************************/

#ifndef MAME_FRONTEND_MAME_INFO_H
#define MAME_FRONTEND_MAME_INFO_H

#pragma once

#include "emuopts.h"

#include <type_traits>
#include <unordered_set>
#include <vector>


class driver_enumerator;


//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

// helper class to putput
class info_xml_creator
{
public:
	// construction/destruction
	info_xml_creator(emu_options const &options, bool dtd = true);

	// output
	void output(FILE *out, std::vector<std::string> const &patterns);
	void output(FILE *out, driver_enumerator &drivlist, bool nodevices);

private:
	typedef std::unordered_set<std::add_pointer_t<device_type> > device_type_set;

	// internal helper
	void output_header();
	void output_footer();

	void output_one(driver_enumerator &drivlist, device_type_set *devtypes);
	void output_sampleof(device_t &device);
	void output_bios(device_t const &device);
	void output_rom(driver_enumerator *drivlist, device_t &device);
	void output_device_refs(device_t &root);
	void output_sample(device_t &device);
	void output_chips(device_t &device, const char *root_tag);
	void output_display(device_t &device, machine_flags::type const *flags, const char *root_tag);
	void output_sound(device_t &device);
	void output_ioport_condition(const ioport_condition &condition, unsigned indent);
	void output_input(const ioport_list &portlist);
	void output_switches(const ioport_list &portlist, const char *root_tag, int type, const char *outertag, const char *loctag, const char *innertag);
	void output_ports(const ioport_list &portlist);
	void output_adjusters(const ioport_list &portlist);
	void output_driver(game_driver const &driver, device_t::feature_type unemulated, device_t::feature_type imperfect);
	void output_features(device_type type, device_t::feature_type unemulated, device_t::feature_type imperfect);
	void output_images(device_t &device, const char *root_tag);
	void output_slots(machine_config &config, device_t &device, const char *root_tag, device_type_set *devtypes);
	void output_software_list(device_t &root);
	void output_ramoptions(device_t &root);

	void output_one_device(machine_config &config, device_t &device, const char *devtag);
	void output_devices(device_type_set const *filter);

	const char *get_merge_name(driver_enumerator &drivlist, util::hash_collection const &romhashes);

	// internal state
	FILE *          m_output; // FIXME: this is not reentrancy-safe
	emu_options     m_lookup_options;

	static const char s_dtd_string[];
	bool m_dtd;
};

#endif // MAME_FRONTEND_MAME_INFO_H
