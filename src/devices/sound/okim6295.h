// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni,Aaron Giles
/***************************************************************************

    okim6295.h

    OKIM 6295 ADCPM sound chip.

***************************************************************************/

#pragma once

#ifndef __OKIM6295_H__
#define __OKIM6295_H__

#include "sound/okiadpcm.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

enum
{
	OKIM6295_PIN7_LOW = 0,
	OKIM6295_PIN7_HIGH = 1
};



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_OKIM6295_ADD(_tag, _clock, _pin7) \
	MCFG_DEVICE_ADD(_tag, OKIM6295, _clock) \
	MCFG_OKIM6295_PIN7(_pin7)

#define MCFG_OKIM6295_REPLACE(_tag, _clock, _pin7) \
	MCFG_DEVICE_REPLACE(_tag, OKIM6295, _clock) \
	MCFG_OKIM6295_PIN7(_pin7)

#define MCFG_OKIM6295_PIN7(_pin7) \
	okim6295_device::static_set_pin7(*device, _pin7);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> okim6295_device

class okim6295_device : public device_t,
						public device_sound_interface,
						public device_memory_interface
{
public:
	// construction/destruction
	okim6295_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	static void static_set_pin7(device_t &device, int pin7);

	// runtime configuration
	void set_bank_base(offs_t base, bool bDontUpdateStream = false);
	void set_pin7(int pin7);

	UINT8 read_status();
	void write_command(UINT8 command);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

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
		void generate_adpcm(direct_read_data &direct, stream_sample_t *buffer, int samples);

		oki_adpcm_state m_adpcm;        // current ADPCM state
		bool            m_playing;
		offs_t          m_base_offset;  // pointer to the base memory location
		UINT32          m_sample;       // current sample number
		UINT32          m_count;        // total samples to play
		INT8            m_volume;       // output volume
	};

	// configuration state
	const address_space_config  m_space_config;
	optional_memory_region  m_region;

	// internal state
	static const int OKIM6295_VOICES = 4;

	okim_voice          m_voice[OKIM6295_VOICES];
	INT32               m_command;
	bool                m_bank_installed;
	offs_t              m_bank_offs;
	sound_stream *      m_stream;
	UINT8               m_pin7_state;
	direct_read_data *  m_direct;

	static const UINT8 s_volume_table[16];
};


// device type definition
extern const device_type OKIM6295;


#endif /* __OKIM6295_H__ */
