// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Eric Smith, Juergen Buchmueller
/*****************************************************************************
 *
 *  POKEY chip emulator 4.9
 *
 *  Based on original info found in Ron Fries' Pokey emulator,
 *  with additions by Brad Oliver, Eric Smith and Juergen Buchmueller,
 *  paddle (a/d conversion) details from the Atari 400/800 Hardware Manual.
 *  Polynomial algorithms according to info supplied by Perry McFarlane.
 *  Additional improvements from Mike Saarna's A7800 MAME fork.
 *
 *  4.9:
 *  - Two-tone mode updated for better accuracy.
 *
 *  4.8:
 *  - Poly5 related modes had a pitch shift issue. The poly4/5 init routine
 *    was replaced with one based on Altira's implementation, which resolved
 *    the issue.
 *
 *  4.7:
 *    [1] https://www.virtualdub.org/downloads/Altirra%20Hardware%20Reference%20Manual.pdf
 *  - updated to reflect that borrowing cycle delays only impacts voices
 *    running at 1.79MHz. (+4 cycles unlinked, or +7 cycles linked)
 *    At slower speeds, cycle overhead still occurs, but only affects
 *    the phase of the timer period, not the actual length.
 *  - Initial two-tone support added. Emulation of two-tone is limited to
 *    audio output effects, and doesn't incorporate any of the aspects of
 *    SIO serial transfer.
 *
 *  4.6:
 *    [2] http://ploguechipsounds.blogspot.de/2009/10/how-i-recorded-and-decoded-pokeys.html
 *  - changed audio emulation to emulate borrow 3 clock delay and
 *    proper channel reset. New frequency only becomes effective
 *    after the counter hits 0. Emulation also treats counters
 *    as 8 bit counters which are linked now instead of monolithic
 *    16 bit counters.
 *
 *  4.51:
 *  - changed to use the attotime datatype
 *
 *  4.5:
 *  - changed the 9/17 bit polynomial formulas such that the values
 *    required for the Tempest Pokey protection will be found.
 *    Tempest expects the upper 4 bits of the RNG to appear in the
 *    lower 4 bits after four cycles, so there has to be a shift
 *    of 1 per cycle (which was not the case before). Bits #6-#13 of the
 *    new RNG give this expected result now, bits #0-7 of the 9 bit poly.
 *  - reading the RNG returns the shift register contents ^ 0xff.
 *    That way resetting the Pokey with SKCTL (which resets the
 *    polynomial shifters to 0) returns the expected 0xff value.
 *
 *  4.4:
 *  - reversed sample values to make OFF channels produce a zero signal.
 *    actually de-reversed them; don't remember that I reversed them ;-/
 *
 *  4.3:
 *  - for POT inputs returning zero, immediately assert the ALLPOT
 *    bit after POTGO is written, otherwise start trigger timer
 *    depending on SK_PADDLE mode, either 1-228 scanlines or 1-2
 *    scanlines, depending on the SK_PADDLE bit of SKCTL.
 *
 *  4.2:
 *  - half volume for channels which are inaudible (this should be
 *    close to the real thing).
 *
 *  4.1:
 *  - default gain increased to closely match the old code.
 *  - random numbers repeat rate depends on POLY9 flag too!
 *  - verified sound output with many, many Atari 800 games,
 *    including the SUPPRESS_INAUDIBLE optimizations.
 *
 *  4.0:
 *  - rewritten from scratch.
 *  - 16bit stream interface.
 *  - serout ready/complete delayed interrupts.
 *  - reworked pot analog/digital conversion timing.
 *  - optional non-indexing pokey update functions.
 *
 *  TODO:  liberatr clipping
 *  TODO:  bit-level serial I/O instead of fake byte read/write handlers
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

#define VERBOSE_SOUND   (1U << 1)
#define VERBOSE_TIMER   (1U << 2)
#define VERBOSE_POLY    (1U << 3)
#define VERBOSE_RAND    (1U << 4)
#define VERBOSE_IRQ     (1U << 5)
#define VERBOSE         (0)

#include "logmacro.h"

#define LOG_SOUND(...) LOGMASKED(VERBOSE_SOUND, __VA_ARGS__)

#define LOG_TIMER(...) LOGMASKED(VERBOSE_TIMER, __VA_ARGS__)

#define LOG_POLY(...) LOGMASKED(VERBOSE_POLY, __VA_ARGS__)

#define LOG_RAND(...) LOGMASKED(VERBOSE_RAND, __VA_ARGS__)

#define LOG_IRQ(...) LOGMASKED(VERBOSE_IRQ, __VA_ARGS__)

#define CHAN1   0
#define CHAN2   1
#define CHAN3   2
#define CHAN4   3

#define TIMER1  0
#define TIMER2  1
#define TIMER4  2

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
#define SK_TWOTONE  0x08    /* Two tone mode */
#define SK_PADDLE   0x04    /* fast paddle a/d conversion */
#define SK_RESET    0x03    /* reset serial/keyboard interface */
#define SK_KEYSCAN  0x02    /* key scanning enabled ? */
#define SK_DEBOUNCE 0x01    /* Debouncing ?*/

#define DIV_64      28       /* divisor for 1.78979 MHz clock to 63.9211 kHz */
#define DIV_15      114      /* divisor for 1.78979 MHz clock to 15.6999 kHz */

#define CLK_1 0
#define CLK_28 1
#define CLK_114 2


// device type definition
DEFINE_DEVICE_TYPE(POKEY, pokey_device, "pokey", "Atari C012294 POKEY")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pokey_device - constructor
//-------------------------------------------------

pokey_device::pokey_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, POKEY, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	device_execute_interface(mconfig, *this),
	device_state_interface(mconfig, *this),
	m_icount(0),
	m_stream(nullptr),
	m_pot_r_cb(*this, 0),
	m_allpot_r_cb(*this, 0),
	m_serin_r_cb(*this, 0),
	m_serout_w_cb(*this),
	m_irq_w_cb(*this),
	m_keyboard_r(*this),
	m_output_type(LEGACY_LINEAR),
	m_serout_ready_timer(nullptr),
	m_serout_complete_timer(nullptr),
	m_serin_ready_timer(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pokey_device::device_start()
{
	//int sample_rate = clock();

	// Set up channels
	for (pokey_channel &chan : m_channel)
		chan.m_INTMask = 0;
	m_channel[CHAN1].m_INTMask = IRQ_TIMR1;
	m_channel[CHAN2].m_INTMask = IRQ_TIMR2;
	m_channel[CHAN4].m_INTMask = IRQ_TIMR4;

	// bind delegates
	m_keyboard_r.resolve();

	/* calculate the A/D times
	 * In normal, slow mode (SKCTL bit SK_PADDLE is clear) the conversion
	 * takes N scanlines, where N is the paddle value. A single scanline
	 * takes approximately 64us to finish (1.78979MHz clock).
	 * In quick mode (SK_PADDLE set) the conversion is done very fast
	 * (takes two scanlines) but the result is not as accurate.
	 */

	/* initialize the poly counters */
	poly_init_4_5(m_poly4, 4);
	poly_init_4_5(m_poly5, 5);

	/* initialize 9 / 17 arrays */
	poly_init_9_17(m_poly9,   9);
	poly_init_9_17(m_poly17, 17);
	vol_init();

	for (int i=0; i<4; i++)
		m_channel[i].m_AUDC = 0xb0;

	/* The pokey does not have a reset line. These should be initialized
	 * with random values.
	 */

	m_KBCODE = 0x09; // Atari 800 'no key'
	m_SKCTL = 0;

	// TODO: several a7800 demos don't explicitly reset pokey at startup
	// See https://atariage.com/forums/topic/337317-a7800-52-release/ and
	// https://atariage.com/forums/topic/268458-a7800-the-atari-7800-emulator/?do=findComment&comment=5079170)
	// m_SKCTL = SK_RESET;

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
	m_out_raw = 0;
	m_old_raw_inval = true;
	m_kbd_state = 0;

	/* reset more internal state */
	std::fill(std::begin(m_clock_cnt), std::end(m_clock_cnt), 0);
	std::fill(std::begin(m_POTx), std::end(m_POTx), 0);

	m_stream = stream_alloc(0, 1, clock());

	m_serout_ready_timer = timer_alloc(FUNC(pokey_device::serout_ready_irq), this);
	m_serout_complete_timer = timer_alloc(FUNC(pokey_device::serout_complete_irq), this);
	m_serin_ready_timer = timer_alloc(FUNC(pokey_device::serin_ready_irq), this);

	save_item(STRUCT_MEMBER(m_channel, m_borrow_cnt));
	save_item(STRUCT_MEMBER(m_channel, m_counter));
	save_item(STRUCT_MEMBER(m_channel, m_filter_sample));
	save_item(STRUCT_MEMBER(m_channel, m_output));
	save_item(STRUCT_MEMBER(m_channel, m_AUDF));
	save_item(STRUCT_MEMBER(m_channel, m_AUDC));

	save_item(NAME(m_clock_cnt));
	save_item(NAME(m_p4));
	save_item(NAME(m_p5));
	save_item(NAME(m_p9));
	save_item(NAME(m_p17));

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

	save_item(NAME(m_pot_counter));
	save_item(NAME(m_kbd_cnt));
	save_item(NAME(m_kbd_latch));
	save_item(NAME(m_kbd_state));

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
	set_icountptr(m_icount);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pokey_device::device_reset()
{
	m_stream->update();
	// a1200xl reads POT4 twice at startup for reading self-test mode jumpers.
	// we need to update POT counters here otherwise it will boot to self-test
	// the first time around no matter the setting.
	pokey_potgo();
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
	m_clock_period = clocks_to_attotime(1);

	if (clock() != 0)
	{
		if (m_stream != nullptr)
			m_stream->set_sample_rate(clock());
		else
			m_stream = stream_alloc(0, 1, clock());
	}
}

//-------------------------------------------------
//  timer callbacks
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(pokey_device::serout_ready_irq)
{
	if (m_IRQEN & IRQ_SEROR)
	{
		m_IRQST |= IRQ_SEROR;
		LOG_IRQ("POKEY SEROR IRQ raised\n");
		m_irq_w_cb(ASSERT_LINE);
	}
}

TIMER_CALLBACK_MEMBER(pokey_device::serout_complete_irq)
{
	m_IRQST |= IRQ_SEROC;
	if (m_IRQEN & IRQ_SEROC)
	{
		LOG_IRQ("POKEY SEROC IRQ raised\n");
		m_irq_w_cb(ASSERT_LINE);
	}
}

TIMER_CALLBACK_MEMBER(pokey_device::serin_ready_irq)
{
	if (m_IRQEN & IRQ_SERIN)
	{
		m_IRQST |= IRQ_SERIN;
		LOG_IRQ("POKEY SERIN IRQ raised\n");
		m_irq_w_cb(ASSERT_LINE);
	}
}

TIMER_CALLBACK_MEMBER(pokey_device::sync_write)
{
	offs_t offset = (param >> 8) & 0xff;
	uint8_t data = param & 0xff;
	write_internal(offset, data);
}

TIMER_CALLBACK_MEMBER(pokey_device::sync_pot)
{
	//logerror("x %02x \n", (param & 0x20));
	m_ALLPOT |= (param & 0xff);
}

TIMER_CALLBACK_MEMBER(pokey_device::sync_set_irqst)
{
	if (m_IRQEN & param)
	{
		LOG_IRQ("POKEY TIMR%d IRQ raised\n", param);
		m_IRQST |=  (param & 0xff);
		m_irq_w_cb(ASSERT_LINE);
	}
}

void pokey_device::execute_run()
{
	do
	{
		step_one_clock();
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
		uint8_t ret = m_keyboard_r(m_kbd_cnt);

		switch (m_kbd_cnt)
		{
		case POK_KEY_BREAK:
			if (ret & 2)
			{
				/* check if the break IRQ is enabled */
				if (m_IRQEN & IRQ_BREAK)
				{
					LOG_IRQ("POKEY BREAK IRQ raised\n");
					m_IRQST |= IRQ_BREAK;
					m_irq_w_cb(ASSERT_LINE);
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
			if (!(m_SKCTL & SK_DEBOUNCE) || (m_kbd_latch & 0x3f) == m_kbd_cnt)
			{
				if (ret & 1)
				{
					m_KBCODE = (m_SKCTL & SK_DEBOUNCE) ? m_kbd_latch : (m_kbd_latch & 0xc0) | m_kbd_cnt;
					m_SKSTAT |= SK_KEYBD;
					if (m_IRQEN & IRQ_KEYBD)
					{
						/* last interrupt not acknowledged ? */
						if (m_IRQST & IRQ_KEYBD)
							m_SKSTAT |= SK_KBERR;
						LOG_IRQ("POKEY KEYBD IRQ raised\n");
						m_IRQST |= IRQ_KEYBD;
						m_irq_w_cb(ASSERT_LINE);
					}
					m_kbd_state++;
				}
				else
					m_kbd_state = 0;
			}
			break;
		case 2: /* waiting for release */
			if (!(m_SKCTL & SK_DEBOUNCE) || (m_kbd_latch & 0x3f) == m_kbd_cnt)
			{
				if ((ret & 1)==0)
					m_kbd_state++;
			}
			break;
		case 3:
			if (!(m_SKCTL & SK_DEBOUNCE) || (m_kbd_latch & 0x3f) == m_kbd_cnt)
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
	m_pot_counter++;
	uint8_t upd = 0;
	for (int pot = 0; pot < 8; pot++)
	{
		if ((m_POTx[pot]<m_pot_counter) || (m_pot_counter == 228))
		{
			upd |= (1 << pot);
			/* latching is emulated in read */
		}
	}
	// some pots latched?
	if (upd != 0)
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(pokey_device::sync_pot), this), upd);
}

/*
 * http://www.atariage.com/forums/topic/3328-sio-protocol/page__st__100#entry1680190:
 * I noticed that the Pokey counters have clocked carry (actually, "borrow") positions that delay the
 * counter by 3 cycles, plus the 1 reset clock. So 16 bit mode has 6 carry delays and a reset clock.
 * I'm sure this was done because the propagation delays limited the number of cells the subtraction could ripple though.
 *
 */

void pokey_device::step_one_clock()
{
	if (m_SKCTL & SK_RESET)
	{
		/* Clocks only count if we are not in a reset */

		/* polynom pointers */
		if (++m_p4 == 0x0000f)
			m_p4 = 0;
		if (++m_p5 == 0x0001f)
			m_p5 = 0;
		if (++m_p9 == 0x001ff)
			m_p9 = 0;
		if (++m_p17 == 0x1ffff)
			m_p17 = 0;

		/* CLK_1: no presacler */
		int clock_triggered[3] = {1,0,0};
		/* CLK_28: prescaler 63.9211 kHz */
		if (++m_clock_cnt[CLK_28] >= DIV_64)
		{
			m_clock_cnt[CLK_28] = 0;
			clock_triggered[CLK_28] = 1;
		}
		/* CLK_114 prescaler 15.6999 kHz */
		if (++m_clock_cnt[CLK_114] >= DIV_15)
		{
			m_clock_cnt[CLK_114] = 0;
			clock_triggered[CLK_114] = 1;
		}

		if ((m_AUDCTL & CH1_HICLK) && (clock_triggered[CLK_1]))
		{
			if (m_AUDCTL & CH12_JOINED)
				m_channel[CHAN1].inc_chan(*this, 7);
			else
				m_channel[CHAN1].inc_chan(*this, 4);
		}

		int base_clock = (m_AUDCTL & CLK_15KHZ) ? CLK_114 : CLK_28;

		if ((!(m_AUDCTL & CH1_HICLK)) && (clock_triggered[base_clock]))
			m_channel[CHAN1].inc_chan(*this, 1);

		if ((m_AUDCTL & CH3_HICLK) && (clock_triggered[CLK_1]))
		{
			if (m_AUDCTL & CH34_JOINED)
				m_channel[CHAN3].inc_chan(*this, 7);
			else
				m_channel[CHAN3].inc_chan(*this, 4);
		}

		if ((!(m_AUDCTL & CH3_HICLK)) && (clock_triggered[base_clock]))
			m_channel[CHAN3].inc_chan(*this, 1);

		if (clock_triggered[base_clock])
		{
			if (!(m_AUDCTL & CH12_JOINED))
				m_channel[CHAN2].inc_chan(*this, 1);
			if (!(m_AUDCTL & CH34_JOINED))
				m_channel[CHAN4].inc_chan(*this, 1);
		}

		/* Potentiometer handling */
		if ((clock_triggered[CLK_114] || (m_SKCTL & SK_PADDLE)) && (m_pot_counter < 228))
			step_pot();

		/* Keyboard */
		if (clock_triggered[CLK_114] && (m_SKCTL & SK_KEYSCAN))
			step_keyboard();
	}

	if (m_channel[CHAN3].check_borrow())
	{
		if (m_AUDCTL & CH34_JOINED)
			m_channel[CHAN4].inc_chan(*this, 1);
		else
			m_channel[CHAN3].reset_channel();

		process_channel(CHAN3);
		/* is this a filtering channel (3/4) and is the filter active? */
		if (m_AUDCTL & CH1_FILTER)
			m_channel[CHAN1].sample();
		else
			m_channel[CHAN1].m_filter_sample = 1;

		m_old_raw_inval = true;
	}

	if (m_channel[CHAN4].check_borrow())
	{
		if (m_AUDCTL & CH34_JOINED)
			m_channel[CHAN3].reset_channel();
		m_channel[CHAN4].reset_channel();
		process_channel(CHAN4);
		/* is this a filtering channel (3/4) and is the filter active? */
		if (m_AUDCTL & CH2_FILTER)
			m_channel[CHAN2].sample();
		else
			m_channel[CHAN2].m_filter_sample = 1;

		m_old_raw_inval = true;
	}

	if ((m_SKCTL & SK_TWOTONE) && (m_channel[CHAN2].m_borrow_cnt == 1))
	{
		m_channel[CHAN1].reset_channel();
		m_old_raw_inval = true;
	}

	if (m_channel[CHAN1].check_borrow())
	{
		if (m_AUDCTL & CH12_JOINED)
			m_channel[CHAN2].inc_chan(*this, 1);
		else
			m_channel[CHAN1].reset_channel();

		// TODO: If two-tone is enabled *and* serial output == 1 then reset the channel 2 timer.

		process_channel(CHAN1);
	}

	if (m_channel[CHAN2].check_borrow())
	{
		if (m_AUDCTL & CH12_JOINED)
			m_channel[CHAN1].reset_channel();

		m_channel[CHAN2].reset_channel();

		process_channel(CHAN2);
	}

	if (m_old_raw_inval)
	{
		uint32_t sum = 0;
		for (int ch = 0; ch < 4; ch++)
		{
			sum |= (((m_channel[ch].m_output ^ m_channel[ch].m_filter_sample) || (m_channel[ch].m_AUDC & VOLUME_ONLY)) ?
				((m_channel[ch].m_AUDC & VOLUME_MASK) << (ch * 4)) : 0);
		}

		if (m_out_raw != sum)
			m_stream->update();

		m_old_raw_inval = false;
		m_out_raw = sum;
	}
}

//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void pokey_device::sound_stream_update(sound_stream &stream)
{
	if (m_output_type == LEGACY_LINEAR)
	{
		int32_t out = 0;
		for (int i = 0; i < 4; i++)
			out += ((m_out_raw >> (4*i)) & 0x0f);
		out *= POKEY_DEFAULT_GAIN;
		out = (out > 0x7fff) ? 0x7fff : out;
		sound_stream::sample_t outsamp = out * sound_stream::sample_t(1.0 / 32768.0);
		stream.fill(0, outsamp);
	}
	else if (m_output_type == RC_LOWPASS)
	{
		double rTot = m_voltab[m_out_raw];

		double V0 = rTot / (rTot+m_r_pullup) * m_v_ref / 5.0;
		double mult = (m_cap == 0.0) ? 1.0 : 1.0 - exp(-(rTot + m_r_pullup) / (m_cap * m_r_pullup * rTot) * m_clock_period.as_double());

		for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
		{
			/* store sum of output signals into the buffer */
			m_out_filter += (V0 - m_out_filter) * mult;
			stream.put(0, sampindex, m_out_filter);
		}
	}
	else if (m_output_type == OPAMP_C_TO_GROUND)
	{
		double rTot = m_voltab[m_out_raw];
		/* In this configuration there is a capacitor in parallel to the pokey output to ground.
		 * With a LM324 in LTSpice this causes the opamp circuit to oscillate at around 100 kHz.
		 * We are ignoring the capacitor here, since this oscillation would not be audible.
		 */

		/* This post-pokey stage usually has a high-pass filter behind it
		 * It is approximated by eliminating m_v_ref ( -1.0 term)
		 */

		double V0 = ((rTot+m_r_pullup) / rTot - 1.0) * m_v_ref  / 5.0;
		stream.fill(0, V0);
	}
	else if (m_output_type == OPAMP_LOW_PASS)
	{
		double rTot = m_voltab[m_out_raw];
		/* This post-pokey stage usually has a low-pass filter behind it
		 * It is approximated by not adding in VRef below.
		 */

		double V0 = (m_r_pullup / rTot) * m_v_ref  / 5.0;
		double mult = (m_cap == 0.0) ? 1.0 : 1.0 - exp(-1.0 / (m_cap * m_r_pullup) * m_clock_period.as_double());

		for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
		{
			/* store sum of output signals into the buffer */
			m_out_filter += (V0 - m_out_filter) * mult;
			stream.put(0, sampindex, m_out_filter);
		}
	}
	else if (m_output_type == DISCRETE_VAR_R)
	{
		stream.fill(0, m_voltab[m_out_raw]);
	}
}

//-------------------------------------------------
//  read - memory interface for reading the active status
//-------------------------------------------------

uint8_t pokey_device::read(offs_t offset)
{
	int data, pot;

	machine().scheduler().synchronize(); /* force resync */

	switch (offset & 15)
	{
	case POT0_C: case POT1_C: case POT2_C: case POT3_C:
	case POT4_C: case POT5_C: case POT6_C: case POT7_C:
		pot = offset & 7;
		if (m_ALLPOT & (1 << pot))
		{
			/* we have a value measured */
			data = m_POTx[pot];
			LOG("%s: POKEY read POT%d (final value)  $%02x\n", machine().describe_context(), pot, data);
		}
		else
		{
			data = m_pot_counter;
			LOG("%s: POKEY read POT%d (interpolated) $%02x\n", machine().describe_context(), pot, data);
		}
		break;

	case ALLPOT_C:
		/****************************************************************
		 * If the 2 least significant bits of SKCTL are 0, the ALLPOTs
		 * are disabled (SKRESET). Thanks to MikeJ for pointing this out.
		 ****************************************************************/
		if ((m_SKCTL & SK_RESET) == 0)
		{
			data = m_ALLPOT;
			LOG("%s: POKEY ALLPOT internal $%02x (reset)\n", machine().describe_context(), data);
		}
		else if (!m_allpot_r_cb.isunset())
		{
			m_ALLPOT = data = m_allpot_r_cb(offset);
			LOG("%s: POKEY ALLPOT callback $%02x\n", machine().describe_context(), data);
		}
		else
		{
			data = m_ALLPOT ^ 0xff;
			LOG("%s: POKEY ALLPOT internal $%02x\n", machine().describe_context(), data);
		}
		break;

	case KBCODE_C:
		data = m_KBCODE;
		break;

	case RANDOM_C:
		if (m_AUDCTL & POLY9)
		{
			data = m_poly9[m_p9] & 0xff;
			LOG_RAND("%s: POKEY rand9[$%05x]: $%02x\n", machine().describe_context(), m_p9, data);
		}
		else
		{
			data = (m_poly17[m_p17] >> 8) & 0xff;
			LOG_RAND("%s: POKEY rand17[$%05x]: $%02x\n", machine().describe_context(), m_p17, data);
		}
		break;

	case SERIN_C:
		if (!m_serin_r_cb.isunset())
			m_SERIN = m_serin_r_cb(offset);
		data = m_SERIN;
		LOG("%s: POKEY SERIN  $%02x\n", machine().describe_context(), data);
		break;

	case IRQST_C:
		/* IRQST is an active low input port; we keep it active high */
		/* internally to ease the (un-)masking of bits */
		data = m_IRQST ^ 0xff;
		LOG("%s: POKEY IRQST  $%02x\n", machine().describe_context(), data);
		break;

	case SKSTAT_C:
		/* SKSTAT is also an active low input port */
		data = m_SKSTAT ^ 0xff;
		LOG("%s: POKEY SKSTAT $%02x\n", machine().describe_context(), data);
		break;

	default:
		LOG("%s: POKEY register $%02x\n", machine().describe_context(), offset);
		data = 0xff;
		break;
	}
	return data;
}


//-------------------------------------------------
//  write - memory interface for write
//-------------------------------------------------

void pokey_device::write(offs_t offset, uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(pokey_device::sync_write), this), (offset << 8) | data);
}

void pokey_device::write_internal(offs_t offset, uint8_t data)
{
	/* determine which address was changed */
	switch (offset & 15)
	{
	case AUDF1_C:
		LOG_SOUND("%s: AUDF1 = $%02x\n", machine().describe_context(), data);
		m_channel[CHAN1].m_AUDF = data;
		break;

	case AUDC1_C:
		LOG_SOUND("%s: POKEY AUDC1  $%02x (%s)\n", machine().describe_context(), data, audc2str(data));
		m_channel[CHAN1].m_AUDC = data;
		m_old_raw_inval = true;
		break;

	case AUDF2_C:
		LOG_SOUND("%s: POKEY AUDF2  $%02x\n", machine().describe_context(), data);
		m_channel[CHAN2].m_AUDF = data;
		break;

	case AUDC2_C:
		LOG_SOUND("%s: POKEY AUDC2  $%02x (%s)\n", machine().describe_context(), data, audc2str(data));
		m_channel[CHAN2].m_AUDC = data;
		m_old_raw_inval = true;
		break;

	case AUDF3_C:
		LOG_SOUND("%s: POKEY AUDF3  $%02x\n", machine().describe_context(), data);
		m_channel[CHAN3].m_AUDF = data;
		break;

	case AUDC3_C:
		LOG_SOUND("%s: POKEY AUDC3  $%02x (%s)\n", machine().describe_context(), data, audc2str(data));
		m_channel[CHAN3].m_AUDC = data;
		m_old_raw_inval = true;
		break;

	case AUDF4_C:
		LOG_SOUND("%s: POKEY AUDF4  $%02x\n", machine().describe_context(), data);
		m_channel[CHAN4].m_AUDF = data;
		break;

	case AUDC4_C:
		LOG_SOUND("%s: POKEY AUDC4  $%02x (%s)\n", machine().describe_context(), data, audc2str(data));
		m_channel[CHAN4].m_AUDC = data;
		m_old_raw_inval = true;
		break;

	case AUDCTL_C:
		if (data == m_AUDCTL)
			return;
		LOG_SOUND("%s: POKEY AUDCTL $%02x (%s)\n", machine().describe_context(), data, audctl2str(data));
		m_AUDCTL = data;
		m_old_raw_inval = true;
		break;

	case STIMER_C:
		LOG_TIMER("%s: POKEY STIMER $%02x\n", machine().describe_context(), data);

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
		m_old_raw_inval = true;
		break;

	case SKREST_C:
		/* reset SKSTAT */
		LOG("%s: POKEY SKREST $%02x\n", machine().describe_context(), data);
		m_SKSTAT &= ~(SK_FRAME|SK_OVERRUN|SK_KBERR);
		break;

	case POTGO_C:
		LOG("%s: POKEY POTGO  $%02x\n", machine().describe_context(), data);
		if (m_SKCTL & SK_RESET)
			pokey_potgo();
		break;

	case SEROUT_C:
		LOG("%s: POKEY SEROUT $%02x\n", machine().describe_context(), data);
		// TODO: convert to real serial comms, fix timings
		// SEROC (1) serial out in progress (0) serial out complete
		// in progress status is necessary for a800 telelnk2 to boot
		m_IRQST &= ~IRQ_SEROC;

		m_serout_w_cb(offset, data);
		m_SKSTAT |= SK_SEROUT;
		/*
		 * These are arbitrary values, tested with some custom boot
		 * loaders from Ballblazer and Escape from Fractalus
		 * The real times are unknown
		 */
		m_serout_ready_timer->adjust(attotime::from_usec(200));
		/* 10 bits (assumption 1 start, 8 data and 1 stop bit) take how long? */
		m_serout_complete_timer->adjust(attotime::from_usec(2000));
		break;

	case IRQEN_C:
		LOG("%s: POKEY IRQEN  $%02x\n", machine().describe_context(), data);

		/* acknowledge one or more IRQST bits ? */
		if (m_IRQST & ~data)
		{
			/* reset IRQST bits that are masked now, except the SEROC bit (acid5200 pokey_seroc test) */
			m_IRQST &= (IRQ_SEROC | data);
		}
		/* store irq enable */
		m_IRQEN = data;
		/* if SEROC irq is enabled trigger an irq (acid5200 pokey_seroc test) */
		if (m_IRQEN & m_IRQST & IRQ_SEROC)
		{
			LOG_IRQ("POKEY SEROC IRQ enabled\n");
			m_irq_w_cb(ASSERT_LINE);
		}
		else if (!(m_IRQEN & m_IRQST))
		{
			LOG_IRQ("POKEY IRQs all cleared\n");
			m_irq_w_cb(CLEAR_LINE);
		}
		break;

	case SKCTL_C:
		if (data == m_SKCTL)
			return;
		LOG("%s: POKEY SKCTL  $%02x\n", machine().describe_context(), data);
		m_SKCTL = data;
		if (!(data & SK_RESET))
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
		if (!(data & SK_KEYSCAN))
		{
			m_SKSTAT &= ~SK_KEYBD;
			m_kbd_cnt = 0;
			m_kbd_state = 0;
		}
		m_old_raw_inval = true;
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

void pokey_device::sid_w(int state)
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
	m_serin_ready_timer->adjust(m_clock_period * after, 0);
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
		m_old_raw_inval = true;
	}
}


void pokey_device::pokey_potgo()
{
	LOG("pokey_potgo\n");

	m_ALLPOT = 0x00;
	m_pot_counter = 0;

	for (int pot = 0; pot < 8; pot++)
	{
		m_POTx[pot] = 228;
		if (!m_pot_r_cb[pot].isunset())
		{
			int r = m_pot_r_cb[pot](pot);

			LOG("POKEY pot_r(%d) returned $%02x\n", pot, r);
			if (r >= 228)
				r = 228;

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
	if (VERBOSE & LOG_GENERAL)
		for (int j=0; j<16; j++)
		{
			rTot = 1.0 / r_chan[j] + 3.0 / r_chan[0];
			rTot = 1.0 / rTot;
			LOG("%3d - %4.3f\n", j, rTot / (rTot+pull_up)*4.75);
		}
	for (int j=0; j<0x10000; j++)
	{
		rTot = 0;
		for (int i=0; i<4; i++)
		{
			rTot += 1.0 / r_chan[(j >> (i*4)) & 0x0f];
		}
		rTot = 1.0 / rTot;
		m_voltab[j] = rTot;
	}

}

void pokey_device::poly_init_4_5(uint32_t *poly, int size)
{
	LOG_POLY("poly %d\n", size);

	int mask = (1 << size) - 1;
	uint32_t lfsr = 0;

	int const xorbit = size - 1;
	for (int i = 0; i < mask; i++)
	{
		lfsr = (lfsr << 1) | (~((lfsr >> 2) ^ (lfsr >> xorbit)) & 1);
		*poly = lfsr & mask;
		poly++;
	}
}

void pokey_device::poly_init_9_17(uint32_t *poly, int size)
{
	LOG_RAND("rand %d\n", size);

	const uint32_t mask = util::make_bitmask<uint32_t>(size);
	uint32_t lfsr = mask;

	if (size == 17)
	{
		for (uint32_t i = 0; i < mask; i++)
		{
			// calculate next bit @ 7
			const uint32_t in8 = BIT(lfsr, 8) ^ BIT(lfsr, 13);
			const uint32_t in = BIT(lfsr, 0);
			lfsr = lfsr >> 1;
			lfsr = (lfsr & 0xff7f) | (in8 << 7);
			lfsr = (in << 16) | lfsr;
			*poly = lfsr;
			LOG_RAND("%05x: %02x\n", i, *poly);
			poly++;
		}
	}
	else // size == 9
	{
		for (uint32_t i = 0; i < mask; i++)
		{
			// calculate next bit
			const uint32_t in = BIT(lfsr, 0) ^ BIT(lfsr, 5);
			lfsr = lfsr >> 1;
			lfsr = (in << 8) | lfsr;
			*poly = lfsr;
			LOG_RAND("%05x: %02x\n", i, *poly);
			poly++;
		}
	}

}

char *pokey_device::audc2str(int val)
{
	static char buff[80];
	if (val & NOTPOLY5)
	{
		if (val & PURE)
			strcpy(buff,"pure");
		else if (val & POLY4)
			strcpy(buff,"poly4");
		else
			strcpy(buff,"poly9/17");
	}
	else
	{
		if (val & PURE)
			strcpy(buff,"poly5");
		else if (val & POLY4)
			strcpy(buff,"poly4+poly5");
		else
			strcpy(buff,"poly9/17+poly5");
	}
	return buff;
}

char *pokey_device::audctl2str(int val)
{
	static char buff[80];
	if (val & POLY9)
		strcpy(buff,"poly9");
	else
		strcpy(buff,"poly17");
	if (val & CH1_HICLK)
		strcat(buff,"+ch1hi");
	if (val & CH3_HICLK)
		strcat(buff,"+ch3hi");
	if (val & CH12_JOINED)
		strcat(buff,"+ch1/2");
	if (val & CH34_JOINED)
		strcat(buff,"+ch3/4");
	if (val & CH1_FILTER)
		strcat(buff,"+ch1filter");
	if (val & CH2_FILTER)
		strcat(buff,"+ch2filter");
	if (val & CLK_15KHZ)
		strcat(buff,"+clk15");
	return buff;
}

pokey_device::pokey_channel::pokey_channel() :
	m_AUDF(0),
	m_AUDC(0),
	m_borrow_cnt(0),
	m_counter(0),
	m_output(0),
	m_filter_sample(0)
{
}
