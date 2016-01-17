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
						public device_memory_interface
{
public:
	// construction/destruction
	okim9810_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	UINT8 read_status();
	void write_TMP_register(UINT8 command);
	void write_command(UINT8 command);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE8_MEMBER( write_TMP_register );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;
	virtual void device_clock_changed() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// a single voice
	class okim_voice
	{
	public:
		okim_voice();
		void generate_audio(direct_read_data &direct,
							stream_sample_t **buffers,
							int samples,
							const UINT8 global_volume,
							const UINT32 clock,
							const UINT8 filter_type);

		// computes volume scale from 3 volume numbers
		UINT8 volume_scale(const UINT8 global_volume,
							const UINT8 channel_volume,
							const UINT8 pan_volume) const;

		oki_adpcm_state m_adpcm;    // current ADPCM state
		oki_adpcm2_state m_adpcm2;  // current ADPCM2 state
		UINT8   m_playbackAlgo;     // current playback method
		bool    m_looping;
		UINT8   m_startFlags;
		UINT8   m_endFlags;
		offs_t  m_base_offset;      // pointer to the base memory location
		UINT32  m_count;            // total samples to play
		UINT32  m_samplingFreq;     // voice sampling frequency

		bool    m_playing;          // playback state
		UINT32  m_sample;           // current sample number

		UINT8   m_channel_volume;   // volume index set with the CVOL command
		UINT8   m_pan_volume_left;  // volume index set with the PAN command
		UINT8   m_pan_volume_right; // volume index set with the PAN command

		INT32   m_startSample;      // interpolation state - sample to interpolate from
		INT32   m_endSample;        // interpolation state - sample to interpolate to
		UINT32  m_interpSampleNum;  // interpolation state - fraction between start & end

		static const UINT8 s_volume_table[16];
	};

	// internal state
	const address_space_config  m_space_config;

	sound_stream* m_stream;
	direct_read_data* m_direct;

	UINT8 m_TMP_register;

	UINT8 m_global_volume;      // volume index set with the OPT command
	UINT8 m_filter_type;        // interpolation filter type set with the OPT command
	UINT8 m_output_level;       // flag stating if a voltage follower is connected

	static const int OKIM9810_VOICES = 8;
	okim_voice m_voice[OKIM9810_VOICES];

	static const UINT32 s_sampling_freq_table[16];
};


// device type definition
extern const device_type OKIM9810;



#endif // __OKIM9810_H__
