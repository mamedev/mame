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
	validity_checker(emu_options &options, bool quick);
	~validity_checker();

	// getters
	int errors() const { return m_errors; }
	int warnings() const { return m_warnings; }
	bool quick() const { return m_quick; }

	// setter
	void set_verbose(bool verbose) { m_print_verbose = verbose; }

	// operations
	void check_driver(const game_driver &driver);
	void check_shared_source(const game_driver &driver);
	bool check_all_matching(const char *string = "*");

	// helpers for devices
	void validate_tag(const char *tag);
	int region_length(const char *tag) { auto const i = m_region_map.find(tag); return (i == m_region_map.end()) ? 0 : i->second; }
	bool ioport_missing(const char *tag) { return !m_checking_card && (m_ioport_set.find(tag) == m_ioport_set.end()); }

	// generic registry of already-checked stuff
	bool already_checked(const char *string) { return !m_already_checked.insert(string).second; }

protected:
	// osd_output interface
	virtual void output_callback(osd_output_channel channel, const util::format_argument_pack<std::ostream> &args) override;

private:
	// internal map types
	using game_driver_map = std::unordered_map<std::string, game_driver const *>;
	using int_map = std::unordered_map<std::string, uintptr_t>;
	using string_set = std::unordered_set<std::string>;

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
	void validate_driver(device_t &root);
	void validate_roms(device_t &root);
	void validate_analog_input_field(const ioport_field &field);
	void validate_dip_settings(const ioport_field &field);
	void validate_condition(const ioport_condition &condition, device_t &device);
	void validate_inputs(device_t &root);
	void validate_devices(machine_config &config);
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
	game_driver const *     m_current_driver;
	device_t const *        m_current_device;
	char const *            m_current_ioport;
	int_map                 m_region_map;
	string_set              m_ioport_set;
	string_set              m_already_checked;
	bool                    m_checking_card;
	bool const              m_quick;
};

#endif // MAME_EMU_VALIDITY_H
