// license:GPL-2.0+
// copyright-holders:Peter Trauner
#ifndef MAME_AUDIO_ARCADIA_H
#define MAME_AUDIO_ARCADIA_H

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> arcadia_sound_device

class arcadia_sound_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	arcadia_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	sound_stream *m_channel;
	uint8_t m_reg[3];
	int m_size, m_pos,m_tval,m_nval;
	unsigned m_mode, m_omode;
	unsigned m_volume;
	unsigned m_lfsr;
};

// device type definition
DECLARE_DEVICE_TYPE(ARCADIA_SOUND, arcadia_sound_device)

#endif // MAME_AUDIO_ARCADIA_H
