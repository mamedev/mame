// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    info.h

    Dumps the MAME internal data as an XML file.

***************************************************************************/

#pragma once

#ifndef __INFO_H__
#define __INFO_H__

#include "drivenum.h"


//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

// helper class to putput
class info_xml_creator
{
public:
	// construction/destruction
	info_xml_creator(driver_enumerator &drivlist);

	// output
	void output(FILE *out, bool nodevices = false);

private:
	// internal helper
	void output_one();
	void output_sampleof();
	void output_bios();
	void output_rom(device_t &device);
	void output_device_roms();
	void output_sample(device_t &device);
	void output_chips(device_t &device, const char *root_tag);
	void output_display(device_t &device, const char *root_tag);
	void output_sound(device_t &device);
	void output_input(const ioport_list &portlist);
	void output_switches(const ioport_list &portlist, const char *root_tag, int type, const char *outertag, const char *innertag);
	void output_ports(const ioport_list &portlist);
	void output_adjusters(const ioport_list &portlist);
	void output_driver();
	void output_images(device_t &device, const char *root_tag);
	void output_slots(device_t &device, const char *root_tag);
	void output_software_list();
	void output_ramoptions();

	void output_one_device(device_t &device, const char *devtag);
	void output_devices();

	const char *get_merge_name(const hash_collection &romhashes);

	// internal state
	FILE *                  m_output;
	driver_enumerator &     m_drivlist;
	emu_options             m_lookup_options;

	static const char s_dtd_string[];
};


#endif  /* __INFO_H__ */
