// license:BSD-3-Clause
// copyright-holders:windyfairy
#ifndef MAME_JALECO_JALECO_VJ_PC_H
#define MAME_JALECO_JALECO_VJ_PC_H

#pragma once

#include "jaleco_vj_qtaro.h"
#include "jaleco_vj_sound.h"

#include "cpu/i386/i386.h"


class jaleco_vj_pc_device :
		public device_t,
		public device_mixer_interface
{
public:
	jaleco_vj_pc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_steppingstage_mode(bool mode) { m_is_steppingstage = mode; }

	uint16_t response_r(offs_t offset, uint16_t mem_mask = ~0) { return m_sound->response_r(offset, mem_mask); }
	void comm_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { m_sound->comm_w(offset, data, mem_mask); }
	void ymz_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { m_sound->ymz_w(offset, data, mem_mask); }

	template <int DeviceId> void render_video_frame(bitmap_rgb32 &bitmap) { m_king_qtaro->render_video_frame<DeviceId>(bitmap); }
	template <int DeviceId> void video_mix_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { m_king_qtaro->video_mix_w<DeviceId>(offset, data); }
	void video_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { m_king_qtaro->video_control_w(offset, data); }

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	void superio_config(device_t &device) ATTR_COLD;
	void sound_config(device_t &device) ATTR_COLD;

private:
	void boot_state_w(uint8_t data);

	required_device<pentium_device> m_maincpu;
	required_device<jaleco_vj_king_qtaro_device> m_king_qtaro;
	required_device<jaleco_vj_isa16_sound_device> m_sound;
	bool m_is_steppingstage;
};


DECLARE_DEVICE_TYPE(JALECO_VJ_PC, jaleco_vj_pc_device)

#endif // MAME_JALECO_JALECO_VJ_PC_H
