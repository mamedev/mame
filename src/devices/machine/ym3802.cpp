// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * ym3802.c - Yamaha MCS MIDI Communication and Service Controller
 *
 * TODO:
 *   - Receive serial data
 *   - Transmit Idle detection
 *   - IRx/ITx (used for MIDI system messages)
 *   - FSK modulation
 *   - Timers (MIDI clock timer and Click counter are working but not guaranteed to be perfectly accurate)
 *   - Interrupts (except for Tx Buffer Empty, MIDI clock detect, Click Counter)
 */

#include "emu.h"
#include "ym3802.h"


DEFINE_DEVICE_TYPE(YM3802, ym3802_device, "ym3802", "Yamaha YM3802 MCS MIDI Communication and Service Controller")

ym3802_device::ym3802_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, YM3802, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_irq_handler(*this)
	, m_txd_handler(*this)
	, m_rxd_handler(*this, 0xff)
	, m_reg(REG_MAX)
	, m_wdr(0)
	, m_irq_status(0)
	, m_vector(0)
	, m_clkm_rate(500000)  // TODO: make these configurable
	, m_clkf_rate(614400)
{
}

void ym3802_device::device_start()
{
	m_midi_timer = timer_alloc(FUNC(ym3802_device::transmit_clk), this);
	m_midi_counter_timer = timer_alloc(FUNC(ym3802_device::midi_clk), this);
	save_item(NAME(m_reg));
}

void ym3802_device::device_reset()
{
	m_reg.assign(REG_MAX, 0);
	reset_irq(0xff);
	transmit_register_reset();
	receive_register_reset();
	reset_midi_timer();
	set_comms_mode();
}

void ym3802_device::set_irq(uint8_t irq)
{
	uint8_t x;

	m_irq_status |= (irq & m_reg[REG_IER]);
	for(x=0;x<8;x++)
	{
		if(m_irq_status & (1 << x))
			break;
	}
	m_vector = (m_reg[REG_IOR] & 0xe0) | (x << 1);
	if(m_irq_status != 0)
		m_irq_handler(ASSERT_LINE);
}

void ym3802_device::reset_irq(uint8_t irq)
{
	m_irq_status &= ~irq;
	if(m_irq_status == 0)
		m_irq_handler(CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(ym3802_device::transmit_clk)
{
	if(m_reg[REG_TCR] & 0x01) // Tx Enable
	{
		if(!m_tx_fifo.empty())
		{
			if (is_transmit_register_empty())
			{
				transmit_register_setup(m_tx_fifo.front());  // start to send first byte in FIFO
				m_tx_fifo.pop();                                // and remove it from the FIFO
				if(m_tx_fifo.empty())
					set_irq(IRQ_FIFOTX_EMPTY);
			}
		}
		/* if diserial has bits to send, make them so */
		if (!is_transmit_register_empty())
		{
			uint8_t data = transmit_register_get_data_bit();
			m_tx_busy = true;
			m_txd_handler(data);
		}
		if (m_tx_fifo.empty() && is_transmit_register_empty())
			m_tx_busy = false;
	}
}

TIMER_CALLBACK_MEMBER(ym3802_device::midi_clk)
{
	if(m_midi_counter_base > 1)  // counter is not guaranteed to work if set to 0 or 1.
	{
		if(m_midi_counter == 0)
		{
			m_midi_counter = m_midi_counter_base;  // reload timer
			if(m_reg[REG_IMR] & 0x08)  // if IRQ1 is set to MIDI clock detect
				set_irq(IRQ_MIDI_CLK);
			if(m_click_counter_base != 0)
			{
				m_click_counter--;
				if(m_click_counter == 0)
				{
					m_click_counter = m_click_counter_base;
					if(!(m_reg[REG_IMR] & 0x08))  // if IRQ1 is set to click counter
						set_irq(IRQ_CLICK);
				}
			}
		}
		else
			m_midi_counter--;
	}
}

void ym3802_device::reset_midi_timer()
{
	uint32_t rate;
	uint8_t divisor = m_reg[REG_TRR] & 0x1f;

	if(!(divisor & 0x10))
	{
		if(divisor & 0x08)
			rate = m_clkm_rate / 32;
		else
			rate = m_clkm_rate / 16;
	}
	else
	{
		if(!(divisor & 0x08))
			rate = m_clkf_rate / 32;
		else
			rate = m_clkf_rate / (64 << (divisor & 0x07));
	}

	if(rate != m_prev_rate)
		m_midi_timer->adjust(attotime::from_hz(rate),0,attotime::from_hz(rate));
	m_prev_rate = rate;
	logerror("MIDI Timer rate set to %iHz\n",rate);
}

void ym3802_device::set_comms_mode()
{
	uint8_t data_bits = (m_reg[REG_TMR] & 0x20) ? 7 : 8;
	parity_t parity;
	stop_bits_t stop_bits = (m_reg[REG_TMR] & 0x02) ? STOP_BITS_2 : STOP_BITS_1;

	if(!(m_reg[REG_TMR] & 0x10))  // parity enable
		parity = PARITY_NONE;
	else
	{
		if(m_reg[REG_TMR] & 0x04)
			parity = PARITY_ODD;
		else
			parity = PARITY_EVEN;
		// TODO: 4-bit parity
	}

	set_data_frame(1, data_bits, parity, stop_bits);
	logerror("MIDI comms set to 1 start bit, %i data bits, %s, parity = %i\n",data_bits, (stop_bits == STOP_BITS_2) ? "2 stop bits" : "1 stop bit", parity);
}

uint8_t ym3802_device::read(offs_t offset)
{
	if(offset < 4)
	{
		if(offset == 3)
			return m_wdr;
		if(offset == 2)
			return m_irq_status;
		if(offset == 0)
			return m_vector;
		return m_reg[offset];
	}
	else
	{
		uint8_t bank = m_reg[REG_RGR] & 0x0f;
		uint8_t ret = 0;

		if(bank > 9)
			return m_wdr;

		switch(offset + (bank * 10))
		{
			case REG_TSR:
				if(m_tx_fifo.empty())
					ret |= 0x80;
				if(m_tx_fifo.size() < 16)
					ret |= 0x40;
				if(m_tx_busy)
					ret |= 0x01;
				break;
			default:
				ret = m_reg[offset + (bank * 10)];
		}
		return ret;
	}
}

void ym3802_device::write(offs_t offset, uint8_t data)
{
	m_wdr = data;
	if(offset == 1)
	{
		m_reg[REG_RGR] = data & 0x0f;
		if(data & 0x80)
			device_reset();
		logerror("MIDI: writing %02x to reg %i\n",data,offset);
	}
	if(offset == 3)
		reset_irq(data);
	if(offset > 4)
	{
		uint8_t bank = m_reg[REG_RGR] & 0x0f;

		if(bank > 9)
			return;

		m_reg[offset + (bank * 10)] = data;
		logerror("MIDI: writing %02x to reg %i\n",data,offset + (bank * 10));

		switch(offset + (bank * 10))
		{
			case REG_IOR:
				logerror("IOR vector write %02\n",data);
				break;
			case REG_IER:
				logerror("IER set to %02x\n",data);
				break;
			case REG_DCR:
				if(data & 0x20)
				{
					if((data & 0x07) == 2)
					{
						const double rate = (m_reg[REG_CCR] & 0x02) ? m_clkm_rate / 4 : m_clkm_rate / 8;

						// start message to click counter
						m_midi_counter_timer->adjust(attotime::from_hz(rate),0,attotime::from_hz(rate));
					}
					if((data & 0x07) == 3)
					{
						// stop message to click counter
						m_midi_counter_timer->adjust(attotime::zero,0,attotime::never);
					}
				}
				break;
			case REG_TMR:
				set_comms_mode();
				break;
			case REG_TCR:
				if(data & 0x01)
					reset_midi_timer();
				break;
			case REG_TDR:
				m_tx_fifo.push(data);
				reset_irq(IRQ_FIFOTX_EMPTY);
				break;
			case REG_GTR_LOW:
				m_general_counter = (m_general_counter & 0xff00) | data;
				//popmessage("General counter set to %i\n",m_general_counter);
				break;
			case REG_GTR_HIGH:
				m_general_counter = (m_general_counter & 0x00ff) | ((data & 0x3f) << 8);
				//popmessage("General counter set to %i\n",m_general_counter);
				break;
			case REG_MTR_LOW:
				m_midi_counter_base = (m_midi_counter & 0xff00) | data;
				m_midi_counter = m_midi_counter_base;
				//popmessage("MIDI counter set to %i\n",m_midi_counter);
				break;
			case REG_MTR_HIGH:
				m_midi_counter_base = (m_midi_counter & 0x00ff) | ((data & 0x3f) << 8);
				m_midi_counter = m_midi_counter_base;
				//popmessage("MIDI counter set to %i\n",m_midi_counter);
				break;
			case REG_CDR:
				m_click_counter_base = data & 0x7f;
				m_click_counter = m_click_counter_base;
				break;
		}
	}
}
