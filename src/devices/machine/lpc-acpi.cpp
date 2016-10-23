// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "lpc-acpi.h"

const device_type LPC_ACPI = &device_creator<lpc_acpi_device>;

DEVICE_ADDRESS_MAP_START(map, 32, lpc_acpi_device)
	AM_RANGE(0x00, 0x03) AM_READWRITE16(pm1_sts_r,        pm1_sts_w,        0x0000ffff)
	AM_RANGE(0x00, 0x03) AM_READWRITE16(pm1_en_r,         pm1_en_w,         0xffff0000)
	AM_RANGE(0x04, 0x07) AM_READWRITE  (pm1_cnt_r,        pm1_cnt_w)
	AM_RANGE(0x08, 0x0b) AM_READ       (pm1_tmr_r)
	AM_RANGE(0x10, 0x13) AM_READWRITE  (proc_cnt_r,       proc_cnt_w)
	AM_RANGE(0x14, 0x17) AM_READ8      (lv2_r,                              0x000000ff)
	AM_RANGE(0x28, 0x2b) AM_READWRITE  (gpe0_sts_r,       gpe0_sts_w)
	AM_RANGE(0x2c, 0x2f) AM_READWRITE  (gpe0_en_r,        gpe0_en_w)
	AM_RANGE(0x30, 0x33) AM_READWRITE  (smi_en_r,         smi_en_w)
	AM_RANGE(0x34, 0x37) AM_READWRITE  (smi_sts_r,        smi_sts_w)
	AM_RANGE(0x38, 0x3b) AM_READWRITE16(alt_gp_smi_en_r,  alt_gp_smi_en_w,  0x0000ffff)
	AM_RANGE(0x38, 0x3b) AM_READWRITE16(alt_gp_smi_sts_r, alt_gp_smi_sts_w, 0xffff0000)
	AM_RANGE(0x44, 0x47) AM_READWRITE16(devact_sts_r,     devact_sts_w,     0x0000ffff)
	AM_RANGE(0x48, 0x4b) AM_READWRITE16(devtrap_en_r,     devtrap_en_w,     0x0000ffff)
	AM_RANGE(0x4c, 0x4f) AM_READ16     (bus_addr_track_r,                   0x0000ffff)
	AM_RANGE(0x4c, 0x4f) AM_READ8      (bus_cyc_track_r,                    0x00ff0000)

	AM_RANGE(0x60, 0x63) AM_READWRITE8 (tco_rld_r,        tco_rld_w,        0x000000ff)
	AM_RANGE(0x60, 0x63) AM_READWRITE8 (tco_tmr_r,        tco_tmr_w,        0x0000ff00)
	AM_RANGE(0x60, 0x63) AM_READWRITE8 (tco_dat_in_r,     tco_dat_in_w,     0x00ff0000)
	AM_RANGE(0x60, 0x63) AM_READWRITE8 (tco_dat_out_r,    tco_dat_out_w,    0xff000000)
	AM_RANGE(0x64, 0x67) AM_READWRITE16(tco1_sts_r,       tco1_sts_w,       0x0000ffff)
	AM_RANGE(0x64, 0x67) AM_READWRITE16(tco2_sts_r,       tco2_sts_w,       0xffff0000)
	AM_RANGE(0x68, 0x6b) AM_READWRITE16(tco1_cnt_r,       tco1_cnt_w,       0x0000ffff)
	AM_RANGE(0x68, 0x6b) AM_READWRITE16(tco2_cnt_r,       tco2_cnt_w,       0xffff0000)
	AM_RANGE(0x6c, 0x6f) AM_READWRITE8 (tco_message1_r,   tco_message1_w,   0x000000ff)
	AM_RANGE(0x6c, 0x6f) AM_READWRITE8 (tco_message2_r,   tco_message2_w,   0x0000ff00)
	AM_RANGE(0x6c, 0x6f) AM_READWRITE8 (tco_wdstatus_r,   tco_wdstatus_w,   0x00ff0000)
	AM_RANGE(0x70, 0x73) AM_READWRITE8 (sw_irq_gen_r,     sw_irq_gen_w,     0x000000ff)
ADDRESS_MAP_END

lpc_acpi_device::lpc_acpi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: lpc_device(mconfig, LPC_ACPI, "LPC ACPI", tag, owner, clock, "lpc_acpi", __FILE__)
{
}

void lpc_acpi_device::map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	io_space->install_device(io_offset, io_window_end, *this, &lpc_acpi_device::map);
}

void lpc_acpi_device::device_start()
{
}

void lpc_acpi_device::device_reset()
{
	pm1_sts = 0;
	pm1_en = 0;
	pm1_cnt = 0;
	proc_cnt = 0;
	gpe0_sts = 0;
	gpe0_en = 0;
	smi_en = 0;
	smi_sts = 0;
	alt_gp_smi_en = 0;
	alt_gp_smi_sts = 0;
	devact_sts = 0;
	devtrap_en = 0;

	tco_rld = 0;
	tco_tmr = 0;
	tco_dat_in = 0;
	tco_dat_out = 0;
	tco1_sts = 0;
	tco2_sts = 0;
	tco1_cnt = 0;
	tco2_cnt = 0;
	tco_message1 = 0;
	tco_message2 = 0;
	tco_wdstatus = 0;
	sw_irq_gen = 0;
}

uint16_t lpc_acpi_device::pm1_sts_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return pm1_sts;
}

void lpc_acpi_device::pm1_sts_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	pm1_sts &= ~data;
	logerror("%s: pm1_sts = %04x\n", tag(), pm1_sts);
}

uint16_t lpc_acpi_device::pm1_en_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return pm1_en;
}

void lpc_acpi_device::pm1_en_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&pm1_en);
	logerror("%s: pm1_en = %04x\n", tag(), pm1_en);
}

uint32_t lpc_acpi_device::pm1_cnt_r(address_space &space, offs_t offset, uint32_t mem_mask)
{
	return pm1_cnt;
}

void lpc_acpi_device::pm1_cnt_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&pm1_cnt);
	logerror("%s: pm1_cnt = %08x\n", tag(), pm1_cnt);
}

uint32_t lpc_acpi_device::pm1_tmr_r(address_space &space, offs_t offset, uint32_t mem_mask)
{
	return machine().time().as_ticks(3579545) & 0xffffff;
}

uint32_t lpc_acpi_device::proc_cnt_r(address_space &space, offs_t offset, uint32_t mem_mask)
{
	return proc_cnt;
}

void lpc_acpi_device::proc_cnt_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&proc_cnt);
	logerror("%s: proc_cnt = %08x\n", tag(), proc_cnt);
}

uint8_t lpc_acpi_device::lv2_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return 0x00;
}

uint32_t lpc_acpi_device::gpe0_sts_r(address_space &space, offs_t offset, uint32_t mem_mask)
{
	return gpe0_sts;
}

void lpc_acpi_device::gpe0_sts_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&gpe0_sts);
	logerror("%s: gpe0_sts = %08x\n", tag(), gpe0_sts);
}

uint32_t lpc_acpi_device::gpe0_en_r(address_space &space, offs_t offset, uint32_t mem_mask)
{
	return gpe0_en;
}

void lpc_acpi_device::gpe0_en_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&gpe0_en);
	logerror("%s: gpe0_en = %08x\n", tag(), gpe0_en);
}

uint32_t lpc_acpi_device::smi_en_r(address_space &space, offs_t offset, uint32_t mem_mask)
{
	return smi_en;
}

void lpc_acpi_device::smi_en_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&smi_en);
	logerror("%s: smi_en = %08x\n", tag(), smi_en);
}

uint32_t lpc_acpi_device::smi_sts_r(address_space &space, offs_t offset, uint32_t mem_mask)
{
	return smi_sts;
}

void lpc_acpi_device::smi_sts_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&smi_sts);
	logerror("%s: smi_sts = %08x\n", tag(), smi_sts);
}

uint16_t lpc_acpi_device::alt_gp_smi_en_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return alt_gp_smi_en;
}

void lpc_acpi_device::alt_gp_smi_en_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&alt_gp_smi_en);
	logerror("%s: alt_gp_smi_en = %04x\n", tag(), alt_gp_smi_en);
}

uint16_t lpc_acpi_device::alt_gp_smi_sts_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return alt_gp_smi_sts;
}

void lpc_acpi_device::alt_gp_smi_sts_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&alt_gp_smi_sts);
	logerror("%s: alt_gp_smi_sts = %04x\n", tag(), alt_gp_smi_sts);
}

uint16_t lpc_acpi_device::devact_sts_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return devact_sts;
}

void lpc_acpi_device::devact_sts_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&devact_sts);
	logerror("%s: devact_sts = %04x\n", tag(), devact_sts);
}

uint16_t lpc_acpi_device::devtrap_en_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return devtrap_en;
}

void lpc_acpi_device::devtrap_en_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&devtrap_en);
	logerror("%s: devtrap_en = %04x\n", tag(), devtrap_en);
}

uint16_t lpc_acpi_device::bus_addr_track_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	logerror("%s: read bus_addr_track\n", tag());
	return 0;
}

uint8_t lpc_acpi_device::bus_cyc_track_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	logerror("%s: read bus_cyc_track\n", tag());
	return 0;
}


uint8_t lpc_acpi_device::tco_rld_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return tco_rld;
}

void lpc_acpi_device::tco_rld_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	tco_rld = data;
	logerror("%s: tco_rld = %02x\n", tag(), tco_rld);
}

uint8_t lpc_acpi_device::tco_tmr_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return tco_tmr;
}

void lpc_acpi_device::tco_tmr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	tco_tmr = data;
	logerror("%s: tco_tmr = %02x\n", tag(), tco_tmr);
}

uint8_t lpc_acpi_device::tco_dat_in_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return tco_dat_in;
}

void lpc_acpi_device::tco_dat_in_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	tco_dat_in = data;
	logerror("%s: tco_dat_in = %02x\n", tag(), tco_dat_in);
}

uint8_t lpc_acpi_device::tco_dat_out_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return tco_dat_out;
}

void lpc_acpi_device::tco_dat_out_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	tco_dat_out = data;
	logerror("%s: tco_dat_out = %02x\n", tag(), tco_dat_out);
}

uint16_t lpc_acpi_device::tco1_sts_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return tco1_sts;
}

void lpc_acpi_device::tco1_sts_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&tco1_sts);
	logerror("%s: tco1_sts = %04x\n", tag(), tco1_sts);
}

uint16_t lpc_acpi_device::tco2_sts_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return tco2_sts;
}

void lpc_acpi_device::tco2_sts_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&tco2_sts);
	logerror("%s: tco2_sts = %04x\n", tag(), tco2_sts);
}

uint16_t lpc_acpi_device::tco1_cnt_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return tco1_cnt;
}

void lpc_acpi_device::tco1_cnt_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&tco1_cnt);
	logerror("%s: tco1_cnt = %04x\n", tag(), tco1_cnt);
}

uint16_t lpc_acpi_device::tco2_cnt_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return tco2_cnt;
}

void lpc_acpi_device::tco2_cnt_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&tco2_cnt);
	logerror("%s: tco2_cnt = %04x\n", tag(), tco2_cnt);
}

uint8_t lpc_acpi_device::tco_message1_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return tco_message1;
}

void lpc_acpi_device::tco_message1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	tco_message1 = data;
	logerror("%s: tco_message1 = %02x\n", tag(), tco_message1);
}

uint8_t lpc_acpi_device::tco_message2_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return tco_message2;
}

void lpc_acpi_device::tco_message2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	tco_message2 = data;
	logerror("%s: tco_message2 = %02x\n", tag(), tco_message2);
}

uint8_t lpc_acpi_device::tco_wdstatus_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return tco_wdstatus;
}

void lpc_acpi_device::tco_wdstatus_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	tco_wdstatus = data;
	logerror("%s: tco_wdstatus = %02x\n", tag(), tco_wdstatus);
}

uint8_t lpc_acpi_device::sw_irq_gen_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return sw_irq_gen;
}

void lpc_acpi_device::sw_irq_gen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	sw_irq_gen = data;
	logerror("%s: sw_irq_gen = %02x\n", tag(), sw_irq_gen);
}
