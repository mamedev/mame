/***************************************************************************

    votrax.h

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

#pragma once

#ifndef __VOTRAX_H__
#define __VOTRAX_H__

#include "sound/samples.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_VOTRAX_ADD(_tag, _clock, _interface) \
	MCFG_DEVICE_ADD(_tag, VOTRAX, 0) \
	votrax_device::static_set_interface(*device, _interface);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> votrax_interface sample

struct votrax_map
{
	const char *phoneme;
	const char *samplename;
};

struct votrax_interface
{
	votrax_map const *m_votrax_map;		// array of map entries
};


// ======================> votrax_device

class votrax_device :	public samples_device,
						public votrax_interface
{
public:
	// construction/destruction
	votrax_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	static void static_set_interface(device_t &device, const votrax_interface &interface);

	// writers
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ_LINE_MEMBER( status );

protected:
	// device-level overrides
	virtual void device_start();

private:
	// internal state
	astring						m_current;
	dynamic_array<const char *>	m_sample_list;

	static const char *const s_phoneme_table[64];
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
extern const device_type VOTRAX;


#endif /* __VOTRAX_H__ */
