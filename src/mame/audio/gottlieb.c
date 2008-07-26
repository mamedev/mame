/***************************************************************************

    Gottlieb hardware
    dedicated to Warren Davis, Jeff Lee, Tim Skelly & David Thiel

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/m6502/m6502.h"
#include "machine/6532riot.h"
#include "sound/samples.h"
#include "sound/dac.h"
#include "sound/ay8910.h"
//#include "sound/votrax.h"
#include "sound/sp0250.h"


#define SOUND1_CLOCK		XTAL_3_579545MHz
#define SOUND2_CLOCK		XTAL_4MHz
#define SOUND2_SPEECH_CLOCK	XTAL_3_12MHz


static UINT8 votrax_queue[100];
static UINT8 votrax_queuepos;

static UINT8 psg_latch;
static emu_timer *nmi_timer;
static int nmi_rate;
static int sp0250_drq;
static UINT8 sp0250_latch;



static void gottlieb1_sh_w(const device_config *riot, UINT8 data);
static void gottlieb2_sh_w(running_machine *machine, UINT8 data);
static void trigger_sample(running_machine *machine, UINT8 data);



/*************************************
 *
 *  Generic interfaces
 *
 *************************************/

WRITE8_HANDLER( gottlieb_sh_w )
{
	const device_config *riot = device_list_find_by_tag(machine->config->devicelist, RIOT6532, "riot");
	
	/* identify rev1 boards by the presence of a 6532 RIOT device */
	if (riot != NULL)
		gottlieb1_sh_w(riot, data);
	else
		gottlieb2_sh_w(machine, data);
}



/*************************************
 *
 *  Rev. 1 handlers
 *
 *************************************/

static SOUND_START( gottlieb1 )
{
	state_save_register_global_array(votrax_queue);
	state_save_register_global(votrax_queuepos);
}


static void gottlieb1_sh_w(const device_config *riot, UINT8 data)
{
	int pa7 = (data & 0x0f) != 0xf;
	int pa0_5 = ~data & 0x3f;

	/* snoop the data looking for commands that need samples */
	if (pa7 && sndti_exists(SOUND_SAMPLES, 0))
		trigger_sample(riot->machine, pa0_5);

	/* write the command data to the low 6 bits, and the trigger to the upper bit */
	riot6532_porta_in_set(riot, pa0_5 | (pa7 << 7), 0xbf);
}



/*************************************
 *
 *  Rev. 1 RIOT interfaces
 *
 *************************************/

static void snd_interrupt(const device_config *device, int state)
{
	cpunum_set_input_line(device->machine, 1, M6502_IRQ_LINE, state);
}


static UINT8 r6532_portb_r(const device_config *device, UINT8 olddata)
{
	return input_port_read(device->machine, "SB1");
}


static void r6532_portb_w(const device_config *device, UINT8 newdata, UINT8 olddata)
{
	/* unsure if this is ever used, but the NMI is connected to the RIOT's PB7 */
	cpunum_set_input_line(device->machine, 1, INPUT_LINE_NMI, (newdata & 0x80) ? CLEAR_LINE : ASSERT_LINE);
}


static const riot6532_interface gottlieb_riot6532_intf =
{
	NULL,
	r6532_portb_r,
	NULL,
	r6532_portb_w,
	snd_interrupt
};



/*************************************
 *
 *  Rev. 1 sample players
 *
 *************************************/

#if 0
static void votrax_ready(running_machine *machine, int state)
{
	cpunum_set_input_line(machine, 1, INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}

static votrax_sample votrax_samples[] =
{
	{ " HEH3LOOW     AH1EH3I3YMTERI2NDAHN", 	"fx_28.wav" },	/* "hello, I'm ready" */
	{ "BAH1EH1Y",								"fx_36.wav" },	/* "byebye" */
};

static votrax_interface votrax_intf =
{
	votrax_ready,
	votrax_samples
};
#endif
#if 0
	"fx_17a.wav", /* random speech, voice clock 255 */
	"fx_17b.wav", /* random speech, voice clock 255 */
	"fx_17c.wav", /* random speech, voice clock 255 */
	"fx_17d.wav", /* random speech, voice clock 255 */
	"fx_17e.wav", /* random speech, voice clock 255 */
	"fx_17f.wav", /* random speech, voice clock 255 */
	"fx_17g.wav", /* random speech, voice clock 255 */
	"fx_17h.wav", /* random speech, voice clock 255 */
	"fx_18a.wav", /* random speech, voice clock 176 */
	"fx_18b.wav", /* random speech, voice clock 176 */
	"fx_18c.wav", /* random speech, voice clock 176 */
	"fx_18d.wav", /* random speech, voice clock 176 */
	"fx_18e.wav", /* random speech, voice clock 176 */
	"fx_18f.wav", /* random speech, voice clock 176 */
	"fx_18g.wav", /* random speech, voice clock 176 */
	"fx_18h.wav", /* random speech, voice clock 176 */
	"fx_19a.wav", /* random speech, voice clock 128 */
	"fx_19b.wav", /* random speech, voice clock 128 */
	"fx_19c.wav", /* random speech, voice clock 128 */
	"fx_19d.wav", /* random speech, voice clock 128 */
	"fx_19e.wav", /* random speech, voice clock 128 */
	"fx_19f.wav", /* random speech, voice clock 128 */
	"fx_19g.wav", /* random speech, voice clock 128 */
	"fx_19h.wav", /* random speech, voice clock 128 */
	"fx_20a.wav", /* random speech, voice clock 96 */
	"fx_20b.wav", /* random speech, voice clock 96 */
	"fx_20c.wav", /* random speech, voice clock 96 */
	"fx_20d.wav", /* random speech, voice clock 96 */
	"fx_20e.wav", /* random speech, voice clock 96 */
	"fx_20f.wav", /* random speech, voice clock 96 */
	"fx_20g.wav", /* random speech, voice clock 96 */
	"fx_20h.wav", /* random speech, voice clock 96 */
	"fx_21a.wav", /* random speech, voice clock 62 */
	"fx_21b.wav", /* random speech, voice clock 62 */
	"fx_21c.wav", /* random speech, voice clock 62 */
	"fx_21d.wav", /* random speech, voice clock 62 */
	"fx_21e.wav", /* random speech, voice clock 62 */
	"fx_21f.wav", /* random speech, voice clock 62 */
	"fx_21g.wav", /* random speech, voice clock 62 */
	"fx_21h.wav", /* random speech, voice clock 62 */
	"fx_22.wav", /* EH2 with decreasing voice clock */
	"fx_23.wav", /* O1 with varying voice clock */
#endif

static void play_sample(const char *phonemes)
{
// 350ms @ clock = A0 (~1018kHz)
// H  + EH3 + L   + OO  + W
// 71 + 59  + 103 + 185 + 80 = 495ms @ nominal clock (720kHz)

// 430ms @ clock = CD (~738kHz)
// B  + AH1 + EH1 + Y
// 71 + 146 + 121 + 103 = 441 @ nominal (720kHz)
//
// B + AH1 + I3 + Y1
// 71 + 146 + 55 + 80 = 
//
	if (strcmp(phonemes, " HEH3LOOW     AH1EH3I3YMTERI2NDAHN") == 0)	  /* Q-Bert - Hello, I am turned on */
		sample_start(0, 42, 0);
	else if (strcmp(phonemes, "BAH1EH1Y") == 0)							  /* Q-Bert - Bye, bye */
		sample_start(0, 43, 0);
	else if (strcmp(phonemes, "A2YHT LEH2FTTH") == 0) 					  /* Reactor - Eight left */
		sample_start(0, 0, 0);
	else if (strcmp(phonemes, "SI3KS DTYN LEH2FTTH") == 0) 				  /* Reactor - Sixteen left */
		sample_start(0, 1, 0);
	else if (strcmp(phonemes, "WO2RNYNG KO2R UH1NSDTABUH1L") == 0) 		  /* Reactor - Warning core unstable */
		sample_start(0, 5, 0);
	else if (strcmp(phonemes, "CHAMBERR   AE1EH2KTI1VA1I3DTEH1DT ") == 0) /* Reactor - Chamber activated */
		sample_start(0, 7, 0);
}


static void trigger_sample(running_machine *machine, UINT8 data)
{
	static int score_sample = 7;
	static int random_offset = 0;

	/* Reactor samples */
	if (strcmp(machine->gamedrv->name, "reactor") == 0)
	{
		switch (data)
		{
			case 55:
			case 56:
			case 57:
			case 59:
				sample_start(0, data - 53, 0);
				break;

			case 31:
				score_sample = 7;
				break;

			case 39:
				score_sample++;
				if (score_sample < 20)
					sample_start(0, score_sample, 0);
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
				sample_start(0, (data - 17) * 8 + random_offset, 0);
				random_offset = (random_offset + 1) & 7;
				break;

			case 22:
				sample_start(0,40,0);
				break;

			case 23:
				sample_start(0,41,0);
				break;
		}
	}
}


#ifdef UNUSED_FUNCTION
void gottlieb_knocker(void)
{
	if (!strcmp(Machine->gamedrv->name,"reactor"))	/* reactor */
	{
	}
	else if (sndti_exists(SOUND_SAMPLES, 0))	/* qbert */
		sample_start(0,44,0);
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
	cpunum_set_input_line(machine, 1,INPUT_LINE_NMI,PULSE_LINE);
}


WRITE8_HANDLER( gottlieb_speech_w )
{
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

	votrax_queue[votrax_queuepos++] = data;

	if ((data & 0x3f) == 0x3f)
	{
		if (votrax_queuepos > 1)
		{
			int last = -1;
			int i;
			char phonemes[200];

			phonemes[0] = 0;
			for (i = 0;i < votrax_queuepos-1;i++)
			{
				static const char *inf[4] = { "[0]", "[1]", "[2]", "[3]" };
				int phoneme = votrax_queue[i] & 0x3f;
				int inflection = votrax_queue[i] >> 6;
				if (inflection != last) strcat(phonemes, inf[inflection]);
				last = inflection;
				if (phoneme == 0x03 || phoneme == 0x3e) strcat(phonemes," ");
				else strcat(phonemes,PhonemeTable[phoneme]);
			}

			printf("Votrax played '%s'\n", phonemes);
			play_sample(phonemes);
#if 0
			popmessage("%s", phonemes);
#endif
		}

		votrax_queuepos = 0;
	}

	/* generate a NMI after a while to make the CPU continue to send data */
	timer_set(ATTOTIME_IN_USEC(50), NULL, 0, gottlieb_nmi_generate);
}

WRITE8_HANDLER( gottlieb_speech_clock_DAC_w )
{
static int last;
if (data != last)
	printf("clock = %02X\n", data);
last = data;
}



/*************************************
 *
 *  Rev 1. address map
 *
 *************************************/

static ADDRESS_MAP_START( sound1_map, ADDRESS_SPACE_PROGRAM, 8 )
	/* A15 not decoded except in expansion socket */
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x0d80) AM_RAM
	AM_RANGE(0x0200, 0x021f) AM_MIRROR(0x0de0) AM_DEVREADWRITE(RIOT6532, "riot", riot6532_r, riot6532_w)
	AM_RANGE(0x1000, 0x1000) AM_MIRROR(0x0fff) AM_WRITE(DAC_0_data_w)
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x0fff) AM_WRITE(gottlieb_speech_w)
	AM_RANGE(0x3000, 0x3000) AM_MIRROR(0x0fff) AM_WRITE(gottlieb_speech_clock_DAC_w)
	AM_RANGE(0x6000, 0x7fff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Rev. 1 machine driver
 *
 *************************************/

MACHINE_DRIVER_START( gottlieb_soundrev1 )
	MDRV_SOUND_START(gottlieb1)

	MDRV_RIOT6532_ADD("riot", SOUND1_CLOCK/4, gottlieb_riot6532_intf)

	/* audio CPU */
	MDRV_CPU_ADD("audio", M6502, SOUND1_CLOCK/4)	/* the board can be set to /2 as well */
	MDRV_CPU_PROGRAM_MAP(sound1_map,0)

	/* sound hardware */
	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END



/*************************************
 *
 *  Rev. 2 handlers
 *
 *************************************/

static void gottlieb2_sh_w(running_machine *machine, UINT8 data)
{
	/* each CPU has its own latch */
	soundlatch_w(machine, 0, data);
	soundlatch2_w(machine, 0, data);

	/* signal an IRQ on each */
	cpunum_set_input_line(machine, 1, M6502_IRQ_LINE, ASSERT_LINE);
	cpunum_set_input_line(machine, 2, M6502_IRQ_LINE, ASSERT_LINE);
}





static TIMER_CALLBACK( nmi_callback );

SOUND_START( gottlieb2 )
{
	nmi_timer = timer_alloc(nmi_callback, NULL);
}

void stooges_sp0250_drq(int level)
{
	sp0250_drq = (level == ASSERT_LINE) ? 1 : 0;
}

READ8_HANDLER( stooges_sound_input_r )
{
	/* bits 0-3 are probably unused (future expansion) */

	/* bits 4 & 5 are two dip switches. Unused? */

	/* bit 6 is the test switch. When 0, the CPU plays a pulsing tone. */

	/* bit 7 comes from the speech chip DATA REQUEST pin */

	return 0x40 | (sp0250_drq << 7);
}

WRITE8_HANDLER( stooges_8910_latch_w )
{
	psg_latch = data;
}

/* callback for the timer */
static TIMER_CALLBACK( nmi_callback )
{
	cpunum_set_input_line(machine, cpu_gettotalcpu()-1, INPUT_LINE_NMI, PULSE_LINE);
}

static WRITE8_HANDLER( common_sound_control_w )
{
	/* Bit 0 enables and starts NMI timer */
	if (data & 0x01)
	{
		/* base clock is 250kHz divided by 256 */
		attotime interval = attotime_mul(ATTOTIME_IN_HZ(250000), 256 * (256-nmi_rate));
		timer_adjust_periodic(nmi_timer, interval, 0, interval);
	}
	else
		timer_adjust_oneshot(nmi_timer, attotime_never, 0);

	/* Bit 1 controls a LED on the sound board. I'm not emulating it */
}

WRITE8_HANDLER( stooges_sp0250_latch_w )
{
	sp0250_latch = data;
}

WRITE8_HANDLER( stooges_sound_control_w )
{
	static int last;

	common_sound_control_w(machine, offset, data);

	/* bit 2 goes to 8913 BDIR pin  */
	if ((last & 0x04) == 0x04 && (data & 0x04) == 0x00)
	{
		/* bit 3 selects which of the two 8913 to enable */
		if (data & 0x08)
		{
			/* bit 4 goes to the 8913 BC1 pin */
			if (data & 0x10)
				AY8910_control_port_0_w(machine,0,psg_latch);
			else
				AY8910_write_port_0_w(machine,0,psg_latch);
		}
		else
		{
			/* bit 4 goes to the 8913 BC1 pin */
			if (data & 0x10)
				AY8910_control_port_1_w(machine,0,psg_latch);
			else
				AY8910_write_port_1_w(machine,0,psg_latch);
		}
	}

	/* bit 5 goes to the speech chip DIRECT DATA TEST pin */

	/* bit 6 = speech chip DATA PRESENT pin; high then low to make the chip read data */
	if ((last & 0x40) == 0x40 && (data & 0x40) == 0x00)
	{
		sp0250_w(machine,0,sp0250_latch);
	}

	/* bit 7 goes to the speech chip RESET pin */

	last = data & 0x44;
}

WRITE8_HANDLER( gottlieb_nmi_rate_w )
{
	nmi_rate = data;
}

WRITE8_HANDLER( gottlieb_cause_dac_nmi_w )
{
	cpunum_set_input_line(machine, cpu_gettotalcpu()-2, INPUT_LINE_NMI, PULSE_LINE);
}



/*************************************
 *
 *  Rev. 2 address map
 *
 *************************************/

static ADDRESS_MAP_START( stooges_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_RAM
	AM_RANGE(0x4000, 0x4001) AM_WRITE(DAC_0_data_w)
	AM_RANGE(0x8000, 0x8000) AM_READ(soundlatch_r)
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( stooges_sound2_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_RAM
	AM_RANGE(0x2000, 0x2000) AM_WRITE(stooges_sp0250_latch_w)	/* speech chip. The game sends strings */
									/* of 15 bytes (clocked by 4000). The chip also */
									/* checks a DATA REQUEST bit in 6000. */
	AM_RANGE(0x4000, 0x4000) AM_WRITE(stooges_sound_control_w)
	AM_RANGE(0x6000, 0x6000) AM_READ(stooges_sound_input_r)	/* various signals */
	AM_RANGE(0x8000, 0x8000) AM_WRITE(stooges_8910_latch_w)
	AM_RANGE(0xa000, 0xa000) AM_WRITE(gottlieb_nmi_rate_w)	/* the timer generates NMIs */
	AM_RANGE(0xa800, 0xa800) AM_READ(soundlatch_r)
	AM_RANGE(0xb000, 0xb000) AM_WRITE(gottlieb_cause_dac_nmi_w)
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END




static const struct sp0250_interface sp0250_interface =
{
	stooges_sp0250_drq
};


/*************************************
 *
 *  Rev. 2 machine driver
 *
 *************************************/

MACHINE_DRIVER_START( gottlieb_soundrev2 )
	/* audio CPUs */
	MDRV_CPU_ADD("audio", M6502, SOUND2_CLOCK/4)
	MDRV_CPU_PROGRAM_MAP(stooges_sound_map,0)

	MDRV_CPU_ADD("audio2", M6502, SOUND2_CLOCK/4)
	MDRV_CPU_PROGRAM_MAP(stooges_sound2_map,0)

	/* sound hardware */
	MDRV_SOUND_START( gottlieb2 )

	MDRV_SOUND_ADD("dac1", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MDRV_SOUND_ADD("dac2", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MDRV_SOUND_ADD("ay1", AY8913, SOUND2_CLOCK/2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MDRV_SOUND_ADD("ay2", AY8913, SOUND2_CLOCK/2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MDRV_SOUND_ADD("sp", SP0250, SOUND2_SPEECH_CLOCK)
	MDRV_SOUND_CONFIG(sp0250_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END
