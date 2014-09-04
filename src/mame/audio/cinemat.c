// license:BSD-3-Clause
// copyright-holders:Aaron Giles
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


/*************************************
 *
 *  Macros
 *
 *************************************/

#define RISING_EDGE(bit, changed, val)  (((changed) & (bit)) && ((val) & (bit)))
#define FALLING_EDGE(bit, changed, val) (((changed) & (bit)) && !((val) & (bit)))

#define SOUNDVAL_RISING_EDGE(bit)       RISING_EDGE(bit, bits_changed, sound_val)
#define SOUNDVAL_FALLING_EDGE(bit)      FALLING_EDGE(bit, bits_changed, sound_val)

#define SHIFTREG_RISING_EDGE(bit)       RISING_EDGE(bit, (m_last_shift ^ m_current_shift), m_current_shift)
#define SHIFTREG_FALLING_EDGE(bit)      FALLING_EDGE(bit, (m_last_shift ^ m_current_shift), m_current_shift)

#define SHIFTREG2_RISING_EDGE(bit)      RISING_EDGE(bit, (m_last_shift2 ^ m_current_shift), m_current_shift)
#define SHIFTREG2_FALLING_EDGE(bit)     FALLING_EDGE(bit, (m_last_shift2 ^ m_current_shift), m_current_shift)


/*************************************
 *
 *  Generic sound write
 *
 *************************************/

WRITE8_MEMBER(cinemat_state::cinemat_sound_control_w)
{
	UINT8 oldval = m_sound_control;

	/* form an 8-bit value with the new bit */
	m_sound_control = (m_sound_control & ~(1 << offset)) | ((data & 1) << offset);

	/* if something changed, call the sound subroutine */
	if ((m_sound_control != oldval) && m_sound_handler)
		(this->*m_sound_handler)(m_sound_control, m_sound_control ^ oldval);
}



/*************************************
 *
 *  Generic sound init
 *
 *************************************/

void cinemat_state::sound_start()
{
	/* register for save states */
	save_item(NAME(m_sound_control));
	save_item(NAME(m_current_shift));
	save_item(NAME(m_last_shift));
	save_item(NAME(m_last_shift2));
	save_item(NAME(m_current_pitch));
	save_item(NAME(m_last_frame));
	save_item(NAME(m_sound_fifo));
	save_item(NAME(m_sound_fifo_in));
	save_item(NAME(m_sound_fifo_out));
	save_item(NAME(m_last_portb_write));
}


void cinemat_state::generic_init(sound_func sound_handler)
{
	/* set the sound handler */
	m_sound_handler = sound_handler;

	/* reset sound control */
	m_sound_control = 0x9f;

	/* reset shift register values */
	m_current_shift = 0xffff;
	m_last_shift = 0xffff;
	m_last_shift2 = 0xffff;

	/* reset frame counters */
	m_last_frame = 0;

	/* reset Star Castle pitch */
	m_current_pitch = 0x10000;
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

void cinemat_state::spacewar_sound_w(UINT8 sound_val, UINT8 bits_changed)
{
	/* Explosion - rising edge */
	if (SOUNDVAL_RISING_EDGE(0x01))
		m_samples->start(0, (machine().rand() & 1) ? 0 : 6);

	/* Fire sound - rising edge */
	if (SOUNDVAL_RISING_EDGE(0x02))
		m_samples->start(1, (machine().rand() & 1) ? 1 : 7);

	/* Player 1 thrust - 0=on, 1=off */
	if (SOUNDVAL_FALLING_EDGE(0x04))
		m_samples->start(3, 3, true);
	if (SOUNDVAL_RISING_EDGE(0x04))
		m_samples->stop(3);

	/* Player 2 thrust - 0=on, 1-off */
	if (SOUNDVAL_FALLING_EDGE(0x08))
		m_samples->start(4, 4, true);
	if (SOUNDVAL_RISING_EDGE(0x08))
		m_samples->stop(4);

	/* Mute - 0=off, 1=on */
	if (SOUNDVAL_FALLING_EDGE(0x10))
		m_samples->start(2, 2, true); /* play idle sound */
	if (SOUNDVAL_RISING_EDGE(0x10))
	{
		int i;

		/* turn off all but the idle sound */
		for (i = 0; i < 5; i++)
			if (i != 2)
				m_samples->stop(i);

		/* Pop when board is shut off */
		m_samples->start(2, 5);
	}
}

SOUND_RESET_MEMBER( cinemat_state, spacewar )
{
	generic_init(&cinemat_state::spacewar_sound_w);
}

MACHINE_CONFIG_FRAGMENT( spacewar_sound )
	MCFG_SOUND_RESET_OVERRIDE(cinemat_state, spacewar)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(8)
	MCFG_SAMPLES_NAMES(spacewar_sample_names)
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

void cinemat_state::barrier_sound_w(UINT8 sound_val, UINT8 bits_changed)
{
	/* Player die - rising edge */
	if (SOUNDVAL_RISING_EDGE(0x01))
		m_samples->start(0, 0);

	/* Player move - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x02))
		m_samples->start(1, 1);

	/* Enemy move - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x04))
		m_samples->start(2, 2);
}

SOUND_RESET_MEMBER( cinemat_state, barrier )
{
	generic_init(&cinemat_state::barrier_sound_w);
}

MACHINE_CONFIG_FRAGMENT( barrier_sound )
	MCFG_SOUND_RESET_OVERRIDE(cinemat_state, barrier)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(3)
	MCFG_SAMPLES_NAMES(barrier_sample_names)
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

void cinemat_state::speedfrk_sound_w(UINT8 sound_val, UINT8 bits_changed)
{
	/* on the falling edge of bit 0x08, clock the inverse of bit 0x04 into the top of the shiftreg */
	if (SOUNDVAL_FALLING_EDGE(0x08))
	{
		m_current_shift = ((m_current_shift >> 1) & 0x7fff) | ((~sound_val << 13) & 1);
		/* high 12 bits control the frequency - counts from value to $FFF, carry triggers */
		/* another counter */

		/* low 4 bits control the volume of the noise output (explosion?) */
	}

	/* off-road - 1=on, 0=off */
	if (SOUNDVAL_RISING_EDGE(0x10))
		m_samples->start(0, 0, true);
	if (SOUNDVAL_FALLING_EDGE(0x10))
		m_samples->stop(0);

	/* start LED is controlled by bit 0x02 */
	set_led_status(machine(), 0, ~sound_val & 0x02);
}

SOUND_RESET_MEMBER( cinemat_state, speedfrk )
{
	generic_init(&cinemat_state::speedfrk_sound_w);
}

MACHINE_CONFIG_FRAGMENT( speedfrk_sound )
	MCFG_SOUND_RESET_OVERRIDE(cinemat_state, speedfrk)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(1)
	MCFG_SAMPLES_NAMES(speedfrk_sample_names)
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

void cinemat_state::starhawk_sound_w(UINT8 sound_val, UINT8 bits_changed)
{
	/* explosion - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x01))
		m_samples->start(0, 0);

	/* right laser - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x02))
		m_samples->start(1, 1);

	/* left laser - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x04))
		m_samples->start(2, 2);

	/* K - 0=on, 1=off */
	if (SOUNDVAL_FALLING_EDGE(0x08))
		m_samples->start(3, 3, true);
	if (SOUNDVAL_RISING_EDGE(0x08))
		m_samples->stop(3);

	/* master - 0=on, 1=off */
	if (SOUNDVAL_FALLING_EDGE(0x10))
		m_samples->start(4, 4, true);
	if (SOUNDVAL_RISING_EDGE(0x10))
		m_samples->stop(4);

	/* K exit - 1=on, 0=off */
	if (SOUNDVAL_RISING_EDGE(0x80))
		m_samples->start(3, 5, true);
	if (SOUNDVAL_FALLING_EDGE(0x80))
		m_samples->stop(3);
}

SOUND_RESET_MEMBER( cinemat_state, starhawk )
{
	generic_init(&cinemat_state::starhawk_sound_w);
}

MACHINE_CONFIG_FRAGMENT( starhawk_sound )
	MCFG_SOUND_RESET_OVERRIDE(cinemat_state, starhawk)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(5)
	MCFG_SAMPLES_NAMES(starhawk_sample_names)
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

void cinemat_state::sundance_sound_w(UINT8 sound_val, UINT8 bits_changed)
{
	/* bong - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x01))
		m_samples->start(0, 0);

	/* whoosh - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x02))
		m_samples->start(1, 1);

	/* explosion - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x04))
		m_samples->start(2, 2);

	/* ping - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x08))
		m_samples->start(3, 3);

	/* ping - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x10))
		m_samples->start(4, 4);

	/* hatch - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x80))
		m_samples->start(5, 5);
}

SOUND_RESET_MEMBER( cinemat_state, sundance )
{
	generic_init(&cinemat_state::sundance_sound_w);
}

MACHINE_CONFIG_FRAGMENT( sundance_sound )
	MCFG_SOUND_RESET_OVERRIDE(cinemat_state, sundance)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(6)
	MCFG_SAMPLES_NAMES(sundance_sample_names)
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

void cinemat_state::tailg_sound_w(UINT8 sound_val, UINT8 bits_changed)
{
	/* the falling edge of bit 0x10 clocks bit 0x08 into the mux selected by bits 0x07 */
	if (SOUNDVAL_FALLING_EDGE(0x10))
	{
		/* update the shift register (actually just a simple mux) */
		m_current_shift = (m_current_shift & ~(1 << (sound_val & 7))) | (((sound_val >> 3) & 1) << (sound_val & 7));

		/* explosion - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x01))
			m_samples->start(0, 0);

		/* rumble - 0=on, 1=off */
		if (SHIFTREG_FALLING_EDGE(0x02))
			m_samples->start(1, 1, true);
		if (SHIFTREG_RISING_EDGE(0x02))
			m_samples->stop(1);

		/* laser - 0=on, 1=off */
		if (SHIFTREG_FALLING_EDGE(0x04))
			m_samples->start(2, 2, true);
		if (SHIFTREG_RISING_EDGE(0x04))
			m_samples->stop(2);

		/* shield - 0=on, 1=off */
		if (SHIFTREG_FALLING_EDGE(0x08))
			m_samples->start(3, 3, true);
		if (SHIFTREG_RISING_EDGE(0x08))
			m_samples->stop(3);

		/* bounce - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x10))
			m_samples->start(4, 4);

		/* hyperspace - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x20))
			m_samples->start(5, 5);

		/* LED */
		set_led_status(machine(), 0, m_current_shift & 0x40);

		/* remember the previous value */
		m_last_shift = m_current_shift;
	}
}

SOUND_RESET_MEMBER( cinemat_state, tailg )
{
	generic_init(&cinemat_state::tailg_sound_w);
}

MACHINE_CONFIG_FRAGMENT( tailg_sound )
	MCFG_SOUND_RESET_OVERRIDE(cinemat_state, tailg)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(6)
	MCFG_SAMPLES_NAMES(tailg_sample_names)
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

void cinemat_state::warrior_sound_w(UINT8 sound_val, UINT8 bits_changed)
{
	/* normal level - 0=on, 1=off */
	if (SOUNDVAL_FALLING_EDGE(0x01))
		m_samples->start(0, 0, true);
	if (SOUNDVAL_RISING_EDGE(0x01))
		m_samples->stop(0);

	/* hi level - 0=on, 1=off */
	if (SOUNDVAL_FALLING_EDGE(0x02))
		m_samples->start(1, 1, true);
	if (SOUNDVAL_RISING_EDGE(0x02))
		m_samples->stop(1);

	/* explosion - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x04))
		m_samples->start(2, 2);

	/* fall - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x08))
		m_samples->start(3, 3);

	/* appear - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x10))
		m_samples->start(4, 4);
}

SOUND_RESET_MEMBER( cinemat_state, warrior )
{
	generic_init(&cinemat_state::warrior_sound_w);
}

MACHINE_CONFIG_FRAGMENT( warrior_sound )
	MCFG_SOUND_RESET_OVERRIDE(cinemat_state, warrior)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(5)
	MCFG_SAMPLES_NAMES(warrior_sample_names)
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

void cinemat_state::armora_sound_w(UINT8 sound_val, UINT8 bits_changed)
{
	/* on the rising edge of bit 0x10, clock bit 0x80 into the shift register */
	if (SOUNDVAL_RISING_EDGE(0x10))
		m_current_shift = ((m_current_shift >> 1) & 0x7f) | (sound_val & 0x80);

	/* execute on the rising edge of bit 0x01 */
	if (SOUNDVAL_RISING_EDGE(0x01))
	{
		/* bits 0-4 control the tank sound speed */

		/* lo explosion - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x10))
			m_samples->start(0, 0);

		/* jeep fire - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x20))
			m_samples->start(1, 1);

		/* hi explosion - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x40))
			m_samples->start(2, 2);

		/* tank fire - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x80))
			m_samples->start(3, 3);

		/* remember the previous value */
		m_last_shift = m_current_shift;
	}

	/* tank sound - 0=on, 1=off */
	/* still not totally correct - should be multiple speeds based on remaining bits in shift reg */
	if (SOUNDVAL_FALLING_EDGE(0x02))
		m_samples->start(4, 4, true);
	if (SOUNDVAL_RISING_EDGE(0x02))
		m_samples->stop(4);

	/* beep sound - 0=on, 1=off */
	if (SOUNDVAL_FALLING_EDGE(0x04))
		m_samples->start(5, 5, true);
	if (SOUNDVAL_RISING_EDGE(0x04))
		m_samples->stop(5);

	/* chopper sound - 0=on, 1=off */
	if (SOUNDVAL_FALLING_EDGE(0x08))
		m_samples->start(6, 6, true);
	if (SOUNDVAL_RISING_EDGE(0x08))
		m_samples->stop(6);
}

SOUND_RESET_MEMBER( cinemat_state, armora )
{
	generic_init(&cinemat_state::armora_sound_w);
}

MACHINE_CONFIG_FRAGMENT( armora_sound )
	MCFG_SOUND_RESET_OVERRIDE(cinemat_state, armora)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(7)
	MCFG_SAMPLES_NAMES(armora_sample_names)
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

void cinemat_state::ripoff_sound_w(UINT8 sound_val, UINT8 bits_changed)
{
	/* on the rising edge of bit 0x02, clock bit 0x01 into the shift register */
	if (SOUNDVAL_RISING_EDGE(0x02))
		m_current_shift = ((m_current_shift >> 1) & 0x7f) | ((sound_val << 7) & 0x80);

	/* execute on the rising edge of bit 0x04 */
	if (SOUNDVAL_RISING_EDGE(0x04))
	{
		/* background - 0=on, 1=off, selected by bits 0x38 */
		if ((((m_current_shift ^ m_last_shift) & 0x38) && !(m_current_shift & 0x04)) || SHIFTREG_FALLING_EDGE(0x04))
			m_samples->start(5, 5 + ((m_current_shift >> 5) & 7), true);
		if (SHIFTREG_RISING_EDGE(0x04))
			m_samples->stop(5);

		/* beep - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x02))
			m_samples->start(0, 0);

		/* motor - 0=on, 1=off */
		if (SHIFTREG_FALLING_EDGE(0x01))
			m_samples->start(1, 1, true);
		if (SHIFTREG_RISING_EDGE(0x01))
			m_samples->stop(1);

		/* remember the previous value */
		m_last_shift = m_current_shift;
	}

	/* torpedo - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x08))
		m_samples->start(2, 2);

	/* laser - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x10))
		m_samples->start(3, 3);

	/* explosion - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x80))
		m_samples->start(4, 4);
}

SOUND_RESET_MEMBER( cinemat_state, ripoff )
{
	generic_init(&cinemat_state::ripoff_sound_w);
}

MACHINE_CONFIG_FRAGMENT( ripoff_sound )
	MCFG_SOUND_RESET_OVERRIDE(cinemat_state, ripoff)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(6)
	MCFG_SAMPLES_NAMES(ripoff_sample_names)
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

void cinemat_state::starcas_sound_w(UINT8 sound_val, UINT8 bits_changed)
{
	UINT32 target_pitch;

	/* on the rising edge of bit 0x10, clock bit 0x80 into the shift register */
	if (SOUNDVAL_RISING_EDGE(0x10))
		m_current_shift = ((m_current_shift >> 1) & 0x7f) | (sound_val & 0x80);

	/* execute on the rising edge of bit 0x01 */
	if (SOUNDVAL_RISING_EDGE(0x01))
	{
		/* fireball - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x80))
			m_samples->start(0, 0);

		/* shield hit - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x40))
			m_samples->start(1, 1);

		/* star sound - 0=off, 1=on */
		if (SHIFTREG_RISING_EDGE(0x20))
			m_samples->start(2, 2, true);
		if (SHIFTREG_FALLING_EDGE(0x20))
			m_samples->stop(2);

		/* thrust sound - 1=off, 0=on*/
		if (SHIFTREG_FALLING_EDGE(0x10))
			m_samples->start(3, 3, true);
		if (SHIFTREG_RISING_EDGE(0x10))
			m_samples->stop(3);

		/* drone - 1=off, 0=on */
		if (SHIFTREG_FALLING_EDGE(0x08))
			m_samples->start(4, 4, true);
		if (SHIFTREG_RISING_EDGE(0x08))
			m_samples->stop(4);

		/* latch the drone pitch */
		target_pitch = (m_current_shift & 7) + ((m_current_shift & 2) << 2);
		target_pitch = 0x5800 + (target_pitch << 12);

		/* once per frame slide the pitch toward the target */
		if (m_screen->frame_number() > m_last_frame)
		{
			if (m_current_pitch > target_pitch)
				m_current_pitch -= 225;
			if (m_current_pitch < target_pitch)
				m_current_pitch += 150;
			m_samples->set_frequency(4, m_current_pitch);
			m_last_frame = m_screen->frame_number();
		}

		/* remember the previous value */
		m_last_shift = m_current_shift;
	}

	/* loud explosion - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x02))
		m_samples->start(5, 5);

	/* soft explosion - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x04))
		m_samples->start(6, 6);

	/* player fire - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x08))
		m_samples->start(7, 7);
}

SOUND_RESET_MEMBER( cinemat_state, starcas )
{
	generic_init(&cinemat_state::starcas_sound_w);
}

MACHINE_CONFIG_FRAGMENT( starcas_sound )
	MCFG_SOUND_RESET_OVERRIDE(cinemat_state, starcas)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(8)
	MCFG_SAMPLES_NAMES(starcas_sample_names)
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

void cinemat_state::solarq_sound_w(UINT8 sound_val, UINT8 bits_changed)
{
	/* on the rising edge of bit 0x10, clock bit 0x80 into the shift register */
	if (SOUNDVAL_RISING_EDGE(0x10))
		m_current_shift = ((m_current_shift >> 1) & 0x7fff) | ((sound_val << 8) & 0x8000);

	/* execute on the rising edge of bit 0x02 */
	if (SOUNDVAL_RISING_EDGE(0x02))
	{
		/* only the upper 8 bits matter */
		m_current_shift >>= 8;

		/* loud explosion - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x80))
			m_samples->start(0, 0);

		/* soft explosion - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x40))
			m_samples->start(1, 1);

		/* thrust - 0=on, 1=off */
		if (SHIFTREG_FALLING_EDGE(0x20))
		{
			m_target_volume = 1.0;
			if (!m_samples->playing(2))
				m_samples->start(2, 2, true);
		}
		if (SHIFTREG_RISING_EDGE(0x20))
			m_target_volume = 0;

		/* ramp the thrust volume */
		if (m_samples->playing(2) && m_screen->frame_number() > m_last_frame)
		{
			if (m_current_volume > m_target_volume)
				m_current_volume -= 0.078f;
			if (m_current_volume < m_target_volume)
				m_current_volume += 0.078f;
			if (m_current_volume > 0)
				m_samples->set_volume(2, m_current_volume);
			else
				m_samples->stop(2);
			m_last_frame = m_screen->frame_number();
		}

		/* fire - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x10))
			m_samples->start(3, 3);

		/* capture - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x08))
			m_samples->start(4, 4);

		/* nuke - 1=on, 0=off */
		if (SHIFTREG_RISING_EDGE(0x04))
			m_samples->start(5, 5, true);
		if (SHIFTREG_FALLING_EDGE(0x04))
			m_samples->stop(5);

		/* photon - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x02))
			m_samples->start(6, 6);

		/* remember the previous value */
		m_last_shift = m_current_shift;
	}

	/* clock music data on the rising edge of bit 0x01 */
	if (SOUNDVAL_RISING_EDGE(0x01))
	{
		int freq, vol;

		/* start/stop the music sample on the high bit */
		if (SHIFTREG2_RISING_EDGE(0x8000))
			m_samples->start(7, 7, true);
		if (SHIFTREG2_FALLING_EDGE(0x8000))
			m_samples->stop(7);

		/* set the frequency */
		freq = 56818.181818 / (4096 - (m_current_shift & 0xfff));
		m_samples->set_frequency(7, 44100 * freq / 1050);

		/* set the volume */
		vol = (~m_current_shift >> 12) & 7;
		m_samples->set_volume(7, vol / 7.0);

		/* remember the previous value */
		m_last_shift2 = m_current_shift;
	}
}

SOUND_RESET_MEMBER( cinemat_state, solarq )
{
	generic_init(&cinemat_state::solarq_sound_w);
}

MACHINE_CONFIG_FRAGMENT( solarq_sound )
	MCFG_SOUND_RESET_OVERRIDE(cinemat_state, solarq)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(8)
	MCFG_SAMPLES_NAMES(solarq_sample_names)
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

void cinemat_state::boxingb_sound_w(UINT8 sound_val, UINT8 bits_changed)
{
	/* on the rising edge of bit 0x10, clock bit 0x80 into the shift register */
	if (SOUNDVAL_RISING_EDGE(0x10))
		m_current_shift = ((m_current_shift >> 1) & 0x7fff) | ((sound_val << 8) & 0x8000);

	/* execute on the rising edge of bit 0x02 */
	if (SOUNDVAL_RISING_EDGE(0x02))
	{
		/* only the upper 8 bits matter */
		m_current_shift >>= 8;

		/* soft explosion - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x80))
			m_samples->start(0, 0);

		/* loud explosion - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x40))
			m_samples->start(1, 1);

		/* chirping birds - 0=on, 1=off */
		if (SHIFTREG_FALLING_EDGE(0x20))
			m_samples->start(2, 2);
		if (SHIFTREG_RISING_EDGE(0x20))
			m_samples->stop(2);

		/* egg cracking - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x10))
			m_samples->start(3, 3);

		/* bug pushing A - rising edge */
		if (SHIFTREG_RISING_EDGE(0x08))
			m_samples->start(4, 4);

		/* bug pushing B - rising edge */
		if (SHIFTREG_RISING_EDGE(0x04))
			m_samples->start(5, 5);

		/* bug dying - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x02))
			m_samples->start(6, 6);

		/* beetle on screen - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x01))
			m_samples->start(7, 7);

		/* remember the previous value */
		m_last_shift = m_current_shift;
	}

	/* clock music data on the rising edge of bit 0x01 */
	if (SOUNDVAL_RISING_EDGE(0x01))
	{
		int freq, vol;

		/* start/stop the music sample on the high bit */
		if (SHIFTREG2_RISING_EDGE(0x8000))
			m_samples->start(8, 8, true);
		if (SHIFTREG2_FALLING_EDGE(0x8000))
			m_samples->stop(8);

		/* set the frequency */
		freq = 56818.181818 / (4096 - (m_current_shift & 0xfff));
		m_samples->set_frequency(8, 44100 * freq / 1050);

		/* set the volume */
		vol = (~m_current_shift >> 12) & 3;
		m_samples->set_volume(8, vol / 3.0);

		/* cannon - falling edge */
		if (SHIFTREG2_RISING_EDGE(0x4000))
			m_samples->start(9, 9);

		/* remember the previous value */
		m_last_shift2 = m_current_shift;
	}

	/* bounce - rising edge */
	if (SOUNDVAL_RISING_EDGE(0x04))
		m_samples->start(10, 10);

	/* bell - falling edge */
	if (SOUNDVAL_RISING_EDGE(0x08))
		m_samples->start(11, 11);
}

SOUND_RESET_MEMBER( cinemat_state, boxingb )
{
	generic_init(&cinemat_state::boxingb_sound_w);
}

MACHINE_CONFIG_FRAGMENT( boxingb_sound )
	MCFG_SOUND_RESET_OVERRIDE(cinemat_state, boxingb)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(12)
	MCFG_SAMPLES_NAMES(boxingb_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  War of the Worlds
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

void cinemat_state::wotw_sound_w(UINT8 sound_val, UINT8 bits_changed)
{
	UINT32 target_pitch;

	/* on the rising edge of bit 0x10, clock bit 0x80 into the shift register */
	if (SOUNDVAL_RISING_EDGE(0x10))
		m_current_shift = ((m_current_shift >> 1) & 0x7f) | (sound_val & 0x80);

	/* execute on the rising edge of bit 0x01 */
	if (SOUNDVAL_RISING_EDGE(0x01))
	{
		/* fireball - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x80))
			m_samples->start(0, 0);

		/* shield hit - falling edge */
		if (SHIFTREG_FALLING_EDGE(0x40))
			m_samples->start(1, 1);

		/* star sound - 0=off, 1=on */
		if (SHIFTREG_RISING_EDGE(0x20))
			m_samples->start(2, 2, true);
		if (SHIFTREG_FALLING_EDGE(0x20))
			m_samples->stop(2);

		/* thrust sound - 1=off, 0=on*/
		if (SHIFTREG_FALLING_EDGE(0x10))
			m_samples->start(3, 3, true);
		if (SHIFTREG_RISING_EDGE(0x10))
			m_samples->stop(3);

		/* drone - 1=off, 0=on */
		if (SHIFTREG_FALLING_EDGE(0x08))
			m_samples->start(4, 4, true);
		if (SHIFTREG_RISING_EDGE(0x08))
			m_samples->stop(4);

		/* latch the drone pitch */
		target_pitch = (m_current_shift & 7) + ((m_current_shift & 2) << 2);
		target_pitch = 0x10000 + (target_pitch << 12);

		/* once per frame slide the pitch toward the target */
		if (m_screen->frame_number() > m_last_frame)
		{
			if (m_current_pitch > target_pitch)
				m_current_pitch -= 300;
			if (m_current_pitch < target_pitch)
				m_current_pitch += 200;
			m_samples->set_frequency(4, m_current_pitch);
			m_last_frame = m_screen->frame_number();
		}

		/* remember the previous value */
		m_last_shift = m_current_shift;
	}

	/* loud explosion - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x02))
		m_samples->start(5, 5);

	/* soft explosion - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x04))
		m_samples->start(6, 6);

	/* player fire - falling edge */
	if (SOUNDVAL_FALLING_EDGE(0x08))
		m_samples->start(7, 7);
}

SOUND_RESET_MEMBER( cinemat_state, wotw )
{
	generic_init(&cinemat_state::wotw_sound_w);
}

MACHINE_CONFIG_FRAGMENT( wotw_sound )
	MCFG_SOUND_RESET_OVERRIDE(cinemat_state, wotw)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(8)
	MCFG_SAMPLES_NAMES(wotw_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  Demon
 *
 *************************************/

TIMER_CALLBACK_MEMBER( cinemat_state::synced_sound_w )
{
	m_sound_fifo[m_sound_fifo_in] = param;
	m_sound_fifo_in = (m_sound_fifo_in + 1) % 16;
}


void cinemat_state::demon_sound_w(UINT8 sound_val, UINT8 bits_changed)
{
	/* all inputs are inverted */
	sound_val = ~sound_val;

	/* watch for a 0->1 edge on bit 4 ("shift in") to clock in the new data */
	if ((bits_changed & 0x10) && (sound_val & 0x10))
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(cinemat_state::synced_sound_w), this), sound_val & 0x0f);
}


READ8_MEMBER(cinemat_state::sound_porta_r)
{
	/* bits 0-3 are the sound data; bit 4 is the data ready */
	return m_sound_fifo[m_sound_fifo_out] | ((m_sound_fifo_in != m_sound_fifo_out) << 4);
}


READ8_MEMBER(cinemat_state::sound_portb_r)
{
	return m_last_portb_write;
}


WRITE8_MEMBER(cinemat_state::sound_portb_w)
{
	/* watch for a 0->1 edge on bit 0 ("shift out") to advance the data pointer */
	if ((data & 1) != (m_last_portb_write & 1) && (data & 1) != 0)
		m_sound_fifo_out = (m_sound_fifo_out + 1) % 16;

	/* watch for a 0->1 edge of bit 1 ("hard reset") to reset the FIFO */
	if ((data & 2) != (m_last_portb_write & 2) && (data & 2) != 0)
		m_sound_fifo_in = m_sound_fifo_out = 0;

	/* bit 2 controls the global mute */
	if ((data & 4) != (m_last_portb_write & 4))
		machine().sound().system_mute(data & 4);

	/* remember the last value written */
	m_last_portb_write = data;
}

WRITE8_MEMBER(cinemat_state::sound_output_w)
{
	logerror("sound_output = %02X\n", data);
}

SOUND_RESET_MEMBER( cinemat_state, demon )
{
	/* generic init */
	generic_init(&cinemat_state::demon_sound_w);

	/* reset the FIFO */
	m_sound_fifo_in = m_sound_fifo_out = 0;
	m_last_portb_write = 0xff;

	/* turn off channel A on AY8910 #0 because it is used as a low-pass filter */
	m_ay1->set_volume(0, 0);
}


static ADDRESS_MAP_START( demon_sound_map, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x3000, 0x33ff) AM_RAM
	AM_RANGE(0x4000, 0x4001) AM_DEVREAD("ay1", ay8910_device, data_r)
	AM_RANGE(0x4002, 0x4003) AM_DEVWRITE("ay1", ay8910_device, data_address_w)
	AM_RANGE(0x5000, 0x5001) AM_DEVREAD("ay2", ay8910_device, data_r)
	AM_RANGE(0x5002, 0x5003) AM_DEVWRITE("ay2", ay8910_device, data_address_w)
	AM_RANGE(0x6000, 0x6001) AM_DEVREAD("ay3", ay8910_device, data_r)
	AM_RANGE(0x6002, 0x6003) AM_DEVWRITE("ay3", ay8910_device, data_address_w)
	AM_RANGE(0x7000, 0x7000) AM_WRITENOP  /* watchdog? */
ADDRESS_MAP_END


static ADDRESS_MAP_START( demon_sound_ports, AS_IO, 8, driver_device )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVWRITE("ctc", z80ctc_device, write)
	AM_RANGE(0x1c, 0x1f) AM_DEVWRITE("ctc", z80ctc_device, write)
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

	MCFG_DEVICE_ADD("ctc", Z80CTC, 3579545 /* same as "audiocpu" */)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("audiocpu", INPUT_LINE_IRQ0))

	MCFG_SOUND_RESET_OVERRIDE(cinemat_state, demon)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 3579545)
	MCFG_AY8910_PORT_A_READ_CB(READ8(cinemat_state, sound_porta_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(cinemat_state, sound_portb_r))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(cinemat_state, sound_portb_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, 3579545)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay3", AY8910, 3579545)


	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(cinemat_state, sound_output_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/*************************************
 *
 *  QB3
 *
 *************************************/

WRITE8_MEMBER(cinemat_state::qb3_sound_w)
{
	UINT16 rega = m_maincpu->state_int(CCPU_A);
	demon_sound_w(0x00 | (~rega & 0x0f), 0x10);
}


SOUND_RESET_MEMBER( cinemat_state, qb3 )
{
	SOUND_RESET_CALL_MEMBER(demon);
	m_maincpu->space(AS_IO).install_write_handler(0x04, 0x04, write8_delegate(FUNC(cinemat_state::qb3_sound_w),this));

	/* this patch prevents the sound ROM from eating itself when command $0A is sent */
	/* on a cube rotate */
	memregion("audiocpu")->base()[0x11dc] = 0x09;
}


MACHINE_CONFIG_DERIVED( qb3_sound, demon_sound )
	MCFG_SOUND_RESET_OVERRIDE(cinemat_state, qb3)
MACHINE_CONFIG_END
