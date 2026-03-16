// license:BSD-3-Clause
// copyright-holders:Angelo Salese
#ifndef MAME_SOUND_AC97_STAC9704_H
#define MAME_SOUND_AC97_STAC9704_H

#pragma once

class ac97_stac9704_device : public device_t
                           , public device_mixer_interface
                           , public device_memory_interface
{
public:
	ac97_stac9704_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_pcm_tag(T &&tag) { m_pcm.set_tag(std::forward<T>(tag)); }

	void codec_write_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 codec_write_r(offs_t offset, u32 mem_mask = ~0);
	void codec_read_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 codec_read_r(offs_t offset, u32 mem_mask = ~0);

protected:
	ac97_stac9704_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

private:
	address_space_config m_space_config;
	required_device<device_sound_interface> m_pcm;

	void mixer_map(address_map &map);

	u8 m_index_r, m_index_w;
	u16 m_data_w;

	u16 m_master_vol, m_lnlvl_vol, m_master_mono_vol;
	u16 m_pc_beep_vol, m_phone_vol, m_mic_vol, m_linein_vol, m_cd_vol;
	u16 m_video_vol, m_aux_vol, m_pcm_out_vol;
	u16 m_record_sel, m_record_gain;
	u16 m_general_purpose, m_3d_control, m_power_ctrl;

	u16 m_vendor_id1, m_vendor_id2;

	void update_gain_levels();
};

DECLARE_DEVICE_TYPE(AC97_STAC9704, ac97_stac9704_device)


#endif // MAME_SOUND_AC97_STAC9704_H
