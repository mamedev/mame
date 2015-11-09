// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Intel 82371AB PCI IDE ISA Xcelerator (PIIX4)

    Part of the Intel 430TX chipset

    - Integrated IDE Controller
    - Enhanced DMA Controller based on two 82C37
    - Interrupt Controller based on two 82C59
    - Timers based on 82C54
    - USB
    - SMBus
    - Real Time Clock based on MC146818

***************************************************************************/

#include "emu.h"
#include "i82371ab.h"

const device_type I82371AB = &device_creator<i82371ab_device>;


i82371ab_device::i82371ab_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: southbridge_device(mconfig, I82371AB, "Intel 82371AB", tag, owner, clock, "i82371ab", __FILE__),
		pci_device_interface( mconfig, *this )
{
}

UINT32 i82371ab_device::pci_isa_r(device_t *busdevice, int offset, UINT32 mem_mask)
{
	UINT32 result = m_regs[0][offset] |
			m_regs[0][offset+1] << 8 |
			m_regs[0][offset+2] << 16|
			m_regs[0][offset+3] << 24;

	logerror("i82371ab_pci_isa_r, offset = %02x, mem_mask = %08x\n", offset, mem_mask);

	return result;
}

void i82371ab_device::pci_isa_w(device_t *busdevice, int offset, UINT32 data, UINT32 mem_mask)
{
	UINT32 cdata = 0;
	int i;
	COMBINE_DATA(&cdata);

	logerror("i82371ab_pci_isa_w, offset = %02x, data = %08x, mem_mask = %08x\n", offset, data, mem_mask);

	for(i = 0; i < 4; i++, offset++, cdata >>= 8)
	{
		switch (offset)
		{
			case 0x04:
				/* clear reserved bits */
				m_regs[0][offset] = cdata & 0x05;
				break;
			case 0x06:
				/* set new status */
				m_regs[0][offset] |= 0x80;
				break;
			case 0x07:
				m_regs[0][offset] |= 0x02;
				break;
		}
	}
}

UINT32 i82371ab_device::pci_ide_r(device_t *busdevice, int offset, UINT32 mem_mask)
{
	UINT32 result = m_regs[1][offset] |
			m_regs[1][offset+1] << 8 |
			m_regs[1][offset+2] << 16|
			m_regs[1][offset+3] << 24;

	logerror("i82371ab_pci_ide_r, offset = %02x, mem_mask = %08x\n", offset, mem_mask);

	return result;
}

void i82371ab_device::pci_ide_w(device_t *busdevice, int offset, UINT32 data, UINT32 mem_mask)
{
	UINT32 cdata = 0;
	int i;
	COMBINE_DATA(&cdata);

	logerror("i82371ab_pci_isa_w, offset = %02x, data = %08x, mem_mask = %08x\n", offset, data, mem_mask);

	for(i = 0; i < 4; i++, offset++, cdata >>= 8)
	{
		switch (offset)
		{
			case 0x04:
				/* clear reserved bits */
				m_regs[1][offset] = cdata & 0x05;
				break;
			case 0x06:
				/* set new status */
				m_regs[1][offset] |= 0x80;
				break;
			case 0x07:
				m_regs[1][offset] |= 0x02;
				break;
		}
	}
}

UINT32 i82371ab_device::pci_usb_r(device_t *busdevice, int offset, UINT32 mem_mask)
{
	UINT32 result = m_regs[2][offset] |
			m_regs[2][offset+1] << 8 |
			m_regs[2][offset+2] << 16|
			m_regs[2][offset+3] << 24;

	logerror("i82371ab_pci_usb_r, offset = %02x, mem_mask = %08x\n", offset, mem_mask);

	return result;
}

void i82371ab_device::pci_usb_w(device_t *busdevice, int offset, UINT32 data, UINT32 mem_mask)
{
	UINT32 cdata = 0;
	int i;
	COMBINE_DATA(&cdata);

	logerror("i82371ab_pci_isa_w, offset = %02x, data = %08x, mem_mask = %08x\n", offset, data, mem_mask);

	for(i = 0; i < 4; i++, offset++, cdata >>= 8)
	{
		switch (offset)
		{
			case 0x04:
				/* clear reserved bits */
				m_regs[2][offset] = cdata & 0x05;
				break;
			case 0x06:
				/* set new status */
				m_regs[2][offset] |= 0x80;
				break;
			case 0x07:
				m_regs[2][offset] |= 0x02;
				break;
		}
	}
}

UINT32 i82371ab_device::pci_acpi_r(device_t *busdevice, int offset, UINT32 mem_mask)
{
	UINT32 result = m_regs[3][offset] |
			m_regs[3][offset+1] << 8 |
			m_regs[3][offset+2] << 16|
			m_regs[3][offset+3] << 24;

	logerror("i82371ab_pci_acpi_r, offset = %02x, mem_mask = %08x\n", offset, mem_mask);

	return result;
}

void i82371ab_device::pci_acpi_w(device_t *busdevice, int offset, UINT32 data, UINT32 mem_mask)
{
	UINT32 cdata = 0;
	int i;
	COMBINE_DATA(&cdata);

	logerror("i82371ab_pci_isa_w, offset = %02x, data = %08x, mem_mask = %08x\n", offset, data, mem_mask);

	for(i = 0; i < 4; i++, offset++, cdata >>= 8)
	{
		switch (offset)
		{
			case 0x04:
				/* clear reserved bits */
				m_regs[3][offset] = cdata & 0x05;
				break;
			case 0x06:
				/* set new status */
				m_regs[3][offset] |= 0x80;
				break;
			case 0x07:
				m_regs[3][offset] |= 0x02;
				break;
		}
	}
}

UINT32 i82371ab_device::pci_read(pci_bus_device *pcibus, int function, int offset, UINT32 mem_mask)
{
	switch (function)
	{
	case 0: return pci_isa_r(pcibus, offset, mem_mask);
	case 1: return pci_ide_r(pcibus, offset, mem_mask);
	case 2: return pci_usb_r(pcibus, offset, mem_mask);
	case 3: return pci_acpi_r(pcibus, offset, mem_mask);
	}

	logerror("i82371ab_pci_read: read from undefined function %d\n", function);

	return 0;
}

void i82371ab_device::pci_write(pci_bus_device *pcibus, int function, int offset, UINT32 data, UINT32 mem_mask)
{
	switch (function)
	{
	case 0: pci_isa_w(pcibus, offset, data, mem_mask); break;
	case 1: pci_ide_w(pcibus, offset, data, mem_mask); break;
	case 2: pci_usb_w(pcibus, offset, data, mem_mask); break;
	case 3: pci_acpi_w(pcibus, offset, data, mem_mask); break;
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i82371ab_device::device_start()
{
	southbridge_device::device_start();
	/* setup save states */
	save_item(NAME(m_regs));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i82371ab_device::device_reset()
{
	southbridge_device::device_reset();
	memset(m_regs, 0, sizeof(m_regs));
	UINT32 (*regs32)[64] = (UINT32 (*)[64])(m_regs);

	/* isa */
	regs32[0][0x00] = 0x71108086;
	regs32[0][0x04] = 0x00000000;
	regs32[0][0x08] = 0x06010000;
	regs32[0][0x0c] = 0x00800000;

	/* ide */
	regs32[1][0x00] = 0x71118086;
	regs32[1][0x04] = 0x02800000;
	regs32[1][0x08] = 0x01018000;
	regs32[1][0x0c] = 0x00000000;

	/* usb */
	regs32[2][0x00] = 0x71128086;
	regs32[2][0x04] = 0x02800000;
	regs32[2][0x08] = 0x0c030000;
	regs32[2][0x0c] = 0x00000000;

	/* acpi */
	regs32[3][0x00] = 0x71138086;
	regs32[3][0x04] = 0x02800000;
	regs32[3][0x08] = 0x06800000;
	regs32[3][0x0c] = 0x02800000;
}
