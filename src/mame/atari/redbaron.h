// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Nicola Salmoria
#ifndef MAME_ATARI_REDBARON_H
#define MAME_ATARI_REDBARON_H

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class redbaron_sound_device : public device_t, public device_sound_interface
{
public:
	redbaron_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void sounds_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	[[maybe_unused]] void pokey_w(offs_t offset, uint8_t data);

	std::unique_ptr<int16_t[]> m_vol_lookup;

	int16_t m_vol_crash[16];

	sound_stream *m_channel;
	uint8_t m_latch;
	int32_t m_poly_counter;
	uint16_t m_poly_shift;

	int32_t m_filter_counter;

	uint8_t m_crash_amp;
	uint16_t m_shot_amp;
	int32_t m_shot_amp_counter;

	uint16_t m_squeal_amp;
	int32_t m_squeal_amp_counter;
	int32_t m_squeal_off_counter;
	int32_t m_squeal_on_counter;
	uint8_t m_squeal_out;
};

DECLARE_DEVICE_TYPE(REDBARON, redbaron_sound_device)

#endif // MAME_ATARI_REDBARON_H
