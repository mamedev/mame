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


class driver_enumerator;


//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

// helper class to putput
class info_xml_creator
{
public:
	// construction/destruction
	info_xml_creator(driver_enumerator &drivlist, bool filter_devices);

	// output
	void output(FILE *out, bool nodevices = false);

private:
	typedef std::unordered_set<std::add_pointer_t<device_type> > device_type_set;

	// internal helper
	void output_one(device_type_set *devtypes);
	void output_sampleof(device_t &device);
	void output_bios();
	void output_rom(device_t &device);
	void output_device_roms();
	void output_sample(device_t &device);
	void output_chips(device_t &device, const char *root_tag);
	void output_display(device_t &device, u32 const *flags, const char *root_tag);
	void output_sound(device_t &device);
	void output_input(const ioport_list &portlist);
	void output_switches(const ioport_list &portlist, const char *root_tag, int type, const char *outertag, const char *innertag);
	void output_ports(const ioport_list &portlist);
	void output_adjusters(const ioport_list &portlist);
	void output_driver();
	void output_images(device_t &device, const char *root_tag);
	void output_slots(machine_config &config, device_t &device, const char *root_tag, device_type_set *devtypes);
	void output_software_list();
	void output_ramoptions();

	void output_one_device(machine_config &config, device_t &device, const char *devtag);
	void output_devices(device_type_set const *filter);

	const char *get_merge_name(const util::hash_collection &romhashes);

	// internal state
	FILE *                  m_output;
	driver_enumerator &     m_drivlist;
	bool                    m_filter_devices;
	emu_options             m_lookup_options;

	static const char s_dtd_string[];
};


#endif  // MAME_FRONTEND_MAME_INFO_H
