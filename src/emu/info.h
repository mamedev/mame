/***************************************************************************

    info.h

    Dumps the MAME internal data as an XML file.

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

#ifndef __INFO_H__
#define __INFO_H__


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
	void output(FILE *out);

private:
	// internal helper
	void output_one();
	void output_sampleof();
	void output_bios();
	void output_rom(device_t &device);
	void output_device_roms();
	void output_sample();
	void output_chips();
	void output_display();
	void output_sound();
	void output_input(const ioport_list &portlist);
	void output_switches(const ioport_list &portlist, int type, const char *outertag, const char *innertag);
	void output_adjusters(const ioport_list &portlist);
	void output_categories(const ioport_list &portlist);
	void output_driver();
	void output_images();
	void output_slots();
	void output_software_list();
	void output_ramoptions();

	void output_devices();

	const char *get_merge_name(const hash_collection &romhashes);

	// internal state
	FILE *					m_output;
	driver_enumerator &		m_drivlist;
	dynamic_array<UINT8>	m_device_used;
	emu_options 			m_lookup_options;

	static const char s_dtd_string[];
};


#endif	/* __INFO_H__ */
