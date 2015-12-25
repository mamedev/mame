// license:BSD-3-Clause
// copyright-holders:Derrick Renaud
/************************************************************************
 * m79amb Sound System Analog emulation
 * Nov 2008, Derrick Renaud
 ************************************************************************/

#include "emu.h"
#include "includes/m79amb.h"
#include "sound/discrete.h"

#define TIME_OF_9602(r, c)              (0.34 * (r) * (c) * (1.0 + 1.0 / (r)))
#define TIME_OF_9602_WITH_DIODE(r, c)   (0.3  * (r) * (c))


/* Discrete Sound Input Nodes */
#define M79AMB_BOOM_EN                  NODE_01
#define M79AMB_THUD_EN                  NODE_02
#define M79AMB_SHOT_EN                  NODE_03
#define M79AMB_MC_REV_EN                NODE_04
#define M79AMB_MC_CONTROL_EN            NODE_05
#define M79AMB_TANK_TRUCK_JEEP_EN       NODE_06
#define M79AMB_WHISTLE_A_EN             NODE_07
#define M79AMB_WHISTLE_B_EN             NODE_08

/* Discrete Sound Output Nodes */
#define M79AMB_BOOM_SND                 NODE_11
#define M79AMB_THUD_SND                 NODE_12
#define M79AMB_SHOT_SND                 NODE_13
#define M79AMB_MC_SND                   NODE_14
#define M79AMB_TANK_TRUCK_JEEP_SND      NODE_15
#define M79AMB_WHISTLE_A_SND            NODE_16
#define M79AMB_WHISTLE_B_SND            NODE_17

/* Parts List - Resistors */
#define M79AMB_R2       RES_K(5.6)
#define M79AMB_R6       220
#define M79AMB_R9       RES_K(4.7)
#define M79AMB_R10      RES_K(2.2)
#define M79AMB_R12      RES_K(5.6)
#define M79AMB_R16      330
#define M79AMB_R19      RES_K(4.7)
#define M79AMB_R20      RES_K(2.2)
#define M79AMB_R22      RES_K(3.3)
#define M79AMB_R26      100
#define M79AMB_R29      RES_K(4.7)
#define M79AMB_R30      RES_K(1.5)
#define M79AMB_R35      RES_K(470)
#define M79AMB_R36      RES_K(39)
#define M79AMB_R37      RES_K(82)
#define M79AMB_R38      100
#define M79AMB_R39      RES_K(10)
#define M79AMB_R41      RES_K(2.2)
#define M79AMB_R42      RES_K(10)
#define M79AMB_R43      220
#define M79AMB_R44      RES_K(39)
#define M79AMB_R45      RES_K(82)
#define M79AMB_R46      RES_K(10)
#define M79AMB_R48      RES_K(2.2)
#define M79AMB_R49      RES_K(1)
#define M79AMB_R51      RES_K(39)
#define M79AMB_R52      RES_K(82)
#define M79AMB_R53      100
#define M79AMB_R54      RES_K(10)
#define M79AMB_R57      RES_K(2.2)
#define M79AMB_R58      RES_K(2.2)
#define M79AMB_R59      RES_K(1)
#define M79AMB_R61      RES_K(39)
#define M79AMB_R62      RES_K(82)
#define M79AMB_R63      100
#define M79AMB_R64      RES_K(10)
#define M79AMB_R67      RES_K(2.2)
#define M79AMB_R68      RES_K(2.2)
#define M79AMB_R69      RES_K(1)
#define M79AMB_R76      RES_K(2.7)
#define M79AMB_R77      RES_K(47)
#define M79AMB_R78      RES_K(47)
#define M79AMB_R79      RES_K(15)
#define M79AMB_R80      RES_K(22)
#define M79AMB_R81      RES_K(100)
#define M79AMB_R82      RES_K(100)
#define M79AMB_R83      RES_K(3.3)
#define M79AMB_R84      RES_K(50)
#define M79AMB_R86      470

/* Parts List - Capacitors */
#define M79AMB_C2       CAP_U(39)
#define M79AMB_C3       CAP_U(22)
#define M79AMB_C6       CAP_U(0.1)
#define M79AMB_C8       CAP_U(39)
#define M79AMB_C9       CAP_U(22)
#define M79AMB_C12      CAP_U(0.1)
#define M79AMB_C14      CAP_U(4.7)
#define M79AMB_C15      CAP_U(3.3)
#define M79AMB_C18      CAP_U(0.1)
#define M79AMB_C20      CAP_U(2.2)
#define M79AMB_C21      CAP_U(1)
#define M79AMB_C22      CAP_U(22)
#define M79AMB_C23      CAP_U(0.01)
#define M79AMB_C25      CAP_U(2.2)
#define M79AMB_C26      CAP_U(22)
#define M79AMB_C27      CAP_U(0.47)
#define M79AMB_C28      CAP_U(0.1)
#define M79AMB_C29      CAP_U(22)
#define M79AMB_C30      CAP_U(0.03)
#define M79AMB_C31      CAP_U(0.1)
#define M79AMB_C32      CAP_U(0.1)
#define M79AMB_C33      CAP_U(22)
#define M79AMB_C34      CAP_U(0.03)
#define M79AMB_C35      CAP_U(0.1)
#define M79AMB_C36      CAP_U(0.1)
#define M79AMB_C37      CAP_P(500)
#define M79AMB_C41      CAP_U(0.1)
#define M79AMB_C42      CAP_U(15)


static const discrete_mixer_desc m79amb_final_mix =
{
	DISC_MIXER_IS_OP_AMP,
	{
		M79AMB_R76 + M79AMB_R9,
		M79AMB_R77 + M79AMB_R19,
		M79AMB_R78 + M79AMB_R29,
		M79AMB_R79 + RES_2_PARALLEL(M79AMB_R41, M79AMB_R42 + M79AMB_R43),
		M79AMB_R80 + RES_2_PARALLEL(M79AMB_R48, M79AMB_R49),
		M79AMB_R81 + RES_2_PARALLEL(M79AMB_R59, M79AMB_R57 + M79AMB_R58),
		M79AMB_R82 + RES_2_PARALLEL(M79AMB_R69, M79AMB_R67 + M79AMB_R68)
	},
	{0},                /* no r_nodes */
	{M79AMB_C6, M79AMB_C12, M79AMB_C18, M79AMB_C23, M79AMB_C28, M79AMB_C32, M79AMB_C36},
	0,
	M79AMB_R83 + M79AMB_R84,
	M79AMB_C37,
	M79AMB_C42,
	0,
	1               /* gain */
};

DISCRETE_SOUND_START( m79amb )
	/************************************************
	 * Input register mapping
	 ************************************************/
	DISCRETE_INPUT_PULSE(M79AMB_BOOM_EN, 0)
	DISCRETE_INPUT_PULSE(M79AMB_THUD_EN, 0)
	DISCRETE_INPUT_PULSE(M79AMB_SHOT_EN, 0)
	DISCRETE_INPUT_LOGIC(M79AMB_MC_REV_EN)
	DISCRETE_INPUT_LOGIC(M79AMB_MC_CONTROL_EN)
	DISCRETE_INPUT_LOGIC(M79AMB_TANK_TRUCK_JEEP_EN)
	DISCRETE_INPUT_LOGIC(M79AMB_WHISTLE_A_EN)
	DISCRETE_INPUT_LOGIC(M79AMB_WHISTLE_B_EN)

	/* Boom, Thud, Shot sounds need more accurate emulation */

	/************************************************
	 * Boom
	 ************************************************/
	DISCRETE_ONESHOT(NODE_20,
				M79AMB_BOOM_EN,             /* TRIG */
				1,                          /* AMPL */
				TIME_OF_9602_WITH_DIODE(M79AMB_R2, M79AMB_C2),
				DISC_ONESHOT_REDGE | DISC_ONESHOT_RETRIG | DISC_OUT_ACTIVE_HIGH)
	DISCRETE_RCDISC2(NODE_21,
				NODE_20,                    /* Q1 base */
				0,                          /* Q1 off, C3 discharges */
				M79AMB_R9 + M79AMB_R10,     /* discharges through amp/filter circuit */
				12,                         /* Q1 on, C3 charges */
				M79AMB_R6,                  /* Q2 on  */
				M79AMB_C3)                  /* controls amplitude */
	DISCRETE_NOISE(M79AMB_BOOM_SND,
				1,                          /* ENAB */
				800,                        /* FREQ - Guess*/
				NODE_21,                    /* AMP  */
				0)                          /* BIAS - fake AC is fine*/

	/************************************************
	 * Thud
	 ************************************************/
	DISCRETE_ONESHOT(NODE_30,
				M79AMB_THUD_EN,         /* TRIG */
				1,                      /* AMPL */
				TIME_OF_9602_WITH_DIODE(M79AMB_R12, M79AMB_C8),
				DISC_ONESHOT_REDGE | DISC_ONESHOT_RETRIG | DISC_OUT_ACTIVE_HIGH)
	DISCRETE_RCDISC2(NODE_31,
				NODE_30,                    /* Q4 base */
				0,                          /* Q4 off, C9 discharges */
				M79AMB_R19 + M79AMB_R20,    /* discharges through amp/filter circuit */
				12,                         /* Q4 on, C9 charges */
				M79AMB_R16,                 /* Q5 on  */
				M79AMB_C9)                  /* controls amplitude */
	DISCRETE_NOISE(M79AMB_THUD_SND,
				1,                          /* ENAB */
				500,                        /* FREQ - Guess*/
				NODE_31,                    /* AMP  */
				0)                          /* BIAS - fake AC is fine*/

	/************************************************
	 * Shot
	 ************************************************/
	DISCRETE_ONESHOT(NODE_40,
				M79AMB_SHOT_EN,         /* TRIG */
				1,                      /* AMPL */
				TIME_OF_9602_WITH_DIODE(M79AMB_R22, M79AMB_C14),
				DISC_ONESHOT_REDGE | DISC_ONESHOT_RETRIG | DISC_OUT_ACTIVE_HIGH)
	DISCRETE_RCDISC2(NODE_41,
				NODE_40,                    /* Q7 base */
				0,                          /* Q7 off, C15 discharges */
				M79AMB_R29 + M79AMB_R30,    /* discharges through amp/filter circuit */
				12,                         /* Q7 on, C15 charges */
				M79AMB_R26,                 /* Q8 on  */
				M79AMB_C15)                 /* controls amplitude */
	DISCRETE_NOISE(M79AMB_SHOT_SND,
				1,                          /* ENAB */
				1000,                       /* FREQ - Guess*/
				NODE_41,                    /* AMP  */
				0)                          /* BIAS - fake AC is fine*/

	/************************************************
	 * MC
	 ************************************************/
	/* not the best implementation of the pin 5 charge circuit, but it is within tolerance */
	DISCRETE_RCDISC2(NODE_50,
				M79AMB_MC_REV_EN,
				/* R35 can be ignored on discharge */
				RES_VOLTAGE_DIVIDER(M79AMB_R36 + M79AMB_R37, M79AMB_R38) * 12,  /* Q12 on  */
				RES_2_PARALLEL(M79AMB_R36 + M79AMB_R37, M79AMB_R38),            /* Q12 on  */
				12.0 * RES_VOLTAGE_DIVIDER(M79AMB_R36, M79AMB_R35),             /* Q12 off */
				RES_2_PARALLEL(M79AMB_R36, M79AMB_R35) + M79AMB_R37,            /* Q12 off */
				M79AMB_C20)
	/* cap charge to B+ ratio changes voltage on pin 5 */
	/* (iR36 + iR35 + iR37) * R36||R35||R37 where iR35 = 0/R35 = 0 */
	DISCRETE_TRANSFORM4(NODE_51, 12.0 / M79AMB_R36, NODE_50, M79AMB_R37, RES_3_PARALLEL(M79AMB_R36, M79AMB_R35, M79AMB_R37), "012/+3*")
	DISCRETE_566(NODE_52,                   /* IC U3, pin 4 */
				NODE_51,                    /* IC U3, pin 5 */
				M79AMB_R39, M79AMB_C21,
				12, 0, 12,                  /* VPOS,VNEG,VCHARGE */
				DISC_566_OUT_DC | DISC_566_OUT_TRIANGLE)
	DISCRETE_CRFILTER(NODE_53,
				NODE_52, M79AMB_R41 + M79AMB_R42 + M79AMB_R43, M79AMB_C22)
	DISCRETE_MULTIPLY(NODE_54, NODE_53, M79AMB_MC_CONTROL_EN)
	DISCRETE_GAIN(M79AMB_MC_SND, NODE_54, RES_VOLTAGE_DIVIDER(M79AMB_R41 + M79AMB_R42, 2200))//M79AMB_R43))

	/************************************************
	 * Tank, Truck, Jeep
	 ************************************************/
	DISCRETE_566(NODE_60,                       /* IC U4, pin 4 */
				12.0 * RES_VOLTAGE_DIVIDER(M79AMB_R44, M79AMB_R45),     /* IC U5, pin 5 */
				M79AMB_R46, M79AMB_C25,
				12, 0, 12,                  /* VPOS,VNEG,VCHARGE */
				DISC_566_OUT_DC | DISC_566_OUT_TRIANGLE)
	DISCRETE_ONOFF(NODE_61,
				M79AMB_TANK_TRUCK_JEEP_EN,      /* Q16, Q17 */
				NODE_60)
	DISCRETE_CRFILTER(NODE_62,
				NODE_61, M79AMB_R48 + M79AMB_R49, M79AMB_C26)
	DISCRETE_GAIN(NODE_63, NODE_62, RES_VOLTAGE_DIVIDER(M79AMB_R48, M79AMB_R49))
	DISCRETE_RCFILTER(M79AMB_TANK_TRUCK_JEEP_SND,
				NODE_63, RES_2_PARALLEL(M79AMB_R48, M79AMB_R49), M79AMB_C27)

	/************************************************
	 * Whisle A
	 ************************************************/
	DISCRETE_RCDISC2(NODE_70,
				M79AMB_WHISTLE_A_EN,
				RES_VOLTAGE_DIVIDER(M79AMB_R51 + M79AMB_R52, M79AMB_R53) * 12,  /* Q15 on  */
				RES_2_PARALLEL(M79AMB_R53, M79AMB_R51 + M79AMB_R52),            /* Q15 on  */
				12, M79AMB_R51 + M79AMB_R52,                                    /* Q15 off */
				M79AMB_C29)
	/* cap charge to B+ ratio changes voltage on pin 5 */
	DISCRETE_TRANSFORM3(NODE_71, 12, NODE_70, RES_VOLTAGE_DIVIDER(M79AMB_R51, M79AMB_R52), "01-2*1+")
	DISCRETE_566(NODE_72,                   /* IC U5, pin 4 */
				NODE_71,                    /* IC U5, pin 5 */
				M79AMB_R54, M79AMB_C30,
				12, 0, 12,                  /* VPOS,VNEG,VCHARGE */
				DISC_566_OUT_DC | DISC_566_OUT_TRIANGLE)
	DISCRETE_CRFILTER(NODE_73,
				NODE_72, M79AMB_R57 + M79AMB_R58 + M79AMB_R59, M79AMB_C31)
	DISCRETE_MULTIPLY(NODE_74, NODE_73, M79AMB_WHISTLE_A_EN) /* Q16, Q17 */
	DISCRETE_GAIN(M79AMB_WHISTLE_A_SND, NODE_74, RES_VOLTAGE_DIVIDER(M79AMB_R57 + M79AMB_R58, M79AMB_R59))

	/************************************************
	 * Whisle B
	 ************************************************/
	DISCRETE_RCDISC2(NODE_80,
				M79AMB_WHISTLE_B_EN,
				RES_VOLTAGE_DIVIDER(M79AMB_R61 + M79AMB_R62, M79AMB_R63) * 12,  /* Q18 on  */
				RES_2_PARALLEL(M79AMB_R63, M79AMB_R61 + M79AMB_R62),            /* Q18 on  */
				12, M79AMB_R61 + M79AMB_R62,                                    /* Q18 off */
				M79AMB_C33)
	/* cap charge to B+ ratio changes voltage on pin 5 */
	DISCRETE_TRANSFORM3(NODE_81, 12, NODE_80, RES_VOLTAGE_DIVIDER(M79AMB_R61, M79AMB_R62), "01-2*1+")
	DISCRETE_566(NODE_82,                   /* IC U5, pin 4 */
				NODE_81,                    /* IC U5, pin 5 */
				M79AMB_R64, M79AMB_C34,
				12, 0, 12,                  /* VPOS,VNEG,VCHARGE */
				DISC_566_OUT_DC | DISC_566_OUT_TRIANGLE)
	DISCRETE_CRFILTER(NODE_83,
				NODE_82, M79AMB_R67 + M79AMB_R68 + M79AMB_R69, M79AMB_C35)
	DISCRETE_MULTIPLY(NODE_84, NODE_83, M79AMB_WHISTLE_B_EN) /* Q19, Q20*/
	DISCRETE_GAIN(M79AMB_WHISTLE_B_SND, NODE_84, RES_VOLTAGE_DIVIDER(M79AMB_R67 + M79AMB_R68, M79AMB_R69))

	/************************************************
	 * Mixer
	 ************************************************/
	DISCRETE_MIXER7(NODE_90,                /* IC U7, pin 6 */
				1,                          /* ENAB */
				M79AMB_BOOM_SND,
				M79AMB_THUD_SND,
				M79AMB_SHOT_SND,
				M79AMB_MC_SND,
				M79AMB_TANK_TRUCK_JEEP_SND,
				M79AMB_WHISTLE_A_SND,
				M79AMB_WHISTLE_B_SND,
				&m79amb_final_mix)
	DISCRETE_RCFILTER(NODE_91,
				NODE_90, M79AMB_R86, M79AMB_C41)

	DISCRETE_OUTPUT(NODE_91, 32000.0/5)

DISCRETE_SOUND_END


/* the ports are guessed from operation */
/* the schematics do not show the actual hookup */

WRITE8_MEMBER(m79amb_state::m79amb_8000_w)
{
	/* these values are not latched */
	/* they are pulsed when the port is addressed */
	/* the discrete system will just trigger from them */
	m_discrete->write(space, M79AMB_SHOT_EN, data & 0x01);
	m_discrete->write(space, M79AMB_BOOM_EN, data & 0x02);
	m_discrete->write(space, M79AMB_THUD_EN, data & 0x04);
}

WRITE8_MEMBER(m79amb_state::m79amb_8003_w)
{
	/* Self Test goes low on reset and lights LED */
	/* LED goes off on pass */
	output_set_value("SELF_TEST", data & 0x01);
	m_discrete->write(space, M79AMB_MC_REV_EN, data & 0x02);
	m_discrete->write(space, M79AMB_MC_CONTROL_EN, data & 0x04);
	m_discrete->write(space, M79AMB_TANK_TRUCK_JEEP_EN, data & 0x08);
	m_discrete->write(space, M79AMB_WHISTLE_B_EN, data & 0x10);
	m_discrete->write(space, M79AMB_WHISTLE_A_EN, data & 0x20);
}
