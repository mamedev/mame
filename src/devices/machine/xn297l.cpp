// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "xn297l.h"

// This is a byte-oriented HLE of the host-visible SPI, register and FIFO
// behaviour used by current drivers.  RF propagation and communication with
// a peer are not emulated; enhanced transmissions therefore exhaust their
// retries and set MAX_RT.
// TODO: Retain the last successfully transmitted frame for REUSE_TX_PL and
// implement ACK payload storage when a driver needs either feature.

#define LOG_COMMAND (1U << 1)
#define LOG_FIFO    (1U << 2)

//#define VERBOSE (LOG_COMMAND | LOG_FIFO)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(XN297L, xn297l_device, "xn297l", "Panchip XN297L 2.4 GHz transceiver")

xn297l_device::xn297l_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, XN297L, tag, owner, clock),
	m_tx_count(0),
	m_rx_count(0),
	m_command(CMD_NOP),
	m_command_pos(0),
	m_payload_length(0),
	m_payload_no_ack(0),
	m_cs(true),
	m_ce(false),
	m_reset_hold(false),
	m_features_active(false),
	m_tx_reuse(false),
	m_transmitting(false),
	m_transmit_timer(nullptr)
{
	std::fill(&m_registers[0][0], &m_registers[0][0] + sizeof(m_registers), 0);
	std::fill(&m_tx_fifo[0][0], &m_tx_fifo[0][0] + sizeof(m_tx_fifo), 0);
	std::fill(&m_rx_fifo[0][0], &m_rx_fifo[0][0] + sizeof(m_rx_fifo), 0);
	std::fill(&m_tx_length[0], &m_tx_length[0] + sizeof(m_tx_length), 0);
	std::fill(&m_tx_no_ack[0], &m_tx_no_ack[0] + sizeof(m_tx_no_ack), 0);
	std::fill(&m_rx_length[0], &m_rx_length[0] + sizeof(m_rx_length), 0);
	std::fill(&m_rx_pipe[0], &m_rx_pipe[0] + sizeof(m_rx_pipe), 0);
	std::fill(&m_payload[0], &m_payload[0] + sizeof(m_payload), 0);
}

void xn297l_device::device_start()
{
	m_transmit_timer = timer_alloc(FUNC(xn297l_device::transmit_complete), this);

	save_item(NAME(m_registers));
	save_item(NAME(m_tx_fifo));
	save_item(NAME(m_tx_length));
	save_item(NAME(m_tx_no_ack));
	save_item(NAME(m_tx_count));
	save_item(NAME(m_rx_fifo));
	save_item(NAME(m_rx_length));
	save_item(NAME(m_rx_pipe));
	save_item(NAME(m_rx_count));
	save_item(NAME(m_command));
	save_item(NAME(m_command_pos));
	save_item(NAME(m_payload));
	save_item(NAME(m_payload_length));
	save_item(NAME(m_payload_no_ack));
	save_item(NAME(m_cs));
	save_item(NAME(m_ce));
	save_item(NAME(m_reset_hold));
	save_item(NAME(m_features_active));
	save_item(NAME(m_tx_reuse));
	save_item(NAME(m_transmitting));
}

void xn297l_device::device_reset()
{
	reset_registers();
	m_cs = true;
	m_ce = false;
	m_reset_hold = false;
	m_features_active = false;
	m_command = CMD_NOP;
	m_command_pos = 0;
	m_payload_length = 0;
	m_payload_no_ack = 0;
	m_transmitting = false;
	m_transmit_timer->adjust(attotime::never);
}

void xn297l_device::reset_registers()
{
	m_features_active = false;
	std::fill(&m_registers[0][0], &m_registers[0][0] + sizeof(m_registers), 0);
	m_registers[REG_CONFIG][0] = 0x08;
	m_registers[REG_EN_AA][0] = 0x01;
	m_registers[0x02][0] = 0x01;
	m_registers[0x03][0] = 0x03;
	m_registers[0x04][0] = 0x03;
	m_registers[REG_RF_CH][0] = 0x4e;
	m_registers[0x06][0] = 0x3f;
	std::fill_n(m_registers[REG_RX_ADDR_P0], 5, 0xe7);
	std::fill_n(m_registers[REG_RX_ADDR_P1], 5, 0xc2);
	m_registers[0x0c][0] = 0xc3;
	m_registers[0x0d][0] = 0xc4;
	m_registers[0x0e][0] = 0xc5;
	m_registers[0x0f][0] = 0xc6;
	std::fill_n(m_registers[REG_TX_ADDR], 5, 0xe7);
	m_registers[0x19][0] = 0x0f;
	const u8 rf_cal2[6] = { 0x45, 0x21, 0xef, 0x2c, 0x5a, 0x42 };
	std::copy(std::begin(rf_cal2), std::end(rf_cal2), m_registers[0x1a]);
	const u8 dem_cal2[3] = { 0x0b, 0xdf, 0x02 };
	std::copy(std::begin(dem_cal2), std::end(dem_cal2), m_registers[0x1b]);
	const u8 rf_cal[3] = { 0xf6, 0x3f, 0x5d };
	std::copy(std::begin(rf_cal), std::end(rf_cal), m_registers[0x1e]);
	const u8 bb_cal[5] = { 0x0a, 0x6d, 0x67, 0x9c, 0x46 };
	std::copy(std::begin(bb_cal), std::end(bb_cal), m_registers[0x1f]);

	flush_tx();
	flush_rx();
	m_registers[REG_STATUS][0] = 0;
	m_registers[REG_OBSERVE_TX][0] = 0;
}

unsigned xn297l_device::register_width(u8 reg) const
{
	switch (reg)
	{
	case REG_RX_ADDR_P0:
	case REG_RX_ADDR_P1:
	case REG_TX_ADDR:
	case 0x1f:
		return 5;
	case 0x1a:
		return 6;
	case 0x1b:
	case 0x1e:
		return 3;
	default:
		return 1;
	}
}

unsigned xn297l_device::payload_limit() const
{
	return (m_registers[REG_FEATURE][0] & 0x18) == 0x18 ? 64 : 32;
}

unsigned xn297l_device::fifo_limit() const
{
	return payload_limit() == 64 ? 1 : FIFO_DEPTH;
}

u8 xn297l_device::status_r() const
{
	const u8 pipe = m_rx_count ? (m_rx_pipe[0] & 7) : 7;
	return (m_registers[REG_STATUS][0] & 0x70) | (pipe << 1) | (m_tx_count >= fifo_limit());
}

u8 xn297l_device::fifo_status_r() const
{
	return (m_tx_reuse ? 0x40 : 0x00)
		| (m_tx_count >= fifo_limit() ? 0x20 : 0x00)
		| (m_tx_count ? 0x00 : 0x10)
		| (m_rx_count >= fifo_limit() ? 0x02 : 0x00)
		| (m_rx_count ? 0x00 : 0x01);
}

u8 xn297l_device::register_r(u8 reg, u8 offset) const
{
	if (offset >= register_width(reg))
		return 0xff;
	if (reg == REG_STATUS)
		return status_r();
	if (reg == REG_FIFO_STATUS)
		return fifo_status_r();
	return m_registers[reg][offset];
}

void xn297l_device::register_w(u8 reg, u8 offset, u8 data)
{
	if (offset >= register_width(reg))
		return;

	if (reg == REG_STATUS)
	{
		m_registers[REG_STATUS][0] &= ~(data & 0x70);
		return;
	}
	if (reg == REG_OBSERVE_TX || reg == REG_FIFO_STATUS)
		return;

	switch (reg)
	{
	case REG_EN_AA:
	case 0x02:
	case 0x1c:
		data &= 0x3f;
		break;
	case 0x03:
		data &= 0x03;
		break;
	case REG_RF_CH:
		data &= 0x7f;
		m_registers[REG_OBSERVE_TX][0] &= 0x0f;
		break;
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
		data &= 0x7f;
		break;
	case REG_FEATURE:
		data &= 0x7f;
		break;
	default:
		break;
	}

	m_registers[reg][offset] = data;
	if (reg == REG_FEATURE && !BIT(data, 5))
		m_ce = false;
	if (reg == REG_CONFIG && (!BIT(data, 1) || BIT(data, 0)))
	{
		m_transmitting = false;
		m_transmit_timer->adjust(attotime::never);
	}
}

void xn297l_device::flush_tx()
{
	m_tx_count = 0;
	std::fill(std::begin(m_tx_length), std::end(m_tx_length), 0);
	std::fill(std::begin(m_tx_no_ack), std::end(m_tx_no_ack), 0);
	m_tx_reuse = false;
	m_transmitting = false;
	if (m_transmit_timer)
		m_transmit_timer->adjust(attotime::never);
}

void xn297l_device::flush_rx()
{
	m_rx_count = 0;
	std::fill(std::begin(m_rx_length), std::end(m_rx_length), 0);
	std::fill(std::begin(m_rx_pipe), std::end(m_rx_pipe), 0);
}

void xn297l_device::pop_tx()
{
	if (!m_tx_count)
		return;
	for (unsigned i = 1; i < m_tx_count; ++i)
	{
		std::copy(std::begin(m_tx_fifo[i]), std::end(m_tx_fifo[i]), m_tx_fifo[i - 1]);
		m_tx_length[i - 1] = m_tx_length[i];
		m_tx_no_ack[i - 1] = m_tx_no_ack[i];
	}
	--m_tx_count;
}

void xn297l_device::pop_rx()
{
	if (!m_rx_count)
		return;
	for (unsigned i = 1; i < m_rx_count; ++i)
	{
		std::copy(std::begin(m_rx_fifo[i]), std::end(m_rx_fifo[i]), m_rx_fifo[i - 1]);
		m_rx_length[i - 1] = m_rx_length[i];
		m_rx_pipe[i - 1] = m_rx_pipe[i];
	}
	--m_rx_count;
}

void xn297l_device::start_transmit()
{
	if (m_transmitting || !m_ce || !m_tx_count || !BIT(m_registers[REG_CONFIG][0], 1)
		|| BIT(m_registers[REG_CONFIG][0], 0) || BIT(m_registers[REG_STATUS][0], 4))
		return;

	m_transmitting = true;
	const bool auto_ack = BIT(m_registers[REG_EN_AA][0], 0) && !m_tx_no_ack[0];
	const unsigned attempts = auto_ack ? ((m_registers[0x04][0] & 0x0f) + 1) : 1;
	const unsigned retry_delay = ((m_registers[0x04][0] >> 4) + 1) * 250;
	const unsigned packet_time = 150 + m_tx_length[0] * 8;
	m_transmit_timer->adjust(attotime::from_usec(packet_time * attempts + (attempts - 1) * retry_delay));
}

TIMER_CALLBACK_MEMBER(xn297l_device::transmit_complete)
{
	m_transmitting = false;
	if (!m_tx_count)
		return;

	const bool auto_ack = BIT(m_registers[REG_EN_AA][0], 0) && !m_tx_no_ack[0];
	if (auto_ack)
	{
		const u8 retries = m_registers[0x04][0] & 0x0f;
		m_registers[REG_OBSERVE_TX][0] = std::min<unsigned>((m_registers[REG_OBSERVE_TX][0] & 0xf0) + 0x10, 0xf0) | retries;
		m_registers[REG_STATUS][0] |= 0x10;
		LOGMASKED(LOG_FIFO, "TX payload reached maximum retry count\n");
	}
	else
	{
		pop_tx();
		m_registers[REG_STATUS][0] |= 0x20;
		LOGMASKED(LOG_FIFO, "TX payload sent without acknowledgement\n");
		start_transmit();
	}
}

void xn297l_device::finish_command()
{
	bool const write_tx = m_command == CMD_W_TX_PAYLOAD
		|| (m_command == CMD_W_TX_PAYLOAD_NO_ACK && m_features_active && BIT(m_registers[REG_FEATURE][0], 0));
	if (write_tx
		&& m_payload_length && m_tx_count < fifo_limit())
	{
		std::copy_n(m_payload, m_payload_length, m_tx_fifo[m_tx_count]);
		m_tx_length[m_tx_count] = m_payload_length;
		m_tx_no_ack[m_tx_count] = m_payload_no_ack;
		++m_tx_count;
		m_tx_reuse = false;
		m_registers[REG_OBSERVE_TX][0] &= 0xf0;
		LOGMASKED(LOG_FIFO, "Queued %u-byte TX payload%s\n", m_payload_length, m_payload_no_ack ? " without ACK" : "");
		start_transmit();
	}
	else if (m_command == CMD_R_RX_PAYLOAD && m_command_pos > 1)
	{
		pop_rx();
	}
	else if (m_command == CMD_RESET && m_command_pos > 1)
	{
		if (m_payload[0] == 0x5a)
		{
			reset_registers();
			m_ce = false;
			m_reset_hold = true;
		}
		else if (m_payload[0] == 0xa5)
		{
			m_reset_hold = false;
		}
	}
	else if (m_command == CMD_ACTIVATE && m_command_pos > 1)
	{
		if (m_payload[0] == 0x73)
			m_features_active = true;
		else if (m_payload[0] == 0x8c)
		{
			m_features_active = false;
			m_tx_reuse = false;
		}
	}
	else if ((m_command == CMD_CE_FSPI_ON || m_command == CMD_CE_FSPI_OFF) && m_command_pos > 1
		&& m_payload[0] == 0x00 && BIT(m_registers[REG_FEATURE][0], 5))
	{
		m_ce = m_command == CMD_CE_FSPI_ON;
		if (m_ce)
			start_transmit();
	}
}

void xn297l_device::cs_w(int state)
{
	state = state ? 1 : 0;
	if (state == m_cs)
		return;

	if (state)
		finish_command();
	else
	{
		m_command = CMD_NOP;
		m_command_pos = 0;
		m_payload_length = 0;
		m_payload_no_ack = 0;
	}
	m_cs = state;
}

u8 xn297l_device::transfer(u8 data)
{
	if (m_cs)
		return 0xff;

	if (!m_command_pos)
	{
		u8 const result = status_r();
		m_command = data;
		m_command_pos = 1;
		LOGMASKED(LOG_COMMAND, "Command %02X (status %02X)\n", data, result);

		if (m_reset_hold && data != CMD_RESET)
			return result;

		switch (data)
		{
		case CMD_FLUSH_TX:
			flush_tx();
			break;
		case CMD_FLUSH_RX:
			flush_rx();
			break;
		case CMD_REUSE_TX_PL:
			m_tx_reuse = m_tx_count != 0;
			break;
		case CMD_R_RX_PL_WID:
		case CMD_R_RX_PAYLOAD:
		case CMD_W_TX_PAYLOAD:
		case CMD_W_TX_PAYLOAD_NO_ACK:
		case CMD_ACTIVATE:
		case CMD_RESET:
		case CMD_CE_FSPI_OFF:
		case CMD_CE_FSPI_ON:
		case CMD_NOP:
			break;
		default:
			if ((data & 0xe0) != CMD_R_REGISTER && (data & 0xe0) != CMD_W_REGISTER && (data & 0xf8) != 0xa8)
				logerror("%s: unknown SPI command %02X\n", machine().describe_context(), data);
			break;
		}
		return result;
	}
	if (m_reset_hold && m_command != CMD_RESET)
		return status_r();

	const u8 pos = m_command_pos++ - 1;
	if ((m_command & 0xe0) == CMD_R_REGISTER)
		return register_r(m_command & 0x1f, pos);
	if ((m_command & 0xe0) == CMD_W_REGISTER)
	{
		register_w(m_command & 0x1f, pos, data);
		return status_r();
	}
	if (m_command == CMD_R_RX_PL_WID)
		return m_features_active && BIT(m_registers[REG_FEATURE][0], 2) && m_rx_count ? m_rx_length[0] : 0;
	if (m_command == CMD_R_RX_PAYLOAD)
		return (m_rx_count && pos < m_rx_length[0]) ? m_rx_fifo[0][pos] : 0xff;
	if (m_command == CMD_W_TX_PAYLOAD || m_command == CMD_W_TX_PAYLOAD_NO_ACK || (m_command & 0xf8) == 0xa8)
	{
		bool const enabled = m_command == CMD_W_TX_PAYLOAD
			|| (m_command == CMD_W_TX_PAYLOAD_NO_ACK && m_features_active && BIT(m_registers[REG_FEATURE][0], 0))
			|| ((m_command & 0xf8) == 0xa8 && m_features_active && BIT(m_registers[REG_FEATURE][0], 1));
		if (enabled && m_payload_length < payload_limit())
			m_payload[m_payload_length++] = data;
		m_payload_no_ack = enabled && m_command == CMD_W_TX_PAYLOAD_NO_ACK;
		return status_r();
	}
	if (m_payload_length < MAX_PAYLOAD)
		m_payload[m_payload_length++] = data;
	return status_r();
}
