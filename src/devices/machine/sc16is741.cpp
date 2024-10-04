// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 I²C/SPI UART with 64-byte transmit and receive FIFOs

                _______
        VDD  1 |*      | 16  XTAL1
   A0   _CS  2 |       | 15  XTAL2
   A1    SI  3 |       | 14  _RESET
 n.c.    SO  4 |       | 13  RX
  SCL  SCLK  5 |       | 12  TX
  SDA   VSS  6 |       | 11  _CTS
       _IRQ  7 |       | 10  _RTS
  I2C  _SPI  8 |_______|  9  VSS

 Partially software-compatible with the ubiquitous 16C450.

 TODO:
 * When are registers considered "read" for side effects?
 * The rest of the registers
 * The rest of the interrupts
 * 9-bit mode
 * Xon/Xoff handshaking
 * Special character detect
 * Loopback
 * Break detection
 * IrDA mode
 * I²C interface
 * Sleep mode
 * SC16IS741 differences
 */
#include "emu.h"
#include "sc16is741.h"

//#define VERBOSE 1
#include "logmacro.h"


namespace {

#define IER_CTS_INT()           (BIT(m_ier, 7))
#define IER_RTS_INT()           (BIT(m_ier, 6))
#define IER_XOFF_INT()          (BIT(m_ier, 5))
#define IER_SLEEP_MODE()        (BIT(m_ier, 4))
#define IER_MODEM_STATUS_INT()  (BIT(m_ier, 3))
#define IER_LINE_STATUS_INT()   (BIT(m_ier, 2))
#define IER_THR_INT()           (BIT(m_ier, 1))
#define IER_RHR_INT()           (BIT(m_ier, 0))

#define FCR_RX_TRIGGER()        (BIT(m_fcr, 6, 2))
#define FCR_TX_TRIGGER()        (BIT(m_fcr, 4, 2))
#define FCR_FIFO_ENABLE()       (BIT(m_fcr, 0))

#define LCR_DL_ENABLE()         (BIT(m_lcr, 7))
#define LCR_BREAK()             (BIT(m_lcr, 6))
#define LCR_SET_PARITY()        (BIT(m_lcr, 5))
#define LCR_EVEN_PARITY()       (BIT(m_lcr, 4))
#define LCR_PARITY_ENABLE()     (BIT(m_lcr, 3))
#define LCR_STOP_BIT()          (BIT(m_lcr, 2))

#define MCR_CLOCK_DIV4()        (BIT(m_mcr, 7))
#define MCR_TCR_TLR_ENABLE()    (BIT(m_mcr, 2))

#define TCR_LEVEL_RESUME()      (BIT(m_tcr, 4, 4))
#define TCR_LEVEL_HALT()        (BIT(m_tcr, 0, 4))

#define EFR_AUTO_CTS()          (BIT(m_efr, 7))
#define EFR_AUTO_RTS()          (BIT(m_efr, 6))
#define EFR_ENHANCED()          (BIT(m_efr, 4))


constexpr u8 RX_TRIGGER_LEVELS[4] = { 8, 16, 56, 60 };
constexpr u8 TX_TRIGGER_LEVELS[4] = { 8, 16, 32, 56 };

char const *const SOFT_FLOW_CONTROL_DESC[16] = {
		"no soft transmit flow control, no soft receive flow control",
		"no soft transmit flow control, receiver compares Xon2, Xoff2",
		"no soft transmit flow control, receiver compares Xon1, Xoff1",
		"no soft transmit flow control, receiver compares Xon1 and Xon2, Xoff1 and Xoff2",
		"transmit Xon2, Xoff2, no soft receive flow control",
		"transmit Xon2, Xoff2, receiver compares Xon2, Xoff2",
		"transmit Xon2, Xoff2, receiver compares Xon1, Xoff1",
		"transmit Xon2, Xoff2, receiver compares Xon1 or Xon2, Xoff1 or Xoff2",
		"transmit Xon1, Xoff1, no soft receive flow control",
		"transmit Xon1, Xoff1, receiver compares Xon2, Xoff2",
		"transmit Xon1, Xoff1, receiver compares Xon1, Xoff1",
		"transmit Xon1, Xoff1, receiver compares Xon1 or Xon2, Xoff1 or Xoff2",
		"transmit Xon1 and Xon2, Xoff1 and Xoff2, no soft receive flow control",
		"transmit Xon1 and Xon2, Xoff1 and Xoff2, receiver compares Xon2, Xoff2",
		"transmit Xon1 and Xon2, Xoff1 and Xoff2, receiver compares Xon1, Xoff1",
		"transmit Xon1 and Xon2, Xoff1 and Xoff2, receiver compares Xon1 and Xon2, Xoff1 and Xoff2" };

} // anonymous namespace


DEFINE_DEVICE_TYPE(SC16IS741A, sc16is741a_device, "sc16is741a", "NXP SC16IS741A UART")


ALLOW_SAVE_TYPE(sc16is741a_device::phase);

enum class sc16is741a_device::phase : u8
{
	IDLE,
	COMMAND,
	WRITE,
	READ
};


enum class sc16is741a_device::parity : u8
{
	NONE,
	ODD,
	EVEN,
	MARK,
	SPACE
};


enum sc16is741a_device::interrupt : u8
{
	INTERRUPT_LINE_STATUS   = 0x80,
	INTERRUPT_RX_TIMEOUT    = 0x40,
	INTERRUPT_RHR           = 0x20,
	INTERRUPT_THR           = 0x10,
	INTERRUPT_MODEM_STATUS  = 0x08,
	INTERRUPT_XOFF          = 0x04,
	INTERRUPT_SPECIAL_CHAR  = 0x02,
	INTERRUPT_CTS_RTS       = 0x01
};


sc16is741a_device::sc16is741a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SC16IS741A, tag, owner, clock),
	m_so_cb(*this),
	m_irq_cb(*this),
	m_tx_cb(*this),
	m_rts_cb(*this),
	m_shift_timer{ nullptr, nullptr },
	m_rx_timeout_timer(nullptr)
{
}

sc16is741a_device::~sc16is741a_device()
{
}


void sc16is741a_device::sclk_w(int state)
{
	if ((phase::COMMAND == m_phase) || (phase::WRITE == m_phase))
	{
		if (state && !m_sclk)
		{
			m_buffer = (m_buffer << 1) | m_si;
			if (!--m_bits)
			{
				if (phase::COMMAND == m_phase)
				{
					m_command = m_buffer;
					if (BIT(m_buffer, 7))
					{
						m_phase = phase::READ;
						reg_r(true);
					}
					else
					{
						m_phase = phase::WRITE;
					}
				}
				else
				{
					reg_w();
				}
				m_bits = 8;
			}
		}
	}
	else if (phase::READ == m_phase)
	{
		if (!state && m_sclk)
		{
			m_so_cb(BIT(m_buffer, 7));
		}
		else if (state && !m_sclk)
		{
			m_buffer = (m_buffer << 1) | (m_buffer >> 7);
			--m_bits;
			if (!m_bits)
			{
				reg_r(false);
				m_bits = 8;
			}
			else if (7 == m_bits)
			{
				if ((BIT(m_command, 3, 4) == 0x00) && ((0xbf == m_lcr) || !LCR_DL_ENABLE()))
					pop_rx_fifo();
			}
		}
	}
	m_sclk = state ? 1 : 0;
}

void sc16is741a_device::cs_w(int state)
{
	if (state)
	{
		m_phase = phase::IDLE;
		m_so_cb(1);
	}
	else if (m_cs)
	{
		m_phase = phase::COMMAND;
		m_bits = 8;
	}
	m_cs = state ? 1 : 0;
}

void sc16is741a_device::si_w(int state)
{
	m_si = state ? 1 : 0;
}

void sc16is741a_device::rx_w(int state)
{
	if (m_divisor) // FIXME: check EFCR[1]
	{
		if (!m_rx_remain)
		{
			if (m_rx && !state)
			{
				// start bit
				m_rx_remain = m_rx_intervals;
				m_rx_count = 0;
				m_shift_timer[0]->adjust(attotime::from_ticks(m_divisor * 16 / 2, clock()));
			}
		}
		else if (!m_rx_count)
		{
			if (state)
			{
				// false start
				m_rx_remain = 0;
				m_shift_timer[0]->reset();
			}
		}
	}
	m_rx = state ? 1 : 0;
}

void sc16is741a_device::cts_w(int state)
{
	bool const asserted(EFR_AUTO_CTS() && !state && m_cts);
	if (bool(state) != bool(m_cts))
	{
		m_interrupts |= INTERRUPT_MODEM_STATUS;
		if (state && IER_CTS_INT() && !(m_interrupts & INTERRUPT_CTS_RTS))
		{
			LOG("CTS deasserted, setting CTS interrupt\n");
			m_interrupts |= INTERRUPT_CTS_RTS;
		}
		update_irq();
	}
	m_cts = state ? 1 : 0;
	if (asserted)
		check_tx();
}


void sc16is741a_device::device_resolve_objects()
{
	m_tx = 1;
	m_rx = 1;
	m_cts = 0;
	m_sclk = 0;
	m_cs = 1;
	m_si = 1;
	m_bits = 0;
	m_buffer = 0;
}

void sc16is741a_device::device_start()
{
	m_shift_timer[0] = timer_alloc(FUNC(sc16is741a_device::rx_shift), this);
	m_shift_timer[1] = timer_alloc(FUNC(sc16is741a_device::tx_shift), this);
	m_rx_timeout_timer = timer_alloc(FUNC(sc16is741a_device::rx_timeout), this);

	m_spr = 0x00;
	m_dl = 0x0000;
	std::fill(std::begin(m_xon_xoff), std::end(m_xon_xoff), 0);
	m_tx_count = 0;
	m_rx_count = 0;
	for (auto &data : m_fifo_data)
		std::fill(std::begin(data), std::end(data), 0);
	m_divisor = 0;

	save_item(NAME(m_irq));
	save_item(NAME(m_tx));
	save_item(NAME(m_rts));
	save_item(NAME(m_rx));
	save_item(NAME(m_cts));
	save_item(NAME(m_sclk));
	save_item(NAME(m_cs));
	save_item(NAME(m_si));
	save_item(NAME(m_phase));
	save_item(NAME(m_bits));
	save_item(NAME(m_buffer));
	save_item(NAME(m_command));
	save_item(NAME(m_ier));
	save_item(NAME(m_fcr));
	save_item(NAME(m_lcr));
	save_item(NAME(m_mcr));
	save_item(NAME(m_spr));
	save_item(NAME(m_tcr));
	save_item(NAME(m_tlr));
	save_item(NAME(m_dl));
	save_item(NAME(m_efr));
	save_item(NAME(m_xon_xoff));
	save_item(NAME(m_shift_reg));
	save_item(NAME(m_rx_remain));
	save_item(NAME(m_rx_count));
	save_item(NAME(m_tx_remain));
	save_item(NAME(m_tx_count));
	save_item(NAME(m_fifo_head));
	save_item(NAME(m_fifo_tail));
	save_item(NAME(m_fifo_empty));
	save_item(NAME(m_fifo_data));
	save_item(NAME(m_fifo_errors));
	save_item(NAME(m_interrupts));
}

void sc16is741a_device::device_reset()
{
	m_shift_timer[0]->reset();
	m_shift_timer[1]->reset();
	m_rx_timeout_timer->reset();

	m_phase = phase::IDLE;

	m_ier = 0x00;
	m_fcr = 0x00;
	m_lcr = 0x1d;
	m_mcr = 0x00;
	m_tcr = 0x00;
	m_tlr = 0x00;
	m_efr = 0x00;

	std::fill(std::begin(m_shift_reg), std::end(m_shift_reg), 0xffff);
	m_rx_remain = 0;
	m_tx_remain = 0;

	std::fill(std::begin(m_fifo_tail), std::end(m_fifo_tail), 0);
	fifo_reset(0);
	fifo_reset(1);
	m_fifo_errors = 0;

	m_interrupts = 0x00;

	update_trigger_levels();
	update_data_frame();
	update_divisor();

	m_irq_cb(m_irq = CLEAR_LINE);
	m_tx_cb(m_tx = 1);
	m_rts_cb(m_rts = 1);
}

void sc16is741a_device::device_post_load()
{
	update_trigger_levels();
	update_data_frame();
	update_divisor();
}


inline void sc16is741a_device::update_irq()
{
	bool const pending(
			(IER_LINE_STATUS_INT()              && (m_interrupts & INTERRUPT_LINE_STATUS)) ||
			(IER_MODEM_STATUS_INT()             && (m_interrupts & INTERRUPT_MODEM_STATUS)) ||
			(IER_RHR_INT()                      && (m_interrupts & (INTERRUPT_RX_TIMEOUT | INTERRUPT_RHR))) ||
			((IER_CTS_INT() || IER_RTS_INT())   && (m_interrupts & INTERRUPT_CTS_RTS)));
	if (pending != (ASSERT_LINE == m_irq))
	{
		LOG(pending ? "asserting IRQ\n" : "deasserting IRQ\n");
		m_irq_cb(m_irq = (pending ? ASSERT_LINE : CLEAR_LINE));
	}
}

inline void sc16is741a_device::update_tx()
{
	u8 const state(LCR_BREAK() ? 0 : BIT(m_shift_reg[1], 0));
	if (state != m_tx)
		m_tx_cb(m_tx = state);
}

inline void sc16is741a_device::set_rts(u8 state)
{
	if (state != m_rts)
		m_rts_cb(m_rts = state);
}


inline void sc16is741a_device::reg_r(bool first)
{
	u8 const ch(BIT(m_command, 1, 2));
	u8 const addr(BIT(m_command, 3, 4));

	// must be zero
	if (0 != ch)
	{
		if (first)
			logerror("read from unsupported ch %1$u register address 0x%2$02x\n", ch, addr);
		m_buffer = 0xff;
		return;
	}

	switch (addr)
	{
	case 0x00:
		if ((0xbf != m_lcr) && LCR_DL_ENABLE())
			m_buffer = BIT(m_dl, 0, 8);
		else
			m_buffer = m_fifo_data[0][m_fifo_tail[0]];
		return;
	case 0x01:
		if ((0xbf != m_lcr) && LCR_DL_ENABLE())
			m_buffer = BIT(m_dl, 8, 8);
		else
			m_buffer = m_ier;
		return;
	case 0x02:
		if (0xbf == m_lcr)
			m_buffer = m_efr;
		else
			iir_r(first);
		return;
	case 0x03:
		m_buffer = m_lcr;
		return;
	case 0x04:
		if (0xbf == m_lcr)
			xon_xoff_r(first);
		else
			m_buffer = m_mcr;
		return;
	case 0x05:
		if (0xbf == m_lcr)
			xon_xoff_r(first);
		else
			lsr_r(first);
		return;
	case 0x06:
		if (0xbf == m_lcr)
			xon_xoff_r(first);
		else if (MCR_TCR_TLR_ENABLE() && EFR_ENHANCED())
			m_buffer = m_tcr;
		else
			msr_r(first);
		return;
	case 0x07:
		if (0xbf == m_lcr)
			xon_xoff_r(first);
		else if (MCR_TCR_TLR_ENABLE() && EFR_ENHANCED())
			m_buffer = m_tcr;
		else
			m_buffer = m_spr;
		return;
	case 0x08:
		txlvl_r(first);
		return;
	case 0x09:
		rxlvl_r(first);
		return;
	}

	if (first)
		logerror("read from unimplemented register address 0x%1$02x\n", addr);
	m_buffer = 0xff;
}

inline void sc16is741a_device::reg_w()
{
	u8 const ch(BIT(m_command, 1, 2));
	u8 const addr(BIT(m_command, 3, 4));

	// must be zero
	if (0 != ch)
	{
		logerror("write to unsupported ch %1$u register address 0x%2$02x = 0x%3$02x\n", ch, addr, m_buffer);
		return;
	}

	switch (addr)
	{
	case 0x00:
		if ((0xbf != m_lcr) && LCR_DL_ENABLE())
			dl_w();
		else
			thr_w();
		return;
	case 0x01:
		if ((0xbf != m_lcr) && LCR_DL_ENABLE())
			dl_w();
		else
			ier_w();
		return;
	case 0x02:
		if (0xbf == m_lcr)
			efr_w();
		else
			fcr_w();
		return;
	case 0x03:
		lcr_w();
		return;
	case 0x04:
		if (0xbf == m_lcr)
			xon_xoff_w();
		else
			mcr_w();
		return;
	case 0x05:
		if (0xbf == m_lcr)
			xon_xoff_w();
		else
			break; // LSR is read-only
		return;
	case 0x06:
		if (0xbf == m_lcr)
			xon_xoff_w();
		else if (MCR_TCR_TLR_ENABLE() && EFR_ENHANCED())
			tcr_w();
		else
			break; // MSR is read-only
		return;
	case 0x07:
		if (0xbf == m_lcr)
			xon_xoff_w();
		else if (MCR_TCR_TLR_ENABLE() && EFR_ENHANCED())
			tlr_w();
		else
			m_spr = m_buffer;
		return;
	case 0x0d:
		reserved_w();
		return;
	case 0x0e:
		uart_reset_w();
		return;
	}

	logerror("write to unimplemented register address 0x%1$02x = 0x%2$02x\n", addr, m_buffer);
}


inline void sc16is741a_device::iir_r(bool first)
{
	if (first)
	{
		m_buffer = BIT(m_fcr, 0) ? 0xc0 : 0x00;
		if (!m_irq)
		{
			m_buffer |= 0x01;
		}
		else if (IER_LINE_STATUS_INT() && (m_interrupts & INTERRUPT_LINE_STATUS))
		{
			m_buffer |= 0x06;
		}
		else if (IER_RHR_INT() && (m_interrupts & INTERRUPT_RX_TIMEOUT))
		{
			m_buffer |= 0x0c;
		}
		else if (IER_RHR_INT() && (m_interrupts & INTERRUPT_RHR))
		{
			m_buffer |= 0x04;
		}
		else if (IER_THR_INT() && (m_interrupts & INTERRUPT_THR))
		{
			m_buffer |= 0x02;

			LOG("clearing THR interrupt\n");
			m_interrupts &= ~INTERRUPT_THR;
		}
		else if (IER_MODEM_STATUS_INT() && (m_interrupts & INTERRUPT_MODEM_STATUS))
		{
			m_buffer |= 0x00;
		}
		else if ((IER_CTS_INT() || IER_RTS_INT()) && (m_interrupts & INTERRUPT_CTS_RTS))
		{
			m_buffer |= 0x20;

			LOG("clearing CTS/RTS interrupt\n");
			m_interrupts &= ~INTERRUPT_CTS_RTS;
		}

		LOG("read IIR (0x%1$02x)\n", m_buffer);
	}
}

inline void sc16is741a_device::lsr_r(bool first)
{
	m_buffer =
			(m_fifo_errors                      ? 0x80 : 0x00) |
			((m_fifo_empty[1] && !m_tx_remain)  ? 0x40 : 0x00) |
			(m_fifo_empty[1]                    ? 0x20 : 0x00) |
			(!m_fifo_empty[0]                   ? 0x01 : 0x00);
	if (!m_fifo_empty[0])
		m_buffer |= m_fifo_data[1][m_fifo_tail[0]];

	if (first)
		LOG("read LSR (0x%1$02x)\n", m_buffer);
}

inline void sc16is741a_device::msr_r(bool first)
{
	if (first)
	{
		m_buffer =
				(!m_cts                                     ? 0x10 : 0x00) |
				((m_interrupts & INTERRUPT_MODEM_STATUS)    ? 0x01 : 0x00);
		m_interrupts &= ~INTERRUPT_MODEM_STATUS;

		LOG("read MSR (0x%1$02x)\n", m_buffer);
	}
}

inline void sc16is741a_device::txlvl_r(bool first)
{
	m_buffer = fifo_spaces(1);

	if (first)
		LOG("read TXLVL (0x%1$02x)\n", m_buffer);
}

inline void sc16is741a_device::rxlvl_r(bool first)
{
	m_buffer = fifo_fill_level(0);

	if (first)
		LOG("read RXLVL (0x%1$02x)\n", m_buffer);
}

inline void sc16is741a_device::xon_xoff_r(bool first)
{
	m_buffer = m_xon_xoff[BIT(m_command, 3, 2)];

	if (first)
		LOG("read %1$s%2$u (0x%3$02x)\n", BIT(m_command, 4) ? "XOFF" : "XON", BIT(m_command, 3) + 1, m_buffer);
}


inline void sc16is741a_device::thr_w()
{
	m_fifo_data[2][fifo_push(1)] = m_buffer;

	if (m_interrupts & INTERRUPT_THR)
	{
		LOG("THR written, clearing THR interrupt\n");
		m_interrupts &= ~INTERRUPT_THR;
	}

	check_tx();
	update_irq(); // doing this here avoids a glitch if the FIFO is immediately popped
}

inline void sc16is741a_device::ier_w()
{
	LOG(EFR_ENHANCED()
				? "IER = 0x%1$02x (CTS interrupt %2$s, RTS interrupt %3$s, Xoff interrupt %4$s, sleep mode %5$s, modem status interrupt %6$s, RX status interrupt %7$s, THR interrupt %8$s, RHR interrupt %9$s)\n"
				: "IER = 0x%1$02x (modem status interrupt %6$s, RX status interrupt %7$s, THR interrupt %8$s, RHR interrupt %9$s)\n",
			m_buffer & (EFR_ENHANCED() ? 0xff : 0x0f),
			BIT(m_buffer, 7) ? "enabled" : "disabled",
			BIT(m_buffer, 6) ? "enabled" : "disabled",
			BIT(m_buffer, 5) ? "enabled" : "disabled",
			BIT(m_buffer, 4) ? "enabled" : "disabled",
			BIT(m_buffer, 3) ? "enabled" : "disabled",
			BIT(m_buffer, 2) ? "enabled" : "disabled",
			BIT(m_buffer, 1) ? "enabled" : "disabled",
			BIT(m_buffer, 0) ? "enabled" : "disabled");

	if (EFR_ENHANCED())
		m_ier = m_buffer;
	else
		m_ier = (m_ier & 0xf0) | (m_buffer & 0x0f);
	update_irq();
}

inline void sc16is741a_device::fcr_w()
{
	LOG(EFR_ENHANCED()
				? "FCR = 0x%1$02x (RX trigger %2$u, TX trigger %3$u, reserved %4$u, %5$sTX FIFO reset, %6$sRX FIFO reset, FIFO %7$s)\n"
				: "FCR = 0x%1$02x (RX trigger %2$u, reserved %4$u, %5$sTX FIFO reset, %6$sRX FIFO reset, FIFO %7$s)\n",
			m_buffer & (EFR_ENHANCED() ? 0xff : 0xcf),
			RX_TRIGGER_LEVELS[BIT(m_buffer, 6, 2)],
			TX_TRIGGER_LEVELS[BIT(m_buffer, 4, 2)],
			BIT(m_buffer, 3),
			BIT(m_buffer, 2) ? "" : "no ",
			BIT(m_buffer, 1) ? "" : "no ",
			BIT(m_buffer, 0) ? "enabled" : "disabled");

	if (BIT(m_buffer, 3))
		logerror("reserved bit FCR[3] is set\n");

	if (BIT(m_buffer, 2))
		fifo_reset(1);

	if (BIT(m_buffer, 1))
	{
		fifo_reset(0);
		m_fifo_errors = 0;
		m_interrupts &= ~(INTERRUPT_LINE_STATUS | INTERRUPT_RX_TIMEOUT | INTERRUPT_RHR);
		if (EFR_AUTO_RTS() && m_rts) // FIXME: check EFCR[4]
		{
			LOG("RX FIFO reset, asserting RTS\n");
			set_rts(0);
		}
		update_irq();
	}

	if (EFR_ENHANCED())
		m_fcr = m_buffer & 0xf9;
	else
		m_fcr = (m_fcr & 0x30) | (m_buffer & 0xc9);
	update_trigger_levels();
}

inline void sc16is741a_device::lcr_w()
{
	LOG("LCR = 0x%1$02x (divisor latch %2$s, %3$sbreak, %4$s parity %5$s, %6$s stop bits, word length %7$u)\n",
			m_buffer,
			BIT(m_buffer, 7) ? "enabled" : "disabled",
			BIT(m_buffer, 6) ? "" : "no ",
			BIT(m_buffer, 5) ? (BIT(m_buffer, 4) ? "0" : "1") : (BIT(m_buffer, 4) ? "even" : "odd"),
			BIT(m_buffer, 3) ? "on" : "off",
			!BIT(m_buffer, 2) ? "1" : !BIT(m_buffer, 0, 2) ? "1.5" : "2",
			BIT(m_buffer, 0, 2) + 5);

	m_lcr = m_buffer;
	update_tx();
	update_data_frame();
}

inline void sc16is741a_device::mcr_w()
{
	LOG(EFR_ENHANCED()
				? "MCR = 0x%1$02x (divide-by-%2$u, %3$s mode, Xon Any %4$s, loopback %5$s, reserved %6$u, TCR and TLR %7$s, RTS %8$s, reserved %9$u)\n"
				: "MCR = 0x%1$02x (loopback %5$s, reserved %6$u, TCR and TLR %7$s, RTS %8$s, reserved %9$u)\n",
			m_buffer & (EFR_ENHANCED() ? 0xff : 0x1f),
			BIT(m_buffer, 7) ? 4 : 1,
			BIT(m_buffer, 6) ? "IrDA" : "normal UART",
			BIT(m_buffer, 5) ? "enabled" : "disabled",
			BIT(m_buffer, 4) ? "enabled" : "disabled",
			BIT(m_buffer, 3),
			BIT(m_buffer, 2) ? "enabled" : "disabled",
			BIT(m_buffer, 1) ? "active" : "inactive",
			BIT(m_buffer, 0));

	if (BIT(m_buffer, 3))
		logerror("reserved bit MCR[3] is set\n");
	if (BIT(m_buffer, 0))
		logerror("reserved bit MCR[0] is set\n");

	if (!EFR_AUTO_RTS()) // FIXME: check EFCR[4]
		set_rts(BIT(~m_buffer, 1));

	if (EFR_ENHANCED())
	{
		m_mcr = m_buffer;
		update_divisor();
	}
	else
	{
		m_mcr = (m_mcr & 0xe0) | (m_buffer & 0x1f);
	}
}

inline void sc16is741a_device::tcr_w()
{
	LOG("TCR = 0x%1$02x (resume transmission at %2$u*4 characters, halt transmission at %3$u*4 characters)\n",
			m_buffer,
			BIT(m_buffer, 4, 4),
			BIT(m_buffer, 0, 4));

	m_tcr = m_buffer;
}

inline void sc16is741a_device::tlr_w()
{
	LOG("TLR = 0x%1$02x (RX FIFO trigger level %2$u*4 characters%3$s, TX FIFO trigger level %4$u*4 spaces%5$s)\n",
			m_buffer,
			BIT(m_buffer, 4, 4),
			BIT(m_buffer, 4, 4) ? "" : " - use FCR[7:6]",
			BIT(m_buffer, 0, 4),
			BIT(m_buffer, 0, 4) ? "" : " - use FCR[5:4]");

	m_tlr = m_buffer;
	update_trigger_levels();
}

inline void sc16is741a_device::reserved_w()
{
	logerror("reserved register address 0x%1$02x = 0x%2$02x\n", BIT(m_command, 3, 4), m_buffer);
}

inline void sc16is741a_device::uart_reset_w()
{
	LOG("UART reset = 0x%1$02x (reserved %2$u, reserved %3$u, reserved %4$u, reserved %5$u, %6$ssoftware reset, reserved %7$u, reserved %8$u, reserved %9$u)\n",
			m_buffer,
			BIT(m_buffer, 7),
			BIT(m_buffer, 6),
			BIT(m_buffer, 5),
			BIT(m_buffer, 4),
			BIT(m_buffer, 3) ? "" : "no ",
			BIT(m_buffer, 2),
			BIT(m_buffer, 1),
			BIT(m_buffer, 0));

	if (BIT(m_buffer, 7))
		logerror("reserved bit UART reset[7] is set\n");
	if (BIT(m_buffer, 6))
		logerror("reserved bit UART reset[6] is set\n");
	if (BIT(m_buffer, 5))
		logerror("reserved bit UART reset[5] is set\n");
	if (BIT(m_buffer, 4))
		logerror("reserved bit UART reset[4] is set\n");
	if (BIT(m_buffer, 2))
		logerror("reserved bit UART reset[2] is set\n");
	if (BIT(m_buffer, 1))
		logerror("reserved bit UART reset[1] is set\n");
	if (BIT(m_buffer, 0))
		logerror("reserved bit UART reset[0] is set\n");

	// TODO: is this instantaneous reset, or is the reset condition held until the bit is cleared?
	if (BIT(m_buffer, 3))
		device_reset();
}

inline void sc16is741a_device::dl_w()
{
	LOG("DL%1$c = 0x%2$02x\n", BIT(m_command, 3) ? 'H' : 'L', m_buffer);

	m_dl = (m_dl & (BIT(m_command, 3) ? 0x00ff : 0xff00)) | (u16(m_buffer) << (BIT(m_command, 3) ? 8 : 0));
	update_divisor();
}

inline void sc16is741a_device::efr_w()
{
	LOG("EFR = 0x%1$02x (CTS flow control %2$s, RTS flow control %3$s, special character detect %4$s, enhanced functions %5$s, %6$s)\n",
			m_buffer,
			BIT(m_buffer, 7) ? "enabled" : "disabled",
			BIT(m_buffer, 6) ? "enabled" : "disabled",
			BIT(m_buffer, 5) ? "enabled" : "disabled",
			BIT(m_buffer, 4) ? "enabled" : "disabled",
			SOFT_FLOW_CONTROL_DESC[BIT(m_buffer, 0, 4)]);

	if (!BIT(m_buffer, 6)) // FIXME: check EFCR[4]
	{
		// auto RTS off, ensure RTS output is up-to-date
		set_rts(BIT(~m_mcr, 1));
	}
	else if (!EFR_AUTO_RTS())
	{
		// enabling auto RTS
		if (FCR_FIFO_ENABLE())
		{
			u8 const level(fifo_fill_level(0));
			set_rts(((level <= (TCR_LEVEL_RESUME() * 4)) || (level < (TCR_LEVEL_HALT() * 4))) ? 0 : 1);
		}
		else
		{
			set_rts(m_fifo_empty[0] ? 0 : 1);
		}
	}

	m_efr = m_buffer;
	check_tx();
}

inline void sc16is741a_device::xon_xoff_w()
{
	LOG("%1$s%2$u = 0x%3$02x\n", BIT(m_command, 4) ? "XOFF" : "XON", BIT(m_command, 3) + 1, m_buffer);

	m_xon_xoff[BIT(m_command, 3, 2)] = m_buffer;
}


inline void sc16is741a_device::pop_rx_fifo()
{
	assert(!m_fifo_empty[0] || !m_fifo_errors);

	if (!m_fifo_empty[0] && m_fifo_data[1][m_fifo_tail[0]])
	{
		assert(m_fifo_errors);
		assert(m_interrupts & INTERRUPT_LINE_STATUS);
		if (!--m_fifo_errors)
		{
			LOG("read last data error, clearing line status interrupt\n");
			m_interrupts &= ~INTERRUPT_LINE_STATUS;
			update_irq();
		}
	}

	fifo_pop(0);
	u8 const level(fifo_fill_level(0));
	if (m_fifo_empty[0])
		m_rx_timeout_timer->reset();
	else
		m_rx_timeout_timer->adjust(attotime::from_ticks(m_divisor * 16 / 2 * 4 * m_rx_intervals, clock()));

	if (m_interrupts & INTERRUPT_RX_TIMEOUT)
	{
		LOG("clearing RX timeout interrupt\n");
		m_interrupts &= ~INTERRUPT_RX_TIMEOUT;
		update_irq();
	}

	if (m_interrupts & INTERRUPT_RHR)
	{
		if (FCR_FIFO_ENABLE())
		{
			if (level < m_rx_trigger)
			{
				LOG("RX FIFO level %1$u within %2$u, clearing RHR interrupt\n", level, m_rx_trigger);
				m_interrupts &= ~INTERRUPT_RHR;
				update_irq();
			}
		}
		else if (m_fifo_empty[0])
		{
			LOG("RHR empty, clearing RHR interrupt\n");
			m_interrupts &= ~INTERRUPT_RHR;
			update_irq();
		}
	}

	if (EFR_AUTO_RTS() && m_rts) // FIXME: check EFCR[4]
	{
		if (FCR_FIFO_ENABLE())
		{
			u8 const trigger(TCR_LEVEL_RESUME());
			if (level <= (trigger * 4))
			{
				LOG("RX FIFO level %1$u within %2$u*4, asserting RTS\n", level, trigger);
				set_rts(0);
			}
		}
		else
		{
			LOG("RHR empty, asserting RTS\n");
			set_rts(0);
		}
	}
}

inline bool sc16is741a_device::check_tx()
{
	if (m_tx_remain || m_fifo_empty[1] || (EFR_AUTO_CTS() && m_cts) || !m_divisor) // FIXME: check EFCR[2]
		return false;

	u16 const data(u16(m_fifo_data[2][fifo_pop(1)] & util::make_bitmask<u8>(m_word_length)) << 1);
	if (parity::NONE == m_parity)
	{
		m_shift_reg[1] = ~util::make_bitmask<u16>(m_word_length + 1) | data;
	}
	else
	{
		m_shift_reg[1] = ~util::make_bitmask<u16>(m_word_length + 2) | data;
		switch (m_parity)
		{
		case parity::ODD:
			m_shift_reg[1] |= BIT(~population_count_32(data), 0) << (m_word_length + 1);
			break;
		case parity::EVEN:
			m_shift_reg[1] |= BIT(population_count_32(data), 0) << (m_word_length + 1);
			break;
		case parity::MARK:
			m_shift_reg[1] |= u16(1) << (m_word_length + 1);
			break;
		default:
			break;
		}
	}
	m_tx_remain = m_tx_intervals;
	m_tx_count = 0;
	update_tx();
	m_shift_timer[1]->adjust(attotime::from_ticks(m_divisor * 16 / 2, clock()));

	if (IER_THR_INT() && !(m_interrupts & INTERRUPT_THR))
	{
		if (FCR_FIFO_ENABLE())
		{
			// TODO: does this only happen at the trigger level, or any time the FIFO is popped above the trigger level?
			u8 const spaces(fifo_spaces(1));
			if (spaces >= m_tx_trigger)
			{
				LOG("TX FIFO spaces %1$u exceed %2$u, setting THR interrupt\n", spaces, m_tx_trigger);
				m_interrupts |= INTERRUPT_THR;
				update_irq();
			}
			else
			{
				LOG("THR empty, setting THR interrupt\n");
				m_interrupts |= INTERRUPT_THR;
				update_irq();
			}
		}
	}

	return true;
}


inline u8 sc16is741a_device::fifo_spaces(unsigned n) const
{
	if (m_fifo_empty[n])
		return FIFO_LENGTH;
	else
		return (FIFO_LENGTH - m_fifo_head[n] + m_fifo_tail[n]) % FIFO_LENGTH;
}

inline u8 sc16is741a_device::fifo_fill_level(unsigned n) const
{
	if (!m_fifo_empty[n] && (m_fifo_head[n] == m_fifo_tail[n]))
		return FIFO_LENGTH;
	else
		return (FIFO_LENGTH + m_fifo_head[n] - m_fifo_tail[n]) % FIFO_LENGTH;
}

inline void sc16is741a_device::fifo_reset(unsigned n)
{
	m_fifo_head[n] = m_fifo_tail[n];
	m_fifo_empty[n] = true;
}

inline u8 sc16is741a_device::fifo_push(unsigned n)
{
	if (!FCR_FIFO_ENABLE())
	{
		if (!m_fifo_empty[n])
			LOG("%1$s FIFO overrun\n", n ? "TX" : "RX");
		m_fifo_empty[n] = false;
		return m_fifo_head[n];
	}
	else if ((m_fifo_head[n] != m_fifo_tail[n]) || m_fifo_empty[n])
	{
		m_fifo_empty[n] = false;
		return std::exchange(m_fifo_head[n], (m_fifo_head[n] + 1) & 0x3f);
	}
	else
	{
		LOG("%1$s FIFO overrun\n", n ? "TX" : "RX");
		return (m_fifo_head[n] - 1) & 0x3f;
	}
}

inline u8 sc16is741a_device::fifo_pop(unsigned n)
{
	if (m_fifo_empty[n])
	{
		assert(m_fifo_head[n] == m_fifo_tail[n]);
		LOG("%1$s FIFO underrun\n", n ? "TX" : "RX");
		return m_fifo_tail[n];
	}
	else if ((m_fifo_head[n] != m_fifo_tail[n]) || FCR_FIFO_ENABLE())
	{
		u8 const result(std::exchange(m_fifo_tail[n], (m_fifo_tail[n] + 1) & 0x3f));
		if (m_fifo_head[n] == m_fifo_tail[n])
			m_fifo_empty[n] = true;
		return result;
	}
	else
	{
		m_fifo_empty[n] = true;
		return m_fifo_tail[n];
	}
}


TIMER_CALLBACK_MEMBER(sc16is741a_device::rx_shift)
{
	assert(m_divisor);

	m_shift_reg[0] = (m_shift_reg[0] >> 1) | (u16(m_rx) << 15);
	--m_rx_remain;
	++m_rx_count;
	if (m_rx_remain)
	{
		m_shift_timer[0]->adjust(attotime::from_ticks(m_divisor * 16, clock()));
	}
	else
	{
		u8 const data(BIT(m_shift_reg[0], 16 + 1 - m_rx_count, m_rx_count - ((parity::NONE == m_parity) ? 2 : 3)));
		u8 lsr(
				(BIT(~m_shift_reg[0], 15)                                                           ? 0x08 : 0x00) |
				((!m_fifo_empty[0] && (!FCR_FIFO_ENABLE() || (m_fifo_head[0] == m_fifo_tail[0])))   ? 0x02 : 0x00));
		switch (m_parity)
		{
		case parity::NONE:
			break;
		case parity::ODD:
			lsr |= BIT(population_count_32(data) ^ BIT(~m_shift_reg[0], 14), 0) << 2;
			break;
		case parity::EVEN:
			lsr |= BIT(population_count_32(data) ^ BIT(m_shift_reg[0], 14), 0) << 2;
			break;
		case parity::MARK:
			lsr |= BIT(~m_shift_reg[0], 14) << 2;
			break;
		case parity::SPACE:
			lsr |= BIT(m_shift_reg[0], 14) << 2;
			break;
		}
		m_shift_reg[0] = 0xffff;
		u8 const pos(fifo_push(0));
		if (lsr && (!BIT(lsr, 1) || !m_fifo_data[1][pos]))
			++m_fifo_errors;
		m_fifo_data[0][pos] = data;
		m_fifo_data[1][pos] = lsr;
		u8 const level(fifo_fill_level(0));
		m_rx_timeout_timer->adjust(attotime::from_ticks(m_divisor * 16 / 2 * 4 * m_rx_intervals, clock()));

		if (!(m_interrupts & INTERRUPT_LINE_STATUS))
		{
			if (lsr)
			{
				assert(1 == m_fifo_errors);
				LOG("data error, setting line status interrupt\n");
				m_interrupts |= INTERRUPT_LINE_STATUS;
				update_irq();
			}
		}

		if (!(m_interrupts & INTERRUPT_RHR))
		{
			if (FCR_FIFO_ENABLE())
			{
				if (level >= m_rx_trigger)
				{
					LOG("RX FIFO level %1$u exceeds %2$u, setting RHR interrupt\n", level, m_rx_trigger);
					m_interrupts |= INTERRUPT_RHR;
					update_irq();
				}
			}
			else
			{
				LOG("RHR full, setting RHR interrupt\n");
				m_interrupts |= INTERRUPT_RHR;
				update_irq();
			}
		}

		if (EFR_AUTO_RTS() && !m_rts) // FIXME: check EFCR[4]
		{
			if (FCR_FIFO_ENABLE())
			{
				u8 const trigger(TCR_LEVEL_HALT());
				if (level >= (trigger * 4))
				{
					LOG("RX FIFO level %1$u exceeds %2$u*4, deasserting RTS\n", level, trigger);
					if (IER_RTS_INT() && !(m_interrupts & INTERRUPT_CTS_RTS))
					{
						LOG("setting RTS interrupt\n");
						m_interrupts |= INTERRUPT_CTS_RTS;
						update_irq();
					}
					set_rts(1);
				}
			}
			else
			{
				LOG("RHR full, deasserting RTS\n");
				if (IER_RTS_INT() && !(m_interrupts & INTERRUPT_CTS_RTS))
				{
					LOG("setting RTS interrupt\n");
					m_interrupts |= INTERRUPT_CTS_RTS;
					update_irq();
				}
				set_rts(1);
			}
		}
	}
}

TIMER_CALLBACK_MEMBER(sc16is741a_device::tx_shift)
{
	assert(m_divisor);

	if (!BIT(++m_tx_count, 0))
	{
		m_shift_reg[1] = (m_shift_reg[1] >> 1) | u16(0x8000);
		update_tx();
	}

	if (--m_tx_remain)
		m_shift_timer[1]->adjust(attotime::from_ticks(m_divisor * 16 / 2, clock()));
	else if (!check_tx())
		m_shift_timer[1]->reset();
}

TIMER_CALLBACK_MEMBER(sc16is741a_device::rx_timeout)
{
	if (IER_RHR_INT() && !(m_interrupts & INTERRUPT_RX_TIMEOUT))
	{
		LOG("setting RX timeout interrupt\n");
		m_interrupts |= INTERRUPT_RX_TIMEOUT;
		update_irq();
	}
}


inline void sc16is741a_device::update_trigger_levels()
{
	u8 const rx_level(BIT(m_tlr, 4, 4));
	u8 const tx_level(BIT(m_tlr, 0, 4));
	m_rx_trigger = rx_level ? (rx_level * 4) : RX_TRIGGER_LEVELS[FCR_RX_TRIGGER()];
	m_tx_trigger = tx_level ? (tx_level * 4) : TX_TRIGGER_LEVELS[FCR_TX_TRIGGER()];
}

inline void sc16is741a_device::update_data_frame()
{
	m_word_length = BIT(m_lcr, 0, 2) + 5;
	if (!LCR_PARITY_ENABLE())
		m_parity = parity::NONE;
	else if (!LCR_SET_PARITY())
		m_parity = LCR_EVEN_PARITY() ? parity::EVEN : parity::ODD;
	else
		m_parity = LCR_EVEN_PARITY() ? parity::SPACE : parity::MARK;
	u8 const stop(!LCR_STOP_BIT() ? 2 : (5 == m_word_length) ? 3 : 4);
	m_rx_intervals = m_word_length + ((parity::NONE == m_parity) ? 2 : 3);
	m_tx_intervals = ((m_word_length + ((parity::NONE == m_parity) ? 1 : 2)) * 2) + stop;
}

inline void sc16is741a_device::update_divisor()
{
	bool const zero(!m_divisor);
	m_divisor = u32(m_dl) * (MCR_CLOCK_DIV4() ? 4 : 1);
	if (!zero && !m_divisor)
	{
		if (m_rx_remain)
		{
			// FIXME: receive shift register immediately transferred to RHR
			LOG("suspending reception due to zero divisor\n");
			m_rx_remain = 0;
			m_shift_timer[0]->reset();
		}

		if (!m_shift_timer[1]->expire().is_never())
		{
			LOG("suspending transmission due to zero divisor\n");
			m_shift_timer[1]->reset();
		}

		m_rx_timeout_timer->reset();
	}
	else if (zero && m_divisor)
	{
		if (m_tx_remain && m_shift_timer[1]->expire().is_never())
		{
			LOG("non-zero divisor caused transmission to resume\n");
			m_shift_timer[1]->adjust(attotime::from_ticks(m_divisor * 16 / 2, clock()));
		}
	}
}
