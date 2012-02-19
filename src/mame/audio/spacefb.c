/***************************************************************************

    Space Firebird hardware

****************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/dac.h"
#include "sound/samples.h"
#include "includes/spacefb.h"


READ8_HANDLER( spacefb_audio_p2_r )
{
	spacefb_state *state = space->machine().driver_data<spacefb_state>();
	return (state->m_sound_latch & 0x18) << 1;
}


READ8_HANDLER( spacefb_audio_t0_r )
{
	spacefb_state *state = space->machine().driver_data<spacefb_state>();
	return state->m_sound_latch & 0x20;
}


READ8_HANDLER( spacefb_audio_t1_r )
{
	spacefb_state *state = space->machine().driver_data<spacefb_state>();
	return state->m_sound_latch & 0x04;
}


WRITE8_HANDLER( spacefb_port_1_w )
{
	spacefb_state *state = space->machine().driver_data<spacefb_state>();
	samples_device *samples = space->machine().device<samples_device>("samples");

	cputag_set_input_line(space->machine(), "audiocpu", 0, (data & 0x02) ? CLEAR_LINE : ASSERT_LINE);

	/* enemy killed */
	if (!(data & 0x01) && (state->m_sound_latch & 0x01))  samples->start(0,0);

	/* ship fire */
	if (!(data & 0x40) && (state->m_sound_latch & 0x40))  samples->start(1,1);

	/*
     *  Explosion Noise
     *
     *  Actual sample has a bit of attack at the start, but these doesn't seem to be an easy way
     *  to play the attack part, then loop the middle bit until the sample is turned off
     *  Fortunately it seems like the recorded sample of the spaceship death is the longest the sample plays for.
     *  We loop it just in case it runs out
     */
	if ((data & 0x80) != (state->m_sound_latch & 0x80))
	{
		if (data & 0x80)
			/* play decaying noise */
			samples->start(2,3);
		else
			/* start looping noise */
			samples->start(2,2, true);
	}

	state->m_sound_latch = data;
}


static const char *const spacefb_sample_names[] =
{
	"*spacefb",
	"ekilled",
	"shipfire",
	"explode1",
	"explode2",
	0
};


static const samples_interface spacefb_samples_interface =
{
	3,
	spacefb_sample_names
};


MACHINE_CONFIG_FRAGMENT( spacefb_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SAMPLES_ADD("samples", spacefb_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END
