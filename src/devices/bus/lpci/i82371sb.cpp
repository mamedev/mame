// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Intel 82371SB PCI IDE ISA Xcelerator (PIIX3)

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
#include "i82371sb.h"


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

const device_type I82371SB = &device_creator<i82371sb_device>;


i82371sb_device::i82371sb_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
		: southbridge_device(mconfig, I82371SB, "Intel 82371SB", tag, owner, clock, "i82371sb", __FILE__),
		pci_device_interface( mconfig, *this )
{
}

UINT32 i82371sb_device::pci_isa_r(device_t *busdevice,int offset, UINT32 mem_mask)
{
	UINT32 result = m_regs[0][offset];

	//logerror("i82371sb_pci_isa_r, offset = %02x, mem_mask = %08x\n", offset, mem_mask);

	return result;
}

void i82371sb_device::pci_isa_w(device_t *busdevice, int offset, UINT32 data, UINT32 mem_mask)
{
	//logerror("i82371sb_pci_isa_w, offset = %02x, data = %08x, mem_mask = %08x\n", offset, data, mem_mask);

	switch (offset)
	{
	case 0x04:
		COMBINE_DATA(&m_regs[0][offset]);

		/* clear reserved bits */
		m_regs[0][offset] &= 0x00000005;

		/* set new status */
		m_regs[0][offset] |= 0x02800000;

		break;
	}
}

UINT32 i82371sb_device::pci_ide_r(device_t *busdevice, int offset, UINT32 mem_mask)
{
	//logerror("i82371sb_pci_ide_r, offset = %02x, mem_mask = %08x\n", offset, mem_mask);
	UINT32 result = m_regs[1][offset];
	return result;
}

void i82371sb_device::pci_ide_w(device_t *busdevice, int offset, UINT32 data, UINT32 mem_mask)
{
	//logerror("i82371sb_pci_ide_w, offset = %02x, data = %08x, mem_mask = %08x\n", offset, data, mem_mask);

	switch (offset)
	{
	case 0x04:
		COMBINE_DATA(&m_regs[1][offset]);

		/* clear reserved bits */
		m_regs[1][offset] &= 0x00000005;

		/* set new status */
		m_regs[1][offset] |= 0x02800000;

		break;
	}
}

UINT32 i82371sb_device::pci_usb_r(device_t *busdevice, int offset, UINT32 mem_mask)
{
	UINT32 result = m_regs[2][offset];

	//logerror("i82371sb_pci_usb_r, offset = %02x, mem_mask = %08x\n", offset, mem_mask);

	return result;
}

void i82371sb_device::pci_usb_w(device_t *busdevice, int offset, UINT32 data, UINT32 mem_mask)
{
	//logerror("i82371sb_pci_usb_w, offset = %02x, data = %08x, mem_mask = %08x\n", offset, data, mem_mask);

	switch (offset)
	{
	case 0x04:
		COMBINE_DATA(&m_regs[2][offset]);

		/* clear reserved bits */
		m_regs[2][offset] &= 0x00000005;

		/* set new status */
		m_regs[2][offset] |= 0x02800000;

		break;
	}
}

UINT32 i82371sb_device::pci_read(pci_bus_device *pcibus, int function, int offset, UINT32 mem_mask)
{
	switch (function)
	{
	case 0: return pci_isa_r(pcibus, offset, mem_mask);
	case 1: return pci_ide_r(pcibus, offset, mem_mask);
	case 2: return pci_usb_r(pcibus, offset, mem_mask);
	}

	//logerror("i82371sb_pci_read: read from undefined function %d\n", function);

	return 0;
}

void i82371sb_device::pci_write(pci_bus_device *pcibus, int function, int offset, UINT32 data, UINT32 mem_mask)
{
	switch (function)
	{
	case 0: pci_isa_w(pcibus, offset, data, mem_mask); break;
	case 1: pci_ide_w(pcibus, offset, data, mem_mask); break;
	case 2: pci_usb_w(pcibus, offset, data, mem_mask); break;
	}
	//logerror("i82371sb_pci_write: write to undefined function %d\n", function);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i82371sb_device::device_start()
{
	southbridge_device::device_start();
	/* setup save states */
	save_item(NAME(m_regs));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i82371sb_device::device_reset()
{
	southbridge_device::device_reset();

	memset(m_regs, 0, sizeof(m_regs));

	/* isa */
	m_regs[0][0x00] = 0x70008086;
	m_regs[0][0x04] = 0x00000000;
	m_regs[0][0x08] = 0x06010000;
	m_regs[0][0x0c] = 0x00800000;

	/* ide */
	m_regs[1][0x00] = 0x70108086;
	m_regs[1][0x04] = 0x02800000;
	m_regs[1][0x08] = 0x01018000;
	m_regs[1][0x0c] = 0x00000000;

	/* usb */
	m_regs[2][0x00] = 0x70208086;
	m_regs[2][0x04] = 0x02800000;
	m_regs[2][0x08] = 0x0c030000;
	m_regs[2][0x0c] = 0x00000000;
}
