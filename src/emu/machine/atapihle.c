#include "atapihle.h"

atapi_hle_device::atapi_hle_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock,const char *shortname, const char *source)
	: ata_hle_device(mconfig, type, name, tag, owner, clock, shortname, source),
	m_packet(0),
	m_data_size(0),
	m_scsidev_device(*this, "device")
{
}

void atapi_hle_device::process_buffer()
{
	if (m_packet)
	{
		int phase;

	//  printf( "atapi command %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
	//      m_buffer[0],m_buffer[1],m_buffer[2],m_buffer[3],
	//      m_buffer[4],m_buffer[5],m_buffer[6],m_buffer[7],
	//      m_buffer[8],m_buffer[9],m_buffer[10],m_buffer[11]);

		m_error = 0; // HACK: This might not be the right place, but firebeat needs this cleared at some point

		m_scsidev_device->SetCommand( m_buffer, m_buffer_size );
		m_scsidev_device->ExecCommand( &m_data_size );
		m_scsidev_device->GetPhase( &phase );

		m_buffer_size = (m_cylinder_high << 8) | m_cylinder_low;
		if (m_buffer_size == 0xffff)
			m_buffer_size = 0xfffe;

	//  printf("atapi result %08x %08x\n", m_data_size, m_buffer_size);

		if (m_buffer_size > ATAPI_BUFFER_LENGTH || m_buffer_size == 0)
			m_buffer_size = ATAPI_BUFFER_LENGTH;

		// TODO: dma flag
		switch (phase)
		{
		case SCSI_PHASE_DATAOUT:
			if (m_buffer_size > m_data_size)
			{
				m_buffer_size = m_data_size;
			}

			m_cylinder_low = m_buffer_size & 0xff;
			m_cylinder_high = m_buffer_size >> 8;

			if (m_buffer_size > 0)
			{
				m_status |= IDE_STATUS_DRQ;
				m_sector_count = 0;
			}
			else
			{
				m_sector_count = ATAPI_INTERRUPT_REASON_IO | ATAPI_INTERRUPT_REASON_CD;
			}
			break;

		case SCSI_PHASE_DATAIN:
			/// TODO: delay data
			fill_buffer();
			break;

		default:
			m_sector_count = ATAPI_INTERRUPT_REASON_IO | ATAPI_INTERRUPT_REASON_CD;
			break;
		}

		set_irq(ASSERT_LINE);

		m_packet = 0;
	}
	else
	{
		switch (m_command)
		{
		case IDE_COMMAND_PACKET:
			m_scsidev_device->WriteData( m_buffer, m_buffer_size );
			m_data_size -= m_buffer_size;

			if (m_buffer_size > m_data_size)
			{
				m_buffer_size = m_data_size;
			}

			m_cylinder_low = m_buffer_size & 0xff;
			m_cylinder_high = m_buffer_size >> 8;

			if (m_buffer_size > 0)
			{
				m_status |= IDE_STATUS_DRQ;
				m_sector_count = 0;
			}
			else
			{
				m_sector_count = ATAPI_INTERRUPT_REASON_IO | ATAPI_INTERRUPT_REASON_CD;
			}

			set_irq(ASSERT_LINE);
			break;
		}
	}
}

void atapi_hle_device::fill_buffer()
{
	switch (m_command)
	{
	case IDE_COMMAND_PACKET:
		if (m_buffer_size > m_data_size)
		{
			m_buffer_size = m_data_size;
		}

		m_cylinder_low = m_buffer_size & 0xff;
		m_cylinder_high = m_buffer_size >> 8;

		if (m_buffer_size > 0)
		{
			m_scsidev_device->ReadData( m_buffer, m_buffer_size );
			m_data_size -= m_buffer_size;

			m_status |= IDE_STATUS_DRQ;
			m_sector_count = ATAPI_INTERRUPT_REASON_IO;
		}
		else
		{
			m_sector_count = ATAPI_INTERRUPT_REASON_IO | ATAPI_INTERRUPT_REASON_CD;
		}

		set_irq(ASSERT_LINE);
		break;

	case IDE_COMMAND_IDENTIFY_PACKET_DEVICE:
		m_sector_count = ATAPI_INTERRUPT_REASON_IO | ATAPI_INTERRUPT_REASON_CD;
		set_irq(ASSERT_LINE);
		break;
	}
}

void atapi_hle_device::signature()
{
	// TODO: IDENTIFY DEVICE & READ SECTORS writes signature too.
	m_sector_count = 1;
	m_sector_number = 1;
	m_cylinder_low = 0x14;
	m_cylinder_high = 0xeb;

	m_device_head &= IDE_DEVICE_HEAD_DRV;
}

void atapi_hle_device::process_command()
{
	m_packet = 0;

	switch (m_command)
	{
	case IDE_COMMAND_DEVICE_RESET:
		soft_reset();
		break;

	case IDE_COMMAND_PACKET:
		m_packet = 1;
		m_buffer_size = 12;
		m_status |= IDE_STATUS_DRQ;
		m_sector_count = ATAPI_INTERRUPT_REASON_CD;
		set_irq(ASSERT_LINE);
		break;

	case IDE_COMMAND_IDENTIFY_PACKET_DEVICE:
		identify_packet_device();

		m_status |= IDE_STATUS_DRQ;
		set_irq(ASSERT_LINE);
		break;

	case IDE_COMMAND_IDENTIFY_DEVICE:
		/// TODO: busy
		signature();
		m_status |= IDE_STATUS_ERR;
		m_error = IDE_ERROR_ABRT;
		set_irq(ASSERT_LINE);
		break;

	default:
		ata_hle_device::process_command();
		break;
	}
}

void atapi_hle_device::finished_command()
{
	switch (m_command)
	{
	default:
		ata_hle_device::finished_command();
		break;
	}
}
