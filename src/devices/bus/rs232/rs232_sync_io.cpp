// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    rs232_sync_io.cpp

    Synchronous I/O on RS232 port

    This device provides a bitbanger-based interface to a synchronous
    RS232 port.
    It has the following functions:
    - Provides Tx and/or Rx clock to RS232 port; clocks can be
      individually turned off when not needed (e.g. Tx clock can
      disabled when receiving in half-duplex mode)
    - In the tx direction, it gathers bits from TxD,
      packs them into bytes LSB to MSB, then sends them out to
      bitbanger
    - In the rx direction, it extracts bytes from bitbanger
      and serializes them to RxD, LSB to MSB.
    - Supports both full-duplex and half-duplex modes
    - Generates CTS signal

    There's no kind of synchronization to any higher-level framing:
    bits are just clocked in/out of the port, packed/unpacked into
    bytes and exchanged with bitbanger.
    There's also no automatic stuffing of idle line characters, this
    function is to be implemented externally if needed.

*********************************************************************/

#include "emu.h"
#include "rs232_sync_io.h"

// Debugging

#include "logmacro.h"
//#undef VERBOSE
//#define VERBOSE LOG_GENERAL

// Bit manipulation
namespace {
	template<typename T> constexpr T BIT_MASK(unsigned n)
	{
		return (T)1U << n;
	}

	template<typename T> void BIT_CLR(T& w , unsigned n)
	{
		w &= ~BIT_MASK<T>(n);
	}

	template<typename T> void BIT_SET(T& w , unsigned n)
	{
		w |= BIT_MASK<T>(n);
	}
}

// Timers
enum {
	TMR_ID_CLK
};

rs232_sync_io_device::rs232_sync_io_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig , RS232_SYNC_IO , tag , owner , clock)
	, device_rs232_port_interface(mconfig , *this)
	, m_stream(*this , "stream")
	, m_rs232_baud(*this , "RS232_BAUD")
	, m_rts_duplex(*this , "rtsduplex")
	, m_txc_setting(*this , "txcontrol")
	, m_rxc_setting(*this , "rxcontrol")
{
}

rs232_sync_io_device:: ~rs232_sync_io_device()
{
}

WRITE_LINE_MEMBER(rs232_sync_io_device::input_txd)
{
	m_txd = state;
	LOG("TxD %d %u\n" , state , m_tx_counter);
}

WRITE_LINE_MEMBER(rs232_sync_io_device::input_rts)
{
	m_rts = state;
}

static INPUT_PORTS_START(rs232_sync_io)
	PORT_RS232_BAUD("RS232_BAUD" , RS232_BAUD_2400 , "Baud rate" , rs232_sync_io_device , update_serial)

	PORT_START("rtsduplex")
	PORT_CONFNAME(1 , 1 , "RTS controls half-duplex")
	PORT_CONFSETTING(0 , DEF_STR(Off))
	PORT_CONFSETTING(1 , DEF_STR(On))

	PORT_START("txcontrol")
	PORT_CONFNAME(1 , 1 , "TxC generated")
	PORT_CONFSETTING(0 , "Always")
	PORT_CONFSETTING(1 , "When Tx is active")

	PORT_START("rxcontrol")
	PORT_CONFNAME(3 , 1 , "RxC generated")
	PORT_CONFSETTING(0 , "Always")
	PORT_CONFSETTING(1 , "When Rx is active")
	PORT_CONFSETTING(2 , "When a byte is available")
INPUT_PORTS_END

ioport_constructor rs232_sync_io_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(rs232_sync_io);
}

void rs232_sync_io_device::device_add_mconfig(machine_config &config)
{
	BITBANGER(config , m_stream , 0);
}

void rs232_sync_io_device::device_start()
{
	m_clk_timer = timer_alloc(TMR_ID_CLK);
}

void rs232_sync_io_device::device_reset()
{
	output_dcd(0);
	output_dsr(0);
	output_rxd(1);
	output_cts(1);

	update_serial(0);

	m_clk = false;
	m_rx_byte = 0xff;
	m_rx_counter = 0;
	m_tx_counter = 0;
	m_tx_enabled = false;
	m_rx_enabled = false;

	output_txc(1);
	output_rxc(1);
}

void rs232_sync_io_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id) {
	case TMR_ID_CLK:
		m_clk = !m_clk;
		if (m_clk) {
			output_txc(1);
			output_rxc(1);
			if (m_tx_enabled) {
				// Rising edge: capture TxD
				m_tx_byte = (m_tx_byte >> 1) & 0x7f;
				if (m_txd) {
					BIT_SET(m_tx_byte , 7);
				}
				if (++m_tx_counter >= 8) {
					LOG("Tx %02x @%s\n" , m_tx_byte , machine().time().to_string());
					m_tx_counter = 0;
					m_stream->output(m_tx_byte);
					m_tx_enabled = false;
				}
			}
		} else {
			if (!m_tx_enabled) {
				m_tx_enabled = !m_rts || !m_rts_duplex->read();
				output_cts(!m_tx_enabled);
			}
			if (m_tx_enabled || m_txc_setting->read() == 0) {
				output_txc(0);
			}
			if (!m_rx_enabled) {
				m_rx_enabled = m_rts || !m_rts_duplex->read();
				if (m_rx_enabled) {
					if (m_stream->input(&m_rx_byte , 1) == 0) {
						m_rx_byte = ~0;
						if (m_rxc_setting->read() == 2) {
							m_rx_enabled = false;
						}
					} else {
						LOG("Rx %02x @%s\n" , m_rx_byte , machine().time().to_string());
					}
				}
			}
			if (m_rx_enabled || m_rxc_setting->read() == 0) {
				output_rxc(0);
			}
			if (m_rx_enabled) {
				// Falling edge: update RxD
				output_rxd(BIT(m_rx_byte , 0));
				m_rx_byte >>= 1;
				if (++m_rx_counter >= 8) {
					m_rx_counter = 0;
					m_rx_enabled = false;
				}
			}
		}
		break;

	default:
		break;
	}
}

WRITE_LINE_MEMBER(rs232_sync_io_device::update_serial)
{
	auto baud_rate = convert_baud(m_rs232_baud->read());
	auto period = attotime::from_hz(baud_rate * 2);
	m_clk_timer->adjust(period , 0 , period);
}

DEFINE_DEVICE_TYPE(RS232_SYNC_IO, rs232_sync_io_device, "rs232_sync_io", "RS232 Synchronous I/O")
