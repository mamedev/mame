// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina,cam900
#ifndef MAME_SOUND_SETAPCM_H
#define MAME_SOUND_SETAPCM_H

#pragma once

#include "dirom.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(NILE_SOUND,   nile_sound_device)
DECLARE_DEVICE_TYPE(ST0032_SOUND, st0032_sound_device)

// ======================> setapcm_device

// TODO: unknown address bus width
template<unsigned MaxVoices, unsigned Divider>
class setapcm_device : public device_t,
					public device_sound_interface,
					public device_rom_interface<32>
{
public:
	void snd_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 snd_r(offs_t offset);

	void key_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 key_r();

protected:
	setapcm_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_rom_interface implementation
	virtual void rom_bank_pre_change() override;

	static constexpr unsigned MAX_VOICES = MaxVoices;  // max voices
	static constexpr unsigned CLOCK_DIVIDER = Divider; // clock divider for generate output rate
private:
	struct voice_t
	{
		bool update();
		void keyon();
		void keyoff();

		u16 reg_r(offs_t reg);
		void reg_w(offs_t reg, u16 data, u16 mem_mask = ~0);

		device_rom_interface<32> *m_host; // host device
		u32 m_start = 0;                  // Start position
		u16 m_flags = 0;                  // Flags (Bit 0 = loop)
		u16 m_freq = 0;                   // Frequency (4.12 fixed point)
		u32 m_lpstart = 0;                // Loop start position
		u32 m_lpend = 0;                  // Loop end position
		u32 m_end = 0;                    // End position
		s32 m_vol_r = 0;                  // Right volume
		s32 m_vol_l = 0;                  // Left volume
		u32 m_pos = 0;                    // Current position
		u32 m_frac = 0;                   // Position fraction
		bool m_lponce = false;            // Is looped once?
		bool m_keyon = false;             // Keyon status
		s32 m_out = 0;                    // output value
	};

	sound_stream *m_stream;

	voice_t m_voice[MaxVoices];           // 8 or 16 Voice engines
	u16 m_keyctrl;                        // Key on/off control bit
};

// ======================> nile_sound_device

class nile_sound_device : public setapcm_device<8, 160>
{
public:
	nile_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> st0032_sound_device

class st0032_sound_device : public setapcm_device<16, 384>
{
public:
	st0032_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

//**************************************************************************
//  EXTERNAL TEMPLATE INSTANTIATIONS
//**************************************************************************

extern template class setapcm_device<8, 160>;
extern template class setapcm_device<16, 384>;

#endif // MAME_SOUND_SETAPCM_H
