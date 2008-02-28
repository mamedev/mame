/*
    6850 ACIA
*/

#include "driver.h"
#include "timer.h"
#include "6850acia.h"

#define CR1_0	0x03
#define CR4_2	0x1C
#define CR6_5	0x60
#define CR7		0x80

enum serial_state
{
	START,
	DATA,
	PARITY,
	STOP,
	STOP2,
};

enum parity_type
{
	NONE,
	ODD,
	EVEN
};

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

typedef struct _acia_6850_
{
	UINT8	ctrl;
	UINT8	status;

	UINT8	*rx_pin;
	UINT8	*tx_pin;
	UINT8	*cts_pin;
	UINT8	*rts_pin;
	UINT8	*dcd_pin;

	UINT8	tdr;
	UINT8	rdr;
	UINT8	rx_shift;
	UINT8	tx_shift;

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
	enum 	parity_type parity;
	int	stopbits;
	int tx_int;

	/* Signals */
	int	overrun;
	int	reset;
	int rts;
	int brk;
	int first_reset;
	int status_read;
	enum 	serial_state rx_state;
	enum 	serial_state tx_state;

	emu_timer *rx_timer;
	emu_timer *tx_timer;

	void (*int_callback)(int param);
} acia_6850;

static acia_6850 acia[MAX_ACIA];

static TIMER_CALLBACK( receive_event );
static TIMER_CALLBACK( transmit_event );

/*
    Reset the chip
*/
static void acia6850_reset(int which)
{
	acia_6850 *acia_p = &acia[which];
	int cts = 0, dcd = 0;

	if (acia_p->cts_pin)
	{
		cts = *acia_p->cts_pin;
	}

	if (acia_p->dcd_pin)
	{
		dcd = *acia_p->dcd_pin;
	}

	acia_p->status = (cts << 3) | (dcd << 2) | ACIA6850_STATUS_TDRE;
	acia_p->tdr = 0;
	acia_p->rdr = 0;
	acia_p->tx_shift = 0;
	acia_p->rx_shift = 0;
	*acia_p->tx_pin = 1;
	acia_p->overrun = 0;
	acia_p->status_read = 0;
	acia_p->brk = 0;

	acia_p->rx_state = START;
	acia_p->tx_state = START;

	if (acia_p->int_callback)
	{
		acia_p->int_callback(1);
	}

	if (acia_p->first_reset)
	{
		acia_p->first_reset = 0;

		if (acia_p->rts_pin)
		{
			*acia_p->rts_pin = 1;
		}
	}
	else
	{
		if (acia_p->rts_pin)
		{
			*acia_p->rts_pin = acia_p->rts;
		}
	}
}

/*
    Called by drivers
*/
void acia6850_config(int which, const struct acia6850_interface *intf)
{
	acia_6850 *acia_p = &acia[which];

	if (which >= MAX_ACIA)
	{
		return;
	}

	acia_p->rx_clock = intf->rx_clock;
	acia_p->tx_clock = intf->tx_clock;
	acia_p->rx_pin = intf->rx_pin;
	acia_p->tx_pin = intf->tx_pin;
	acia_p->cts_pin = intf->cts_pin;
	acia_p->rts_pin = intf->rts_pin;
	acia_p->dcd_pin = intf->dcd_pin;
	acia_p->rx_timer = timer_alloc(receive_event, NULL);
	acia_p->tx_timer = timer_alloc(transmit_event, NULL);
	acia_p->int_callback = intf->int_callback;
	acia_p->first_reset = 1;
	acia_p->status_read = 0;
	acia_p->brk = 0;

	timer_reset(acia_p->rx_timer, attotime_never);
	timer_reset(acia_p->tx_timer, attotime_never);

	state_save_register_item("acia6850", which, acia_p->ctrl);
	state_save_register_item("acia6850", which, acia_p->status);
	state_save_register_item("acia6850", which, acia_p->rx_clock);
	state_save_register_item("acia6850", which, acia_p->tx_clock);
	state_save_register_item("acia6850", which, acia_p->rx_shift);
	state_save_register_item("acia6850", which, acia_p->tx_shift);
	state_save_register_item("acia6850", which, acia_p->rdr);
	state_save_register_item("acia6850", which, acia_p->tdr);
	state_save_register_item("acia6850", which, acia_p->rx_bits);
	state_save_register_item("acia6850", which, acia_p->tx_bits);
	state_save_register_item("acia6850", which, acia_p->rx_parity);
	state_save_register_item("acia6850", which, acia_p->tx_parity);
	state_save_register_item("acia6850", which, acia_p->tx_int);

	state_save_register_item("acia6850", which, acia_p->divide);
	state_save_register_item("acia6850", which, acia_p->overrun);
	state_save_register_item("acia6850", which, acia_p->reset);
	state_save_register_item("acia6850", which, acia_p->first_reset);
	state_save_register_item("acia6850", which, acia_p->rts);
	state_save_register_item("acia6850", which, acia_p->brk);
	state_save_register_item("acia6850", which, acia_p->status_read);
}

/*
    Read Status Register
*/
static UINT8 acia6850_stat_r(int which)
{
	acia_6850 *acia_p = &acia[which];

	acia_p->status_read = 1;

	return acia_p->status;
}

/*
    Write Control Register
*/
static void acia6850_ctrl_w(int which, UINT8 data)
{
	acia_6850 *acia_p = &acia[which];

	int wordsel;
	int divide;

	acia_p->ctrl = data;

	// Counter Divide Select Bits

	divide = data & CR1_0;

	if (divide == 3)
	{
		acia_p->reset = 1;
		acia6850_reset(which);
	}
	else
	{
		acia_p->reset = 0;
		acia_p->divide = ACIA6850_DIVIDE[divide];
	}

	// Word Select Bits

	wordsel = (data & CR4_2) >> 2;

	acia_p->bits = ACIA6850_WORD[wordsel][0];
	acia_p->parity = ACIA6850_WORD[wordsel][1];
	acia_p->stopbits = ACIA6850_WORD[wordsel][2];

	// Transmitter Control Bits

	switch ((data & CR6_5) >> 5)
	{
	case 0:
		if (acia_p->rts_pin)
		{
			*acia_p->rts_pin = acia_p->rts = 0;
		}

		acia_p->tx_int = 0;
		acia_p->brk = 0;
		break;

	case 1:
		if (acia_p->rts_pin)
		{
			*acia_p->rts_pin = acia_p->rts = 0;
		}

		acia_p->tx_int = 1;
		acia_p->brk = 0;
		break;

	case 2:
		if (acia_p->rts_pin)
		{
			*acia_p->rts_pin = acia_p->rts = 1;
		}

		acia_p->tx_int = 0;
		acia_p->brk = 0;
		break;

	case 3:
		if (acia_p->rts_pin)
		{
			*acia_p->rts_pin = acia_p->rts = 0;
		}

		acia_p->tx_int = 0;
		acia_p->brk = 1;
		break;
	}

	// After writing the word type, start receive clock

	if (!acia_p->reset)
	{
		attotime rx_period = attotime_mul(ATTOTIME_IN_HZ(acia_p->rx_clock), acia_p->divide);
		attotime tx_period = attotime_mul(ATTOTIME_IN_HZ(acia_p->tx_clock), acia_p->divide);
		timer_adjust_periodic(acia_p->rx_timer, rx_period, which, rx_period);
		timer_adjust_periodic(acia_p->tx_timer, tx_period, which, tx_period);
	}
}

static void acia6850_check_interrupts(int which)
{
	acia_6850 *acia_p = &acia[which];

	int irq = (acia_p->tx_int && (acia_p->status & ACIA6850_STATUS_TDRE)) ||
		((acia_p->ctrl & 0x80) && ((acia_p->status & (ACIA6850_STATUS_RDRF|ACIA6850_STATUS_DCD)) || acia_p->overrun));

	if (irq)
	{
		acia_p->status |= ACIA6850_STATUS_IRQ;

		if (acia_p->int_callback)
		{
			acia_p->int_callback(0);
		}
	}
	else
	{
		acia_p->status &= ~ACIA6850_STATUS_IRQ;

		if (acia_p->int_callback)
		{
			acia_p->int_callback(1);
		}
	}
}

/*
    Write transmit register
*/
static void acia6850_data_w(int which, UINT8 data)
{
	acia_6850 *acia_p = &acia[which];

	if (!acia_p->reset)
	{
		acia_p->tdr = data;
		acia_p->status &= ~ACIA6850_STATUS_TDRE;
		acia6850_check_interrupts(which);
	}
	else
	{
		logerror("ACIA %d: Data write while in reset! (%x)\n", which, activecpu_get_previouspc());
	}
}

/*
    Read character
*/
static UINT8 acia6850_data_r(int which)
{
	acia_6850 *acia_p = &acia[which];

	acia_p->status &= ~(ACIA6850_STATUS_RDRF | ACIA6850_STATUS_IRQ | ACIA6850_STATUS_PE);

	if (acia_p->status_read)
	{
		acia_p->status_read = 0;
		acia_p->status &= ~(ACIA6850_STATUS_OVRN | ACIA6850_STATUS_DCD);

		if (acia_p->dcd_pin && *acia_p->dcd_pin)
		{
			acia_p->status |= ACIA6850_STATUS_DCD;
		}
	}

	if (acia_p->overrun == 1)
	{
		acia_p->status |= ACIA6850_STATUS_OVRN;
		acia_p->overrun = 0;
	}

	acia6850_check_interrupts(which);

	return acia_p->rdr;
}

/*
    Transmit a bit
*/
static TIMER_CALLBACK( transmit_event )
{
	int which = param;
	acia_6850 *acia_p = &acia[which];

	if (acia_p->cts_pin && *acia_p->cts_pin)
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

				*acia_p->tx_pin = 0;
			}
			else
			{
				if (acia_p->status & ACIA6850_STATUS_TDRE)
				{
					// transmitter idle

					*acia_p->tx_pin = 1;
				}
				else
				{
					// transmit character

					//logerror("ACIA6850 #%u: TX DATA %x\n", which, acia_p->tdr);
					//logerror("ACIA6850 #%u: TX START BIT\n", which);

					*acia_p->tx_pin = 0;

					acia_p->tx_bits = acia_p->bits;
					acia_p->tx_shift = acia_p->tdr;

					// inhibit TDRE bit if Clear-to-Send is high

					if (!(acia_p->status & ACIA6850_STATUS_CTS))
					{
						acia_p->status |= ACIA6850_STATUS_TDRE;
					}

					acia6850_check_interrupts(which);

					acia_p->tx_state = DATA;
				}
			}
			break;
		}
		case DATA:
		{
			int val = acia_p->tx_shift & 1;
			//logerror("ACIA6850 #%u: TX DATA BIT %x\n", which, val);

			*acia_p->tx_pin = val;
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
				*acia_p->tx_pin = (acia_p->tx_parity & 1) ? 1 : 0;
			}
			else
			{
				*acia_p->tx_pin = (acia_p->tx_parity & 1) ? 0 : 1;
			}

			//logerror("ACIA6850 #%u: TX PARITY BIT %x\n", which, *acia_p->tx_pin);
			acia_p->tx_parity = 0;
			acia_p->tx_state = STOP;
			break;
		}
		case STOP:
		{
			//logerror("ACIA6850 #%u: TX STOP BIT\n", which);
			*acia_p->tx_pin = 1;

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
			*acia_p->tx_pin = 1;
			acia_p->tx_state = START;
			break;
		}
	}
}

/* Called on receive timer event */
static TIMER_CALLBACK( receive_event )
{
	int which = param;
	acia_6850 *acia_p = &acia[which];

	if (acia_p->dcd_pin && *acia_p->dcd_pin)
	{
		acia_p->status |= ACIA6850_STATUS_DCD;
		acia6850_check_interrupts(which);
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
		switch (acia_p->rx_state)
		{
			case START:
			{
				if (*acia_p->rx_pin == 0)
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
				//logerror("ACIA6850 #%u: RX DATA BIT %x\n", which, *acia_p->rx_pin);
				acia_p->rx_shift |= *acia_p->rx_pin ? 0x80 : 0;
				acia_p->rx_parity ^= *acia_p->rx_pin;

				if (--acia_p->rx_bits == 0)
				{
					if (acia_p->status & ACIA6850_STATUS_RDRF)
					{
						acia_p->overrun = 1;
						acia6850_check_interrupts(which);
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
				//logerror("ACIA6850 #%u: RX PARITY BIT %x\n", which, *acia_p->rx_pin);
				acia_p->rx_parity ^= *acia_p->rx_pin;

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
				if (*acia_p->rx_pin == 1)
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
							acia6850_check_interrupts(which);
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
				if (*acia_p->rx_pin == 1)
				{
					//logerror("ACIA6850 #%u: RX STOP BIT\n", which);
					acia_p->status &= ~ACIA6850_STATUS_FE;

					if (!(acia_p->status & ACIA6850_STATUS_RDRF))
					{
						//logerror("ACIA6850 #%u: RX DATA %x\n", which, acia_p->rx_shift);
						acia_p->rdr = acia_p->rx_shift;
						acia_p->status |= ACIA6850_STATUS_RDRF;
						acia6850_check_interrupts(which);
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

void acia6850_set_rx_clock(int which, int clock)
{
	acia_6850 *acia_p = &acia[which];
	acia_p->rx_clock = clock;
}

void acia6850_set_tx_clock(int which, int clock)
{
	acia_6850 *acia_p = &acia[which];
	acia_p->tx_clock = clock;
}


WRITE8_HANDLER( acia6850_0_ctrl_w ) { acia6850_ctrl_w(0, data); }
WRITE8_HANDLER( acia6850_1_ctrl_w ) { acia6850_ctrl_w(1, data); }
WRITE8_HANDLER( acia6850_2_ctrl_w ) { acia6850_ctrl_w(2, data); }
WRITE8_HANDLER( acia6850_3_ctrl_w ) { acia6850_ctrl_w(3, data); }

WRITE8_HANDLER( acia6850_0_data_w ) { acia6850_data_w(0, data); }
WRITE8_HANDLER( acia6850_1_data_w ) { acia6850_data_w(1, data); }
WRITE8_HANDLER( acia6850_2_data_w ) { acia6850_data_w(2, data); }
WRITE8_HANDLER( acia6850_3_data_w ) { acia6850_data_w(3, data); }

READ8_HANDLER( acia6850_0_stat_r ) { return acia6850_stat_r(0); }
READ8_HANDLER( acia6850_1_stat_r ) { return acia6850_stat_r(1); }
READ8_HANDLER( acia6850_2_stat_r ) { return acia6850_stat_r(2); }
READ8_HANDLER( acia6850_3_stat_r ) { return acia6850_stat_r(3); }

READ8_HANDLER( acia6850_0_data_r ) { return acia6850_data_r(0); }
READ8_HANDLER( acia6850_1_data_r ) { return acia6850_data_r(1); }
READ8_HANDLER( acia6850_2_data_r ) { return acia6850_data_r(2); }
READ8_HANDLER( acia6850_3_data_r ) { return acia6850_data_r(3); }

READ16_HANDLER( acia6850_0_stat_lsb_r ) { return acia6850_stat_r(0); }
READ16_HANDLER( acia6850_1_stat_lsb_r ) { return acia6850_stat_r(1); }
READ16_HANDLER( acia6850_2_stat_lsb_r ) { return acia6850_stat_r(2); }
READ16_HANDLER( acia6850_3_stat_lsb_r ) { return acia6850_stat_r(3); }

READ16_HANDLER( acia6850_0_stat_msb_r ) { return acia6850_stat_r(0) << 8; }
READ16_HANDLER( acia6850_1_stat_msb_r ) { return acia6850_stat_r(1) << 8; }
READ16_HANDLER( acia6850_2_stat_msb_r ) { return acia6850_stat_r(2) << 8; }
READ16_HANDLER( acia6850_3_stat_msb_r ) { return acia6850_stat_r(3) << 8; }

READ16_HANDLER( acia6850_0_data_lsb_r ) { return acia6850_data_r(0); }
READ16_HANDLER( acia6850_1_data_lsb_r ) { return acia6850_data_r(1); }
READ16_HANDLER( acia6850_2_data_lsb_r ) { return acia6850_data_r(2); }
READ16_HANDLER( acia6850_3_data_lsb_r ) { return acia6850_data_r(3); }

READ16_HANDLER( acia6850_0_data_msb_r ) { return acia6850_data_r(0) << 8; }
READ16_HANDLER( acia6850_1_data_msb_r ) { return acia6850_data_r(1) << 8; }
READ16_HANDLER( acia6850_2_data_msb_r ) { return acia6850_data_r(2) << 8; }
READ16_HANDLER( acia6850_3_data_msb_r ) { return acia6850_data_r(3) << 8; }

WRITE16_HANDLER( acia6850_0_ctrl_msb_w ) { if (ACCESSING_MSB) acia6850_ctrl_w(0, (data >> 8) & 0xff); }
WRITE16_HANDLER( acia6850_1_ctrl_msb_w ) { if (ACCESSING_MSB) acia6850_ctrl_w(1, (data >> 8) & 0xff); }
WRITE16_HANDLER( acia6850_2_ctrl_msb_w ) { if (ACCESSING_MSB) acia6850_ctrl_w(2, (data >> 8) & 0xff); }
WRITE16_HANDLER( acia6850_3_ctrl_msb_w ) { if (ACCESSING_MSB) acia6850_ctrl_w(3, (data >> 8) & 0xff); }

WRITE16_HANDLER( acia6850_0_ctrl_lsb_w ) { if (ACCESSING_LSB) acia6850_ctrl_w(0, data & 0xff); }
WRITE16_HANDLER( acia6850_1_ctrl_lsb_w ) { if (ACCESSING_LSB) acia6850_ctrl_w(1, data & 0xff); }
WRITE16_HANDLER( acia6850_2_ctrl_lsb_w ) { if (ACCESSING_LSB) acia6850_ctrl_w(2, data & 0xff); }
WRITE16_HANDLER( acia6850_3_ctrl_lsb_w ) { if (ACCESSING_LSB) acia6850_ctrl_w(3, data & 0xff); }

WRITE16_HANDLER( acia6850_0_data_msb_w ) { if (ACCESSING_MSB) acia6850_data_w(0, (data >> 8) & 0xff); }
WRITE16_HANDLER( acia6850_1_data_msb_w ) { if (ACCESSING_MSB) acia6850_data_w(1, (data >> 8) & 0xff); }
WRITE16_HANDLER( acia6850_2_data_msb_w ) { if (ACCESSING_MSB) acia6850_data_w(2, (data >> 8) & 0xff); }
WRITE16_HANDLER( acia6850_3_data_msb_w ) { if (ACCESSING_MSB) acia6850_data_w(3, (data >> 8) & 0xff); }

WRITE16_HANDLER( acia6850_0_data_lsb_w ) { if (ACCESSING_LSB) acia6850_data_w(0, data & 0xff); }
WRITE16_HANDLER( acia6850_1_data_lsb_w ) { if (ACCESSING_LSB) acia6850_data_w(1, data & 0xff); }
WRITE16_HANDLER( acia6850_2_data_lsb_w ) { if (ACCESSING_LSB) acia6850_data_w(2, data & 0xff); }
WRITE16_HANDLER( acia6850_3_data_lsb_w ) { if (ACCESSING_LSB) acia6850_data_w(3, data & 0xff); }
