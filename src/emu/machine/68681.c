/*
    68681 DUART

    Written by Mariusz Wojcieszek

    Emulation is preliminary, only features required by Touch Master games are implemented

    ToDo:
    - interrupts other than RXRDY
    - timer/counter
    - input port
    - input port change
    - output port
    - output port when used as control signals
    - MAME device interface
    - multiple instances
*/

#include "driver.h"

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

#define MODE_RX_INT_SELECT_BIT		6

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

} DUART_68681_CHANNEL;

typedef struct
{
	int   frequency;

	// registers
	UINT8 ACR;  /* Auxiliary Control Register */
	UINT8 IMR;  /* Interrupt Mask Register */
	UINT8 ISR;  /* Interrupt Status Register */
	UINT8 IVR;  /* Interrupt Vector Register */
	UINT8 OPCR; /* Output Port Conf. Register */

	// state
	UINT8 IP_last_state; /* last state of IP bits */

	// callbacks
	UINT8 (*input_port_read)(void);
	void  (*tx_callback)(int channel, UINT8 data);
	void  (*irq_handler)(running_machine *machine, int vector);

	DUART_68681_CHANNEL channel[2];
} DUART_68681;

static DUART_68681 duart_68681;

static void duart_68681_update_interrupts( running_machine *machine )
{
	if ( (duart_68681.ISR & duart_68681.IMR) != 0 )
	{
		if ( duart_68681.irq_handler )
		{
			duart_68681.irq_handler( machine, duart_68681.IVR );
		}
	}
};

static void duart_68681_write_MR( int ch, UINT8 data )
{
	if ( duart_68681.channel[ch].MR_ptr == 0 )
	{
		duart_68681.channel[ch].MR1 = data;
		// TODO:
		duart_68681.channel[ch].MR_ptr = 1;
	}
	else
	{
		duart_68681.channel[ch].MR2 = data;
		// TODO:
	}
};

static void duart_68681_write_CSR(int ch, UINT8 data, UINT8 ACR)
{
	const int baud_rate_ACR_0[] = { 50, 110, 134, 200, 300, 600, 1200, 1050, 2400, 4800, 7200, 9600, 38400, 0, 0, 0 };
	const int baud_rate_ACR_1[] = { 75, 110, 134, 150, 300, 600, 1200, 2000, 2400, 4800, 1800, 9600, 19200, 0, 0, 0 };

	duart_68681.channel[ch].CSR = data;

	if ( BIT(ACR,7) == 0 )
	{
		duart_68681.channel[ch].baud_rate = baud_rate_ACR_0[data & 0x0f];
	}
	else
	{
		duart_68681.channel[ch].baud_rate = baud_rate_ACR_1[data & 0x0f];
	}
	if ( duart_68681.channel[ch].baud_rate == 0 )
	{
		logerror( "Unsupported transmitter clock: channel %d, clock select = %02x\n", ch, data );
	}
};

static void duart_68681_write_CR(int ch, UINT8 data)
{
	duart_68681.channel[ch].CR = data;
	if ( BIT(data,0) )
	{
		duart_68681.channel[ch].rx_enabled = 1;
	}
	if ( BIT(data,1) )
	{
		duart_68681.channel[ch].rx_enabled = 0;
		duart_68681.channel[ch].SR &= ~STATUS_RECEIVER_READY;
	}

	if ( BIT(data,2) )
	{
		duart_68681.channel[ch].tx_enabled = 1;
		duart_68681.channel[ch].tx_ready = 1;
		duart_68681.channel[ch].SR |= STATUS_TRANSMITTER_READY;
	}
	if ( BIT(data,3) )
	{
		duart_68681.channel[ch].tx_enabled = 0;
		duart_68681.channel[ch].tx_ready = 0;
		duart_68681.channel[ch].SR &= ~STATUS_TRANSMITTER_READY;
	}

	switch( (data >> 4) & 0x07 )
	{
		case 0: /* No command */
			break;
		case 1: /* Reset MR pointer. Causes the Channel A MR pointer to point to MR1 */
			duart_68681.channel[ch].MR_ptr = 0;
			break;
		case 2: /* Reset channel A receiver (disable receiver and flush fifo) */
			duart_68681.channel[ch].rx_enabled = 0;
			duart_68681.channel[ch].SR &= ~STATUS_RECEIVER_READY;
			duart_68681.channel[ch].rx_fifo_read_ptr = 0;
			duart_68681.channel[ch].rx_fifo_write_ptr = 0;
			duart_68681.channel[ch].rx_fifo_num = 0;
			break;
		case 3: /* Reset channel A transmitter */
			//TODO:
			break;
		case 4: /* Reset Error Status */
			// TODO:
			duart_68681.channel[ch].SR &= ~(STATUS_RECEIVED_BREAK | STATUS_FRAMING_ERROR | STATUS_PARITY_ERROR | STATUS_OVERRUN_ERROR);
			break;
		default:
			logerror( "68681: Unhandled command (%x) in CR%d\n", ch, (data >> 4) & 0x07 );
			break;
	}
};

static UINT8 duart_68681_read_rx_fifo( running_machine *machine, int ch )
{
	UINT8 r;

	if ( duart_68681.channel[ch].rx_fifo_num == 0 )
	{
		logerror( "68681: rx fifo underflow\n" );
		return 0;
	}

	r = duart_68681.channel[ch].rx_fifo[duart_68681.channel[ch].rx_fifo_read_ptr++];
	if ( duart_68681.channel[ch].rx_fifo_read_ptr == RX_FIFO_SIZE )
	{
		duart_68681.channel[ch].rx_fifo_read_ptr = 0;
	}

	duart_68681.channel[ch].rx_fifo_num--;

	if ( duart_68681.channel[ch].rx_fifo_num > 0 )
	{
		duart_68681.channel[ch].SR |= STATUS_RECEIVER_READY;
		if ( ch == 0 )
		{
			duart_68681.ISR |= INT_RXRDY_FFULLA;
		}
		else
		{
			duart_68681.ISR |= INT_RXRDY_FFULLB;
		}
	}
	else
	{
		duart_68681.channel[ch].SR &= ~STATUS_RECEIVER_READY;
		if ( ch == 0 )
		{
			duart_68681.ISR &= ~INT_RXRDY_FFULLA;
		}
		else
		{
			duart_68681.ISR &= ~INT_RXRDY_FFULLB;
		}
	}
	duart_68681_update_interrupts(machine);
	return r;
};

static TIMER_CALLBACK( tx_timer_callback )
{
	int ch = param & 1;

	if (duart_68681.tx_callback)
		duart_68681.tx_callback(ch, duart_68681.channel[ch].tx_data);

	duart_68681.channel[ch].tx_ready = 1;
	duart_68681.channel[ch].SR |= STATUS_TRANSMITTER_READY;

	if (ch == 0)
		duart_68681.ISR |= INT_TXRDYA;
	else
		duart_68681.ISR |= INT_TXRDYB;

	duart_68681_update_interrupts(machine);
	timer_adjust_oneshot(duart_68681.channel[ch].tx_timer, attotime_never, ch);
};

static void duart_68681_write_TX(running_machine *machine, int ch, UINT8 data)
{
	attotime period;

	duart_68681.channel[ch].tx_data = data;

	duart_68681.channel[ch].tx_ready = 0;
	duart_68681.channel[ch].SR &= ~STATUS_TRANSMITTER_READY;

	if (ch == 0)
		duart_68681.ISR &= ~INT_TXRDYA;
	else
		duart_68681.ISR &= ~INT_TXRDYB;

	duart_68681_update_interrupts(machine);

	period = ATTOTIME_IN_HZ(duart_68681.channel[ch].baud_rate / 10 );
	timer_adjust_oneshot(duart_68681.channel[ch].tx_timer, period, ch);
};

READ16_HANDLER(duart_68681_r)
{
	UINT16 r = 0x0;

	//logerror( "Reading 68681 reg %x\n", offset );
	switch (offset)
	{
		case 0x01: /* SRA */
			r = duart_68681.channel[0].SR;
			break;
		case 0x03: /* Rx Holding Register A */
			r = duart_68681_read_rx_fifo(machine, 0);
			break;
		case 0x04: /* IPCR */
			{
				UINT8 IP;
				if ( duart_68681.input_port_read != NULL )
					IP = duart_68681.input_port_read();
				else
					IP = 0x0;

				r = (((duart_68681.IP_last_state ^ IP) & 0x0f) << 4) | (IP & 0x0f);
				duart_68681.IP_last_state = IP;
				duart_68681.ISR &= ~INT_INPUT_PORT_CHANGE;
				duart_68681_update_interrupts(machine);
			}
			break;
		case 0x09: /* SRB */
			r = duart_68681.channel[1].SR;
			break;
		case 0x0b: /* RHRB */
			r = duart_68681_read_rx_fifo(machine, 1);
			break;
		default:
			logerror( "Reading unhandled 68681 reg %x\n", offset );
			break;
	}
	return r;
}

WRITE16_HANDLER(duart_68681_w)
{
	//logerror( "Writing 68681 reg %x with %04x\n", offset, data );
	switch(offset)
	{
		case 0x00: /* MRA */
			duart_68681_write_MR(0, data);
			break;
		case 0x01: /* CSRA */
			duart_68681_write_CSR(0, data, duart_68681.ACR);
			break;
		case 0x02: /* CRA */
			duart_68681_write_CR(0, data);
			break;
		case 0x03: /* THRA */
			duart_68681_write_TX(machine, 0, data);
			break;
		case 0x04: /* ACR */
			duart_68681.ACR = data;
			// TODO:
			//       bits 6-4: Counter/Timer Mode And Clock Source Select
			//       bits 3-0: IP3-0 Change-Of-State Interrupt Enable
			duart_68681_write_CSR(0, duart_68681.channel[0].CSR, data);
			duart_68681_write_CSR(1, duart_68681.channel[1].CSR, data);
			break;
		case 0x05: /* IMR */
			duart_68681.IMR = data;
			// TODO: handle update interrupts?
			break;
		case 0x08: /* MRB */
			duart_68681_write_MR(1, data);
			break;
		case 0x09: /* CSRB */
			duart_68681_write_CSR(1, data, duart_68681.ACR);
			break;
		case 0x0a: /* CRB */
			duart_68681_write_CR(1, data);
			break;
		case 0x0b: /* THRB */
			duart_68681_write_TX(machine, 1, data);
			break;
		case 0x0c: /* IVR */
			duart_68681.IVR = data;
			break;
		case 0x0d: /* OPCR */
			duart_68681.OPCR = data;
			break;
		case 0x0e: /* Set Output Port Bits */
			// TODO: bits set to 1 causes corresponding output port bit to be set to 0
			break;
		case 0x0f: /* Reset Output Port Bits */
			// TODO: bits set to 1 causes corresponding output port bit to be set to 1
			break;
		default:
			logerror( "Writing 68681 reg %x with %04x\n", offset, data );
			break;
	}
}

void duart_68681_rx_data( running_machine *machine, int ch, UINT8 data )
{
	if ( duart_68681.channel[ch].rx_enabled )
	{
		if ( duart_68681.channel[ch].rx_fifo_num >= RX_FIFO_SIZE )
		{
			logerror( "68681: FIFO overflow\n" );
			return;
		}
		duart_68681.channel[ch].rx_fifo[duart_68681.channel[ch].rx_fifo_write_ptr++] = data;
		if ( duart_68681.channel[ch].rx_fifo_write_ptr == RX_FIFO_SIZE )
		{
			duart_68681.channel[ch].rx_fifo_write_ptr = 0;
		}
		duart_68681.channel[ch].rx_fifo_num++;

		duart_68681.channel[ch].SR |= STATUS_RECEIVER_READY;
		if ( BIT(duart_68681.channel[ch].MR1,MODE_RX_INT_SELECT_BIT) == 0 )
		{
			duart_68681.ISR |= (ch == 0) ? INT_RXRDY_FFULLA : INT_RXRDY_FFULLB;
		}
		if ( duart_68681.channel[ch].rx_fifo_num == RX_FIFO_SIZE )
		{
			duart_68681.channel[ch].SR |= STATUS_FIFO_FULL;
			if ( BIT(duart_68681.channel[ch].MR1,MODE_RX_INT_SELECT_BIT) == 1 )
			{
				duart_68681.ISR |= (ch == 0) ? INT_RXRDY_FFULLA : INT_RXRDY_FFULLB;
			}
		}
		duart_68681_update_interrupts(machine);
	}
};

void duart_68681_init(int frequency, void (*irq_handler)(running_machine *machine, int vector), void (*tx_callback)(int channel, UINT8 data))
{
	memset(&duart_68681, 0, sizeof(duart_68681));

	duart_68681.frequency = frequency;
	duart_68681.tx_callback = tx_callback;
	duart_68681.irq_handler = irq_handler;

	// allocate timers
	duart_68681.channel[0].tx_timer = timer_alloc(tx_timer_callback, NULL);
	timer_adjust_oneshot(duart_68681.channel[0].tx_timer, attotime_never, 0);

	duart_68681.channel[1].tx_timer = timer_alloc(tx_timer_callback, NULL);
	timer_adjust_oneshot(duart_68681.channel[1].tx_timer, attotime_never, 1);
};
