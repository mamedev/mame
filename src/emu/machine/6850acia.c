/*********************************************************************

    6850acia.c

    6850 ACIA code

*********************************************************************/

#include "emu.h"
#include "6850acia.h"


/***************************************************************************
    MACROS
***************************************************************************/

#define CR1_0	0x03
#define CR4_2	0x1C
#define CR6_5	0x60
#define CR7		0x80

#define TXD(_data) \
	devcb_call_write_line(&acia_p->out_tx_func, _data)

#define RTS(_data) \
	devcb_call_write_line(&acia_p->out_rts_func, _data)

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

enum serial_state
{
	START,
	DATA,
	PARITY,
	STOP,
	STOP2,
};

enum _parity_type
{
	NONE,
	ODD,
	EVEN
};
typedef enum _parity_type parity_type;

typedef struct _acia6850_t acia6850_t;
struct _acia6850_t
{
	devcb_resolved_read_line	in_rx_func;
	devcb_resolved_write_line	out_tx_func;
	devcb_resolved_read_line	in_cts_func;
	devcb_resolved_write_line	out_rts_func;
	devcb_resolved_read_line	in_dcd_func;
	devcb_resolved_write_line	out_irq_func;

	UINT8	ctrl;
	UINT8	status;

	UINT8	tdr;
	UINT8	rdr;
	UINT8	rx_shift;
	UINT8	tx_shift;

	UINT8	rx_counter;
	UINT8	tx_counter;

	int	rx_clock;
	int	tx_clock;

	int	divide;

	/* Counters */
	int	tx_bits;
	int	rx_bits;
	int	tx_parity;
	int	rx_parity;

	/* TX/RX state */
	int	bits;
	parity_type parity;
	int	stopbits;
	int tx_int;

	/* Signals */
	int	overrun;
	int	reset;
	int rts;
	int brk;
	int first_reset;
	int status_read;
	enum	serial_state rx_state;
	enum	serial_state tx_state;

	emu_timer *rx_timer;
	emu_timer *tx_timer;
};


/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

static const int ACIA6850_DIVIDE[3] = { 1, 16, 64 };

static const int ACIA6850_WORD[8][3] =
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
    PROTOTYPES
***************************************************************************/

static TIMER_CALLBACK( receive_event );
static TIMER_CALLBACK( transmit_event );


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE acia6850_t *get_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->type == ACIA6850);
	return (acia6850_t *) device->token;
}


INLINE acia6850_interface *get_interface(const device_config *device)
{
	assert(device != NULL);
	assert(device->type == ACIA6850);
	return (acia6850_interface *) device->static_config;
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    DEVICE_RESET( acia6850 )
-------------------------------------------------*/

static DEVICE_RESET( acia6850 )
{
	acia6850_t *acia_p = get_token(device);

	int cts = devcb_call_read_line(&acia_p->in_cts_func);
	int dcd = devcb_call_read_line(&acia_p->in_dcd_func);

	acia_p->status = (cts << 3) | (dcd << 2) | ACIA6850_STATUS_TDRE;
	acia_p->tdr = 0;
	acia_p->rdr = 0;
	acia_p->tx_shift = 0;
	acia_p->rx_shift = 0;
	acia_p->tx_counter = 0;
	acia_p->rx_counter = 0;

	TXD(1);
	acia_p->overrun = 0;
	acia_p->status_read = 0;
	acia_p->brk = 0;

	acia_p->rx_state = START;
	acia_p->tx_state = START;

	devcb_call_write_line(&acia_p->out_irq_func, 1);

	if (acia_p->first_reset)
	{
		acia_p->first_reset = 0;

		RTS(1);
	}
	else
	{
		RTS(acia_p->rts);
	}
}



/*-------------------------------------------------
    DEVICE_START( acia6850 )
-------------------------------------------------*/

static DEVICE_START( acia6850 )
{
	acia6850_t *acia_p = get_token(device);
	acia6850_interface *intf = get_interface(device);

	/* resolve callbacks */
	devcb_resolve_read_line(&acia_p->in_rx_func, &intf->in_rx_func, device);
	devcb_resolve_write_line(&acia_p->out_tx_func, &intf->out_tx_func, device);
	devcb_resolve_read_line(&acia_p->in_cts_func, &intf->in_cts_func, device);
	devcb_resolve_write_line(&acia_p->out_rts_func, &intf->out_rts_func, device);
	devcb_resolve_read_line(&acia_p->in_dcd_func, &intf->in_dcd_func, device);
	devcb_resolve_write_line(&acia_p->out_irq_func, &intf->out_irq_func, device);

	acia_p->rx_clock = intf->rx_clock;
	acia_p->tx_clock = intf->tx_clock;
	acia_p->tx_counter = 0;
	acia_p->rx_counter = 0;
	acia_p->rx_timer = timer_alloc(device->machine, receive_event, (void *) device);
	acia_p->tx_timer = timer_alloc(device->machine, transmit_event, (void *) device);
	acia_p->first_reset = 1;
	acia_p->status_read = 0;
	acia_p->brk = 0;

	timer_reset(acia_p->rx_timer, attotime_never);
	timer_reset(acia_p->tx_timer, attotime_never);

	state_save_register_device_item(device, 0, acia_p->ctrl);
	state_save_register_device_item(device, 0, acia_p->status);
	state_save_register_device_item(device, 0, acia_p->rx_clock);
	state_save_register_device_item(device, 0, acia_p->tx_clock);
	state_save_register_device_item(device, 0, acia_p->rx_counter);
	state_save_register_device_item(device, 0, acia_p->tx_counter);
	state_save_register_device_item(device, 0, acia_p->rx_shift);
	state_save_register_device_item(device, 0, acia_p->tx_shift);
	state_save_register_device_item(device, 0, acia_p->rdr);
	state_save_register_device_item(device, 0, acia_p->tdr);
	state_save_register_device_item(device, 0, acia_p->rx_bits);
	state_save_register_device_item(device, 0, acia_p->tx_bits);
	state_save_register_device_item(device, 0, acia_p->rx_parity);
	state_save_register_device_item(device, 0, acia_p->tx_parity);
	state_save_register_device_item(device, 0, acia_p->tx_int);

	state_save_register_device_item(device, 0, acia_p->divide);
	state_save_register_device_item(device, 0, acia_p->overrun);
	state_save_register_device_item(device, 0, acia_p->reset);
	state_save_register_device_item(device, 0, acia_p->first_reset);
	state_save_register_device_item(device, 0, acia_p->rts);
	state_save_register_device_item(device, 0, acia_p->brk);
	state_save_register_device_item(device, 0, acia_p->status_read);
}


/*-------------------------------------------------
    acia6850_stat_r - Read Status Register
-------------------------------------------------*/

READ8_DEVICE_HANDLER( acia6850_stat_r )
{
	acia6850_t *acia_p = get_token(device);

	acia_p->status_read = 1;

	return acia_p->status;
}


/*-------------------------------------------------
    acia6850_ctrl_w - Write Control Register
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( acia6850_ctrl_w )
{
	acia6850_t *acia_p = get_token(device);

	int wordsel;
	int divide;


	// Counter Divide Select Bits

	divide = data & CR1_0;

	if (divide == 3)
	{
		acia_p->reset = 1;
		device_reset(device);
	}
	else
	{
		acia_p->reset = 0;
		acia_p->divide = ACIA6850_DIVIDE[divide];
	}

	// Word Select Bits

	wordsel = (data & CR4_2) >> 2;

	acia_p->bits = ACIA6850_WORD[wordsel][0];
	acia_p->parity = (parity_type)ACIA6850_WORD[wordsel][1];
	acia_p->stopbits = ACIA6850_WORD[wordsel][2];

	// Transmitter Control Bits

	switch ((data & CR6_5) >> 5)
	{
	case 0:
		acia_p->rts = 0;
		RTS(acia_p->rts);

		acia_p->tx_int = 0;
		acia_p->brk = 0;
		break;

	case 1:
		acia_p->rts = 0;
		RTS(acia_p->rts);

		acia_p->tx_int = 1;
		acia_p->brk = 0;
		break;

	case 2:
		acia_p->rts = 1;
		RTS(acia_p->rts);

		acia_p->tx_int = 0;
		acia_p->brk = 0;
		break;

	case 3:
		acia_p->rts = 0;
		RTS(acia_p->rts);

		acia_p->tx_int = 0;
		acia_p->brk = 1;
		break;
	}

	// After writing the word type, set the rx/tx clocks (provided the divide values have changed)

	if ((acia_p->ctrl ^ data) & CR1_0)
	{
		if (!acia_p->reset)
		{
			if (acia_p->rx_clock)
			{
				attotime rx_period = attotime_mul(ATTOTIME_IN_HZ(acia_p->rx_clock), acia_p->divide);
				timer_adjust_periodic(acia_p->rx_timer, rx_period, 0, rx_period);
			}

			if (acia_p->tx_clock)
			{
				attotime tx_period = attotime_mul(ATTOTIME_IN_HZ(acia_p->tx_clock), acia_p->divide);
				timer_adjust_periodic(acia_p->tx_timer, tx_period, 0, tx_period);
			}
		}
	}
	acia_p->ctrl = data;
}


/*-------------------------------------------------
    acia6850_check_interrupts
-------------------------------------------------*/

static void acia6850_check_interrupts(const device_config *device)
{
	acia6850_t *acia_p = get_token(device);

	int irq = (acia_p->tx_int && (acia_p->status & ACIA6850_STATUS_TDRE)) ||
		((acia_p->ctrl & 0x80) && ((acia_p->status & (ACIA6850_STATUS_RDRF|ACIA6850_STATUS_DCD)) || acia_p->overrun));

	if (irq)
	{
		acia_p->status |= ACIA6850_STATUS_IRQ;
		devcb_call_write_line(&acia_p->out_irq_func, 0);
	}
	else
	{
		acia_p->status &= ~ACIA6850_STATUS_IRQ;
		devcb_call_write_line(&acia_p->out_irq_func, 1);
	}
}


/*-------------------------------------------------
    acia6850_data_w - Write transmit register
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( acia6850_data_w )
{
	acia6850_t *acia_p = get_token(device);

	if (!acia_p->reset)
	{
		acia_p->tdr = data;
		acia_p->status &= ~ACIA6850_STATUS_TDRE;
		acia6850_check_interrupts(device);
	}
	else
	{
		logerror("%s:ACIA %p: Data write while in reset!\n", cpuexec_describe_context(device->machine), device);
	}
}


/*-------------------------------------------------
    acia6850_data_r - Read character
-------------------------------------------------*/

READ8_DEVICE_HANDLER( acia6850_data_r )
{
	acia6850_t *acia_p = get_token(device);

	acia_p->status &= ~(ACIA6850_STATUS_RDRF | ACIA6850_STATUS_IRQ | ACIA6850_STATUS_PE);

	if (acia_p->status_read)
	{
		int dcd = devcb_call_read_line(&acia_p->in_dcd_func);

		acia_p->status_read = 0;
		acia_p->status &= ~(ACIA6850_STATUS_OVRN | ACIA6850_STATUS_DCD);

		if (dcd)
		{
			acia_p->status |= ACIA6850_STATUS_DCD;
		}
	}

	if (acia_p->overrun == 1)
	{
		acia_p->status |= ACIA6850_STATUS_OVRN;
		acia_p->overrun = 0;
	}

	acia6850_check_interrupts(device);

	return acia_p->rdr;
}


/*-------------------------------------------------
    tx_tick - Transmit a bit
-------------------------------------------------*/

static void tx_tick(const device_config *device)
{
	acia6850_t *acia_p = get_token(device);

	int cts = devcb_call_read_line(&acia_p->in_cts_func);

	if (cts)
	{
		acia_p->status |= ACIA6850_STATUS_CTS;
	}
	else
	{
		acia_p->status &= ~ACIA6850_STATUS_CTS;
	}

	switch (acia_p->tx_state)
	{
		case START:
		{
			if (acia_p->brk)
			{
				// transmit break

				TXD(0);
			}
			else
			{
				if (acia_p->status & ACIA6850_STATUS_TDRE)
				{
					// transmitter idle

					TXD(1);
				}
				else
				{
					// transmit character

					//logerror("ACIA6850 #%u: TX DATA %x\n", which, acia_p->tdr);
					//logerror("ACIA6850 #%u: TX START BIT\n", which);

					TXD(0);

					acia_p->tx_bits = acia_p->bits;
					acia_p->tx_shift = acia_p->tdr;

					// inhibit TDRE bit if Clear-to-Send is high

					if (!(acia_p->status & ACIA6850_STATUS_CTS))
					{
						acia_p->status |= ACIA6850_STATUS_TDRE;
					}

					acia6850_check_interrupts(device);

					acia_p->tx_state = DATA;
				}
			}
			break;
		}
		case DATA:
		{
			int val = acia_p->tx_shift & 1;
			//logerror("ACIA6850 #%u: TX DATA BIT %x\n", which, val);

			TXD(val);
			acia_p->tx_parity ^= val;
			acia_p->tx_shift >>= 1;

			if (--(acia_p->tx_bits) == 0)
			{
				acia_p->tx_state = (acia_p->parity == NONE) ? STOP : PARITY;
			}

			break;
		}
		case PARITY:
		{
			if (acia_p->parity == EVEN)
			{
				TXD((acia_p->tx_parity & 1) ? 1 : 0);
			}
			else
			{
				TXD((acia_p->tx_parity & 1) ? 0 : 1);
			}

			//logerror("ACIA6850 #%u: TX PARITY BIT %x\n", which, *acia_p->tx_pin);
			acia_p->tx_parity = 0;
			acia_p->tx_state = STOP;
			break;
		}
		case STOP:
		{
			//logerror("ACIA6850 #%u: TX STOP BIT\n", which);
			TXD(1);

			if (acia_p->stopbits == 1)
			{
				acia_p->tx_state = START;
			}
			else
			{
				acia_p->tx_state = STOP2;
			}
			break;
		}
		case STOP2:
		{
			//logerror("ACIA6850 #%u: TX STOP BIT\n", which);
			TXD(1);
			acia_p->tx_state = START;
			break;
		}
	}
}


/*-------------------------------------------------
    TIMER_CALLBACK( transmit_event )
-------------------------------------------------*/

static TIMER_CALLBACK( transmit_event )
{
	const device_config *device = (const device_config *)ptr;
	acia6850_t *acia_p = get_token(device);
	tx_tick(device);
	acia_p->tx_counter = 0;
}


/*-------------------------------------------------
    acia6850_tx_clock_in - As above, but using the tx pin
-------------------------------------------------*/

void acia6850_tx_clock_in(const device_config *device)
{
	acia6850_t *acia_p = get_token(device);

	int cts = devcb_call_read_line(&acia_p->in_cts_func);

	if (cts)
	{
		acia_p->status |= ACIA6850_STATUS_CTS;
	}
	else
	{
		acia_p->status &= ~ACIA6850_STATUS_CTS;
	}

	acia_p->tx_counter ++;

	if ( acia_p->tx_counter > acia_p->divide-1)
	{
		tx_tick(device);
		acia_p->tx_counter = 0;
	}

}


/*-------------------------------------------------
    rx_tick - Receive a bit
-------------------------------------------------*/

static void rx_tick(const device_config *device)
{
	acia6850_t *acia_p = get_token(device);

	int dcd = devcb_call_read_line(&acia_p->in_dcd_func);

	if (dcd)
	{
		acia_p->status |= ACIA6850_STATUS_DCD;
		acia6850_check_interrupts(device);
	}
	else if ((acia_p->status & (ACIA6850_STATUS_DCD|ACIA6850_STATUS_IRQ)) == ACIA6850_STATUS_DCD)
	{
		acia_p->status &= ~ACIA6850_STATUS_DCD;
	}

	if (acia_p->status & ACIA6850_STATUS_DCD)
	{
		acia_p->rx_state = START;
	}
	else
	{
		int rxd = devcb_call_read_line(&acia_p->in_rx_func);

		switch (acia_p->rx_state)
		{
			case START:
			{
				if (rxd == 0)
				{
					//logerror("ACIA6850 #%u: RX START BIT\n", which);
					acia_p->rx_shift = 0;
					acia_p->rx_parity = 0;
					acia_p->rx_bits = acia_p->bits;
					acia_p->rx_state = DATA;
				}
				break;
			}
			case DATA:
			{
				//logerror("ACIA6850 #%u: RX DATA BIT %x\n", which, rxd);
				acia_p->rx_shift |= rxd ? 0x80 : 0;
				acia_p->rx_parity ^= rxd;

				if (--acia_p->rx_bits == 0)
				{
					if (acia_p->status & ACIA6850_STATUS_RDRF)
					{
						acia_p->overrun = 1;
						acia6850_check_interrupts(device);
					}

					acia_p->rx_state = acia_p->parity == NONE ? STOP : PARITY;
				}
				else
				{
					acia_p->rx_shift >>= 1;
				}
				break;
			}
			case PARITY:
			{
				//logerror("ACIA6850 #%u: RX PARITY BIT %x\n", which, rxd);
				acia_p->rx_parity ^= rxd;

				if (acia_p->parity == EVEN)
				{
					if (acia_p->rx_parity)
					{
						acia_p->status |= ACIA6850_STATUS_PE;
					}
				}
				else
				{
					if (!acia_p->rx_parity)
					{
						acia_p->status |= ACIA6850_STATUS_PE;
					}
				}

				acia_p->rx_state = STOP;
				break;
			}
			case STOP:
			{
				if (rxd == 1)
				{
					//logerror("ACIA6850 #%u: RX STOP BIT\n", which);
					if (acia_p->stopbits == 1)
					{
						acia_p->status &= ~ACIA6850_STATUS_FE;

						if (!(acia_p->status & ACIA6850_STATUS_RDRF))
						{
							//logerror("ACIA6850 #%u: RX DATA %x\n", which, acia_p->rx_shift);
							acia_p->rdr = acia_p->rx_shift;
							acia_p->status |= ACIA6850_STATUS_RDRF;
							acia6850_check_interrupts(device);
						}

						acia_p->rx_state = START;
					}
					else
					{
						acia_p->rx_state = STOP2;
					}
				}
				else
				{
					acia_p->status |= ACIA6850_STATUS_FE;
					acia_p->rx_state = START;
				}
				break;
			}
			case STOP2:
			{
				if (rxd == 1)
				{
					//logerror("ACIA6850 #%u: RX STOP BIT\n", which);
					acia_p->status &= ~ACIA6850_STATUS_FE;

					if (!(acia_p->status & ACIA6850_STATUS_RDRF))
					{
						//logerror("ACIA6850 #%u: RX DATA %x\n", which, acia_p->rx_shift);
						acia_p->rdr = acia_p->rx_shift;
						acia_p->status |= ACIA6850_STATUS_RDRF;
						acia6850_check_interrupts(device);
					}

					acia_p->rx_state = START;
				}
				else
				{
					acia_p->status |= ACIA6850_STATUS_FE;
					acia_p->rx_state = START;
				}
				break;
			}
		}
	}
}


/*-------------------------------------------------
    TIMER_CALLBACK( receive_event ) - Called on
    receive timer event
-------------------------------------------------*/

static TIMER_CALLBACK( receive_event )
{
	const device_config *device = (const device_config *)ptr;
	acia6850_t *acia_p = get_token(device);
	rx_tick(device);
	acia_p->rx_counter = 0;
}


/*-------------------------------------------------
    acia6850_rx_clock_in - As above, but using the rx pin
-------------------------------------------------*/

void acia6850_rx_clock_in(const device_config *device)
{
	acia6850_t *acia_p = get_token(device);

	int dcd = devcb_call_read_line(&acia_p->in_dcd_func);

	if (dcd)
	{
		acia_p->status |= ACIA6850_STATUS_DCD;
		acia6850_check_interrupts(device);
	}
	else if ((acia_p->status & (ACIA6850_STATUS_DCD|ACIA6850_STATUS_IRQ)) == ACIA6850_STATUS_DCD)
	{
		acia_p->status &= ~ACIA6850_STATUS_DCD;
	}

	acia_p->rx_counter ++;

	if ( acia_p->rx_counter > acia_p->divide-1)
	{
		rx_tick(device);
		acia_p->rx_counter = 0;
	}
}


/*-------------------------------------------------
    acia6850_set_rx_clock - Set clock frequencies
    dynamically
-------------------------------------------------*/

void acia6850_set_rx_clock(const device_config *device, int clock)
{
	acia6850_t *acia_p = get_token(device);
	acia_p->rx_clock = clock;
}


/*-------------------------------------------------
    acia6850_set_tx_clock - Set clock frequencies
    dynamically
-------------------------------------------------*/

void acia6850_set_tx_clock(const device_config *device, int clock)
{
	acia6850_t *acia_p = get_token(device);
	acia_p->tx_clock = clock;
}


/*-------------------------------------------------
    DEVICE_GET_INFO( acia6850 )
-------------------------------------------------*/

DEVICE_GET_INFO( acia6850 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(acia6850_t);					break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;									break;
		case DEVINFO_INT_CLASS:							info->i = DEVICE_CLASS_PERIPHERAL;				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(acia6850);		break;
		case DEVINFO_FCT_STOP:							/* Nothing */									break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(acia6850);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "6850 ACIA");					break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "6850 ACIA");					break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:						/* Nothing */									break;
	}
}
