// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/***************************************************************************

    okim9810.h

    OKI MSM9810 ADPCM(2) sound chip.

    Notes:
    The master clock frequency for this chip can range from 3.5MHz to 4.5Mhz.
      The typical oscillator is a 4.096Mhz crystal.

***************************************************************************/

#ifndef MAME_SOUND_OKIM9810_H
#define MAME_SOUND_OKIM9810_H

#pragma once

#include "dirom.h"
#include "okiadpcm.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> okim9810_device

class okim9810_device : public device_t,
						public device_sound_interface,
						public device_rom_interface<24, 0, 0, ENDIANNESS_BIG>
{
public:
	// construction/destruction
	okim9810_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read_status();
	void write_tmp_register(uint8_t command);
	void write_command(uint8_t command);

	uint8_t read();
	void write(uint8_t data);
	void tmp_register_w(uint8_t data);

	// serial read/write handlers
	void serial_w(int state);
	void si_w(int state);
	void sd_w(int state);
	void ud_w(int state);
	void cmd_w(int state);
	int so_r();
	int sr0_r();
	int sr1_r();
	int sr2_r();
	int sr3_r();

protected:
	enum
	{
		ADPCM_PLAYBACK = 0,
		ADPCM2_PLAYBACK = 1,
		NONLINEAR8_PLAYBACK = 2,
		STRAIGHT8_PLAYBACK = 3,
		EIGHTBIT_PLAYBACK = 2
	};

	enum
	{
		SECONDARY_FILTER = 0,
		PRIMARY_FILTER = 1,
		NO_FILTER = 2,
		NO_FILTER2 = 3
	};

	enum
	{
		SEQ_STOP = 0,
		SEQ_ACTIVE = 1,
		SEQ_PLAY = 2,
		SEQ_PAUSE = 3
	};

	enum
	{
		OUTPUT_TO_DIRECT_DAC = 0,
		OUTPUT_TO_VOLTAGE_FOLLOWER = 1
	};

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;
	virtual void device_clock_changed() override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// device_rom_interface overrides
	virtual void rom_bank_pre_change() override;

	// a single voice
	class okim_voice
	{
	public:
		okim_voice();
		void generate_audio(device_rom_interface &rom,
							std::vector<write_stream_view> &buffers,
							const uint8_t global_volume,
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
		uint32_t  m_phrase_offset;
		int32_t   m_phrase_count;
		int32_t   m_phrase_wait_cnt;
		uint8_t   m_phrase_state;

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

	int       m_dadr;
	offs_t    m_dadr_start_offset;
	offs_t    m_dadr_end_offset;
	uint8_t   m_dadr_flags;

	int       m_serial;
	int       m_serial_read_latch;
	int       m_serial_write_latch;
	int       m_serial_bits;
	int       m_ud;
	int       m_si;
	int       m_sd;
	int       m_cmd;

	static constexpr int OKIM9810_VOICES = 8;
	okim_voice m_voice[OKIM9810_VOICES];

	static const uint32_t s_sampling_freq_div_table[16];
};


// device type definition
DECLARE_DEVICE_TYPE(OKIM9810, okim9810_device)

#endif // MAME_SOUND_OKIM9810_H
