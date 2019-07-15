// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "lpc-acpi.h"

DEFINE_DEVICE_TYPE(LPC_ACPI, lpc_acpi_device, "lpc_acpi", "LPC ACPI")

void lpc_acpi_device::map(address_map &map)
{
	map(0x00, 0x01).rw(FUNC(lpc_acpi_device::pm1_sts_r), FUNC(lpc_acpi_device::pm1_sts_w));
	map(0x02, 0x03).rw(FUNC(lpc_acpi_device::pm1_en_r), FUNC(lpc_acpi_device::pm1_en_w));
	map(0x04, 0x07).rw(FUNC(lpc_acpi_device::pm1_cnt_r), FUNC(lpc_acpi_device::pm1_cnt_w));
	map(0x08, 0x0b).r(FUNC(lpc_acpi_device::pm1_tmr_r));
	map(0x10, 0x13).rw(FUNC(lpc_acpi_device::proc_cnt_r), FUNC(lpc_acpi_device::proc_cnt_w));
	map(0x14, 0x14).r(FUNC(lpc_acpi_device::lv2_r));
	map(0x28, 0x2b).rw(FUNC(lpc_acpi_device::gpe0_sts_r), FUNC(lpc_acpi_device::gpe0_sts_w));
	map(0x2c, 0x2f).rw(FUNC(lpc_acpi_device::gpe0_en_r), FUNC(lpc_acpi_device::gpe0_en_w));
	map(0x30, 0x33).rw(FUNC(lpc_acpi_device::smi_en_r), FUNC(lpc_acpi_device::smi_en_w));
	map(0x34, 0x37).rw(FUNC(lpc_acpi_device::smi_sts_r), FUNC(lpc_acpi_device::smi_sts_w));
	map(0x38, 0x39).rw(FUNC(lpc_acpi_device::alt_gp_smi_en_r), FUNC(lpc_acpi_device::alt_gp_smi_en_w));
	map(0x3a, 0x3b).rw(FUNC(lpc_acpi_device::alt_gp_smi_sts_r), FUNC(lpc_acpi_device::alt_gp_smi_sts_w));
	map(0x44, 0x45).rw(FUNC(lpc_acpi_device::devact_sts_r), FUNC(lpc_acpi_device::devact_sts_w));
	map(0x48, 0x49).rw(FUNC(lpc_acpi_device::devtrap_en_r), FUNC(lpc_acpi_device::devtrap_en_w));
	map(0x4c, 0x4d).r(FUNC(lpc_acpi_device::bus_addr_track_r));
	map(0x4e, 0x4e).r(FUNC(lpc_acpi_device::bus_cyc_track_r));

	map(0x60, 0x60).rw(FUNC(lpc_acpi_device::tco_rld_r), FUNC(lpc_acpi_device::tco_rld_w));
	map(0x61, 0x61).rw(FUNC(lpc_acpi_device::tco_tmr_r), FUNC(lpc_acpi_device::tco_tmr_w));
	map(0x62, 0x62).rw(FUNC(lpc_acpi_device::tco_dat_in_r), FUNC(lpc_acpi_device::tco_dat_in_w));
	map(0x63, 0x63).rw(FUNC(lpc_acpi_device::tco_dat_out_r), FUNC(lpc_acpi_device::tco_dat_out_w));
	map(0x64, 0x65).rw(FUNC(lpc_acpi_device::tco1_sts_r), FUNC(lpc_acpi_device::tco1_sts_w));
	map(0x66, 0x67).rw(FUNC(lpc_acpi_device::tco2_sts_r), FUNC(lpc_acpi_device::tco2_sts_w));
	map(0x68, 0x69).rw(FUNC(lpc_acpi_device::tco1_cnt_r), FUNC(lpc_acpi_device::tco1_cnt_w));
	map(0x6a, 0x6b).rw(FUNC(lpc_acpi_device::tco2_cnt_r), FUNC(lpc_acpi_device::tco2_cnt_w));
	map(0x6c, 0x6c).rw(FUNC(lpc_acpi_device::tco_message1_r), FUNC(lpc_acpi_device::tco_message1_w));
	map(0x6d, 0x6d).rw(FUNC(lpc_acpi_device::tco_message2_r), FUNC(lpc_acpi_device::tco_message2_w));
	map(0x6e, 0x6e).rw(FUNC(lpc_acpi_device::tco_wdstatus_r), FUNC(lpc_acpi_device::tco_wdstatus_w));
	map(0x70, 0x70).rw(FUNC(lpc_acpi_device::sw_irq_gen_r), FUNC(lpc_acpi_device::sw_irq_gen_w));
}

lpc_acpi_device::lpc_acpi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: lpc_device(mconfig, LPC_ACPI, tag, owner, clock)
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

READ16_MEMBER( lpc_acpi_device::pm1_sts_r)
{
	return pm1_sts;
}

WRITE16_MEMBER(lpc_acpi_device::pm1_sts_w)
{
	pm1_sts &= ~data;
	logerror("%s: pm1_sts = %04x\n", tag(), pm1_sts);
}

READ16_MEMBER( lpc_acpi_device::pm1_en_r)
{
	return pm1_en;
}

WRITE16_MEMBER(lpc_acpi_device::pm1_en_w)
{
	COMBINE_DATA(&pm1_en);
	logerror("%s: pm1_en = %04x\n", tag(), pm1_en);
}

READ32_MEMBER( lpc_acpi_device::pm1_cnt_r)
{
	return pm1_cnt;
}

WRITE32_MEMBER(lpc_acpi_device::pm1_cnt_w)
{
	COMBINE_DATA(&pm1_cnt);
	logerror("%s: pm1_cnt = %08x\n", tag(), pm1_cnt);
}

READ32_MEMBER(lpc_acpi_device::pm1_tmr_r)
{
	return machine().time().as_ticks(3579545) & 0xffffff;
}

READ32_MEMBER( lpc_acpi_device::proc_cnt_r)
{
	return proc_cnt;
}

WRITE32_MEMBER(lpc_acpi_device::proc_cnt_w)
{
	COMBINE_DATA(&proc_cnt);
	logerror("%s: proc_cnt = %08x\n", tag(), proc_cnt);
}

READ8_MEMBER(  lpc_acpi_device::lv2_r)
{
	return 0x00;
}

READ32_MEMBER( lpc_acpi_device::gpe0_sts_r)
{
	return gpe0_sts;
}

WRITE32_MEMBER(lpc_acpi_device::gpe0_sts_w)
{
	COMBINE_DATA(&gpe0_sts);
	logerror("%s: gpe0_sts = %08x\n", tag(), gpe0_sts);
}

READ32_MEMBER( lpc_acpi_device::gpe0_en_r)
{
	return gpe0_en;
}

WRITE32_MEMBER(lpc_acpi_device::gpe0_en_w)
{
	COMBINE_DATA(&gpe0_en);
	logerror("%s: gpe0_en = %08x\n", tag(), gpe0_en);
}

READ32_MEMBER( lpc_acpi_device::smi_en_r)
{
	return smi_en;
}

WRITE32_MEMBER(lpc_acpi_device::smi_en_w)
{
	COMBINE_DATA(&smi_en);
	logerror("%s: smi_en = %08x\n", tag(), smi_en);
}

READ32_MEMBER( lpc_acpi_device::smi_sts_r)
{
	return smi_sts;
}

WRITE32_MEMBER(lpc_acpi_device::smi_sts_w)
{
	COMBINE_DATA(&smi_sts);
	logerror("%s: smi_sts = %08x\n", tag(), smi_sts);
}

READ16_MEMBER( lpc_acpi_device::alt_gp_smi_en_r)
{
	return alt_gp_smi_en;
}

WRITE16_MEMBER(lpc_acpi_device::alt_gp_smi_en_w)
{
	COMBINE_DATA(&alt_gp_smi_en);
	logerror("%s: alt_gp_smi_en = %04x\n", tag(), alt_gp_smi_en);
}

READ16_MEMBER( lpc_acpi_device::alt_gp_smi_sts_r)
{
	return alt_gp_smi_sts;
}

WRITE16_MEMBER(lpc_acpi_device::alt_gp_smi_sts_w)
{
	COMBINE_DATA(&alt_gp_smi_sts);
	logerror("%s: alt_gp_smi_sts = %04x\n", tag(), alt_gp_smi_sts);
}

READ16_MEMBER( lpc_acpi_device::devact_sts_r)
{
	return devact_sts;
}

WRITE16_MEMBER(lpc_acpi_device::devact_sts_w)
{
	COMBINE_DATA(&devact_sts);
	logerror("%s: devact_sts = %04x\n", tag(), devact_sts);
}

READ16_MEMBER( lpc_acpi_device::devtrap_en_r)
{
	return devtrap_en;
}

WRITE16_MEMBER(lpc_acpi_device::devtrap_en_w)
{
	COMBINE_DATA(&devtrap_en);
	logerror("%s: devtrap_en = %04x\n", tag(), devtrap_en);
}

READ16_MEMBER( lpc_acpi_device::bus_addr_track_r)
{
	logerror("%s: read bus_addr_track\n", tag());
	return 0;
}

READ8_MEMBER(  lpc_acpi_device::bus_cyc_track_r)
{
	logerror("%s: read bus_cyc_track\n", tag());
	return 0;
}


READ8_MEMBER(  lpc_acpi_device::tco_rld_r)
{
	return tco_rld;
}

WRITE8_MEMBER( lpc_acpi_device::tco_rld_w)
{
	tco_rld = data;
	logerror("%s: tco_rld = %02x\n", tag(), tco_rld);
}

READ8_MEMBER(  lpc_acpi_device::tco_tmr_r)
{
	return tco_tmr;
}

WRITE8_MEMBER( lpc_acpi_device::tco_tmr_w)
{
	tco_tmr = data;
	logerror("%s: tco_tmr = %02x\n", tag(), tco_tmr);
}

READ8_MEMBER(  lpc_acpi_device::tco_dat_in_r)
{
	return tco_dat_in;
}

WRITE8_MEMBER( lpc_acpi_device::tco_dat_in_w)
{
	tco_dat_in = data;
	logerror("%s: tco_dat_in = %02x\n", tag(), tco_dat_in);
}

READ8_MEMBER(  lpc_acpi_device::tco_dat_out_r)
{
	return tco_dat_out;
}

WRITE8_MEMBER( lpc_acpi_device::tco_dat_out_w)
{
	tco_dat_out = data;
	logerror("%s: tco_dat_out = %02x\n", tag(), tco_dat_out);
}

READ16_MEMBER( lpc_acpi_device::tco1_sts_r)
{
	return tco1_sts;
}

WRITE16_MEMBER(lpc_acpi_device::tco1_sts_w)
{
	COMBINE_DATA(&tco1_sts);
	logerror("%s: tco1_sts = %04x\n", tag(), tco1_sts);
}

READ16_MEMBER( lpc_acpi_device::tco2_sts_r)
{
	return tco2_sts;
}

WRITE16_MEMBER(lpc_acpi_device::tco2_sts_w)
{
	COMBINE_DATA(&tco2_sts);
	logerror("%s: tco2_sts = %04x\n", tag(), tco2_sts);
}

READ16_MEMBER( lpc_acpi_device::tco1_cnt_r)
{
	return tco1_cnt;
}

WRITE16_MEMBER(lpc_acpi_device::tco1_cnt_w)
{
	COMBINE_DATA(&tco1_cnt);
	logerror("%s: tco1_cnt = %04x\n", tag(), tco1_cnt);
}

READ16_MEMBER( lpc_acpi_device::tco2_cnt_r)
{
	return tco2_cnt;
}

WRITE16_MEMBER(lpc_acpi_device::tco2_cnt_w)
{
	COMBINE_DATA(&tco2_cnt);
	logerror("%s: tco2_cnt = %04x\n", tag(), tco2_cnt);
}

READ8_MEMBER(  lpc_acpi_device::tco_message1_r)
{
	return tco_message1;
}

WRITE8_MEMBER( lpc_acpi_device::tco_message1_w)
{
	tco_message1 = data;
	logerror("%s: tco_message1 = %02x\n", tag(), tco_message1);
}

READ8_MEMBER(  lpc_acpi_device::tco_message2_r)
{
	return tco_message2;
}

WRITE8_MEMBER( lpc_acpi_device::tco_message2_w)
{
	tco_message2 = data;
	logerror("%s: tco_message2 = %02x\n", tag(), tco_message2);
}

READ8_MEMBER(  lpc_acpi_device::tco_wdstatus_r)
{
	return tco_wdstatus;
}

WRITE8_MEMBER( lpc_acpi_device::tco_wdstatus_w)
{
	tco_wdstatus = data;
	logerror("%s: tco_wdstatus = %02x\n", tag(), tco_wdstatus);
}

READ8_MEMBER(  lpc_acpi_device::sw_irq_gen_r)
{
	return sw_irq_gen;
}

WRITE8_MEMBER( lpc_acpi_device::sw_irq_gen_w)
{
	sw_irq_gen = data;
	logerror("%s: sw_irq_gen = %02x\n", tag(), sw_irq_gen);
}
