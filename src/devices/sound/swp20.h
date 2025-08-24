// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWP20, rompler

#ifndef MAME_SOUND_SWP20_H
#define MAME_SOUND_SWP20_H

#pragma once

#include "dirom.h"

class swp20_device : public device_t, public device_sound_interface, public device_rom_interface<23+2, 1, 0, ENDIANNESS_LITTLE>
{
public:
	swp20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 11289600);

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	sound_stream *m_stream;

	std::array<u8,  0x20>  m_pan_l;
	std::array<u8,  0x20>  m_pan_r;
	std::array<u16, 0x20>  m_pitch;
	std::array<u32, 0x20>  m_sample_start;
	std::array<u16, 0x20>  m_sample_end;
	std::array<u8,  0x20>  m_sample_format;
	std::array<u32, 0x20>  m_sample_address;

	u16 m_waverom_val;
	u8 m_waverom_access;

	u8 m_eq_port;
	bool m_eq_address;
	u8 m_voice;
	u32 m_keyon;
	u32 m_keyoff;

	void voice_w(u8 data);

	void pan_l_w(u8 data);
	u8 pan_l_r();
	void pan_r_w(u8 data);
	u8 pan_r_r();
	template<int sel> void pitch_w(u8 data);
	template<int sel> u8 pitch_r();
	template<int sel> void sample_start_w(u8 data);
	template<int sel> u8 sample_start_r();
	template<int sel> void sample_end_w(u8 data);
	template<int sel> u8 sample_end_r();
	void sample_format_w(u8 data);
	u8 sample_format_r();
	template<int sel> void sample_address_w(u8 data);
	template<int sel> u8 sample_address_r();

	void waverom_access_w(u8 data);
	template<int sel> u8 waverom_val_r();

	// Generic upload port, connected to the EQ on the first swp20
	void eq_w(u8 data);

	// Generic catch-all
	u8 snd_r(offs_t offset);
	void snd_w(offs_t offset, u8 data);
};

DECLARE_DEVICE_TYPE(SWP20, swp20_device)

#endif // MAME_SOUND_SWP20_H
