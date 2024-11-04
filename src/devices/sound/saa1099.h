// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, Manuel Abadia
/**********************************************
    Philips SAA1099 Sound driver
**********************************************/

#ifndef MAME_SOUND_SAA1099_H
#define MAME_SOUND_SAA1099_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> saa1099_device

class saa1099_device : public device_t,
						public device_sound_interface
{
public:
	saa1099_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void control_w(u8 data);
	void data_w(u8 data);

	void write(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	struct saa1099_channel
	{
		saa1099_channel() : amplitude{ 0, 0 }, envelope{ 0, 0 } { }

		u8 frequency      = 0;      // frequency (0x00..0xff)
		bool freq_enable  = false;  // frequency enable
		bool noise_enable = false;  // noise enable
		u8 octave         = 0;      // octave (0x00..0x07)
		u16 amplitude[2];           // amplitude
		u8 envelope[2];             // envelope (0x00..0x0f or 0x10 == off)

		/* vars to simulate the square wave */
		inline u32 freq() const { return (511 - frequency) << (8 - octave); } // clock / ((511 - frequency) * 2^(8 - octave))
		int counter = 0;
		u8 level = 0;
	};

	struct saa1099_noise
	{
		saa1099_noise() { }

		/* vars to simulate the noise generator output */
		int counter = 0;
		int freq = 0;
		u32 level = 0xffffffffU;    // noise polynomial shifter
	};

	void envelope_w(int ch);

	sound_stream *m_stream;         // our stream
	u8 m_noise_params[2];           // noise generators parameters
	bool m_env_enable[2];           // envelope generators enable
	bool m_env_reverse_right[2];    // envelope reversed for right channel
	u8 m_env_mode[2];               // envelope generators mode
	bool m_env_bits[2];             // true = 3 bits resolution
	bool m_env_clock[2];            // envelope clock mode (true external)
	u8 m_env_step[2];               // current envelope step
	bool m_all_ch_enable;           // all channels enable
	bool m_sync_state;              // sync all channels
	u8 m_selected_reg;              // selected register
	saa1099_channel m_channels[6];  // channels
	saa1099_noise m_noise[2];       // noise generators
};

DECLARE_DEVICE_TYPE(SAA1099, saa1099_device)

#endif // MAME_SOUND_SAA1099_H
