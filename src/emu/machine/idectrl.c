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

#define MINIMUM_COMMAND_TIME                (attotime::from_usec(10))

#define TIME_PER_SECTOR                     (attotime::from_usec(100))
#define TIME_PER_ROTATION                   (attotime::from_hz(5400/60))
#define TIME_SECURITY_ERROR                 (attotime::from_msec(1000))

#define TIME_SEEK_MULTISECTOR               (attotime::from_msec(13))
#define TIME_NO_SEEK_MULTISECTOR            (attotime::from_nsec(16300))

#define IDE_STATUS_ERROR                    0x01
#define IDE_STATUS_HIT_INDEX                0x02
#define IDE_STATUS_BUFFER_READY             0x08
#define IDE_STATUS_SEEK_COMPLETE            0x10
#define IDE_STATUS_DRIVE_READY              0x40
#define IDE_STATUS_BUSY                     0x80

#define IDE_BANK0_DATA                      0
#define IDE_BANK0_ERROR                     1
#define IDE_BANK0_SECTOR_COUNT              2
#define IDE_BANK0_SECTOR_NUMBER             3
#define IDE_BANK0_CYLINDER_LSB              4
#define IDE_BANK0_CYLINDER_MSB              5
#define IDE_BANK0_HEAD_NUMBER               6
#define IDE_BANK0_STATUS_COMMAND            7

#define IDE_BANK1_STATUS_CONTROL            6

#define IDE_BANK2_CONFIG_UNK                4
#define IDE_BANK2_CONFIG_REGISTER           8
#define IDE_BANK2_CONFIG_DATA               0xc

#define IDE_COMMAND_READ_MULTIPLE           0x20
#define IDE_COMMAND_READ_MULTIPLE_NORETRY   0x21
#define IDE_COMMAND_WRITE_MULTIPLE          0x30
#define IDE_COMMAND_WRITE_MULTIPLE_NORETRY  0x31
#define IDE_COMMAND_DIAGNOSTIC              0x90
#define IDE_COMMAND_SET_CONFIG              0x91
#define IDE_COMMAND_READ_MULTIPLE_BLOCK     0xc4
#define IDE_COMMAND_WRITE_MULTIPLE_BLOCK    0xc5
#define IDE_COMMAND_SET_BLOCK_COUNT         0xc6
#define IDE_COMMAND_READ_DMA                0xc8
#define IDE_COMMAND_WRITE_DMA               0xca
#define IDE_COMMAND_GET_INFO                0xec
#define IDE_COMMAND_SET_FEATURES            0xef
#define IDE_COMMAND_SECURITY_UNLOCK         0xf2
#define IDE_COMMAND_UNKNOWN_F9              0xf9
#define IDE_COMMAND_VERIFY_MULTIPLE         0x40
#define IDE_COMMAND_VERIFY_NORETRY          0x41
#define IDE_COMMAND_ATAPI_IDENTIFY          0xa1
#define IDE_COMMAND_RECALIBRATE             0x10
#define IDE_COMMAND_SEEK                    0x70
#define IDE_COMMAND_IDLE_IMMEDIATE          0xe1
#define IDE_COMMAND_IDLE                    0xe3
#define IDE_COMMAND_TAITO_GNET_UNLOCK_1     0xfe
#define IDE_COMMAND_TAITO_GNET_UNLOCK_2     0xfc
#define IDE_COMMAND_TAITO_GNET_UNLOCK_3     0x0f

#define IDE_ERROR_NONE                      0x00
#define IDE_ERROR_DEFAULT                   0x01
#define IDE_ERROR_TRACK0_NOT_FOUND          0x02
#define IDE_ERROR_UNKNOWN_COMMAND           0x04
#define IDE_ERROR_BAD_LOCATION              0x10
#define IDE_ERROR_BAD_SECTOR                0x80


void ide_controller_device::set_irq(int state)
{
	if (state == ASSERT_LINE)
		LOG(("IDE interrupt assert\n"));
	else
		LOG(("IDE interrupt clear\n"));
	
	/* signal an interrupt */
	m_irq_handler(state);
	interrupt_pending = state;
}

void ide_controller_device::set_dmarq(int state)
{
}

enum
{
	TID_NULL,
	TID_DELAYED_INTERRUPT,
	TID_DELAYED_INTERRUPT_BUFFER_READY,
	TID_RESET_CALLBACK,
	TID_SECURITY_ERROR_DONE,
	TID_READ_SECTOR_DONE_CALLBACK,
	TID_WRITE_SECTOR_DONE_CALLBACK
};

void ide_controller_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TID_DELAYED_INTERRUPT:
		status &= ~IDE_STATUS_BUSY;
		set_irq(ASSERT_LINE);
		break;

	case TID_DELAYED_INTERRUPT_BUFFER_READY:
		status &= ~IDE_STATUS_BUSY;
		status |= IDE_STATUS_BUFFER_READY;
		set_irq(ASSERT_LINE);
		break;

	case TID_RESET_CALLBACK:
		reset();
		break;

	case TID_SECURITY_ERROR_DONE:
		/* clear error state */
		status &= ~IDE_STATUS_ERROR;
		status |= IDE_STATUS_DRIVE_READY;
		break;

	case TID_READ_SECTOR_DONE_CALLBACK:
		read_sector_done();
		break;

	case TID_WRITE_SECTOR_DONE_CALLBACK:
		write_sector_done();
		break;
	}
}


void ide_controller_device::signal_delayed_interrupt(attotime time, int buffer_ready)
{
	/* clear buffer ready and set the busy flag */
	status &= ~IDE_STATUS_BUFFER_READY;
	status |= IDE_STATUS_BUSY;

	/* set a timer */
	if (buffer_ready)
		timer_set(time, TID_DELAYED_INTERRUPT_BUFFER_READY);
	else
		timer_set(time, TID_DELAYED_INTERRUPT);
}



/***************************************************************************
    INITIALIZATION AND RESET
***************************************************************************/

UINT8 *ide_controller_device::ide_get_features(int _drive)
{
	return slot[_drive]->dev()->get_features();
}

void ide_controller_device::ide_set_gnet_readlock(const UINT8 onoff)
{
	gnetreadlock = onoff;
}

void ide_controller_device::ide_set_master_password(const UINT8 *password)
{
	master_password = password;
	master_password_enable = (master_password != NULL);
}


void ide_controller_device::ide_set_user_password(const UINT8 *password)
{
	user_password = password;
	user_password_enable = (user_password != NULL);
}



/*************************************
 *
 *  Advance to the next sector
 *
 *************************************/

void ide_controller_device::next_sector()
{
	/* LBA direct? */
	ide_device_interface *dev = slot[cur_drive]->dev();
	if (dev->cur_head_reg & 0x40)
	{
		dev->cur_sector++;
		if (dev->cur_sector == 0)
		{
			dev->cur_cylinder++;
			if (dev->cur_cylinder == 0)
				dev->cur_head++;
		}
	}

	/* standard CHS */
	else
	{
		/* sectors are 1-based */
		dev->cur_sector++;
		if (dev->cur_sector > dev->get_sectors())
		{
			/* heads are 0 based */
			dev->cur_sector = 1;
			dev->cur_head++;
			if (dev->cur_head >= dev->get_heads())
			{
				dev->cur_head = 0;
				dev->cur_cylinder++;
			}
		}
	}

	dev->cur_lba = dev->lba_address();
}



/*************************************
 *
 *  security error handling
 *
 *************************************/

void ide_controller_device::security_error()
{
	/* set error state */
	status |= IDE_STATUS_ERROR;
	status &= ~IDE_STATUS_DRIVE_READY;

	/* just set a timer and mark ourselves error */
	timer_set(TIME_SECURITY_ERROR, TID_SECURITY_ERROR_DONE);
}



/*************************************
 *
 *  Sector reading
 *
 *************************************/

void ide_controller_device::read_buffer_empty()
{
	/* reset the totals */
	buffer_offset = 0;

	/* clear the buffer ready and busy flag */
	status &= ~IDE_STATUS_BUFFER_READY;
	status &= ~IDE_STATUS_BUSY;
	error = IDE_ERROR_DEFAULT;
	set_dmarq(0);

	if (master_password_enable || user_password_enable)
	{
		security_error();

		sector_count = 0;

		return;
	}

	/* if there is more data to read, keep going */
	if (sector_count > 0)
		sector_count--;
	if (sector_count > 0)
		read_next_sector();
}


void ide_controller_device::read_sector_done()
{
	ide_device_interface *dev = slot[cur_drive]->dev();
	int lba = dev->lba_address(), count = 0;

	/* GNET readlock check */
	if (gnetreadlock) {
		status &= ~IDE_STATUS_ERROR;
		status &= ~IDE_STATUS_BUSY;
		return;
	}
	/* now do the read */
	count = dev->read_sector(lba, buffer);

	/* by default, mark the buffer ready and the seek complete */
	if (!verify_only)
		status |= IDE_STATUS_BUFFER_READY;
	status |= IDE_STATUS_SEEK_COMPLETE;

	/* and clear the busy and error flags */
	status &= ~IDE_STATUS_ERROR;
	status &= ~IDE_STATUS_BUSY;

	/* if we succeeded, advance to the next sector and set the nice bits */
	if (count == 1)
	{
		/* advance the pointers, unless this is the last sector */
		/* Gauntlet: Dark Legacy checks to make sure we stop on the last sector */
		if (sector_count != 1)
			next_sector();

		/* clear the error value */
		error = IDE_ERROR_NONE;

		/* signal an interrupt */
		if (!verify_only)
			sectors_until_int--;
		if (sectors_until_int == 0 || sector_count == 1)
		{
			sectors_until_int = ((command == IDE_COMMAND_READ_MULTIPLE_BLOCK) ? block_count : 1);
			set_irq(ASSERT_LINE);
		}

		/* handle DMA */
		if (dma_active)
			set_dmarq(1);

		/* if we're just verifying we can read the next sector */
		if (verify_only)
			read_buffer_empty();
	}

	/* if we got an error, we need to report it */
	else
	{
		/* set the error flag and the error */
		status |= IDE_STATUS_ERROR;
		error = IDE_ERROR_BAD_SECTOR;

		/* signal an interrupt */
		set_irq(ASSERT_LINE);
	}
}


void ide_controller_device::read_first_sector()
{
	ide_device_interface *dev = slot[cur_drive]->dev();
	/* mark ourselves busy */
	status |= IDE_STATUS_BUSY;

	/* just set a timer */
	if (command == IDE_COMMAND_READ_MULTIPLE_BLOCK)
	{
		int new_lba = dev->lba_address();
		attotime seek_time;

		if (new_lba == dev->cur_lba || new_lba == dev->cur_lba + 1)
			seek_time = TIME_NO_SEEK_MULTISECTOR;
		else
			seek_time = TIME_SEEK_MULTISECTOR;

		dev->cur_lba = new_lba;
		timer_set(seek_time, TID_READ_SECTOR_DONE_CALLBACK);
	}
	else
		timer_set(TIME_PER_SECTOR, TID_READ_SECTOR_DONE_CALLBACK);
}


void ide_controller_device::read_next_sector()
{
	/* mark ourselves busy */
	status |= IDE_STATUS_BUSY;

	if (command == IDE_COMMAND_READ_MULTIPLE_BLOCK)
	{
		if (sectors_until_int != 1)
			/* make ready now */
			read_sector_done();
		else
			/* just set a timer */
			timer_set(attotime::from_usec(1), TID_READ_SECTOR_DONE_CALLBACK);
	}
	else
		/* just set a timer */
		timer_set(TIME_PER_SECTOR, TID_READ_SECTOR_DONE_CALLBACK);
}



/*************************************
 *
 *  Sector writing
 *
 *************************************/

void ide_controller_device::continue_write()
{
	/* reset the totals */
	buffer_offset = 0;

	/* clear the buffer ready flag */
	status &= ~IDE_STATUS_BUFFER_READY;
	status |= IDE_STATUS_BUSY;

	if (command == IDE_COMMAND_WRITE_MULTIPLE_BLOCK)
	{
		if (sectors_until_int != 1)
		{
			/* ready to write now */
			write_sector_done();
		}
		else
		{
			/* set a timer to do the write */
			timer_set(TIME_PER_SECTOR, TID_WRITE_SECTOR_DONE_CALLBACK);
		}
	}
	else
	{
		/* set a timer to do the write */
		timer_set(TIME_PER_SECTOR, TID_WRITE_SECTOR_DONE_CALLBACK);
	}
}


void ide_controller_device::write_buffer_full()
{
	ide_device_interface *dev = slot[cur_drive]->dev();

	set_dmarq(0);
	if (command == IDE_COMMAND_SECURITY_UNLOCK)
	{
		if (user_password_enable && memcmp(buffer, user_password, 2 + 32) == 0)
		{
			LOGPRINT(("IDE Unlocked user password\n"));
			user_password_enable = 0;
		}
		if (master_password_enable && memcmp(buffer, master_password, 2 + 32) == 0)
		{
			LOGPRINT(("IDE Unlocked master password\n"));
			master_password_enable = 0;
		}
		if (PRINTF_IDE_PASSWORD)
		{
			int i;

			for (i = 0; i < 34; i += 2)
			{
				if (i % 8 == 2)
					mame_printf_debug("\n");

				mame_printf_debug("0x%02x, 0x%02x, ", buffer[i], buffer[i + 1]);
				//mame_printf_debug("0x%02x%02x, ", buffer[i], buffer[i + 1]);
			}
			mame_printf_debug("\n");
		}

		/* clear the busy and error flags */
		status &= ~IDE_STATUS_ERROR;
		status &= ~IDE_STATUS_BUSY;
		status &= ~IDE_STATUS_BUFFER_READY;

		if (master_password_enable || user_password_enable)
			security_error();
		else
			status |= IDE_STATUS_DRIVE_READY;
	}
	else if (command == IDE_COMMAND_TAITO_GNET_UNLOCK_2)
	{
		UINT8 key[5] = { 0 };
		int i, bad = 0;
		dev->read_key(key);

		for (i=0; !bad && i<512; i++)
			bad = ((i < 2 || i >= 7) && buffer[i]) || ((i >= 2 && i < 7) && buffer[i] != key[i-2]);

		status &= ~IDE_STATUS_BUSY;
		status &= ~IDE_STATUS_BUFFER_READY;
		if (bad)
			status |= IDE_STATUS_ERROR;
		else {
			status &= ~IDE_STATUS_ERROR;
			gnetreadlock= 0;
		}
	}
	else
	{
		continue_write();
	}
}


void ide_controller_device::write_sector_done()
{
	ide_device_interface *dev = slot[cur_drive]->dev();
	int lba = dev->lba_address(), count = 0;

	/* now do the write */
	count = dev->write_sector(lba, buffer);

	/* by default, mark the buffer ready and the seek complete */
	status |= IDE_STATUS_BUFFER_READY;
	status |= IDE_STATUS_SEEK_COMPLETE;

	/* and clear the busy adn error flags */
	status &= ~IDE_STATUS_ERROR;
	status &= ~IDE_STATUS_BUSY;

	/* if we succeeded, advance to the next sector and set the nice bits */
	if (count == 1)
	{
		/* advance the pointers, unless this is the last sector */
		/* Gauntlet: Dark Legacy checks to make sure we stop on the last sector */
		if (sector_count != 1)
			next_sector();

		/* clear the error value */
		error = IDE_ERROR_NONE;

		/* signal an interrupt */
		if (--sectors_until_int == 0 || sector_count == 1)
		{
			sectors_until_int = ((command == IDE_COMMAND_WRITE_MULTIPLE_BLOCK) ? block_count : 1);
			set_irq(ASSERT_LINE);
		}

		/* signal an interrupt if there's more data needed */
		if (sector_count > 0)
			sector_count--;
		if (sector_count == 0)
			status &= ~IDE_STATUS_BUFFER_READY;

		/* keep going for DMA */
		if (dma_active && sector_count != 0)
		{
			set_dmarq(1);
		}
	}

	/* if we got an error, we need to report it */
	else
	{
		/* set the error flag and the error */
		status |= IDE_STATUS_ERROR;
		error = IDE_ERROR_BAD_SECTOR;

		/* signal an interrupt */
		set_irq(ASSERT_LINE);
	}
}



/*************************************
 *
 *  Handle IDE commands
 *
 *************************************/

void ide_controller_device::handle_command(UINT8 _command)
{
	UINT8 key[5];
	ide_device_interface *dev = slot[cur_drive]->dev();

	/* implicitly clear interrupts & dmarq here */
	set_irq(CLEAR_LINE);
	set_dmarq(0);
	command = _command;
	switch (command)
	{
		case IDE_COMMAND_READ_MULTIPLE:
		case IDE_COMMAND_READ_MULTIPLE_NORETRY:
			LOGPRINT(("IDE Read multiple: C=%d H=%d S=%d LBA=%d count=%d\n",
				dev->cur_cylinder, dev->cur_head, dev->cur_sector, dev->lba_address(), sector_count));

			/* reset the buffer */
			buffer_offset = 0;
			sectors_until_int = 1;
			dma_active = 0;
			verify_only = 0;

			/* start the read going */
			read_first_sector();
			break;

		case IDE_COMMAND_READ_MULTIPLE_BLOCK:
			LOGPRINT(("IDE Read multiple block: C=%d H=%d S=%d LBA=%d count=%d\n",
				dev->cur_cylinder, dev->cur_head, dev->cur_sector, dev->lba_address(), sector_count));

			/* reset the buffer */
			buffer_offset = 0;
			sectors_until_int = 1;
			dma_active = 0;
			verify_only = 0;

			/* start the read going */
			read_first_sector();
			break;

		case IDE_COMMAND_VERIFY_MULTIPLE:
		case IDE_COMMAND_VERIFY_NORETRY:
			LOGPRINT(("IDE Read verify multiple with/without retries: C=%d H=%d S=%d LBA=%d count=%d\n",
				dev->cur_cylinder, dev->cur_head, dev->cur_sector, dev->lba_address(), sector_count));

			/* reset the buffer */
			buffer_offset = 0;
			sectors_until_int = 1;
			dma_active = 0;
			verify_only = 1;

			/* start the read going */
			read_first_sector();
			break;

		case IDE_COMMAND_READ_DMA:
			LOGPRINT(("IDE Read multiple DMA: C=%d H=%d S=%d LBA=%d count=%d\n",
				dev->cur_cylinder, dev->cur_head, dev->cur_sector, dev->lba_address(), sector_count));

			/* reset the buffer */
			buffer_offset = 0;
			sectors_until_int = sector_count;
			dma_active = 1;
			verify_only = 0;

			/* start the read going */
			read_first_sector();
			break;

		case IDE_COMMAND_WRITE_MULTIPLE:
		case IDE_COMMAND_WRITE_MULTIPLE_NORETRY:
			LOGPRINT(("IDE Write multiple: C=%d H=%d S=%d LBA=%d count=%d\n",
				dev->cur_cylinder, dev->cur_head, dev->cur_sector, dev->lba_address(), sector_count));

			/* reset the buffer */
			buffer_offset = 0;
			sectors_until_int = 1;
			dma_active = 0;

			/* mark the buffer ready */
			status |= IDE_STATUS_BUFFER_READY;
			break;

		case IDE_COMMAND_WRITE_MULTIPLE_BLOCK:
			LOGPRINT(("IDE Write multiple block: C=%d H=%d S=%d LBA=%d count=%d\n",
				dev->cur_cylinder, dev->cur_head, dev->cur_sector, dev->lba_address(), sector_count));

			/* reset the buffer */
			buffer_offset = 0;
			sectors_until_int = 1;
			dma_active = 0;

			/* mark the buffer ready */
			status |= IDE_STATUS_BUFFER_READY;
			break;

		case IDE_COMMAND_WRITE_DMA:
			LOGPRINT(("IDE Write multiple DMA: C=%d H=%d S=%d LBA=%d count=%d\n",
				dev->cur_cylinder, dev->cur_head, dev->cur_sector, dev->lba_address(), sector_count));

			/* reset the buffer */
			buffer_offset = 0;
			sectors_until_int = sector_count;
			dma_active = 1;

			/* start the read going */
			set_dmarq(1);
			break;

		case IDE_COMMAND_SECURITY_UNLOCK:
			LOGPRINT(("IDE Security Unlock\n"));

			/* reset the buffer */
			buffer_offset = 0;
			sectors_until_int = 0;
			dma_active = 0;

			/* mark the buffer ready */
			status |= IDE_STATUS_BUFFER_READY;
			set_irq(ASSERT_LINE);
			break;

		case IDE_COMMAND_GET_INFO:
			LOGPRINT(("IDE Read features\n"));

			/* reset the buffer */
			buffer_offset = 0;
			sector_count = 1;

			/* build the features page */
			memcpy(buffer, slot[cur_drive]->dev()->get_features(), sizeof(buffer));

			/* indicate everything is ready */
			status |= IDE_STATUS_BUFFER_READY;
			status |= IDE_STATUS_SEEK_COMPLETE;
			status |= IDE_STATUS_DRIVE_READY;

			/* and clear the busy adn error flags */
			status &= ~IDE_STATUS_ERROR;
			status &= ~IDE_STATUS_BUSY;

			/* clear the error too */
			error = IDE_ERROR_NONE;

			/* signal an interrupt */
			signal_delayed_interrupt(MINIMUM_COMMAND_TIME, 1);
			break;

		case IDE_COMMAND_DIAGNOSTIC:
			error = IDE_ERROR_DEFAULT;

			/* signal an interrupt */
			signal_delayed_interrupt(MINIMUM_COMMAND_TIME, 0);
			break;

		case IDE_COMMAND_RECALIBRATE:
			/* clear the error too */
			error = IDE_ERROR_NONE;
			/* signal an interrupt */
			signal_delayed_interrupt(MINIMUM_COMMAND_TIME, 0);
			break;

		case IDE_COMMAND_IDLE:
			/* clear the error too */
			error = IDE_ERROR_NONE;

			/* for timeout disabled value is 0 */
			sector_count = 0;
			/* signal an interrupt */
			set_irq(ASSERT_LINE);
			break;

		case IDE_COMMAND_SET_CONFIG:
			LOGPRINT(("IDE Set configuration (%d heads, %d sectors)\n", dev->cur_head + 1, sector_count));
			status &= ~IDE_STATUS_ERROR;
			error = IDE_ERROR_NONE;
			dev->set_geometry(sector_count,dev->cur_head + 1);

			/* signal an interrupt */
			signal_delayed_interrupt(MINIMUM_COMMAND_TIME, 0);
			break;

		case IDE_COMMAND_UNKNOWN_F9:
			/* only used by Killer Instinct AFAICT */
			LOGPRINT(("IDE unknown command (F9)\n"));

			/* signal an interrupt */
			set_irq(ASSERT_LINE);
			break;

		case IDE_COMMAND_SET_FEATURES:
			LOGPRINT(("IDE Set features (%02X %02X %02X %02X %02X)\n", precomp_offset, sector_count & 0xff, dev->cur_sector, dev->cur_cylinder & 0xff, dev->cur_cylinder >> 8));

			/* signal an interrupt */
			signal_delayed_interrupt(MINIMUM_COMMAND_TIME, 0);
			break;

		case IDE_COMMAND_SET_BLOCK_COUNT:
			LOGPRINT(("IDE Set block count (%02X)\n", sector_count));

			block_count = sector_count;
			// judge dredd wants 'drive ready' on this command
			status |= IDE_STATUS_DRIVE_READY;

			/* signal an interrupt */
			set_irq(ASSERT_LINE);
			break;

		case IDE_COMMAND_TAITO_GNET_UNLOCK_1:
			LOGPRINT(("IDE GNET Unlock 1\n"));

			sector_count = 1;
			status |= IDE_STATUS_DRIVE_READY;
			status &= ~IDE_STATUS_ERROR;
			set_irq(ASSERT_LINE);
			break;

		case IDE_COMMAND_TAITO_GNET_UNLOCK_2:
			LOGPRINT(("IDE GNET Unlock 2\n"));

			/* reset the buffer */
			buffer_offset = 0;
			sectors_until_int = 0;
			dma_active = 0;

			/* mark the buffer ready */
			status |= IDE_STATUS_BUFFER_READY;
			set_irq(ASSERT_LINE);
			break;

		case IDE_COMMAND_TAITO_GNET_UNLOCK_3:
			LOGPRINT(("IDE GNET Unlock 3\n"));

			/* key check */
			dev->read_key(key);
			if ((precomp_offset == key[0]) && (sector_count == key[1]) && (dev->cur_sector == key[2]) && (dev->cur_cylinder == (((UINT16)key[4]<<8)|key[3])))
			{
				gnetreadlock= 0;
			}

			/* update flags */
			status |= IDE_STATUS_DRIVE_READY;
			status &= ~IDE_STATUS_ERROR;
			set_irq(ASSERT_LINE);
			break;

		case IDE_COMMAND_SEEK:
			/*
			    cur_cylinder, cur_sector and cur_head
			    are all already set in this case so no need
			    so that implements actual seek
			*/
			/* clear the error too */
			error = IDE_ERROR_NONE;

			/* for timeout disabled value is 0 */
			sector_count = 0;
			/* signal an interrupt */
			set_irq(ASSERT_LINE);
			break;


		default:
			LOGPRINT(("IDE unknown command (%02X)\n", command));
			status |= IDE_STATUS_ERROR;
			error = IDE_ERROR_UNKNOWN_COMMAND;
			set_irq(ASSERT_LINE);
			//debugger_break(device->machine());
			break;
	}
}



/*************************************
 *
 *  IDE controller read
 *
 *************************************/

READ8_MEMBER( ide_controller_device::read_via_config )
{
	UINT16 result = 0;

	/* logit */
	LOG(("%s:IDE via config read at %X, mem_mask=%d\n", machine().describe_context(), offset, mem_mask));

	switch(offset)
	{
		/* unknown config register */
		case IDE_BANK2_CONFIG_UNK:
			result = config_unknown;
			break;

		/* active config register */
		case IDE_BANK2_CONFIG_REGISTER:
			result = config_register_num;
			break;

		/* data from active config register */
		case IDE_BANK2_CONFIG_DATA:
			if (config_register_num < IDE_CONFIG_REGISTERS)
				result = config_register[config_register_num];
			break;

		default:
			logerror("%s:unknown IDE via config read at %03X, mem_mask=%d\n", machine().describe_context(), offset, mem_mask);
			break;
	}

//	printf( "read via config %04x %04x %04x\n", offset, result, mem_mask );
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

UINT16 ide_controller_device::read_dma()
{
	UINT16 result = buffer[buffer_offset++];
	result |= buffer[buffer_offset++] << 8;

	if (buffer_offset >= IDE_DISK_SECTOR_SIZE)
	{
		LOG(("%s:IDE completed DMA read\n", machine().describe_context()));
		read_buffer_empty();
	}

	return result;
}

READ16_MEMBER( ide_controller_device::read_cs0 )
{
	UINT16 result = 0;
	ide_device_interface *dev = slot[cur_drive]->dev();

	/* logit */
//  if (offset != IDE_BANK0_DATA && offset != IDE_BANK0_STATUS_COMMAND)
		LOG(("%s:IDE cs0 read at %X, mem_mask=%d\n", machine().describe_context(), offset, mem_mask));

	if (dev != NULL)
	{
		if (dev->is_ready()) {
			status |= IDE_STATUS_DRIVE_READY;
		} else {
			status &= ~IDE_STATUS_DRIVE_READY;
		}
	}
	else
	{
		/* even a do-nothing operation should take a little time */

		status ^= IDE_STATUS_BUSY;
		return status;
	}

	switch (offset)
	{
		/* read data if there's data to be read */
		case IDE_BANK0_DATA:
			if (status & IDE_STATUS_BUFFER_READY)
			{
				/* fetch the correct amount of data */
				result = buffer[buffer_offset++];
				if (mem_mask == 0xffff)
					result |= buffer[buffer_offset++] << 8;

				/* if we're at the end of the buffer, handle it */
				if (buffer_offset >= IDE_DISK_SECTOR_SIZE)
				{
					LOG(("%s:IDE completed PIO read\n", machine().describe_context()));
					read_buffer_empty();
				}
			}
			break;

		/* return the current error */
		case IDE_BANK0_ERROR:
			result = error;
			break;

		/* return the current sector count */
		case IDE_BANK0_SECTOR_COUNT:
			result = sector_count;
			break;

		/* return the current sector */
		case IDE_BANK0_SECTOR_NUMBER:
			result = dev->cur_sector;
			break;

		/* return the current cylinder LSB */
		case IDE_BANK0_CYLINDER_LSB:
			result = dev->cur_cylinder & 0xff;
			break;

		/* return the current cylinder MSB */
		case IDE_BANK0_CYLINDER_MSB:
			result = dev->cur_cylinder >> 8;
			break;

		/* return the current head */
		case IDE_BANK0_HEAD_NUMBER:
			result = dev->cur_head_reg;
			break;

		/* return the current status and clear any pending interrupts */
		case IDE_BANK0_STATUS_COMMAND:
			result = status;
			if (last_status_timer->elapsed() > TIME_PER_ROTATION)
			{
				result |= IDE_STATUS_HIT_INDEX;
				last_status_timer->adjust(attotime::never);
			}
			if (interrupt_pending == ASSERT_LINE)
				set_irq(CLEAR_LINE);
			break;

		/* log anything else */
		default:
			logerror("%s:unknown IDE cs0 read at %03X, mem_mask=%d\n", machine().describe_context(), offset, mem_mask);
			break;
	}

//	printf( "read cs0 %04x %04x %04x\n", offset, result, mem_mask );

	/* return the result */
	return result;
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

READ16_MEMBER( ide_controller_device::read_cs1 )
{
	UINT16 result = 0;
	ide_device_interface *dev = slot[cur_drive]->dev();

	if (dev != NULL)
	{
		if (dev->is_ready()) {
			status |= IDE_STATUS_DRIVE_READY;
		} else {
			status &= ~IDE_STATUS_DRIVE_READY;
		}
	}
	else
	{
		/* even a do-nothing operation should take a little time */

		status ^= IDE_STATUS_BUSY;
		return status;
	}

	/* logit */
//  if (offset != IDE_BANK1_STATUS_CONTROL)
		LOG(("%s:IDE cs1 read at %X, mem_mask=%d\n", machine().describe_context(), offset, mem_mask));
		/* return the current status but don't clear interrupts */

	switch (offset)
	{
		case IDE_BANK1_STATUS_CONTROL:
			result = status;
			if (last_status_timer->elapsed() > TIME_PER_ROTATION)
			{
				result |= IDE_STATUS_HIT_INDEX;
				last_status_timer->adjust(attotime::never);
			}
			break;

		/* log anything else */
		default:
			logerror("%s:unknown IDE cs1 read at %03X, mem_mask=%d\n", machine().describe_context(), offset, mem_mask);
			break;
	}

//	printf( "read cs1 %04x %04x %04x\n", offset, result, mem_mask );

	/* return the result */
	return result;
}


/*************************************
 *
 *  IDE controller write
 *
 *************************************/

WRITE8_MEMBER( ide_controller_device::write_via_config )
{
//	printf( "write via config %04x %04x %04x\n", offset, data, mem_mask );

	/* logit */
	LOG(("%s:IDE via config write to %X = %08X, mem_mask=%d\n", machine().describe_context(), offset, data, mem_mask));

	switch (offset)
	{
		/* unknown config register */
		case IDE_BANK2_CONFIG_UNK:
			config_unknown = data;
			break;

		/* active config register */
		case IDE_BANK2_CONFIG_REGISTER:
			config_register_num = data;
			break;

		/* data from active config register */
		case IDE_BANK2_CONFIG_DATA:
			if (config_register_num < IDE_CONFIG_REGISTERS)
				config_register[config_register_num] = data;
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

void ide_controller_device::write_dma( UINT16 data )
{
	buffer[buffer_offset++] = data;
	buffer[buffer_offset++] = data >> 8;

	/* if we're at the end of the buffer, handle it */
	if (buffer_offset >= IDE_DISK_SECTOR_SIZE)
	{
		LOG(("%s:IDE completed DMA write\n", machine().describe_context()));
		write_buffer_full();
	}
}

WRITE16_MEMBER( ide_controller_device::write_cs0 )
{
//	printf( "write cs0 %04x %04x %04x\n", offset, data, mem_mask );

	switch (offset)
	{
		case IDE_BANK0_HEAD_NUMBER:
			cur_drive = (data & 0x10) >> 4;
			break;
	}

	ide_device_interface *dev = slot[cur_drive]->dev();
	if (dev == NULL)
		return;

	/* logit */
	if (offset != IDE_BANK0_DATA)
		LOG(("%s:IDE cs0 write to %X = %08X, mem_mask=%d\n", machine().describe_context(), offset, data, mem_mask));
	//  fprintf(stderr, "ide write %03x %02x mem_mask=%d\n", offset, data, size);
	switch (offset)
	{
		/* write data */
		case IDE_BANK0_DATA:
			if (status & IDE_STATUS_BUFFER_READY)
			{
				/* store the correct amount of data */
				buffer[buffer_offset++] = data;
				if (mem_mask == 0xffff)
					buffer[buffer_offset++] = data >> 8;

				/* if we're at the end of the buffer, handle it */
				if (buffer_offset >= IDE_DISK_SECTOR_SIZE)
				{
					LOG(("%s:IDE completed PIO write\n", machine().describe_context()));
					write_buffer_full();
				}
			}
			break;

		/* precompensation offset?? */
		case IDE_BANK0_ERROR:
			precomp_offset = data;
			break;

		/* sector count */
		case IDE_BANK0_SECTOR_COUNT:
			sector_count = data ? data : 256;
			break;

		/* current sector */
		case IDE_BANK0_SECTOR_NUMBER:
			dev->cur_sector = data;
			break;

		/* current cylinder LSB */
		case IDE_BANK0_CYLINDER_LSB:
			dev->cur_cylinder = (dev->cur_cylinder & 0xff00) | (data & 0xff);
			break;

		/* current cylinder MSB */
		case IDE_BANK0_CYLINDER_MSB:
			dev->cur_cylinder = (dev->cur_cylinder & 0x00ff) | ((data & 0xff) << 8);
			break;

		/* current head */
		case IDE_BANK0_HEAD_NUMBER:
			dev->cur_head = data & 0x0f;
			dev->cur_head_reg = data;
			// LBA mode = data & 0x40
			break;

		/* command */
		case IDE_BANK0_STATUS_COMMAND:
			handle_command(data);
			break;
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

WRITE16_MEMBER( ide_controller_device::write_cs1 )
{
//	printf( "write cs1 %04x %04x %04x\n", offset, data, mem_mask );

	/* logit */
	LOG(("%s:IDE cs1 write to %X = %08X, mem_mask=%d\n", machine().describe_context(), offset, data, mem_mask));

	switch (offset)
	{
		/* adapter control */
		case IDE_BANK1_STATUS_CONTROL:
			adapter_control = data;

			/* handle controller reset */
			//if (data == 0x04)
			if (data & 0x04)
			{
				status |= IDE_STATUS_BUSY;
				status &= ~IDE_STATUS_DRIVE_READY;
				reset_timer->adjust(attotime::from_msec(5));
			}
			break;
	}
}


SLOT_INTERFACE_START(ide_devices)
	SLOT_INTERFACE("hdd", IDE_HARDDISK)
SLOT_INTERFACE_END

ide_controller_device::ide_controller_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, type, name, tag, owner, clock),
	status(0),
	adapter_control(0),
	error(0),
	command(0),
	interrupt_pending(0),
	precomp_offset(0),
	buffer_offset(0),
	sector_count(0),
	block_count(0),
	sectors_until_int(0),
	verify_only(0),
	config_unknown(0),
	config_register_num(0),
	master_password_enable(0),
	user_password_enable(0),
	master_password(NULL),
	user_password(NULL),
	gnetreadlock(0),
	cur_drive(0),
	m_irq_handler(*this)
{
}


const device_type IDE_CONTROLLER = &device_creator<ide_controller_device>;

ide_controller_device::ide_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, IDE_CONTROLLER, "IDE Controller", tag, owner, clock),
	adapter_control(0),
	error(0),
	command(0),
	interrupt_pending(0),
	precomp_offset(0),
	buffer_offset(0),
	sector_count(0),
	block_count(0),
	sectors_until_int(0),
	verify_only(0),
	config_unknown(0),
	config_register_num(0),
	master_password_enable(0),
	user_password_enable(0),
	master_password(NULL),
	user_password(NULL),
	gnetreadlock(0),
	cur_drive(0),
	m_irq_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ide_controller_device::device_start()
{
	m_irq_handler.resolve_safe();

	/* set MAME harddisk handle */
	slot[0] = owner()->subdevice<ide_slot_device>("drive_0");
	slot[1] = owner()->subdevice<ide_slot_device>("drive_1");

	/* create a timer for timing status */
	last_status_timer = timer_alloc(TID_NULL);
	reset_timer = timer_alloc(TID_RESET_CALLBACK);

	/* register ide states */
	save_item(NAME(adapter_control));
	save_item(NAME(status));
	save_item(NAME(error));
	save_item(NAME(command));
	save_item(NAME(interrupt_pending));
	save_item(NAME(precomp_offset));

	save_item(NAME(buffer));
	save_item(NAME(buffer_offset));
	save_item(NAME(sector_count));

	save_item(NAME(block_count));
	save_item(NAME(sectors_until_int));

	save_item(NAME(dma_active));

	save_item(NAME(config_unknown));
	save_item(NAME(config_register));
	save_item(NAME(config_register_num));

	save_item(NAME(master_password_enable));
	save_item(NAME(user_password_enable));

	save_item(NAME(gnetreadlock));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ide_controller_device::device_reset()
{
	LOG(("IDE controller reset performed\n"));
	/* reset the drive state */
	cur_drive = 0;
	status = IDE_STATUS_DRIVE_READY | IDE_STATUS_SEEK_COMPLETE;
	error = IDE_ERROR_DEFAULT;
	buffer_offset = 0;
	gnetreadlock = 0;
	master_password_enable = (master_password != NULL);
	user_password_enable = (user_password != NULL);
	set_irq(CLEAR_LINE);
	set_dmarq(0);
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
