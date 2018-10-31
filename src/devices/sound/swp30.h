// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWP30/30B, rompler/dsp combo

#ifndef DEVICES_SOUND_SWP30_H
#define DEVICES_SOUND_SWP30_H

#pragma once

class swp30_device : public device_t, public device_sound_interface, public device_rom_interface
{
public:
	swp30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void map(address_map &map);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
	virtual void rom_bank_updated() override;

private:
	sound_stream *m_stream;

	s32 m_sample_increment[0x4000];
	s32 m_linear_attenuation[0x100];
	s16 m_sample_log8[0x100];

	u64 m_keyon_mask, m_active_mask;
	u32 m_pre_size[0x40], m_post_size[0x40], m_address[0x40];

	s32 m_sample_pos[64];

	u16 m_volume[0x40], m_freq[0x40], m_pan[0x40];

	u16 volume_r(offs_t offset);
	void volume_w(offs_t offset, u16 data);
	u16 freq_r(offs_t offset);
	void freq_w(offs_t offset, u16 data);
	u16 pre_size_r(offs_t offset);
	void pre_size_w(offs_t offset, u16 data);
	u16 post_size_r(offs_t offset);
	void post_size_w(offs_t offset, u16 data);
	u16 address_r(offs_t offset);
	void address_w(offs_t offset, u16 data);
	u16 pan_r(offs_t offset);
	void pan_w(offs_t offset, u16 data);
	template<int sel> u16 keyon_mask_r();
	template<int sel> void keyon_mask_w(u16 data);
	u16 keyon_r();
	void keyon_w(u16);

	u16 snd_r(offs_t offset);
	void snd_w(offs_t offset, u16 data);
};

DECLARE_DEVICE_TYPE(SWP30, swp30_device)

#endif
