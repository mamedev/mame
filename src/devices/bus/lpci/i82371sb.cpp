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

//#define VERBOSE

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

DEFINE_DEVICE_TYPE(I82371SB, i82371sb_device, "i82371sb", "Intel 82371SB")


i82371sb_device::i82371sb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: southbridge_device(mconfig, I82371SB, tag, owner, clock)
	, pci_device_interface( mconfig, *this )
	, m_smi_callback(*this)
	, m_boot_state_hook(*this)
	, m_csmigate(0)
	, m_smien(0)
	, m_apmc(0)
	, m_apms(0)
	, m_base(0)
{
}

uint32_t i82371sb_device::pci_isa_r(device_t *busdevice,int offset, uint32_t mem_mask)
{
	uint32_t result = m_regs[0][offset];

#ifdef VERBOSE
	logerror("i82371sb_pci_isa_r, offset = %02x, mem_mask = %08x\n", offset, mem_mask);
#endif

	return result;
}

void i82371sb_device::pci_isa_w(device_t *busdevice, int offset, uint32_t data, uint32_t mem_mask)
{
#ifdef VERBOSE
	logerror("i82371sb_pci_isa_w, offset = %02x, data = %08x, mem_mask = %08x\n", offset, data, mem_mask);
#endif

	switch (offset)
	{
	case 0x04:
		COMBINE_DATA(&m_regs[0][offset]);

		// clear reserved bits
		m_regs[0][offset] &= 0x00000005;

		// set new status
		m_regs[0][offset] |= 0x02800000;

		break;

	case 0xa0:
		COMBINE_DATA(&m_regs[0][offset]);

		m_csmigate = m_regs[0][offset] & 1;
		m_smien = (m_regs[0][offset] >> 16) & 511;
		update_smireq_line();
		break;

	case 0xa8:
		COMBINE_DATA(&m_regs[0][offset]);
		update_smireq_line();
		break;
	}
}

uint32_t i82371sb_device::pci_ide_r(device_t *busdevice, int offset, uint32_t mem_mask)
{
	uint32_t result = m_regs[1][offset];

#ifdef VERBOSE
	logerror("i82371sb_pci_ide_r, offset = %02x, mem_mask = %08x\n", offset, mem_mask);
#endif

	return result;
}

void i82371sb_device::pci_ide_w(device_t *busdevice, int offset, uint32_t data, uint32_t mem_mask)
{
#ifdef VERBOSE
	logerror("i82371sb_pci_ide_w, offset = %02x, data = %08x, mem_mask = %08x\n", offset, data, mem_mask);
#endif

	switch (offset)
	{
	case 0x04:
		COMBINE_DATA(&m_regs[1][offset]);

		// clear reserved bits
		m_regs[1][offset] &= 0x38000007;

		// set new status
		m_regs[1][offset] |= 0x02800002;

		// if bit 0 is 0 access to ide io ports must not be enabled
		if (m_regs[1][offset] & 1)
		{
			if (m_ide_io_ports_enabled == false)
			{
				m_ide_io_ports_enabled = true;
				map_busmaster_dma();
			}
		}
		else
		{
			if (m_ide_io_ports_enabled == true)
			{
				m_ide_io_ports_enabled = false;
				if (m_base != 0)
					spaceio->unmap_readwrite(m_base, m_base + 0xf);
			}
		}
		break;

	case 0x0c:
		COMBINE_DATA(&m_regs[1][offset]);
		m_regs[1][offset] &= 0x0000f000;
		break;


	case 0x20:
		COMBINE_DATA(&m_regs[1][offset]);
		m_regs[1][offset] = (m_regs[1][offset] & 0x0000fff0) | 1;
		if (m_ide_io_ports_enabled == true)
			if (m_base != 0)
				spaceio->unmap_readwrite(m_base, m_base + 0xf); // unmap current
		m_base = m_regs[1][offset] & 0x0000fff0;
		if (m_ide_io_ports_enabled == true)
			map_busmaster_dma();
		break;

	case 0x3c:
	case 0x40:
	case 0x44:
		COMBINE_DATA(&m_regs[1][offset]);
		break;
	}
}

void i82371sb_device::map_busmaster_dma()
{
	if (m_base != 0)
	{
		spaceio->install_readwrite_handler(m_base, m_base + 0x7, read32_delegate(FUNC(bus_master_ide_controller_device::bmdma_r), &(*m_ide)), write32_delegate(FUNC(bus_master_ide_controller_device::bmdma_w), &(*m_ide)), 0xffffffff);
		spaceio->install_readwrite_handler(m_base + 0x8, m_base + 0xf, read32_delegate(FUNC(bus_master_ide_controller_device::bmdma_r), &(*m_ide2)), write32_delegate(FUNC(bus_master_ide_controller_device::bmdma_w), &(*m_ide2)), 0xffffffff);
	}
}

uint32_t i82371sb_device::pci_usb_r(device_t *busdevice, int offset, uint32_t mem_mask)
{
	uint32_t result = m_regs[2][offset];

#ifdef VERBOSE
	logerror("i82371sb_pci_usb_r, offset = %02x, mem_mask = %08x\n", offset, mem_mask);
#endif

	return result;
}

void i82371sb_device::pci_usb_w(device_t *busdevice, int offset, uint32_t data, uint32_t mem_mask)
{
#ifdef VERBOSE
	logerror("i82371sb_pci_usb_w, offset = %02x, data = %08x, mem_mask = %08x\n", offset, data, mem_mask);
#endif

	switch (offset)
	{
	case 0x04:
		COMBINE_DATA(&m_regs[2][offset]);

		// clear reserved bits
		m_regs[2][offset] &= 0x00000005;

		// set new status
		m_regs[2][offset] |= 0x02800000;

		break;

	case 0x20:
		COMBINE_DATA(&m_regs[2][offset]);
		m_regs[2][offset] = (m_regs[2][offset] & 0x0000ffe0) | 1;
		break;

	case 0x3c:
		COMBINE_DATA(&m_regs[2][offset]);
		m_regs[2][offset] = (m_regs[2][offset] & 0x000000ff) | 0x400;
		break;
	}
}

uint32_t i82371sb_device::pci_read(pci_bus_device *pcibus, int function, int offset, uint32_t mem_mask)
{
	switch (function)
	{
	case 0: return pci_isa_r(pcibus, offset, mem_mask);
	case 1: return pci_ide_r(pcibus, offset, mem_mask);
	case 2: return pci_usb_r(pcibus, offset, mem_mask);
	}
#ifdef VERBOSE
	logerror("i82371sb_pci_read: read from undefined function %d offset %08X mask %08X\n", function,offset,mem_mask);
#endif

	return 0;
}

void i82371sb_device::pci_write(pci_bus_device *pcibus, int function, int offset, uint32_t data, uint32_t mem_mask)
{
	switch (function)
	{
	case 0: pci_isa_w(pcibus, offset, data, mem_mask); break;
	case 1: pci_ide_w(pcibus, offset, data, mem_mask); break;
	case 2: pci_usb_w(pcibus, offset, data, mem_mask); break;
	}
#ifdef VERBOSE
	logerror("i82371sb_pci_write: write to undefined function %d offset %08X mask %08X data %08X\n", function, offset, mem_mask, data);
#endif
}

void i82371sb_device::remap(int space_id, offs_t start, offs_t end)
{
	m_isabus->remap(space_id, start, end);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i82371sb_device::device_start()
{
	address_space& spaceio = machine().device(":maincpu")->memory().space(AS_IO);

	southbridge_device::device_start();
	m_ide_io_ports_enabled = false;
	spaceio.install_readwrite_handler(0x00b0, 0x00b3, read8_delegate(FUNC(i82371sb_device::read_apmcapms), this), write8_delegate(FUNC(i82371sb_device::write_apmcapms), this), 0xffff0000);
	m_smi_callback.resolve_safe();
	m_boot_state_hook.resolve_safe();
	// setup save states
	save_item(NAME(m_regs));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i82371sb_device::device_reset()
{
	southbridge_device::device_reset();

	memset(m_regs, 0, sizeof(m_regs));

	// isa
	m_regs[0][0x00] = 0x70008086;
	m_regs[0][0x04] = 0x00000000;
	m_regs[0][0x08] = 0x06010000;
	m_regs[0][0x0c] = 0x00800000;

	// ide
	m_regs[1][0x00] = 0x70108086;
	m_regs[1][0x04] = 0x02800002;
	m_regs[1][0x08] = 0x01018000;
	m_regs[1][0x0c] = 0x00000000;
	m_regs[1][0x20] = 0x00000001;
	m_regs[2][0x3c] = 0x00000100;

	// usb
	m_regs[2][0x00] = 0x70208086;
	m_regs[2][0x04] = 0x02800000;
	m_regs[2][0x08] = 0x0c030000;
	m_regs[2][0x0c] = 0x00000000;
	m_regs[2][0x3c] = 0x00000400;
	m_regs[2][0x60] = 0x00000010;
	m_regs[2][0x68] = 0x00010000;
	m_regs[2][0xc0] = 0x00002000;
}

void i82371sb_device::port80_debug_write(uint8_t value)
{
	m_boot_state_hook((offs_t)0, value);
}

READ8_MEMBER(i82371sb_device::read_apmcapms)
{
	if (offset == 0)
		return m_apmc;
	else
		return m_apms;
}

WRITE8_MEMBER(i82371sb_device::write_apmcapms)
{
	if (offset == 0)
	{
		m_apmc = data;
		if (m_smien & 128)
			m_regs[0][0xa8] |= (1 << 7) << 16;
		update_smireq_line();
	}
	else
		m_apms = data;
}

void i82371sb_device::update_smireq_line()
{
	int smireq = (m_regs[0][0xa8] >> 16) & 511;
	int needed = smireq & m_smien;

	if (needed && m_csmigate)
		m_smi_callback(1);
	else
		m_smi_callback(0);
}
