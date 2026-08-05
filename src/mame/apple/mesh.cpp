// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple Macintosh Enhanced SCSI Handler (343S1146)
    Emulation by R. Belmont
    Portions based on ncr53c90.cpp by Olivier Galibert

    MESH is a heavily customized (but recognizable) 539x-derived SCSI controller.
    It's found primarily in PCI-era macio ASICs (O'Hare, Heathrow, and Paddington),
    but there was also a discrete version found in the Pippin and some other early
    PCI PowerMacs.
*/

#include "emu.h"
#include "mesh.h"

#define LOG_FIFO (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(APPLE_MESH_SCSI, mesh_device, "mesh", "Apple MESH SCSI Controller")

ALLOW_SAVE_TYPE(mesh_device::sequence_state);

mesh_device::mesh_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, APPLE_MESH_SCSI, tag, owner, clock)
	, nscsi_device_interface(mconfig, *this)
	, m_xfer_count(0)
	, m_xfer_started(false)
	, m_fifo_count(0)
	, m_sequence(0)
	, m_exception(0)
	, m_error(0)
	, m_interrupt_mask(0)
	, m_cmd_done(false)
	, m_source_id(7)
	, m_dest_id(0)
	, m_sync_params(0)
	, m_mesh_id(0xe2)
	, m_selection_timeout(0)
	, m_timer(nullptr)
	, m_state(sequence_state::IDLE)
	, m_expected_phase(0)
	, m_stepping(0)
	, m_irq(0)
	, m_cmd_done_line(0)
	, m_error_line(0)
	, m_exception_line(0)
	, m_drq(0)
	, m_irq_handler(*this)
	, m_cmd_done_handler(*this)
	, m_error_handler(*this)
	, m_exception_handler(*this)
	, m_drq_handler(*this)
{
}

void mesh_device::map(address_map &map)
{
	map(REG_XFER_COUNT_0, REG_XFER_COUNT_0).rw(FUNC(mesh_device::xfer_count_0_r), FUNC(mesh_device::xfer_count_0_w));
	map(REG_XFER_COUNT_1, REG_XFER_COUNT_1).rw(FUNC(mesh_device::xfer_count_1_r), FUNC(mesh_device::xfer_count_1_w));
	map(REG_FIFO, REG_FIFO).rw(FUNC(mesh_device::fifo_r), FUNC(mesh_device::fifo_w));
	map(REG_SEQUENCE, REG_SEQUENCE).rw(FUNC(mesh_device::sequence_r), FUNC(mesh_device::sequence_w));
	map(REG_BUS_STATUS_0, REG_BUS_STATUS_0).rw(FUNC(mesh_device::bus_status_0_r), FUNC(mesh_device::bus_status_0_w));
	map(REG_BUS_STATUS_1, REG_BUS_STATUS_1).rw(FUNC(mesh_device::bus_status_1_r), FUNC(mesh_device::bus_status_1_w));
	map(REG_FIFO_COUNT, REG_FIFO_COUNT).r(FUNC(mesh_device::fifo_count_r));
	map(REG_EXCEPTION, REG_EXCEPTION).rw(FUNC(mesh_device::exception_r), FUNC(mesh_device::exception_w));
	map(REG_ERROR, REG_ERROR).rw(FUNC(mesh_device::error_r), FUNC(mesh_device::error_w));
	map(REG_INT_MASK, REG_INT_MASK).rw(FUNC(mesh_device::interrupt_mask_r), FUNC(mesh_device::interrupt_mask_w));
	map(REG_INTERRUPT, REG_INTERRUPT).rw(FUNC(mesh_device::interrupt_r), FUNC(mesh_device::interrupt_w));
	map(REG_SOURCE_ID, REG_SOURCE_ID).rw(FUNC(mesh_device::source_id_r), FUNC(mesh_device::source_id_w));
	map(REG_DEST_ID, REG_DEST_ID).rw(FUNC(mesh_device::dest_id_r), FUNC(mesh_device::dest_id_w));
	map(REG_SYNC_PARAMS, REG_SYNC_PARAMS).rw(FUNC(mesh_device::sync_params_r), FUNC(mesh_device::sync_params_w));
	map(REG_MESH_ID, REG_MESH_ID).r(FUNC(mesh_device::mesh_id_r));
	map(REG_SEL_TIMEOUT, REG_SEL_TIMEOUT).rw(FUNC(mesh_device::selection_timeout_r), FUNC(mesh_device::selection_timeout_w));
}

void mesh_device::device_start()
{
	m_timer = timer_alloc(FUNC(mesh_device::sequence_timer), this);

	save_item(NAME(m_xfer_count));
	save_item(NAME(m_xfer_started));
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_count));
	save_item(NAME(m_sequence));
	save_item(NAME(m_exception));
	save_item(NAME(m_error));
	save_item(NAME(m_interrupt_mask));
	save_item(NAME(m_cmd_done));
	save_item(NAME(m_source_id));
	save_item(NAME(m_dest_id));
	save_item(NAME(m_sync_params));
	save_item(NAME(m_mesh_id));
	save_item(NAME(m_selection_timeout));
	save_item(NAME(m_state));
	save_item(NAME(m_expected_phase));
	save_item(NAME(m_stepping));
	save_item(NAME(m_irq));
	save_item(NAME(m_cmd_done_line));
	save_item(NAME(m_error_line));
	save_item(NAME(m_exception_line));
	save_item(NAME(m_drq));
}

void mesh_device::device_reset()
{
	reset_sequence_state();
	fifo_clear();
	m_exception = 0;
	m_error = 0;
	m_interrupt_mask = 0;
	m_cmd_done = true;
	m_xfer_started = false;

	m_scsi_id = m_source_id & 7;
	m_scsi_bus->ctrl_wait(m_scsi_refid, S_SEL | S_BSY | S_RST, S_ALL);
	m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ALL);
	m_scsi_bus->data_w(m_scsi_refid, 0);

	update_lines();
	update_drq();
}

void mesh_device::scsi_ctrl_changed()
{
	if (m_scsi_bus->ctrl_r() & S_RST)
	{
		m_error |= ERR_SCSI_RST;
		update_lines();
	}

	step(false);
}

u8 mesh_device::read(offs_t offset)
{
	u8 data;

	switch (offset & 0x0f)
	{
	case REG_XFER_COUNT_0: data = xfer_count_0_r(); break;
	case REG_XFER_COUNT_1: data = xfer_count_1_r(); break;
	case REG_FIFO:         data = fifo_r(); break;
	case REG_SEQUENCE:     data = sequence_r(); break;
	case REG_BUS_STATUS_0: data = bus_status_0_r(); break;
	case REG_BUS_STATUS_1: data = bus_status_1_r(); break;
	case REG_FIFO_COUNT:   data = fifo_count_r(); break;
	case REG_EXCEPTION:    data = exception_r(); break;
	case REG_ERROR:        data = error_r(); break;
	case REG_INT_MASK:     data = interrupt_mask_r(); break;
	case REG_INTERRUPT:    data = interrupt_r(); break;
	case REG_SOURCE_ID:    data = source_id_r(); break;
	case REG_DEST_ID:      data = dest_id_r(); break;
	case REG_SYNC_PARAMS:  data = sync_params_r(); break;
	case REG_MESH_ID:      data = mesh_id_r(); break;
	case REG_SEL_TIMEOUT:  data = selection_timeout_r(); break;
	default:               data = 0xff; break;
	}

	LOG("Read MESH @ %x = %02x\n", offset, data);
	return data;
}

void mesh_device::write(offs_t offset, u8 data)
{
	LOG("Write %02x to MESH at %x\n", data, offset);
	switch (offset & 0x0f)
	{
	case REG_XFER_COUNT_0: xfer_count_0_w(data); break;
	case REG_XFER_COUNT_1: xfer_count_1_w(data); break;
	case REG_FIFO:         fifo_w(data); break;
	case REG_SEQUENCE:     sequence_w(data); break;
	case REG_BUS_STATUS_0: bus_status_0_w(data); break;
	case REG_BUS_STATUS_1: bus_status_1_w(data); break;
	case REG_EXCEPTION:    exception_w(data); break;
	case REG_ERROR:        error_w(data); break;
	case REG_INT_MASK:     interrupt_mask_w(data); break;
	case REG_INTERRUPT:    interrupt_w(data); break;
	case REG_SOURCE_ID:    source_id_w(data); break;
	case REG_DEST_ID:      dest_id_w(data); break;
	case REG_SYNC_PARAMS:  sync_params_w(data); break;
	case REG_SEL_TIMEOUT:  selection_timeout_w(data); break;
	default:               break;
	}
}

u8 mesh_device::xfer_count_0_r()
{
	return m_xfer_count & 0xff;
}

void mesh_device::xfer_count_0_w(u8 data)
{
	m_xfer_count = (m_xfer_count & 0xff00) | data;
	m_xfer_started = false;
	update_drq();
}

u8 mesh_device::xfer_count_1_r()
{
	return (m_xfer_count >> 8) & 0xff;
}

void mesh_device::xfer_count_1_w(u8 data)
{
	m_xfer_count = (m_xfer_count & 0x00ff) | (u32(data) << 8);
	m_xfer_started = false;
	update_drq();
}

u8 mesh_device::fifo_r()
{
	if (m_sequence & SEQ_DMA)
	{
		LOGMASKED(LOG_FIFO, "fifo_r disabled during DMA (%s)\n", machine().describe_context());
		return 0;
	}

	u8 const data = fifo_pop();
	LOGMASKED(LOG_FIFO, "fifo_r %02x fifo_count %u (%s)\n", data, unsigned(m_fifo_count), machine().describe_context());
	step(false);
	return data;
}

void mesh_device::fifo_w(u8 data)
{
	if (m_sequence & SEQ_DMA)
	{
		LOGMASKED(LOG_FIFO, "fifo_w %02x disabled during DMA (%s)\n", data, machine().describe_context());
		return;
	}

	LOGMASKED(LOG_FIFO, "fifo_w %02x fifo_count %u xfer_count %u (%s)\n", data, unsigned(m_fifo_count), unsigned(m_xfer_count), machine().describe_context());
	fifo_push(data);

	if (transfer_to_scsi())
	{
		decrement_xfer_count(1);
	}

	update_drq();
	step(false);
}

u8 mesh_device::sequence_r()
{
	return m_sequence;
}

void mesh_device::sequence_w(u8 data)
{
	LOG("sequence_w %02x (%s)\n", data, machine().describe_context());

	if (m_exception || m_error)
	{
		m_error |= ERR_SEQ;
		update_lines();
		return;
	}

	start_sequence(data);
}

TIMER_CALLBACK_MEMBER(mesh_device::sequence_timer)
{
	step(true);
}

void mesh_device::start_sequence(u8 data)
{
	m_timer->reset();
	m_sequence = data;
	m_xfer_started = false;
	m_cmd_done = false;
	m_expected_phase = 0;
	update_lines();
	update_drq();

	switch (m_sequence & SEQ_CMD_MASK)
	{
	case SEQ_ARBITRATE:
		m_state = sequence_state::ARB_WAIT_FREE;
		step(false);
		break;

	case SEQ_SELECT:
		m_scsi_bus->data_w(m_scsi_refid, (1 << m_source_id) | (1 << m_dest_id));
		m_scsi_bus->ctrl_w(m_scsi_refid, (m_sequence & SEQ_ATN) ? (S_SEL | S_ATN) : S_SEL, S_SEL | S_ATN | S_BSY);
		m_state = sequence_state::SELECT_WAIT_BSY;
		step(false);
		break;

	case SEQ_COMMAND:
	case SEQ_STATUS:
	case SEQ_DATA_OUT:
	case SEQ_DATA_IN:
	case SEQ_MSG_OUT:
	case SEQ_MSG_IN:
		m_expected_phase = phase_for_sequence();
		m_state = sequence_state::XFER_WAIT_REQ;
		m_scsi_bus->data_w(m_scsi_refid, 0);
		m_scsi_bus->ctrl_wait(m_scsi_refid, S_REQ, S_REQ);
		m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ACK);
		step(false);
		break;

	case SEQ_BUS_FREE:
		m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ACK | S_REQ | S_BSY | S_SEL | S_ATN);
		m_scsi_bus->data_w(m_scsi_refid, 0);
		m_state = sequence_state::BUS_FREE_WAIT;
		step(false);
		break;

	case SEQ_EN_PARITY:
	case SEQ_DIS_PARITY:
	case SEQ_EN_RESEL:
	case SEQ_DIS_RESEL:
		command_done();
		break;

	case SEQ_RESET:
		device_reset();
		break;

	case SEQ_FLUSH_FIFO:
		fifo_clear();
		command_done();
		break;

	default:
		finish_error(ERR_SEQ);
		break;
	}
}

void mesh_device::step(bool timeout)
{
	if (m_stepping)
	{
		return;
	}

	m_stepping++;

	u32 const ctrl = m_scsi_bus->ctrl_r();
	u32 const data = m_scsi_bus->data_r();

	switch (m_state)
	{
	case sequence_state::IDLE:
		break;

	case sequence_state::ARB_WAIT_FREE:
		if (ctrl & (S_BSY | S_SEL))
		{
			m_scsi_bus->ctrl_wait(m_scsi_refid, S_BSY | S_SEL, S_BSY | S_SEL);
		}
		else
		{
			m_state = sequence_state::ARB_ASSERT_BUS;
			schedule(attotime::from_nsec(1200));
		}
		break;

	case sequence_state::ARB_ASSERT_BUS:
		if (timeout)
		{
			m_scsi_bus->data_w(m_scsi_refid, 1 << m_source_id);
			m_scsi_bus->ctrl_w(m_scsi_refid, S_BSY, S_BSY);
			m_state = sequence_state::ARB_CHECK_WIN;
			schedule(attotime::from_nsec(2400));
		}
		break;

	case sequence_state::ARB_CHECK_WIN:
		if (timeout)
		{
			int winner = -1;
			for (int bit = 7; bit >= 0; bit--)
			{
				if (data & (1 << bit))
				{
					winner = bit;
					break;
				}
			}

			if (winner != m_source_id || (ctrl & S_SEL))
			{
				m_scsi_bus->data_w(m_scsi_refid, 0);
				m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_BSY);
				finish_exception(EXC_ARB_LOST);
			}
			else
			{
				command_done();
			}
		}
		break;

	case sequence_state::SELECT_WAIT_BSY:
		if (ctrl & S_BSY)
		{
			m_scsi_bus->data_w(m_scsi_refid, 0);
			m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_SEL);
			command_done();
		}
		else if (timeout)
		{
			m_scsi_bus->data_w(m_scsi_refid, 0);
			m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_SEL | S_ATN);
			finish_exception(EXC_SEL_TO);
		}
		else
		{
			m_scsi_bus->ctrl_wait(m_scsi_refid, S_BSY, S_BSY);
			schedule(attotime::from_msec(active_selection_timeout() * 10));
		}
		break;

	case sequence_state::BUS_FREE_WAIT:
		if (!(ctrl & (S_BSY | S_SEL)))
		{
			command_done();
		}
		else if (ctrl & S_REQ)
		{
			finish_exception(EXC_PHASE_MM);
		}
		else
		{
			m_scsi_bus->ctrl_wait(m_scsi_refid, S_BSY | S_SEL | S_REQ, S_BSY | S_SEL | S_REQ);
		}
		break;

	case sequence_state::XFER_WAIT_REQ:
	case sequence_state::XFER_WAIT_REQ_FALSE:
		do_information_transfer();
		break;
	}

	m_stepping--;
}

void mesh_device::schedule(attotime delay)
{
	m_timer->adjust(delay);
}

void mesh_device::command_done()
{
	m_timer->reset();
	m_state = sequence_state::IDLE;
	m_cmd_done = true;
	update_lines();
	update_drq();
}

void mesh_device::finish_exception(u8 mask)
{
	m_timer->reset();
	m_state = sequence_state::IDLE;
	m_exception |= mask;
	m_cmd_done = false;
	update_lines();
	update_drq();
}

void mesh_device::finish_error(u8 mask)
{
	m_timer->reset();
	m_state = sequence_state::IDLE;
	m_error |= mask;
	m_cmd_done = false;
	update_lines();
	update_drq();
}

void mesh_device::do_information_transfer()
{
	if (m_sequence & SEQ_TARGET)
	{
		finish_error(ERR_SEQ);
		return;
	}

	if (transfer_from_scsi() && xfer_count_done())
	{
		if (!data_in_fifo_pending())
		{
			command_done();
		}
		else
		{
			update_drq();
		}
		return;
	}

	for (;;)
	{
		u32 const ctrl = m_scsi_bus->ctrl_r();

		if (m_state == sequence_state::XFER_WAIT_REQ_FALSE)
		{
			if (ctrl & S_REQ)
			{
				m_scsi_bus->ctrl_wait(m_scsi_refid, S_REQ, S_REQ);
				return;
			}

			m_state = sequence_state::XFER_WAIT_REQ;
			m_scsi_bus->ctrl_wait(m_scsi_refid, S_REQ, S_REQ);
			m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ACK);
			m_scsi_bus->data_w(m_scsi_refid, 0);
			continue;
		}

		if (transfer_to_scsi())
		{
			if (m_fifo_count == 0 && xfer_count_done())
			{
				command_done();
				return;
			}

			if (m_fifo_count == 0)
			{
				update_drq();
				return;
			}
		}
		else if (transfer_from_scsi())
		{
			if (xfer_count_done())
			{
				if (!data_in_fifo_pending())
				{
					command_done();
				}
				else
				{
					update_drq();
				}
				return;
			}

			if (m_fifo_count >= std::size(m_fifo))
			{
				update_drq();
				return;
			}
		}
		else
		{
			finish_error(ERR_SEQ);
			return;
		}

		if (!(ctrl & S_REQ))
		{
			m_scsi_bus->ctrl_wait(m_scsi_refid, S_REQ, S_REQ);
			return;
		}

		if ((ctrl & S_PHASE_MASK) != m_expected_phase)
		{
			finish_exception(EXC_PHASE_MM);
			return;
		}

		if (transfer_to_scsi())
		{
			m_scsi_bus->data_w(m_scsi_refid, fifo_pop());
		}
		else
		{
			fifo_push_from_scsi(m_scsi_bus->data_r() & 0xff);
		}

		m_state = sequence_state::XFER_WAIT_REQ_FALSE;
		m_scsi_bus->ctrl_wait(m_scsi_refid, S_REQ, S_REQ);
		m_scsi_bus->ctrl_w(m_scsi_refid, S_ACK, S_ACK);
		update_drq();

		if (xfer_count_done())
		{
			if (!data_in_fifo_pending())
			{
				command_done();
			}
			else
			{
				update_drq();
			}
			return;
		}
	}
}

bool mesh_device::xfer_count_done() const
{
	if (m_xfer_count != 0)
	{
		return false;
	}

	return m_xfer_started || !transfer_from_scsi();
}

u32 mesh_device::phase_for_sequence() const
{
	switch (m_sequence & SEQ_CMD_MASK)
	{
	case SEQ_COMMAND:  return S_PHASE_COMMAND;
	case SEQ_STATUS:   return S_PHASE_STATUS;
	case SEQ_DATA_OUT: return S_PHASE_DATA_OUT;
	case SEQ_DATA_IN:  return S_PHASE_DATA_IN;
	case SEQ_MSG_OUT:  return S_PHASE_MSG_OUT;
	case SEQ_MSG_IN:   return S_PHASE_MSG_IN;
	default:           return 0;
	}
}

u32 mesh_device::active_selection_timeout() const
{
	return m_selection_timeout ? m_selection_timeout : 25;
}

u8 mesh_device::bus_status_0_r()
{
	return bus_status_0_value();
}

void mesh_device::bus_status_0_w(u8 data)
{
	if (m_exception)
	{
		return;
	}

	write_bus_status_0(data);
}

u8 mesh_device::bus_status_1_r()
{
	return bus_status_1_value();
}

void mesh_device::bus_status_1_w(u8 data)
{
	if (m_exception)
	{
		return;
	}

	write_bus_status_1(data);
}

u8 mesh_device::fifo_count_r()
{
	return m_fifo_count;
}

u8 mesh_device::exception_r()
{
	return m_exception;
}

void mesh_device::exception_w(u8 data)
{
	m_exception &= ~data;
	update_lines();
}

u8 mesh_device::error_r()
{
	return m_error;
}

void mesh_device::error_w(u8 data)
{
	m_error &= ~data;
	update_lines();
}

u8 mesh_device::interrupt_mask_r()
{
	return m_interrupt_mask;
}

void mesh_device::interrupt_mask_w(u8 data)
{
	m_interrupt_mask = data & (INT_ERROR | INT_EXCEPTION | INT_CMD_DONE);
	update_lines();
}

u8 mesh_device::interrupt_r()
{
	return interrupt_value();
}

void mesh_device::interrupt_w(u8 data)
{
	if (data & INT_ERROR)
	{
		m_error = 0;
	}
	if (data & INT_EXCEPTION)
	{
		m_exception = 0;
	}
	if (data & INT_CMD_DONE)
	{
		m_cmd_done = false;
	}

	update_lines();
}

u8 mesh_device::source_id_r()
{
	return m_source_id;
}

void mesh_device::source_id_w(u8 data)
{
	m_source_id = data & 7;
	m_scsi_id = m_source_id;
}

u8 mesh_device::dest_id_r()
{
	return m_dest_id;
}

void mesh_device::dest_id_w(u8 data)
{
	m_dest_id = data & 7;
}

u8 mesh_device::sync_params_r()
{
	return m_sync_params;
}

void mesh_device::sync_params_w(u8 data)
{
	m_sync_params = data;
}

u8 mesh_device::mesh_id_r()
{
	return m_mesh_id;
}

u8 mesh_device::selection_timeout_r()
{
	return m_selection_timeout;
}

void mesh_device::selection_timeout_w(u8 data)
{
	m_selection_timeout = data;
}

u8 mesh_device::dma8_r()
{
	if (!(m_sequence & SEQ_DMA) || !transfer_from_scsi())
	{
		LOGMASKED(LOG_FIFO, "dma8_r disabled sequence %02x (%s)\n", m_sequence, machine().describe_context());
		return 0xff;
	}

	if (machine().side_effects_disabled())
	{
		return m_fifo_count ? m_fifo[0] : 0xff;
	}

	u8 data = 0xff;
	if (m_fifo_count)
	{
		data = fifo_pop();
		if (sync_data_in())
		{
			decrement_xfer_count(1);
		}
	}

	update_drq();
	LOGMASKED(LOG_FIFO, "dma8_r %02x fifo_count %u xfer_count %u (%s)\n", data, unsigned(m_fifo_count), unsigned(m_xfer_count), machine().describe_context());
	step(false);
	return data;
}

void mesh_device::dma8_w(u8 data)
{
	if (!(m_sequence & SEQ_DMA) || !transfer_to_scsi())
	{
		LOGMASKED(LOG_FIFO, "dma8_w %02x disabled sequence %02x (%s)\n", data, m_sequence, machine().describe_context());
		return;
	}

	if (m_xfer_started && m_xfer_count == 0)
	{
		LOGMASKED(LOG_FIFO, "dma8_w %02x ignored after terminal count (%s)\n", data, machine().describe_context());
		return;
	}

	if (m_fifo_count < std::size(m_fifo))
	{
		fifo_push(data);
		decrement_xfer_count(1);
	}

	update_drq();
	LOGMASKED(LOG_FIFO, "dma8_w %02x fifo_count %u xfer_count %u (%s)\n", data, unsigned(m_fifo_count), unsigned(m_xfer_count), machine().describe_context());
	step(false);
}

u16 mesh_device::dma16_r()
{
	if (!(m_sequence & SEQ_DMA) || !transfer_from_scsi())
	{
		LOGMASKED(LOG_FIFO, "dma16_r disabled sequence %02x (%s)\n", m_sequence, machine().describe_context());
		return 0xffff;
	}

	if (machine().side_effects_disabled())
	{
		if (m_fifo_count > 1)
		{
			return m_fifo[0] | (m_fifo[1] << 8);
		}

		return m_fifo_count ? (m_fifo[0] | 0xff00) : 0xffff;
	}

	u16 data = 0xffff;
	u8 count = 0;

	if (m_fifo_count)
	{
		data = (data & 0xff00) | fifo_pop();
		count++;
	}

	if (m_fifo_count)
	{
		data = (data & 0x00ff) | (u16(fifo_pop()) << 8);
		count++;
	}

	if (sync_data_in())
	{
		decrement_xfer_count(count);
	}

	update_drq();
	LOGMASKED(LOG_FIFO, "dma16_r %04x fifo_count %u xfer_count %u (%s)\n", data, unsigned(m_fifo_count), unsigned(m_xfer_count), machine().describe_context());
	step(false);
	return data;
}

void mesh_device::dma16_w(u16 data)
{
	if (!(m_sequence & SEQ_DMA) || !transfer_to_scsi())
	{
		LOGMASKED(LOG_FIFO, "dma16_w %04x disabled sequence %02x (%s)\n", data, m_sequence, machine().describe_context());
		return;
	}

	if (m_xfer_started && m_xfer_count == 0)
	{
		LOGMASKED(LOG_FIFO, "dma16_w %04x ignored after terminal count (%s)\n", data, machine().describe_context());
		return;
	}

	LOGMASKED(LOG_FIFO, "dma16_w %04x fifo_count %u xfer_count %u (%s)\n", data, unsigned(m_fifo_count), unsigned(m_xfer_count), machine().describe_context());
	if (m_fifo_count < std::size(m_fifo))
	{
		fifo_push(data & 0xff);
		decrement_xfer_count(1);
	}

	if (m_fifo_count < std::size(m_fifo) && m_xfer_count != 0)
	{
		fifo_push(data >> 8);
		decrement_xfer_count(1);
	}

	update_drq();
	step(false);
}

u8 mesh_device::interrupt_value() const
{
	return (m_error ? INT_ERROR : 0)
		| (m_exception ? INT_EXCEPTION : 0)
		| (m_cmd_done ? INT_CMD_DONE : 0);
}

void mesh_device::update_lines()
{
	int const irq = (interrupt_value() & m_interrupt_mask) != 0;
	int const cmd_done = m_cmd_done;
	int const error = m_error != 0;
	int const exception = m_exception != 0;

	if (irq != m_irq)
	{
		m_irq = irq;
		m_irq_handler(m_irq);
	}

	if (cmd_done != m_cmd_done_line)
	{
		m_cmd_done_line = cmd_done;
		m_cmd_done_handler(m_cmd_done_line);
	}

	if (error != m_error_line)
	{
		m_error_line = error;
		m_error_handler(m_error_line);
	}

	if (exception != m_exception_line)
	{
		m_exception_line = exception;
		m_exception_handler(m_exception_line);
	}
}

void mesh_device::update_drq()
{
	bool drq = false;

	if (m_sequence & SEQ_DMA)
	{
		if (transfer_to_scsi())
		{
			drq = (m_fifo_count < std::size(m_fifo)) && ((m_xfer_count != 0) || !m_xfer_started);
		}
		else if (transfer_from_scsi())
		{
			drq = m_fifo_count >= 2 || (m_fifo_count && m_xfer_count <= 1 && m_xfer_started);
		}
	}

	if (drq != m_drq)
	{
		m_drq = drq;
		m_drq_handler(m_drq);
	}
}

u8 mesh_device::bus_status_0_value() const
{
	u32 const ctrl = m_scsi_bus->ctrl_r();
	u8 data = 0;

	if (ctrl & S_REQ)
	{
		data |= BUS0_REQ;
	}
	if (ctrl & S_ACK)
	{
		data |= BUS0_ACK;
	}
	if (ctrl & S_ATN)
	{
		data |= BUS0_ATN;
	}
	if (ctrl & S_CTL)
	{
		data |= BUS0_CD;
	}
	if (ctrl & S_MSG)
	{
		data |= BUS0_MSG;
	}
	if (ctrl & S_INP)
	{
		data |= BUS0_IO;
	}

	return data;
}

u8 mesh_device::bus_status_1_value() const
{
	u32 const ctrl = m_scsi_bus->ctrl_r();
	u8 data = 0;

	if (ctrl & S_RST)
	{
		data |= BUS1_RST;
	}
	if (ctrl & S_BSY)
	{
		data |= BUS1_BSY;
	}
	if (ctrl & S_SEL)
	{
		data |= BUS1_SEL;
	}

	return data;
}

void mesh_device::write_bus_status_0(u8 data)
{
	u32 lines = 0;

	if (data & BUS0_REQ)
	{
		lines |= S_REQ;
	}
	if (data & BUS0_ACK)
	{
		lines |= S_ACK;
	}
	if (data & BUS0_ATN)
	{
		lines |= S_ATN;
	}
	if (data & BUS0_CD)
	{
		lines |= S_CTL;
	}
	if (data & BUS0_MSG)
	{
		lines |= S_MSG;
	}
	if (data & BUS0_IO)
	{
		lines |= S_INP;
	}

	m_scsi_bus->ctrl_w(m_scsi_refid, lines, S_REQ | S_ACK | S_ATN | S_CTL | S_MSG | S_INP);
}

void mesh_device::write_bus_status_1(u8 data)
{
	u32 lines = 0;

	if (data & BUS1_RST)
	{
		lines |= S_RST;
	}
	if (data & BUS1_BSY)
	{
		lines |= S_BSY;
	}
	if (data & BUS1_SEL)
	{
		lines |= S_SEL;
	}

	m_scsi_bus->ctrl_w(m_scsi_refid, lines, S_RST | S_BSY | S_SEL);
}

void mesh_device::reset_sequence_state()
{
	if (m_timer)
	{
		m_timer->reset();
	}

	m_sequence = 0;
	m_xfer_started = false;
	m_state = sequence_state::IDLE;
	m_expected_phase = 0;
	update_drq();
}

void mesh_device::fifo_clear()
{
	LOGMASKED(LOG_FIFO, "fifo_clear fifo_count %u (%s)\n", unsigned(m_fifo_count), machine().describe_context());
	m_fifo_count = 0;
	memset(m_fifo, 0, sizeof(m_fifo));
	update_drq();
}

void mesh_device::fifo_push(u8 data)
{
	if (m_fifo_count < std::size(m_fifo))
	{
		LOGMASKED(LOG_FIFO, "fifo_push %02x at %u\n", data, unsigned(m_fifo_count));
		m_fifo[m_fifo_count++] = data;
	}
	else
	{
		LOGMASKED(LOG_FIFO, "fifo_push %02x ignored, FIFO full\n", data);
	}
}

void mesh_device::fifo_push_from_scsi(u8 data)
{
	LOGMASKED(LOG_FIFO, "fifo_push_from_scsi %02x fifo_count %u xfer_count %u\n", data, unsigned(m_fifo_count), unsigned(m_xfer_count));
	fifo_push(data);

	if (transfer_from_scsi() && !sync_data_in())
	{
		decrement_xfer_count(1);
	}

	update_drq();
}

u8 mesh_device::fifo_pop()
{
	u8 data = 0;

	if (m_fifo_count)
	{
		data = m_fifo[0];
		m_fifo_count--;
		memmove(m_fifo, m_fifo + 1, m_fifo_count);
		LOGMASKED(LOG_FIFO, "fifo_pop %02x fifo_count %u\n", data, unsigned(m_fifo_count));
	}
	else
	{
		LOGMASKED(LOG_FIFO, "fifo_pop empty\n");
	}

	update_drq();
	return data;
}

void mesh_device::decrement_xfer_count(u32 count)
{
	if (!is_data_xfer() || count == 0)
	{
		return;
	}

	if (!m_xfer_started)
	{
		m_xfer_started = true;

		if (m_xfer_count == 0)
		{
			m_xfer_count = 0x10000;
		}
	}

	if (count >= m_xfer_count)
	{
		m_xfer_count = 0;
	}
	else
	{
		m_xfer_count -= count;
	}

	update_drq();
}

bool mesh_device::is_data_xfer() const
{
	switch (m_sequence & SEQ_CMD_MASK)
	{
	case SEQ_COMMAND:
	case SEQ_STATUS:
	case SEQ_DATA_OUT:
	case SEQ_DATA_IN:
	case SEQ_MSG_OUT:
	case SEQ_MSG_IN:
		return true;
	default:
		return false;
	}
}

bool mesh_device::transfer_to_scsi() const
{
	bool const target = bool(m_sequence & SEQ_TARGET);

	switch (m_sequence & SEQ_CMD_MASK)
	{
	case SEQ_COMMAND:
	case SEQ_DATA_OUT:
	case SEQ_MSG_OUT:
		return !target;
	case SEQ_STATUS:
	case SEQ_DATA_IN:
	case SEQ_MSG_IN:
		return target;
	default:
		return false;
	}
}

bool mesh_device::transfer_from_scsi() const
{
	bool const target = bool(m_sequence & SEQ_TARGET);

	switch (m_sequence & SEQ_CMD_MASK)
	{
	case SEQ_COMMAND:
	case SEQ_DATA_OUT:
	case SEQ_MSG_OUT:
		return target;
	case SEQ_STATUS:
	case SEQ_DATA_IN:
	case SEQ_MSG_IN:
		return !target;
	default:
		return false;
	}
}

bool mesh_device::data_in_fifo_pending() const
{
	return transfer_from_scsi()
		&& ((m_sequence & SEQ_CMD_MASK) == SEQ_DATA_IN)
		&& m_fifo_count;
}

bool mesh_device::sync_data_in() const
{
	return !(m_sequence & SEQ_TARGET)
		&& ((m_sequence & SEQ_CMD_MASK) == SEQ_DATA_IN)
		&& ((m_sync_params >> 4) != 0);
}
