// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi, David Haywood
#ifndef MAME_NICHIBUTSU_GOMOKU_A_H
#define MAME_NICHIBUTSU_GOMOKU_A_H

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> gomoku_sound_device

class gomoku_sound_device : public device_t,
							public device_sound_interface
{
public:
	gomoku_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 48'000);

	void sound1_w(offs_t offset, uint8_t data);
	void sound2_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	void make_mixer_table(int voices, int gain);

	// 4 voices max
	static constexpr unsigned MAX_VOICES = 4;

	struct sound_channel
	{
		sound_channel() { }

		int channel = 0;
		int frequency = 0;
		int counter = 0;
		int volume = 0;
		int oneshotplaying = 0;
	};


	// data about the sound system
	sound_channel m_channel_list[MAX_VOICES];

	// global sound parameters
	required_region_ptr<uint8_t> m_sound_rom;
	bool m_sound_enable;
	sound_stream *m_stream;

	// mixer tables and internal buffers
	std::vector<short> m_mixer_buffer;

	uint8_t m_soundregs1[0x20];
	uint8_t m_soundregs2[0x20];
};

DECLARE_DEVICE_TYPE(GOMOKU_SOUND, gomoku_sound_device)

#endif // MAME_NICHIBUTSU_GOMOKU_A_H
