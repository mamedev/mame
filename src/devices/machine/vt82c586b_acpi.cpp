// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

VT82C586B PCIC ACPI section

APM v1.2 and ACPI v0.9

TODO:
- Earlier 3040E variant uses BAR4 for register space;
- ls5ampv3 pclass programming looks buggy:
  documentation claims 61h reprograms 09h, 62h -> 0ah and 63h -> 0bh
  What actually happens:
  63h: pclass write 06 -> pclass 068000
  62h: pclass write 00 -> pclass 060000 <- would reprogram the ACPI as Host Bridge.
  Notice that the BIOS also tries to read from non-existant dev number 3.x, mapping PIPC there
  will just miss programming ACPI entirely (including its I/O space).
  So in order to avoid problems we knock off bit 7 clearance, definitely needs to be tested on HW.

**************************************************************************************************/

#include "emu.h"
#include "vt82c586b_acpi.h"

#define LOG_ACPI   (1U << 1) // log ACPI internals
#define LOG_ACPIEX (1U << 2) // verbose ACPI internals

#define VERBOSE (LOG_GENERAL | LOG_ACPI | LOG_ACPIEX)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGACPI(...)   LOGMASKED(LOG_ACPI, __VA_ARGS__)
#define LOGACPIEX(...) LOGMASKED(LOG_ACPIEX, __VA_ARGS__)

DEFINE_DEVICE_TYPE(VT82C586B_ACPI, vt82c586b_acpi_device, "vt82c586b_acpi", "VT82C586B \"PIPC\" Power Management and ACPI")
DEFINE_DEVICE_TYPE(ACPI_PIPC, acpi_pipc_device, "acpi_pipc", "ACPI PIPC")

vt82c586b_acpi_device::vt82c586b_acpi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, VT82C586B_ACPI, tag, owner, clock)
	, m_acpi(*this, "acpi")
{
	// xxxx ---- Silicon Version Code
	// ---- xxxx Silicon Revision Code
	// 0x00 3040E OEM Version Rev. E
	// 0x01 3040F OEM Version Rev. F
	// 0x10 3041A Production Version
	// pclass writeable thru registers $61 ~ $63
	set_ids(0x11063040, 0x10, 0x068000, 0x00000000);
}

void vt82c586b_acpi_device::device_start()
{
	pci_device::device_start();

	save_item(NAME(m_pin_config));
	save_item(NAME(m_general_config));
	save_item(NAME(m_sci_irq_config));
	save_item(NAME(m_iobase));
	save_item(NAME(m_irq_channel));
}


void vt82c586b_acpi_device::device_reset()
{
	pci_device::device_reset();

	command = 0x0000;
	// doc claims unable to do anything, guess at least I/O space is still possible
	command_mask = 1;
	// medium DEVSEL#, Fast Back to Back
	status = 0x0280;

	m_pin_config = 0xc0;
	m_general_config = 0;
	m_sci_irq_config = 0;
	// undefined, use ls5ampv3 default
	m_iobase = 0x5000;
	std::fill(std::begin(m_irq_channel), std::end(m_irq_channel), 0);
	remap_cb();
}

void vt82c586b_acpi_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	// latency timer
	map(0x0d, 0x0d).lr8(NAME([] () { return 0x16; }));

	map(0x40, 0x40).lrw8(
		NAME([this] () { return m_pin_config; }),
		NAME([this] (offs_t offset, u8 data) {
			m_pin_config = data & 0xc0;
			LOG("40h: Pin Configuration %02x\n", data);
		})
	);
	map(0x41, 0x41).lrw8(
		NAME([this] () { return m_general_config; }),
		NAME([this] (offs_t offset, u8 data) {
			m_general_config = data & 0xce;
			LOG("41h: General Configuration %02x\n", data);
			remap_cb();
		})
	);
	map(0x42, 0x42).lrw8(
		NAME([this] () { return m_sci_irq_config; }),
		NAME([this] (offs_t offset, u8 data) {
			m_sci_irq_config = data & 0xf;
			LOG("42h: SCI Interrupt Configuration %02x\n", data);
		})
	);
	map(0x44, 0x47).lrw16(
		NAME([this] (offs_t offset) { return m_irq_channel[offset]; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_irq_channel[offset]);
			m_irq_channel[offset] &= ~4;
			LOG("%02Xh: %s Interrupt Channel %08x & %08x\n"
				, (offset * 2) + 0x44
				, offset ? "Secondary" : "Primary"
				, data
				, mem_mask
			);
		})
	);
	map(0x48, 0x4b).lrw32(
		NAME([this] () { return m_iobase | 1; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_iobase);
			m_iobase &= 0xff00;
			if (ACCESSING_BITS_8_15)
			{
				LOG("48h: IOBASE %04x\n", m_iobase);
				remap_cb();
			}
		})
	);
	map(0x50, 0x53).lrw32(
		NAME([this] () { return m_gp_timer_control; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_gp_timer_control);
			LOG("50h: GP Timer Control %08x & %08x\n"
				, data
				, mem_mask
			);
		})
	);
	map(0x61, 0x63).lw8(
		NAME([this] (offs_t offset, u8 data) {
			switch(offset)
			{
				case 2:
					pclass &= 0x00ffff;
					pclass |= (data << 16);
					break;
				case 1:
					// HACK: avoid setting an invalid pclass (cfr. note on top)
					pclass &= 0xff80ff;
					pclass |= (data << 8);
					break;
				case 0:
					pclass &= 0xffff00;
					pclass |= (data << 0);
					break;
			}
			LOG("%02Xh: pclass write %02x -> pclass %06x\n", offset + 0x61, data, pclass);
		})
	);
}

void vt82c586b_acpi_device::map_extra(
		uint64_t memory_window_start,
		uint64_t memory_window_end,
		uint64_t memory_offset,
		address_space *memory_space,
		uint64_t io_window_start,
		uint64_t io_window_end,
		uint64_t io_offset,
		address_space *io_space)
{
	if (BIT(m_general_config, 7))
	{
		io_space->install_device(m_iobase & 0xfffe, m_iobase | 0xff, *m_acpi, &acpi_pipc_device::map);
	}
}

/*
 * ACPI Power Management internals
 */

acpi_pipc_device::acpi_pipc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ACPI_PIPC, tag, owner, clock)
{
}

void acpi_pipc_device::device_start()
{
	save_item(NAME(m_pmsts));
	save_item(NAME(m_pmen));
	save_item(NAME(m_pmcntrl));
	save_item(NAME(m_gpsts));
	save_item(NAME(m_gpen));
	save_item(NAME(m_pcntrl));
}

void acpi_pipc_device::device_reset()
{
	m_pmsts = 0;
	m_pmen = 0;
	m_pmcntrl = 0;
	m_gpsts = 0;
	m_gpen = 0;
	m_pcntrl = 0;
}

void acpi_pipc_device::device_validity_check(validity_checker &valid) const
{
	if (!this->clock())
		osd_printf_error("%s: clock set to 0 MHz, please use implicit default of 3.5 MHz in config setter instead\n", this->tag());
}

// TODO: up to offset $15 looks 1:1 with the Intel PIIX4 equivalent
void acpi_pipc_device::map(address_map &map)
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
}
