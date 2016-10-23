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
	k053260_device::set_region_tag(*device, "^" _tag);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> k053260_device

class k053260_device : public device_t,
						public device_sound_interface,
						public device_rom_interface
{
public:
	k053260_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~k053260_device() { }

	uint8_t main_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void main_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// device_rom_interface overrides
	virtual void rom_bank_updated() override;

private:
	// configuration
	sound_stream *  m_stream;

	// live state
	uint8_t           m_portdata[4];
	uint8_t           m_keyon;
	uint8_t           m_mode;

	// per voice state
	class KDSC_Voice
	{
	public:
		KDSC_Voice() : m_device(nullptr), m_position(0), m_counter(0), m_output(0), m_playing(false), m_start(0), m_length(0), m_pitch(0), m_volume(0), m_pan(0), m_loop(false), m_kadpcm(false)
		{
		}

		inline void voice_start(k053260_device &device, int index);
		inline void voice_reset();
		inline void set_register(offs_t offset, uint8_t data);
		inline void set_loop_kadpcm(uint8_t data);
		inline void set_pan(uint8_t data);
		inline void update_pan_volume();
		inline void key_on();
		inline void key_off();
		inline void play(stream_sample_t *outputs);
		inline bool playing() { return m_playing; }
		inline uint8_t read_rom();

	private:
		// pointer to owning device
		k053260_device *m_device;

		// live state
		uint32_t m_position;
		uint16_t m_pan_volume[2];
		uint16_t m_counter;
		int8_t   m_output;
		bool   m_playing;

		// per voice registers
		uint32_t m_start;
		uint16_t m_length;
		uint16_t m_pitch;
		uint8_t  m_volume;

		// bit packed registers
		uint8_t  m_pan;
		bool   m_loop;
		bool   m_kadpcm;
	} m_voice[4];

	friend class k053260_device::KDSC_Voice;
};

extern const device_type K053260;

#endif /* __K053260_H__ */
