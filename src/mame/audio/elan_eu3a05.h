// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_AUDIO_ELAN_EU3A05_H
#define MAME_AUDIO_ELAN_EU3A05_H


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> elan_eu3a05_sound_device

class elan_eu3a05_sound_device : public device_t, public device_sound_interface
{
public:
	elan_eu3a05_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto space_read_callback() { return m_space_read_cb.bind(); }

	template <unsigned N> auto sound_end_cb() { return m_sound_end_cb[N].bind(); }
	
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ8_MEMBER(read);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	sound_stream *m_stream;
	devcb_read8 m_space_read_cb;

	uint32_t m_sound_byte_address[6];
	uint32_t m_sound_byte_len[6];
	uint32_t m_sound_current_nib_pos[6];

	uint8_t m_sound_trigger;
	uint8_t m_sound_unk;

	uint8_t m_isstopped;

	uint8_t m_volumes[2];
	uint8_t m_5024;
	uint8_t m_50a9;

	void handle_sound_trigger(int which);

	void handle_sound_addr_w(int which, int offset, uint8_t data);
	uint8_t handle_sound_addr_r(int which, int offset);
	void handle_sound_size_w(int which, int offset, uint8_t data);
	uint8_t handle_sound_size_r(int which, int offset);

	DECLARE_WRITE8_MEMBER(elan_eu3a05_sound_addr_w);
	DECLARE_READ8_MEMBER(elan_eu3a05_sound_addr_r);
	DECLARE_WRITE8_MEMBER(elan_eu3a05_sound_size_w);
	DECLARE_READ8_MEMBER(elan_eu3a05_sound_size_r);
	DECLARE_READ8_MEMBER(elan_eu3a05_sound_trigger_r);
	DECLARE_WRITE8_MEMBER(elan_eu3a05_sound_trigger_w);
	DECLARE_READ8_MEMBER(elan_eu3a05_sound_unk_r);
	DECLARE_WRITE8_MEMBER(elan_eu3a05_sound_unk_w);

	DECLARE_READ8_MEMBER(elan_eu3a05_50a8_r);

	DECLARE_READ8_MEMBER(elan_eu3a05_sound_volume_r);
	DECLARE_WRITE8_MEMBER(elan_eu3a05_sound_volume_w);

	devcb_write_line m_sound_end_cb[6];
};

DECLARE_DEVICE_TYPE(ELAN_EU3A05_SOUND, elan_eu3a05_sound_device)

#endif // MAME_AUDIO_RAD_EU3A05_H
