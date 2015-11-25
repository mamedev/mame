// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Intel 82439TX System Controller (MTXC)

***************************************************************************/

#include "emu.h"
#include "i82439tx.h"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

const device_type I82439TX = &device_creator<i82439tx_device>;


i82439tx_device::i82439tx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: northbridge_device(mconfig, I82439TX, "Intel 82439TX", tag, owner, clock, "i82439tx", __FILE__),
	pci_device_interface( mconfig, *this ),
	m_cpu_tag( NULL ),
	m_region_tag( NULL ),
	m_space(nullptr),
	m_rom(nullptr)
{
}

void i82439tx_device::i82439tx_configure_memory(UINT8 val, offs_t begin, offs_t end)
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

UINT32 i82439tx_device::pci_read(pci_bus_device *pcibus, int function, int offset, UINT32 mem_mask)
{
	UINT32 result = 0;

	if (function != 0)
		return 0;

	switch(offset)
	{
		case 0x00:  /* vendor/device ID */
			result = 0x71008086;
			break;

		case 0x08:  /* revision identification register and class code register*/
			result = 0x06000001;
			break;

		case 0x04:  /* PCI command register */
		case 0x0C:
		case 0x10:  /* reserved */
		case 0x14:  /* reserved */
		case 0x18:  /* reserved */
		case 0x1C:  /* reserved */
		case 0x20:  /* reserved */
		case 0x24:  /* reserved */
		case 0x28:  /* reserved */
		case 0x2C:  /* reserved */
		case 0x30:  /* reserved */
		case 0x34:  /* reserved */
		case 0x38:  /* reserved */
		case 0x3C:  /* reserved */
		case 0x40:  /* reserved */
		case 0x44:  /* reserved */
		case 0x48:  /* reserved */
		case 0x4C:  /* reserved */
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

void i82439tx_device::pci_write(pci_bus_device *pcibus, int function, int offset, UINT32 data, UINT32 mem_mask)
{
	if (function != 0)
		return;

	switch(offset)
	{
		case 0x00:  /* vendor/device ID */
		case 0x10:  /* reserved */
		case 0x14:  /* reserved */
		case 0x18:  /* reserved */
		case 0x1C:  /* reserved */
		case 0x20:  /* reserved */
		case 0x24:  /* reserved */
		case 0x28:  /* reserved */
		case 0x2C:  /* reserved */
		case 0x30:  /* reserved */
		case 0x3C:  /* reserved */
		case 0x40:  /* reserved */
		case 0x44:  /* reserved */
		case 0x48:  /* reserved */
		case 0x4C:  /* reserved */
			/* read only */
			break;

		case 0x04:  /* PCI command register */
		case 0x0C:
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
			switch(offset)
			{
				case 0x58:
					if ((mem_mask & 0x0000f000))
						i82439tx_configure_memory(data >> 12, 0xf0000, 0xfffff);
					if ((mem_mask & 0x000f0000))
						i82439tx_configure_memory(data >> 16, 0xc0000, 0xc3fff);
					if ((mem_mask & 0x00f00000))
						i82439tx_configure_memory(data >> 20, 0xc4000, 0xc7fff);
					if ((mem_mask & 0x0f000000))
						i82439tx_configure_memory(data >> 24, 0xc8000, 0xccfff);
					if ((mem_mask & 0xf0000000))
						i82439tx_configure_memory(data >> 28, 0xcc000, 0xcffff);
					break;

				case 0x5C:
					if ((mem_mask & 0x0000000f))
						i82439tx_configure_memory(data >>  0, 0xd0000, 0xd3fff);
					if ((mem_mask & 0x000000f0))
						i82439tx_configure_memory(data >>  4, 0xd4000, 0xd7fff);
					if ((mem_mask & 0x00000f00))
						i82439tx_configure_memory(data >>  8, 0xd8000, 0xdbfff);
					if ((mem_mask & 0x0000f000))
						i82439tx_configure_memory(data >> 12, 0xdc000, 0xdffff);
					if ((mem_mask & 0x000f0000))
						i82439tx_configure_memory(data >> 16, 0xe0000, 0xe3fff);
					if ((mem_mask & 0x00f00000))
						i82439tx_configure_memory(data >> 20, 0xe4000, 0xe7fff);
					if ((mem_mask & 0x0f000000))
						i82439tx_configure_memory(data >> 24, 0xe8000, 0xecfff);
					if ((mem_mask & 0xf0000000))
						i82439tx_configure_memory(data >> 28, 0xec000, 0xeffff);
					break;
			}

			assert(((offset - 0x50) / 4) >= 0 && ((offset - 0x50) / 4) < ARRAY_LENGTH(m_regs));
			COMBINE_DATA(&m_regs[(offset - 0x50) / 4]);
			break;

		default:
			fatalerror("i82439tx_pci_write(): Unexpected PCI write 0x%02X <-- 0x%08X\n", offset, data);
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i82439tx_device::device_start()
{
	northbridge_device::device_start();
	/* get address space we are working on */
	device_t *cpu = machine().device(m_cpu_tag);
	assert(cpu != NULL);

	m_space = &cpu->memory().space(AS_PROGRAM);

	/* get rom region */
	m_rom = machine().root_device().memregion(m_region_tag)->base();

	/* setup save states */
	save_item(NAME(m_regs));
	save_item(NAME(m_bios_ram));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i82439tx_device::device_reset()
{
	northbridge_device::device_reset();
	/* setup initial values */
	m_regs[0x00] = 0x14020000;
	m_regs[0x01] = 0x01520000;
	m_regs[0x02] = 0x00000000;
	m_regs[0x03] = 0x00000000;
	m_regs[0x04] = 0x02020202;
	m_regs[0x05] = 0x00000002;
	m_regs[0x06] = 0x00000000;
	m_regs[0x07] = 0x00000000;

	memset(m_bios_ram, 0, sizeof(m_bios_ram));

	/* configure initial memory state */
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
