// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/***************************************************************************

    okim9810.h

    OKI MSM9810 ADPCM(2) sound chip.

    Notes:
    The master clock frequency for this chip can range from 3.5MHz to 4.5Mhz.
      The typical oscillator is a 4.096Mhz crystal.

***************************************************************************/

#pragma once

#ifndef __OKIM9810_H__
#define __OKIM9810_H__

#include "okiadpcm.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

enum
{
	OKIM9810_ADPCM_PLAYBACK = 0,
	OKIM9810_ADPCM2_PLAYBACK = 1,
	OKIM9810_STRAIGHT8_PLAYBACK = 2,
	OKIM9810_NONLINEAR8_PLAYBACK = 3
};

enum
{
	OKIM9810_SECONDARY_FILTER = 0,
	OKIM9810_PRIMARY_FILTER = 1,
	OKIM9810_NO_FILTER = 2,
	OKIM9810_NO_FILTER2 = 3
};

enum
{
	OKIM9810_OUTPUT_TO_DIRECT_DAC = 0,
	OKIM9810_OUTPUT_TO_VOLTAGE_FOLLOWER = 1
};


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_OKIM9810_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, OKIM9810, _clock)

#define MCFG_OKIM9810_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, OKIM9810, _clock)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> okim9810_device

class okim9810_device : public device_t,
						public device_sound_interface,
						public device_rom_interface
{
public:
	// construction/destruction
	okim9810_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read_status();
	void write_TMP_register(uint8_t command);
	void write_command(uint8_t command);

	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void write_TMP_register(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;
	virtual void device_clock_changed() override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// device_rom_interface overrides
	virtual void rom_bank_updated() override;

	// a single voice
	class okim_voice
	{
	public:
		okim_voice();
		void generate_audio(device_rom_interface &rom,
							stream_sample_t **buffers,
							int samples,
							const uint8_t global_volume,
							const uint32_t clock,
							const uint8_t filter_type);

		// computes volume scale from 3 volume numbers
		uint8_t volume_scale(const uint8_t global_volume,
							const uint8_t channel_volume,
							const uint8_t pan_volume) const;

		oki_adpcm_state m_adpcm;    // current ADPCM state
		oki_adpcm2_state m_adpcm2;  // current ADPCM2 state
		uint8_t   m_playbackAlgo;     // current playback method
		bool    m_looping;
		uint8_t   m_startFlags;
		uint8_t   m_endFlags;
		offs_t  m_base_offset;      // pointer to the base memory location
		uint32_t  m_count;            // total samples to play
		uint32_t  m_samplingFreq;     // voice sampling frequency

		bool    m_playing;          // playback state
		uint32_t  m_sample;           // current sample number

		uint8_t   m_channel_volume;   // volume index set with the CVOL command
		uint8_t   m_pan_volume_left;  // volume index set with the PAN command
		uint8_t   m_pan_volume_right; // volume index set with the PAN command

		int32_t   m_startSample;      // interpolation state - sample to interpolate from
		int32_t   m_endSample;        // interpolation state - sample to interpolate to
		uint32_t  m_interpSampleNum;  // interpolation state - fraction between start & end

		static const uint8_t s_volume_table[16];
	};

	// internal state

	sound_stream* m_stream;

	uint8_t m_TMP_register;

	uint8_t m_global_volume;      // volume index set with the OPT command
	uint8_t m_filter_type;        // interpolation filter type set with the OPT command
	uint8_t m_output_level;       // flag stating if a voltage follower is connected

	static const int OKIM9810_VOICES = 8;
	okim_voice m_voice[OKIM9810_VOICES];

	static const uint32_t s_sampling_freq_table[16];
};


// device type definition
extern const device_type OKIM9810;



#endif // __OKIM9810_H__
