/*
    68681 DUART

    Written by Mariusz Wojcieszek
    Updated by Jonathan Gevaryahu AKA Lord Nightmare
    Improved interrupt handling by R. Belmont
    Rewrite and modernization in progress by R. Belmont
*/

#include "emu.h"
#include "n68681.h"

#define VERBOSE 0
#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

static const char *const duart68681_reg_read_names[0x10] =
{
	"MRA", "SRA", "BRG Test", "RHRA", "IPCR", "ISR", "CTU", "CTL", "MRB", "SRB", "1X/16X Test", "RHRB", "IVR", "Input Ports", "Start Counter", "Stop Counter"
};

static const char *const duart68681_reg_write_names[0x10] =
{
	"MRA", "CSRA", "CRA", "THRA", "ACR", "IMR", "CRUR", "CTLR", "MRB", "CSRB", "CRB", "THRB", "IVR", "OPCR", "Set OP Bits", "Reset OP Bits"
};

#define INT_INPUT_PORT_CHANGE		0x80
#define INT_DELTA_BREAK_B			0x40
#define INT_RXRDY_FFULLB			0x20
#define INT_TXRDYB					0x10
#define INT_COUNTER_READY			0x08
#define INT_DELTA_BREAK_A			0x04
#define INT_RXRDY_FFULLA			0x02
#define INT_TXRDYA					0x01

#define STATUS_RECEIVED_BREAK		0x80
#define STATUS_FRAMING_ERROR		0x40
#define STATUS_PARITY_ERROR			0x20
#define STATUS_OVERRUN_ERROR		0x10
#define STATUS_TRANSMITTER_EMPTY	0x08
#define STATUS_TRANSMITTER_READY	0x04
#define STATUS_FIFO_FULL			0x02
#define STATUS_RECEIVER_READY		0x01

#define MODE_RX_INT_SELECT_BIT		0x40

// device type definition
const device_type DUARTN68681 = &device_creator<duartn68681_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

duartn68681_device::duartn68681_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DUARTN68681, "DUART 68681 (new)", tag, owner, clock),
	device_serial_interface(mconfig, *this)
{
}

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

void duartn68681_device::device_start()
{
	m_out_irq_func.resolve(m_out_irq_cb, *this);
	m_out_a_tx_func.resolve(m_out_a_tx_cb, *this);
	m_out_b_tx_func.resolve(m_out_b_tx_cb, *this);
	m_in_port_func.resolve(m_in_port_cb, *this);
	m_out_port_func.resolve(m_out_port_cb, *this);

	channel[0].tx_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(duartn68681_device::tx_timer_callback),this), NULL);
	channel[1].tx_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(duartn68681_device::tx_timer_callback),this), NULL);
	duart_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(duartn68681_device::duart_timer_callback),this), NULL);

	save_item(NAME(ACR));
	save_item(NAME(IMR));
	save_item(NAME(ISR));
	save_item(NAME(IVR));
	save_item(NAME(OPCR));
	save_item(NAME(CTR));
	save_item(NAME(IP_last_state));
	save_item(NAME(half_period));

	save_item(NAME(channel[0].CR));
	save_item(NAME(channel[0].CSR));
	save_item(NAME(channel[0].MR1));
	save_item(NAME(channel[0].MR2));
	save_item(NAME(channel[0].MR_ptr));
	save_item(NAME(channel[0].SR));
	save_item(NAME(channel[0].baud_rate));
	save_item(NAME(channel[0].rx_enabled));
	save_item(NAME(channel[0].rx_fifo));
	save_item(NAME(channel[0].rx_fifo_read_ptr));
	save_item(NAME(channel[0].rx_fifo_write_ptr));
	save_item(NAME(channel[0].rx_fifo_num));
	save_item(NAME(channel[0].tx_enabled));
	save_item(NAME(channel[0].tx_data));
	save_item(NAME(channel[0].tx_ready));

	save_item(NAME(channel[1].CR));
	save_item(NAME(channel[1].CSR));
	save_item(NAME(channel[1].MR1));
	save_item(NAME(channel[1].MR2));
	save_item(NAME(channel[1].MR_ptr));
	save_item(NAME(channel[1].SR));
	save_item(NAME(channel[1].baud_rate));
	save_item(NAME(channel[1].rx_enabled));
	save_item(NAME(channel[1].rx_fifo));
	save_item(NAME(channel[1].rx_fifo_read_ptr));
	save_item(NAME(channel[1].rx_fifo_write_ptr));
	save_item(NAME(channel[1].rx_fifo_num));
	save_item(NAME(channel[1].tx_enabled));
	save_item(NAME(channel[1].tx_data));
	save_item(NAME(channel[1].tx_ready));
}

/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

void duartn68681_device::device_reset()
{
	emu_timer *save0, *save1;

	ACR = 0;  /* Interrupt Vector Register */
	IVR = 0x0f;  /* Interrupt Vector Register */
	IMR = 0;  /* Interrupt Mask Register */
	ISR = 0;  /* Interrupt Status Register */
	OPCR = 0; /* Output Port Conf. Register */
	OPR = 0;  /* Output Port Register */
	CTR.d = 0;  /* Counter/Timer Preset Value */
	IP_last_state = 0;  /* last state of IP bits */
	// "reset clears internal registers (SRA, SRB, IMR, ISR, OPR, OPCR) puts OP0-7 in the high state, stops the counter/timer, and puts channels a/b in the inactive state"
	save0 = channel[0].tx_timer;
	save1 = channel[1].tx_timer;
	memset(channel, 0, sizeof(channel));
	channel[0].tx_timer = save0;
	channel[1].tx_timer = save1;

	m_out_port_func(0, OPR ^ 0xff);

	// reset timers
	channel[0].tx_timer->adjust(attotime::never);
	channel[1].tx_timer->adjust(attotime::never, 1);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void duartn68681_device::device_config_complete()
{
	m_shortname = "dun68681";

	// inherit a copy of the static data
	const duartn68681_config *intf = reinterpret_cast<const duartn68681_config *>(static_config());
	if (intf != NULL)
	{
		*static_cast<duartn68681_config *>(this) = *intf;
	}
	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_irq_cb, 0, sizeof(m_out_irq_cb));
		memset(&m_out_a_tx_cb, 0, sizeof(m_out_a_tx_cb));
		memset(&m_out_b_tx_cb, 0, sizeof(m_out_b_tx_cb));
		memset(&m_in_port_cb, 0, sizeof(m_in_port_cb));
		memset(&m_out_port_cb, 0, sizeof(m_out_port_cb));
	}
}

void duartn68681_device::update_interrupts()
{
	/* update SR state and update interrupt ISR state for the following bits:
    SRn: bits 7-4: handled elsewhere.
    SRn: bit 3 (TxEMTn) (we can assume since we're not actually emulating the delay/timing of sending bits, that as long as TxRDYn is set, TxEMTn is also set since the transmit byte has 'already happened', therefore TxEMTn is always 1 assuming tx is enabled on channel n and the MSR2n mode is 0 or 2; in mode 1 it is explicitly zeroed, and mode 3 is undefined)
    SRn: bit 2 (TxRDYn) (we COULD assume since we're not emulating delay and timing output, that as long as tx is enabled on channel n, TxRDY is 1 for channel n and the MSR2n mode is 0 or 2; in mode 1 it is explicitly zeroed, and mode 3 is undefined; however, tx_ready is already nicely handled for us elsewhere, so we can use that instead for now, though we may need to retool that code as well)
    SRn: bit 1 (FFULLn) (this bit we actually emulate; if the receive fifo for channel n is full, this bit is 1, otherwise it is 0. the receive fifo should be three words long.)
    SRn: bit 0 (RxRDYn) (this bit we also emulate; the bit is always asserted if the receive fifo is not empty)
    ISR: bit 7: Input Port change; this should be handled elsewhere, on the input port handler
    ISR: bit 6: Delta Break B; this should be handled elsewhere, on the data receive handler
    ISR: bit 5: RxRDYB/FFULLB: this is handled here; depending on whether MSR1B bit 6 is 0 or 1, this bit holds the state of SRB bit 0 or bit 1 respectively
    ISR: bit 4: TxRDYB: this is handled here; it mirrors SRB bit 2
    ISR: bit 3: Counter ready; this should be handled by the timer generator
    ISR: bit 2: Delta Break A; this should be handled elsewhere, on the data receive handler
    ISR: bit 1: RxRDYA/FFULLA: this is handled here; depending on whether MSR1A bit 6 is 0 or 1, this bit holds the state of SRA bit 0 or bit 1 respectively
    ISR: bit 0: TxRDYA: this is handled here; it mirrors SRA bit 2
    */
	UINT8 ch = 0;
	//logerror("DEBUG: 68681 int check: upon func call, SRA is %02X, SRB is %02X, ISR is %02X\n", duart68681->channel[0].SR, duart68681->channel[1].SR, duart68681->ISR);
	for (ch = 0; ch < 2; ch++)
	{
		//if ( duart68681->channel[ch].rx_enabled )
		//{
			if ( channel[ch].rx_fifo_num > 0 )
			{
				channel[ch].SR |= STATUS_RECEIVER_READY;
			}
			else
			{
				channel[ch].SR &= ~STATUS_RECEIVER_READY;
			}
			if ( channel[ch].rx_fifo_num == MC68681_RX_FIFO_SIZE )
			{
				channel[ch].SR |= STATUS_FIFO_FULL;
			}
			else
			{
				channel[ch].SR &= ~STATUS_FIFO_FULL;
			}
		//}
		//else
		//{
		//duart68681->channel[ch].SR &= ~STATUS_RECEIVER_READY;
		//duart68681->channel[ch].SR &= ~STATUS_FIFO_FULL;
		//}
		// Handle the TxEMT and TxRDY bits based on mode
		switch( channel[ch].MR2&0xC0) // what mode are we in?
			{
			case 0x00: // normal mode
				if ( channel[ch].tx_enabled )
				{
					channel[ch].SR |= STATUS_TRANSMITTER_EMPTY;
				}
				else
				{
					channel[ch].SR &= ~STATUS_TRANSMITTER_EMPTY;
				}
			break;
			case 0x40: // automatic echo mode
				channel[ch].SR &= ~STATUS_TRANSMITTER_EMPTY;
				channel[ch].SR &= ~STATUS_TRANSMITTER_READY;
			break;
			case 0x80: // local loopback mode
				if ( channel[ch].tx_enabled )
				{
					channel[ch].SR |= STATUS_TRANSMITTER_EMPTY;
				}
				else
				{
					channel[ch].SR &= ~STATUS_TRANSMITTER_EMPTY;
				}
			break;
			case 0xC0: // remote loopback mode
				// write me, what the txrdy/txemt regs do for remote loopback mode is undocumented afaik, for now just clear both
				channel[ch].SR &= ~STATUS_TRANSMITTER_EMPTY;
				channel[ch].SR &= ~STATUS_TRANSMITTER_READY;
			break;
			}
		// now handle the ISR bits
		if ( channel[ch].SR & STATUS_TRANSMITTER_READY )
		{
			if (ch == 0)
				ISR |= INT_TXRDYA;
			else
				ISR |= INT_TXRDYB;
		}
		else
		{
			if (ch == 0)
				ISR &= ~INT_TXRDYA;
			else
				ISR &= ~INT_TXRDYB;
		}
		//logerror("DEBUG: 68681 int check: before receiver test, SR%c is %02X, ISR is %02X\n", (ch+0x41), duart68681->channel[ch].SR, duart68681->ISR);
		if ( channel[ch].MR1 & MODE_RX_INT_SELECT_BIT )
		{
			if ( channel[ch].SR & STATUS_FIFO_FULL )
			{
				ISR |= ((ch == 0) ? INT_RXRDY_FFULLA : INT_RXRDY_FFULLB);
			}
			else
			{
				ISR &= ((ch == 0) ? ~INT_RXRDY_FFULLA : ~INT_RXRDY_FFULLB);
			}
		}
		else
		{
			if ( channel[ch].SR & STATUS_RECEIVER_READY )
			{
				ISR |= ((ch == 0) ? INT_RXRDY_FFULLA : INT_RXRDY_FFULLB);
			}
			else
			{
				ISR &= ((ch == 0) ? ~INT_RXRDY_FFULLA : ~INT_RXRDY_FFULLB);
			}
		}
		//logerror("DEBUG: 68681 int check: after receiver test, SR%c is %02X, ISR is %02X\n", (ch+0x41), duart68681->channel[ch].SR, duart68681->ISR);
	}
	if ( (ISR & IMR) != 0 )
	{
		LOG(( "68681: Interrupt line active (IMR & ISR = %02X)\n", (ISR & IMR) ));
		m_out_irq_func(ASSERT_LINE);
	}
    else
    {
		LOG(( "68681: Interrupt line not active (IMR & ISR = %02X)\n", ISR & IMR));
		m_out_irq_func(CLEAR_LINE);
    }
};

double duartn68681_device::duart68681_get_ct_rate()
{
	double rate = 0.0f;

	if (ACR & 0x40)
	{
		// Timer mode
		switch ((ACR >> 4) & 3)
		{
			case 0: // IP2
			case 1: // IP2 / 16
				//logerror( "68681 (%s): Unhandled timer/counter mode %d\n", duart68681->tag(), (duart68681->ACR >> 4) & 3);
				rate = clock();
				break;
			case 2: // X1/CLK
				rate = clock();
				break;
			case 3: // X1/CLK / 16
				rate = clock() / 16;
				break;
		}
	}
	else
	{
		// Counter mode
		switch ((ACR >> 4) & 3)
		{
			case 0: // IP2
			case 1: // TxCA
			case 2: // TxCB
				//logerror( "68681 (%s): Unhandled timer/counter mode %d\n", device->tag(), (duart68681->ACR >> 4) & 3);
				rate = clock();
				break;
			case 3: // X1/CLK / 16
				rate = clock() / 16;
				break;
		}
	}

	return rate;
}

UINT16 duartn68681_device::duart68681_get_ct_count()
{
	double clock = duart68681_get_ct_rate();
	return (duart_timer->remaining() * clock).as_double();
}

void duartn68681_device::duart68681_start_ct(int count)
{
	double clock = duart68681_get_ct_rate();
	duart_timer->adjust(attotime::from_hz(clock) * count, 0);
}

TIMER_CALLBACK_MEMBER( duartn68681_device::duart_timer_callback )
{
	if (ACR & 0x40)
	{
		// Timer mode
		half_period ^= 1;

		// TODO: Set OP3

		if (!half_period)
		{
			ISR |= INT_COUNTER_READY;
			update_interrupts();
		}

		int count = MAX(CTR.w.l, 1);
		duart68681_start_ct(count);
	}
	else
	{
		// Counter mode
		ISR |= INT_COUNTER_READY;
		update_interrupts();
		duart68681_start_ct(0xffff);
	}

};

void duartn68681_device::duart68681_write_MR(int ch, UINT8 data)
{
	if ( channel[ch].MR_ptr == 0 )
	{
		channel[ch].MR1 = data;
		channel[ch].MR_ptr = 1;
	}
	else
	{
		channel[ch].MR2 = data;
	}
	update_interrupts();
};

void duartn68681_device::duart68681_write_CSR(int ch, UINT8 data, UINT8 inACR)
{
	static const int baud_rate_ACR_0[] = { 50, 110, 134, 200, 300, 600, 1200, 1050, 2400, 4800, 7200, 9600, 38400, 0, 0, 0 };
	static const int baud_rate_ACR_1[] = { 75, 110, 134, 150, 300, 600, 1200, 2000, 2400, 4800, 1800, 9600, 19200, 0, 0, 0 };

	channel[ch].CSR = data;

	if ( BIT(inACR,7) == 0 )
	{
		channel[ch].baud_rate = baud_rate_ACR_0[data & 0x0f];

		if (ch == 0)
		{
			if ((data & 0xf) == 0xe)
			{
				channel[ch].baud_rate = ip3clk/16;
			}
			else if ((data & 0xf) == 0xf)
			{
				channel[ch].baud_rate = ip3clk;
			}
		}
		else if (ch == 1)
		{
			if ((data & 0xf) == 0xe)
			{
				channel[ch].baud_rate = ip5clk/16;
			}
			else if ((data & 0xf) == 0xf)
			{
				channel[ch].baud_rate = ip5clk;
			}
		}
	}
	else
	{
		channel[ch].baud_rate = baud_rate_ACR_1[data & 0x0f];
	}
	if ( channel[ch].baud_rate == 0 )
	{
		LOG(( "Unsupported transmitter clock: channel %d, clock select = %02x\n", ch, data ));
	}
};

void duartn68681_device::duart68681_write_CR(int ch, UINT8 data)
{
	channel[ch].CR = data;

	switch( (data >> 4) & 0x07 )
	{
		case 0: /* No command */
			break;
		case 1: /* Reset MR pointer. Causes the Channel A MR pointer to point to MR1 */
			channel[ch].MR_ptr = 0;
			break;
		case 2: /* Reset channel A receiver (disable receiver and flush fifo) */
			channel[ch].rx_enabled = 0;
			channel[ch].SR &= ~STATUS_RECEIVER_READY;
			channel[ch].SR &= ~STATUS_OVERRUN_ERROR; // is this correct?
			channel[ch].rx_fifo_read_ptr = 0;
			channel[ch].rx_fifo_write_ptr = 0;
			channel[ch].rx_fifo_num = 0;
			break;
		case 3: /* Reset channel A transmitter */
			channel[ch].tx_enabled = 0;
			channel[ch].SR &= ~STATUS_TRANSMITTER_READY;
			if (ch == 0)
				ISR &= ~INT_TXRDYA;
			else
				ISR &= ~INT_TXRDYB;
			channel[ch].tx_timer->adjust(attotime::never, ch);
            break;
		case 4: /* Reset Error Status */
			channel[ch].SR &= ~(STATUS_RECEIVED_BREAK | STATUS_FRAMING_ERROR | STATUS_PARITY_ERROR | STATUS_OVERRUN_ERROR);
			break;
		case 5: /* Reset Channel break change interrupt */
			if ( ch == 0 )
			{
				ISR &= ~INT_DELTA_BREAK_A;
			}
			else
			{
				ISR &= ~INT_DELTA_BREAK_B;
			}
			break;
		/* TODO: case 6 and case 7 are start break and stop break respectively, which start or stop holding the TxDA or TxDB line low (space) after whatever data is in the buffer finishes transmitting (following the stop bit?), or after two bit-times if no data is being transmitted  */
		default:
			LOG(( "68681: Unhandled command (%x) in CR%d\n", (data >> 4) & 0x07, ch ));
			break;
	}

	if (BIT(data, 0)) {
		channel[ch].rx_enabled = 1;
	}
	if (BIT(data, 1)) {
		channel[ch].rx_enabled = 0;
		channel[ch].SR &= ~STATUS_RECEIVER_READY;
	}

	if (BIT(data, 2)) {
		channel[ch].tx_enabled = 1;
		channel[ch].tx_ready = 1;
		channel[ch].SR |= STATUS_TRANSMITTER_READY;
		if (ch == 0)
			ISR |= INT_TXRDYA;
		else
			ISR |= INT_TXRDYB;
	}
	if (BIT(data, 3)) {
		channel[ch].tx_enabled = 0;
		channel[ch].tx_ready = 0;
		channel[ch].SR &= ~STATUS_TRANSMITTER_READY;
		if (ch == 0)
			ISR &= ~INT_TXRDYA;
		else
			ISR &= ~INT_TXRDYB;
	}

    update_interrupts();
};

UINT8 duartn68681_device::duart68681_read_rx_fifo(int ch)
{
	UINT8 r;

	if ( channel[ch].rx_fifo_num == 0 )
	{
		LOG(( "68681: rx fifo underflow\n" ));
		return 0x0;
	}

	r = channel[ch].rx_fifo[channel[ch].rx_fifo_read_ptr++];
	if ( channel[ch].rx_fifo_read_ptr == MC68681_RX_FIFO_SIZE )
	{
		channel[ch].rx_fifo_read_ptr = 0;
	}

	channel[ch].rx_fifo_num--;
	update_interrupts();

	return r;
};

TIMER_CALLBACK_MEMBER( duartn68681_device::tx_timer_callback )
{
	int ch = param & 1;

	// send the byte unless we're in loopback mode;
	// in loopback mode do NOT 'actually' send the byte: the TXn pin is held high when loopback mode is on.
	if ((channel[ch].MR2&0xC0) != 0x80)
	{
		if (ch == 0)
		{
			m_out_a_tx_func(0, channel[0].tx_data);
		}
		else if (ch == 1)
		{
			m_out_b_tx_func(0, channel[1].tx_data);
		}
	}

	// if local loopback is on, write the transmitted data as if a byte had been received
	if ((channel[ch].MR2 & 0xC0) == 0x80)
	{
		if (channel[ch].rx_fifo_num >= MC68681_RX_FIFO_SIZE)
		{
			LOG(( "68681: FIFO overflow\n" ));
			channel[ch].SR |= STATUS_OVERRUN_ERROR;
		}
		else
		{
			channel[ch].rx_fifo[channel[ch].rx_fifo_write_ptr++]
					= channel[ch].tx_data;
			if (channel[ch].rx_fifo_write_ptr == MC68681_RX_FIFO_SIZE)
			{
				channel[ch].rx_fifo_write_ptr = 0;
			}
			channel[ch].rx_fifo_num++;
		}
	}

	channel[ch].tx_ready = 1;
	channel[ch].SR |= STATUS_TRANSMITTER_READY;

	if (ch == 0)
		ISR |= INT_TXRDYA;
	else
		ISR |= INT_TXRDYB;

	update_interrupts();
	channel[ch].tx_timer->adjust(attotime::never, ch);
};

void duartn68681_device::duart68681_write_TX(int ch, UINT8 data)
{
	attotime period;

	channel[ch].tx_data = data;

	channel[ch].tx_ready = 0;
	channel[ch].SR &= ~STATUS_TRANSMITTER_READY;

	if (ch == 0)
		ISR &= ~INT_TXRDYA;
	else
		ISR &= ~INT_TXRDYB;

	update_interrupts();

	period = attotime::from_hz(channel[ch].baud_rate / 10 );
	channel[ch].tx_timer->adjust(period, ch);
};

READ8_MEMBER( duartn68681_device::read )
{
	UINT8 r = 0xff;

	offset &= 0xf;

	LOG(( "Reading 68681 (%s) reg %x (%s) ", tag(), offset, duart68681_reg_read_names[offset] ));

	switch (offset)
	{
		case 0x00: /* MR1A/MR2A */
			if ( channel[0].MR_ptr == 0 )
			{
				r = channel[0].MR1;
				channel[0].MR_ptr = 1;
			}
			else
			{
				r = channel[0].MR2;
			}
			break;

		case 0x01: /* SRA */
			r = channel[0].SR;
			break;

		case 0x03: /* Rx Holding Register A */
			r = duart68681_read_rx_fifo(0);
			break;

		case 0x04: /* IPCR */
		{
			UINT8 IP = m_in_port_func(0);

			r = (((IP_last_state ^ IP) & 0x0f) << 4) | (IP & 0x0f);
			IP_last_state = IP;
			ISR &= ~INT_INPUT_PORT_CHANGE;
			update_interrupts();
		}
		break;

		case 0x05: /* ISR */
			r = ISR;
			break;

		case 0x06: /* CUR */
			r = duart68681_get_ct_count() >> 8;
			break;

		case 0x07: /* CLR */
			r = duart68681_get_ct_count() & 0xff;
			break;

		case 0x08: /* MR1B/MR2B */
			if ( channel[1].MR_ptr == 0 )
			{
				r = channel[1].MR1;
				channel[1].MR_ptr = 1;
			}
			else
			{
				r = channel[1].MR2;
			}
			break;

		case 0x09: /* SRB */
			r = channel[1].SR;
			break;

		case 0x0b: /* RHRB */
			r = duart68681_read_rx_fifo(1);
			break;

		case 0x0d: /* IP */
			r = m_in_port_func(0);
			break;

		case 0x0e: /* Start counter command */
		{
			if (ACR & 0x40)
			{
				// Reset the timer
				half_period = 0;
				// TODO: Set OP3 to 1
			}

			int count = MAX(CTR.w.l, 1);
			duart68681_start_ct(count);
			break;
		}

		case 0x0f: /* Stop counter command */
			ISR &= ~INT_COUNTER_READY;

			// Stop the counter only
			if (!(ACR & 0x40))
				duart_timer->adjust(attotime::never);

			update_interrupts();
			break;

		default:
			LOG(( "Reading unhandled 68681 reg %x\n", offset ));
			break;
	}
	LOG(("returned %02x\n", r));

	return r;
}

WRITE8_MEMBER( duartn68681_device::write )
{
	offset &= 0x0f;
	LOG(( "Writing 68681 (%s) reg %x (%s) with %04x\n", tag(), offset, duart68681_reg_write_names[offset], data ));
	switch(offset)
	{
		case 0x00: /* MRA */
			duart68681_write_MR(0, data);
			break;

		case 0x01: /* CSRA */
			duart68681_write_CSR(0, data, ACR);
			break;

		case 0x02: /* CRA */
			duart68681_write_CR(0, data);
			break;

		case 0x03: /* THRA */
			duart68681_write_TX(0, data);
			break;

		case 0x04: /* ACR */
		{
			UINT8 old_acr = ACR;
			ACR = data;

			//       bits 6-4: Counter/Timer Mode And Clock Source Select
			//       bits 3-0: IP3-0 Change-Of-State Interrupt Enable
			if ((old_acr ^ data) & 0x40)
			{
				if (data & 0x40)
				{
					// Entering timer mode
					UINT16 count = MAX(CTR.w.l, 1);
					half_period = 0;

					// TODO: Set OP3
					duart68681_start_ct(count);
				}
				else
				{
					// Leaving timer mode (TODO: is this correct?)
					duart_timer->adjust(attotime::never);
				}
			}

			duart68681_write_CSR(0, channel[0].CSR, data);
			duart68681_write_CSR(1, channel[1].CSR, data);
			update_interrupts(); // need to add ACR checking for IP delta ints
			break;
		}
		case 0x05: /* IMR */
			IMR = data;
			update_interrupts();
			break;

		case 0x06: /* CTUR */
			CTR.b.h = data;
			break;

		case 0x07: /* CTLR */
			CTR.b.l = data;
			break;

		case 0x08: /* MRB */
			duart68681_write_MR(1, data);
			break;

		case 0x09: /* CSRB */
			duart68681_write_CSR(1, data, ACR);
			break;

		case 0x0a: /* CRB */
			duart68681_write_CR(1, data);
			break;

		case 0x0b: /* THRB */
			duart68681_write_TX(1, data);
			break;

		case 0x0c: /* IVR */
			IVR = data;
			break;

		case 0x0d: /* OPCR */
			if (data != 0x00)
				logerror( "68681 (%s): Unhandled OPCR value: %02x\n", tag(), data);
			OPCR = data;
			break;

		case 0x0e: /* Set Output Port Bits */
			OPR |= data;
			m_out_port_func(0, OPR ^ 0xff);
			break;

		case 0x0f: /* Reset Output Port Bits */
			OPR &= ~data;
			m_out_port_func(0, OPR ^ 0xff);
			break;
	}
}

void duartn68681_device::duart68681_rx_data(int ch, UINT8 data)
{
	if ( channel[ch].rx_enabled )
	{
		if ( channel[ch].rx_fifo_num >= MC68681_RX_FIFO_SIZE )
		{
			LOG(( "68681: FIFO overflow\n" ));
			channel[ch].SR |= STATUS_OVERRUN_ERROR;
			return;
		}
		channel[ch].rx_fifo[channel[ch].rx_fifo_write_ptr++] = data;
		if ( channel[ch].rx_fifo_write_ptr == MC68681_RX_FIFO_SIZE )
		{
			channel[ch].rx_fifo_write_ptr = 0;
		}
		channel[ch].rx_fifo_num++;
		update_interrupts();
	}
};

// serial device virtual overrides
void duartn68681_device::rcv_complete()
{
}

void duartn68681_device::tra_complete()
{
}

void duartn68681_device::tra_callback()
{
}

void duartn68681_device::input_callback(UINT8 state) 
{ 
}

