// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Alex W. Jackson
/*********************************************************

    Konami 053260 KDSC

*********************************************************/

#ifndef MAME_SOUND_K053260_H
#define MAME_SOUND_K053260_H

#pragma once


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_K053260_ADD(tag, clock) \
		MCFG_DEVICE_ADD((tag), K053260, (clock))

#define MCFG_K053260_REPLACE(tag, clock) \
		MCFG_DEVICE_REPLACE((tag), K053260, (clock))

#define MCFG_K053260_REGION(tag) \
		k053260_device::set_region_tag(*device, ("^" tag));


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
		KDSC_Voice(k053260_device &device) : m_device(device), m_pan_volume{ 0, 0 } { }

		inline void voice_start(int index);
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
		k053260_device &m_device;

		// live state
		uint32_t m_position = 0;
		uint16_t m_pan_volume[2];
		uint16_t m_counter = 0;
		int8_t   m_output = 0;
		bool   m_playing = false;

		// per voice registers
		uint32_t m_start = 0;
		uint16_t m_length = 0;
		uint16_t m_pitch = 0;
		uint8_t  m_volume = 0;

		// bit packed registers
		uint8_t  m_pan = 0;
		bool   m_loop = false;
		bool   m_kadpcm = false;
	} m_voice[4];
};

DECLARE_DEVICE_TYPE(K053260, k053260_device)

#endif // MAME_SOUND_K053260_H
