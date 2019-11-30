// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    idectrl.c

    Generic (PC-style) IDE controller implementation.

***************************************************************************/

#include "emu.h"
#include "idectrl.h"


/***************************************************************************
    DEBUGGING
***************************************************************************/

//#define VERBOSE 1
#include "logmacro.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

DEFINE_DEVICE_TYPE(IDE_CONTROLLER, ide_controller_device, "idectrl", "IDE Controller (16-bit)")

ide_controller_device::ide_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ide_controller_device(mconfig, IDE_CONTROLLER, tag, owner, clock)
{
}

ide_controller_device::ide_controller_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	abstract_ata_interface_device(mconfig, type, tag, owner, clock)
{
}

uint16_t ide_controller_device::read_cs0(offs_t offset, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		if (ACCESSING_BITS_8_15 && offset == 1) { offset = 0; popmessage("requires ide_controller_32_device"); }
		return internal_read_cs0(offset * 2, mem_mask);
	}
	else
	{
		return internal_read_cs0((offset * 2) + 1, 0xff) << 8;
	}
}

uint16_t ide_controller_device::read_cs1(offs_t offset, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		return internal_read_cs1(offset * 2, mem_mask);
	}
	else
	{
		return internal_read_cs1((offset * 2) + 1, mem_mask >> 8) << 8;
	}
}

void ide_controller_device::write_cs0(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		if (ACCESSING_BITS_8_15 && offset == 1) { offset = 0; popmessage("requires ide_controller_32_device"); }
		return internal_write_cs0(offset * 2, data, mem_mask);
	}
	else
	{
		return internal_write_cs0((offset * 2) + 1, data >> 8, mem_mask >> 8);
	}
}

void ide_controller_device::write_cs1(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		return internal_write_cs1(offset * 2, data, mem_mask);
	}
	else
	{
		return internal_write_cs1((offset * 2) + 1, data >> 8, mem_mask >> 8);
	}
}

DEFINE_DEVICE_TYPE(IDE_CONTROLLER_32, ide_controller_32_device, "idectrl32", "IDE Controller (32-bit)")

ide_controller_32_device::ide_controller_32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ide_controller_32_device(mconfig, IDE_CONTROLLER_32, tag, owner, clock)
{
}

ide_controller_32_device::ide_controller_32_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	abstract_ata_interface_device(mconfig, type, tag, owner, clock)
{
}

uint32_t ide_controller_32_device::read_cs0(offs_t offset, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_15)
	{
		if (ACCESSING_BITS_0_7)
		{
			uint32_t data = internal_read_cs0((offset * 4), mem_mask);

			if (offset == 0 && ACCESSING_BITS_16_31)
				data |= internal_read_cs0((offset * 4), mem_mask >> 16) << 16;

			return data;
		}
		else
		{
			return internal_read_cs0((offset * 4) + 1, mem_mask >> 8) << 8;
		}
	}
	else if (ACCESSING_BITS_16_31)
	{
		if (ACCESSING_BITS_16_23)
		{
			return internal_read_cs0((offset * 4) + 2, mem_mask >> 16) << 16;
		}
		else
		{
			return internal_read_cs0((offset * 4) + 3, mem_mask >> 24) << 24;
		}
	}

	return 0;
}

uint32_t ide_controller_32_device::read_cs1(offs_t offset, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_15)
	{
		if (ACCESSING_BITS_0_7)
		{
			return internal_read_cs1((offset * 4), mem_mask);
		}
		else
		{
			return internal_read_cs1((offset * 4) + 1, mem_mask >> 8) << 8;
		}
	}
	else if (ACCESSING_BITS_16_31)
	{
		if (ACCESSING_BITS_16_23)
		{
			return internal_read_cs1((offset * 4) + 2, mem_mask >> 16) << 16;
		}
		else
		{
			return internal_read_cs1((offset * 4) + 3, mem_mask >> 24) << 24;
		}
	}

	return 0;
}

void ide_controller_32_device::write_cs0(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_15)
	{
		if (ACCESSING_BITS_0_7)
		{
			internal_write_cs0((offset * 4), data, mem_mask);

			if (offset == 0 && ACCESSING_BITS_16_31)
				internal_write_cs0((offset * 4), data >> 16, mem_mask >> 16);
		}
		else
		{
			internal_write_cs0((offset * 4) + 1, data >> 8, mem_mask >> 8);
		}
	}
	else if (ACCESSING_BITS_16_31)
	{
		if (ACCESSING_BITS_16_23)
		{
			internal_write_cs0((offset * 4) + 2, data >> 16, mem_mask >> 16);
		}
		else
		{
			internal_write_cs0((offset * 4) + 3, data >> 24, mem_mask >> 24);
		}
	}
}

void ide_controller_32_device::write_cs1(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_15)
	{
		if (ACCESSING_BITS_0_7)
		{
			internal_write_cs1((offset * 4), data, mem_mask);
		}
		else
		{
			internal_write_cs1((offset * 4) + 1, data >> 8, mem_mask >> 8);
		}
	}
	else if (ACCESSING_BITS_16_31)
	{
		if (ACCESSING_BITS_16_23)
		{
			internal_write_cs1((offset * 4) + 2, data >> 16, mem_mask >> 16);
		}
		else
		{
			internal_write_cs1((offset * 4) + 3, data >> 24, mem_mask >> 24);
		}
	}
}

#define IDE_BUSMASTER_STATUS_ACTIVE         0x01
#define IDE_BUSMASTER_STATUS_ERROR          0x02
#define IDE_BUSMASTER_STATUS_IRQ            0x04

DEFINE_DEVICE_TYPE(BUS_MASTER_IDE_CONTROLLER, bus_master_ide_controller_device, "idectrl32bm", "Bus Master IDE Controller")

bus_master_ide_controller_device::bus_master_ide_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ide_controller_32_device(mconfig, BUS_MASTER_IDE_CONTROLLER, tag, owner, clock),
	m_dma_space(*this, finder_base::DUMMY_TAG, -1, 32),
	m_dma_address(0),
	m_dma_bytes_left(0),
	m_dma_descriptor(0),
	m_dma_last_buffer(0),
	m_bus_master_command(0),
	m_bus_master_status(0),
	m_bus_master_descriptor(0),
	m_irq(0),
	m_dmarq(0)
{
}

void bus_master_ide_controller_device::device_start()
{
	ide_controller_32_device::device_start();

	/* find the bus master space */
	m_dma_address_xor = (m_dma_space->endianness() == ENDIANNESS_LITTLE) ? 0 : 3;

	save_item(NAME(m_dma_address));
	save_item(NAME(m_dma_bytes_left));
	save_item(NAME(m_dma_descriptor));
	save_item(NAME(m_dma_last_buffer));
	save_item(NAME(m_bus_master_command));
	save_item(NAME(m_bus_master_status));
	save_item(NAME(m_bus_master_descriptor));
}

void bus_master_ide_controller_device::set_irq(int state)
{
	abstract_ata_interface_device::set_irq(state);

	if (m_irq != state)
	{
		m_irq = state;

		if( m_irq )
			m_bus_master_status |= IDE_BUSMASTER_STATUS_IRQ;
	}
}

void bus_master_ide_controller_device::set_dmarq(int state)
{
	abstract_ata_interface_device::set_dmarq(state);

	if (m_dmarq != state)
	{
		m_dmarq = state;

		execute_dma();
	}
}

/*************************************
 *
 *  Bus master read
 *
 *************************************/

READ32_MEMBER( bus_master_ide_controller_device::bmdma_r )
{
	uint32_t result = 0;
	switch( offset )
	{
	case 0:
		/* command register/status register */
		result = m_bus_master_command | (m_bus_master_status << 16);
		break;
	case 1:
		/* descriptor table register */
		result = m_bus_master_descriptor;
		break;
	default:
		result = 0xffffffff;
		break;
	}

	LOG("%s:ide_bus_master32_r(%d, %08x, %08x)\n", machine().describe_context(), offset, mem_mask, result);

	return result;
}



/*************************************
 *
 *  Bus master write
 *
 *************************************/

WRITE32_MEMBER( bus_master_ide_controller_device::bmdma_w )
{
	LOG("%s:ide_bus_master32_w(%d, %08x, %08X)\n", machine().describe_context(), offset, mem_mask, data);

	switch( offset )
	{
	case 0:
		if( ACCESSING_BITS_0_7 )
		{
			/* Bus Master IDE Command register */
			uint8_t old = m_bus_master_command;
			uint8_t val = data & 0xff;

			/* save the "Read or Write Control" bit 3 and the "Start/Stop Bus Master" bit 0 */
			m_bus_master_command = (old & 0xf6) | (val & 0x09);

			if ((old ^ m_bus_master_command) & 1)
			{
				if (m_bus_master_command & 1)
				{
					/* handle starting a transfer */
					m_bus_master_status |= IDE_BUSMASTER_STATUS_ACTIVE;

					/* reset all the DMA data */
					m_dma_bytes_left = 0;
					m_dma_descriptor = m_bus_master_descriptor;

					/* if we're going live, start the pending read/write */
					execute_dma();
				}
				else if (m_bus_master_status & IDE_BUSMASTER_STATUS_ACTIVE)
				{
					m_bus_master_status &= ~IDE_BUSMASTER_STATUS_ACTIVE;

					LOG("DMA Aborted!\n");
				}
			}
		}

		if( ACCESSING_BITS_16_23 )
		{
			/* Bus Master IDE Status register */
			uint8_t old = m_bus_master_status;
			uint8_t val = data >> 16;

			/* save the DMA capable bits */
			m_bus_master_status = (old & 0x9f) | (val & 0x60);

			/* clear interrupt and error bits */
			if (val & IDE_BUSMASTER_STATUS_IRQ)
				m_bus_master_status &= ~IDE_BUSMASTER_STATUS_IRQ;
			if (val & IDE_BUSMASTER_STATUS_ERROR)
				m_bus_master_status &= ~IDE_BUSMASTER_STATUS_ERROR;
		}
		break;

	case 1:
		/* descriptor table register */
		m_bus_master_descriptor = data & 0xfffffffc;
		break;
	}
}

void bus_master_ide_controller_device::execute_dma()
{
	write_dmack(ASSERT_LINE);

	while (m_dmarq && (m_bus_master_status & IDE_BUSMASTER_STATUS_ACTIVE))
	{
		/* if we're out of space, grab the next Physical Region Descriptor */
		if (m_dma_bytes_left == 0)
		{
			/* fetch the address */
			m_dma_address = m_dma_space->read_byte(m_dma_descriptor++ ^ m_dma_address_xor);
			m_dma_address |= m_dma_space->read_byte(m_dma_descriptor++ ^ m_dma_address_xor) << 8;
			m_dma_address |= m_dma_space->read_byte(m_dma_descriptor++ ^ m_dma_address_xor) << 16;
			m_dma_address |= m_dma_space->read_byte(m_dma_descriptor++ ^ m_dma_address_xor) << 24;
			m_dma_address &= 0xfffffffe;

			/* fetch the length */
			m_dma_bytes_left = m_dma_space->read_byte(m_dma_descriptor++ ^ m_dma_address_xor);
			m_dma_bytes_left |= m_dma_space->read_byte(m_dma_descriptor++ ^ m_dma_address_xor) << 8;
			m_dma_bytes_left |= m_dma_space->read_byte(m_dma_descriptor++ ^ m_dma_address_xor) << 16;
			m_dma_bytes_left |= m_dma_space->read_byte(m_dma_descriptor++ ^ m_dma_address_xor) << 24;
			m_dma_last_buffer = (m_dma_bytes_left >> 31) & 1;
			m_dma_bytes_left &= 0xfffe;
			if (m_dma_bytes_left == 0)
				m_dma_bytes_left = 0x10000;
			LOG("New DMA descriptor: address = %08X  bytes = %04X  last = %d time: %s\n", m_dma_address, m_dma_bytes_left, m_dma_last_buffer, machine().time().as_string());
		}

		if (m_bus_master_command & 8)
		{
			// read from ata bus
			uint16_t data = read_dma();

			// write to memory
			m_dma_space->write_byte(m_dma_address++, data & 0xff);
			m_dma_space->write_byte(m_dma_address++, data >> 8);
		}
		else
		{
			// read from memory;
			uint16_t data = m_dma_space->read_byte(m_dma_address++);
			data |= m_dma_space->read_byte(m_dma_address++) << 8;

			// write to ata bus
			write_dma(data);
		}

		m_dma_bytes_left -= 2;

		if (m_dma_bytes_left == 0 && m_dma_last_buffer)
		{
			m_bus_master_status &= ~IDE_BUSMASTER_STATUS_ACTIVE;
			LOG("DMA Complete time: %s\n", machine().time().as_string());
			if (m_dmarq)
			{
				LOG("DMA Out of buffer space!\n");
			}
		}
	}

	write_dmack(CLEAR_LINE);
}
