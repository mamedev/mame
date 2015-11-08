// license:BSD-3-Clause
// copyright-holders:smf
#include "atahle.h"

#define VERBOSE                     0
#define PRINTF_IDE_COMMANDS         0

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)
#define LOGPRINT(x) do { if (VERBOSE) logerror x; if (PRINTF_IDE_COMMANDS) osd_printf_debug x; } while (0)

enum
{
	IDE_CS0_DATA_RW = 0,
	IDE_CS0_ERROR_R = 1,
	IDE_CS0_FEATURE_W = 1,
	IDE_CS0_SECTOR_COUNT_RW = 2,
	IDE_CS0_SECTOR_NUMBER_RW = 3,
	IDE_CS0_CYLINDER_LOW_RW = 4,
	IDE_CS0_CYLINDER_HIGH_RW = 5,
	IDE_CS0_DEVICE_HEAD_RW = 6,
	IDE_CS0_STATUS_R = 7,
	IDE_CS0_COMMAND_W = 7
};

enum
{
	IDE_CS1_ALTERNATE_STATUS_R = 6,
	IDE_CS1_DEVICE_CONTROL_W = 6,
	IDE_CS1_ACTIVE_STATUS = 7
};

enum
{
	IDE_DEVICE_CONTROL_NIEN = 0x02,
	IDE_DEVICE_CONTROL_SRST = 0x04
};

#define DETECT_DEVICE1_TIME                 (attotime::from_msec(2))
#define DEVICE1_PDIAG_TIME                  (attotime::from_msec(2))
#define DIAGNOSTIC_TIME                     (attotime::from_msec(2))

ata_hle_device::ata_hle_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock,const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	ata_device_interface(mconfig, *this),
	device_slot_card_interface(mconfig, *this),
	m_buffer_offset(0),
	m_buffer_size(0),
	m_error(0),
	m_feature(0),
	m_sector_count(0),
	m_sector_number(0),
	m_cylinder_low(0),
	m_cylinder_high(0),
	m_device_head(0),
	m_status(0),
	m_command(0),
	m_device_control(0),
	m_revert_to_defaults(true),
	m_8bit_data_transfers(false),
	m_csel(0),
	m_daspin(0),
	m_daspout(0),
	m_dmack(0),
	m_dmarq(0),
	m_irq(0),
	m_pdiagin(0),
	m_pdiagout(0),
	m_single_device(0),
	m_resetting(0)
{
}

void ata_hle_device::device_start()
{
	MINIMUM_COMMAND_TIME = attotime::from_usec(10);

	m_irq_handler.resolve_safe();
	m_dmarq_handler.resolve_safe();
	m_dasp_handler.resolve_safe();
	m_pdiag_handler.resolve_safe();

	m_buffer.resize(sector_length());
	save_item(NAME(m_buffer));
	save_item(NAME(m_buffer_offset));
	save_item(NAME(m_buffer_size));
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
	save_item(NAME(m_revert_to_defaults));

	save_item(NAME(m_single_device));
	save_item(NAME(m_resetting));

	save_item(NAME(m_csel));
	save_item(NAME(m_daspin));
	save_item(NAME(m_daspout));
	save_item(NAME(m_dmack));
	save_item(NAME(m_dmarq));
	save_item(NAME(m_irq));
	save_item(NAME(m_pdiagin));
	save_item(NAME(m_pdiagout));

	save_item(NAME(m_identify_buffer));

	m_busy_timer = timer_alloc(TID_BUSY);
	m_buffer_empty_timer = timer_alloc(TID_BUFFER_EMPTY);
}

void ata_hle_device::device_reset()
{
	/* reset the drive state */
	set_dasp(CLEAR_LINE);
	set_dmarq(CLEAR_LINE);
	set_irq(CLEAR_LINE);
	set_pdiag(CLEAR_LINE);

	m_status = 0;
	m_device_control = 0;
	m_resetting = true;

	if (m_csel == 0)
	{
		start_busy(DETECT_DEVICE1_TIME, PARAM_DETECT_DEVICE1);
	}
	else
	{
		set_dasp(ASSERT_LINE);
		soft_reset();
	}
}

void ata_hle_device::soft_reset()
{
	m_buffer_offset = 0;
	m_buffer_size = 0;
	m_status = 0;

	if (is_ready())
	{
		m_status |= IDE_STATUS_DRDY;
	}

	start_busy(DIAGNOSTIC_TIME, PARAM_DIAGNOSTIC);
}

void ata_hle_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TID_BUSY:
		m_status &= ~IDE_STATUS_BSY;

		finished_busy(param);
		break;
	case TID_BUFFER_EMPTY:
		m_buffer_empty_timer->enable(false);
		fill_buffer();
		break;
	}
}

void ata_hle_device::finished_busy(int param)
{
	switch (param)
	{
	case PARAM_DETECT_DEVICE1:
		m_single_device = (m_daspin == CLEAR_LINE);
		soft_reset();
		break;

	case PARAM_DIAGNOSTIC:
		start_diagnostic();
		break;

	case PARAM_WAIT_FOR_PDIAG:
		m_error |= IDE_ERROR_DIAGNOSTIC_DEVICE1_FAILED;
		finished_diagnostic();
		break;

	case PARAM_COMMAND:
		finished_command();
		break;
	}
}

void ata_hle_device::process_command()
{
	switch (m_command)
	{
	case IDE_COMMAND_DIAGNOSTIC:
		start_busy(DIAGNOSTIC_TIME, PARAM_COMMAND);
		break;

	case IDE_COMMAND_SET_FEATURES:
		start_busy(MINIMUM_COMMAND_TIME, PARAM_COMMAND);
		break;

	case IDE_COMMAND_CACHE_FLUSH:
		start_busy(MINIMUM_COMMAND_TIME, PARAM_COMMAND);
		break;

	default:
		LOGPRINT(("IDE unknown command (%02X)\n", m_command));
		m_status |= IDE_STATUS_ERR;
		m_error = IDE_ERROR_ABRT;
		set_irq(ASSERT_LINE);
		//debugger_break(device->machine());
		break;
	}
}

void ata_hle_device::finished_command()
{
	switch (m_command)
	{
	case IDE_COMMAND_DIAGNOSTIC:
		start_diagnostic();

		if (m_csel == 0)
			set_irq(ASSERT_LINE);
		break;

	case IDE_COMMAND_SET_FEATURES:
		if (!set_features())
		{
			LOGPRINT(("IDE Set features failed (%02X %02X %02X %02X %02X)\n", m_feature, m_sector_count & 0xff, m_sector_number, m_cylinder_low, m_cylinder_high));

			m_status |= IDE_STATUS_ERR;
			m_error = IDE_ERROR_ABRT;
		}
		set_irq(ASSERT_LINE);
		break;

	case IDE_COMMAND_CACHE_FLUSH:
		m_status |= IDE_STATUS_DRDY;
		break;

	default:
		logerror( "finished_command() unhandled command %02x\n", m_command );
		break;
	}
}

bool ata_hle_device::set_dma_mode(int word)
{
	if ((m_identify_buffer[word] >> (m_sector_count & 7)) & 1)
	{
		m_identify_buffer[62] &= 0xff;
		m_identify_buffer[63] &= 0xff;
		m_identify_buffer[88] &= 0xff;

		m_identify_buffer[word] |= 0x100 << (m_sector_count & 7);
		return true;
	}

	return false;
}

bool ata_hle_device::set_features()
{
	switch (m_feature)
	{
	case IDE_SET_FEATURES_ENABLE_8BIT_DATA_TRANSFERS:
		m_8bit_data_transfers = true;
		return true;

	case IDE_SET_FEATURES_TRANSFER_MODE:
		switch (m_sector_count & IDE_TRANSFER_TYPE_MASK)
		{
		case IDE_TRANSFER_TYPE_PIO_DEFAULT:
			switch (m_sector_count & 7)
			{
			case 0:
			case 1:
				return true;
			}
			break;

		case IDE_TRANSFER_TYPE_PIO_FLOW_CONTROL:
			switch (m_sector_count & 7)
			{
			case 0:
			case 1:
			case 2:
				return true;

			default:
				if ((m_identify_buffer[64] >> ((m_sector_count & 7) - 3)) & 1)
				{
					return true;
				}
			}
			break;

		case IDE_TRANSFER_TYPE_SINGLE_WORD_DMA:
			return set_dma_mode(62);

		case IDE_TRANSFER_TYPE_MULTI_WORD_DMA:
			return set_dma_mode(63);

		case IDE_TRANSFER_TYPE_ULTRA_DMA:
			return set_dma_mode(88);
		}
		break;

	case IDE_SET_FEATURES_DISABLE_REVERTING_TO_POWER_ON_DEFAULTS:
		m_revert_to_defaults = false;
		return true;

	case IDE_SET_FEATURES_DISABLE_8BIT_DATA_TRANSFERS:
		m_8bit_data_transfers = false;
		return true;

	case IDE_SET_FEATURES_ENABLE_REVERTING_TO_POWER_ON_DEFAULTS:
		m_revert_to_defaults = true;
		return true;
	}

	return false;
}

int ata_hle_device::bit_to_mode(UINT16 word)
{
	switch (word>>8)
	{
	case 0x01:
		return 0;
	case 0x02:
		return 1;
	case 0x04:
		return 2;
	case 0x08:
		return 3;
	case 0x10:
		return 4;
	case 0x20:
		return 5;
	case 0x40:
		return 6;
	case 0x080:
		return 7;
	}

	return -1;
}

// Return the currently selected single word dma mode, -1 if none selected
int ata_hle_device::single_word_dma_mode()
{
	return bit_to_mode(m_identify_buffer[62]);
}

// Return the currently selected multi word dma mode, -1 if none selected
int ata_hle_device::multi_word_dma_mode()
{
	return bit_to_mode(m_identify_buffer[63]);
}

// Return the currently selected ultra dma mode, -1 if none selected
int ata_hle_device::ultra_dma_mode()
{
	return bit_to_mode(m_identify_buffer[88]);
}

UINT16 ata_hle_device::read_data()
{
	/* fetch the correct amount of data */
	UINT16 result = m_buffer[m_buffer_offset++];
	if (!m_8bit_data_transfers)
		result |= m_buffer[m_buffer_offset++] << 8;

	/* if we're at the end of the buffer, handle it */
	if (m_buffer_offset >= m_buffer_size)
	{
		LOG(("%s:IDE completed PIO read\n", machine().describe_context()));
		read_buffer_empty();
	}

	return result;
}

void ata_hle_device::write_data(UINT16 data)
{
	/* store the correct amount of data */
	m_buffer[m_buffer_offset++] = data;
	if (!m_8bit_data_transfers)
		m_buffer[m_buffer_offset++] = data >> 8;

	/* if we're at the end of the buffer, handle it */
	if (m_buffer_offset >= m_buffer_size)
	{
		LOG(("%s:IDE completed PIO write\n", machine().describe_context()));
		write_buffer_full();
	}
}

void ata_hle_device::update_irq()
{
	if (device_selected() && (m_device_control & IDE_DEVICE_CONTROL_NIEN) == 0)
		m_irq_handler(m_irq);
	else
		m_irq_handler(CLEAR_LINE);
}

void ata_hle_device::set_irq(int state)
{
	if (m_irq != state)
	{
		m_irq = state;

		update_irq();
	}
}

void ata_hle_device::set_dmarq(int state)
{
	if (m_dmarq != state)
	{
		m_dmarq = state;

		m_dmarq_handler(state);
	}
}

void ata_hle_device::set_dasp(int state)
{
	if (m_daspout != state)
	{
		m_daspout = state;

		m_dasp_handler(state);
	}
}

void ata_hle_device::set_pdiag(int state)
{
	if (m_pdiagout != state)
	{
		m_pdiagout = state;

		m_pdiag_handler(state);
	}
}

void ata_hle_device::start_busy(const attotime &time, int param)
{
	m_status |= IDE_STATUS_BSY;
	m_busy_timer->adjust(time, param);
}

void ata_hle_device::stop_busy()
{
	m_status &= ~IDE_STATUS_BSY;
	m_busy_timer->adjust(attotime::never);
}

void ata_hle_device::read_buffer_empty()
{
	m_buffer_offset = 0;

	m_status &= ~IDE_STATUS_DRQ;

	if ((multi_word_dma_mode() >= 0) || (ultra_dma_mode() >= 0))
		set_dmarq(CLEAR_LINE);

	if (ultra_dma_mode() >= 0) {
		m_buffer_empty_timer->enable(true);
		m_buffer_empty_timer->adjust(attotime::zero);
	}
	else
		fill_buffer();
}

void ata_hle_device::write_buffer_full()
{
	m_buffer_offset = 0;

	m_status &= ~IDE_STATUS_DRQ;

	if ((multi_word_dma_mode() >= 0) || (ultra_dma_mode() >= 0))
		set_dmarq(CLEAR_LINE);

	process_buffer();
}

void ata_hle_device::start_diagnostic()
{
	m_error = IDE_ERROR_DIAGNOSTIC_FAILED;

	perform_diagnostic();

	if (m_csel == 1 && m_error == IDE_ERROR_DIAGNOSTIC_PASSED)
		set_pdiag(ASSERT_LINE);

	if (m_csel == 0 && !m_single_device && m_pdiagin == CLEAR_LINE)
		start_busy(DEVICE1_PDIAG_TIME, PARAM_WAIT_FOR_PDIAG);
	else
		finished_diagnostic();
}

void ata_hle_device::finished_diagnostic()
{
	m_resetting = false;

	signature();
}


WRITE_LINE_MEMBER( ata_hle_device::write_csel )
{
	m_csel = state;
}

WRITE_LINE_MEMBER( ata_hle_device::write_dasp )
{
	m_daspin = state;
}

WRITE_LINE_MEMBER( ata_hle_device::write_dmack )
{
	if (state && !m_dmack && single_word_dma_mode() >= 0)
		set_dmarq(CLEAR_LINE);

	m_dmack = state;
}

WRITE_LINE_MEMBER( ata_hle_device::write_pdiag )
{
	m_pdiagin = state;

	if (m_pdiagin == ASSERT_LINE && m_busy_timer->param() == PARAM_WAIT_FOR_PDIAG)
	{
		stop_busy();
		finished_diagnostic();
	}
}

UINT16 ata_hle_device::read_dma()
{
	UINT16 result = 0xffff;

	if (device_selected())
	{
		if (!m_dmack)
		{
			logerror( "%s: %s dev %d read_dma ignored (!DMACK)\n", machine().describe_context(), tag(), dev() );
		}
		else if (m_dmarq && single_word_dma_mode() >= 0)
		{
			logerror( "%s: %s dev %d read_dma ignored (DMARQ)\n", machine().describe_context(), tag(), dev() );
		}
		else if (!m_dmarq && multi_word_dma_mode() >= 0)
		{
			logerror( "%s: %s dev %d read_dma ignored (!DMARQ)\n", machine().describe_context(), tag(), dev() );
		}
		else if (!m_dmarq && ultra_dma_mode() >= 0)
		{
			logerror("%s: %s dev %d read_dma ignored (!DMARQ)\n", machine().describe_context(), tag(), dev());
		}
		else if (m_status & IDE_STATUS_BSY)
		{
			logerror( "%s: %s dev %d read_dma ignored (BSY)\n", machine().describe_context(), tag(), dev() );
		}
		else if (!(m_status & IDE_STATUS_DRQ))
		{
			logerror( "%s: %s dev %d read_dma ignored (!DRQ)\n", machine().describe_context(), tag(), dev() );
		}
		else
		{
			result = read_data();

			if ((m_status & IDE_STATUS_DRQ) && single_word_dma_mode() >= 0)
				set_dmarq(ASSERT_LINE);
		}
	}

	return result;
}

READ16_MEMBER( ata_hle_device::read_cs0 )
{
	/* logit */
//  if (offset != IDE_CS0_DATA_RW && offset != IDE_CS0_STATUS_R)
		LOG(("%s:IDE cs0 read at %X, mem_mask=%d\n", machine().describe_context(), offset, mem_mask));

	UINT16 result = 0xffff;

	if (device_selected() || m_single_device)
	{
		if (m_dmack)
		{
			logerror( "%s: %s dev %d read_cs0 %04x %04x ignored (DMACK)\n", machine().describe_context(), tag(), dev(), offset, mem_mask );
		}
		else if ((m_status & IDE_STATUS_BSY) && offset != IDE_CS0_STATUS_R)
		{
			// ATA5 spec says status reads should also go through here, but this breaks Primal Rage 2.
			// Real hardware might work due to read ahead in the vt83c461.
			if (device_selected())
			{
				switch (offset)
				{
					case IDE_CS0_DATA_RW:
						logerror( "%s: %s dev %d read_cs0 %04x %04x ignored (BSY)\n", machine().describe_context(), tag(), dev(), offset, mem_mask );
						break;

					default:
						result = calculate_status();
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
						if (!(m_status & IDE_STATUS_DRQ))
						{
							logerror( "%s: %s dev %d read_cs0 ignored (!DRQ)\n", machine().describe_context(), tag(), dev() );
						}
						else
						{
							result = read_data();
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
						result = calculate_status();

						if (!(m_status & IDE_STATUS_DRDY) && is_ready())
							m_status |= IDE_STATUS_DRDY;

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

READ16_MEMBER( ata_hle_device::read_cs1 )
{
	/* logit */
//  if (offset != IDE_CS1_ALTERNATE_STATUS_R)
		LOG(("%s:IDE cs1 read at %X, mem_mask=%d\n", machine().describe_context(), offset, mem_mask));

	UINT16 result = 0xffff;

	if (device_selected() || m_single_device)
	{
		if (m_dmack)
		{
			logerror( "%s: %s dev %d read_cs1 %04x %04x ignored (DMACK)\n", machine().describe_context(), tag(), dev(), offset, mem_mask );
		}
		else
		{
			switch (offset)
			{
				case IDE_CS1_ALTERNATE_STATUS_R:
					if (device_selected())
					{
						result = calculate_status();
					}
					else
					{
						result = 0;
					}
					break;

				case IDE_CS1_ACTIVE_STATUS:
					/*

					    bit     description

					    0       master active
					    1       slave active
					    2       complement of active disk head bit 0
					    3       complement of active disk head bit 1
					    4       complement of active disk head bit 2
					    5       complement of active disk head bit 3
					    6       write in progress
					    7       floppy present (unused)

					*/
					if (device_selected())
					{
						result = 0x01;
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

void ata_hle_device::write_dma( UINT16 data )
{
	if (device_selected())
	{
		if (!m_dmack)
		{
			logerror( "%s: %s dev %d write_dma %04x ignored (!DMACK)\n", machine().describe_context(), tag(), dev(), data );
		}
		else if (m_dmarq && single_word_dma_mode() >= 0)
		{
			logerror( "%s: %s dev %d write_dma %04x ignored (DMARQ)\n", machine().describe_context(), tag(), dev(), data );
		}
		else if (!m_dmarq && multi_word_dma_mode() >= 0)
		{
			logerror( "%s: %s dev %d write_dma %04x ignored (!DMARQ)\n", machine().describe_context(), tag(), dev(), data );
		}
		else if (!m_dmarq && ultra_dma_mode() >= 0)
		{
			logerror("%s: %s dev %d write_dma %04x ignored (!DMARQ)\n", machine().describe_context(), tag(), dev(), data);
		}
		else if (m_status & IDE_STATUS_BSY)
		{
			logerror( "%s: %s dev %d write_dma %04x ignored (BSY)\n", machine().describe_context(), tag(), dev(), data );
		}
		else if (!(m_status & IDE_STATUS_DRQ))
		{
			logerror( "%s: %s dev %d write_dma %04x ignored (!DRQ)\n", machine().describe_context(), tag(), dev(), data );
		}
		else
		{
			write_data(data);

			if ((m_status & IDE_STATUS_DRQ) && single_word_dma_mode() >= 0)
				set_dmarq(ASSERT_LINE);
		}
	}
}

WRITE16_MEMBER( ata_hle_device::write_cs0 )
{
	/* logit */
	if (offset != IDE_CS0_DATA_RW)
		LOG(("%s:IDE cs0 write to %X = %08X, mem_mask=%d\n", machine().describe_context(), offset, data, mem_mask));
	//  fprintf(stderr, "ide write %03x %02x mem_mask=%d\n", offset, data, size);

	if (m_dmack)
	{
		logerror( "%s: %s dev %d write_cs0 %04x %04x %04x ignored (DMACK)\n", machine().describe_context(), tag(), dev(), offset, data, mem_mask );
	}
	else if ((m_status & IDE_STATUS_BSY) && offset != IDE_CS0_COMMAND_W)
	{
		logerror( "%s: %s dev %d write_cs0 %04x %04x %04x ignored (BSY) command %02x\n", machine().describe_context(), tag(), dev(), offset, data, mem_mask, m_command );
	}
	else if ((m_status & IDE_STATUS_DRQ) && offset != IDE_CS0_DATA_RW && offset != IDE_CS0_COMMAND_W)
	{
		logerror( "%s: %s dev %d write_cs0 %04x %04x %04x ignored (DRQ) command %02x\n", machine().describe_context(), tag(), dev(), offset, data, mem_mask, m_command );
	}
	else
	{
		UINT8 old;

		switch (offset)
		{
			/* write data */
			case IDE_CS0_DATA_RW:
				if (device_selected())
				{
					if (!(m_status & IDE_STATUS_DRQ))
					{
						logerror( "%s: %s dev %d write_cs0 %04x %04x %04x ignored (!DRQ)\n", machine().describe_context(), tag(), dev(), offset, data, mem_mask );
					}
					else
					{
						write_data(data);
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
				// Packet devices can accept DEVICE RESET when BSY or DRQ is set.
				if (m_status & IDE_STATUS_BSY)
				{
					logerror( "%s: %s dev %d write_cs0 %04x %04x %04x ignored (BSY) command %02x\n", machine().describe_context(), tag(), dev(), offset, data, mem_mask, m_command );
				}
				else if (m_status & IDE_STATUS_DRQ)
				{
					logerror( "%s: %s dev %d write_cs0 %04x %04x %04x ignored (DRQ) command %02x\n", machine().describe_context(), tag(), dev(), offset, data, mem_mask, m_command );
				}
				else if (device_selected() || m_command == IDE_COMMAND_DIAGNOSTIC)
				{
					m_command = data;

					/* implicitly clear interrupts & dmarq here */
					set_irq(CLEAR_LINE);
					set_dmarq(CLEAR_LINE);

					m_buffer_offset = 0;

					set_dasp(CLEAR_LINE);
					m_status &= ~IDE_STATUS_DRQ;
					m_status &= ~IDE_STATUS_ERR;

					process_command();
				}
				break;

			default:
				logerror("%s:unknown IDE cs0 write at %03X = %04x, mem_mask=%d\n", machine().describe_context(), offset, data, mem_mask);
				break;
		}
	}
}

WRITE16_MEMBER( ata_hle_device::write_cs1 )
{
	/* logit */
	LOG(("%s:IDE cs1 write to %X = %08X, mem_mask=%d\n", machine().describe_context(), offset, data, mem_mask));

	if (m_dmack)
	{
		logerror( "%s: %s dev %d write_cs1 %04x %04x %04x ignored (DMACK)\n", machine().describe_context(), tag(), dev(), offset, data, mem_mask );
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
						if (m_resetting)
						{
							logerror( "%s: %s dev %d write_cs1 %04x %04x %04x ignored (RESET)\n", machine().describe_context(), tag(), dev(), offset, data, mem_mask );
						}
						else
						{
							set_dasp(CLEAR_LINE);
							set_dmarq(CLEAR_LINE);
							set_irq(CLEAR_LINE);
							set_pdiag(CLEAR_LINE);

							start_busy(attotime::never, PARAM_RESET);
						}
					}
					else if (m_busy_timer->param() == PARAM_RESET)
					{
						soft_reset();
					}
				}
				break;

			default:
				logerror("%s:unknown IDE cs1 write at %03X = %04x, mem_mask=%d\n", machine().describe_context(), offset, data, mem_mask);
				break;
		}
	}
}
