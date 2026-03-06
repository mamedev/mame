// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

SiS 950 ACPI implementation

Based on lpc-acpi

TODO:
- basically ported over plus the extra register required by SMI to work;
- gamecstl: still fails BIOS boot after entering SMI from here at 0xf3cf2 (on jp $-2)
- win2k based HDDs wants an event at $[50]04 to not throw an ACPI BSoD after a while

**************************************************************************************************/

#include "emu.h"
#include "sis950_acpi.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

DEFINE_DEVICE_TYPE(SIS950_ACPI, sis950_acpi_device, "sis950_acpi", "SiS 950 ACPI interface")

sis950_acpi_device::sis950_acpi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: lpc_device(mconfig, SIS950_ACPI, tag, owner, clock)
	, m_write_smi(*this)
//	, m_write_sci(*this)
{
}

void sis950_acpi_device::device_start()
{
}

void sis950_acpi_device::device_reset()
{
	m_leg_sts = m_leg_en = 0;
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


void sis950_acpi_device::map(address_map &map)
{
	map(0x00, 0x01).rw(FUNC(sis950_acpi_device::pm1_sts_r), FUNC(sis950_acpi_device::pm1_sts_w));
	map(0x02, 0x03).rw(FUNC(sis950_acpi_device::pm1_en_r), FUNC(sis950_acpi_device::pm1_en_w));
	map(0x04, 0x07).rw(FUNC(sis950_acpi_device::pm1_cnt_r), FUNC(sis950_acpi_device::pm1_cnt_w));
	map(0x08, 0x0b).r(FUNC(sis950_acpi_device::pm1_tmr_r));
	map(0x10, 0x13).rw(FUNC(sis950_acpi_device::proc_cnt_r), FUNC(sis950_acpi_device::proc_cnt_w));
	map(0x14, 0x14).r(FUNC(sis950_acpi_device::lv2_r));

	map(0x28, 0x2b).rw(FUNC(sis950_acpi_device::gpe0_sts_r), FUNC(sis950_acpi_device::gpe0_sts_w));
	map(0x2c, 0x2f).rw(FUNC(sis950_acpi_device::gpe0_en_r), FUNC(sis950_acpi_device::gpe0_en_w));
	map(0x30, 0x33).rw(FUNC(sis950_acpi_device::smi_en_r), FUNC(sis950_acpi_device::smi_en_w));
	map(0x34, 0x37).rw(FUNC(sis950_acpi_device::smi_sts_r), FUNC(sis950_acpi_device::smi_sts_w));
	map(0x38, 0x39).rw(FUNC(sis950_acpi_device::alt_gp_smi_en_r), FUNC(sis950_acpi_device::alt_gp_smi_en_w));
	map(0x3a, 0x3b).rw(FUNC(sis950_acpi_device::alt_gp_smi_sts_r), FUNC(sis950_acpi_device::alt_gp_smi_sts_w));

	map(0x40, 0x43).lrw32(
		NAME([this] (offs_t offset) { return m_leg_sts | (m_leg_en << 16); }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_0_7)
				m_leg_sts &= ~(data & 0xfd);
			if (ACCESSING_BITS_8_15)
				m_leg_sts &= ~(data & 0xff);
			if (ACCESSING_BITS_16_31)
			{
				m_leg_en = (data >> 16) & 0x0fff;
				LOG("%s: leg_en = %08x & %08x -> %04x\n", tag(), data, mem_mask, m_leg_en);
			}
			check_smi();
		})
	);
	map(0x44, 0x45).rw(FUNC(sis950_acpi_device::devact_sts_r), FUNC(sis950_acpi_device::devact_sts_w));
	map(0x48, 0x49).rw(FUNC(sis950_acpi_device::devtrap_en_r), FUNC(sis950_acpi_device::devtrap_en_w));
	map(0x4c, 0x4d).r(FUNC(sis950_acpi_device::bus_addr_track_r));
	map(0x4e, 0x4e).r(FUNC(sis950_acpi_device::bus_cyc_track_r));

	map(0x60, 0x60).rw(FUNC(sis950_acpi_device::tco_rld_r), FUNC(sis950_acpi_device::tco_rld_w));
	map(0x61, 0x61).rw(FUNC(sis950_acpi_device::tco_tmr_r), FUNC(sis950_acpi_device::tco_tmr_w));
	map(0x62, 0x62).rw(FUNC(sis950_acpi_device::tco_dat_in_r), FUNC(sis950_acpi_device::tco_dat_in_w));
	map(0x63, 0x63).rw(FUNC(sis950_acpi_device::tco_dat_out_r), FUNC(sis950_acpi_device::tco_dat_out_w));
	map(0x64, 0x65).rw(FUNC(sis950_acpi_device::tco1_sts_r), FUNC(sis950_acpi_device::tco1_sts_w));
	map(0x66, 0x67).rw(FUNC(sis950_acpi_device::tco2_sts_r), FUNC(sis950_acpi_device::tco2_sts_w));
	map(0x68, 0x69).rw(FUNC(sis950_acpi_device::tco1_cnt_r), FUNC(sis950_acpi_device::tco1_cnt_w));
	map(0x6a, 0x6b).rw(FUNC(sis950_acpi_device::tco2_cnt_r), FUNC(sis950_acpi_device::tco2_cnt_w));
	map(0x6c, 0x6c).rw(FUNC(sis950_acpi_device::tco_message1_r), FUNC(sis950_acpi_device::tco_message1_w));
	map(0x6d, 0x6d).rw(FUNC(sis950_acpi_device::tco_message2_r), FUNC(sis950_acpi_device::tco_message2_w));
	map(0x6e, 0x6e).rw(FUNC(sis950_acpi_device::tco_wdstatus_r), FUNC(sis950_acpi_device::tco_wdstatus_w));
	map(0x70, 0x70).rw(FUNC(sis950_acpi_device::sw_irq_gen_r), FUNC(sis950_acpi_device::sw_irq_gen_w));
}

void sis950_acpi_device::map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	io_space->install_device(io_offset, io_window_end, *this, &sis950_acpi_device::map);
}


uint16_t sis950_acpi_device::pm1_sts_r()
{
	return pm1_sts;
}

void sis950_acpi_device::pm1_sts_w(uint16_t data)
{
	pm1_sts &= ~data;
	LOG("%s: pm1_sts = %04x\n", tag(), pm1_sts);
}

uint16_t sis950_acpi_device::pm1_en_r()
{
	return pm1_en;
}

void sis950_acpi_device::pm1_en_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&pm1_en);
	LOG("%s: pm1_en = %04x\n", tag(), pm1_en);
}

uint32_t sis950_acpi_device::pm1_cnt_r()
{
	return pm1_cnt;
}

void sis950_acpi_device::pm1_cnt_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&pm1_cnt);
	LOG("%s: pm1_cnt = %08x\n", tag(), pm1_cnt);
}

uint32_t sis950_acpi_device::pm1_tmr_r()
{
	return machine().time().as_ticks(3579545) & 0xffffff;
}

uint32_t sis950_acpi_device::proc_cnt_r()
{
	return proc_cnt;
}

void sis950_acpi_device::proc_cnt_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&proc_cnt);
	LOG("%s: proc_cnt = %08x\n", tag(), proc_cnt);
}

uint8_t sis950_acpi_device::lv2_r()
{
	return 0x00;
}

uint32_t sis950_acpi_device::gpe0_sts_r()
{
	return gpe0_sts;
}

void sis950_acpi_device::gpe0_sts_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&gpe0_sts);
	LOG("%s: gpe0_sts = %08x\n", tag(), gpe0_sts);
}

uint32_t sis950_acpi_device::gpe0_en_r()
{
	return gpe0_en;
}

void sis950_acpi_device::gpe0_en_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&gpe0_en);
	LOG("%s: gpe0_en = %08x\n", tag(), gpe0_en);
}

uint32_t sis950_acpi_device::smi_en_r()
{
	return smi_en;
}

void sis950_acpi_device::smi_en_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&smi_en);
	LOG("%s: smi_en = %08x\n", tag(), smi_en);
}

uint32_t sis950_acpi_device::smi_sts_r()
{
	return smi_sts;
}

void sis950_acpi_device::smi_sts_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&smi_sts);
	LOG("%s: smi_sts = %08x\n", tag(), smi_sts);
}

uint16_t sis950_acpi_device::alt_gp_smi_en_r()
{
	return alt_gp_smi_en;
}

void sis950_acpi_device::alt_gp_smi_en_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&alt_gp_smi_en);
	LOG("%s: alt_gp_smi_en = %04x\n", tag(), alt_gp_smi_en);
}

uint16_t sis950_acpi_device::alt_gp_smi_sts_r()
{
	return alt_gp_smi_sts;
}

void sis950_acpi_device::alt_gp_smi_sts_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&alt_gp_smi_sts);
	LOG("%s: alt_gp_smi_sts = %04x\n", tag(), alt_gp_smi_sts);
}

uint16_t sis950_acpi_device::devact_sts_r()
{
	return devact_sts;
}

void sis950_acpi_device::devact_sts_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		devact_sts &= ~(data & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		devact_sts &= ~(data & 0xcf);
	}
	//LOG("%s: devact_sts = %04x\n", tag(), devact_sts);
}

uint16_t sis950_acpi_device::devtrap_en_r()
{
	return devtrap_en;
}

// TODO: 48h is SMICMD_PORT, 49h is MAIL_BOX, 4A / 4Bh are SW watchdog related
void sis950_acpi_device::devtrap_en_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&devtrap_en);
	LOG("%s: devtrap_en = %04x & %04x\n", tag(), devtrap_en, mem_mask);
	if (ACCESSING_BITS_0_7)
	{
		m_leg_sts |= 1 << 5;
		check_smi();
	}
}

uint16_t sis950_acpi_device::bus_addr_track_r()
{
	LOG("%s: read bus_addr_track\n", tag());
	return 0;
}

uint8_t sis950_acpi_device::bus_cyc_track_r()
{
	LOG("%s: read bus_cyc_track\n", tag());
	return 0;
}


uint8_t sis950_acpi_device::tco_rld_r()
{
	return tco_rld;
}

void sis950_acpi_device::tco_rld_w(uint8_t data)
{
	tco_rld = data;
	LOG("%s: tco_rld = %02x\n", tag(), tco_rld);
}

uint8_t sis950_acpi_device::tco_tmr_r()
{
	return tco_tmr;
}

void sis950_acpi_device::tco_tmr_w(uint8_t data)
{
	tco_tmr = data;
	LOG("%s: tco_tmr = %02x\n", tag(), tco_tmr);
}

uint8_t sis950_acpi_device::tco_dat_in_r()
{
	return tco_dat_in;
}

void sis950_acpi_device::tco_dat_in_w(uint8_t data)
{
	tco_dat_in = data;
	LOG("%s: tco_dat_in = %02x\n", tag(), tco_dat_in);
}

uint8_t sis950_acpi_device::tco_dat_out_r()
{
	return tco_dat_out;
}

void sis950_acpi_device::tco_dat_out_w(uint8_t data)
{
	tco_dat_out = data;
	LOG("%s: tco_dat_out = %02x\n", tag(), tco_dat_out);
}

uint16_t sis950_acpi_device::tco1_sts_r()
{
	return tco1_sts;
}

void sis950_acpi_device::tco1_sts_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&tco1_sts);
	LOG("%s: tco1_sts = %04x\n", tag(), tco1_sts);
}

uint16_t sis950_acpi_device::tco2_sts_r()
{
	return tco2_sts;
}

void sis950_acpi_device::tco2_sts_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&tco2_sts);
	LOG("%s: tco2_sts = %04x\n", tag(), tco2_sts);
}

uint16_t sis950_acpi_device::tco1_cnt_r()
{
	return tco1_cnt;
}

void sis950_acpi_device::tco1_cnt_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&tco1_cnt);
	LOG("%s: tco1_cnt = %04x\n", tag(), tco1_cnt);
}

uint16_t sis950_acpi_device::tco2_cnt_r()
{
	return tco2_cnt;
}

void sis950_acpi_device::tco2_cnt_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&tco2_cnt);
	LOG("%s: tco2_cnt = %04x\n", tag(), tco2_cnt);
}

uint8_t sis950_acpi_device::tco_message1_r()
{
	return tco_message1;
}

void sis950_acpi_device::tco_message1_w(uint8_t data)
{
	tco_message1 = data;
	LOG("%s: tco_message1 = %02x\n", tag(), tco_message1);
}

uint8_t sis950_acpi_device::tco_message2_r()
{
	return tco_message2;
}

void sis950_acpi_device::tco_message2_w(uint8_t data)
{
	tco_message2 = data;
	LOG("%s: tco_message2 = %02x\n", tag(), tco_message2);
}

uint8_t sis950_acpi_device::tco_wdstatus_r()
{
	return tco_wdstatus;
}

void sis950_acpi_device::tco_wdstatus_w(uint8_t data)
{
	tco_wdstatus = data;
	LOG("%s: tco_wdstatus = %02x\n", tag(), tco_wdstatus);
}

uint8_t sis950_acpi_device::sw_irq_gen_r()
{
	return sw_irq_gen;
}

void sis950_acpi_device::sw_irq_gen_w(uint8_t data)
{
	sw_irq_gen = data;
	LOG("%s: sw_irq_gen = %02x\n", tag(), sw_irq_gen);
}

void sis950_acpi_device::check_smi()
{
	if (m_leg_sts & m_leg_en && BIT(m_leg_en, 0))
	{
		m_write_smi(1);
	}
	else
		m_write_smi(0);
}
