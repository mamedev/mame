/***************************************************************************

Irem Red Alert sound hardware

The manual lists two sets of sounds.

Analogue:
- Formation Aircraft
- Dive bombers
- Helicopters
- Launcher firing
- Explosion #1
- Explosion #2
- Explosion #3

Digital:
- Melody #1.  Starting sound.
- Melody #2.  Ending sound
- Time signal
- Chirping birds
- Alarm
- Excellent
- Coin insertion
- MIRV division
- Megaton bomb - long
- Megaton bomb - short
- Megaton bomb landing

If you have any questions about how this driver works, don't hesitate to
ask.  - Mike Balfour (mab22@po.cwru.edu)
***************************************************************************/

#include "driver.h"
#include "cpu/m6502/m6502.h"
#include "cpu/i8085/i8085.h"
#include "sound/ay8910.h"

static int AY8910_A_input_data = 0;
static int c030_data = 0;
static int sound_register_IC1 = 0;
static int sound_register_IC2 = 0;

WRITE8_HANDLER( redalert_c030_w )
{
	c030_data = data & 0x3F;

	/* Is this some type of sound command? */
	if (data & 0x80)
		/* Cause an NMI on the voice CPU here? */
		cpunum_set_input_line(2,I8085_RST75_LINE,HOLD_LINE);
}

READ8_HANDLER( redalert_voicecommand_r )
{
	return c030_data;
}

WRITE8_HANDLER( redalert_soundlatch_w )
{
	/* The byte is connected to Port A of the AY8910 */
	AY8910_A_input_data = data;

	/* Bit D7 is also connected to the NMI input of the CPU */
	if ((data & 0x80)!=0x80)
		cpunum_set_input_line(1,INPUT_LINE_NMI,PULSE_LINE);
}

READ8_HANDLER( redalert_AY8910_A_r )
{
	return AY8910_A_input_data;
}

WRITE8_HANDLER( redalert_AY8910_w )
{
	/* BC2 is connected to a pull-up resistor, so BC2=1 always */
	switch (data)
	{
		case 0x00:
			/* BC1=0, BDIR=0 : INACTIVE */
			break;
		case 0x01:
			/* BC1=1, BDIR=0 : READ FROM PSG */
			sound_register_IC1 = AY8910_read_port_0_r(offset);
			break;
		case 0x02:
			/* BC1=0, BDIR=1 : WRITE TO PSG */
			AY8910_write_port_0_w(offset,sound_register_IC2);
			break;
		case 0x03:
			/* BC1=1, BDIR=1 : LATCH ADDRESS */
			AY8910_control_port_0_w(offset,sound_register_IC2);
			break;
		default:
			logerror("Invalid Sound Command: %02X\n",data);
			break;
	}
}

READ8_HANDLER( redalert_sound_register_IC1_r )
{
	return sound_register_IC1;
}

WRITE8_HANDLER( redalert_sound_register_IC2_w )
{
	sound_register_IC2 = data;
}

WRITE8_HANDLER( redalert_AY8910_B_w )
{
	/* I'm fairly certain this port triggers analog sounds */
	logerror("Port B Trigger: %02X\n",data);
	/* D0 = Formation Aircraft? */
	/* D1 = Dive bombers? */
	/* D2 = Helicopters? */
	/* D3 = Launcher firing? */
	/* D4 = Explosion #1? */
	/* D5 = Explosion #2? */
	/* D6 = Explosion #3? */
}

