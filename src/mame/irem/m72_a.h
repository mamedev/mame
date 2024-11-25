// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    M72 audio interface

****************************************************************************/
#ifndef MAME_IREM_M72_A_H
#define MAME_IREM_M72_A_H

#pragma once

#include "sound/dac.h"

#include "dirom.h"


class m72_audio_device : public device_t, public device_rom_interface<32> // unknown address bits
{
public:
	m72_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	~m72_audio_device() {}

	template <typename T> void set_dac_tag(T &&tag) { m_dac.set_tag(std::forward<T>(tag)); }
	u8 sample_r();
	void sample_w(u8 data);

	/* the port goes to different address bits depending on the game */
	void set_sample_start(int start);
	void vigilant_sample_addr_w(offs_t offset, u8 data);
	void shisen_sample_addr_w(offs_t offset, u8 data);
	void rtype2_sample_addr_w(offs_t offset, u8 data);
	void poundfor_sample_addr_w(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	// internal state
	uint32_t m_sample_addr;
	optional_device<dac_byte_interface> m_dac;
};

DECLARE_DEVICE_TYPE(IREM_M72_AUDIO, m72_audio_device)

#endif // MAME_IREM_M72_A_H
