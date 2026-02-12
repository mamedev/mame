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

#define VERBOSE (LOG_GENERAL | LOG_IO | LOG_TODO | LOG_MAP)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGIO(...)     LOGMASKED(LOG_IO,   __VA_ARGS__)
#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)
#define LOGTODO(...)   LOGMASKED(LOG_TODO, __VA_ARGS__)

DEFINE_DEVICE_TYPE(I82371EB_ACPI, i82371eb_acpi_device, "i82371eb_acpi", "Intel 82371EB PIIX4E Power Management and ACPI")

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

void i82371eb_acpi_device::device_add_mconfig(machine_config &config)
{

}

void i82371eb_acpi_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	// TODO: has interrupt pin
	map(0x10, 0xd7).unmaprw();
	map(0x10, 0xd7).rw(FUNC(i82371eb_acpi_device::unmap_log_r), FUNC(i82371eb_acpi_device::unmap_log_w));
	// I/O space
	map(0x40, 0x43).rw(FUNC(i82371eb_acpi_device::pmba_r), FUNC(i82371eb_acpi_device::pmba_w));
	map(0x58, 0x5b).rw(FUNC(i82371eb_acpi_device::devactb_r), FUNC(i82371eb_acpi_device::devactb_w));
	map(0x5c, 0x5f).rw(FUNC(i82371eb_acpi_device::devresa_r), FUNC(i82371eb_acpi_device::devresa_w));
	map(0x80, 0x80).rw(FUNC(i82371eb_acpi_device::pmregmisc_r), FUNC(i82371eb_acpi_device::pmregmisc_w));
	// SMBus space
	map(0x90, 0x93).rw(FUNC(i82371eb_acpi_device::smbba_r), FUNC(i82371eb_acpi_device::smbba_w));
	map(0xd2, 0xd2).rw(FUNC(i82371eb_acpi_device::smbhstcfg_r), FUNC(i82371eb_acpi_device::smbhstcfg_w));
}

void i82371eb_acpi_device::io_map(address_map &map)
{
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
		// TODO: subset, should map up to 0x3f only (and current lpc-acpi don't)
		m_acpi->map_device(memory_window_start, memory_window_end, 0, memory_space, io_window_start, m_pmba + 0x3f, m_pmba, io_space);
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
	LOGTODO("I82371EB_ACPI Unemulated [%02x] R\n", offset + 0x10);
	return 0;
}

void i82371eb_acpi_device::unmap_log_w(offs_t offset, u8 data)
{
	LOGTODO("I82371EB_ACPI Unemulated [%02x] %02x W\n", offset + 0x10, data);
}
