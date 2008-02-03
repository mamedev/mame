/***************************************************************************

    This code is used by the following module:

    timeplt.c
    pooyan.c
    locomotn.c
    tutankhm.c
    rocnrope.c

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "sound/ay8910.h"
#include "sound/flt_rc.h"
#include "timeplt.h"


#define MASTER_CLOCK		XTAL_14_31818MHz


static UINT8 timeplt_last_irq_state;


/*************************************
 *
 *  Initialization
 *
 *************************************/

static SOUND_START( timeplt )
{
	timeplt_last_irq_state = 0;
	state_save_register_global(timeplt_last_irq_state);
}



/*************************************
 *
 *  Sound timer
 *
 *************************************/

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

static READ8_HANDLER( timeplt_portB_r )
{
	static const int timeplt_timer[10] =
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x90, 0xa0, 0xb0, 0xa0, 0xd0
	};
	return timeplt_timer[(activecpu_gettotalcycles() / 512) % 10];
}



/*************************************
 *
 *  Filter controls
 *
 *************************************/

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



/*************************************
 *
 *  External interfaces
 *
 *************************************/

WRITE8_HANDLER( timeplt_sh_irqtrigger_w )
{
	if (timeplt_last_irq_state == 0 && data)
	{
		/* setting bit 0 low then high triggers IRQ on the sound CPU */
		cpunum_set_input_line_and_vector(Machine, 1,0,HOLD_LINE,0xff);
	}

	timeplt_last_irq_state = data;
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( timeplt_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x3000, 0x33ff) AM_MIRROR(0x0c00) AM_RAM
	AM_RANGE(0x4000, 0x4000) AM_MIRROR(0x0fff) AM_READWRITE(AY8910_read_port_0_r, AY8910_write_port_0_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0x0fff) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x0fff) AM_READWRITE(AY8910_read_port_1_r, AY8910_write_port_1_w)
	AM_RANGE(0x7000, 0x7000) AM_MIRROR(0x0fff) AM_WRITE(AY8910_control_port_1_w)
	AM_RANGE(0x8000, 0xffff) AM_WRITE(timeplt_filter_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( locomotn_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_MIRROR(0x0c00) AM_RAM
	AM_RANGE(0x3000, 0x3fff) AM_WRITE(timeplt_filter_w)
	AM_RANGE(0x4000, 0x4000) AM_MIRROR(0x0fff) AM_READWRITE(AY8910_read_port_0_r, AY8910_write_port_0_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0x0fff) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x0fff) AM_READWRITE(AY8910_read_port_1_r, AY8910_write_port_1_w)
	AM_RANGE(0x7000, 0x7000) AM_MIRROR(0x0fff) AM_WRITE(AY8910_control_port_1_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Sound chip interfaces
 *
 *************************************/

static const struct AY8910interface timeplt_ay8910_interface =
{
	soundlatch_r,
	timeplt_portB_r
};



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

MACHINE_DRIVER_START( timeplt_sound )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("tpsound",Z80,MASTER_CLOCK/8)
	MDRV_CPU_PROGRAM_MAP(timeplt_sound_map,0)

	MDRV_SOUND_START(timeplt)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, MASTER_CLOCK/8)
	MDRV_SOUND_CONFIG(timeplt_ay8910_interface)
	MDRV_SOUND_ROUTE(0, "filter.0.0", 0.60)
	MDRV_SOUND_ROUTE(1, "filter.0.1", 0.60)
	MDRV_SOUND_ROUTE(2, "filter.0.2", 0.60)

	MDRV_SOUND_ADD(AY8910, MASTER_CLOCK/8)
	MDRV_SOUND_ROUTE(0, "filter.1.0", 0.60)
	MDRV_SOUND_ROUTE(1, "filter.1.1", 0.60)
	MDRV_SOUND_ROUTE(2, "filter.1.2", 0.60)

	MDRV_SOUND_ADD_TAG("filter.0.0", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_SOUND_ADD_TAG("filter.0.1", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_SOUND_ADD_TAG("filter.0.2", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD_TAG("filter.1.0", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_SOUND_ADD_TAG("filter.1.1", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_SOUND_ADD_TAG("filter.1.2", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( locomotn_sound )
	MDRV_IMPORT_FROM(timeplt_sound)

	/* basic machine hardware */
	MDRV_CPU_MODIFY("tpsound")
	MDRV_CPU_PROGRAM_MAP(locomotn_sound_map,0)
MACHINE_DRIVER_END
