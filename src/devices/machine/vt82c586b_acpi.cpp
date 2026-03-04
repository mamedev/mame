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
- win98se: hangs on ACPI SCIEN requiring being set during PnP phase install;

**************************************************************************************************/

#include "emu.h"
#include "vt82c586b_acpi.h"

#define LOG_ACPI   (1U << 1) // log ACPI internals
#define LOG_ACPIEX (1U << 2) // verbose ACPI internals
#define LOG_GPIO   (1U << 3)
#define LOG_PMTMR  (1U << 4) // verbose timer reads and processor levels

#define VERBOSE (LOG_GENERAL | LOG_ACPI | LOG_ACPIEX | LOG_GPIO)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGACPI(...)   LOGMASKED(LOG_ACPI, __VA_ARGS__)
#define LOGACPIEX(...) LOGMASKED(LOG_ACPIEX, __VA_ARGS__)
#define LOGGPIO(...)   LOGMASKED(LOG_GPIO, __VA_ARGS__)
#define LOGPMTR(...)   LOGMASKED(LOG_PMTMR, __VA_ARGS__)

DEFINE_DEVICE_TYPE(VT82C586B_ACPI, vt82c586b_acpi_device, "vt82c586b_acpi", "VT82C586B \"PIPC\" Power Management and ACPI")
DEFINE_DEVICE_TYPE(ACPI_PIPC, acpi_pipc_device, "acpi_pipc", "ACPI PIPC")

vt82c586b_acpi_device::vt82c586b_acpi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, VT82C586B_ACPI, tag, owner, clock)
	, m_acpi(*this, "acpi")
	, m_sci_pin_cb(*this)
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
			LOG("42h: SCI Interrupt Configuration %02x (%d)\n", data, data);
			m_sci_pin_cb(m_sci_irq_config);
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
//		io_space->install_device(m_iobase & 0xfffe, m_iobase | 0xff, *m_acpi, &acpi_pipc_device::map);
		m_acpi->map_device(memory_window_start, memory_window_end, 0, memory_space, io_window_start, io_window_end, m_iobase, io_space);
	}
}

/*
 * ACPI Power Management internals
 */

acpi_pipc_device::acpi_pipc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: lpc_device(mconfig, ACPI_PIPC, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_write_smi(*this)
	, m_write_sci(*this)
{
	m_space_config = address_space_config("io_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(acpi_pipc_device::io_map), this));
}

void acpi_pipc_device::device_start()
{
	save_item(NAME(m_pmsts));
	save_item(NAME(m_pmen));
	save_item(NAME(m_pmcntrl));
	save_item(NAME(m_gpsts));
	save_item(NAME(m_gpen));
	save_item(NAME(m_pcntrl));
	save_item(NAME(m_gp_sci_enable));
	save_item(NAME(m_gp_smi_enable));
	save_item(NAME(m_power_supply_control));
	save_item(NAME(m_global_status));
	save_item(NAME(m_global_enable));
	save_item(NAME(m_gbl_ctl));
	save_item(NAME(m_smi_cmd));
	save_item(NAME(m_primary_activity_status));
	save_item(NAME(m_primary_activity_enable));
	save_item(NAME(m_gp_timer_reload_enable));
	save_item(NAME(m_gpio_dir));
	save_item(NAME(m_gpio_val));
	save_item(NAME(m_gpo_val));
}

void acpi_pipc_device::device_reset()
{
	m_pmsts = 0;
	m_pmen = 0;
	m_pmcntrl = 0;
	m_gpsts = 0;
	m_gpen = 0;
	m_pcntrl = 0;
	m_gp_sci_enable = 0;
	m_gp_smi_enable = 0;
	// PB_CTL on by default
	m_power_supply_control = 1 << 9;
	m_global_status = m_global_enable = 0;
	m_gbl_ctl = 0;
	m_smi_cmd = 0;
	m_primary_activity_enable = m_primary_activity_status = 0;
	m_gp_timer_reload_enable = 0;
	m_gpio_dir = m_gpio_val = 0;
	m_gpo_val = 0;
}

void acpi_pipc_device::device_validity_check(validity_checker &valid) const
{
	if (!this->clock())
		osd_printf_error("%s: clock set to 0 MHz, please use implicit default of 3.5 MHz in config setter instead\n", this->tag());
}

device_memory_interface::space_config_vector acpi_pipc_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

void acpi_pipc_device::map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	io_space->install_device(io_offset, io_window_end, *this, &acpi_pipc_device::map, 0xffffffff);
}

// FIXME: trampoline to avoid mapping getting confused and overrides nibbles on 32-bit word units
void acpi_pipc_device::map(address_map &map)
{
	map(0x00, 0xff).lrw8(
		NAME([this] (offs_t offset) { return space(0).read_byte(offset); }),
		NAME([this] (offs_t offset, u8 data) { space(0).write_byte(offset, data); })
	);
}

// Similar but not exactly identical to Intel PIIX4 equivalent
void acpi_pipc_device::io_map(address_map &map)
{
	// Power Management Status
	// x--- ---- ---- ---- WAK_STS
	// ---- x--- ---- ---- PBOR_STS
	// ---- -x-- ---- ---- RTC_STS
	// ---- ---x ---- ---- PB_STS
	// ---- ---- --x- ---- GBL_STS (set by BIOS_RLS)
	// ---- ---- ---x ---- BM_STS (Bus Master)
	// ---- ---- ---- ---x TMR_STS
	map(0x00, 0x01).lrw8(
		NAME([this] (offs_t offset) { return (offset) ? m_pmsts >> 8 : m_pmsts & 0xff; }),
		NAME([this] (offs_t offset, u8 data) {
			if (offset)
			{
				m_pmsts &= ~(data & 0x8d00);
			}
			else
			{
				m_pmsts &= ~(data & 0x31);
				// clear BIOS_RLS
				if (BIT(data, 5))
					m_gbl_ctl &= ~(1 << 5);
			}
		})
	);
	// Power Management Resume Enable
	map(0x02, 0x03).lrw8(
		NAME([this] (offs_t offset) { return (offset) ? m_pmen >> 8 : m_pmen & 0xff; }),
		NAME([this] (offs_t offset, u8 data) {
			LOGACPI("PMEN: [%d] %02x\n", offset, data);
			if (offset)
			{
				m_pmen &= 0x00ff;
				m_pmen |= (data & 0x5) << 8;
			}
			else
			{
				m_pmen &= 0xff00;
				m_pmen |= data & 0x21;
			}
			LOGACPIEX("\tRTC_EN %d PWRBTN_EN %d GBL_EN %d TMROF_EN %d\n"
				, BIT(m_pmen, 10)
				, BIT(m_pmen, 8)
				, BIT(m_pmen, 5)
				, BIT(m_pmen, 0)
			);
		})
	);
	// Power Management Control
	map(0x04, 0x05).lrw8(
		NAME([this] (offs_t offset) { return (offset) ? m_pmcntrl >> 8 : m_pmcntrl & 0xff; }),
		NAME([this] (offs_t offset, u8 data) {
			LOGACPI("PMCNTRL: [%d] %02x\n", offset, data);
			if (offset)
			{
				m_pmcntrl &= 0x00ff;
				m_pmcntrl |= (data & 0x3c) << 8;
				if (BIT(m_pmcntrl, 13))
				{
					const u8 slp_typ = (m_pmcntrl >> 10) & 7;
					LOGACPI("SLP_EN Sleep Enable issued %d\n", slp_typ);
					// TODO: SLP_EN cannot be '1'
					// (generates a suspend mode if enabled, flips to '0')
				}
			}
			else
			{
				m_pmcntrl &= 0xff00;
				m_pmcntrl |= (data & 0x7);
			}

			LOGACPIEX("\tSLP_EN %d SLP_TYP %d GBL_RLS %d BRLD_EN_BM %d SCI_EN %d\n"
				, BIT(m_pmcntrl, 13)
				, (m_pmcntrl >> 10) & 7
				, BIT(m_pmcntrl, 2)
				, BIT(m_pmcntrl, 1)
				, BIT(m_pmcntrl, 0)
			);
		})
	);
	// Power Management Timer
	map(0x08, 0x0b).lr8(
		NAME([this] (offs_t offset) -> u8 {
			const u32 tmr_val = machine().time().as_ticks(clock());
			// TODO: resets with PCI reset
			// TODO: sets TMROF_STS to 1 on bit 23 transitions, generates a SCI irq with TMROF_EN
			// TODO: can be configured with 32-bit resolution in this variant (from PCI ACPI)
			LOGPMTR("PMTMR%d: %08x\n", offset, tmr_val);
			if (offset == 3)
				return 0;

			const unsigned shift = offset * 8;
			return (tmr_val >> shift) & 0xff;
		})
	);
	// General Purpose Enable
	map(0x0e, 0x0f).lrw8(
		NAME([this] (offs_t offset) { return (offset) ? m_gpen >> 8 : m_gpen & 0xff; }),
		NAME([this] (offs_t offset, u8 data) {
			LOGACPI("GPEN: [%d] %02x\n", offset, data);
			if (offset)
			{
				m_gpen &= 0x00ff;
				m_gpen |= (data & 0xf) << 8;
			}
			else
			{
				m_gpen &= 0xff00;
				m_gpen |= (data & 1);
			}
			LOGACPIEX("\tLID_EN %d RI_EN %d GPI_EN %d USB_EN %d THRM_EN %d\n"
				, BIT(m_gpen, 11)
				, BIT(m_gpen, 10)
				, BIT(m_gpen, 9)
				, BIT(m_gpen, 8)
				, BIT(m_gpen, 0)
			);
		})
	);
	// Processor Control (I/O)
	// bits 31-5 are <reserved>
	// has just bits 4-1 compared to Intel equivalent
	// TODO: different on 3040 Silicon
	map(0x10, 0x13).lrw8(
		NAME([this] (offs_t offset) { return (offset) ? 0 : m_pcntrl & 0xff; }),
		NAME([this] (offs_t offset, u8 data) {
			if (offset == 0)
			{
				m_pcntrl &= 0xffff'ff00;
				m_pcntrl |= data & 0x1e;
				LOGACPI("PCNTRL: %02x\n", data);
				LOGACPIEX("\tTHT_EN %d THTL_DTY %f%\n"
					, BIT(m_pcntrl, 4)
					// NOTE: setting 0 <reserved>
					, ((m_pcntrl >> 1) & 7) * 12.5
				);
			}
		})
	);
	// Processor Level 2/3
	map(0x14, 0x15).lr8(
		NAME([this] (offs_t offset, u16 mem_mask) {
			if (!machine().side_effects_disabled())
			{
				LOGPMTR("PLVL%d read\n", offset + 2);
			}
			return 0;
		})
	);

	// start of truly different stuff vs. ACPI_PIIX4 ...

	// General Purpose Status
	// ---- --x- ---- ---- USB_STS
	// ---- ---x ---- ---- RI_STS
	// ---- ---- xxxx xxxx EXTSMI7~0
	map(0x20, 0x21).rw(FUNC(acpi_pipc_device::gpsts_r), FUNC(acpi_pipc_device::gpsts_w));
	map(0x22, 0x23).rw(FUNC(acpi_pipc_device::gp_sci_enable_r), FUNC(acpi_pipc_device::gp_sci_enable_w));
	map(0x24, 0x25).lrw8(
		NAME([this] (offs_t offset) { return (offset) ? m_gp_smi_enable >> 8 : m_gp_smi_enable & 0xff; }),
		NAME([this] (offs_t offset, u8 data) {
			LOGACPI("General Purpose SMI Enable: [%d] %02x\n", offset, data);
			if (offset)
			{
				m_gp_smi_enable &= 0x00ff;
				m_gp_smi_enable |= (data & 3) << 8;
			}
			else
			{
				m_gp_smi_enable &= 0xff00;
				m_gp_smi_enable |= data;
			}
		})
	);
	map(0x26, 0x27).lrw8(
		NAME([this] (offs_t offset) { return (offset) ?  m_power_supply_control >> 8 : m_power_supply_control & 0xff; }),
		NAME([this] (offs_t offset, u8 data) {
			LOGACPI("Power Supply Control: [%d] %02x\n", offset, data);
			if (offset)
			{
				m_power_supply_control &= 0x00ff;
				m_power_supply_control |= (data & 7) << 8;
			}
			else
			{
				m_power_supply_control &= 0xff00;
				m_power_supply_control |= (data & 1);
			}
			LOGACPIEX("\tRI_PS_CTL %d PB_CTL %d RTC_PS_CTL %d E0_PS_CTL\n"
				, BIT(m_power_supply_control, 10)
				, BIT(m_power_supply_control, 9)
				, BIT(m_power_supply_control, 8)
				, BIT(m_power_supply_control, 0)
			);
		})
	);
	// -x-- ---- SW_SMI_STS
	// --x- ---- BIOS_STS
	// ---x ---- LEG_USB_STS
	// ---- x--- GP1TO_STS
	// ---- -x-- GP0TO_STS
	// ---- --x- STTO_STS
	// ---- ---x PACT_STS
	map(0x28, 0x29).lrw8(
		NAME([this] (offs_t offset) { return (offset) ? 0 : m_global_status & 0xff; }),
		NAME([this] (offs_t offset, u8 data) {
			if (!offset)
			{
				m_global_status &= ~(data & 0x7f);
				check_smi();
			}
		})
	);
	map(0x2a, 0x2b).lrw8(
		NAME([this] (offs_t offset) { return (offset) ? 0 : m_global_enable & 0xff; }),
		NAME([this] (offs_t offset, u8 data) {
			if (!offset)
			{
				LOGACPI("Global Enable: %02x\n", data);
				m_global_enable = data & 0x7f;
				LOGACPIEX("\tSW_SMI_EN %d BIOS_EN %d LEG_USB_EN %d GP1TO_EN %d GP0TO_EN %d STTO_EN %d PACT_EN %d\n"
					, BIT(m_global_enable, 6)
					, BIT(m_global_enable, 5)
					, BIT(m_global_enable, 4)
					, BIT(m_global_enable, 3)
					, BIT(m_global_enable, 2)
					, BIT(m_global_enable, 1)
					, BIT(m_global_enable, 0)
				);
			}
		})
	);
	map(0x2c, 0x2d).lrw8(
		NAME([this] (offs_t offset) { return (offset) ? m_gbl_ctl >> 8 : m_gbl_ctl & 0xff; }),
		NAME([this] (offs_t offset, u8 data) {
			LOGACPI("Global Control: [%d] %02x\n", offset, data);
			if (offset)
			{
				m_gbl_ctl &= 0x00ff;
				m_gbl_ctl |= (data & 1) << 8;
			}
			else
			{
				m_gbl_ctl &= 0xff00;
				m_gbl_ctl |= (data & 0x17);

				// set GBL_STS
				if (BIT(m_gbl_ctl, 1))
					m_pmsts |= 1 << 5;
				// refresh for SMI_EN (global SMI enable)
				check_smi();
			}
			LOGACPIEX("\tINSMI %d SMIIG %d Power Button Trigger %d BIOS_RLS %d SMI_EN %d\n"
				, BIT(m_gbl_ctl, 8)
				, BIT(m_gbl_ctl, 4)
				, BIT(m_gbl_ctl, 2)
				, BIT(m_gbl_ctl, 1)
				, BIT(m_gbl_ctl, 0)
			);
		})
	);
	map(0x2f, 0x2f).lrw8(
		NAME([this] (offs_t offset) {
			return m_smi_cmd;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_smi_cmd = data;
			m_global_status |= 1 << 6;
			check_smi();
			LOGACPIEX("SMI_CMD %02x (SW_SMI_EN=%d)\n", data, BIT(m_global_enable, 6));
		})
	);
	map(0x30, 0x33).lrw8(
		NAME([this] (offs_t offset) { return (offset) ? 0 : m_primary_activity_status & 0xff; }),
		NAME([this] (offs_t offset, u8 data) {
			if (!offset)
			{
				m_primary_activity_status &= ~(data & 0xfb);
				// check_activity();
			}
		})
	);
	map(0x34, 0x37).lrw8(
		NAME([this] (offs_t offset) { return (offset) ? 0 : m_primary_activity_enable; }),
		NAME([this] (offs_t offset, u8 data) {
			if (!offset)
			{
				LOGACPI("Primary Activity Enable: %02x\n", data);
				m_primary_activity_enable = data & 0xfb;
				// TODO: set PACT_STS bit 0
				// check_activity();
				LOGACPIEX("\tKBC_EN %d SER_EN %d PAR_EN %d VID_EN %d IDE_EN %d PIRQ_EN %d DRQ_EN %d\n"
					, BIT(m_primary_activity_enable, 7)
					, BIT(m_primary_activity_enable, 6)
					, BIT(m_primary_activity_enable, 5)
					, BIT(m_primary_activity_enable, 4)
					, BIT(m_primary_activity_enable, 3)
					, BIT(m_primary_activity_enable, 1)
					, BIT(m_primary_activity_enable, 0)
				);
			}
		})
	);
	map(0x38, 0x3b).lrw8(
		NAME([this] (offs_t offset) { return (offset) ? 0 : m_gp_timer_reload_enable; }),
		NAME([this] (offs_t offset, u8 data) {
			if (!offset)
			{
				LOGACPI("GP Timer Reload Enable: %02x\n", data);
				m_gp_timer_reload_enable = data & 0xf9;
				LOGACPIEX("\tGP1 Reload = KBC %d SER %d VID %d IDE/Floppy %d | GP0 Reload = Primary %d\n"
					, BIT(m_gp_timer_reload_enable, 7)
					, BIT(m_gp_timer_reload_enable, 6)
					, BIT(m_gp_timer_reload_enable, 4)
					, BIT(m_gp_timer_reload_enable, 3)
					, BIT(m_gp_timer_reload_enable, 0)
				);
			}
		})
	);

	// GPIO
	map(0x40, 0x40).lrw8(
		NAME([this] (offs_t offset) { return m_gpio_dir; }),
		NAME([this] (offs_t offset, u8 data) {
			m_gpio_dir = data & 0x7f;
			LOGGPIO("GPIO Dir: %02x (%02x)\n", data, data & 0x1f);
			LOGGPIO("\tSMI/SCI Event Disable %d Interrupt resume from power on %d\n"
				, BIT(data, 6)
				, BIT(data, 5)
			);
		})
	);
	map(0x42, 0x42).lrw8(
		NAME([this] (offs_t offset) { return m_gpio_val; }),
		NAME([this] (offs_t offset, u8 data) {
			m_gpio_val = data & 0x1f;
			LOGGPIO("GPIO Output: %02x\n", data);
		})
	);
	map(0x44, 0x44).lr8(
		NAME([this] (offs_t offset) {
			if (!machine().side_effects_disabled())
				LOGGPIO("GPIO Input read (EXTSMI_VAL)\n");
			return 0;
		})
	);
	map(0x46, 0x47).lrw8(
		NAME([this] (offs_t offset) { return (offset) ? m_gpo_val >> 8 : m_gpo_val & 0xff; }),
		NAME([this] (offs_t offset, u8 data) {
			LOGGPIO("GPO_VAL [%d] %02x\n", offset, data);
			if (offset)
			{
				m_gpo_val &= 0x00ff;
				m_gpo_val |= data << 8;
			}
			else
			{
				m_gpo_val &= 0xff00;
				m_gpo_val |= data;
			}
		})
	);
	map(0x48, 0x49).lr8(
		NAME([this] (offs_t offset) -> u8 {
			if (!machine().side_effects_disabled())
				LOGGPIO("GPI Port Input read %d (GPI_VAL%d-%d)\n", offset, offset * 8 + 7, offset * 8);
			return 0;
		})
	);
}

void acpi_pipc_device::check_smi()
{
	if (m_global_status & m_global_enable && BIT(m_gbl_ctl, 0))
	{
		m_write_smi(1);
	}
	else
		m_write_smi(0);
}

u8 acpi_pipc_device::gpsts_r(offs_t offset)
{
	return (offset) ? m_gpsts >> 8 : m_gpsts & 0xff;
}

void acpi_pipc_device::gpsts_w(offs_t offset, u8 data)
{
	if (!offset)
		m_gpsts &= ~(data & 0x00ff);
	else
		m_gpsts &= ~((data << 8) & 0x0300);
}

u8 acpi_pipc_device::gp_sci_enable_r(offs_t offset)
{
	return (offset) ? m_gp_sci_enable >> 8 : m_gp_sci_enable & 0xff;
}

void acpi_pipc_device::gp_sci_enable_w(offs_t offset, u8 data)
{
	LOGACPI("General Purpose SCI Enable: [%d] %02x\n", offset, data);
	if (offset)
	{
		m_gp_sci_enable &= 0x00ff;
		m_gp_sci_enable |= (data & 3) << 8;
	}
	else
	{
		m_gp_sci_enable &= 0xff00;
		m_gp_sci_enable |= data;
	}
}

