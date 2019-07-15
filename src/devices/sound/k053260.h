// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Alex W. Jackson
/*********************************************************

    Konami 053260 KDSC

*********************************************************/

#ifndef MAME_SOUND_K053260_H
#define MAME_SOUND_K053260_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> k053260_device

class k053260_device : public device_t,
						public device_sound_interface,
						public device_rom_interface
{
public:
	k053260_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u8 main_read(offs_t offset);
	void main_write(offs_t offset, u8 data);
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_clock_changed() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// device_rom_interface overrides
	virtual void rom_bank_updated() override;

private:
	// configuration
	sound_stream *  m_stream;

	// live state
	u8           m_portdata[4];
	u8           m_keyon;
	u8           m_mode;

	// per voice state
	class KDSC_Voice
	{
	public:
		KDSC_Voice(k053260_device &device) : m_device(device), m_pan_volume{ 0, 0 } { }

		inline void voice_start(int index);
		inline void voice_reset();
		inline void set_register(offs_t offset, u8 data);
		inline void set_loop_kadpcm(u8 data);
		inline void set_pan(u8 data);
		inline void update_pan_volume();
		inline void key_on();
		inline void key_off();
		inline void play(stream_sample_t *outputs);
		inline bool playing() { return m_playing; }
		inline u8 read_rom();

	private:
		// pointer to owning device
		k053260_device &m_device;

		// live state
		u32  m_position = 0;
		u16  m_pan_volume[2];
		u16  m_counter = 0;
		s8   m_output = 0;
		bool m_playing = false;

		// per voice registers
		u32 m_start = 0;
		u16 m_length = 0;
		u16 m_pitch = 0;
		u8  m_volume = 0;

		// bit packed registers
		u8   m_pan = 0;
		bool m_loop = false;
		bool m_kadpcm = false;
	} m_voice[4];
};

DECLARE_DEVICE_TYPE(K053260, k053260_device)

#endif // MAME_SOUND_K053260_H
