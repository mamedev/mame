/***************************************************************************

    Irem Red Alert hardware

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

****************************************************************************/

#include "driver.h"
#include "rescap.h"
#include "cpu/i8085/i8085.h"
#include "sound/ay8910.h"
#include "sound/hc55516.h"



#define AUDIO_PCB_CLOCK		(XTAL_12_5MHz)
#define AUDIO_CPU_CLOCK		(AUDIO_PCB_CLOCK / 12)
#define AY8910_CLOCK		(AUDIO_PCB_CLOCK / 6)
#define AUDIO_CPU_IRQ_FREQ	(1.0 / attotime_to_double(PERIOD_OF_555_ASTABLE(RES_K(120), RES_K(2.7), CAP_U(0.01))))

#define VOICE_PCB_CLOCK		(XTAL_6MHz)
#define VOICE_CPU_CLOCK		(VOICE_PCB_CLOCK)
#define HC55516_CLOCK		(VOICE_PCB_CLOCK / 256)



/*************************************
 *
 *  Statics
 *
 *************************************/

static UINT8 audio_register_IC1;
static UINT8 audio_register_IC2;



/*************************************
 *
 *  Analog sounds
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
 *  Audio board
 *
 *************************************/

WRITE8_HANDLER( redalert_audio_command_w )
{
	/* the byte is connected to port A of the AY8910 */
	soundlatch_w(0, data);

	/* D7 is also connected to the NMI input of the CPU -
       the NMI is actually toggled by a 74121 */
	if ((data & 0x80) == 0x00)
		cpunum_set_input_line(1, INPUT_LINE_NMI, PULSE_LINE);
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
			audio_register_IC1 = AY8910_read_port_0_r(offset);
			break;

		/* BC1=0, BDIR=1 : write to PSG */
		case 0x02:
			AY8910_write_port_0_w(offset, audio_register_IC2);
			break;

		/* BC1=1, BDIR=1 : latch address */
		default:
		case 0x03:
			AY8910_control_port_0_w(offset, audio_register_IC2);
			break;
	}
}


static READ8_HANDLER( audio_register_IC1_r )
{
	return audio_register_IC1;
}


static WRITE8_HANDLER( audio_register_IC2_w )
{
	audio_register_IC2 = data;
}


static const struct AY8910interface ay8910_interface =
{
	soundlatch_r, 0,		/* port A/B read */
	0, redalert_analog_w	/* port A/B write */
};


static ADDRESS_MAP_START( redalert_audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(15) )
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x0c00) AM_RAM
	AM_RANGE(0x1000, 0x1000) AM_MIRROR(0x0ffe) AM_READWRITE(MRA8_NOP, redalert_AY8910_w)
	AM_RANGE(0x1001, 0x1001) AM_MIRROR(0x0ffe) AM_READWRITE(audio_register_IC1_r, audio_register_IC2_w)
	AM_RANGE(0x2000, 0x6fff) AM_NOP
	AM_RANGE(0x7000, 0x77ff) AM_MIRROR(0x0800) AM_ROM
ADDRESS_MAP_END


static SOUND_START( redalert_audio )
{
	state_save_register_global(audio_register_IC1);
	state_save_register_global(audio_register_IC2);
}



/*************************************
 *
 *  Voice board
 *
 *************************************/

WRITE8_HANDLER( redalert_voice_command_w )
{
	soundlatch2_w(0, (data & 0x78) >> 3);

	cpunum_set_input_line(2, I8085_RST75_LINE, (~data & 0x80) ? ASSERT_LINE : CLEAR_LINE);
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
	AM_RANGE(0xc000, 0xc000) AM_MIRROR(0x3fff) AM_READWRITE(soundlatch2_r, MWA8_NOP)
ADDRESS_MAP_END



/*************************************
 *
 *  Audio start
 *
 *************************************/

static SOUND_START( redalert )
{
	SOUND_START_CALL(redalert_audio);
	SOUND_START_CALL(redalert_voice);
}



/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_DRIVER_START( redalert_audio )

	MDRV_CPU_ADD(M6502, AUDIO_CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(redalert_audio_map,0)
	MDRV_CPU_PERIODIC_INT(irq0_line_hold, AUDIO_CPU_IRQ_FREQ)

	MDRV_CPU_ADD(8085A, VOICE_CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(redalert_voice_map,0)

	MDRV_SOUND_START( redalert )

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, AY8910_CLOCK)
	MDRV_SOUND_CONFIG(ay8910_interface)
	MDRV_SOUND_ROUTE(0, "mono", 0.50)
	MDRV_SOUND_ROUTE(1, "mono", 0.50)
	/* channel C is used a noise source and is not connected to a speaker */

	MDRV_SOUND_ADD(HC55516, HC55516_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END
