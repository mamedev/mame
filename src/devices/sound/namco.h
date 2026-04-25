// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Aaron Giles
#ifndef MAME_SOUND_NAMCO_H
#define MAME_SOUND_NAMCO_H

#pragma once


template <unsigned Voices, bool Packed>
class namco_audio_device : public device_t,
							public device_sound_interface,
							public device_memory_interface
{
public:
	// configuration
	void sound_enable_w(int state);

protected:
	static inline constexpr unsigned MAX_VOICES = Voices;
	static inline constexpr unsigned MAX_VOLUME = 16;
	static inline constexpr int32_t MIX_RES = 128 * MAX_VOICES;

	// this structure defines the parameters for a channel
	struct sound_channel
	{
		uint32_t frequency = 0;
		uint32_t counter = 0;
		int32_t volume[4]{};
		int32_t noise_sw = 0;
		int32_t noise_state = 0;
		int32_t noise_seed = 1;
		uint32_t noise_counter = 0;
		int32_t noise_hold = 0;
		uint16_t waveform_select = 0;
	};

	namco_audio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream) override;

	// internal state
	void build_decoded_waveform(uint8_t *rgnbase);
	uint32_t namco_update_one(sound_stream &stream, int output, uint16_t select, int volume, uint32_t counter, uint32_t freq);

	// a position of waveform sample
	uint8_t waveform_position(int n) const { return (n >> m_f_fracbits) & 0x1f; }

	// get waveform
	int waveform_r(uint16_t pos);

	// address space
	const address_space_config m_data_config;
	memory_access<8, 0, 0, ENDIANNESS_BIG>::cache m_data;

	// waveform region
	optional_region_ptr<uint8_t> m_wave_ptr;

	// data about the sound system
	sound_channel m_channel_list[MAX_VOICES];

	// global sound parameters
	bool m_sound_enable;
	sound_stream *m_stream;
	int32_t m_namco_clock;
	int32_t m_sample_rate;
	int32_t m_f_fracbits;

	std::unique_ptr<uint8_t[]> m_waveram_alloc;

	std::unique_ptr<uint8_t[]> m_soundregs;	

};

class namco_wsg_device : public namco_audio_device<3, false>
{
public:
	namco_wsg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void pacman_sound_w(offs_t offset, uint8_t data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream) override;
};


class polepos_wsg_device : public namco_audio_device<8, false>
{
public:
	// unverified and/or incorrect panning?
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	polepos_wsg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t polepos_sound_r(offs_t offset);
	void polepos_sound_w(offs_t offset, uint8_t data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream) override;
};


class namco_15xx_device : public namco_audio_device<8, false>
{
public:
	namco_15xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t namco_15xx_r(offs_t offset);
	void namco_15xx_w(offs_t offset, uint8_t data);

	void amap(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream) override;

private:
	required_shared_ptr<uint8_t> m_sharedram;
};


class namco_cus30_device : public namco_audio_device<8, true>
{
public:
	namco_cus30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_stereo(bool stereo) { m_stereo = stereo; }

	uint8_t cus30_r(offs_t offset);
	void cus30_w(offs_t offset, uint8_t data); // wavedata + sound registers + RAM

	void amap(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream) override;

private:
	required_shared_ptr<uint8_t> m_sharedram;
	required_shared_ptr<uint8_t> m_waveram;
	bool m_stereo; // set to indicate stereo (e.g., System 1)
};

extern template class namco_audio_device<3, false>;
extern template class namco_audio_device<8, false>;
extern template class namco_audio_device<8, true>;

DECLARE_DEVICE_TYPE(NAMCO_WSG,   namco_wsg_device)
DECLARE_DEVICE_TYPE(POLEPOS_WSG, polepos_wsg_device)
DECLARE_DEVICE_TYPE(NAMCO_15XX,  namco_15xx_device)
DECLARE_DEVICE_TYPE(NAMCO_CUS30, namco_cus30_device)

#endif // MAME_SOUND_NAMCO_H
