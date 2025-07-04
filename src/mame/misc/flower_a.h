// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Flower custom sound chip

***************************************************************************/

#ifndef MAME_MISC_FLOWER_A_H
#define MAME_MISC_FLOWER_A_H

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
	flower_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// I/O operations
	void lower_write(offs_t offset, u8 data);
	void upper_write(offs_t offset, u8 data);
//  virtual void lower_map(address_map &map) ATTR_COLD;
//  virtual void upper_map(address_map &map) ATTR_COLD;

	void regs_map(address_map &map) ATTR_COLD;
protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	//virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;
	virtual void sound_stream_update(sound_stream &stream) override;

	address_space *m_iospace = nullptr;
private:

	const address_space_config m_io_space_config;
	sound_stream *m_stream;

	static constexpr unsigned MAX_VOICES = 8;
	static constexpr int defgain = 48;

	u8 m_io_regs[0x80] = {0}; // for debug purpose

	std::vector<s16> m_mixer_table;
	s16 *m_mixer_lookup;
	std::vector<short> m_mixer_buffer;

	struct fl_sound_channel
	{
		u8 start_nibbles[6] = {0};
		u8 raw_frequency[4] = {0};
		u32 start_address = 0;
		u32 position = 0;
		u16 frequency = 0;
		u8 volume = 0;
		u8 volume_bank = 0;
		//u8 effect = 0;
		bool enable = false;
		bool repeat = false;
	};

	/* data about the sound system */
	fl_sound_channel m_channel_list[MAX_VOICES];

	void make_mixer_table(int voices, int gain);

	required_region_ptr<u8> m_sample_rom;
	required_region_ptr<u8> m_volume_rom;

	void frequency_w(offs_t offset, u8 data);
	void repeat_w(offs_t offset, u8 data);
	void unk_w(offs_t offset, u8 data);
	void volume_w(offs_t offset, u8 data);
	void start_address_w(offs_t offset, u8 data);
	void sample_trigger_w(offs_t offset, u8 data);
};


// device type definition
DECLARE_DEVICE_TYPE(FLOWER_CUSTOM, flower_sound_device)


#endif // MAME_MISC_FLOWER_A_H
