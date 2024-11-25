// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_TVGAMES_ELAN_EU3A05_A_H
#define MAME_TVGAMES_ELAN_EU3A05_A_H

#include "sound/okiadpcm.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> elan_eu3a05_sound_device

class elan_eu3a05_sound_device : public device_t, public device_sound_interface, public device_memory_interface
{
public:
	elan_eu3a05_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto space_read_callback() { return m_space_read_cb.bind(); }

	template <unsigned N> auto sound_end_cb() { return m_sound_end_cb[N].bind(); }

	void map(address_map &map) ATTR_COLD;


protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;
	virtual space_config_vector memory_space_config() const override;
	const address_space_config      m_space_config;

private:
	sound_stream *m_stream;
	devcb_read8 m_space_read_cb;

	uint32_t m_sound_byte_address[6];
	uint32_t m_sound_byte_len[6];
	uint32_t m_sound_current_nib_pos[6];
	oki_adpcm_state m_adpcm[6];

	uint8_t m_sound_trigger;
	uint8_t m_sound_unk;

	uint8_t m_isstopped;

	uint8_t m_volumes[2];
	uint8_t m_50a4;
	uint8_t m_50a9;

	void handle_sound_trigger(int which);

	void handle_sound_addr_w(int which, int offset, uint8_t data);
	uint8_t handle_sound_addr_r(int which, int offset);
	void handle_sound_size_w(int which, int offset, uint8_t data);
	uint8_t handle_sound_size_r(int which, int offset);

	void elan_eu3a05_sound_addr_w(offs_t offset, uint8_t data);
	uint8_t elan_eu3a05_sound_addr_r(offs_t offset);
	void elan_eu3a05_sound_size_w(offs_t offset, uint8_t data);
	uint8_t elan_eu3a05_sound_size_r(offs_t offset);
	uint8_t elan_eu3a05_sound_trigger_r();
	void elan_eu3a05_sound_trigger_w(uint8_t data);
	uint8_t elan_eu3a05_sound_unk_r();
	void elan_eu3a05_sound_unk_w(uint8_t data);

	uint8_t elan_eu3a05_50a8_r();

	uint8_t reg50a4_r();
	void reg50a4_w(uint8_t data);
	uint8_t reg50a9_r();
	void reg50a9_w(uint8_t data);

	uint8_t elan_eu3a05_sound_volume_r(offs_t offset);
	void elan_eu3a05_sound_volume_w(offs_t offset, uint8_t data);

	void write_unmapped(offs_t offset, uint8_t data);
	uint8_t read_unmapped(offs_t offset);

	devcb_write_line::array<6> m_sound_end_cb;
};

DECLARE_DEVICE_TYPE(ELAN_EU3A05_SOUND, elan_eu3a05_sound_device)

#endif // MAME_TVGAMES_RAD_EU3A05_A_H
