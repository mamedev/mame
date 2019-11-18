// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWP00, rompler/dsp combo

#ifndef DEVICES_SOUND_SWP00_H
#define DEVICES_SOUND_SWP00_H

#pragma once

#include "meg.h"

class swp00_device : public device_t, public device_sound_interface, public device_rom_interface
{
public:
	swp00_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 33868800);

	void map(address_map &map);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
	virtual void rom_bank_updated() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	//  required_device<meg_embedded_device> m_meg;

	sound_stream *m_stream;

	// Generic catch-all
	u8 snd_r(offs_t offset);
	void snd_w(offs_t offset, u8 data);

	inline auto &rchan(address_map &map, int idx) {
		return map(idx*2, idx*2+1).select(0x7c0);
	}

	inline auto &rctrl(address_map &map, int idx) {
		int slot = 0x20*(idx >> 1) | 0xe | (idx & 1);
		return map(slot*2, slot*2+1);
	}
};

DECLARE_DEVICE_TYPE(SWP00, swp00_device)

#endif
