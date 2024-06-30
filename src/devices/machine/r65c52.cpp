// license:BSD-3-Clause
// copyright-holders:jwallace
/**********************************************************************

	Rockwell 65C52  Dual Asynchronous Communication Interface Adapter

	A slightly tweaked combination of two 6551 ACIAs on a single chip

**********************************************************************/

#include "emu.h"
#include "r65c52.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(R65C52, r65c52_device, "r65c52", "Rockwell 65C52 DACIA")

r65c52_device::r65c52_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) : device_t(mconfig, R65C52, tag, owner, clock),
																												m_internal_clock1(*this, "clock1"),
																												m_internal_clock2(*this, "clock2"),
																												m_irq_handler(*this),
																												m_txd_handler(*this),
																												m_rxc_handler(*this),
																												m_rts_handler(*this),
																												m_dtr_handler(*this),
																												m_aux_ctrl{0, 0}, m_control{0, 0},
																												m_compare{0, 0},
																												m_format{0, 0}, m_status{0, 0},
																												m_tdr{0, 0}, m_tdre{true, true},
																												m_rdr{0, 0}, m_rdrf{false, false},
																												m_ier{0, 0}, m_isr{0, 0},
																												m_irq{0, 0},
																												m_overrun{false, false},
																												m_parity_err{false, false},
																												m_parity_err_mode{0, 0},
																												m_txd{0, 0},
																												m_rxc(0),
																												m_rts{0, 0},
																												m_dtr{0, 0},
																												m_xtal(clock),
																												m_divide{0, 0},
																												m_cts{0, 0},
																												m_dsr{0, 0},
																												m_dcd{0, 0},
																												m_rxd{0, 0}, m_wordlength{0, 0}, m_stoplength{0, 0}, m_brk{0, 0}, m_echo_mode{0, 0}, m_parity{0, 0},
																												m_rx_state{STATE_START, STATE_START},
																												m_rx_clock{0, 0}, m_rx_bits{0, 0}, m_rx_shift{0, 0}, m_rx_parity{0, 0},
																												m_rx_counter{0, 0},
																												m_tx_state{STATE_START, STATE_START},
																												m_tx_output{OUTPUT_MARK, OUTPUT_MARK},
																												m_tx_clock{0, 0}, m_tx_bits{0, 0}, m_tx_shift{0, 0}, m_tx_parity{0, 0},
																												m_tx_counter{0, 0}
{
}

const int r65c52_device::internal_divider[16] =
	{
		4608,
		2096,
		1713,
		1536,
		768,
		384,
		192,
		128,
		96,
		64,
		48,
		32,
		24,
		12,
		6,
		1};

void r65c52_device::device_add_mconfig(machine_config &config)
{
	CLOCK(config, m_internal_clock1, 0);
	CLOCK(config, m_internal_clock2, 0);
	m_internal_clock1->signal_handler().set(FUNC(r65c52_device::internal_clock1));
	m_internal_clock2->signal_handler().set(FUNC(r65c52_device::internal_clock2));
}

void r65c52_device::device_start()
{
	// state saving
	save_item(NAME(m_aux_ctrl));
	save_item(NAME(m_compare));
	save_item(NAME(m_control));
	save_item(NAME(m_format));
	save_item(NAME(m_status));
	save_item(NAME(m_tdr));
	save_item(NAME(m_tdre));
	save_item(NAME(m_rdr));
	save_item(NAME(m_rdrf));

	save_item(NAME(m_ier));
	save_item(NAME(m_isr));
	save_item(NAME(m_irq));

	save_item(NAME(m_overrun));
	save_item(NAME(m_parity_err));
	save_item(NAME(m_parity_err_mode));

	save_item(NAME(m_txd));
	save_item(NAME(m_rxc));
	save_item(NAME(m_rts));
	save_item(NAME(m_dtr));

	save_item(NAME(m_xtal));
	save_item(NAME(m_divide));
	save_item(NAME(m_cts));
	save_item(NAME(m_dsr));
	save_item(NAME(m_dcd));
	save_item(NAME(m_rxd));

	save_item(NAME(m_wordlength));
	save_item(NAME(m_stoplength));
	save_item(NAME(m_brk));
	save_item(NAME(m_echo_mode));
	save_item(NAME(m_parity));

	save_item(NAME(m_rx_state));
	save_item(NAME(m_rx_clock));
	save_item(NAME(m_rx_bits));
	save_item(NAME(m_rx_shift));
	save_item(NAME(m_rx_parity));
	save_item(NAME(m_rx_counter));

	save_item(NAME(m_tx_state));
	save_item(NAME(m_tx_output));
	save_item(NAME(m_tx_clock));
	save_item(NAME(m_tx_bits));
	save_item(NAME(m_tx_shift));
	save_item(NAME(m_tx_parity));

	save_item(NAME(m_tx_counter));

	m_internal_clock1->set_unscaled_clock(m_xtal);
	m_internal_clock2->set_unscaled_clock(m_xtal);

	set_xtal(m_xtal);
}

void r65c52_device::device_reset()
{
	for (int i = 0; i < 2; i++)
	{
		m_tdre[i] = true;
		m_rdrf[i] = true;

		m_status[i] = SR_FRAMING_ERROR;

		m_aux_ctrl[i] = 0;
		m_compare[i] = 0;
		m_rdr[i] = 0x00;
		m_isr[i] = 0x80;
		m_ier[i] = 0x80;

		m_rts[i] = 1;
		m_dtr[i] = 1;

		output_rts(i, 1);

		output_dtr(i, 1);

		m_rx_state[i] = STATE_START;
		m_rx_counter[i] = 0;
	}
}

u8 r65c52_device::stoplengthcounter(int idx)
{
	if (m_stoplength[idx] == 2)
	{
		if (m_wordlength[idx] == 5 && m_parity[idx] == PARITY_NONE)
		{
			return m_divide[idx] + (m_divide[idx] / 2);
		}

		if (m_wordlength[idx] < 8 || m_parity[idx] == PARITY_NONE)
		{
			return m_divide[idx] * 2;
		}
	}

	return m_divide[idx];
}

void r65c52_device::output_irq(int idx, int irq)
{
	LOG("R65C52: %x  IRQ %x \n", idx + 1, irq);

	if (m_irq[idx] != irq)
	{
		m_irq[idx] = irq;

		m_irq_handler[idx](!m_irq[idx]);
	}
}

void r65c52_device::output_txd(int idx, int txd)
{
	switch (m_tx_output[idx])
	{
	case OUTPUT_MARK:
		txd = 1;
		break;

	case OUTPUT_BREAK:
		txd = 0;
		break;
	}

	if (m_txd[idx] != txd)
	{
		m_txd[idx] = txd;
		m_txd_handler[idx](m_txd[idx]);
	}
}

void r65c52_device::output_rts(int idx, int rts)
{
	LOG("R65C52: %x  RTS %x \n", idx + 1, rts);
	if (m_rts[idx] != rts)
	{
		m_rts[idx] = rts;
		m_rts_handler[idx](m_rts[idx]);
		if (rts)
		{
			m_status[idx] |= SR_RTS;
		}
		else
		{
			m_status[idx] &= ~SR_RTS;
		}
	}
}

void r65c52_device::output_dtr(int idx, int dtr)
{
	LOG("R65C52: %x  DTR %x \n", idx + 1, dtr);
	if (m_dtr[idx] != dtr)
	{
		m_dtr[idx] = dtr;
		m_dtr_handler[idx](m_dtr[idx]);
		if (dtr)
		{
			m_status[idx] |= SR_DTR;
		}
		else
		{
			m_status[idx] &= ~SR_DTR;
		}
	}
}

void r65c52_device::update_irq(int idx)
{
	bool irq = false;
	LOG("R65C52: %x  IER  %x ISR %x\n", idx + 1, m_ier[idx], m_isr[idx]);
	for (int i = 0; i < 8; i++)
	{
		if ((m_ier[idx] & (1 >> i)) && ((m_isr[idx] & (1 >> i))))
		{
			irq = true;
		}
	}

	output_irq(idx, irq);
}

void r65c52_device::update_divider(int idx)
{
	// bits 0-3
	double scale = internal_divider[(m_control[idx] >> 0) & 0xf];

	if (m_xtal != 0)
	{
		m_divide[idx] = 16;
		if (!m_dtr[idx] || m_rx_state[idx] != STATE_START)
		{
			scale = (double)1 / scale;
		}
		else
		{
			scale = 0;
		}

		LOG("R65C52: %x  CLOCK %d SCALE %f \n", idx + 1, m_xtal * scale, scale);
	}
	else
	{
		m_divide[idx] = scale * 16;
		scale = 0;
	}

	if (idx == 0)
	{
		m_internal_clock1->set_clock_scale(scale);
	}
	else
	{
		m_internal_clock2->set_clock_scale(scale);
	}
}

u8 r65c52_device::read_rdr(int idx)
{
	m_status[idx] &= ~(SR_BRK | SR_FRAMING_ERROR);
	m_isr[idx] &= ~(IRQ_PAR | IRQ_FOB | IRQ_RDRF);
	m_rdrf[idx] = false;
	m_parity_err[idx] = false;
	m_overrun[idx] = false;
	update_irq(idx);
	LOG("R65C52: %x  RDR %x \n", idx + 1, m_rdr[idx]);
	return m_rdr[idx];
}

u8 r65c52_device::read_status(int idx)
{
	LOG("R65C52: %x  STATUS %x \n", idx + 1, m_status[idx]);
	m_dtr[idx] = false;
	m_rts[idx] = false;
	return m_status[idx];
}

void r65c52_device::write_ier(int idx, u8 data)
{
	if (data & 0x80)
	{
		m_ier[idx] |= (data & 0x7f);
	}
	else
	{
		m_ier[idx] &= ~(data & 0x7f);
	}

	LOG("R65C52: %x  IER %x \n", idx + 1, m_ier[idx]);
}

void r65c52_device::write_tdr(int idx, u8 data)
{
	m_tdr[idx] = data;
	m_tdre[idx] = false;
	m_isr[idx] &= ~IRQ_TDRE;
	LOG("R65C52: %x  TDR %x \n", idx + 1, m_tdr[idx]);
}

void r65c52_device::write_control(int idx, u8 data)
{
	m_control[idx] = data;

	// bits 0-3
	update_divider(idx);

	// bit 4
	m_echo_mode[idx] = (m_control[idx] >> 4) & 1;

	// bit 5
	m_stoplength[idx] = 1 + ((m_control[idx] >> 5) & 1);

	LOG("R65C52: %x CTRL%X ECHO %x STOP%x\n", idx + 1, m_control[idx], m_echo_mode[idx], m_stoplength[idx]);
}

void r65c52_device::write_format(int idx, u8 data)
{
	m_format[idx] = data;

	// bit 0
	output_rts(idx, ((m_format[idx] >> 0) & 1));

	// bit 1
	output_dtr(idx, ((m_format[idx] >> 1) & 1));

	// bit 2
	if (!((m_format[idx] >> 2) & 1))
	{
		m_parity[idx] = PARITY_NONE;
	}
	else
	{
		// bits 3-4
		m_parity[idx] = (m_format[idx] >> 3) & 3;
	}

	// bits 5-6
	m_wordlength[idx] = 5 + ((m_format[idx] >> 5) & 3);

	LOG("R65C52: %x FMT %x RTS %x DTR %x PARITY %x WORDLENGTH %x \n", idx + 1, m_format[idx], (m_format[idx] >> 0) & 1, (m_format[idx] >> 1) & 1, m_parity[idx], m_wordlength[idx]);

	update_divider(idx);
}

void r65c52_device::write_aux_ctrl(int idx, u8 data)
{
	m_aux_ctrl[idx] = data;

	// bit 0
	m_parity_err_mode[idx] = (m_format[idx] >> 0) & 1;

	// bit 1
	m_brk[idx] = (m_format[idx] >> 1) & 1;

	LOG("R65C52: %x  AUX CTRL %x \n", idx + 1, m_aux_ctrl[idx]);
}

void r65c52_device::write_compare(int idx, u8 data)
{
	LOG("R65C52: %x COMPARE %x \n", idx + 1, data);
	m_compare[idx] = data;
}

u8 r65c52_device::read_isr(int idx)
{

	if (m_status[idx] & SR_BRK || m_status[idx] & SR_FRAMING_ERROR || m_overrun[idx])
	{
		m_isr[idx] |= IRQ_FOB;
	}

	u8 isr = m_isr[idx];

	if (isr != 0)
	{
		isr |= 0x80;
	}
	if (!m_echo_mode[idx])
	{
		if (m_cts[idx])
		{
			isr |= 0x80;
		}
	}
	else
	{
		isr &= ~0x80;
	}

	m_isr[idx] &= ~(IRQ_CTS | IRQ_DCD | IRQ_DSR | IRQ_FOB);

	LOG("R65C52: %x  ISR %x \n", idx + 1, m_isr[idx]);

	return isr;
}

void r65c52_device::map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(r65c52_device::isr_0_r), FUNC(r65c52_device::ier_0_w));
	map(0x01, 0x01).rw(FUNC(r65c52_device::status_0_r), FUNC(r65c52_device::format_ctrl_0_w));
	map(0x02, 0x02).nopr().w(FUNC(r65c52_device::aux_compare_0_w));
	map(0x03, 0x03).rw(FUNC(r65c52_device::rdr_0_r), FUNC(r65c52_device::tdr_0_w));
	map(0x04, 0x04).rw(FUNC(r65c52_device::isr_1_r), FUNC(r65c52_device::ier_1_w));
	map(0x05, 0x05).rw(FUNC(r65c52_device::status_1_r), FUNC(r65c52_device::format_ctrl_1_w));
	map(0x06, 0x06).nopr().w(FUNC(r65c52_device::aux_compare_1_w));
	map(0x07, 0x07).rw(FUNC(r65c52_device::rdr_1_r), FUNC(r65c52_device::tdr_1_w));
}

void r65c52_device::format_ctrl_0_w(u8 data)
{
	if (data & 0x80)
	{
		write_format(0, data);
	}
	else
	{
		write_control(0, data);
	}
}

void r65c52_device::format_ctrl_1_w(u8 data)
{
	if (data & 0x80)
	{
		write_format(1, data);
	}
	else
	{
		write_control(1, data);
	}
}

void r65c52_device::aux_compare_0_w(u8 data)
{
	if (data & 0x40)
	{
		write_aux_ctrl(0, data);
	}
	else
	{
		write_compare(0, data);
	}
}

void r65c52_device::aux_compare_1_w(u8 data)
{
	if (data & 0x40)
	{
		write_aux_ctrl(1, data);
	}
	else
	{
		write_compare(1, data);
	}
}

void r65c52_device::set_xtal(u32 xtal)
{
	m_xtal = xtal;

	if (started())
	{
		m_internal_clock1->set_unscaled_clock(m_xtal);
		m_internal_clock2->set_unscaled_clock(m_xtal);
		update_divider(0);
		update_divider(1);
	}
}

void r65c52_device::internal_clock1(int state)
{
	transmitter_clock(0, state);
	receiver_clock(0, state);
}

void r65c52_device::internal_clock2(int state)
{
	transmitter_clock(1, state);
	receiver_clock(1, state);
}

void r65c52_device::write_rxc(int state)
{
	for (int i = 0; i < 2; i++)
	{
		receiver_clock(i, state);
	}
}

void r65c52_device::write_txc(int state)
{
	for (int i = 0; i < 2; i++)
	{
		transmitter_clock(i, state);
	}
}

void r65c52_device::write_rxd1(int state)
{
	m_rxd[0] = state;
}

void r65c52_device::write_rxd2(int state)
{
	m_rxd[1] = state;
}

void r65c52_device::write_cts1(int state)
{
	_write_cts(0, state);
}

void r65c52_device::write_cts2(int state)
{
	_write_cts(1, state);
}

void r65c52_device::_write_cts(int idx, int state)
{
	if (m_cts[idx] != state)
	{
		m_cts[idx] = state;
		m_isr[idx] |= IRQ_CTS;

		if (m_cts[idx])
		{
			m_status[idx] |= SR_CTS;

			if (m_tx_output[idx] == OUTPUT_TXD)
			{
				m_tx_output[idx] = OUTPUT_MARK;
				output_txd(idx, 1);
			}
		}
		else
		{
			m_status[idx] &= ~SR_CTS;
		}
		update_irq(idx);
	}
	LOG("R65C52: %x  CTS STATUS %x \n", idx + 1, m_status[idx]);
}

void r65c52_device::write_dsr1(int state)
{
	_write_dsr(0, state);
}

void r65c52_device::write_dsr2(int state)
{
	_write_dsr(1, state);
}

void r65c52_device::_write_dsr(int idx, int state)
{
	if (m_dsr[idx] != state)
	{
		m_dsr[idx] = state;
		m_isr[idx] |= IRQ_DSR;

		if (m_dsr[idx])
		{
			m_status[idx] |= SR_DSR;
		}
		else
		{
			m_status[idx] &= ~SR_DSR;
		}
		update_irq(idx);
	}
	LOG("R65C52: %x  DSR STATUS %x \n", idx + 1, m_status[idx]);
}

void r65c52_device::write_dcd1(int state)
{
	_write_dcd(0, state);
}

void r65c52_device::write_dcd2(int state)
{
	_write_dcd(1, state);
}

void r65c52_device::_write_dcd(int idx, int state)
{
	if (m_dcd[idx] != state)
	{
		m_dcd[idx] = state;
		m_isr[idx] |= IRQ_DCD;

		if (m_dcd[idx])
		{
			m_status[idx] |= SR_DCD;
		}
		else
		{
			m_status[idx] &= ~SR_DCD;
		}
		update_irq(idx);
	}
	LOG("R65C52: %x  DCD STATUS %x \n", idx + 1, m_status[idx]);
}

void r65c52_device::receiver_clock(int idx, int state)
{
	if (m_rx_clock[idx] != state)
	{
		m_rx_clock[idx] = state;

		if (m_rx_clock[idx])
		{
			m_rx_counter[idx]++;

			switch (m_rx_state[idx])
			{
			case STATE_START:
				if (m_rx_counter[idx] == 1)
				{
					if (!m_rxd[idx] && !m_dtr[idx])
					{
						LOG("R65C52: RX%x START BIT \n", idx + 1);
					}
					else
					{
						m_rx_counter[idx] = 0;
					}
				}

				if (m_rx_counter[idx] >= m_divide[idx] / 2)
				{
					if (!m_rxd[idx])
					{
						m_rx_state[idx] = STATE_DATA;
						m_rx_counter[idx] = 0;
						m_rx_shift[idx] = 0;
						m_rx_parity[idx] = 0;
						m_rx_bits[idx] = 0;
					}
					else
					{
						m_rx_counter[idx] = 0;

						LOG("R65C52: RX%x false START BIT\n", idx + 1);
					}
				}
				break;

			case STATE_DATA:
				if (m_rx_counter[idx] == m_divide[idx])
				{
					m_rx_counter[idx] = 0;

					if (m_rx_bits[idx] < m_wordlength[idx])
					{
						LOG("R65C52: RX%x DATA BIT %d %d\n", idx + 1, m_rx_bits[idx], m_rxd[idx]);
					}
					else
					{
						LOG("R65C52: RX%x PARITY BIT %x\n", idx + 1, m_rxd[idx]);
					}

					if (m_rxd[idx])
					{
						m_rx_shift[idx] |= 1 << m_rx_bits[idx];
					}

					m_rx_bits[idx]++;

					m_rx_parity[idx] ^= m_rxd[idx];

					if ((m_rx_bits[idx] == m_wordlength[idx] && m_parity[idx] == PARITY_NONE) ||
						(m_rx_bits[idx] == (m_wordlength[idx] + 1) && m_parity[idx] != PARITY_NONE))
					{
						m_rx_state[idx] = STATE_STOP;
					}
				}
				break;

			case STATE_STOP:
				if (m_rx_counter[idx] >= stoplengthcounter(idx))
				{
					m_rx_counter[idx] = 0;

					LOG("R65C52: RX%x STOP BIT\n", idx + 1);
					if (!(m_rdrf[idx]))
					{
						if (!m_rxd[idx])
						{
							m_status[idx] |= SR_FRAMING_ERROR;
						}

						if ((m_parity[idx] == PARITY_ODD && !m_rx_parity[idx]) ||
							(m_parity[idx] == PARITY_EVEN && m_rx_parity[idx]))
						{
							m_parity_err[idx] = true;
						}
						else
						{
							m_parity_err[idx] = false;
						}

						if (m_parity_err_mode[idx])
						{
							if (m_parity_err[idx])
							{
								m_isr[idx] |= IRQ_PAR;
							}
							else
							{
								m_isr[idx] &= ~IRQ_PAR;
							}
							update_irq(idx);
						}
						m_rdr[idx] = m_rx_shift[idx];

						if (m_wordlength[idx] == 7 && m_parity[idx] != PARITY_NONE)
						{
							m_rdr[idx] &= 0x7f;
						}

						// In compare mode, we only flip RDRF if the data matches
						if (m_compare[idx] == 0 || (m_compare[idx] == m_rdr[idx]))
						{
							m_rdrf[idx] = true;
							m_isr[idx] |= IRQ_RDRF;
							m_compare[idx] = 0;
							update_irq(idx);
						}
						m_overrun[idx] = false;
					}
					else
					{
						m_overrun[idx] = true;
					}

					LOG("R65C52: RX%x DATA %x\n", idx + 1, m_rdr[idx]);

					m_rx_state[idx] = STATE_START;

					if (m_dtr[idx])
					{
						update_divider(idx);
					}
				}
				break;
			}
		}
	}
}

void r65c52_device::transmitter_clock(int idx, int state)
{
	if (m_tx_clock[idx] != state)
	{
		m_tx_clock[idx] = state;

		if (!m_tx_clock[idx] && !m_dtr[idx])
		{
			if (m_echo_mode[idx])
			{
				if (!(m_overrun[idx]))
				{
					output_txd(idx, m_rxd[idx]);
				}
				else
				{
					output_txd(idx, 1);
				}
			}

			if (!m_cts[idx] && m_tx_output[idx] == OUTPUT_MARK && !(m_tdre[idx]))
			{
				m_tx_state[idx] = STATE_START;
				m_tx_counter[idx] = 0;
			}

			m_tx_counter[idx]++;

			switch (m_tx_state[idx])
			{
			case STATE_START:
			{
				m_tx_counter[idx] = 0;

				m_tx_state[idx] = STATE_DATA;
				m_tx_shift[idx] = m_tdr[idx];
				m_tx_bits[idx] = 0;
				m_tx_parity[idx] = 0;

				if (m_cts[idx])
				{
					m_tx_output[idx] = OUTPUT_MARK;
					LOG("R65C52: TX%x CTS MARK START\n", idx + 1);
				}
				else if (!(m_tdre[idx]))
				{
					LOG("R65C52: TX%x DATA %x\n", idx + 1, m_tdr[idx]);

					m_tx_output[idx] = OUTPUT_TXD;

					LOG("R65C52: TX%x START BIT\n", idx + 1);

					m_tdre[idx] = true;
					m_isr[idx] |= IRQ_TDRE;
					update_irq(idx);
				}
				else if (m_brk[idx])
				{
					m_tx_output[idx] = OUTPUT_BREAK;

					LOG("R65C52: TX%x BREAK START\n", idx + 1);
				}
				else
				{
					m_tx_output[idx] = OUTPUT_MARK;
					LOG("R65C52: TX%x MARK START\n", idx + 1);
				}

				if (m_tx_output[idx] != OUTPUT_BREAK)
				{
					m_tdre[idx] = true;
					m_isr[idx] |= IRQ_TDRE;
					update_irq(idx);
				}

				// fatalerror("setup");
				output_txd(idx, 0);
				break;
			}
			case STATE_DATA:
			{
				if (m_tx_counter[idx] == m_divide[idx])
				{
					m_tx_counter[idx] = 0;

					if (m_tx_bits[idx] < m_wordlength[idx])
					{
						output_txd(idx, (m_tx_shift[idx] >> m_tx_bits[idx]) & 1);

						m_tx_bits[idx]++;
						m_tx_parity[idx] ^= m_txd[idx];

						if (m_tx_output[idx] == OUTPUT_TXD)
						{
							LOG("R65C52: TX%x DATA BIT TXD %d %d\n", idx + 1, m_tx_bits[idx], m_txd[idx]);
						}
					}
					else if (m_tx_bits[idx] == m_wordlength[idx] && m_parity[idx] != PARITY_NONE)
					{
						m_tx_bits[idx]++;

						switch (m_parity[idx])
						{
						case PARITY_ODD:
							m_tx_parity[idx] = !m_tx_parity[idx];
							break;

						case PARITY_MARK:
							m_tx_parity[idx] = 1;
							break;

						case PARITY_SPACE:
							m_tx_parity[idx] = 0;
							break;
						}

						output_txd(idx, m_tx_parity[idx]);

						if (m_tx_output[idx] == OUTPUT_TXD)
						{
							LOG("R65C52: TX%x PARITY BIT %d\n", idx + 1, m_txd);
						}

						if (!m_parity_err_mode[idx])
						{
							if (m_tx_parity[idx])
							{
								m_isr[idx] |= IRQ_PAR;
							}
							else
							{
								m_isr[idx] &= ~IRQ_PAR;
							}
							update_irq(idx);
						}
					}
					else
					{
						m_tx_state[idx] = STATE_STOP;

						output_txd(idx, 1);
						// m_tdre[idx] = true;
						// m_isr[idx] |= IRQ_TDRE;
						// update_irq(idx);

						if (m_tx_output[idx] == OUTPUT_TXD)
						{
							LOG("R65C52: TX%x STOP BIT\n", idx + 1);
						}
					}
				}
				break;
			}

			case STATE_STOP:
			{
				if (m_tx_counter[idx] >= stoplengthcounter(idx))
				{
					if (m_tx_output[idx] == OUTPUT_BREAK)
					{
						if (!m_brk[idx])
						{
							LOG("R65C52: TX%x BREAK END\n", idx + 1);

							m_tx_counter[idx] = 0;
							m_tx_state[idx] = STATE_STOP;
							m_tx_output[idx] = OUTPUT_TXD;

							output_txd(idx, 1);
						}
						else
						{
							m_tx_counter[idx]--;
						}
					}
					else
					{
						m_tx_state[idx] = STATE_START;
						m_tx_counter[idx] = 0;
					}
				}
				break;
			}
			}
		}
	}
}