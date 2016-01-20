// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef LPC_ACPI_H
#define LPC_ACPI_H

#include "lpc.h"

#define MCFG_LPC_ACPI_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, LPC_ACPI, 0)

class lpc_acpi_device : public lpc_device {
public:
	lpc_acpi_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual void map_device(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
							UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space) override;

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

protected:
	void device_start() override;
	void device_reset() override;

private:
	UINT32 pm1_cnt, proc_cnt, gpe0_sts, gpe0_en, smi_en, smi_sts;
	UINT16 pm1_sts, pm1_en, alt_gp_smi_en, alt_gp_smi_sts, devact_sts, devtrap_en;
	UINT16 tco1_sts, tco2_sts, tco1_cnt, tco2_cnt;
	UINT8  tco_rld, tco_tmr, tco_dat_in, tco_dat_out, tco_message1, tco_message2;
	UINT8  tco_wdstatus, sw_irq_gen;

	DECLARE_ADDRESS_MAP(map, 32);
};

extern const device_type LPC_ACPI;

#endif
