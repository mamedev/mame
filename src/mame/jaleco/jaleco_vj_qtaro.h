// license:BSD-3-Clause
// copyright-holders:windyfairy

#ifndef MAME_JALECO_JALECO_VJ_QTARO_H
#define MAME_JALECO_JALECO_VJ_QTARO_H

#pragma once

#include "machine/pci.h"


class jaleco_vj_qtaro_device : public device_t
{
public:
	jaleco_vj_qtaro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type imperfect_features() {
		return feature::TIMING; // DMA timings aren't perfectly synced between all displays so one video stream may end up out of sync
	}

	void video_mix_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint8_t reg_r(offs_t offset);
	void reg_w(offs_t offset, uint8_t data);

	uint8_t reg2_r(offs_t offset);

	uint32_t reg3_r(offs_t offset);
	void reg3_w(offs_t offset, uint32_t data);

	void write(uint8_t *data, uint32_t len);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_int;
	uint32_t m_mix_level;
};


/////////////////////////////////////////////

class jaleco_vj_king_qtaro_device : public pci_device
{
public:
	jaleco_vj_king_qtaro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <int DeviceId> void video_mix_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { m_qtaro[DeviceId]->video_mix_w(offset, data, mem_mask); }

	void video_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	jaleco_vj_king_qtaro_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void map(address_map &map) ATTR_COLD;

	uint32_t qtaro_fpga_firmware_status_r(offs_t offset);
	void qtaro_fpga_firmware_status_w(offs_t offset, uint32_t data);

	uint32_t qtaro_fpga_firmware_r(offs_t offset);
	void qtaro_fpga_firmware_w(offs_t offset, uint32_t data);

	uint32_t fpga_firmware_status_r(offs_t offset);
	void fpga_firmware_status_w(offs_t offset, uint32_t data);

	uint32_t fpga_firmware_r(offs_t offset);
	void fpga_firmware_w(offs_t offset, uint32_t data);

	uint8_t event_io_mask_r(offs_t offset);
	void event_io_mask_w(offs_t offset, uint8_t data);

	uint8_t event_unk_r(offs_t offset);
	void event_unk_w(offs_t offset, uint8_t data);

	uint8_t event_io_r(offs_t offset);
	void event_io_w(offs_t offset, uint8_t data);

	uint32_t event_r(offs_t offset);
	void event_w(offs_t offset, uint32_t data);

	uint32_t event_mask_r(offs_t offset);
	void event_mask_w(offs_t offset, uint32_t data);

	uint32_t int_r(offs_t offset);
	void int_w(offs_t offset, uint32_t data);

	uint32_t int_fpga_r(offs_t offset);
	void int_fpga_w(offs_t offset, uint32_t data);

	template <int DeviceId> void dma_requested_w(offs_t offset, uint32_t data);

	template <int DeviceId> void dma_descriptor_phys_addr_w(offs_t offset, uint32_t data);

	template <int DeviceId> uint32_t dma_running_r(offs_t offset);
	template <int DeviceId> void dma_running_w(offs_t offset, uint32_t data);

	TIMER_CALLBACK_MEMBER(video_dma_callback);

	required_device_array<jaleco_vj_qtaro_device, 3> m_qtaro;

	emu_timer *m_dma_timer;

	uint32_t m_int;
	uint32_t m_int_fpga;

	uint32_t m_event, m_event_mask;
	uint8_t m_event_io[5], m_event_io_mask[5];
	uint8_t m_event_unk[5], m_event_unk_mask[5];

	bool m_dma_running[3];
	uint32_t m_dma_descriptor_requested_addr[3];
	uint32_t m_dma_descriptor_addr[3];
	uint32_t m_dma_descriptor_length[3];
};


/////////////////////////////////////////////

DECLARE_DEVICE_TYPE(JALECO_VJ_QTARO, jaleco_vj_qtaro_device)
DECLARE_DEVICE_TYPE(JALECO_VJ_KING_QTARO, jaleco_vj_king_qtaro_device)

#endif  // MAME_JALECO_JALECO_VJ_QTARO_H
