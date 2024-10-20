// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_LPC_ACPI_H
#define MAME_MACHINE_LPC_ACPI_H

#pragma once

#include "lpc.h"

class lpc_acpi_device : public lpc_device {
public:
	lpc_acpi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

private:
	uint32_t pm1_cnt, proc_cnt, gpe0_sts, gpe0_en, smi_en, smi_sts;
	uint16_t pm1_sts, pm1_en, alt_gp_smi_en, alt_gp_smi_sts, devact_sts, devtrap_en;
	uint16_t tco1_sts, tco2_sts, tco1_cnt, tco2_cnt;
	uint8_t  tco_rld, tco_tmr, tco_dat_in, tco_dat_out, tco_message1, tco_message2;
	uint8_t  tco_wdstatus, sw_irq_gen;

	void map(address_map &map) ATTR_COLD;

	uint16_t pm1_sts_r();
	void pm1_sts_w(uint16_t data);
	uint16_t pm1_en_r();
	void pm1_en_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t pm1_cnt_r();
	void pm1_cnt_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t pm1_tmr_r();
	uint32_t proc_cnt_r();
	void proc_cnt_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t lv2_r();
	uint32_t gpe0_sts_r();
	void gpe0_sts_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t gpe0_en_r();
	void gpe0_en_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t smi_en_r();
	void smi_en_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t smi_sts_r();
	void smi_sts_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t alt_gp_smi_en_r();
	void alt_gp_smi_en_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t alt_gp_smi_sts_r();
	void alt_gp_smi_sts_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t devact_sts_r();
	void devact_sts_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t devtrap_en_r();
	void devtrap_en_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t bus_addr_track_r();
	uint8_t bus_cyc_track_r();

	uint8_t tco_rld_r();
	void tco_rld_w(uint8_t data);
	uint8_t tco_tmr_r();
	void tco_tmr_w(uint8_t data);
	uint8_t tco_dat_in_r();
	void tco_dat_in_w(uint8_t data);
	uint8_t tco_dat_out_r();
	void tco_dat_out_w(uint8_t data);
	uint16_t tco1_sts_r();
	void tco1_sts_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t tco2_sts_r();
	void tco2_sts_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t tco1_cnt_r();
	void tco1_cnt_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t tco2_cnt_r();
	void tco2_cnt_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t tco_message1_r();
	void tco_message1_w(uint8_t data);
	uint8_t tco_message2_r();
	void tco_message2_w(uint8_t data);
	uint8_t tco_wdstatus_r();
	void tco_wdstatus_w(uint8_t data);
	uint8_t sw_irq_gen_r();
	void sw_irq_gen_w(uint8_t data);
};

DECLARE_DEVICE_TYPE(LPC_ACPI, lpc_acpi_device)

#endif // MAME_MACHINE_LPC_ACPI_H
