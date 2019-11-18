// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    NEC μPD934G

    Percussion Generator

***************************************************************************/

#ifndef MAME_SOUND_UPD934G_H
#define MAME_SOUND_UPD934G_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class upd934g_device : public device_t, public device_sound_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	// construction/destruction
	upd934g_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	auto data_callback() { return m_data_cb.bind(); }

	DECLARE_WRITE8_MEMBER(write);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	devcb_read8 m_data_cb;
	sound_stream *m_stream;

	uint16_t m_addr[16];

	struct
	{
		uint16_t pos;
		int playing;
		int volume;
	}
	m_channel[4];

	int m_sample;
	bool m_ready;
};

// device type definition
DECLARE_DEVICE_TYPE(UPD934G, upd934g_device)

#endif // MAME_SOUND_UPD934G_H
