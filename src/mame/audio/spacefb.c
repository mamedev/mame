/***************************************************************************

    Space Firebird hardware

****************************************************************************/

#include "driver.h"
#include "cpu/i8039/i8039.h"
#include "sound/dac.h"
#include "sound/samples.h"
#include "includes/spacefb.h"



static UINT8 spacefb_sound_latch;



READ8_HANDLER( spacefb_audio_p2_r )
{
	return (spacefb_sound_latch & 0x18) << 1;
}


READ8_HANDLER( spacefb_audio_t0_r )
{
	return spacefb_sound_latch & 0x20;
}


READ8_HANDLER( spacefb_audio_t1_r )
{
	return spacefb_sound_latch & 0x04;
}


WRITE8_HANDLER( spacefb_port_1_w )
{
	cpunum_set_input_line(1, 0, (data & 0x02) ? CLEAR_LINE : ASSERT_LINE);

	/* enemy killed */
	if (!(data & 0x01) && (spacefb_sound_latch & 0x01))  sample_start(0,0,0);

	/* ship fire */
	if (!(data & 0x40) && (spacefb_sound_latch & 0x40))  sample_start(1,1,0);

	/*
     *  Explosion Noise
     *
     *  Actual sample has a bit of attack at the start, but these doesn't seem to be an easy way
     *  to play the attack part, then loop the middle bit until the sample is turned off
     *  Fortunately it seems like the recorded sample of the spaceship death is the longest the sample plays for.
     *  We loop it just in case it runs out
     */
	if ((data & 0x80) != (spacefb_sound_latch & 0x80))
	{
		if (data & 0x80)
			/* play decaying noise */
			sample_start(2,3,0);
		else
			/* start looping noise */
			sample_start(2,2,1);
	}

	spacefb_sound_latch = data;
}


static const char *spacefb_sample_names[] =
{
	"*spacefb",
	"ekilled.wav",
	"shipfire.wav",
	"explode1.wav",
	"explode2.wav",
	0
};


static struct Samplesinterface spacefb_samples_interface =
{
	3,
	spacefb_sample_names
};


MACHINE_DRIVER_START( spacefb_audio )
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD(SAMPLES, 0)
	MDRV_SOUND_CONFIG(spacefb_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END
