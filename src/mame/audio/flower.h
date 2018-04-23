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
	DECLARE_WRITE8_MEMBER( lower_write );
	DECLARE_WRITE8_MEMBER( upper_write );
//  virtual void lower_map(address_map &map);
//  virtual void upper_map(address_map &map);
	DECLARE_WRITE8_MEMBER( frequency_w );
	DECLARE_WRITE8_MEMBER( repeat_w );
	DECLARE_WRITE8_MEMBER( unk_w );
	DECLARE_WRITE8_MEMBER( volume_w );
	DECLARE_WRITE8_MEMBER( start_address_w );
	DECLARE_WRITE8_MEMBER( sample_trigger_w );

	void regs_map(address_map &map);
protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	//virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual space_config_vector memory_space_config() const override;
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	address_space *m_iospace;
private:

	const address_space_config m_io_space_config;
	sound_stream *m_stream;

	static constexpr unsigned MAX_VOICES = 8;
	static constexpr int defgain = 48;

	std::unique_ptr<int16_t[]> m_mixer_table;
	int16_t *m_mixer_lookup;
	std::unique_ptr<short[]> m_mixer_buffer;

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
};


// device type definition
DECLARE_DEVICE_TYPE(FLOWER_CUSTOM, flower_sound_device)


#endif // MAME_AUDIO_FLOWER_H
