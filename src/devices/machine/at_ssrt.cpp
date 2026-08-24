// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * PC/AT Keyboard/Mouse Protocol Synchronous Serial Receiver/Transmitter
 *
 * This device emulates a synchronous serial receiver/transmitter for the PC/AT
 * keyboard protocol. It is intended to be used by systems which support PC
 * keyboards, but do not use a standard PC keyboard controller. Such systems
 * often contain a device similar to this embedded within an ASIC or FPGA,
 * without the other PC-centric features of the keyboard controller.
 *
 * TODO:
 *  - handle contention
 */

#include "emu.h"
#include "at_ssrt.h"

#define LOG_RXTX (1U << 1)

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

enum state : u8
{
	RX_START, // idle state
	RX_DATA1,
	RX_DATA2,
	RX_DATA3,
	RX_DATA4,
	RX_DATA5,
	RX_DATA6,
	RX_DATA7,
	RX_DATA8,
	RX_PARITY,
	RX_STOP,

	TX_START,
	TX_DATA1,
	TX_DATA2,
	TX_DATA3,
	TX_DATA4,
	TX_DATA5,
	TX_DATA6,
	TX_DATA7,
	TX_DATA8,
	TX_PARITY,
	TX_STOP,
	TX_ACK,
};

DEFINE_DEVICE_TYPE(AT_SSRT, at_ssrt_device, "at_ssrt", "PC/AT Keyboard/Mouse Protocol Synchronous Serial Receiver/Transmitter")

at_ssrt_device::at_ssrt_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, AT_SSRT, tag, owner, clock)
	, m_clk_cb(*this)
	, m_txd_cb(*this)
	, m_pe_cb(*this)
	, m_rx_cb(*this)
	, m_tx_cb(*this)
	, m_clk(false)
	, m_rxd(false)
{
}

void at_ssrt_device::device_start()
{
	save_item(NAME(m_state));

	save_item(NAME(m_latch));
	save_item(NAME(m_shift));
	save_item(NAME(m_parity));

	save_item(NAME(m_clk));
	save_item(NAME(m_rxd));

	m_timer = timer_alloc(FUNC(at_ssrt_device::timer), this);
}

void at_ssrt_device::device_reset()
{
	m_state = RX_START;

	m_latch = 0;
	m_shift = 0;
	m_parity = false;

	m_timer->reset();
}

void at_ssrt_device::clk_w(int state)
{
	// falling edge?
	if (m_clk && state == 0)
	{
		LOGMASKED(LOG_RXTX, "clk\n");

		switch (m_state)
		{
		case RX_START:
			if (!m_rxd)
			{
				// rx start bit
				m_shift = 0;
				m_parity = false;

				m_state++;
			}
			break;
		case RX_DATA1: case RX_DATA2: case RX_DATA3: case RX_DATA4:
		case RX_DATA5: case RX_DATA6: case RX_DATA7: case RX_DATA8:
			// rx data bit
			if (m_rxd)
			{
				m_shift |= 1U << (m_state - RX_DATA1);
				m_parity = !m_parity;
			}

			m_state++;
			break;
		case RX_PARITY:
			// rx parity bit
			if (m_rxd)
				m_parity = !m_parity;

			m_state++;
			break;
		case RX_STOP:
			// rx stop bit
			if (m_rxd)
			{
				LOG("rx 0x%02x\n", m_shift);

				m_latch = m_shift;

				if (!m_parity)
				{
					LOG("rx parity error\n");
					m_pe_cb(1);
				}
			}
			else
				LOG("rx framing error\n");

			m_state = RX_START;

			// signal rx done
			if (m_rxd)
				m_rx_cb(1);
			break;

		case TX_START:
			break;
		case TX_DATA1: case TX_DATA2: case TX_DATA3: case TX_DATA4:
		case TX_DATA5: case TX_DATA6: case TX_DATA7: case TX_DATA8:
			// tx data bit
			LOGMASKED(LOG_RXTX, "txd %u data\n", BIT(m_shift, m_state - TX_DATA1));
			if (BIT(m_shift, m_state - TX_DATA1))
			{
				m_parity = !m_parity;
				m_txd_cb(1);
			}
			else
				m_txd_cb(0);

			m_state++;
			break;
		case TX_PARITY:
			// tx parity bit
			LOGMASKED(LOG_RXTX, "txd %u parity\n", m_parity ? 0 : 1);
			m_txd_cb(m_parity ? 0 : 1);

			m_state++;
			break;
		case TX_STOP:
			// tx stop bit
			LOGMASKED(LOG_RXTX, "txd stop\n");
			m_txd_cb(1);

			m_state++;
			break;
		case TX_ACK:
			// wait keyboard acknowledge
			if (!m_rxd)
			{
				LOGMASKED(LOG_RXTX, "txd ack\n");

				m_state = RX_START;

				// signal tx done
				m_tx_cb(1);
			}
			break;
		}
	}

	m_clk = bool(state);
}

void at_ssrt_device::rxd_w(int state)
{
	// don't log transmitted data
	if (m_state < TX_START)
		LOGMASKED(LOG_RXTX, "rxd %d\n", state);

	m_rxd = bool(state);
}

u8 at_ssrt_device::data_r()
{
	if (!machine().side_effects_disabled())
	{
		LOG("%s: data_r 0x%02x\n", machine().describe_context(), m_latch);

		m_pe_cb(0);
		m_rx_cb(0);
	}

	return m_latch;
}

void at_ssrt_device::data_w(u8 data)
{
	LOG("%s: data_w 0x%02x\n", machine().describe_context(), data);

	// load data latch
	m_latch = data;
	m_parity = false;

	m_state = TX_START;
	m_tx_cb(0);

	// inhibit keyboard for 60µs
	// TODO: test receiver active
	m_clk_cb(0);
	m_timer->adjust(attotime::from_usec(60));
}

void at_ssrt_device::timer(s32 param)
{
	if (m_state == TX_START)
	{
		// load shift register
		m_shift = m_latch;

		// tx start bit
		LOGMASKED(LOG_RXTX, "txd 0 start\n");
		m_txd_cb(0);
		m_state = TX_DATA1;

		// release clock
		m_clk_cb(1);
	}
}

bool at_ssrt_device::busy() const
{
	return (m_state != RX_START);
}

bool at_ssrt_device::rx_busy() const
{
	return (m_state > RX_START && m_state <= RX_STOP);
}

bool at_ssrt_device::tx_busy() const
{
	return (m_state >= TX_START && m_state <= TX_ACK);
}
