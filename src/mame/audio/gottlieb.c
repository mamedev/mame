/***************************************************************************

    Gottlieb hardware
    dedicated to Warren Davis, Jeff Lee, Tim Skelly & David Thiel

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "cpu/m6502/m6502.h"
#include "machine/6532riot.h"
#include "sound/samples.h"
#include "sound/dac.h"
#include "sound/ay8910.h"
//#include "sound/votrax.h"
#include "sound/sp0250.h"
#include "includes/gottlieb.h"


#define SOUND1_CLOCK		XTAL_3_579545MHz
#define SOUND2_CLOCK		XTAL_4MHz
#define SOUND2_SPEECH_CLOCK	XTAL_3_12MHz









static void gottlieb1_sh_w(device_t *riot, UINT8 data);
static void gottlieb2_sh_w(address_space *space, UINT8 data);
static void trigger_sample(samples_device *samples, UINT8 data);



/*************************************
 *
 *  Generic interfaces
 *
 *************************************/

WRITE8_HANDLER( gottlieb_sh_w )
{
	device_t *riot = space->machine().device("riot");

	/* identify rev1 boards by the presence of a 6532 RIOT device */
	if (riot != NULL)
		gottlieb1_sh_w(riot, data);
	else
		gottlieb2_sh_w(space, data);
}



/*************************************
 *
 *  Rev. 1 handlers
 *
 *************************************/

static void gottlieb1_sh_w(device_t *riot, UINT8 data)
{
	samples_device *samples = riot->machine().device<samples_device>("samples");
	int pa7 = (data & 0x0f) != 0xf;
	int pa0_5 = ~data & 0x3f;

	/* snoop the data looking for commands that need samples */
	if (pa7 && samples != NULL)
		trigger_sample(samples, pa0_5);

	/* write the command data to the low 6 bits, and the trigger to the upper bit */
	riot6532_porta_in_set(riot, pa0_5 | (pa7 << 7), 0xbf);
}



/*************************************
 *
 *  Rev. 1 RIOT interfaces
 *
 *************************************/

static WRITE_LINE_DEVICE_HANDLER( snd_interrupt )
{
	cputag_set_input_line(device->machine(), "audiocpu", M6502_IRQ_LINE, state);
}


static WRITE8_DEVICE_HANDLER( r6532_portb_w )
{
	/* unsure if this is ever used, but the NMI is connected to the RIOT's PB7 */
	cputag_set_input_line(device->machine(), "audiocpu", INPUT_LINE_NMI, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
}


static const riot6532_interface gottlieb_riot6532_intf =
{
	DEVCB_NULL,
	DEVCB_INPUT_PORT("SB1"),
	DEVCB_NULL,
	DEVCB_HANDLER(r6532_portb_w),
	DEVCB_LINE(snd_interrupt)
};



/*************************************
 *
 *  Rev. 1 sample players
 *
 *************************************/

static void play_sample(samples_device *samples, const char *phonemes)
{
	if (strcmp(phonemes, "[0] HEH3LOOW     AH1EH3I3YMTERI2NDAHN") == 0)	  /* Q-Bert - Hello, I am turned on */
		samples->start(0, 42);
	else if (strcmp(phonemes, "[0]BAH1EH1Y") == 0)							  /* Q-Bert - Bye, bye */
		samples->start(0, 43);
	else if (strcmp(phonemes, "[0]A2YHT LEH2FTTH") == 0)					  /* Reactor - Eight left */
		samples->start(0, 0);
	else if (strcmp(phonemes, "[0]SI3KS DTYN LEH2FTTH") == 0)				  /* Reactor - Sixteen left */
		samples->start(0, 1);
	else if (strcmp(phonemes, "[0]WO2RNYNG KO2R UH1NSDTABUH1L") == 0)		  /* Reactor - Warning core unstable */
		samples->start(0, 5);
	else if (strcmp(phonemes, "[0]CHAMBERR   AE1EH2KTI1VA1I3DTEH1DT ") == 0) /* Reactor - Chamber activated */
		samples->start(0, 7);
}


static void trigger_sample(samples_device *samples, UINT8 data)
{
	gottlieb_state *state = samples->machine().driver_data<gottlieb_state>();
	/* Reactor samples */
	if (strcmp(samples->machine().system().name, "reactor") == 0)
	{
		switch (data)
		{
			case 55:
			case 56:
			case 57:
			case 59:
				samples->start(0, data - 53);
				break;

			case 31:
				state->m_score_sample = 7;
				break;

			case 39:
				state->m_score_sample++;
				if (state->m_score_sample < 20)
					samples->start(0, state->m_score_sample);
				break;
		}
	}

	/* Q*Bert samples */
	else
	{
		switch (data)
		{
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
				samples->start(0, (data - 17) * 8 + state->m_random_offset);
				state->m_random_offset = (state->m_random_offset + 1) & 7;
				break;

			case 22:
				samples->start(0,40);
				break;

			case 23:
				samples->start(0,41);
				break;
		}
	}
}


#ifdef UNUSED_FUNCTION
void gottlieb_knocker(running_machine &machine)
{
	samples_device *samples = space->machine().device<samples_device>("samples");
	if (!strcmp(machine.system().name,"reactor"))	/* reactor */
	{
	}
	else if (samples != NULL)	/* qbert */
		samples->start(0,44);
}
#endif



/*************************************
 *
 *  Rev. 1 speech interface
 *
 *************************************/

/* callback for the timer */
static TIMER_CALLBACK( gottlieb_nmi_generate )
{
	cputag_set_input_line(machine, "audiocpu", INPUT_LINE_NMI, PULSE_LINE);
}


static WRITE8_HANDLER( vortrax_data_w )
{
	gottlieb_state *state = space->machine().driver_data<gottlieb_state>();
	static const char *const PhonemeTable[0x40] =
	{
		"EH3", "EH2", "EH1", "PA0", "DT" , "A1" , "A2" , "ZH",
		"AH2", "I3" , "I2" , "I1" , "M"  , "N"  , "B"  , "V",
		"CH" , "SH" , "Z"  , "AW1", "NG" , "AH1", "OO1", "OO",
		"L"  , "K"  , "J"  , "H"  , "G"  , "F"  , "D"  , "S",
		"A"  , "AY" , "Y1" , "UH3", "AH" , "P"  , "O"  , "I",
		"U"  , "Y"  , "T"  , "R"  , "E"  , "W"  , "AE" , "AE1",
		"AW2", "UH2", "UH1", "UH" , "O2" , "O1" , "IU" , "U1",
		"THV", "TH" , "ER" , "EH" , "E1" , "AW" , "PA1", "STOP"
	};

	data ^= 0xff;

logerror("Votrax: intonation %d, phoneme %02x %s\n",data >> 6,data & 0x3f,PhonemeTable[data & 0x3f]);

	state->m_votrax_queue[state->m_votrax_queuepos++] = data;

	if ((data & 0x3f) == 0x3f)
	{
		if (state->m_votrax_queuepos > 1)
		{
			samples_device *samples = space->machine().device<samples_device>("samples");
			int last = -1;
			int i;
			char phonemes[200];

			phonemes[0] = 0;
			for (i = 0;i < state->m_votrax_queuepos-1;i++)
			{
				static const char *const inf[4] = { "[0]", "[1]", "[2]", "[3]" };
				int phoneme = state->m_votrax_queue[i] & 0x3f;
				int inflection = state->m_votrax_queue[i] >> 6;
				if (inflection != last) strcat(phonemes, inf[inflection]);
				last = inflection;
				if (phoneme == 0x03 || phoneme == 0x3e) strcat(phonemes," ");
				else strcat(phonemes,PhonemeTable[phoneme]);
			}

			mame_printf_debug("Votrax played '%s'\n", phonemes);
			play_sample(samples, phonemes);
#if 0
			popmessage("%s", phonemes);
#endif
		}

		state->m_votrax_queuepos = 0;
	}

	/* generate a NMI after a while to make the CPU continue to send data */
	space->machine().scheduler().timer_set(attotime::from_usec(50), FUNC(gottlieb_nmi_generate));
}

static WRITE8_HANDLER( speech_clock_dac_w )
{
	gottlieb_state *state = space->machine().driver_data<gottlieb_state>();
if (data != state->m_last)
	mame_printf_debug("clock = %02X\n", data);
state->m_last = data;
}


/*************************************
 *
 *  Rev 1. initialization
 *
 *************************************/

static SOUND_START( gottlieb1 )
{
	gottlieb_state *state = machine.driver_data<gottlieb_state>();
	state->m_score_sample = 7;
	state->m_random_offset = 0;

	state_save_register_global_array(machine, state->m_votrax_queue);
	state_save_register_global(machine, state->m_votrax_queuepos);
}



/*************************************
 *
 *  Rev 1. address map
 *
 *************************************/

static ADDRESS_MAP_START( gottlieb_sound1_map, AS_PROGRAM, 8 )
	/* A15 not decoded except in expansion socket */
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x0d80) AM_RAM
	AM_RANGE(0x0200, 0x021f) AM_MIRROR(0x0de0) AM_DEVREADWRITE("riot", riot6532_r, riot6532_w)
	AM_RANGE(0x1000, 0x1000) AM_MIRROR(0x0fff) AM_DEVWRITE("dac", dac_w)
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x0fff) AM_WRITE(vortrax_data_w)
	AM_RANGE(0x3000, 0x3000) AM_MIRROR(0x0fff) AM_WRITE(speech_clock_dac_w)
	AM_RANGE(0x6000, 0x7fff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Rev. 1 machine driver
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( gottlieb_soundrev1 )
	MCFG_SOUND_START(gottlieb1)

	MCFG_RIOT6532_ADD("riot", SOUND1_CLOCK/4, gottlieb_riot6532_intf)

	MCFG_CPU_ADD("audiocpu", M6502, SOUND1_CLOCK/4)	/* the board can be set to /2 as well */
	MCFG_CPU_PROGRAM_MAP(gottlieb_sound1_map)

	/* sound hardware */
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  Rev. 1 input ports
 *
 *************************************/

INPUT_PORTS_START( gottlieb1_sound )
	PORT_START("SB1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SB1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SB1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SB1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SB1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SB1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SB1:3" )
	PORT_DIPNAME( 0x40, 0x40, "Sound Test" )			PORT_DIPLOCATION("SB1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, 0x80, IPT_UNKNOWN )	/* To U3-6 on QBert */
INPUT_PORTS_END



/*************************************
 *
 *  Rev. 2 communication handlers
 *
 *************************************/

static void gottlieb2_sh_w(address_space *space, UINT8 data)
{
	gottlieb_state *state = space->machine().driver_data<gottlieb_state>();
	/* when data is not 0xff, the transparent latch at A3 allows it to pass through unmolested */
	if (data != 0xff)
	{
		/* each CPU has its own latch */
		soundlatch_w(space, 0, data);
		soundlatch2_w(space, 0, data);

		/* if the previous data was 0xff, clock an IRQ on each */
		if (state->m_last_command == 0xff)
		{
			cputag_set_input_line(space->machine(), "audiocpu", M6502_IRQ_LINE, ASSERT_LINE);
			cputag_set_input_line(space->machine(), "speech", M6502_IRQ_LINE, ASSERT_LINE);
		}
	}
	state->m_last_command = data;
}


static READ8_HANDLER( speech_data_r )
{
	cputag_set_input_line(space->machine(), "speech", M6502_IRQ_LINE, CLEAR_LINE);
	return soundlatch_r(space, offset);
}


static READ8_HANDLER( audio_data_r )
{
	cputag_set_input_line(space->machine(), "audiocpu", M6502_IRQ_LINE, CLEAR_LINE);
	return soundlatch2_r(space, offset);
}


static WRITE8_HANDLER( signal_audio_nmi_w )
{
	cputag_set_input_line(space->machine(), "audiocpu", INPUT_LINE_NMI, ASSERT_LINE);
	cputag_set_input_line(space->machine(), "audiocpu", INPUT_LINE_NMI, CLEAR_LINE);
}



/*************************************
 *
 *  Rev. 2 NMI timer
 *
 *************************************/

INLINE void nmi_timer_adjust(running_machine &machine)
{
	gottlieb_state *state = machine.driver_data<gottlieb_state>();
	/* adjust timer to go off in the future based on the current rate */
	state->m_nmi_timer->adjust(attotime::from_hz(SOUND2_CLOCK/16) * (256 * (256 - state->m_nmi_rate)));
}


INLINE void nmi_state_update(running_machine &machine)
{
	gottlieb_state *state = machine.driver_data<gottlieb_state>();
	/* update the NMI line state based on the enable and state */
	cputag_set_input_line(machine, "speech", INPUT_LINE_NMI, (state->m_nmi_state && (state->m_speech_control & 1)) ? ASSERT_LINE : CLEAR_LINE);
}


static TIMER_CALLBACK( nmi_clear )
{
	gottlieb_state *state = machine.driver_data<gottlieb_state>();
	/* clear the NMI state and update it */
	state->m_nmi_state = 0;
	nmi_state_update(machine);
}


static TIMER_CALLBACK( nmi_callback )
{
	gottlieb_state *state = machine.driver_data<gottlieb_state>();
	/* assert the NMI if it is not disabled */
	state->m_nmi_state = 1;
	nmi_state_update(machine);

	/* set a timer to turn it off again on hte next SOUND_CLOCK/16 */
	machine.scheduler().timer_set(attotime::from_hz(SOUND2_CLOCK/16), FUNC(nmi_clear));

	/* adjust the NMI timer for the next time */
	nmi_timer_adjust(machine);
}


static WRITE8_HANDLER( nmi_rate_w )
{
	gottlieb_state *state = space->machine().driver_data<gottlieb_state>();
	/* the new rate is picked up when the previous timer expires */
	state->m_nmi_rate = data;
}



/*************************************
 *
 *  Rev. 2 sound chip access
 *
 *************************************/

static CUSTOM_INPUT( speech_drq_custom_r )
{
	return sp0250_drq_r(field.machine().device("spsnd"));
}


static WRITE8_DEVICE_HANDLER( gottlieb_dac_w )
{
	gottlieb_state *state = device->machine().driver_data<gottlieb_state>();
	/* dual DAC; the first DAC serves as the reference voltage for the
       second, effectively scaling the output */
	state->m_dac_data[offset] = data;
	dac_data_16_w(device, state->m_dac_data[0] * state->m_dac_data[1]);
}


static WRITE8_HANDLER( speech_control_w )
{
	gottlieb_state *state = space->machine().driver_data<gottlieb_state>();
	UINT8 previous = state->m_speech_control;
	state->m_speech_control = data;

	/* bit 0 enables/disables the NMI line */
	nmi_state_update(space->machine());

	/* bit 1 controls a LED on the sound board */

	/* bit 2 goes to 8913 BDIR pin */
	if ((previous & 0x04) != 0 && (data & 0x04) == 0)
	{
		/* bit 3 selects which of the two 8913 to enable */
		/* bit 4 goes to the 8913 BC1 pin */
		device_t *ay = space->machine().device((data & 0x08) ? "ay1" : "ay2");
		ay8910_data_address_w(ay, data >> 4, *state->m_psg_latch);
	}

	/* bit 5 goes to the speech chip DIRECT DATA TEST pin */

	/* bit 6 = speech chip DATA PRESENT pin; high then low to make the chip read data */
	if ((previous & 0x40) == 0 && (data & 0x40) != 0)
	{
		device_t *sp = space->machine().device("spsnd");
		sp0250_w(sp, 0, *state->m_sp0250_latch);
	}

	/* bit 7 goes to the speech chip RESET pin */
	if ((previous ^ data) & 0x80)
		space->machine().device("spsnd")->reset();
}

static WRITE8_HANDLER( cobram3_speech_control_w )
{
	gottlieb_state *state = space->machine().driver_data<gottlieb_state>();
	UINT8 previous = state->m_speech_control;
	state->m_speech_control = data;

	/* bit 0 enables/disables the NMI line */
	nmi_state_update(space->machine());

	/* bit 1 controls a LED on the sound board */

	if ( data & 0x10 )
	{
		state->m_psg_data_latch = *state->m_psg_latch;
	}
	else
	{
		device_t *ay = space->machine().device((data & 0x08) ? "ay1" : "ay2");
		ay8910_address_w(ay, 0, *state->m_psg_latch);
		ay8910_data_w(ay, 0, state->m_psg_data_latch);
	}
	/* bit 5 goes to the speech chip DIRECT DATA TEST pin */

	/* bit 6 = speech chip DATA PRESENT pin; high then low to make the chip read data */
	if ((previous & 0x40) == 0 && (data & 0x40) != 0)
	{
		device_t *sp = space->machine().device("spsnd");
		sp0250_w(sp, 0, *state->m_sp0250_latch);
	}

	/* bit 7 goes to the speech chip RESET pin */
	if ((previous ^ data) & 0x80)
		space->machine().device("spsnd")->reset();
}


/*************************************
 *
 *  Rev. 2 initialization
 *
 *************************************/

static SOUND_START( gottlieb2 )
{
	gottlieb_state *state = machine.driver_data<gottlieb_state>();
	/* set up the NMI timer */
	state->m_nmi_timer = machine.scheduler().timer_alloc(FUNC(nmi_callback));
	state->m_nmi_rate = 0;
	nmi_timer_adjust(machine);

	state->m_dac_data[0] = state->m_dac_data[1] = 0xff;

	/* register for save states */
	state_save_register_global(machine, state->m_nmi_rate);
	state_save_register_global(machine, state->m_nmi_state);
	state_save_register_global(machine, state->m_speech_control);
	state_save_register_global(machine, state->m_last_command);
}



/*************************************
 *
 *  Rev. 2 address map
 *
 *************************************/

static ADDRESS_MAP_START( gottlieb_speech2_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x1c00) AM_RAM
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x1fff) AM_WRITEONLY AM_BASE_MEMBER(gottlieb_state, m_sp0250_latch)
	AM_RANGE(0x4000, 0x4000) AM_MIRROR(0x1fff) AM_WRITE(speech_control_w)
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x1fff) AM_READ_PORT("SB2")
	AM_RANGE(0x8000, 0x8000) AM_MIRROR(0x1fff) AM_WRITEONLY AM_BASE_MEMBER(gottlieb_state, m_psg_latch)
	AM_RANGE(0xa000, 0xa000) AM_MIRROR(0x07ff) AM_WRITE(nmi_rate_w)
	AM_RANGE(0xa800, 0xa800) AM_MIRROR(0x07ff) AM_READ(speech_data_r)
	AM_RANGE(0xb000, 0xb000) AM_MIRROR(0x07ff) AM_WRITE(signal_audio_nmi_w)
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( gottlieb_cobram3_speech2_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x1c00) AM_RAM
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x1fff) AM_WRITEONLY AM_BASE_MEMBER(gottlieb_state, m_sp0250_latch)
	AM_RANGE(0x4000, 0x4000) AM_MIRROR(0x1fff) AM_WRITE(cobram3_speech_control_w)
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x1fff) AM_READ_PORT("SB2")
	AM_RANGE(0x8000, 0x8000) AM_MIRROR(0x1fff) AM_WRITEONLY AM_BASE_MEMBER(gottlieb_state, m_psg_latch)
	AM_RANGE(0xa000, 0xa000) AM_MIRROR(0x07ff) AM_WRITE(nmi_rate_w)
	AM_RANGE(0xa800, 0xa800) AM_MIRROR(0x07ff) AM_READ(speech_data_r)
	AM_RANGE(0xb000, 0xb000) AM_MIRROR(0x07ff) AM_WRITE(signal_audio_nmi_w)
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( gottlieb_audio2_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x3c00) AM_RAM
	AM_RANGE(0x4000, 0x4001) AM_MIRROR(0x3ffe) AM_DEVWRITE("dac1", gottlieb_dac_w) AM_BASE_MEMBER(gottlieb_state, m_dac_data)
	AM_RANGE(0x8000, 0x8000) AM_MIRROR(0x3fff) AM_READ(audio_data_r)
	AM_RANGE(0xe000, 0xffff) AM_MIRROR(0x2000) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Rev. 2 machine driver
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( gottlieb_soundrev2 )
	/* audio CPUs */
	MCFG_CPU_ADD("audiocpu", M6502, SOUND2_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(gottlieb_audio2_map)

	MCFG_CPU_ADD("speech", M6502, SOUND2_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(gottlieb_speech2_map)

	/* sound hardware */
	MCFG_SOUND_START( gottlieb2 )

	MCFG_SOUND_ADD("dac1", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("dac2", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("ay1", AY8913, SOUND2_CLOCK/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("ay2", AY8913, SOUND2_CLOCK/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("spsnd", SP0250, SOUND2_SPEECH_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( gottlieb_cobram3_soundrev2 )
	/* audio CPUs */
	MCFG_CPU_ADD("audiocpu", M6502, SOUND2_CLOCK/4)
	MCFG_DEVICE_DISABLE()
	MCFG_CPU_PROGRAM_MAP(gottlieb_audio2_map)

	MCFG_CPU_ADD("speech", M6502, SOUND2_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(gottlieb_cobram3_speech2_map)

	/* sound hardware */
	MCFG_SOUND_START( gottlieb2 )

	MCFG_SOUND_ADD("dac1", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("dac2", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("ay1", AY8913, SOUND2_CLOCK/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("ay2", AY8913, SOUND2_CLOCK/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("spsnd", SP0250, SOUND2_SPEECH_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/*************************************
 *
 *  Rev. 2 input ports
 *
 *************************************/

INPUT_PORTS_START( gottlieb2_sound )
	PORT_START("SB2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SB2:1")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SB2:2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SB2:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SB2:4")
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SB2:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SB2:6")
	PORT_DIPNAME( 0x40, 0x40, "Sound Test" )			PORT_DIPLOCATION("SB2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(speech_drq_custom_r, NULL)
INPUT_PORTS_END
