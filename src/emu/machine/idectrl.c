/***************************************************************************

    Generic (PC-style) IDE controller implementation

***************************************************************************/

#include "emu.h"
#include "idectrl.h"
#include "debugger.h"

/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE                     0
#define PRINTF_IDE_COMMANDS         0
#define PRINTF_IDE_PASSWORD         0

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

#define LOGPRINT(x) do { if (VERBOSE) logerror x; if (PRINTF_IDE_COMMANDS) mame_printf_debug x; } while (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define IDE_BANK2_CONFIG_UNK                4
#define IDE_BANK2_CONFIG_REGISTER           8
#define IDE_BANK2_CONFIG_DATA               0xc


void ide_controller_device::set_irq(int state)
{
	if (state == ASSERT_LINE)
		LOG(("IDE interrupt assert\n"));
	else
		LOG(("IDE interrupt clear\n"));

	/* signal an interrupt */
	m_irq_handler(state);
}

void ide_controller_device::set_dmarq(int state)
{
	m_dmarq_handler(state);
}

WRITE_LINE_MEMBER( ide_controller_device::irq0_write_line )
{
	m_irq[0] = state;

	set_irq(m_irq[0] == ASSERT_LINE || m_irq[1] == ASSERT_LINE);
}

WRITE_LINE_MEMBER( ide_controller_device::irq1_write_line )
{
	m_irq[1] = state;

	set_irq(m_irq[0] == ASSERT_LINE || m_irq[1] == ASSERT_LINE);
}

WRITE_LINE_MEMBER( ide_controller_device::dmarq0_write_line )
{
	m_dmarq[0] = state;

	set_dmarq(m_dmarq[0] == ASSERT_LINE || m_dmarq[1] == ASSERT_LINE);
}

WRITE_LINE_MEMBER( ide_controller_device::dmarq1_write_line )
{
	m_dmarq[1] = state;

	set_dmarq(m_dmarq[0] == ASSERT_LINE || m_dmarq[1] == ASSERT_LINE);
}


/***************************************************************************
    INITIALIZATION AND RESET
***************************************************************************/

UINT8 *ide_controller_device::ide_get_features(int _drive)
{
	return m_slot[_drive]->dev()->get_features();
}

void ide_controller_device::ide_set_master_password(int _drive, const UINT8 *password)
{
	m_slot[_drive]->dev()->m_master_password = password;
	m_slot[_drive]->dev()->m_master_password_enable = (password != NULL);
}


void ide_controller_device::ide_set_user_password(int _drive, const UINT8 *password)
{
	m_slot[_drive]->dev()->m_user_password = password;
	m_slot[_drive]->dev()->m_user_password_enable = (password != NULL);
}

/*************************************
 *
 *  IDE controller read
 *
 *************************************/

UINT16 ide_controller_device::read_dma()
{
	UINT16 result = 0xffff;
	for (int i = 0; i < 2; i++)
		if (m_slot[i]->dev() != NULL)
			result &= m_slot[i]->dev()->read_dma();

//  printf( "read_dma %04x\n", result );
	return result;
}

READ16_MEMBER( ide_controller_device::read_cs0 )
{
	UINT16 result = mem_mask;
	for (int i = 0; i < 2; i++)
		if (m_slot[i]->dev() != NULL)
			result &= m_slot[i]->dev()->read_cs0(space, offset, mem_mask);

//  printf( "read cs0 %04x %04x %04x\n", offset, result, mem_mask );

	return result;
}

READ16_MEMBER( ide_controller_device::read_cs1 )
{
	UINT16 result = mem_mask;
	for (int i = 0; i < 2; i++)
		if (m_slot[i]->dev() != NULL)
			result &= m_slot[i]->dev()->read_cs1(space, offset, mem_mask);

//  printf( "read cs1 %04x %04x %04x\n", offset, result, mem_mask );

	return result;
}

READ8_MEMBER( ide_controller_device::read_via_config )
{
	UINT16 result = 0;

	/* logit */
	LOG(("%s:IDE via config read at %X, mem_mask=%d\n", machine().describe_context(), offset, mem_mask));

	switch(offset)
	{
		/* unknown config register */
		case IDE_BANK2_CONFIG_UNK:
			result = m_config_unknown;
			break;

		/* active config register */
		case IDE_BANK2_CONFIG_REGISTER:
			result = m_config_register_num;
			break;

		/* data from active config register */
		case IDE_BANK2_CONFIG_DATA:
			if (m_config_register_num < IDE_CONFIG_REGISTERS)
				result = m_config_register[m_config_register_num];
			break;

		default:
			logerror("%s:unknown IDE via config read at %03X, mem_mask=%d\n", machine().describe_context(), offset, mem_mask);
			break;
	}

//  printf( "read via config %04x %04x %04x\n", offset, result, mem_mask );
	return result;
}

READ16_MEMBER( ide_controller_device::read_cs0_pc )
{
	if (mem_mask == 0xffff && offset == 1 ) offset = 0; // hack for 32 bit read of data register
	if (mem_mask == 0xff00)
	{
		return read_cs0(space, (offset * 2) + 1, 0xff) << 8;
	}
	else
	{
		return read_cs0(space, offset * 2, mem_mask);
	}
}

READ16_MEMBER( ide_controller_device::read_cs1_pc )
{
	if (mem_mask == 0xff00)
	{
		return read_cs1(space, (offset * 2) + 1, 0xff) << 8;
	}
	else
	{
		return read_cs1(space, offset * 2, mem_mask);
	}
}


/*************************************
 *
 *  IDE controller write
 *
 *************************************/

void ide_controller_device::write_dma( UINT16 data )
{
//  printf( "write_dma %04x\n", data );

	for (int i = 0; i < 2; i++)
		if (m_slot[i]->dev() != NULL)
			m_slot[i]->dev()->write_dma(data);
}

WRITE16_MEMBER( ide_controller_device::write_cs0 )
{
//  printf( "write cs0 %04x %04x %04x\n", offset, data, mem_mask );

	for (int i = 0; i < 2; i++)
		if (m_slot[i]->dev() != NULL)
			m_slot[i]->dev()->write_cs0(space, offset, data, mem_mask);
}

WRITE16_MEMBER( ide_controller_device::write_cs1 )
{
//  printf( "write cs1 %04x %04x %04x\n", offset, data, mem_mask );

	for (int i = 0; i < 2; i++)
		if (m_slot[i]->dev() != NULL)
			m_slot[i]->dev()->write_cs1(space, offset, data, mem_mask);
}

WRITE8_MEMBER( ide_controller_device::write_via_config )
{
//  printf( "write via config %04x %04x %04x\n", offset, data, mem_mask );

	/* logit */
	LOG(("%s:IDE via config write to %X = %08X, mem_mask=%d\n", machine().describe_context(), offset, data, mem_mask));

	switch (offset)
	{
		/* unknown config register */
		case IDE_BANK2_CONFIG_UNK:
			m_config_unknown = data;
			break;

		/* active config register */
		case IDE_BANK2_CONFIG_REGISTER:
			m_config_register_num = data;
			break;

		/* data from active config register */
		case IDE_BANK2_CONFIG_DATA:
			if (m_config_register_num < IDE_CONFIG_REGISTERS)
				m_config_register[m_config_register_num] = data;
			break;
	}
}

WRITE16_MEMBER( ide_controller_device::write_cs0_pc )
{
	if (mem_mask == 0xffff && offset == 1 ) offset = 0; // hack for 32 bit write to data register
	if (mem_mask == 0xff00)
	{
		return write_cs0(space, (offset * 2) + 1, data >> 8, 0xff);
	}
	else
	{
		return write_cs0(space, offset * 2, data, mem_mask);
	}
}

WRITE16_MEMBER( ide_controller_device::write_cs1_pc )
{
	if (mem_mask == 0xff00)
	{
		return write_cs1(space, (offset * 2) + 1, data >> 8, 0xff);
	}
	else
	{
		return write_cs1(space, offset * 2, data, mem_mask);
	}
}

SLOT_INTERFACE_START(ide_devices)
	SLOT_INTERFACE("hdd", IDE_HARDDISK)
SLOT_INTERFACE_END

ide_controller_device::ide_controller_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, type, name, tag, owner, clock),
	m_config_unknown(0),
	m_config_register_num(0),
	m_irq_handler(*this),
	m_dmarq_handler(*this)
{
}


const device_type IDE_CONTROLLER = &device_creator<ide_controller_device>;

ide_controller_device::ide_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, IDE_CONTROLLER, "IDE Controller", tag, owner, clock),
	m_config_unknown(0),
	m_config_register_num(0),
	m_irq_handler(*this),
	m_dmarq_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ide_controller_device::device_start()
{
	m_irq_handler.resolve_safe();
	m_dmarq_handler.resolve_safe();

	/* set MAME harddisk handle */
	m_slot[0] = subdevice<ide_slot_device>("0");
	m_slot[1] = subdevice<ide_slot_device>("1");

	for (int i = 0; i < 2; i++)
	{
		m_irq[i] = 0;
		m_dmarq[i] = 0;

		ide_device_interface *dev = m_slot[i]->dev();
		if (dev != NULL)
		{
			if (i == 0)
			{
				dev->m_irq_handler.set_callback(DEVCB2_DEVWRITELINE("^", ide_controller_device, irq0_write_line));
				dev->m_dmarq_handler.set_callback(DEVCB2_DEVWRITELINE("^", ide_controller_device, dmarq0_write_line));
			}
			else
			{
				dev->m_irq_handler.set_callback(DEVCB2_DEVWRITELINE("^", ide_controller_device, irq1_write_line));
				dev->m_dmarq_handler.set_callback(DEVCB2_DEVWRITELINE("^", ide_controller_device, dmarq1_write_line));
			}

			dev->m_csel = i;
			dev->m_dasp = m_slot[1]->dev() != NULL;
		}
	}

	/* register ide states */
	save_item(NAME(m_config_unknown));
	save_item(NAME(m_config_register));
	save_item(NAME(m_config_register_num));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ide_controller_device::device_reset()
{
	LOG(("IDE controller reset performed\n"));
}



//**************************************************************************
//  IDE SLOT DEVICE
//**************************************************************************

// device type definition
const device_type IDE_SLOT = &device_creator<ide_slot_device>;

//-------------------------------------------------
//  ide_slot_device - constructor
//-------------------------------------------------

ide_slot_device::ide_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, IDE_SLOT, "IDE Connector", tag, owner, clock),
		device_slot_interface(mconfig, *this),
		m_dev(NULL)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ide_slot_device::device_config_complete()
{
	m_dev = dynamic_cast<ide_device_interface *>(get_card_device());
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ide_slot_device::device_start()
{
}



#define IDE_BUSMASTER_STATUS_ACTIVE         0x01
#define IDE_BUSMASTER_STATUS_ERROR          0x02
#define IDE_BUSMASTER_STATUS_IRQ            0x04

const device_type BUS_MASTER_IDE_CONTROLLER = &device_creator<bus_master_ide_controller_device>;

bus_master_ide_controller_device::bus_master_ide_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	ide_controller_device(mconfig, BUS_MASTER_IDE_CONTROLLER, "Bus Master IDE Controller", tag, owner, clock),
	dma_address(0),
	dma_bytes_left(0),
	dma_descriptor(0),
	dma_last_buffer(0),
	bus_master_command(0),
	bus_master_status(0),
	bus_master_descriptor(0)
{
}

void bus_master_ide_controller_device::device_start()
{
	ide_controller_device::device_start();

	/* find the bus master space */
	if (bmcpu != NULL)
	{
		device_t *bmtarget = machine().device(bmcpu);
		if (bmtarget == NULL)
			throw emu_fatalerror("IDE controller '%s' bus master target '%s' doesn't exist!", tag(), bmcpu);
		device_memory_interface *memory;
		if (!bmtarget->interface(memory))
			throw emu_fatalerror("IDE controller '%s' bus master target '%s' has no memory!", tag(), bmcpu);
		dma_space = &memory->space(bmspace);
		dma_address_xor = (dma_space->endianness() == ENDIANNESS_LITTLE) ? 0 : 3;
	}

	save_item(NAME(dma_address));
	save_item(NAME(dma_bytes_left));
	save_item(NAME(dma_descriptor));
	save_item(NAME(dma_last_buffer));
	save_item(NAME(bus_master_command));
	save_item(NAME(bus_master_status));
	save_item(NAME(bus_master_descriptor));
}

void bus_master_ide_controller_device::set_irq(int state)
{
	ide_controller_device::set_irq(state);

	if (m_irq != state)
	{
		m_irq = state;

		if( m_irq )
			bus_master_status |= IDE_BUSMASTER_STATUS_IRQ;
	}
}

void bus_master_ide_controller_device::set_dmarq(int state)
{
	ide_controller_device::set_dmarq(state);

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

READ32_MEMBER( bus_master_ide_controller_device::ide_bus_master32_r )
{
	LOG(("%s:ide_bus_master32_r(%d, %08x)\n", machine().describe_context(), offset, mem_mask));

	switch( offset )
	{
	case 0:
		/* command register/status register */
		return bus_master_command | (bus_master_status << 16);

	case 1:
		/* descriptor table register */
		return bus_master_descriptor;
	}

	return 0xffffffff;
}



/*************************************
 *
 *  Bus master write
 *
 *************************************/

WRITE32_MEMBER( bus_master_ide_controller_device::ide_bus_master32_w )
{
	LOG(("%s:ide_bus_master32_w(%d, %08x, %08X)\n", machine().describe_context(), offset, mem_mask, data));

	switch( offset )
	{
	case 0:
		if( ACCESSING_BITS_0_7 )
		{
			/* command register */
			UINT8 old = bus_master_command;
			UINT8 val = data & 0xff;

			/* save the read/write bit and the start/stop bit */
			bus_master_command = (old & 0xf6) | (val & 0x09);

			if ((old ^ bus_master_command) & 1)
			{
				if (bus_master_command & 1)
				{
					/* handle starting a transfer */
					bus_master_status |= IDE_BUSMASTER_STATUS_ACTIVE;

					/* reset all the DMA data */
					dma_bytes_left = 0;
					dma_descriptor = bus_master_descriptor;

					/* if we're going live, start the pending read/write */
					execute_dma();
				}
				else if (bus_master_status & IDE_BUSMASTER_STATUS_ACTIVE)
				{
					bus_master_status &= ~IDE_BUSMASTER_STATUS_ACTIVE;

					LOG(("DMA Aborted!\n"));
				}
			}
		}

		if( ACCESSING_BITS_16_23 )
		{
			/* status register */
			UINT8 old = bus_master_status;
			UINT8 val = data >> 16;

			/* save the DMA capable bits */
			bus_master_status = (old & 0x9f) | (val & 0x60);

			/* clear interrupt and error bits */
			if (val & IDE_BUSMASTER_STATUS_IRQ)
				bus_master_status &= ~IDE_BUSMASTER_STATUS_IRQ;
			if (val & IDE_BUSMASTER_STATUS_ERROR)
				bus_master_status &= ~IDE_BUSMASTER_STATUS_ERROR;
		}
		break;

	case 1:
		/* descriptor table register */
		bus_master_descriptor = data & 0xfffffffc;
		break;
	}
}

void bus_master_ide_controller_device::execute_dma()
{
	while (m_dmarq && (bus_master_status & IDE_BUSMASTER_STATUS_ACTIVE))
	{
		/* if we're out of space, grab the next descriptor */
		if (dma_bytes_left == 0)
		{
			/* fetch the address */
			dma_address = dma_space->read_byte(dma_descriptor++ ^ dma_address_xor);
			dma_address |= dma_space->read_byte(dma_descriptor++ ^ dma_address_xor) << 8;
			dma_address |= dma_space->read_byte(dma_descriptor++ ^ dma_address_xor) << 16;
			dma_address |= dma_space->read_byte(dma_descriptor++ ^ dma_address_xor) << 24;
			dma_address &= 0xfffffffe;

			/* fetch the length */
			dma_bytes_left = dma_space->read_byte(dma_descriptor++ ^ dma_address_xor);
			dma_bytes_left |= dma_space->read_byte(dma_descriptor++ ^ dma_address_xor) << 8;
			dma_bytes_left |= dma_space->read_byte(dma_descriptor++ ^ dma_address_xor) << 16;
			dma_bytes_left |= dma_space->read_byte(dma_descriptor++ ^ dma_address_xor) << 24;
			dma_last_buffer = (dma_bytes_left >> 31) & 1;
			dma_bytes_left &= 0xfffe;
			if (dma_bytes_left == 0)
				dma_bytes_left = 0x10000;

//          LOG(("New DMA descriptor: address = %08X  bytes = %04X  last = %d\n", dma_address, dma_bytes_left, dma_last_buffer));
		}

		if (bus_master_command & 8)
		{
			// read from ata bus
			UINT16 data = read_dma();

			// write to memory
			dma_space->write_byte(dma_address++, data & 0xff);
			dma_space->write_byte(dma_address++, data >> 8);
		}
		else
		{
			// read from memory;
			UINT16 data = dma_space->read_byte(dma_address++);
			data |= dma_space->read_byte(dma_address++) << 8;

			// write to ata bus
			write_dma(data);
		}

		dma_bytes_left -= 2;

		if (dma_bytes_left == 0 && dma_last_buffer)
		{
			bus_master_status &= ~IDE_BUSMASTER_STATUS_ACTIVE;

			if (m_dmarq)
			{
				LOG(("DMA Out of buffer space!\n"));
			}
		}
	}
}
