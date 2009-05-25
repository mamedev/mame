/*
    68681 DUART

    Written by Mariusz Wojcieszek
*/

#include "driver.h"
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

} DUART68681_CHANNEL;

typedef struct
{
	/* device */
	const device_config *device;

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

INLINE duart68681_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == DUART68681);

	return (duart68681_state *)device->token;
}

static void duart68681_update_interrupts(duart68681_state *duart68681)
{
	if ( (duart68681->ISR & duart68681->IMR) != 0 )
	{
		if ( duart68681->duart_config->irq_handler )
		{
			duart68681->duart_config->irq_handler( duart68681->device, duart68681->IVR );
		}
	}
};

static TIMER_CALLBACK( duart_timer_callback )
{
	const device_config *device = (const device_config *)ptr;
	duart68681_state	*duart68681 = get_safe_token(device);

	duart68681->ISR |= INT_COUNTER_READY;
	duart68681_update_interrupts(duart68681);
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
};

static void duart68681_write_CSR(duart68681_state *duart68681, int ch, UINT8 data, UINT8 ACR)
{
	static const int baud_rate_ACR_0[] = { 50, 110, 134, 200, 300, 600, 1200, 1050, 2400, 4800, 7200, 9600, 38400, 0, 0, 0 };
	static const int baud_rate_ACR_1[] = { 75, 110, 134, 150, 300, 600, 1200, 2000, 2400, 4800, 1800, 9600, 19200, 0, 0, 0 };

	duart68681->channel[ch].CSR = data;

	if ( BIT(ACR,7) == 0 )
	{
		duart68681->channel[ch].baud_rate = baud_rate_ACR_0[data & 0x0f];
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
	if ( BIT(data,0) )
	{
		duart68681->channel[ch].rx_enabled = 1;
	}
	if ( BIT(data,1) )
	{
		duart68681->channel[ch].rx_enabled = 0;
		duart68681->channel[ch].SR &= ~STATUS_RECEIVER_READY;
	}

	if ( BIT(data,2) )
	{
		duart68681->channel[ch].tx_enabled = 1;
		duart68681->channel[ch].tx_ready = 1;
		duart68681->channel[ch].SR |= STATUS_TRANSMITTER_READY;
	}
	if ( BIT(data,3) )
	{
		duart68681->channel[ch].tx_enabled = 0;
		duart68681->channel[ch].tx_ready = 0;
		duart68681->channel[ch].SR &= ~STATUS_TRANSMITTER_READY;
	}

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
			duart68681->channel[ch].rx_fifo_read_ptr = 0;
			duart68681->channel[ch].rx_fifo_write_ptr = 0;
			duart68681->channel[ch].rx_fifo_num = 0;
			break;
		case 3: /* Reset channel A transmitter */
			duart68681->channel[ch].tx_enabled = 0;
			duart68681->channel[ch].SR &= ~STATUS_TRANSMITTER_READY;
			timer_adjust_oneshot(duart68681->channel[ch].tx_timer, attotime_never, ch);
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
			duart68681_update_interrupts(duart68681);
			break;
		default:
			LOG(( "68681: Unhandled command (%x) in CR%d\n", (data >> 4) & 0x07, ch ));
			break;
	}
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

	if ( duart68681->channel[ch].rx_fifo_num > 0 )
	{
		duart68681->channel[ch].SR |= STATUS_RECEIVER_READY;
		if ( ch == 0 )
		{
			duart68681->ISR |= INT_RXRDY_FFULLA;
		}
		else
		{
			duart68681->ISR |= INT_RXRDY_FFULLB;
		}
	}
	else
	{
		duart68681->channel[ch].SR &= ~STATUS_RECEIVER_READY;
		if ( ch == 0 )
		{
			duart68681->ISR &= ~INT_RXRDY_FFULLA;
		}
		else
		{
			duart68681->ISR &= ~INT_RXRDY_FFULLB;
		}
	}
	duart68681_update_interrupts(duart68681);

	return r;
};

static TIMER_CALLBACK( tx_timer_callback )
{
	const device_config *device = (const device_config *)ptr;
	duart68681_state	*duart68681 = get_safe_token(device);
	int ch = param & 1;

	if (duart68681->duart_config->tx_callback)
		duart68681->duart_config->tx_callback(device, ch, duart68681->channel[ch].tx_data);

	duart68681->channel[ch].tx_ready = 1;
	duart68681->channel[ch].SR |= STATUS_TRANSMITTER_READY;

	if (ch == 0)
		duart68681->ISR |= INT_TXRDYA;
	else
		duart68681->ISR |= INT_TXRDYB;

	duart68681_update_interrupts(duart68681);
	timer_adjust_oneshot(duart68681->channel[ch].tx_timer, attotime_never, ch);
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

	period = ATTOTIME_IN_HZ(duart68681->channel[ch].baud_rate / 10 );
	timer_adjust_oneshot(duart68681->channel[ch].tx_timer, period, ch);
};

READ8_DEVICE_HANDLER(duart68681_r)
{
	duart68681_state* duart68681 = get_safe_token(device);
	UINT8 r = 0xff;

	offset &= 0xf;

	LOG(( "Reading 68681 (%s) reg %x (%s) ", device->tag, offset, duart68681_reg_read_names[offset] ));

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
					/*
                    if (input_code_pressed(KEYCODE_1)) r ^= 0x0001;
                    if (input_code_pressed(KEYCODE_2)) r ^= 0x0002;
                    if (input_code_pressed(KEYCODE_3)) r ^= 0x0004;
                    if (input_code_pressed(KEYCODE_4)) r ^= 0x0008;
                    if (input_code_pressed(KEYCODE_5)) r ^= 0x0010;
                    if (input_code_pressed(KEYCODE_6)) r ^= 0x0020;
                    if (input_code_pressed(KEYCODE_7)) r ^= 0x0040;
                    if (input_code_pressed(KEYCODE_8)) r ^= 0x0080;
                    */
				}
			break;
		case 0x0e: /* Start counter command */
			switch( (duart68681->ACR >> 4) & 0x07 )
			{
				case 0x07: /* Timer, CLK/16 */
					{
						//double hz;
						//attotime rate = attotime_mul(ATTOTIME_IN_HZ(duart68681->clock), 16*duart68681->CTR.w.l);
						attotime rate = ATTOTIME_IN_HZ(2*device->clock/(2*16*16*duart68681->CTR.w.l));
						//hz = ATTOSECONDS_TO_HZ(rate.attoseconds);
						timer_adjust_periodic(duart68681->duart_timer, rate, 0, rate);
					}
					break;
			}
			break;
		case 0x0f: /* Stop counter command */
			duart68681->ISR &= ~INT_COUNTER_READY;
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
	LOG(( "Writing 68681 (%s) reg %x (%s) with %04x\n", device->tag, offset, duart68681_reg_write_names[offset], data ));

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
			if (((data >> 4) & 0x07) != 0x07)
				logerror( "68681 (%s): Unhandled timer/counter mode %d\n", device->tag, (data >> 4) & 0x07);

			duart68681_write_CSR(duart68681, 0, duart68681->channel[0].CSR, data);
			duart68681_write_CSR(duart68681, 1, duart68681->channel[1].CSR, data);
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
				logerror( "68681 (%s): Unhandled OPCR value: %02x\n", device->tag, data);
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

void duart68681_rx_data( const device_config* device, int ch, UINT8 data )
{
	duart68681_state *duart68681 = get_safe_token(device);

	if ( duart68681->channel[ch].rx_enabled )
	{
		if ( duart68681->channel[ch].rx_fifo_num >= RX_FIFO_SIZE )
		{
			LOG(( "68681: FIFO overflow\n" ));
			return;
		}
		duart68681->channel[ch].rx_fifo[duart68681->channel[ch].rx_fifo_write_ptr++] = data;
		if ( duart68681->channel[ch].rx_fifo_write_ptr == RX_FIFO_SIZE )
		{
			duart68681->channel[ch].rx_fifo_write_ptr = 0;
		}
		duart68681->channel[ch].rx_fifo_num++;

		duart68681->channel[ch].SR |= STATUS_RECEIVER_READY;
		if ( BIT(duart68681->channel[ch].MR1,MODE_RX_INT_SELECT_BIT) == 0 )
		{
			duart68681->ISR |= (ch == 0) ? INT_RXRDY_FFULLA : INT_RXRDY_FFULLB;
		}
		if ( duart68681->channel[ch].rx_fifo_num == RX_FIFO_SIZE )
		{
			duart68681->channel[ch].SR |= STATUS_FIFO_FULL;
			if ( BIT(duart68681->channel[ch].MR1,MODE_RX_INT_SELECT_BIT) == 1 )
			{
				duart68681->ISR |= (ch == 0) ? INT_RXRDY_FFULLA : INT_RXRDY_FFULLB;
			}
		}
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
	assert(device->tag != NULL);

	state_save_register_device_item(device, 0, duart68681->ACR);
	state_save_register_device_item(device, 0, duart68681->IMR);
	state_save_register_device_item(device, 0, duart68681->ISR);
	state_save_register_device_item(device, 0, duart68681->IVR);
	state_save_register_device_item(device, 0, duart68681->OPCR);
	state_save_register_device_item(device, 0, duart68681->CTR);
	state_save_register_device_item(device, 0, duart68681->IP_last_state);

	state_save_register_device_item(device, 0, duart68681->channel[0].CR);
	state_save_register_device_item(device, 0, duart68681->channel[0].CSR);
	state_save_register_device_item(device, 0, duart68681->channel[0].MR1);
	state_save_register_device_item(device, 0, duart68681->channel[0].MR2);
	state_save_register_device_item(device, 0, duart68681->channel[0].MR_ptr);
	state_save_register_device_item(device, 0, duart68681->channel[0].SR);
	state_save_register_device_item(device, 0, duart68681->channel[0].baud_rate);
	state_save_register_device_item(device, 0, duart68681->channel[0].rx_enabled);
	state_save_register_device_item_array(device, 0, duart68681->channel[0].rx_fifo);
	state_save_register_device_item(device, 0, duart68681->channel[0].rx_fifo_read_ptr);
	state_save_register_device_item(device, 0, duart68681->channel[0].rx_fifo_write_ptr);
	state_save_register_device_item(device, 0, duart68681->channel[0].rx_fifo_num);
	state_save_register_device_item(device, 0, duart68681->channel[0].tx_enabled);
	state_save_register_device_item(device, 0, duart68681->channel[0].tx_data);
	state_save_register_device_item(device, 0, duart68681->channel[0].tx_ready);

	state_save_register_device_item(device, 0, duart68681->channel[1].CR);
	state_save_register_device_item(device, 0, duart68681->channel[1].CSR);
	state_save_register_device_item(device, 0, duart68681->channel[1].MR1);
	state_save_register_device_item(device, 0, duart68681->channel[1].MR2);
	state_save_register_device_item(device, 0, duart68681->channel[1].MR_ptr);
	state_save_register_device_item(device, 0, duart68681->channel[1].SR);
	state_save_register_device_item(device, 0, duart68681->channel[1].baud_rate);
	state_save_register_device_item(device, 0, duart68681->channel[1].rx_enabled);
	state_save_register_device_item_array(device, 0, duart68681->channel[1].rx_fifo);
	state_save_register_device_item(device, 0, duart68681->channel[1].rx_fifo_read_ptr);
	state_save_register_device_item(device, 0, duart68681->channel[1].rx_fifo_write_ptr);
	state_save_register_device_item(device, 0, duart68681->channel[1].rx_fifo_num);
	state_save_register_device_item(device, 0, duart68681->channel[1].tx_enabled);
	state_save_register_device_item(device, 0, duart68681->channel[1].tx_data);
	state_save_register_device_item(device, 0, duart68681->channel[1].tx_ready);
}

/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET(duart68681)
{
	duart68681_state *duart68681 = get_safe_token(device);

	memset(duart68681, 0, sizeof(duart68681_state));
	duart68681->duart_config = (const duart68681_config *)device->static_config;
	duart68681->device = device;
	duart68681->IVR = 0x0f;

	// allocate timers
	duart68681->channel[0].tx_timer = timer_alloc(device->machine, tx_timer_callback, (void*)device);
	timer_adjust_oneshot(duart68681->channel[0].tx_timer, attotime_never, 0);

	duart68681->channel[1].tx_timer = timer_alloc(device->machine, tx_timer_callback, (void*)device);
	timer_adjust_oneshot(duart68681->channel[1].tx_timer, attotime_never, 1);

	duart68681->duart_timer = timer_alloc(device->machine, duart_timer_callback, (void*)device);
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
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_PERIPHERAL;		break;

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
