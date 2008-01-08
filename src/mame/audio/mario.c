#include "driver.h"
#include "cpu/i8039/i8039.h"
#include "sound/dac.h"
#include "sound/ay8910.h"
#include "sound/discrete.h"

#include "includes/mario.h"

/****************************************************************
 *
 * Defines and Macros
 *
 ****************************************************************/

#define ACTIVELOW_PORT_BIT(P,A,D)   ((P & (~(1 << A))) | ((D ^ 1) << A))
#define ACTIVEHIGH_PORT_BIT(P,A,D)   ((P & (~(1 << A))) | (D << A))

#define I8035_T_R(N) ((soundlatch2_r(0) >> (N)) & 1)
#define I8035_T_W_AH(N,D) do { state->portT = ACTIVEHIGH_PORT_BIT(state->portT,N,D); soundlatch2_w(0, state->portT); } while (0)

#define I8035_P1_R() (soundlatch3_r(0))
#define I8035_P2_R() (soundlatch4_r(0))
#define I8035_P1_W(D) soundlatch3_w(0,D)
#define I8035_P2_W(D) soundlatch4_w(0,D)

#define I8035_P1_W_AH(B,D) I8035_P1_W(ACTIVEHIGH_PORT_BIT(I8035_P1_R(),B,(D)))
#define I8035_P2_W_AH(B,D) I8035_P2_W(ACTIVEHIGH_PORT_BIT(I8035_P2_R(),B,(D)))

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
 ****************************************************************/

#define MR_R6		RES_K(4.7)
#define MR_R7		RES_K(4.7)
#define MR_R17		RES_K(30)
#define MR_R18		RES_K(30)
#define MR_R61		RES_K(47)
#define MR_R64		RES_K(20)
#define MR_R65		RES_K(10)

#define MR_C3		CAP_U(10)
#define MR_C4		CAP_U(47)		
#define MR_C5		CAP_N(39)
#define MR_C6		CAP_N(3.9)
#define MR_C14		CAP_U(4.7)
#define MR_C15		CAP_U(4.7)
#define MR_C16		CAP_N(6.8)
#define MR_C17		CAP_N(22)
#define MR_C39		CAP_N(4.7)
#define MR_C40		CAP_N(22)
#define MR_C41		CAP_U(4.7)
#define MR_C43		CAP_U(3.3)
#define MR_C44		CAP_U(3.3)

/* KT = 0.25 for diode circuit, 0.33 else */

#define DISCRETE_LS123(_N, _T, _R, _C) \
	DISCRETE_ONESHOTR(_N, 0, _T, TTL_HIGH, (0.25 * (_R) * (_C) * (1.0+700./(_R))), DISC_ONESHOT_RETRIG | DISC_ONESHOT_REDGE)
#define DISCRETE_LS123_INV(_N, _T, _R, _C) \
	DISCRETE_ONESHOTR(_N, 0, _T, TTL_HIGH, (0.25 * (_R) * (_C) * (1.0+700./(_R))), DISC_ONESHOT_RETRIG | DISC_ONESHOT_REDGE | DISC_OUT_ACTIVE_LOW)

#define DISCRETE_BITSET(_N, _N1, _B) DISCRETE_TRANSFORM3(_N, 1, _N1, 1 << ((_B)-1), 0, "01&2>")
#define DISCRETE_ENERGY_NAND(_N, _E, _N1, _N2) DISCRETE_TRANSFORM3(_N, _E, _N1, _N2, 1, "201*-")

#define MR_R20		RES_K(22)
#define MR_R19		RES_K(22)

#define MR_R41		RES_K(100)
#define MR_R40		RES_K(22)

#define MR_C32		CAP_U(1)
#define MR_C31		CAP_U(0.022)

static const discrete_mixer_desc mario_mixer_desc =
	{DISC_MIXER_IS_RESISTOR,
		{MR_R20, MR_R19, MR_R41, MR_R40},
		{0,0,0,0,0},	// no variable resistors
		{0,0,0,0,0},  // no node capacitors
		0, RES_M(1), // Dummy,
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
	/* SOUND7                              */
	/************************************************/

	DISCRETE_COUNTER(NODE_100,1,0,NODE_118,0xFFFF,DISC_COUNT_UP,0,DISC_CLK_BY_COUNT)

	DISCRETE_BITSET(NODE_102, NODE_100, 4) 	//LS157 2B
	DISCRETE_BITSET(NODE_104, NODE_100, 12) //LS157 3B

	DISCRETE_LS123(NODE_110, DS_SOUND7_INV, MR_R61, MR_C41)
	DISCRETE_TRANSFORM2(NODE_111, 1, TTL_HIGH, NODE_110, "01-")
	DISCRETE_RCFILTER(NODE_112, 1, NODE_111, MR_R65, MR_C44)
	DISCRETE_74LS624(NODE_113, 1, NODE_112, VSS, MR_C40, DISC_LS624_OUT_LOGIC)

	DISCRETE_LOGIC_XOR(NODE_115, 1, NODE_102, NODE_113)

	DISCRETE_TRANSFORM2(NODE_116,1, NODE_104, TTL_HIGH, "0!1*")
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
	DISCRETE_OUTPUT(NODE_295, 32767.0/5.0 * 3 )
	DISCRETE_WAVELOG2(DS_SOUND7_INV, 32767/5, NODE_110, 32767/5)

DISCRETE_SOUND_END

/****************************************************************
 *
 * Initialization
 *
 ****************************************************************/

static SOUND_START( mario )
{
	mario_state	*state = machine->driver_data;

	state_save_register_global(state->last);
	state_save_register_global(state->portT);
	soundlatch_clear_w(0,0);
	soundlatch2_clear_w(0,0);
	soundlatch3_clear_w(0,0);
	soundlatch4_clear_w(0,0);
	/*
	 * The code below will play the correct start up sound
	 * However, it is not backed by hardware at all.
	 */
	//soundlatch_w(0,2);
}

static SOUND_RESET( mario )
{
	mario_state	*state = machine->driver_data;

	state->last = 0;
}

/****************************************************************
 *
 * I/O Handlers - static
 *
 ****************************************************************/

static READ8_HANDLER( mario_sh_p1_r )
{
	return I8035_P1_R();
}

static READ8_HANDLER( mario_sh_p2_r )
{
	return I8035_P2_R() & 0xE0; /* Bit 4 connected to GND! */
}

static READ8_HANDLER( mario_sh_t0_r )
{
	return I8035_T_R(0);
}

static READ8_HANDLER( mario_sh_t1_r )
{
	return I8035_T_R(1);
}

static READ8_HANDLER( mario_sh_tune_r )
{
	UINT8 *SND = memory_region(REGION_CPU2);
	UINT16 mask = memory_region_length(REGION_CPU2)-1;
	UINT8 p2 = I8035_P2_R();
	
	if ((p2 >> 7) & 1)
		return soundlatch_r(offset);
	else
		return (SND[(0x1000 + (p2 & 0x0f)*256+offset) & mask]);
}

static WRITE8_HANDLER( mario_sh_sound_w )
{
	discrete_sound_w(DS_DAC,data);
}

static WRITE8_HANDLER( mario_sh_p1_w )
{
	I8035_P1_W(data);
}

static WRITE8_HANDLER( mario_sh_p2_w )
{
	I8035_P2_W(data);
}

/****************************************************************
 *
 * I/O Handlers - global
 *
 ****************************************************************/

WRITE8_HANDLER( masao_sh_irqtrigger_w )
{
	mario_state	*state = Machine->driver_data;

	if (state->last == 1 && data == 0)
	{
		/* setting bit 0 high then low triggers IRQ on the sound CPU */
		cpunum_set_input_line_and_vector(1,0,HOLD_LINE,0xff);
	}

	state->last = data;
}

WRITE8_HANDLER( mario_sh_tuneselect_w )
{
	soundlatch_w(offset,data);
}

/* Mario running sample */
WRITE8_HANDLER( mario_sh1_w )
{
	discrete_sound_w(DS_SOUND0_INP,data & 1);
}

/* Luigi running sample */
WRITE8_HANDLER( mario_sh2_w )
{
	discrete_sound_w(DS_SOUND1_INP,data & 1);
}

/* Misc samples */
WRITE8_HANDLER( mario_sh3_w )
{
	mario_state	*state = Machine->driver_data;
	
	switch (offset)
	{
		case 0: /* death */
			if (data)
				cpunum_set_input_line(1,0,ASSERT_LINE);
			else
				cpunum_set_input_line(1,0,CLEAR_LINE);
			break;
		case 1: /* get coin */
			I8035_T_W_AH(0,data & 1);
			break;
		case 2: /* ice */
			I8035_T_W_AH(1,data & 1);
			break;
		case 3: /* crab */
			I8035_P1_W_AH(0,data & 1);
			break;
		case 4: /* turtle */
			I8035_P1_W_AH(1,data & 1);
			break;
		case 5: /* fly */
			I8035_P1_W_AH(2,data & 1);
			break;
		case 6: /* coin */
			I8035_P1_W_AH(3,data & 1);
			break;
		case 7: /* skid */
			discrete_sound_w(DS_SOUND7_INP,data & 1);
			break;
	}
}

/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( mario_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mario_sound_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0xff) AM_READWRITE(mario_sh_tune_r, mario_sh_sound_w)
	AM_RANGE(I8039_p1, I8039_p1) AM_READWRITE(mario_sh_p1_r, mario_sh_p1_w)
	AM_RANGE(I8039_p2, I8039_p2) AM_READWRITE(mario_sh_p2_r, mario_sh_p2_w)
	AM_RANGE(I8039_t0, I8039_t0) AM_READ(mario_sh_t0_r)
	AM_RANGE(I8039_t1, I8039_t1) AM_READ(mario_sh_t1_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( masao_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x4000, 0x4000) AM_READWRITE(AY8910_read_port_0_r, AY8910_write_port_0_w)
	AM_RANGE(0x6000, 0x6000) AM_WRITE(AY8910_control_port_0_w)
ADDRESS_MAP_END

/*************************************
 *
 *  Sound Interfaces
 *
 *************************************/

static const struct AY8910interface ay8910_interface =
{
	soundlatch_r
};


/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_DRIVER_START( mario_audio )

	MDRV_CPU_ADD(I8039, I8035_CLOCK)  /* audio CPU */         /* 730 kHz */
	MDRV_CPU_PROGRAM_MAP(mario_sound_map, 0)
	MDRV_CPU_IO_MAP(mario_sound_io_map, 0)

	MDRV_SOUND_START(mario)
	MDRV_SOUND_RESET(mario)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD_TAG("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(mario)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.5)

MACHINE_DRIVER_END

MACHINE_DRIVER_START( masao_audio )

	MDRV_CPU_ADD(Z80,24576000/16) /* audio CPU */	/* ???? */
	MDRV_CPU_PROGRAM_MAP(masao_sound_map,0)

	MDRV_SOUND_START(mario)
	MDRV_SOUND_RESET(mario)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 14318000/6)
	MDRV_SOUND_CONFIG(ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

MACHINE_DRIVER_END

