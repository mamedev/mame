/***************************************************************************

    Irem Red Alert hardware

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

****************************************************************************/

#include "driver.h"
#include "rescap.h"
#include "cpu/i8085/i8085.h"
#include "machine/6821pia.h"
#include "sound/ay8910.h"
#include "sound/hc55516.h"
#include "redalert.h"



#define REDALERT_AUDIO_PCB_CLOCK	(XTAL_12_5MHz)
#define REDALERT_AUDIO_CPU_CLOCK	(REDALERT_AUDIO_PCB_CLOCK / 12)
#define REDALERT_AY8910_CLOCK		(REDALERT_AUDIO_PCB_CLOCK / 6)
#define REDALERT_AUDIO_CPU_IRQ_FREQ	(1000000000 / PERIOD_OF_555_ASTABLE_NSEC(RES_K(120), RES_K(2.7), CAP_U(0.01)))

#define REDALERT_VOICE_PCB_CLOCK	(XTAL_6MHz)
#define REDALERT_VOICE_CPU_CLOCK	(REDALERT_VOICE_PCB_CLOCK)
#define REDALERT_HC55516_CLOCK		(REDALERT_VOICE_PCB_CLOCK / 256)

#define DEMONEYE_AUDIO_PCB_CLOCK	(XTAL_3_579545MHz)
#define DEMONEYE_AUDIO_CPU_CLOCK	(DEMONEYE_AUDIO_PCB_CLOCK / 4)  /* what's the real divisor? */
#define DEMONEYE_AY8910_CLOCK		(DEMONEYE_AUDIO_PCB_CLOCK / 2)  /* what's the real divisor? */



/*************************************
 *
 *  Statics
 *
 *************************************/

static UINT8 ay8910_latch_1;
static UINT8 ay8910_latch_2;

static UINT8 ay8910_latch_1;



/*************************************
 *
 *  Read Alert analog sounds
 *
 *************************************/

static WRITE8_HANDLER( redalert_analog_w )
{
	/* this port triggers analog sounds
       D0 = Formation Aircraft?
       D1 = Dive bombers?
       D2 = Helicopters?
       D3 = Launcher firing?
       D4 = Explosion #1?
       D5 = Explosion #2?
       D6 = Explosion #3? */

	logerror("Analog: %02X\n",data);
}



/*************************************
 *
 *  Red Alert audio board
 *
 *************************************/

WRITE8_HANDLER( redalert_audio_command_w )
{
	/* the byte is connected to port A of the AY8910 */
	soundlatch_w(machine, 0, data);

	/* D7 is also connected to the NMI input of the CPU -
       the NMI is actually toggled by a 74121 */
	if ((data & 0x80) == 0x00)
		cpunum_set_input_line(machine, 1, INPUT_LINE_NMI, PULSE_LINE);
}


static WRITE8_HANDLER( redalert_AY8910_w )
{
	/* BC2 is connected to a pull-up resistor, so BC2=1 always */
	switch (data & 0x03)
	{
		/* BC1=0, BDIR=0 : inactive */
		case 0x00:
			break;

		/* BC1=1, BDIR=0 : read from PSG */
		case 0x01:
			ay8910_latch_1 = AY8910_read_port_0_r(machine, 0);
			break;

		/* BC1=0, BDIR=1 : write to PSG */
		case 0x02:
			AY8910_write_port_0_w(machine, 0, ay8910_latch_2);
			break;

		/* BC1=1, BDIR=1 : latch address */
		default:
		case 0x03:
			AY8910_control_port_0_w(machine, 0, ay8910_latch_2);
			break;
	}
}


static READ8_HANDLER( redalert_ay8910_latch_1_r )
{
	return ay8910_latch_1;
}


static WRITE8_HANDLER( redalert_ay8910_latch_2_w )
{
	ay8910_latch_2 = data;
}


static const struct AY8910interface redalert_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	soundlatch_r, NULL,		/* port A/B read */
	NULL, redalert_analog_w	/* port A/B write */
};


static ADDRESS_MAP_START( redalert_audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x0c00) AM_RAM
	AM_RANGE(0x1000, 0x1000) AM_MIRROR(0x0ffe) AM_READWRITE(SMH_NOP, redalert_AY8910_w)
	AM_RANGE(0x1001, 0x1001) AM_MIRROR(0x0ffe) AM_READWRITE(redalert_ay8910_latch_1_r, redalert_ay8910_latch_2_w)
	AM_RANGE(0x2000, 0x6fff) AM_NOP
	AM_RANGE(0x7000, 0x77ff) AM_MIRROR(0x0800) AM_ROM
ADDRESS_MAP_END


static SOUND_START( redalert_audio )
{
	state_save_register_global(ay8910_latch_1);
	state_save_register_global(ay8910_latch_2);
}



/*************************************
 *
 * Red Alert voice board
 *
 *************************************/

WRITE8_HANDLER( redalert_voice_command_w )
{
	soundlatch2_w(machine, 0, (data & 0x78) >> 3);
	cpunum_set_input_line(machine, 2, I8085_RST75_LINE, (~data & 0x80) ? ASSERT_LINE : CLEAR_LINE);
}


static void sod_callback(int data)
{
	hc55516_digit_w(0, data);
}


static int sid_callback(void)
{
	return hc55516_clock_state_r(0);
}


static SOUND_START( redalert_voice )
{
	cpunum_set_info_fct(2, CPUINFO_PTR_I8085_SOD_CALLBACK, (void *)sod_callback);
	cpunum_set_info_fct(2, CPUINFO_PTR_I8085_SID_CALLBACK, (void *)sid_callback);
}


static ADDRESS_MAP_START( redalert_voice_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_NOP
	AM_RANGE(0x8000, 0x83ff) AM_MIRROR(0x3c00) AM_RAM
	AM_RANGE(0xc000, 0xc000) AM_MIRROR(0x3fff) AM_READWRITE(soundlatch2_r, SMH_NOP)
ADDRESS_MAP_END



/*************************************
 *
 *  Red Alert audio start
 *
 *************************************/

static SOUND_START( redalert )
{
	SOUND_START_CALL(redalert_audio);
	SOUND_START_CALL(redalert_voice);
}



/*************************************
 *
 *  Red Alert machine driver
 *
 *************************************/

MACHINE_DRIVER_START( redalert_audio )

	MDRV_CPU_ADD("audio", M6502, REDALERT_AUDIO_CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(redalert_audio_map,0)
	MDRV_CPU_PERIODIC_INT(irq0_line_hold, REDALERT_AUDIO_CPU_IRQ_FREQ)

	MDRV_CPU_ADD("voice", 8085A, REDALERT_VOICE_CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(redalert_voice_map,0)

	MDRV_SOUND_START( redalert )

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay", AY8910, REDALERT_AY8910_CLOCK)
	MDRV_SOUND_CONFIG(redalert_ay8910_interface)
	MDRV_SOUND_ROUTE(0, "mono", 0.50)
	MDRV_SOUND_ROUTE(1, "mono", 0.50)
	/* channel C is used a noise source and is not connected to a speaker */

	MDRV_SOUND_ADD("cvsd", HC55516, REDALERT_HC55516_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END



/*************************************
 *
 *  Demoneye-X audio board
 *
 *************************************/


WRITE8_HANDLER( demoneye_audio_command_w )
{
	/* the byte is connected to port A of the AY8910 */
	soundlatch_w(machine, 0, data);
	cpunum_set_input_line(machine, 1, INPUT_LINE_NMI, PULSE_LINE);
}


static WRITE8_HANDLER( demoneye_ay8910_latch_1_w )
{
	ay8910_latch_1 = data;
}


static READ8_HANDLER( demoneye_ay8910_latch_2_r )
{
	return ay8910_latch_2;
}


static WRITE8_HANDLER( demoneye_ay8910_data_w )
{
	switch (ay8910_latch_1 & 0x03)
	{
		case 0x00:
			if (ay8910_latch_1 & 0x10)
				AY8910_write_port_0_w(machine, 0, data);

			if (ay8910_latch_1 & 0x20)
				AY8910_write_port_1_w(machine, 0, data);

			break;

		case 0x01:
			if (ay8910_latch_1 & 0x10)
				ay8910_latch_2 = AY8910_read_port_0_r(machine, 0);

			if (ay8910_latch_1 & 0x20)
				ay8910_latch_2 = AY8910_read_port_1_r(machine, 0);

			break;

		case 0x03:
			if (ay8910_latch_1 & 0x10)
				AY8910_control_port_0_w(machine, 0, data);

			if (ay8910_latch_1 & 0x20)
				AY8910_control_port_1_w(machine, 0, data);

			break;

		default:
			logerror("demoneye_ay8910_data_w called with latch %02X  data %02X\n", ay8910_latch_1, data);
			break;
	}
}


static ADDRESS_MAP_START( demoneye_audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x0500, 0x0503) AM_READWRITE(pia_0_r, pia_0_w)
	AM_RANGE(0x2000, 0x3fff) AM_ROM
ADDRESS_MAP_END


static const struct AY8910interface demoneye_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	soundlatch_r, NULL,	/* port A/B read */
	NULL, NULL				/* port A/B write */
};


static const pia6821_interface demoneye_pia_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ demoneye_ay8910_latch_2_r, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ demoneye_ay8910_data_w, demoneye_ay8910_latch_1_w, 0, 0,
	/*irqs   : A/B             */ 0, 0
};



/*************************************
 *
 *  Demoneye-X audio start
 *
 *************************************/

static SOUND_START( demoneye )
{
	pia_config(0, &demoneye_pia_intf);

	state_save_register_global(ay8910_latch_1);
	state_save_register_global(ay8910_latch_2);
}



/*************************************
 *
 *  Demoneye-X audio reset
 *
 *************************************/

static SOUND_RESET( demoneye )
{
	pia_reset();
}



/*************************************
 *
 *  Demoneye-X machine driver
 *
 *************************************/

MACHINE_DRIVER_START( demoneye_audio )

	MDRV_CPU_ADD("audio", M6802, DEMONEYE_AUDIO_CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(demoneye_audio_map,0)
	MDRV_CPU_PERIODIC_INT(irq0_line_hold, REDALERT_AUDIO_CPU_IRQ_FREQ)  /* guess */

	MDRV_SOUND_START( demoneye )
	MDRV_SOUND_RESET( demoneye )

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay1", AY8910, DEMONEYE_AY8910_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("ay2", AY8910, DEMONEYE_AY8910_CLOCK)
	MDRV_SOUND_CONFIG(demoneye_ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END
