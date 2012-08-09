/***************************************************************************

    dac.h

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

#ifndef __DAC_H__
#define __DAC_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_DAC_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DAC, 0) \



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> dac_device

class dac_device : public device_t,
				   public device_sound_interface
{
	// default to 4x oversampling
	static const UINT32 DEFAULT_SAMPLE_RATE = 48000 * 4;

public:
	// construction/destruction
	dac_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// public interface
	INT16 output() const { return m_output; }
	void write(INT16 data) { m_stream->update(); m_output = data; }
	void write_unsigned8(UINT8 data) { write(data * 32767 / 255); }			// 0..255, mapped to 0..32767
	void write_signed8(UINT8 data) { write((data - 0x80) * 32767 / 128); }	// 0..255, mapped to -32767..32767
	void write_unsigned16(UINT16 data) { write(data / 2); }					// 0..65535, mapped to 0..32767
	void write_signed16(UINT16 data) { write(data - 0x8000); }				// 0..65535, mapped to -32768..32767

	// wrappers
	DECLARE_WRITE8_MEMBER( write_unsigned8 );
	DECLARE_WRITE8_MEMBER( write_signed8 );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	// internal state
	sound_stream *				m_stream;
	INT16						m_output;
};


// device type definition
extern const device_type DAC;


#endif /* __DAC_H__ */
