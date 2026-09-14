// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "atastorage.h"

#include "multibyte.h"

#define LOG_READ (1U << 2)
#define LOG_WRITE (1U << 3)
#define LOG_BUFFER (1U << 4)
#define LOG_SECURITY (1U << 5)

//#define VERBOSE (LOG_GENERAL | LOG_SECURITY)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGREAD(...) LOGMASKED(LOG_READ, __VA_ARGS__)
#define LOGWRITE(...) LOGMASKED(LOG_WRITE, __VA_ARGS__)
#define LOGBUFFER(...) LOGMASKED(LOG_SECURITY, __VA_ARGS__)
#define LOGSECURITY(...) LOGMASKED(LOG_SECURITY, __VA_ARGS__)


#define TIME_PER_SECTOR_WRITE               (attotime::from_usec(100))
#define TIME_PER_ROTATION                   (attotime::from_hz(5400/60))
#define TIME_BETWEEN_SECTORS                (attotime::from_nsec(400))

#define TIME_FULL_STROKE_SEEK               (attotime::from_usec(13000))
#define TIME_AVERAGE_ROTATIONAL_LATENCY     (attotime::from_usec(1300))

device_ata_mass_storage_device_interface::device_ata_mass_storage_device_interface(const machine_config &mconfig, device_t &device) :
	device_ata_hle_interface(mconfig, device),
	m_can_identify_device(0),
	m_num_cylinders(0),
	m_num_sectors(0),
	m_num_heads(0), m_cur_lba(0), m_block_count(0), m_sectors_until_int(0), m_sectors_remaining(0), m_master_password_enable(0), m_user_password_enable(0),
	m_master_password(nullptr),
	m_user_password(nullptr),
	m_dma_transfer_time(attotime::zero)
{
}

uint64_t device_ata_mass_storage_device_interface::lba_address()
{
	/* LBA direct? */
	if (m_device_head & IDE_DEVICE_HEAD_L)
	{
		return ((is_lba48() ?
			((uint64_t)m_cylinder_high_hob << 16) | ((uint64_t)m_cylinder_low_hob << 8) | m_sector_number_hob : m_device_head & IDE_DEVICE_HEAD_HS) << 24) |
			((uint64_t)m_cylinder_high << 16) | ((uint64_t)m_cylinder_low << 8) | m_sector_number;
	}

	/* standard CHS */
	else
		return (((((m_cylinder_high << 8) | m_cylinder_low) * m_num_heads) + (m_device_head & IDE_DEVICE_HEAD_HS)) * m_num_sectors) + m_sector_number - 1;
}

uint32_t device_ata_mass_storage_device_interface::sectors_remaining()
{
	if (is_lba48())
		return (m_sector_count_hob || m_sector_count) ? (m_sector_count_hob << 8) | m_sector_count : 0x10000;

	return m_sector_count ? m_sector_count : 0x100;
}

bool device_ata_mass_storage_device_interface::is_lba48()
{
	return m_command == IDE_COMMAND_READ_SECTORS_EXT ||
		m_command == IDE_COMMAND_READ_DMA_EXT ||
		m_command == IDE_COMMAND_READ_MULTIPLE_EXT ||
		m_command == IDE_COMMAND_WRITE_SECTORS_EXT ||
		m_command == IDE_COMMAND_WRITE_DMA_EXT ||
		m_command == IDE_COMMAND_WRITE_MULTIPLE_EXT ||
		m_command == IDE_COMMAND_VERIFY_SECTORS_EXT;
}

void device_ata_mass_storage_device_interface::ide_build_identify_device()
{
	memset(m_identify_buffer, 0, sizeof(m_identify_buffer));
	uint64_t total_sectors = m_num_cylinders * m_num_heads * m_num_sectors;

	/* basic geometry */
	m_identify_buffer[0] = 0x045a;                     /*  0: configuration bits */
	m_identify_buffer[1] = m_num_cylinders;            /*  1: logical cylinders */
	m_identify_buffer[2] = 0;                          /*  2: reserved */
	m_identify_buffer[3] = m_num_heads;                /*  3: logical heads */
	m_identify_buffer[4] = 0;                          /*  4: vendor specific (obsolete) */
	m_identify_buffer[5] = 0;                          /*  5: vendor specific (obsolete) */
	m_identify_buffer[6] = m_num_sectors;              /*  6: logical sectors per logical track */
	m_identify_buffer[7] = 0;                          /*  7: vendor-specific */
	m_identify_buffer[8] = 0;                          /*  8: vendor-specific */
	m_identify_buffer[9] = 0;                          /*  9: vendor-specific */
	identify(10, 10, "00000000000000000000");          /* 10-19: serial number */
	m_identify_buffer[20] = 0;                         /* 20: vendor-specific */
	m_identify_buffer[21] = 0;                         /* 21: vendor-specific */
	m_identify_buffer[22] = 4;                         /* 22: # of vendor-specific bytes on read/write long commands */
	identify(23, 4, "1.0");                            /* 23-26: firmware revision */
	identify(27, 20, "MAME    Virtual Hard Disk");     /* 27-46: model number */
	m_identify_buffer[47] = 0x8010;                    /* 47: read/write multiple support, value from Seagate Quantum Fireball */
	m_identify_buffer[48] = 0;                         /* 48: reserved */
	m_identify_buffer[49] = 0x0f03;                    /* 49: capabilities */
	m_identify_buffer[50] = 0;                         /* 50: reserved */
	m_identify_buffer[51] = 2;                         /* 51: PIO data transfer cycle timing mode */
	m_identify_buffer[52] = 2;                         /* 52: single word DMA transfer cycle timing mode */
	m_identify_buffer[53] = 3;                         /* 53: field validity */

	if (total_sectors >= 16514064)
	{
		/// CHS limit
		m_identify_buffer[1] = 16383;                  /*  1: logical cylinders */
		m_identify_buffer[3] = 16;                     /*  3: logical heads */
		m_identify_buffer[6] = 63;                     /*  6: logical sectors per logical track */
		m_identify_buffer[54] = 16383;                 /* 54: number of current logical cylinders */
		m_identify_buffer[55] = 16;                    /* 55: number of current logical heads */
		m_identify_buffer[56] = 63;                    /* 56: number of current logical sectors per track */
		m_identify_buffer[57] = 16514064 & 0xffff;     /* 57-58: current capacity in sectors (ATA-1 through ATA-5; obsoleted in ATA-6) */
		m_identify_buffer[58] = 16514064 >> 16;
	}
	else
	{
		m_identify_buffer[54] = m_num_cylinders;       /* 54: number of current logical cylinders */
		m_identify_buffer[55] = m_num_heads;           /* 55: number of current logical heads */
		m_identify_buffer[56] = m_num_sectors;         /* 56: number of current logical sectors per track */
		m_identify_buffer[57] = total_sectors & 0xffff; /* 57-58: current capacity in sectors (ATA-1 through ATA-5; obsoleted in ATA-6) */
		m_identify_buffer[58] = total_sectors >> 16;
	}

	m_identify_buffer[59] = 0;                         /* 59: multiple sector timing */

	if (total_sectors > 0xfffffffULL)
	{
		m_identify_buffer[60] = 0xfffffffULL & 0xffff;
		m_identify_buffer[61] = 0xfffffffULL >> 16;
	}
	else
	{
		m_identify_buffer[60] = total_sectors & 0xffff; /* 60-61: total user addressable sectors for LBA mode (ATA-1 through ATA-7) */
		m_identify_buffer[61] = total_sectors >> 16;
	}

	m_identify_buffer[62] = 0x0007;                    /* 62: single word dma transfer */
	m_identify_buffer[63] = 0x0407;                    /* 63: multiword DMA transfer */
	m_identify_buffer[64] = 0x0003;                    /* 64: flow control PIO transfer modes supported */
	m_identify_buffer[65] = 0x78;                      /* 65: minimum multiword DMA transfer cycle time per word */
	m_identify_buffer[66] = 0x78;                      /* 66: mfr's recommended multiword DMA transfer cycle time */
	m_identify_buffer[67] = 0x014d;                    /* 67: minimum PIO transfer cycle time without flow control */
	m_identify_buffer[68] = 0x78;                      /* 68: minimum PIO transfer cycle time with IORDY */
	m_identify_buffer[69] = 0x00;                      /* 69-70: reserved */
	m_identify_buffer[71] = 0x00;                      /* 71: reserved for IDENTIFY PACKET command */
	m_identify_buffer[72] = 0x00;                      /* 72: reserved for IDENTIFY PACKET command */
	m_identify_buffer[73] = 0x00;                      /* 73: reserved for IDENTIFY PACKET command */
	m_identify_buffer[74] = 0x00;                      /* 74: reserved for IDENTIFY PACKET command */
	m_identify_buffer[75] = 0x00;                      /* 75: queue depth */
	m_identify_buffer[76] = 0x00;                      /* 76-79: reserved */
	m_identify_buffer[80] = 0x00;                      /* 80: major version number */
	m_identify_buffer[81] = 0x00;                      /* 81: minor version number */
	m_identify_buffer[82] = 0x00;                      /* 82: command set supported */
	m_identify_buffer[83] = 0x00;                      /* 83: command sets supported */

	if (total_sectors > 0xfffffffULL)
		m_identify_buffer[83] |= COMMAND_SET_SUPPORTED_LBA48;

	m_identify_buffer[84] = 0x00;                      /* 84: command set/feature supported extension */
	m_identify_buffer[85] = 0x00;                      /* 85: command set/feature enabled */
	m_identify_buffer[86] = 0x00;                      /* 86: command set/feature enabled */
	m_identify_buffer[87] = 0x00;                      /* 87: command set/feature default */
	m_identify_buffer[88] = 0x00;                      /* 88: additional DMA modes (ultra dma) */
	m_identify_buffer[89] = 0x00;                      /* 89: time required for security erase unit completion */
	m_identify_buffer[90] = 0x00;                      /* 90: time required for enhanced security erase unit completion */
	m_identify_buffer[91] = 0x00;                      /* 91: current advanced power management value */
	m_identify_buffer[92] = 0x00;                      /* 92: master password revision code */
	m_identify_buffer[93] = 0x00;                      /* 93: hardware reset result */
	m_identify_buffer[94] = 0x00;                      /* 94: acoustic management values */
	m_identify_buffer[95] = 0x00;                      /* 95-99: reserved */

	if (m_identify_buffer[83] & COMMAND_SET_SUPPORTED_LBA48)
	{
		m_identify_buffer[100] = total_sectors & 0xffff; /* 100-103: maximum 48-bit LBA */
		m_identify_buffer[101] = total_sectors >> 16;
		m_identify_buffer[102] = total_sectors >> 32;
		m_identify_buffer[103] = total_sectors >> 48;
	}

	m_identify_buffer[104] = 0x00;                     /* 104-126: reserved */
	m_identify_buffer[127] = 0x00;                     /* 127: removable media status notification */
	m_identify_buffer[128] = 0x00;                     /* 128: security status */
	m_identify_buffer[129] = 0x00;                     /* 129-159: vendor specific */
	m_identify_buffer[160] = 0x00;                     /* 160: CFA power mode 1 */
	m_identify_buffer[161] = 0x00;                     /* 161-175: reserved for CompactFlash */
	m_identify_buffer[176] = 0x00;                     /* 176-205: current media serial number */
	m_identify_buffer[206] = 0x00;                     /* 206-254: reserved */
	m_identify_buffer[255] = 0x00;                     /* 255: integrity word */
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void device_ata_mass_storage_device_interface::interface_post_start()
{
	device_ata_hle_interface::interface_post_start();

	device().save_item(NAME(m_can_identify_device));
	device().save_item(NAME(m_num_cylinders));
	device().save_item(NAME(m_num_sectors));
	device().save_item(NAME(m_num_heads));

	device().save_item(NAME(m_cur_lba));
	device().save_item(NAME(m_block_count));
	device().save_item(NAME(m_sectors_until_int));
	device().save_item(NAME(m_sectors_remaining));
	device().save_item(NAME(m_master_password_enable));
	device().save_item(NAME(m_user_password_enable));
}

void device_ata_mass_storage_device_interface::soft_reset()
{
	device_ata_hle_interface::soft_reset();

	m_cur_lba = 0;
	m_status |= IDE_STATUS_DSC;

	m_master_password_enable = (m_master_password != nullptr);
	m_user_password_enable = (m_user_password != nullptr);
}

void device_ata_mass_storage_device_interface::perform_diagnostic()
{
	if (m_can_identify_device)
		m_error = IDE_ERROR_DIAGNOSTIC_PASSED;
}

void device_ata_mass_storage_device_interface::signature()
{
	m_sector_count = 1;
	m_sector_number = 1;
	m_cylinder_low = 0;
	m_cylinder_high = 0;
	m_device_head = 0;
}

void device_ata_mass_storage_device_interface::finished_command()
{
	int total_sectors = m_num_cylinders * m_num_heads * m_num_sectors;

	switch (m_command)
	{
	case IDE_COMMAND_IDENTIFY_DEVICE:
		if (m_can_identify_device)
		{
			for( int w = 0; w < 256; w++ )
				put_u16le(&m_buffer[w * 2], m_identify_buffer[ w ]);

			m_status |= IDE_STATUS_DRQ;
		}
		else
		{
			m_status |= IDE_STATUS_ERR;
			m_error = IDE_ERROR_NONE;
		}

		set_irq(ASSERT_LINE);
		break;

	case IDE_COMMAND_SET_CONFIG:
		set_geometry(m_sector_count ? m_sector_count : 0x100, (m_device_head & IDE_DEVICE_HEAD_HS) + 1);
		set_irq(ASSERT_LINE);
		break;

	case IDE_COMMAND_READ_SECTORS:
	case IDE_COMMAND_READ_SECTORS_EXT:
	case IDE_COMMAND_READ_SECTORS_NORETRY:
	case IDE_COMMAND_READ_MULTIPLE:
	case IDE_COMMAND_READ_MULTIPLE_EXT:
	case IDE_COMMAND_VERIFY_SECTORS:
	case IDE_COMMAND_VERIFY_SECTORS_EXT:
	case IDE_COMMAND_VERIFY_SECTORS_NORETRY:
	case IDE_COMMAND_READ_DMA:
	case IDE_COMMAND_READ_DMA_EXT:
	case IDE_COMMAND_READ_BUFFER:
		finished_read();
		break;

	case IDE_COMMAND_WRITE_SECTORS:
	case IDE_COMMAND_WRITE_SECTORS_EXT:
	case IDE_COMMAND_WRITE_SECTORS_NORETRY:
	case IDE_COMMAND_WRITE_MULTIPLE:
	case IDE_COMMAND_WRITE_MULTIPLE_EXT:
	case IDE_COMMAND_WRITE_DMA:
	case IDE_COMMAND_WRITE_DMA_EXT:
	case IDE_COMMAND_WRITE_BUFFER:
		finished_write();
		break;

	case IDE_COMMAND_RECALIBRATE:
		set_irq(ASSERT_LINE);
		break;

	case IDE_COMMAND_READ_NATIVE_MAX_ADDRESS:
		put_u32be(&m_buffer[0], total_sectors);
		set_irq(ASSERT_LINE);
		break;

	default:
		device_ata_hle_interface::finished_command();
		break;
	}
}

void device_ata_mass_storage_device_interface::next_sector()
{
	uint8_t cur_head = m_device_head & IDE_DEVICE_HEAD_HS;

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
				if (m_cylinder_high == 0)
					cur_head++;
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
			cur_head++;
			if (cur_head >= m_num_heads)
			{
				cur_head = 0;
				m_cylinder_low++;
				if (m_cylinder_low == 0)
					m_cylinder_high++;
			}
		}
	}

	m_device_head = (m_device_head & ~IDE_DEVICE_HEAD_HS) | cur_head;

	m_cur_lba = lba_address();
}


void device_ata_mass_storage_device_interface::security_error()
{
	LOGSECURITY("Security error\n");
	/* set error state */
	m_status |= IDE_STATUS_ERR;
	m_error = IDE_ERROR_ABRT;
}

attotime device_ata_mass_storage_device_interface::seek_time()
{
	int sectors_per_cylinder =  m_num_heads * m_num_sectors;

	if (sectors_per_cylinder == 0 || m_num_cylinders == 0)
		return attotime::zero;

	uint64_t new_lba = lba_address();
	int64_t old_cylinder = m_cur_lba / sectors_per_cylinder;
	int64_t new_cylinder = new_lba / sectors_per_cylinder;
	int64_t diff = abs(old_cylinder - new_cylinder);

	m_cur_lba = new_lba;

	if (diff == 0)
	{
		return TIME_BETWEEN_SECTORS;
	}

	attotime seek_time = (TIME_FULL_STROKE_SEEK * diff) / m_num_cylinders;

	return seek_time + TIME_AVERAGE_ROTATIONAL_LATENCY;
}

void device_ata_mass_storage_device_interface::fill_buffer()
{
	switch (m_command)
	{
	case IDE_COMMAND_IDENTIFY_DEVICE:
		break;

	case IDE_COMMAND_READ_MULTIPLE:
	case IDE_COMMAND_READ_MULTIPLE_EXT:
		/* if there is more data to read, keep going */
		if (m_sectors_remaining > 0)
		{
			m_sectors_remaining--;
			m_sector_count = m_sectors_remaining;
			m_sector_count_hob = m_sectors_remaining >> 8;
		}

		if (m_sectors_remaining > 0)
		{
			// Read the next sector with no delay
			finished_read();
		}
		break;

	case IDE_COMMAND_READ_BUFFER:
		set_irq(ASSERT_LINE);
		break;

	default:
		/* if there is more data to read, keep going */
		if (m_sectors_remaining > 0)
		{
			m_sectors_remaining--;
			m_sector_count = m_sectors_remaining;
			m_sector_count_hob = m_sectors_remaining >> 8;
		}

		if (m_sectors_remaining > 0)
		{
			set_irq(CLEAR_LINE);
			set_dasp(ASSERT_LINE);
			if (m_command == IDE_COMMAND_READ_DMA || m_command == IDE_COMMAND_READ_DMA_EXT)
				start_busy(TIME_BETWEEN_SECTORS + m_dma_transfer_time, PARAM_COMMAND);
			else
				start_busy(TIME_BETWEEN_SECTORS, PARAM_COMMAND);
		}
		break;
	}
}


void device_ata_mass_storage_device_interface::finished_read()
{
	uint64_t lba = lba_address();
	int read_status;

	set_dasp(CLEAR_LINE);

	/* now do the read */
	if (m_command == IDE_COMMAND_READ_BUFFER)
	{
		read_status = 1;
	}
	else
	{
		read_status = read_sector(lba, &m_buffer[0]);
	}

	/* if we succeeded, advance to the next sector and set the nice bits */
	if (read_status)
	{
		/* advance the pointers, unless this is the last sector */
		/* Gauntlet: Dark Legacy checks to make sure we stop on the last sector */
		if (m_sectors_remaining != 1)
			next_sector();

		/* signal an interrupt, IDE_COMMAND_READ_MULTIPLE & IDE_COMMAND_READ_MULTIPLE_EXT sets the interrupt at the start the block */
		if (--m_sectors_until_int == 0 || (m_sectors_remaining == 1 && m_command != IDE_COMMAND_READ_MULTIPLE && m_command != IDE_COMMAND_READ_MULTIPLE_EXT))
		{
			m_sectors_until_int = ((m_command == IDE_COMMAND_READ_MULTIPLE || m_command == IDE_COMMAND_READ_MULTIPLE_EXT) ? m_block_count : 1);
			set_irq(ASSERT_LINE);
		}

		/* if we're just verifying we can read the next sector */
		if (m_command == IDE_COMMAND_VERIFY_SECTORS ||
			m_command == IDE_COMMAND_VERIFY_SECTORS_EXT ||
			m_command == IDE_COMMAND_VERIFY_SECTORS_NORETRY )
		{
			read_buffer_empty();
		}
		else
		{
			m_status |= IDE_STATUS_DRQ;

			if (m_command == IDE_COMMAND_READ_DMA || m_command == IDE_COMMAND_READ_DMA_EXT)
				set_dmarq(ASSERT_LINE);
		}
	}
	else
	{
		/* set the error flag and the error */
		m_status |= IDE_STATUS_ERR;
		m_error = IDE_ERROR_BAD_SECTOR;

		/* signal an interrupt */
		set_irq(ASSERT_LINE);
	}
}


void device_ata_mass_storage_device_interface::read_first_sector()
{
	if (m_master_password_enable || m_user_password_enable)
	{
		security_error();
	}
	else
	{
		set_dasp(ASSERT_LINE);

		if (m_command == IDE_COMMAND_READ_BUFFER)
		{
			// don't call seek_time() here (that will trash m_cur_lba), just give a nominal delay
			start_busy(TIME_BETWEEN_SECTORS, PARAM_COMMAND);
		}
		else
		{
			start_busy(seek_time(), PARAM_COMMAND);
		}
	}
}

void device_ata_mass_storage_device_interface::process_buffer()
{
	if (m_command == IDE_COMMAND_SECURITY_UNLOCK)
	{
		const uint8_t *password = (m_buffer[0] & IDE_SECURITY_UNLOCK_MASTER) ? m_master_password : m_user_password;

		if (VERBOSE & LOG_SECURITY)
		{
			std::string unlock;
			for (int i = 0; i < 32; i ++)
				unlock += util::string_format(" 0x%02x,", m_buffer[i + 2]);

			std::string stored;
			if (password)
				for (int i = 0; i < 32; i++)
					stored += util::string_format(" 0x%02x,", password[i]);

			std::string_view type = (m_buffer[0] & IDE_SECURITY_UNLOCK_MASTER) ? "master" : "user";
			std::string_view status = memcmp(password, &m_buffer[2], 32) == 0 ? "ok" : "error";

			LOGSECURITY("Unlock %s password %s\n", type, unlock);
			LOGSECURITY("Stored %s password %s (%s)\n", type, stored, status);
		}

		if (memcmp(password, &m_buffer[2], 32) == 0)
		{
			if (m_buffer[0] & IDE_SECURITY_UNLOCK_MASTER)
				m_master_password_enable = 0;
			else
				m_user_password_enable = 0;
		}
		else
			LOG("Security unlock failed\n");

		if (m_master_password_enable || m_user_password_enable)
			security_error();
	}
	else if (m_command == IDE_COMMAND_SECURITY_DISABLE_PASSWORD)
	{
		LOG("IDE Done unimplemented SECURITY_DISABLE_PASSWORD command\n");
	}
	else if (m_command == IDE_COMMAND_WRITE_BUFFER)
	{
		set_irq(ASSERT_LINE);
	}
	else
	{
		set_dasp(ASSERT_LINE);

		if (m_command == IDE_COMMAND_WRITE_MULTIPLE || m_command == IDE_COMMAND_WRITE_MULTIPLE_EXT)
		{
			if (m_sectors_until_int != 1)
			{
				/* ready to write now */
				finished_write();
			}
			else
			{
				/* set a timer to do the write */
				start_busy(TIME_PER_SECTOR_WRITE, PARAM_COMMAND);
			}
		}
		else
		{
			/* set a timer to do the write */
			start_busy(TIME_PER_SECTOR_WRITE, PARAM_COMMAND);
		}
	}
}


void device_ata_mass_storage_device_interface::finished_write()
{
	uint64_t lba = lba_address();
	int count;

	set_dasp(CLEAR_LINE);

	/* now do the write */
	if (m_command == IDE_COMMAND_WRITE_BUFFER)
	{
		count = 1;
	}
	else
	{
		count = write_sector(lba, &m_buffer[0]);
	}

	/* if we succeeded, advance to the next sector and set the nice bits */
	if (count == 1)
	{
		/* advance the pointers, unless this is the last sector */
		/* Gauntlet: Dark Legacy checks to make sure we stop on the last sector */
		if (m_sectors_remaining != 1)
			next_sector();

		/* signal an interrupt */
		if (--m_sectors_until_int == 0 || m_sectors_remaining == 1)
		{
			m_sectors_until_int = ((m_command == IDE_COMMAND_WRITE_MULTIPLE || m_command == IDE_COMMAND_WRITE_MULTIPLE_EXT) ? m_block_count : 1);
			set_irq(ASSERT_LINE);
		}

		/* signal an interrupt if there's more data needed */
		if (m_sectors_remaining > 0)
		{
			m_sectors_remaining--;
			m_sector_count = m_sectors_remaining;
			m_sector_count_hob = m_sectors_remaining >> 8;
		}

		if (m_sectors_remaining > 0)
		{
			m_status |= IDE_STATUS_DRQ;

			if (m_command == IDE_COMMAND_WRITE_DMA || m_command == IDE_COMMAND_WRITE_DMA_EXT)
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


void device_ata_mass_storage_device_interface::process_command()
{
	if (is_lba48() && !(m_identify_buffer[83] & COMMAND_SET_SUPPORTED_LBA48))
	{
		device_ata_hle_interface::process_command();
		return;
	}

	m_sectors_until_int = 0;
	m_buffer_size = IDE_DISK_SECTOR_SIZE;

	switch (m_command)
	{
	case IDE_COMMAND_READ_SECTORS:
	case IDE_COMMAND_READ_SECTORS_EXT:
	case IDE_COMMAND_READ_SECTORS_NORETRY:
		m_sectors_remaining = sectors_remaining();
		LOGREAD("Read sectors: LBA=0x%06x count=%u\n", lba_address(), m_sectors_remaining);

		m_sectors_until_int = 1;

		/* start the read going */
		read_first_sector();
		break;

	case IDE_COMMAND_READ_BUFFER:
		m_sectors_remaining = sectors_remaining();
		LOGBUFFER("Read buffer\n");

		m_sectors_until_int = 1;
		m_buffer_offset = 0;

		/* start the read going */
		read_first_sector();
		break;

	case IDE_COMMAND_READ_MULTIPLE:
	case IDE_COMMAND_READ_MULTIPLE_EXT:
		m_sectors_remaining = sectors_remaining();
		LOGREAD("Read multiple: LBA=0x%06x count=%u\n", lba_address(), m_sectors_remaining);

		m_sectors_until_int = 1;

		/* start the read going */
		read_first_sector();
		break;

	case IDE_COMMAND_VERIFY_SECTORS:
	case IDE_COMMAND_VERIFY_SECTORS_NORETRY:
	case IDE_COMMAND_VERIFY_SECTORS_EXT:
		m_sectors_remaining = sectors_remaining();
		LOGREAD("Verify sectors: LBA=0x%06x count=%u\n", lba_address(), m_sectors_remaining);

		/* reset the buffer */
		m_sectors_until_int = m_sectors_remaining;

		/* start the read going */
		read_first_sector();
		break;

	case IDE_COMMAND_READ_DMA:
	case IDE_COMMAND_READ_DMA_EXT:
		m_sectors_remaining = sectors_remaining();
		LOGREAD("Read DMA: LBA=0x%06x count=%u\n", lba_address(), m_sectors_remaining);

		/* reset the buffer */
		m_sectors_until_int = m_sectors_remaining;

		/* start the read going */
		read_first_sector();
		break;

	case IDE_COMMAND_WRITE_SECTORS:
	case IDE_COMMAND_WRITE_SECTORS_EXT:
	case IDE_COMMAND_WRITE_SECTORS_NORETRY:
		m_sectors_remaining = sectors_remaining();
		LOGWRITE("Write sectors: LBA=0x%06x count=%u\n", lba_address(), m_sectors_remaining);

		/* reset the buffer */
		m_sectors_until_int = 1;

		/* mark the buffer ready */
		m_status |= IDE_STATUS_DRQ;
		break;

	case IDE_COMMAND_WRITE_BUFFER:
		m_sectors_remaining = sectors_remaining();
		LOGBUFFER("Write buffer\n");

		/* reset the buffer */
		m_sectors_until_int = 1;
		m_buffer_offset = 0;

		/* mark the buffer ready */
		m_status |= IDE_STATUS_DRQ;
		break;

	case IDE_COMMAND_WRITE_MULTIPLE:
	case IDE_COMMAND_WRITE_MULTIPLE_EXT:
		m_sectors_remaining = sectors_remaining();
		LOGWRITE("Write multiple: LBA=0x%06x count=%u\n", lba_address(), m_sectors_remaining);

		/* reset the buffer */
		m_sectors_until_int = m_block_count;

		/* mark the buffer ready */
		m_status |= IDE_STATUS_DRQ;
		break;

	case IDE_COMMAND_WRITE_DMA:
	case IDE_COMMAND_WRITE_DMA_EXT:
		m_sectors_remaining = sectors_remaining();
		LOGWRITE("Write multiple DMA: LBA=0x%06x count=%u\n", lba_address(), m_sectors_remaining);

		/* reset the buffer */
		m_sectors_until_int = m_sectors_remaining;

		/* mark the buffer ready */
		m_status |= IDE_STATUS_DRQ;

		/* start the read going */
		set_dmarq(ASSERT_LINE);
		break;

	case IDE_COMMAND_SECURITY_UNLOCK:
		LOGSECURITY("Security unlock\n");

		/* mark the buffer ready */
		m_status |= IDE_STATUS_DRQ;

		set_irq(ASSERT_LINE);
		break;

	case IDE_COMMAND_SECURITY_DISABLE_PASSWORD:
		LOG("IDE Unimplemented SECURITY DISABLE PASSWORD command\n");

		/* mark the buffer ready */
		m_status |= IDE_STATUS_DRQ;

		set_irq(ASSERT_LINE);
		break;

	case IDE_COMMAND_IDENTIFY_DEVICE:
		LOG("IDE Identify device\n");

		start_busy(MINIMUM_COMMAND_TIME, PARAM_COMMAND);
		break;

	case IDE_COMMAND_RECALIBRATE:
		start_busy(MINIMUM_COMMAND_TIME, PARAM_COMMAND);
		break;

	case IDE_COMMAND_IDLE:
		/* signal an interrupt */
		set_irq(ASSERT_LINE);
		break;

	case IDE_COMMAND_SET_CONFIG:
		LOG("IDE Set configuration (%u heads, %u sectors)\n",
				(m_device_head & IDE_DEVICE_HEAD_HS) + 1,
				m_sector_count);

		start_busy(MINIMUM_COMMAND_TIME, PARAM_COMMAND);
		break;

	case IDE_COMMAND_SET_MAX:
		LOG("IDE Set max (%02X %02X %02X %02X %02X)\n",
				m_feature,
				m_sector_count,
				m_sector_number,
				m_cylinder_low, m_cylinder_high);

		/* signal an interrupt */
		set_irq(ASSERT_LINE);
		break;

	case IDE_COMMAND_SET_BLOCK_COUNT:
		m_block_count = m_sector_count ? m_sector_count : 0x100;

		LOG("IDE Set block count (%u)\n", m_block_count);

		/* signal an interrupt */
		set_irq(ASSERT_LINE);
		break;

	case IDE_COMMAND_SEEK:
		/* signal an interrupt */
		set_irq(ASSERT_LINE);
		break;

	case IDE_COMMAND_READ_NATIVE_MAX_ADDRESS:
		start_busy(MINIMUM_COMMAND_TIME, PARAM_COMMAND);
		break;

	case IDE_COMMAND_IDLE_IMMEDIATE:
		set_irq(ASSERT_LINE);
		break;

	default:
		device_ata_hle_interface::process_command();
		break;
	}
}

ide_hdd_device_base::ide_hdd_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_ata_mass_storage_device_interface(mconfig, *this),
	m_image(*this, "image")
{
}

void ide_hdd_device_base::device_start()
{
	/* create a timer for timing status */
	m_last_status_timer = machine().scheduler().timer_alloc(timer_expired_delegate());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ide_hdd_device_base::device_reset()
{
	if (m_image->exists() && !m_can_identify_device)
	{
		const auto &hdinfo = m_image->get_info();
		if (hdinfo.sectorbytes == IDE_DISK_SECTOR_SIZE)
		{
			m_num_cylinders = hdinfo.cylinders;
			m_num_sectors = hdinfo.sectors;
			m_num_heads = hdinfo.heads;
			osd_printf_verbose("%s: Mounted disk image CHS: %u %u %u\n",
					tag(),
					m_num_cylinders,
					m_num_heads,
					m_num_sectors);
		}

		// build the features page
		std::vector<u8> ident;
		m_image->get_inquiry_data(ident);
		if (ident.size() == 512)
		{
			for( int w = 0; w < 256; w++ )
				m_identify_buffer[w] = get_u16le(&ident[w * 2]);
		}
		else
		{
			ide_build_identify_device();
		}

		m_can_identify_device = 1;
	}
}

uint8_t ide_hdd_device_base::calculate_status()
{
	uint8_t result = device_ata_hle_interface::calculate_status();

	if (m_last_status_timer->elapsed() > TIME_PER_ROTATION)
	{
		result |= IDE_STATUS_IDX;
		m_last_status_timer->adjust(attotime::never);
	}

	return result;
}

void ide_hdd_device_base::device_add_mconfig(machine_config &config)
{
	HARDDISK(config, "image", "ide_hdd,hdd");
}

cf_device_base::cf_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	ide_hdd_device_base(mconfig, type, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void cf_device_base::device_add_mconfig(machine_config &config)
{
	HARDDISK(config, "image", "ata_cf");
}

//-------------------------------------------------
//  ide_build_identify_device
//-------------------------------------------------

void cf_device_base::ide_build_identify_device()
{
	memset(m_identify_buffer, 0, sizeof(m_identify_buffer));
	int total_sectors = m_num_cylinders * m_num_heads * m_num_sectors;

	/* basic geometry */
	m_identify_buffer[0] = 0x848a;                     /*  0: configuration bits */
	m_identify_buffer[1] = m_num_cylinders;            /*  1: logical cylinders */
	m_identify_buffer[2] = 0;                          /*  2: reserved */
	m_identify_buffer[3] = m_num_heads;                /*  3: logical heads */
	m_identify_buffer[4] = 0;                          /*  4: number of unformatted bytes per track */
	m_identify_buffer[5] = 0;                          /*  5: number of unformatted bytes per sector */
	m_identify_buffer[6] = m_num_sectors;              /*  6: logical sectors per logical track */
	m_identify_buffer[7] = total_sectors >> 16;        /*  7: number of sectors per card MSW */
	m_identify_buffer[8] = total_sectors & 0xffff;     /*  8: number of sectors per card LSW  */
	m_identify_buffer[9] = 0;                          /*  9: vendor-specific */
	identify(10, 10, "00000000000000000000");          /* 10-19: serial number */
	m_identify_buffer[20] = 0;                         /* 20: buffer type */
	m_identify_buffer[21] = 0;                         /* 21: buffer size in 512 byte increments */
	m_identify_buffer[22] = 4;                         /* 22: # of vendor-specific bytes on read/write long commands */
	identify(23, 4, "1.0");                            /* 23-26: firmware revision */
	identify(27, 20, "MAME    Virtual CompactFlash");  /* 27-46: model number */
	m_identify_buffer[47] = 0x0001;                    /* 47: read/write multiple support */
	m_identify_buffer[48] = 0;                         /* 48: double word not supported */
	m_identify_buffer[49] = 0x0200;                    /* 49: capabilities */
	m_identify_buffer[50] = 0;                         /* 50: reserved */
	m_identify_buffer[51] = 0x0200;                    /* 51: PIO data transfer cycle timing mode */
	m_identify_buffer[52] = 0x0000;                    /* 52: single word DMA transfer cycle timing mode */
	m_identify_buffer[53] = 0x0003;                    /* 53: translation parameters are valid */
	m_identify_buffer[54] = m_num_cylinders;           /* 54: number of current logical cylinders */
	m_identify_buffer[55] = m_num_heads;               /* 55: number of current logical heads */
	m_identify_buffer[56] = m_num_sectors;             /* 56: number of current logical sectors per track */
	m_identify_buffer[57] = total_sectors & 0xffff;    /* 57-58: current capacity in sectors */
	m_identify_buffer[58] = total_sectors >> 16;
	m_identify_buffer[59] = 0;                         /* 59: multiple sector timing */
	m_identify_buffer[60] = total_sectors & 0xffff;    /* 60-61: total user addressable sectors for LBA mode */
	m_identify_buffer[61] = total_sectors >> 16;
	m_identify_buffer[62] = 0x00;                      /* 62-127: reserved */
	m_identify_buffer[128] = 0x00;                     /* 128: security status */
	m_identify_buffer[129] = 0x00;                     /* 129-159: vendor specific */
	m_identify_buffer[160] = 0x00;                     /* 160:  Power requirement description*/
	m_identify_buffer[161] = 0x00;                     /* 161-255: reserved */
}

//-------------------------------------------------
//  seek_time
//-------------------------------------------------

attotime cf_device_base::seek_time()
{
	return attotime::zero;
}
