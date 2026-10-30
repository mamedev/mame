// license:BSD-3-Clause
// copyright-holders:Dave L. Rand
/***************************************************************************

    Toshiba T-200 / T-250 CCM (Comms Control Module)

    Intel 8251 USART + Intel 8253 PIT (baud) + RS-232 connector on a separate
    card edge.  See t250_ccm.h.

***************************************************************************/

#include "emu.h"
#include "t250_ccm.h"


// Default mode for an attached terminal: 19200 8N1 (matching the BIOS' default
// mode byte 0x6E).  Users can override per-session via -ccm:rs232:rxbaud etc.
static DEVICE_INPUT_DEFAULTS_START(terminal)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD",   0xff, RS232_BAUD_19200)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD",   0xff, RS232_BAUD_19200)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY",   0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END


DEFINE_DEVICE_TYPE(T250_CCM, t250_ccm_device, "t250_ccm", "Toshiba T-200/T-250 CCM (Comms Control Module)")

t250_ccm_device::t250_ccm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, T250_CCM, tag, owner, clock)
	, m_pit(*this, "pit")
	, m_usart(*this, "usart")
	, m_rs232(*this, "rs232")
	, m_rxrdy_cb(*this)
	, m_txrdy_cb(*this)
	, m_rxrdy_state(false)
	, m_txrdy_state(false)
	, m_usart_inited(false)
	, m_cts(true)
	, m_ri(false)
{
}

void t250_ccm_device::device_start()
{
	save_item(NAME(m_rxrdy_state));
	save_item(NAME(m_txrdy_state));
	save_item(NAME(m_usart_inited));
	save_item(NAME(m_cts));
	save_item(NAME(m_ri));
}

void t250_ccm_device::device_reset()
{
	m_usart->write_cts(0);  // 8251 /CTS tied low on real HW; BIOS handles flow via port 0xA7
	m_rxrdy_state = false;
	m_txrdy_state = false;
	m_usart_inited = false;
	m_cts = true;
	m_ri  = false;
}

void t250_ccm_device::device_add_mconfig(machine_config &config)
{
	// CCM-board baud clock is 1.9968 MHz (15.9744 MHz crystal / 8), not the
	// standard 1.8432 MHz async baud crystal.  Per the set utility's divisor
	// table: 19200 -> 6 gives 1996800/6/16 = 20800 (~8% high; lower rates exact).
	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(15.9744_MHz_XTAL / 8);
	m_pit->set_clk<1>(15.9744_MHz_XTAL / 8);
	m_pit->set_clk<2>(15.9744_MHz_XTAL / 8);
	m_pit->out_handler<2>().set(m_usart, FUNC(i8251_device::write_txc));
	m_pit->out_handler<2>().append(m_usart, FUNC(i8251_device::write_rxc));

	I8251(config, m_usart, 1'843'200);
	m_usart->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_usart->dtr_handler().set(m_rs232, FUNC(rs232_port_device::write_dtr));
	m_usart->rts_handler().set(m_rs232, FUNC(rs232_port_device::write_rts));
	m_usart->rxrdy_handler().set(FUNC(t250_ccm_device::usart_rxrdy_w));
	m_usart->txrdy_handler().set(FUNC(t250_ccm_device::usart_txrdy_w));

	// The RS-232 connector lives on the CCM card edge, so the port is a
	// sub-device of the CCM (command-line tag -ccm:rs232).
	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->set_option_device_input_defaults("terminal",   DEVICE_INPUT_DEFAULTS_NAME(terminal));
	m_rs232->set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(terminal));
	m_rs232->rxd_handler().set(m_usart, FUNC(i8251_device::write_rxd));
	m_rs232->dsr_handler().set(m_usart, FUNC(i8251_device::write_dsr));
	m_rs232->cts_handler().set(FUNC(t250_ccm_device::rs232_cts_w));
	m_rs232->ri_handler().set(FUNC(t250_ccm_device::rs232_ri_w));
}

void t250_ccm_device::usart_rxrdy_w(int state)
{
	m_rxrdy_state = state;
	m_rxrdy_cb(state);
}

void t250_ccm_device::usart_txrdy_w(int state)
{
	m_txrdy_state = state;
	m_txrdy_cb(state);
}

// CCM CI/CTS status register (0xA7).  BIOS reads bit 5 to test /CTS
// (active low: 0 = ready to send).
u8 t250_ccm_device::cicts_r()
{
	u8 d = 0xff;
	if (m_cts) d &= ~0x20;
	return d;
}

// CCM mode register (0xA6).  BIOS init pulses 0x18 then 0x00 ("asyn - rxdinh -
// txdwa" sequence per handler.lib).  Exact function not modeled — no observable
// side-effect on emulated traffic, so writes are dropped.
void t250_ccm_device::mode_w(u8 data)
{
}

// MAME RS232 convention: cts_handler is called with state=0 for "CTS asserted"
// (clear to send) and state=1 for deasserted.  Track the inverted value so
// m_cts == true means "asserted" (matches cicts_r / ios_r consumers).  Same
// for RI.
void t250_ccm_device::rs232_cts_w(int state) { m_cts = !state; }
void t250_ccm_device::rs232_ri_w(int state)  { m_ri  = !state; }

// 8251 write port shim.  BIOS initiu writes a stale A=0x18 to the 8251 data
// register (port 0xA4) *before* having set up the mode byte.  On real hardware
// the 8251 ignores CPU writes until properly initialized, but MAME's i8251
// accepts the byte and locks TX_READY=0 forever because TxEN is also 0 at that
// moment and start_tx() never fires.  We drop data-port writes until the BIOS
// sets the mode byte (any write to 0xA5 after a reset / internal reset).
// Control-port writes pass through normally so the 8251 can transition through
// its mode/cmd init sequence.
void t250_ccm_device::usart_w(offs_t offset, u8 data)
{
	if (offset == 0)  // 0xA4 = data port
	{
		if (!m_usart_inited)
			return;
	}
	else  // 0xA5 = control port
	{
		m_usart_inited = true;
	}
	m_usart->write(offset, data);
}
