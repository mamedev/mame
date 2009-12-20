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
#include "sound/tms5110.h"
#include "sound/ay8910.h"
#include "includes/galaxold.h"

#define AD2083_TMS5110_CLOCK 		XTAL_640kHz

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

static const int scramble_timer[10] =
{
	0x00, 0x10, 0x20, 0x30, 0x40, 0x90, 0xa0, 0xb0, 0xa0, 0xd0
};

READ8_DEVICE_HANDLER( scramble_portB_r )
{
	return scramble_timer[(cputag_get_total_cycles(device->machine, "audiocpu")/512) % 10];
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

static const int frogger_timer[10] =
{
	0x00, 0x10, 0x08, 0x18, 0x40, 0x90, 0x88, 0x98, 0x88, 0xd0
};

READ8_DEVICE_HANDLER( frogger_portB_r )
{
	return frogger_timer[(cputag_get_total_cycles(device->machine, "audiocpu")/512) % 10];
}

WRITE8_DEVICE_HANDLER( scramble_sh_irqtrigger_w )
{
	const device_config *target = devtag_get_device(device->machine, "konami_7474");

	/* the complement of bit 3 is connected to the flip-flop's clock */
	ttl7474_clock_w(target, ~data & 0x08);
	ttl7474_update(target);

	/* bit 4 is sound disable */
	sound_global_enable(device->machine, ~data & 0x10);
}

WRITE8_DEVICE_HANDLER( mrkougar_sh_irqtrigger_w )
{
	const device_config *target = devtag_get_device(device->machine, "konami_7474");

	/* the complement of bit 3 is connected to the flip-flop's clock */
	ttl7474_clock_w(target, ~data & 0x08);
	ttl7474_update(target);
}

static IRQ_CALLBACK(scramble_sh_irq_callback)
{
	const device_config *target = devtag_get_device(device->machine, "konami_7474");

	/* interrupt acknowledge clears the flip-flop --
       we need to pulse the CLR line because MAME's core never clears this
       line, only asserts it */
	ttl7474_clear_w(target, 0);
	ttl7474_update(target);

	ttl7474_clear_w(target, 1);
	ttl7474_update(target);

	return 0xff;
}

void scramble_sh_7474_callback(const device_config *device)
{
	/* the Q bar is connected to the Z80's INT line.  But since INT is complemented, */
	/* we need to complement Q bar */
	cputag_set_input_line(device->machine, "audiocpu", 0, !ttl7474_output_comp_r(device) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_HANDLER( hotshock_sh_irqtrigger_w )
{
	cputag_set_input_line(space->machine, "audiocpu", 0, ASSERT_LINE);
}

READ8_DEVICE_HANDLER( hotshock_soundlatch_r )
{
	cputag_set_input_line(device->machine, "audiocpu", 0, CLEAR_LINE);
	return soundlatch_r(cputag_get_address_space(device->machine, "audiocpu", ADDRESS_SPACE_PROGRAM),0);
}

static void filter_w(const device_config *device, int data)
{
	int C;


	C = 0;
	if (data & 1)
		C += 220000;	/* 220000pF = 0.220uF */
	if (data & 2)
		C +=  47000;	/*  47000pF = 0.047uF */
	if (device != NULL)
		filter_rc_set_RC(device,FLT_RC_LOWPASS,1000,5100,0,CAP_P(C));
}

WRITE8_HANDLER( scramble_filter_w )
{
	filter_w(devtag_get_device(space->machine, "filter.1.0"), (offset >>  0) & 3);
	filter_w(devtag_get_device(space->machine, "filter.1.1"), (offset >>  2) & 3);
	filter_w(devtag_get_device(space->machine, "filter.1.2"), (offset >>  4) & 3);
	filter_w(devtag_get_device(space->machine, "filter.0.0"), (offset >>  6) & 3);
	filter_w(devtag_get_device(space->machine, "filter.0.1"), (offset >>  8) & 3);
	filter_w(devtag_get_device(space->machine, "filter.0.2"), (offset >> 10) & 3);
}

WRITE8_HANDLER( frogger_filter_w )
{
	filter_w(devtag_get_device(space->machine, "filter.0.0"), (offset >>  6) & 3);
	filter_w(devtag_get_device(space->machine, "filter.0.1"), (offset >>  8) & 3);
	filter_w(devtag_get_device(space->machine, "filter.0.2"), (offset >> 10) & 3);
}

void scramble_sh_init(running_machine *machine)
{
	cpu_set_irq_callback(cputag_get_cpu(machine, "audiocpu"), scramble_sh_irq_callback);

	/* PR is always 0, D is always 1 */
	ttl7474_d_w(devtag_get_device(machine, "konami_7474"), 1);
}


/***************************************************************************
    AD2083 TMS5110 implementation (still wrong!)

    ROMA: BIT1: Hypersoni ?
    ROMA: BIT2: Two thousand eighty three
    ROMA: BIT4: Atomic City Attack
    ROMA: BIT5: Thank you, please try again
    ROMA: BIT6: Two, one, fire
    ROMA: BIT7: Hyperjump
    ROMB: BIT1: Noise (Explosion ?)
    ROMB: BIT2: Welcome to the top 20
    ROMB: BIT4: Atomic City Attack
    ROMB: BIT5: Noise (Explosion ?)
    ROMB: BIT6: Keep going pressing
    ROMB: BIT7: You are the new champion

    The circuit consist of

    2x 2532 32Kbit roms
    2x 74LS393 quad 4bit counters to address roms
    1x 74LS174 hex-d-flipflops to latch control byte
    2x 74LS139 decoder to decode rom selects
    1x 74LS161 8bit input multiplexor to select sound bit for rom data read
    1x 74LS00  quad nand to decode signals (unknown) to latch control byte
               ==> 74LS139
    1x 74LS16  quad open collector inverters
    1x 74S288  32x8 Prom

    The prom obviously is used to provide the right timing when
    fetching data bits. This circuit should be comparable to bagman.

    Prom
    Q0      ==> NC
    Q1      ==> PDC
    Q2      ==> CTL2/CTL8 (Speak/Reset)
    Q6      ==> Reset Counters (LS393)
    Q7      ==> Trigger Logic

  ***************************************************************************/

/*
 *      Alt1 Alt2
 * 1 ==> C     B   Bit select
 * 2 ==> B     C   Bit select
 * 3 ==> A     A   Bit select
 * 4 ==> B1        Rom select
 * 5 ==> A1        Rom select
 *
 *      ALT1      ALT2
 * 321  CBA       CBA
 * 000  000       000
 * 001  100       010
 * 010  010       100
 * 011  110       110
 * 100  001       001
 * 101  101       011
 * 110  011       101
 * 111  111       111
 *
 * Alt1 provides more sensible sound. Both Alt1 and Alt2 are
 * possible on PCB so Alt2 is left in for documentation.
 *
 */

static UINT32 speech_rom_address;
static UINT32 speech_rom_address_hi;
static UINT8 speech_rom_bit;
static UINT8 speech_cnt;

static TIMER_CALLBACK( ad2083_step )
{
	const device_config *tms = devtag_get_device(machine, "tms");

	/* only 16 bytes needed ... The original dump is bad. This
     * is what is needed to get speech to work. The prom data has
     * been updated and marked as BAD_DUMP. The information below
     * is given for reference once another dump should surface.
     *
     * static const int prom[16] = {0x00, 0x00, 0x02, 0x00, 0x00, 0x02, 0x00, 0x00,
     *              0x02, 0x00, 0x40, 0x00, 0x04, 0x06, 0x04, 0x84 };
     */
	UINT8 *prom = memory_region(machine, "5110ctrl");
	UINT8 ctrl;

	if (param == 0)
	{
		if (speech_cnt < 0x10)
		{
			/* Just reset and exit */
			speech_cnt = 0;
			return;
		}
		speech_cnt = 0;
	}
	ctrl = prom[speech_cnt++];

	if (ctrl & 0x40)
		speech_rom_address = 0;

	tms5110_ctl_w(tms, 0, ctrl & 0x04 ? TMS5110_CMD_SPEAK : TMS5110_CMD_RESET);
	tms5110_pdc_w(tms, 0, ctrl & 0x02 ? 0 : 1);

	if (!(ctrl & 0x80))
		timer_set(machine, ATTOTIME_IN_HZ(AD2083_TMS5110_CLOCK / 2),NULL,1,ad2083_step);
}

static int ad2083_speech_rom_read_bit(const device_config *device)
{
	UINT8 *ROM = memory_region(device->machine, "tms5110");
	int bit;

	speech_rom_address %= 4096;

	bit = (ROM[speech_rom_address | speech_rom_address_hi] >> speech_rom_bit) & 1;
	speech_rom_address++;

	return bit;
}

static WRITE8_HANDLER( ad2083_tms5110_ctrl_w )
{
	static const int tbl[8] = {0,4,2,6,1,5,3,7};

	speech_rom_bit = tbl[data & 0x07];
	switch (data>>3)
	{
		case 0x00:
			logerror("Rom 0 select .. \n");
			break;
		case 0x01:
			/* Rom 2 select */
			speech_rom_address_hi = 0x1000;
			break;
		case 0x02:
			logerror("Rom 1 select\n");
			break;
		case 0x03:
			/* Rom 3 select */
			speech_rom_address_hi = 0x0000;
			break;
	}
	timer_set(space->machine, attotime_zero,NULL,0,ad2083_step);
}

static const tms5110_interface ad2083_tms5110_interface =
{
	ad2083_speech_rom_read_bit	/* M0 callback function. Called whenever chip requests a single bit of data */
};


static const ay8910_interface ad2083_ay8910_interface_1 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_HANDLER(scramble_portB_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static const ay8910_interface ad2083_ay8910_interface_2 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_HANDLER(hotshock_soundlatch_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static ADDRESS_MAP_START( ad2083_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ad2083_sound_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_WRITE(ad2083_tms5110_ctrl_w)
	AM_RANGE(0x10, 0x10) AM_DEVWRITE("ay1", ay8910_address_w)
	AM_RANGE(0x20, 0x20) AM_DEVREADWRITE("ay1", ay8910_r, ay8910_data_w)
	AM_RANGE(0x40, 0x40) AM_DEVREADWRITE("ay2", ay8910_r, ay8910_data_w)
	AM_RANGE(0x80, 0x80) AM_DEVWRITE("ay2", ay8910_address_w)
ADDRESS_MAP_END

static SOUND_START( ad2083 )
{
	speech_rom_address = 0;
	speech_rom_address_hi = 0;
	speech_rom_bit = 0;
	speech_cnt = 0x10;

	state_save_register_global(machine, speech_rom_address);
	state_save_register_global(machine, speech_rom_address_hi);
	state_save_register_global(machine, speech_rom_bit);
	state_save_register_global(machine, speech_cnt);
}

MACHINE_DRIVER_START( ad2083_audio )

	MDRV_CPU_ADD("audiocpu", Z80, 14318000/8)	/* 1.78975 MHz */
	MDRV_CPU_PROGRAM_MAP(ad2083_sound_map)
	MDRV_CPU_IO_MAP(ad2083_sound_io_map)

	MDRV_SOUND_START(ad2083)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("ay1", AY8910, 14318000/8)
	MDRV_SOUND_CONFIG(ad2083_ay8910_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)

	MDRV_SOUND_ADD("ay2", AY8910, 14318000/8)
	MDRV_SOUND_CONFIG(ad2083_ay8910_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)

	MDRV_SOUND_ADD("tms", TMS5110A, AD2083_TMS5110_CLOCK)
	MDRV_SOUND_CONFIG(ad2083_tms5110_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END
