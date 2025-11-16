// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_SOUND_L4003_H
#define MAME_SOUND_L4003_H
#pragma once
class l4003_sound_device : public device_t,
							 public device_sound_interface,
							 public device_memory_interface
{
public:
	l4003_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void map(address_map &map);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	static const int NUM_VOICES = 16;

	struct l4003_voice
	{
		constexpr l4003_voice() {}

		uint32_t start = 0;
		uint32_t loop_start = 0;
		uint32_t pos = 0;
		uint32_t frac = 0;
		uint32_t step = 0;
		uint16_t env_volume = 0;
		uint8_t volume = 0;
		uint8_t send_level = 0;
		uint8_t send_dest = 0;
	};

	sound_stream    *m_stream;
	address_space_config m_mem_config;
	l4003_voice m_voices[NUM_VOICES];

	memory_access<21, 1, 0, ENDIANNESS_LITTLE>::cache m_cache;

	uint16_t m_data, m_control, m_key_status;
	uint32_t m_sample_rate;

	uint16_t data_r();
	void data_w(uint16_t data);
	uint16_t control_r();
	void control_w(uint16_t data);
};

DECLARE_DEVICE_TYPE(L4003, l4003_sound_device)

#endif // MAME_SOUND_L4003_H
