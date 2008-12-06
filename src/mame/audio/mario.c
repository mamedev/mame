#include "driver.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/dac.h"
#include "sound/ay8910.h"
#include "sound/discrete.h"

#include "includes/mario.h"

/****************************************************************
 *
 * Defines and Macros
 *
 ****************************************************************/

#define USE_8039	(0)			/* set to 1 to try 8039 hack */

#define ACTIVELOW_PORT_BIT(P,A,D)   ((P & (~(1 << A))) | ((D ^ 1) << A))
#define ACTIVEHIGH_PORT_BIT(P,A,D)   ((P & (~(1 << A))) | (D << A))

#define I8035_T_R(M,N) ((soundlatch2_r(M,0) >> (N)) & 1)
#define I8035_T_W_AH(M,N,D) do { state->portT = ACTIVEHIGH_PORT_BIT(state->portT,N,D); soundlatch2_w(M, 0, state->portT); } while (0)

#define I8035_P1_R(M) (soundlatch3_r(M,0))
#define I8035_P2_R(M) (soundlatch4_r(M,0))
#define I8035_P1_W(M,D) soundlatch3_w(M,0,D)

#if (USE_8039)
#define I8035_P2_W(M,D) do { soundlatch4_w(M,0,D); } while (0)
#else
#define I8035_P2_W(M,D) do { set_ea(M, ((D) & 0x20) ? 0 : 1);  soundlatch4_w(M,0,D); } while (0)
#endif

#define I8035_P1_W_AH(M,B,D) I8035_P1_W(M,ACTIVEHIGH_PORT_BIT(I8035_P1_R(M),B,(D)))
#define I8035_P2_W_AH(M,B,D) I8035_P2_W(M,ACTIVEHIGH_PORT_BIT(I8035_P2_R(M),B,(D)))

/****************************************************************
 *
 * Discrete Sound defines
 *
 ****************************************************************/

/* Discrete sound inputs */

#define DS_SOUND0_INV		NODE_01
#define DS_SOUND1_INV		NODE_02
#define DS_SOUND7_INV		NODE_05
#define DS_DAC				NODE_07

#define DS_SOUND0			NODE_208
#define DS_SOUND1			NODE_209
#define DS_SOUND7			NODE_212

#define DS_OUT_SOUND0		NODE_241
#define DS_OUT_SOUND1		NODE_242
#define DS_OUT_SOUND7		NODE_248
#define DS_OUT_DAC			NODE_250

/* Input definitions for write handlers */

#define DS_SOUND0_INP		DS_SOUND0_INV
#define DS_SOUND1_INP		DS_SOUND1_INV
#define DS_SOUND7_INP		DS_SOUND7_INV

/* General defines */

#define VSS					5.0
#define TTL_HIGH			4.0
#define GND					0.0

#define R_PARALLEL(R1,R2) ((R1)*(R2)/((R1)+(R2)))
#define R_SERIE(R1,R2)	  ((R1)+(R2))

/*************************************
 *
 *  statics
 *
 *************************************/

/****************************************************************
 *
 * Mario Discrete Sound Interface
 *
 * verified against picture of a TMA1-04-CPU Board. Some
 * of the values were hard to read, though.
 *
 ****************************************************************/

#define MR_R6		RES_K(4.7)		/* verified                             */
#define MR_R7		RES_K(4.7)		/* verified                             */
#define MR_R17		RES_K(27)		/* verified, 30K in schematics          */
#define MR_R18		RES_K(27)		/* verified, 30K in schematics          */
#define MR_R19		RES_K(22)		/* verified                             */
#define MR_R20		RES_K(22)		/* verified                             */
#define MR_R40		RES_K(22)		/* verified                             */
#define MR_R41		RES_K(100)		/* verified, hard to read               */
#define MR_R61		RES_K(47)		/* verified, hard to read               */
#define MR_R64		RES_K(20)		/* verified                             */
#define MR_R65		RES_K(10)		/* verified                             */

#define MR_C3		CAP_U(10)		/* verified                             */
#define MR_C4		CAP_U(47)		/* illegible, 4.7 or 47 pcb/schematics  */
#define MR_C5		CAP_N(39)		/* illegible on pcb                     */
#define MR_C6		CAP_N(3.9)		/* illegible on pcb                     */
#define MR_C14		CAP_U(4.7)		/* verified                             */
#define MR_C15		CAP_U(4.7)		/* verified                             */
#define MR_C16		CAP_N(6.8)		/* verified                             */
#define MR_C17		CAP_N(22)		/* illegible on pcb                     */
#define MR_C31		CAP_U(0.022)	/* not found                            */
#define MR_C32		CAP_U(1)		/* illegible on pcb                     */
#define MR_C39		CAP_N(4.7)		/* not found                            */
#define MR_C40		CAP_N(22)		/* verified                             */
#define MR_C41		CAP_U(4.7)		/* verified, hard to read               */
#define MR_C43		CAP_U(3.3)		/* verified                             */
#define MR_C44		CAP_U(3.3)		/* verified                             */



/* KT = 0.25 for diode circuit, 0.33 else */

#define DISCRETE_LS123(_N, _T, _R, _C) \
	DISCRETE_ONESHOTR(_N, 0, _T, TTL_HIGH, (0.25 * (_R) * (_C) * (1.0+700./(_R))), DISC_ONESHOT_RETRIG | DISC_ONESHOT_REDGE)
#define DISCRETE_LS123_INV(_N, _T, _R, _C) \
	DISCRETE_ONESHOTR(_N, 0, _T, TTL_HIGH, (0.25 * (_R) * (_C) * (1.0+700./(_R))), DISC_ONESHOT_RETRIG | DISC_ONESHOT_REDGE | DISC_OUT_ACTIVE_LOW)

#define DISCRETE_BITSET(_N, _N1, _B) DISCRETE_TRANSFORM3(_N, _N1, 1 << ((_B)-1), 0, "01&2>")
#define DISCRETE_ENERGY_NAND(_N, _E, _N1, _N2) DISCRETE_TRANSFORM3(_N, _N1, _N2, 1, "201*-")


static const discrete_mixer_desc mario_mixer_desc =
	{DISC_MIXER_IS_RESISTOR,
		{MR_R20, MR_R19, MR_R41, MR_R40},
		{0,0,0,0,0},	// no variable resistors
		{0,0,0,0,0},  // no node capacitors
		0, 0,
		MR_C31,
		MR_C32,
		0, 1};


static DISCRETE_SOUND_START(mario)

	/************************************************/
	/* Input register mapping for dkongjr           */
	/************************************************/

	/* DISCRETE_INPUT_DATA */
    DISCRETE_INPUT_NOT(DS_SOUND0_INV)
    DISCRETE_INPUT_NOT(DS_SOUND1_INV)
    DISCRETE_INPUT_NOT(DS_SOUND7_INV)
    DISCRETE_INPUT_DATA(DS_DAC)

	/************************************************/
	/* SIGNALS                                      */
	/************************************************/

    //DISCRETE_LOGIC_INVERT(DS_SOUND7,1,DS_SOUND7_INV)
	//DISCRETE_LOGIC_INVERT(DS_SOUND9,1,DS_SOUND9_INV)

	/************************************************/
	/* SOUND0                                       */
	/************************************************/

	DISCRETE_LS123(NODE_10, DS_SOUND0_INV, MR_R17, MR_C14)
	DISCRETE_RCFILTER(NODE_11, 1, NODE_10, MR_R6, MR_C3)
	DISCRETE_74LS624( NODE_12, 1, NODE_11, VSS, MR_C6, DISC_LS624_OUT_ENERGY)
	DISCRETE_74LS624( NODE_13, 1, NODE_11, VSS, MR_C17, DISC_LS624_OUT_ENERGY)

	DISCRETE_LOGIC_XOR(NODE_14, 1, NODE_12, NODE_13)
	DISCRETE_LOGIC_AND(NODE_15, 1, NODE_14, NODE_10)
	DISCRETE_MULTIPLY(DS_OUT_SOUND0, 1, NODE_15, TTL_HIGH)

	/************************************************/
	/* SOUND1                                       */
	/************************************************/

	DISCRETE_LS123(NODE_20, DS_SOUND1_INV, MR_R18, MR_C15)
	DISCRETE_RCFILTER(NODE_21, 1, NODE_20, MR_R7, MR_C4)
	DISCRETE_74LS624( NODE_22, 1, NODE_21, VSS, MR_C5, DISC_LS624_OUT_ENERGY)
	DISCRETE_74LS624( NODE_23, 1, NODE_21, VSS, MR_C16, DISC_LS624_OUT_ENERGY)

	DISCRETE_LOGIC_XOR(NODE_24, 1, NODE_22, NODE_23)
	DISCRETE_LOGIC_AND(NODE_25, 1, NODE_24, NODE_20)
	DISCRETE_MULTIPLY(DS_OUT_SOUND1, 1, NODE_25, TTL_HIGH)

	/************************************************/
	/* SOUND7                                       */
	/************************************************/

	DISCRETE_COUNTER(NODE_100,1,0,NODE_118,0xFFFF,DISC_COUNT_UP,0,DISC_CLK_BY_COUNT)

	DISCRETE_BITSET(NODE_102, NODE_100, 4) 	//LS157 2B
	DISCRETE_BITSET(NODE_104, NODE_100, 12) //LS157 3B

	DISCRETE_LS123(NODE_110, DS_SOUND7_INV, MR_R61, MR_C41)
	DISCRETE_TRANSFORM2(NODE_111, TTL_HIGH, NODE_110, "01-")
	DISCRETE_RCFILTER(NODE_112, 1, NODE_111, MR_R65, MR_C44)
	DISCRETE_74LS624(NODE_113, 1, NODE_112, VSS, MR_C40, DISC_LS624_OUT_LOGIC)

	DISCRETE_LOGIC_XOR(NODE_115, 1, NODE_102, NODE_113)

	DISCRETE_TRANSFORM2(NODE_116, NODE_104, TTL_HIGH, "0!1*")
	DISCRETE_RCFILTER(NODE_117, 1, NODE_116, MR_R64, MR_C43)
	DISCRETE_74LS624(NODE_118, 1, NODE_117, VSS, MR_C39, DISC_LS624_OUT_COUNT_F)

	DISCRETE_LOGIC_AND(NODE_120, 1, NODE_115, NODE_110)
	DISCRETE_MULTIPLY(DS_OUT_SOUND7, 1, NODE_120, TTL_HIGH)

	/************************************************/
	/* DAC                                          */
	/************************************************/

	/* following the resistor DAC are two opamps. The first is a 1:1 amplifier, the second
     * is a filter circuit. Simulation in LTSPICE shows, that the following is equivalent:
     */

	DISCRETE_MULTIPLY(NODE_170, 1, DS_DAC, TTL_HIGH/256.0)
	DISCRETE_RCFILTER(DS_OUT_DAC, 1, NODE_170, RES_K(750), CAP_P(200))

	/************************************************/
	/* Amplifier                                    */
	/************************************************/

	DISCRETE_MIXER4(NODE_295, 1, DS_OUT_SOUND0, DS_OUT_SOUND1, DS_OUT_SOUND7, DS_OUT_DAC, &mario_mixer_desc)

	/* Amplifier: internal amplifier
     * Just a 1:n amplifier without filters and no output capacitor
     */
	// EZV20 equivalent filter circuit ...
	DISCRETE_CRFILTER(NODE_296,1,NODE_295, RES_K(1), CAP_U(4.7))
	DISCRETE_OUTPUT(NODE_296, 32767.0/5.0 * 3 )
	//DISCRETE_WAVELOG1(DS_OUT_DAC, 32767/5.0)

DISCRETE_SOUND_END

/****************************************************************
 *
 * EA / Banking
 *
 ****************************************************************/

static void set_ea(const address_space *space, int ea)
{
	mario_state	*state = space->machine->driver_data;
	//printf("ea: %d\n", ea);
	//cputag_set_input_line(machine, "audio", MCS48_INPUT_EA, (ea) ? ASSERT_LINE : CLEAR_LINE);
	if (state->eabank != 0)
		memory_set_bank(space->machine, state->eabank, ea);
}

/****************************************************************
 *
 * Initialization
 *
 ****************************************************************/

static SOUND_START( mario )
{
	mario_state	*state = machine->driver_data;
	const device_config *audiocpu = cputag_get_cpu(machine, "audio");
#if USE_8039
	UINT8 *SND = memory_region(machine, "audio");

	SND[0x1001] = 0x01;
#endif

	state->eabank = 0;
	if (audiocpu != NULL && ((const cpu_class_header *)audiocpu->classtoken)->cputype != CPU_Z80)
	{
		state->eabank = 1;
		memory_install_read8_handler(cpu_get_address_space(audiocpu, ADDRESS_SPACE_PROGRAM), 0x000, 0x7ff, 0, 0, SMH_BANK1);
		memory_configure_bank(machine, 1, 0, 1, memory_region(machine, "audio"), 0);
	    memory_configure_bank(machine, 1, 1, 1, memory_region(machine, "audio") + 0x1000, 0x800);
	}

    state_save_register_global(machine, state->last);
	state_save_register_global(machine, state->portT);
}

static SOUND_RESET( mario )
{
	mario_state	*state = machine->driver_data;
	const address_space *space = cpu_get_address_space(machine->cpu[1], ADDRESS_SPACE_PROGRAM);

#if USE_8039
    set_ea(machine, 1);
#endif

    /* FIXME: convert to latch8 */
	soundlatch_clear_w(space,0,0);
	soundlatch2_clear_w(space,0,0);
	soundlatch3_clear_w(space,0,0);
	soundlatch4_clear_w(space,0,0);
	I8035_P1_W(space,0x00); /* Input port */
	I8035_P2_W(space,0xff); /* Port is in high impedance state after reset */

	state->last = 0;
}

/****************************************************************
 *
 * I/O Handlers - static
 *
 ****************************************************************/

static READ8_HANDLER( mario_sh_p1_r )
{
	return I8035_P1_R(space);
}

static READ8_HANDLER( mario_sh_p2_r )
{
	return I8035_P2_R(space) & 0xEF; /* Bit 4 connected to GND! */
}

static READ8_HANDLER( mario_sh_t0_r )
{
	return I8035_T_R(space,0);
}

static READ8_HANDLER( mario_sh_t1_r )
{
	return I8035_T_R(space,1);
}

static READ8_HANDLER( mario_sh_tune_r )
{
	UINT8 *SND = memory_region(space->machine, "audio");
	UINT16 mask = memory_region_length(space->machine, "audio")-1;
	UINT8 p2 = I8035_P2_R(space);

	if ((p2 >> 7) & 1)
		return soundlatch_r(space,offset);
	else
		return (SND[(0x1000 + (p2 & 0x0f)*256+offset) & mask]);
}

static WRITE8_HANDLER( mario_sh_sound_w )
{
	discrete_sound_w(space,DS_DAC,data);
}

static WRITE8_HANDLER( mario_sh_p1_w )
{
	I8035_P1_W(space,data);
}

static WRITE8_HANDLER( mario_sh_p2_w )
{
	I8035_P2_W(space,data);
}

/****************************************************************
 *
 * I/O Handlers - global
 *
 ****************************************************************/

WRITE8_HANDLER( masao_sh_irqtrigger_w )
{
	mario_state	*state = space->machine->driver_data;

	if (state->last == 1 && data == 0)
	{
		/* setting bit 0 high then low triggers IRQ on the sound CPU */
		cputag_set_input_line_and_vector(space->machine, "audio",0,HOLD_LINE,0xff);
	}

	state->last = data;
}

WRITE8_HANDLER( mario_sh_tuneselect_w )
{
	soundlatch_w(space,offset,data);
}

/* Mario running sample */
WRITE8_HANDLER( mario_sh1_w )
{
	discrete_sound_w(space,DS_SOUND0_INP,data & 1);
}

/* Luigi running sample */
WRITE8_HANDLER( mario_sh2_w )
{
	discrete_sound_w(space,DS_SOUND1_INP,data & 1);
}

/* Misc samples */
WRITE8_HANDLER( mario_sh3_w )
{
	mario_state	*state = space->machine->driver_data;

	switch (offset)
	{
		case 0: /* death */
			if (data)
				cputag_set_input_line(space->machine, "audio",0,ASSERT_LINE);
			else
				cputag_set_input_line(space->machine, "audio",0,CLEAR_LINE);
			break;
		case 1: /* get coin */
			I8035_T_W_AH(space,0,data & 1);
			break;
		case 2: /* ice */
			I8035_T_W_AH(space,1,data & 1);
			break;
		case 3: /* crab */
			I8035_P1_W_AH(space,0,data & 1);
			break;
		case 4: /* turtle */
			I8035_P1_W_AH(space,1,data & 1);
			break;
		case 5: /* fly */
			I8035_P1_W_AH(space,2,data & 1);
			break;
		case 6: /* coin */
			I8035_P1_W_AH(space,3,data & 1);
			break;
		case 7: /* skid */
			discrete_sound_w(space,DS_SOUND7_INP,data & 1);
			break;
	}
}

/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( mario_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_ROMBANK(1) AM_REGION("audio", 0)
	AM_RANGE(0x0800, 0x0fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mario_sound_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0xff) AM_READWRITE(mario_sh_tune_r, mario_sh_sound_w)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READWRITE(mario_sh_p1_r, mario_sh_p1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(mario_sh_p2_r, mario_sh_p2_w)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(mario_sh_t0_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(mario_sh_t1_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( masao_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x4000, 0x4000) AM_READWRITE(ay8910_read_port_0_r, ay8910_write_port_0_w)
	AM_RANGE(0x6000, 0x6000) AM_WRITE(ay8910_control_port_0_w)
ADDRESS_MAP_END

/*************************************
 *
 *  Sound Interfaces
 *
 *************************************/

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	soundlatch_r,
	NULL,
	NULL,
	NULL
};


/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_DRIVER_START( mario_audio )

#if USE_8039
	MDRV_CPU_ADD("audio", I8039, I8035_CLOCK)         /* 730 kHz */
#else
	MDRV_CPU_ADD("audio", M58715, I8035_CLOCK)        /* 730 kHz */
#endif
	MDRV_CPU_PROGRAM_MAP(mario_sound_map, 0)
	MDRV_CPU_IO_MAP(mario_sound_io_map, 0)

	MDRV_SOUND_START(mario)
	MDRV_SOUND_RESET(mario)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(mario)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.5)

MACHINE_DRIVER_END

MACHINE_DRIVER_START( masao_audio )

	MDRV_CPU_ADD("audio", Z80,24576000/16)	/* ???? */
	MDRV_CPU_PROGRAM_MAP(masao_sound_map,0)

	MDRV_SOUND_START(mario)
	MDRV_SOUND_RESET(mario)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay", AY8910, 14318000/6)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

MACHINE_DRIVER_END

