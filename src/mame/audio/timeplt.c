/***************************************************************************

    This code is used by the following module:

    timeplt.c
    pooyan.c
    rallyx.c (for locomotn)
    tutankhm.c
    rocnrope.c

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/flt_rc.h"
#include "audio/timeplt.h"


#define MASTER_CLOCK         XTAL_14_31818MHz

typedef struct _timeplt_audio_state timeplt_audio_state;
struct _timeplt_audio_state
{
	UINT8    last_irq_state;
	cpu_device *soundcpu;

	running_device *filter_0_0;
	running_device *filter_0_1;
	running_device *filter_0_2;
	running_device *filter_1_0;
	running_device *filter_1_1;
	running_device *filter_1_2;
};

INLINE timeplt_audio_state *get_safe_token( running_device *device )
{
	assert(device != NULL);
	assert(device->type() == TIMEPLT_AUDIO);

	return (timeplt_audio_state *)downcast<legacy_device_base *>(device)->token();
}


/*************************************
 *
 *  Initialization
 *
 *************************************/

static DEVICE_START( timeplt_audio )
{
	running_machine *machine = device->machine;
	timeplt_audio_state *state = get_safe_token(device);

	state->soundcpu = machine->device<cpu_device>("tpsound");
	state->filter_0_0 = machine->device("filter.0.0");
	state->filter_0_1 = machine->device("filter.0.1");
	state->filter_0_2 = machine->device("filter.0.2");
	state->filter_1_0 = machine->device("filter.1.0");
	state->filter_1_1 = machine->device("filter.1.1");
	state->filter_1_2 = machine->device("filter.1.2");

	state->last_irq_state = 0;
	state_save_register_device_item(device, 0, state->last_irq_state);
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

static READ8_DEVICE_HANDLER( timeplt_portB_r )
{
	timeplt_audio_state *state = get_safe_token(device);

	static const int timeplt_timer[10] =
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x90, 0xa0, 0xb0, 0xa0, 0xd0
	};

	return timeplt_timer[(state->soundcpu->total_cycles() / 512) % 10];
}



/*************************************
 *
 *  Filter controls
 *
 *************************************/

static void filter_w( running_device *device, int data )
{
	int C = 0;

	if (data & 1)
		C += 220000;	/* 220000pF = 0.220uF */
	if (data & 2)
		C +=  47000;	/*  47000pF = 0.047uF */

	filter_rc_set_RC(device, FLT_RC_LOWPASS, 1000, 5100, 0, CAP_P(C));
}


static WRITE8_DEVICE_HANDLER( timeplt_filter_w )
{
	timeplt_audio_state *state = get_safe_token(device);

	filter_w(state->filter_1_0, (offset >>  0) & 3);
	filter_w(state->filter_1_1, (offset >>  2) & 3);
	filter_w(state->filter_1_2, (offset >>  4) & 3);
	filter_w(state->filter_0_0, (offset >>  6) & 3);
	filter_w(state->filter_0_1, (offset >>  8) & 3);
	filter_w(state->filter_0_2, (offset >> 10) & 3);
}



/*************************************
 *
 *  External interfaces
 *
 *************************************/

WRITE8_HANDLER( timeplt_sh_irqtrigger_w )
{
	running_device *audio = space->machine->device("timeplt_audio");
	timeplt_audio_state *state = get_safe_token(audio);

	if (state->last_irq_state == 0 && data)
	{
		/* setting bit 0 low then high triggers IRQ on the sound CPU */
		cpu_set_input_line_and_vector(state->soundcpu, 0, HOLD_LINE, 0xff);
	}

	state->last_irq_state = data;
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( timeplt_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x3000, 0x33ff) AM_MIRROR(0x0c00) AM_RAM
	AM_RANGE(0x4000, 0x4000) AM_MIRROR(0x0fff) AM_DEVREADWRITE("ay1", ay8910_r, ay8910_data_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0x0fff) AM_DEVWRITE("ay1", ay8910_address_w)
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x0fff) AM_DEVREADWRITE("ay2", ay8910_r, ay8910_data_w)
	AM_RANGE(0x7000, 0x7000) AM_MIRROR(0x0fff) AM_DEVWRITE("ay2", ay8910_address_w)
	AM_RANGE(0x8000, 0xffff) AM_DEVWRITE("timeplt_audio", timeplt_filter_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( locomotn_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_MIRROR(0x0c00) AM_RAM
	AM_RANGE(0x3000, 0x3fff) AM_DEVWRITE("timeplt_audio", timeplt_filter_w)
	AM_RANGE(0x4000, 0x4000) AM_MIRROR(0x0fff) AM_DEVREADWRITE("ay1", ay8910_r, ay8910_data_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0x0fff) AM_DEVWRITE("ay1", ay8910_address_w)
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x0fff) AM_DEVREADWRITE("ay2", ay8910_r, ay8910_data_w)
	AM_RANGE(0x7000, 0x7000) AM_MIRROR(0x0fff) AM_DEVWRITE("ay2", ay8910_address_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Sound chip interfaces
 *
 *************************************/

static const ay8910_interface timeplt_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_MEMORY_HANDLER("tpsound", PROGRAM, soundlatch_r),
	DEVCB_DEVICE_HANDLER("timeplt_audio", timeplt_portB_r),
	DEVCB_NULL,
	DEVCB_NULL
};



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( timeplt_sound )

	/* basic machine hardware */
	MDRV_CPU_ADD("tpsound",Z80,MASTER_CLOCK/8)
	MDRV_CPU_PROGRAM_MAP(timeplt_sound_map)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("timeplt_audio", TIMEPLT_AUDIO, 0)

	MDRV_SOUND_ADD("ay1", AY8910, MASTER_CLOCK/8)
	MDRV_SOUND_CONFIG(timeplt_ay8910_interface)
	MDRV_SOUND_ROUTE(0, "filter.0.0", 0.60)
	MDRV_SOUND_ROUTE(1, "filter.0.1", 0.60)
	MDRV_SOUND_ROUTE(2, "filter.0.2", 0.60)

	MDRV_SOUND_ADD("ay2", AY8910, MASTER_CLOCK/8)
	MDRV_SOUND_ROUTE(0, "filter.1.0", 0.60)
	MDRV_SOUND_ROUTE(1, "filter.1.1", 0.60)
	MDRV_SOUND_ROUTE(2, "filter.1.2", 0.60)

	MDRV_SOUND_ADD("filter.0.0", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_SOUND_ADD("filter.0.1", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_SOUND_ADD("filter.0.2", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("filter.1.0", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_SOUND_ADD("filter.1.1", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_SOUND_ADD("filter.1.2", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


MACHINE_CONFIG_DERIVED( locomotn_sound, timeplt_sound )

	/* basic machine hardware */
	MDRV_CPU_MODIFY("tpsound")
	MDRV_CPU_PROGRAM_MAP(locomotn_sound_map)
MACHINE_CONFIG_END

/*****************************************************************************
    DEVICE DEFINITION
*****************************************************************************/


static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)				p##timeplt_audio##s
#define DEVTEMPLATE_FEATURES			DT_HAS_START
#define DEVTEMPLATE_NAME				"Time Pilot Audio"
#define DEVTEMPLATE_FAMILY				"Time Pilot Audio IC"
#include "devtempl.h"

DEFINE_LEGACY_SOUND_DEVICE(TIMEPLT_AUDIO, timeplt_audio);

