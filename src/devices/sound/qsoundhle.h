// license:BSD-3-Clause
// copyright-holders:Paul Leaman, Miguel Angel Horna
/*********************************************************

    Capcom System QSound™ (HLE)

*********************************************************/
#ifndef MAME_SOUND_QSOUNDHLE_H
#define MAME_SOUND_QSOUNDHLE_H

#pragma once

#include "cpu/dsp16/dsp16.h"

// default 60MHz clock (divided by 2 for DSP core clock, and then by 1248 for sample rate)
#define QSOUND_CLOCK 60_MHz_XTAL


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************


// ======================> qsound_hle_device

class qsound_hle_device : public device_t, public device_sound_interface, public device_rom_interface
{
public:
	qsound_hle_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	DECLARE_WRITE8_MEMBER(qsound_w);
	DECLARE_READ8_MEMBER(qsound_r);

protected:
	// device_t implementation
	tiny_rom_entry const *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// device_rom_interface implementation
	virtual void rom_bank_updated() override;

private:

	// MAME resources
	required_device<dsp16_device_base> m_dsp;
	sound_stream *m_stream;

	struct qsound_channel
	{
		uint32_t bank;        // bank
		uint32_t address;     // start/cur address
		uint16_t loop;        // loop address
		uint16_t end;         // end address
		uint32_t freq;        // frequency
		uint16_t vol;         // master volume

		// work variables
		bool enabled;       // key on / key off
		int lvol;           // left volume
		int rvol;           // right volume
		uint32_t step_ptr;    // current offset counter
	} m_channel[16];

	int m_pan_table[33];    // pan volume table
	uint16_t m_data;          // register latch data

	inline int8_t read_sample(uint32_t offset) { return (int8_t)read_byte(offset); }
	void write_data(uint8_t address, uint16_t data);
};

DECLARE_DEVICE_TYPE(QSOUND_HLE, qsound_hle_device)

#endif // MAME_SOUND_QSOUNDHLE_H
