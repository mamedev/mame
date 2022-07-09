// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

    SiS 5513 IDE controller

    TODO:
    - Derive from common pci-ide.cpp interface
      (what flavour that emulates tho? PCI regs 0x40-0x52 don't match)

**************************************************************************************************/

#include "emu.h"
#include "sis5513_ide.h"

#define LOG_IO     (1U << 1) // log PCI register accesses
#define LOG_TODO   (1U << 2) // log unimplemented registers
#define LOG_MAP    (1U << 3) // log full remaps

#define VERBOSE (LOG_GENERAL | LOG_IO | LOG_TODO | LOG_MAP)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGIO(...)     LOGMASKED(LOG_IO,   __VA_ARGS__)
#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)
#define LOGTODO(...)   LOGMASKED(LOG_TODO, __VA_ARGS__)

DEFINE_DEVICE_TYPE(SIS5513_IDE, sis5513_ide_device, "sis5513_ide", "SiS 5513 IDE Controller")

sis5513_ide_device::sis5513_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, SIS5513_IDE, tag, owner, clock)
	, m_ide1(*this, "ide1")
	, m_ide2(*this, "ide2")
	, m_irq_pri_callback(*this)
	, m_irq_sec_callback(*this)
	, m_bus_master_space(*this, finder_base::DUMMY_TAG, AS_PROGRAM)
{
}

void sis5513_ide_device::device_add_mconfig(machine_config &config)
{
	BUS_MASTER_IDE_CONTROLLER(config, m_ide1).options(ata_devices, "hdd", nullptr, false);
	m_ide1->irq_handler().set([this](int state) { m_irq_pri_callback(state); });
	m_ide1->set_bus_master_space(m_bus_master_space);

	BUS_MASTER_IDE_CONTROLLER(config, m_ide2).options(ata_devices, "cdrom", nullptr, false);
	m_ide2->irq_handler().set([this](int state) { m_irq_sec_callback(state); });
	m_ide2->set_bus_master_space(m_bus_master_space);
}

void sis5513_ide_device::config_map(address_map &map)
{
	pci_device::config_map(map);
//  map(0x10, 0x4f).unmaprw();
	map(0x09, 0x09).w(FUNC(sis5513_ide_device::prog_if_w));
	// base I/O relocation (effective only when native mode is on)
//  map(0x10, 0x13) Primary channel command base
//  map(0x14, 0x17) Primary channel control base
//  map(0x18, 0x1b) Secondary channel command base
//  map(0x1c, 0x1f) Secondary channel control base

//  map(0x20, 0x23) Bus master IDE control register base (0x10 I/O regs)
	map(0x10, 0x23).rw(FUNC(sis5513_ide_device::bar_r), FUNC(sis5513_ide_device::bar_w));
	map(0x24, 0x27).unmaprw();
	map(0x40, 0x52).rw(FUNC(sis5513_ide_device::unmap_log_r), FUNC(sis5513_ide_device::unmap_log_w));
	map(0x4a, 0x4a).rw(FUNC(sis5513_ide_device::ide_ctrl_0_r), FUNC(sis5513_ide_device::ide_ctrl_0_w));
	map(0x52, 0x52).rw(FUNC(sis5513_ide_device::ide_misc_ctrl_r), FUNC(sis5513_ide_device::ide_misc_ctrl_w));


//  map(0x2c, 0x2f) subsystem ID (written once)

//  map(0x3c, 0x3d) interrupt line/pin

//  map(0x40, 0x40) IDE Primary channel master data recovery time
//  map(0x41, 0x41) IDE Primary channel master data active time (Ultra DMA)
//  map(0x42, 0x42) IDE Primary channel slave data recovery time
//  map(0x43, 0x43) IDE Primary channel slave data active time (Ultra DMA)
//  map(0x44, 0x47) ^ Same for IDE Secondary

//  map(0x48, 0x48) IDE status
//  map(0x4a, 0x4b) IDE general control regs

//  map(0x4c, 0x4d) prefetch count of primary channel
//  map(0x4e, 0x4f) prefetch count of secondary channel

//  map(0x52, 0x52) IDE misc control regs
}

#if 0
void sis5513_ide_device::compatible_io_map(address_map &map)
{
	map(0x0170, 0x0177).rw(FUNC(sis5513_ide_device::ide2_read32_cs0_r), FUNC(sis5513_ide_device::ide2_write32_cs0_w));
	map(0x01f0, 0x01f7).rw(FUNC(sis5513_ide_device::ide1_read32_cs0_r), FUNC(sis5513_ide_device::ide1_write32_cs0_w));
	map(0x0376, 0x0376).rw(FUNC(sis5513_ide_device::ide2_read_cs1_r), FUNC(sis5513_ide_device::ide2_write_cs1_w));
	map(0x03f6, 0x03f6).rw(FUNC(sis5513_ide_device::ide1_read_cs1_r), FUNC(sis5513_ide_device::ide1_write_cs1_w));
}
#endif

// $1f0
void sis5513_ide_device::ide1_command_map(address_map &map)
{
	map(0, 7).rw(FUNC(sis5513_ide_device::ide1_read32_cs0_r), FUNC(sis5513_ide_device::ide1_write32_cs0_w));
}

// $3f4
void sis5513_ide_device::ide1_control_map(address_map &map)
{
	map(2, 2).rw(FUNC(sis5513_ide_device::ide1_read_cs1_r), FUNC(sis5513_ide_device::ide1_write_cs1_w));
}

// $170
void sis5513_ide_device::ide2_command_map(address_map &map)
{
	map(0, 7).rw(FUNC(sis5513_ide_device::ide2_read32_cs0_r), FUNC(sis5513_ide_device::ide2_write32_cs0_w));
}

// $374
void sis5513_ide_device::ide2_control_map(address_map &map)
{
	map(2, 2).rw(FUNC(sis5513_ide_device::ide2_read_cs1_r), FUNC(sis5513_ide_device::ide2_write_cs1_w));
}

void sis5513_ide_device::bus_master_ide_control_map(address_map &map)
{
	map(0x0, 0x7).rw(m_ide1, FUNC(bus_master_ide_controller_device::bmdma_r), FUNC(bus_master_ide_controller_device::bmdma_w));
	map(0x8, 0xf).rw(m_ide2, FUNC(bus_master_ide_controller_device::bmdma_r), FUNC(bus_master_ide_controller_device::bmdma_w));
}

void sis5513_ide_device::device_start()
{
	pci_device::device_start();

	m_irq_pri_callback.resolve();
	m_irq_sec_callback.resolve();

	add_map(8, M_IO, FUNC(sis5513_ide_device::ide1_command_map));
	add_map(4, M_IO, FUNC(sis5513_ide_device::ide1_control_map));
	add_map(8, M_IO, FUNC(sis5513_ide_device::ide2_command_map));
	add_map(4, M_IO, FUNC(sis5513_ide_device::ide2_control_map));
	add_map(16, M_IO, FUNC(sis5513_ide_device::bus_master_ide_control_map));
}


void sis5513_ide_device::device_reset()
{
	pci_device::device_reset();

	command = 0x0000;
	status = 0x0000;
	pclass = 0x01018a;
	m_ide_ctrl0 = 0;
	m_ide_misc = 0;
	m_bar[0] = 0x1f0;
	m_bar[1] = 0x3f4;
	m_bar[2] = 0x170;
	m_bar[3] = 0x374;
	m_bar[4] = 0xf00;
	for (int i = 0; i < 5; i++)
		bank_infos[i].adr = m_bar[i];
	remap_cb();
}

inline bool sis5513_ide_device::ide1_mode()
{
	return (pclass & 0x3) == 3;
}

inline bool sis5513_ide_device::ide2_mode()
{
	return (pclass & 0xc) == 0xc;
}

// In compatible mode BARs with legacy addresses but can values written can still be readout.
// In practice we need to override writes and make sure we flush remapping accordingly
inline void sis5513_ide_device::flush_ide_mode()
{
	// Map Primary IDE Channel
	if (ide1_mode())
	{
		// PCI Mode
		pci_device::address_base_w(0, m_bar[0]);
		pci_device::address_base_w(1, m_bar[1]);
	}
	else
	{
		// Legacy Mode
		pci_device::address_base_w(0, 0x1f0);
		pci_device::address_base_w(1, 0x3f4);
	}

	// Map Secondary IDE Channel
	if (ide2_mode())
	{
		// PCI Mode
		pci_device::address_base_w(2, m_bar[2]);
		pci_device::address_base_w(3, m_bar[3]);
	}
	else
	{
		// Legacy Mode
		pci_device::address_base_w(2, 0x170);
		pci_device::address_base_w(3, 0x374);
	}
}

void sis5513_ide_device::prog_if_w(u8 data)
{
	uint32_t oldVal = pclass;
	pclass = (pclass & ~(0xff)) | (data & 0xff);
	// Check for switch to/from compatibility (legacy) mode from/to pci mode
	if ((oldVal ^ pclass) & 0xf)
		flush_ide_mode();
}

u32 sis5513_ide_device::bar_r(offs_t offset)
{
	if (bank_reg_infos[offset].bank == -1)
		return 0;
	int bid = bank_reg_infos[offset].bank;
	if (bank_reg_infos[offset].hi)
		return bank_infos[bid].adr >> 32;
	int flags = bank_infos[bid].flags;
	return (m_bar[offset] & ~(bank_infos[bid].size - 1)) | (flags & M_IO ? 1 : 0) | (flags & M_64A ? 4 : 0) | (flags & M_PREF ? 8 : 0);
}

void sis5513_ide_device::bar_w(offs_t offset, u32 data)
{
	m_bar[offset] = data;
	// Bits 0 (primary) and 2 (secondary) control if the mapping is legacy or BAR
	switch (offset) {
	case 0:
	case 1:
		if (ide1_mode())
			pci_device::address_base_w(offset, data);
		break;
	case 2:
	case 3:
		if (ide2_mode())
			pci_device::address_base_w(offset, data);
		break;
	default:
		// Only the first 4 bars are controlled by pif
		pci_device::address_base_w(offset, data);
	}
	logerror("Mapping bar[%i] = %08x\n", offset, data);
}

u8 sis5513_ide_device::ide_ctrl_0_r()
{
	LOGIO("IDE ctrl 0 read [$4a] %02x\n", m_ide_ctrl0);
	return m_ide_ctrl0;
}

void sis5513_ide_device::ide_ctrl_0_w(u8 data)
{
	LOGIO("IDE ctrl 0 write [$4a] %02x\n", data);
	m_ide_ctrl0 = data;
	// TODO: bit 1 disables IDE ch. 0, bit 2 ch. 1
//  remap_cb();
}

u8 sis5513_ide_device::ide_misc_ctrl_r()
{
	LOGIO("IDE misc ctrl read [$52] %02x\n", m_ide_misc);
	return m_ide_misc;
}

void sis5513_ide_device::ide_misc_ctrl_w(u8 data)
{
	LOGIO("IDE misc ctrl write [$52] %02x\n", data);
	m_ide_misc = data;

	const bool compatible_mode = BIT(m_ide_misc, 2);
	pclass &= 0xffff85;

	if (compatible_mode)
	{
		//LOGMAP("- Compatible Mode\n");
		intr_pin = 0;
	}
	else
	{
		// Native Mode
		pclass |= 0xa;
		intr_pin = 1;
	}

	flush_ide_mode();
}

/*
 * Debugging
 */

u8 sis5513_ide_device::unmap_log_r(offs_t offset)
{
	LOGTODO("IDE Unemulated [%02x] R\n", offset + 0x40);
	return 0;
}

void sis5513_ide_device::unmap_log_w(offs_t offset, u8 data)
{
	LOGTODO("IDE Unemulated [%02x] %02x W\n", offset + 0x40, data);
}

/*
 * Start of legacy handling, to be moved out
 */

uint32_t sis5513_ide_device::ide1_read32_cs0_r(offs_t offset, uint32_t mem_mask)
{
	if (!(command & 1))
		return 0xffffffff;
	return m_ide1->read_cs0(offset, mem_mask);
}

void sis5513_ide_device::ide1_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (!(command & 1))
		return;
	m_ide1->write_cs0(offset, data, mem_mask);
}

uint32_t sis5513_ide_device::ide2_read32_cs0_r(offs_t offset, uint32_t mem_mask)
{
	if (!(command & 1))
		return 0xffffffff;
	return m_ide2->read_cs0(offset, mem_mask);
}

void sis5513_ide_device::ide2_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (!(command & 1))
		return;
	m_ide2->write_cs0(offset, data, mem_mask);
}

uint8_t sis5513_ide_device::ide1_read_cs1_r()
{
	if (!(command & 1))
		return 0xff;
	return m_ide1->read_cs1(1, 0xff0000) >> 16;
}

void sis5513_ide_device::ide1_write_cs1_w(uint8_t data)
{
	if (!(command & 1))
		return;
	m_ide1->write_cs1(1, data << 16, 0xff0000);
}

uint8_t sis5513_ide_device::ide2_read_cs1_r()
{
	if (!(command & 1))
		return 0xff;
	return m_ide2->read_cs1(1, 0xff0000) >> 16;
}

void sis5513_ide_device::ide2_write_cs1_w(uint8_t data)
{
	if (!(command & 1))
		return;
	m_ide2->write_cs1(1, data << 16, 0xff0000);
}
