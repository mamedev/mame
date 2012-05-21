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
#define VERBOSE_SOUND	0
#define VERBOSE_TIMER	1
#define VERBOSE_POLY	0
#define VERBOSE_RAND	0

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

#define P4(chip)  chip->poly4[chip->p4]
#define P5(chip)  chip->poly5[chip->p5]
#define P9(chip)  chip->poly9[chip->p9]
#define P17(chip) chip->poly17[chip->p17]

#define CLK_1 0
#define CLK_28 1
#define CLK_114 2

static const int clock_divisors[3] = {1, 28, 114};




// device type definition
const device_type POKEYN = &device_creator<pokeyn_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  okim9810_device - constructor
//-------------------------------------------------

pokeyn_device::pokeyn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, POKEYN, "POKEYN", tag, owner, clock),
	  device_sound_interface(mconfig, *this),
#ifdef POKEY_EXEC_INTERFACE
	  device_execute_interface(mconfig, *this),
#endif
	  m_icount(0),
	  m_stream(NULL)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pokeyn_device::device_start()
{
	//int sample_rate = clock();
	int i;

	if (static_config())
		memcpy(&m_intf, static_config(), sizeof(pokey_interface));
	m_clock_period = attotime::from_hz(clock());

	/* calculate the A/D times
     * In normal, slow mode (SKCTL bit SK_PADDLE is clear) the conversion
     * takes N scanlines, where N is the paddle value. A single scanline
     * takes approximately 64us to finish (1.78979MHz clock).
     * In quick mode (SK_PADDLE set) the conversion is done very fast
     * (takes two scanlines) but the result is not as accurate.
     */

	m_ad_time_fast = (attotime::from_nsec(64000*2/228) * FREQ_17_EXACT) / clock();
	m_ad_time_slow = (attotime::from_nsec(64000      ) * FREQ_17_EXACT) / clock();

	/* initialize the poly counters */
	poly_init_4_5(m_poly4, 4, 1, 0);
	poly_init_4_5(m_poly5, 5, 2, 1);
	//poly_init(m_poly9,   9, 8, 1, 0x00180);
	//poly_init(m_poly17, 17,16, 1, 0x1c000);

	/* initialize 9 / 17 arrays */
	poly_init_9_17(m_poly9,   9);
	poly_init_9_17(m_poly17, 17);

	m_divisor[CHAN1] = 4;
	m_divisor[CHAN2] = 4;
	m_divisor[CHAN3] = 4;
	m_divisor[CHAN4] = 4;
	m_clockmult = DIV_64;
	m_KBCODE = 0x09;		 /* Atari 800 'no key' */
	m_SKCTL = SK_RESET;	 /* let the RNG run after reset */
	m_rtimer = timer_alloc(0);

	m_timer[0] = timer_alloc(1);
	m_timer[1] = timer_alloc(1);
	m_timer[2] = timer_alloc(1);

	/* reset more internal state */
	for (i=0; i<3; i++)
	{
		m_clock_cnt[i] = 0;
	}

	for (i=0; i<8; i++)
	{
		m_ptimer[i] = timer_alloc(2);
		m_pot_r[i].resolve(m_intf.pot_r[i], *this);
	}
	m_allpot_r.resolve(m_intf.allpot_r, *this);
	m_serin_r.resolve(m_intf.serin_r, *this);
	m_serout_w.resolve(m_intf.serout_w, *this);
	m_interrupt_cb = m_intf.interrupt_cb;

	m_stream = stream_alloc(0, 1, clock());

	for (i=0; i<POKEY_CHANNELS; i++)
	{
		save_item(NAME(m_channel[i].m_borrow_cnt), i);
		save_item(NAME(m_channel[i].m_counter), i);
		save_item(NAME(m_channel[i].m_filter_sample), i);
		save_item(NAME(m_channel[i].m_output), i);
		save_item(NAME(m_channel[i].m_volume), i);
		save_item(NAME(m_channel[i].m_AUDF), i);
		save_item(NAME(m_channel[i].m_AUDC), i);
	}
	save_item(NAME(m_divisor));
	save_item(NAME(m_clock_cnt));
	save_item(NAME(m_p4));
	save_item(NAME(m_p5));
	save_item(NAME(m_p9));
	save_item(NAME(m_p17));
	save_item(NAME(m_r9));
	save_item(NAME(m_r17));
	save_item(NAME(m_clockmult));
	save_item(NAME(m_timer_period[0]));
	save_item(NAME(m_timer_period[1]));
	save_item(NAME(m_timer_period[2]));
	save_item(NAME(m_timer_param));
	save_item(NAME(m_POTx));
	save_item(NAME(m_AUDCTL));
	save_item(NAME(m_ALLPOT));
	save_item(NAME(m_KBCODE));
	save_item(NAME(m_RANDOM));
	save_item(NAME(m_SERIN));
	save_item(NAME(m_SEROUT));
	save_item(NAME(m_IRQST));
	save_item(NAME(m_IRQEN));
	save_item(NAME(m_SKSTAT));
	save_item(NAME(m_SKCTL));

	// set our instruction counter
#ifdef POKEY_EXEC_INTERFACE
	m_icountptr = &m_icount;
#endif

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pokeyn_device::device_reset()
{
	m_stream->update();
}


//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void pokeyn_device::device_post_load()
{
}


//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

void pokeyn_device::device_clock_changed()
{
}

//-------------------------------------------------
//  stream_generate - handle update requests for
//  our sound stream
//-------------------------------------------------

void pokeyn_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == 1)
	{
		int timers = param;

		LOG_TIMER(("POKEY #%p timer %d with IRQEN $%02x\n", this, timers, m_IRQEN));

	    /* check if some of the requested timer interrupts are enabled */
		timers &= m_IRQEN;

	    if( timers )
	    {
			/* set the enabled timer irq status bits */
			m_IRQST |= timers;
	        /* call back an application supplied function to handle the interrupt */
			if( m_interrupt_cb )
				(*m_interrupt_cb)(this, timers);
	    }
	}
	else if (id == 2)
	{
		int pot = param;

		LOG(("POKEY #%p POT%d triggers after %dus\n", this, pot, (int)(1000000 * m_ptimer[pot]->elapsed().as_double())));
		m_ALLPOT &= ~(1 << pot);	/* set the enabled timer irq status bits */

	}
	else if (id == 3)
	{
		/* serout_ready_cb */
	    if( m_IRQEN & IRQ_SEROR )
		{
			m_IRQST |= IRQ_SEROR;
			if( m_interrupt_cb )
				(m_interrupt_cb)(this, IRQ_SEROR);
		}
	}
	else if (id == 4)
	{
		/* serout_complete */
	    if( m_IRQEN & IRQ_SEROC )
		{
			m_IRQST |= IRQ_SEROC;
			if( m_interrupt_cb )
				(m_interrupt_cb)(this, IRQ_SEROC);
		}
	}
	else if (id == 5)
	{
		/* serin_ready */
	    if( m_IRQEN & IRQ_SERIN )
		{
			/* set the enabled timer irq status bits */
			m_IRQST |= IRQ_SERIN;
			/* call back an application supplied function to handle the interrupt */
			if( m_interrupt_cb )
				(m_interrupt_cb)(this, IRQ_SERIN);
		}
	}
	else
		assert_always(FALSE, "Unknown id in pokey_device::device_timer");
}

//-------------------------------------------------
//  stream_generate - handle update requests for
//  our sound stream
//-------------------------------------------------

/*
 * http://www.atariage.com/forums/topic/3328-sio-protocol/page__st__100#entry1680190:
 * I noticed that the Pokey counters have clocked carry (actually, "borrow") positions that delay the
 * counter by 3 cycles, plus the 1 reset clock. So 16 bit mode has 6 carry delays and a reset clock.
 * I'm sure this was done because the propagation delays limited the number of cells the subtraction could ripple though.
 *
 */

void pokeyn_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *buffer = outputs[0];
	int base_clock = (m_AUDCTL & CLK_15KHZ) ? CLK_114 : CLK_28;

	while( samples > 0 )
	{
		int ch, clk;
		UINT32 sum = 0;
		int clock_triggered[3] = {0,0,0};

		for (clk = 0; clk < 3; clk++)
		{
			m_clock_cnt[clk]++;
			if (m_clock_cnt[clk] >= clock_divisors[clk])
			{
				m_clock_cnt[clk] = 0;
				clock_triggered[clk] = 1;
			}
		}

		m_p4 = (m_p4 + 1) % 0x0000f;
		m_p5 = (m_p5 + 1) % 0x0001f;
		m_p9 = (m_p9 + 1) % 0x001ff;
		m_p17 = (m_p17 + 1 ) % 0x1ffff;


		clk = (m_AUDCTL & CH1_HICLK) ? CLK_1 : base_clock;
		if (clock_triggered[clk])
			m_channel[CHAN1].inc_chan();

		clk = (m_AUDCTL & CH3_HICLK) ? CLK_1 : base_clock;
		if (clock_triggered[clk])
			m_channel[CHAN3].inc_chan();

		if (clock_triggered[base_clock])
		{
			if (!(m_AUDCTL & CH12_JOINED))
				m_channel[CHAN2].inc_chan();
			if (!(m_AUDCTL & CH34_JOINED))
				m_channel[CHAN4].inc_chan();
		}

		/* do CHAN2 before CHAN1 because CHAN1 may set borrow! */
		if (m_channel[CHAN2].check_borrow())
		{
			int isJoined = (m_AUDCTL & CH12_JOINED);
			if (isJoined)
				m_channel[CHAN1].reset_channel();
			m_channel[CHAN2].reset_channel();
			process_channel(CHAN2);
		}

		if (m_channel[CHAN1].check_borrow())
		{
			int isJoined = (m_AUDCTL & CH12_JOINED);
			if (isJoined)
				m_channel[CHAN2].inc_chan();
			else
				m_channel[CHAN1].reset_channel();
			process_channel(CHAN1);
		}

		/* do CHAN4 before CHAN3 because CHAN3 may set borrow! */
		if (m_channel[CHAN4].check_borrow())
		{
			int isJoined = (m_AUDCTL & CH34_JOINED);
			if (isJoined)
				m_channel[CHAN3].reset_channel();
			m_channel[CHAN4].reset_channel();
			process_channel(CHAN4);
			/* is this a filtering channel (3/4) and is the filter active? */
			if (m_AUDCTL & CH2_FILTER)
				m_channel[CHAN2].sample();
			else
				m_channel[CHAN2].m_filter_sample = 1;
		}

		if (m_channel[CHAN3].check_borrow())
		{
			int isJoined = (m_AUDCTL & CH34_JOINED);
			if (isJoined)
				m_channel[CHAN4].inc_chan();
			else
				m_channel[CHAN3].reset_channel();
			process_channel(CHAN3);
			/* is this a filtering channel (3/4) and is the filter active? */
			if (m_AUDCTL & CH1_FILTER)
				m_channel[CHAN1].sample();
			else
				m_channel[CHAN1].m_filter_sample = 1;
		}

		for (ch = 0; ch < 4; ch++)
		{
			sum += (((m_channel[ch].m_output ^ m_channel[ch].m_filter_sample) || (m_channel[ch].m_AUDC & VOLUME_ONLY)) ? m_channel[ch].m_volume : 0 );
		}

    	/* store sum of output signals into the buffer */

    	*buffer++ = (sum > 0x7fff) ? 0x7fff : sum;
    	samples--;

	}
	m_rtimer->adjust(attotime::never);

}



//-------------------------------------------------
//  read - memory interface for reading the active status
//-------------------------------------------------

READ8_MEMBER( pokeyn_device::read )
{
	return read(offset);
}

UINT8 pokeyn_device::read(offs_t offset)
{
	int data = 0, pot;
	UINT32 adjust = 0;

	switch (offset & 15)
	{
	case POT0_C: case POT1_C: case POT2_C: case POT3_C:
	case POT4_C: case POT5_C: case POT6_C: case POT7_C:
		pot = offset & 7;
		if( !m_pot_r[pot].isnull() )
		{
			/*
             * If the conversion is not yet finished (ptimer running),
             * get the current value by the linear interpolation of
             * the final value using the elapsed time.
             */
			if( m_ALLPOT & (1 << pot) )
			{
				data = m_ptimer[pot]->elapsed().attoseconds / ((m_SKCTL & SK_PADDLE) ? m_ad_time_fast : m_ad_time_slow).attoseconds;
				LOG(("POKEY '%s' read POT%d (interpolated) $%02x\n", tag(), pot, data));
            }
			else
			{
				data = m_POTx[pot];
				LOG(("POKEY '%s' read POT%d (final value)  $%02x\n", tag(), pot, data));
			}
		}
		else
			logerror("%s: warning - read '%s' POT%d\n", machine().describe_context(), tag(), pot);
		break;

    case ALLPOT_C:
		/****************************************************************
         * If the 2 least significant bits of SKCTL are 0, the ALLPOTs
         * are disabled (SKRESET). Thanks to MikeJ for pointing this out.
         ****************************************************************/
		if( (m_SKCTL & SK_RESET) == 0)
		{
			data = 0;
			LOG(("POKEY '%s' ALLPOT internal $%02x (reset)\n", tag(), data));
		}
		else if( !m_allpot_r.isnull() )
		{
			data = m_allpot_r(offset);
			LOG(("POKEY '%s' ALLPOT callback $%02x\n", tag(), data));
		}
		else
		{
			data = m_ALLPOT;
			LOG(("POKEY '%s' ALLPOT internal $%02x\n", tag(), data));
		}
		break;

	case KBCODE_C:
		data = m_KBCODE;
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
		if( m_SKCTL & SK_RESET )
		{
			adjust = m_rtimer->elapsed().as_double() / m_clock_period.as_double();
			m_r9 = (m_r9 + adjust) % 0x001ff;
			m_r17 = (m_r17 + adjust) % 0x1ffff;
		}
		else
		{
			adjust = 1;
			m_r9 = 0;
			m_r17 = 0;
            LOG_RAND(("POKEY '%s' rand17 frozen (SKCTL): $%02x\n", tag(), m_RANDOM));
		}
		if( m_AUDCTL & POLY9 )
		{
			m_RANDOM = m_poly9[m_r9] & 0xff;
			LOG_RAND(("POKEY '%s' adjust %u rand9[$%05x]: $%02x\n", tag(), adjust, m_r9, m_RANDOM));
		}
		else
		{
			m_RANDOM = (m_poly17[m_r17] >> 8) & 0xff;
			LOG_RAND(("POKEY '%s' adjust %u rand17[$%05x]: $%02x\n", tag(), adjust, m_r17, m_RANDOM));
		}
		if (adjust > 0)
			m_rtimer->adjust(attotime::never);
		data = m_RANDOM ^ 0xff;
		break;

	case SERIN_C:
		if( !m_serin_r.isnull() )
			m_SERIN = m_serin_r(offset);
		data = m_SERIN;
		LOG(("POKEY '%s' SERIN  $%02x\n", tag(), data));
		break;

	case IRQST_C:
		/* IRQST is an active low input port; we keep it active high */
		/* internally to ease the (un-)masking of bits */
		data = m_IRQST ^ 0xff;
		LOG(("POKEY '%s' IRQST  $%02x\n", tag(), data));
		break;

	case SKSTAT_C:
		/* SKSTAT is also an active low input port */
		data = m_SKSTAT ^ 0xff;
		LOG(("POKEY '%s' SKSTAT $%02x\n", tag(), data));
		break;

	default:
		LOG(("POKEY '%s' register $%02x\n", tag(), offset));
        break;
    }
    return data;

}


//-------------------------------------------------
//  write - memory interface for write
//-------------------------------------------------

WRITE8_MEMBER( pokeyn_device::write )
{
	write(offset, data);
}

void pokeyn_device::write(offs_t offset, UINT8 data)
{
	int ch_mask = 0, new_val;

	m_stream->update();

    /* determine which address was changed */
	switch (offset & 15)
    {
    case AUDF1_C:
		if( data == m_channel[CHAN1].m_AUDF )
            return;
		LOG_SOUND(("POKEY '%s' AUDF1  $%02x\n", tag(), data));
		m_channel[CHAN1].m_AUDF = data;
        ch_mask = 1 << CHAN1;
		if( m_AUDCTL & CH12_JOINED )		/* if ch 1&2 tied together */
            ch_mask |= 1 << CHAN2;    /* then also change on ch2 */
        break;

    case AUDC1_C:
		if( data == m_channel[CHAN1].m_AUDC )
            return;
		LOG_SOUND(("POKEY '%s' AUDC1  $%02x (%s)\n", tag(), data, audc2str(data)));
		m_channel[CHAN1].m_AUDC = data;
        ch_mask = 1 << CHAN1;
        break;

    case AUDF2_C:
		if( data == m_channel[CHAN2].m_AUDF )
            return;
		LOG_SOUND(("POKEY '%s' AUDF2  $%02x\n", tag(), data));
		m_channel[CHAN2].m_AUDF = data;
        ch_mask = 1 << CHAN2;
        break;

    case AUDC2_C:
		if( data == m_channel[CHAN2].m_AUDC )
            return;
		LOG_SOUND(("POKEY '%s' AUDC2  $%02x (%s)\n", tag(), data, audc2str(data)));
		m_channel[CHAN2].m_AUDC = data;
        ch_mask = 1 << CHAN2;
        break;

    case AUDF3_C:
		if( data == m_channel[CHAN3].m_AUDF )
            return;
		LOG_SOUND(("POKEY '%s' AUDF3  $%02x\n", tag(), data));
		m_channel[CHAN3].m_AUDF = data;
        ch_mask = 1 << CHAN3;

		if( m_AUDCTL & CH34_JOINED )	/* if ch 3&4 tied together */
            ch_mask |= 1 << CHAN4;  /* then also change on ch4 */
        break;

    case AUDC3_C:
		if( data == m_channel[CHAN3].m_AUDC )
            return;
		LOG_SOUND(("POKEY '%s' AUDC3  $%02x (%s)\n", tag(), data, audc2str(data)));
		m_channel[CHAN3].m_AUDC = data;
        ch_mask = 1 << CHAN3;
        break;

    case AUDF4_C:
		if( data == m_channel[CHAN4].m_AUDF )
            return;
		LOG_SOUND(("POKEY '%s' AUDF4  $%02x\n", tag(), data));
		m_channel[CHAN4].m_AUDF = data;
        ch_mask = 1 << CHAN4;
        break;

    case AUDC4_C:
		if( data == m_channel[CHAN4].m_AUDC )
            return;
		LOG_SOUND(("POKEY '%s' AUDC4  $%02x (%s)\n", tag(), data, audc2str(data)));
		m_channel[CHAN4].m_AUDC = data;
        ch_mask = 1 << CHAN4;
        break;

    case AUDCTL_C:
		if( data == m_AUDCTL )
            return;
		LOG_SOUND(("POKEY '%s' AUDCTL $%02x (%s)\n", tag(), data, audctl2str(data)));
		m_AUDCTL = data;
        ch_mask = 15;       /* all channels */
        /* determine the base multiplier for the 'div by n' calculations */
		m_clockmult = (m_AUDCTL & CLK_15KHZ) ? DIV_15 : DIV_64;
        break;

    case STIMER_C:
        /* first remove any existing timers */
		LOG_TIMER(("POKEY '%s' STIMER $%02x\n", tag(), data));

		m_timer[TIMER1]->adjust(attotime::never, m_timer_param[TIMER1]);
		m_timer[TIMER2]->adjust(attotime::never, m_timer_param[TIMER2]);
		m_timer[TIMER4]->adjust(attotime::never, m_timer_param[TIMER4]);

		/* From the pokey documentation */
        /* reset all counters to zero (side effect) */
		for (int i = 0; i < POKEY_CHANNELS; i++)
		{
			m_channel[i].reset_channel();
			m_channel[i].m_output = 0;
			m_channel[i].m_filter_sample = (i<2 ? 1 : 0);
		}

		/* joined chan#1 and chan#2 ? */
		if( m_AUDCTL & CH12_JOINED )
        {
			if( m_divisor[CHAN2] > 4 )
			{
				LOG_TIMER(("POKEY '%s' timer1+2 after %d clocks\n", tag(), m_divisor[CHAN2]));
				/* set timer #1 _and_ #2 event after timer_div clocks of joined CHAN1+CHAN2 */
				m_timer_period[TIMER2] = m_clock_period * m_divisor[CHAN2];
				m_timer_param[TIMER2] = IRQ_TIMR2|IRQ_TIMR1;
				m_timer[TIMER2]->adjust(m_timer_period[TIMER2], m_timer_param[TIMER2], m_timer_period[TIMER2]);
			}
        }
        else
        {
			if( m_divisor[CHAN1] > 4 )
			{
				LOG_TIMER(("POKEY '%s' timer1 after %d clocks\n", tag(), m_divisor[CHAN1]));
				/* set timer #1 event after timer_div clocks of CHAN1 */
				m_timer_period[TIMER1] = m_clock_period * m_divisor[CHAN1];
				m_timer_param[TIMER1] = IRQ_TIMR1;
				m_timer[TIMER1]->adjust(m_timer_period[TIMER1], m_timer_param[TIMER1], m_timer_period[TIMER1]);
			}

			if( m_divisor[CHAN2] > 4 )
			{
				LOG_TIMER(("POKEY '%s' timer2 after %d clocks\n", tag(), m_divisor[CHAN2]));
				/* set timer #2 event after timer_div clocks of CHAN2 */
				m_timer_period[TIMER2] = m_clock_period * m_divisor[CHAN2];
				m_timer_param[TIMER2] = IRQ_TIMR2;
				m_timer[TIMER2]->adjust(m_timer_period[TIMER2], m_timer_param[TIMER2], m_timer_period[TIMER2]);
			}
        }

		/* Note: p[chip] does not have a timer #3 */

		if( m_AUDCTL & CH34_JOINED )
        {
            /* not sure about this: if audc4 == 0000xxxx don't start timer 4 ? */
			if( m_channel[CHAN4].m_AUDC & 0xf0 )
            {
				if( m_divisor[CHAN4] > 4 )
				{
					LOG_TIMER(("POKEY '%s' timer4 after %d clocks\n", tag(), m_divisor[CHAN4]));
					/* set timer #4 event after timer_div clocks of CHAN4 */
					m_timer_period[TIMER4] = m_clock_period * m_divisor[CHAN4];
					m_timer_param[TIMER4] = IRQ_TIMR4;
					m_timer[TIMER4]->adjust(m_timer_period[TIMER4], m_timer_param[TIMER4], m_timer_period[TIMER4]);
				}
            }
        }
        else
        {
			if( m_divisor[CHAN4] > 4 )
			{
				LOG_TIMER(("POKEY '%s' timer4 after %d clocks\n", tag(), m_divisor[CHAN4]));
				/* set timer #4 event after timer_div clocks of CHAN4 */
				m_timer_period[TIMER4] = m_clock_period * m_divisor[CHAN4];
				m_timer_param[TIMER4] = IRQ_TIMR4;
				m_timer[TIMER4]->adjust(m_timer_period[TIMER4], m_timer_param[TIMER4], m_timer_period[TIMER4]);
			}
        }

		m_timer[TIMER1]->enable(m_IRQEN & IRQ_TIMR1);
		m_timer[TIMER2]->enable(m_IRQEN & IRQ_TIMR2);
		m_timer[TIMER4]->enable(m_IRQEN & IRQ_TIMR4);
        break;

    case SKREST_C:
        /* reset SKSTAT */
		LOG(("POKEY '%s' SKREST $%02x\n", tag(), data));
		m_SKSTAT &= ~(SK_FRAME|SK_OVERRUN|SK_KBERR);
        break;

    case POTGO_C:
		LOG(("POKEY '%s' POTGO  $%02x\n", tag(), data));
		pokey_potgo();
        break;

    case SEROUT_C:
		LOG(("POKEY '%s' SEROUT $%02x\n", tag(), data));
		m_serout_w(offset, data);
		m_SKSTAT |= SK_SEROUT;
        /*
         * These are arbitrary values, tested with some custom boot
         * loaders from Ballblazer and Escape from Fractalus
         * The real times are unknown
         */
        timer_set(attotime::from_usec(200), 3);
        /* 10 bits (assumption 1 start, 8 data and 1 stop bit) take how long? */
        timer_set(attotime::from_usec(2000), 4);// FUNC(pokey_serout_complete), 0, p);
        break;

    case IRQEN_C:
		LOG(("POKEY '%s' IRQEN  $%02x\n", tag(), data));

        /* acknowledge one or more IRQST bits ? */
		if( m_IRQST & ~data )
        {
            /* reset IRQST bits that are masked now */
			m_IRQST &= data;
        }
        else
        {
			/* enable/disable timers now to avoid unneeded
               breaking of the CPU cores for masked timers */
			if( m_timer[TIMER1] && ((m_IRQEN^data) & IRQ_TIMR1) )
				m_timer[TIMER1]->enable(data & IRQ_TIMR1);
			if( m_timer[TIMER2] && ((m_IRQEN^data) & IRQ_TIMR2) )
				m_timer[TIMER2]->enable(data & IRQ_TIMR2);
			if( m_timer[TIMER4] && ((m_IRQEN^data) & IRQ_TIMR4) )
				m_timer[TIMER4]->enable(data & IRQ_TIMR4);
        }
		/* store irq enable */
		m_IRQEN = data;
        break;

    case SKCTL_C:
		if( data == m_SKCTL )
            return;
		LOG(("POKEY '%s' SKCTL  $%02x\n", tag(), data));
		m_SKCTL = data;
        if( !(data & SK_RESET) )
        {
            write(IRQEN_C,  0);
            write(SKREST_C, 0);
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
		if( m_AUDCTL & CH1_HICLK )
			new_val = m_channel[CHAN1].m_AUDF + DIVADD_HICLK;
        else
			new_val = (m_channel[CHAN1].m_AUDF + DIVADD_LOCLK) * m_clockmult;

		LOG_SOUND(("POKEY '%s' chan1 %d\n", tag(), new_val));

		m_channel[CHAN1].m_volume = (m_channel[CHAN1].m_AUDC & VOLUME_MASK) * POKEY_DEFAULT_GAIN;
        m_divisor[CHAN1] = new_val;
		if( m_interrupt_cb && m_timer[TIMER1] )
			m_timer[TIMER1]->adjust(m_clock_period * new_val, m_timer_param[TIMER1], m_timer_period[TIMER1]);
    }

    if( ch_mask & (1 << CHAN2) )
    {
        /* process channel 2 frequency */
		if( m_AUDCTL & CH12_JOINED )
        {
			if( m_AUDCTL & CH1_HICLK )
				new_val = m_channel[CHAN2].m_AUDF * 256 + m_channel[CHAN1].m_AUDF + DIVADD_HICLK_JOINED;
            else
				new_val = (m_channel[CHAN2].m_AUDF * 256 + m_channel[CHAN1].m_AUDF + DIVADD_LOCLK) * m_clockmult;
			LOG_SOUND(("POKEY '%s' chan1+2 %d\n", tag(), new_val));
        }
        else
		{
			new_val = (m_channel[CHAN2].m_AUDF + DIVADD_LOCLK) * m_clockmult;
			LOG_SOUND(("POKEY '%s' chan2 %d\n", tag(), new_val));
		}

		m_channel[CHAN2].m_volume = (m_channel[CHAN2].m_AUDC & VOLUME_MASK) * POKEY_DEFAULT_GAIN;
		m_divisor[CHAN2] = new_val;
		if( m_interrupt_cb && m_timer[TIMER2] )
			m_timer[TIMER2]->adjust(m_clock_period * new_val, m_timer_param[TIMER2], m_timer_period[TIMER2]);
    }

    if( ch_mask & (1 << CHAN3) )
    {
        /* process channel 3 frequency */
		if( m_AUDCTL & CH3_HICLK )
			new_val = m_channel[CHAN3].m_AUDF + DIVADD_HICLK;
        else
			new_val = (m_channel[CHAN3].m_AUDF + DIVADD_LOCLK) * m_clockmult;

		LOG_SOUND(("POKEY '%s' chan3 %d\n", tag(), new_val));

		m_channel[CHAN3].m_volume = (m_channel[CHAN3].m_AUDC & VOLUME_MASK) * POKEY_DEFAULT_GAIN;
		m_divisor[CHAN3] = new_val;
		/* channel 3 does not have a timer associated */
    }

    if( ch_mask & (1 << CHAN4) )
    {
        /* process channel 4 frequency */
		if( m_AUDCTL & CH34_JOINED )
        {
			if( m_AUDCTL & CH3_HICLK )
				new_val = m_channel[CHAN4].m_AUDF * 256 + m_channel[CHAN3].m_AUDF + DIVADD_HICLK_JOINED;
            else
				new_val = (m_channel[CHAN4].m_AUDF * 256 + m_channel[CHAN3].m_AUDF + DIVADD_LOCLK) * m_clockmult;
			LOG_SOUND(("POKEY '%s' chan3+4 %d\n", tag(), new_val));
        }
        else
		{
			new_val = (m_channel[CHAN4].m_AUDF + DIVADD_LOCLK) * m_clockmult;
			LOG_SOUND(("POKEY '%s' chan4 %d\n", tag(), new_val));
		}

		m_channel[CHAN4].m_volume = (m_channel[CHAN4].m_AUDC & VOLUME_MASK) * POKEY_DEFAULT_GAIN;
		m_divisor[CHAN4] = new_val;
		if( m_interrupt_cb && m_timer[TIMER4] )
			m_timer[TIMER4]->adjust(m_clock_period * new_val, m_timer_param[TIMER4], m_timer_period[TIMER4]);
    }
}

void pokeyn_device::serin_ready(int after)
{
	timer_set(m_clock_period * after, 5, 0);
}

void pokeyn_device::break_w(int shift)
{
	if( shift )                     /* shift code ? */
		m_SKSTAT |= SK_SHIFT;
	else
		m_SKSTAT &= ~SK_SHIFT;
	/* check if the break IRQ is enabled */
	if( m_IRQEN & IRQ_BREAK )
	{
		/* set break IRQ status and call back the interrupt handler */
		m_IRQST |= IRQ_BREAK;
		if( m_interrupt_cb )
			(*m_interrupt_cb)(this, IRQ_BREAK);
	}
}

void pokeyn_device::kbcode_w(int kbcode, int make)
{
    /* make code ? */
	if( make )
	{
		m_KBCODE = kbcode;
		m_SKSTAT |= SK_KEYBD;
		if( kbcode & 0x40 ) 		/* shift code ? */
			m_SKSTAT |= SK_SHIFT;
		else
			m_SKSTAT &= ~SK_SHIFT;

		if( m_IRQEN & IRQ_KEYBD )
		{
			/* last interrupt not acknowledged ? */
			if( m_IRQST & IRQ_KEYBD )
				m_SKSTAT |= SK_KBERR;
			m_IRQST |= IRQ_KEYBD;
			if( m_interrupt_cb )
				(*m_interrupt_cb)(this, IRQ_KEYBD);
		}
	}
	else
	{
		m_KBCODE = kbcode;
		m_SKSTAT &= ~SK_KEYBD;
    }
}



//-------------------------------------------------
//  private stuff
//-------------------------------------------------

inline void pokeyn_device::process_channel(int ch)
{
	int toggle = 0;

	if( (m_channel[ch].m_AUDC & NOTPOLY5) || (m_poly5[m_p5] & 1) )
	{
		if( m_channel[ch].m_AUDC & PURE )
			toggle = 1;
		else
		if( m_channel[ch].m_AUDC & POLY4 )
			toggle = m_channel[ch].m_output == !(m_poly4[m_p4] & 1);
		else
		if( m_AUDCTL & POLY9 )
			toggle = m_channel[ch].m_output == !(m_poly9[m_p9] & 1);
		else
			toggle = m_channel[ch].m_output == !(m_poly17[m_p17] & 1);
	}
	if( toggle )
	{
		m_channel[ch].m_output ^= 1;
	}
}





void pokeyn_device::pokey_potgo(void)
{
    int pot;

	LOG(("POKEY #%p pokey_potgo\n", this));

    m_ALLPOT = 0xff;

    for( pot = 0; pot < 8; pot++ )
	{
		m_POTx[pot] = 0xff;
		if( !m_pot_r[pot].isnull() )
		{
			int r = m_pot_r[pot](pot);

			LOG(("POKEY %s pot_r(%d) returned $%02x\n", tag(), pot, r));
			if( r != -1 )
			{
				if (r > 228)
                    r = 228;

                /* final value */
                m_POTx[pot] = r;
				m_ptimer[pot]->adjust(((m_SKCTL & SK_PADDLE) ? m_ad_time_fast : m_ad_time_slow) * r, pot);
			}
		}
	}
}


void pokeyn_device::poly_init_4_5(UINT32 *poly, int size, int xorbit, int invert)
{
	int mask = (1 << size) - 1;
    int i;
    UINT32 lfsr = 0;

	LOG_POLY(("poly %d\n", size));
    for( i = 0; i < mask; i++ )
	{
        /* calculate next bit */
    	int in = !((lfsr >> 0) & 1) ^ ((lfsr >> xorbit) & 1);
    	lfsr = lfsr >> 1;
    	lfsr = (in << (size-1)) | lfsr;
		*poly = lfsr ^ invert;
        LOG_POLY(("%05x: %02x\n", lfsr, *poly));
        poly++;
	}
}

void pokeyn_device::poly_init_9_17(UINT32 *poly, int size)
{
    int mask = (1 << size) - 1;
    int i;
    UINT32 lfsr = 0;

	LOG_RAND(("rand %d\n", size));

	if (size == 17)
	{
	    for( i = 0; i < mask; i++ )
		{
	        /* calculate next bit @ 7 */
	    	int in8 = !((lfsr >> 8) & 1) ^ ((lfsr >> 13) & 1);
	    	int in = (lfsr & 1);
	    	lfsr = lfsr >> 1;
	    	lfsr = (lfsr & 0xff7f) | (in8 << 7);
	    	lfsr = (in << 16) | lfsr;
			*poly = lfsr;
	        LOG_RAND(("%05x: %02x\n", lfsr, *poly));
	        poly++;
		}
	}
	else
	{
	    for( i = 0; i < mask; i++ )
		{
	        /* calculate next bit */
	    	int in = !((lfsr >> 0) & 1) ^ ((lfsr >> 5) & 1);
	    	lfsr = lfsr >> 1;
	    	lfsr = (in << 8) | lfsr;
			*poly = lfsr;
	        LOG_RAND(("%05x: %02x\n", lfsr, *poly));
	        poly++;
		}
	}

}

char *pokeyn_device::audc2str(int val)
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

char *pokeyn_device::audctl2str(int val)
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

pokeyn_device::pokey_channel::pokey_channel()
	:	m_AUDF(0),
		m_AUDC(0),
		m_borrow_cnt(0),
		m_counter(0),
		m_volume(0),
		m_output(0),
		m_filter_sample(0)
{
}

//-------------------------------------------------
//  Quad Pokey support - should be in game drivers, really
//-------------------------------------------------


READ8_HANDLER( quad_pokeyn_r )
{
	static const char *const devname[4] = { "pokey1", "pokey2", "pokey3", "pokey4" };
	int pokey_num = (offset >> 3) & ~0x04;
	int control = (offset & 0x20) >> 2;
	int pokey_reg = (offset % 8) | control;
	pokeyn_device *pokey = space->machine().device<pokeyn_device>(devname[pokey_num]);

	return pokey->read(pokey_reg);
}

WRITE8_HANDLER( quad_pokeyn_w )
{
	static const char *const devname[4] = { "pokey1", "pokey2", "pokey3", "pokey4" };
    int pokey_num = (offset >> 3) & ~0x04;
    int control = (offset & 0x20) >> 2;
    int pokey_reg = (offset % 8) | control;
	pokeyn_device *pokey = space->machine().device<pokeyn_device>(devname[pokey_num]);

    pokey->write(pokey_reg, data);
}

