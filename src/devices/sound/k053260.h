// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Alex W. Jackson
/*********************************************************

    Konami 053260 KDSC

*********************************************************/

#pragma once

#ifndef __K053260_H__
#define __K053260_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_K053260_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, K053260, _clock)
#define MCFG_K053260_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, K053260, _clock)

#define MCFG_K053260_REGION(_tag) \
	k053260_device::set_region_tag(*device, _tag);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> k053260_device

class k053260_device : public device_t,
						public device_sound_interface
{
public:
	k053260_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~k053260_device() { }

	static void set_region_tag(device_t &device, std::string tag) { downcast<k053260_device &>(device).m_rgnoverride = tag; }

	DECLARE_READ8_MEMBER( main_read );
	DECLARE_WRITE8_MEMBER( main_write );
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// configuration
	std::string     m_rgnoverride;

	sound_stream *  m_stream;
	UINT8 *         m_rom;
	UINT32          m_rom_size;

	// live state
	UINT8           m_portdata[4];
	UINT8           m_keyon;
	UINT8           m_mode;

	// per voice state
	class KDSC_Voice
	{
	public:
		KDSC_Voice() : m_device(nullptr), m_position(0), m_counter(0), m_output(0), m_playing(false), m_start(0), m_length(0), m_pitch(0), m_volume(0), m_pan(0), m_loop(false), m_kadpcm(false)
		{
		}

		inline void voice_start(k053260_device &device, int index);
		inline void voice_reset();
		inline void set_register(offs_t offset, UINT8 data);
		inline void set_loop_kadpcm(UINT8 data);
		inline void set_pan(UINT8 data);
		inline void update_pan_volume();
		inline void key_on();
		inline void key_off();
		inline void play(stream_sample_t *outputs);
		inline bool playing() { return m_playing; }
		inline UINT8 read_rom();

	private:
		// pointer to owning device
		k053260_device *m_device;

		// live state
		UINT32 m_position;
		UINT16 m_pan_volume[2];
		UINT16 m_counter;
		INT8   m_output;
		bool   m_playing;

		// per voice registers
		UINT32 m_start;
		UINT16 m_length;
		UINT16 m_pitch;
		UINT8  m_volume;

		// bit packed registers
		UINT8  m_pan;
		bool   m_loop;
		bool   m_kadpcm;
	} m_voice[4];

	friend class k053260_device::KDSC_Voice;
};

extern const device_type K053260;

#endif /* __K053260_H__ */
