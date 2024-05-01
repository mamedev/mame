// license:BSD-3-Clause
// copyright-holders:windyfairy
#ifndef MAME_JALECO_JALECO_VJ_SOUND_H
#define MAME_JALECO_JALECO_VJ_SOUND_H

#pragma once

#include "bus/isa/isa.h"
#include "sound/ymz280b.h"


class jaleco_vj_isa16_sound_device :
		public device_t,
		public device_isa16_card_interface,
		public device_mixer_interface
{
public:
	// construction/destruction
	jaleco_vj_isa16_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_steppingstage_mode(bool mode) { m_is_steppingstage = mode; } // TODO: Split this out into a device specific to Stepping Stage

	uint16_t response_r(offs_t offset, uint16_t mem_mask = ~0);
	void comm_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void ymz_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0)
	{
		if (ACCESSING_BITS_0_7)
			m_ymz[0]->write(offset, BIT(data, 0, 8));
		if (ACCESSING_BITS_8_15)
			m_ymz[1]->write(offset, BIT(data, 8, 8));
	}

	virtual void remap(int space_id, offs_t start, offs_t end) override;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t comm_r(offs_t offset);
	uint8_t unk2_r(offs_t offset);
	uint16_t buffer_status_r(offs_t offset, uint16_t mem_mask = ~0);
	void response_w(offs_t offset, uint8_t data);
	uint16_t target_buffer_r(offs_t offset, uint16_t mem_mask = ~0);
	void target_buffer_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sound_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t unkc_r(offs_t offset, uint16_t mem_mask = ~0);
	uint16_t unke_r(offs_t offset, uint16_t mem_mask = ~0);

	void io_map(address_map &map) ATTR_COLD;
	void ymz280b_map(address_map &map) ATTR_COLD;
	void ymz280b_map2(address_map &map) ATTR_COLD;

	required_device_array<ymz280b_device, 2> m_ymz;
	required_shared_ptr<uint8_t> m_ymzram;
	required_shared_ptr<uint8_t> m_ymzram2;

	uint16_t m_response;
	uint16_t m_comm;
	uint16_t m_target_buffer;
	uint32_t m_target_addr;
	uint32_t m_unke_read_cnt;
	bool m_request_extra_data;
	bool m_receiving_extra_data;
	bool m_is_checking_additional_bgm;

	bool m_is_steppingstage;
};


DECLARE_DEVICE_TYPE(JALECO_VJ_ISA16_SOUND, jaleco_vj_isa16_sound_device)

#endif // MAME_JALECO_JALECO_VJ_SOUND_H
