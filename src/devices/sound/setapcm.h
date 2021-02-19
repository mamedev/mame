// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
#ifndef MAME_SOUND_SETAPCM_H
#define MAME_SOUND_SETAPCM_H

#pragma once

#include "dirom.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> setapcm_device

// TODO: unknown address bus width
class setapcm_device : public device_t,
					public device_sound_interface,
					public device_rom_interface<32>
{
public:
	void snd_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 snd_r(offs_t offset);
	void sndctrl_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 sndctrl_r();

protected:
	setapcm_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// device_rom_interface implementation
	virtual void rom_bank_updated() override;

	int m_max_voices = 16; // max voices
private:
	struct voice_t
	{
		u32 m_start = 0;        // Start position
		u16 m_flags = 0;        // Flags (Bit 0 = loop)
		u16 m_freq = 0;         // Frequency (4.12 fixed point)
		u32 m_lpstart = 0;      // Loop start position
		u32 m_lpend = 0;        // Loop end position
		u32 m_end = 0;          // End position
		s32 m_vol_r = 0;        // Right volume
		s32 m_vol_l = 0;        // Left volume
		u32 m_pos = 0;          // Current position
		u32 m_frac = 0;         // Position fraction
		bool m_lponce = false;  // Is looped once?
		bool m_keyon = false;   // Keyon
	};

	sound_stream *m_stream;

	voice_t m_voice[16];
	u16 m_sound_regs[0x100];
	u16 m_ctrl;
};

class nile_sound_device : public setapcm_device
{
public:
	nile_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class st0032_sound_device : public setapcm_device
{
public:
	st0032_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(NILE_SOUND,   nile_sound_device)
DECLARE_DEVICE_TYPE(ST0032_SOUND, st0032_sound_device)

#endif // MAME_SOUND_SETAPCM_H
