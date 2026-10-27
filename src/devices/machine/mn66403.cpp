// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Panasonic MN664031A CD-ROM/SCSI controller

    Register names and behaviour are inferred from Matsushita CR-504
    firmware 4.0i; no data sheet is available.

    TODO: error interrupts, ATN during data transfers, extended DMA modes,
    ECC, buffer arbitration, synchronous transfers and hardware timings.

***************************************************************************/

#include "emu.h"
#include "mn66403.h"

#include "multibyte.h"

#include <algorithm>
#include <iterator>

#define LOG_REGISTERS (1U << 1)
#define LOG_SCSI      (1U << 2)
#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(MN664031, mn664031_device, "mn664031", "Panasonic MN664031A CD-ROM/SCSI controller")

mn664031_device::mn664031_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MN664031, tag, owner, clock)
	, nscsi_device_interface(mconfig, *this)
	, m_buffer(*this, "buffer", BUFFER_SIZE, ENDIANNESS_LITTLE)
	, m_gpio_cb(*this)
	, m_irq_cb(*this)
	, m_decoder_irq_cb(*this)
	, m_bus_timer(nullptr)
	, m_regs{}
	, m_decoder_regs{}
	, m_sector{}
	, m_input_sectors{}
	, m_input_index(0)
	, m_input_count(0)
	, m_headers{}
	, m_sector_addresses{}
	, m_pipeline_valid(0)
	, m_decoder_pending(0)
	, m_decoder_irq(false)
	, m_capture_address(0)
	, m_subcode_address(0)
	, m_subcode_addresses{}
	, m_subcode_index(0)
	, m_subcode_count(0)
	, m_command_fifo{}
	, m_data_fifo{}
	, m_data_count(0)
	, m_data_read(0)
	, m_fifo_count(0)
	, m_fifo_read(0)
	, m_command_size(0)
	, m_pending(0)
	, m_state(BUS_FREE)
	, m_status(0)
	, m_message(0)
	, m_message_fifo{}
	, m_message_count(0)
	, m_message_read(0)
	, m_message_valid(false)
	, m_disconnect(false)
	, m_reset(false)
	, m_irq(false)
	, m_ack(false)
	, m_dma_address(0)
	, m_dma_remaining(0)
	, m_dma_block_size(0)
	, m_dma_block_address(0)
	, m_dma_blocks(0)
	, m_dma_command(0)
{
}

void mn664031_device::map(address_map &map)
{
	map(0x00, 0x3f).rw(FUNC(mn664031_device::reg_r), FUNC(mn664031_device::reg_w));
	map(0x03, 0x0a).r(FUNC(mn664031_device::decoder_r<0x03>));
	map(0x0b, 0x0b).r(FUNC(mn664031_device::decoder_ack_r));
	map(0x0c, 0x0e).r(FUNC(mn664031_device::decoder_r<0x0c>));
	map(0x10, 0x12).r(FUNC(mn664031_device::decoder_r<0x10>));
	map(0x13, 0x13).rw(FUNC(mn664031_device::sector_ack_r), FUNC(mn664031_device::capture_control_w));
	map(0x14, 0x14).w(FUNC(mn664031_device::pio_count_w));
	map(0x1f, 0x1f).r(FUNC(mn664031_device::dma_blocks_low_r));
	map(0x20, 0x20).r(FUNC(mn664031_device::dma_blocks_high_r));
	map(0x21, 0x21).rw(FUNC(mn664031_device::command_fifo_r), FUNC(mn664031_device::scsi_status_w));
	map(0x22, 0x22).rw(FUNC(mn664031_device::data_fifo_r), FUNC(mn664031_device::data_fifo_w));
	map(0x23, 0x23).rw(FUNC(mn664031_device::message_r), FUNC(mn664031_device::message_w));
	map(0x29, 0x29).r(FUNC(mn664031_device::status_r));
	map(0x2a, 0x2a).w(FUNC(mn664031_device::abort_w));
	map(0x2b, 0x2b).w(FUNC(mn664031_device::flush_w));
	map(0x2c, 0x2c).rw(FUNC(mn664031_device::sequencer_r), FUNC(mn664031_device::sequencer_w));
	map(0x2e, 0x2e).w(FUNC(mn664031_device::command_extension_w));
	map(0x32, 0x32).rw(FUNC(mn664031_device::buffer_r), FUNC(mn664031_device::buffer_w));
	map(0x33, 0x33).lr8(NAME([this] () -> u8 { return ~m_pending; })).w(FUNC(mn664031_device::pending_w));
	map(0x34, 0x34).lr8(NAME([this] () -> u8 { return ~m_decoder_pending; }));
	map(0x35, 0x35).lr8(NAME([] () -> u8 { return 0x40; })); // CPU buffer write ready; writes set the IRQ mask
	map(0x35, 0x36).w(FUNC(mn664031_device::irq_mask_w));
	map(0x37, 0x37).r(FUNC(mn664031_device::semaphore_r));
	map(0x3b, 0x3b).lr8(NAME([this] () -> u8 { return BIT(~m_scsi_id, 2); })).w(FUNC(mn664031_device::gpio_w));
}

void mn664031_device::device_start()
{
	m_bus_timer = timer_alloc(FUNC(mn664031_device::bus_step), this);

	save_item(NAME(m_regs));
	save_item(NAME(m_decoder_regs));
	save_item(NAME(m_sector));
	save_item(NAME(m_input_sectors));
	save_item(NAME(m_input_index));
	save_item(NAME(m_input_count));
	save_item(NAME(m_headers));
	save_item(NAME(m_sector_addresses));
	save_item(NAME(m_pipeline_valid));
	save_item(NAME(m_decoder_pending));
	save_item(NAME(m_decoder_irq));
	save_item(NAME(m_capture_address));
	save_item(NAME(m_subcode_address));
	save_item(NAME(m_subcode_addresses));
	save_item(NAME(m_subcode_index));
	save_item(NAME(m_subcode_count));
	save_item(NAME(m_dma_block_size));
	save_item(NAME(m_dma_block_address));
	save_item(NAME(m_dma_blocks));
	save_item(NAME(m_dma_command));
	save_item(NAME(m_command_fifo));
	save_item(NAME(m_data_fifo));
	save_item(NAME(m_data_count));
	save_item(NAME(m_data_read));
	save_item(NAME(m_fifo_count));
	save_item(NAME(m_fifo_read));
	save_item(NAME(m_command_size));
	save_item(NAME(m_pending));
	save_item(NAME(m_state));
	save_item(NAME(m_status));
	save_item(NAME(m_message));
	save_item(NAME(m_message_fifo));
	save_item(NAME(m_message_count));
	save_item(NAME(m_message_read));
	save_item(NAME(m_message_valid));
	save_item(NAME(m_disconnect));
	save_item(NAME(m_reset));
	save_item(NAME(m_irq));
	save_item(NAME(m_ack));
	save_item(NAME(m_dma_address));
	save_item(NAME(m_dma_remaining));
}

void mn664031_device::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	std::fill(std::begin(m_decoder_regs), std::end(m_decoder_regs), 0);
	m_pipeline_valid = m_decoder_pending = 0;
	m_decoder_irq = false;
	m_input_index = m_input_count = 0;
	m_capture_address = m_subcode_address = 0;
	std::fill(std::begin(m_subcode_addresses), std::end(m_subcode_addresses), 0);
	m_subcode_index = m_subcode_count = 0;
	m_dma_blocks = m_dma_block_size = m_dma_block_address = m_dma_command = 0;
	m_decoder_irq_cb(0);
	m_gpio_cb(0);
	m_fifo_count = m_fifo_read = m_command_size = 0;
	m_data_count = m_data_read = 0;
	m_pending = 0;
	m_state = BUS_FREE;
	m_status = m_message = 0;
	m_message_count = m_message_read = 0;
	m_message_valid = m_disconnect = m_reset = false;
	m_ack = false;
	m_dma_address = m_dma_remaining = 0;
	m_irq = false;
	m_irq_cb(0);
	m_bus_timer->adjust(attotime::never);
	m_scsi_bus->ctrl_wait(m_scsi_refid, S_ALL, S_ALL);
	m_scsi_bus->data_w(m_scsi_refid, 0);
	m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ALL);
}

u32 mn664031_device::buffer_address() const
{
	return get_u24le(&m_regs[0x2f]) & (BUFFER_SIZE - 1);
}

void mn664031_device::advance_buffer()
{
	put_u24le(&m_regs[0x2f], (buffer_address() + 1) & (BUFFER_SIZE - 1));
}

void mn664031_device::update_irq()
{
	const bool decoder_irq = bool(m_decoder_pending & m_regs[0x36] & 3);
	if (m_decoder_irq != decoder_irq)
	{
		m_decoder_irq = decoder_irq;
		m_decoder_irq_cb(decoder_irq);
	}

	const bool irq = bool(m_pending & m_regs[0x35]);
	if (m_irq != irq)
	{
		m_irq = irq;
		m_irq_cb(irq);
	}
}

void mn664031_device::interrupt(u8 bits)
{
	m_pending |= bits;
	update_irq();
}

u8 mn664031_device::reg_r(offs_t offset)
{
	return m_regs[offset];
}

void mn664031_device::reg_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_REGISTERS, "%s: register %02x = %02x\n", machine().describe_context(), offset, data);
	m_regs[offset] = data;
}

u8 mn664031_device::decoder_ack_r()
{
	// read to acknowledge (inferred)
	if (!machine().side_effects_disabled())
	{
		m_decoder_pending &= ~1;
		update_irq();
	}
	return m_decoder_regs[0x0b];
}

u8 mn664031_device::sector_ack_r()
{
	// read to acknowledge (inferred)
	if (!machine().side_effects_disabled())
	{
		m_decoder_pending &= ~2;
		update_irq();
	}
	return m_decoder_regs[0x13];
}

u8 mn664031_device::dma_blocks_low_r()
{
	if (!machine().side_effects_disabled())
	{
		m_pending &= ~0x08;
		update_irq();
	}
	return (m_dma_blocks - 1) & 0xff;
}

u8 mn664031_device::dma_blocks_high_r()
{
	return BIT(m_dma_blocks - 1, 8);
}

u8 mn664031_device::command_fifo_r()
{
	if (m_fifo_read >= m_fifo_count)
	{
		return 0xff;
	}

	const u8 data = m_command_fifo[m_fifo_read];
	if (!machine().side_effects_disabled())
	{
		if (++m_fifo_read == m_fifo_count)
		{
			m_pending &= ~0x10;
		}
		update_irq();
	}
	return data;
}

u8 mn664031_device::data_fifo_r()
{
	if (m_data_read >= m_data_count)
	{
		return 0xff;
	}

	const u8 data = m_data_fifo[m_data_read];
	if (!machine().side_effects_disabled())
	{
		++m_data_read;
	}
	return data;
}

u8 mn664031_device::message_r()
{
	if (!machine().side_effects_disabled())
	{
		m_message_valid = false;
	}
	return m_message;
}

u8 mn664031_device::status_r()
{
	// bit 3 is live /RST, bit 7 means no received message is available
	return ((m_data_read == m_data_count) ? 0x01 : 0x00) | (m_reset ? 0x00 : 0x08) | (m_message_valid ? 0x00 : 0x80);
}

u8 mn664031_device::sequencer_r()
{
	// bit 5 is /ATN
	return (m_regs[0x2c] & 0x1f) | 0x40 | ((m_scsi_bus->ctrl_r() & S_ATN) ? 0x00 : 0x20);
}

u8 mn664031_device::buffer_r()
{
	const u8 data = m_buffer[buffer_address()];
	if (!machine().side_effects_disabled())
	{
		advance_buffer();
	}
	return data;
}

u8 mn664031_device::semaphore_r()
{
	// test-and-set semaphore shared by the two MCU contexts
	const u8 result = m_regs[0x37];
	if (!machine().side_effects_disabled())
	{
		m_regs[0x37] |= 1;
	}
	return result;
}

void mn664031_device::capture_control_w(u8 data)
{
	const u8 old = m_regs[0x13];
	reg_w(0x13, data);
	if (BIT(data, 6) && !BIT(old, 6))
	{
		m_subcode_address = (get_u24le(&m_regs[3]) + m_regs[6] * 16) & (BUFFER_SIZE - 1);
		m_subcode_index = m_subcode_count = 0;
	}

	if ((data & 0x0c) == 0x0c && (old & 0x0c) != 0x0c)
	{
		m_capture_address = get_u24le(&m_regs[0]) & (BUFFER_SIZE - 1);
		m_pipeline_valid = 0;
	}

	if (!BIT(data, 7))
	{
		m_pipeline_valid = m_decoder_pending = 0;
		update_irq();
	}
}

void mn664031_device::pio_count_w(u8 data)
{
	reg_w(0x14, data);
	if (BIT(m_regs[0x2b], 4) && data && (m_regs[0x2c] == 0x10 || m_regs[0x2c] == 0x20))
	{
		m_dma_remaining = data;
		m_dma_blocks = 1;
		m_dma_command = m_regs[0x2c];
		if (m_regs[0x2c] == 0x10)
		{
			set_phase(DATA_IN, S_PHASE_DATA_IN);
		}
		else
		{
			set_phase(DATA_OUT, S_PHASE_DATA_OUT);
		}
	}
}

void mn664031_device::scsi_status_w(u8 data)
{
	reg_w(0x21, data);
	m_status = data;
}

void mn664031_device::data_fifo_w(u8 data)
{
	reg_w(0x22, data);
	if (m_data_count < std::size(m_data_fifo))
	{
		m_data_fifo[m_data_count++] = data;
	}
}

void mn664031_device::message_w(u8 data)
{
	reg_w(0x23, data);
	if (m_message_count < std::size(m_message_fifo))
	{
		m_message_fifo[m_message_count++] = data;
	}
}

void mn664031_device::abort_w(u8 data)
{
	reg_w(0x2a, data);
	if (BIT(data, 6))
	{
		abort_transfer();
	}
}

void mn664031_device::flush_w(u8 data)
{
	reg_w(0x2b, data);
	if (BIT(data, 7))
	{
		// transfer engine reset, completes with an interrupt
		m_data_count = m_data_read = 0;
		interrupt(0x20);
	}
}

void mn664031_device::sequencer_w(u8 data)
{
	reg_w(0x2c, data);
	switch (data)
	{
	case 0x00:
		if (m_regs[0x2d] == 0x01)
		{
			m_fifo_count = m_fifo_read = 0;
			set_phase(COMMAND, S_PHASE_COMMAND);
		}
		else if (m_regs[0x2d] == 0x02)
		{
			set_phase(MESSAGE_OUT, S_PHASE_MSG_OUT);
		}
		else
		{
			LOGMASKED(LOG_SCSI, "Unimplemented command extension %02x\n", m_regs[0x2d]);
		}
		break;
	case 0x03: // one outgoing message byte, then bus free
	case 0x07: // two outgoing message bytes, then bus free
	case 0x80: // one outgoing message byte, retain connection
		m_disconnect = data != 0x80;
		set_phase(MESSAGE_IN, S_PHASE_MSG_IN);
		break;
	case 0x8d:
	case 0x0d:
		m_disconnect = true;
		set_phase(STATUS, S_PHASE_STATUS);
		break;
	case 0x10:
	case 0x17:
	case 0x1d:
	case 0x20:
		if (BIT(m_regs[0x2b], 4))
		{
			// PIO: the payload is supplied first, then the count starts the transfer
			m_data_count = m_data_read = 0;
			break;
		}
		m_dma_address = get_u24le(&m_regs[0x15]) & (BUFFER_SIZE - 1);
		m_dma_remaining = m_dma_block_size = 1 + get_u24le(&m_regs[0x18]);
		m_dma_blocks = 1 + (get_u16le(&m_regs[0x1f]) & 0x1ff);
		m_dma_block_address = m_dma_address;
		m_dma_command = data;
		if (data == 0x20)
		{
			set_phase(DATA_OUT, S_PHASE_DATA_OUT);
		}
		else
		{
			set_phase(DATA_IN, S_PHASE_DATA_IN);
		}
		break;
	default:
		LOGMASKED(LOG_SCSI, "Unimplemented command %02x\n", data);
		break;
	}
}

void mn664031_device::command_extension_w(u8 data)
{
	// nonstandard command groups receive six bytes, then firmware requests the rest
	reg_w(0x2e, data);
	if (m_state == WAIT_COMMAND && (m_regs[0x2c] & 0x1f) == 0x05
			&& data && data <= std::size(m_command_fifo) - m_fifo_count)
	{
		m_command_size = m_fifo_count + data;
		set_phase(COMMAND, S_PHASE_COMMAND);
	}
}

void mn664031_device::buffer_w(u8 data)
{
	reg_w(0x32, data);
	m_buffer[buffer_address()] = data;
	advance_buffer();
}

void mn664031_device::pending_w(u8 data)
{
	// write one to acknowledge; bit 4 is the FIFO level
	reg_w(0x33, data);
	m_pending &= ~(data & 0xe6);
	update_irq();
}

void mn664031_device::irq_mask_w(offs_t offset, u8 data)
{
	reg_w(0x35 + offset, data);
	update_irq();
}

void mn664031_device::gpio_w(u8 data)
{
	reg_w(0x3b, data);
	m_gpio_cb(data);
}

void mn664031_device::scsi_ctrl_changed()
{
	// provisional settling delay
	m_bus_timer->adjust(attotime::from_nsec(100));
}

void mn664031_device::set_phase(u8 state, u32 phase)
{
	if (m_reset)
	{
		return;
	}

	m_state = state;
	m_ack = false;
	m_scsi_bus->ctrl_w(m_scsi_refid, S_BSY | phase, S_BSY | S_PHASE_MASK | S_REQ);
	request_byte();
}

void mn664031_device::request_byte()
{
	switch (m_state)
	{
	case DATA_IN:
		if (BIT(m_regs[0x2b], 4))
		{
			m_scsi_bus->data_w(m_scsi_refid, (m_data_read < m_data_count) ? m_data_fifo[m_data_read] : 0xff);
		}
		else
		{
			m_scsi_bus->data_w(m_scsi_refid, m_buffer[m_dma_address]);
		}
		break;
	case STATUS:
		m_scsi_bus->data_w(m_scsi_refid, m_status);
		break;
	case MESSAGE_IN:
		m_scsi_bus->data_w(m_scsi_refid, (m_message_read < m_message_count) ? m_message_fifo[m_message_read] : 0xff);
		break;
	default:
		m_scsi_bus->data_w(m_scsi_refid, 0);
		break;
	}
	m_scsi_bus->ctrl_w(m_scsi_refid, S_REQ, S_REQ);
}

void mn664031_device::finish_transfer(u8 result)
{
	m_state = WAIT_COMMAND;
	m_regs[0x2c] = result;
	m_scsi_bus->data_w(m_scsi_refid, 0);
	interrupt(0x20);
}

void mn664031_device::abort_transfer()
{
	m_state = BUS_FREE;
	m_fifo_count = m_fifo_read = m_command_size = 0;
	m_data_count = m_data_read = 0;
	m_message_count = m_message_read = 0;
	m_message_valid = m_disconnect = m_ack = false;
	m_dma_address = m_dma_remaining = 0;
	m_dma_blocks = m_dma_block_size = m_dma_block_address = m_dma_command = 0;
	m_regs[0x2c] = 0;
	m_pending &= 0x04; // keep a latched bus reset
	update_irq();
	m_scsi_bus->data_w(m_scsi_refid, 0);
	m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ALL);
}

TIMER_CALLBACK_MEMBER(mn664031_device::bus_step)
{
	const u32 ctrl = m_scsi_bus->ctrl_r();
	if (ctrl & S_RST)
	{
		if (!m_reset)
		{
			// latched once on assertion; /RST itself is visible in the status register
			m_reset = true;
			LOGMASKED(LOG_SCSI, "SCSI reset asserted\n");
			abort_transfer();
			interrupt(0x04);
		}
		return;
	}

	if (m_reset)
	{
		LOGMASKED(LOG_SCSI, "SCSI reset released\n");
	}
	m_reset = false;

	if (m_state == BUS_FREE)
	{
		if ((ctrl & (S_SEL | S_BSY)) == S_SEL && (m_scsi_bus->data_r() & m_regs[0x27]))
		{
			m_regs[0x28] = m_scsi_bus->data_r() & ~m_regs[0x27];
			LOGMASKED(LOG_SCSI, "select initiator=%02x ATN=%d\n", m_regs[0x28], bool(ctrl & S_ATN));
			m_fifo_count = m_fifo_read = 0;
			m_state = SELECTED;
			m_scsi_bus->ctrl_w(m_scsi_refid, S_BSY, S_BSY);
			interrupt(0x80);
		}
		return;
	}

	if (m_state == SELECTED)
	{
		if (!(ctrl & S_SEL))
		{
			if (ctrl & S_ATN)
			{
				set_phase(MESSAGE_OUT, S_PHASE_MSG_OUT);
			}
			else
			{
				set_phase(COMMAND, S_PHASE_COMMAND);
			}
		}
		return;
	}

	if (m_state == WAIT_COMMAND)
	{
		return;
	}

	if (!m_ack && (ctrl & S_ACK))
	{
		if (m_state == DATA_IN || m_state == STATUS || m_state == MESSAGE_IN)
		{
			LOGMASKED(LOG_SCSI, "acked %s byte %02x\n", (m_state == DATA_IN) ? "data" : (m_state == STATUS) ? "status" : "message", m_scsi_bus->data_r());
		}
		m_ack = true;
		if (m_state == COMMAND && m_fifo_count < std::size(m_command_fifo))
		{
			m_command_fifo[m_fifo_count++] = m_scsi_bus->data_r();
			LOGMASKED(LOG_SCSI, "CDB[%u]=%02x\n", m_fifo_count - 1, m_command_fifo[m_fifo_count - 1]);
			if (m_fifo_count == 1)
			{
				// standard command group lengths
				static constexpr u8 LENGTHS[8] = { 6, 10, 10, 6, 16, 12, 6, 6 };
				m_command_size = LENGTHS[m_command_fifo[0] >> 5];
			}
		}
		else if (m_state == MESSAGE_OUT)
		{
			m_message = m_scsi_bus->data_r();
			m_message_valid = true;
			LOGMASKED(LOG_SCSI, "message out %02x ATN=%d\n", m_message, bool(ctrl & S_ATN));
		}
		else if (m_state == DATA_OUT)
		{
			const u8 data = m_scsi_bus->data_r();
			LOGMASKED(LOG_SCSI, "data out byte %02x\n", data);
			if (!BIT(m_regs[0x2b], 4))
			{
				m_buffer[m_dma_address] = data;
			}
			else if (m_data_count < std::size(m_data_fifo))
			{
				m_data_fifo[m_data_count++] = data;
			}
		}
		m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_REQ);
	}
	else if (m_ack && !(ctrl & S_ACK))
	{
		m_ack = false;
		switch (m_state)
		{
		case COMMAND:
			if (m_fifo_count == m_command_size)
			{
				m_pending |= 0x10;
				finish_transfer(0x05);
				return;
			}
			break;
		case MESSAGE_OUT:
			m_pending |= 0x02;
			finish_transfer();
			return;
		case DATA_IN:
		case DATA_OUT:
			m_dma_address = (m_dma_address + 1) & (BUFFER_SIZE - 1);
			if (m_state == DATA_IN && BIT(m_regs[0x2b], 4) && m_data_read < m_data_count)
			{
				++m_data_read;
			}

			if (!--m_dma_remaining)
			{
				if (!--m_dma_blocks)
				{
					finish_dma();
					return;
				}

				// TODO: segment/skip controls at 801b-1e; normal DMA skips to the next raw sector slot
				m_dma_block_address += 0x0a68;
				if (m_dma_block_address >= 0x3f168)
				{
					m_dma_block_address -= 0x3f168;
				}
				m_dma_address = m_dma_block_address;
				m_dma_remaining = m_dma_block_size;
				interrupt(0x08);
			}
			break;
		case STATUS:
			set_phase(MESSAGE_IN, S_PHASE_MSG_IN);
			return;
		case MESSAGE_IN:
			if (++m_message_read < m_message_count)
			{
				break;
			}
			m_message_count = m_message_read = 0;
			finish_transfer();
			if (m_disconnect)
			{
				m_state = BUS_FREE;
				m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ALL);
			}
			return;
		}
		request_byte();
	}
}

void mn664031_device::subcode_w(const u8 *q)
{
	// TODO: alternate subcode buffer mode and R-W channels
	if (!BIT(m_regs[0x13], 6) || !BIT(m_regs[0x0a], 5) || !BIT(m_regs[9], 0))
	{
		return;
	}

	const auto advance = [] (u32 address)
	{
		address += 96;
		if (address % 0x0a68 == 0x900)
		{
			address += 0x168;
		}
		return (address >= 0x3f168) ? 0 : address;
	};
	m_subcode_address = advance(m_subcode_address);
	m_subcode_addresses[m_subcode_index] = m_subcode_address;
	m_subcode_index = (m_subcode_index + 1) % std::size(m_subcode_addresses);
	if (m_subcode_count < 2)
	{
		++m_subcode_count;
	}
	else
	{
		// pointer readback trails capture by two frames
		put_u24le(&m_decoder_regs[7], m_subcode_addresses[m_subcode_index]);
	}

	// raw P-W and deinterleaved Q share a slot
	const u32 address = m_subcode_address;
	const u32 q_address = address / 0x0a68 * 0x0a68 + 0x900 + (address % 0x0a68) / 96 * 12;
	u8 packet[12];
	std::copy_n(q, 10, packet);
	u16 crc = 0;
	for (int i = 0; i < 10; ++i)
	{
		crc ^= u16(packet[i]) << 8;
		for (int bit = 0; bit < 8; ++bit)
		{
			crc = (crc << 1) ^ (BIT(crc, 15) ? 0x1021 : 0);
		}
	}
	packet[10] = ~crc >> 8;
	packet[11] = ~crc;
	for (int i = 0; i < 12; ++i)
	{
		m_buffer[(q_address + i) & (BUFFER_SIZE - 1)] = packet[i];
		for (int bit = 0; bit < 8; ++bit)
		{
			m_buffer[(address + i * 8 + bit) & (BUFFER_SIZE - 1)] = (BIT(packet[i], 7 - bit) ? 0x40 : 0) | (!q[2] ? 0x80 : 0);
		}
	}
	m_decoder_regs[0x0a] = m_decoder_regs[0x0b] = 0; // valid synthesized Q, no overrun
	m_decoder_pending |= 1;
	update_irq();
}

void mn664031_device::sector_w(const u8 *data)
{
	// two frames of provisional front-end latency before a header is reported
	std::copy_n(data, sizeof(m_sector), m_input_sectors[m_input_index]);
	m_input_index = (m_input_index + 1) % std::size(m_input_sectors);
	if (m_input_count < 2)
	{
		++m_input_count;
		return;
	}
	data = m_input_sectors[m_input_index];
	std::copy_n(data, sizeof(m_sector), m_sector);
	if (!BIT(m_regs[0x13], 7))
	{
		return;
	}

	std::copy_n(data + 12, 4, m_decoder_regs + 3);
	m_decoder_regs[0x0c] = 0x80;
	m_decoder_regs[0x0d] = 0;
	m_decoder_regs[0x0e] = data[15] << 4;
	m_decoder_regs[0x13] = 0x80;
	if (BIT(m_pipeline_valid, 1))
	{
		if (BIT(m_regs[0x13], 5))
		{
			std::copy_n(m_headers[1], 4, m_decoder_regs + 3);
		}
		put_u24le(&m_decoder_regs[0x10], (m_sector_addresses[1] + 12) & (BUFFER_SIZE - 1));
	}

	// capture starts on the following sector, with results two frames later
	std::copy_n(m_headers[0], 4, m_headers[1]);
	m_sector_addresses[1] = m_sector_addresses[0];
	m_pipeline_valid = (m_pipeline_valid << 1) & 3;
	if ((m_regs[0x13] & 0x0c) == 0x0c)
	{
		capture_sector();
	}
	m_decoder_pending |= 2;
	update_irq();
}

void mn664031_device::capture_sector()
{
	for (u32 i = 0; i < sizeof(m_sector); ++i)
	{
		m_buffer[(m_capture_address + i) & (BUFFER_SIZE - 1)] = m_sector[i];
	}
	std::copy_n(m_sector + 12, 4, m_headers[0]);
	m_sector_addresses[0] = m_capture_address;
	m_pipeline_valid |= 1;

	// TODO: 8008 sets the ring limit in sector units (firmware writes 09)
	m_capture_address += 0x0a68;
	if (m_capture_address >= 0x3f168)
	{
		m_capture_address -= 0x3f168;
	}
}

void mn664031_device::finish_dma()
{
	if (m_dma_command == 0x1d)
	{
		m_disconnect = true;
		set_phase(STATUS, S_PHASE_STATUS);
	}
	else if (m_dma_command == 0x17)
	{
		m_disconnect = true;
		set_phase(MESSAGE_IN, S_PHASE_MSG_IN);
	}
	else
	{
		finish_transfer();
	}
}
