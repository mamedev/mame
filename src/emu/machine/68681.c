/*
    68681 DUART

    Written by Mariusz Wojcieszek
    Updated by Jonathan Gevaryahu AKA Lord Nightmare
    Improved interrupt handling by R. Belmont
*/

#include "emu.h"
#include "68681.h"

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

#define RX_FIFO_SIZE				3

typedef struct
{
	/* Registers */
	UINT8 CR;  /* Command register */
	UINT8 CSR; /* Clock select register */
	UINT8 MR1; /* Mode register 1 */
	UINT8 MR2; /* Mode register 2 */
	UINT8 MR_ptr; /* Mode register pointer */
	UINT8 SR;  /* Status register */

	/* State */
	int   baud_rate;

	/* Receiver */
	UINT8 rx_enabled;
	UINT8 rx_fifo[RX_FIFO_SIZE];
	int   rx_fifo_read_ptr;
	int   rx_fifo_write_ptr;
	int   rx_fifo_num;

	/* Transmitter */
	UINT8 tx_enabled;
	UINT8 tx_data;
	UINT8 tx_ready;
	emu_timer *tx_timer;

} DUART68681_CHANNEL;

typedef struct
{
	/* device */
	device_t *device;

	/* config */
	const duart68681_config *duart_config;

	/* registers */
	UINT8 ACR;  /* Auxiliary Control Register */
	UINT8 IMR;  /* Interrupt Mask Register */
	UINT8 ISR;  /* Interrupt Status Register */
	UINT8 IVR;  /* Interrupt Vector Register */
	UINT8 OPCR; /* Output Port Conf. Register */
	UINT8 OPR;  /* Output Port Register */
	PAIR  CTR;  /* Counter/Timer Preset Value */

	/* state */
	UINT8 IP_last_state; /* last state of IP bits */

	/* timer */
	emu_timer *duart_timer;

	/* UART channels */
	DUART68681_CHANNEL channel[2];

} duart68681_state;

INLINE duart68681_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == DUART68681);

	return (duart68681_state *)downcast<legacy_device_base *>(device)->token();
}

static void duart68681_update_interrupts(duart68681_state *duart68681)
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
			if ( duart68681->channel[ch].rx_fifo_num > 0 )
			{
				duart68681->channel[ch].SR |= STATUS_RECEIVER_READY;
			}
			else
			{
				duart68681->channel[ch].SR &= ~STATUS_RECEIVER_READY;
			}
			if ( duart68681->channel[ch].rx_fifo_num == RX_FIFO_SIZE )
			{
				duart68681->channel[ch].SR |= STATUS_FIFO_FULL;
			}
			else
			{
				duart68681->channel[ch].SR &= ~STATUS_FIFO_FULL;
			}
		//}
		//else
		//{
		//duart68681->channel[ch].SR &= ~STATUS_RECEIVER_READY;
		//duart68681->channel[ch].SR &= ~STATUS_FIFO_FULL;
		//}
		// Handle the TxEMT and TxRDY bits based on mode
		switch( duart68681->channel[ch].MR2&0xC0) // what mode are we in?
			{
			case 0x00: // normal mode
				if ( duart68681->channel[ch].tx_enabled )
				{
					duart68681->channel[ch].SR |= STATUS_TRANSMITTER_EMPTY;
				}
				else
				{
					duart68681->channel[ch].SR &= ~STATUS_TRANSMITTER_EMPTY;
				}
			break;
			case 0x40: // automatic echo mode
				duart68681->channel[ch].SR &= ~STATUS_TRANSMITTER_EMPTY;
				duart68681->channel[ch].SR &= ~STATUS_TRANSMITTER_READY;
			break;
			case 0x80: // local loopback mode
				if ( duart68681->channel[ch].tx_enabled )
				{
					duart68681->channel[ch].SR |= STATUS_TRANSMITTER_EMPTY;
				}
				else
				{
					duart68681->channel[ch].SR &= ~STATUS_TRANSMITTER_EMPTY;
				}
			break;
			case 0xC0: // remote loopback mode
				// write me, what the txrdy/txemt regs do for remote loopback mode is undocumented afaik, for now just clear both
				duart68681->channel[ch].SR &= ~STATUS_TRANSMITTER_EMPTY;
				duart68681->channel[ch].SR &= ~STATUS_TRANSMITTER_READY;
			break;
			}
		// now handle the ISR bits
		if ( duart68681->channel[ch].SR & STATUS_TRANSMITTER_READY )
		{
			if (ch == 0)
				duart68681->ISR |= INT_TXRDYA;
			else
				duart68681->ISR |= INT_TXRDYB;
		}
		else
		{
			if (ch == 0)
				duart68681->ISR &= ~INT_TXRDYA;
			else
				duart68681->ISR &= ~INT_TXRDYB;
		}
		//logerror("DEBUG: 68681 int check: before receiver test, SR%c is %02X, ISR is %02X\n", (ch+0x41), duart68681->channel[ch].SR, duart68681->ISR);
		if ( duart68681->channel[ch].MR1 & MODE_RX_INT_SELECT_BIT )
		{
			if ( duart68681->channel[ch].SR & STATUS_FIFO_FULL )
			{
				duart68681->ISR |= ((ch == 0) ? INT_RXRDY_FFULLA : INT_RXRDY_FFULLB);
			}
			else
			{
				duart68681->ISR &= ((ch == 0) ? ~INT_RXRDY_FFULLA : ~INT_RXRDY_FFULLB);
			}
		}
		else
		{
			if ( duart68681->channel[ch].SR & STATUS_RECEIVER_READY )
			{
				duart68681->ISR |= ((ch == 0) ? INT_RXRDY_FFULLA : INT_RXRDY_FFULLB);
			}
			else
			{
				duart68681->ISR &= ((ch == 0) ? ~INT_RXRDY_FFULLA : ~INT_RXRDY_FFULLB);
			}
		}
		//logerror("DEBUG: 68681 int check: after receiver test, SR%c is %02X, ISR is %02X\n", (ch+0x41), duart68681->channel[ch].SR, duart68681->ISR);
	}
	if ( (duart68681->ISR & duart68681->IMR) != 0 )
	{
		if ( duart68681->duart_config->irq_handler )
		{
			LOG(( "68681: Interrupt line active (IMR & ISR = %02X)\n", (duart68681->ISR & duart68681->IMR) ));
			duart68681->duart_config->irq_handler( duart68681->device, ASSERT_LINE, duart68681->IVR );
		}
	}
    else
    {
		if ( duart68681->duart_config->irq_handler )
		{
			LOG(( "68681: Interrupt line not active (IMR & ISR = %02X)\n", (duart68681->ISR & duart68681->IMR) ));
			duart68681->duart_config->irq_handler( duart68681->device, CLEAR_LINE, duart68681->IVR );
		}
    }
};

static TIMER_CALLBACK( duart_timer_callback )
{
	device_t *device = (device_t *)ptr;
	duart68681_state	*duart68681 = get_safe_token(device);

	duart68681->ISR |= INT_COUNTER_READY;
	duart68681_update_interrupts(duart68681);

//  if ((duart68681->OPCR & 0x0c)== 0x04) {
//      duart68681->OPR ^= 0x08;
//      if (duart68681->duart_config->output_port_write)
//          duart68681->duart_config->output_port_write(duart68681->device, duart68681->OPR ^ 0xff);
//
//  }
};

static void duart68681_write_MR(duart68681_state *duart68681, int ch, UINT8 data)
{
	if ( duart68681->channel[ch].MR_ptr == 0 )
	{
		duart68681->channel[ch].MR1 = data;
		duart68681->channel[ch].MR_ptr = 1;
	}
	else
	{
		duart68681->channel[ch].MR2 = data;
	}
	duart68681_update_interrupts(duart68681);
};

static void duart68681_write_CSR(duart68681_state *duart68681, int ch, UINT8 data, UINT8 ACR)
{
	static const int baud_rate_ACR_0[] = { 50, 110, 134, 200, 300, 600, 1200, 1050, 2400, 4800, 7200, 9600, 38400, 0, 0, 0 };
	static const int baud_rate_ACR_1[] = { 75, 110, 134, 150, 300, 600, 1200, 2000, 2400, 4800, 1800, 9600, 19200, 0, 0, 0 };

	duart68681->channel[ch].CSR = data;

	if ( BIT(ACR,7) == 0 )
	{
		duart68681->channel[ch].baud_rate = baud_rate_ACR_0[data & 0x0f];

		if (ch == 0)
		{
			if ((data & 0xf) == 0xe)
			{
				duart68681->channel[ch].baud_rate = duart68681->duart_config->ip3clk/16;
			}
			else if ((data & 0xf) == 0xf)
			{
				duart68681->channel[ch].baud_rate = duart68681->duart_config->ip3clk;
			}
		}
		else if (ch == 1)
		{
			if ((data & 0xf) == 0xe)
			{
				duart68681->channel[ch].baud_rate = duart68681->duart_config->ip5clk/16;
			}
			else if ((data & 0xf) == 0xf)
			{
				duart68681->channel[ch].baud_rate = duart68681->duart_config->ip5clk;
			}
		}
	}
	else
	{
		duart68681->channel[ch].baud_rate = baud_rate_ACR_1[data & 0x0f];
	}
	if ( duart68681->channel[ch].baud_rate == 0 )
	{
		LOG(( "Unsupported transmitter clock: channel %d, clock select = %02x\n", ch, data ));
	}
};

static void duart68681_write_CR(duart68681_state *duart68681, int ch, UINT8 data)
{
	duart68681->channel[ch].CR = data;

	switch( (data >> 4) & 0x07 )
	{
		case 0: /* No command */
			break;
		case 1: /* Reset MR pointer. Causes the Channel A MR pointer to point to MR1 */
			duart68681->channel[ch].MR_ptr = 0;
			break;
		case 2: /* Reset channel A receiver (disable receiver and flush fifo) */
			duart68681->channel[ch].rx_enabled = 0;
			duart68681->channel[ch].SR &= ~STATUS_RECEIVER_READY;
			duart68681->channel[ch].SR &= ~STATUS_OVERRUN_ERROR; // is this correct?
			duart68681->channel[ch].rx_fifo_read_ptr = 0;
			duart68681->channel[ch].rx_fifo_write_ptr = 0;
			duart68681->channel[ch].rx_fifo_num = 0;
			break;
		case 3: /* Reset channel A transmitter */
			duart68681->channel[ch].tx_enabled = 0;
			duart68681->channel[ch].SR &= ~STATUS_TRANSMITTER_READY;
			if (ch == 0)
				duart68681->ISR &= ~INT_TXRDYA;
			else
				duart68681->ISR &= ~INT_TXRDYB;
			duart68681->channel[ch].tx_timer->adjust(attotime::never, ch);
            break;
		case 4: /* Reset Error Status */
			duart68681->channel[ch].SR &= ~(STATUS_RECEIVED_BREAK | STATUS_FRAMING_ERROR | STATUS_PARITY_ERROR | STATUS_OVERRUN_ERROR);
			break;
		case 5: /* Reset Channel break change interrupt */
			if ( ch == 0 )
			{
				duart68681->ISR &= ~INT_DELTA_BREAK_A;
			}
			else
			{
				duart68681->ISR &= ~INT_DELTA_BREAK_B;
			}
			break;
		/* TODO: case 6 and case 7 are start break and stop break respectively, which start or stop holding the TxDA or TxDB line low (space) after whatever data is in the buffer finishes transmitting (following the stop bit?), or after two bit-times if no data is being transmitted  */
		default:
			LOG(( "68681: Unhandled command (%x) in CR%d\n", (data >> 4) & 0x07, ch ));
			break;
	}

	if (BIT(data, 0)) {
		duart68681->channel[ch].rx_enabled = 1;
	}
	if (BIT(data, 1)) {
		duart68681->channel[ch].rx_enabled = 0;
		duart68681->channel[ch].SR &= ~STATUS_RECEIVER_READY;
	}

	if (BIT(data, 2)) {
		duart68681->channel[ch].tx_enabled = 1;
		duart68681->channel[ch].tx_ready = 1;
		duart68681->channel[ch].SR |= STATUS_TRANSMITTER_READY;
		if (ch == 0)
			duart68681->ISR |= INT_TXRDYA;
		else
			duart68681->ISR |= INT_TXRDYB;
	}
	if (BIT(data, 3)) {
		duart68681->channel[ch].tx_enabled = 0;
		duart68681->channel[ch].tx_ready = 0;
		duart68681->channel[ch].SR &= ~STATUS_TRANSMITTER_READY;
		if (ch == 0)
			duart68681->ISR &= ~INT_TXRDYA;
		else
			duart68681->ISR &= ~INT_TXRDYB;
	}

    duart68681_update_interrupts(duart68681);
};

static UINT8 duart68681_read_rx_fifo(duart68681_state *duart68681, int ch)
{
	UINT8 r;

	if ( duart68681->channel[ch].rx_fifo_num == 0 )
	{
		LOG(( "68681: rx fifo underflow\n" ));
		return 0x0;
	}

	r = duart68681->channel[ch].rx_fifo[duart68681->channel[ch].rx_fifo_read_ptr++];
	if ( duart68681->channel[ch].rx_fifo_read_ptr == RX_FIFO_SIZE )
	{
		duart68681->channel[ch].rx_fifo_read_ptr = 0;
	}

	duart68681->channel[ch].rx_fifo_num--;
	duart68681_update_interrupts(duart68681);

	return r;
};

static TIMER_CALLBACK( tx_timer_callback )
{
	device_t *device = (device_t *)ptr;
	duart68681_state	*duart68681 = get_safe_token(device);
	int ch = param & 1;

	// send the byte unless we're in loopback mode;
	// in loopback mode do NOT 'actually' send the byte: the TXn pin is held high when loopback mode is on.
	if ((duart68681->duart_config->tx_callback) && ((duart68681->channel[ch].MR2&0xC0) != 0x80))
		duart68681->duart_config->tx_callback(device, ch, duart68681->channel[ch].tx_data);

	// if local loopback is on, write the transmitted data as if a byte had been received
	if ((duart68681->channel[ch].MR2 & 0xC0) == 0x80)
	{
		if (duart68681->channel[ch].rx_fifo_num >= RX_FIFO_SIZE)
		{
			LOG(( "68681: FIFO overflow\n" ));
			duart68681->channel[ch].SR |= STATUS_OVERRUN_ERROR;
		}
		else
		{
			duart68681->channel[ch].rx_fifo[duart68681->channel[ch].rx_fifo_write_ptr++]
					= duart68681->channel[ch].tx_data;
			if (duart68681->channel[ch].rx_fifo_write_ptr == RX_FIFO_SIZE)
			{
				duart68681->channel[ch].rx_fifo_write_ptr = 0;
			}
			duart68681->channel[ch].rx_fifo_num++;
		}
	}

	duart68681->channel[ch].tx_ready = 1;
	duart68681->channel[ch].SR |= STATUS_TRANSMITTER_READY;

	if (ch == 0)
		duart68681->ISR |= INT_TXRDYA;
	else
		duart68681->ISR |= INT_TXRDYB;

	duart68681_update_interrupts(duart68681);
	duart68681->channel[ch].tx_timer->adjust(attotime::never, ch);
};

static void duart68681_write_TX(duart68681_state* duart68681, int ch, UINT8 data)
{
	attotime period;

	duart68681->channel[ch].tx_data = data;

	duart68681->channel[ch].tx_ready = 0;
	duart68681->channel[ch].SR &= ~STATUS_TRANSMITTER_READY;

	if (ch == 0)
		duart68681->ISR &= ~INT_TXRDYA;
	else
		duart68681->ISR &= ~INT_TXRDYB;

	duart68681_update_interrupts(duart68681);

	period = attotime::from_hz(duart68681->channel[ch].baud_rate / 10 );
	duart68681->channel[ch].tx_timer->adjust(period, ch);

};

READ8_DEVICE_HANDLER(duart68681_r)
{
	duart68681_state* duart68681 = get_safe_token(device);
	UINT8 r = 0xff;

	offset &= 0xf;

	LOG(( "Reading 68681 (%s) reg %x (%s) ", device->tag(), offset, duart68681_reg_read_names[offset] ));

	switch (offset)
	{
		case 0x00: /* MR1A/MR2A */
			if ( duart68681->channel[0].MR_ptr == 0 )
			{
				r = duart68681->channel[0].MR1;
				duart68681->channel[0].MR_ptr = 1;
			}
			else
			{
				r = duart68681->channel[0].MR2;
			}
			break;
		case 0x01: /* SRA */
			r = duart68681->channel[0].SR;
			break;
		case 0x03: /* Rx Holding Register A */
			r = duart68681_read_rx_fifo(duart68681, 0);
			break;
		case 0x04: /* IPCR */
			{
				UINT8 IP;
				if ( duart68681->duart_config->input_port_read != NULL )
					IP = duart68681->duart_config->input_port_read(duart68681->device);
				else
					IP = 0x0;

				r = (((duart68681->IP_last_state ^ IP) & 0x0f) << 4) | (IP & 0x0f);
				duart68681->IP_last_state = IP;
				duart68681->ISR &= ~INT_INPUT_PORT_CHANGE;
				duart68681_update_interrupts(duart68681);
			}
			break;
		case 0x05: /* ISR */
			r = duart68681->ISR;
			break;
		case 0x08: /* MR1B/MR2B */
			if ( duart68681->channel[1].MR_ptr == 0 )
			{
				r = duart68681->channel[1].MR1;
				duart68681->channel[1].MR_ptr = 1;
			}
			else
			{
				r = duart68681->channel[1].MR2;
			}
			break;
		case 0x09: /* SRB */
			r = duart68681->channel[1].SR;
			break;
		case 0x0b: /* RHRB */
			r = duart68681_read_rx_fifo(duart68681, 1);
			break;
		case 0x0d: /* IP */
			if ( duart68681->duart_config->input_port_read != NULL )
				r = duart68681->duart_config->input_port_read(duart68681->device);
			else
				{
					r = 0xff;
#if 0
					if (device->machine().input().code_pressed(KEYCODE_1)) r ^= 0x0001;
					if (device->machine().input().code_pressed(KEYCODE_2)) r ^= 0x0002;
					if (device->machine().input().code_pressed(KEYCODE_3)) r ^= 0x0004;
					if (device->machine().input().code_pressed(KEYCODE_4)) r ^= 0x0008;
					if (device->machine().input().code_pressed(KEYCODE_5)) r ^= 0x0010;
					if (device->machine().input().code_pressed(KEYCODE_6)) r ^= 0x0020;
					if (device->machine().input().code_pressed(KEYCODE_7)) r ^= 0x0040;
					if (device->machine().input().code_pressed(KEYCODE_8)) r ^= 0x0080;
#endif
				}
			break;
		case 0x0e: /* Start counter command */
			switch( (duart68681->ACR >> 4) & 0x07 )
			{
				/* TODO: implement modes 0,1,2,4,5 */
				case 0x03: /* Counter, CLK/16 */
					{
						attotime rate = attotime::from_hz(2*device->clock()/(2*16*16*duart68681->CTR.w.l));
						duart68681->duart_timer->adjust(rate, 0, rate);
					}
					break;
			}
			break;
		case 0x0f: /* Stop counter command */
			duart68681->ISR &= ~INT_COUNTER_READY;
			if (((duart68681->ACR >>4)& 0x07) < 4) // if in counter mode...
			duart68681->duart_timer->adjust(attotime::never); // shut down timer
			duart68681_update_interrupts(duart68681);
			break;
		default:
			LOG(( "Reading unhandled 68681 reg %x\n", offset ));
			break;
	}
	LOG(("returned %02x\n", r));

	return r;
}

WRITE8_DEVICE_HANDLER(duart68681_w)
{
	duart68681_state* duart68681 = get_safe_token(device);

	offset &= 0x0f;
	LOG(( "Writing 68681 (%s) reg %x (%s) with %04x\n", device->tag(), offset, duart68681_reg_write_names[offset], data ));

	switch(offset)
	{
		case 0x00: /* MRA */
			duart68681_write_MR(duart68681, 0, data);
			break;
		case 0x01: /* CSRA */
			duart68681_write_CSR(duart68681, 0, data, duart68681->ACR);
			break;
		case 0x02: /* CRA */
			duart68681_write_CR(duart68681, 0, data);
			break;
		case 0x03: /* THRA */
			duart68681_write_TX(duart68681, 0, data);
			break;
		case 0x04: /* ACR */
			duart68681->ACR = data;
			//       bits 6-4: Counter/Timer Mode And Clock Source Select
			//       bits 3-0: IP3-0 Change-Of-State Interrupt Enable
			switch ((data >> 4) & 0x07)
			{
				case 0: case 1: case 2: case 4: case 5: // TODO: handle these cases!
				logerror( "68681 (%s): Unhandled timer/counter mode %d\n", device->tag(), (data >> 4) & 0x07);
				break;
            case 3:
				break;
            case 0x06: /* Timer, CLK/1 */       // Timer modes start without reading address 0xe, as per the Freescale 68681 manual
                {
                    attotime rate;
                    if (duart68681->CTR.w.l > 0)
                    {
                        rate = attotime::from_hz(2*device->clock()/(2*16*duart68681->CTR.w.l));
                    }
                    else
                    {
                        rate = attotime::from_hz(2*device->clock()/(2*16*0x10000));
                    }
                    duart68681->duart_timer->adjust(rate, 0, rate);
                }
                break;
            case 0x07: /* Timer, CLK/16 */
                {
                    //double hz;
                    //attotime rate = attotime::from_hz(duart68681->clock) * (16*duart68681->CTR.w.l);
                    attotime rate;
                    if (duart68681->CTR.w.l > 0)
                    {
                        rate = attotime::from_hz(2*device->clock()/(2*16*16*duart68681->CTR.w.l));

                        // workaround for maygay1b locking up MAME
                        if ((2*device->clock()/(2*16*16*duart68681->CTR.w.l)) == 0)
                        {
                            rate = attotime::from_hz(1);
                        }
                    }
                    else
                    {
                        if (2*device->clock()/(2*16*16*0x10000) == 0)
                        {
                            rate = attotime::from_hz(1);
                        }
                        else
                        {
                            rate = attotime::from_hz(2*device->clock()/(2*16*16*0x10000));
                        }
                    }
                        
                    //hz = ATTOSECONDS_TO_HZ(rate.attoseconds);

                    duart68681->duart_timer->adjust(rate, 0, rate);
                }
                break;
			}
			duart68681_write_CSR(duart68681, 0, duart68681->channel[0].CSR, data);
			duart68681_write_CSR(duart68681, 1, duart68681->channel[1].CSR, data);
			duart68681_update_interrupts(duart68681); // need to add ACR checking for IP delta ints
			break;
		case 0x05: /* IMR */
			duart68681->IMR = data;
			duart68681_update_interrupts(duart68681);
			break;
		case 0x06: /* CTUR */
			duart68681->CTR.b.h = data;
			break;
		case 0x07: /* CTLR */
			duart68681->CTR.b.l = data;
			break;
		case 0x08: /* MRB */
			duart68681_write_MR(duart68681, 1, data);
			break;
		case 0x09: /* CSRB */
			duart68681_write_CSR(duart68681, 1, data, duart68681->ACR);
			break;
		case 0x0a: /* CRB */
			duart68681_write_CR(duart68681, 1, data);
			break;
		case 0x0b: /* THRB */
			duart68681_write_TX(duart68681, 1, data);
			break;
		case 0x0c: /* IVR */
			duart68681->IVR = data;
			break;
		case 0x0d: /* OPCR */
			if (data != 0x00)
				logerror( "68681 (%s): Unhandled OPCR value: %02x\n", device->tag(), data);
			duart68681->OPCR = data;
			break;
		case 0x0e: /* Set Output Port Bits */
			duart68681->OPR |= data;
			if (duart68681->duart_config->output_port_write)
				duart68681->duart_config->output_port_write(duart68681->device, duart68681->OPR ^ 0xff);
			break;
		case 0x0f: /* Reset Output Port Bits */
			duart68681->OPR &= ~data;
			if (duart68681->duart_config->output_port_write)
				duart68681->duart_config->output_port_write(duart68681->device, duart68681->OPR ^ 0xff);
			break;
	}
}

void duart68681_rx_data( device_t* device, int ch, UINT8 data )
{
	duart68681_state *duart68681 = get_safe_token(device);

	if ( duart68681->channel[ch].rx_enabled )
	{
		if ( duart68681->channel[ch].rx_fifo_num >= RX_FIFO_SIZE )
		{
			LOG(( "68681: FIFO overflow\n" ));
			duart68681->channel[ch].SR |= STATUS_OVERRUN_ERROR;
			return;
		}
		duart68681->channel[ch].rx_fifo[duart68681->channel[ch].rx_fifo_write_ptr++] = data;
		if ( duart68681->channel[ch].rx_fifo_write_ptr == RX_FIFO_SIZE )
		{
			duart68681->channel[ch].rx_fifo_write_ptr = 0;
		}
		duart68681->channel[ch].rx_fifo_num++;
		duart68681_update_interrupts(duart68681);
	}
};

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START(duart68681)
{
	duart68681_state *duart68681 = get_safe_token(device);

	/* validate arguments */
	assert(device != NULL);

	duart68681->duart_config = (const duart68681_config *)device->static_config();
	duart68681->device = device;

	duart68681->channel[0].tx_timer = device->machine().scheduler().timer_alloc(FUNC(tx_timer_callback), (void*)device);
	duart68681->channel[1].tx_timer = device->machine().scheduler().timer_alloc(FUNC(tx_timer_callback), (void*)device);
	duart68681->duart_timer = device->machine().scheduler().timer_alloc(FUNC(duart_timer_callback), (void*)device);

	device->save_item(NAME(duart68681->ACR));
	device->save_item(NAME(duart68681->IMR));
	device->save_item(NAME(duart68681->ISR));
	device->save_item(NAME(duart68681->IVR));
	device->save_item(NAME(duart68681->OPCR));
	device->save_item(NAME(duart68681->CTR));
	device->save_item(NAME(duart68681->IP_last_state));

	device->save_item(NAME(duart68681->channel[0].CR));
	device->save_item(NAME(duart68681->channel[0].CSR));
	device->save_item(NAME(duart68681->channel[0].MR1));
	device->save_item(NAME(duart68681->channel[0].MR2));
	device->save_item(NAME(duart68681->channel[0].MR_ptr));
	device->save_item(NAME(duart68681->channel[0].SR));
	device->save_item(NAME(duart68681->channel[0].baud_rate));
	device->save_item(NAME(duart68681->channel[0].rx_enabled));
	device->save_item(NAME(duart68681->channel[0].rx_fifo));
	device->save_item(NAME(duart68681->channel[0].rx_fifo_read_ptr));
	device->save_item(NAME(duart68681->channel[0].rx_fifo_write_ptr));
	device->save_item(NAME(duart68681->channel[0].rx_fifo_num));
	device->save_item(NAME(duart68681->channel[0].tx_enabled));
	device->save_item(NAME(duart68681->channel[0].tx_data));
	device->save_item(NAME(duart68681->channel[0].tx_ready));

	device->save_item(NAME(duart68681->channel[1].CR));
	device->save_item(NAME(duart68681->channel[1].CSR));
	device->save_item(NAME(duart68681->channel[1].MR1));
	device->save_item(NAME(duart68681->channel[1].MR2));
	device->save_item(NAME(duart68681->channel[1].MR_ptr));
	device->save_item(NAME(duart68681->channel[1].SR));
	device->save_item(NAME(duart68681->channel[1].baud_rate));
	device->save_item(NAME(duart68681->channel[1].rx_enabled));
	device->save_item(NAME(duart68681->channel[1].rx_fifo));
	device->save_item(NAME(duart68681->channel[1].rx_fifo_read_ptr));
	device->save_item(NAME(duart68681->channel[1].rx_fifo_write_ptr));
	device->save_item(NAME(duart68681->channel[1].rx_fifo_num));
	device->save_item(NAME(duart68681->channel[1].tx_enabled));
	device->save_item(NAME(duart68681->channel[1].tx_data));
	device->save_item(NAME(duart68681->channel[1].tx_ready));
}

/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET(duart68681)
{
	duart68681_state *duart68681 = get_safe_token(device);
	emu_timer *save0, *save1;

	duart68681->ACR = 0;  /* Interrupt Vector Register */
	duart68681->IVR = 0x0f;  /* Interrupt Vector Register */
	duart68681->IMR = 0;  /* Interrupt Mask Register */
	duart68681->ISR = 0;  /* Interrupt Status Register */
	duart68681->OPCR = 0; /* Output Port Conf. Register */
	duart68681->OPR = 0;  /* Output Port Register */
	duart68681->CTR.d = 0;  /* Counter/Timer Preset Value */
	duart68681->IP_last_state = 0;  /* last state of IP bits */
	// "reset clears internal registers (SRA, SRB, IMR, ISR, OPR, OPCR) puts OP0-7 in the high state, stops the counter/timer, and puts channels a/b in the inactive state"
	save0 = duart68681->channel[0].tx_timer;
	save1 = duart68681->channel[1].tx_timer;
	memset(duart68681->channel, 0, sizeof(duart68681->channel));
	duart68681->channel[0].tx_timer = save0;
	duart68681->channel[1].tx_timer = save1;

	if (duart68681->duart_config->output_port_write)
		duart68681->duart_config->output_port_write(duart68681->device, duart68681->OPR ^ 0xff);

	// reset timers
	duart68681->channel[0].tx_timer->adjust(attotime::never);
	duart68681->channel[1].tx_timer->adjust(attotime::never, 1);
}

/*-------------------------------------------------
    device get info callback
-------------------------------------------------*/

DEVICE_GET_INFO(duart68681)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(duart68681_state);	break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = sizeof(duart68681_config);	break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(duart68681); break;
		case DEVINFO_FCT_STOP:					/* nothing */ break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(duart68681);break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "DUART 68681");			break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "DUART");				break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

DEFINE_LEGACY_DEVICE(DUART68681, duart68681);
