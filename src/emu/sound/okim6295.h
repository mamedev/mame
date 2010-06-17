/***************************************************************************

    okim6295.h

    OKIM 6295 ADCPM sound chip.

***************************************************************************/

#pragma once

#ifndef __OKIM6295_H__
#define __OKIM6295_H__

#include "streams.h"



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

#define MDRV_OKIM6295_ADD(_tag, _clock, _pin7) \
	MDRV_DEVICE_ADD(_tag, SOUND_OKIM6295, _clock) \
	MDRV_OKIM6295_PIN7(_pin7)

#define MDRV_OKIM6295_REPLACE(_tag, _clock, _pin7) \
	MDRV_DEVICE_REPLACE(_tag, SOUND_OKIM6295, _clock) \
	MDRV_OKIM6295_PIN7(_pin7)

#define MDRV_OKIM6295_PIN7(_pin7) \
	MDRV_DEVICE_INLINE_DATA16(okim6295_device_config::INLINE_PIN7, _pin7)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> adpcm_state

// Internal ADPCM state, used by external ADPCM generators with compatible specs to the OKIM 6295.
class adpcm_state
{
public:
	adpcm_state() { compute_tables(); reset(); }

	void reset();
	INT16 clock(UINT8 nibble);

	INT32	m_signal;
	INT32	m_step;

private:
	static const INT8 s_index_shift[8];
	static int s_diff_lookup[49*16];

	static void compute_tables();
	static bool s_tables_computed;
};



// ======================> okim6295_device_config

class okim6295_device_config :	public device_config,
								public device_config_sound_interface,
								public device_config_memory_interface
{
	friend class okim6295_device;

	// construction/destruction
	okim6295_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
	// allocators
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;

	// basic information getters
	virtual const char *name() const { return "OKI6295"; }

	// inline configuration indexes
	enum
	{
		INLINE_PIN7
	};

protected:
	// device_config overrides
	virtual void device_config_complete();
	virtual const address_space_config *memory_space_config(int spacenum = 0) const;

	// internal state
	const address_space_config  m_space_config;
	UINT8						m_pin7;
};



// ======================> okim6295_device

class okim6295_device : public device_t,
						public device_sound_interface,
						public device_memory_interface
{
	friend class okim6295_device_config;

	// construction/destruction
	okim6295_device(running_machine &_machine, const okim6295_device_config &config);

public:
	void set_bank_base(offs_t base);
	void set_pin7(int pin7);

	UINT8 status_read();
	void data_write(UINT8 data);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_post_load();
	virtual void device_clock_changed();

	// internal callbacks
	static STREAM_UPDATE( static_stream_generate );
	virtual void stream_generate(stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	// a single voice
	class okim_voice
	{
	public:
		okim_voice();
		void generate_adpcm(const address_space *space, stream_sample_t *buffer, int samples);

		adpcm_state m_adpcm;			// current ADPCM state
		bool		m_playing;
		offs_t		m_base_offset;		// pointer to the base memory location
		UINT32		m_sample;			// current sample number
		UINT32		m_count;			// total samples to play
		INT8		m_volume;			// output volume
	};

	// internal state
	static const int OKIM6295_VOICES = 4;

	const okim6295_device_config &m_config;

	okim_voice		m_voice[OKIM6295_VOICES];
	INT32			m_command;
	bool			m_bank_installed;
	offs_t			m_bank_offs;
	sound_stream *	m_stream;
	UINT8			m_pin7_state;

	static const UINT8 s_volume_table[16];
};


// device type definition
const device_type SOUND_OKIM6295 = okim6295_device_config::static_alloc_device_config;



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ8_DEVICE_HANDLER( okim6295_r );
WRITE8_DEVICE_HANDLER( okim6295_w );


#endif /* __OKIM6295_H__ */
