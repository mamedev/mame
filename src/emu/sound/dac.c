/***************************************************************************

    dac.c

    DAC device emulator.

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
#include "dac.h"


// device type definition
const device_type DAC = &device_creator<dac_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dac_device - constructor
//-------------------------------------------------

dac_device::dac_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DAC, "DAC", "dac", tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  m_stream(NULL),
	  m_output(0)
{
}


//-------------------------------------------------
//  write_unsigned8 - write an 8-bit value,
//  keeping the scaled result unsigned
//-------------------------------------------------

WRITE8_MEMBER( dac_device::write_unsigned8 )
{
	write_unsigned8(data);
}


//-------------------------------------------------
//  write_signed8 - write an 8-bit value,
//  keeping the scaled result signed
//-------------------------------------------------

WRITE8_MEMBER( dac_device::write_signed8 )
{
	write_signed8(data);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dac_device::device_start()
{
	// create the stream
	m_stream = stream_alloc(0, 1, DEFAULT_SAMPLE_RATE);

	// register for save states
	save_item(NAME(m_output));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dac_device::device_reset()
{
	m_output = 0;
}


//-------------------------------------------------
//  sound_stream_update - handle update requests
//  for our sound stream
//-------------------------------------------------

void dac_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// just fill with current value
	for (int samp = 0; samp < samples; samp++)
		outputs[0][samp] = m_output;
}
