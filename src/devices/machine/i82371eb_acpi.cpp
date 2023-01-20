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
#define LOG_MAP    (1U << 3) // log full remaps

#define VERBOSE (LOG_GENERAL | LOG_IO | LOG_TODO | LOG_MAP)
#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGIO(...)     LOGMASKED(LOG_IO,   __VA_ARGS__)
#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)
#define LOGTODO(...)   LOGMASKED(LOG_TODO, __VA_ARGS__)

DEFINE_DEVICE_TYPE(I82371EB_ACPI, i82371eb_acpi_device, "i82371eb_acpi", "Intel 82371EB PIIX4E Power Management and ACPI")

i82371eb_acpi_device::i82371eb_acpi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, I82371EB_ACPI, tag, owner, clock)

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
	map(0x40, 0x43).lrw32(
		NAME([this] () { return address_base_r(0); }), 
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { address_base_w(0, data); })
	);
	// SMBus space
	map(0x90, 0x93).lrw32(
		NAME([this] () { return address_base_r(1); }), 
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { address_base_w(1, data); })
	);
}

void i82371eb_acpi_device::io_map(address_map &map)
{

}

// TODO: convert to lpc-smbus
void i82371eb_acpi_device::smbus_map(address_map &map)
{
	map(0x00, 0x00).lr8(NAME([]() { return 0x02; }));
}

void i82371eb_acpi_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
//	io_space->install_device(0, 0x03ff, *this, &i82371eb_acpi_device::io_map);
}

void i82371eb_acpi_device::device_start()
{
	pci_device::device_start();

	// TODO: verify size of these
	add_map(512, M_IO, FUNC(i82371eb_acpi_device::io_map));
	add_map(2048, M_IO, FUNC(i82371eb_acpi_device::smbus_map));

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
