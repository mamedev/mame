// license:BSD-3-Clause
// copyright-holders:R. Belmont

/*
 * An emulation of the Digital Equipment Corporation DC7085 (also called "DZ") quad-UART
 *
 * Used in:
 *
 *   Several models of MIPS DECstation
 *   Some VAXstations
 *
 * Sources:
 *
 *   http://www.vanade.com/~blc/DS3100/pmax/DS3100.func.spec.pdf
 *
 */

#include "emu.h"
#include "dc7085.h"

#define LOG_GENERAL (1U << 0)
#define LOG_REG     (1U << 1)
#define LOG_RX      (1U << 2)
#define LOG_TX      (1U << 3)
#define LOG_IRQ     (1U << 4)

//#define VERBOSE (LOG_GENERAL|LOG_REG|LOG_RX|LOG_TX|LOG_IRQ)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(DC7085, dc7085_device, "dc7085", "Digital Equipment Corporation DC7085 Quad UART")
DEFINE_DEVICE_TYPE(DC7085_CHANNEL, dc7085_channel, "dc7085_channel", "DC7085 UART channel")

enum csr_mask : u16
{
	CSR_TRDY  = 0x8000, // transmitter ready
	CSR_TIE   = 0x4000, // transmitter interrupt enable
	CSR_TLINE = 0x0300, // transmitter line number
	CSR_RDONE = 0x0080, // receiver done
	CSR_RIE   = 0x0040, // receiver interrupt enable
	CSR_MSE   = 0x0020, // master scan enable
	CSR_CLR   = 0x0010, // master clear
	CSR_MAINT = 0x0008, // maintenance (loopback)
};

enum rbuf_mask : u16
{
	RBUF_DVAL   = 0x8000, // data valid
	RBUF_OERR   = 0x4000, // overrun error
	RBUF_FERR   = 0x2000, // framing error
	RBUF_PERR   = 0x1000, // parity error
	RBUF_RLINE  = 0x0300, // received line number
	RBUF_RLINE3 = 0x0300,
	RBUF_RLINE2 = 0x0200,
	RBUF_RLINE1 = 0x0100,
	RBUF_RLINE0 = 0x0000,
	RBUF_DATA   = 0x00ff, // received character
};

enum lpr_mask : u16
{
	LPR_RXENAB = 0x1000, // receiver enable
	LPR_SC     = 0x0f00, // speed code
	LPR_ODDPAR = 0x0080, // odd parity
	LPR_PARENB = 0x0040, // parity enable
	LPR_STOP   = 0x0020, // stop code
	LPR_CHAR   = 0x0018, // character length
	LPR_LINE   = 0x0003, // parameter line number
};

enum tcr_mask : u16
{
	TCR_DTR3   = 0x0800, // modem control
	TCR_DTR2   = 0x0400,
	TCR_DTR1   = 0x0200,
	TCR_DTR0   = 0x0100,
	TCR_LNENB3 = 0x0008, // transmitter line enable
	TCR_LNENB2 = 0x0004,
	TCR_LNENB1 = 0x0002,
	TCR_LNENB0 = 0x0001,
};

enum tdr_mask : u16
{
	TDR_BRK3 = 0x0800, // break control
	TDR_BRK2 = 0x0400,
	TDR_BRK1 = 0x0200,
	TDR_BRK0 = 0x0100,
	TDR_TBUF = 0x00ff, // transmitter buffer
};

dc7085_device::dc7085_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DC7085, tag, owner, clock)
	, m_chan(*this, "ch%u", 0U)
	, m_int_cb(*this)
	, m_tx_cb(*this)
	, m_dtr_cb(*this)
	, m_int_state(false)
{
}

void dc7085_device::device_add_mconfig(machine_config &config)
{
	/*
	 * Configure all four channels such that:
	 *  - line numbers are inesrted into received data words
	 *  - transmitter output is looped back to receiver when enabled
	 *  - transmitter completion is signalled
	 */
	for (unsigned i = 0; i < std::size(m_chan); i++)
	{
		DC7085_CHANNEL(config, m_chan[i], 0);

		m_chan[i]->rx_done().set([this, i](u16 data) { rx_done((i << 8) | data); });
		m_chan[i]->tx_cb().set([this, i](int state) { m_tx_cb[i](state); if (m_csr & CSR_MAINT) m_chan[i]->rx_w(state); });
		m_chan[i]->tx_done().set(*this, FUNC(dc7085_device::tx_done));
	}
}

void dc7085_device::map(address_map &map)
{
	map(0x00, 0x01).rw(FUNC(dc7085_device::csr_r), FUNC(dc7085_device::csr_w));
	map(0x04, 0x05).rw(FUNC(dc7085_device::rbuf_r), FUNC(dc7085_device::lpr_w));
	map(0x08, 0x09).rw(FUNC(dc7085_device::tcr_r), FUNC(dc7085_device::tcr_w));
	map(0x0c, 0x0d).rw(FUNC(dc7085_device::msr_r), FUNC(dc7085_device::tdr_w));
}

void dc7085_device::device_start()
{
	m_int_cb.resolve_safe();
	m_tx_cb.resolve_all_safe();
	m_dtr_cb.resolve_all_safe();

	save_item(NAME(m_csr));
	save_item(NAME(m_tcr));
	save_item(NAME(m_msr));
	//save_item(NAME(m_fifo));
	save_item(NAME(m_rx_buf));
	save_item(NAME(m_int_state));
}

void dc7085_device::device_reset()
{
	m_csr = 0;
	m_tcr = 0;
	m_msr = 0;

	m_fifo.clear();
	m_rx_buf = 0;

	set_int(false);
}

u16 dc7085_device::rbuf_r()
{
	if (m_fifo.empty())
		return 0;

	u16 const data = m_fifo.dequeue();

	LOGMASKED(LOG_RX, "rbuf_r 0x%04x fifo_length %d\n", data, m_fifo.queue_length());

	if (m_fifo.empty())
		m_csr &= ~CSR_RDONE;

	// FIXME: insert pending data into fifo
	if (m_rx_buf & RBUF_DVAL)
	{
		rx_fifo_push(m_rx_buf);
		m_rx_buf = 0;
	}

	recalc_irqs();

	return data;
}

void dc7085_device::csr_w(u16 data)
{
	LOGMASKED(LOG_REG, "csr_w %04x tie %d rie %d scan %d clear %d loopback %d\n", data,
		bool(data & CSR_TIE), bool(data & CSR_RIE), bool(data & CSR_MSE), bool(data & CSR_CLR), bool(data & CSR_MAINT));

	if (!(data & CSR_CLR))
	{
		data &= (CSR_TIE | CSR_RIE | CSR_MSE | CSR_MAINT);
		m_csr &= ~(CSR_TIE | CSR_RIE | CSR_MSE | CSR_MAINT);
		m_csr |= data;
	}
	else
		reset();
}

void dc7085_device::lpr_w(u16 data)
{
	static const int bauds[] = { 50, 75, 110, 134, 150, 300, 600, 1200, 1800, 2000, 2400, 3600, 4800, 7200, 9600, 19800 };

	unsigned const baud = (data & LPR_SC) >> 8;
	unsigned const data_bits = ((data & LPR_CHAR) >> 3) + 5;
	unsigned const parity = (data & LPR_PARENB) ? ((data & LPR_ODDPAR) ? 1 : 2) : 0;
	unsigned const stop_bits = (data & LPR_STOP) ? 2 : 1;

	m_chan[data & LPR_LINE]->set_format(bauds[baud], data_bits, parity, stop_bits);
	m_chan[data & LPR_LINE]->set_enable(data & LPR_RXENAB);
}

void dc7085_device::tcr_w(u16 data)
{
	LOGMASKED(LOG_REG, "tcr_w %04x\n", data);

	if ((data ^ m_tcr) & TCR_DTR0)
		m_dtr_cb[0](bool(data & TCR_DTR0));
	if ((data ^ m_tcr) & TCR_DTR1)
		m_dtr_cb[1](bool(data & TCR_DTR1));
	if ((data ^ m_tcr) & TCR_DTR2)
		m_dtr_cb[2](bool(data & TCR_DTR2));
	if ((data ^ m_tcr) & TCR_DTR3)
		m_dtr_cb[3](bool(data & TCR_DTR3));

	m_tcr = data;

	recalc_irqs();
}

void dc7085_device::tdr_w(u16 data)
{
	LOGMASKED(LOG_REG, "tdr_w %04x (%s)\n", data, machine().describe_context());

	unsigned const ch = (m_csr & CSR_TLINE) >> 8;

	if (BIT(m_tcr, ch))
		m_chan[ch]->tx_w(data & TDR_TBUF);

	m_csr &= ~CSR_TRDY;
	recalc_irqs();
}

void dc7085_device::recalc_irqs()
{
	LOGMASKED(LOG_IRQ, "recalc_irqs enter\n");
	m_csr &= ~(CSR_TRDY | CSR_TLINE);
	for (unsigned i = 0; i < 4; i++)
	{
		if (BIT(m_tcr, i) && m_chan[i]->tx_ready())
		{
			m_csr |= CSR_TRDY;
			m_csr |= (i << 8);
			LOGMASKED(LOG_IRQ, "ch %u: set TRDY\n", i);

			break;
		}
	}

	set_int(((m_csr & CSR_TIE) && (m_csr & CSR_TRDY)) || ((m_csr & CSR_RIE) && (m_csr & CSR_RDONE)));
}

void dc7085_device::rx_fifo_push(u16 data)
{
	if (!m_fifo.full())
	{
		LOGMASKED(LOG_RX, "rx_fifo_push 0x%04x fifo_length %d\n", data, m_fifo.queue_length());

		m_fifo.enqueue(data);

		m_csr |= CSR_RDONE;
	}
	else
		throw emu_fatalerror("fifo overflow\n");
}

void dc7085_device::rx_done(u16 data)
{
	// check if receive buffer is full
	if (m_rx_buf & RBUF_DVAL)
	{
		// push buffer into fifo if not full
		if (!m_fifo.full())
		{
			rx_fifo_push(m_rx_buf);
			m_rx_buf = 0;
		}
		else
			// flag buffer overrun
			data |= RBUF_OERR;
	}

	// store received data in fifo or buffer
	if (!m_fifo.full())
		rx_fifo_push(data);
	else
		m_rx_buf = data;

	recalc_irqs();
}

void dc7085_device::tx_done(int state)
{
	recalc_irqs();
}

dc7085_channel::dc7085_channel(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DC7085_CHANNEL, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_tx_cb(*this)
	, m_tx_done(*this)
	, m_rx_done(*this)
	, m_rx_enabled(false)
{
}

void dc7085_channel::device_start()
{
	m_tx_cb.resolve_safe();
	m_rx_done.resolve_safe();
	m_tx_done.resolve_safe();

	save_item(NAME(m_rx_enabled));
}

void dc7085_channel::device_reset()
{
	transmit_register_reset();

	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_tra_rate(0);
	set_rcv_rate(0);

	m_rx_enabled = false;
}

void dc7085_channel::rcv_complete()
{
	receive_register_extract();

	if (m_rx_enabled)
	{
		u16 data = RBUF_DVAL | get_received_char();

		if (is_receive_framing_error())
			data |= RBUF_FERR;
		if (is_receive_parity_error())
			data |= RBUF_PERR;

		m_rx_done(data);
	}
}

void dc7085_channel::tra_complete()
{
	m_tx_done(1);
}

void dc7085_channel::tra_callback()
{
	m_tx_cb(transmit_register_get_data_bit());
}

void dc7085_channel::set_format(unsigned baud, unsigned data_bits, unsigned parity, unsigned stop_bits)
{
	set_data_frame(1, data_bits, parity ? (parity == 1 ? PARITY_ODD : PARITY_EVEN) : PARITY_NONE,
		stop_bits == 1 ? STOP_BITS_1 : (data_bits == 5 ? STOP_BITS_1_5 : STOP_BITS_2));

	set_tra_rate(baud);
	set_rcv_rate(baud);
}

void dc7085_channel::tx_w(u8 data)
{
	if (is_transmit_register_empty())
		transmit_register_setup(data);
}
