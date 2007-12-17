/***************************************************************************

    This code is used by the following module:

    timeplt.c
    pooyan.c
    locomotn.c
    tutankhm.c
    rocnrope.c

***************************************************************************/

#include "driver.h"
#include "sound/ay8910.h"
#include "sound/flt_rc.h"
#include "timeplt.h"

static READ8_HANDLER( timeplt_portB_r );
static WRITE8_HANDLER( timeplt_filter_w );

ADDRESS_MAP_START( timeplt_sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x2000, 0x23ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x3000, 0x33ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x4000, 0x4000) AM_READ(AY8910_read_port_0_r)
	AM_RANGE(0x6000, 0x6000) AM_READ(AY8910_read_port_1_r)
ADDRESS_MAP_END

ADDRESS_MAP_START( timeplt_sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x2000, 0x23ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x3000, 0x33ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x4000, 0x4000) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x5000, 0x5000) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x6000, 0x6000) AM_WRITE(AY8910_write_port_1_w)
	AM_RANGE(0x7000, 0x7000) AM_WRITE(AY8910_control_port_1_w)
	AM_RANGE(0x8000, 0x8fff) AM_WRITE(timeplt_filter_w)
ADDRESS_MAP_END

ADDRESS_MAP_START( locomotn_sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x2000, 0x23ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x4000, 0x4000) AM_READ(AY8910_read_port_0_r)
	AM_RANGE(0x6000, 0x6000) AM_READ(AY8910_read_port_1_r)
ADDRESS_MAP_END

ADDRESS_MAP_START( locomotn_sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x2000, 0x23ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x3000, 0x3fff) AM_WRITE(timeplt_filter_w)
	AM_RANGE(0x4000, 0x4000) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x5000, 0x5000) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x6000, 0x6000) AM_WRITE(AY8910_write_port_1_w)
	AM_RANGE(0x7000, 0x7000) AM_WRITE(AY8910_control_port_1_w)
ADDRESS_MAP_END


struct AY8910interface timeplt_ay8910_interface =
{
	soundlatch_r,
	timeplt_portB_r
};


/* The timer clock which feeds the upper 4 bits of                      */
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

static int timeplt_timer[10] =
{
	0x00, 0x10, 0x20, 0x30, 0x40, 0x90, 0xa0, 0xb0, 0xa0, 0xd0
};

static READ8_HANDLER( timeplt_portB_r )
{
	/* need to protect from totalcycles overflow */
	static int last_totalcycles = 0;

	/* number of Z80 clock cycles to count */
	static int clock;

	int current_totalcycles;

	current_totalcycles = activecpu_gettotalcycles();
	clock = (clock + (current_totalcycles-last_totalcycles)) % 5120;

	last_totalcycles = current_totalcycles;

	return timeplt_timer[clock/512];
}


static void filter_w(int chip, int channel, int data)
{
	int C = 0;

	if (data & 1) C += 220000;	/* 220000pF = 0.220uF */
	if (data & 2) C +=  47000;	/*  47000pF = 0.047uF */
	filter_rc_set_RC(3*chip + channel,FLT_RC_LOWPASS, 1000,5100,0,CAP_P(C));
}

static WRITE8_HANDLER( timeplt_filter_w )
{
	filter_w(0, 0, (offset >>  6) & 3);
	filter_w(0, 1, (offset >>  8) & 3);
	filter_w(0, 2, (offset >> 10) & 3);
	filter_w(1, 0, (offset >>  0) & 3);
	filter_w(1, 1, (offset >>  2) & 3);
	filter_w(1, 2, (offset >>  4) & 3);
}


WRITE8_HANDLER( timeplt_sh_irqtrigger_w )
{
	static int last;

	if (last == 0 && data)
	{
		/* setting bit 0 low then high triggers IRQ on the sound CPU */
		cpunum_set_input_line_and_vector(1,0,HOLD_LINE,0xff);
	}

	last = data;
}

