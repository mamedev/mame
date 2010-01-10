/***************************************************************************

    Z80 CTC (Z8430) implementation

    based on original version (c) 1997, Tatsuyuki Satoh

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "z80ctc.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE		0

#define VPRINTF(x) do { if (VERBOSE) logerror x; } while (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* these are the bits of the incoming commands to the CTC */
#define INTERRUPT			0x80
#define INTERRUPT_ON		0x80
#define INTERRUPT_OFF		0x00

#define MODE				0x40
#define MODE_TIMER			0x00
#define MODE_COUNTER		0x40

#define PRESCALER			0x20
#define PRESCALER_256		0x20
#define PRESCALER_16		0x00

#define EDGE				0x10
#define EDGE_FALLING		0x00
#define EDGE_RISING			0x10

#define TRIGGER				0x08
#define TRIGGER_AUTO		0x00
#define TRIGGER_CLOCK		0x08

#define CONSTANT			0x04
#define CONSTANT_LOAD		0x04
#define CONSTANT_NONE		0x00

#define RESET				0x02
#define RESET_CONTINUE		0x00
#define RESET_ACTIVE		0x02

#define CONTROL				0x01
#define CONTROL_VECTOR		0x00
#define CONTROL_WORD		0x01

/* these extra bits help us keep things accurate */
#define WAITING_FOR_TRIG	0x100



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _ctc_channel ctc_channel;
struct _ctc_channel
{
	devcb_resolved_write_line	zc;			/* zero crossing callbacks */

	UINT8				notimer;			/* no timer masks */
	UINT16				mode;				/* current mode */
	UINT16				tconst;				/* time constant */
	UINT16				down;				/* down counter (clock mode only) */
	UINT8				extclk;				/* current signal from the external clock */
	emu_timer *			timer;				/* array of active timers */
	UINT8				int_state;			/* interrupt status (for daisy chain) */
};


typedef struct _z80ctc z80ctc;
struct _z80ctc
{
	devcb_resolved_write_line intr;			/* interrupt callback */

	UINT8				vector;				/* interrupt vector */
	attotime			period16;			/* 16/system clock */
	attotime			period256;			/* 256/system clock */
	ctc_channel			channel[4];			/* data for each channel */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static int z80ctc_irq_state(const device_config *device);
static int z80ctc_irq_ack(const device_config *device);
static void z80ctc_irq_reti(const device_config *device);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE z80ctc *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == Z80CTC);
	return (z80ctc *)device->token;
}



/***************************************************************************
    INTERNAL STATE MANAGEMENT
***************************************************************************/

static void interrupt_check(const device_config *device)
{
	z80ctc *ctc = get_safe_token(device);
	int state = (z80ctc_irq_state(device) & Z80_DAISY_INT) ? ASSERT_LINE : CLEAR_LINE;

	devcb_call_write_line(&ctc->intr, state);
}

static TIMER_CALLBACK( timercallback )
{
	const device_config *device = (const device_config *)ptr;
	z80ctc *ctc = get_safe_token(device);
	ctc_channel *channel = &ctc->channel[param];

	/* down counter has reached zero - see if we should interrupt */
	if ((channel->mode & INTERRUPT) == INTERRUPT_ON)
	{
		channel->int_state |= Z80_DAISY_INT;
		VPRINTF(("CTC timer ch%d\n", param));
		interrupt_check(device);
	}

	/* generate the clock pulse */
	devcb_call_write_line(&channel->zc, 1);
	devcb_call_write_line(&channel->zc, 0);

	/* reset the down counter */
	channel->down = channel->tconst;
}



/***************************************************************************
    INITIALIZATION/CONFIGURATION
***************************************************************************/

attotime z80ctc_getperiod(const device_config *device, int ch)
{
	z80ctc *ctc = get_safe_token(device);
	ctc_channel *channel = &ctc->channel[ch];
	attotime period;

	/* if reset active, no period */
	if ((channel->mode & RESET) == RESET_ACTIVE)
		return attotime_zero;

	/* if counter mode, no real period */
	if ((channel->mode & MODE) == MODE_COUNTER)
	{
		logerror("CTC %d is CounterMode : Can't calculate period\n", ch );
		return attotime_zero;
	}

	/* compute the period */
	period = ((channel->mode & PRESCALER) == PRESCALER_16) ? ctc->period16 : ctc->period256;
	return attotime_mul(period, channel->tconst);
}



/***************************************************************************
    WRITE HANDLERS
***************************************************************************/

WRITE8_DEVICE_HANDLER( z80ctc_w )
{
	z80ctc *ctc = get_safe_token(device);
	int ch = offset & 3;
	ctc_channel *channel = &ctc->channel[ch];
	int mode;

	/* get the current mode */
	mode = channel->mode;

	/* if we're waiting for a time constant, this is it */
	if ((mode & CONSTANT) == CONSTANT_LOAD)
	{
		VPRINTF(("CTC ch.%d constant = %02x\n", ch, data));

		/* set the time constant (0 -> 0x100) */
		channel->tconst = data ? data : 0x100;

		/* clear the internal mode -- we're no longer waiting */
		channel->mode &= ~CONSTANT;

		/* also clear the reset, since the constant gets it going again */
		channel->mode &= ~RESET;

		/* if we're in timer mode.... */
		if ((mode & MODE) == MODE_TIMER)
		{
			/* if we're triggering on the time constant, reset the down counter now */
			if ((mode & TRIGGER) == TRIGGER_AUTO)
			{
				if (!channel->notimer)
				{
					attotime period = ((mode & PRESCALER) == PRESCALER_16) ? ctc->period16 : ctc->period256;
					period = attotime_mul(period, channel->tconst);

					timer_adjust_periodic(channel->timer, period, ch, period);
				}
				else
					timer_adjust_oneshot(channel->timer, attotime_never, 0);
			}

			/* else set the bit indicating that we're waiting for the appropriate trigger */
			else
				channel->mode |= WAITING_FOR_TRIG;
		}

		/* also set the down counter in case we're clocking externally */
		channel->down = channel->tconst;

		/* all done here */
		return;
	}

	/* if we're writing the interrupt vector, handle it specially */
#if 0	/* Tatsuyuki Satoh changes */
	/* The 'Z80family handbook' wrote,                            */
	/* interrupt vector is able to set for even channel (0 or 2)  */
	if ((data & CONTROL) == CONTROL_VECTOR && (ch&1) == 0)
#else
	if ((data & CONTROL) == CONTROL_VECTOR && ch == 0)
#endif
	{
		ctc->vector = data & 0xf8;
		logerror("CTC Vector = %02x\n", ctc->vector);
		return;
	}

	/* this must be a control word */
	if ((data & CONTROL) == CONTROL_WORD)
	{
		/* set the new mode */
		channel->mode = data;
		VPRINTF(("CTC ch.%d mode = %02x\n", ch, data));

		/* if we're being reset, clear out any pending timers for this channel */
		if ((data & RESET) == RESET_ACTIVE)
		{
			timer_adjust_oneshot(channel->timer, attotime_never, 0);
			/* note that we don't clear the interrupt state here! */
		}

		/* all done here */
		return;
	}
}



/***************************************************************************
    READ HANDLERS
***************************************************************************/

READ8_DEVICE_HANDLER( z80ctc_r )
{
	z80ctc *ctc = get_safe_token(device);
	int ch = offset & 3;
	ctc_channel *channel = &ctc->channel[ch];

	/* if we're in counter mode, just return the count */
	if ((channel->mode & MODE) == MODE_COUNTER || (channel->mode & WAITING_FOR_TRIG))
		return channel->down;

	/* else compute the down counter value */
	else
	{
		attotime period = ((channel->mode & PRESCALER) == PRESCALER_16) ? ctc->period16 : ctc->period256;

		VPRINTF(("CTC clock %f\n",ATTOSECONDS_TO_HZ(period.attoseconds)));

		if (channel->timer != NULL)
			return ((int)(attotime_to_double(timer_timeleft(channel->timer)) * attotime_to_double(period)) + 1) & 0xff;
		else
			return 0;
	}
}



/***************************************************************************
    EXTERNAL TRIGGERS
***************************************************************************/

static void z80ctc_trg_w(const device_config *device, int ch, UINT8 data)
{
	z80ctc *ctc = get_safe_token(device);
	ctc_channel *channel = &ctc->channel[ch];

	/* normalize data */
	data = data ? 1 : 0;

	/* see if the trigger value has changed */
	if (data != channel->extclk)
	{
		channel->extclk = data;

		/* see if this is the active edge of the trigger */
		if (((channel->mode & EDGE) == EDGE_RISING && data) || ((channel->mode & EDGE) == EDGE_FALLING && !data))
		{
			/* if we're waiting for a trigger, start the timer */
			if ((channel->mode & WAITING_FOR_TRIG) && (channel->mode & MODE) == MODE_TIMER)
			{
				if (!channel->notimer)
				{
					attotime period = ((channel->mode & PRESCALER) == PRESCALER_16) ? ctc->period16 : ctc->period256;
					period = attotime_mul(period, channel->tconst);

					VPRINTF(("CTC period %s\n", attotime_string(period, 9)));
					timer_adjust_periodic(channel->timer, period, ch, period);
				}
				else
				{
					VPRINTF(("CTC disabled\n"));

					timer_adjust_oneshot(channel->timer, attotime_never, 0);
				}
			}

			/* we're no longer waiting */
			channel->mode &= ~WAITING_FOR_TRIG;

			/* if we're clocking externally, decrement the count */
			if ((channel->mode & MODE) == MODE_COUNTER)
			{
				channel->down--;

				/* if we hit zero, do the same thing as for a timer interrupt */
				if (!channel->down)
				{
					void *ptr = (void *)device;
					timercallback(device->machine, ptr, ch);
				}
			}
		}
	}
}
WRITE_LINE_DEVICE_HANDLER( z80ctc_trg0_w ) { z80ctc_trg_w(device, 0, state); }
WRITE_LINE_DEVICE_HANDLER( z80ctc_trg1_w ) { z80ctc_trg_w(device, 1, state); }
WRITE_LINE_DEVICE_HANDLER( z80ctc_trg2_w ) { z80ctc_trg_w(device, 2, state); }
WRITE_LINE_DEVICE_HANDLER( z80ctc_trg3_w ) { z80ctc_trg_w(device, 3, state); }



/***************************************************************************
    DAISY CHAIN INTERFACE
***************************************************************************/

static int z80ctc_irq_state(const device_config *device)
{
	z80ctc *ctc = get_safe_token(device);
	int state = 0;
	int ch;

	VPRINTF(("CTC IRQ state = %d%d%d%d\n", ctc->channel[0].int_state, ctc->channel[1].int_state, ctc->channel[2].int_state, ctc->channel[3].int_state));

	/* loop over all channels */
	for (ch = 0; ch < 4; ch++)
	{
		ctc_channel *channel = &ctc->channel[ch];

		/* if we're servicing a request, don't indicate more interrupts */
		if (channel->int_state & Z80_DAISY_IEO)
		{
			state |= Z80_DAISY_IEO;
			break;
		}
		state |= channel->int_state;
	}

	return state;
}


static int z80ctc_irq_ack(const device_config *device)
{
	z80ctc *ctc = get_safe_token(device);
	int ch;

	/* loop over all channels */
	for (ch = 0; ch < 4; ch++)
	{
		ctc_channel *channel = &ctc->channel[ch];

		/* find the first channel with an interrupt requested */
		if (channel->int_state & Z80_DAISY_INT)
		{
			VPRINTF(("CTC IRQAck ch%d\n", ch));

			/* clear interrupt, switch to the IEO state, and update the IRQs */
			channel->int_state = Z80_DAISY_IEO;
			interrupt_check(device);
			return ctc->vector + ch * 2;
		}
	}

	logerror("z80ctc_irq_ack: failed to find an interrupt to ack!\n");
	return ctc->vector;
}


static void z80ctc_irq_reti(const device_config *device)
{
	z80ctc *ctc = get_safe_token(device);
	int ch;

	/* loop over all channels */
	for (ch = 0; ch < 4; ch++)
	{
		ctc_channel *channel = &ctc->channel[ch];

		/* find the first channel with an IEO pending */
		if (channel->int_state & Z80_DAISY_IEO)
		{
			VPRINTF(("CTC IRQReti ch%d\n", ch));

			/* clear the IEO state and update the IRQs */
			channel->int_state &= ~Z80_DAISY_IEO;
			interrupt_check(device);
			return;
		}
	}

	logerror("z80ctc_irq_reti: failed to find an interrupt to clear IEO on!\n");
}

static DEVICE_START( z80ctc )
{
	const z80ctc_interface *intf = (const z80ctc_interface *)device->static_config;
	z80ctc *ctc = get_safe_token(device);
	astring tempstring;
	int ch;

	ctc->period16 = attotime_mul(ATTOTIME_IN_HZ(device->clock), 16);
	ctc->period256 = attotime_mul(ATTOTIME_IN_HZ(device->clock), 256);
	for (ch = 0; ch < 4; ch++)
	{
		ctc_channel *channel = &ctc->channel[ch];
		void *ptr = (void *)device;
		channel->notimer = (intf->notimer >> ch) & 1;
		channel->timer = timer_alloc(device->machine, timercallback, ptr);
	}

	/* resolve callbacks */
	devcb_resolve_write_line(&ctc->intr, &intf->intr, device);
	devcb_resolve_write_line(&ctc->channel[0].zc, &intf->zc0, device);
	devcb_resolve_write_line(&ctc->channel[1].zc, &intf->zc1, device);
	devcb_resolve_write_line(&ctc->channel[2].zc, &intf->zc2, device);

	/* register for save states */
    state_save_register_device_item(device, 0, ctc->vector);
    for (ch = 0; ch < 4; ch++)
    {
		ctc_channel *channel = &ctc->channel[ch];
	    state_save_register_device_item(device, ch, channel->mode);
	    state_save_register_device_item(device, ch, channel->tconst);
	    state_save_register_device_item(device, ch, channel->down);
	    state_save_register_device_item(device, ch, channel->extclk);
	    state_save_register_device_item(device, ch, channel->int_state);
	}
}


static DEVICE_RESET( z80ctc )
{
	z80ctc *ctc = get_safe_token(device);
	int ch;

	/* set up defaults */
	for (ch = 0; ch < 4; ch++)
	{
		ctc_channel *channel = &ctc->channel[ch];
		channel->mode = RESET_ACTIVE;
		channel->tconst = 0x100;
		timer_adjust_oneshot(channel->timer, attotime_never, 0);
		channel->int_state = 0;
	}
	interrupt_check(device);
	VPRINTF(("CTC Reset\n"));
}


DEVICE_GET_INFO( z80ctc )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(z80ctc);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;							break;
		case DEVINFO_INT_CLASS:							info->i = DEVICE_CLASS_PERIPHERAL;		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(z80ctc);break;
		case DEVINFO_FCT_STOP:							/* Nothing */							break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(z80ctc);break;
		case DEVINFO_FCT_IRQ_STATE:						info->f = (genf *)z80ctc_irq_state;		break;
		case DEVINFO_FCT_IRQ_ACK:						info->f = (genf *)z80ctc_irq_ack;		break;
		case DEVINFO_FCT_IRQ_RETI:						info->f = (genf *)z80ctc_irq_reti;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Zilog Z80 CTC");		break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Z80");					break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

