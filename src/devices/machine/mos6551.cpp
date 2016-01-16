// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    MOS Technology 6551 Asynchronous Communication Interface Adapter

**********************************************************************/

#include "mos6551.h"

#define LOG 0

const device_type MOS6551 = &device_creator<mos6551_device>;

mos6551_device::mos6551_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, MOS6551, "MOS6551", tag, owner, clock, "mos6551", __FILE__),
	m_internal_clock(*this, "clock"),
	m_irq_handler(*this),
	m_txd_handler(*this),
	m_rxc_handler(*this),
	m_rts_handler(*this),
	m_dtr_handler(*this),
	m_control(0), m_command(0),
	m_status(0),
	m_tdr(0), m_rdr(0),
	m_irq_state(0),
	m_irq(0),
	m_txd(0),
	m_rxc(0),
	m_rts(0),
	m_dtr(0),
	m_xtal(0),
	m_divide(0),
	m_cts(1),
	m_dsr(1),
	m_dcd(1),
	m_rxd(1), m_wordlength(0), m_extrastop(0), m_brk(0), m_echo_mode(0), m_parity(0),
	m_rx_state(STATE_START),
	m_rx_clock(0), m_rx_bits(0), m_rx_shift(0), m_rx_parity(0),
	m_rx_counter(0), m_rx_irq_enable(0),
	m_rx_internal_clock(0),
	m_tx_state(STATE_START),
	m_tx_output(OUTPUT_MARK),
	m_tx_clock(0), m_tx_bits(0), m_tx_shift(0), m_tx_parity(0),
	m_tx_counter(0), m_tx_enable(0), m_tx_irq_enable(0), m_tx_internal_clock(0)
{
}

const int mos6551_device::internal_divider[] =
{
	1, 2304, 1536, 1048, 856, 768, 384, 192, 96, 64, 48, 32, 24, 16, 12, 6
};

const int mos6551_device::transmitter_controls[4][3] =
{
	//tx irq, tx ena, brk
	{0, 0, 0},
	{1, 1, 0},
	{0, 1, 0},
	{0, 1, 1}
};

static MACHINE_CONFIG_FRAGMENT( mos6551 )
	MCFG_DEVICE_ADD("clock", CLOCK, 0)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(mos6551_device, internal_clock))
MACHINE_CONFIG_END

machine_config_constructor mos6551_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( mos6551 );
}

void mos6551_device::device_start()
{
	// resolve callbacks
	m_irq_handler.resolve_safe();
	m_txd_handler.resolve_safe();
	m_rxc_handler.resolve_safe();
	m_rts_handler.resolve_safe();
	m_dtr_handler.resolve_safe();

	// state saving
	save_item(NAME(m_control));
	save_item(NAME(m_command));
	save_item(NAME(m_status));
	save_item(NAME(m_tdr));
	save_item(NAME(m_rdr));

	save_item(NAME(m_irq_state));

	save_item(NAME(m_irq));
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
	save_item(NAME(m_extrastop));
	save_item(NAME(m_brk));
	save_item(NAME(m_echo_mode));
	save_item(NAME(m_parity));

	save_item(NAME(m_rx_state));
	save_item(NAME(m_rx_clock));
	save_item(NAME(m_rx_bits));
	save_item(NAME(m_rx_shift));
	save_item(NAME(m_rx_parity));
	save_item(NAME(m_rx_counter));
	save_item(NAME(m_rx_irq_enable));
	save_item(NAME(m_rx_internal_clock));

	save_item(NAME(m_tx_state));
	save_item(NAME(m_tx_output));
	save_item(NAME(m_tx_clock));
	save_item(NAME(m_tx_bits));
	save_item(NAME(m_tx_shift));
	save_item(NAME(m_tx_parity));
	save_item(NAME(m_tx_counter));
	save_item(NAME(m_tx_enable));
	save_item(NAME(m_tx_irq_enable));
	save_item(NAME(m_tx_internal_clock));

	m_internal_clock->set_unscaled_clock(m_xtal);

	output_irq(1);
	output_txd(1);
	output_rxc(1);
	output_rts(1);
	output_dtr(1);
}

void mos6551_device::device_reset()
{
	m_status = SR_TDRE;

	if (m_dsr)
	{
		m_status |= SR_DSR;
	}

	if (m_dcd)
	{
		m_status |= SR_DCD;
	}

	m_rx_state = STATE_START;
	m_rx_counter = 0;

	write_command(0);
	write_control(0);
}

void mos6551_device::output_irq(int irq)
{
	if (m_irq != irq)
	{
		m_irq = irq;

		if (m_irq)
		{
			m_status &= ~SR_IRQ;
		}
		else
		{
			m_status |= SR_IRQ;
		}

		m_irq_handler(!m_irq);
	}
}

void mos6551_device::output_txd(int txd)
{
	switch (m_tx_output)
	{
	case OUTPUT_MARK:
		txd = 1;
		break;

	case OUTPUT_BREAK:
		txd = 0;
		break;
	}

	if (m_txd != txd)
	{
		m_txd = txd;
		m_txd_handler(m_txd);
	}
}

void mos6551_device::output_rxc(int rxc)
{
	if (m_rxc != rxc)
	{
		m_rxc = rxc;
		m_rxc_handler(m_rxc);
	}
}

void mos6551_device::output_rts(int rts)
{
	if (m_rts != rts)
	{
		m_rts = rts;
		m_rts_handler(m_rts);
	}
}

void mos6551_device::output_dtr(int dtr)
{
	if (m_dtr != dtr)
	{
		m_dtr = dtr;
		m_dtr_handler(m_dtr);
	}
}

void mos6551_device::update_irq()
{
	if (m_irq_state != 0)
	{
		output_irq(0);
	}
	else
	{
		output_irq(1);
	}
}

void mos6551_device::update_divider()
{
	// bits 0-3
	double scale = internal_divider[(m_control >> 0) & 0xf];

	// The 6551 allows an external clock (hooked up to xtal1 with xtal2 floating) with the internal clock generator,
	// it is unknown whether it allows a xtal (hooked up to xtal1 & xtal2) to be used as an external clock. It is
	// allowed here for performance reasons.
	if (m_xtal != 0)
	{
		m_tx_internal_clock = true;

		m_divide = 16;

		if (!m_dtr || m_rx_state != STATE_START)
		{
			scale = (double) 1 / scale;
		}
		else
		{
			scale = 0;
		}
	}
	else
	{
		m_tx_internal_clock = false;

		m_divide = scale * 16;
		scale = 0;
	}

	m_internal_clock->set_clock_scale(scale);
}

UINT8 mos6551_device::read_rdr()
{
	m_status &= ~(SR_PARITY_ERROR | SR_FRAMING_ERROR | SR_OVERRUN | SR_RDRF);
	return m_rdr;
}

UINT8 mos6551_device::read_status()
{
	UINT8 status = m_status;

	if (m_cts)
	{
		status &= ~SR_TDRE;
	}

	if (m_irq_state != 0)
	{
		m_irq_state = 0;
		update_irq();
	}

	return status;
}

UINT8 mos6551_device::read_command()
{
	return m_command;
}

UINT8 mos6551_device::read_control()
{
	return m_control;
}

void mos6551_device::write_tdr(UINT8 data)
{
	m_tdr = data;
	m_status &= ~SR_TDRE;
}

void mos6551_device::write_reset(UINT8 data)
{
	m_status &= ~SR_OVERRUN;
	m_irq_state &= ~(IRQ_DCD | IRQ_DSR);

	write_command(m_command & ~0x1f);
}

void mos6551_device::write_control(UINT8 data)
{
	m_control = data;

	update_divider();

	// bit 4
	m_rx_internal_clock = (m_control >> 4) & 1;

	// bits 5-6
	m_wordlength = 8 - ((m_control >> 5) & 3);

	// bit 7
	m_extrastop = (m_control >> 7) & 1;

	if (!m_rx_internal_clock)
	{
		output_rxc(1);
	}
}

void mos6551_device::write_command(UINT8 data)
{
	m_command = data;

	// bit 0
	output_dtr(!((m_command >> 0) & 1));

	// bit 1
	m_rx_irq_enable = !((m_command >> 1) & 1) && !m_dtr;

	// bits 2-3
	int transmitter_control = (m_command >> 2) & 3;
	m_tx_irq_enable = transmitter_controls[transmitter_control][0] && !m_dtr;
	m_tx_enable = transmitter_controls[transmitter_control][1];
	m_brk = transmitter_controls[transmitter_control][2];

	// bit 4
	m_echo_mode = (m_command >> 4) & 1;

	// bits 5-7
	m_parity = (m_command >> 5) & 7;
	if (!(m_parity & 1))
	{
		m_parity = PARITY_NONE;
	}

	output_rts(!(m_tx_enable || m_echo_mode));

	if (m_dtr || m_rts)
	{
		m_tx_output = OUTPUT_MARK;
		output_txd(1);
	}

	update_divider();
}

READ8_MEMBER( mos6551_device::read )
{
	if (space.debugger_access())
		return 0xff;

	switch (offset & 0x03)
	{
	case 0:
		return read_rdr();

	case 1:
		return read_status();

	case 2:
		return read_command();

	case 3:
	default:
		return read_control();
	}
}

WRITE8_MEMBER( mos6551_device::write )
{
	switch (offset & 0x03)
	{
	case 0:
		write_tdr(data);
		break;

	case 1:
		write_reset(data);
		break;

	case 2:
		write_command(data);
		break;

	case 3:
		write_control(data);
		break;
	}
}

int mos6551_device::stoplength()
{
	if (m_extrastop == 1)
	{
		if (m_wordlength == 5 && m_parity == PARITY_NONE)
		{
			return m_divide + (m_divide / 2);
		}

		if (m_wordlength < 8 || m_parity == PARITY_NONE)
		{
			return m_divide * 2;
		}
	}

	return m_divide;
}

void mos6551_device::set_xtal(UINT32 xtal)
{
	m_xtal = xtal;

	if (started())
	{
		m_internal_clock->set_unscaled_clock(m_xtal);
		update_divider();
	}
}

WRITE_LINE_MEMBER( mos6551_device::internal_clock )
{
	if (m_tx_internal_clock)
	{
		transmitter_clock(state);
	}
}

WRITE_LINE_MEMBER(mos6551_device::write_xtal1)
{
	if (!m_tx_internal_clock)
	{
		transmitter_clock(state);
	}
}

WRITE_LINE_MEMBER( mos6551_device::write_rxd )
{
	m_rxd = state;
}

WRITE_LINE_MEMBER( mos6551_device::write_rxc )
{
	if (!m_rx_internal_clock)
	{
		receiver_clock(state);
	}
}

WRITE_LINE_MEMBER( mos6551_device::write_cts )
{
	if (m_cts != state)
	{
		m_cts = state;

		if (m_cts)
		{
			if (m_tx_output == OUTPUT_TXD)
			{
				m_tx_output = OUTPUT_MARK;
				output_txd(1);
			}
		}
	}
}

WRITE_LINE_MEMBER( mos6551_device::write_dsr )
{
	if (m_dsr != state)
	{
		m_dsr = state;
	}
}

WRITE_LINE_MEMBER( mos6551_device::write_dcd )
{
	if (m_dcd != state)
	{
		m_dcd = state;
	}
}

WRITE_LINE_MEMBER(mos6551_device::receiver_clock)
{
	if (m_rx_clock != state)
	{
		m_rx_clock = state;

		if (m_rx_clock)
		{
			/// TODO: find out whether this should be here or in write_dcd
			if ((m_irq_state & IRQ_DCD) == 0 && !m_dcd != !(m_status & SR_DCD))
			{
				m_status ^= SR_DCD;

				if (!m_dtr)
				{
					m_irq_state |= IRQ_DCD;
					update_irq();
				}
			}

			/// TODO: find out whether this should be here or in write_dsr
			if ((m_irq_state & IRQ_DSR) == 0 && !m_dsr != !(m_status & SR_DSR))
			{
				m_status ^= SR_DSR;

				if (!m_dtr)
				{
					m_irq_state |= IRQ_DSR;
					update_irq();
				}
			}

			m_rx_counter++;

			switch (m_rx_state)
			{
			case STATE_START:
				if (m_rx_counter == 1)
				{
					if (!m_rxd && !m_dtr)
					{
						if (LOG) logerror("MOS6551 '%s': RX START BIT\n", tag().c_str());
					}
					else
					{
						m_rx_counter = 0;
					}
				}

				if (m_rx_counter >= m_divide / 2)
				{
					if (!m_rxd)
					{
						m_rx_state = STATE_DATA;
						m_rx_counter = 0;
						m_rx_shift = 0;
						m_rx_parity = 0;
						m_rx_bits = 0;
					}
					else
					{
						m_rx_counter = 0;

						if (LOG) logerror("MOS6551 '%s': RX FALSE START BIT\n", tag().c_str());
					}
				}
				break;

			case STATE_DATA:
				if (m_rx_counter == m_divide)
				{
					m_rx_counter = 0;

					if (m_rx_bits < m_wordlength)
					{
						if (LOG) logerror("MOS6551 '%s': RX DATA BIT %d %d\n", tag().c_str(), m_rx_bits, m_rxd);
					}
					else
					{
						if (LOG) logerror("MOS6551 '%s': RX PARITY BIT %x\n", tag().c_str(), m_rxd);
					}

					if (m_rxd)
					{
						m_rx_shift |= 1 << m_rx_bits;
					}

					m_rx_bits++;

					m_rx_parity ^= m_rxd;

					if ((m_rx_bits == m_wordlength && m_parity == PARITY_NONE) ||
						(m_rx_bits == (m_wordlength + 1) && m_parity != PARITY_NONE))
					{
						m_rx_state = STATE_STOP;
					}
				}
				break;

			case STATE_STOP:
				if (m_rx_counter >= stoplength())
				{
					m_rx_counter = 0;

					if (LOG) logerror("MOS6551 '%s': RX STOP BIT\n", tag().c_str());

					if (!(m_status & SR_RDRF))
					{
						if (!m_rxd)
						{
							m_status |= SR_FRAMING_ERROR;
						}

						if ((m_parity == PARITY_ODD && !m_rx_parity) ||
							(m_parity == PARITY_EVEN && m_rx_parity))
						{
							m_status |= SR_PARITY_ERROR;
						}

						m_rdr = m_rx_shift;

						if (m_wordlength == 7 && m_parity != PARITY_NONE)
						{
							m_rdr &= 0x7f;
						}

						m_status |= SR_RDRF;
					}
					else
					{
						m_status |= SR_OVERRUN;
					}

					if (m_rx_irq_enable)
					{
						m_irq_state |= IRQ_RDRF;
						update_irq();
					}

					m_rx_state = STATE_START;

					if (m_dtr)
					{
						update_divider();
					}
				}
				break;
			}
		}
	}
}

WRITE_LINE_MEMBER(mos6551_device::transmitter_clock)
{
	if (m_rx_internal_clock)
	{
		output_rxc(state);
		receiver_clock(state);
	}

	if (m_tx_clock != state)
	{
		m_tx_clock = state;

		if (!m_tx_clock && !m_dtr)
		{
			if (m_echo_mode)
			{
				if (!(m_status & SR_OVERRUN))
				{
					output_txd(m_rxd);
				}
				else
				{
					output_txd(1);
				}
			}

			if (m_tx_enable)
			{
				if (!m_cts && m_tx_output == OUTPUT_MARK && !(m_status & SR_TDRE))
				{
					m_tx_state = STATE_START;
					m_tx_counter = 0;
				}

				m_tx_counter++;

				switch (m_tx_state)
				{
				case STATE_START:
					m_tx_counter = 0;

					m_tx_state = STATE_DATA;
					m_tx_shift = m_tdr;
					m_tx_bits = 0;
					m_tx_parity = 0;

					if (m_cts)
					{
						m_tx_output = OUTPUT_MARK;
					}
					else if (!(m_status & SR_TDRE))
					{
						if (LOG) logerror("MOS6551 '%s': TX DATA %x\n", tag().c_str(), m_tdr);

						m_tx_output = OUTPUT_TXD;

						if (LOG) logerror("MOS6551 '%s': TX START BIT\n", tag().c_str());

						m_status |= SR_TDRE;
					}
					else if (m_brk)
					{
						m_tx_output = OUTPUT_BREAK;

						if (LOG) logerror("MOS6551 '%s': TX BREAK START\n", tag().c_str());
					}
					else
					{
						m_tx_output = OUTPUT_MARK;
					}

					if (m_tx_irq_enable && m_tx_output != OUTPUT_BREAK)
					{
						m_irq_state |= IRQ_TDRE;
						update_irq();
					}

					output_txd(0);
					break;

				case STATE_DATA:
					if (m_tx_counter == m_divide)
					{
						m_tx_counter = 0;

						if (m_tx_bits < m_wordlength)
						{
							output_txd((m_tx_shift >> m_tx_bits) & 1);

							m_tx_bits++;
							m_tx_parity ^= m_txd;

							if (m_tx_output == OUTPUT_TXD)
							{
								if (LOG) logerror("MOS6551 '%s': TX DATA BIT %d %d\n", tag().c_str(), m_tx_bits, m_txd);
							}
						}
						else if (m_tx_bits == m_wordlength && m_parity != PARITY_NONE)
						{
							m_tx_bits++;

							switch (m_parity)
							{
							case PARITY_ODD:
								m_tx_parity = !m_tx_parity;
								break;

							case PARITY_MARK:
								m_tx_parity = 1;
								break;

							case PARITY_SPACE:
								m_tx_parity = 0;
								break;
							}

							output_txd(m_tx_parity);

							if (m_tx_output == OUTPUT_TXD)
							{
								if (LOG) logerror("MOS6551 '%s': TX PARITY BIT %d\n", tag().c_str(), m_txd);
							}
						}
						else
						{
							m_tx_state = STATE_STOP;

							output_txd(1);

							if (m_tx_output == OUTPUT_TXD)
							{
								if (LOG) logerror("MOS6551 '%s': TX STOP BIT\n", tag().c_str());
							}
						}
					}
					break;

				case STATE_STOP:
					if (m_tx_counter >= stoplength())
					{
						if (m_tx_output == OUTPUT_BREAK)
						{
							if (!m_brk)
							{
								if (LOG) logerror("MOS6551 '%s': TX BREAK END\n", tag().c_str());

								m_tx_counter = 0;
								m_tx_state = STATE_STOP;
								m_tx_output = OUTPUT_TXD;

								output_txd(1);
							}
							else
							{
								m_tx_counter--;
							}
						}
						else
						{
							m_tx_state = STATE_START;
							m_tx_counter = 0;
						}
					}
					break;
				}
			}
		}
	}
}
