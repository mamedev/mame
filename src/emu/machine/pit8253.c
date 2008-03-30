/*****************************************************************************
 *
 *  Programmable Interval Timer 8253/8254
 *
 *  Three Independent Timers
 *  (gate, clock, out pins)
 *
 *  8254 has an additional readback feature
 *
 *  Revision History
 *      8-Jul-2004 - AJ:    Fixed some bugs. Styx now runs correctly.
 *                          Implemented 8254 features.
 *      1-Mar-2004 - NPW:   Did an almost total rewrite and cleaned out much
 *                          of the ugliness in the previous design.  Bug #430
 *                          seems to be fixed
 *      1-Jul-2000 - PeT:   Split off from PC driver and componentized
 *
 *****************************************************************************/

#include <math.h>
#include "driver.h"
#include "memconv.h"
#include "machine/pit8253.h"



/***************************************************************************

    Structures & macros

***************************************************************************/

#define	MAX_TIMER		3
#define	VERBOSE			0

#define	LOG1(msg)		do { if (VERBOSE >= 1) logerror msg; } while (0)
#define	LOG2(msg)		do { if (VERBOSE >= 2) logerror msg; } while (0)


#define	TIMER_TIME_NEVER ((UINT64) -1)

#define	CYCLES_NEVER ((UINT32) -1)

struct pit8253_timer
{
	double clockin;					/* input clock frequency in Hz */

	void (*output_callback_func)(int);	/* callback function for when output changes */
	void (*freq_callback)(double);	/* callback function for when output frequency changes */

	attotime last_updated;			/* time when last updated */

	emu_timer *outputtimer;		/* MAME timer for output change callback */
	emu_timer *freqtimer;			/* MAME timer for output frequency change callback */

	UINT16 value;					/* current counter value ("CE" in Intel docs) */
	UINT16 latch;					/* latched counter value ("OL" in Intel docs) */
	UINT16 count;					/* new counter value ("CR" in Intel docs) */
	UINT8 control;					/* 6-bit control byte */
	UINT8 status;					/* status byte - 8254 only */
	UINT8 lowcount;					/* LSB of new counter value for 16-bit writes */
	INT32 rmsb;						/* 1 = Next read is MSB of 16-bit value */
	INT32 wmsb;						/* 1 = Next write is MSB of 16-bit value */
	INT32 output;						/* 0 = low, 1 = high */

	INT32 gate;						/* gate input (0 = low, 1 = high) */
	INT32 latched_count;				/* number of bytes of count latched */
	INT32 latched_status;				/* 1 = status latched (8254 only) */
	INT32 null_count;					/* 1 = mode control or count written, 0 = count loaded */
	INT32 phase;						/* see phase definition tables in simulate2(), below */

	UINT32 cycles_to_output;		/* cycles until output callback called */
	UINT32 cycles_to_freq;			/* cycles until frequency callback called */
	UINT32 freq_count;				/* counter period for periodic modes, 0 if counter non-periodic */
};

struct pit8253
{
	const struct pit8253_config *config;
	struct pit8253_timer timers[MAX_TIMER];
};

#define	CTRL_ACCESS(control)		(((control)	>> 4) &	0x03)
#define	CTRL_MODE(control)			(((control)	>> 1) &	(((control)	& 0x04)	? 0x03 : 0x07))
#define	CTRL_BCD(control)			(((control)	>> 0) &	0x01)


static int pit_count;
static struct pit8253 *pits;



/***************************************************************************

    Functions

***************************************************************************/

static struct pit8253 *get_pit(int which)
{
	return &pits[which];
}


static struct pit8253_timer	*get_timer(struct pit8253 *pit,int which)
{
	which &= 3;
	if (which < MAX_TIMER)
		return &pit->timers[which];
	return NULL;
}


static UINT32 decimal_from_bcd(UINT16 val)
{
	/* In BCD mode, a nybble loaded with value A-F counts down the same as in
       binary mode, but wraps around to 9 instead of F after 0, so loading the
       count register with 0xFFFF gives a period of
              0xF  - for the units to count down to 0
       +   10*0xF  - for the tens to count down to 0
       +  100*0xF  - for the hundreds to count down to 0
       + 1000*0xF  - for the thousands to count down to 0
       = 16665 cycles
    */
	return
		((val>>12) & 0xF) *	 1000 +
		((val>>	8) & 0xF) *	  100 +
		((val>>	4) & 0xF) *	   10 +
		( val	   & 0xF);
}


static UINT32 adjusted_count(int bcd,UINT16	val)
{
	if (bcd	== 0)
		return val == 0	? 0x10000 :	val;
	return val == 0	? 10000	: decimal_from_bcd(val);
}


/* This function subtracts 1 from timer->value "cycles" times, taking into
   account binary or BCD operation, and wrapping around from 0 to 0xFFFF or
   0x9999 as necessary. */
static void	decrease_counter_value(struct pit8253_timer	*timer,UINT64 cycles)
{
	UINT16 value;
	int units, tens, hundreds, thousands;

	if (CTRL_BCD(timer->control) ==	0)
	{
		timer->value -=	(cycles	& 0xFFFF);
		return;
	}

	value = timer->value;
	units	  =	 value		  &	0xF;
	tens	  =	(value >>  4) &	0xF;
	hundreds  =	(value >>  8) &	0xF;
	thousands =	(value >> 12) &	0xF;

	if (cycles <= units)
	{
		units -= cycles;
	}
	else
	{
		cycles -= units;
		units =	(10	- cycles%10)%10;

		cycles =(cycles+9)/10; /* the +9    is so we get a carry if cycles%10 wasn't 0 */
		if (cycles <= tens)
		{
			tens -=	cycles;
		}
		else
		{
			cycles -= tens;
			tens = (10 - cycles%10) % 10;

			cycles = (cycles+9) / 10;
			if (cycles <= hundreds)
			{
				hundreds -=	cycles;
			}
			else
			{
				cycles -= hundreds;
				hundreds = (10 - cycles%10)%10;
				cycles=(cycles+9)/10;
				thousands =	(10	+ thousands	- cycles%10)%10;
			}
		}
	}

	timer->value = (thousands << 12) | (hundreds <<	8) | (tens << 4) | units;
}


static double get_frequency(struct pit8253_timer *timer)
{
	LOG2(("pit8253: get_frequency() : %lf\n",(double)(timer->freq_count == 0 ? 0 : timer->clockin / timer->freq_count)));
	return timer->freq_count ==	0 ?	0 :	timer->clockin / timer->freq_count;
}


/* Call the frequency callback in "cycles" cycles */
static void	freq_callback_in(struct	pit8253_timer *timer,UINT32	cycles)
{
	LOG2(("pit8253: freq_callback_in(): %d cycles\n",cycles));

	if (timer->freq_callback ==	NULL)
	{
		return;
	}

	if (timer->clockin == 0	|| cycles == CYCLES_NEVER)
	{
		timer_reset(timer->freqtimer,attotime_never);
	}
	else
	{
		timer_reset(timer->freqtimer,double_to_attotime(cycles / timer->clockin));
	}
	timer->cycles_to_freq =	cycles;
}


static void	set_freq_count(struct pit8253_timer	*timer)
{
	int	mode = CTRL_MODE(timer->control);
	UINT32 freq_count;

	if ((mode == 2 || mode == 3) &&	timer->gate	!= 0 &&	timer->phase !=	0)
	{
		freq_count = adjusted_count(CTRL_BCD(timer->control),timer->count);
	}
	else
	{
		freq_count = 0;
	}

	if (freq_count != timer->freq_count)
	{
		timer->freq_count =	freq_count;
		if (timer->freq_callback !=	NULL)
		{
			timer->freq_callback(get_frequency(timer));
			freq_callback_in(timer,CYCLES_NEVER);
		}
	}

	LOG2(("pit8253: set_freq_count() : %d\n",freq_count));
}


/* Call the output callback in "cycles" cycles */
static void	trigger_countdown(struct pit8253_timer *timer)
{
	LOG2(("pit8253: trigger_countdown()\n"));

	timer->phase = 1;
	timer->value = timer->count;
	if (CTRL_MODE(timer->control) == 3 && timer->output	== 0)
		timer->value &=	0xfffe;

	set_freq_count(timer);
}


static void	set_output(struct pit8253_timer	*timer,int output)
{
	if (output != timer->output)
	{
		timer->output =	output;
		if (timer->output_callback_func != NULL)
		{
			timer->output_callback_func(output);
		}
	}
}


/* This emulates timer "timer" for "elapsed_cycles" cycles and assumes no
   callbacks occur during that time. */
static void	simulate2(struct pit8253_timer *timer,UINT64 elapsed_cycles)
{
	UINT32 adjusted_value;
	int	bcd	= CTRL_BCD(timer->control);
	int	mode = CTRL_MODE(timer->control);
	int	cycles_to_output = 0;

	if (timer->cycles_to_freq != CYCLES_NEVER)
	{
		timer->cycles_to_freq -= elapsed_cycles;
	}

	LOG2(("pit8253: simulate2(): simulating %d cycles in mode %d, bcd = %d, phase = %d, gate = %d, value = 0x%04x\n",
		  (int)elapsed_cycles,mode,bcd,timer->phase,timer->gate,timer->value));

	switch (mode) {
	case 0:
		/* Mode 0: (Interrupt on Terminal Count)

                  +------------------
                  |
        ----------+
          <- n+1 ->

          ^
          +- counter load

        phase|output|length  |value|next|comment
        -----+------+--------+-----+----+----------------------------------
            0|low   |infinity|     |1   |waiting for count
            1|low   |1       |     |2   |internal delay when counter loaded
            2|low   |n       |n..1 |3   |counting down
            3|high  |infinity|0..1 |3   |counting down

        Gate level sensitive only. Low disables counting, high enables it. */

		if (timer->gate	== 0 ||	timer->phase ==	0)
		{
			cycles_to_output = CYCLES_NEVER;
		}
		else
		{
			if (elapsed_cycles > 0 && timer->phase == 1)
			{
				--elapsed_cycles;
				timer->phase = 2;
			}

			if (timer->phase ==	2)
			{
				adjusted_value = adjusted_count(bcd,timer->value);
				if (elapsed_cycles < adjusted_value)
				{
					/* Counter didn't wrap */
					decrease_counter_value(timer,elapsed_cycles);
				}
				else
				{
					/* Counter wrapped, output goes high */
					elapsed_cycles -= adjusted_value;
					timer->phase = 3;
					timer->value = 0;
				}
			}

			if (timer->phase ==	3)
			{
				decrease_counter_value(timer,elapsed_cycles);
				cycles_to_output = CYCLES_NEVER;
			}
			else
			{
				cycles_to_output = adjusted_count(bcd,timer->value)	+ (timer->phase	== 1 ? 1 : 0);
			}
		}

		set_output(timer,timer->phase == 3 ? 1 : 0);
		break;


	case 1:
		/* Mode 1: (Hardware Retriggerable One-Shot a.k.a. Programmable One-Shot)

        --+       +------------------
          |       |
          +-------+
          <-  n  ->

          ^
          +- trigger

        phase|output|length  |value|next|comment
        -----+------+--------+-----+----+----------------------------------
            0|high  |infinity|0..1 |1   |counting down
            1|low   |n       |n..1 |0   |counting down

        Gate rising-edge sensitive only.
        Rising edge initiates counting and resets output after next clock. */

		adjusted_value = adjusted_count(bcd,timer->value);
		if (elapsed_cycles < adjusted_value)
		{
			/* Counter didn't wrap */
			decrease_counter_value(timer,elapsed_cycles);
			cycles_to_output = (timer->phase ==	0 ?	CYCLES_NEVER : adjusted_count(bcd,timer->value));
		}
		else
		{
			/* Counter wrapped, output goes high */
			elapsed_cycles -= adjusted_value;
			timer->phase = 0;
			timer->value = 0;
			decrease_counter_value(timer,elapsed_cycles);
			cycles_to_output = CYCLES_NEVER;
		}
		set_output(timer,timer->phase == 0 ? 1 : 0);
		break;


	case 2:
		/* Mode 2: (Rate Generator)

        --------------+ +---------+ +----
                      | |         | |
                      +-+         +-+
            <-    n    -X-    n    ->
                      <1>
            ^
            +- counter load or trigger

        phase|output|length  |value|next|comment
        -----+------+--------+-----+----+----------------------------------
            0|high  |infinity|     |1   |waiting for count
            1|v!=1  |n       |n..1 |1   |counting down

        Counter rewrite has no effect until repeated

        Gate rising-edge and level sensitive.
        Gate low disables counting and sets output immediately high.
        Rising-edge reloads count and initiates counting
        Gate high enables counting. */

		if (timer->gate	== 0 ||	timer->phase ==	0)
		{
			/* Gate low or mode control write forces output high */
			set_output(timer,1);
			cycles_to_output = CYCLES_NEVER;
		}
		else
		{
			adjusted_value = adjusted_count(bcd,timer->value);
			if (elapsed_cycles < adjusted_value)
			{
				/* Counter didn't wrap */
				decrease_counter_value(timer,elapsed_cycles);
			}
			else
			{
				/* Counter wrapped around one or more times */
				elapsed_cycles -= adjusted_value;
				trigger_countdown(timer);
				decrease_counter_value(timer,elapsed_cycles	% adjusted_count(bcd,timer->count));
			}
			cycles_to_output = (timer->value ==	1 ?	1 :	(adjusted_count(bcd,timer->value) -	1));

			set_output(timer,timer->value != 1 ? 1 : 0);
		}
		break;


	case 3:
		/* Mode 3: (Square Wave Generator)

        ----------------+           +-----------+           +----
                        |           |           |           |
                        +-----------+           +-----------+
            <- (n+1)/2 -X-   n/2   ->
            ^
            +- counter load or trigger

        phase|output|length  |value|next|comment
        -----+------+--------+-----+----+----------------------------------
            0|high  |infinity|     |1   |waiting for count
            1|      |infinity|n..0 |1   |counting down double speed

        Counter rewrite has no effect until repeated (output falling or rising)

        Gate rising-edge and level sensitive.
        Gate low disables counting and sets output immediately high.
        Rising-edge reloads count and initiates counting
        Gate high enables counting. */

		if (timer->gate	== 0 ||	timer->phase ==	0)
		{
			/* Gate low or mode control write forces output high */
			set_output(timer,1);
			cycles_to_output = CYCLES_NEVER;
		}
		else
		{
			adjusted_value = adjusted_count(bcd,timer->value);
			if ((elapsed_cycles<<1)	< adjusted_value)
			{
				/* Counter didn't wrap around */
				decrease_counter_value(timer,elapsed_cycles<<1);
			}
			else
			{
				/* Counter wrapped around one or more times */
				elapsed_cycles -= ((adjusted_value+1)>>1);

				set_output(timer,1 - timer->output);
				trigger_countdown(timer);

				elapsed_cycles %= adjusted_count(bcd,timer->count);
				adjusted_value = adjusted_count(bcd,timer->value);
				if ((elapsed_cycles<<1)	>= adjusted_value)
				{
					/* Counter wrapped around an even number of times */
					elapsed_cycles -= ((adjusted_value+1)>>1);

					set_output(timer,1 - timer->output);
					trigger_countdown(timer);
				}
				decrease_counter_value(timer,elapsed_cycles<<1);
			}
			cycles_to_output = (adjusted_count(bcd,timer->value) + 1) >> 1;
		}
		break;


	case 4:
	case 5:
		/* Mode 4: (Software Trigger Strobe)
           Mode 5: (Hardware Trigger Strobe)

        --------------+ +--------------------
                      | |
                      +-+
            <-  n+1  ->
            ^         <1>
            +- counter load (mode 4) or trigger (mode 5)

        phase|output|length  |value|next|comment
        -----+------+--------+-----+----+----------------------------------
            0|high  |infinity|0..1 |0   |waiting for count
            1|high  |1       |     |2   |internal delay when counter loaded
            2|high  |n       |n..1 |3   |counting down
            3|low   |1       |0    |0   |strobe

        Mode 4 only: counter rewrite loads new counter
        Mode 5 only: count not reloaded immediately.
        Mode control write doesn't stop count but sets output high

        Mode 4 only: Gate level sensitive only. Low disables counting, high enables it.
        Mode 5 only: Gate rising-edge sensitive only. Rising edge initiates counting */

		if (timer->gate	== 0 &&	mode ==	4)
		{
			cycles_to_output = CYCLES_NEVER;
		}
		else
		{
			if (elapsed_cycles > 0 && timer->phase == 1)
			{
				--elapsed_cycles;
				timer->phase = 2;
			}

			if (elapsed_cycles > 0 && timer->phase == 3)
			{
				--elapsed_cycles;
				timer->phase = 0;
				decrease_counter_value(timer,1);
			}

			if (timer->value ==	0 && timer->phase == 2)
				adjusted_value = 0;
			else
				adjusted_value = adjusted_count(bcd,timer->value);

			if (elapsed_cycles < adjusted_value)
			{
				/* Counter didn't wrap */
				decrease_counter_value(timer,elapsed_cycles);
			}
			else
			{
				elapsed_cycles -= adjusted_value;
				timer->value = 0;
				if (elapsed_cycles == 0)
				{
					/* We hit the strobe cycle */
					timer->phase = 3;
				}
				else
				{
					decrease_counter_value(timer,elapsed_cycles);
					timer->phase = 0;
				}
			}
			switch(timer->phase) {
			case 0:
				cycles_to_output = CYCLES_NEVER;
				break;
			case 1:
				cycles_to_output = adjusted_count(bcd,timer->value)	+ 1;
				break;
			case 2:
				cycles_to_output = adjusted_count(bcd,timer->value);
				break;
			case 3:
				cycles_to_output = 1;
				break;
			}
		}
		set_output(timer,timer->phase != 3 ? 1 : 0);
		break;
	}

	if (timer->output_callback_func != NULL)
	{
		timer->cycles_to_output	= cycles_to_output;
		if (cycles_to_output ==	CYCLES_NEVER ||	timer->clockin == 0)
		{
			timer_reset(timer->outputtimer,attotime_never);
		}
		else
		{
			timer_reset(timer->outputtimer,
				double_to_attotime(cycles_to_output / timer->clockin));
		}
	}

	if (timer->cycles_to_freq == 0)
		timer->cycles_to_freq =	CYCLES_NEVER;
}


/* This emulates timer "timer" for "elapsed_cycles" cycles, broken down into
   sections punctuated by callbacks.

   The loop technically should never execute even once. It's here to eliminate
   the following potential bug:

   1) The mame timer isn't perfectly accurate.
   2) The output callback is executed too late, after an update which
      brings the timer's local time past the callback time.
   3) A short pulse is skipped.
   4) That short pulse would have triggered an interrupt. The interrupt is
      skipped.

   This is a loop instead of an "if" statement in case the mame timer is
   inaccurate by more than one cycle, and the output changed multiple
   times during the discrepancy. In practice updates should still be O(1).
*/
static void	simulate(struct	pit8253_timer *timer,UINT64	elapsed_cycles)
{
	while ((timer->cycles_to_output	!= CYCLES_NEVER	&&
			timer->cycles_to_output	<= elapsed_cycles) ||
		   (timer->cycles_to_freq != CYCLES_NEVER &&
			timer->cycles_to_freq <= elapsed_cycles))
	{
		UINT32 cycles_to_callback;

		if (timer->cycles_to_output	< timer->cycles_to_freq	&&
			timer->cycles_to_output	!= CYCLES_NEVER)
		{
			cycles_to_callback = timer->cycles_to_output;
		}
		else
		{
			cycles_to_callback = timer->cycles_to_freq;
		}

		simulate2(timer,cycles_to_callback);
		elapsed_cycles -= cycles_to_callback;
	}
	simulate2(timer,elapsed_cycles);
}


/* This brings timer "timer" up to date */
static void	update(struct pit8253_timer	*timer)
{
	/* With the 82C54's maximum clockin of 10MHz, 64 bits is nearly 60,000
       years of time. Should be enough for now. */
	attotime now =	timer_get_time();
	attotime elapsed_time = attotime_sub(now,timer->last_updated);
	INT64 elapsed_cycles =	attotime_to_double(elapsed_time) *	timer->clockin;

	timer->last_updated	= attotime_add(timer->last_updated,double_to_attotime(elapsed_cycles/timer->clockin));

	simulate(timer,elapsed_cycles);
}


void pit8253_reset(int which)
{
	struct pit8253 *pit	= get_pit(which);
	struct pit8253_timer *timer;
	int	i;

	LOG1(("pit8253_reset(): resetting pit %d\n", which));

	for	(i = 0;	i <	MAX_TIMER; i++)
	{
		timer =	get_timer(pit,i);
		/* According to Intel's 8254 docs, the state of a timer is undefined
           until the first mode control word is written. Here we define this
           undefined behaviour */
		timer->control = timer->status = 0x30;
		timer->rmsb	= timer->wmsb =	0;
		timer->count = timer->value	= timer->latch = 0;
		timer->lowcount	= 0;
		timer->gate	= 1;
		timer->output =	0;
		timer->latched_count = 0;
		timer->latched_status =	0;
		timer->null_count =	1;
		timer->cycles_to_output	= timer->cycles_to_freq	= CYCLES_NEVER;

		timer->last_updated	= timer_get_time();

		update(timer);
	}
}


static TIMER_CALLBACK( freqcallback )
{
	struct pit8253_timer *timer = get_timer(get_pit(param &	0x0F),(param >>	4) & 0x0F);
	INT64 cycles =	timer->cycles_to_freq;
	double t;

	LOG2(("pit8253: freqcallback(): pit %d, timer %d, %d cycles\n",param & 0xf,(param >> 4) & 0xf,(UINT32)cycles));

	simulate(timer,cycles);

	t = cycles / timer->clockin;

	timer->last_updated	= attotime_add(timer->last_updated, double_to_attotime(t));
}


static TIMER_CALLBACK( outputcallback )
{
	struct pit8253_timer *timer = get_timer(get_pit(param &	0x0F),(param >>	4) & 0x0F);
	INT64 cycles =	timer->cycles_to_output;
	double t;

	LOG2(("pit8253: outputcallback(): pit %d, timer %d, %d cycles\n",param & 0xf,(param >> 4) & 0xf,(UINT32)cycles));

	simulate(timer,cycles);

	t = cycles / timer->clockin;

	timer->last_updated	= attotime_add(timer->last_updated, double_to_attotime(t));
}


int	pit8253_init(int count,	const struct pit8253_config *config)
{
	int	i, timerno,	n=0;
	struct pit8253 *pit;
	struct pit8253_timer *timer;

	LOG2(("pit8253_init(): initializing %d pit(s)\n", count));

	pit_count =	count;
	pits = auto_malloc(count * sizeof(struct pit8253));

	memset(pits, 0,	count *	sizeof(struct pit8253));

	for (i = 0;	i < count; i++)
	{
		pit	= get_pit(i);
		pit->config	= &config[i];

		for	(timerno = 0; timerno <	MAX_TIMER; timerno++)
		{
			timer =	get_timer(pit,timerno);

			timer->clockin = pit->config->timer[timerno].clockin;
			timer->output_callback_func = pit->config->timer[timerno].output_callback_func;
			timer->freq_callback = pit->config->timer[timerno].clock_callback;

			if (timer->output_callback_func == NULL)
				timer->outputtimer = NULL;
			else
			{
				timer->outputtimer = timer_alloc(outputcallback, NULL);
				timer_adjust_oneshot(timer->outputtimer, attotime_never, i	| (timerno<<4));
			}
			if (timer->freq_callback ==	NULL)
				timer->freqtimer = NULL;
			else
			{
				timer->freqtimer = timer_alloc(freqcallback, NULL);
				timer_adjust_oneshot(timer->freqtimer,	attotime_never,	i |	(timerno<<4));
			}

			/* set up state save values */
			state_save_register_item("pit8253", n, timer->clockin);
			state_save_register_item("pit8253", n, timer->control);
			state_save_register_item("pit8253", n, timer->status);
			state_save_register_item("pit8253", n, timer->lowcount);
			state_save_register_item("pit8253", n, timer->latch);
			state_save_register_item("pit8253", n, timer->count);
			state_save_register_item("pit8253", n, timer->value);
			state_save_register_item("pit8253", n, timer->wmsb);
			state_save_register_item("pit8253", n, timer->rmsb);
			state_save_register_item("pit8253", n, timer->output);
			state_save_register_item("pit8253", n, timer->gate);
			state_save_register_item("pit8253", n, timer->latched_count);
			state_save_register_item("pit8253", n, timer->latched_status);
			state_save_register_item("pit8253", n, timer->null_count);
			state_save_register_item("pit8253", n, timer->phase);
			state_save_register_item("pit8253", n, timer->cycles_to_output);
			state_save_register_item("pit8253", n, timer->cycles_to_freq);
			state_save_register_item("pit8253", n, timer->freq_count);
			state_save_register_item("pit8253", n, timer->last_updated.seconds);
			state_save_register_item("pit8253", n, timer->last_updated.attoseconds);
			++n;
		}
		pit8253_reset(i);
	}

	LOG1(("pit8253_init(): initialized successfully\n"));

	return 0;
}


/* We recycle bit 0 of timer->value to hold the phase in mode 3 when count is
   odd. Since read commands in mode 3 always return even numbers, we need to
   mask this bit off. */
static UINT16 masked_value(struct pit8253_timer	*timer)
{
	LOG2(("pit8253: masked_value\n"));

	if (CTRL_MODE(timer->control) == 3)
		return timer->value	& 0xfffe;
	return timer->value;
}

/* Reads only affect the following bits of the counter state:
     latched_status
     latched_count
     rmsb
  so they don't affect any timer operations except other reads. */
static UINT8 pit8253_read(int	which,offs_t offset)
{
	struct pit8253 *pit	= get_pit(which);
	struct pit8253_timer *timer	= get_timer(pit,offset);
	UINT8	data;
	UINT16 value;

	LOG2(("pit8253_read(): pit %d, offset %d\n",which,offset));

	if (timer == NULL)
	{
		/* Reading mode control register is illegal according to docs */
		/* Experimentally determined: reading it returns 0 */
		data = 0;
	}
	else
	{
		update(timer);

		if (timer->latched_status)
		{
			/* Read status register (8254 only) */
			data = timer->status;
			timer->latched_status =	0;
		}
		else
		{
			if (timer->latched_count !=	0)
			{
				/* Read back latched count */
				data = (timer->latch >>	(timer->rmsb !=	0 ?	8 :	0))	& 0xff;
				timer->rmsb	= 1	- timer->rmsb;
				--timer->latched_count;
			}
			else {
				value =	masked_value(timer);

				/* Read back current count */
				switch(CTRL_ACCESS(timer->control))	{
				case 0:
				default:
					/* This should never happen */
					data = 0; /* Appease compiler */
					break;

				case 1:
					/* read counter bits 0-7 only */
					data = (value >> 0)	& 0xff;
					break;

				case 2:
					/* read counter bits 8-15 only */
					data = (value >> 8)	& 0xff;
					break;

				case 3:
					/* read bits 0-7 first, then 8-15 */
					data = (value >> (timer->rmsb != 0 ? 8 : 0)) & 0xff;
					timer->rmsb	= 1	- timer->rmsb;
					break;
				}
			}
		}
	}

	LOG2(("pit8253_read(): PIT #%d offset=%d data=0x%02x\n", which, (int) offset, (unsigned) data));
	return data;
}


/* Loads a new value from the bus to the count register (CR) */
static void	load_count(struct pit8253_timer	*timer,	UINT16 newcount)
{
	int	mode = CTRL_MODE(timer->control);

	LOG1(("pit8253: load_count(): %04x\n",newcount));

	if (newcount ==	1)
	{
		/* Count of 1 is illegal in modes 2 and 3. What happens here was
           determined experimentally. */
		if (mode ==	2)
			newcount = 2;
		if (mode ==	3)
			newcount = 0;
	}
	timer->count = newcount;
	timer->null_count =	1;
	if (mode ==	2 || mode == 3)
	{
		if (timer->phase ==	0)
		{
			trigger_countdown(timer);
		}
		else
		{
			int	bcd	= CTRL_BCD(timer->control);
			if (mode ==	2)
			{
				freq_callback_in(timer,adjusted_count(bcd,timer->value));
			}
			else
			{
				freq_callback_in(timer,(adjusted_count(bcd,timer->value) + 1) >> 1);
			}
		}
	}
	else
	{
		if (mode ==	0 || mode == 4)
		{
			trigger_countdown(timer);
		}
	}
}


static void	readback(struct	pit8253_timer *timer,int command)
{
	UINT16 value;
	update(timer);

	if ((command & 1) == 0)
	{
		/* readback status command */
		if (timer->latched_status == 0)
		{
			timer->status =	timer->control | (timer->output	!= 0 ? 0x80	: 0) | (timer->null_count != 0 ? 0x40 :	0);
		}

		timer->latched_status =	1;
	}
	/* Experimentally determined: the read latch command seems to have no
       effect if we're halfway through a 16-bit read */
	if ((command & 2) == 0 && timer->rmsb == 0)
	{
		/* readback count command */

		if (timer->latched_count ==	0)
		{
			value =	masked_value(timer);
			switch(CTRL_ACCESS(timer->control))	{
			case 0:
				/* This should never happen */
				break;

			case 1:
				/* latch bits 0-7 only */
				timer->latch = ((value << 8) & 0xff00) | (value	& 0xff);
				timer->latched_count = 1;
				break;

			case 2:
				/* read bits 8-15 only */
				timer->latch = (value &	0xff00)	| ((value >> 8)	& 0xff);
				timer->latched_count = 1;
				break;

			case 3:
				/* latch all 16 bits */
				timer->latch = value;
				timer->latched_count = 2;
				break;
			}
		}
	}
}


static void	pit8253_write(int which, offs_t	offset,	int	data)
{
	struct pit8253 *pit	= get_pit(which);
	struct pit8253_timer *timer	= get_timer(pit,offset);
	int	read_command;

	LOG2(("pit8253_write(): PIT #%d offset=%d data=0x%02x\n", which, (int) offset, (unsigned) data));

	if (timer == NULL) {
		/* Write to mode control register */
		timer =	get_timer(pit, (data >>	6) & 3);
		if (timer == NULL)
		{
			/* Readback command. Illegal on 8253 */
			/* Todo: find out what (if anything) the 8253 hardware actually does here. */
			if (pit->config->type == TYPE8254)
			{
				LOG1(("pit8253_write(): PIT #%d readback %02x\n", which, data & 0x3f));

				/* Bit 0 of data must be 0. Todo: find out what the hardware does if it isn't. */
				read_command = (data >>	4) & 3;
				if ((data &	2) != 0)
					readback(get_timer(pit,0),read_command);
				if ((data &	4) != 0)
					readback(get_timer(pit,1),read_command);
				if ((data &	8) != 0)
					readback(get_timer(pit,2),read_command);
			}
			return;
		}

		update(timer);

		if (CTRL_ACCESS(data) == 0)
		{
			LOG1(("pit8253_write(): PIT #%d timer=%d readback\n", which, (data >> 6) & 3));

			/* Latch current timer value */
			/* Experimentally verified: this command does not affect the mode control register */
			readback(timer,1);
		}
		else {
			LOG1(("pit8253_write(): PIT #%d timer=%d bytes=%d mode=%d bcd=%d\n", which, (data >> 6) & 3, (data >> 4) & 3, (data >> 1) & 7,data & 1));

			timer->control = (data & 0x3f);
			timer->null_count =	1;
			timer->wmsb	= timer->rmsb =	0;
			/* Phase 0 is always the phase after a mode control write */
			timer->phase = 0;
			set_output(timer,1);
			set_freq_count(timer);
		}
	}
	else
	{
		update(timer);

		switch(CTRL_ACCESS(timer->control))	{
		case 0:
			/* This should never happen */
			break;

		case 1:
			/* read/write counter bits 0-7 only */
			load_count(timer,data);
			break;

		case 2:
			/* read/write counter bits 8-15 only */
			load_count(timer,data << 8);
			break;

		case 3:
			/* read/write bits 0-7 first, then 8-15 */
			if (timer->wmsb	!= 0)
			{
				load_count(timer,timer->lowcount | (data <<	8));
			}
			else
			{
				timer->lowcount	= data;
				if (CTRL_MODE(timer->control) == 0)
				{
					/* The Intel docs say that writing the MSB in mode 0, phase
                       2 won't stop the count, but this was experimentally
                       determined to be false. */
					timer->phase = 0;
				}
			}
			timer->wmsb	= 1	- timer->wmsb;
			break;
		}
	}
	update(timer);
}


static void	pit8253_gate_write(int which,int offset,int	data)
{
	struct pit8253_timer *timer	= get_timer(get_pit(which),offset);
	int	mode;
	int	gate = (data!=0	? 1	: 0);

	LOG2(("pit8253_gate_write(): PIT #%d offset=%d gate=%d\n", which, (int) offset, (unsigned) data));

	if (timer == NULL)
		return;

	mode = CTRL_MODE(timer->control);

	if (gate !=	timer->gate)
	{
		update(timer);
		timer->gate	= gate;
		set_freq_count(timer);
		if (gate !=	0 &&
			(mode == 1 || mode == 5	||
			 (timer->phase == 1	&& (mode ==	2 || mode == 3))))
		{
			trigger_countdown(timer);
		}
		update(timer);
	}
}



/* ----------------------------------------------------------------------- */

int	pit8253_get_frequency(int which, int timerno)
{
	struct pit8253_timer *timer	= get_timer(get_pit(which),timerno);

	update(timer);
	return get_frequency(timer);
}



int	pit8253_get_output(int which, int timerno)
{
	struct pit8253_timer *timer	= get_timer(get_pit(which),timerno);
	int	result;

	update(timer);
	result = timer->output;
	LOG2(("pit8253_get_output(): PIT #%d timer=%d result=%d\n", which, timerno, result));
	return result;
}



void pit8253_set_clockin(int which,	int	timerno, double	new_clockin)
{
	struct pit8253_timer *timer	= get_timer(get_pit(which),timerno);

	LOG2(("pit8253_set_clockin(): PIT #%d timer=%d, clockin = %lf\n", which, (int) timerno,new_clockin));

	update(timer);
	timer->clockin = new_clockin;
	update(timer);

	if (timer->freq_callback !=	NULL)
	{
		timer->freq_callback(get_frequency(timer));
		if (timer->cycles_to_freq != CYCLES_NEVER)
		{
			freq_callback_in(timer,timer->cycles_to_freq);
		}
	}
}



/* ----------------------------------------------------------------------- */

READ8_HANDLER (	pit8253_0_r	) {	return pit8253_read(0, offset);	}
READ8_HANDLER (	pit8253_1_r	) {	return pit8253_read(1, offset);	}
WRITE8_HANDLER ( pit8253_0_w ) { pit8253_write(0, offset, data); }
WRITE8_HANDLER ( pit8253_1_w ) { pit8253_write(1, offset, data); }

READ16_HANDLER ( pit8253_0_lsb_r ) { return pit8253_read(0, offset);	}
READ16_HANDLER ( pit8253_1_lsb_r ) { return pit8253_read(1, offset);	}
WRITE16_HANDLER ( pit8253_0_lsb_w ) { if (ACCESSING_BYTE_0) pit8253_write(0, offset, data); }
WRITE16_HANDLER ( pit8253_1_lsb_w ) { if (ACCESSING_BYTE_0) pit8253_write(1, offset, data); }

READ16_HANDLER ( pit8253_16le_0_r ) { return read16le_with_read8_handler(pit8253_0_r, machine, offset, mem_mask); }
READ16_HANDLER ( pit8253_16le_1_r ) { return read16le_with_read8_handler(pit8253_1_r, machine, offset, mem_mask); }
WRITE16_HANDLER	( pit8253_16le_0_w ) { write16le_with_write8_handler(pit8253_0_w, machine, offset, data,	mem_mask); }
WRITE16_HANDLER	( pit8253_16le_1_w ) { write16le_with_write8_handler(pit8253_1_w, machine, offset, data,	mem_mask); }

READ32_HANDLER ( pit8253_32le_0_r ) { return read32le_with_read8_handler(pit8253_0_r, machine, offset, mem_mask); }
READ32_HANDLER ( pit8253_32le_1_r ) { return read32le_with_read8_handler(pit8253_1_r, machine, offset, mem_mask); }
WRITE32_HANDLER	( pit8253_32le_0_w ) { write32le_with_write8_handler(pit8253_0_w, machine, offset, data,	mem_mask); }
WRITE32_HANDLER	( pit8253_32le_1_w ) { write32le_with_write8_handler(pit8253_1_w, machine, offset, data,	mem_mask); }

READ64_HANDLER ( pit8253_64be_0_r ) { return read64be_with_read8_handler(pit8253_0_r, machine, offset, mem_mask); }
READ64_HANDLER ( pit8253_64be_1_r ) { return read64be_with_read8_handler(pit8253_1_r, machine, offset, mem_mask); }
WRITE64_HANDLER	( pit8253_64be_0_w ) { write64be_with_write8_handler(pit8253_0_w, machine, offset, data,	mem_mask); }
WRITE64_HANDLER	( pit8253_64be_1_w ) { write64be_with_write8_handler(pit8253_1_w, machine, offset, data,	mem_mask); }

WRITE8_HANDLER ( pit8253_0_gate_w )	{ pit8253_gate_write(0,	offset,	data); }
WRITE8_HANDLER ( pit8253_1_gate_w )	{ pit8253_gate_write(1,	offset,	data); }

