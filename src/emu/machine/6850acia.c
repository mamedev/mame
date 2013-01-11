/*********************************************************************

    6850acia.c

    6850 ACIA code

*********************************************************************/

#include "emu.h"
#include "6850acia.h"


/***************************************************************************
    MACROS
***************************************************************************/

#define LOG 0

#define CR1_0   0x03
#define CR4_2   0x1C
#define CR6_5   0x60
#define CR7     0x80

#define TXD(_data) \
	m_out_tx_func(_data)

#define RTS(_data) \
	m_out_rts_func(_data)

/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

const int acia6850_device::ACIA6850_DIVIDE[3] = { 1, 16, 64 };

const int acia6850_device::ACIA6850_WORD[8][3] =
{
	{ 7, EVEN, 2 },
	{ 7, ODD,  2 },
	{ 7, EVEN, 1 },
	{ 7, ODD,  1 },
	{ 8, NONE, 2 },
	{ 8, NONE, 1 },
	{ 8, EVEN, 1 },
	{ 8, ODD,  1 }
};



/***************************************************************************
    LIVE DEVICE
***************************************************************************/

// device type definition
const device_type ACIA6850 = &device_creator<acia6850_device>;

//-------------------------------------------------
//  acia6850_device - constructor
//-------------------------------------------------

acia6850_device::acia6850_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ACIA6850, "6850 ACIA", tag, owner, clock)
{
	memset(static_cast<acia6850_interface *>(this), 0, sizeof(acia6850_interface));
}


//-------------------------------------------------
//  static_set_interface - set the interface
//  struct
//-------------------------------------------------

void acia6850_device::static_set_interface(device_t &device, const acia6850_interface &interface)
{
	acia6850_device &ptm = downcast<acia6850_device &>(device);
	static_cast<acia6850_interface &>(ptm) = interface;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acia6850_device::device_start()
{
	// resolve callbacks
	m_in_rx_func.resolve(m_in_rx_cb, *this);
	m_out_tx_func.resolve(m_out_tx_cb, *this);
	m_in_cts_func.resolve(m_in_cts_cb, *this);
	m_out_rts_func.resolve(m_out_rts_cb, *this);
	m_in_dcd_func.resolve(m_in_dcd_cb, *this);
	m_out_irq_func.resolve(m_out_irq_cb, *this);

	m_tx_counter = 0;
	m_rx_counter = 0;
	m_rx_timer = timer_alloc(TIMER_ID_RECEIVE);
	m_tx_timer = timer_alloc(TIMER_ID_TRANSMIT);
	m_first_reset = 1;
	m_status_read = 0;
	m_brk = 0;

	m_rx_timer->reset();
	m_tx_timer->reset();

	save_item(NAME(m_ctrl));
	save_item(NAME(m_status));
	save_item(NAME(m_rx_clock));
	save_item(NAME(m_tx_clock));
	save_item(NAME(m_rx_counter));
	save_item(NAME(m_tx_counter));
	save_item(NAME(m_rx_shift));
	save_item(NAME(m_tx_shift));
	save_item(NAME(m_rdr));
	save_item(NAME(m_tdr));
	save_item(NAME(m_rx_bits));
	save_item(NAME(m_tx_bits));
	save_item(NAME(m_rx_parity));
	save_item(NAME(m_tx_parity));
	save_item(NAME(m_tx_int));

	save_item(NAME(m_divide));
	save_item(NAME(m_overrun));
	save_item(NAME(m_reset));
	save_item(NAME(m_first_reset));
	save_item(NAME(m_rts));
	save_item(NAME(m_brk));
	save_item(NAME(m_status_read));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void acia6850_device::device_reset()
{
	int cts = m_in_cts_func();
	int dcd = m_in_dcd_func();

	m_status = (cts << 3) | (dcd << 2) | ACIA6850_STATUS_TDRE;
	m_tdr = 0;
	m_rdr = 0;
	m_tx_shift = 0;
	m_rx_shift = 0;
	m_tx_counter = 0;
	m_rx_counter = 0;

	TXD(1);
	m_overrun = 0;
	m_status_read = 0;
	m_brk = 0;

	m_rx_state = START;
	m_tx_state = START;
	m_irq = 0;

	m_out_irq_func(CLEAR_LINE);

	if (m_first_reset)
	{
		m_first_reset = 0;

		RTS(1);
	}
	else
	{
		RTS(m_rts);
	}
}


//-------------------------------------------------
//  device_timer - handle timer callbacks
//-------------------------------------------------

void acia6850_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_ID_TRANSMIT:
			tx_tick();
			m_tx_counter = 0;
			break;

		case TIMER_ID_RECEIVE:
			rx_tick();
			m_rx_counter = 0;
			break;
	}
}


//-------------------------------------------------
//  acia6850_stat_r - Read Status Register
//-------------------------------------------------

READ8_MEMBER( acia6850_device::status_read )
{
	UINT8 status;

	m_status_read = 1;
	status = m_status;

	if (status & ACIA6850_STATUS_CTS)
	{
		status &= ~ACIA6850_STATUS_TDRE;
	}

	return status;
}


//-------------------------------------------------
//  control_write - Write Control Register
//-------------------------------------------------

WRITE8_MEMBER( acia6850_device::control_write )
{
	if (LOG) logerror("MC6850 '%s' Control: %02x\n", tag(), data);

	// Counter Divide Select Bits

	int divide = data & CR1_0;

	if (divide == 3)
	{
		m_reset = 1;
		device_reset();
	}
	else
	{
		m_reset = 0;
		m_divide = ACIA6850_DIVIDE[divide];
	}

	// Word Select Bits

	int wordsel = (data & CR4_2) >> 2;

	m_bits = ACIA6850_WORD[wordsel][0];
	m_parity = (parity_type)ACIA6850_WORD[wordsel][1];
	m_stopbits = ACIA6850_WORD[wordsel][2];

	// Transmitter Control Bits

	switch ((data & CR6_5) >> 5)
	{
	case 0:
		m_rts = 0;
		RTS(m_rts);

		m_tx_int = 0;
		m_brk = 0;
		break;

	case 1:
		m_rts = 0;
		RTS(m_rts);

		m_tx_int = 1;
		m_brk = 0;
		break;

	case 2:
		m_rts = 1;
		RTS(m_rts);

		m_tx_int = 0;
		m_brk = 0;
		break;

	case 3:
		m_rts = 0;
		RTS(m_rts);

		m_tx_int = 0;
		m_brk = 1;
		break;
	}

	check_interrupts();

	// After writing the word type, set the rx/tx clocks (provided the divide values have changed)

	if ((m_ctrl ^ data) & CR1_0)
	{
		if (!m_reset)
		{
			if (m_rx_clock)
			{
				attotime rx_period = attotime::from_hz(m_rx_clock) *  m_divide;
				m_rx_timer->adjust(rx_period, 0, rx_period);
			}

			if (m_tx_clock)
			{
				attotime tx_period = attotime::from_hz(m_tx_clock) * m_divide;
				m_tx_timer->adjust(tx_period, 0, tx_period);
			}
		}
	}
	m_ctrl = data;
}


//-------------------------------------------------
//  check_interrupts
//-------------------------------------------------

void acia6850_device::check_interrupts()
{
	int irq = (m_tx_int && (m_status & ACIA6850_STATUS_TDRE) && (~m_status & ACIA6850_STATUS_CTS)) ||
		((m_ctrl & 0x80) && ((m_status & (ACIA6850_STATUS_RDRF|ACIA6850_STATUS_DCD)) || m_overrun));

	if (irq != m_irq)
	{
		m_irq = irq;

		if (irq)
		{
			m_status |= ACIA6850_STATUS_IRQ;
			m_out_irq_func(ASSERT_LINE);
		}
		else
		{
			m_status &= ~ACIA6850_STATUS_IRQ;
			m_out_irq_func(CLEAR_LINE);
		}
	}
}


//-------------------------------------------------
//  data_write - Write transmit register
//-------------------------------------------------

WRITE8_MEMBER( acia6850_device::data_write )
{
	if (LOG) logerror("MC6850 '%s' Data: %02x\n", tag(), data);

	if (!m_reset)
	{
		m_tdr = data;
		m_status &= ~ACIA6850_STATUS_TDRE;
		check_interrupts();
	}
	else
	{
		logerror("%s:ACIA %p: Data write while in reset!\n", machine().describe_context(), this);
	}
}


//-------------------------------------------------
//  data_r - Read character
//-------------------------------------------------

READ8_MEMBER( acia6850_device::data_read )
{
	m_status &= ~(ACIA6850_STATUS_RDRF | ACIA6850_STATUS_IRQ | ACIA6850_STATUS_PE);

	if (m_status_read)
	{
		int dcd = m_in_dcd_func();

		m_status_read = 0;
		m_status &= ~(ACIA6850_STATUS_OVRN | ACIA6850_STATUS_DCD);

		if (dcd)
		{
			m_status |= ACIA6850_STATUS_DCD;
		}
	}

	if (m_overrun == 1)
	{
		m_status |= ACIA6850_STATUS_OVRN;
		m_overrun = 0;
	}

	check_interrupts();

	return m_rdr;
}


//-------------------------------------------------
//  tx_tick - Transmit a bit
//-------------------------------------------------

void acia6850_device::tx_tick()
{
	switch (m_tx_state)
	{
		case START:
		{
			if (m_brk)
			{
				// transmit break

				TXD(0);
			}
			else
			{
				int _cts = m_in_cts_func();

				if (_cts)
				{
					m_status |= ACIA6850_STATUS_CTS;
				}
				else
				{
					m_status &= ~ACIA6850_STATUS_CTS;
				}

				check_interrupts();

				if (m_status & ACIA6850_STATUS_TDRE)
				{
					// transmitter idle
					TXD(1);
				}
				else
				{
					// transmit character

					if (LOG) logerror("MC6850 '%s': TX DATA %x\n", tag(), m_tdr);
					if (LOG) logerror("MC6850 '%s': TX START BIT\n", tag());

					TXD(0);

					m_tx_bits = m_bits;
					m_tx_shift = m_tdr;
					m_tx_parity = 0;
					m_tx_state = DATA;
				}
			}
			break;
		}
		case DATA:
		{
			int val = m_tx_shift & 1;
			if (LOG) logerror("MC6850 '%s': TX DATA BIT %x\n", tag(), val);

			TXD(val);
			m_tx_parity ^= val;
			m_tx_shift >>= 1;

			if (--(m_tx_bits) == 0)
			{
				m_tx_state = (m_parity == NONE) ? STOP : PARITY;
			}

			break;
		}
		case PARITY:
		{
			int parity = 0;

			if (m_parity == EVEN)
			{
				parity = (m_tx_parity & 1) ? 1 : 0;
			}
			else
			{
				parity = (m_tx_parity & 1) ? 0 : 1;
			}

			TXD(parity);

			if (LOG) logerror("MC6850 '%s': TX PARITY BIT %x\n", tag(), parity);
			m_tx_state = STOP;
			break;
		}
		case STOP:
		{
			if (LOG) logerror("MC6850 '%s': TX STOP BIT\n", tag());
			TXD(1);

			if (m_stopbits == 1)
			{
				m_tx_state = START;
				m_status |= ACIA6850_STATUS_TDRE;
			}
			else
			{
				m_tx_state = STOP2;
			}
			break;
		}
		case STOP2:
		{
			if (LOG) logerror("MC6850 '%s': TX STOP BIT\n", tag());
			TXD(1);
			m_tx_state = START;
			m_status |= ACIA6850_STATUS_TDRE;
			break;
		}
	}
}


//-------------------------------------------------
//  tx_clock_in - As above, but using the tx pin
//-------------------------------------------------

void acia6850_device::tx_clock_in()
{
	int _cts = m_in_cts_func();

	if (_cts)
	{
		m_status |= ACIA6850_STATUS_CTS;
	}
	else
	{
		m_status &= ~ACIA6850_STATUS_CTS;
	}

	m_tx_counter ++;

	if ( m_tx_counter > m_divide - 1)
	{
		tx_tick();
		m_tx_counter = 0;
	}

}


//-------------------------------------------------
//  rx_tick - Receive a bit
//-------------------------------------------------

void acia6850_device::rx_tick()
{
	int dcd = m_in_dcd_func();

	if (dcd)
	{
		m_status |= ACIA6850_STATUS_DCD;
		check_interrupts();
	}
	else if ((m_status & (ACIA6850_STATUS_DCD | ACIA6850_STATUS_IRQ)) == ACIA6850_STATUS_DCD)
	{
		m_status &= ~ACIA6850_STATUS_DCD;
	}

	if (m_status & ACIA6850_STATUS_DCD)
	{
		m_rx_state = START;
	}
	else
	{
		int rxd = m_in_rx_func();

		switch (m_rx_state)
		{
			case START:
			{
				if (rxd == 0)
				{
					if (LOG) logerror("MC6850 '%s': RX START BIT\n", tag());
					m_rx_shift = 0;
					m_rx_parity = 0;
					m_rx_bits = m_bits;
					m_rx_state = DATA;
				}
				break;
			}
			case DATA:
			{
				if (LOG) logerror("MC6850 '%s': RX DATA BIT %x\n", tag(), rxd);
				m_rx_shift |= rxd ? 0x80 : 0;
				m_rx_parity ^= rxd;

				if (--m_rx_bits == 0)
				{
					if (m_status & ACIA6850_STATUS_RDRF)
					{
						m_overrun = 1;
						check_interrupts();
					}

					m_rx_state = m_parity == NONE ? STOP : PARITY;
				}
				else
				{
					m_rx_shift >>= 1;
				}
				break;
			}
			case PARITY:
			{
				if (LOG) logerror("MC6850 '%s': RX PARITY BIT %x\n", tag(), rxd);
				m_rx_parity ^= rxd;

				if (m_parity == EVEN)
				{
					if (m_rx_parity)
					{
						m_status |= ACIA6850_STATUS_PE;
					}
				}
				else
				{
					if (!m_rx_parity)
					{
						m_status |= ACIA6850_STATUS_PE;
					}
				}

				m_rx_state = STOP;
				break;
			}
			case STOP:
			{
				if (rxd == 1)
				{
					if (LOG) logerror("MC6850 '%s': RX STOP BIT\n", tag());
					if (m_stopbits == 1)
					{
						m_status &= ~ACIA6850_STATUS_FE;

						if (!(m_status & ACIA6850_STATUS_RDRF))
						{
							if (LOG) logerror("MC6850 '%s': RX DATA %x\n", tag(), m_rx_shift);
							m_rdr = m_rx_shift;
							m_status |= ACIA6850_STATUS_RDRF;
							check_interrupts();
						}

						m_rx_state = START;
					}
					else
					{
						m_rx_state = STOP2;
					}
				}
				else
				{
					m_status |= ACIA6850_STATUS_FE;
					m_rx_state = START;
				}
				break;
			}
			case STOP2:
			{
				if (rxd == 1)
				{
					if (LOG) logerror("MC6850 '%s': RX STOP BIT\n", tag());
					m_status &= ~ACIA6850_STATUS_FE;

					if (!(m_status & ACIA6850_STATUS_RDRF))
					{
						if (LOG) logerror("MC6850 '%s': RX DATA %x\n", tag(), m_rx_shift);
						m_rdr = m_rx_shift;
						m_status |= ACIA6850_STATUS_RDRF;
						check_interrupts();
					}

					m_rx_state = START;
				}
				else
				{
					m_status |= ACIA6850_STATUS_FE;
					m_rx_state = START;
				}
				break;
			}
		}
	}
}


//-------------------------------------------------
//  rx_clock_in - As above, but using the rx pin
//-------------------------------------------------

void acia6850_device::rx_clock_in()
{
	int dcd = m_in_dcd_func();

	if (dcd)
	{
		m_status |= ACIA6850_STATUS_DCD;
		check_interrupts();
	}
	else if ((m_status & (ACIA6850_STATUS_DCD|ACIA6850_STATUS_IRQ)) == ACIA6850_STATUS_DCD)
	{
		m_status &= ~ACIA6850_STATUS_DCD;
	}

	m_rx_counter ++;

	if ( m_rx_counter > m_divide - 1)
	{
		rx_tick();
		m_rx_counter = 0;
	}
}


//-------------------------------------------------
//  set_rx_clock - set receiver clock
//-------------------------------------------------

void acia6850_device::set_rx_clock(int clock)
{
	m_rx_clock = clock;

	if (m_rx_clock)
	{
		attotime rx_period = attotime::from_hz(m_rx_clock) * m_divide;
		m_rx_timer->adjust(rx_period, 0, rx_period);
	}
}


//-------------------------------------------------
//  set_tx_clock - set receiver clock
//-------------------------------------------------

void acia6850_device::set_tx_clock(int clock)
{
	m_tx_clock = clock;

	if (m_tx_clock)
	{
		attotime tx_period = attotime::from_hz(m_tx_clock) * m_divide;
		m_tx_timer->adjust(tx_period, 0, tx_period);
	}
}


//-------------------------------------------------
//  receive_data - receive data byte
//-------------------------------------------------

void acia6850_device::receive_data(UINT8 data)
{
	m_rdr = data;
	m_status |= ACIA6850_STATUS_RDRF;
	check_interrupts();
}
