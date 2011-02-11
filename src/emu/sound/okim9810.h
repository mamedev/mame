/***************************************************************************

    okim9810.h

    OKI MSM9810 ADCPM(2) sound chip.

***************************************************************************/

#pragma once

#ifndef __OKIM9810_H__
#define __OKIM9810_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

enum
{
	OKIM9810_SERIAL_PIN_LOW = 0,
	OKIM9810_SERIAL_PIN_HIGH = 1,
    // etc
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

// ======================> adpcm_stateCopy

// Internal ADPCM state, used by external ADPCM generators with compatible specs to the OKIM 6295.
class adpcm_stateCopy
{
public:
	adpcm_stateCopy() { compute_tables(); reset(); }

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


// ======================> okim9810_device_config

class okim9810_device_config :	public device_config,
								public device_config_sound_interface,
								public device_config_memory_interface
{
	friend class okim9810_device;

	// construction/destruction
	okim9810_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
	// allocators
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;

protected:
	// device_config overrides
	virtual const address_space_config *memory_space_config(int spacenum = 0) const;

	// internal state
	const address_space_config  m_space_config;
};



// ======================> okim9810_device

class okim9810_device : public device_t,
						public device_sound_interface,
						public device_memory_interface
{
	friend class okim9810_device_config;

	// construction/destruction
	okim9810_device(running_machine &_machine, const okim9810_device_config &config);

public:

	UINT8 read_status();
	void write_TMP_register(UINT8 command);
	void write_command(UINT8 command);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE8_MEMBER( write_TMP_register );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_post_load();
	virtual void device_clock_changed();

	// sound interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

    // a single voice
	class okim_voice
	{
	public:
		okim_voice();
		void generate_adpcm(direct_read_data &direct, stream_sample_t *buffer, int samples);

		adpcm_stateCopy m_adpcm;			// current ADPCM state
		bool		m_playing;
		bool		m_looping;
		UINT8		m_startFlags;
		UINT8		m_endFlags;
		offs_t		m_base_offset;		// pointer to the base memory location
		UINT32		m_sample;			// current sample number
		UINT32		m_count;			// total bytes to play

		INT8		m_volume;			// output volume
		// TODO:	m_volume_left;      // stereo volume
		// TODO:	m_volume_right;     // stereo volume
	};


	// internal state
	const okim9810_device_config &m_config;

	sound_stream* m_stream;
	direct_read_data* m_direct;

    UINT8 m_TMP_register;

	static const int OKIM9810_VOICES = 8;
	okim_voice m_voice[OKIM9810_VOICES];

	static const UINT8 s_volume_table[16];
};


// device type definition
extern const device_type OKIM9810;



#endif // __OKIM9810_H__
