// license:BSD-3-Clause
// copyright-holders:cam900, Brad Smith, Brezza
/***************************************************************************

    Ricoh RP2C33 Sound emulation

***************************************************************************/

#ifndef MAME_SOUND_RP2C33_SND_H
#define MAME_SOUND_RP2C33_SND_H

#pragma once

#include <algorithm>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> rp2c33_sound_device

class rp2c33_sound_device : public device_t, public device_sound_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; } // one or more features are not verified, and possibly incorrect

	// construction/destruction
	rp2c33_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// host interface
	void write(offs_t offset, u8 data);
	u8 read(offs_t offset);

	// wavetable handlers
	void wave_w(offs_t offset, u8 data) { if (m_wave_write) { m_stream->update(); m_wave[offset & 0x3f] = data & 0x3f; } }
	// with writes disabled ($4089.7 = 0) a read anywhere in $4040-$407F gives the sample at the current wave position (NESdev FDS audio)
	u8 wave_r(offs_t offset)
	{
		if (!m_wave_write)
			m_stream->update();
		return (m_wave[(m_wave_write ? offset : m_wave_addr) & 0x3f] & 0x3f) | 0x40; // TODO: bit 6-7 is open bus?
	}

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	sound_stream *m_stream = nullptr;

	// sound states
	u8 m_regs[16];
	u8 m_wave[64];                  // 64 entries of wavetable, 6 bit unsigned

	bool m_vol_env_disable = true;  // disable volume envelope
	bool m_vol_env_mode    = false; // volume envelope direction, increase or decrease
	int m_vol_env_spd      = 0;     // volume envelope speed/gain
	u32 m_vol_env_clk      = 0;     // volume envelope clock counter
	int m_vol_env_out      = 0;     // output result of volume envelope
	bool m_wave_halt       = true;  // halt wavetable
	bool m_env_halt        = true;  // halt envelope
	int m_wave_freq        = 0;     // wavetable frequency
	u32 m_wave_acc         = 0;     // wavetable accumulator (.16 fixed point)
	u8 m_wave_addr         = 0;     // wavetable address counter (.16 fixed point)

	u8 m_mod_table[32];             // 32 entries of modulator table, 3 bit unsigned
	bool m_mod_env_disable = true;  // disable modulator envelope
	bool m_mod_env_mode    = false; // modulator envelope direction, increase or decrease
	int m_mod_env_spd      = 0;     // modulator envelope speed/gain
	u32 m_mod_env_clk      = 0;     // modulator envelope clock counter
	int m_mod_env_out      = 0;     // output result of modulator envelope
	bool m_mod_halt        = true;  // halt modulator
	bool m_mod_force       = false; // $4087 bit 6: a carry out of the accumulator on every tick of the unit, every 16 clocks
	int m_mod_tick         = 0;     // clocks since the last forced tick
	int m_mod_freq         = 0;     // modulator frequency
	u32 m_mod_acc          = 0;     // modulator accumulator (.16 fixed point)
	u8 m_mod_addr          = 0;     // modulator table address counter (.16 fixed point)
	int m_mod_pos          = 0;     // modulator position, 7 bit signed

	int m_env_spd          = 0;     // overall envelope speed
	bool m_wave_write      = true;  // play sound or write wavetable
	int m_mvol             = 0;     // master volume
	int m_output           = 0;     // output result
	int m_vol_latch         = 0;     // the volume gain the output uses: it follows the envelope while the wave position is 0
	float m_lp              = 0;     // the output filter's state
	float m_lp_coeff        = 0;     // and its coefficient for the clock
	int m_mvol_table[4];            // master volume table

	const int mod_inc[8] = { 0, 1, 2, 4, -4, -4, -2, -1 };

	// inlines
	inline void exec_vol_env();
	inline void exec_mod_env();

	inline void exec_mod();
	inline void exec_wave();
};


// device type definition
DECLARE_DEVICE_TYPE(RP2C33_SOUND, rp2c33_sound_device)

#endif // MAME_SOUND_RP2C33_SND_H
