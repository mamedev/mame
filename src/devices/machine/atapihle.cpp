// license:BSD-3-Clause
// copyright-holders:smf
#include "atapihle.h"

atapi_hle_device::atapi_hle_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock,std::string shortname, std::string source)
	: ata_hle_device(mconfig, type, name, tag, owner, clock, shortname, source),
	m_packet(0),
	m_data_size(0)
{
}

void atapi_hle_device::device_start()
{
	t10_start(*this);
	ata_hle_device::device_start();
}

void atapi_hle_device::device_reset()
{
	t10_reset();
	ata_hle_device::device_reset();
}

void atapi_hle_device::process_buffer()
{
	if (m_packet)
	{
		//printf( "atapi command %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
		//    m_buffer[0],m_buffer[1],m_buffer[2],m_buffer[3],
		//    m_buffer[4],m_buffer[5],m_buffer[6],m_buffer[7],
		//    m_buffer[8],m_buffer[9],m_buffer[10],m_buffer[11]);

		m_error = 0; // HACK: This might not be the right place, but firebeat needs this cleared at some point

		SetCommand(&m_buffer[0], m_buffer_size);
		ExecCommand();
		GetLength(&m_data_size);

		if (m_status_code == SCSI_STATUS_CODE_CHECK_CONDITION)
			m_status |= IDE_STATUS_ERR;

		m_buffer_size = (m_cylinder_high << 8) | m_cylinder_low;
		if (m_buffer_size == 0xffff)
			m_buffer_size = 0xfffe;

		//printf("atapi result %08x %08x\n", m_data_size, m_buffer_size);

		if (m_buffer_size > ATAPI_BUFFER_LENGTH || m_buffer_size == 0)
			m_buffer_size = ATAPI_BUFFER_LENGTH;

		if (m_feature & ATAPI_FEATURES_FLAG_OVL)
		{
			printf( "ATAPI_FEATURES_FLAG_OVL not supported\n" );
		}

		switch (m_phase)
		{
		case SCSI_PHASE_DATAOUT:
			wait_buffer();
			break;

		case SCSI_PHASE_DATAIN:
			/// TODO: delay data
			fill_buffer();
			break;

		default:
			m_cylinder_low = 0;
			m_cylinder_high = 0;
			m_sector_count = ATAPI_INTERRUPT_REASON_IO | ATAPI_INTERRUPT_REASON_CD;
			set_irq(ASSERT_LINE);
			break;
		}

		m_packet = 0;
	}
	else
	{
		switch (m_command)
		{
		case IDE_COMMAND_PACKET:
			WriteData( &m_buffer[0], m_buffer_size );
			m_data_size -= m_buffer_size;

			wait_buffer();
			break;
		}
	}
}

void atapi_hle_device::fill_buffer()
{
	switch (m_command)
	{
	case IDE_COMMAND_PACKET:
		if (m_buffer_size >= m_data_size)
		{
			m_buffer_size = m_data_size;
		}
		else if (m_buffer_size & 1)
		{
			m_buffer_size--;
		}

		m_cylinder_low = m_buffer_size & 0xff;
		m_cylinder_high = m_buffer_size >> 8;

		if (m_buffer_size > 0)
		{
			ReadData( &m_buffer[0], m_buffer_size );
			m_data_size -= m_buffer_size;

			m_status |= IDE_STATUS_DRQ;
			m_sector_count = ATAPI_INTERRUPT_REASON_IO;

			if (m_feature & ATAPI_FEATURES_FLAG_DMA)
			{
				set_dmarq(ASSERT_LINE);
			}
			else
			{
				set_irq(ASSERT_LINE);
			}
		}
		else
		{
			m_sector_count = ATAPI_INTERRUPT_REASON_IO | ATAPI_INTERRUPT_REASON_CD;
			set_irq(ASSERT_LINE);
		}
		break;

	case IDE_COMMAND_IDENTIFY_PACKET_DEVICE:
		m_cylinder_low = 0;
		m_cylinder_high = 0;

		m_sector_count = ATAPI_INTERRUPT_REASON_IO | ATAPI_INTERRUPT_REASON_CD;
		set_irq(ASSERT_LINE);
		break;
	}
}

void atapi_hle_device::wait_buffer()
{
	if (m_buffer_size >= m_data_size)
	{
		m_buffer_size = m_data_size;
	}
	else if (m_buffer_size & 1)
	{
		m_buffer_size--;
	}

	m_cylinder_low = m_buffer_size & 0xff;
	m_cylinder_high = m_buffer_size >> 8;

	if (m_buffer_size > 0)
	{
		m_status |= IDE_STATUS_DRQ;
		m_sector_count = 0;

		if (m_feature & ATAPI_FEATURES_FLAG_DMA)
		{
			set_dmarq(ASSERT_LINE);
		}
		else
		{
			set_irq(ASSERT_LINE);
		}
	}
	else
	{
		m_sector_count = ATAPI_INTERRUPT_REASON_IO | ATAPI_INTERRUPT_REASON_CD;
		set_irq(ASSERT_LINE);
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

		if (packet_command_length() == PACKET_COMMAND_LENGTH_16)
		{
			m_buffer_size = 16;
		}
		else
		{
			m_buffer_size = 12;
		}

		m_status |= IDE_STATUS_DRQ;
		m_sector_count = ATAPI_INTERRUPT_REASON_CD;

		if (packet_command_response() == PACKET_COMMAND_RESPONSE_INTRQ)
		{
			set_irq(ASSERT_LINE);
		}
		break;

	case IDE_COMMAND_IDENTIFY_PACKET_DEVICE:
		identify_packet_device();

		for( int w = 0; w < 256; w++ )
		{
			m_buffer[w * 2] = m_identify_buffer[ w ] & 0xff;
			m_buffer[(w * 2) + 1] = m_identify_buffer[ w ] >> 8;
		}

		m_buffer_size = 512;

		m_error = 0;
		m_cylinder_low = m_buffer_size & 0xff;
		m_cylinder_high = m_buffer_size >> 8;

		m_status |= IDE_STATUS_DRQ;
		m_sector_count = ATAPI_INTERRUPT_REASON_IO;
		set_irq(ASSERT_LINE);
		break;

	case IDE_COMMAND_IDENTIFY_DEVICE:
		/// TODO: busy
		signature();
		m_status |= IDE_STATUS_ERR;
		m_error = IDE_ERROR_ABRT;
		set_irq(ASSERT_LINE);
		break;

	case IDE_COMMAND_CHECK_POWER_MODE:
		m_status = IDE_STATUS_DRDY;
		m_sector_count = 0xff;      // Power mode: 0x00 = Standby, 0x80 = Idle mode, 0xff = Active mode or Idle mode
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

atapi_hle_device::packet_command_length_t atapi_hle_device::packet_command_length()
{
	return (packet_command_length_t) (m_identify_buffer[0] & 3);
}

atapi_hle_device::packet_command_response_t atapi_hle_device::packet_command_response()
{
	return (packet_command_response_t) ((m_identify_buffer[0] >> 5 ) & 3);
}
