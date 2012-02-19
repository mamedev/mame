/***************************************************************************

    Cinematronics vector hardware

    Special thanks to Neil Bradley, Zonn Moore, and Jeff Mitchell of the
    Retrocade Alliance

    Update:
    6/27/99 Jim Hernandez -- 1st Attempt at Fixing Drone Star Castle sound and
                             pitch adjustments.
    6/30/99 MLR added Rip Off, Solar Quest, Armor Attack (no samples yet)
    11/04/08 Jim Hernandez -- Fixed Drone Star Castle sound again. It was
                              broken for a long time due to some changes.

    Bugs: Sometimes the death explosion (small explosion) does not trigger.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/ccpu/ccpu.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "includes/cinemat.h"
#include "sound/samples.h"
#include "sound/ay8910.h"


/*************************************
 *
 *  Macros
 *
 *************************************/

#define RISING_EDGE(bit, changed, val)	(((changed) & (bit)) && ((val) & (bit)))
#define FALLING_EDGE(bit, changed, val)	(((changed) & (bit)) && !((val) & (bit)))

#define SOUNDVAL_RISING_EDGE(bit)		RISING_EDGE(bit, bits_changed, sound_val)
#define SOUNDVAL_FALLING_EDGE(bit)		FALLING_EDGE(bit, bits_changed, sound_val)

#define SHIFTREG_RISING_EDGE(bit)		RISING_EDGE(bit, (state->m_last_shift ^ state->m_current_shift), state->m_current_shift)
#define SHIFTREG_FALLING_EDGE(bit)		FALLING_EDGE(bit, (state->m_last_shift ^ state->m_current_shift), state->m_current_shift)

#define SHIFTREG2_RISING_EDGE(bit)		RISING_EDGE(bit, (state->m_last_shift2 ^ state->m_current_shift), state->m_current_shift)
#define SHIFTREG2_FALLING_EDGE(bit)		FALLING_EDGE(bit, (state->m_last_shift2 ^ state->m_current_shift), state->m_current_shift)


/*************************************
 *
 *  Generic sound write
 *
 *************************************/

WRITE8_HANDLER( cinemat_sound_control_w )
{
	cinemat_state *state = space->machine().driver_data<cinemat_state>();
	UINT8 oldval = state->m_sound_control;

	/* form an 8-bit value with the new bit */
	state->m_sound_control = (state->m_sound_control & ~(1 << offset)) | ((data & 1) << offset);

	/* if something changed, call the sound subroutine */
	if ((state->m_sound_control != oldval) && state->m_sound_handler)
		(*state->m_sound_handler)(space->machine(), state->m_sound_control, state->m_sound_control ^ oldval);
}



/*************************************
 *
 *  Generic sound init
 *
 *************************************/

static MACHINE_START( generic )
{
	cinemat_state *state = machine.driver_data<cinemat_state>();
    /* register for save states */
    state_save_register_global(machine, state->m_sound_control);
    state_save_register_global(machine, state->m_current_shift);
    state_save_register_global(machine, state->m_last_shift);
    state_save_register_global(machine, state->m_last_shift2);
    state_save_register_global(machine, state->m_current_pitch);
    state_save_register_global(machine, state->m_last_frame);
    state_save_register_global_array(machine, state->m_sound_fifo);
    state_save_register_global(machine, state->m_sound_fifo_in);
    state_save_register_global(machine, state->m_sound_fifo_out);
    state_save_register_global(machine, state->m_last_portb_write);
}


static void generic_init(running_machine &machine, void (*callback)(running_machine &,UINT8, UINT8))
{
	cinemat_state *state = machine.driver_data<cinemat_state>();
	/* call the standard init */
	MACHINE_RESET_CALL(cinemat);

	/* set the sound handler */
	state->m_sound_handler = callback;

	/* reset sound control */
	state->m_sound_control = 0x9f;

	/* reset shift register values */
    state->m_current_shift = 0xffff;
    state->m_last_shift = 0xffff;
    state->m_last_shift2 = 0xffff;

	/* reset frame counters */
    state->m_last_frame = 0;

	/* reset Star Castle pitch */
    state->m_current_pitch = 0x10000;
}



/*************************************
 *
 *  Space Wars
 *
 *************************************/

static const char *const spacewar_sample_names[] =
{
	"*spacewar",
	"explode1",
	"fire1",
	"idle",
	"thrust1",
	"thrust2",
	"pop",
	"explode2",
	"fire2",
    0
};

static const samples_interface spacewar_samples_interface =
{
	8,
	spacewar_sample_names
};

static void spacewar_sound_w(running_machine &machine, UINT8 sound_val, UINT8 bits_changed)
{
	samples_device *samples = machine.device<samples_device>("samples");

	/* Explosion - rising edge */
	if (SOUNDVAL_RISING_EDGE(0x01))
		samples->start(0, (machine.rand() & 1) ? 0 : 6);

	/* Fire sound - rising edge */
	if (SOUNDVAL_RISING_EDGE(0x02))
		samples->start(1, (machine.rand() & 1) ? 1 : 7);

	/* Player 1 thrust - 0=on, 1=off */
	if (SOUNDVAL_FALLING_EDGE(0x04))
		samples->start(3, 3, true);
	if (SOUNDVAL_RISING_EDGE(0x04))
		samples->stop(3);

	/* Player 2 thrust - 0=on, 1-off */
	if (SOUNDVAL_FALLING_EDGE(0x08))
		samples->start(4, 4, true);
	if (SOUNDVAL_RISING_EDGE(0x08))
		samples->stop(4);

	/* Mute - 0=off, 1=on */
	if (SOUNDVAL_FALLING_EDGE(0x10))
		samples->start(2, 2, true);	/* play idle sound */
	if (SOUNDVAL_RISING_EDGE(0x10))
	{
        int i;

		/* turn off all but the idle sound */
		for (i = 0; i < 5; i++)
			if (i != 2)
				samples->stop(i);

		/* Pop when board is shut off */
		samples->start(2, 5);
	}
}

static MACHINE_RESET( spacewar )
{
	generic_init(machine, spacewar_sound_w);
}

MACHINE_CONFIG_FRAGMENT( spacewar_sound )
	MCFG_MACHINE_START(generic)
	MCFG_MACHINE_RESET(spacewar)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SAMPLES_ADD("samples", spacewar_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  Barrier
 *
 *************************************/

static const char *const barrier_sample_names[] =
{
	"*barrier",
	"playrdie",
	"playmove",
	"enemmove",
    0
};

static const samples_interface barrier_samples_interface =
{
	3,
	barrier_sample_names
};

static void barrier_sound_w(running_machine &machine, UINT8 sound_val, UINT8 bits_changed)
{
	samples_device *samples = machine.device<samples_device>("samples");

	/* Player die - rising edge */
	if (SOUNDVAL_RISING_EDGE(0x01))
		samples->start(0, 0);

	/* Player move - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x02))
		samples->start(1, 1);

	/* Enemy move - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x04))
		samples->start(2, 2);
}

static MACHINE_RESET( barrier )
{
	generic_init(machine, barrier_sound_w);
}

MACHINE_CONFIG_FRAGMENT( barrier_sound )
	MCFG_MACHINE_START(generic)
	MCFG_MACHINE_RESET(barrier)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SAMPLES_ADD("samples", barrier_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  Speed Freak
 *
 *************************************/

static const char *const speedfrk_sample_names[] =
{
	"*speedfrk",
	"offroad",
    NULL
};

static const samples_interface speedfrk_samples_interface =
{
	1,
	speedfrk_sample_names
};

static void speedfrk_sound_w(running_machine &machine, UINT8 sound_val, UINT8 bits_changed)
{
	cinemat_state *state = machine.driver_data<cinemat_state>();
	samples_device *samples = machine.device<samples_device>("samples");

	/* on the falling edge of bit 0x08, clock the inverse of bit 0x04 into the top of the shiftreg */
	if (SOUNDVAL_FALLING_EDGE(0x08))
	{
		state->m_current_shift = ((state->m_current_shift >> 1) & 0x7fff) | ((~sound_val << 13) & 1);
		/* high 12 bits control the frequency - counts from value to $FFF, carry triggers */
		/* another counter */

		/* low 4 bits control the volume of the noise output (explosion?) */
	}

	/* off-road - 1=on, 0=off */
	if (SOUNDVAL_RISING_EDGE(0x10))
		samples->start(0, 0, true);
	if (SOUNDVAL_FALLING_EDGE(0x10))
		samples->stop(0);

    /* start LED is controlled by bit 0x02 */
    set_led_status(machine, 0, ~sound_val & 0x02);
}

static MACHINE_RESET( speedfrk )
{
	generic_init(machine, speedfrk_sound_w);
}

MACHINE_CONFIG_FRAGMENT( speedfrk_sound )
	MCFG_MACHINE_START(generic)
	MCFG_MACHINE_RESET(speedfrk)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SAMPLES_ADD("samples", speedfrk_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  Star Hawk
 *
 *************************************/

static const char *const starhawk_sample_names[] =
{
	"*starhawk",
	"explode",
	"rlaser",
	"llaser",
	"k",
	"master",
	"kexit",
    NULL
};

static const samples_interface starhawk_samples_interface =
{
	5,
	starhawk_sample_names
};

static void starhawk_sound_w(running_machine &machine, UINT8 sound_val, UINT8 bits_changed)
{
	samples_device *samples = machine.device<samples_device>("samples");

	/* explosion - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x01))
		samples->start(0, 0);

	/* right laser - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x02))
		samples->start(1, 1);

	/* left laser - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x04))
		samples->start(2, 2);

	/* K - 0=on, 1=off */
	if (SOUNDVAL_FALLING_EDGE(0x08))
		samples->start(3, 3, true);
	if (SOUNDVAL_RISING_EDGE(0x08))
		samples->stop(3);

	/* master - 0=on, 1=off */
	if (SOUNDVAL_FALLING_EDGE(0x10))
		samples->start(4, 4, true);
	if (SOUNDVAL_RISING_EDGE(0x10))
		samples->stop(4);

	/* K exit - 1=on, 0=off */
	if (SOUNDVAL_RISING_EDGE(0x80))
		samples->start(3, 5, true);
	if (SOUNDVAL_FALLING_EDGE(0x80))
		samples->stop(3);
}

static MACHINE_RESET( starhawk )
{
	generic_init(machine, starhawk_sound_w);
}

MACHINE_CONFIG_FRAGMENT( starhawk_sound )
	MCFG_MACHINE_START(generic)
	MCFG_MACHINE_RESET(starhawk)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SAMPLES_ADD("samples", starhawk_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  Sundance
 *
 *************************************/

static const char *const sundance_sample_names[] =
{
	"*sundance",
	"bong",
	"whoosh",
	"explsion",
	"ping1",
	"ping2",
	"hatch",
    0
};

static const samples_interface sundance_samples_interface =
{
	6,
	sundance_sample_names
};

static void sundance_sound_w(running_machine &machine, UINT8 sound_val, UINT8 bits_changed)
{
	samples_device *samples = machine.device<samples_device>("samples");

	/* bong - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x01))
		samples->start(0, 0);

	/* whoosh - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x02))
		samples->start(1, 1);

	/* explosion - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x04))
		samples->start(2, 2);

	/* ping - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x08))
		samples->start(3, 3);

	/* ping - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x10))
		samples->start(4, 4);

	/* hatch - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x80))
		samples->start(5, 5);
}

static MACHINE_RESET( sundance )
{
	generic_init(machine, sundance_sound_w);
}

MACHINE_CONFIG_FRAGMENT( sundance_sound )
	MCFG_MACHINE_START(generic)
	MCFG_MACHINE_RESET(sundance)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SAMPLES_ADD("samples", sundance_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  Tail Gunner
 *
 *************************************/

static const char *const tailg_sample_names[] =
{
	"*tailg",
	"sexplode",
	"thrust1",
	"slaser",
	"shield",
	"bounce",
	"hypersp",
    NULL
};

static const samples_interface tailg_samples_interface =
{
	6,
	tailg_sample_names
};

static void tailg_sound_w(running_machine &machine, UINT8 sound_val, UINT8 bits_changed)
{
	cinemat_state *state = machine.driver_data<cinemat_state>();
	/* the falling edge of bit 0x10 clocks bit 0x08 into the mux selected by bits 0x07 */
	if (SOUNDVAL_FALLING_EDGE(0x10))
	{
		samples_device *samples = machine.device<samples_device>("samples");

		/* update the shift register (actually just a simple mux) */
		state->m_current_shift = (state->m_current_shift & ~(1 << (sound_val & 7))) | (((sound_val >> 3) & 1) << (sound_val & 7));

		/* explosion - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x01))
			samples->start(0, 0);

		/* rumble - 0=on, 1=off */
		if (SHIFTREG_FALLING_EDGE(0x02))
			samples->start(1, 1, true);
		if (SHIFTREG_RISING_EDGE(0x02))
			samples->stop(1);

		/* laser - 0=on, 1=off */
		if (SHIFTREG_FALLING_EDGE(0x04))
			samples->start(2, 2, true);
		if (SHIFTREG_RISING_EDGE(0x04))
			samples->stop(2);

		/* shield - 0=on, 1=off */
		if (SHIFTREG_FALLING_EDGE(0x08))
			samples->start(3, 3, true);
		if (SHIFTREG_RISING_EDGE(0x08))
			samples->stop(3);

		/* bounce - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x10))
			samples->start(4, 4);

		/* hyperspace - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x20))
			samples->start(5, 5);

		/* LED */
		set_led_status(machine, 0, state->m_current_shift & 0x40);

		/* remember the previous value */
		state->m_last_shift = state->m_current_shift;
	}
}

static MACHINE_RESET( tailg )
{
	generic_init(machine, tailg_sound_w);
}

MACHINE_CONFIG_FRAGMENT( tailg_sound )
	MCFG_MACHINE_START(generic)
	MCFG_MACHINE_RESET(tailg)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SAMPLES_ADD("samples", tailg_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  Warrior
 *
 *************************************/

static const char *const warrior_sample_names[] =
{
	"*warrior",
	"bgmhum1",
	"bgmhum2",
	"killed",
	"fall",
	"appear",
    NULL
};

static const samples_interface warrior_samples_interface =
{
	5,
	warrior_sample_names
};

static void warrior_sound_w(running_machine &machine, UINT8 sound_val, UINT8 bits_changed)
{
	samples_device *samples = machine.device<samples_device>("samples");

	/* normal level - 0=on, 1=off */
	if (SOUNDVAL_FALLING_EDGE(0x01))
		samples->start(0, 0, true);
	if (SOUNDVAL_RISING_EDGE(0x01))
		samples->stop(0);

	/* hi level - 0=on, 1=off */
	if (SOUNDVAL_FALLING_EDGE(0x02))
		samples->start(1, 1, true);
	if (SOUNDVAL_RISING_EDGE(0x02))
		samples->stop(1);

	/* explosion - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x04))
		samples->start(2, 2);

	/* fall - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x08))
		samples->start(3, 3);

	/* appear - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x10))
		samples->start(4, 4);
}

static MACHINE_RESET( warrior )
{
	generic_init(machine, warrior_sound_w);
}

MACHINE_CONFIG_FRAGMENT( warrior_sound )
	MCFG_MACHINE_START(generic)
	MCFG_MACHINE_RESET(warrior)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SAMPLES_ADD("samples", warrior_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  Armor Attack
 *
 *************************************/

static const char *const armora_sample_names[] =
{
	"*armora",
    "loexp",
    "jeepfire",
	"hiexp",
	"tankfire",
	"tankeng",
	"beep",
	"chopper",
    NULL
};

static const samples_interface armora_samples_interface =
{
	7,
	armora_sample_names
};

static void armora_sound_w(running_machine &machine, UINT8 sound_val, UINT8 bits_changed)
{
	cinemat_state *state = machine.driver_data<cinemat_state>();
	samples_device *samples = machine.device<samples_device>("samples");

	/* on the rising edge of bit 0x10, clock bit 0x80 into the shift register */
	if (SOUNDVAL_RISING_EDGE(0x10))
		state->m_current_shift = ((state->m_current_shift >> 1) & 0x7f) | (sound_val & 0x80);

	/* execute on the rising edge of bit 0x01 */
	if (SOUNDVAL_RISING_EDGE(0x01))
	{
		/* bits 0-4 control the tank sound speed */

		/* lo explosion - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x10))
			samples->start(0, 0);

		/* jeep fire - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x20))
			samples->start(1, 1);

		/* hi explosion - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x40))
			samples->start(2, 2);

		/* tank fire - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x80))
			samples->start(3, 3);

		/* remember the previous value */
		state->m_last_shift = state->m_current_shift;
	}

	/* tank sound - 0=on, 1=off */
	/* still not totally correct - should be multiple speeds based on remaining bits in shift reg */
	if (SOUNDVAL_FALLING_EDGE(0x02))
		samples->start(4, 4, true);
	if (SOUNDVAL_RISING_EDGE(0x02))
		samples->stop(4);

	/* beep sound - 0=on, 1=off */
	if (SOUNDVAL_FALLING_EDGE(0x04))
		samples->start(5, 5, true);
	if (SOUNDVAL_RISING_EDGE(0x04))
		samples->stop(5);

	/* chopper sound - 0=on, 1=off */
	if (SOUNDVAL_FALLING_EDGE(0x08))
		samples->start(6, 6, true);
	if (SOUNDVAL_RISING_EDGE(0x08))
		samples->stop(6);
}

static MACHINE_RESET( armora )
{
	generic_init(machine, armora_sound_w);
}

MACHINE_CONFIG_FRAGMENT( armora_sound )
	MCFG_MACHINE_START(generic)
	MCFG_MACHINE_RESET(armora)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SAMPLES_ADD("samples", armora_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  Ripoff
 *
 *************************************/

static const char *const ripoff_sample_names[] =
{
	"*ripoff",
	"bonuslvl",
	"eattack",
	"shipfire",
    "efire",
	"explosn",
	"bg1",
	"bg2",
	"bg3",
	"bg4",
	"bg5",
	"bg6",
	"bg7",
	"bg8",
    NULL
};

static const samples_interface ripoff_samples_interface =
{
	6,
	ripoff_sample_names
};

static void ripoff_sound_w(running_machine &machine, UINT8 sound_val, UINT8 bits_changed)
{
	cinemat_state *state = machine.driver_data<cinemat_state>();
	samples_device *samples = machine.device<samples_device>("samples");

	/* on the rising edge of bit 0x02, clock bit 0x01 into the shift register */
	if (SOUNDVAL_RISING_EDGE(0x02))
		state->m_current_shift = ((state->m_current_shift >> 1) & 0x7f) | ((sound_val << 7) & 0x80);

	/* execute on the rising edge of bit 0x04 */
	if (SOUNDVAL_RISING_EDGE(0x04))
	{
		/* background - 0=on, 1=off, selected by bits 0x38 */
		if ((((state->m_current_shift ^ state->m_last_shift) & 0x38) && !(state->m_current_shift & 0x04)) || SHIFTREG_FALLING_EDGE(0x04))
			samples->start(5, 5 + ((state->m_current_shift >> 5) & 7), true);
		if (SHIFTREG_RISING_EDGE(0x04))
			samples->stop(5);

		/* beep - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x02))
			samples->start(0, 0);

		/* motor - 0=on, 1=off */
		if (SHIFTREG_FALLING_EDGE(0x01))
			samples->start(1, 1, true);
		if (SHIFTREG_RISING_EDGE(0x01))
			samples->stop(1);

		/* remember the previous value */
		state->m_last_shift = state->m_current_shift;
	}

	/* torpedo - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x08))
		samples->start(2, 2);

	/* laser - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x10))
		samples->start(3, 3);

	/* explosion - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x80))
		samples->start(4, 4);
}

static MACHINE_RESET( ripoff )
{
	generic_init(machine, ripoff_sound_w);
}

MACHINE_CONFIG_FRAGMENT( ripoff_sound )
	MCFG_MACHINE_START(generic)
	MCFG_MACHINE_RESET(ripoff)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SAMPLES_ADD("samples", ripoff_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  Star Castle
 *
 *************************************/

static const char *const starcas_sample_names[] =
{
	"*starcas",
	"cfire",
	"shield",
	"star",
	"thrust",
	"drone",
	"lexplode",
	"sexplode",
	"pfire",
    0
};

static const samples_interface starcas_samples_interface =
{
	8,
	starcas_sample_names
};

static void starcas_sound_w(running_machine &machine, UINT8 sound_val, UINT8 bits_changed)
{
	cinemat_state *state = machine.driver_data<cinemat_state>();
	samples_device *samples = machine.device<samples_device>("samples");
	UINT32 target_pitch;

	/* on the rising edge of bit 0x10, clock bit 0x80 into the shift register */
	if (SOUNDVAL_RISING_EDGE(0x10))
		state->m_current_shift = ((state->m_current_shift >> 1) & 0x7f) | (sound_val & 0x80);

	/* execute on the rising edge of bit 0x01 */
	if (SOUNDVAL_RISING_EDGE(0x01))
	{
		/* fireball - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x80))
			samples->start(0, 0);

		/* shield hit - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x40))
			samples->start(1, 1);

		/* star sound - 0=off, 1=on */
		if (SHIFTREG_RISING_EDGE(0x20))
			samples->start(2, 2, true);
		if (SHIFTREG_FALLING_EDGE(0x20))
			samples->stop(2);

		/* thrust sound - 1=off, 0=on*/
		if (SHIFTREG_FALLING_EDGE(0x10))
			samples->start(3, 3, true);
		if (SHIFTREG_RISING_EDGE(0x10))
			samples->stop(3);

		/* drone - 1=off, 0=on */
		if (SHIFTREG_FALLING_EDGE(0x08))
			samples->start(4, 4, true);
		if (SHIFTREG_RISING_EDGE(0x08))
			samples->stop(4);

		/* latch the drone pitch */
		target_pitch = (state->m_current_shift & 7) + ((state->m_current_shift & 2) << 2);
        target_pitch = 0x5800 + (target_pitch << 12);

        /* once per frame slide the pitch toward the target */
        if (machine.primary_screen->frame_number() > state->m_last_frame)
        {
            if (state->m_current_pitch > target_pitch)
                state->m_current_pitch -= 225;
            if (state->m_current_pitch < target_pitch)
                state->m_current_pitch += 150;
            samples->set_frequency(4, state->m_current_pitch);
            state->m_last_frame = machine.primary_screen->frame_number();
        }

		/* remember the previous value */
		state->m_last_shift = state->m_current_shift;
	}

	/* loud explosion - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x02))
		samples->start(5, 5);

	/* soft explosion - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x04))
		samples->start(6, 6);

	/* player fire - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x08))
		samples->start(7, 7);
}

static MACHINE_RESET( starcas )
{
	generic_init(machine, starcas_sound_w);
}

MACHINE_CONFIG_FRAGMENT( starcas_sound )
	MCFG_MACHINE_START(generic)
	MCFG_MACHINE_RESET(starcas)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SAMPLES_ADD("samples", starcas_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END



/*************************************
 *
 *  Solar Quest
 *
 *************************************/

static const char *const solarq_sample_names[] =
{
	"*solarq",
    "bigexpl",
	"smexpl",
	"lthrust",
	"slaser",
	"pickup",
	"nuke2",
	"nuke1",
    "music",
    NULL
};

static const samples_interface solarq_samples_interface =
{
	8,
	solarq_sample_names
};

static void solarq_sound_w(running_machine &machine, UINT8 sound_val, UINT8 bits_changed)
{
	cinemat_state *state = machine.driver_data<cinemat_state>();
	samples_device *samples = machine.device<samples_device>("samples");

	/* on the rising edge of bit 0x10, clock bit 0x80 into the shift register */
	if (SOUNDVAL_RISING_EDGE(0x10))
		state->m_current_shift = ((state->m_current_shift >> 1) & 0x7fff) | ((sound_val << 8) & 0x8000);

	/* execute on the rising edge of bit 0x02 */
	if (SOUNDVAL_RISING_EDGE(0x02))
	{
		/* only the upper 8 bits matter */
		state->m_current_shift >>= 8;

		/* loud explosion - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x80))
			samples->start(0, 0);

		/* soft explosion - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x40))
			samples->start(1, 1);

		/* thrust - 0=on, 1=off */
		if (SHIFTREG_FALLING_EDGE(0x20))
		{
			state->m_target_volume = 1.0;
			if (!samples->playing(2))
				samples->start(2, 2, true);
		}
		if (SHIFTREG_RISING_EDGE(0x20))
			state->m_target_volume = 0;

		/* ramp the thrust volume */
        if (samples->playing(2) && machine.primary_screen->frame_number() > state->m_last_frame)
        {
            if (state->m_current_volume > state->m_target_volume)
                state->m_current_volume -= 0.078f;
            if (state->m_current_volume < state->m_target_volume)
                state->m_current_volume += 0.078f;
            if (state->m_current_volume > 0)
                samples->set_volume(2, state->m_current_volume);
            else
                samples->stop(2);
            state->m_last_frame = machine.primary_screen->frame_number();
        }

		/* fire - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x10))
			samples->start(3, 3);

		/* capture - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x08))
			samples->start(4, 4);

		/* nuke - 1=on, 0=off */
		if (SHIFTREG_RISING_EDGE(0x04))
			samples->start(5, 5, true);
		if (SHIFTREG_FALLING_EDGE(0x04))
			samples->stop(5);

		/* photon - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x02))
			samples->start(6, 6);

		/* remember the previous value */
		state->m_last_shift = state->m_current_shift;
	}

	/* clock music data on the rising edge of bit 0x01 */
	if (SOUNDVAL_RISING_EDGE(0x01))
	{
		int freq, vol;

		/* start/stop the music sample on the high bit */
		if (SHIFTREG2_RISING_EDGE(0x8000))
			samples->start(7, 7, true);
		if (SHIFTREG2_FALLING_EDGE(0x8000))
			samples->stop(7);

		/* set the frequency */
		freq = 56818.181818 / (4096 - (state->m_current_shift & 0xfff));
		samples->set_frequency(7, 44100 * freq / 1050);

		/* set the volume */
		vol = (~state->m_current_shift >> 12) & 7;
		samples->set_volume(7, vol / 7.0);

		/* remember the previous value */
		state->m_last_shift2 = state->m_current_shift;
    }
}

static MACHINE_RESET( solarq )
{
	generic_init(machine, solarq_sound_w);
}

MACHINE_CONFIG_FRAGMENT( solarq_sound )
	MCFG_MACHINE_START(generic)
	MCFG_MACHINE_RESET(solarq)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SAMPLES_ADD("samples", solarq_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END



/*************************************
 *
 *  Boxing Bugs
 *
 *************************************/

static const char *const boxingb_sample_names[] =
{
	"*boxingb",
    "softexpl",
	"loudexpl",
	"chirp",
	"eggcrack",
	"bugpusha",
	"bugpushb",
	"bugdie",
    "beetle",
    "music",
    "cannon",
    "bounce",
    "bell",
    NULL
};

static const samples_interface boxingb_samples_interface =
{
	12,
	boxingb_sample_names
};

static void boxingb_sound_w(running_machine &machine, UINT8 sound_val, UINT8 bits_changed)
{
	cinemat_state *state = machine.driver_data<cinemat_state>();
	samples_device *samples = machine.device<samples_device>("samples");

	/* on the rising edge of bit 0x10, clock bit 0x80 into the shift register */
	if (SOUNDVAL_RISING_EDGE(0x10))
		state->m_current_shift = ((state->m_current_shift >> 1) & 0x7fff) | ((sound_val << 8) & 0x8000);

	/* execute on the rising edge of bit 0x02 */
	if (SOUNDVAL_RISING_EDGE(0x02))
	{
		/* only the upper 8 bits matter */
		state->m_current_shift >>= 8;

		/* soft explosion - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x80))
			samples->start(0, 0);

		/* loud explosion - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x40))
			samples->start(1, 1);

		/* chirping birds - 0=on, 1=off */
		if (SHIFTREG_FALLING_EDGE(0x20))
			samples->start(2, 2);
		if (SHIFTREG_RISING_EDGE(0x20))
			samples->stop(2);

		/* egg cracking - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x10))
			samples->start(3, 3);

		/* bug pushing A - rising edge */
		if (SHIFTREG_RISING_EDGE(0x08))
			samples->start(4, 4);

		/* bug pushing B - rising edge */
		if (SHIFTREG_RISING_EDGE(0x04))
			samples->start(5, 5);

		/* bug dying - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x02))
			samples->start(6, 6);

		/* beetle on screen - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x01))
			samples->start(7, 7);

		/* remember the previous value */
		state->m_last_shift = state->m_current_shift;
	}

	/* clock music data on the rising edge of bit 0x01 */
	if (SOUNDVAL_RISING_EDGE(0x01))
	{
		int freq, vol;

		/* start/stop the music sample on the high bit */
		if (SHIFTREG2_RISING_EDGE(0x8000))
			samples->start(8, 8, true);
		if (SHIFTREG2_FALLING_EDGE(0x8000))
			samples->stop(8);

		/* set the frequency */
		freq = 56818.181818 / (4096 - (state->m_current_shift & 0xfff));
		samples->set_frequency(8, 44100 * freq / 1050);

		/* set the volume */
		vol = (~state->m_current_shift >> 12) & 3;
		samples->set_volume(8, vol / 3.0);

        /* cannon - falling edge */
        if (SHIFTREG2_RISING_EDGE(0x4000))
        	samples->start(9, 9);

		/* remember the previous value */
		state->m_last_shift2 = state->m_current_shift;
    }

	/* bounce - rising edge */
	if (SOUNDVAL_RISING_EDGE(0x04))
		samples->start(10, 10);

	/* bell - falling edge */
	if (SOUNDVAL_RISING_EDGE(0x08))
		samples->start(11, 11);
}

static MACHINE_RESET( boxingb )
{
	generic_init(machine, boxingb_sound_w);
}

MACHINE_CONFIG_FRAGMENT( boxingb_sound )
	MCFG_MACHINE_START(generic)
	MCFG_MACHINE_RESET(boxingb)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SAMPLES_ADD("samples", boxingb_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  War of the Worlds (B&W)
 *
 *************************************/

static const char *const wotw_sample_names[] =
{
	"*wotw",
	"cfire",
	"shield",
	"star",
	"thrust",
	"drone",
	"lexplode",
	"sexplode",
	"pfire",
    0
};

static const samples_interface wotw_samples_interface =
{
	8,
	wotw_sample_names
};

static void wotw_sound_w(running_machine &machine, UINT8 sound_val, UINT8 bits_changed)
{
	cinemat_state *state = machine.driver_data<cinemat_state>();
	samples_device *samples = machine.device<samples_device>("samples");
	UINT32 target_pitch;

	/* on the rising edge of bit 0x10, clock bit 0x80 into the shift register */
	if (SOUNDVAL_RISING_EDGE(0x10))
		state->m_current_shift = ((state->m_current_shift >> 1) & 0x7f) | (sound_val & 0x80);

	/* execute on the rising edge of bit 0x01 */
	if (SOUNDVAL_RISING_EDGE(0x01))
	{
		/* fireball - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x80))
			samples->start(0, 0);

		/* shield hit - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x40))
			samples->start(1, 1);

		/* star sound - 0=off, 1=on */
		if (SHIFTREG_RISING_EDGE(0x20))
			samples->start(2, 2, true);
		if (SHIFTREG_FALLING_EDGE(0x20))
			samples->stop(2);

		/* thrust sound - 1=off, 0=on*/
		if (SHIFTREG_FALLING_EDGE(0x10))
			samples->start(3, 3, true);
		if (SHIFTREG_RISING_EDGE(0x10))
			samples->stop(3);

		/* drone - 1=off, 0=on */
		if (SHIFTREG_FALLING_EDGE(0x08))
			samples->start(4, 4, true);
		if (SHIFTREG_RISING_EDGE(0x08))
			samples->stop(4);

		/* latch the drone pitch */
		target_pitch = (state->m_current_shift & 7) + ((state->m_current_shift & 2) << 2);
        target_pitch = 0x10000 + (target_pitch << 12);

        /* once per frame slide the pitch toward the target */
        if (machine.primary_screen->frame_number() > state->m_last_frame)
        {
            if (state->m_current_pitch > target_pitch)
                state->m_current_pitch -= 300;
            if (state->m_current_pitch < target_pitch)
                state->m_current_pitch += 200;
            samples->set_frequency(4, state->m_current_pitch);
            state->m_last_frame = machine.primary_screen->frame_number();
        }

		/* remember the previous value */
		state->m_last_shift = state->m_current_shift;
	}

	/* loud explosion - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x02))
		samples->start(5, 5);

	/* soft explosion - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x04))
		samples->start(6, 6);

	/* player fire - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x08))
		samples->start(7, 7);
}

static MACHINE_RESET( wotw )
{
	generic_init(machine, wotw_sound_w);
}

MACHINE_CONFIG_FRAGMENT( wotw_sound )
	MCFG_MACHINE_START(generic)
	MCFG_MACHINE_RESET(wotw)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SAMPLES_ADD("samples", wotw_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  War of the Worlds (color)
 *
 *************************************/

static const char *const wotwc_sample_names[] =
{
	"*wotwc",
	"cfire",
	"shield",
	"star",
	"thrust",
	"drone",
	"lexplode",
	"sexplode",
	"pfire",
    0
};

static const samples_interface wotwc_samples_interface =
{
	8,
	wotwc_sample_names
};

static void wotwc_sound_w(running_machine &machine, UINT8 sound_val, UINT8 bits_changed)
{
	cinemat_state *state = machine.driver_data<cinemat_state>();
	samples_device *samples = machine.device<samples_device>("samples");
	UINT32 target_pitch;

	/* on the rising edge of bit 0x10, clock bit 0x80 into the shift register */
	if (SOUNDVAL_RISING_EDGE(0x10))
		state->m_current_shift = ((state->m_current_shift >> 1) & 0x7f) | (sound_val & 0x80);

	/* execute on the rising edge of bit 0x01 */
	if (SOUNDVAL_RISING_EDGE(0x01))
	{
		/* fireball - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x80))
			samples->start(0, 0);

		/* shield hit - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x40))
			samples->start(1, 1);

		/* star sound - 0=off, 1=on */
		if (SHIFTREG_RISING_EDGE(0x20))
			samples->start(2, 2, true);
		if (SHIFTREG_FALLING_EDGE(0x20))
			samples->stop(2);

		/* thrust sound - 1=off, 0=on*/
		if (SHIFTREG_FALLING_EDGE(0x10))
			samples->start(3, 3, true);
		if (SHIFTREG_RISING_EDGE(0x10))
			samples->stop(3);

		/* drone - 1=off, 0=on */
		if (SHIFTREG_FALLING_EDGE(0x08))
			samples->start(4, 4, true);
		if (SHIFTREG_RISING_EDGE(0x08))
			samples->stop(4);

		/* latch the drone pitch */
		target_pitch = (state->m_current_shift & 7) + ((state->m_current_shift & 2) << 2);
        target_pitch = 0x10000 + (target_pitch << 12);

        /* once per frame slide the pitch toward the target */
        if (machine.primary_screen->frame_number() > state->m_last_frame)
        {
            if (state->m_current_pitch > target_pitch)
                state->m_current_pitch -= 300;
            if (state->m_current_pitch < target_pitch)
                state->m_current_pitch += 200;
            samples->set_frequency(4, state->m_current_pitch);
            state->m_last_frame = machine.primary_screen->frame_number();
        }

		/* remember the previous value */
		state->m_last_shift = state->m_current_shift;
	}

	/* loud explosion - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x02))
		samples->start(5, 5);

	/* soft explosion - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x04))
		samples->start(6, 6);

	/* player fire - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x08))
		samples->start(7, 7);
}

static MACHINE_RESET( wotwc )
{
	generic_init(machine, wotwc_sound_w);
}

MACHINE_CONFIG_FRAGMENT( wotwc_sound )
	MCFG_MACHINE_START(generic)
	MCFG_MACHINE_RESET(wotwc)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SAMPLES_ADD("samples", wotwc_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  Demon
 *
 *************************************/

static TIMER_CALLBACK( synced_sound_w )
{
	cinemat_state *state = machine.driver_data<cinemat_state>();
	state->m_sound_fifo[state->m_sound_fifo_in] = param;
	state->m_sound_fifo_in = (state->m_sound_fifo_in + 1) % 16;
}


static void demon_sound_w(running_machine &machine, UINT8 sound_val, UINT8 bits_changed)
{
	/* all inputs are inverted */
	sound_val = ~sound_val;

	/* watch for a 0->1 edge on bit 4 ("shift in") to clock in the new data */
	if ((bits_changed & 0x10) && (sound_val & 0x10))
		machine.scheduler().synchronize(FUNC(synced_sound_w), sound_val & 0x0f);
}


static READ8_DEVICE_HANDLER( sound_porta_r )
{
	cinemat_state *state = device->machine().driver_data<cinemat_state>();
	/* bits 0-3 are the sound data; bit 4 is the data ready */
	return state->m_sound_fifo[state->m_sound_fifo_out] | ((state->m_sound_fifo_in != state->m_sound_fifo_out) << 4);
}


static READ8_DEVICE_HANDLER( sound_portb_r )
{
	cinemat_state *state = device->machine().driver_data<cinemat_state>();
	return state->m_last_portb_write;
}


static WRITE8_DEVICE_HANDLER( sound_portb_w )
{
	cinemat_state *state = device->machine().driver_data<cinemat_state>();
	/* watch for a 0->1 edge on bit 0 ("shift out") to advance the data pointer */
	if ((data & 1) != (state->m_last_portb_write & 1) && (data & 1) != 0)
		state->m_sound_fifo_out = (state->m_sound_fifo_out + 1) % 16;

	/* watch for a 0->1 edge of bit 1 ("hard reset") to reset the FIFO */
	if ((data & 2) != (state->m_last_portb_write & 2) && (data & 2) != 0)
		state->m_sound_fifo_in = state->m_sound_fifo_out = 0;

	/* bit 2 controls the global mute */
	if ((data & 4) != (state->m_last_portb_write & 4))
		device->machine().sound().system_mute(data & 4);

	/* remember the last value written */
	state->m_last_portb_write = data;
}


static WRITE8_DEVICE_HANDLER( sound_output_w )
{
	logerror("sound_output = %02X\n", data);
}


static const ay8910_interface demon_ay8910_interface_1 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_HANDLER(sound_porta_r),
	DEVCB_HANDLER(sound_portb_r),
	DEVCB_NULL,
	DEVCB_HANDLER(sound_portb_w)
};

static const ay8910_interface demon_ay8910_interface_3 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(sound_output_w)
};


static Z80CTC_INTERFACE( demon_z80ctc_interface )
{
	0,               /* timer disables */
	DEVCB_CPU_INPUT_LINE("audiocpu", INPUT_LINE_IRQ0),   /* interrupt handler */
	DEVCB_NULL,     /* ZC/TO0 callback */
	DEVCB_NULL,		/* ZC/TO1 callback */
	DEVCB_NULL		/* ZC/TO2 callback */
};


static MACHINE_RESET( demon_sound )
{
	cinemat_state *state = machine.driver_data<cinemat_state>();
	/* generic init */
	generic_init(machine, demon_sound_w);

	/* reset the FIFO */
	state->m_sound_fifo_in = state->m_sound_fifo_out = 0;
	state->m_last_portb_write = 0xff;

	/* turn off channel A on AY8910 #0 because it is used as a low-pass filter */
	ay8910_set_volume(machine.device("ay1"), 0, 0);
}


static ADDRESS_MAP_START( demon_sound_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x3000, 0x33ff) AM_RAM
	AM_RANGE(0x4000, 0x4001) AM_DEVREAD("ay1", ay8910_r)
	AM_RANGE(0x4002, 0x4003) AM_DEVWRITE("ay1", ay8910_data_address_w)
	AM_RANGE(0x5000, 0x5001) AM_DEVREAD("ay2", ay8910_r)
	AM_RANGE(0x5002, 0x5003) AM_DEVWRITE("ay2", ay8910_data_address_w)
	AM_RANGE(0x6000, 0x6001) AM_DEVREAD("ay3", ay8910_r)
	AM_RANGE(0x6002, 0x6003) AM_DEVWRITE("ay3", ay8910_data_address_w)
	AM_RANGE(0x7000, 0x7000) AM_WRITENOP  /* watchdog? */
ADDRESS_MAP_END


static ADDRESS_MAP_START( demon_sound_ports, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVWRITE("ctc", z80ctc_w)
	AM_RANGE(0x1c, 0x1f) AM_DEVWRITE("ctc", z80ctc_w)
ADDRESS_MAP_END


static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ NULL }
};


MACHINE_CONFIG_FRAGMENT( demon_sound )

	/* basic machine hardware */
	MCFG_CPU_ADD("audiocpu", Z80, 3579545)
	MCFG_CPU_CONFIG(daisy_chain)
	MCFG_CPU_PROGRAM_MAP(demon_sound_map)
	MCFG_CPU_IO_MAP(demon_sound_ports)

	MCFG_Z80CTC_ADD("ctc", 3579545 /* same as "audiocpu" */, demon_z80ctc_interface)

	MCFG_MACHINE_START(generic)
	MCFG_MACHINE_RESET(demon_sound)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 3579545)
	MCFG_SOUND_CONFIG(demon_ay8910_interface_1)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, 3579545)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay3", AY8910, 3579545)
	MCFG_SOUND_CONFIG(demon_ay8910_interface_3)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/*************************************
 *
 *  QB3
 *
 *************************************/

static WRITE8_HANDLER( qb3_sound_w )
{
	UINT16 rega = cpu_get_reg(space->machine().device("maincpu"), CCPU_A);
	demon_sound_w(space->machine(), 0x00 | (~rega & 0x0f), 0x10);
}


static MACHINE_RESET( qb3_sound )
{
	MACHINE_RESET_CALL(demon_sound);
	machine.device("maincpu")->memory().space(AS_IO)->install_legacy_write_handler(0x04, 0x04, FUNC(qb3_sound_w));

	/* this patch prevents the sound ROM from eating itself when command $0A is sent */
	/* on a cube rotate */
	machine.region("audiocpu")->base()[0x11dc] = 0x09;
}


MACHINE_CONFIG_DERIVED( qb3_sound, demon_sound )
	MCFG_MACHINE_RESET(qb3_sound)
MACHINE_CONFIG_END
