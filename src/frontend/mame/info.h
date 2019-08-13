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
	void output(std::ostream &out, const std::vector<std::string> &patterns);
	void output(std::ostream &out, const std::function<bool(const char *shortname, bool &done)> &filter = { }, bool include_devices = true);

private:
	class device_type_compare
	{
	public:
		bool operator()(const std::add_pointer_t<device_type> &lhs, const std::add_pointer_t<device_type> &rhs) const;
	};

	typedef std::set<std::add_pointer_t<device_type>, device_type_compare> device_type_set;

	// internal helper
	void output_header(std::ostream &out);
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
	void output_software_list(std::ostream &out, device_t &root);
	void output_ramoptions(std::ostream &out, device_t &root);

	void output_one_device(std::ostream &out, machine_config &config, device_t &device, const char *devtag);
	void output_devices(std::ostream &out, device_type_set const *filter);

	const char *get_merge_name(driver_enumerator &drivlist, const game_driver &driver, util::hash_collection const &romhashes);

	// internal state
	emu_options     m_lookup_options;

	static const char s_dtd_string[];
	bool m_dtd;
};

#endif // MAME_FRONTEND_MAME_INFO_H
