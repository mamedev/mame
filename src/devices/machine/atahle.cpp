// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "atahle.h"

#define LOG_COMMAND (1U << 1)
#define LOG_READ (1U << 2)
#define LOG_READDATA (1U << 3)
#define LOG_READSTATUS (1U << 4)
#define LOG_READCOMPLETED (1U << 5)
#define LOG_WRITE (1U << 6)
#define LOG_WRITEDATA (1U << 7)
#define LOG_WRITECOMPLETED (1U << 8)

//#define VERBOSE (LOG_GENERAL | LOG_COMMAND | LOG_READ | /* LOG_READDATA | LOG_READSTATUS | */ LOG_READCOMPLETED | LOG_WRITE | /* LOG_WRITEDATA | */ LOG_WRITECOMPLETED)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGCOMMAND(...)        LOGMASKED(LOG_COMMAND, __VA_ARGS__)
#define LOGREAD(...)           LOGMASKED(LOG_READ, __VA_ARGS__)
#define LOGREADDATA(...)       LOGMASKED(LOG_READDATA, __VA_ARGS__)
#define LOGREADSTATUS(...)     LOGMASKED(LOG_READSTATUS, __VA_ARGS__)
#define LOGREADCOMPLETED(...)  LOGMASKED(LOG_READCOMPLETED, __VA_ARGS__)
#define LOGWRITE(...)          LOGMASKED(LOG_WRITE, __VA_ARGS__)
#define LOGWRITEDATA(...)      LOGMASKED(LOG_WRITEDATA, __VA_ARGS__)
#define LOGWRITECOMPLETED(...) LOGMASKED(LOG_WRITECOMPLETED, __VA_ARGS__)

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

ata_hle_device_base::ata_hle_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
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
	m_resetting(0), m_busy_timer(nullptr), m_buffer_empty_timer(nullptr)
{
}

void ata_hle_device_base::device_start()
{
	MINIMUM_COMMAND_TIME = attotime::from_usec(10);

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
	save_item(NAME(m_8bit_data_transfers));

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

	m_busy_timer = timer_alloc(FUNC(ata_hle_device_base::busy_tick), this);
	m_buffer_empty_timer = timer_alloc(FUNC(ata_hle_device_base::empty_tick), this);
}

void ata_hle_device_base::device_reset()
{
	set_dasp(CLEAR_LINE);
	set_dmarq(CLEAR_LINE);
	set_irq(CLEAR_LINE);
	set_pdiag(CLEAR_LINE);

	m_status = 0;
	m_device_control = 0;
	m_resetting = true;

	if (m_csel == 0)
		start_busy(DETECT_DEVICE1_TIME, PARAM_DETECT_DEVICE1);
	else
	{
		set_dasp(ASSERT_LINE);
		soft_reset();
	}
}

void ata_hle_device_base::soft_reset()
{
	m_buffer_offset = 0;
	m_buffer_size = 0;
	m_status = 0;

	if (is_ready())
		m_status |= IDE_STATUS_DRDY;

	start_busy(DIAGNOSTIC_TIME, PARAM_DIAGNOSTIC);
}

TIMER_CALLBACK_MEMBER(ata_hle_device_base::busy_tick)
{
	m_status &= ~IDE_STATUS_BSY;
	finished_busy(param);
}

TIMER_CALLBACK_MEMBER(ata_hle_device_base::empty_tick)
{
	m_buffer_empty_timer->enable(false);
	fill_buffer();
}

void ata_hle_device_base::finished_busy(int32_t param)
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

void ata_hle_device_base::process_command()
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
		LOG("%s device %d process_command() unknown command (0x%02x)\n", machine().describe_context(), m_csel, m_command);
		m_status |= IDE_STATUS_ERR;
		m_error = IDE_ERROR_ABRT;
		set_irq(ASSERT_LINE);
		//machine().debug_break();
		break;
	}
}

void ata_hle_device_base::finished_command()
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
			LOGCOMMAND("IDE Set features failed (%02X %02X %02X %02X %02X)\n", m_feature, m_sector_count & 0xff, m_sector_number, m_cylinder_low, m_cylinder_high);

			m_status |= IDE_STATUS_ERR;
			m_error = IDE_ERROR_ABRT;
		}
		set_irq(ASSERT_LINE);
		break;

	case IDE_COMMAND_CACHE_FLUSH:
		m_status |= IDE_STATUS_DRDY;
		break;

	default:
		LOG("device %d finished_command() unhandled command 0x%02x\n", m_csel, m_command);
		break;
	}
}

void ata_hle_device_base::clear_dma_modes()
{
	m_identify_buffer[62] &= 0xff;
	m_identify_buffer[63] &= 0xff;
	m_identify_buffer[88] &= 0xff;
}

bool ata_hle_device_base::set_dma_mode(int word)
{
	if ((m_identify_buffer[word] >> (m_sector_count & 7)) & 1)
	{
		clear_dma_modes();

		m_identify_buffer[word] |= 0x100 << (m_sector_count & 7);
		return true;
	}

	return false;
}

bool ata_hle_device_base::set_features()
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
					clear_dma_modes();
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

	case IDE_SET_FEATURES_ENABLE_ECC:
	case IDE_SET_FEATURES_ENABLE_RETRIES:
	case IDE_SET_FEATURES_ENABLE_READ_LOOK_AHEAD:
		// not actually handled, but reply as if we did
		return true;
	}

	return false;
}

int ata_hle_device_base::bit_to_mode(uint16_t word)
{
	switch (word >> 8)
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
	case 0x80:
		return 7;
	}

	return -1;
}

int ata_hle_device_base::single_word_dma_mode()
{
	return bit_to_mode(m_identify_buffer[62]);
}

int ata_hle_device_base::multi_word_dma_mode()
{
	return bit_to_mode(m_identify_buffer[63]);
}

int ata_hle_device_base::ultra_dma_mode()
{
	return bit_to_mode(m_identify_buffer[88]);
}

uint16_t ata_hle_device_base::read_data()
{
	uint16_t result = m_buffer[m_buffer_offset++];
	if (!m_8bit_data_transfers)
		result |= m_buffer[m_buffer_offset++] << 8;

	if (m_buffer_offset >= m_buffer_size)
	{
		LOGREADCOMPLETED("%s device %d read_data() completed\n", machine().describe_context(), m_csel);
		read_buffer_empty();
	}

	return result;
}

void ata_hle_device_base::write_data(uint16_t data)
{
	m_buffer[m_buffer_offset++] = data;
	if (!m_8bit_data_transfers)
		m_buffer[m_buffer_offset++] = data >> 8;

	if (m_buffer_offset >= m_buffer_size)
	{
		LOGWRITECOMPLETED("%s device %d write_data() completed\n", machine().describe_context(), m_csel);
		write_buffer_full();
	}
}

void ata_hle_device_base::update_irq()
{
	if (device_selected() && (m_device_control & IDE_DEVICE_CONTROL_NIEN) == 0)
		set_irq_out(m_irq);
	else
		set_irq_out(CLEAR_LINE);
}

void ata_hle_device_base::start_busy(const attotime &time, int32_t param)
{
	m_status |= IDE_STATUS_BSY;
	m_busy_timer->adjust(time, param);
}

void ata_hle_device_base::stop_busy()
{
	m_status &= ~IDE_STATUS_BSY;
	m_busy_timer->adjust(attotime::never);
}

void ata_hle_device_base::read_buffer_empty()
{
	m_buffer_offset = 0;

	m_status &= ~IDE_STATUS_DRQ;
	set_dmarq(CLEAR_LINE);

	if (ultra_dma_mode() >= 0)
	{
		m_buffer_empty_timer->enable(true);
		m_buffer_empty_timer->adjust(attotime::zero);
	}
	else
		fill_buffer();
}

void ata_hle_device_base::write_buffer_full()
{
	m_buffer_offset = 0;

	m_status &= ~IDE_STATUS_DRQ;
	set_dmarq(CLEAR_LINE);

	process_buffer();
}

void ata_hle_device_base::start_diagnostic()
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

void ata_hle_device_base::finished_diagnostic()
{
	m_resetting = false;

	signature();
}


void ata_hle_device_base::set_dmack_in(int state)
{
	if (state && !m_dmack && single_word_dma_mode() >= 0)
		set_dmarq(CLEAR_LINE);

	m_dmack = state;
}

void ata_hle_device_base::set_pdiag_in(int state)
{
	m_pdiagin = state;

	if (m_pdiagin == ASSERT_LINE && m_busy_timer->param() == PARAM_WAIT_FOR_PDIAG)
	{
		stop_busy();
		finished_diagnostic();
	}
}

uint16_t ata_hle_device_base::dma_r()
{
	if (device_selected())
	{
		if (!m_dmack)
			LOG("%s device %d dma_r 0x%04x (ignored DMACK)\n", machine().describe_context(), m_csel, 0xffff);
		else if (m_dmarq && single_word_dma_mode() >= 0)
			LOG("%s device %d dma_r 0x%04x (ignored DMARQ)\n", machine().describe_context(), m_csel, 0xffff);
		else if (!m_dmarq && multi_word_dma_mode() >= 0)
			LOG("%s device %d dma_r 0x%04x (ignored !DMARQ)\n", machine().describe_context(), m_csel, 0xffff);
		else if (!m_dmarq && ultra_dma_mode() >= 0)
			LOG("%s device %d dma_r 0x%04x (ignored !DMARQ)\n", machine().describe_context(), m_csel, 0xffff);
		else if (m_status & IDE_STATUS_BSY)
			LOG("%s device %d dma_r 0x%04x (ignored BSY)\n", machine().describe_context(), m_csel, 0xffff);
		else if (!(m_status & IDE_STATUS_DRQ))
			LOG("%s device %d dma_r 0x%04x (ignored !DRQ)\n", machine().describe_context(), m_csel, 0xffff);
		else
		{
			uint16_t data = read_data();

			if ((m_status & IDE_STATUS_DRQ) && single_word_dma_mode() >= 0)
				set_dmarq(ASSERT_LINE);

			LOGREADDATA("%s device %d dma_r 0x%04x\n", machine().describe_context(), m_csel, data);
			return data;
		}
	}

	return 0xffff;
}

uint16_t ata_hle_device_base::command_r(offs_t offset)
{
	if (!(m_status & IDE_STATUS_DRDY) && is_ready())
		m_status |= IDE_STATUS_DRDY;

	if (device_selected() || m_single_device)
	{
		if (m_dmack)
		{
			if (!machine().side_effects_disabled())
				LOG("%s device %d cs0_r (0x%x) 0x%04x (ignored DMACK)\n", machine().describe_context(), dev(), offset, 0xffff);
		}
		else if ((m_status & IDE_STATUS_BSY) && offset != IDE_CS0_DATA_RW)
		{
			uint8_t status = device_selected() ? calculate_status() : 0;

			if (!machine().side_effects_disabled())
				LOGREADSTATUS("%s device %d cs0_r (0x%x) 0x%04x (status BSY)\n", machine().describe_context(), dev(), offset, status);

			return status;
		}
		else
		{
			switch (offset)
			{
			case IDE_CS0_DATA_RW:
				if (!device_selected())
				{
					if (!machine().side_effects_disabled())
						LOG("%s device %d cs0_r data 0x%04x (ignored !selected)\n", machine().describe_context(), dev(), 0xffff);
				}
				else if (m_status & IDE_STATUS_BSY)
				{
					if (!machine().side_effects_disabled())
						LOG("%s device %d cs0_r data 0x%04x (ignored BSY)\n", machine().describe_context(), dev(), 0xffff);
				}
				else if (!(m_status & IDE_STATUS_DRQ))
				{
					if (!machine().side_effects_disabled())
						LOG("%s device %d cs0_r data 0x%04x (ignored !DRQ)\n", machine().describe_context(), dev(), 0xffff);
				}
				else
				{
					uint16_t data = device_selected() ? read_data() : 0xffff;
					LOGREADDATA("%s device %d cs0_r data 0x%04x\n", machine().describe_context(), m_csel, data);
					return data;
				}
				break;

			case IDE_CS0_ERROR_R:
				if (!machine().side_effects_disabled())
					LOGREAD("%s device %d cs0_r error register 0x%02x\n", machine().describe_context(), dev(), m_error);

				return m_error;

			case IDE_CS0_SECTOR_COUNT_RW:
				if (!machine().side_effects_disabled())
					LOGREAD("%s device %d cs0_r sector count 0x%02x\n", machine().describe_context(), dev(), m_sector_count);

				return m_sector_count;

			case IDE_CS0_SECTOR_NUMBER_RW:
				if (!machine().side_effects_disabled())
					LOGREAD("%s device %d cs0_r sector number 0x%02x\n", machine().describe_context(), dev(), m_sector_number);

				return m_sector_number;

			case IDE_CS0_CYLINDER_LOW_RW:
				if (!machine().side_effects_disabled())
					LOGREAD("%s device %d cs0_r cylinder low 0x%02x\n", machine().describe_context(), dev(), m_cylinder_low);

				return m_cylinder_low;

			case IDE_CS0_CYLINDER_HIGH_RW:
				if (!machine().side_effects_disabled())
					LOGREAD("%s device %d cs0_r cylinder high 0x%02x\n", machine().describe_context(), dev(), m_cylinder_high);

				return m_cylinder_high;

			case IDE_CS0_DEVICE_HEAD_RW:
				if (!machine().side_effects_disabled())
					LOGREAD("%s device %d cs0_r device/head 0x%02x\n", machine().describe_context(), dev(), m_device_head);

				return m_device_head;

			case IDE_CS0_STATUS_R:
			{
				uint8_t status = device_selected() ? calculate_status() : 0;

				if (device_selected())
					set_irq(CLEAR_LINE);

				if (!machine().side_effects_disabled())
					LOGREADSTATUS("%s device %d cs0_r status 0x%04x\n", machine().describe_context(), dev(), status);

				return status;
			}

			default:
				if (!machine().side_effects_disabled())
					LOG("%s device %d cs0_r unknown (0x%x) 0x%04x\n", machine().describe_context(), dev(), offset, 0xffff);
			}
		}
	}

	return 0xffff;
}

uint16_t ata_hle_device_base::control_r(offs_t offset)
{
	if (!(m_status & IDE_STATUS_DRDY) && is_ready())
		m_status |= IDE_STATUS_DRDY;

	if (device_selected() || m_single_device)
	{
		if (m_dmack)
		{
			if (!machine().side_effects_disabled())
				LOG("%s device %d cs1_r (0x%x) 0x%0x (ignored DMACK)\n", machine().describe_context(), dev(), offset, 0xffff);

			return 0xffff;
		}
		else
		{
			switch (offset)
			{
			case IDE_CS1_ALTERNATE_STATUS_R:
			{
				uint8_t status = device_selected() ? calculate_status() : 0;

				if (!machine().side_effects_disabled())
					LOGREADSTATUS("%s device %d cs1_r status 0x%04x\n", machine().describe_context(), dev(), status);
				return status;
			}

			case IDE_CS1_ACTIVE_STATUS:
			{
				uint8_t active_status = device_selected() ? 1 : 0;

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

				if (!machine().side_effects_disabled())
					LOGREAD("%s device %d cs1_r active status 0x%04x\n", machine().describe_context(), dev(), active_status);

				return active_status;
			}

			default:
				if (!machine().side_effects_disabled())
					LOG("%s device %d cs1_r unknown (0x%x) 0x%04x\n", machine().describe_context(), dev(), offset, 0xffff);

				return 0xffff;
			}
		}
	}

	return 0xffff;
}

void ata_hle_device_base::dma_w(uint16_t data)
{
	if (device_selected())
	{
		if (!m_dmack)
			LOG("%s device %d dma_w 0x%04x (ignored !DMACK)\n", machine().describe_context(), m_csel, data);
		else if (m_dmarq && single_word_dma_mode() >= 0)
			LOG("%s device %d dma_w 0x%04x (ignored DMARQ)\n", machine().describe_context(), m_csel, data);
		else if (!m_dmarq && multi_word_dma_mode() >= 0)
			LOG("%s device %d dma_w 0x%04x (ignored !DMARQ)\n", machine().describe_context(), m_csel, data);
		else if (!m_dmarq && ultra_dma_mode() >= 0)
			LOG("%s device %d dma_w 0x%04x (ignored !DMARQ)\n", machine().describe_context(), m_csel, data);
		else if (m_status & IDE_STATUS_BSY)
			LOG("%s device %d dma_w 0x%04x (ignored BSY)\n", machine().describe_context(), m_csel, data);
		else if (!(m_status & IDE_STATUS_DRQ))
			LOG("%s device %d dma_w 0x%04x (ignored !DRQ)\n", machine().describe_context(), m_csel, data);
		else
		{
			write_data(data);

			if ((m_status & IDE_STATUS_DRQ) && single_word_dma_mode() >= 0)
				set_dmarq(ASSERT_LINE);
		}
	}
}

void ata_hle_device_base::command_w(offs_t offset, uint16_t data)
{
	if ((m_status & IDE_STATUS_DRQ) && offset != IDE_CS0_DATA_RW && (offset != IDE_CS0_DEVICE_HEAD_RW || data != m_device_head))
	{
		LOG("%s device %d cs0_w (0x%x) aborted after %d bytes (%02x)\n", machine().describe_context(), m_csel, offset, m_buffer_offset, m_command);
		stop_busy();
		m_status &= ~IDE_STATUS_DRQ;
		set_dmarq(CLEAR_LINE);
		m_status |= IDE_STATUS_ERR;
		m_error = IDE_ERROR_ABRT;
	}

	if (m_status & IDE_STATUS_BSY)
		LOG("%s device %d cs0_w (0x%x) 0x%04x (ignored BSY)\n", machine().describe_context(), m_csel, offset, data);
	else if (m_dmack)
		LOG("%s device %d cs0_w (0x%x) 0x%04x (ignored DMACK)\n", machine().describe_context(), m_csel, offset, data);
	else
	{
		switch (offset)
		{
		case IDE_CS0_DATA_RW:
			if (device_selected() || m_single_device)
			{
				if (!device_selected())
					LOG("%s device %d cs0_w data 0x%04x (ignored !selected)\n", machine().describe_context(), m_csel, data);
				else if (!(m_status & IDE_STATUS_DRQ))
					LOG("%s device %d cs0_w data 0x%04x (ignored !DRQ)\n", machine().describe_context(), m_csel, data);
				else
				{
					LOGWRITEDATA("%s device %d cs0_w data 0x%04x\n", machine().describe_context(), m_csel, data);
					write_data(data);
				}
			}
			break;

		case IDE_CS0_FEATURE_W:
			LOGWRITE("%s device %d cs0_w feature 0x%02x\n", machine().describe_context(), m_csel, data);
			m_feature = data;
			break;

		case IDE_CS0_SECTOR_COUNT_RW:
			LOGWRITE("%s device %d cs0_w sector count 0x%02x\n", machine().describe_context(), m_csel, data);
			m_sector_count = (data & 0xff) ? (data & 0xff) : 0x100;
			break;

		case IDE_CS0_SECTOR_NUMBER_RW:
			LOGWRITE("%s device %d cs0_w sector number 0x%02x\n", machine().describe_context(), m_csel, data);
			m_sector_number = data;
			break;

		case IDE_CS0_CYLINDER_LOW_RW:
			LOGWRITE("%s device %d cs0_w cylinder low 0x%02x\n", machine().describe_context(), m_csel, data);
			m_cylinder_low = data;
			break;

		case IDE_CS0_CYLINDER_HIGH_RW:
			LOGWRITE("%s device %d cs0_w cylinder high 0x%02x\n", machine().describe_context(), m_csel, data);
			m_cylinder_high = data;
			break;

		case IDE_CS0_DEVICE_HEAD_RW:
		{
			LOGWRITE("%s device %d cs0_w device/head 0x%02x\n", machine().describe_context(), m_csel, data);

			bool drv_changed = ((m_device_head ^ data) & IDE_DEVICE_HEAD_DRV) != 0;
			m_device_head = data;

			if (drv_changed)
				update_irq();
			break;
		}

		case IDE_CS0_COMMAND_W:
			if (device_selected() || m_command == IDE_COMMAND_DIAGNOSTIC)
			{
				LOGWRITE("%s device %d cs0_w command 0x%02x\n", machine().describe_context(), m_csel, data);

				m_command = data;
				m_buffer_offset = 0;
				m_status &= ~IDE_STATUS_ERR;
				m_error = IDE_ERROR_NONE;

				set_irq(CLEAR_LINE);
				set_dasp(CLEAR_LINE);

				process_command();
			}
			break;

		default:
			LOG("%s device %d cs0_w unknown (0x%x) 0x%02x\n", machine().describe_context(), m_csel, offset, data);
			break;
		}
	}
}

void ata_hle_device_base::control_w(offs_t offset, uint16_t data)
{
	if (m_dmack)
		LOG("%s device %d cs1_w (0x%x) 0x%04x (ignored DMACK)\n", machine().describe_context(), m_csel, offset, data);
	else
	{
		switch (offset)
		{
		case IDE_CS1_DEVICE_CONTROL_W:
		{
			LOGWRITE("%s device %d cs1_w device control 0x%02x\n", machine().describe_context(), m_csel, data);

			uint8_t old = m_device_control;
			m_device_control = data;

			if ((m_device_control ^ old) & IDE_DEVICE_CONTROL_NIEN)
				update_irq();

			if ((m_device_control ^ old) & IDE_DEVICE_CONTROL_SRST)
			{
				if (m_device_control & IDE_DEVICE_CONTROL_SRST)
				{
					if (m_resetting)
						LOG("%s device %d cs1_w (0x%x) %04x (ignored RESET)\n", machine().describe_context(), m_csel, offset, data);
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
					soft_reset();
			}
			break;
		}

		default:
			LOG("%s device %d cs1_w unknown (0x%x) 0x%02x\n", machine().describe_context(), m_csel, offset, data);
			break;
		}
	}
}
