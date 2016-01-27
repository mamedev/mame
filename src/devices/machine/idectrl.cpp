// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    idectrl.c

    Generic (PC-style) IDE controller implementation.

***************************************************************************/

#include "idectrl.h"


/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE                     0

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)


/***************************************************************************
    CONSTANTS
***************************************************************************/

const device_type IDE_CONTROLLER = &device_creator<ide_controller_device>;

ide_controller_device::ide_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	ata_interface_device(mconfig, IDE_CONTROLLER, "IDE Controller", tag, owner, clock, "ide_controller", __FILE__)
{
}

ide_controller_device::ide_controller_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	ata_interface_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

READ16_MEMBER( ide_controller_device::read_cs0 )
{
	if (mem_mask == 0xffff && offset == 1 ){ offset = 0; popmessage( "requires ide_controller_32_device" ); }
	if (mem_mask == 0xff00)
	{
		return ata_interface_device::read_cs0(space, (offset * 2) + 1, 0xff) << 8;
	}
	else
	{
		return ata_interface_device::read_cs0(space, offset * 2, mem_mask);
	}
}

READ16_MEMBER( ide_controller_device::read_cs1 )
{
	if (mem_mask == 0xff00)
	{
		return ata_interface_device::read_cs1(space, (offset * 2) + 1, 0xff) << 8;
	}
	else
	{
		return ata_interface_device::read_cs1(space, offset * 2, mem_mask);
	}
}

WRITE16_MEMBER( ide_controller_device::write_cs0 )
{
	if (mem_mask == 0xffff && offset == 1 ){ offset = 0; popmessage( "requires ide_controller_32_device" ); }
	if (mem_mask == 0xff00)
	{
		return ata_interface_device::write_cs0(space, (offset * 2) + 1, data >> 8, 0xff);
	}
	else
	{
		return ata_interface_device::write_cs0(space, offset * 2, data, mem_mask);
	}
}

WRITE16_MEMBER( ide_controller_device::write_cs1 )
{
	if (mem_mask == 0xff00)
	{
		return ata_interface_device::write_cs1(space, (offset * 2) + 1, data >> 8, 0xff);
	}
	else
	{
		return ata_interface_device::write_cs1(space, offset * 2, data, mem_mask);
	}
}


const device_type IDE_CONTROLLER_32 = &device_creator<ide_controller_32_device>;

ide_controller_32_device::ide_controller_32_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	ide_controller_device(mconfig, IDE_CONTROLLER, "IDE Controller (32 bit)", tag, owner, clock, "ide_controller32", __FILE__)
{
}

ide_controller_32_device::ide_controller_32_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	ide_controller_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

READ32_MEMBER(ide_controller_32_device::read_cs0)
{
	UINT32 data = 0;

	if (ACCESSING_BITS_0_15)
	{
		data = ide_controller_device::read_cs0(space, (offset * 2), mem_mask);

		if (offset == 0 && ACCESSING_BITS_16_31)
			data |= ide_controller_device::read_cs0(space, (offset * 2), mem_mask >> 16) << 16;
	}
	else if (ACCESSING_BITS_16_31)
	{
		data = ide_controller_device::read_cs0(space, (offset * 2) + 1, mem_mask >> 16) << 16;
	}

	return data;
}

READ32_MEMBER(ide_controller_32_device::read_cs1)
{
	UINT32 data = 0;

	if (ACCESSING_BITS_0_15)
	{
		data = ide_controller_device::read_cs1(space, (offset * 2), mem_mask);
	}
	else if (ACCESSING_BITS_16_23)
	{
		data = ide_controller_device::read_cs1(space, (offset * 2) + 1, mem_mask >> 16) << 16;
	}

	return data;
}

WRITE32_MEMBER(ide_controller_32_device::write_cs0)
{
	if (ACCESSING_BITS_0_15)
	{
		ide_controller_device::write_cs0(space, (offset * 2), data, mem_mask);

		if (offset == 0 && ACCESSING_BITS_16_31)
			ata_interface_device::write_cs0(space, (offset * 2), data >> 16, mem_mask >> 16);
	}
	else if (ACCESSING_BITS_16_31)
	{
		ide_controller_device::write_cs0(space, (offset * 2) + 1, data >> 16, mem_mask >> 16);
	}
}

WRITE32_MEMBER(ide_controller_32_device::write_cs1)
{
	if (ACCESSING_BITS_0_15)
	{
		ide_controller_device::write_cs1(space, (offset * 2), data, mem_mask);
	}
	else if (ACCESSING_BITS_16_31)
	{
		ide_controller_device::write_cs1(space, (offset * 2) + 1, data >> 16, mem_mask >> 16);
	}
}


#define IDE_BUSMASTER_STATUS_ACTIVE         0x01
#define IDE_BUSMASTER_STATUS_ERROR          0x02
#define IDE_BUSMASTER_STATUS_IRQ            0x04

const device_type BUS_MASTER_IDE_CONTROLLER = &device_creator<bus_master_ide_controller_device>;

bus_master_ide_controller_device::bus_master_ide_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	ide_controller_32_device(mconfig, BUS_MASTER_IDE_CONTROLLER, "Bus Master IDE Controller", tag, owner, clock, "bus_master_ide_controller", __FILE__),
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
	if (m_bmcpu != nullptr)
	{
		device_t *bmtarget = machine().device(m_bmcpu);
		if (bmtarget == nullptr)
			throw emu_fatalerror("IDE controller '%s' bus master target '%s' doesn't exist!", tag(), m_bmcpu);
		device_memory_interface *memory;
		if (!bmtarget->interface(memory))
			throw emu_fatalerror("IDE controller '%s' bus master target '%s' has no memory!", tag(), m_bmcpu);
		m_dma_space = &memory->space(m_bmspace);
		m_dma_address_xor = (m_dma_space->endianness() == ENDIANNESS_LITTLE) ? 0 : 3;
	}

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
	ata_interface_device::set_irq(state);

	if (m_irq != state)
	{
		m_irq = state;

		if( m_irq )
			m_bus_master_status |= IDE_BUSMASTER_STATUS_IRQ;
	}
}

void bus_master_ide_controller_device::set_dmarq(int state)
{
	ata_interface_device::set_dmarq(state);

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
	LOG(("%s:ide_bus_master32_r(%d, %08x)\n", machine().describe_context(), offset, mem_mask));

	switch( offset )
	{
	case 0:
		/* command register/status register */
		return m_bus_master_command | (m_bus_master_status << 16);

	case 1:
		/* descriptor table register */
		return m_bus_master_descriptor;
	}

	return 0xffffffff;
}



/*************************************
 *
 *  Bus master write
 *
 *************************************/

WRITE32_MEMBER( bus_master_ide_controller_device::bmdma_w )
{
	LOG(("%s:ide_bus_master32_w(%d, %08x, %08X)\n", machine().describe_context(), offset, mem_mask, data));

	switch( offset )
	{
	case 0:
		if( ACCESSING_BITS_0_7 )
		{
			/* Bus Master IDE Command register */
			UINT8 old = m_bus_master_command;
			UINT8 val = data & 0xff;

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

					LOG(("DMA Aborted!\n"));
				}
			}
		}

		if( ACCESSING_BITS_16_23 )
		{
			/* Bus Master IDE Status register */
			UINT8 old = m_bus_master_status;
			UINT8 val = data >> 16;

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

//          LOG(("New DMA descriptor: address = %08X  bytes = %04X  last = %d\n", m_dma_address, m_dma_bytes_left, m_dma_last_buffer));
		}

		if (m_bus_master_command & 8)
		{
			// read from ata bus
			UINT16 data = read_dma();

			// write to memory
			m_dma_space->write_byte(m_dma_address++, data & 0xff);
			m_dma_space->write_byte(m_dma_address++, data >> 8);
		}
		else
		{
			// read from memory;
			UINT16 data = m_dma_space->read_byte(m_dma_address++);
			data |= m_dma_space->read_byte(m_dma_address++) << 8;

			// write to ata bus
			write_dma(data);
		}

		m_dma_bytes_left -= 2;

		if (m_dma_bytes_left == 0 && m_dma_last_buffer)
		{
			m_bus_master_status &= ~IDE_BUSMASTER_STATUS_ACTIVE;

			if (m_dmarq)
			{
				LOG(("DMA Out of buffer space!\n"));
			}
		}
	}

	write_dmack(CLEAR_LINE);
}
