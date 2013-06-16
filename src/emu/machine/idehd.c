#include "idehd.h"

/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE                     0
#define PRINTF_IDE_COMMANDS         0
#define PRINTF_IDE_PASSWORD         0

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

#define LOGPRINT(x) do { if (VERBOSE) logerror x; if (PRINTF_IDE_COMMANDS) mame_printf_debug x; } while (0)

#define MINIMUM_COMMAND_TIME                (attotime::from_usec(10))

#define TIME_PER_SECTOR                     (attotime::from_usec(100))
#define TIME_PER_ROTATION                   (attotime::from_hz(5400/60))

#define TIME_SEEK_MULTISECTOR               (attotime::from_msec(13))
#define TIME_NO_SEEK_MULTISECTOR            (attotime::from_nsec(16300))

#define IDE_CS0_DATA_RW              0
#define IDE_CS0_ERROR_R              1
#define IDE_CS0_FEATURE_W            1
#define IDE_CS0_SECTOR_COUNT_RW      2
#define IDE_CS0_SECTOR_NUMBER_RW     3
#define IDE_CS0_CYLINDER_LOW_RW      4
#define IDE_CS0_CYLINDER_HIGH_RW     5
#define IDE_CS0_DEVICE_HEAD_RW       6
#define IDE_CS0_STATUS_R             7
#define IDE_CS0_COMMAND_W            7

#define IDE_CS1_ALTERNATE_STATUS_R   6
#define IDE_CS1_DEVICE_CONTROL_W     6

#define IDE_COMMAND_READ_SECTORS            0x20
#define IDE_COMMAND_READ_SECTORS_NORETRY    0x21
#define IDE_COMMAND_WRITE_SECTORS           0x30
#define IDE_COMMAND_WRITE_SECTORS_NORETRY   0x31
#define IDE_COMMAND_DIAGNOSTIC              0x90
#define IDE_COMMAND_SET_CONFIG              0x91
#define IDE_COMMAND_READ_MULTIPLE           0xc4
#define IDE_COMMAND_WRITE_MULTIPLE          0xc5
#define IDE_COMMAND_SET_BLOCK_COUNT         0xc6
#define IDE_COMMAND_READ_DMA                0xc8
#define IDE_COMMAND_WRITE_DMA               0xca
#define IDE_COMMAND_IDENTIFY_DEVICE         0xec
#define IDE_COMMAND_SET_FEATURES            0xef
#define IDE_COMMAND_SECURITY_UNLOCK         0xf2
#define IDE_COMMAND_UNKNOWN_F9              0xf9
#define IDE_COMMAND_VERIFY_SECTORS          0x40
#define IDE_COMMAND_VERIFY_SECTORS_NORETRY  0x41
#define IDE_COMMAND_ATAPI_IDENTIFY          0xa1
#define IDE_COMMAND_RECALIBRATE             0x10
#define IDE_COMMAND_SEEK                    0x70
#define IDE_COMMAND_IDLE_IMMEDIATE          0xe1
#define IDE_COMMAND_IDLE                    0xe3

#define IDE_DEVICE_CONTROL_NIEN 0x02
#define IDE_DEVICE_CONTROL_SRST 0x04

enum
{
	TID_NULL,
	TID_DELAYED_INTERRUPT,
	TID_DELAYED_INTERRUPT_BUFFER_READY,
	TID_RESET_CALLBACK,
	TID_READ_SECTOR_DONE_CALLBACK,
	TID_WRITE_SECTOR_DONE_CALLBACK
};

//**************************************************************************
//  IDE DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  ide_device_interface - constructor
//-------------------------------------------------

ide_device_interface::ide_device_interface(const machine_config &mconfig, device_t &device) :
	m_master_password(NULL),
	m_user_password(NULL),
	m_irq_handler(device),
	m_dmarq_handler(device)
{
}

ide_mass_storage_device::ide_mass_storage_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock,const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	ide_device_interface(mconfig, *this),
	device_slot_card_interface(mconfig, *this),
	m_csel(0),
	m_dasp(0),
	m_dmack(0),
	m_dmarq(0),
	m_irq(0)
{
}

void ide_mass_storage_device::update_irq()
{
	if (device_selected() && (m_device_control & IDE_DEVICE_CONTROL_NIEN) == 0)
		m_irq_handler(m_irq);
	else
		m_irq_handler(CLEAR_LINE);
}

void ide_mass_storage_device::set_irq(int state)
{
	if (m_irq != state)
	{
		m_irq = state;

		if (state == ASSERT_LINE)
			LOG(("IDE interrupt assert\n"));
		else
			LOG(("IDE interrupt clear\n"));

		update_irq();
	}
}

void ide_mass_storage_device::set_dmarq(int state)
{
	if (m_dmarq != state)
	{
		m_dmarq = state;

		m_dmarq_handler(state);
	}
}

WRITE_LINE_MEMBER( ide_mass_storage_device::write_csel )
{
	m_csel = state;
}

WRITE_LINE_MEMBER( ide_mass_storage_device::write_dasp )
{
	m_dasp = state;
}

WRITE_LINE_MEMBER( ide_mass_storage_device::write_dmack )
{
	m_dmack = state;
}

/*************************************
 *
 *  Compute the LBA address
 *
 *************************************/

UINT32 ide_mass_storage_device::lba_address()
{
	/* LBA direct? */
	if (m_device_head & IDE_DEVICE_HEAD_L)
		return ((m_device_head & IDE_DEVICE_HEAD_HS) << 24) | (m_cylinder_high << 16) | (m_cylinder_low << 8) | m_sector_number;

	/* standard CHS */
	else
		return (((((m_cylinder_high << 8 ) | m_cylinder_low) * m_num_heads) + (m_device_head & IDE_DEVICE_HEAD_HS)) * m_num_sectors) + m_sector_number - 1;
}


/*************************************
 *
 *  Build a features page
 *
 *************************************/

static void swap_strncpy(UINT8 *dst, const char *src, int field_size_in_words)
{
	int i;

	assert(strlen(src) <= (field_size_in_words*2));

	for (i = 0; i < strlen(src); i++)
		dst[i ^ 1] = src[i];
	for ( ; i < field_size_in_words * 2; i++)
		dst[i ^ 1] = ' ';
}


void ide_mass_storage_device::ide_build_identify_device()
{
	memset(m_identify_device, 0, IDE_DISK_SECTOR_SIZE);
	int total_sectors = m_num_cylinders * m_num_heads * m_num_sectors;

	/* basic geometry */
	m_identify_device[ 0*2+0] = 0x5a;                      /*  0: configuration bits */
	m_identify_device[ 0*2+1] = 0x04;
	m_identify_device[ 1*2+0] = m_num_cylinders & 0xff;    /*  1: logical cylinders */
	m_identify_device[ 1*2+1] = m_num_cylinders >> 8;
	m_identify_device[ 2*2+0] = 0;                         /*  2: reserved */
	m_identify_device[ 2*2+1] = 0;
	m_identify_device[ 3*2+0] = m_num_heads & 0xff;        /*  3: logical heads */
	m_identify_device[ 3*2+1] = 0;/*num_heads >> 8;*/
	m_identify_device[ 4*2+0] = 0;                         /*  4: vendor specific (obsolete) */
	m_identify_device[ 4*2+1] = 0;
	m_identify_device[ 5*2+0] = 0;                         /*  5: vendor specific (obsolete) */
	m_identify_device[ 5*2+1] = 0;
	m_identify_device[ 6*2+0] = m_num_sectors & 0xff;  /*  6: logical sectors per logical track */
	m_identify_device[ 6*2+1] = 0;/*num_sectors >> 8;*/
	m_identify_device[ 7*2+0] = 0;                         /*  7: vendor-specific */
	m_identify_device[ 7*2+1] = 0;
	m_identify_device[ 8*2+0] = 0;                         /*  8: vendor-specific */
	m_identify_device[ 8*2+1] = 0;
	m_identify_device[ 9*2+0] = 0;                         /*  9: vendor-specific */
	m_identify_device[ 9*2+1] = 0;
	swap_strncpy(&m_identify_device[10*2+0],               /* 10-19: serial number */
			"00000000000000000000", 10);
	m_identify_device[20*2+0] = 0;                         /* 20: vendor-specific */
	m_identify_device[20*2+1] = 0;
	m_identify_device[21*2+0] = 0;                         /* 21: vendor-specific */
	m_identify_device[21*2+1] = 0;
	m_identify_device[22*2+0] = 4;                         /* 22: # of vendor-specific bytes on read/write long commands */
	m_identify_device[22*2+1] = 0;
	swap_strncpy(&m_identify_device[23*2+0],               /* 23-26: firmware revision */
			"1.0", 4);
	swap_strncpy(&m_identify_device[27*2+0],               /* 27-46: model number */
			"MAME Compressed Hard Disk", 20);
	m_identify_device[47*2+0] = 0x01;                      /* 47: read/write multiple support */
	m_identify_device[47*2+1] = 0x80;
	m_identify_device[48*2+0] = 0;                         /* 48: reserved */
	m_identify_device[48*2+1] = 0;
	m_identify_device[49*2+0] = 0x03;                      /* 49: capabilities */
	m_identify_device[49*2+1] = 0x0f;
	m_identify_device[50*2+0] = 0;                         /* 50: reserved */
	m_identify_device[50*2+1] = 0;
	m_identify_device[51*2+0] = 2;                         /* 51: PIO data transfer cycle timing mode */
	m_identify_device[51*2+1] = 0;
	m_identify_device[52*2+0] = 2;                         /* 52: single word DMA transfer cycle timing mode */
	m_identify_device[52*2+1] = 0;
	m_identify_device[53*2+0] = 3;                         /* 53: field validity */
	m_identify_device[53*2+1] = 0;
	m_identify_device[54*2+0] = m_num_cylinders & 0xff;    /* 54: number of current logical cylinders */
	m_identify_device[54*2+1] = m_num_cylinders >> 8;
	m_identify_device[55*2+0] = m_num_heads & 0xff;        /* 55: number of current logical heads */
	m_identify_device[55*2+1] = 0;/*num_heads >> 8;*/
	m_identify_device[56*2+0] = m_num_sectors & 0xff;  /* 56: number of current logical sectors per track */
	m_identify_device[56*2+1] = 0;/*num_sectors >> 8;*/
	m_identify_device[57*2+0] = total_sectors & 0xff;  /* 57-58: current capacity in sectors (ATA-1 through ATA-5; obsoleted in ATA-6) */
	m_identify_device[57*2+1] = total_sectors >> 8;
	m_identify_device[58*2+0] = total_sectors >> 16;
	m_identify_device[58*2+1] = total_sectors >> 24;
	m_identify_device[59*2+0] = 0;                         /* 59: multiple sector timing */
	m_identify_device[59*2+1] = 0;
	m_identify_device[60*2+0] = total_sectors & 0xff;      /* 60-61: total user addressable sectors for LBA mode (ATA-1 through ATA-7) */
	m_identify_device[60*2+1] = total_sectors >> 8;
	m_identify_device[61*2+0] = total_sectors >> 16;
	m_identify_device[61*2+1] = total_sectors >> 24;
	m_identify_device[62*2+0] = 0x07;                      /* 62: single word dma transfer */
	m_identify_device[62*2+1] = 0x00;
	m_identify_device[63*2+0] = 0x07;                      /* 63: multiword DMA transfer */
	m_identify_device[63*2+1] = 0x04;
	m_identify_device[64*2+0] = 0x03;                      /* 64: flow control PIO transfer modes supported */
	m_identify_device[64*2+1] = 0x00;
	m_identify_device[65*2+0] = 0x78;                      /* 65: minimum multiword DMA transfer cycle time per word */
	m_identify_device[65*2+1] = 0x00;
	m_identify_device[66*2+0] = 0x78;                      /* 66: mfr's recommended multiword DMA transfer cycle time */
	m_identify_device[66*2+1] = 0x00;
	m_identify_device[67*2+0] = 0x4d;                      /* 67: minimum PIO transfer cycle time without flow control */
	m_identify_device[67*2+1] = 0x01;
	m_identify_device[68*2+0] = 0x78;                      /* 68: minimum PIO transfer cycle time with IORDY */
	m_identify_device[68*2+1] = 0x00;
	m_identify_device[69*2+0] = 0x00;                      /* 69-70: reserved */
	m_identify_device[69*2+1] = 0x00;
	m_identify_device[71*2+0] = 0x00;                      /* 71: reserved for IDENTIFY PACKET command */
	m_identify_device[71*2+1] = 0x00;
	m_identify_device[72*2+0] = 0x00;                      /* 72: reserved for IDENTIFY PACKET command */
	m_identify_device[72*2+1] = 0x00;
	m_identify_device[73*2+0] = 0x00;                      /* 73: reserved for IDENTIFY PACKET command */
	m_identify_device[73*2+1] = 0x00;
	m_identify_device[74*2+0] = 0x00;                      /* 74: reserved for IDENTIFY PACKET command */
	m_identify_device[74*2+1] = 0x00;
	m_identify_device[75*2+0] = 0x00;                      /* 75: queue depth */
	m_identify_device[75*2+1] = 0x00;
	m_identify_device[76*2+0] = 0x00;                      /* 76-79: reserved */
	m_identify_device[76*2+1] = 0x00;
	m_identify_device[80*2+0] = 0x00;                      /* 80: major version number */
	m_identify_device[80*2+1] = 0x00;
	m_identify_device[81*2+0] = 0x00;                      /* 81: minor version number */
	m_identify_device[81*2+1] = 0x00;
	m_identify_device[82*2+0] = 0x00;                      /* 82: command set supported */
	m_identify_device[82*2+1] = 0x00;
	m_identify_device[83*2+0] = 0x00;                      /* 83: command sets supported */
	m_identify_device[83*2+1] = 0x00;
	m_identify_device[84*2+0] = 0x00;                      /* 84: command set/feature supported extension */
	m_identify_device[84*2+1] = 0x00;
	m_identify_device[85*2+0] = 0x00;                      /* 85: command set/feature enabled */
	m_identify_device[85*2+1] = 0x00;
	m_identify_device[86*2+0] = 0x00;                      /* 86: command set/feature enabled */
	m_identify_device[86*2+1] = 0x00;
	m_identify_device[87*2+0] = 0x00;                      /* 87: command set/feature default */
	m_identify_device[87*2+1] = 0x00;
	m_identify_device[88*2+0] = 0x00;                      /* 88: additional DMA modes */
	m_identify_device[88*2+1] = 0x00;
	m_identify_device[89*2+0] = 0x00;                      /* 89: time required for security erase unit completion */
	m_identify_device[89*2+1] = 0x00;
	m_identify_device[90*2+0] = 0x00;                      /* 90: time required for enhanced security erase unit completion */
	m_identify_device[90*2+1] = 0x00;
	m_identify_device[91*2+0] = 0x00;                      /* 91: current advanced power management value */
	m_identify_device[91*2+1] = 0x00;
	m_identify_device[92*2+0] = 0x00;                      /* 92: master password revision code */
	m_identify_device[92*2+1] = 0x00;
	m_identify_device[93*2+0] = 0x00;                      /* 93: hardware reset result */
	m_identify_device[93*2+1] = 0x00;
	m_identify_device[94*2+0] = 0x00;                      /* 94: acoustic management values */
	m_identify_device[94*2+1] = 0x00;
	m_identify_device[95*2+0] = 0x00;                      /* 95-99: reserved */
	m_identify_device[95*2+1] = 0x00;
	m_identify_device[100*2+0] = total_sectors & 0xff;     /* 100-103: maximum 48-bit LBA */
	m_identify_device[100*2+1] = total_sectors >> 8;
	m_identify_device[101*2+0] = total_sectors >> 16;
	m_identify_device[101*2+1] = total_sectors >> 24;
	m_identify_device[102*2+0] = 0x00;
	m_identify_device[102*2+1] = 0x00;
	m_identify_device[103*2+0] = 0x00;
	m_identify_device[103*2+1] = 0x00;
	m_identify_device[104*2+0] = 0x00;                     /* 104-126: reserved */
	m_identify_device[104*2+1] = 0x00;
	m_identify_device[127*2+0] = 0x00;                     /* 127: removable media status notification */
	m_identify_device[127*2+1] = 0x00;
	m_identify_device[128*2+0] = 0x00;                     /* 128: security status */
	m_identify_device[128*2+1] = 0x00;
	m_identify_device[129*2+0] = 0x00;                     /* 129-159: vendor specific */
	m_identify_device[129*2+1] = 0x00;
	m_identify_device[160*2+0] = 0x00;                     /* 160: CFA power mode 1 */
	m_identify_device[160*2+1] = 0x00;
	m_identify_device[161*2+0] = 0x00;                     /* 161-175: reserved for CompactFlash */
	m_identify_device[161*2+1] = 0x00;
	m_identify_device[176*2+0] = 0x00;                     /* 176-205: current media serial number */
	m_identify_device[176*2+1] = 0x00;
	m_identify_device[206*2+0] = 0x00;                     /* 206-254: reserved */
	m_identify_device[206*2+1] = 0x00;
	m_identify_device[255*2+0] = 0x00;                     /* 255: integrity word */
	m_identify_device[255*2+1] = 0x00;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ide_mass_storage_device::device_start()
{
	m_irq_handler.resolve_safe();
	m_dmarq_handler.resolve_safe();

	save_item(NAME(m_buffer));
	save_item(NAME(m_buffer_offset));
	save_item(NAME(m_error));
	save_item(NAME(m_feature));
	save_item(NAME(m_sector_count));
	save_item(NAME(m_sector_number));
	save_item(NAME(m_cylinder_low));
	save_item(NAME(m_cylinder_high));
	save_item(NAME(m_device_head));
	save_item(NAME(m_status));
	save_item(NAME(m_command));
	save_item(NAME(m_device_control));

	save_item(NAME(m_can_identify_device));
	save_item(NAME(m_identify_device));
	save_item(NAME(m_cur_lba));
	save_item(NAME(m_irq));
	save_item(NAME(m_dmarq));
	save_item(NAME(m_sectors_until_int));
	save_item(NAME(m_master_password_enable));
	save_item(NAME(m_user_password_enable));
	save_item(NAME(m_block_count));

	/* create a timer for timing status */
	m_last_status_timer = timer_alloc(TID_NULL);
	m_reset_timer = timer_alloc(TID_RESET_CALLBACK);
}

void ide_mass_storage_device::device_reset()
{
	m_buffer_offset = 0;
	m_master_password_enable = (m_master_password != NULL);
	m_user_password_enable = (m_user_password != NULL);

	m_status = IDE_STATUS_DSC;

	if (is_ready())
	{
		m_status |= IDE_STATUS_DRDY;
	}

	if (m_can_identify_device)
		m_error = IDE_ERROR_DIAGNOSTIC_PASSED;
	else
		m_error = IDE_ERROR_DIAGNOSTIC_FAILED;

	m_sector_count = 1;
	m_sector_number = 1;
	m_cylinder_low = 0;
	m_cylinder_high = 0;
	m_device_head = 0;

	/* reset the drive state */
	set_irq(CLEAR_LINE);
	set_dmarq(CLEAR_LINE);
}

void ide_mass_storage_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TID_DELAYED_INTERRUPT:
		m_status &= ~IDE_STATUS_BSY;

		set_irq(ASSERT_LINE);
		break;

	case TID_DELAYED_INTERRUPT_BUFFER_READY:
		m_status &= ~IDE_STATUS_BSY;
		m_status |= IDE_STATUS_DRQ;

		set_irq(ASSERT_LINE);
		break;

	case TID_RESET_CALLBACK:
		reset();
		break;

	case TID_READ_SECTOR_DONE_CALLBACK:
		read_sector_done();
		break;

	case TID_WRITE_SECTOR_DONE_CALLBACK:
		write_sector_done();
		break;
	}
}

void ide_mass_storage_device::signal_delayed_interrupt(attotime time, int buffer_ready)
{
	/* clear buffer ready and set the busy flag */
	m_status &= ~IDE_STATUS_DRQ;
	m_status |= IDE_STATUS_BSY;

	/* set a timer */
	if (buffer_ready)
		timer_set(time, TID_DELAYED_INTERRUPT_BUFFER_READY);
	else
		timer_set(time, TID_DELAYED_INTERRUPT);
}

/*************************************
 *
 *  Advance to the next sector
 *
 *************************************/

void ide_mass_storage_device::next_sector()
{
	/* LBA direct? */
	if (m_device_head & IDE_DEVICE_HEAD_L)
	{
		m_sector_number++;
		if (m_sector_number == 0)
		{
			m_cylinder_low++;
			if (m_cylinder_low == 0)
			{
				m_cylinder_high++;
				if( m_cylinder_high == 0)
					m_device_head = (m_device_head & ~IDE_DEVICE_HEAD_HS) | ((m_device_head + 1) & IDE_DEVICE_HEAD_HS);
			}
		}
	}

	/* standard CHS */
	else
	{
		/* sectors are 1-based */
		m_sector_number++;
		if (m_sector_number > m_num_sectors)
		{
			/* heads are 0 based */
			m_sector_number = 1;
			m_device_head = (m_device_head & ~IDE_DEVICE_HEAD_HS) | ((m_device_head + 1) & IDE_DEVICE_HEAD_HS);
			if ((m_device_head & IDE_DEVICE_HEAD_HS) >= m_num_heads)
			{
				m_device_head &= ~IDE_DEVICE_HEAD_HS;
				m_cylinder_low++;
				if (m_cylinder_low == 0)
					m_cylinder_high++;
			}
		}
	}

	m_cur_lba = lba_address();
}



/*************************************
 *
 *  security error handling
 *
 *************************************/

void ide_mass_storage_device::security_error()
{
	/* set error state */
	m_status |= IDE_STATUS_ERR;
	m_error = IDE_ERROR_NONE;
	m_status &= ~IDE_STATUS_DRDY;
}



/*************************************
 *
 *  Sector reading
 *
 *************************************/

void ide_mass_storage_device::read_buffer_empty()
{
	/* reset the totals */
	m_buffer_offset = 0;

	m_status &= ~IDE_STATUS_DRQ;
	set_dmarq(CLEAR_LINE);

	if (m_master_password_enable || m_user_password_enable)
	{
		security_error();
		return;
	}

	/* if there is more data to read, keep going */
	if (m_sector_count > 0)
		m_sector_count--;

	if (m_sector_count > 0)
		read_next_sector();
}


void ide_mass_storage_device::read_sector_done()
{
	int lba = lba_address(), count = 0;

	m_status &= ~IDE_STATUS_BSY;

	/* now do the read */
	count = read_sector(lba, m_buffer);

	/* if we succeeded, advance to the next sector and set the nice bits */
	if (count == 1)
	{
		/* advance the pointers, unless this is the last sector */
		/* Gauntlet: Dark Legacy checks to make sure we stop on the last sector */
		if (m_sector_count != 1)
			next_sector();

		/* signal an interrupt */
		if (--m_sectors_until_int == 0 || m_sector_count == 1)
		{
			m_sectors_until_int = ((m_command == IDE_COMMAND_READ_MULTIPLE) ? m_block_count : 1);
			set_irq(ASSERT_LINE);
		}

		/* if we're just verifying we can read the next sector */
		if (m_command == IDE_COMMAND_VERIFY_SECTORS ||
			m_command == IDE_COMMAND_VERIFY_SECTORS_NORETRY )
		{
			read_buffer_empty();
		}
		else
		{
			m_status |= IDE_STATUS_DRQ;

			if (m_command == IDE_COMMAND_READ_DMA)
				set_dmarq(ASSERT_LINE);
		}
	}

	/* if we got an error, we need to report it */
	else
	{
		/* set the error flag and the error */
		m_status |= IDE_STATUS_ERR;
		m_error = IDE_ERROR_BAD_SECTOR;

		/* signal an interrupt */
		set_irq(ASSERT_LINE);
	}
}


void ide_mass_storage_device::read_first_sector()
{
	/* mark ourselves busy */
	m_status |= IDE_STATUS_BSY;

	/* just set a timer */
	if (m_command == IDE_COMMAND_READ_MULTIPLE)
	{
		int new_lba = lba_address();
		attotime seek_time;

		if (new_lba == m_cur_lba || new_lba == m_cur_lba + 1)
			seek_time = TIME_NO_SEEK_MULTISECTOR;
		else
			seek_time = TIME_SEEK_MULTISECTOR;

		m_cur_lba = new_lba;
		timer_set(seek_time, TID_READ_SECTOR_DONE_CALLBACK);
	}
	else
		timer_set(TIME_PER_SECTOR, TID_READ_SECTOR_DONE_CALLBACK);
}


void ide_mass_storage_device::read_next_sector()
{
	/* mark ourselves busy */
	m_status |= IDE_STATUS_BSY;

	if (m_command == IDE_COMMAND_READ_MULTIPLE)
	{
		if (m_sectors_until_int != 1)
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

void ide_mass_storage_device::continue_write()
{
	/* reset the totals */
	m_buffer_offset = 0;

	/* clear the buffer ready flag */
	m_status |= IDE_STATUS_BSY;

	if (m_command == IDE_COMMAND_WRITE_MULTIPLE)
	{
		if (m_sectors_until_int != 1)
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


void ide_mass_storage_device::write_buffer_full()
{
	m_status &= ~IDE_STATUS_DRQ;
	set_dmarq(CLEAR_LINE);

	process_buffer();
}

void ide_mass_storage_device::process_buffer()
{
	if (m_command == IDE_COMMAND_SECURITY_UNLOCK)
	{
		if (m_user_password_enable && memcmp(m_buffer, m_user_password, 2 + 32) == 0)
		{
			LOGPRINT(("IDE Unlocked user password\n"));
			m_user_password_enable = 0;
		}
		if (m_master_password_enable && memcmp(m_buffer, m_master_password, 2 + 32) == 0)
		{
			LOGPRINT(("IDE Unlocked master password\n"));
			m_master_password_enable = 0;
		}
		if (PRINTF_IDE_PASSWORD)
		{
			int i;

			for (i = 0; i < 34; i += 2)
			{
				if (i % 8 == 2)
					mame_printf_debug("\n");

				mame_printf_debug("0x%02x, 0x%02x, ", m_buffer[i], m_buffer[i + 1]);
				//mame_printf_debug("0x%02x%02x, ", m_buffer[i], m_buffer[i + 1]);
			}
			mame_printf_debug("\n");
		}

		if (m_master_password_enable || m_user_password_enable)
			security_error();
	}
	else
	{
		continue_write();
	}
}


void ide_mass_storage_device::write_sector_done()
{
	int lba = lba_address(), count = 0;

	m_status &= ~IDE_STATUS_BSY;

	/* now do the write */
	count = write_sector(lba, m_buffer);

	/* if we succeeded, advance to the next sector and set the nice bits */
	if (count == 1)
	{
		/* advance the pointers, unless this is the last sector */
		/* Gauntlet: Dark Legacy checks to make sure we stop on the last sector */
		if (m_sector_count != 1)
			next_sector();

		/* signal an interrupt */
		if (--m_sectors_until_int == 0 || m_sector_count == 1)
		{
			m_sectors_until_int = ((m_command == IDE_COMMAND_WRITE_MULTIPLE) ? m_block_count : 1);
			set_irq(ASSERT_LINE);
		}

		/* signal an interrupt if there's more data needed */
		if (m_sector_count > 0)
			m_sector_count--;

		if (m_sector_count > 0)
		{
			m_status |= IDE_STATUS_DRQ;

			if (m_command == IDE_COMMAND_WRITE_DMA)
				set_dmarq(ASSERT_LINE);
		}
	}

	/* if we got an error, we need to report it */
	else
	{
		/* set the error flag and the error */
		m_status |= IDE_STATUS_ERR;
		m_error = IDE_ERROR_BAD_SECTOR;

		/* signal an interrupt */
		set_irq(ASSERT_LINE);
	}
}


/*************************************
 *
 *  Handle IDE commands
 *
 *************************************/

bool ide_mass_storage_device::process_command()
{
	switch (m_command)
	{
	case IDE_COMMAND_READ_SECTORS:
	case IDE_COMMAND_READ_SECTORS_NORETRY:
		LOGPRINT(("IDE Read multiple: C=%d H=%d S=%d LBA=%d count=%d\n",
			(m_cylinder_high << 8) | m_cylinder_low, m_device_head & IDE_DEVICE_HEAD_HS, m_sector_number, lba_address(), m_sector_count));

		m_sectors_until_int = 1;

		/* start the read going */
		read_first_sector();
		return true;

	case IDE_COMMAND_READ_MULTIPLE:
		LOGPRINT(("IDE Read multiple block: C=%d H=%d S=%d LBA=%d count=%d\n",
			(m_cylinder_high << 8) | m_cylinder_low, m_device_head & IDE_DEVICE_HEAD_HS, m_sector_number, lba_address(), m_sector_count));

		m_sectors_until_int = 1;

		/* start the read going */
		read_first_sector();
		return true;

	case IDE_COMMAND_VERIFY_SECTORS:
	case IDE_COMMAND_VERIFY_SECTORS_NORETRY:
		LOGPRINT(("IDE Read verify multiple with/without retries: C=%d H=%d S=%d LBA=%d count=%d\n",
			(m_cylinder_high << 8) | m_cylinder_low, m_device_head & IDE_DEVICE_HEAD_HS, m_sector_number, lba_address(), m_sector_count));

		/* reset the buffer */
		m_sectors_until_int = m_sector_count;

		/* start the read going */
		read_first_sector();
		return true;

	case IDE_COMMAND_READ_DMA:
		LOGPRINT(("IDE Read multiple DMA: C=%d H=%d S=%d LBA=%d count=%d\n",
			(m_cylinder_high << 8) | m_cylinder_low, m_device_head & IDE_DEVICE_HEAD_HS, m_sector_number, lba_address(), m_sector_count));

		/* reset the buffer */
		m_sectors_until_int = m_sector_count;

		/* start the read going */
		read_first_sector();
		return true;

	case IDE_COMMAND_WRITE_SECTORS:
	case IDE_COMMAND_WRITE_SECTORS_NORETRY:
		LOGPRINT(("IDE Write multiple: C=%d H=%d S=%d LBA=%d count=%d\n",
			(m_cylinder_high << 8) | m_cylinder_low, m_device_head & IDE_DEVICE_HEAD_HS, m_sector_number, lba_address(), m_sector_count));

		/* reset the buffer */
		m_sectors_until_int = 1;

		/* mark the buffer ready */
		m_status |= IDE_STATUS_DRQ;
		return true;

	case IDE_COMMAND_WRITE_MULTIPLE:
		LOGPRINT(("IDE Write multiple block: C=%d H=%d S=%d LBA=%d count=%d\n",
			(m_cylinder_high << 8) | m_cylinder_low, m_device_head & IDE_DEVICE_HEAD_HS, m_sector_number, lba_address(), m_sector_count));

		/* reset the buffer */
		m_sectors_until_int = 1;

		/* mark the buffer ready */
		m_status |= IDE_STATUS_DRQ;
		return true;

	case IDE_COMMAND_WRITE_DMA:
		LOGPRINT(("IDE Write multiple DMA: C=%d H=%d S=%d LBA=%d count=%d\n",
			(m_cylinder_high << 8) | m_cylinder_low, m_device_head & IDE_DEVICE_HEAD_HS, m_sector_number, lba_address(), m_sector_count));

		/* reset the buffer */
		m_sectors_until_int = m_sector_count;

		/* mark the buffer ready */
		m_status |= IDE_STATUS_DRQ;

		/* start the read going */
		set_dmarq(ASSERT_LINE);
		return true;

	case IDE_COMMAND_SECURITY_UNLOCK:
		LOGPRINT(("IDE Security Unlock\n"));

		/* mark the buffer ready */
		m_status |= IDE_STATUS_DRQ;

		set_irq(ASSERT_LINE);
		return true;

	case IDE_COMMAND_IDENTIFY_DEVICE:
		LOGPRINT(("IDE Identify device\n"));

		if (m_can_identify_device)
		{
			memcpy(m_buffer, m_identify_device, sizeof(m_buffer));

			/* signal an interrupt */
			signal_delayed_interrupt(MINIMUM_COMMAND_TIME, 1);
		}
		else
		{
			m_status |= IDE_STATUS_ERR;
			m_error = IDE_ERROR_NONE;
			m_status &= ~IDE_STATUS_DRDY;
			signal_delayed_interrupt(MINIMUM_COMMAND_TIME, 0);
		}
		return true;

	case IDE_COMMAND_DIAGNOSTIC:
		if (m_can_identify_device)
			m_error = IDE_ERROR_DIAGNOSTIC_PASSED;
		else
			m_error = IDE_ERROR_DIAGNOSTIC_FAILED;

		/* signal an interrupt */
		signal_delayed_interrupt(MINIMUM_COMMAND_TIME, 0);
		return true;

	case IDE_COMMAND_RECALIBRATE:
		/* signal an interrupt */
		signal_delayed_interrupt(MINIMUM_COMMAND_TIME, 0);
		return true;

	case IDE_COMMAND_IDLE:
		/* signal an interrupt */
		set_irq(ASSERT_LINE);
		return true;

	case IDE_COMMAND_SET_CONFIG:
		LOGPRINT(("IDE Set configuration (%d heads, %d sectors)\n", (m_device_head & IDE_DEVICE_HEAD_HS) + 1, m_sector_count));
		set_geometry(m_sector_count,(m_device_head & IDE_DEVICE_HEAD_HS) + 1);

		/* signal an interrupt */
		signal_delayed_interrupt(MINIMUM_COMMAND_TIME, 0);
		return true;

	case IDE_COMMAND_UNKNOWN_F9:
		/* only used by Killer Instinct AFAICT */
		LOGPRINT(("IDE unknown command (F9)\n"));

		/* signal an interrupt */
		set_irq(ASSERT_LINE);
		return true;

	case IDE_COMMAND_SET_FEATURES:
		LOGPRINT(("IDE Set features (%02X %02X %02X %02X %02X)\n", m_feature, m_sector_count & 0xff, m_sector_number, m_cylinder_low, m_cylinder_high));

		/* signal an interrupt */
		signal_delayed_interrupt(MINIMUM_COMMAND_TIME, 0);
		return true;

	case IDE_COMMAND_SET_BLOCK_COUNT:
		LOGPRINT(("IDE Set block count (%02X)\n", m_sector_count));

		m_block_count = m_sector_count;

		/* signal an interrupt */
		set_irq(ASSERT_LINE);
		return true;

	case IDE_COMMAND_SEEK:
		/* signal an interrupt */
		set_irq(ASSERT_LINE);
		return true;

	default:
		return false;
	}
}

UINT16 ide_mass_storage_device::read_dma()
{
	UINT16 result = 0xffff;

	if (device_selected())
	{
		if (!m_dmack)
		{
			logerror( "%s: read_dma ignored (!DMACK)\n", machine().describe_context() );
		}
		else if( !m_dmarq)
		{
			logerror( "%s: read_dma ignored (!DMARQ)\n", machine().describe_context() );
		}
		else if (m_status & IDE_STATUS_BSY)
		{
			logerror( "%s: read_dma ignored (BSY)\n", machine().describe_context() );
		}
		else if (!(m_status & IDE_STATUS_DRQ))
		{
			logerror( "%s: read_dma ignored (!DRQ)\n", machine().describe_context() );
		}
		else
		{
			result = m_buffer[m_buffer_offset++];
			result |= m_buffer[m_buffer_offset++] << 8;

			if (m_buffer_offset >= IDE_DISK_SECTOR_SIZE)
			{
				LOG(("%s:IDE completed DMA read\n", machine().describe_context()));
				read_buffer_empty();
			}
		}
	}

	return result;
}

READ16_MEMBER( ide_mass_storage_device::read_cs0 )
{
	/* logit */
//  if (offset != IDE_CS0_DATA_RW && offset != IDE_CS0_STATUS_R)
		LOG(("%s:IDE cs0 read at %X, mem_mask=%d\n", machine().describe_context(), offset, mem_mask));

	UINT16 result = 0xffff;

	if (device_selected() || single_device())
	{
		if (m_dmack)
		{
			logerror( "%s: read_cs0 %04x %04x ignored (DMACK)\n", machine().describe_context(), offset, mem_mask );
		}
		else if ((m_status & IDE_STATUS_BSY) && offset != IDE_CS0_STATUS_R)
		{
			if (device_selected())
			{
				switch (offset)
				{
					case IDE_CS0_DATA_RW:
						logerror( "%s: read_cs0 %04x %04x ignored (BSY)\n", machine().describe_context(), offset, mem_mask );
						break;

					default:
						result = m_status;

						if (m_last_status_timer->elapsed() > TIME_PER_ROTATION)
						{
							result |= IDE_STATUS_IDX;
							m_last_status_timer->adjust(attotime::never);
						}
						break;
				}
			}
			else
			{
				result = 0;
			}
		}
		else
		{
			switch (offset)
			{
				/* read data if there's data to be read */
				case IDE_CS0_DATA_RW:
					if (device_selected())
					{
						if (m_status & IDE_STATUS_DRQ)
						{
							/* fetch the correct amount of data */
							result = m_buffer[m_buffer_offset++];
							if (mem_mask == 0xffff)
								result |= m_buffer[m_buffer_offset++] << 8;

							/* if we're at the end of the buffer, handle it */
							if (m_buffer_offset >= IDE_DISK_SECTOR_SIZE)
							{
								LOG(("%s:IDE completed PIO read\n", machine().describe_context()));
								read_buffer_empty();
							}
						}
					}
					else
					{
						result = 0;
					}
					break;

				/* return the current error */
				case IDE_CS0_ERROR_R:
					result = m_error;
					break;

				/* return the current sector count */
				case IDE_CS0_SECTOR_COUNT_RW:
					result = m_sector_count;
					break;

				/* return the current sector */
				case IDE_CS0_SECTOR_NUMBER_RW:
					result = m_sector_number;
					break;

				/* return the current cylinder LSB */
				case IDE_CS0_CYLINDER_LOW_RW:
					result = m_cylinder_low;
					break;

				/* return the current cylinder MSB */
				case IDE_CS0_CYLINDER_HIGH_RW:
					result = m_cylinder_high;
					break;

				/* return the current head */
				case IDE_CS0_DEVICE_HEAD_RW:
					result = m_device_head;
					break;

				/* return the current status and clear any pending interrupts */
				case IDE_CS0_STATUS_R:
					if (device_selected())
					{
						result = m_status;

						if (m_last_status_timer->elapsed() > TIME_PER_ROTATION)
						{
							result |= IDE_STATUS_IDX;
							m_last_status_timer->adjust(attotime::never);
						}

						if (!(m_status & IDE_STATUS_DRDY) && is_ready())
						{
							m_status |= IDE_STATUS_DRDY;
						}

						set_irq(CLEAR_LINE);
					}
					else
					{
						result = 0;
					}
					break;

				/* log anything else */
				default:
					logerror("%s:unknown IDE cs0 read at %03X, mem_mask=%d\n", machine().describe_context(), offset, mem_mask);
					break;
			}
		}
	}

	/* return the result */
	return result;
}

READ16_MEMBER( ide_mass_storage_device::read_cs1 )
{
	/* logit */
//  if (offset != IDE_CS1_ALTERNATE_STATUS_R)
		LOG(("%s:IDE cs1 read at %X, mem_mask=%d\n", machine().describe_context(), offset, mem_mask));

	UINT16 result = 0xffff;

	if (device_selected() || single_device())
	{
		if (m_dmack)
		{
			logerror( "%s: read_cs1 %04x %04x ignored (DMACK)\n", machine().describe_context(), offset, mem_mask );
		}
		else
		{
			switch (offset)
			{
				case IDE_CS1_ALTERNATE_STATUS_R:
					if( device_selected() )
					{
						/* return the current status but don't clear interrupts */
						result = m_status;
						if (m_last_status_timer->elapsed() > TIME_PER_ROTATION)
						{
							result |= IDE_STATUS_IDX;
							m_last_status_timer->adjust(attotime::never);
						}
					}
					else
					{
						result = 0;
					}
					break;

				/* log anything else */
				default:
					logerror("%s:unknown IDE cs1 read at %03X, mem_mask=%d\n", machine().describe_context(), offset, mem_mask);
					break;
			}
		}
	}

	/* return the result */
	return result;
}

void ide_mass_storage_device::write_dma( UINT16 data )
{
	if (device_selected())
	{
		if (!m_dmack)
		{
			logerror( "%s: write_dma %04x ignored (!DMACK)\n", machine().describe_context(), data );
		}
		else if( !m_dmarq)
		{
			logerror( "%s: write_dma %04x ignored (!DMARQ)\n", machine().describe_context(), data );
		}
		else if (m_status & IDE_STATUS_BSY)
		{
			logerror( "%s: write_dma %04x ignored (BSY)\n", machine().describe_context(), data );
		}
		else if (!(m_status & IDE_STATUS_DRQ))
		{
			logerror( "%s: write_dma %04x ignored (!DRQ)\n", machine().describe_context(), data );
		}
		else
		{
			m_buffer[m_buffer_offset++] = data;
			m_buffer[m_buffer_offset++] = data >> 8;

			/* if we're at the end of the buffer, handle it */
			if (m_buffer_offset >= IDE_DISK_SECTOR_SIZE)
			{
				LOG(("%s:IDE completed DMA write\n", machine().describe_context()));
				write_buffer_full();
			}
		}
	}
}

WRITE16_MEMBER( ide_mass_storage_device::write_cs0 )
{
	/* logit */
	if (offset != IDE_CS0_DATA_RW)
		LOG(("%s:IDE cs0 write to %X = %08X, mem_mask=%d\n", machine().describe_context(), offset, data, mem_mask));
	//  fprintf(stderr, "ide write %03x %02x mem_mask=%d\n", offset, data, size);

	if (m_dmack)
	{
		logerror( "%s: write_cs0 %04x %04x %04x ignored (DMACK)\n", machine().describe_context(), offset, data, mem_mask );
	}
	else if ((m_status & IDE_STATUS_BSY) && offset != IDE_CS0_COMMAND_W)
	{
		logerror( "%s: write_cs0 %04x %04x %04x ignored (BSY)\n", machine().describe_context(), offset, data, mem_mask );
	}
	else if ((m_status & IDE_STATUS_DRQ) && offset != IDE_CS0_DATA_RW && offset != IDE_CS0_COMMAND_W)
	{
		logerror( "%s: write_cs0 %04x %04x %04x ignored (DRQ)\n", machine().describe_context(), offset, data, mem_mask );
	}
	else
	{
		UINT8 old;

		switch (offset)
		{
			/* write data */
			case IDE_CS0_DATA_RW:
				if( device_selected() )
				{
					if (!(m_status & IDE_STATUS_DRQ))
					{
						logerror( "%s: write_cs0 %04x %04x %04x ignored (!DRQ)\n", machine().describe_context(), offset, data, mem_mask );
					}
					else
					{
						/* store the correct amount of data */
						m_buffer[m_buffer_offset++] = data;
						if (mem_mask == 0xffff)
							m_buffer[m_buffer_offset++] = data >> 8;

						/* if we're at the end of the buffer, handle it */
						if (m_buffer_offset >= IDE_DISK_SECTOR_SIZE)
						{
							LOG(("%s:IDE completed PIO write\n", machine().describe_context()));
							write_buffer_full();
						}
					}
				}
				break;

			case IDE_CS0_FEATURE_W:
				m_feature = data;
				break;

			/* sector count */
			case IDE_CS0_SECTOR_COUNT_RW:
				m_sector_count = data ? data : 256;
				break;

			/* current sector */
			case IDE_CS0_SECTOR_NUMBER_RW:
				m_sector_number = data;
				break;

			/* current cylinder LSB */
			case IDE_CS0_CYLINDER_LOW_RW:
				m_cylinder_low = data;
				break;

			/* current cylinder MSB */
			case IDE_CS0_CYLINDER_HIGH_RW:
				m_cylinder_high = data;
				break;

			/* current head */
			case IDE_CS0_DEVICE_HEAD_RW:
				old = m_device_head;
				m_device_head = data;

				if ((m_device_head ^ old) & IDE_DEVICE_HEAD_DRV)
					update_irq();
				break;

			/* command */
			case IDE_CS0_COMMAND_W:
				m_command = data;

				if (device_selected() || m_command == IDE_COMMAND_DIAGNOSTIC)
				{
					/* implicitly clear interrupts & dmarq here */
					set_irq(CLEAR_LINE);
					set_dmarq(CLEAR_LINE);

					m_buffer_offset = 0;
					m_sectors_until_int = 0;

					m_status &= ~IDE_STATUS_ERR;

					if (!process_command())
					{
						LOGPRINT(("IDE unknown command (%02X)\n", m_command));
						m_status |= IDE_STATUS_ERR;
						m_error = IDE_ERROR_UNKNOWN_COMMAND;
						set_irq(ASSERT_LINE);
						//debugger_break(device->machine());
					}
				}
				break;
		}
	}
}

WRITE16_MEMBER( ide_mass_storage_device::write_cs1 )
{
	/* logit */
	LOG(("%s:IDE cs1 write to %X = %08X, mem_mask=%d\n", machine().describe_context(), offset, data, mem_mask));

	if (m_dmack)
	{
		logerror( "%s: write_cs1 %04x %04x %04x ignored (DMACK)\n", machine().describe_context(), offset, data, mem_mask );
	}
	else
	{
		UINT8 old;

		switch (offset)
		{
			/* adapter control */
			case IDE_CS1_DEVICE_CONTROL_W:
				old = m_device_control;
				m_device_control = data;

				if ((m_device_control ^ old) & IDE_DEVICE_CONTROL_NIEN)
					update_irq();

				if ((m_device_control ^ old) & IDE_DEVICE_CONTROL_SRST)
				{
					if (m_device_control & IDE_DEVICE_CONTROL_SRST)
					{
						m_status |= IDE_STATUS_BSY;
						set_irq(CLEAR_LINE);
						m_reset_timer->adjust(attotime::from_msec(5));
					}
				}
				break;
		}
	}
}

//**************************************************************************
//  IDE HARD DISK DEVICE
//**************************************************************************

// device type definition
const device_type IDE_HARDDISK = &device_creator<ide_hdd_device>;

//-------------------------------------------------
//  ide_hdd_device - constructor
//-------------------------------------------------

ide_hdd_device::ide_hdd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ide_mass_storage_device(mconfig, IDE_HARDDISK, "IDE Hard Disk", tag, owner, clock, "hdd", __FILE__)
{
}

ide_hdd_device::ide_hdd_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: ide_mass_storage_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ide_hdd_device::device_reset()
{
	m_handle = subdevice<harddisk_image_device>("harddisk")->get_chd_file();

	if (m_handle)
	{
		m_disk = subdevice<harddisk_image_device>("harddisk")->get_hard_disk_file();
	}
	else
	{
		m_handle = get_disk_handle(machine(), tag());
		m_disk = hard_disk_open(m_handle);
	}

	ide_mass_storage_device::device_reset();

	if (m_disk != NULL)
	{
		const hard_disk_info *hdinfo = hard_disk_get_info(m_disk);
		if (hdinfo->sectorbytes == IDE_DISK_SECTOR_SIZE)
		{
			m_num_cylinders = hdinfo->cylinders;
			m_num_sectors = hdinfo->sectors;
			m_num_heads = hdinfo->heads;
			if (PRINTF_IDE_COMMANDS) mame_printf_debug("CHS: %d %d %d\n", m_num_cylinders, m_num_heads, m_num_sectors);
			mame_printf_debug("CHS: %d %d %d\n", m_num_cylinders, m_num_heads, m_num_sectors);
		}
		// build the features page
		UINT32 metalength;
		if (m_handle->read_metadata (HARD_DISK_IDENT_METADATA_TAG, 0, m_identify_device, IDE_DISK_SECTOR_SIZE, metalength) != CHDERR_NONE)
			ide_build_identify_device();

		m_can_identify_device = 1;
	}
}

//-------------------------------------------------
//  read device key
//-------------------------------------------------

void ide_hdd_device::read_key(UINT8 key[])
{
	UINT32 metalength;
	m_handle->read_metadata(HARD_DISK_KEY_METADATA_TAG, 0, key, 5, metalength);
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------
static MACHINE_CONFIG_FRAGMENT( hdd_image )
	MCFG_HARDDISK_ADD( "harddisk" )
MACHINE_CONFIG_END

machine_config_constructor ide_hdd_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( hdd_image );
}
