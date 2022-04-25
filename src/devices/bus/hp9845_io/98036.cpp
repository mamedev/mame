// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    98036.cpp

    98036 module (RS232 interface)

    Main reference for this module:
    HP, 98036A Serial I/O interface installation & service manual

*********************************************************************/

#include "emu.h"
#include "98036.h"

// Debugging
#define VERBOSE 0
#include "logmacro.h"

// Bit manipulation
namespace {
	template <typename T> constexpr T BIT_MASK(unsigned n)
	{
		return T(1U) << n;
	}

	template <typename T> void BIT_SET(T &w, unsigned n)
	{
		w |= BIT_MASK<T>(n);
	}
}

// device type definitions
DEFINE_DEVICE_TYPE(HP98036_IO_CARD, hp98036_io_card_device, "hp98036", "HP98036 card")

hp98036_io_card_device::hp98036_io_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HP98036_IO_CARD, tag, owner, clock)
	, device_hp9845_io_interface(mconfig, *this)
	, m_uart(*this, "uart")
	, m_clock_gen(*this, "clock_gen")
	, m_rs232(*this, "rs232")
	, m_s1_input(*this, "s1")
	, m_s2_input(*this, "s2")
	, m_s3_input(*this, "s3")
	, m_jumper_input(*this, "jumper")
	, m_loopback_en(*this, "loop")
{
}

hp98036_io_card_device::~hp98036_io_card_device()
{
}

uint16_t hp98036_io_card_device::reg_r(address_space &space, offs_t offset)
{
	uint16_t res = 0;

	switch (offset) {
	case 0:
		// R4IN
		m_r4_dir = false;
		res = m_r4in_data;
		break;

	case 1:
		// R5IN
		if (m_tx_int_en) {
			BIT_SET(res, 0);
		}
		if (m_rx_int_en) {
			BIT_SET(res, 1);
		}
		BIT_SET(res, 4);
		if (m_int_en) {
			BIT_SET(res, 7);
		}
		break;

	case 2:
		// R6IN
		// Bits 3&4: jumpers
		res = (m_jumper_input->read() & 3) << 3;
		if (m_loopback) {
			res |= m_lineout;
		} else {
			if (!m_rs232->dcd_r()) {
				BIT_SET(res, 0);
			}
			if (!m_rs232->ri_r()) {
				BIT_SET(res, 1);
			}
			// Bit 2: secondary CD, always 0
		}
		BIT_SET(res, 5);
		BIT_SET(res, 6);
		BIT_SET(res, 7);
		break;

	default:
		logerror("Unmapped read @%u\n", offset);
	}

	LOG("RD R%u=%02x\n", offset + 4, res);
	return res;
}

void hp98036_io_card_device::reg_w(address_space &space, offs_t offset, uint16_t data)
{
	LOG("WR R%u=%02x\n", offset + 4, data);

	switch (offset) {
	case 0:
		// R4OUT
		m_r4_dir = true;
		m_wrrq = true;
		m_r4out_data = data;
		uart_wr();
		update_flg();
		break;

	case 1:
		// R5OUT
		m_uart_cmd = BIT(data, 0);
		m_tx_int_en = BIT(data, 1);
		m_rx_int_en = BIT(data, 2);
		m_int_en = BIT(data, 7);
		if (BIT(data, 5)) {
			int_reset();
		} else {
			uart_rd();
			uart_wr();
			update_irq();
			update_flg();
		}
		break;

	case 2:
		// R6OUT
		m_half_baud = BIT(data, 4);
		m_lineout = data & 7;
		break;

	case 3:
		// R7OUT
		if (!m_r4_dir) {
			m_rdrq = true;
			uart_rd();
			update_irq();
			update_flg();
		}
		break;

	default:
		logerror("Unmapped write @%u\n", offset);
		break;
	}
}

void hp98036_io_card_device::device_add_mconfig(machine_config &config)
{
	I8251(config, m_uart, 0);
	m_uart->txd_handler().set(FUNC(hp98036_io_card_device::txd_w));
	m_uart->dtr_handler().set(FUNC(hp98036_io_card_device::dtr_w));
	m_uart->rts_handler().set(FUNC(hp98036_io_card_device::rts_w));
	m_uart->rxrdy_handler().set(FUNC(hp98036_io_card_device::rxrdy_w));
	m_uart->txrdy_handler().set(FUNC(hp98036_io_card_device::txrdy_w));

	MC14411(config, m_clock_gen, 1.8432_MHz_XTAL);
	m_clock_gen->out_f<1>().set(FUNC(hp98036_io_card_device::bit_rate_clock));
	m_clock_gen->out_f<3>().set(FUNC(hp98036_io_card_device::bit_rate_clock));
	m_clock_gen->out_f<5>().set(FUNC(hp98036_io_card_device::bit_rate_clock));
	m_clock_gen->out_f<6>().set(FUNC(hp98036_io_card_device::bit_rate_clock));
	m_clock_gen->out_f<7>().set(FUNC(hp98036_io_card_device::bit_rate_clock));
	m_clock_gen->out_f<8>().set(FUNC(hp98036_io_card_device::bit_rate_clock));
	m_clock_gen->out_f<9>().set(FUNC(hp98036_io_card_device::bit_rate_clock));
	m_clock_gen->out_f<11>().set(FUNC(hp98036_io_card_device::bit_rate_clock));
	m_clock_gen->out_f<13>().set(FUNC(hp98036_io_card_device::bit_rate_clock));
	m_clock_gen->out_f<14>().set(FUNC(hp98036_io_card_device::bit_rate_clock));
	m_clock_gen->out_f<12>().set(FUNC(hp98036_io_card_device::slow_clock));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(FUNC(hp98036_io_card_device::rxd_w));
	m_rs232->dsr_handler().set(FUNC(hp98036_io_card_device::dsr_w));
	m_rs232->cts_handler().set(FUNC(hp98036_io_card_device::cts_w));
}

static INPUT_PORTS_START(hp98036_port)
	PORT_HP9845_IO_SC(11)

	PORT_START("s1")
	PORT_DIPNAME(3, 3, "Clock factor")
	PORT_DIPLOCATION("S1:1,2")
	PORT_DIPSETTING(1, "x1")
	PORT_DIPSETTING(2, "x16")
	PORT_DIPSETTING(3, "x64")
	PORT_DIPNAME(0x0c, 0x0c, "Character length")
	PORT_DIPLOCATION("S1:3,4")
	PORT_DIPSETTING(0x00, "5")
	PORT_DIPSETTING(0x04, "6")
	PORT_DIPSETTING(0x08, "7")
	PORT_DIPSETTING(0x0c, "8")
	PORT_DIPNAME(0x10, 0, "Parity enable")
	PORT_DIPLOCATION("S1:5")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x10, DEF_STR(On))
	PORT_DIPNAME(0x20, 0, "Parity")
	PORT_DIPLOCATION("S1:6")
	PORT_DIPSETTING(0x00, "Odd")
	PORT_DIPSETTING(0x20, "Even")
	PORT_DIPNAME(0xc0, 0xc0, "Stop bits")
	PORT_DIPLOCATION("S1:7,8")
	PORT_DIPSETTING(0x40, "1")
	PORT_DIPSETTING(0x80, "1.5")
	PORT_DIPSETTING(0xc0, "2")

	PORT_START("s2")
	PORT_DIPNAME(1, 1, "CTS")
	PORT_DIPLOCATION("S2:1")
	PORT_DIPSETTING(0, "Device controlled")
	PORT_DIPSETTING(1, "Always active")

	PORT_START("s3")
	PORT_CONFNAME(0xf, 6, "Baud rate")
	PORT_CONFSETTING(0, "9600")
	PORT_CONFSETTING(1, "4800")
	PORT_CONFSETTING(2, "2400")
	PORT_CONFSETTING(3, "1800")
	PORT_CONFSETTING(4, "1200")
	PORT_CONFSETTING(5, "600")
	PORT_CONFSETTING(6, "300")
	PORT_CONFSETTING(7, "150")
	PORT_CONFSETTING(8, "110")
	PORT_CONFSETTING(9, "75")

	PORT_START("jumper")
	PORT_CONFNAME(1, 0, "J1")
	PORT_CONFSETTING(0, DEF_STR(Off))
	PORT_CONFSETTING(1, DEF_STR(On))
	PORT_CONFNAME(2, 0, "J2")
	PORT_CONFSETTING(0, DEF_STR(Off))
	PORT_CONFSETTING(2, DEF_STR(On))

	PORT_START("loop")
	PORT_CONFNAME(1, 0, "Test loopback")
	PORT_CONFSETTING(0, DEF_STR(Off))
	PORT_CONFSETTING(1, DEF_STR(On))
INPUT_PORTS_END

ioport_constructor hp98036_io_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hp98036_port);
}

void hp98036_io_card_device::device_start()
{
	save_item(NAME(m_init_state));
	save_item(NAME(m_irq));
	save_item(NAME(m_int_en));
	save_item(NAME(m_tx_int_en));
	save_item(NAME(m_rx_int_en));
	save_item(NAME(m_txrdy));
	save_item(NAME(m_rxrdy));
	save_item(NAME(m_wrrq));
	save_item(NAME(m_rdrq));
	save_item(NAME(m_uart_cmd));
	save_item(NAME(m_uart_wr_mode));
	save_item(NAME(m_flg));
	save_item(NAME(m_half_baud));
	save_item(NAME(m_baud_div));
	save_item(NAME(m_r4_dir));
	save_item(NAME(m_r4in_data));
	save_item(NAME(m_r4out_data));
	save_item(NAME(m_lineout));
}

void hp98036_io_card_device::device_reset()
{
	sts_w(true);

	ioport_value const baud_sel = m_s3_input->read();
	m_clock_gen->timer_enable(mc14411_device::TIMER_F1, baud_sel == 0);
	m_clock_gen->timer_enable(mc14411_device::TIMER_F3, baud_sel == 1);
	m_clock_gen->timer_enable(mc14411_device::TIMER_F5, baud_sel == 2);
	m_clock_gen->timer_enable(mc14411_device::TIMER_F6, baud_sel == 3);
	m_clock_gen->timer_enable(mc14411_device::TIMER_F7, baud_sel == 4);
	m_clock_gen->timer_enable(mc14411_device::TIMER_F8, baud_sel == 5);
	m_clock_gen->timer_enable(mc14411_device::TIMER_F9, baud_sel == 6);
	m_clock_gen->timer_enable(mc14411_device::TIMER_F11, baud_sel == 7);
	m_clock_gen->timer_enable(mc14411_device::TIMER_F13, baud_sel == 8);
	m_clock_gen->timer_enable(mc14411_device::TIMER_F14, baud_sel == 9);

	m_loopback = m_loopback_en->read() != 0;

	if (!m_loopback && BIT(m_s2_input->read(), 0)) {
		m_uart->write_cts(0);
	}

	int_reset();
}

WRITE_LINE_MEMBER(hp98036_io_card_device::bit_rate_clock)
{
	if (m_half_baud) {
		if (state) {
			m_baud_div = !m_baud_div;
			m_uart->write_txc(m_baud_div);
			m_uart->write_rxc(m_baud_div);
		}
	} else {
		m_uart->write_txc(state);
		m_uart->write_rxc(state);
	}
}

WRITE_LINE_MEMBER(hp98036_io_card_device::slow_clock)
{
	switch (m_init_state) {
	case 0:
		// Reset
		if (state) {
			m_init_state++;
		}
		break;

	case 1:
		// Write mode word
		if (state) {
			m_init_state++;
		} else {
			write_uart(true, uint8_t(m_s1_input->read()));
		}
		break;

	case 2:
		// Write default control word
		if (state) {
			m_init_state++;
			update_flg();
		} else {
			write_uart(true, 0x05);
		}
		break;

	default:
		break;
	}
}

WRITE_LINE_MEMBER(hp98036_io_card_device::txd_w)
{
	if (m_loopback) {
		m_uart->write_rxd(state);
	} else {
		m_rs232->write_txd(state);
	}
}

WRITE_LINE_MEMBER(hp98036_io_card_device::dtr_w)
{
	if (m_loopback) {
		m_uart->write_dsr(state);
	} else {
		m_rs232->write_dtr(state);
	}
}

WRITE_LINE_MEMBER(hp98036_io_card_device::rts_w)
{
	if (m_loopback) {
		m_uart->write_cts(state);
	} else {
		m_rs232->write_rts(state);
	}
}

WRITE_LINE_MEMBER(hp98036_io_card_device::txrdy_w)
{
	if (m_txrdy != state) {
		m_txrdy = state;
		LOG("txrdy %d\n", m_txrdy);
		uart_wr();
		update_irq();
		update_flg();
	}
}

WRITE_LINE_MEMBER(hp98036_io_card_device::rxrdy_w)
{
	if (m_rxrdy != state) {
		m_rxrdy = state;
		LOG("rxrdy %d\n", m_rxrdy);
		uart_rd();
		update_irq();
		update_flg();
	}
}

WRITE_LINE_MEMBER(hp98036_io_card_device::rxd_w)
{
	if (!m_loopback) {
		m_uart->write_rxd(state);
	}
}

WRITE_LINE_MEMBER(hp98036_io_card_device::dsr_w)
{
	if (!m_loopback) {
		m_uart->write_dsr(state);
	}
}

WRITE_LINE_MEMBER(hp98036_io_card_device::cts_w)
{
	if (!m_loopback && !BIT(m_s2_input->read(), 0)) {
		m_uart->write_cts(state);
	}
}

void hp98036_io_card_device::int_reset()
{
	m_init_state = 0;
	m_r4in_data = 0;
	m_r4out_data = 0;
	m_wrrq = false;
	m_rdrq = false;
	m_int_en = false;
	m_tx_int_en = false;
	m_rx_int_en = false;
	m_uart_cmd = false;
	m_uart_wr_mode = true;
	m_half_baud = false;
	m_lineout = 0;
	m_clock_gen->rate_select_w(0);
	m_flg = true; // Ensure update in update_flg
	update_flg();
	m_irq = true; // Ensure update in update_irq
	update_irq();
	m_uart->reset();
}

void hp98036_io_card_device::update_flg()
{
	bool new_flg = (m_init_state == 3) && (!m_wrrq || m_tx_int_en) && (!m_rdrq || m_rx_int_en);

	if (m_flg != new_flg) {
		m_flg = new_flg;
		LOG("FLG %d\n", m_flg);
		flg_w(m_flg);
	}
}

void hp98036_io_card_device::update_irq()
{
	bool new_irq = m_int_en && ((m_tx_int_en && m_txrdy) || (m_rx_int_en && !m_rdrq));

	if (m_irq != new_irq) {
		m_irq = new_irq;
		LOG("IRQ %d\n", m_irq);
		irq_w(m_irq);
	}
}

void hp98036_io_card_device::uart_rd()
{
	if (m_rdrq && (m_rxrdy || m_uart_cmd)) {
		m_rdrq = false;
		m_r4in_data = m_uart->read(m_uart_cmd);
		LOG("uart rd%d=%02x\n", m_uart_cmd,  m_r4in_data);
		update_flg();
	}
}

void hp98036_io_card_device::uart_wr()
{
	if (m_wrrq && (m_txrdy || m_uart_cmd)) {
		m_wrrq = false;
		write_uart(m_uart_cmd, m_r4out_data);
		update_flg();
	}
}

void hp98036_io_card_device::write_uart(bool cmd, uint8_t data)
{
	if (cmd && m_uart_wr_mode) {
		// Capture bits 1 & 0 of mode word (clock factor) and
		// adjust rate selection in clock generator
		// Mode word    RSB/RSA Factor
		// 0 0          0 0     -- INVALID --
		// 0 1          0 0     x1
		// 1 0          1 0     x16
		// 1 1          1 1     x64
		m_clock_gen->rate_select_w((BIT(data, 1) ? mc14411_device::RSB : 0) | ((BIT(data, 1) && BIT(data, 0)) ? mc14411_device::RSA : 0));
		LOG("RS %u\n", data & 3);
	}
	if (cmd) {
		if (BIT(data, 6)) {
			m_uart_wr_mode = !m_uart_wr_mode;
		} else {
			m_uart_wr_mode = false;
		}
	}
	LOG("uart wr%d=%02x\n", cmd, data);
	m_uart->write(cmd, data);
}
