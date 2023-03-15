// license:BSD-3-Clause
// copyright-holders:smf
/*********************************************************************

    6850acia.cpp

    6850 ACIA code

*********************************************************************/

#include "emu.h"
#include "6850acia.h"


/***************************************************************************
    MACROS
***************************************************************************/

#define LOG_SETUP    (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_SETUP)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGSETUP(...)    LOGMASKED(LOG_SETUP,    __VA_ARGS__)

/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

const int acia6850_device::counter_divide_select[4] =
{
	1,
	16,
	64,
	0
};

const int acia6850_device::word_select[8][3] =
{
	// word length, parity, stop bits
	{ 7, PARITY_EVEN, 2 },
	{ 7, PARITY_ODD,  2 },
	{ 7, PARITY_EVEN, 1 },
	{ 7, PARITY_ODD,  1 },
	{ 8, PARITY_NONE, 2 },
	{ 8, PARITY_NONE, 1 },
	{ 8, PARITY_EVEN, 1 },
	{ 8, PARITY_ODD,  1 }
};

const int acia6850_device::transmitter_control[4][3] =
{
	// rts, brk, tx irq
	{ 0, 0, 0 },
	{ 0, 0, 1 },
	{ 1, 0, 0 },
	{ 0, 1, 0 }
};


/***************************************************************************
    LIVE DEVICE
***************************************************************************/

// device type definition
DEFINE_DEVICE_TYPE(ACIA6850, acia6850_device, "acia6850", "MC6850 ACIA")

//-------------------------------------------------
//  acia6850_device - constructor
//-------------------------------------------------

acia6850_device::acia6850_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: acia6850_device(mconfig, ACIA6850, tag, owner, clock)
{
}

acia6850_device::acia6850_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_txd_handler(*this)
	, m_rts_handler(*this)
	, m_irq_handler(*this)
	, m_status(SR_TDRE)
	, m_tdr(0)
	, m_first_master_reset(true)
	, m_dcd_irq_pending(false)
	, m_overrun_pending(false)
	, m_divide(0)
	, m_rts(0)
	, m_dcd(0)
	, m_irq(0)
	, m_txc(0)
	, m_txd(0)
	, m_tx_state(0)
	, m_tx_counter(0)
	, m_tx_irq_enable(false)
	, m_rxc(0)
	, m_rxd(1)
	, m_rx_state(0)
	, m_rx_counter(0)
	, m_rx_irq_enable(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acia6850_device::device_start()
{
	// resolve callbacks
	m_txd_handler.resolve_safe();
	m_rts_handler.resolve_safe();
	m_irq_handler.resolve_safe();

	save_item(NAME(m_status));
	save_item(NAME(m_tdr));
	save_item(NAME(m_rdr));

	save_item(NAME(m_first_master_reset));
	save_item(NAME(m_dcd_irq_pending));
	save_item(NAME(m_overrun_pending));

	save_item(NAME(m_divide));
	save_item(NAME(m_bits));
	save_item(NAME(m_stopbits));
	save_item(NAME(m_parity));
	save_item(NAME(m_brk));

	save_item(NAME(m_rts));
	save_item(NAME(m_dcd));
	save_item(NAME(m_irq));

	save_item(NAME(m_txc));
	save_item(NAME(m_txd));
	save_item(NAME(m_tx_state));
	save_item(NAME(m_tx_bits));
	save_item(NAME(m_tx_shift));
	save_item(NAME(m_tx_parity));
	save_item(NAME(m_tx_counter));
	save_item(NAME(m_tx_irq_enable));

	save_item(NAME(m_rxc));
	save_item(NAME(m_rxd));
	save_item(NAME(m_rx_state));
	save_item(NAME(m_rx_bits));
	save_item(NAME(m_rx_shift));
	save_item(NAME(m_rx_parity));
	save_item(NAME(m_rx_counter));
	save_item(NAME(m_rx_irq_enable));
}

void acia6850_device::device_reset()
{
	output_txd(1);
	output_rts(1);
	output_irq(1);
}

uint8_t acia6850_device::status_r()
{
	uint8_t status = m_status;

	if (m_divide == 0 || (status & SR_CTS))
	{
		status &= ~SR_TDRE;
	}

	if (!machine().side_effects_disabled())
	{
		if (m_dcd_irq_pending == DCD_IRQ_READ_STATUS)
		{
			m_dcd_irq_pending = DCD_IRQ_READ_DATA;
		}
	}

	return status;
}

void acia6850_device::control_w(uint8_t data)
{
	LOG("MC6850 '%s' Control: %02x\n", tag(), data);

	// CR0 & CR1
	int counter_divide_select_bits = (data >> 0) & 3;
	m_divide = counter_divide_select[counter_divide_select_bits];
	LOGSETUP(" - Divide: x%d\n", counter_divide_select[counter_divide_select_bits]);

	// CR2, CR3 & CR4
	int word_select_bits = (data >> 2) & 7;
	m_bits = word_select[word_select_bits][0];
	m_parity = word_select[word_select_bits][1];
	m_stopbits = word_select[word_select_bits][2];
	LOGSETUP(" - %d%c%d\n", m_bits, m_parity == PARITY_NONE ? 'N' : (m_parity == PARITY_ODD ? 'O' : 'E'), m_stopbits);

	// CR5 & CR6
	int transmitter_control_bits = (data >> 5) & 3;
	int rts = transmitter_control[transmitter_control_bits][0];
	m_brk = transmitter_control[transmitter_control_bits][1];
	m_tx_irq_enable = transmitter_control[transmitter_control_bits][2];

	// CR7
	m_rx_irq_enable = (data >> 7) & 1;
	LOGSETUP(" - RTS:%d BRK:%d TxIE:%d RxIE:%d\n", rts, m_brk, m_tx_irq_enable, m_rx_irq_enable);

	if (m_divide == 0)
	{
		if (m_first_master_reset)
		{
			/// TODO: find out whether you need to leave master reset before it stops counting as the first
			rts = 1;
			m_first_master_reset = false;
		}

		m_dcd_irq_pending = DCD_IRQ_NONE;
		m_overrun_pending = false;

		m_rx_state = STATE_START;
		m_rx_counter = 0;

		m_tx_state = STATE_START;
		output_txd(1);

		// TDRE flag reads as zero in this reset state, but the drivers
		// internal SR_TDRE is masked elsewhere when in the reset
		// state. When taken out of reset the TDRE flag reads as one.
		m_status |= SR_TDRE;

		// SR_CTS is not affected by a reset, it should still reflect
		// the pin status.
		m_status &= SR_CTS | SR_TDRE;

		if (m_dcd)
		{
			m_status |= SR_DCD;
		}
	}

	output_rts(rts);

	update_irq();
}


int acia6850_device::calculate_txirq()
{
	return !(m_tx_irq_enable && m_divide && (m_status & SR_TDRE) && !(m_status & SR_CTS));
}

int acia6850_device::calculate_rxirq()
{
	return !(m_rx_irq_enable && m_divide && ((m_status & SR_RDRF) || m_dcd_irq_pending != DCD_IRQ_NONE));
}

void acia6850_device::update_irq()
{
	output_irq(calculate_txirq() && calculate_rxirq());
}

void acia6850_device::data_w(uint8_t data)
{
	LOG("MC6850 '%s' Data: %02x\n", tag(), data);

	if (m_divide == 0)
	{
		logerror("%s: ACIA data write while in reset!\n", machine().describe_context());
	}
	else
	{
		// TDRE reads as set when taken out of reset even if data is
		// written while in reset.

		// TODO: find out whether this overwrites previous data or has
		// no effect if TDRE is already clear when you write?
		m_tdr = data;
		m_status &= ~SR_TDRE;
	}

	update_irq();
}

uint8_t acia6850_device::data_r()
{
	if (!machine().side_effects_disabled())
	{
		if (m_overrun_pending)
		{
			m_status |= SR_OVRN;
			m_overrun_pending = false;
		}
		else
		{
			m_status &= ~SR_OVRN;
			m_status &= ~SR_RDRF;
		}

		if (m_dcd_irq_pending == DCD_IRQ_READ_DATA)
		{
			m_dcd_irq_pending = DCD_IRQ_NONE;
		}

		update_irq();
	}

	return m_rdr;
}

void acia6850_device::write(offs_t offset, uint8_t data)
{
	if (BIT(offset, 0))
		data_w(data);
	else
		control_w(data);
}

uint8_t acia6850_device::read(offs_t offset)
{
	return BIT(offset, 0) ? data_r() : status_r();
}

DECLARE_WRITE_LINE_MEMBER( acia6850_device::write_cts )
{
	if (state)
	{
		m_status |= SR_CTS;
	}
	else
	{
		m_status &= ~SR_CTS;
	}
}

DECLARE_WRITE_LINE_MEMBER( acia6850_device::write_dcd )
{
	m_dcd = state;
}

WRITE_LINE_MEMBER( acia6850_device::write_rxc )
{
	if (m_rxc != state)
	{
		m_rxc = state;

		if (state && m_divide > 0)
		{
			if (m_dcd)
			{
				if (!(m_status & SR_DCD))
				{
					m_status |= SR_DCD;
					m_dcd_irq_pending = DCD_IRQ_READ_STATUS;
				}

				m_rx_state = STATE_START;
				m_rx_counter = 0;
			}
			else
			{
				if (m_dcd_irq_pending == DCD_IRQ_NONE)
				{
					m_status &= ~SR_DCD;
				}

				m_rx_counter++;

				switch (m_rx_state)
				{
				case STATE_START:
					if (m_rxd == 0)
					{
						if (m_rx_counter == 1)
						{
							LOG("MC6850 '%s': RX START BIT\n", tag());
						}

						if (m_rx_counter >= m_divide / 2)
						{
							m_rx_state = STATE_DATA;
							m_rx_counter = 0;
							m_rx_shift = 0;
							m_rx_parity = 0;
							m_rx_bits = 0;
						}
					}
					else
					{
						if (m_rx_counter != 1)
						{
							LOG("MC6850 '%s': RX false START BIT\n", tag());
						}

						m_rx_counter = 0;
					}
					break;

				case STATE_DATA:
					if (m_rx_counter == m_divide)
					{
						m_rx_counter = 0;

						if (m_rx_bits < m_bits)
						{
							LOG("MC6850 '%s': RX DATA BIT %d %d\n", tag(), m_rx_bits, m_rxd);
						}
						else
						{
							LOG("MC6850 '%s': RX PARITY BIT %x\n", tag(), m_rxd);
						}

						if (m_rxd)
						{
							m_rx_shift |= 1 << m_rx_bits;
						}

						m_rx_bits++;

						m_rx_parity ^= m_rxd;

						if ((m_rx_bits == m_bits && m_parity == PARITY_NONE) ||
							(m_rx_bits == (m_bits + 1) && m_parity != PARITY_NONE))
						{
							if (m_status & SR_RDRF)
							{
								m_overrun_pending = true;
							}
							else
							{
								/// TODO: find out if this is the correct place to calculate parity
								if (m_parity == PARITY_ODD)
								{
									m_rx_parity = !m_rx_parity;
								}

								if (m_parity != PARITY_NONE && m_rx_parity)
								{
									m_status |= SR_PE;
								}
								else
								{
									m_status &= ~SR_PE;
								}

								m_rdr = m_rx_shift;

								if (m_bits == 7 && m_parity != PARITY_NONE)
								{
									m_rdr &= 0x7f;
								}

								m_status |= SR_RDRF;
							}

							m_rx_state = STATE_STOP;
						}
					}
					break;

				case STATE_STOP:
					if (m_rx_counter == m_divide)
					{
						m_rx_counter = 0;

						LOG("MC6850 '%s': RX STOP BIT\n", tag());

						if (!m_rxd)
						{
							m_status |= SR_FE;
						}
						else
						{
							m_status &= ~SR_FE;
						}

						/// TODO: find out if 6850 only waits for 1 STOP bit when receiving
						m_rx_state = STATE_START;
					}
					break;
				}
			}

			update_irq();
		}
	}
}

DECLARE_WRITE_LINE_MEMBER( acia6850_device::write_rxd )
{
	m_rxd = state;
}

WRITE_LINE_MEMBER( acia6850_device::write_txc )
{
	if (m_txc != state)
	{
		m_txc = state;

		if (!state && m_divide > 0)
		{
			m_tx_counter++;

			switch (m_tx_state)
			{
			case STATE_START:
				m_tx_counter = 0;

				if (!(m_status & SR_TDRE) && !(m_status & SR_CTS))
				{
					LOG("MC6850 '%s': TX DATA %x\n", tag(), m_tdr);

					m_tx_state = STATE_DATA;
					m_tx_shift = m_tdr;
					m_tx_bits = 0;
					m_tx_parity = 0;
					m_status |= SR_TDRE;

					LOG("MC6850 '%s': TX START BIT\n", tag());

					output_txd(0);
				}
				else
				{
					/// TODO: find out if break stops transmitter
					output_txd(!m_brk);
				}
				break;

			case STATE_DATA:
				if (m_tx_counter == m_divide)
				{
					m_tx_counter = 0;

					if (m_tx_bits < m_bits)
					{
						output_txd((m_tx_shift >> m_tx_bits) & 1);

						m_tx_bits++;
						m_tx_parity ^= m_txd;

						LOG("MC6850 '%s': TX DATA BIT %d %d\n", tag(), m_tx_bits, m_txd);
					}
					else if (m_tx_bits == m_bits && m_parity != PARITY_NONE)
					{
						m_tx_bits++;

						/// TODO: find out if this is the correct place to calculate parity
						if (m_parity == PARITY_ODD)
						{
							m_tx_parity = !m_tx_parity;
						}

						output_txd(m_tx_parity);

						LOG("MC6850 '%s': TX PARITY BIT %d\n", tag(), m_txd);
					}
					else
					{
						m_tx_state = STATE_STOP;
						m_tx_bits = 0;

						output_txd(1);
					}
				}
				break;

			case STATE_STOP:
				if (m_tx_counter == m_divide)
				{
					m_tx_counter = 0;

					m_tx_bits++;

					LOG("MC6850 '%s': TX STOP BIT %d\n", tag(), m_tx_bits);

					if (m_tx_bits == m_stopbits)
					{
						m_tx_state = STATE_START;
					}
				}
				break;
			}
		}

		update_irq();
	}
}

void acia6850_device::output_txd(int txd)
{
	if (m_txd != txd)
	{
		m_txd = txd;

		m_txd_handler(m_txd);
	}
}

void acia6850_device::output_rts(int rts)
{
	if (m_rts != rts)
	{
		m_rts = rts;
		m_rts_handler(m_rts);
	}
}

void acia6850_device::output_irq(int irq)
{
	if (m_irq != irq)
	{
		m_irq = irq;

		machine().scheduler().synchronize(timer_expired_delegate(FUNC(acia6850_device::delayed_output_irq), this), irq);
	}
}

TIMER_CALLBACK_MEMBER(acia6850_device::delayed_output_irq)
{
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
