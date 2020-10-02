// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Flower custom sound chip

***************************************************************************/

#ifndef MAME_AUDIO_FLOWER_H
#define MAME_AUDIO_FLOWER_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> flower_sound_device

class flower_sound_device : public device_t,
							public device_sound_interface,
							public device_memory_interface
{
public:
	// construction/destruction
	flower_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void lower_write(offs_t offset, uint8_t data);
	void upper_write(offs_t offset, uint8_t data);
//  virtual void lower_map(address_map &map);
//  virtual void upper_map(address_map &map);

	void regs_map(address_map &map);
protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	//virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual space_config_vector memory_space_config() const override;
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	address_space *m_iospace;
private:

	const address_space_config m_io_space_config;
	sound_stream *m_stream;

	static constexpr unsigned MAX_VOICES = 8;
	static constexpr int defgain = 48;

	std::vector<int16_t> m_mixer_table;
	int16_t *m_mixer_lookup;
	std::vector<short> m_mixer_buffer;

	struct fl_sound_channel
	{
		uint8_t start_nibbles[6];
		uint8_t raw_frequency[4];
		uint32_t start_address;
		uint32_t position;
		uint16_t frequency;
		uint8_t volume;
		uint8_t volume_bank;
		uint8_t effect;
		bool enable;
		bool repeat;
		int channel_number;
	};

	/* data about the sound system */
	fl_sound_channel m_channel_list[MAX_VOICES];
	fl_sound_channel *m_last_channel;

	void make_mixer_table(int voices, int gain);

	const uint8_t *m_sample_rom;
	const uint8_t *m_volume_rom;

	void frequency_w(offs_t offset, uint8_t data);
	void repeat_w(offs_t offset, uint8_t data);
	void unk_w(offs_t offset, uint8_t data);
	void volume_w(offs_t offset, uint8_t data);
	void start_address_w(offs_t offset, uint8_t data);
	void sample_trigger_w(offs_t offset, uint8_t data);
};


// device type definition
DECLARE_DEVICE_TYPE(FLOWER_CUSTOM, flower_sound_device)


#endif // MAME_AUDIO_FLOWER_H
