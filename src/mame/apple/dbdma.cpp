// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple "Descriptor-Based DMA" channel device
    Emulation by R. Belmont
*/

#include "emu.h"
#include "dbdma.h"

#define VERBOSE (0)
#include "logmacro.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(DBDMA_CHANNEL, dbdma_device, "appledbdma", "Apple descriptor-based DMA channel")

static constexpr u16 STATUS_RUN     = 0x8000;
static constexpr u16 STATUS_PAUSE   = 0x4000;
static constexpr u16 STATUS_FLUSH   = 0x2000;
static constexpr u16 STATUS_WAKE    = 0x1000;
[[maybe_unused]] static constexpr u16 STATUS_DEAD = 0x0800;
static constexpr u16 STATUS_ACTIVE  = 0x0400;

//-------------------------------------------------
//  dbdma_device - constructor
//-------------------------------------------------

dbdma_device::dbdma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, DBDMA_CHANNEL, tag, owner, clock),
	write_irq(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dbdma_device::device_start()
{
	save_item(NAME(m_status));
	save_item(NAME(m_command_pointer));
	save_item(NAME(m_intselect));
	save_item(NAME(m_branchselect));
	save_item(NAME(m_waitselect));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dbdma_device::device_reset()
{
	m_status = 0;
	m_command_pointer = 0;
	m_intselect = m_branchselect = m_waitselect = 0;
}

// 00 = control (write only)
// 04 = status (read only)
// 0c = command pointer (read/write)
// 10 = interrupt select
// 14 = branch select
// 18 = wait select

void dbdma_device::map(address_map &map)
{
	map(0x00, 0x03).w(FUNC(dbdma_device::control_w));
	map(0x04, 0x07).r(FUNC(dbdma_device::status_r));
	map(0x0c, 0x0f).rw(FUNC(dbdma_device::cmdpointer_r), FUNC(dbdma_device::cmdpointer_w));
	map(0x10, 0x13).rw(FUNC(dbdma_device::intselect_r), FUNC(dbdma_device::intselect_w));
	map(0x14, 0x17).rw(FUNC(dbdma_device::branchselect_r), FUNC(dbdma_device::branchselect_w));
	map(0x18, 0x1b).rw(FUNC(dbdma_device::waitselect_r), FUNC(dbdma_device::waitselect_w));
}

void dbdma_device::control_w(u32 data)
{
	// the top 16 bits of data are a mask selecting which bits of
	// the status are changed by the write.
	const u16 mask = (data >> 16);

	m_status &= (mask ^ 0xffff);
	m_status |= (data & mask);
	LOG("%s: channel status/control to %04x (raw %08x)\n", tag(), m_status, data);

	if (m_status & STATUS_RUN)
	{
		LOG("%s: channel set to RUN, also setting ACTIVE\n", tag());
		m_status |= STATUS_ACTIVE;
	}
	else
	{
		m_status &= ~STATUS_ACTIVE;
	}

	if (m_status & STATUS_PAUSE)
	{
		m_status &= ~STATUS_ACTIVE;
	}

	if (m_status & STATUS_FLUSH)
	{
		// TODO
	}

	if (m_status & STATUS_WAKE)
	{
		// TODO
	}
}

u32 dbdma_device::status_r()
{
	return m_status;
}

u32 dbdma_device::cmdpointer_r()
{
	return m_command_pointer;
}

void dbdma_device::cmdpointer_w(u32 data)
{
	if (!(m_status & STATUS_ACTIVE))
	{
		LOG("%s: %08x to command pointer\n", tag(), swapendian_int32(data));
		m_command_pointer = data;

		new_command();
	}
}

u32 dbdma_device::intselect_r()
{
	return m_intselect;
}

void dbdma_device::intselect_w(u32 data)
{
	m_intselect = data;
}

u32 dbdma_device::branchselect_r()
{
	return m_branchselect;
}

void dbdma_device::branchselect_w(u32 data)
{
	m_branchselect = data;
}

u32 dbdma_device::waitselect_r()
{
	return m_waitselect;
}

void dbdma_device::waitselect_w(u32 data)
{
	m_waitselect = data;
}

u32 dbdma_device::dma_read(u32 offset)
{
	step_program();
	return m_xfer_word;
}

void dbdma_device::dma_write(u32 offset, u32 data)
{
	m_xfer_word = data;
	step_program();
}

void dbdma_device::step_program()
{
	if ((m_status & STATUS_PAUSE) || !(m_status & STATUS_ACTIVE))
	{
		return;
	}

	switch (m_opcode >> 28)
	{
		case 0: // OUTPUT MORE
		case 1: // OUTPUT LAST
			m_xfer_word = m_pci_memory->read_dword(m_address);
			m_address += 4;
			m_currentXfer += 4;
			break;

		case 2: // INPUT MORE
		case 3: // INPUT LAST
			m_pci_memory->write_dword(m_address, m_xfer_word);
			m_address += 4;
			m_currentXfer += 4;
			break;

		case 4: // STORE QUAD
			switch (m_xferLimit & 7)
			{
				case 1:
					m_pci_memory->write_byte(m_address, m_cmdDep);
					break;

				case 2:
					m_pci_memory->write_word(m_address, m_cmdDep);
					break;

				case 4:
					m_pci_memory->write_dword(m_address, m_cmdDep);
					break;
			}
			m_currentXfer += m_opcode & 7;
			break;

		case 5: // LOAD QUAD
			switch (m_xferLimit & 7)
			{
				case 1:
					m_xfer_word = m_pci_memory->read_byte(m_address);
					m_pci_memory->write_byte(m_cmdDep, m_xfer_word);
					break;

				case 2:
					m_xfer_word = m_pci_memory->read_word(m_address);
					m_pci_memory->write_word(m_cmdDep, m_xfer_word);
					break;

				case 4:
					m_xfer_word = m_pci_memory->read_dword(m_address);
					m_pci_memory->write_dword(m_cmdDep, m_xfer_word);
					break;
			}
			m_currentXfer += m_opcode & 7;
			break;

		case 6: // NOP
				break;

		case 7: // STOP
				m_status &= ~STATUS_ACTIVE;
				break;
		}

		if (m_status & STATUS_ACTIVE)
		{
			if (m_currentXfer >= m_xferLimit)
			{
				m_command_pointer += 16;
				LOG("%s: Advancing command pointer to %08x\n", tag(), m_command_pointer);
				new_command();
			}
		}
}

void dbdma_device::new_command()
{
	m_opcode = m_pci_memory->read_dword(m_command_pointer);
	m_address = m_pci_memory->read_dword(m_command_pointer + 4);
	m_cmdDep = m_pci_memory->read_dword(m_command_pointer + 8);
	m_statusCount = m_pci_memory->read_dword(m_command_pointer + 12);
	m_currentXfer = m_statusCount & 0xffff;
	m_xferLimit = m_opcode & 0xffff;

	LOG("%s: new command %08x %08x %08x %08x\n", tag(), m_opcode, m_address, m_cmdDep, m_statusCount);
	LOG("%s: opcode %d\n", tag(), m_opcode >> 28);
}
