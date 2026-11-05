// license:BSD-3-Clause
// copyright-holders:MrBaristaNZ
/**************************************************************************************************

    PLX PCI9054 PCI I/O accelerator

    A PCI function (class 0x068000, "other bridge") with a local bus. Emulated is the direct-master
    side, where a local bus CPU reaches PCI through the 9054 (as on the Aristocrat MK6, an SH-4 board
    with no host bridge: the PCI root is set up hostless and the 9054 masters the bus):
    - the local configuration registers (local offsets 0x000-0x1ff; the PCI configuration header is
      at local 0x000-0x03f): PCICR/PCISR, LMISC1 and the direct-master registers
    - direct-master memory window: local address -> PCI memory, remapped by DMRR/DMPBAM
    - direct-master I/O window: PCI I/O, or a type 0/1 configuration cycle when DMCFGA bit 31 is set;
      a configuration cycle no device claims sets PCISR "received master abort"

    The local base addresses (DMLBAM/DMLBAI) are stored but not decoded: the board maps the windows,
    and the handlers receive window-relative offsets.

    Reference: PLX PCI 9054 Data Book, version 1.0 (1998)

    TODO:
    - PCIBAR0-3: the PCI side view of the registers and the direct-slave (PCI to local) spaces
    - the other local configuration registers, including the PCI header seen from the local side
    - mailboxes, doorbells, INTCSR and the interrupts
    - DMA channels and the messaging queue
    - serial EEPROM loading of the configuration registers
    - IDs: revision 0x0b (AB silicon) and the subsystem IDs are the power-on defaults without an EEPROM

**************************************************************************************************/

#include "emu.h"
#include "pci9054.h"

#define LOG_REGS    (1U << 1)
#define LOG_WINDOW  (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"

#define LOGREGS(...)    LOGMASKED(LOG_REGS, __VA_ARGS__)
#define LOGWINDOW(...)  LOGMASKED(LOG_WINDOW, __VA_ARGS__)


DEFINE_DEVICE_TYPE(PCI9054, pci9054_device, "pci9054", "PLX PCI9054 PCI I/O Accelerator")

namespace {

// local offsets of the registers this device implements
enum : offs_t
{
	PCICR    = 0x004 / 4,   // PCICR (bits 15-0) / PCISR (bits 31-16)
	LMISC    = 0x08c / 4,   // BIGEND, LMISC1 (bits 15-8), PROT_AREA, LMISC2
	DMRR     = 0x09c / 4,
	DMLBAM   = 0x0a0 / 4,
	DMLBAI   = 0x0a4 / 4,
	DMPBAM   = 0x0a8 / 4,
	DMCFGA   = 0x0ac / 4
};

// PCISR bits that are write-1-to-clear: parity error, target aborts, master abort, system error
constexpr u16 PCISR_W1C = 0xf900;
constexpr u16 PCISR_RECEIVED_MASTER_ABORT = 0x2000;

} // anonymous namespace


pci9054_device::pci9054_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, PCI9054, tag, owner, clock)
{
	// "other bridge device"; power-on defaults without a serial EEPROM, revision 0x0b = AB silicon
	set_ids(0x10b59054, 0x0b, 0x068000, 0x10b59054);
}

void pci9054_device::device_start()
{
	pci_device::device_start();

	save_item(NAME(m_lmisc1));
	save_item(NAME(m_dmrr));
	save_item(NAME(m_dmlbam));
	save_item(NAME(m_dmlbai));
	save_item(NAME(m_dmpbam));
	save_item(NAME(m_dmcfga));
}

void pci9054_device::device_reset()
{
	pci_device::device_reset();

	command = 0x0000;
	status = 0x0290;   // capabilities list, fast back-to-back capable, medium DEVSEL# timing
	m_lmisc1 = 0;
	m_dmrr = 0;
	m_dmlbam = 0;
	m_dmlbai = 0;
	m_dmpbam = 0;
	m_dmcfga = 0;
}

void pci9054_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x06, 0x07).w(FUNC(pci9054_device::pcisr_w));
}

void pci9054_device::pcisr_w(offs_t offset, u16 data, u16 mem_mask)
{
	status &= ~(data & mem_mask & PCISR_W1C);
}


/**************************************************************************************************
    Local configuration registers
**************************************************************************************************/

void pci9054_device::log_unimplemented_read(offs_t offset, u32 mem_mask)
{
	if (!machine().side_effects_disabled())
		logerror("%s: read from unimplemented register %03x & %08x\n", machine().describe_context(), offset << 2, mem_mask);
}

void pci9054_device::log_unimplemented_write(offs_t offset, u32 data, u32 mem_mask)
{
	logerror("%s: write to unimplemented register %03x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
}

u32 pci9054_device::local_r(offs_t offset, u32 mem_mask)
{
	offset &= 0x1ff / 4;
	switch (offset)
	{
	case PCICR:
		return (u32(status) << 16) | command;

	case LMISC:
		if (mem_mask & ~0x0000ff00)
			log_unimplemented_read(offset, mem_mask & ~0x0000ff00);   // BIGEND, PROT_AREA, LMISC2
		return u32(m_lmisc1) << 8;

	case DMRR:
		return m_dmrr;

	case DMLBAM:
		return m_dmlbam;

	case DMLBAI:
		return m_dmlbai;

	case DMPBAM:
		return m_dmpbam;

	case DMCFGA:
		return m_dmcfga;

	default:
		log_unimplemented_read(offset, mem_mask);
		return 0;
	}
}

void pci9054_device::local_w(offs_t offset, u32 data, u32 mem_mask)
{
	offset &= 0x1ff / 4;
	switch (offset)
	{
	case PCICR:
		if (ACCESSING_BITS_0_15)
			command_w(0, data, mem_mask);
		if (ACCESSING_BITS_16_31)
			pcisr_w(0, data >> 16, mem_mask >> 16);
		break;

	case LMISC:
		if (ACCESSING_BITS_8_15)
		{
			// local miscellaneous control 1: stored only
			m_lmisc1 = data >> 8;
			LOGREGS("%s: LMISC1 = %02x\n", machine().describe_context(), m_lmisc1);
		}
		if (mem_mask & ~0x0000ff00)
			log_unimplemented_write(offset, data, mem_mask & ~0x0000ff00);   // BIGEND, PROT_AREA, LMISC2
		break;

	case DMRR:
		// direct-master range: the local address bits the windows pass through (bits 15-0 reserved)
		COMBINE_DATA(&m_dmrr);
		m_dmrr &= 0xffff0000;
		LOGREGS("%s: DMRR = %08x\n", machine().describe_context(), m_dmrr);
		break;

	case DMLBAM:
		// local base address of the memory window: stored only, the board decodes the window
		COMBINE_DATA(&m_dmlbam);
		m_dmlbam &= 0xffff0000;
		LOGREGS("%s: DMLBAM = %08x\n", machine().describe_context(), m_dmlbam);
		break;

	case DMLBAI:
		// local base address of the I/O and configuration window: stored only, the board decodes the window
		COMBINE_DATA(&m_dmlbai);
		m_dmlbai &= 0xffff0000;
		LOGREGS("%s: DMLBAI = %08x\n", machine().describe_context(), m_dmlbai);
		break;

	case DMPBAM:
		// bit 0 memory enable, bit 1 I/O enable, bit 13 I/O remap select, bits 31-16 the PCI address bits
		// replacing the local address bits that DMRR does not pass through
		COMBINE_DATA(&m_dmpbam);
		LOGREGS("%s: DMPBAM = %08x\n", machine().describe_context(), m_dmpbam);
		break;

	case DMCFGA:
		// bit 31 configuration enable, bits 23-16 bus, 15-8 device and function, 7-2 register,
		// 1-0 type (bits 30-24 reserved)
		COMBINE_DATA(&m_dmcfga);
		m_dmcfga &= 0x80ffffff;
		LOGREGS("%s: DMCFGA = %08x\n", machine().describe_context(), m_dmcfga);
		break;

	default:
		log_unimplemented_write(offset, data, mem_mask);
		break;
	}
}


/**************************************************************************************************
    Direct-master windows
**************************************************************************************************/

offs_t pci9054_device::dm_address(offs_t offset, bool io) const
{
	const u32 address = (offset << 2) & ~m_dmrr;
	if (io && BIT(m_dmpbam, 13))
		return address & 0xffff;   // I/O remap select: PCI address bits 31-16 forced to 0
	return (m_dmpbam & m_dmrr) | address;
}

u32 pci9054_device::dm_mem_r(offs_t offset, u32 mem_mask)
{
	if (!BIT(m_dmpbam, 0))
	{
		if (!machine().side_effects_disabled())
			logerror("%s: direct-master memory read %08x with memory access disabled\n", machine().describe_context(), offset << 2);
		return 0;
	}
	return get_pci_busmaster_space()->read_dword(dm_address(offset, false), mem_mask);
}

void pci9054_device::dm_mem_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (!BIT(m_dmpbam, 0))
	{
		logerror("%s: direct-master memory write %08x = %08x with memory access disabled\n", machine().describe_context(), offset << 2, data);
		return;
	}
	get_pci_busmaster_space()->write_dword(dm_address(offset, false), data, mem_mask);
}

// decode DMCFGA for a configuration cycle; false (and master abort) if no device claims it
bool pci9054_device::config_cycle(u8 &bus, u8 &devfn, u16 &reg)
{
	bus = BIT(m_dmcfga, 0) ? BIT(m_dmcfga, 16, 8) : 0;   // type 1 or type 0
	devfn = BIT(m_dmcfga, 8, 8);
	reg = m_dmcfga & 0xfc;

	if (!m_pci_root->config_claimed(bus, devfn))
	{
		if (!machine().side_effects_disabled())
		{
			LOGWINDOW("%s: configuration cycle %02x:%02x.%x:%02x master abort\n", machine().describe_context(), bus, devfn >> 3, devfn & 7, reg);
			status |= PCISR_RECEIVED_MASTER_ABORT;
		}
		return false;
	}
	return true;
}

u32 pci9054_device::dm_io_r(offs_t offset, u32 mem_mask)
{
	if (!BIT(m_dmpbam, 1))
	{
		if (!machine().side_effects_disabled())
			logerror("%s: direct-master I/O read %08x with I/O access disabled\n", machine().describe_context(), offset << 2);
		return 0;
	}

	if (BIT(m_dmcfga, 31))
	{
		u8 bus, devfn;
		u16 reg;
		if (!config_cycle(bus, devfn, reg))
			return 0xffffffff;
		return m_pci_root->config_read(bus, devfn, reg, mem_mask);
	}

	address_space *const io = get_pci_busmaster_io_space();
	if (!io)
	{
		if (!machine().side_effects_disabled())
			logerror("%s: direct-master I/O read %08x: no PCI I/O space (bus has a host bridge)\n", machine().describe_context(), offset << 2);
		return 0;
	}
	return io->read_dword(dm_address(offset, true), mem_mask);
}

void pci9054_device::dm_io_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (!BIT(m_dmpbam, 1))
	{
		logerror("%s: direct-master I/O write %08x = %08x with I/O access disabled\n", machine().describe_context(), offset << 2, data);
		return;
	}

	if (BIT(m_dmcfga, 31))
	{
		u8 bus, devfn;
		u16 reg;
		if (config_cycle(bus, devfn, reg))
			m_pci_root->config_write(bus, devfn, reg, data, mem_mask);
		return;
	}

	address_space *const io = get_pci_busmaster_io_space();
	if (!io)
	{
		logerror("%s: direct-master I/O write %08x = %08x: no PCI I/O space (bus has a host bridge)\n", machine().describe_context(), offset << 2, data);
		return;
	}
	io->write_dword(dm_address(offset, true), data, mem_mask);
}
