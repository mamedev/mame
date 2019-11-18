// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Intel 82439TX System Controller (MTXC)

***************************************************************************/

#include "emu.h"
#include "i82439tx.h"

//#define VERBOSE

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

DEFINE_DEVICE_TYPE(I82439TX_LEGACY, i82439tx_device, "i82439tx_legacy", "Intel 82439TX")


i82439tx_device::i82439tx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	northbridge_device(mconfig, I82439TX_LEGACY, tag, owner, clock),
	pci_device_interface(mconfig, *this),
	m_cpu_tag(nullptr),
	m_region_tag(nullptr),
	m_space(nullptr),
	m_rom(nullptr)
{
	m_smram.smiact_n = 1;
}

void i82439tx_device::i82439tx_configure_memory(uint8_t val, offs_t begin, offs_t end)
{
	switch (val & 0x03)
	{
	case 0:
		m_space->install_rom(begin, end, m_rom + (begin - 0xc0000));
		m_space->nop_write(begin, end);
		break;
	case 1:
		m_space->install_rom(begin, end, m_bios_ram + (begin - 0xc0000) / 4);
		m_space->nop_write(begin, end);
		break;
	case 2:
		m_space->install_rom(begin, end, m_rom + (begin - 0xc0000));
		m_space->install_writeonly(begin, end, m_bios_ram + (begin - 0xc0000) / 4);
		break;
	case 3:
		m_space->install_ram(begin, end, m_bios_ram + (begin - 0xc0000) / 4);
		break;
	}
}


/***************************************************************************
    PCI INTERFACE
***************************************************************************/

uint32_t i82439tx_device::pci_read(pci_bus_device *pcibus, int function, int offset, uint32_t mem_mask)
{
	uint32_t result;

#ifdef VERBOSE
	logerror("i82439tx_pci_read: read from function %d offset %08X mask %08X\n", function, offset, mem_mask);
#endif
	if (function != 0)
		return 0;

	switch(offset)
	{
		case 0x00:  // vendor/device ID
			result = 0x71008086;
			break;

		case 0x08:  // revision identification register and class code register
			result = 0x06000001;
			break;

		case 0x04:  // PCI command register
		case 0x0C:
		case 0x10:  // reserved
		case 0x14:  // reserved
		case 0x18:  // reserved
		case 0x1C:  // reserved
		case 0x20:  // reserved
		case 0x24:  // reserved
		case 0x28:  // reserved
		case 0x2C:  // reserved
		case 0x30:  // reserved
		case 0x34:  // reserved
		case 0x38:  // reserved
		case 0x3C:  // reserved
		case 0x40:  // reserved
		case 0x44:  // reserved
		case 0x48:  // reserved
		case 0x4C:  // reserved
			logerror("i82439tx_pci_read(): Unemulated PCI read 0x%02X, returning 0\n", offset);
			result = 0;
			break;

		case 0x50:
		case 0x54:
		case 0x58:
		case 0x5C:
		case 0x60:
		case 0x64:
		case 0x68:
		case 0x6C:
		case 0x70:
		case 0x74:
		case 0x78:
		case 0x7C:
		case 0x80:
		case 0x84:
		case 0x88:
		case 0x8C:
		case 0x90:
		case 0x94:
		case 0x98:
		case 0x9C:
		case 0xA0:
		case 0xA4:
		case 0xA8:
		case 0xAC:
		case 0xB0:
		case 0xB4:
		case 0xB8:
		case 0xBC:
		case 0xC0:
		case 0xC4:
		case 0xC8:
		case 0xCC:
		case 0xD0:
		case 0xD4:
		case 0xD8:
		case 0xDC:
		case 0xE0:
		case 0xE4:
		case 0xE8:
		case 0xEC:
		case 0xF0:
		case 0xF4:
		case 0xF8:
		case 0xFC:
			assert(((offset - 0x50) / 4) >= 0 && ((offset - 0x50) / 4) < ARRAY_LENGTH(m_regs));
			result = m_regs[(offset - 0x50) / 4];
			break;

		default:
			fatalerror("i82439tx_pci_read(): Unexpected PCI read 0x%02X\n", offset);
	}
	return result;
}

void i82439tx_device::pci_write(pci_bus_device *pcibus, int function, int offset, uint32_t data, uint32_t mem_mask)
{
#ifdef VERBOSE
	logerror("i82439tx_pci_write: write to function %d offset %08X mask %08X data %08X\n", function, offset, mem_mask, data);
#endif
	if (function != 0)
		return;

	switch(offset)
	{
		case 0x00:  // vendor/device ID
		case 0x10:  // reserved
		case 0x14:  // reserved
		case 0x18:  // reserved
		case 0x1C:  // reserved
		case 0x20:  // reserved
		case 0x24:  // reserved
		case 0x28:  // reserved
		case 0x2C:  // reserved
		case 0x30:  // reserved
		case 0x3C:  // reserved
		case 0x40:  // reserved
		case 0x44:  // reserved
		case 0x48:  // reserved
		case 0x4C:  // reserved
			// read only
			break;

		case 0x58:
			if ((mem_mask & 0x0000f000))
				i82439tx_configure_memory(data >> 12, 0xf0000, 0xfffff);
			if ((mem_mask & 0x000f0000))
				i82439tx_configure_memory(data >> 16, 0xc0000, 0xc3fff);
			if ((mem_mask & 0x00f00000))
				i82439tx_configure_memory(data >> 20, 0xc4000, 0xc7fff);
			if ((mem_mask & 0x0f000000))
				i82439tx_configure_memory(data >> 24, 0xc8000, 0xcbfff);
			if ((mem_mask & 0xf0000000))
				i82439tx_configure_memory(data >> 28, 0xcc000, 0xcffff);
			COMBINE_DATA(&m_regs[(offset - 0x50) / 4]);
			break;

		case 0x5C:
			if ((mem_mask & 0x0000000f))
				i82439tx_configure_memory(data >> 0, 0xd0000, 0xd3fff);
			if ((mem_mask & 0x000000f0))
				i82439tx_configure_memory(data >> 4, 0xd4000, 0xd7fff);
			if ((mem_mask & 0x00000f00))
				i82439tx_configure_memory(data >> 8, 0xd8000, 0xdbfff);
			if ((mem_mask & 0x0000f000))
				i82439tx_configure_memory(data >> 12, 0xdc000, 0xdffff);
			if ((mem_mask & 0x000f0000))
				i82439tx_configure_memory(data >> 16, 0xe0000, 0xe3fff);
			if ((mem_mask & 0x00f00000))
				i82439tx_configure_memory(data >> 20, 0xe4000, 0xe7fff);
			if ((mem_mask & 0x0f000000))
				i82439tx_configure_memory(data >> 24, 0xe8000, 0xebfff);
			if ((mem_mask & 0xf0000000))
				i82439tx_configure_memory(data >> 28, 0xec000, 0xeffff);
			COMBINE_DATA(&m_regs[(offset - 0x50) / 4]);
			break;

		case 0x04:  // PCI command register
		case 0x0C:
			logerror("i82439tx_pci_write(): Unemulated PCI write 0x%02X = 0x%04X\n", offset, data);
			break;

		case 0x70:
			if (ACCESSING_BITS_8_15)
			{
				// ESMRAMC: EXTENDED SYSTEM MANAGEMENT RAM CONTROL REGISTER
				data = (data & ~(1 << 14)) | ((m_regs[(offset - 0x50) / 4] & ~data) & (1 << 14)); // e_smerr cleared only by writing 1
				m_smram.tseg_en = (data >> 8) & 1;
				m_smram.tseg_sz = (data >> 9) & 3;
				m_smram.e_smerr = (data >> 14) & 1;
				m_smram.h_smrame = (data >> 15) & 1;
				m_smram.tseg_size = 128 * (1 << m_smram.tseg_sz) * 1024;
#ifdef VERBOSE
				logerror("i82439tx_pci_write(): ESMRAMC h_smrame %d e_smerr %d tseg_sz %d tseg_en %d\n", m_smram.h_smrame, m_smram.e_smerr, m_smram.tseg_sz, m_smram.tseg_en);
#endif
			}
			if (ACCESSING_BITS_16_23)
			{
				// SMRAMC: SYSTEM MANAGEMENT RAM CONTROL REGISTER
				data = data | (m_regs[(offset - 0x50) / 4] & (1 << 20)); // d_lck can only be set to 1
				m_smram.c_base_seg = (data >> 16) & 7;
				m_smram.g_smrame = (data >> 19) & 1;
				m_smram.d_lck = (data >> 20) & 1;
				m_smram.d_cls = (data >> 21) & 1;
				m_smram.d_open = (data >> 22) & 1;
#ifdef VERBOSE
				logerror("i82439tx_pci_write(): SMRAMC d_open %d d_cls %d d_lck %d g_smrame %d c_base_seg %d\n", m_smram.d_open, m_smram.d_cls, m_smram.d_lck, m_smram.g_smrame, m_smram.c_base_seg);
#endif
			}
			update_smram_mappings();
			COMBINE_DATA(&m_regs[(offset - 0x50) / 4]);
			break;

		case 0x50:
		case 0x54:
		case 0x60:
		case 0x64:
		case 0x68:
		case 0x6C:
		case 0x74:
		case 0x78:
		case 0x7C:
		case 0x80:
		case 0x84:
		case 0x88:
		case 0x8C:
		case 0x90:
		case 0x94:
		case 0x98:
		case 0x9C:
		case 0xA0:
		case 0xA4:
		case 0xA8:
		case 0xAC:
		case 0xB0:
		case 0xB4:
		case 0xB8:
		case 0xBC:
		case 0xC0:
		case 0xC4:
		case 0xC8:
		case 0xCC:
		case 0xD0:
		case 0xD4:
		case 0xD8:
		case 0xDC:
		case 0xE0:
		case 0xE4:
		case 0xE8:
		case 0xEC:
		case 0xF0:
		case 0xF4:
		case 0xF8:
		case 0xFC:
			assert(((offset - 0x50) / 4) >= 0 && ((offset - 0x50) / 4) < ARRAY_LENGTH(m_regs));
			COMBINE_DATA(&m_regs[(offset - 0x50) / 4]);
			break;

		default:
			fatalerror("i82439tx_pci_write(): Unexpected PCI write 0x%02X <-- 0x%08X\n", offset, data);
	}
}

/*
A is the compatible smram area, the memory area from 0xa0000 to 0xbffff
S is the extended smram area, the memory area from 0x100a0000 to 0x100fffff
T is the TSEG (T segment) smram area, the memory area from 0x10000000+dram_size-tseg_size to 0x10000000+dram_size-1
  where dram_size is the size of the dram memory on the motherboard and tseg_size is the size of TSEG
A and S are mutually exclusive and are stored in the last 384 kilobytes of the first megabyte of dram memory
T is stored in the last tseg_size bytes of the dram memory on the motherboard

g       s h
s       m s t
m       i m s
r     o a r e
a l c p c a g
m c l e t m e
e k s n # e n           Code                Data
-------------  -------------------- --------------------
0 x x x x x x   A-PCI  S-PCI  T-PCI  A-PCI  S-PCI  T-PCI
1 0 1 1 x x x         Invalid              Invalid

1 0 0 0 0 0 0   A-DRAM S-PCI  T-PCI  A-DRAM S-PCI  T-PCI
1 0 0 0 0 0 1   A-DRAM S-PCI  T-DRAM A-DRAM S-PCI  T-DRAM
1 0 0 0 0 1 0   A-PCI  S-DRAM T-PCI  A-PCI  S-DRAM T-PCI
1 0 0 0 0 1 1   A-PCI  S-DRAM T-DRAM A-PCI  S-DRAM T-DRAM

1 0 0 1 0 0 0   A-DRAM S-PCI  T-PCI  A-DRAM S-PCI  T-PCI
1 0 0 1 0 0 1   A-DRAM S-PCI  T-DRAM A-DRAM S-PCI  T-DRAM
1 0 0 1 0 1 0   A-PCI  S-DRAM T-PCI  A-PCI  S-DRAM T-PCI
1 0 0 1 0 1 1   A-PCI  S-DRAM T-DRAM A-PCI  S-DRAM T-DRAM

1 1 0 0 0 0 0   A-DRAM S-PCI  T-PCI  A-DRAM S-PCI  T-PCI
1 1 0 0 0 0 1   A-DRAM S-PCI  T-DRAM A-DRAM S-PCI  T-DRAM
1 1 0 0 0 1 0   A-PCI  S-DRAM T-PCI  A-PCI  S-DRAM T-PCI
1 1 0 0 0 1 1   A-PCI  S-DRAM T-DRAM A-PCI  S-DRAM T-DRAM

1 0 1 0 0 0 0   A-DRAM S-PCI  T-PCI  A-PCI  S-PCI  T-PCI
1 0 1 0 0 0 1   A-DRAM S-PCI  T-DRAM A-PCI  S-PCI  T-PCI
1 0 1 0 0 1 0   A-dram S-DRAM T-PCI  A-PCI  S-PCI  T-PCI
1 0 1 0 0 1 1   A-PCI  S-DRAM T-DRAM A-PCI  S-PCI  T-PCI

1 1 1 0 0 0 0   A-DRAM S-PCI  T-PCI  A-PCI  S-PCI  T-PCI
1 1 1 0 0 0 1   A-DRAM S-PCI  T-DRAM A-PCI  S-PCI  T-PCI
1 1 1 0 0 1 0   A-PCI  S-DRAM T-PCI  A-PCI  S-PCI  T-PCI
1 1 1 0 0 1 1   A-PCI  S-DRAM T-DRAM A-PCI  S-PCI  T-PCI

1 0 0 1 1 0 0   A-DRAM S-PCI  T-PCI  A-DRAM S-PCI  T-PCI
1 0 0 1 1 0 1   A-DRAM S-PCI  T-DRAM A-DRAM S-PCI  T-DRAM
1 0 0 1 1 1 0   A-PCI  S-DRAM T-PCI  A-PCI  S-DRAM T-PCI
1 0 0 1 1 1 1   A-PCI  S-DRAM T-DRAM A-PCI  S-DRAM T-DRAM

1 0 0 0 1 x x   A-PCI  S-PCI  T-PCI  A-PCI  S-PCI  T-PCI
1 0 1 0 1 x x   A-PCI  S-PCI  T-PCI  A-PCI  S-PCI  T-PCI
1 1 0 0 1 x x   A-PCI  S-PCI  T-PCI  A-PCI  S-PCI  T-PCI
1 1 1 0 1 x x   A-PCI  S-PCI  T-PCI  A-PCI  S-PCI  T-PCI
*/

void i82439tx_device::update_smram_mappings()
{
	int old = m_smram.mapping;
	if (m_smram.g_smrame == 0)
		m_smram.mapping = 0; // pci active in all ranges
	else if (m_smram.smiact_n == 1)
	{
		if (m_smram.d_open == 1)
		{
			// TSEG_EN decides if tseg is present or not
			// H_SMRAME decides between smram compatible and extended
			m_smram.mapping = (1 - m_smram.h_smrame) | (m_smram.h_smrame << 1) | (m_smram.tseg_en << 2);
		}
		else
			m_smram.mapping = 0; // pci active in all ranges
	}
	else // SMIACT# is 0
	{
		// TSEG_EN decides if tseg is present or not
		// H_SMRAME decides between smram compatible and extended
		m_smram.mapping = (1 - m_smram.h_smrame) | (m_smram.h_smrame << 1) | (m_smram.tseg_en << 2);
	}
	if (m_smram.mapping != old)
	{
		int change = old ^ m_smram.mapping;

		if (change & 1)
		{
			// unmap current 0xa0000 to 0xbffff
			m_space->unmap_readwrite(0xa0000, 0xbffff);
			// map new
			if (m_smram.mapping & 1)
			{
				m_space->install_ram(0xa0000, 0xbffff, m_ram->pointer() + 0xa0000);
			}
			else
				m_pci_bus->remap(AS_PROGRAM, 0xa0000, 0xbffff); // remap all pci and isa devices
		}
		if (change & 2)
		{
			// unmap current 0x100a0000 to 0x100fffff
			m_space->unmap_readwrite(0x100a0000, 0x100fffff);
			// map new
			if (m_smram.mapping & 2)
			{
				m_space->install_ram(0x100a0000, 0x100fffff, m_ram->pointer() + 0xa0000); // TODO: exclude areas used by shadow ram
			}
			else
				m_pci_bus->remap(AS_PROGRAM, 0x100a0000, 0x100fffff); // remap all pci and isa devices
		}
		if (change & 4)
		{
			offs_t s = 0x10000000 + m_ram->size() - (offs_t)m_smram.tseg_size;
			offs_t e = 0x10000000 + m_ram->size() - 1;

			// unmap current 0x10000000+dram_size-tseg_size to 0x10000000+dram_size-1
			m_space->unmap_readwrite(s, e);
			// map new
			if (m_smram.mapping & 4)
			{
				m_space->install_ram(s, e, m_ram->pointer() + m_ram->size() - (offs_t)m_smram.tseg_size);
			}
			else
				m_pci_bus->remap(AS_PROGRAM, s, e); // remap all pci and isa devices
		}
	}
}

WRITE_LINE_MEMBER(i82439tx_device::smi_act_w)
{
	// state is 0 when smm is not active
	// but smiact_n reflects the state of the SMIACT# pin
	if (state == 0)
		m_smram.smiact_n = 1;
	else
		m_smram.smiact_n = 0;
	update_smram_mappings();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i82439tx_device::device_start()
{
	northbridge_device::device_start();
	// get address space we are working on
	device_t *cpu = machine().device(m_cpu_tag);
	assert(cpu != nullptr);

	m_space = &cpu->memory().space(AS_PROGRAM);

	// get rom region
	m_rom = machine().root_device().memregion(m_region_tag)->base();

	// setup save states
	save_item(NAME(m_regs));
	save_item(NAME(m_bios_ram));
	// TODO: savestate m_smram
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i82439tx_device::device_reset()
{
	northbridge_device::device_reset();
	// setup initial values
	m_regs[0x00] = 0x14020000;
	m_regs[0x01] = 0x01520000;
	m_regs[0x02] = 0x00000000;
	m_regs[0x03] = 0x00000000;
	m_regs[0x04] = 0x02020202;
	m_regs[0x05] = 0x00000002;
	m_regs[0x06] = 0x00000000;
	m_regs[0x07] = 0x00000000;
	m_regs[0x20] = 0x00000000;

	memset(m_bios_ram, 0, sizeof(m_bios_ram));

	// configure initial memory state
	i82439tx_configure_memory(0, 0xf0000, 0xfffff);
	i82439tx_configure_memory(0, 0xc0000, 0xc3fff);
	i82439tx_configure_memory(0, 0xc4000, 0xc7fff);
	i82439tx_configure_memory(0, 0xc8000, 0xccfff);
	i82439tx_configure_memory(0, 0xcc000, 0xcffff);
	i82439tx_configure_memory(0, 0xd0000, 0xd3fff);
	i82439tx_configure_memory(0, 0xd4000, 0xd7fff);
	i82439tx_configure_memory(0, 0xd8000, 0xdbfff);
	i82439tx_configure_memory(0, 0xdc000, 0xdffff);
	i82439tx_configure_memory(0, 0xe0000, 0xe3fff);
	i82439tx_configure_memory(0, 0xe4000, 0xe7fff);
	i82439tx_configure_memory(0, 0xe8000, 0xecfff);
	i82439tx_configure_memory(0, 0xec000, 0xeffff);
}
