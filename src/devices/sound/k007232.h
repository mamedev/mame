// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*********************************************************/
/*    Konami PCM controller                              */
/*********************************************************/

#ifndef MAME_SOUND_K007232_H
#define MAME_SOUND_K007232_H

#pragma once

class k007232_device : public device_t, public device_sound_interface, public device_memory_interface
{
public:
	k007232_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto port_write() { return m_port_write_handler.bind(); }

	void write(offs_t offset, u8 data);
	u8 read(offs_t offset);

	/*
	The 007232 has two channels and produces two outputs. The volume control
	is external, however to make it easier to use we handle that inside the
	emulation. You can control volume and panning: for each of the two channels
	you can set the volume of the two outputs. If panning is not required,
	then volumeB will be 0 for channel 0, and volumeA will be 0 for channel 1.
	Volume is in the range 0-255.
	*/
	void set_volume(int channel,int volumeA,int volumeB);

	void set_bank( int chABank, int chBBank );

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// device_memory_interface configuration
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_data_config;

private:
	static constexpr unsigned KDAC_A_PCM_MAX = 2;      /* Channels per chip */

	// internal state
	memory_access<17, 0, 0, ENDIANNESS_LITTLE>::cache m_cache;
	optional_region_ptr<u8> m_rom;

	struct channel_t
	{
		u8           vol[2]; /* volume for the left and right channel */
		u32          addr;
		int          counter;
		u32          start;
		u16          step;
		u32          bank;
		bool         play;
	};

	u8 read_rom_default(offs_t offset) { return m_rom[(m_bank + (offset & 0x1ffff)) & (m_rom.length() - 1)]; }
	inline u8 read_sample(int channel, u32 addr) { m_bank = m_channel[channel].bank; return m_cache.read_byte(addr & 0x1ffff); }

	channel_t     m_channel[KDAC_A_PCM_MAX]; // 2 channels

	u8            m_wreg[0x10]; /* write data */

	u32           m_pcmlimit;
	u32           m_bank;

	sound_stream *m_stream;
	devcb_write8  m_port_write_handler;
};

DECLARE_DEVICE_TYPE(K007232, k007232_device)

#endif // MAME_SOUND_K007232_H
