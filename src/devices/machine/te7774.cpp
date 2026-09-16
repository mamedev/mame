// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
 * te7774.cpp - Tokyo Electron Device Ltd. (TEL) quad UART
 *
 * This is a UART with 4 i8251-shaped channels but the register bits
 * seem to be different.
 *
 * TODO: registers are complete guesses, and the MPC3000 configures all 4 channels for 31250 baud 8N1 (MIDI) so
 * it's basically impossible to derive more information.
 *
 * MPC3000 init values for channels 0-3:
 * 00 00 00 c0 02 95 to control 1
 * 00 to control 2
 * 04 to control 3
 *
 * Channel 4:
 * 00 00 00 c0 02 91 to control 1
 * 00 to control 2
 * 00 to control 3
 *
 * Because of the mapping we can assume channel 4 doesn't want Rx enabled since channel 3 already takes care of it.
 */

#include "emu.h"
#include "te7774.h"

#define LOG_REGISTERS   (1U << 1)
#define LOG_TX          (1U << 2)
#define LOG_RX          (1U << 3)

#define VERBOSE (0)

//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

DEFINE_DEVICE_TYPE(TE7774, te7774_device, "te7774", "TE7774 quad UART")

te7774_device::te7774_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TE7774, tag, owner, clock)
	, m_rxrdy_handler{{*this}, {*this}, {*this}, {*this}}
	, m_txrdy_handler{{*this}, {*this}, {*this}, {*this}}
	, m_txd_handler{{*this}, {*this}, {*this}, {*this}}
	, m_channels(*this, "ch%u", 0U)
	, m_rxd_handler{{*this, 0xff}, {*this, 0xff}, {*this, 0xff}, {*this, 0xff}}
{
}

void te7774_device::device_add_mconfig(machine_config &config)
{
	for (required_device<te7774_channel> &te7774_channel : m_channels)
	{
		TE7774_CHANNEL(config, te7774_channel, clock());
	}
}

void te7774_device::device_start()
{
	for (int i = 0; i < 4; i++)
	{
		m_channels[i]->set_channel(i);
	}
}

void te7774_device::device_reset()
{
}

template <int ch> uint8_t te7774_device::read_cs(offs_t offset)
{
	return m_channels[ch]->read(offset & 3);
}

template uint8_t te7774_device::read_cs<0>(offs_t offset);
template uint8_t te7774_device::read_cs<1>(offs_t offset);
template uint8_t te7774_device::read_cs<2>(offs_t offset);
template uint8_t te7774_device::read_cs<3>(offs_t offset);

template <int ch> void te7774_device::write_cs(offs_t offset, uint8_t data)
{
	m_channels[ch]->write(offset & 3, data);
}

template void te7774_device::write_cs<0>(offs_t offset, uint8_t data);
template void te7774_device::write_cs<1>(offs_t offset, uint8_t data);
template void te7774_device::write_cs<2>(offs_t offset, uint8_t data);
template void te7774_device::write_cs<3>(offs_t offset, uint8_t data);

template <int ch> void te7774_device::rx_w(int state)
{
	m_channels[ch]->rx_w(state);
}

template void te7774_device::rx_w<0>(int state);
template void te7774_device::rx_w<1>(int state);
template void te7774_device::rx_w<2>(int state);
template void te7774_device::rx_w<3>(int state);

// Channel device
DEFINE_DEVICE_TYPE(TE7774_CHANNEL, te7774_channel, "te7774_channel", "TE7774 UART Channel")

enum
{
	STATUS_TxRdy = 0x01,
	STATUS_RxRdy = 0x02,
};

enum
{
	CONTROL1_TxEnable = 0x01,
	CONTROL1_RxEnable = 0x04
};

te7774_channel::te7774_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TE7774_CHANNEL, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_rx_enabled(false)
	, m_tx_data(0)
	, m_rx_data(0)
	, m_control1(0)
	, m_control2(0)
	, m_control3(0)
	, m_status(0)
	, m_tx_enabled(false)
{
}

void te7774_channel::device_start()
{
	m_parent = downcast<te7774_device *>(owner());
}

void te7774_channel::device_reset()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rcv_rate(31250);
	set_tra_rate(31250);

	m_tx_enabled = m_rx_enabled = false;
}

// serial device virtual overrides
void te7774_channel::rcv_complete()
{
	receive_register_extract();

	m_rx_data = get_received_char();
	LOGMASKED(LOG_RX, "%s ch rcv complete %02x\n", tag(), m_rx_data);

	m_status &= ~STATUS_RxRdy; // set rx ready

	if (m_rx_enabled)
	{
		m_parent->m_rxrdy_handler[m_ch](ASSERT_LINE);
	}
}

void te7774_channel::tra_complete()
{
	if (m_tx_enabled)
	{
		m_status |= STATUS_TxRdy; // set tx ready
		m_parent->m_txrdy_handler[m_ch](ASSERT_LINE);
	}
	m_tx_data_in_buffer = false;
}

void te7774_channel::tra_callback()
{
	const int bit = transmit_register_get_data_bit();
	m_parent->m_txd_handler[m_ch](bit);
}

uint8_t te7774_channel::read(offs_t offset)
{
	uint8_t rv = 0;

	switch (offset)
	{
		case 0: // data
			rv = m_rx_data;
			m_status |= STATUS_RxRdy; // clear rx ready
			m_parent->m_rxrdy_handler[m_ch](CLEAR_LINE);
			break;

		case 1: // status
			rv = m_status;
			break;

		case 2: // control 2
			rv = m_control2;
			break;

		case 3: // control 3
			rv = m_control3;
			break;
	}
	LOGMASKED(LOG_REGISTERS, "%s ch read %d = %02x\n", tag(), offset, rv);
	return rv;
}

void te7774_channel::write(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_REGISTERS, "%s ch write %d = %02x (%s)\n", tag(), offset, data, machine().describe_context().c_str());
	switch (offset)
	{
		case 0: // data
			if (m_tx_enabled && !m_tx_data_in_buffer)
			{
				m_tx_data = data;
				transmit_register_setup(m_tx_data);
				m_tx_data_in_buffer = true;
				m_status &= ~STATUS_TxRdy;
				m_parent->m_txrdy_handler[m_ch](CLEAR_LINE);
			}
			break;

		case 1: // control 1
			m_tx_enabled = (data & CONTROL1_TxEnable) != 0;
			m_rx_enabled = (data & CONTROL1_RxEnable) != 0;
			m_control1 = data;
			LOGMASKED(LOG_REGISTERS, "%s tx enable %d rx enable %d\n", tag(), m_tx_enabled, m_rx_enabled);
			break;

		case 2: // control 2
			m_control2 = data;
			break;

		case 3: // control 3
			m_control3 = data;
			break;
	}
}
