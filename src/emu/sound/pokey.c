/*****************************************************************************
 *
 *  POKEY chip emulator 4.6
 *  Copyright Nicola Salmoria and the MAME Team
 *
 *  Based on original info found in Ron Fries' Pokey emulator,
 *  with additions by Brad Oliver, Eric Smith and Juergen Buchmueller,
 *  paddle (a/d conversion) details from the Atari 400/800 Hardware Manual.
 *  Polynome algorithms according to info supplied by Perry McFarlane.
 *
 *  This code is subject to the MAME license, which besides other
 *  things means it is distributed as is, no warranties whatsoever.
 *  For more details read mame.txt that comes with MAME.
 *
 *  4.6:
 *  - changed audio emulation to emulate borrow 3 clock delay and
 *    proper channel reset. New frequency only becomes effective
 *    after the counter hits 0. Emulation also treats counters
 *    as 8 bit counters which are linked now instead of monolytic
 *    16 bit counters.
 *
 *  4.51:
 *  - changed to use the attotime datatype
 *  4.5:
 *  - changed the 9/17 bit polynomial formulas such that the values
 *    required for the Tempest Pokey protection will be found.
 *    Tempest expects the upper 4 bits of the RNG to appear in the
 *    lower 4 bits after four cycles, so there has to be a shift
 *    of 1 per cycle (which was not the case before). Bits #6-#13 of the
 *    new RNG give this expected result now, bits #0-7 of the 9 bit poly.
 *  - reading the RNG returns the shift register contents ^ 0xff.
 *    That way resetting the Pokey with SKCTL (which resets the
 *    polynome shifters to 0) returns the expected 0xff value.
 *  4.4:
 *  - reversed sample values to make OFF channels produce a zero signal.
 *    actually de-reversed them; don't remember that I reversed them ;-/
 *  4.3:
 *  - for POT inputs returning zero, immediately assert the ALLPOT
 *    bit after POTGO is written, otherwise start trigger timer
 *    depending on SK_PADDLE mode, either 1-228 scanlines or 1-2
 *    scanlines, depending on the SK_PADDLE bit of SKCTL.
 *  4.2:
 *  - half volume for channels which are inaudible (this should be
 *    close to the real thing).
 *  4.1:
 *  - default gain increased to closely match the old code.
 *  - random numbers repeat rate depends on POLY9 flag too!
 *  - verified sound output with many, many Atari 800 games,
 *    including the SUPPRESS_INAUDIBLE optimizations.
 *  4.0:
 *  - rewritten from scratch.
 *  - 16bit stream interface.
 *  - serout ready/complete delayed interrupts.
 *  - reworked pot analog/digital conversion timing.
 *  - optional non-indexing pokey update functions.
 *
 *****************************************************************************/

#include "emu.h"
#include "pokey.h"

/*
 * Defining this produces much more (about twice as much)
 * but also more efficient code. Ideally this should be set
 * for processors with big code cache and for healthy compilers :)
 */
#ifndef BIG_SWITCH
#ifndef HEAVY_MACRO_USAGE
#define HEAVY_MACRO_USAGE   1
#endif
#else
#define HEAVY_MACRO_USAGE	BIG_SWITCH
#endif

#define SUPPRESS_INAUDIBLE	1

/* Four channels with a range of 0..32767 and volume 0..15 */
//#define POKEY_DEFAULT_GAIN (32767/15/4)

/*
 * But we raise the gain and risk clipping, the old Pokey did
 * this too. It defined POKEY_DEFAULT_GAIN 6 and this was
 * 6 * 15 * 4 = 360, 360/256 = 1.40625
 * I use 15/11 = 1.3636, so this is a little lower.
 */
#define POKEY_DEFAULT_GAIN (32767/11/4)

#define VERBOSE 		1
#define VERBOSE_SOUND	1
#define VERBOSE_TIMER	1
#define VERBOSE_POLY	1
#define VERBOSE_RAND	1

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

#define LOG_SOUND(x) do { if (VERBOSE_SOUND) logerror x; } while (0)

#define LOG_TIMER(x) do { if (VERBOSE_TIMER) logerror x; } while (0)

#define LOG_POLY(x) do { if (VERBOSE_POLY) logerror x; } while (0)

#define LOG_RAND(x) do { if (VERBOSE_RAND) logerror x; } while (0)

#define CHAN1	0
#define CHAN2	1
#define CHAN3	2
#define CHAN4	3

#define TIMER1	0
#define TIMER2	1
#define TIMER4	2

/* values to add to the divisors for the different modes */
#define DIVADD_LOCLK		1
#define DIVADD_HICLK		4
#define DIVADD_HICLK_JOINED 7

/* AUDCx */
#define NOTPOLY5	0x80	/* selects POLY5 or direct CLOCK */
#define POLY4		0x40	/* selects POLY4 or POLY17 */
#define PURE		0x20	/* selects POLY4/17 or PURE tone */
#define VOLUME_ONLY 0x10	/* selects VOLUME OUTPUT ONLY */
#define VOLUME_MASK 0x0f	/* volume mask */

/* AUDCTL */
#define POLY9		0x80	/* selects POLY9 or POLY17 */
#define CH1_HICLK	0x40	/* selects 1.78979 MHz for Ch 1 */
#define CH3_HICLK	0x20	/* selects 1.78979 MHz for Ch 3 */
#define CH12_JOINED 0x10	/* clocks channel 1 w/channel 2 */
#define CH34_JOINED 0x08	/* clocks channel 3 w/channel 4 */
#define CH1_FILTER	0x04	/* selects channel 1 high pass filter */
#define CH2_FILTER	0x02	/* selects channel 2 high pass filter */
#define CLK_15KHZ	0x01	/* selects 15.6999 kHz or 63.9211 kHz */

/* IRQEN (D20E) */
#define IRQ_BREAK	0x80	/* BREAK key pressed interrupt */
#define IRQ_KEYBD	0x40	/* keyboard data ready interrupt */
#define IRQ_SERIN	0x20	/* serial input data ready interrupt */
#define IRQ_SEROR	0x10	/* serial output register ready interrupt */
#define IRQ_SEROC	0x08	/* serial output complete interrupt */
#define IRQ_TIMR4	0x04	/* timer channel #4 interrupt */
#define IRQ_TIMR2	0x02	/* timer channel #2 interrupt */
#define IRQ_TIMR1	0x01	/* timer channel #1 interrupt */

/* SKSTAT (R/D20F) */
#define SK_FRAME	0x80	/* serial framing error */
#define SK_OVERRUN	0x40	/* serial overrun error */
#define SK_KBERR	0x20	/* keyboard overrun error */
#define SK_SERIN	0x10	/* serial input high */
#define SK_SHIFT	0x08	/* shift key pressed */
#define SK_KEYBD	0x04	/* keyboard key pressed */
#define SK_SEROUT	0x02	/* serial output active */

/* SKCTL (W/D20F) */
#define SK_BREAK	0x80	/* serial out break signal */
#define SK_BPS		0x70	/* bits per second */
#define SK_FM		0x08	/* FM mode */
#define SK_PADDLE	0x04	/* fast paddle a/d conversion */
#define SK_RESET	0x03	/* reset serial/keyboard interface */

#define DIV_64		28		 /* divisor for 1.78979 MHz clock to 63.9211 kHz */
#define DIV_15		114 	 /* divisor for 1.78979 MHz clock to 15.6999 kHz */

typedef struct _pokey_state pokey_state;
struct _pokey_state
{
	INT32 clock_cnt[3];		/* clock counters */
	INT32 borrow_cnt[4];	/* borrow counters */

	INT32 counter[4];		/* channel counter */
	INT32 divisor[4];		/* channel divisor (modulo value) */
	UINT32 volume[4];		/* channel volume - derived */
	UINT8 output[4];		/* channel output signal (1 active, 0 inactive) */
	UINT8 filter_sample[4];  /* hi-pass filter sample */
	UINT32 p4;              /* poly4 index */
	UINT32 p5;              /* poly5 index */
	UINT32 p9;              /* poly9 index */
	UINT32 p17;             /* poly17 index */
	UINT32 r9;				/* rand9 index */
	UINT32 r17;             /* rand17 index */
	UINT32 clockmult;		/* clock multiplier */
	device_t *device;
	sound_stream * channel; /* streams channel */
	emu_timer *timer[3];	/* timers for channel 1,2 and 4 events */
	attotime timer_period[3];	/* computed periods for these timers */
	int timer_param[3];		/* computed parameters for these timers */
	emu_timer *rtimer;     /* timer for calculating the random offset */
	emu_timer *ptimer[8];	/* pot timers */
	devcb_resolved_read8 pot_r[8];
	devcb_resolved_read8 allpot_r;
	devcb_resolved_read8 serin_r;
	devcb_resolved_write8 serout_w;
	void (*interrupt_cb)(device_t *device, int mask);
	UINT8 AUDF[4];          /* AUDFx (D200, D202, D204, D206) */
	UINT8 AUDC[4];			/* AUDCx (D201, D203, D205, D207) */
	UINT8 POTx[8];			/* POTx   (R/D200-D207) */
	UINT8 AUDCTL;			/* AUDCTL (W/D208) */
	UINT8 ALLPOT;			/* ALLPOT (R/D208) */
	UINT8 KBCODE;			/* KBCODE (R/D209) */
	UINT8 RANDOM;			/* RANDOM (R/D20A) */
	UINT8 SERIN;			/* SERIN  (R/D20D) */
	UINT8 SEROUT;			/* SEROUT (W/D20D) */
	UINT8 IRQST;			/* IRQST  (R/D20E) */
	UINT8 IRQEN;			/* IRQEN  (W/D20E) */
	UINT8 SKSTAT;			/* SKSTAT (R/D20F) */
	UINT8 SKCTL;			/* SKCTL  (W/D20F) */
	pokey_interface intf;
	attotime clock_period;
	attotime ad_time_fast;
	attotime ad_time_slow;

	UINT8 poly4[0x0f];
	UINT8 poly5[0x1f];
	UINT8 poly9[0x1ff];
	UINT8 poly17[0x1ffff];

	UINT8 rand9[0x1ff];
	UINT8 rand17[0x1ffff];
};


#define P4(chip)  chip->poly4[chip->p4]
#define P5(chip)  chip->poly5[chip->p5]
#define P9(chip)  chip->poly9[chip->p9]
#define P17(chip) chip->poly17[chip->p17]

static TIMER_CALLBACK( pokey_timer_expire );
static TIMER_CALLBACK( pokey_pot_trigger );

#define CLK_1 0
#define CLK_28 1
#define CLK_114 2

static const int clock_divisors[3] = {1, 28, 114};

INLINE pokey_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == POKEY);
	return (pokey_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE void reset_channel(pokey_state *chip, int ch)
{
	chip->counter[ch] = chip->AUDF[ch] ^ 0xff;
}


INLINE void process_channel(pokey_state *chip, int ch)
{
	int toggle = 0;

	if( (chip->AUDC[ch] & NOTPOLY5) || P5(chip) )
	{
		if( chip->AUDC[ch] & PURE )
			toggle = 1;
		else
		if( chip->AUDC[ch] & POLY4 )
			toggle = chip->output[ch] == !P4(chip);
		else
		if( chip->AUDCTL & POLY9 )
			toggle = chip->output[ch] == !P9(chip);
		else
			toggle = chip->output[ch] == !P17(chip);
	}
	if( toggle )
	{
		chip->output[ch] ^= 1;
	}
	/* is this a filtering channel (3/4) and is the filter active? */

	if( chip->AUDCTL & ((CH1_FILTER|CH2_FILTER) & (0x10 >> ch)) )
    {
		if( ch >= 2)
        {
			chip->filter_sample[ch-2] = chip->output[ch-2];
        }
    }

}

/*
 * http://www.atariage.com/forums/topic/3328-sio-protocol/page__st__100#entry1680190:
 * I noticed that the Pokey counters have clocked carry (actually, "borrow") positions that delay the
 * counter by 3 cycles, plus the 1 reset clock. So 16 bit mode has 6 carry delays and a reset clock.
 * I'm sure this was done because the propagation delays limited the number of cells the subtraction could ripple though.
 *
 */

INLINE void inc_chan(pokey_state *chip, int ch)
{
	chip->counter[ch] = (chip->counter[ch] + 1) & 0xff;
	if (chip->counter[ch] == 0 && chip->borrow_cnt[ch] == 0)
		chip->borrow_cnt[ch] = 3;
}

INLINE int check_borrow(pokey_state *chip, int ch)
{
	if (chip->borrow_cnt[ch] > 0)
	{
		chip->borrow_cnt[ch]--;
		return (chip->borrow_cnt[CHAN1] == 0);
	}
	return 0;
}

static STREAM_UPDATE( pokey_update )
{
	pokey_state *chip = (pokey_state *)param;
	stream_sample_t *buffer = outputs[0];
	int base_clock = (chip->AUDCTL & CLK_15KHZ) ? CLK_114 : CLK_28;

	while( samples > 0 )
	{
		int ch, clk;
		UINT32 sum = 0;
		int clock_triggered[3] = {0,0,0};

		for (clk = 0; clk < 3; clk++)
		{
			chip->clock_cnt[clk]++;
			if (chip->clock_cnt[clk] >= clock_divisors[clk])
			{
				chip->clock_cnt[clk] = 0;
				clock_triggered[clk] = 1;
			}
		}

		chip->p4 = (chip->p4 + 1) % 0x0000f;
		chip->p5 = (chip->p5 + 1) % 0x0001f;
		chip->p9 = (chip->p9 + 1) % 0x001ff;
		chip->p17 = (chip->p17 + 1 ) % 0x1ffff;


		clk = (chip->AUDCTL & CH1_HICLK) ? CLK_1 : base_clock;
		if (clock_triggered[clk])
			inc_chan(chip, CHAN1);

		clk = (chip->AUDCTL & CH3_HICLK) ? CLK_1 : base_clock;
		if (clock_triggered[clk])
			inc_chan(chip, CHAN3);

		if (clock_triggered[base_clock])
		{
			if (!(chip->AUDCTL & CH12_JOINED))
				inc_chan(chip, CHAN2);
			if (!(chip->AUDCTL & CH34_JOINED))
				inc_chan(chip, CHAN4);
		}

		/* do CHAN2 before CHAN1 because CHAN1 may set borrow! */
		if (check_borrow(chip, CHAN2))
		{
			int isJoined = (chip->AUDCTL & CH12_JOINED);
			if (isJoined)
				reset_channel(chip, CHAN1);
			reset_channel(chip, CHAN2);
			process_channel(chip, CHAN2);
		}

		if (check_borrow(chip, CHAN1))
		{
			int isJoined = (chip->AUDCTL & CH12_JOINED);
			if (isJoined)
				inc_chan(chip, CHAN2);
			else
				reset_channel(chip, CHAN1);
			process_channel(chip, CHAN1);
		}

		/* do CHAN4 before CHAN3 because CHAN3 may set borrow! */
		if (check_borrow(chip, CHAN4))
		{
			int isJoined = (chip->AUDCTL & CH34_JOINED);
			if (isJoined)
				reset_channel(chip, CHAN3);
			reset_channel(chip, CHAN4);
			process_channel(chip, CHAN4);
		}

		if (check_borrow(chip, CHAN3))
		{
			int isJoined = (chip->AUDCTL & CH34_JOINED);
			if (isJoined)
				inc_chan(chip, CHAN4);
			else
				reset_channel(chip, CHAN3);
			process_channel(chip, CHAN3);
		}

		for (ch = 0; ch < 4; ch++)
		{
			sum += (((chip->output[ch] ^ chip->filter_sample[ch]) || (chip->AUDC[ch] & VOLUME_ONLY)) ? chip->volume[ch] : 0 );
		}

       	/* store sum of output signals into the buffer */
       	*buffer++ = (sum > 0x7fff) ? 0x7fff : sum;
       	samples--;

	}
	chip->rtimer->adjust(attotime::never);

}


static void poly_init(UINT8 *poly, int size, int left, int right, int add)
{
	int mask = (1 << size) - 1;
    int i, x = 0;

	LOG_POLY(("poly %d\n", size));
	for( i = 0; i < mask; i++ )
	{
		*poly++ = x & 1;
		LOG_POLY(("%05x: %d\n", x, x&1));
        /* calculate next bit */
		x = ((x << left) + (x >> right) + add) & mask;
	}
}

static void rand_init(UINT8 *rng, int size, int left, int right, int add)
{
    int mask = (1 << size) - 1;
    int i, x = 0;

	LOG_RAND(("rand %d\n", size));
    for( i = 0; i < mask; i++ )
	{
		if (size == 17)
			*rng = x >> 6;	/* use bits 6..13 */
		else
			*rng = x;		/* use bits 0..7 */
        LOG_RAND(("%05x: %02x\n", x, *rng));
        rng++;
        /* calculate next bit */
		x = ((x << left) + (x >> right) + add) & mask;
	}
}


static void register_for_save(pokey_state *chip, device_t *device)
{
	device->save_item(NAME(chip->counter));
	device->save_item(NAME(chip->divisor));
	device->save_item(NAME(chip->volume));
	device->save_item(NAME(chip->output));
	device->save_item(NAME(chip->filter_sample));
	device->save_item(NAME(chip->clock_cnt));
	device->save_item(NAME(chip->p4));
	device->save_item(NAME(chip->p5));
	device->save_item(NAME(chip->p9));
	device->save_item(NAME(chip->p17));
	device->save_item(NAME(chip->r9));
	device->save_item(NAME(chip->r17));
	device->save_item(NAME(chip->clockmult));
	device->save_item(NAME(chip->timer_period[0]));
	device->save_item(NAME(chip->timer_period[1]));
	device->save_item(NAME(chip->timer_period[2]));
	device->save_item(NAME(chip->timer_param));
	device->save_item(NAME(chip->AUDF));
	device->save_item(NAME(chip->AUDC));
	device->save_item(NAME(chip->POTx));
	device->save_item(NAME(chip->AUDCTL));
	device->save_item(NAME(chip->ALLPOT));
	device->save_item(NAME(chip->KBCODE));
	device->save_item(NAME(chip->RANDOM));
	device->save_item(NAME(chip->SERIN));
	device->save_item(NAME(chip->SEROUT));
	device->save_item(NAME(chip->IRQST));
	device->save_item(NAME(chip->IRQEN));
	device->save_item(NAME(chip->SKSTAT));
	device->save_item(NAME(chip->SKCTL));
}


static DEVICE_START( pokey )
{
	pokey_state *chip = get_safe_token(device);
	int sample_rate = device->clock();
	int i;

	if (device->static_config())
		memcpy(&chip->intf, device->static_config(), sizeof(pokey_interface));
	chip->device = device;
	chip->clock_period = attotime::from_hz(device->clock());

	/* calculate the A/D times
     * In normal, slow mode (SKCTL bit SK_PADDLE is clear) the conversion
     * takes N scanlines, where N is the paddle value. A single scanline
     * takes approximately 64us to finish (1.78979MHz clock).
     * In quick mode (SK_PADDLE set) the conversion is done very fast
     * (takes two scanlines) but the result is not as accurate.
     */
	chip->ad_time_fast = (attotime::from_nsec(64000*2/228) * FREQ_17_EXACT) / device->clock();
	chip->ad_time_slow = (attotime::from_nsec(64000      ) * FREQ_17_EXACT) / device->clock();

	/* initialize the poly counters */
	poly_init(chip->poly4,   4, 3, 1, 0x00004);
	poly_init(chip->poly5,   5, 3, 2, 0x00008);
	poly_init(chip->poly9,   9, 8, 1, 0x00180);
	poly_init(chip->poly17, 17,16, 1, 0x1c000);

	/* initialize the random arrays */
	rand_init(chip->rand9,   9, 8, 1, 0x00180);
	rand_init(chip->rand17, 17,16, 1, 0x1c000);

	chip->divisor[CHAN1] = 4;
	chip->divisor[CHAN2] = 4;
	chip->divisor[CHAN3] = 4;
	chip->divisor[CHAN4] = 4;
	chip->clockmult = DIV_64;
	chip->KBCODE = 0x09;		 /* Atari 800 'no key' */
	chip->SKCTL = SK_RESET;	 /* let the RNG run after reset */
	chip->rtimer = device->machine().scheduler().timer_alloc(FUNC_NULL);

	chip->timer[0] = device->machine().scheduler().timer_alloc(FUNC(pokey_timer_expire), chip);
	chip->timer[1] = device->machine().scheduler().timer_alloc(FUNC(pokey_timer_expire), chip);
	chip->timer[2] = device->machine().scheduler().timer_alloc(FUNC(pokey_timer_expire), chip);

	for (i=0; i<8; i++)
	{
		chip->ptimer[i] = device->machine().scheduler().timer_alloc(FUNC(pokey_pot_trigger), chip);
		chip->pot_r[i].resolve(chip->intf.pot_r[i], *device);
	}
	chip->allpot_r.resolve(chip->intf.allpot_r, *device);
	chip->serin_r.resolve(chip->intf.serin_r, *device);
	chip->serout_w.resolve(chip->intf.serout_w, *device);
	chip->interrupt_cb = chip->intf.interrupt_cb;

	chip->channel = device->machine().sound().stream_alloc(*device, 0, 1, sample_rate, chip, pokey_update);

	register_for_save(chip, device);
}

static TIMER_CALLBACK( pokey_timer_expire )
{
	pokey_state *p = (pokey_state *)ptr;
	int timers = param;

	LOG_TIMER(("POKEY #%p timer %d with IRQEN $%02x\n", p, timers, p->IRQEN));

    /* check if some of the requested timer interrupts are enabled */
	timers &= p->IRQEN;

    if( timers )
    {
		/* set the enabled timer irq status bits */
		p->IRQST |= timers;
        /* call back an application supplied function to handle the interrupt */
		if( p->interrupt_cb )
			(*p->interrupt_cb)(p->device, timers);
    }
}

static char *audc2str(int val)
{
	static char buff[80];
	if( val & NOTPOLY5 )
	{
		if( val & PURE )
			strcpy(buff,"pure");
		else
		if( val & POLY4 )
			strcpy(buff,"poly4");
		else
			strcpy(buff,"poly9/17");
	}
	else
	{
		if( val & PURE )
			strcpy(buff,"poly5");
		else
		if( val & POLY4 )
			strcpy(buff,"poly4+poly5");
		else
			strcpy(buff,"poly9/17+poly5");
    }
	return buff;
}

static char *audctl2str(int val)
{
	static char buff[80];
	if( val & POLY9 )
		strcpy(buff,"poly9");
	else
		strcpy(buff,"poly17");
	if( val & CH1_HICLK )
		strcat(buff,"+ch1hi");
	if( val & CH3_HICLK )
		strcat(buff,"+ch3hi");
	if( val & CH12_JOINED )
		strcat(buff,"+ch1/2");
	if( val & CH34_JOINED )
		strcat(buff,"+ch3/4");
	if( val & CH1_FILTER )
		strcat(buff,"+ch1filter");
	if( val & CH2_FILTER )
		strcat(buff,"+ch2filter");
	if( val & CLK_15KHZ )
		strcat(buff,"+clk15");
    return buff;
}

static TIMER_CALLBACK( pokey_serin_ready_cb )
{
	pokey_state *p = (pokey_state *)ptr;
    if( p->IRQEN & IRQ_SERIN )
	{
		/* set the enabled timer irq status bits */
		p->IRQST |= IRQ_SERIN;
		/* call back an application supplied function to handle the interrupt */
		if( p->interrupt_cb )
			(*p->interrupt_cb)(p->device, IRQ_SERIN);
	}
}

static TIMER_CALLBACK( pokey_serout_ready_cb )
{
	pokey_state *p = (pokey_state *)ptr;
    if( p->IRQEN & IRQ_SEROR )
	{
		p->IRQST |= IRQ_SEROR;
		if( p->interrupt_cb )
			(*p->interrupt_cb)(p->device, IRQ_SEROR);
	}
}

static TIMER_CALLBACK( pokey_serout_complete )
{
	pokey_state *p = (pokey_state *)ptr;
    if( p->IRQEN & IRQ_SEROC )
	{
		p->IRQST |= IRQ_SEROC;
		if( p->interrupt_cb )
			(*p->interrupt_cb)(p->device, IRQ_SEROC);
	}
}

static TIMER_CALLBACK( pokey_pot_trigger )
{
	pokey_state *p = (pokey_state *)ptr;
	int pot = param;
	LOG(("POKEY #%p POT%d triggers after %dus\n", p, pot, (int)(1000000 * p->ptimer[pot]->elapsed().as_double())));
	p->ALLPOT &= ~(1 << pot);	/* set the enabled timer irq status bits */
}

#define AD_TIME  ((p->SKCTL & SK_PADDLE) ? p->ad_time_fast : p->ad_time_slow)

static void pokey_potgo(pokey_state *p)
{
    int pot;

	LOG(("POKEY #%p pokey_potgo\n", p));

    p->ALLPOT = 0xff;

    for( pot = 0; pot < 8; pot++ )
	{
		p->POTx[pot] = 0xff;
		if( !p->pot_r[pot].isnull() )
		{
			int r = p->pot_r[pot](pot);

			LOG(("POKEY %s pot_r(%d) returned $%02x\n", p->device->tag(), pot, r));
			if( r != -1 )
			{
				if (r > 228)
                    r = 228;

                /* final value */
                p->POTx[pot] = r;
				p->ptimer[pot]->adjust(AD_TIME * r, pot);
			}
		}
	}
}

READ8_DEVICE_HANDLER( pokey_r )
{
	pokey_state *p = get_safe_token(device);
	int data = 0, pot;
	UINT32 adjust = 0;

	switch (offset & 15)
	{
	case POT0_C: case POT1_C: case POT2_C: case POT3_C:
	case POT4_C: case POT5_C: case POT6_C: case POT7_C:
		pot = offset & 7;
		if( !p->pot_r[pot].isnull() )
		{
			/*
             * If the conversion is not yet finished (ptimer running),
             * get the current value by the linear interpolation of
             * the final value using the elapsed time.
             */
			if( p->ALLPOT & (1 << pot) )
			{
				data = p->ptimer[pot]->elapsed().attoseconds / AD_TIME.attoseconds;
				LOG(("POKEY '%s' read POT%d (interpolated) $%02x\n", p->device->tag(), pot, data));
            }
			else
			{
				data = p->POTx[pot];
				LOG(("POKEY '%s' read POT%d (final value)  $%02x\n", p->device->tag(), pot, data));
			}
		}
		else
			logerror("%s: warning - read '%s' POT%d\n", p->device->machine().describe_context(), p->device->tag(), pot);
		break;

    case ALLPOT_C:
		/****************************************************************
         * If the 2 least significant bits of SKCTL are 0, the ALLPOTs
         * are disabled (SKRESET). Thanks to MikeJ for pointing this out.
         ****************************************************************/
		if( (p->SKCTL & SK_RESET) == 0)
		{
			data = 0;
			LOG(("POKEY '%s' ALLPOT internal $%02x (reset)\n", p->device->tag(), data));
		}
		else if( !p->allpot_r.isnull() )
		{
			data = p->allpot_r(offset);
			LOG(("POKEY '%s' ALLPOT callback $%02x\n", p->device->tag(), data));
		}
		else
		{
			data = p->ALLPOT;
			LOG(("POKEY '%s' ALLPOT internal $%02x\n", p->device->tag(), data));
		}
		break;

	case KBCODE_C:
		data = p->KBCODE;
		break;

	case RANDOM_C:
		/****************************************************************
         * If the 2 least significant bits of SKCTL are 0, the random
         * number generator is disabled (SKRESET). Thanks to Eric Smith
         * for pointing out this critical bit of info! If the random
         * number generator is enabled, get a new random number. Take
         * the time gone since the last read into account and read the
         * new value from an appropriate offset in the rand17 table.
         ****************************************************************/
		if( p->SKCTL & SK_RESET )
		{
			adjust = p->rtimer->elapsed().as_double() / p->clock_period.as_double();
			p->r9 = (p->r9 + adjust) % 0x001ff;
			p->r17 = (p->r17 + adjust) % 0x1ffff;
		}
		else
		{
			adjust = 1;
			p->r9 = 0;
			p->r17 = 0;
            LOG_RAND(("POKEY '%s' rand17 frozen (SKCTL): $%02x\n", p->device->tag(), p->RANDOM));
		}
		if( p->AUDCTL & POLY9 )
		{
			p->RANDOM = p->rand9[p->r9];
			LOG_RAND(("POKEY '%s' adjust %u rand9[$%05x]: $%02x\n", p->device->tag(), adjust, p->r9, p->RANDOM));
		}
		else
		{
			p->RANDOM = p->rand17[p->r17];
			LOG_RAND(("POKEY '%s' adjust %u rand17[$%05x]: $%02x\n", p->device->tag(), adjust, p->r17, p->RANDOM));
		}
		if (adjust > 0)
			p->rtimer->adjust(attotime::never);
		data = p->RANDOM ^ 0xff;
		break;

	case SERIN_C:
		if( !p->serin_r.isnull() )
			p->SERIN = p->serin_r(offset);
		data = p->SERIN;
		LOG(("POKEY '%s' SERIN  $%02x\n", p->device->tag(), data));
		break;

	case IRQST_C:
		/* IRQST is an active low input port; we keep it active high */
		/* internally to ease the (un-)masking of bits */
		data = p->IRQST ^ 0xff;
		LOG(("POKEY '%s' IRQST  $%02x\n", p->device->tag(), data));
		break;

	case SKSTAT_C:
		/* SKSTAT is also an active low input port */
		data = p->SKSTAT ^ 0xff;
		LOG(("POKEY '%s' SKSTAT $%02x\n", p->device->tag(), data));
		break;

	default:
		LOG(("POKEY '%s' register $%02x\n", p->device->tag(), offset));
        break;
    }
    return data;
}

READ8_HANDLER( quad_pokey_r )
{
	static const char *const devname[4] = { "pokey1", "pokey2", "pokey3", "pokey4" };
	int pokey_num = (offset >> 3) & ~0x04;
	int control = (offset & 0x20) >> 2;
	int pokey_reg = (offset % 8) | control;

	return pokey_r(space->machine().device(devname[pokey_num]), pokey_reg);
}


WRITE8_DEVICE_HANDLER( pokey_w )
{
	pokey_state *p = get_safe_token(device);
	int ch_mask = 0, new_val;

	p->channel->update();

    /* determine which address was changed */
	switch (offset & 15)
    {
    case AUDF1_C:
		if( data == p->AUDF[CHAN1] )
            return;
		LOG_SOUND(("POKEY '%s' AUDF1  $%02x\n", p->device->tag(), data));
		p->AUDF[CHAN1] = data;
        ch_mask = 1 << CHAN1;
		if( p->AUDCTL & CH12_JOINED )		/* if ch 1&2 tied together */
            ch_mask |= 1 << CHAN2;    /* then also change on ch2 */
        break;

    case AUDC1_C:
		if( data == p->AUDC[CHAN1] )
            return;
		LOG_SOUND(("POKEY '%s' AUDC1  $%02x (%s)\n", p->device->tag(), data, audc2str(data)));
		p->AUDC[CHAN1] = data;
        ch_mask = 1 << CHAN1;
        break;

    case AUDF2_C:
		if( data == p->AUDF[CHAN2] )
            return;
		LOG_SOUND(("POKEY '%s' AUDF2  $%02x\n", p->device->tag(), data));
		p->AUDF[CHAN2] = data;
        ch_mask = 1 << CHAN2;
        break;

    case AUDC2_C:
		if( data == p->AUDC[CHAN2] )
            return;
		LOG_SOUND(("POKEY '%s' AUDC2  $%02x (%s)\n", p->device->tag(), data, audc2str(data)));
		p->AUDC[CHAN2] = data;
        ch_mask = 1 << CHAN2;
        break;

    case AUDF3_C:
		if( data == p->AUDF[CHAN3] )
            return;
		LOG_SOUND(("POKEY '%s' AUDF3  $%02x\n", p->device->tag(), data));
		p->AUDF[CHAN3] = data;
        ch_mask = 1 << CHAN3;

		if( p->AUDCTL & CH34_JOINED )	/* if ch 3&4 tied together */
            ch_mask |= 1 << CHAN4;  /* then also change on ch4 */
        break;

    case AUDC3_C:
		if( data == p->AUDC[CHAN3] )
            return;
		LOG_SOUND(("POKEY '%s' AUDC3  $%02x (%s)\n", p->device->tag(), data, audc2str(data)));
		p->AUDC[CHAN3] = data;
        ch_mask = 1 << CHAN3;
        break;

    case AUDF4_C:
		if( data == p->AUDF[CHAN4] )
            return;
		LOG_SOUND(("POKEY '%s' AUDF4  $%02x\n", p->device->tag(), data));
		p->AUDF[CHAN4] = data;
        ch_mask = 1 << CHAN4;
        break;

    case AUDC4_C:
		if( data == p->AUDC[CHAN4] )
            return;
		LOG_SOUND(("POKEY '%s' AUDC4  $%02x (%s)\n", p->device->tag(), data, audc2str(data)));
		p->AUDC[CHAN4] = data;
        ch_mask = 1 << CHAN4;
        break;

    case AUDCTL_C:
		if( data == p->AUDCTL )
            return;
		LOG_SOUND(("POKEY '%s' AUDCTL $%02x (%s)\n", p->device->tag(), data, audctl2str(data)));
		p->AUDCTL = data;
        ch_mask = 15;       /* all channels */
        /* determine the base multiplier for the 'div by n' calculations */
		p->clockmult = (p->AUDCTL & CLK_15KHZ) ? DIV_15 : DIV_64;
        break;

    case STIMER_C:
        /* first remove any existing timers */
		LOG_TIMER(("POKEY '%s' STIMER $%02x\n", p->device->tag(), data));

		p->timer[TIMER1]->adjust(attotime::never, p->timer_param[TIMER1]);
		p->timer[TIMER2]->adjust(attotime::never, p->timer_param[TIMER2]);
		p->timer[TIMER4]->adjust(attotime::never, p->timer_param[TIMER4]);

        /* reset all counters to zero (side effect) */
		p->counter[CHAN1] = 0;
		p->counter[CHAN2] = 0;
		p->counter[CHAN3] = 0;
		p->counter[CHAN4] = 0;
		/* From the pokey documentation */
		p->output[CHAN1] = 1;
		p->output[CHAN2] = 1;
		p->output[CHAN3] = 0;
		p->output[CHAN4] = 0;

        /* joined chan#1 and chan#2 ? */
		if( p->AUDCTL & CH12_JOINED )
        {
			if( p->divisor[CHAN2] > 4 )
			{
				LOG_TIMER(("POKEY '%s' timer1+2 after %d clocks\n", p->device->tag(), p->divisor[CHAN2]));
				/* set timer #1 _and_ #2 event after timer_div clocks of joined CHAN1+CHAN2 */
				p->timer_period[TIMER2] = p->clock_period * p->divisor[CHAN2];
				p->timer_param[TIMER2] = IRQ_TIMR2|IRQ_TIMR1;
				p->timer[TIMER2]->adjust(p->timer_period[TIMER2], p->timer_param[TIMER2], p->timer_period[TIMER2]);
			}
        }
        else
        {
			if( p->divisor[CHAN1] > 4 )
			{
				LOG_TIMER(("POKEY '%s' timer1 after %d clocks\n", p->device->tag(), p->divisor[CHAN1]));
				/* set timer #1 event after timer_div clocks of CHAN1 */
				p->timer_period[TIMER1] = p->clock_period * p->divisor[CHAN1];
				p->timer_param[TIMER1] = IRQ_TIMR1;
				p->timer[TIMER1]->adjust(p->timer_period[TIMER1], p->timer_param[TIMER1], p->timer_period[TIMER1]);
			}

			if( p->divisor[CHAN2] > 4 )
			{
				LOG_TIMER(("POKEY '%s' timer2 after %d clocks\n", p->device->tag(), p->divisor[CHAN2]));
				/* set timer #2 event after timer_div clocks of CHAN2 */
				p->timer_period[TIMER2] = p->clock_period * p->divisor[CHAN2];
				p->timer_param[TIMER2] = IRQ_TIMR2;
				p->timer[TIMER2]->adjust(p->timer_period[TIMER2], p->timer_param[TIMER2], p->timer_period[TIMER2]);
			}
        }

		/* Note: p[chip] does not have a timer #3 */

		if( p->AUDCTL & CH34_JOINED )
        {
            /* not sure about this: if audc4 == 0000xxxx don't start timer 4 ? */
			if( p->AUDC[CHAN4] & 0xf0 )
            {
				if( p->divisor[CHAN4] > 4 )
				{
					LOG_TIMER(("POKEY '%s' timer4 after %d clocks\n", p->device->tag(), p->divisor[CHAN4]));
					/* set timer #4 event after timer_div clocks of CHAN4 */
					p->timer_period[TIMER4] = p->clock_period * p->divisor[CHAN4];
					p->timer_param[TIMER4] = IRQ_TIMR4;
					p->timer[TIMER4]->adjust(p->timer_period[TIMER4], p->timer_param[TIMER4], p->timer_period[TIMER4]);
				}
            }
        }
        else
        {
			if( p->divisor[CHAN4] > 4 )
			{
				LOG_TIMER(("POKEY '%s' timer4 after %d clocks\n", p->device->tag(), p->divisor[CHAN4]));
				/* set timer #4 event after timer_div clocks of CHAN4 */
				p->timer_period[TIMER4] = p->clock_period * p->divisor[CHAN4];
				p->timer_param[TIMER4] = IRQ_TIMR4;
				p->timer[TIMER4]->adjust(p->timer_period[TIMER4], p->timer_param[TIMER4], p->timer_period[TIMER4]);
			}
        }

		p->timer[TIMER1]->enable(p->IRQEN & IRQ_TIMR1);
		p->timer[TIMER2]->enable(p->IRQEN & IRQ_TIMR2);
		p->timer[TIMER4]->enable(p->IRQEN & IRQ_TIMR4);
        break;

    case SKREST_C:
        /* reset SKSTAT */
		LOG(("POKEY '%s' SKREST $%02x\n", p->device->tag(), data));
		p->SKSTAT &= ~(SK_FRAME|SK_OVERRUN|SK_KBERR);
        break;

    case POTGO_C:
		LOG(("POKEY '%s' POTGO  $%02x\n", p->device->tag(), data));
		pokey_potgo(p);
        break;

    case SEROUT_C:
		LOG(("POKEY '%s' SEROUT $%02x\n", p->device->tag(), data));
		p->serout_w(offset, data);
		p->SKSTAT |= SK_SEROUT;
        /*
         * These are arbitrary values, tested with some custom boot
         * loaders from Ballblazer and Escape from Fractalus
         * The real times are unknown
         */
        device->machine().scheduler().timer_set(attotime::from_usec(200), FUNC(pokey_serout_ready_cb), 0, p);
        /* 10 bits (assumption 1 start, 8 data and 1 stop bit) take how long? */
        device->machine().scheduler().timer_set(attotime::from_usec(2000), FUNC(pokey_serout_complete), 0, p);
        break;

    case IRQEN_C:
		LOG(("POKEY '%s' IRQEN  $%02x\n", p->device->tag(), data));

        /* acknowledge one or more IRQST bits ? */
		if( p->IRQST & ~data )
        {
            /* reset IRQST bits that are masked now */
			p->IRQST &= data;
        }
        else
        {
			/* enable/disable timers now to avoid unneeded
               breaking of the CPU cores for masked timers */
			if( p->timer[TIMER1] && ((p->IRQEN^data) & IRQ_TIMR1) )
				p->timer[TIMER1]->enable(data & IRQ_TIMR1);
			if( p->timer[TIMER2] && ((p->IRQEN^data) & IRQ_TIMR2) )
				p->timer[TIMER2]->enable(data & IRQ_TIMR2);
			if( p->timer[TIMER4] && ((p->IRQEN^data) & IRQ_TIMR4) )
				p->timer[TIMER4]->enable(data & IRQ_TIMR4);
        }
		/* store irq enable */
		p->IRQEN = data;
        break;

    case SKCTL_C:
		if( data == p->SKCTL )
            return;
		LOG(("POKEY '%s' SKCTL  $%02x\n", p->device->tag(), data));
		p->SKCTL = data;
        if( !(data & SK_RESET) )
        {
            pokey_w(device, IRQEN_C,  0);
            pokey_w(device, SKREST_C, 0);
        }
        break;
    }

	/************************************************************
     * As defined in the manual, the exact counter values are
     * different depending on the frequency and resolution:
     *    64 kHz or 15 kHz - AUDF + 1
     *    1.79 MHz, 8-bit  - AUDF + 4
     *    1.79 MHz, 16-bit - AUDF[CHAN1]+256*AUDF[CHAN2] + 7
     ************************************************************/

    /* only reset the channels that have changed */

    if( ch_mask & (1 << CHAN1) )
    {
        /* process channel 1 frequency */
		if( p->AUDCTL & CH1_HICLK )
			new_val = p->AUDF[CHAN1] + DIVADD_HICLK;
        else
			new_val = (p->AUDF[CHAN1] + DIVADD_LOCLK) * p->clockmult;

		LOG_SOUND(("POKEY '%s' chan1 %d\n", p->device->tag(), new_val));

		p->volume[CHAN1] = (p->AUDC[CHAN1] & VOLUME_MASK) * POKEY_DEFAULT_GAIN;
        p->divisor[CHAN1] = new_val;
		if( p->interrupt_cb && p->timer[TIMER1] )
			p->timer[TIMER1]->adjust(p->clock_period * new_val, p->timer_param[TIMER1], p->timer_period[TIMER1]);
    }

    if( ch_mask & (1 << CHAN2) )
    {
        /* process channel 2 frequency */
		if( p->AUDCTL & CH12_JOINED )
        {
			if( p->AUDCTL & CH1_HICLK )
				new_val = p->AUDF[CHAN2] * 256 + p->AUDF[CHAN1] + DIVADD_HICLK_JOINED;
            else
				new_val = (p->AUDF[CHAN2] * 256 + p->AUDF[CHAN1] + DIVADD_LOCLK) * p->clockmult;
			LOG_SOUND(("POKEY '%s' chan1+2 %d\n", p->device->tag(), new_val));
        }
        else
		{
			new_val = (p->AUDF[CHAN2] + DIVADD_LOCLK) * p->clockmult;
			LOG_SOUND(("POKEY '%s' chan2 %d\n", p->device->tag(), new_val));
		}

		p->volume[CHAN2] = (p->AUDC[CHAN2] & VOLUME_MASK) * POKEY_DEFAULT_GAIN;
		p->divisor[CHAN2] = new_val;
		if( p->interrupt_cb && p->timer[TIMER2] )
			p->timer[TIMER2]->adjust(p->clock_period * new_val, p->timer_param[TIMER2], p->timer_period[TIMER2]);
    }

    if( ch_mask & (1 << CHAN3) )
    {
        /* process channel 3 frequency */
		if( p->AUDCTL & CH3_HICLK )
			new_val = p->AUDF[CHAN3] + DIVADD_HICLK;
        else
			new_val = (p->AUDF[CHAN3] + DIVADD_LOCLK) * p->clockmult;

		LOG_SOUND(("POKEY '%s' chan3 %d\n", p->device->tag(), new_val));

		p->volume[CHAN3] = (p->AUDC[CHAN3] & VOLUME_MASK) * POKEY_DEFAULT_GAIN;
		p->divisor[CHAN3] = new_val;
		/* channel 3 does not have a timer associated */
    }

    if( ch_mask & (1 << CHAN4) )
    {
        /* process channel 4 frequency */
		if( p->AUDCTL & CH34_JOINED )
        {
			if( p->AUDCTL & CH3_HICLK )
				new_val = p->AUDF[CHAN4] * 256 + p->AUDF[CHAN3] + DIVADD_HICLK_JOINED;
            else
				new_val = (p->AUDF[CHAN4] * 256 + p->AUDF[CHAN3] + DIVADD_LOCLK) * p->clockmult;
			LOG_SOUND(("POKEY '%s' chan3+4 %d\n", p->device->tag(), new_val));
        }
        else
		{
			new_val = (p->AUDF[CHAN4] + DIVADD_LOCLK) * p->clockmult;
			LOG_SOUND(("POKEY '%s' chan4 %d\n", p->device->tag(), new_val));
		}

		p->volume[CHAN4] = (p->AUDC[CHAN4] & VOLUME_MASK) * POKEY_DEFAULT_GAIN;
		p->divisor[CHAN4] = new_val;
		if( p->interrupt_cb && p->timer[TIMER4] )
			p->timer[TIMER4]->adjust(p->clock_period * new_val, p->timer_param[TIMER4], p->timer_period[TIMER4]);
    }
}

WRITE8_HANDLER( quad_pokey_w )
{
	static const char *const devname[4] = { "pokey1", "pokey2", "pokey3", "pokey4" };
    int pokey_num = (offset >> 3) & ~0x04;
    int control = (offset & 0x20) >> 2;
    int pokey_reg = (offset % 8) | control;

    pokey_w(space->machine().device(devname[pokey_num]), pokey_reg, data);
}

void pokey_serin_ready(device_t *device, int after)
{
	pokey_state *p = get_safe_token(device);
	device->machine().scheduler().timer_set(p->clock_period * after, FUNC(pokey_serin_ready_cb), 0, p);
}

void pokey_break_w(device_t *device, int shift)
{
	pokey_state *p = get_safe_token(device);
	if( shift )                     /* shift code ? */
		p->SKSTAT |= SK_SHIFT;
	else
		p->SKSTAT &= ~SK_SHIFT;
	/* check if the break IRQ is enabled */
	if( p->IRQEN & IRQ_BREAK )
	{
		/* set break IRQ status and call back the interrupt handler */
		p->IRQST |= IRQ_BREAK;
		if( p->interrupt_cb )
			(*p->interrupt_cb)(device, IRQ_BREAK);
	}
}

void pokey_kbcode_w(device_t *device, int kbcode, int make)
{
	pokey_state *p = get_safe_token(device);
    /* make code ? */
	if( make )
	{
		p->KBCODE = kbcode;
		p->SKSTAT |= SK_KEYBD;
		if( kbcode & 0x40 ) 		/* shift code ? */
			p->SKSTAT |= SK_SHIFT;
		else
			p->SKSTAT &= ~SK_SHIFT;

		if( p->IRQEN & IRQ_KEYBD )
		{
			/* last interrupt not acknowledged ? */
			if( p->IRQST & IRQ_KEYBD )
				p->SKSTAT |= SK_KBERR;
			p->IRQST |= IRQ_KEYBD;
			if( p->interrupt_cb )
				(*p->interrupt_cb)(device, IRQ_KEYBD);
		}
	}
	else
	{
		p->KBCODE = kbcode;
		p->SKSTAT &= ~SK_KEYBD;
    }
}




/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( pokey )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(pokey_state);		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( pokey );			break;
		case DEVINFO_FCT_STOP:							/* Nothing */									break;
		case DEVINFO_FCT_RESET:							/* Nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "POKEY");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Atari custom");				break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "4.6");						break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(POKEY, pokey);
