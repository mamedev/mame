// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_SOUND_N63701X_H
#define MAME_SOUND_N63701X_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> namco_63701x_device

class namco_63701x_device : public device_t,
							public device_sound_interface
{
public:
	namco_63701x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	struct voice_63701x
	{
		voice_63701x() { }

		int select          = 0;
		int playing         = 0;
		int base_addr       = 0;
		int position        = 0;
		int volume          = 0;
		int silence_counter = 0;
	};

	required_region_ptr<uint8_t> m_rom;
	voice_63701x m_voices[2];
	sound_stream *m_stream; /* channel assigned by the mixer */
};

DECLARE_DEVICE_TYPE(NAMCO_63701X, namco_63701x_device)

#endif // MAME_SOUND_N63701X_H
