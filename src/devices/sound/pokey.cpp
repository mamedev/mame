// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Eric Smith, Juergen Buchmueller
/*****************************************************************************
 *
 *  POKEY chip emulator 4.6
 *
 *  Based on original info found in Ron Fries' Pokey emulator,
 *  with additions by Brad Oliver, Eric Smith and Juergen Buchmueller,
 *  paddle (a/d conversion) details from the Atari 400/800 Hardware Manual.
 *  Polynome algorithms according to info supplied by Perry McFarlane.
 *
 *  4.6:
 *    [1] http://ploguechipsounds.blogspot.de/2009/10/how-i-recorded-and-decoded-pokeys.html
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
 *  TODO:  liberatr clipping
 *
 *
 *****************************************************************************/

#include "emu.h"
#include "pokey.h"

#include "debugger.h"

/* Four channels with a range of 0..32767 and volume 0..15 */
//#define POKEY_DEFAULT_GAIN (32767/15/4)

/*
 * But we raise the gain and risk clipping, the old Pokey did
 * this too. It defined POKEY_DEFAULT_GAIN 6 and this was
 * 6 * 15 * 4 = 360, 360/256 = 1.40625
 * I use 15/11 = 1.3636, so this is a little lower.
 */

#define POKEY_DEFAULT_GAIN (32767/11/4)

#define VERBOSE         0
#define VERBOSE_SOUND   0
#define VERBOSE_TIMER   0
#define VERBOSE_POLY    0
#define VERBOSE_RAND    0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

#define LOG_SOUND(x) do { if (VERBOSE_SOUND) logerror x; } while (0)

#define LOG_TIMER(x) do { if (VERBOSE_TIMER) logerror x; } while (0)

#define LOG_POLY(x) do { if (VERBOSE_POLY) logerror x; } while (0)

#define LOG_RAND(x) do { if (VERBOSE_RAND) logerror x; } while (0)

#define CHAN1   0
#define CHAN2   1
#define CHAN3   2
#define CHAN4   3

#define TIMER1  0
#define TIMER2  1
#define TIMER4  2

/* values to add to the divisors for the different modes */
#define DIVADD_LOCLK        1
#define DIVADD_HICLK        4
#define DIVADD_HICLK_JOINED 7

/* AUDCx */
#define NOTPOLY5    0x80    /* selects POLY5 or direct CLOCK */
#define POLY4       0x40    /* selects POLY4 or POLY17 */
#define PURE        0x20    /* selects POLY4/17 or PURE tone */
#define VOLUME_ONLY 0x10    /* selects VOLUME OUTPUT ONLY */
#define VOLUME_MASK 0x0f    /* volume mask */

/* AUDCTL */
#define POLY9       0x80    /* selects POLY9 or POLY17 */
#define CH1_HICLK   0x40    /* selects 1.78979 MHz for Ch 1 */
#define CH3_HICLK   0x20    /* selects 1.78979 MHz for Ch 3 */
#define CH12_JOINED 0x10    /* clocks channel 1 w/channel 2 */
#define CH34_JOINED 0x08    /* clocks channel 3 w/channel 4 */
#define CH1_FILTER  0x04    /* selects channel 1 high pass filter */
#define CH2_FILTER  0x02    /* selects channel 2 high pass filter */
#define CLK_15KHZ   0x01    /* selects 15.6999 kHz or 63.9211 kHz */

/* IRQEN (D20E) */
#define IRQ_BREAK   0x80    /* BREAK key pressed interrupt */
#define IRQ_KEYBD   0x40    /* keyboard data ready interrupt */
#define IRQ_SERIN   0x20    /* serial input data ready interrupt */
#define IRQ_SEROR   0x10    /* serial output register ready interrupt */
#define IRQ_SEROC   0x08    /* serial output complete interrupt */
#define IRQ_TIMR4   0x04    /* timer channel #4 interrupt */
#define IRQ_TIMR2   0x02    /* timer channel #2 interrupt */
#define IRQ_TIMR1   0x01    /* timer channel #1 interrupt */

/* SKSTAT (R/D20F) */
#define SK_FRAME    0x80    /* serial framing error */
#define SK_KBERR    0x40    /* keyboard overrun error - pokey documentation states *some bit as IRQST */
#define SK_OVERRUN  0x20    /* serial overrun error - pokey documentation states *some bit as IRQST */
#define SK_SERIN    0x10    /* serial input high */
#define SK_SHIFT    0x08    /* shift key pressed */
#define SK_KEYBD    0x04    /* keyboard key pressed */
#define SK_SEROUT   0x02    /* serial output active */

/* SKCTL (W/D20F) */
#define SK_BREAK    0x80    /* serial out break signal */
#define SK_BPS      0x70    /* bits per second */
#define SK_FM       0x08    /* FM mode */
#define SK_PADDLE   0x04    /* fast paddle a/d conversion */
#define SK_RESET    0x03    /* reset serial/keyboard interface */
#define SK_KEYSCAN  0x02    /* key scanning enabled ? */
#define SK_DEBOUNCE 0x01    /* Debouncing ?*/

#define DIV_64      28       /* divisor for 1.78979 MHz clock to 63.9211 kHz */
#define DIV_15      114      /* divisor for 1.78979 MHz clock to 15.6999 kHz */

#define P4(chip)  chip->poly4[chip->p4]
#define P5(chip)  chip->poly5[chip->p5]
#define P9(chip)  chip->poly9[chip->p9]
#define P17(chip) chip->poly17[chip->p17]

#define CLK_1 0
#define CLK_28 1
#define CLK_114 2

static const int clock_divisors[3] = {1, DIV_64, DIV_15};




// device type definition
const device_type POKEY = &device_creator<pokey_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  okim9810_device - constructor
//-------------------------------------------------

pokey_device::pokey_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, POKEY, "POKEY", tag, owner, clock, "pokey", __FILE__),
		device_sound_interface(mconfig, *this),
		device_execute_interface(mconfig, *this),
		device_state_interface(mconfig, *this),
		m_output_type(LEGACY_LINEAR),
		m_icount(0),
		m_stream(nullptr),
		m_pot0_r_cb(*this),
		m_pot1_r_cb(*this),
		m_pot2_r_cb(*this),
		m_pot3_r_cb(*this),
		m_pot4_r_cb(*this),
		m_pot5_r_cb(*this),
		m_pot6_r_cb(*this),
		m_pot7_r_cb(*this),
		m_allpot_r_cb(*this),
		m_serin_r_cb(*this),
		m_serout_w_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pokey_device::device_start()
{
	//int sample_rate = clock();
	int i;

	m_clock_period = attotime::from_hz(clock());

	/* Setup channels */
	for (i=0; i<POKEY_CHANNELS; i++)
	{
		m_channel[i].m_parent = this;
		m_channel[i].m_INTMask = 0;
	}
	m_channel[CHAN1].m_INTMask = IRQ_TIMR1;
	m_channel[CHAN2].m_INTMask = IRQ_TIMR2;
	m_channel[CHAN4].m_INTMask = IRQ_TIMR4;

	// bind callbacks
	m_keyboard_r.bind_relative_to(*owner());
	m_irq_f.bind_relative_to(*owner());

	/* calculate the A/D times
	 * In normal, slow mode (SKCTL bit SK_PADDLE is clear) the conversion
	 * takes N scanlines, where N is the paddle value. A single scanline
	 * takes approximately 64us to finish (1.78979MHz clock).
	 * In quick mode (SK_PADDLE set) the conversion is done very fast
	 * (takes two scanlines) but the result is not as accurate.
	 */

	/* initialize the poly counters */
	poly_init_4_5(m_poly4, 4, 1, 0);
	poly_init_4_5(m_poly5, 5, 2, 1);

	/* initialize 9 / 17 arrays */
	poly_init_9_17(m_poly9,   9);
	poly_init_9_17(m_poly17, 17);
	vol_init();

	/* The pokey does not have a reset line. These should be initialized
	 * with random values.
	 */

	m_KBCODE = 0x09;         /* Atari 800 'no key' */
	m_SKCTL = SK_RESET;  /* let the RNG run after reset */
	m_SKSTAT = 0;
	/* This bit should probably get set later. Acid5200 pokey_setoc test tests this. */
	m_IRQST = IRQ_SEROC;
	m_IRQEN = 0;
	m_AUDCTL = 0;
	m_p4 = 0;
	m_p5 = 0;
	m_p9 = 0;
	m_p17 = 0;
	m_ALLPOT = 0x00;

	m_pot_counter = 0;
	m_kbd_cnt = 0;
	m_out_filter = 0;
	m_output = 0;
	m_kbd_state = 0;

	/* reset more internal state */
	for (i=0; i<3; i++)
	{
		m_clock_cnt[i] = 0;
	}

	for (i=0; i<8; i++)
	{
		m_POTx[i] = 0;
	}

	m_pot0_r_cb.resolve();
	m_pot1_r_cb.resolve();
	m_pot2_r_cb.resolve();
	m_pot3_r_cb.resolve();
	m_pot4_r_cb.resolve();
	m_pot5_r_cb.resolve();
	m_pot6_r_cb.resolve();
	m_pot7_r_cb.resolve();
	m_allpot_r_cb.resolve();
	m_serin_r_cb.resolve();
	m_serout_w_cb.resolve_safe();

	m_stream = stream_alloc(0, 1, clock());

	timer_alloc(SYNC_WRITE);    /* timer for sync operation */
	timer_alloc(SYNC_NOOP);
	timer_alloc(SYNC_POT);
	timer_alloc(SYNC_SET_IRQST);

	for (i=0; i<POKEY_CHANNELS; i++)
	{
		save_item(NAME(m_channel[i].m_borrow_cnt), i);
		save_item(NAME(m_channel[i].m_counter), i);
		save_item(NAME(m_channel[i].m_filter_sample), i);
		save_item(NAME(m_channel[i].m_output), i);
		save_item(NAME(m_channel[i].m_AUDF), i);
		save_item(NAME(m_channel[i].m_AUDC), i);
	}

	save_item(NAME(m_divisor));
	save_item(NAME(m_clock_cnt));
	save_item(NAME(m_p4));
	save_item(NAME(m_p5));
	save_item(NAME(m_p9));
	save_item(NAME(m_p17));
	save_item(NAME(m_pot_counter));
	save_item(NAME(m_kbd_cnt));
	save_item(NAME(m_kbd_latch));
	save_item(NAME(m_kbd_state));

	save_item(NAME(m_POTx));
	save_item(NAME(m_AUDCTL));
	save_item(NAME(m_ALLPOT));
	save_item(NAME(m_KBCODE));
	save_item(NAME(m_SERIN));
	save_item(NAME(m_SEROUT));
	save_item(NAME(m_IRQST));
	save_item(NAME(m_IRQEN));
	save_item(NAME(m_SKSTAT));
	save_item(NAME(m_SKCTL));

	// State support

	state_add(AUDF1_C, "AUDF1", m_channel[0].m_AUDF);
	state_add(AUDC1_C, "AUDC1", m_channel[0].m_AUDC);
	state_add(AUDF2_C, "AUDF2", m_channel[1].m_AUDF);
	state_add(AUDC2_C, "AUDC2", m_channel[1].m_AUDC);
	state_add(AUDF3_C, "AUDF3", m_channel[2].m_AUDF);
	state_add(AUDC3_C, "AUDC3", m_channel[2].m_AUDC);
	state_add(AUDF4_C, "AUDF4", m_channel[3].m_AUDF);
	state_add(AUDC4_C, "AUDC4", m_channel[3].m_AUDC);
	state_add(AUDCTL_C, "AUDCTL", m_AUDCTL);
#if 0
	state_add(STIMER_C, "STIMER", m_STIMER);
	state_add(SKREST_C, "SKREST_C", m_SKREST);
	state_add(POTGO_C, "POTGO", m_POTGO_C);
#endif
	state_add(SEROUT_C, "SEROUT", m_SEROUT);
	state_add(IRQEN_C, "IRQEN", m_IRQEN);
	state_add(SKCTL_C, "SKCTL", m_SKCTL);

	// set our instruction counter
	m_icountptr = &m_icount;

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pokey_device::device_reset()
{
	m_stream->update();
}


//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void pokey_device::device_post_load()
{
}


//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

void pokey_device::device_clock_changed()
{
}

//-------------------------------------------------
//  stream_generate - handle update requests for
//  our sound stream
//-------------------------------------------------

void pokey_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case 3:
		/* serout_ready_cb */
		if (m_IRQEN & IRQ_SEROR)
		{
			m_IRQST |= IRQ_SEROR;
			if (!m_irq_f.isnull())
				m_irq_f(IRQ_SEROR);
		}
		break;
	case 4:
		/* serout_complete */
		if (m_IRQEN & IRQ_SEROC)
		{
			m_IRQST |= IRQ_SEROC;
			if (!m_irq_f.isnull())
				m_irq_f(IRQ_SEROC);
		}
		break;
	case 5:
		/* serin_ready */
		if (m_IRQEN & IRQ_SERIN)
		{
			m_IRQST |= IRQ_SERIN;
			if (!m_irq_f.isnull())
				m_irq_f(IRQ_SERIN);
		}
		break;
	case SYNC_WRITE:
		{
			offs_t offset = (param >> 8) & 0xff;
			UINT8 data = param & 0xff;
			write_internal(offset, data);
		}
		break;
	case SYNC_NOOP:
		/* do nothing, caused by a forced resync */
		break;
	case SYNC_POT:
		//logerror("x %02x \n", (param & 0x20));
		m_ALLPOT |= (param & 0xff);
		break;
	case SYNC_SET_IRQST:
		m_IRQST |=  (param & 0xff);
		break;
	default:
		assert_always(FALSE, "Unknown id in pokey_device::device_timer");
	}
}

void pokey_device::execute_run()
{
	bool check_debugger = ((device_t::machine().debug_flags & DEBUG_FLAG_ENABLED) != 0);

	do
	{
		// debugging
		//m_ppc = m_pc; // copy PC to previous PC
		if (check_debugger)
			debugger_instruction_hook(this, 0); //m_pc);

		// instruction fetch
		//UINT16 op = opcode_read();

		UINT32 new_out = step_one_clock();
		if (m_output != new_out)
		{
			//printf("forced update %08d %08x\n", m_icount, m_output);
			m_stream->update();
			m_output = new_out;
		}

		m_icount--;
	} while (m_icount > 0);

}


//-------------------------------------------------
//  step_one_clock - step the whole chip one
//  clock cycle.
//-------------------------------------------------

void pokey_device::step_keyboard()
{
	if (++m_kbd_cnt > 63)
		m_kbd_cnt = 0;
	if (!m_keyboard_r.isnull())
	{
		UINT8 ret = m_keyboard_r(m_kbd_cnt);

		switch (m_kbd_cnt)
		{
		case POK_KEY_BREAK:
			if (ret & 2)
			{
				/* check if the break IRQ is enabled */
				if (m_IRQEN & IRQ_BREAK)
				{
					m_IRQST |= IRQ_BREAK;
					if (!m_irq_f.isnull())
						m_irq_f(IRQ_BREAK);
				}
			}
			break;
		case POK_KEY_SHIFT:
			m_kbd_latch = (m_kbd_latch & 0xbf) | ((ret & 2) << 5);
			if (m_kbd_latch & 0x40)
				m_SKSTAT |= SK_SHIFT;
			else
				m_SKSTAT &= ~SK_SHIFT;
			/* FIXME: sync ? */
			break;
		case POK_KEY_CTRL:
			m_kbd_latch = (m_kbd_latch & 0x7f) | ((ret & 2) << 6);
			break;
		}
		switch (m_kbd_state)
		{
		case 0: /* waiting for key */
			if (ret & 1)
			{
				m_kbd_latch = (m_kbd_latch & 0xc0) | m_kbd_cnt;
				m_kbd_state++;
			}
			break;
		case 1: /* waiting for key confirmation */
			if ((m_kbd_latch & 0x3f) == m_kbd_cnt)
			{
				if (ret & 1)
				{
					m_KBCODE = m_kbd_latch;
					m_SKSTAT |= SK_KEYBD;
					if (m_IRQEN & IRQ_KEYBD)
					{
						/* last interrupt not acknowledged ? */
						if(m_IRQST & IRQ_KEYBD)
							m_SKSTAT |= SK_KBERR;
						m_IRQST |= IRQ_KEYBD;
						if (!m_irq_f.isnull())
							m_irq_f(IRQ_KEYBD);
					}
					m_kbd_state++;
				}
				else
					m_kbd_state = 0;
			}
			break;
		case 2: /* waiting for release */
			if ((m_kbd_latch & 0x3f) == m_kbd_cnt)
			{
				if ((ret & 1)==0)
					m_kbd_state++;
				else
					m_SKSTAT |= SK_KEYBD;
			}
			break;
		case 3:
			if ((m_kbd_latch & 0x3f) == m_kbd_cnt)
			{
				if (ret & 1)
					m_kbd_state = 2;
				else
				{
					m_SKSTAT &= ~SK_KEYBD;
					m_kbd_state = 0;
				}
			}
			break;
		}
	}
}

void pokey_device::step_pot()
{
	int pot;
	UINT8 upd = 0;
	m_pot_counter++;
	for (pot = 0; pot < 8; pot++)
	{
		if ((m_POTx[pot]<m_pot_counter) || (m_pot_counter == 228))
		{
			upd |= (1<<pot);
			/* latching is emulated in read */
		}
	}
	synchronize(SYNC_POT, upd);
}

/*
 * http://www.atariage.com/forums/topic/3328-sio-protocol/page__st__100#entry1680190:
 * I noticed that the Pokey counters have clocked carry (actually, "borrow") positions that delay the
 * counter by 3 cycles, plus the 1 reset clock. So 16 bit mode has 6 carry delays and a reset clock.
 * I'm sure this was done because the propagation delays limited the number of cells the subtraction could ripple though.
 *
 */

UINT32 pokey_device::step_one_clock(void)
{
	int ch, clk;
	UINT32 sum = 0;
	int clock_triggered[3] = {0,0,0};
	int base_clock = (m_AUDCTL & CLK_15KHZ) ? CLK_114 : CLK_28;

	if( (m_SKCTL & SK_RESET) )
	{
		/* Clocks only count if we are not in a reset */
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

		/* Potentiometer handling */
		if ((clock_triggered[CLK_114] || (m_SKCTL & SK_PADDLE)) && (m_pot_counter < 228))
			step_pot();

		/* Keyboard */
		if (clock_triggered[CLK_114] && (m_SKCTL & SK_KEYSCAN))
			step_keyboard();
	}

	/* do CHAN2 before CHAN1 because CHAN1 may set borrow! */
	if (m_channel[CHAN2].check_borrow())
	{
		int isJoined = (m_AUDCTL & CH12_JOINED);
		if (isJoined)
			m_channel[CHAN1].reset_channel();
		m_channel[CHAN2].reset_channel();
		process_channel(CHAN2);

		/* check if some of the requested timer interrupts are enabled */
		if ((m_IRQST & IRQ_TIMR2) && !m_irq_f.isnull())
				m_irq_f(IRQ_TIMR2);
	}

	if (m_channel[CHAN1].check_borrow())
	{
		int isJoined = (m_AUDCTL & CH12_JOINED);
		if (isJoined)
			m_channel[CHAN2].inc_chan();
		else
			m_channel[CHAN1].reset_channel();
		process_channel(CHAN1);
		/* check if some of the requested timer interrupts are enabled */
		if ((m_IRQST & IRQ_TIMR1) && !m_irq_f.isnull())
			m_irq_f(IRQ_TIMR1);
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
		if ((m_IRQST & IRQ_TIMR4) && !m_irq_f.isnull())
			m_irq_f(IRQ_TIMR4);
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
		sum |= (((((m_channel[ch].m_output ^ m_channel[ch].m_filter_sample) || (m_channel[ch].m_AUDC & VOLUME_ONLY)) ? (m_channel[ch].m_AUDC & VOLUME_MASK) : 0 )) << (ch * 4));
	}
	return sum;
}

//-------------------------------------------------
//  stream_generate - handle update requests for
//  our sound stream
//-------------------------------------------------


void pokey_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *buffer = outputs[0];

	if (m_output_type == LEGACY_LINEAR)
	{
		INT32 out = 0;
		for (int i = 0; i < 4; i++)
			out += ((m_output >> (4*i)) & 0x0f);
		out *= POKEY_DEFAULT_GAIN;
		out = (out > 0x7fff) ? 0x7fff : out;
		while( samples > 0 )
		{
			*buffer++ = out;
			samples--;
		}
	}
	else if (m_output_type == RC_LOWPASS)
	{
		double rTot = m_voltab[m_output];

		double V0 = rTot / (rTot+m_r_pullup) * m_v_ref / 5.0 * 32767.0;
		double mult = (m_cap == 0.0) ? 1.0 : 1.0 - exp(-(rTot + m_r_pullup) / (m_cap * m_r_pullup * rTot) * m_clock_period.as_double());

		while( samples > 0 )
		{
			/* store sum of output signals into the buffer */
			m_out_filter += (V0 - m_out_filter) * mult;
			*buffer++ = m_out_filter;
			samples--;

		}
	}
	else if (m_output_type == OPAMP_C_TO_GROUND)
	{
		double rTot = m_voltab[m_output];
		/* In this configuration there is a capacitor in parallel to the pokey output to ground.
		 * With a LM324 in LTSpice this causes the opamp circuit to oscillate at around 100 kHz.
		 * We are ignoring the capacitor here, since this oscillation would not be audible.
		 */

		/* This post-pokey stage usually has a high-pass filter behind it
		 * It is approximated by eliminating m_v_ref ( -1.0 term)
		 */

		double V0 = ((rTot+m_r_pullup) / rTot - 1.0) * m_v_ref  / 5.0 * 32767.0;

		while( samples > 0 )
		{
			/* store sum of output signals into the buffer */
			*buffer++ = V0;
			samples--;

		}
	}
	else if (m_output_type == OPAMP_LOW_PASS)
	{
		double rTot = m_voltab[m_output];
		/* This post-pokey stage usually has a low-pass filter behind it
		 * It is approximated by not adding in VRef below.
		 */

		double V0 = (m_r_pullup / rTot) * m_v_ref  / 5.0 * 32767.0;
		double mult = (m_cap == 0.0) ? 1.0 : 1.0 - exp(-1.0 / (m_cap * m_r_pullup) * m_clock_period.as_double());

		while( samples > 0 )
		{
			/* store sum of output signals into the buffer */
			m_out_filter += (V0 - m_out_filter) * mult;
			*buffer++ = m_out_filter /* + m_v_ref */;       // see above
			samples--;
		}
	}
	else if (m_output_type == DISCRETE_VAR_R)
	{
		INT32 out = m_voltab[m_output];
		while( samples > 0 )
		{
			*buffer++ = out;
			samples--;
		}
	}
}

//-------------------------------------------------
//  read - memory interface for reading the active status
//-------------------------------------------------

READ8_MEMBER( pokey_device::read )
{
	return read(offset);
}

UINT8 pokey_device::read(offs_t offset)
{
	int data, pot;

	synchronize(SYNC_NOOP); /* force resync */

	switch (offset & 15)
	{
	case POT0_C: case POT1_C: case POT2_C: case POT3_C:
	case POT4_C: case POT5_C: case POT6_C: case POT7_C:
		pot = offset & 7;
		if( m_ALLPOT & (1 << pot) )
		{
			/* we have a value measured */
			data = m_POTx[pot];
			LOG(("POKEY '%s' read POT%d (final value)  $%02x\n", tag(), pot, data));
		}
		else
		{
			data = m_pot_counter;
			LOG(("POKEY '%s' read POT%d (interpolated) $%02x\n", tag(), pot, data));
		}
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
		else if( !m_allpot_r_cb.isnull() )
		{
			data = m_allpot_r_cb(offset);
			LOG(("%s: POKEY '%s' ALLPOT callback $%02x\n", machine().describe_context(), tag(), data));
		}
		else
		{
			data = m_ALLPOT ^ 0xff;
			LOG(("POKEY '%s' ALLPOT internal $%02x\n", tag(), data));
		}
		break;

	case KBCODE_C:
		data = m_KBCODE;
		break;

	case RANDOM_C:
		if( m_AUDCTL & POLY9 )
		{
			data = m_poly9[m_p9] & 0xff;
			LOG_RAND(("POKEY '%s' rand9[$%05x]: $%02x\n", tag(), m_p9, data));
		}
		else
		{
			data = (m_poly17[m_p17] >> 8) & 0xff;
			LOG_RAND(("POKEY '%s' rand17[$%05x]: $%02x\n", tag(), m_p17, data));
		}
		break;

	case SERIN_C:
		if( !m_serin_r_cb.isnull() )
			m_SERIN = m_serin_r_cb(offset);
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
		data = 0xff;
		break;
	}
	return data;

}


//-------------------------------------------------
//  write - memory interface for write
//-------------------------------------------------

void pokey_device::write(offs_t offset, UINT8 data)
{
	synchronize(SYNC_WRITE, (offset<<8) | data);
}

WRITE8_MEMBER( pokey_device::write )
{
	write(offset, data);
}

void pokey_device::write_internal(offs_t offset, UINT8 data)
{
	/* determine which address was changed */
	switch (offset & 15)
	{
	case AUDF1_C:
		LOG_SOUND(("POKEY '%s' AUDF1  $%02x\n", tag(), data));
		m_channel[CHAN1].m_AUDF = data;
		break;

	case AUDC1_C:
		LOG_SOUND(("POKEY '%s' AUDC1  $%02x (%s)\n", tag(), data, audc2str(data)));
		m_channel[CHAN1].m_AUDC = data;
		break;

	case AUDF2_C:
		LOG_SOUND(("POKEY '%s' AUDF2  $%02x\n", tag(), data));
		m_channel[CHAN2].m_AUDF = data;
		break;

	case AUDC2_C:
		LOG_SOUND(("POKEY '%s' AUDC2  $%02x (%s)\n", tag(), data, audc2str(data)));
		m_channel[CHAN2].m_AUDC = data;
		break;

	case AUDF3_C:
		LOG_SOUND(("POKEY '%s' AUDF3  $%02x\n", tag(), data));
		m_channel[CHAN3].m_AUDF = data;
		break;

	case AUDC3_C:
		LOG_SOUND(("POKEY '%s' AUDC3  $%02x (%s)\n", tag(), data, audc2str(data)));
		m_channel[CHAN3].m_AUDC = data;
		break;

	case AUDF4_C:
		LOG_SOUND(("POKEY '%s' AUDF4  $%02x\n", tag(), data));
		m_channel[CHAN4].m_AUDF = data;
		break;

	case AUDC4_C:
		LOG_SOUND(("POKEY '%s' AUDC4  $%02x (%s)\n", tag(), data, audc2str(data)));
		m_channel[CHAN4].m_AUDC = data;
		break;

	case AUDCTL_C:
		if( data == m_AUDCTL )
			return;
		LOG_SOUND(("POKEY '%s' AUDCTL $%02x (%s)\n", tag(), data, audctl2str(data)));
		m_AUDCTL = data;

		break;

	case STIMER_C:
		LOG_TIMER(("POKEY '%s' STIMER $%02x\n", tag(), data));

		/* From the pokey documentation:
		 * reset all counters to zero (side effect)
		 * Actually this takes 4 cycles to actually happen.
		 * FIXME: Use timer for delayed reset !
		 */
		for (int i = 0; i < POKEY_CHANNELS; i++)
		{
			m_channel[i].reset_channel();
			m_channel[i].m_output = 0;
			m_channel[i].m_filter_sample = (i<2 ? 1 : 0);
		}

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
		m_serout_w_cb(offset, data);
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
			/* reset IRQST bits that are masked now, except the SEROC bit (acid5200 pokey_seroc test) */
			m_IRQST &= (IRQ_SEROC | data);
		}
		/* store irq enable */
		m_IRQEN = data;
		/* if SEROC irq is enabled trigger an irq (acid5200 pokey_seroc test) */
		if (m_IRQEN & m_IRQST & IRQ_SEROC)
		{
			if (!m_irq_f.isnull())
				m_irq_f(IRQ_SEROC);
		}
		break;

	case SKCTL_C:
		if( data == m_SKCTL )
			return;
		LOG(("POKEY '%s' SKCTL  $%02x\n", tag(), data));
		m_SKCTL = data;
		if( !(data & SK_RESET) )
		{
			write_internal(IRQEN_C,  0);
			write_internal(SKREST_C, 0);
			/****************************************************************
			 * If the 2 least significant bits of SKCTL are 0, the random
			 * number generator is disabled (SKRESET). Thanks to Eric Smith
			 * for pointing out this critical bit of info!
			 * Couriersud: Actually, the 17bit poly is reset and kept in a
			 * reset state.
			 ****************************************************************/
			m_p9 = 0;
			m_p17 = 0;
			m_p4 = 0;
			m_p5 = 0;
			m_clock_cnt[0] = 0;
			m_clock_cnt[1] = 0;
			m_clock_cnt[2] = 0;
			/* FIXME: Serial port reset ! */
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

}

WRITE_LINE_MEMBER( pokey_device::sid_w )
{
	if (state)
	{
		m_SKSTAT |= SK_SERIN;
	}
	else
	{
		m_SKSTAT &= ~SK_SERIN;
	}
}

void pokey_device::serin_ready(int after)
{
	timer_set(m_clock_period * after, 5, 0);
}

//-------------------------------------------------
//  private stuff
//-------------------------------------------------

inline void pokey_device::process_channel(int ch)
{
	if ((m_channel[ch].m_AUDC & NOTPOLY5) || (m_poly5[m_p5] & 1))
	{
		if (m_channel[ch].m_AUDC & PURE)
			m_channel[ch].m_output ^= 1;
		else if (m_channel[ch].m_AUDC & POLY4)
			m_channel[ch].m_output = (m_poly4[m_p4] & 1);
		else if (m_AUDCTL & POLY9)
			m_channel[ch].m_output = (m_poly9[m_p9] & 1);
		else
			m_channel[ch].m_output = (m_poly17[m_p17] & 1);
	}
}


void pokey_device::pokey_potgo(void)
{
	int pot;

	LOG(("POKEY #%p pokey_potgo\n", (void *) this));

	m_ALLPOT = 0x00;
	m_pot_counter = 0;

	for( pot = 0; pot < 8; pot++ )
	{
		m_POTx[pot] = 228;
		switch (pot)
		{
			case 0:
				if( !m_pot0_r_cb.isnull() )
				{
					int r = m_pot0_r_cb(pot);

					LOG(("POKEY %s pot_r(%d) returned $%02x\n", tag(), pot, r));
					if (r >= 228)
					{
						r = 228;
					}
					if (r == 0)
					{
						/* immediately set the ready - bit of m_ALLPOT
						 * In this case, most likely no capacitor is connected
						 */
						m_ALLPOT |= (1<<pot);
					}

					/* final value */
					m_POTx[pot] = r;
				}
				break;
			case 1:
				if( !m_pot1_r_cb.isnull() )
				{
					int r = m_pot1_r_cb(pot);

					LOG(("POKEY %s pot_r(%d) returned $%02x\n", tag(), pot, r));
					if (r >= 228)
					{
						r = 228;
					}
					if (r == 0)
					{
						/* immediately set the ready - bit of m_ALLPOT
						 * In this case, most likely no capacitor is connected
						 */
						m_ALLPOT |= (1<<pot);
					}

					/* final value */
					m_POTx[pot] = r;
				}
				break;
			case 2:
				if( !m_pot2_r_cb.isnull() )
				{
					int r = m_pot2_r_cb(pot);

					LOG(("POKEY %s pot_r(%d) returned $%02x\n", tag(), pot, r));
					if (r >= 228)
					{
						r = 228;
					}
					if (r == 0)
					{
						/* immediately set the ready - bit of m_ALLPOT
						 * In this case, most likely no capacitor is connected
						 */
						m_ALLPOT |= (1<<pot);
					}

					/* final value */
					m_POTx[pot] = r;
				}
				break;
			case 3:
				if( !m_pot3_r_cb.isnull() )
				{
					int r = m_pot3_r_cb(pot);

					LOG(("POKEY %s pot_r(%d) returned $%02x\n", tag(), pot, r));
					if (r >= 228)
					{
						r = 228;
					}
					if (r == 0)
					{
						/* immediately set the ready - bit of m_ALLPOT
						 * In this case, most likely no capacitor is connected
						 */
						m_ALLPOT |= (1<<pot);
					}

					/* final value */
					m_POTx[pot] = r;
				}
				break;
			case 4:
				if( !m_pot4_r_cb.isnull() )
				{
					int r = m_pot4_r_cb(pot);

					LOG(("POKEY %s pot_r(%d) returned $%02x\n", tag(), pot, r));
					if (r >= 228)
					{
						r = 228;
					}
					if (r == 0)
					{
						/* immediately set the ready - bit of m_ALLPOT
						 * In this case, most likely no capacitor is connected
						 */
						m_ALLPOT |= (1<<pot);
					}

					/* final value */
					m_POTx[pot] = r;
				}
				break;
			case 5:
				if( !m_pot5_r_cb.isnull() )
				{
					int r = m_pot5_r_cb(pot);

					LOG(("POKEY %s pot_r(%d) returned $%02x\n", tag(), pot, r));
					if (r >= 228)
					{
						r = 228;
					}
					if (r == 0)
					{
						/* immediately set the ready - bit of m_ALLPOT
						 * In this case, most likely no capacitor is connected
						 */
						m_ALLPOT |= (1<<pot);
					}

					/* final value */
					m_POTx[pot] = r;
				}
				break;
			case 6:
				if( !m_pot6_r_cb.isnull() )
				{
					int r = m_pot6_r_cb(pot);

					LOG(("POKEY %s pot_r(%d) returned $%02x\n", tag(), pot, r));
					if (r >= 228)
					{
						r = 228;
					}
					if (r == 0)
					{
						/* immediately set the ready - bit of m_ALLPOT
						 * In this case, most likely no capacitor is connected
						 */
						m_ALLPOT |= (1<<pot);
					}

					/* final value */
					m_POTx[pot] = r;
				}
				break;
			case 7:
				if( !m_pot7_r_cb.isnull() )
				{
					int r = m_pot7_r_cb(pot);

					LOG(("POKEY %s pot_r(%d) returned $%02x\n", tag(), pot, r));
					if (r >= 228)
					{
						r = 228;
					}
					if (r == 0)
					{
						/* immediately set the ready - bit of m_ALLPOT
						 * In this case, most likely no capacitor is connected
						 */
						m_ALLPOT |= (1<<pot);
					}

					/* final value */
					m_POTx[pot] = r;
				}
				break;
		}
	}
}

void pokey_device::vol_init()
{
	double resistors[4] = {90000, 26500, 8050, 3400};
	double pull_up = 10000;
	/* just a guess, there has to be a resistance since the doc specifies that
	 * Vout is at least 4.2V if all channels turned off.
	 */
	double r_off = 8e6;
	double r_chan[16];
	double rTot;

	for (int j=0; j<16; j++)
	{
		rTot = 1.0 / 1e12; /* avoid div by 0 */;
		for (int i=0; i<4; i++)
		{
			if (j & (1 << i))
				rTot += 1.0 / resistors[i];
			else
				rTot += 1.0 / r_off;
		}
		r_chan[j] = 1.0 / rTot;
	}
	if (VERBOSE)
		for (int j=0; j<16; j++)
		{
			rTot = 1.0 / r_chan[j] + 3.0 / r_chan[0];
			rTot = 1.0 / rTot;
			LOG(("%s: %3d - %4.3f\n", tag(), j, rTot / (rTot+pull_up)*4.75));
		}
	for (int j=0; j<0x10000; j++)
	{
		rTot = 0;
		for (int i=0; i<4; i++)
		{
			rTot += 1.0 / r_chan[(j>>(i*4)) & 0x0f];
		}
		rTot = 1.0 / rTot;
		m_voltab[j] = rTot;
	}

}

void pokey_device::poly_init_4_5(UINT32 *poly, int size, int xorbit, int invert)
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
		LOG_POLY(("%05x: %02x\n", i, *poly));
		poly++;
	}
}

void pokey_device::poly_init_9_17(UINT32 *poly, int size)
{
	int mask = (1 << size) - 1;
	int i;
	UINT32 lfsr =mask;

	LOG_RAND(("rand %d\n", size));

	if (size == 17)
	{
		for( i = 0; i < mask; i++ )
		{
			/* calculate next bit @ 7 */
			int in8 = ((lfsr >> 8) & 1) ^ ((lfsr >> 13) & 1);
			int in = (lfsr & 1);
			lfsr = lfsr >> 1;
			lfsr = (lfsr & 0xff7f) | (in8 << 7);
			lfsr = (in << 16) | lfsr;
			*poly = lfsr;
			LOG_RAND(("%05x: %02x\n", i, *poly));
			poly++;
		}
	}
	else
	{
		for( i = 0; i < mask; i++ )
		{
			/* calculate next bit */
			int in = ((lfsr >> 0) & 1) ^ ((lfsr >> 5) & 1);
			lfsr = lfsr >> 1;
			lfsr = (in << 8) | lfsr;
			*poly = lfsr;
			LOG_RAND(("%05x: %02x\n", i, *poly));
			poly++;
		}
	}

}

char *pokey_device::audc2str(int val)
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

char *pokey_device::audctl2str(int val)
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

pokey_device::pokey_channel::pokey_channel()
	:   m_AUDF(0),
		m_AUDC(0),
		m_borrow_cnt(0),
		m_counter(0),
		m_output(0),
		m_filter_sample(0),
		m_div2(0)
{
}
