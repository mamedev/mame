// license:BSD-3-Clause
// copyright-holders:Paul Leaman, Miguel Angel Horna
/*********************************************************

    Capcom Q-Sound system

*********************************************************/

#ifndef MAME_SOUND_QSOUND_H
#define MAME_SOUND_QSOUND_H

#pragma once

#include "cpu/dsp16/dsp16.h"

#define QSOUND_CLOCK 4000000    /* default 4MHz clock (60MHz/15?) */


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_QSOUND_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, QSOUND, _clock)
#define MCFG_QSOUND_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, QSOUND, _clock)


// ======================> qsound_device

class qsound_device : public device_t,
						public device_sound_interface,
						public device_rom_interface
{
public:
	qsound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER(qsound_w);
	DECLARE_READ8_MEMBER(qsound_r);

	void dsp16_data_map(address_map &map);
	void dsp16_program_map(address_map &map);
protected:
	// device-level overrides
	const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// device_rom_interface overrides
	virtual void rom_bank_updated() override;

private:
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

	required_device<dsp16_device> m_cpu;

	int m_pan_table[33];    // pan volume table
	uint16_t m_data;          // register latch data
	sound_stream *m_stream; // audio stream

	inline int8_t read_sample(uint32_t offset) { return (int8_t)read_byte(offset); }
	void write_data(uint8_t address, uint16_t data);
};

DECLARE_DEVICE_TYPE(QSOUND, qsound_device)

#endif // MAME_SOUND_QSOUND_H
