/***************************************************************************

  This file contains functions to emulate the sound hardware found on
  Scramble type boards.

  There are two types, one has 2 AY8910's and the other one has one of the
  AY8910's removed.  Interestingly, it appears that the one AY8910 version
  came after the 2 AY8910 one.  This is supported by the fact that in the
  one 8190 version, the filter system uses bits 6-11, while bits 0-5 are
  left unused.

***************************************************************************/


#include "driver.h"
#include "cpu/z80/z80.h"
#include "machine/7474.h"
#include "sound/flt_rc.h"
#include "includes/galaxian.h"


/* The timer clock in Scramble which feeds the upper 4 bits of          */
/* AY-3-8910 port A is based on the same clock                          */
/* feeding the sound CPU Z80.  It is a divide by                        */
/* 5120, formed by a standard divide by 512,                            */
/* followed by a divide by 10 using a 4 bit                             */
/* bi-quinary count sequence. (See LS90 data sheet                      */
/* for an example).                                                     */
/*                                                                      */
/* Bit 4 comes from the output of the divide by 1024                    */
/*       0, 1, 0, 1, 0, 1, 0, 1, 0, 1                                   */
/* Bit 5 comes from the QC output of the LS90 producing a sequence of   */
/*       0, 0, 1, 1, 0, 0, 1, 1, 1, 0                                   */
/* Bit 6 comes from the QD output of the LS90 producing a sequence of   */
/*       0, 0, 0, 0, 1, 0, 0, 0, 0, 1                                   */
/* Bit 7 comes from the QA output of the LS90 producing a sequence of   */
/*       0, 0, 0, 0, 0, 1, 1, 1, 1, 1                                   */

static int scramble_timer[10] =
{
	0x00, 0x10, 0x20, 0x30, 0x40, 0x90, 0xa0, 0xb0, 0xa0, 0xd0
};

READ8_HANDLER( scramble_portB_r )
{
	/* need to protect from totalcycles overflow */
	static int last_totalcycles = 0;

	/* number of Z80 clock cycles to count */
	static int clock;

	int current_totalcycles;

	current_totalcycles = activecpu_gettotalcycles();
	clock = (clock + (current_totalcycles-last_totalcycles)) % 5120;

	last_totalcycles = current_totalcycles;

	return scramble_timer[clock/512];
}



/* The timer clock in Frogger which feeds the upper 4 bits of           */
/* AY-3-8910 port A is based on the same clock                          */
/* feeding the sound CPU Z80.  It is a divide by                        */
/* 5120, formed by a standard divide by 512,                            */
/* followed by a divide by 10 using a 4 bit                             */
/* bi-quinary count sequence. (See LS90 data sheet                      */
/* for an example).                                                     */
/*                                                                      */
/* Bit 4 comes from the output of the divide by 1024                    */
/*       0, 1, 0, 1, 0, 1, 0, 1, 0, 1                                   */
/* Bit 3 comes from the QC output of the LS90 producing a sequence of   */
/*       0, 0, 1, 1, 0, 0, 1, 1, 1, 0                                   */
/* Bit 6 comes from the QD output of the LS90 producing a sequence of   */
/*       0, 0, 0, 0, 1, 0, 0, 0, 0, 1                                   */
/* Bit 7 comes from the QA output of the LS90 producing a sequence of   */
/*       0, 0, 0, 0, 0, 1, 1, 1, 1, 1                                   */

static int frogger_timer[10] =
{
	0x00, 0x10, 0x08, 0x18, 0x40, 0x90, 0x88, 0x98, 0x88, 0xd0
};

READ8_HANDLER( frogger_portB_r )
{
	/* need to protect from totalcycles overflow */
	static int last_totalcycles = 0;

	/* number of Z80 clock cycles to count */
	static int clock;

	int current_totalcycles;

	current_totalcycles = activecpu_gettotalcycles();
	clock = (clock + (current_totalcycles-last_totalcycles)) % 5120;

	last_totalcycles = current_totalcycles;

	return frogger_timer[clock/512];
}


WRITE8_HANDLER( scramble_sh_irqtrigger_w )
{
	/* the complement of bit 3 is connected to the flip-flop's clock */
	TTL7474_clock_w(2, ~data & 0x08);
	TTL7474_update(2);

	/* bit 4 is sound disable */
	sound_global_enable(~data & 0x10);
}

WRITE8_HANDLER( sfx_sh_irqtrigger_w )
{
	/* bit 1 is connected to the flip-flop's clock */
	TTL7474_clock_w(3, data & 0x01);
	TTL7474_update(3);
}

WRITE8_HANDLER( mrkougar_sh_irqtrigger_w )
{
	/* the complement of bit 3 is connected to the flip-flop's clock */
	TTL7474_clock_w(2, ~data & 0x08);
	TTL7474_update(2);
}

WRITE8_HANDLER( froggrmc_sh_irqtrigger_w )
{
	/* the complement of bit 0 is connected to the flip-flop's clock */
	TTL7474_clock_w(2, ~data & 0x01);
	TTL7474_update(2);
}


static int scramble_sh_irq_callback(int irqline)
{
	/* interrupt acknowledge clears the flip-flop --
       we need to pulse the CLR line because MAME's core never clears this
       line, only asserts it */
	TTL7474_clear_w(2, 0);
	TTL7474_update(2);

	TTL7474_clear_w(2, 1);
	TTL7474_update(2);

	return 0xff;
}

static int sfx_sh_irq_callback(int irqline)
{
	/* interrupt acknowledge clears the flip-flop --
       we need to pulse the CLR line because MAME's core never clears this
       line, only asserts it */
	TTL7474_clear_w(3, 0);
	TTL7474_update(3);

	TTL7474_clear_w(3, 1);
	TTL7474_update(3);

	return 0xff;
}


static void scramble_sh_7474_callback(void)
{
	/* the Q bar is connected to the Z80's INT line.  But since INT is complemented, */
	/* we need to complement Q bar */
	cpunum_set_input_line(1, 0, !TTL7474_output_comp_r(2) ? ASSERT_LINE : CLEAR_LINE);
}

static void sfx_sh_7474_callback(void)
{
	/* the Q bar is connected to the Z80's INT line.  But since INT is complemented, */
	/* we need to complement Q bar */
	cpunum_set_input_line(2, 0, !TTL7474_output_comp_r(3) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_HANDLER( hotshock_sh_irqtrigger_w )
{
	cpunum_set_input_line(1, 0, ASSERT_LINE);
}

READ8_HANDLER( hotshock_soundlatch_r )
{
	cpunum_set_input_line(1, 0, CLEAR_LINE);
	return soundlatch_r(0);
}

static void filter_w(int chip, int channel, int data)
{
	int C;


	C = 0;
	if (data & 1)
		C += 220000;	/* 220000pF = 0.220uF */
	if (data & 2)
		C +=  47000;	/*  47000pF = 0.047uF */
	if (sndti_exists(SOUND_FILTER_RC, 3*chip + channel))
		filter_rc_set_RC(3*chip + channel,FLT_RC_LOWPASS,1000,5100,0,CAP_P(C));
}

WRITE8_HANDLER( scramble_filter_w )
{
	filter_w(1, 0, (offset >>  0) & 3);
	filter_w(1, 1, (offset >>  2) & 3);
	filter_w(1, 2, (offset >>  4) & 3);
	filter_w(0, 0, (offset >>  6) & 3);
	filter_w(0, 1, (offset >>  8) & 3);
	filter_w(0, 2, (offset >> 10) & 3);
}

WRITE8_HANDLER( frogger_filter_w )
{
	filter_w(0, 0, (offset >>  6) & 3);
	filter_w(0, 1, (offset >>  8) & 3);
	filter_w(0, 2, (offset >> 10) & 3);
}


static const struct TTL7474_interface scramble_sh_7474_intf =
{
	scramble_sh_7474_callback
};

static const struct TTL7474_interface sfx_sh_7474_intf =
{
	sfx_sh_7474_callback
};


void scramble_sh_init(void)
{
	cpunum_set_irq_callback(1, scramble_sh_irq_callback);

	TTL7474_config(2, &scramble_sh_7474_intf);

	/* PR is always 0, D is always 1 */
	TTL7474_d_w(2, 1);
}

void sfx_sh_init(void)
{
	cpunum_set_irq_callback(2, sfx_sh_irq_callback);

	TTL7474_config(3, &sfx_sh_7474_intf);

	/* PR is always 0, D is always 1 */
	TTL7474_d_w(3, 1);
}


