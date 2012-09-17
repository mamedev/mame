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
 *      1-Apr-2008 - WFP:   Changed the implementation into a device.
 *      8-Jul-2004 - AJ:    Fixed some bugs. Styx now runs correctly.
 *                          Implemented 8254 features.
 *      1-Mar-2004 - NPW:   Did an almost total rewrite and cleaned out much
 *                          of the ugliness in the previous design.  Bug #430
 *                          seems to be fixed
 *      1-Jul-2000 - PeT:   Split off from PC driver and componentized
 *
 *****************************************************************************/

#include "emu.h"
#include "machine/pit8253.h"



/***************************************************************************

    Structures & macros

***************************************************************************/

#define	MAX_TIMER		3
#define	VERBOSE			0

#define	LOG1(msg)		do { if (VERBOSE >= 1) logerror msg; } while (0)
#define	LOG2(msg)		do { if (VERBOSE >= 2) logerror msg; } while (0)

#define	CYCLES_NEVER ((UINT32) -1)

/* device types */
enum {
	TYPE_PIT8253 = 0,
	TYPE_PIT8254,

	NUM_TYPES
};


/* device tags */
static const char * const device_tags[NUM_TYPES] = { "pit8253", "pit8254" };


struct pit8253_timer
{
	int index;						/* index number of the timer */
	double clockin;					/* input clock frequency in Hz */
	int clock;						/* clock signal when clockin is 0 */

	devcb_resolved_read_line	in_gate_func;	/* callback for gate input */
	devcb_resolved_write_line	out_out_func;	/* callback function for when output changes */

	attotime last_updated;			/* time when last updated */

	emu_timer *updatetimer;			/* MAME timer to process updates */

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
};

struct	pit8253_t
{
	const pit8253_config *config;
	int	device_type;
	pit8253_timer timers[MAX_TIMER];
};

#define	CTRL_ACCESS(control)		(((control)	>> 4) &	0x03)
#define	CTRL_MODE(control)			(((control)	>> 1) &	(((control)	& 0x04)	? 0x03 : 0x07))
#define	CTRL_BCD(control)			(((control)	>> 0) &	0x01)


/***************************************************************************

    Functions

***************************************************************************/

/* makes sure that the passed in device is of the right type */
INLINE pit8253_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert((device->type() == PIT8253) || (device->type() == PIT8254));

	return (pit8253_t *) downcast<pit8253_device *>(device)->token();
}


static pit8253_timer	*get_timer(pit8253_t *pit,int which)
{
	which &= 3;
	if (which < MAX_TIMER)
		return &pit->timers[which];
	return NULL;
}


static int pit8253_gate(pit8253_timer *timer)
{
	if (!timer->in_gate_func.isnull())
		return timer->in_gate_func();
	else
		return timer->gate;
}


INLINE UINT32 decimal_from_bcd(UINT16 val)
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
static void	decrease_counter_value(pit8253_timer	*timer,UINT64 cycles)
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


/* Counter loading: transfer of a count from the CR to the CE */
static void load_counter_value(device_t *device, pit8253_timer *timer)
{
	timer->value = timer->count;
	timer->null_count = 1;
	if ( CTRL_MODE(timer->control) == 3 && timer->output == 0)
		timer->value &= 0xfffe;
}


static void	set_output(device_t *device, pit8253_timer *timer,int output)
{
	if (output != timer->output)
	{
		timer->output =	output;
		timer->out_out_func(timer->output);
	}
}


/* This emulates timer "timer" for "elapsed_cycles" cycles and assumes no
   callbacks occur during that time. */
static void	simulate2(device_t *device, pit8253_timer *timer, INT64 elapsed_cycles)
{
	UINT32 adjusted_value;
	int	bcd	= CTRL_BCD(timer->control);
	int	mode = CTRL_MODE(timer->control);
	int	cycles_to_output = 0;

	LOG2(("pit8253: simulate2(): simulating %d cycles for %d in mode %d, bcd = %d, phase = %d, gate = %d, output %d, value = 0x%04x\n",
		  (int)elapsed_cycles,timer->index,mode,bcd,timer->phase,pit8253_gate(timer),timer->output,timer->value));

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

		if (timer->phase ==	0)
		{
			cycles_to_output = CYCLES_NEVER;
		}
		else
		{
			if (elapsed_cycles >= 0 && timer->phase == 1)
			{
				/* Counter load cycle */
				if (elapsed_cycles > 0) {
					--elapsed_cycles;
					timer->phase = 2;
				}
				load_counter_value( device, timer );
			}

			if ( pit8253_gate(timer) == 0 )
			{
				cycles_to_output = CYCLES_NEVER;
			}
			else
			{
				if (timer->phase ==	2)
				{
					adjusted_value = adjusted_count(bcd,timer->value);
					if (elapsed_cycles >= adjusted_value)
					{
						/* Counter wrapped, output goes high */
						elapsed_cycles -= adjusted_value;
						timer->phase = 3;
						timer->value = 0;
						set_output( device, timer, 1 );
					}
				}

				decrease_counter_value(timer,elapsed_cycles);

				switch( timer->phase )
				{
				case 1:		cycles_to_output = 1; break;
				case 2:		cycles_to_output = adjusted_count( bcd, timer->value ); break;
				case 3:		cycles_to_output = adjusted_count( bcd, timer->value ); break;
				}
			}
		}
		break;


	case 1:
		/* Mode 1: (Hardware Retriggerable One-Shot a.k.a. Programmable One-Shot)

        -----+       +------------------
             |       |
             +-------+
             <-  n  ->

          ^
          +- trigger

        phase|output|length  |value|next|comment
        -----+------+--------+-----+----+----------------------------------
            0|high  |infinity|     |1   |counting down
            1|high  |1       |     |2   |internal delay to load counter
            2|low   |n       |n..1 |3   |counting down
            3|high  |infinity|0..1 |3   |counting down

        Gate rising-edge sensitive only.
        Rising edge initiates counting and resets output after next clock. */

		if ( elapsed_cycles >= 0 && timer->phase == 1 )
		{
			/* Counter load cycle, output goes low */
			if (elapsed_cycles > 0) {
				--elapsed_cycles;
				timer->phase = 2;
			}
			load_counter_value( device, timer );
			set_output( device, timer, 0 );
		}

		if ( timer->phase == 2 )
		{
			adjusted_value = adjusted_count( bcd, timer->value );
			if ( elapsed_cycles >= adjusted_value )
			{
				/* Counter wrapped, output goes high */
				timer->phase = 3;
				set_output( device, timer, 1 );
			}
		}

		decrease_counter_value( timer, elapsed_cycles );

		switch( timer->phase )
		{
		case 1:		cycles_to_output = 1; break;
		case 2:		cycles_to_output = adjusted_count( bcd, timer->value ); break;
		default:	cycles_to_output = CYCLES_NEVER; break;
		}
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
            1|high  |1       |     |2   |internal delay to load counter
            2|high  |n       |n..2 |3   |counting down
            3|low   |1       |1    |2   |reload counter

        Counter rewrite has no effect until repeated

        Gate rising-edge and level sensitive.
        Gate low disables counting and sets output immediately high.
        Rising-edge reloads count and initiates counting
        Gate high enables counting. */

		if (pit8253_gate(timer)	== 0 ||	timer->phase ==	0)
		{
			/* Gate low or mode control write forces output high */
			set_output(device, timer, 1);
			cycles_to_output = CYCLES_NEVER;
		}
		else
		{
			if ( elapsed_cycles >= 0 && timer->phase == 1 )
			{
				if (elapsed_cycles > 0) {
					--elapsed_cycles;
					timer->phase = 2;
				}
				load_counter_value( device, timer );
			}

			adjusted_value = adjusted_count( bcd, timer->value );

			do
			{
				if ( timer->phase == 2 )
				{
					if ( elapsed_cycles + 1 >= adjusted_value )
					{
						/* Coounter hits 1, output goes low */
						timer->phase = 3;
						set_output( device, timer, 0 );
					}
				}

				if ( elapsed_cycles > 0 && timer->phase == 3 )
				{
					/* Reload counter, output goes high */
					--elapsed_cycles;
					timer->phase = 2;
					load_counter_value( device, timer );
					adjusted_value = adjusted_count( bcd, timer->value );
					set_output( device, timer, 1 );
				}
			}
			while( elapsed_cycles >= adjusted_value );

			/* Calculate counter value */
			decrease_counter_value(timer,elapsed_cycles);

			switch( timer->phase )
			{
			case 1:		cycles_to_output = 1; break;
			default:	cycles_to_output = (timer->value ==	1 ?	1 :	(adjusted_count(bcd,timer->value) -	1));
			}
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
            1|high  |1       |     |2   |internal delay to load counter
            2|high  |n/2(+1) |n..0 |3   |counting down double speed, reload counter
            3|low   |n/2     |n..0 |2   |counting down double speed, reload counter

        Counter rewrite has no effect until repeated (output falling or rising)

        Gate rising-edge and level sensitive.
        Gate low disables counting and sets output immediately high.
        Rising-edge reloads count and initiates counting
        Gate high enables counting. */

		if (pit8253_gate(timer)	== 0 ||	timer->phase ==	0)
		{
			/* Gate low or mode control write forces output high */
			set_output(device, timer, 1);
			cycles_to_output = CYCLES_NEVER;
		}
		else
		{
			if ( elapsed_cycles >= 0 && timer->phase == 1 )
			{
				if (elapsed_cycles > 0) {
					--elapsed_cycles;
					timer->phase = 2;
				}
				load_counter_value( device, timer );
			}

			if (elapsed_cycles > 0) {
				adjusted_value = adjusted_count( bcd, timer->value );

				do
				{
					if ( timer->phase == 2 && elapsed_cycles >= ( ( adjusted_value + 1 ) >> 1 ) )
					{
						/* High phase expired, output goes low */
						elapsed_cycles -= ( ( adjusted_value + 1 ) >> 1 );
						timer->phase = 3;
						load_counter_value( device, timer );
						adjusted_value = adjusted_count( bcd, timer->value );
						set_output( device, timer, 0 );
					}

					if ( timer->phase == 3 && elapsed_cycles >= ( adjusted_value >> 1 ) )
					{
						/* Low phase expired, output goes high */
						elapsed_cycles -= ( adjusted_value >> 1 );
						timer->phase = 2;
						load_counter_value( device, timer );
						adjusted_value = adjusted_count( bcd, timer->value );
						set_output( device, timer, 1 );
					}
				}
				while( ( timer->phase == 2 && elapsed_cycles >= ( ( adjusted_value + 1 ) >> 1 ) ) ||
					   ( timer->phase == 3 && elapsed_cycles >= ( adjusted_value >> 1 ) ) );

				decrease_counter_value(timer,elapsed_cycles<<1);
				switch( timer->phase )
				{
				case 1:		cycles_to_output = 1; break;
				case 2:		cycles_to_output = ( adjusted_count( bcd, timer->value ) + 1 ) >> 1; break;
				case 3:		cycles_to_output = adjusted_count( bcd, timer->value ) >> 1; break;
				}
			}
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
            0|high  |infinity|0..1 |0   |waiting for count/counting down
            1|high  |1       |     |2   |internal delay when counter loaded
            2|high  |n       |n..1 |3   |counting down
            3|low   |1       |0    |0   |strobe

        Mode 4 only: counter rewrite loads new counter
        Mode 5 only: count not reloaded immediately.
        Mode control write doesn't stop count but sets output high

        Mode 4 only: Gate level sensitive only. Low disables counting, high enables it.
        Mode 5 only: Gate rising-edge sensitive only. Rising edge initiates counting */

		if (pit8253_gate(timer)	== 0 &&	mode ==	4)
		{
			cycles_to_output = CYCLES_NEVER;
		}
		else
		{
			if (elapsed_cycles >= 0 && timer->phase == 1)
			{
				if (elapsed_cycles > 0) {
					--elapsed_cycles;
					timer->phase = 2;
				}
				load_counter_value( device, timer );
			}

			if ( timer->value == 0 && timer->phase == 2 )
				adjusted_value = 0;
			else
				adjusted_value = adjusted_count( bcd, timer->value );

			if ( timer->phase == 2 && elapsed_cycles >= adjusted_value )
			{
				/* Counter has hit zero, set output to low */
				elapsed_cycles -= adjusted_value;
				timer->phase = 3;
				timer->value = 0;
				set_output( device, timer, 0 );
			}

			if (elapsed_cycles > 0 && timer->phase == 3)
			{
				--elapsed_cycles;
				timer->phase = 0;
				decrease_counter_value(timer,1);
				set_output( device, timer, 1 );
			}

			decrease_counter_value(timer,elapsed_cycles);

			switch( timer->phase )
			{
			case 1:		cycles_to_output = 1; break;
			case 2:		cycles_to_output = adjusted_count( bcd, timer->value ); break;
			case 3:		cycles_to_output = 1; break;
			}
		}
		break;
	}

	timer->cycles_to_output	= cycles_to_output;
	if (cycles_to_output ==	CYCLES_NEVER ||	timer->clockin == 0)
	{
		timer->updatetimer->adjust(attotime::never, timer->index);
	}
	else
	{
		attotime next_fire_time = timer->last_updated + cycles_to_output * attotime::from_hz( timer->clockin );

		timer->updatetimer->adjust(next_fire_time - device->machine().time(), timer->index );
	}

    LOG2(("pit8253: simulate2(): simulating %d cycles for %d in mode %d, bcd = %d, phase = %d, gate = %d, output %d, value = 0x%04x, cycles_to_output = %04x\n",
          (int)elapsed_cycles,timer->index,mode,bcd,timer->phase,pit8253_gate(timer),timer->output,timer->value,cycles_to_output));
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
static void	simulate(device_t *device, pit8253_timer *timer, INT64 elapsed_cycles)
{
	if ( elapsed_cycles > 0 )
		simulate2(device, timer, elapsed_cycles);
	else
		if ( timer->clockin )
			timer->updatetimer->adjust(attotime::from_hz( timer->clockin ), timer->index );
}


/* This brings timer "timer" up to date */
static void	update(device_t *device, pit8253_timer *timer)
{
	/* With the 82C54's maximum clockin of 10MHz, 64 bits is nearly 60,000
       years of time. Should be enough for now. */
	attotime now =	device->machine().time();
	attotime elapsed_time = now - timer->last_updated;
	INT64 elapsed_cycles =	elapsed_time.as_double() * timer->clockin;

	LOG1(("pit8253: update(): timer %d, %" I64FMT "d elapsed_cycles\n", timer->index, elapsed_cycles));

	if ( timer->clockin )
	{
		timer->last_updated	+= elapsed_cycles * attotime::from_hz(timer->clockin);
	}
	else
	{
		timer->last_updated = now;
	}

	simulate(device, timer, elapsed_cycles);
}


static TIMER_CALLBACK( update_timer_cb )
{
	device_t *device = (device_t *)ptr;
	pit8253_t	*pit8253 = get_safe_token(device);
	pit8253_timer *timer = get_timer(pit8253,param);

	LOG2(("pit8253: output_changed(): timer %d\n",param));

	update( device, timer );
}


/* We recycle bit 0 of timer->value to hold the phase in mode 3 when count is
   odd. Since read commands in mode 3 always return even numbers, we need to
   mask this bit off. */
static UINT16 masked_value(pit8253_timer	*timer)
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
READ8_DEVICE_HANDLER( pit8253_r )
{
	pit8253_t	*pit8253 = get_safe_token(device);
	pit8253_timer *timer	= get_timer(pit8253,offset);
	UINT8	data;
	UINT16 value;

	LOG2(("pit8253_r(): offset %d\n", offset));

	if (timer == NULL)
	{
		/* Reading mode control register is illegal according to docs */
		/* Experimentally determined: reading it returns 0 */
		data = 0;
	}
	else
	{
		update(device, timer);

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

	LOG2(("pit8253_r(): offset=%d data=0x%02x\n", offset, data));
	return data;
}


/* Loads a new value from the bus to the count register (CR) */
static void	load_count(device_t *device, pit8253_timer *timer, UINT16 newcount)
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
	if (mode ==	2 || mode == 3)
	{
		if (timer->phase ==	0)
		{
			timer->phase = 1;
		}
	}
	else
	{
		if (mode ==	0 || mode == 4)
		{
			timer->phase = 1;
		}
	}
}


static void	readback(device_t *device, pit8253_timer *timer,int command)
{
	UINT16 value;
	update(device, timer);

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


WRITE8_DEVICE_HANDLER( pit8253_w )
{
	pit8253_t	*pit8253 = get_safe_token(device);
	pit8253_timer *timer	= get_timer(pit8253,offset);
	int	read_command;

	LOG2(("pit8253_w(): offset=%d data=0x%02x\n", offset, data));

	if (timer == NULL) {
		/* Write to mode control register */
		timer =	get_timer(pit8253, (data >>	6) & 3);
		if (timer == NULL)
		{
			/* Readback command. Illegal on 8253 */
			/* Todo: find out what (if anything) the 8253 hardware actually does here. */
			if (pit8253->device_type == TYPE_PIT8254)
			{
				LOG1(("pit8253_w(): readback %02x\n", data & 0x3f));

				/* Bit 0 of data must be 0. Todo: find out what the hardware does if it isn't. */
				read_command = (data >>	4) & 3;
				if ((data &	2) != 0)
					readback(device, get_timer(pit8253,0), read_command);
				if ((data &	4) != 0)
					readback(device, get_timer(pit8253,1), read_command);
				if ((data &	8) != 0)
					readback(device, get_timer(pit8253,2), read_command);
			}
			return;
		}

		update(device, timer);

		if (CTRL_ACCESS(data) == 0)
		{
			LOG1(("pit8253_write(): timer=%d readback\n", (data >> 6) & 3));

			/* Latch current timer value */
			/* Experimentally verified: this command does not affect the mode control register */
			readback(device, timer, 1);
		}
		else {
			LOG1(("pit8253_write(): timer=%d bytes=%d mode=%d bcd=%d\n", (data >> 6) & 3, (data >> 4) & 3, (data >> 1) & 7,data & 1));

			timer->control = (data & 0x3f);
			timer->null_count =	1;
			timer->wmsb	= timer->rmsb =	0;
			/* Phase 0 is always the phase after a mode control write */
			timer->phase = 0;
			set_output(device, timer, CTRL_MODE(timer->control) ? 1 : 0);
		}
	}
	else
	{
		int	middle_of_a_cycle = 0;

		update(device, timer);

		if ( device->machine().time() > timer->last_updated && timer->clockin != 0 )
		{
			middle_of_a_cycle = 1;
		}

		switch(CTRL_ACCESS(timer->control))	{
		case 0:
			/* This should never happen */
			break;

		case 1:
			/* read/write counter bits 0-7 only */

			/* check if we should compensate for not being on a cycle boundary */
			if ( middle_of_a_cycle )
				timer->last_updated += attotime::from_hz(timer->clockin);

			load_count(device, timer, data);
			simulate2(device, timer, 0 );
			if ( CTRL_MODE(timer->control) == 0 )
			{
				set_output( device, timer, 0 );
			}
			break;

		case 2:
			/* read/write counter bits 8-15 only */

			/* check if we should compensate for not being on a cycle boundary */
			if ( middle_of_a_cycle )
				timer->last_updated += attotime::from_hz(timer->clockin);

			load_count(device, timer, data << 8);
			simulate2(device, timer, 0 );
			break;

		case 3:
			/* read/write bits 0-7 first, then 8-15 */
			if (timer->wmsb	!= 0)
			{
				/* check if we should compensate for not being on a cycle boundary */
				if ( middle_of_a_cycle )
					timer->last_updated += attotime::from_hz(timer->clockin);

				load_count(device, timer,timer->lowcount | (data << 8));
				simulate2(device, timer, 0 );
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
					set_output( device, timer, 0 );
				}
			}
			timer->wmsb	= 1	- timer->wmsb;
			break;
		}
	}
}

static void pit8253_gate_w(device_t *device, int gate, int state)
{
	pit8253_t	*pit8253 = get_safe_token(device);
	pit8253_timer *timer	= get_timer(pit8253, gate);

	LOG2(("pit8253_gate_w(): gate=%d state=%d\n", gate, state));

	if (timer == NULL)
		return;

	if (!timer->in_gate_func.isnull())
	{
		logerror("pit8253_gate_w: write has no effect because a read handler is already defined!\n");
	}
	else
	{
		if (state != timer->gate)
		{
			int mode = CTRL_MODE(timer->control);

			update(device, timer);
			timer->gate	= state;
			if (state != 0 && ( mode == 1 || mode == 2 || mode == 5 ))
			{
				timer->phase = 1;
			}
			update(device, timer);
		}
	}
}

WRITE_LINE_DEVICE_HANDLER( pit8253_gate0_w ) { pit8253_gate_w(device, 0, state); }
WRITE_LINE_DEVICE_HANDLER( pit8253_gate1_w ) { pit8253_gate_w(device, 1, state); }
WRITE_LINE_DEVICE_HANDLER( pit8253_gate2_w ) { pit8253_gate_w(device, 2, state); }


/* ----------------------------------------------------------------------- */

int	pit8253_get_output(device_t *device, int timerno)
{
	pit8253_t	*pit8253 = get_safe_token(device);
	pit8253_timer *timer	= get_timer(pit8253,timerno);
	int	result;

	update(device, timer);
	result = timer->output;
	LOG2(("pit8253_get_output(): PIT timer=%d result=%d\n", timerno, result));
	return result;
}



void pit8253_set_clockin(device_t *device, int timerno, double new_clockin)
{
	pit8253_t	*pit8253 = get_safe_token(device);
	pit8253_timer *timer	= get_timer(pit8253,timerno);

	LOG2(("pit8253_set_clockin(): PIT timer=%d, clockin = %lf\n", timerno,new_clockin));

	update(device, timer);
	timer->clockin = new_clockin;
	update(device, timer);
}


static void pit8253_set_clock_signal(device_t *device, int timerno, int state)
{
	pit8253_t	*pit8253 = get_safe_token(device);
	pit8253_timer *timer = get_timer(pit8253,timerno);

	LOG2(("pit8253_set_clock_signal(): PIT timer=%d, state = %d\n", timerno, state));

	/* Trigger on low to high transition */
	if ( !timer->clock && state )
	{
		/* Advance a cycle */
		simulate2(device, timer, 1);
	}
	timer->clock = state;
}

WRITE_LINE_DEVICE_HANDLER( pit8253_clk0_w ) { pit8253_set_clock_signal(device, 0, state); }
WRITE_LINE_DEVICE_HANDLER( pit8253_clk1_w ) { pit8253_set_clock_signal(device, 1, state); }
WRITE_LINE_DEVICE_HANDLER( pit8253_clk2_w ) { pit8253_set_clock_signal(device, 2, state); }


static void common_start( device_t *device, int device_type ) {
	pit8253_t	*pit8253 = get_safe_token(device);
	int			timerno;

	pit8253->config = (const struct pit8253_config *)device->static_config();
	pit8253->device_type = device_type;

	/* register for state saving */
	for (timerno = 0; timerno < MAX_TIMER; timerno++)
	{
		pit8253_timer *timer = get_timer(pit8253, timerno);

		/* initialize timer */
		timer->clockin = pit8253->config->timer[timerno].clockin;
		timer->updatetimer = device->machine().scheduler().timer_alloc(FUNC(update_timer_cb), (void *)device);
		timer->updatetimer->adjust(attotime::never, timerno);

		/* resolve callbacks */
		timer->in_gate_func.resolve(pit8253->config->timer[timerno].in_gate_func, *device);
		timer->out_out_func.resolve(pit8253->config->timer[timerno].out_out_func, *device);

		/* set up state save values */
		device->save_item(NAME(timer->clockin), timerno);
		device->save_item(NAME(timer->control), timerno);
		device->save_item(NAME(timer->status), timerno);
		device->save_item(NAME(timer->lowcount), timerno);
		device->save_item(NAME(timer->latch), timerno);
		device->save_item(NAME(timer->count), timerno);
		device->save_item(NAME(timer->value), timerno);
		device->save_item(NAME(timer->wmsb), timerno);
		device->save_item(NAME(timer->rmsb), timerno);
		device->save_item(NAME(timer->output), timerno);
		device->save_item(NAME(timer->gate), timerno);
		device->save_item(NAME(timer->latched_count), timerno);
		device->save_item(NAME(timer->latched_status), timerno);
		device->save_item(NAME(timer->null_count), timerno);
		device->save_item(NAME(timer->phase), timerno);
		device->save_item(NAME(timer->cycles_to_output), timerno);
		device->save_item(NAME(timer->last_updated), timerno);
		device->save_item(NAME(timer->clock), timerno);
	}
}


static DEVICE_START( pit8253 ) {
	common_start( device, TYPE_PIT8253 );
}


static DEVICE_START( pit8254 ) {
	common_start( device, TYPE_PIT8254 );
}


static DEVICE_RESET( pit8253 ) {
	pit8253_t *pit  = get_safe_token(device);
	int i;

	for (i = 0; i < MAX_TIMER; i++)
	{
		pit8253_timer *timer = get_timer(pit,i);
		/* According to Intel's 8254 docs, the state of a timer is undefined
           until the first mode control word is written. Here we define this
           undefined behaviour */
		timer->index = i;
		timer->control = timer->status = 0x30;
		timer->rmsb = timer->wmsb = 0;
		timer->count = timer->value = timer->latch = 0;
		timer->lowcount = 0;

		if (!timer->in_gate_func.isnull())
			timer->gate = timer->in_gate_func();
		else
			timer->gate = 1;

		timer->output = 2;	/* output is undetermined */
		timer->latched_count = 0;
		timer->latched_status = 0;
		timer->null_count = 1;
		timer->cycles_to_output = CYCLES_NEVER;

		timer->last_updated = device->machine().time();

		update(device, timer);
	}
}

const device_type PIT8253 = &device_creator<pit8253_device>;

pit8253_device::pit8253_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PIT8253, "Intel PIT8253", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(pit8253_t));
}
pit8253_device::pit8253_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(pit8253_t));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void pit8253_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pit8253_device::device_start()
{
	DEVICE_START_NAME( pit8253 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pit8253_device::device_reset()
{
	DEVICE_RESET_NAME( pit8253 )(this);
}


const device_type PIT8254 = &device_creator<pit8254_device>;

pit8254_device::pit8254_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pit8253_device(mconfig, PIT8254, "Intel PIT8254", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pit8254_device::device_start()
{
	DEVICE_START_NAME( pit8254 )(this);
}


