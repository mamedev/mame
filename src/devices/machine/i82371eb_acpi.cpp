// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

    PIIX4E ACPI interface

    TODO:
    - PIIX4 / PIIX4M dispatches

**************************************************************************************************/

#include "emu.h"
#include "i82371eb_acpi.h"

#define LOG_IO     (1U << 1) // log PCI register accesses
#define LOG_TODO   (1U << 2) // log unimplemented registers
#define LOG_MAP    (1U << 3) // log full remaps (verbose)
#define LOG_ACPI   (1U << 4) // log ACPI internals
#define LOG_ACPIEX (1U << 5) // verbose ACPI internals

#define VERBOSE (LOG_GENERAL | LOG_IO | LOG_TODO | LOG_ACPI | LOG_ACPIEX)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGIO(...)     LOGMASKED(LOG_IO,   __VA_ARGS__)
#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)
#define LOGTODO(...)   LOGMASKED(LOG_TODO, __VA_ARGS__)
#define LOGACPI(...)   LOGMASKED(LOG_ACPI, __VA_ARGS__)
#define LOGACPIEX(...) LOGMASKED(LOG_ACPIEX, __VA_ARGS__)

DEFINE_DEVICE_TYPE(I82371EB_ACPI, i82371eb_acpi_device, "i82371eb_acpi", "Intel 82371EB PIIX4E Power Management and ACPI")
DEFINE_DEVICE_TYPE(ACPI_PIIX4, acpi_piix4_device, "acpi_piix4", "ACPI PIIX4")

i82371eb_acpi_device::i82371eb_acpi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, I82371EB_ACPI, tag, owner, clock)
	, m_acpi(*this, "acpi")
	, m_smbus(*this, "smbus")
	, m_apmc_en_w(*this)
{
	// 0x068000 - Bridge devices, other bridge device
	// rev 0x02 for PIIX4E A-0, rev 0x03 for PIIX4M
	set_ids(0x80867113, 0x02, 0x068000, 0x00);
}

void i82371eb_acpi_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x40, 0xd7).unmaprw();
	map(0x40, 0xd7).rw(FUNC(i82371eb_acpi_device::unmap_log_r), FUNC(i82371eb_acpi_device::unmap_log_w));
	// I/O space
	map(0x40, 0x43).rw(FUNC(i82371eb_acpi_device::pmba_r), FUNC(i82371eb_acpi_device::pmba_w));
	map(0x58, 0x5b).rw(FUNC(i82371eb_acpi_device::devactb_r), FUNC(i82371eb_acpi_device::devactb_w));
	map(0x5c, 0x5f).rw(FUNC(i82371eb_acpi_device::devresa_r), FUNC(i82371eb_acpi_device::devresa_w));
	map(0x80, 0x80).rw(FUNC(i82371eb_acpi_device::pmregmisc_r), FUNC(i82371eb_acpi_device::pmregmisc_w));
	// SMBus space
	map(0x90, 0x93).rw(FUNC(i82371eb_acpi_device::smbba_r), FUNC(i82371eb_acpi_device::smbba_w));
	map(0xd2, 0xd2).rw(FUNC(i82371eb_acpi_device::smbhstcfg_r), FUNC(i82371eb_acpi_device::smbhstcfg_w));
}



void i82371eb_acpi_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
//  printf("%08llx %08llx %08llx %04llx %04llx %04llx\n", memory_window_start, memory_window_end, memory_offset ,io_window_start, io_window_end, io_offset);
	if (io_offset != 0)
		throw emu_fatalerror("I82371EB_ACPI io_offset != 0 (%04llx)", io_offset);

	//LOGMAP("PMIOSE %s\n", m_pmiose ? "Enable" : "Disable");

	if (m_pmiose)
	{
		LOGMAP("- PMBA %04x-%04x\n", m_pmba, m_pmba + 0x3f);
		io_space->install_device(m_pmba, m_pmba | 0x3f, *m_acpi, &acpi_piix4_device::map);
	}

	const bool iose = bool(BIT(command, 0));

	LOGMAP("IOSE (SMBus) %s\n", m_pmiose ? "Enable" : "Disable");

	// presume if SMB_HST_EN is zero will also remove SMBUS mapping
	if (iose && BIT(m_smbus_host_config, 0))
	{
		LOGMAP("- SMBBA %04x-%04x (%08x %08x)\n", m_smbba, m_smbba + 0xf, io_window_start, io_window_end);
		io_space->install_device(m_smbba, m_smbba | 0xf, *m_smbus, &smbus_device::map);
	}
}

void i82371eb_acpi_device::device_start()
{
	pci_device::device_start();

	status = 0x0280;
	command = 0;
	command_mask = 1;

	// INTA#
	intr_pin = 1;

#if 0
	memory_window_start = 0;
	memory_window_end   = 0xffffffff;
	memory_offset       = 0;
	io_window_start = 0;
	io_window_end   = 0xffff;
	io_offset       = 0;
#endif
}


void i82371eb_acpi_device::device_reset()
{
	pci_device::device_reset();

	command = 0x0000;
	status = 0x0280;
	m_pmiose = false;
	m_pmba = 0;
	m_smbba = 0;
	m_devresa = 0;
}

u8 i82371eb_acpi_device::pmregmisc_r()
{
	return m_pmiose;
}

void i82371eb_acpi_device::pmregmisc_w(u8 data)
{
	m_pmiose = bool(BIT(data, 0));
	remap_cb();
}

u32 i82371eb_acpi_device::pmba_r()
{
	// RTE bit 0 high (I/O space)
	return m_pmba | 1;
}

void i82371eb_acpi_device::pmba_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pmba);
	m_pmba &= 0xffc0;
	remap_cb();
}

u32 i82371eb_acpi_device::devactb_r()
{
	return m_devactb;
}

void i82371eb_acpi_device::devactb_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_devactb);
	LOGIO("devactb w %08x\n", m_devactb);
	if (ACCESSING_BITS_24_31)
		m_apmc_en_w(BIT(data, 25));
//  remap_cb();
}


u32 i82371eb_acpi_device::devresa_r()
{
	return m_devresa;
}

void i82371eb_acpi_device::devresa_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_devresa);
	LOGIO("devresa w %08x\n", m_devresa);
//  remap_cb();
}

u32 i82371eb_acpi_device::smbba_r()
{
	// RTE bit 0 high (I/O space)
	return m_smbba | 1;
}

void i82371eb_acpi_device::smbba_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_smbba);
	m_smbba &= 0xfff0;
	remap_cb();
}

u8 i82371eb_acpi_device::smbhstcfg_r()
{
	return m_smbus_host_config;
}

void i82371eb_acpi_device::smbhstcfg_w(u8 data)
{
	m_smbus_host_config = data;
	remap_cb();
}

/*
 * Debugging
 */

u8 i82371eb_acpi_device::unmap_log_r(offs_t offset)
{
	LOGTODO("I82371EB_ACPI Unemulated [%02x] R\n", offset + 0x40);
	return 0;
}

void i82371eb_acpi_device::unmap_log_w(offs_t offset, u8 data)
{
	LOGTODO("I82371EB_ACPI Unemulated [%02x] %02x W\n", offset + 0x40, data);
}

/*
 * ACPI Power Management internals
 */

acpi_piix4_device::acpi_piix4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ACPI_PIIX4, tag, owner, clock)
{
}

void acpi_piix4_device::device_start()
{
	save_item(NAME(m_pmsts));
	save_item(NAME(m_pmen));
	save_item(NAME(m_pmcntrl));
	save_item(NAME(m_gpsts));
	save_item(NAME(m_gpen));
	save_item(NAME(m_pcntrl));
	save_item(NAME(m_glbsts));
	save_item(NAME(m_devsts));
	save_item(NAME(m_glben));
	save_item(NAME(m_glbctl));
	save_item(NAME(m_devctl));
	save_item(NAME(m_gporeg));

}

void acpi_piix4_device::device_reset()
{
	m_pmsts = 0;
	m_pmen = 0;
	m_pmcntrl = 0;
	m_gpsts = 0;
	m_gpen = 0;
	m_pcntrl = 0;
	m_glbsts = 0;
	m_devsts[0] = m_devsts[1] = 0;
	m_glben = 0;
	m_glbctl[0] = m_glbctl[1] = 0;
	m_devctl[0] = m_devctl[1] = 0;

	m_gporeg[0] = 0x7fff;
	m_gporeg[1] = 0xbfff;
}

void acpi_piix4_device::device_validity_check(validity_checker &valid) const
{
	if (!this->clock())
		osd_printf_error("%s: clock set to 0 MHz, please use implicit default of 3.5 MHz in config setter instead\n", this->tag());
}

void acpi_piix4_device::map(address_map &map)
{
	// Power Management Status
	map(0x00, 0x01).lrw16(
		NAME([this] () { return m_pmsts; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			m_pmsts &= ~(data & 0x8d31);
		})
	);
	// Power Management Resume Enable
	map(0x02, 0x03).lrw16(
		NAME([this] () { return m_pmen; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_pmen);
			LOGACPI("PMEN: %04x & %04x\n", data, mem_mask);
			LOGACPIEX("\tRTC_EN %d PWRBTN_EN %d GBL_EN %d TMROF_EN %d\n"
				, BIT(data, 10)
				, BIT(data, 8)
				, BIT(data, 5)
				, BIT(data, 0)
			);
		})
	);
	// Power Management Control
	map(0x04, 0x05).lrw16(
		NAME([this] (offs_t offset) { return m_pmcntrl; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_pmcntrl);
			// TODO: SUS_EN cannot be '1'
			// (generates a suspend mode if enabled, flips to '0')
			LOGACPI("PMCNTRL: %04x & %04x\n", data, mem_mask);
			LOGACPIEX("\tSUS_EN %d SUS_TYPE %d GBL_RLS %d BRLD_EN_BM %d SCI_EN %d\n"
				, BIT(data, 13)
				, (data >> 10) & 7
				, BIT(data, 2)
				, BIT(data, 1)
				, BIT(data, 0)
			);
		})
	);
	// Power Management Timer
	map(0x08, 0x0b).lr16(
		NAME([this] (offs_t offset) {
			const u32 tmr_val = machine().time().as_ticks(clock());
			// TODO: resets with PCI reset
			// TODO: sets TMROF_STS to 1 on bit 23 transitions, generates a SCI irq with TMROF_EN
			LOGACPI("PMTMR%d: %08x\n", offset, tmr_val);
			if (offset)
				return (tmr_val >> 16) & 0xff;

			return tmr_val & 0xffff;
		})
	);
	// General Purpose Status
	map(0x0c, 0x0d).lrw16(
		NAME([this] () { return m_gpsts; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			m_gpsts &= ~(data & 0xf81);
		})
	);
	// General Purpose Enable
	map(0x0e, 0x0f).lrw16(
		NAME([this] (offs_t offset) { return m_gpen; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_gpen);
			LOGACPI("GPEN: %04x & %04x\n", data, mem_mask);
			LOGACPIEX("\tLID_EN %d RI_EN %d GPI_EN %d USB_EN %d THRM_EN %d\n"
				, BIT(data, 11)
				, BIT(data, 10)
				, BIT(data, 9)
				, BIT(data, 8)
				, BIT(data, 0)
			);
		})
	);
	// Processor Control (I/O)
	map(0x10, 0x11).lr16(NAME([this] () {
		// CC_STS (r/o)
		if (!machine().side_effects_disabled())
			LOGACPI("PCNTRL: CC_STS read\n");
		return 0x0002;
	}));
	map(0x12, 0x13).lrw16(
		NAME([this] () { return m_pcntrl; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_pcntrl);
			LOGACPI("PCNTRL: %04x & %04x\n", data, mem_mask);
			LOGACPIEX("\tCLKRUN_EN %d STPCLK_EN %d SLEEP_EN %d BST_EN %d CC_EN %d THT_EN %d THTL_DTY %f%\n"
				, BIT(data, 13)
				, BIT(data, 12)
				, BIT(data, 11)
				, BIT(data, 10)
				, BIT(data, 9)
				, BIT(data, 4)
				// NOTE: setting 0 <reserved>
				, ((data >> 1) & 7) * 12.5
			);
		})
	);
	// Processor Level 2/3
	map(0x14, 0x15).lr16(
		NAME([this] (offs_t offset, u16 mem_mask) {
			if (!machine().side_effects_disabled())
			{
				LOGACPI("PLVL%d read\n", offset + 2);
				// Must be byte access
				if (mem_mask == 0xffff)
					LOG("\tInvalid word access!\n");
			}
			return 0;
		})
	);
	// Global Status
	map(0x18, 0x19).lrw16(
		NAME([this] () { return m_glbsts; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			// ---- x--- ---- ---- IRQ Resume Status
			// ---- -x-- ---- ---- External SMI Status (wc) EXTSMI# asserted
			// ---- ---x ---- ---- Global Standby Status (wc)
			if (ACCESSING_BITS_8_15)
			{
				// IRQ_RMS_STS
				m_glbsts &= ~(0x0800);
				m_glbsts |= (BIT(data, 11) << 11);
				// EXTSMI_STS
				if (BIT(data, 10))
					m_glbsts &= ~(1 << 10);
				// GSTBY_STS
				if (BIT(data, 8))
					m_glbsts &= ~(1 << 8);
			}
			// ---- ---- x--- ---- GP Status (r/o) '1' if any GPSTS bits goes high
			// ---- ---- -x-- ---- PM1 Status (r/o) '1' if any PMSTS bits goes high
			// ---- ---- --x- ---- APM Status (wc)
			// ---- ---- ---x ---- All Devices Status (r/o), '1' if any DEVSTS bits goes high
			// ---- ---- ---- -x-- P4MA Status (wc)
			// ---- ---- ---- --x- Legacy USB Status (wc)
			// ---- ---- ---- ---x BIOS Status (wc)
			if (ACCESSING_BITS_0_7)
			{
				m_glbsts &= ~(data & 0x27);
			}
			LOG("GBLSTS %04x & %04x -> %04x\n", data, mem_mask, m_glbsts);
		})
	);
	// Device Status
	// [1] TRP_STS_DEV Trap Status Bits
	// [0] IDL_STS_DEV Idle Status Bits
	map(0x1c, 0x1f).lrw16(
		NAME([this] (offs_t offset) { return m_devsts[offset]; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			const u16 wc_mask = offset ? 0x3fff : 0xfff;
			m_devsts[offset] &= ~(data & wc_mask);
		})
	);
	// Global Enable
	map(0x20, 0x21).lrw16(
		NAME([this] () { return m_glben; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_glben);
			LOGACPI("GLBEN %04x & %04x\n");
			LOGACPIEX("\tBATLOW_EN %d IRQ_RSM_EN %d EXTSMI_EN %d GSTBY_EN %d P4MA_EN %d BM_TRP_EN %d BIOS_EN %d LEGACY_USB_EN %d\n"
				, BIT(data, 15)
				, BIT(data, 11)
				, BIT(data, 10)
				, BIT(data, 8)
				, BIT(data, 4)
				, BIT(data, 3)
				, BIT(data, 1)
				, BIT(data, 0)
			);
		})
	);
	// Global Control
	map(0x28, 0x2b).lrw16(
		NAME([this] (offs_t offset) { return m_glbctl[offset]; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_glbctl[offset]);
			LOGACPI("GLBCTL%d %04x & %04x\n", offset, data, mem_mask);
			if (offset)
			{
				// NOTE: EOS gets automatically cleared on SMI#
				LOGACPIEX("\tGSTBY_SELB %d LID_POL %d SM_FREEZE %d EOS %d\n"
					, BIT(data, 10)
					, BIT(data, 9)
					, BIT(data, 8)
					, BIT(data, 0)
				);
			}
			else
			{
				LOGACPIEX("\tGSTBY_CNT %d GSTBY_SELA %d THRM_POL %d RIOS_RLS %d SMI_EN %d\n"
					, (data >> 9) & 0x7f
					, BIT(data, 8)
					, BIT(data, 2)
					, BIT(data, 1)
					, BIT(data, 0)
				);
			}
			// GSTBY
			// BA
			// 11 4 seconds
			// 10 4 msec
			// 01 4 minutes
			// 00 32 seconds (default)
		})
	);
	// Device Control
	// bits 31:28 <reserved>
	map(0x2c, 0x2f).lrw16(
		NAME([this] (offs_t offset) { return m_devctl[offset]; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_devctl[offset]);
			LOGACPI("DEVCTL%d %04x & %04x\n", offset, data, mem_mask);
		})
	);
	// General Purpose Input
	// bits 23:22 <reserved>, [0x33] not hooked up (assume reserved too)
	map(0x30, 0x33).lr16(
		NAME([this] (offs_t offset, u16 mem_mask) {
			if (!machine().side_effects_disabled())
			{
				LOGACPI("GPIREG%d read\n", offset);
				if (mem_mask == 0xffff)
					LOG("\tInvalid word access!\n");
			}
			return 0;
		})
	);
	map(0x34, 0x37).lrw16(
		NAME([this] (offs_t offset, u16 mem_mask) {
			if (!machine().side_effects_disabled())
			{
				LOGACPI("GPOREG%d read\n", offset);
				if (mem_mask == 0xffff)
					LOG("\tInvalid word access!\n");
			}
			return m_gporeg[offset];
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_gporeg[offset]);
			LOGACPI("GPOREG%d write %04x & %04x\n", offset, data, mem_mask);
			if (mem_mask == 0xffff)
				LOG("\tInvalid word access!\n");
		})
	);
}
