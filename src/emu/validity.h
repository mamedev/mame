/***************************************************************************

    validity.h

    Validity checks

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __VALIDITY_H__
#define __VALIDITY_H__

#include "emu.h"
#include "drivenum.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declarations
class machine_config;


// core validity checker class
class validity_checker
{
	// internal map types
	typedef tagmap_t<const game_driver *> game_driver_map;
	typedef tagmap_t<FPTR> int_map;

public:
	validity_checker(emu_options &options);

	// getters
	int errors() const { return m_errors; }
	int warnings() const { return m_warnings; }

	// operations
	void check_driver(const game_driver &driver);
	void check_shared_source(const game_driver &driver);
	void check_all();

	// helpers for devices
	void validate_tag(const char *tag);

private:
	// internal helpers
	const char *ioport_string_from_index(UINT32 index);
	int get_defstr_index(const char *string, bool suppress_error = false);

	// core helpers
	void validate_begin();
	void validate_end();
	void validate_one(const game_driver &driver);

	// internal sub-checks
	void validate_core();
	void validate_inlines();
	void validate_driver();
	void validate_roms();
	void validate_display();
	void validate_gfx();
	void validate_analog_input_field(ioport_field &field);
	void validate_dip_settings(ioport_field &field);
	void validate_condition(ioport_condition &condition, device_t &device, int_map &port_map);
	void validate_inputs();
	void validate_devices();

	// output helpers
	void build_output_prefix(astring &string);
	void error_output(const char *format, va_list argptr);
	void warning_output(const char *format, va_list argptr);
	void output_via_delegate(output_delegate &delegate, const char *format, ...);

	// internal driver list
	driver_enumerator       m_drivlist;

	// error tracking
	int                     m_errors;
	int                     m_warnings;
	astring                 m_error_text;
	astring                 m_warning_text;

	// maps for finding duplicates
	game_driver_map         m_names_map;
	game_driver_map         m_descriptions_map;
	game_driver_map         m_roms_map;
	int_map                 m_defstr_map;

	// current state
	const game_driver *     m_current_driver;
	const machine_config *  m_current_config;
	const device_t *        m_current_device;
	const char *            m_current_ioport;
	int_map                 m_region_map;

	// callbacks
	output_delegate         m_saved_error_output;
	output_delegate         m_saved_warning_output;
};

#endif
