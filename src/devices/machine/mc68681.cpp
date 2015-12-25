// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek, R. Belmont
/*
    68681 DUART

    Written by Mariusz Wojcieszek
    Updated by Jonathan Gevaryahu AKA Lord Nightmare
    Improved interrupt handling by R. Belmont
    Rewrite and modernization in progress by R. Belmont
*/

#include "emu.h"
#include "mc68681.h"

#define VERBOSE 0
#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

static const char *const duart68681_reg_read_names[0x10] =
{
	"MRA", "SRA", "BRG Test", "RHRA", "IPCR", "ISR", "CTU", "CTL", "MRB", "SRB", "1X/16X Test", "RHRB", "IVR", "Input Ports", "Start Counter", "Stop Counter"
};

static const char *const duart68681_reg_write_names[0x10] =
{
	"MRA", "CSRA", "CRA", "THRA", "ACR", "IMR", "CRUR", "CTLR", "MRB", "CSRB", "CRB", "THRB", "IVR", "OPCR", "Set OP Bits", "Reset OP Bits"
};

static const int baud_rate_ACR_0[] = { 50, 110, 134, 200, 300, 600, 1200, 1050, 2400, 4800, 7200, 9600, 38400, 0, 0, 0 };
static const int baud_rate_ACR_1[] = { 75, 110, 134, 150, 300, 600, 1200, 2000, 2400, 4800, 1800, 9600, 19200, 0, 0, 0 };

#define INT_INPUT_PORT_CHANGE       0x80
#define INT_DELTA_BREAK_B           0x40
#define INT_RXRDY_FFULLB            0x20
#define INT_TXRDYB                  0x10
#define INT_COUNTER_READY           0x08
#define INT_DELTA_BREAK_A           0x04
#define INT_RXRDY_FFULLA            0x02
#define INT_TXRDYA                  0x01

#define STATUS_RECEIVED_BREAK       0x80
#define STATUS_FRAMING_ERROR        0x40
#define STATUS_PARITY_ERROR         0x20
#define STATUS_OVERRUN_ERROR        0x10
#define STATUS_TRANSMITTER_EMPTY    0x08
#define STATUS_TRANSMITTER_READY    0x04
#define STATUS_FIFO_FULL            0x02
#define STATUS_RECEIVER_READY       0x01

#define MODE_RX_INT_SELECT_BIT      0x40

#define CHANA_TAG   "cha"
#define CHANB_TAG   "chb"

// device type definition
const device_type MC68681 = &device_creator<mc68681_device>;
const device_type MC68681_CHANNEL = &device_creator<mc68681_channel>;

MACHINE_CONFIG_FRAGMENT( duart68681 )
	MCFG_DEVICE_ADD(CHANA_TAG, MC68681_CHANNEL, 0)
	MCFG_DEVICE_ADD(CHANB_TAG, MC68681_CHANNEL, 0)
MACHINE_CONFIG_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

mc68681_device::mc68681_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MC68681, "MC68681 DUART", tag, owner, clock, "mc68681", __FILE__),
	m_chanA(*this, CHANA_TAG),
	m_chanB(*this, CHANB_TAG),
	write_irq(*this),
	write_a_tx(*this),
	write_b_tx(*this),
	read_inport(*this),
	write_outport(*this),
	ip3clk(0),
	ip4clk(0),
	ip5clk(0),
	ip6clk(0),
	ACR(0),
	m_read_vector(false),
	IP_last_state(0)
{
}

//-------------------------------------------------
//  static_set_clocks - configuration helper to set
//  the external clocks
//-------------------------------------------------

void mc68681_device::static_set_clocks(device_t &device, int clk3, int clk4, int clk5, int clk6)
{
	mc68681_device &duart = downcast<mc68681_device &>(device);
	duart.ip3clk = clk3;
	duart.ip4clk = clk4;
	duart.ip5clk = clk5;
	duart.ip6clk = clk6;
}

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

void mc68681_device::device_start()
{
	write_irq.resolve_safe();
	write_a_tx.resolve_safe();
	write_b_tx.resolve_safe();
	read_inport.resolve();
	write_outport.resolve_safe();

	duart_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mc68681_device::duart_timer_callback),this), nullptr);

	save_item(NAME(ACR));
	save_item(NAME(IMR));
	save_item(NAME(ISR));
	save_item(NAME(IVR));
	save_item(NAME(OPCR));
	save_item(NAME(CTR));
	save_item(NAME(IP_last_state));
	save_item(NAME(half_period));
}

/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

void mc68681_device::device_reset()
{
	ACR = 0;  /* Interrupt Vector Register */
	IVR = 0x0f;  /* Interrupt Vector Register */
	IMR = 0;  /* Interrupt Mask Register */
	ISR = 0;  /* Interrupt Status Register */
	OPCR = 0; /* Output Port Conf. Register */
	OPR = 0;  /* Output Port Register */
	CTR.d = 0;  /* Counter/Timer Preset Value */
	m_read_vector = false;
	// "reset clears internal registers (SRA, SRB, IMR, ISR, OPR, OPCR) puts OP0-7 in the high state, stops the counter/timer, and puts channels a/b in the inactive state"
	IPCR = 0;

	write_outport(OPR ^ 0xff);
}

machine_config_constructor mc68681_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( duart68681 );
}

void mc68681_device::update_interrupts()
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
	if ( (ISR & IMR) != 0 )
	{
		LOG(( "68681: Interrupt line active (IMR & ISR = %02X)\n", (ISR & IMR) ));
		write_irq(ASSERT_LINE);
	}
	else
	{
		LOG(( "68681: Interrupt line not active (IMR & ISR = %02X)\n", ISR & IMR));
		write_irq(CLEAR_LINE);
		m_read_vector = false;  // clear IACK too
	}
}

double mc68681_device::duart68681_get_ct_rate()
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

UINT16 mc68681_device::duart68681_get_ct_count()
{
	double clock = duart68681_get_ct_rate();
	return (duart_timer->remaining() * clock).as_double();
}

void mc68681_device::duart68681_start_ct(int count)
{
	double clock = duart68681_get_ct_rate();
	duart_timer->adjust(attotime::from_hz(clock) * count, 0);
}

TIMER_CALLBACK_MEMBER( mc68681_device::duart_timer_callback )
{
	if (ACR & 0x40)
	{
		// Timer mode
		half_period ^= 1;

		// timer output to bit 3?
		if ((OPCR & 0xc) == 0x4)
		{
			OPR ^= 0x8;
			write_outport(OPR ^ 0xff);
		}

		// timer driving any serial channels?
		if (BIT(ACR, 7) == 1)
		{
			UINT8 csr = m_chanA->get_chan_CSR();

			if ((csr & 0xf0) == 0xd0)   // tx is timer driven
			{
				m_chanA->tx_clock_w(half_period);
			}
			if ((csr & 0x0f) == 0x0d)   // rx is timer driven
			{
				m_chanA->rx_clock_w(half_period);
			}

			csr = m_chanB->get_chan_CSR();
			if ((csr & 0xf0) == 0xd0)   // tx is timer driven
			{
				m_chanB->tx_clock_w(half_period);
			}
			if ((csr & 0x0f) == 0x0d)   // rx is timer driven
			{
				m_chanB->rx_clock_w(half_period);
			}
		}

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

}

READ8_MEMBER( mc68681_device::read )
{
	UINT8 r = 0xff;

	offset &= 0xf;

	LOG(( "Reading 68681 (%s) reg %x (%s) ", tag(), offset, duart68681_reg_read_names[offset] ));

	switch (offset)
	{
		case 0x00: /* MR1A/MR2A */
		case 0x01: /* SRA */
		case 0x03: /* Rx Holding Register A */
			r = m_chanA->read_chan_reg(offset & 3);
			break;

		case 0x04: /* IPCR */
		{
			r = IPCR;

			// reading this clears all the input change bits
			IPCR &= 0x0f;
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
		case 0x09: /* SRB */
		case 0x0b: /* RHRB */
			r = m_chanB->read_chan_reg(offset & 3);
			break;

		case 0x0a: /* 1X/16X Test */
			r = 0x61;   // the old 68681 returned this and it makes Apollo happy
			break;

		case 0x0d: /* IP */
			if (!read_inport.isnull())
			{
				r = read_inport();  // TODO: go away
			}
			else
			{
				r = IP_last_state;
			}

			r |= 0x80;  // bit 7 is always set

			// bit 6 is /IACK (note the active-low)
			if (m_read_vector)
			{
				r &= ~0x40;
			}
			else
			{
				r |= 0x40;
			}
			break;

		case 0x0e: /* Start counter command */
		{
			if (ACR & 0x40)
			{
				// Reset the timer
				half_period = 0;
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

WRITE8_MEMBER( mc68681_device::write )
{
	offset &= 0x0f;
	LOG(( "Writing 68681 (%s) reg %x (%s) with %04x\n", tag(), offset, duart68681_reg_write_names[offset], data ));
	switch(offset)
	{
		case 0x00: /* MRA */
		case 0x01: /* CSRA */
		case 0x02: /* CRA */
		case 0x03: /* THRA */
			m_chanA->write_chan_reg(offset&3, data);
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

					duart68681_start_ct(count);
				}
				else
				{
					// Leaving timer mode (TODO: is this correct?)
					duart_timer->adjust(attotime::never);
				}
			}

			// check for pending input port delta interrupts
			if ((((IPCR>>4) & data) & 0x0f) != 0)
			{
				ISR |= INT_INPUT_PORT_CHANGE;
			}

			m_chanA->ACR_updated();
			m_chanB->ACR_updated();
			m_chanA->update_interrupts();
			m_chanB->update_interrupts();
			update_interrupts();
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
		case 0x09: /* CSRB */
		case 0x0a: /* CRB */
		case 0x0b: /* THRB */
			m_chanB->write_chan_reg(offset&3, data);
			break;

		case 0x0c: /* IVR */
			IVR = data;
			break;

		case 0x0d: /* OPCR */
			if ((data != 0x00) && ((data & 0xc) != 0x4))
				logerror( "68681 (%s): Unhandled OPCR value: %02x\n", tag(), data);
			OPCR = data;
			break;

		case 0x0e: /* Set Output Port Bits */
			OPR |= data;
			write_outport(OPR ^ 0xff);
			break;

		case 0x0f: /* Reset Output Port Bits */
			OPR &= ~data;
			write_outport(OPR ^ 0xff);
			break;
	}
}

WRITE_LINE_MEMBER( mc68681_device::ip0_w )
{
	UINT8 newIP = (IP_last_state & ~0x01) | ((state == ASSERT_LINE) ? 1 : 0);

	if (newIP != IP_last_state)
	{
		IPCR &= ~0x0f;
		IPCR |= (newIP & 0x0f);
		IPCR |= 0x10;

		if (ACR & 1)
		{
			ISR |= INT_INPUT_PORT_CHANGE;
			update_interrupts();
		}
	}

	IP_last_state = newIP;
}

WRITE_LINE_MEMBER( mc68681_device::ip1_w )
{
	UINT8 newIP = (IP_last_state & ~0x02) | ((state == ASSERT_LINE) ? 2 : 0);

	if (newIP != IP_last_state)
	{
		IPCR &= ~0x0f;
		IPCR |= (newIP & 0x0f);
		IPCR |= 0x20;

		if (ACR & 2)
		{
			ISR |= INT_INPUT_PORT_CHANGE;
			update_interrupts();
		}
	}

	IP_last_state = newIP;
}

WRITE_LINE_MEMBER( mc68681_device::ip2_w )
{
	UINT8 newIP = (IP_last_state & ~0x04) | ((state == ASSERT_LINE) ? 4 : 0);

	if (newIP != IP_last_state)
	{
		IPCR &= ~0x0f;
		IPCR |= (newIP & 0x0f);
		IPCR |= 0x40;

		if (ACR & 4)
		{
			ISR |= INT_INPUT_PORT_CHANGE;
			update_interrupts();
		}
	}

	IP_last_state = newIP;
}

WRITE_LINE_MEMBER( mc68681_device::ip3_w )
{
	UINT8 newIP = (IP_last_state & ~0x08) | ((state == ASSERT_LINE) ? 8 : 0);

	if (newIP != IP_last_state)
	{
		IPCR &= ~0x0f;
		IPCR |= (newIP & 0x0f);
		IPCR |= 0x80;

		if (ACR & 8)
		{
			ISR |= INT_INPUT_PORT_CHANGE;
			update_interrupts();
		}
	}

	IP_last_state = newIP;
}

WRITE_LINE_MEMBER( mc68681_device::ip4_w )
{
	UINT8 newIP = (IP_last_state & ~0x10) | ((state == ASSERT_LINE) ? 0x10 : 0);
// TODO: special mode for ip4 (Ch. A Rx clock)
	IP_last_state = newIP;
}

WRITE_LINE_MEMBER( mc68681_device::ip5_w )
{
	UINT8 newIP = (IP_last_state & ~0x20) | ((state == ASSERT_LINE) ? 0x20 : 0);
// TODO: special mode for ip5 (Ch. B Tx clock)
	IP_last_state = newIP;
}

mc68681_channel *mc68681_device::get_channel(int chan)
{
	if (chan == 0)
	{
		return m_chanA;
	}

	return m_chanB;
}

int mc68681_device::calc_baud(int ch, UINT8 data)
{
	int baud_rate = 0;

	if ( BIT(ACR, 7) == 0 )
	{
		baud_rate = baud_rate_ACR_0[data & 0x0f];

		if (ch == 0)
		{
			if ((data & 0xf) == 0xe)
			{
				baud_rate = ip3clk/16;
			}
			else if ((data & 0xf) == 0xf)
			{
				baud_rate = ip3clk;
			}
		}
		else if (ch == 1)
		{
			if ((data & 0xf) == 0xe)
			{
				baud_rate = ip5clk/16;
			}
			else if ((data & 0xf) == 0xf)
			{
				baud_rate = ip5clk;
			}
		}
	}
	else
	{
		baud_rate = baud_rate_ACR_1[data & 0x0f];
	}

	if ((baud_rate == 0) && ((data & 0xf) != 0xd))
	{
		LOG(( "Unsupported transmitter clock: channel %d, clock select = %02x\n", ch, data ));
	}

	return baud_rate;
}

void mc68681_device::clear_ISR_bits(int mask)
{
	ISR &= ~mask;
}

void mc68681_device::set_ISR_bits(int mask)
{
	ISR |= mask;
}

// DUART channel class stuff

mc68681_channel::mc68681_channel(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MC68681_CHANNEL, "MC68681 DUART CHANNEL", tag, owner, clock, "mc68681_channel", __FILE__),
	device_serial_interface(mconfig, *this),
	MR1(0),
	MR2(0),
	SR(0),
	rx_enabled(0),
	rx_fifo_num(0),
	tx_enabled(0)
{
}

void mc68681_channel::device_start()
{
	m_uart = downcast<mc68681_device *>(owner());
	m_ch = m_uart->get_ch(this);    // get our channel number

	save_item(NAME(CR));
	save_item(NAME(CSR));
	save_item(NAME(MR1));
	save_item(NAME(MR2));
	save_item(NAME(MR_ptr));
	save_item(NAME(SR));
	save_item(NAME(rx_baud_rate));
	save_item(NAME(tx_baud_rate));
	save_item(NAME(rx_enabled));
	save_item(NAME(rx_fifo));
	save_item(NAME(rx_fifo_read_ptr));
	save_item(NAME(rx_fifo_write_ptr));
	save_item(NAME(rx_fifo_num));
	save_item(NAME(tx_enabled));
	save_item(NAME(tx_data));
	save_item(NAME(tx_ready));
}

void mc68681_channel::device_reset()
{
	write_CR(0x10); // reset MR
	write_CR(0x20); // reset Rx
	write_CR(0x30); // reset Tx
	write_CR(0x40); // reset errors

	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);

	tx_baud_rate = rx_baud_rate = 0;
	CSR = 0;
}

void mc68681_channel::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	device_serial_interface::device_timer(timer, id, param, ptr);
}

// serial device virtual overrides
void mc68681_channel::rcv_complete()
{
	receive_register_extract();

//  printf("%s ch %d rcv complete\n", tag(), m_ch);

	if ( rx_enabled )
	{
		if ( rx_fifo_num >= MC68681_RX_FIFO_SIZE )
		{
			logerror("68681: FIFO overflow\n");
			SR |= STATUS_OVERRUN_ERROR;
			return;
		}
		rx_fifo[rx_fifo_write_ptr++] = get_received_char();
		if ( rx_fifo_write_ptr == MC68681_RX_FIFO_SIZE )
		{
			rx_fifo_write_ptr = 0;
		}
		rx_fifo_num++;
		update_interrupts();
	}
}

void mc68681_channel::tra_complete()
{
//  printf("%s ch %d Tx complete\n", tag(), m_ch);
	tx_ready = 1;
	SR |= STATUS_TRANSMITTER_READY;

	if (m_ch == 0)
		m_uart->clear_ISR_bits(INT_TXRDYA);
	else
		m_uart->clear_ISR_bits(INT_TXRDYB);

	// if local loopback is on, write the transmitted data as if a byte had been received
	if ((MR2 & 0xC0) == 0x80)
	{
		if (rx_fifo_num >= MC68681_RX_FIFO_SIZE)
		{
			LOG(( "68681: FIFO overflow\n" ));
			SR |= STATUS_OVERRUN_ERROR;
		}
		else
		{
			rx_fifo[rx_fifo_write_ptr++]= tx_data;
			if (rx_fifo_write_ptr == MC68681_RX_FIFO_SIZE)
			{
				rx_fifo_write_ptr = 0;
			}
			rx_fifo_num++;
		}
	}

	update_interrupts();
}

void mc68681_channel::tra_callback()
{
	// don't actually send in loopback mode
	if ((MR2&0xC0) != 0x80)
	{
		int bit = transmit_register_get_data_bit();
//      printf("%s ch %d transmit %d\n", tag(), m_ch, bit);
		if (m_ch == 0)
		{
			m_uart->write_a_tx(bit);
		}
		else
		{
			m_uart->write_b_tx(bit);
		}
	}
	else    // must call this to advance the transmitter
	{
		transmit_register_get_data_bit();
	}
}

void mc68681_channel::update_interrupts()
{
	if (rx_enabled)
	{
		if (rx_fifo_num > 0)
		{
			SR |= STATUS_RECEIVER_READY;
		}
		else
		{
			SR &= ~STATUS_RECEIVER_READY;
		}
		if ( rx_fifo_num == MC68681_RX_FIFO_SIZE )
		{
			SR |= STATUS_FIFO_FULL;
		}
		else
		{
			SR &= ~STATUS_FIFO_FULL;
		}
	}

	// Handle the TxEMT and TxRDY bits based on mode
	switch(MR2&0xC0) // what mode are we in?
		{
		case 0x00: // normal mode
			if ( tx_enabled )
			{
				SR |= STATUS_TRANSMITTER_EMPTY;
			}
			else
			{
				SR &= ~STATUS_TRANSMITTER_EMPTY;
			}
		break;
		case 0x40: // automatic echo mode
			SR &= ~STATUS_TRANSMITTER_EMPTY;
			SR &= ~STATUS_TRANSMITTER_READY;
		break;
		case 0x80: // local loopback mode
			if ( tx_enabled )
			{
				SR |= STATUS_TRANSMITTER_EMPTY;
			}
			else
			{
				SR &= ~STATUS_TRANSMITTER_EMPTY;
			}
		break;
		case 0xC0: // remote loopback mode
			// write me, what the txrdy/txemt regs do for remote loopback mode is undocumented afaik, for now just clear both
			SR &= ~STATUS_TRANSMITTER_EMPTY;
			SR &= ~STATUS_TRANSMITTER_READY;
		break;
		}
	// now handle the ISR bits
	if ( SR & STATUS_TRANSMITTER_READY )
	{
		if (m_ch == 0)
			m_uart->set_ISR_bits(INT_TXRDYA);
		else
			m_uart->set_ISR_bits(INT_TXRDYB);
	}
	else
	{
		if (m_ch == 0)
			m_uart->clear_ISR_bits(INT_TXRDYA);
		else
			m_uart->clear_ISR_bits(INT_TXRDYB);
	}
	//logerror("DEBUG: 68681 int check: before receiver test, SR%c is %02X, ISR is %02X\n", (ch+0x41), duart68681->channel[ch].SR, duart68681->ISR);
	if ( MR1 & MODE_RX_INT_SELECT_BIT )
	{
		if ( SR & STATUS_FIFO_FULL )
		{
			m_uart->set_ISR_bits((m_ch == 0) ? INT_RXRDY_FFULLA : INT_RXRDY_FFULLB);
		}
		else
		{
			m_uart->clear_ISR_bits((m_ch == 0) ? INT_RXRDY_FFULLA : INT_RXRDY_FFULLB);
		}
	}
	else
	{
		if ( SR & STATUS_RECEIVER_READY )
		{
			m_uart->set_ISR_bits((m_ch == 0) ? INT_RXRDY_FFULLA : INT_RXRDY_FFULLB);
		}
		else
		{
			m_uart->clear_ISR_bits((m_ch == 0) ? INT_RXRDY_FFULLA : INT_RXRDY_FFULLB);
		}
	}

	m_uart->update_interrupts();

	//logerror("DEBUG: 68681 int check: after receiver test, SR%c is %02X, ISR is %02X\n", (ch+0x41), duart68681->channel[ch].SR, duart68681->ISR);
}

UINT8 mc68681_channel::read_rx_fifo()
{
	UINT8 rv = 0;

//  printf("read_rx_fifo: rx_fifo_num %d\n", rx_fifo_num);

	if ( rx_fifo_num == 0 )
	{
		LOG(( "68681 channel: rx fifo underflow\n" ));
		update_interrupts();
		return 0;
	}

	rv = rx_fifo[rx_fifo_read_ptr++];
	if ( rx_fifo_read_ptr == MC68681_RX_FIFO_SIZE )
	{
		rx_fifo_read_ptr = 0;
	}

	rx_fifo_num--;
	update_interrupts();

//  printf("Rx read %02x\n", rv);

	return rv;
}

UINT8 mc68681_channel::read_chan_reg(int reg)
{
	UINT8 rv = 0xff;

	switch (reg)
	{
		case 0: // MR1/MR2
			if ( MR_ptr == 0 )
			{
				rv = MR1;
				MR_ptr = 1;
			}
			else
			{
				rv = MR2;
			}
			break;

		case 1: // SRA
			rv = SR;
			break;

		case 2: // CSRA: reading this is prohibited
			break;

		case 3: // Rx holding register A
			rv = read_rx_fifo();
			break;
	}

	return rv;
}

void mc68681_channel::write_chan_reg(int reg, UINT8 data)
{
	switch (reg)
	{
	case 0x00: /* MRA */
		write_MR(data);
		break;

	case 0x01: /* CSR */
		CSR = data;
		tx_baud_rate = m_uart->calc_baud(m_ch, data & 0xf);
		rx_baud_rate = m_uart->calc_baud(m_ch, (data>>4) & 0xf);
//      printf("%s ch %d CSR %02x Tx baud %d Rx baud %d\n", tag(), m_ch, data, tx_baud_rate, rx_baud_rate);
		set_rcv_rate(rx_baud_rate);
		set_tra_rate(tx_baud_rate);
		break;

	case 0x02: /* CR */
		write_CR(data);
		break;

	case 0x03: /* THR */
		write_TX(data);
		break;
	}
}

void mc68681_channel::write_MR(UINT8 data)
{
	if ( MR_ptr == 0 )
	{
		MR1 = data;
		MR_ptr = 1;
	}
	else
	{
		MR2 = data;
	}
	recalc_framing();
	update_interrupts();
}

void mc68681_channel::recalc_framing()
{
	parity_t parity = PARITY_NONE;
	switch ((MR1>>3) & 3)
	{
		case 0: // with parity
			if (MR1 & 4)
			{
				parity = PARITY_ODD;
			}
			else
			{
				parity = PARITY_EVEN;
			}
			break;

		case 1: // force parity
			if (MR1 & 4)
			{
				parity = PARITY_MARK;
			}
			else
			{
				parity = PARITY_SPACE;
			}
			break;

		case 2: // no parity
			parity = PARITY_NONE;
			break;

		case 3: // multidrop mode
			// fatalerror("68681: multidrop parity not supported\n");
			// Apollo DEX CPU will test this; omit to abort the emulation
			logerror("68681: multidrop parity not supported\n");
			break;
	}

	stop_bits_t stopbits = STOP_BITS_0;
	switch ((MR2 >> 2) & 3)
	{
		case 0:
		case 1:
			stopbits = STOP_BITS_1;
			break;

		case 2: // "1.5 async, 2 sync"
			stopbits = STOP_BITS_1_5;
			break;

		case 3:
			stopbits = STOP_BITS_2;
			break;
	}

//  printf("%s ch %d MR1 %02x MR2 %02x => %d bits / char, %d stop bits, parity %d\n", tag(), m_ch, MR1, MR2, (MR1 & 3)+5, stopbits, parity);

	set_data_frame(1, (MR1 & 3)+5, parity, stopbits);
}

void mc68681_channel::write_CR(UINT8 data)
{
	CR = data;

	switch( (data >> 4) & 0x07 )
	{
		case 0: /* No command */
			break;
		case 1: /* Reset MR pointer. Causes the channel MR pointer to point to MR1 */
			MR_ptr = 0;
			break;
		case 2: /* Reset channel receiver (disable receiver and flush fifo) */
			rx_enabled = 0;
			SR &= ~STATUS_RECEIVER_READY;
			SR &= ~STATUS_OVERRUN_ERROR; // is this correct?
			rx_fifo_read_ptr = 0;
			rx_fifo_write_ptr = 0;
			rx_fifo_num = 0;
			receive_register_reset();
			break;
		case 3: /* Reset channel transmitter */
			tx_enabled = 0;
			SR &= ~STATUS_TRANSMITTER_READY;
			if (m_ch == 0)
				m_uart->clear_ISR_bits(INT_TXRDYA);
			else
				m_uart->clear_ISR_bits(INT_TXRDYB);
			transmit_register_reset();
			break;
		case 4: /* Reset Error Status */
			SR &= ~(STATUS_RECEIVED_BREAK | STATUS_FRAMING_ERROR | STATUS_PARITY_ERROR | STATUS_OVERRUN_ERROR);
			break;
		case 5: /* Reset Channel break change interrupt */
			if ( m_ch == 0 )
			{
				m_uart->clear_ISR_bits(INT_DELTA_BREAK_A);
			}
			else
			{
				m_uart->clear_ISR_bits(INT_DELTA_BREAK_B);
			}
			break;
		/* TODO: case 6 and case 7 are start break and stop break respectively, which start or stop holding the TxDA or TxDB line low (space) after whatever data is in the buffer finishes transmitting (following the stop bit?), or after two bit-times if no data is being transmitted  */
		default:
			LOG(( "68681: Unhandled command (%x) in CR%d\n", (data >> 4) & 0x07, m_ch ));
			break;
	}

	if (BIT(data, 0)) {
		rx_enabled = 1;
	}
	if (BIT(data, 1)) {
		rx_enabled = 0;
		SR &= ~STATUS_RECEIVER_READY;
	}

	if (BIT(data, 2)) {
		tx_enabled = 1;
		tx_ready = 1;
		SR |= STATUS_TRANSMITTER_READY;
		if (m_ch == 0)
			m_uart->set_ISR_bits(INT_TXRDYA);
		else
			m_uart->set_ISR_bits(INT_TXRDYB);
	}
	if (BIT(data, 3)) {
		tx_enabled = 0;
		tx_ready = 0;
		SR &= ~STATUS_TRANSMITTER_READY;
		if (m_ch == 0)
			m_uart->clear_ISR_bits(INT_TXRDYA);
		else
			m_uart->clear_ISR_bits(INT_TXRDYB);
	}

	update_interrupts();
}

void mc68681_channel::write_TX(UINT8 data)
{
	tx_data = data;

/*  if (!tx_ready)
    {
         printf("Write %02x to TX when TX not ready!\n", data);
    }*/

//  printf("%s ch %d Tx %02x\n", tag(), m_ch, data);

	tx_ready = 0;
	SR &= ~STATUS_TRANSMITTER_READY;

	if (m_ch == 0)
		m_uart->clear_ISR_bits(INT_TXRDYA);
	else
		m_uart->clear_ISR_bits(INT_TXRDYB);

	// send tx_data
	transmit_register_setup(tx_data);

	update_interrupts();
}

void mc68681_channel::ACR_updated()
{
	write_chan_reg(1, CSR);
}

UINT8 mc68681_channel::get_chan_CSR()
{
	return CSR;
}
