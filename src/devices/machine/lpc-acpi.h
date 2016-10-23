// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef LPC_ACPI_H
#define LPC_ACPI_H

#include "lpc.h"

#define MCFG_LPC_ACPI_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, LPC_ACPI, 0)

class lpc_acpi_device : public lpc_device {
public:
	lpc_acpi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	uint16_t pm1_sts_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void pm1_sts_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t pm1_en_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void pm1_en_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t pm1_cnt_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void pm1_cnt_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t pm1_tmr_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t proc_cnt_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void proc_cnt_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint8_t lv2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint32_t gpe0_sts_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void gpe0_sts_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t gpe0_en_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void gpe0_en_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t smi_en_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void smi_en_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t smi_sts_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void smi_sts_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint16_t alt_gp_smi_en_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void alt_gp_smi_en_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t alt_gp_smi_sts_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void alt_gp_smi_sts_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t devact_sts_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void devact_sts_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t devtrap_en_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void devtrap_en_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t bus_addr_track_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint8_t bus_cyc_track_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	uint8_t tco_rld_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tco_rld_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tco_tmr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tco_tmr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tco_dat_in_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tco_dat_in_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tco_dat_out_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tco_dat_out_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t tco1_sts_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void tco1_sts_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t tco2_sts_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void tco2_sts_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t tco1_cnt_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void tco1_cnt_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t tco2_cnt_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void tco2_cnt_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t tco_message1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tco_message1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tco_message2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tco_message2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tco_wdstatus_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tco_wdstatus_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sw_irq_gen_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sw_irq_gen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	void device_start() override;
	void device_reset() override;

private:
	uint32_t pm1_cnt, proc_cnt, gpe0_sts, gpe0_en, smi_en, smi_sts;
	uint16_t pm1_sts, pm1_en, alt_gp_smi_en, alt_gp_smi_sts, devact_sts, devtrap_en;
	uint16_t tco1_sts, tco2_sts, tco1_cnt, tco2_cnt;
	uint8_t  tco_rld, tco_tmr, tco_dat_in, tco_dat_out, tco_message1, tco_message2;
	uint8_t  tco_wdstatus, sw_irq_gen;

	DECLARE_ADDRESS_MAP(map, 32);
};

extern const device_type LPC_ACPI;

#endif
