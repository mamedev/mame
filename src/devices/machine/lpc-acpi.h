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
	void device_start() override;
	void device_reset() override;

private:
	uint32_t pm1_cnt, proc_cnt, gpe0_sts, gpe0_en, smi_en, smi_sts;
	uint16_t pm1_sts, pm1_en, alt_gp_smi_en, alt_gp_smi_sts, devact_sts, devtrap_en;
	uint16_t tco1_sts, tco2_sts, tco1_cnt, tco2_cnt;
	uint8_t  tco_rld, tco_tmr, tco_dat_in, tco_dat_out, tco_message1, tco_message2;
	uint8_t  tco_wdstatus, sw_irq_gen;

	void map(address_map &map);

	DECLARE_READ16_MEMBER( pm1_sts_r);
	DECLARE_WRITE16_MEMBER(pm1_sts_w);
	DECLARE_READ16_MEMBER( pm1_en_r);
	DECLARE_WRITE16_MEMBER(pm1_en_w);
	DECLARE_READ32_MEMBER( pm1_cnt_r);
	DECLARE_WRITE32_MEMBER(pm1_cnt_w);
	DECLARE_READ32_MEMBER( pm1_tmr_r);
	DECLARE_READ32_MEMBER( proc_cnt_r);
	DECLARE_WRITE32_MEMBER(proc_cnt_w);
	DECLARE_READ8_MEMBER(  lv2_r);
	DECLARE_READ32_MEMBER( gpe0_sts_r);
	DECLARE_WRITE32_MEMBER(gpe0_sts_w);
	DECLARE_READ32_MEMBER( gpe0_en_r);
	DECLARE_WRITE32_MEMBER(gpe0_en_w);
	DECLARE_READ32_MEMBER( smi_en_r);
	DECLARE_WRITE32_MEMBER(smi_en_w);
	DECLARE_READ32_MEMBER( smi_sts_r);
	DECLARE_WRITE32_MEMBER(smi_sts_w);
	DECLARE_READ16_MEMBER( alt_gp_smi_en_r);
	DECLARE_WRITE16_MEMBER(alt_gp_smi_en_w);
	DECLARE_READ16_MEMBER( alt_gp_smi_sts_r);
	DECLARE_WRITE16_MEMBER(alt_gp_smi_sts_w);
	DECLARE_READ16_MEMBER( devact_sts_r);
	DECLARE_WRITE16_MEMBER(devact_sts_w);
	DECLARE_READ16_MEMBER( devtrap_en_r);
	DECLARE_WRITE16_MEMBER(devtrap_en_w);
	DECLARE_READ16_MEMBER( bus_addr_track_r);
	DECLARE_READ8_MEMBER(  bus_cyc_track_r);

	DECLARE_READ8_MEMBER(  tco_rld_r);
	DECLARE_WRITE8_MEMBER( tco_rld_w);
	DECLARE_READ8_MEMBER(  tco_tmr_r);
	DECLARE_WRITE8_MEMBER( tco_tmr_w);
	DECLARE_READ8_MEMBER(  tco_dat_in_r);
	DECLARE_WRITE8_MEMBER( tco_dat_in_w);
	DECLARE_READ8_MEMBER(  tco_dat_out_r);
	DECLARE_WRITE8_MEMBER( tco_dat_out_w);
	DECLARE_READ16_MEMBER( tco1_sts_r);
	DECLARE_WRITE16_MEMBER(tco1_sts_w);
	DECLARE_READ16_MEMBER( tco2_sts_r);
	DECLARE_WRITE16_MEMBER(tco2_sts_w);
	DECLARE_READ16_MEMBER( tco1_cnt_r);
	DECLARE_WRITE16_MEMBER(tco1_cnt_w);
	DECLARE_READ16_MEMBER( tco2_cnt_r);
	DECLARE_WRITE16_MEMBER(tco2_cnt_w);
	DECLARE_READ8_MEMBER(  tco_message1_r);
	DECLARE_WRITE8_MEMBER( tco_message1_w);
	DECLARE_READ8_MEMBER(  tco_message2_r);
	DECLARE_WRITE8_MEMBER( tco_message2_w);
	DECLARE_READ8_MEMBER(  tco_wdstatus_r);
	DECLARE_WRITE8_MEMBER( tco_wdstatus_w);
	DECLARE_READ8_MEMBER(  sw_irq_gen_r);
	DECLARE_WRITE8_MEMBER( sw_irq_gen_w);
};

DECLARE_DEVICE_TYPE(LPC_ACPI, lpc_acpi_device)

#endif // MAME_MACHINE_LPC_ACPI_H
