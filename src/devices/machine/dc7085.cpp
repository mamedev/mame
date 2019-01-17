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

dc7085_device::dc7085_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DC7085, tag, owner, clock),
	m_chan0(*this, "ch0"),
	m_chan1(*this, "ch1"),
	m_chan2(*this, "ch2"),
	m_chan3(*this, "ch3"),
	m_int_cb(*this),
	write_0_tx(*this),
	write_1_tx(*this),
	write_2_tx(*this),
	write_3_tx(*this)
{
	std::fill_n(&rx_fifo[0], DC7085_RX_FIFO_SIZE, 0);
	rx_fifo_num = 0;
}

void dc7085_device::device_add_mconfig(machine_config &config)
{
	DC7085_CHANNEL(config, m_chan0, 0);
	DC7085_CHANNEL(config, m_chan1, 0);
	DC7085_CHANNEL(config, m_chan2, 0);
	DC7085_CHANNEL(config, m_chan3, 0);
}

void dc7085_device::map(address_map &map)
{
	map(0x00, 0x01).rw(FUNC(dc7085_device::status_r), FUNC(dc7085_device::control_w));
	map(0x04, 0x05).rw(FUNC(dc7085_device::rxbuffer_r), FUNC(dc7085_device::lineparams_w));
	map(0x08, 0x09).rw(FUNC(dc7085_device::txparams_r), FUNC(dc7085_device::txparams_w));
	map(0x0c, 0x0d).rw(FUNC(dc7085_device::modem_status_r), FUNC(dc7085_device::txdata_w));
}

void dc7085_device::device_start()
{
	m_int_cb.resolve_safe();
	write_0_tx.resolve_safe();
	write_1_tx.resolve_safe();
	write_2_tx.resolve_safe();
	write_3_tx.resolve_safe();

	save_item(NAME(m_status));
	save_item(NAME(rx_fifo));
	save_item(NAME(rx_fifo_read_ptr));
	save_item(NAME(rx_fifo_write_ptr));
	save_item(NAME(rx_fifo_num));
}

void dc7085_device::device_reset()
{
	m_chan0->clear();
	m_chan1->clear();
	m_chan2->clear();
	m_chan3->clear();
	m_status = 0;
	std::fill_n(&rx_fifo[0], DC7085_RX_FIFO_SIZE, 0);
	rx_fifo_write_ptr = rx_fifo_read_ptr = 0;
	rx_fifo_num = 0;
}

u16 dc7085_device::status_r()
{
	return m_status;
}

u16 dc7085_device::rxbuffer_r()
{
	u16 rv;

	LOGMASKED(LOG_RX, "rxbuffer_r: rx_fifo_num %d\n", rx_fifo_num);

	if (rx_fifo_num == 0)
	{
		LOGMASKED(LOG_RX, "rx fifo underflow\n");
		m_status &= ~CTRL_RX_DONE;
		recalc_irqs();
		return 0;
	}

	rv = rx_fifo[rx_fifo_read_ptr++];
	if (rx_fifo_read_ptr == DC7085_RX_FIFO_SIZE)
	{
		rx_fifo_read_ptr = 0;
	}

	rx_fifo_num--;
	if (rx_fifo_num == 0)
	{
		m_status &= ~CTRL_RX_DONE;
	}

	recalc_irqs();

	//printf("Rx read %02x\n", rv);

	return rv;
}

u16 dc7085_device::txparams_r()
{
	return 0;
}

u16 dc7085_device::modem_status_r()
{
	return 0;
}

void dc7085_device::control_w(u16 data)
{
	LOGMASKED(LOG_REG, "control_w %04x\n", data);
	LOGMASKED(LOG_REG, "\tTx IRQ %d  Rx IRQ %d\n", (data & CTRL_TX_IRQ_ENABLE) ? "1" : "0", (data & CTRL_RX_IRQ_ENABLE) ? "1" : "0");
	LOGMASKED(LOG_REG, "\tScan enable %d  Master clear %d\n", (data & CTRL_MASTER_SCAN) ? "1" : "0", (data & CTRL_MASTER_CLEAR) ? "1" : "0");
	LOGMASKED(LOG_REG, "\tLocal loopback %d\n", (data & CTRL_LOOPBACK) ? "1" : "0");

	if (data & CTRL_MASTER_CLEAR)
	{
		m_chan0->clear();
		m_chan1->clear();
		m_chan2->clear();
		m_chan3->clear();
		m_status = 0;
		rx_fifo_write_ptr = rx_fifo_read_ptr = 0;
		rx_fifo_num = 0;
		return;
	}

	data &= (CTRL_TX_IRQ_ENABLE|CTRL_RX_IRQ_ENABLE|CTRL_MASTER_SCAN|CTRL_LOOPBACK);
	m_status &= ~(CTRL_TX_IRQ_ENABLE|CTRL_RX_IRQ_ENABLE|CTRL_MASTER_SCAN|CTRL_LOOPBACK);
	m_status |= data;
}

void dc7085_device::lineparams_w(u16 data)
{
	static const int bauds[] = { 50, 75, 110, 134, 150, 300, 600, 1200, 1800, 2000, 2400, 3600, 4800, 7200, 9600, 19800 };

	LOGMASKED(LOG_REG, "lineparams_w %04x\n", data);
	LOGMASKED(LOG_REG, "\tline %d baud %d rx enabled %d\n", data & 3, bauds[(data>>8) & 0x0f], (data & 0x1000) ? 1 : 0);
	LOGMASKED(LOG_REG, "\tline %d %d data bits, %d stop bits\n", data & 3, ((data>>3) & 3) + 5, ((data>>5) & 1));
	LOGMASKED(LOG_REG, "\tline %d parity %s %s\n", data & 3, (data & LPARAM_ODD_PARITY) ? "odd" : "even", (data & LPARAM_PARITY_ENB) ? "enabled" : "disabled");

	int parity = -1;
	if (data & LPARAM_PARITY_ENB)
	{
		parity = (data & LPARAM_ODD_PARITY) ? 1 : 0;
	}

	switch (data & 3)
	{
		case 0:
			m_chan0->set_format(((data>>3) & 3) + 5, parity, ((data>>5) & 1));
			m_chan0->set_baud_rate(bauds[(data>>8) & 0x0f]);
			m_chan0->set_rx_enable(data & LPARAM_RX_ENABLE);
			break;
		case 1:
			m_chan1->set_format(((data>>3) & 3) + 5, parity, ((data>>5) & 1));
			m_chan1->set_baud_rate(bauds[(data>>8) & 0x0f]);
			m_chan1->set_rx_enable(data & LPARAM_RX_ENABLE);
			break;
		case 2:
			m_chan2->set_format(((data>>3) & 3) + 5, parity, ((data>>5) & 1));
			m_chan2->set_baud_rate(bauds[(data>>8) & 0x0f]);
			m_chan2->set_rx_enable(data & LPARAM_RX_ENABLE);
			break;
		case 3:
			m_chan3->set_format(((data>>3) & 3) + 5, parity, ((data>>5) & 1));
			m_chan3->set_baud_rate(bauds[(data>>8) & 0x0f]);
			m_chan3->set_rx_enable(data & LPARAM_RX_ENABLE);
			break;
	}
}

void dc7085_device::txparams_w(u16 data)
{
	LOGMASKED(LOG_REG, "txparams_w %04x\n", data);

	m_chan0->set_tx_enable(data & TXCTRL_LINE0_ENB);
	m_chan1->set_tx_enable(data & TXCTRL_LINE1_ENB);
	m_chan2->set_tx_enable(data & TXCTRL_LINE2_ENB);
	m_chan3->set_tx_enable(data & TXCTRL_LINE3_ENB);
	recalc_irqs();
}

void dc7085_device::txdata_w(u16 data)
{
	LOGMASKED(LOG_REG, "txdata_w %04x\n", data);
	switch ((m_status >> 8) & 3)
	{
		case 0:
			m_chan0->write_TX(data&0xff);
			break;
		case 1:
			m_chan1->write_TX(data&0xff);
			break;
		case 2:
			m_chan2->write_TX(data&0xff);
			break;
		case 3:
			m_chan3->write_TX(data&0xff);
			break;
	}
}

void dc7085_device::recalc_irqs()
{
	bool bIRQ = false;

	LOGMASKED(LOG_IRQ, "recalc_irqs enter\n");
	if (m_chan0->is_tx_ready())
	{
		m_status |= CTRL_TRDY;
		m_status &= ~CTRL_LINE_MASK;
		LOGMASKED(LOG_IRQ, "ch 0: set TRDY\n");
	}
	else if (m_chan1->is_tx_ready())
	{
		m_status |= CTRL_TRDY;
		m_status &= ~CTRL_LINE_MASK;
		m_status |= (1 << 8);
		LOGMASKED(LOG_IRQ, "ch 1: set TRDY\n");
	}
	else if (m_chan2->is_tx_ready())
	{
		m_status |= CTRL_TRDY;
		m_status &= ~CTRL_LINE_MASK;
		m_status |= (2 << 8);
		LOGMASKED(LOG_IRQ, "ch 2: set TRDY\n");
	}
	else if (m_chan3->is_tx_ready())
	{
		m_status |= CTRL_TRDY;
		m_status &= ~CTRL_LINE_MASK;
		m_status |= (3 << 8);
		LOGMASKED(LOG_IRQ, "ch 3: set TRDY\n");
	}

	if ((m_status & CTRL_TRDY) && (m_status & CTRL_TX_IRQ_ENABLE))
	{
		bIRQ = true;
	}
	else if ((m_status & CTRL_RX_DONE) && (m_status & CTRL_RX_IRQ_ENABLE))
	{
		bIRQ = true;
	}

	if (bIRQ)
	{
		m_int_cb(ASSERT_LINE);
	}
	else
	{
		m_int_cb(CLEAR_LINE);
	}
}

void dc7085_device::rx_fifo_push(uint16_t data, uint16_t errors)
{
	if (rx_fifo_num >= DC7085_RX_FIFO_SIZE)
	{
		LOGMASKED(LOG_RX, "DC7085: FIFO overflow\n");
		data |= dc7085_device::RXMASK_OVERRUN_ERR;
		return;
	}

	rx_fifo[rx_fifo_write_ptr++] = data | errors;
	if (rx_fifo_write_ptr == DC7085_RX_FIFO_SIZE)
		rx_fifo_write_ptr = 0;

	rx_fifo_num++;

	LOGMASKED(LOG_RX, "ch %d, got %02x, fifo_num %d\n", (data>>8) & 3, data&0xff, rx_fifo_num);

	m_status |= dc7085_device::CTRL_RX_DONE;
	recalc_irqs();
}

// UART channel class stuff

dc7085_channel::dc7085_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DC7085_CHANNEL, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, rx_enabled(0)
	, tx_enabled(0)
{
}

void dc7085_channel::device_start()
{
	m_base = downcast<dc7085_device *>(owner());
	m_ch = m_base->get_ch(this);    // get our channel number

	save_item(NAME(baud_rate));
	save_item(NAME(rx_enabled));
	save_item(NAME(tx_enabled));
	save_item(NAME(tx_data));
	save_item(NAME(tx_ready));
}

void dc7085_channel::device_reset()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);

	baud_rate = 0;
	rx_enabled = 0;
	tx_enabled = 0;
	tx_ready = 1;
}

// serial device virtual overrides
void dc7085_channel::rcv_complete()
{
	receive_register_extract();

	//printf("%s ch %d rcv complete\n", tag(), m_ch);

	if (rx_enabled)
	{
		uint16_t errors = 0;
		if (is_receive_framing_error())
			errors |= dc7085_device::RXMASK_FRAMING_ERR;
		if (is_receive_parity_error())
			errors |= dc7085_device::RXMASK_PARITY_ERR;

		m_base->rx_fifo_push(get_received_char() | (m_ch << 8) | dc7085_device::RXMASK_DATA_VALID, errors);
	}
}

void dc7085_channel::tra_complete()
{
	LOGMASKED(LOG_TX, "ch %d Tx complete\n", m_ch);
	tx_ready = 1;

	// if local loopback is on, write the transmitted data as if a byte had been received
	if (m_base->m_status & dc7085_device::CTRL_LOOPBACK)
		m_base->rx_fifo_push(tx_data | (m_ch << 8) | dc7085_device::RXMASK_DATA_VALID, 0);

	m_base->recalc_irqs();
}

void dc7085_channel::tra_callback()
{
	int bit = transmit_register_get_data_bit();

	LOGMASKED(LOG_TX, "transmit %d\n", bit);
	switch (m_ch)
	{
		case 0: m_base->write_0_tx(bit); break;
		case 1: m_base->write_1_tx(bit); break;
		case 2: m_base->write_2_tx(bit); break;
		case 3: m_base->write_3_tx(bit); break;
	}
}

void dc7085_channel::set_baud_rate(int baud)
{
	set_tra_rate(baud);
	set_rcv_rate(baud);
	baud_rate = baud;
}

void dc7085_channel::set_format(int data_bits, int parity, int stop_bits)
{
	switch (parity)
	{
		case -1:
			set_data_frame(1, data_bits, PARITY_NONE, stop_bits ? STOP_BITS_1 : STOP_BITS_0);
			break;

		case 0:
			set_data_frame(1, data_bits, PARITY_EVEN, stop_bits ? STOP_BITS_1 : STOP_BITS_0);
			break;

		case 1:
			set_data_frame(1, data_bits, PARITY_ODD, stop_bits ? STOP_BITS_1 : STOP_BITS_0);
			break;
	}
}

// called on a master clear
void dc7085_channel::clear()
{
	transmit_register_reset();
	set_baud_rate(0);
	rx_enabled = 0;
	tx_enabled = 0;
	tx_ready = 1;
}

void dc7085_channel::set_tx_enable(bool bEnabled)
{
	LOGMASKED(LOG_TX, "ch %d set_tx_enable %s\n", m_ch, bEnabled ? "true" : "false");
	tx_enabled = bEnabled ? 1 : 0;
}

void dc7085_channel::set_rx_enable(bool bEnabled)
{
	rx_enabled = bEnabled ? 1 : 0;
}

void dc7085_channel::write_TX(uint8_t data)
{
	tx_data = data;

	if (!tx_ready)
	{
		LOGMASKED(LOG_TX, "Write %02x to TX when TX not ready!\n", data);
	}

	LOGMASKED(LOG_TX, "ch %d Tx [%02x] (%d baud)\n", m_ch, data, baud_rate);

	tx_ready = 0;

	// send tx_data
	transmit_register_setup(tx_data);

	m_base->recalc_irqs();
}
