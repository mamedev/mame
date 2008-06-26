/***************************************************************************

    Z80 CTC (Z8430) implementation

    based on original version (c) 1997, Tatsuyuki Satoh

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
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
#define INTERRUPT_ON 		0x80
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

typedef struct _z80ctc z80ctc;
struct _z80ctc
{
	UINT8 vector;				/* interrupt vector */
	UINT32 clock;				/* system clock */
	attotime period16;			/* 16/system clock */
	attotime period256;		/* 256/system clock */
	void (*intr)(running_machine *machine, int which);	/* interrupt callback */
	write8_machine_func zc[4];		/* zero crossing callbacks */
	UINT8 notimer;				/* no timer masks */
	UINT16 mode[4];				/* current mode */
	UINT16 tconst[4];			/* time constant */
	UINT16 down[4];				/* down counter (clock mode only) */
	UINT8 extclk[4];			/* current signal from the external clock */
	emu_timer *timer[4];				/* array of active timers */
	UINT8 int_state[4];			/* interrupt status (for daisy chain) */
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static z80ctc ctcs[MAX_CTC];



/***************************************************************************
    INTERNAL STATE MANAGEMENT
***************************************************************************/

static void interrupt_check(running_machine *machine, int which)
{
	z80ctc *ctc = ctcs + which;

	/* if we have a callback, update it with the current state */
	if (ctc->intr)
		(*ctc->intr)(machine, (z80ctc_irq_state(which) & Z80_DAISY_INT) ? ASSERT_LINE : CLEAR_LINE);
}


static TIMER_CALLBACK( timercallback )
{
	int which = param >> 2;
	int ch = param & 3;
	z80ctc *ctc = ctcs + which;

	/* down counter has reached zero - see if we should interrupt */
	if ((ctc->mode[ch] & INTERRUPT) == INTERRUPT_ON)
	{
		ctc->int_state[ch] |= Z80_DAISY_INT;
		VPRINTF(("CTC timer ch%d\n", ch));
		interrupt_check(machine, which);
	}

	/* generate the clock pulse */
	if (ctc->zc[ch])
	{
		(*ctc->zc[ch])(machine,0,1);
		(*ctc->zc[ch])(machine,0,0);
	}

	/* reset the down counter */
	ctc->down[ch] = ctc->tconst[ch];
}



/***************************************************************************
    INITIALIZATION/CONFIGURATION
***************************************************************************/

void z80ctc_init(int which, z80ctc_interface *intf)
{
	z80ctc *ctc = &ctcs[which];

	assert(which < MAX_CTC);

	memset(ctc, 0, sizeof(*ctc));

	ctc->clock = intf->baseclock;
	ctc->period16 = attotime_mul(ATTOTIME_IN_HZ(intf->baseclock), 16);
	ctc->period256 = attotime_mul(ATTOTIME_IN_HZ(intf->baseclock), 256);
	ctc->notimer = intf->notimer;
	ctc->intr = intf->intr;
	ctc->timer[0] = timer_alloc(timercallback, NULL);
	ctc->timer[1] = timer_alloc(timercallback, NULL);
	ctc->timer[2] = timer_alloc(timercallback, NULL);
	ctc->timer[3] = timer_alloc(timercallback, NULL);
	ctc->zc[0] = intf->zc0;
	ctc->zc[1] = intf->zc1;
	ctc->zc[2] = intf->zc2;
	ctc->zc[3] = 0;
	z80ctc_reset(which);

    state_save_register_item("z80ctc", which, ctc->vector);
    state_save_register_item_array("z80ctc", which, ctc->mode);
    state_save_register_item_array("z80ctc", which, ctc->tconst);
    state_save_register_item_array("z80ctc", which, ctc->down);
    state_save_register_item_array("z80ctc", which, ctc->extclk);
    state_save_register_item_array("z80ctc", which, ctc->int_state);
}


void z80ctc_reset(int which)
{
	z80ctc *ctc = ctcs + which;
	int i;

	/* set up defaults */
	for (i = 0; i < 4; i++)
	{
		ctc->mode[i] = RESET_ACTIVE;
		ctc->tconst[i] = 0x100;
		timer_adjust_oneshot(ctc->timer[i], attotime_never, 0);
		ctc->int_state[i] = 0;
	}
	interrupt_check(Machine, which);
	VPRINTF(("CTC Reset\n"));
}


attotime z80ctc_getperiod(int which, int ch)
{
	z80ctc *ctc = ctcs + which;
	attotime period;

	/* if reset active, no period */
	if ((ctc->mode[ch] & RESET) == RESET_ACTIVE)
		return attotime_zero;

	/* if counter mode, no real period */
	if ((ctc->mode[ch] & MODE) == MODE_COUNTER)
	{
		logerror("CTC %d is CounterMode : Can't calculate period\n", ch );
		return attotime_zero;
	}

	/* compute the period */
	period = ((ctc->mode[ch] & PRESCALER) == PRESCALER_16) ? ctc->period16 : ctc->period256;
	return attotime_mul(period, ctc->tconst[ch]);
}



/***************************************************************************
    WRITE HANDLERS
***************************************************************************/

void z80ctc_w(int which, int ch, UINT8 data)
{
	z80ctc *ctc = ctcs + which;
	int mode;

	/* get the current mode */
	mode = ctc->mode[ch];

	/* if we're waiting for a time constant, this is it */
	if ((mode & CONSTANT) == CONSTANT_LOAD)
	{
		VPRINTF(("CTC ch.%d constant = %02x\n", ch, data));

		/* set the time constant (0 -> 0x100) */
		ctc->tconst[ch] = data ? data : 0x100;

		/* clear the internal mode -- we're no longer waiting */
		ctc->mode[ch] &= ~CONSTANT;

		/* also clear the reset, since the constant gets it going again */
		ctc->mode[ch] &= ~RESET;

		/* if we're in timer mode.... */
		if ((mode & MODE) == MODE_TIMER)
		{
			/* if we're triggering on the time constant, reset the down counter now */
			if ((mode & TRIGGER) == TRIGGER_AUTO)
			{
				if (!(ctc->notimer & (1<<ch)))
				{
					attotime period = ((mode & PRESCALER) == PRESCALER_16) ? ctc->period16 : ctc->period256;
					period = attotime_mul(period, ctc->tconst[ch]);

					timer_adjust_periodic(ctc->timer[ch], period, (which << 2) + ch, period);
				}
				else
					timer_adjust_oneshot(ctc->timer[ch], attotime_never, 0);
			}

			/* else set the bit indicating that we're waiting for the appropriate trigger */
			else
				ctc->mode[ch] |= WAITING_FOR_TRIG;
		}

		/* also set the down counter in case we're clocking externally */
		ctc->down[ch] = ctc->tconst[ch];

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
		ctc->mode[ch] = data;
		VPRINTF(("CTC ch.%d mode = %02x\n", ch, data));

		/* if we're being reset, clear out any pending timers for this channel */
		if ((data & RESET) == RESET_ACTIVE)
		{
			timer_adjust_oneshot(ctc->timer[ch], attotime_never, 0);
			/* note that we don't clear the interrupt state here! */
		}

		/* all done here */
		return;
	}
}

WRITE8_HANDLER( z80ctc_0_w ) { z80ctc_w(0, offset, data); }
WRITE8_HANDLER( z80ctc_1_w ) { z80ctc_w(1, offset, data); }



/***************************************************************************
    READ HANDLERS
***************************************************************************/

UINT8 z80ctc_r(int which, int ch)
{
	z80ctc *ctc = ctcs + which;

	/* if we're in counter mode, just return the count */
	if ((ctc->mode[ch] & MODE) == MODE_COUNTER || (ctc->mode[ch] & WAITING_FOR_TRIG))
		return ctc->down[ch];

	/* else compute the down counter value */
	else
	{
		attotime period = ((ctc->mode[ch] & PRESCALER) == PRESCALER_16) ? ctc->period16 : ctc->period256;

		VPRINTF(("CTC clock %f\n",ATTOSECONDS_TO_HZ(period.attoseconds)));

		if (ctc->timer[ch])
			return ((int)(attotime_to_double(timer_timeleft(ctc->timer[ch])) * attotime_to_double(period)) + 1) & 0xff;
		else
			return 0;
	}
}

READ8_HANDLER( z80ctc_0_r ) { return z80ctc_r(0, offset); }
READ8_HANDLER( z80ctc_1_r ) { return z80ctc_r(1, offset); }



/***************************************************************************
    EXTERNAL TRIGGERS
***************************************************************************/

void z80ctc_trg_w(running_machine *machine, int which, int ch, UINT8 data)
{
	z80ctc *ctc = ctcs + which;

	/* normalize data */
	data = data ? 1 : 0;

	/* see if the trigger value has changed */
	if (data != ctc->extclk[ch])
	{
		ctc->extclk[ch] = data;

		/* see if this is the active edge of the trigger */
		if (((ctc->mode[ch] & EDGE) == EDGE_RISING && data) || ((ctc->mode[ch] & EDGE) == EDGE_FALLING && !data))
		{
			/* if we're waiting for a trigger, start the timer */
			if ((ctc->mode[ch] & WAITING_FOR_TRIG) && (ctc->mode[ch] & MODE) == MODE_TIMER)
			{
				if (!(ctc->notimer & (1<<ch)))
				{
					attotime period = ((ctc->mode[ch] & PRESCALER) == PRESCALER_16) ? ctc->period16 : ctc->period256;
					period = attotime_mul(period, ctc->tconst[ch]);

					VPRINTF(("CTC period %s\n", attotime_string(period, 9)));
					timer_adjust_periodic(ctc->timer[ch], period, (which << 2) + ch, period);
				}
				else
				{
					VPRINTF(("CTC disabled\n"));

					timer_adjust_oneshot(ctc->timer[ch], attotime_never, 0);
				}
			}

			/* we're no longer waiting */
			ctc->mode[ch] &= ~WAITING_FOR_TRIG;

			/* if we're clocking externally, decrement the count */
			if ((ctc->mode[ch] & MODE) == MODE_COUNTER)
			{
				ctc->down[ch]--;

				/* if we hit zero, do the same thing as for a timer interrupt */
				if (!ctc->down[ch])
					timercallback(machine, NULL, (which << 2) + ch);
			}
		}
	}
}

WRITE8_HANDLER( z80ctc_0_trg0_w ) { z80ctc_trg_w(machine, 0, 0, data); }
WRITE8_HANDLER( z80ctc_0_trg1_w ) { z80ctc_trg_w(machine, 0, 1, data); }
WRITE8_HANDLER( z80ctc_0_trg2_w ) { z80ctc_trg_w(machine, 0, 2, data); }
WRITE8_HANDLER( z80ctc_0_trg3_w ) { z80ctc_trg_w(machine, 0, 3, data); }
WRITE8_HANDLER( z80ctc_1_trg0_w ) { z80ctc_trg_w(machine, 1, 0, data); }
WRITE8_HANDLER( z80ctc_1_trg1_w ) { z80ctc_trg_w(machine, 1, 1, data); }
WRITE8_HANDLER( z80ctc_1_trg2_w ) { z80ctc_trg_w(machine, 1, 2, data); }
WRITE8_HANDLER( z80ctc_1_trg3_w ) { z80ctc_trg_w(machine, 1, 3, data); }



/***************************************************************************
    DAISY CHAIN INTERFACE
***************************************************************************/

int z80ctc_irq_state(int which)
{
	z80ctc *ctc = ctcs + which;
	int state = 0;
	int ch;

	VPRINTF(("CTC IRQ state = %d%d%d%d\n", ctc->int_state[0], ctc->int_state[1], ctc->int_state[2], ctc->int_state[3]));

	/* loop over all channels */
	for (ch = 0; ch < 4; ch++)
	{
		/* if we're servicing a request, don't indicate more interrupts */
		if (ctc->int_state[ch] & Z80_DAISY_IEO)
		{
			state |= Z80_DAISY_IEO;
			break;
		}
		state |= ctc->int_state[ch];
	}

	return state;
}


int z80ctc_irq_ack(int which)
{
	z80ctc *ctc = ctcs + which;
	int ch;

	/* loop over all channels */
	for (ch = 0; ch < 4; ch++)

		/* find the first channel with an interrupt requested */
		if (ctc->int_state[ch] & Z80_DAISY_INT)
		{
			VPRINTF(("CTC IRQAck ch%d\n", ch));

			/* clear interrupt, switch to the IEO state, and update the IRQs */
			ctc->int_state[ch] = Z80_DAISY_IEO;
			interrupt_check(Machine, which);
			return ctc->vector + ch * 2;
		}

	logerror("z80ctc_irq_ack: failed to find an interrupt to ack!\n");
	return ctc->vector;
}


void z80ctc_irq_reti(int which)
{
	z80ctc *ctc = ctcs + which;
	int ch;

	/* loop over all channels */
	for (ch = 0; ch < 4; ch++)

		/* find the first channel with an IEO pending */
		if (ctc->int_state[ch] & Z80_DAISY_IEO)
		{
			VPRINTF(("CTC IRQReti ch%d\n", ch));

			/* clear the IEO state and update the IRQs */
			ctc->int_state[ch] &= ~Z80_DAISY_IEO;
			interrupt_check(Machine, which);
			return;
		}

	logerror("z80ctc_irq_reti: failed to find an interrupt to clear IEO on!\n");
}
