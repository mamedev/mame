// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_SOUND_CT1745_H
#define MAME_SOUND_CT1745_H

#pragma once

class ct1745_mixer_device : public device_t, public device_memory_interface, public device_mixer_interface
{
public:
	ct1745_mixer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// reg $82, depends on DSP + irq stuff coming from host
	auto irq_status_cb() { return m_irq_status_cb.bind(); }

	// from DSP commands $d3/$d8 TODO: actual pin name
	void dac_speaker_off_cb(int state);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	template <typename T> void set_fm_tag(T &&tag) { m_fm.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_ldac_tag(T &&tag) { m_ldac.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_rdac_tag(T &&tag) { m_rdac.set_tag(std::forward<T>(tag)); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;

	void map(address_map &map) ATTR_COLD;

private:
	address_space_config      m_space_config;
	devcb_read8 m_irq_status_cb;

	required_device<device_sound_interface> m_fm;
	required_device<device_sound_interface> m_ldac;
	required_device<device_sound_interface> m_rdac;

	void reset_state();
	void update_gain_levels();

	u8 m_index;

	u8 m_master_level[2];
	u8 m_dac_level[2];
	u8 m_fm_level[2];
	u8 m_cd_level[2];
	u8 m_linein_level[2];
	u8 m_mic_level;
	u8 m_pc_speaker_level;
	u8 m_output_sw;
	u8 m_input_sw[2];
	u8 m_input_gain[2];
	u8 m_output_gain[2];
	// Automatic Gain Control
	bool m_agc;
	u8 m_treble[2];
	u8 m_bass[2];

	bool m_dac_speaker_off;
};

DECLARE_DEVICE_TYPE(CT1745, ct1745_mixer_device)

#endif // MAME_SOUND_CT1745_H
