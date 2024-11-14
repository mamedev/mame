// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/*****************************************************************************
 *
 * dai_snd.h
 *
 ****************************************************************************/

#ifndef MAME_DAI_DAI_SND_H
#define MAME_DAI_DAI_SND_H


// ======================> dai_sound_device

class dai_sound_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	dai_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void set_input_ch0(int state);
	void set_input_ch1(int state);
	void set_input_ch2(int state);
	void set_volume(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	sound_stream *      m_mixer_channel = nullptr;
	int                 m_dai_input[3]{};
	uint8_t             m_osc_volume[3]{};
	uint8_t             m_noise_volume = 0;

	static const uint16_t s_osc_volume_table[];
	static const uint16_t s_noise_volume_table[];
};

DECLARE_DEVICE_TYPE(DAI_SOUND, dai_sound_device)

#endif // MAME_DAI_DAI_SND_H
