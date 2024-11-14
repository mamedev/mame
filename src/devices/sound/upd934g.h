// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    NEC μPD934G

    Percussion Generator

***************************************************************************/

#ifndef MAME_SOUND_UPD934G_H
#define MAME_SOUND_UPD934G_H

#pragma once

#include "dirom.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class upd934g_device : public device_t, public device_sound_interface, public device_rom_interface<16>
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	// construction/destruction
	upd934g_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	sound_stream *m_stream;

	uint16_t m_addr[16];

	struct
	{
		uint16_t pos;
		int playing;
		int effect;
	}
	m_channel[4];

	int m_sample;
	bool m_ready;
};

// device type definition
DECLARE_DEVICE_TYPE(UPD934G, upd934g_device)

#endif // MAME_SOUND_UPD934G_H
