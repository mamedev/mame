// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    dac.h

    DAC device emulator.

***************************************************************************/

#ifndef __DAC_H__
#define __DAC_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_DAC_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DAC, 0)


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
	dac_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// public interface
	INT16 output() const { return m_output; }
	void write(INT16 data) { m_stream->update(); m_output = data; }
	void write_unsigned8(UINT8 data) { write(data * 32767 / 255); }         // 0..255, mapped to 0..32767
	void write_signed8(UINT8 data) { write((data - 0x80) * 32767 / 128); }  // 0..255, mapped to -32767..32767
	void write_unsigned16(UINT16 data) { write(data / 2); }                 // 0..65535, mapped to 0..32767
	void write_signed16(UINT16 data) { write(data - 0x8000); }              // 0..65535, mapped to -32768..32767

	// wrappers
	DECLARE_WRITE8_MEMBER( write_unsigned8 );
	DECLARE_WRITE8_MEMBER( write_signed8 );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// internal state
	sound_stream *              m_stream;
	INT16                       m_output;
};


// device type definition
extern const device_type DAC;


#endif /* __DAC_H__ */
