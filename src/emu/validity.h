// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    validity.h

    Validity checks

***************************************************************************/

#ifndef MAME_EMU_VALIDITY_H
#define MAME_EMU_VALIDITY_H

#pragma once

#include "drivenum.h"
#include "emuopts.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// core validity checker class
class validity_checker : public osd_output
{
public:
	validity_checker(emu_options &options);
	~validity_checker();

	// getters
	int errors() const { return m_errors; }
	int warnings() const { return m_warnings; }
	bool validate_all() const { return m_validate_all; }

	// setter
	void set_verbose(bool verbose) { m_print_verbose = verbose; }
	void set_validate_all(bool all) { m_validate_all = all; }

	// operations
	void check_driver(const game_driver &driver);
	void check_shared_source(const game_driver &driver);
	bool check_all_matching(const char *string = "*");

	// helpers for devices
	void validate_tag(const char *tag);
	int region_length(const char *tag) { auto i = m_region_map.find(tag); return i == m_region_map.end() ? 0 : i->second; }

	// generic registry of already-checked stuff
	bool already_checked(const char *string) { return !m_already_checked.insert(string).second; }

	// osd_output interface

protected:
	virtual void output_callback(osd_output_channel channel, const util::format_argument_pack<std::ostream> &args) override;

private:
	// internal map types
	typedef std::unordered_map<std::string,const game_driver *> game_driver_map;
	typedef std::unordered_map<std::string,uintptr_t> int_map;

	// internal helpers
	const char *ioport_string_from_index(u32 index);
	int get_defstr_index(const char *string, bool suppress_error = false);

	// core helpers
	void validate_begin();
	void validate_end();
	void validate_one(const game_driver &driver);

	// internal sub-checks
	void validate_core();
	void validate_inlines();
	void validate_rgb();
	void validate_driver();
	void validate_roms(device_t &root);
	void validate_analog_input_field(ioport_field &field);
	void validate_dip_settings(ioport_field &field);
	void validate_condition(ioport_condition &condition, device_t &device, std::unordered_set<std::string> &port_map);
	void validate_inputs();
	void validate_devices();
	void validate_device_types();

	// output helpers
	void build_output_prefix(std::ostream &str) const;
	template <typename Format, typename... Params> void output_via_delegate(osd_output_channel channel, Format &&fmt, Params &&...args);
	void output_indented_errors(std::string &text, const char *header);

	// random number generation
	s32 random_i32();
	u32 random_u32();
	s64 random_i64();
	u64 random_u64();

	// internal driver list
	driver_enumerator       m_drivlist;

	// blank options for use during validation
	emu_options             m_blank_options;

	// error tracking
	int                     m_errors;
	int                     m_warnings;
	bool                    m_print_verbose;
	std::string             m_error_text;
	std::string             m_warning_text;
	std::string             m_verbose_text;

	// maps for finding duplicates
	game_driver_map         m_names_map;
	game_driver_map         m_descriptions_map;
	game_driver_map         m_roms_map;
	int_map                 m_defstr_map;

	// current state
	const game_driver *     m_current_driver;
	machine_config *        m_current_config;
	const device_t *        m_current_device;
	const char *            m_current_ioport;
	int_map                 m_region_map;
	std::unordered_set<std::string>   m_already_checked;
	bool                    m_validate_all;
};

#endif // MAME_EMU_VALIDITY_H
