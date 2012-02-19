/***************************************************************************

    votrax.c

    Hacked up votrax simulator that maps to samples, until a real one
    is written.

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

#include "emu.h"
#include "votrax.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type VOTRAX = &device_creator<votrax_device>;

const char *const votrax_device::s_phoneme_table[64] =
{
	"EH3",	"EH2",	"EH1",	" "/*PA0*/"DT",	"A1", 	"A2",  	"ZH",
	"AH2",	"I3", 	"I2", 	"I1", 	"M",  	"N",  	"B",   	"V",
	"CH", 	"SH", 	"Z",  	"AW1",	"NG", 	"AH1",	"OO1", 	"OO",
	"L",  	"K",  	"J",  	"H",  	"G",  	"F",  	"D",   	"S",
	"A",  	"AY", 	"Y1", 	"UH3",	"AH", 	"P",  	"O",   	"I",
	"U",  	"Y",  	"T",  	"R",  	"E",  	"W",  	"AE",  	"AE1",
	"AW2",	"UH2",	"UH1",	"UH", 	"O2", 	"O1", 	"IU",  	"U1",
	"THV",	"TH", 	"ER", 	"EH", 	"E1", 	"AW", 	" "/*PA1*/, "."/*STOP*/
};



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  votrax_device - constructor
//-------------------------------------------------

votrax_device::votrax_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: samples_device(mconfig, VOTRAX, "VOTRAX SC-01", tag, owner, clock)
{
}


//-------------------------------------------------
//  static_set_interface - configuration helper
//  to set the interface
//-------------------------------------------------

void votrax_device::static_set_interface(device_t &device, const votrax_interface &interface)
{
	votrax_device &samples = downcast<votrax_device &>(device);
	static_cast<votrax_interface &>(samples) = interface;
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  write - handle a write to the control register
//-------------------------------------------------

WRITE8_MEMBER( votrax_device::write )
{
	// append to the current string
	m_current.cat(s_phoneme_table[data & 0x3f]);
	
	// look for a match in our sample table
	for (int index = 0; m_votrax_map[index].phoneme != NULL; index++)
		if (m_current.find(m_votrax_map[index].phoneme) != -1)
		{
			// if we found it, play the corresponding sample and flush the buffer
			start(0, index);
			m_current.replace(m_votrax_map[index].phoneme, "");
			m_current.trimspace();
			if (m_current.len() > 0)
				mame_printf_warning("Votrax missed partial match: %s\n", m_current.cstr());
			m_current.reset();
			return;
		}
	
	// if we got a stop and didn't find a match, print it
	if ((data & 0x3f) == 0x3f)
	{
		mame_printf_warning("Votrax missed match: %s\n", m_current.cstr());
		m_current.reset();
	}
}


//-------------------------------------------------
//  status - read the status line
//-------------------------------------------------

READ_LINE_MEMBER( votrax_device::status )
{
	// is this correct, or is it really a ready line and should be inverted?
	return (m_current.len() > 0 || playing(0)) ? ASSERT_LINE : CLEAR_LINE;
}



//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_start - handle device startup
//-------------------------------------------------

void votrax_device::device_start()
{
	// build up a samples list
	for (const votrax_map *curmap = m_votrax_map; curmap->phoneme != NULL; curmap++)
		m_sample_list.append((curmap->samplename != NULL) ? curmap->samplename : "");

	// create the samples interface
	m_channels = 1;
	m_names = m_sample_list;
	m_start = NULL;
	
	// let the samples device do the rest
	samples_device::device_start();
}
