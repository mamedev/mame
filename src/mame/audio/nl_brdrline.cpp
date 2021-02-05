// license:CC0
// copyright-holders: beta-tester (https://github.com/beta-tester)

//NL_CONTAINS brdrline

//
// Netlist for Borderline (brdrline)
//
// Derived from the schematics in the Borderline Quest Owner's Manual.
//
// Known problems/issues:
//
//  * all transistors in schematics of sound genertors were drawn
//      in a wrong orientation.
//      base and collectors were swaped
//
//  * MB4391 is missing. a fake substitution is used
//
//  * it takes up to 30seconds to charge C24 to its working point
//      in ANIMAL_SOUND
//
//  * noise generation with MM5837_DIP eats a lot of CPU power
//


// ---------------------------------------------------------------------------
// Borderline
// Owner's Manual
// Sega/Gremlin, Manual Part No. 420-0613
//
// ...........................................................................
// Page 26
// Sound Board Assembly
// Drawing No. 834-0055, Rev. A, Sheet 6 of 7
// () = location in schematics
// ...........................................................................
// GUN_TRG (C8);    GUN_SOUND (D6);   (C8,D8,C7,D7,C6,D6)
// JEEP_ON (C8);    JEEP_SOUND (C6);  (C8,B8,C7,B7,C6,B6)
// POINT_TRG (A8);  POINT_SOUND (B3); (A8,B8,A7,B7,A6,B6,A5,B5,C5,D5,A4,B4,C5,
//                                        D4,B3)
// HIT_TRG (A8);    HIT_SOUND (A4);   (A8,A7,A6,A5,A4,B4,C4,D4,A3,B4,C3,D3)
//
// ...........................................................................
// Page 27
// Sound Board Assembly
// Drawing No. 834-0055, Rev. A, Sheet 7 of 7
// () = location in schematics
// ...........................................................................
// ANIMAL_TRG (D8); ANIMAL_SOUND (C6); (D8,C8,B8,A8,D7,C7,B7,A8,D6,C6,D5,C5)
// EMAR_TRG (A8);   EMAR_SOUND (C5);   (A8,A7,B7,C6,B6,B5,C5,B4,C4,D4,D3)
// WALK_TRG (A8);   WALK_SOUND (B5);   (A8,A7,A6,B6,B5)
// CRY_TRG (A8);    CRY_SOUND (C1);    (A8,A7,A6,A5,A4,A3,B3,C3,A2,B2,C2,A1,
//                                         B1,C1)
// ---------------------------------------------------------------------------


// dry run:
// X=SOUND_OUT; ./nltool -q -c run --progress -t 50 -n brdrline -l $X -i ./src/mame/audio/nl_brdrline.csv ./src/mame/audio/nl_brdrline.cpp && ./nlwav -f tab -i 0.001 -s 1 -n -1 log_$X.log | gnuplot -p -e "reset; plot '-' using 1 with lines"
// X=SOUND_OUT; ./nltool -v -s -c run --progress -t 50 -n brdrline -l $X -i ./src/mame/audio/nl_brdrline.csv ./src/mame/audio/nl_brdrline.cpp | tee ./nl_brdrline.txt && ./nlwav -f tab -i 0.001 -s 1 -n -1 log_$X.log | gnuplot -p -e "reset; plot '-' using 1 with lines"
//
// X=SOUND_OUT; ./nlwav -f wav32f -a 0.5 -o ../$X.wav log_$X.log


#include "netlist/devices/net_lib.h"

#define ENABLE_FRONTIERS (1)


/* ----------------------------------------------------------------------------
 *  Library section content START
 * ---------------------------------------------------------------------------*/
#if 0
//
// WARNING: fake implementation for MB4391 based on guesses only
// MB4391_DIP
// MB4391(A)
//            +--------------+
//  .1/*IN1*/ |1     ++    16| .16/*VCC1*/
// .2/*CON1*/ |2           15| .15/*OUT1*/
// .3/*GND1*/ |3           14| .14/*RO1*/
//            |4   MB4391  13|
//  .5/*IN2*/ |5           12| .12/*VCC2*/
// .6/*CON2*/ |6           11| .11/*OUT2*/
// .7/*GND2*/ |7           10| .10/*RO2*/
//            |8            9|
//            +--------------+
/*
 * VCC -----+-----------+-------------- VCC
 *          |           |
 *         +-+          |
 *         |R|          |
 *         |1|         2|
 *         +-+  +-------+-+
 *          |  0|  AFUNC  |Q  +----+
 *  IN -----+---+         +---|Rout|--- OUT
 *          |   |   fx    |   +----+
 *         +-+  +-+--+--+-+
 *         |R|   1| 4| 3|
 *         |2|    |  |  |
 *         +-+    |  |  |
 *          |     |  |  |
 * GND -----+-----|--|--+-------------- GND
 *                |  |
 * CON -----------+  |
 *                   |
 *  RO --------------+
 *
 */
#endif
static NETLIST_START(_MB4391)
	// MB4391 (fake implementation)
	// 2020 by beta-tester (https://github.com/beta-tester)
	//
	// values by guesses
	// for brdrline schematics only
	//
	// Vmix  = -(Vcon - 4.759384) / (4.759384 - 2.839579)
	// Vmix2 = pow(Vmix, 2)
	// Vout  = Vin * Vmix2
	//
	// Vcon(max) = 4.759384
	// Vcon(min) = 2.839584...2.839579 (@3.002360417e+01)(@3.152360417e+01)
	//AFUNC(fx, 5, "(A1)")
	//
	//AFUNC(fx, 5, "min(1, max(0, -(A1 - 4.759384) / (4.759384 - 2.839579)))")
	//AFUNC(fx, 5, "min(1, max(0, -(A1 - (A2-0.24)) / ((A2-0.24) - (A2/2+0.34))))")
	//
	// Vin(offset)=2.5 (VCC/2)
	//AFUNC(fx, 5, "(A0)")
	//AFUNC(fx, 5, "(A0-2.5)")
	//AFUNC(fx, 5, "(A0-(A2/2))")
	//
	// vin * mix
	//AFUNC(fx, 5, "(A0-(A2/2)) * (min(1, max(0, -(A1 - 4.759384) / (4.759384 - 2.839579))))")
	//
	// vin * pow(mix, 2)
	AFUNC(fx, 5, "(A0-(A2/2)) * pow((min(1, max(0, -(A1 - 4.759384) / (4.759384 - 2.839579))), 2))")

	RES(R1, RES_K(10))
	RES(R2, RES_K(10))
	NET_C(VCC, R1.1)
	NET_C(IN,  R1.2, R2.1)
	NET_C(R2.2, GND)

	RES(Rout, RES_K(1))
	NET_C(fx.Q, Rout.1) // fx(IN, CON, VCC, GND, RO) = OUT

	// INPUT
	ALIAS(IN,  fx.A0) // IN
	ALIAS(CON, fx.A1) // CON
	ALIAS(VCC, fx.A2) // VCC
	ALIAS(GND, fx.A3) // GND
	ALIAS(RO,  fx.A4) // RO

	// OUTPUT
	ALIAS(OUT, Rout.2)  // OUT
NETLIST_END()


static NETLIST_START(_MB4391_DIP)
	SUBMODEL(_MB4391, A)
	SUBMODEL(_MB4391, B)

	NC_PIN(NC)

	DIPPINS(    /*      +--------------+       */
		  A.IN, /*  IN1 |1     ++    16| VCC1  */ A.VCC,
		 A.CON, /* CON1 |2           15| OUT1  */ A.OUT,
		 A.GND, /* GND1 |3           14| RO1   */ A.RO,
		  NC.I, /*      |4   MB4391  13|       */ NC.I,
		  B.IN, /*  IN2 |5           12| VCC2  */ B.VCC,
		 B.CON, /* CON2 |6           11| OUT2  */ B.OUT,
		 B.GND, /* GND2 |7           10| RO2   */ B.RO,
		  NC.I, /*      |8            9|       */ NC.I
				/*      +--------------+       */
	)
NETLIST_END()
/* ----------------------------------------------------------------------------
 *  Library section content END
 * ---------------------------------------------------------------------------*/




/*
 * hack
 *
 * terminates unused parts of a DIP
 * WARNING: eats CPU time
 *
 * REQUIREMENT:
 * - definition of a voltage splitter and a given alias UNUSED_OP_REF as
 */
#define UNUSED_TTL_Q(_name) HINT(_name, NC)
#define UNUSED_TTL_I(_name) NET_C(_name, GND)
#define UNUSED_OP_OUT_MINUS(_name_out, _name_minus) \
			NET_C(_name_out, _name_minus)
#define UNUSED_OP_PLUS(_name_plus) \
			NET_C(UNUSED_OP_REF, _name_plus)

/*
 * hack
 */
#define NC_(_name) HINT(_name, NC)
#define GND_(_name) NET_C(_name, GND)
#define TMP_(_name) NET_C(_name, GND)

/*
 * hack
 */
// diode MA150 (nothing special, only for quick discharging caps)
#define MA150(_name) \
			DIODE(_name, "D")

// npn transistor 2SC458
#define C458(_name) \
			QBJT_EB(_name, "NPN")

// MB4391 one half of
//#define MB4391(_name)  NET_REGISTER_DEV_X(MB4391, _name)
#define MB4391(_name) \
			SUBMODEL(_MB4391, _name)

// MB4391 full dip
//#define MB4391_DIP(_name)  NET_REGISTER_DEV_X(MB4391_DIP, _name)
#define MB4391_DIP(_name) \
			SUBMODEL(_MB4391_DIP, _name)




// ---------------------------------------------------------------------------
// Borderline
// Owner's Manual
// Sega/Gremlin, Manual Part No. 420-0613
//
// ...........................................................................
// Page 26
// Sound Board Assembly
// Drawing No. 834-0055, Rev. A, Sheet 6 of 7
// () = location in schematics
// ...........................................................................
// GUN_TRG (C8);    GUN_SOUND (D6);   (C8,D8,C7,D7,C6,D6)
// JEEP_ON (C8);    JEEP_SOUND (C6);  (C8,B8,C7,B7,C6,B6)
// POINT_TRG (A8);  POINT_SOUND (B3); (A8,B8,A7,B7,A6,B6,A5,B5,C5,D5,A4,B4,C5,
//                                        D4,B3)
// HIT_TRG (A8);    HIT_SOUND (A4);   (A8,A7,A6,A5,A4,B4,C4,D4,A3,B4,C3,D3)
//
// ...........................................................................
// Page 27
// Sound Board Assembly
// Drawing No. 834-0055, Rev. A, Sheet 7 of 7
// () = location in schematics
// ...........................................................................
// ANIMAL_TRG (D8); ANIMAL_SOUND (C6); (D8,C8,B8,A8,D7,C7,B7,A8,D6,C6,D5,C5)
// EMAR_TRG (A8);   EMAR_SOUND (C5);   (A8,A7,B7,C6,B6,B5,C5,B4,C4,D4,D3)
// WALK_TRG (A8);   WALK_SOUND (B5);   (A8,A7,A6,B6,B5)
// CRY_TRG (A8);    CRY_SOUND (C1);    (A8,A7,A6,A5,A4,A3,B3,C3,A2,B2,C2,A1,
//                                         B1,C1)
// ---------------------------------------------------------------------------
static NETLIST_START(brdrline_schematics)

	/* ------------------------------------------------------------------------
	 *  UNUSED_OP_REF
	 * -----------------------------------------------------------------------*/
	// UNUSED_OP_REF
	POT(UNUSED_OP_RES, RES_K(20))
	PARAM(UNUSED_OP_RES.DIAL, 0.5)
	NET_C(V12, UNUSED_OP_RES.1)
	NET_C(GND, UNUSED_OP_RES.3)
	ALIAS(UNUSED_OP_REF, UNUSED_OP_RES.2)


	/* ------------------------------------------------------------------------
	 *  RST (RESET for sound)
	 *  t(0s) = lo, t(2s) = high, t(infinite) = high
	 * -----------------------------------------------------------------------*/
	RES(R171, RES_K(47))
	CAP(C78, CAP_U(47))

	TTL_74123_DIP(IC37) // shared by GUN_SOUND, RST
	NET_C(GND, IC37.8/*GND*/)
	NET_C(V5,  IC37.16/*VCC*/)

	NET_C(GND, IC37.9/*A2*/)
	NET_C(V5,  IC37.10/*B2*/, IC37.11/*CLRQ2*/, R171.1)
	NET_C(IC37.6/*C2*/, C78.1)
	NET_C(IC37.7/*RC2*/, C78.2, R171.2)

	// --------------------------------
	// UNUSED PINS
	NC_(IC37.5/*Q2*/)

	// --------------------------------
	// OUTPUT
	ALIAS(RST, IC37.12/*QQ2*/)


	/* ------------------------------------------------------------------------
	 *  GUN_SOUND
	 * -----------------------------------------------------------------------*/
	// INPUT
	NET_C(GUN_TRG, R156.2)

	// --------------------------------
	// TRG -> CON
	MA150(D1)
	RES(R156, RES_K(1))
	RES(R155, RES_K(47))
	RES(R89, 470)
	RES(R90, RES_K(470))
	RES(R91, RES_K(470))
	CAP(C73, CAP_U(1))
	CAP(C45, CAP_U(1))
	NET_C(GND, C45.2)
	NET_C(V5, IC37.2/*B1*/, IC37.3/*CLRQ1*/
			, R156.1, R155.1, R91.1)
	NET_C(R156.2, IC37.1/*A1*/)
	NET_C(IC37.14/*C1*/, C73.1)
	NET_C(IC37.15/*RC1*/, C73.2, R155.2)
	NET_C(IC37.4/*QQ1*/, D1.K)
	NET_C(D1.A, R89.1)
	NET_C(R89.2, C45.1, R90.2)
	NET_C(R91.2, R90.1, IC27.12/*PLUS4*/)
	NET_C(IC27.14/*OUT4*/, IC27.13/*MINUS4*/, IC20.2/*CON1*/)

	// --------------------------------
	// NOISE
	RES(R132, RES_K(220))
	RES(R133, RES_K(22))
	RES(R134, RES_K(15))
	RES(R93, RES_K(15))
	RES(R92, RES_K(150))
	RES(R88, RES_K(100))
	CAP(C62, CAP_U(2.2))
	CAP(C63, CAP_U(0.01))
	CAP(C48, CAP_U(0.01))

	MM5837(IC26)
	PARAM(IC26.FREQ, 24000)
	NET_C(GND, IC26.VDD/*1*/, IC26.VGG/*2*/)
	NET_C(V12, IC26.VSS/*4*/) // left out decoupling caps (0.1uF & 22uF parallel to GND)

	LM324_DIP(IC27) // shared by GUN_SOUND, _, GUN_SOUND, GUN_SOUND
	NET_C(GND, IC27.11/*GND*/)
	NET_C(V12, IC27.4/*VCC*/)

	NET_C(IC26.OUT/*3*/, C62.1)
	NET_C(C62.2, R132.1)
	NET_C(R132.2, IC27.2/*MINUS1*/, R133.1)
	NET_C(IC27.3/*PLUS1*/, V6)
	NET_C(IC27.1/*OUT1*/, R133.2, R134.1)
	NET_C(R134.2, R93.1, C63.1)
	NET_C(R93.2, C48.1, IC27.10/*PLUS3*/)
	NET_C(C48.2, GND)
	NET_C(IC27.8/*OUT3*/, C63.2, R92.1, C46.1)
	NET_C(IC27.9/*MINUS3*/, R92.2, R88.1)
	NET_C(R88.2, V6)

	// --------------------------------
	// CON, MIX -> OUT
	MB4391_DIP(IC20) // shared by GUN_SOUND, CRY_SOUND
	NET_C(GND, IC20.3/*GND1*/, IC20.7/*GND2*/)
	NET_C(V5, IC20.16/*VCC1*/, IC20.12/*VCC2*/)
	CAP(C46, CAP_U(2.2))
	CAP(C25, CAP_U(2.2))
	CAP(C26, CAP_P(680))
	NET_C(C26.2, GND)
	NET_C(IC20.14/*RO1*/, C26.1)
	NET_C(IC20.1/*IN1*/, C46.2)
	NET_C(IC20.15/*OUT1*/, C25.1)

	// --------------------------------
	// UNUSED PINS
	NC_(IC37.13/*Q1*/)

	UNUSED_OP_OUT_MINUS(IC27.7/*OUT2*/, IC27.6/*MINUS2*/)
	UNUSED_OP_PLUS(IC27.5/*PLUS2*/)

	// --------------------------------
	// OUTPUT
	NET_C(GUN_SOUND, C25.2)


	/* ------------------------------------------------------------------------
	 *  JEEP_SOUND
	 * -----------------------------------------------------------------------*/
	// INPUT
	NET_C(JEEP_ON, R151.2)

	// --------------------------------
	TTL_7408_DIP(IC29)    // shared by _, JEEP_SOUND, _, _
	NET_C(GND, IC29.7/*GND*/)
	NET_C(V5, IC29.14/*VCC*/)

	TTL_7416_DIP(IC36)   // shared by _, _, JEEP_SOUND, _, _, _
	NET_C(GND, IC36.7/*GND*/)
	NET_C(V5, IC36.14/*VCC*/)

	TTL_7474_DIP(IC30)
	NET_C(GND, IC30.7/*GND*/)
	NET_C(V5, IC30.14/*VCC*/)

	TTL_7474_DIP(IC32)
	NET_C(GND, IC32.7/*GND*/)
	NET_C(V5, IC32.14/*VCC*/)

	NE555(IC31)
	NET_C(GND, IC31.GND/*1*/)
	NET_C(V5, IC31.VCC/*8*/)

	RES(R123, RES_K(51))
	RES(R150, RES_K(1))
	RES(R151, RES_K(1))
	RES(R152, RES_K(1))
	RES(R153, RES_K(33))
	RES(R154, RES_K(1))
	RES(R157, RES_K(51))
	RES(R158, RES_K(51))
	CAP(C58, CAP_U(0.047))
	CAP(C59, CAP_U(2.2))
	CAP(C72, CAP_U(0.1))

	NET_C(GND, C72.2, C58.2)
	NET_C(V5, R150.1, R152.1, R154.1, R151.1
			, IC30.1/*CLR1*/, IC30.4/*PR1*/, IC30.10/*PR2*/, IC30.13/*CLR2*/
			, IC32.1/*CLR1*/, IC32.4/*PR1*/, IC32.10/*PR2*/, IC32.13/*CLR2*/
			)
	NET_C(IC36.5/*A3*/, R151.2)
	NET_C(IC36.6/*Q3*/, R152.2
					  , IC31.RESET/*4*/
					  )
	NET_C(IC31.DISCH/*7*/, R150.2, R153.1)
	NET_C(IC31.THRESH/*6*/, IC31.TRIG/*2*/, R153.2, C72.1)
	NET_C(IC31.OUT/*3*/, R154.2
				  , IC32.3/*CLK1*/, IC30.3/*CLK1*/, IC30.11/*CLK2*/
				  )
	NET_C(IC32.6/*QQ1*/, R157.1
					   , IC32.2/*D1*/, IC32.11/*CLK2*/
					   )
	NET_C(IC32.8/*QQ2*/, IC32.12/*D2*/, R158.1)
	NET_C(IC29.6/*Q2*/, IC30.2/*D1*/)
	NET_C(IC30.5/*Q1*/, IC30.12/*D2*/)
	NET_C(IC30.6/*QQ1*/, IC29.4/*A2*/)
	NET_C(IC30.8/*QQ2*/, IC29.5/*B2*/, R123.1)
	NET_C(R123.2, R157.2, R158.2, C58.1, C59.1)

	// --------------------------------
	// UNUSED PINS
	NC_(IC30.9/*Q2*/)

	NC_(IC31.CONT/*5*/)

	NC_(IC32.5/*Q1*/)
	NC_(IC32.9/*Q2*/)

	UNUSED_TTL_I(IC29.1/*A1*/)
	UNUSED_TTL_I(IC29.2/*B1*/)
	UNUSED_TTL_Q(IC29.3/*Q1*/)
	UNUSED_TTL_Q(IC29.8/*Q3*/)
	UNUSED_TTL_I(IC29.9/*A3*/)
	UNUSED_TTL_I(IC29.10/*B3*/)
	UNUSED_TTL_Q(IC29.11/*Q4*/)
	UNUSED_TTL_I(IC29.12/*A4*/)
	UNUSED_TTL_I(IC29.13/*B4*/)

	UNUSED_TTL_I(IC36.1/*A1*/)
	UNUSED_TTL_Q(IC36.2/*Q1*/)
	UNUSED_TTL_I(IC36.3/*A2*/)
	UNUSED_TTL_Q(IC36.4/*Q2*/)
	UNUSED_TTL_Q(IC36.8/*Q4*/)
	UNUSED_TTL_I(IC36.9/*A4*/)
	UNUSED_TTL_Q(IC36.10/*Q5*/)
	UNUSED_TTL_I(IC36.11/*A5*/)
	UNUSED_TTL_Q(IC36.12/*Q6*/)
	UNUSED_TTL_I(IC36.13/*A6*/)

	// --------------------------------
	// OUTPUT
	NET_C(JEEP_SOUND, C59.2)


	/* ------------------------------------------------------------------------
	 *  POINT_SOUND
	 * -----------------------------------------------------------------------*/
	// INPUT
	NET_C(POINT_TRG, R148.2)

	// --------------------------------
	// TRG -> CON
	RES(R148, RES_K(1))
	RES(R149, RES_K(47))
	MA150(D5)
	RES(R159, 470)
	RES(R164, RES_K(470))
	RES(R165, RES_K(470))
	RES(R163, RES_K(27))
	RES(R166, RES_K(12))
	RES(R169, RES_K(30))
	RES(R168, RES_K(10))
	CAP(C71, CAP_U(1))
	CAP(C74, CAP_U(1))
	NET_C(R148.1, V5)
	NET_C(IC35.9/*A2*/, R148.2)
	NET_C(IC35.10/*B2*/, IC35.11/*CLRQ2*/, V5)
	NET_C(IC35.6/*C2*/, C71.1)
	NET_C(IC35.7/*RC2*/, C71.2, R149.2)
	NET_C(R149.1, V5)
	NET_C(IC35.12/*QQ2*/, D5.K)
	NET_C(D5.A, R159.1)
	NET_C(R159.2, C74.1, R165.2)
	NET_C(C74.2, GND)
	NET_C(IC38.12/*PLUS4*/, R165.1, R164.2)
	NET_C(R164.1, V5)
	NET_C(IC38.14/*OUT4*/, IC38.13/*MINUS4*/, R166.1)
	NET_C(R163.1, V12)
	NET_C(IC38.10/*PLUS3*/, R163.2, R168.1)
	NET_C(R168.2, GND)
	NET_C(IC38.9/*MINUS3*/, R166.2, R169.1)
	NET_C(IC38.8/*OUT3*/, R169.2)

	// --------------------------------
	// UPPER STAGE -> MIX
	RES(R79, RES_K(30))
	RES(R80, RES_K(51))
	RES(R81, RES_K(51))
	RES(R78, RES_K(15))
	RES(R32, RES_K(30))
	RES(R33, RES_K(10))
	CAP(C43, CAP_U(0.022))
	C458(TR13)
	NE555(IC18)
	NET_C(GND, IC18.GND/*1*/)
	NET_C(V5,  IC18.VCC/*8*/)
	LM324_DIP(IC25) // shared by ANIMAL_SOUND, POINT_SOUND, POINT_SOUND, POINT_SOUND
	NET_C(GND, IC25.11/*GND*/)
	NET_C(V12, IC25.4/*VCC*/)
	NET_C(IC38.8/*OUT3*/, R79.1, R80.1)
	NET_C(IC25.12/*PLUS4*/, R80.2, R81.1)
	NET_C(R81.2, GND)
	NET_C(IC25.13/*MINUS4*/, R79.2, C43.1, R78.1)
	NET_C(IC18.RESET/*4*/, RST)
	NET_C(R33.1, V5)
	NET_C(IC18.DISCH/*7*/, R33.2)
	NET_C(TR13.B, IC18.DISCH/*7*/) // B & C swapped, wrong in schematics
	NET_C(TR13.C, R78.2) // B & C swapped, wrong in schematics
	NET_C(TR13.E, GND)
	NET_C(IC25.14/*OUT4*/, IC18.TRIG/*2*/, IC18.THRESH/*6*/, C43.2, R32.1)

	// --------------------------------
	// MIDDLE STAGE -> UPPER STAGE
	RES(R84, RES_K(30))
	RES(R83, RES_K(51))
	RES(R82, RES_K(51))
	RES(R85, RES_K(15))
	RES(R35, RES_K(30))
	RES(R36, RES_K(10))
	RES(R37, RES_K(2.2))
	CAP(C44, CAP_P(6800))
	C458(TR14)
	NE555(IC19)
	NET_C(GND, IC19.GND/*1*/)
	NET_C(V5,  IC19.VCC/*8*/)
	NET_C(IC38.8/*OUT3*/, R83.1, R84.1)
	NET_C(IC25.10/*PLUS3*/, R83.2, R82.1)
	NET_C(R82.2, GND)
	NET_C(IC25.9/*MINUS3*/, R84.2, C44.1, R85.1)
	NET_C(IC19.RESET/*4*/, RST)
	NET_C(R36.1, V5)
	NET_C(IC19.DISCH/*7*/, R36.2)
	NET_C(TR14.B, IC19.DISCH/*7*/) // B & C swapped, wrong in schematics
	NET_C(TR14.C, R85.2) // B & C swapped, wrong in schematics
	NET_C(TR14.E, GND)
	NET_C(IC25.8/*OUT3*/, IC19.TRIG/*2*/, IC19.THRESH/*6*/, C44.2, R35.2)
	NET_C(R35.1, R32.2)
	NET_C(R37.1, IC18.CONT/*5*/, IC19.CONT/*5*/)

	// --------------------------------
	// LOWER STAGE -> UPPER STAGE
	RES(R130, RES_K(30))
	RES(R129, RES_K(51))
	RES(R128, RES_K(51))
	RES(R131, RES_K(15))
	RES(R34, RES_K(10))
	CAP(C61, CAP_U(0.068))
	CAP(C60, CAP_U(0.022))
	CAP(C23, CAP_U(0.01))
	C458(TR4)
	NE555(IC8)
	NET_C(GND, IC8.GND/*1*/)
	NET_C(V5,  IC8.VCC/*8*/)
	NET_C(IC38.8/*OUT3*/, R130.1, R129.1)
	NET_C(IC25.5/*PLUS2*/, R129.2, R128.1)
	NET_C(R128.2, GND)
	NET_C(IC25.6/*MINUS2*/, R130.2, C60.1, C61.1, R131.1)
	NET_C(IC8.RESET/*4*/, RST)
	NET_C(IC8.CONT/*5*/, C23.1)
	NET_C(C23.2, GND)
	NET_C(R34.1, V5)
	NET_C(IC8.DISCH/*7*/, R34.2)
	NET_C(TR4.B, IC8.DISCH/*7*/) // B & C swapped, wrong in schematics
	NET_C(TR4.C, R131.2) // B & C swapped, wrong in schematics
	NET_C(TR4.E, GND)
	NET_C(IC25.7/*OUT2*/, IC8.TRIG/*2*/, IC8.THRESH/*6*/, C61.2, C60.2, R37.2)

	// --------------------------------
	// CON, MIX -> OUT
	CAP(C15, CAP_U(2.2))
	CAP(C16, CAP_P(680))
	CAP(C13, CAP_U(2.2))
	NET_C(R32.2, C13.1)
	NET_C(IC5.1/*IN1*/, C13.2)
	NET_C(IC5.15/*OUT1*/, C15.1)
	NET_C(IC5.14/*RO1*/, C16.1)
	NET_C(C16.2, GND)
	NET_C(IC38.14/*OUT4*/, IC5.2/*CON1*/)

	// --------------------------------
	// UNUSED PINS
	NC_(IC35.5/*Q2*/)

	NET_C(IC8.OUT/*3*/, NC_IC8_OUT.1) // not connected in schematics
			RES(NC_IC8_OUT, RES_M(1))
			NET_C(NC_IC8_OUT.2, GND)
	NET_C(IC18.OUT/*3*/, NC_IC18_OUT.1) // not connected in schematics
			RES(NC_IC18_OUT, RES_M(1))
			NET_C(NC_IC18_OUT.2, GND)
	NET_C(IC19.OUT/*3*/, NC_IC19_OUT.1) // not connected in schematics
			RES(NC_IC19_OUT, RES_M(1))
			NET_C(NC_IC19_OUT.2, GND)

	// --------------------------------
	// OUTPUT
	NET_C(POINT_SOUND, C15.2)


	/* ------------------------------------------------------------------------
	 *  HIT_SOUND
	 * -----------------------------------------------------------------------*/
	// INPUT
	NET_C(HIT_TRG, R147.1)

	// --------------------------------
	// TRG -> CON
	TTL_74123_DIP(IC35) // shared by HIT_SOUND, POINT_SOUND
	NET_C(GND, IC35.8/*GND*/)
	NET_C(V5,  IC35.16/*VCC*/)
	RES(R147, RES_K(1))
	RES(R146, RES_K(47))
	RES(R160, 470)
	RES(R161, RES_K(470))
	RES(R162, RES_K(470))
	CAP(C70, CAP_U(1))
	CAP(C75, CAP_U(1))
	MA150(D6)
	NET_C(IC35.1/*A1*/, R147.1)
	NET_C(R147.2, V5)
	NET_C(IC35.2/*B1*/, IC35.3/*CLRQ1*/, V5)
	NET_C(IC35.14/*C1*/, C70.1)
	NET_C(IC35.15/*RC1*/, C70.2, R146.2)
	NET_C(R146.1, V5)
	NET_C(IC35.4/*QQ1*/, D6.K)
	NET_C(D6.A, R160.1)
	NET_C(R160.2, C75.1, R161.2)
	NET_C(C75.2, GND)
	NET_C(IC38.3/*PLUS1*/, R161.1, R162.2)
	NET_C(R162.1, V5)
	NET_C(IC38.1/*OUT1*/, IC38.2/*MINUS1*/)

	// --------------------------------
	// CON, MIX -> OUT
	MB4391_DIP(IC5) // shared by POINT_SOUND, HIT_SOUND
	NET_C(GND, IC5.3/*GND1*/, IC5.7/*GND2*/)
	NET_C(V5, IC5.16/*VCC1*/, IC5.12/*VCC2*/)
	CAP(C18, CAP_P(680))
	NET_C(C18.1, IC5.10/*RO2*/)
	NET_C(C18.2, GND)
	CAP(C14, CAP_U(2.2))
	CAP(C17, CAP_U(2.2))
	NET_C(IC5.6/*CON2*/, IC38.1/*OUT1*/)
	NET_C(IC5.11/*OUT2*/, C17.1)
	NET_C(IC5.5/*IN2*/, C14.2)

	// --------------------------------
	// LOWER STAGE -> MIX
	C458(TR11)
	NE555(IC16)
	NET_C(GND, IC16.GND/*1*/)
	NET_C(V5,  IC16.VCC/*8*/)
	RES(R71, RES_K(51))
	RES(R70, RES_K(30))
	RES(R72, RES_K(51))
	RES(R69, RES_K(15))
	RES(R28, RES_K(10))
	RES(R76, RES_K(1))
	CAP(C39, CAP_U(0.01))
	CAP(C40, CAP_U(0.0033))
	NET_C(IC24.14/*OUT4*/, C14.1)
	NET_C(IC24.7/*OUT2*/, R70.1, R71.1)
	NET_C(IC24.12/*PLUS4*/, R71.2, R72.1)
	NET_C(R72.2, GND)
	NET_C(IC24.13/*MINUS4*/, R70.2, C39.1, C40.1, R69.1)
	NET_C(IC24.14/*OUT4*/, IC16.TRIG/*2*/, IC16.THRESH/*6*/, C39.2, C40.2)
	NET_C(IC16.RESET/*4*/, RST)
	NET_C(IC16.CONT/*5*/, R76.2)
	NET_C(R28.1, V5)
	NET_C(TR11.B, IC16.DISCH/*7*/, R28.2) // B & C swapped, wrong in schematics
	NET_C(TR11.C, R69.2) // B & C swapped, wrong in schematics
	NET_C(TR11.E, GND)

	NET_C(IC24.8/*OUT3*/, R76.1)

	RES(R119, RES_K(10))
	RES(R120, RES_K(39))
	RES(R121, RES_K(47))
	RES(R122, RES_K(15))

	LM324_DIP(IC24) // shared by EMAR_SOUND, HIT_SOUND, HIT_SOUND, HIT_SOUND
	NET_C(GND, IC24.11/*GND*/)
	NET_C(V12, IC24.4/*VCC*/)

	NET_C(V12, R120.1)
	NET_C(R120.2, R119.1)
	NET_C(IC24.5/*PLUS2*/, R120.2)
	NET_C(R119.2, GND)
	NET_C(IC24.6/*MINUS2*/, R122.2, R121.1)
	NET_C(IC24.7/*OUT2*/, R121.2)

	NET_C(IC24.8/*OUT3*/, R122.1)

	// --------------------------------
	// UPPER STAGE -> LOWER STAGE
	C458(TR12)
	NE555(IC17)
	NET_C(GND, IC17.GND/*1*/)
	NET_C(V5,  IC17.VCC/*8*/)
	RES(R75, RES_K(470))
	RES(R74, RES_K(51))
	RES(R73, RES_K(51))
	RES(R77, RES_K(5.1))
	RES(R30, RES_K(10))
	CAP(C41, CAP_U(2.2))
	CAP(C42, CAP_U(2.2))
	CAP(C21, CAP_U(0.01))
	NET_C(V6, R75.1, R74.1)
	NET_C(C41.1, C42.1)
	NET_C(IC24.9/*MINUS3*/, R75.2, R77.1, C41.2)
	NET_C(IC24.10/*PLUS3*/, R74.2, R73.1)
	NET_C(R73.2, GND)
	NET_C(IC17.RESET/*4*/, RST)
	NET_C(R30.1, V5)
	NET_C(IC17.DISCH/*7*/, R30.2)
	NET_C(IC17.CONT/*5*/, C21.1)
	NET_C(C21.2, GND)
	NET_C(TR12.B, IC17.DISCH/*7*/) // B & C swapped, wrong in schematics
	NET_C(TR12.C, R77.2) // B & C swapped, wrong in schematics
	NET_C(TR12.E, GND)
	NET_C(IC24.8/*OUT3*/, IC17.TRIG/*2*/, IC17.THRESH/*6*/, C42.2)

	// --------------------------------
	// UNUSED PINS
	NC_(IC35.13/*Q1*/)

	NET_C(IC17.OUT/*3*/, NC_IC17_OUT.1) // not connected in schematics
			RES(NC_IC17_OUT, RES_M(1))
			NET_C(NC_IC17_OUT.2, GND)
	NET_C(IC16.OUT/*3*/, NC_IC16_OUT.1) // not connected in schematics
			RES(NC_IC16_OUT, RES_M(1))
			NET_C(NC_IC16_OUT.2, GND)

	// --------------------------------
	// OUTPUT
	NET_C(HIT_SOUND, C17.2)


	/* ------------------------------------------------------------------------
	 *  ANIMAL_SOUND
	 * -----------------------------------------------------------------------*/
	// INPUT
	NET_C(ANIMAL_TRG, R145.2)

	// --------------------------------
	// TGG -> TRG2 -> CON
	RES(R145, RES_K(1))
	RES(R143, RES_K(47))
	RES(R144, RES_K(47))
	RES(R106, 470)
	RES(R108, RES_K(470))
	RES(R107, RES_K(470))
	CAP(C67, CAP_U(47))
	CAP(C66, CAP_U(47))
	CAP(C69, CAP_U(1))
	CAP(C53, CAP_U(1))
	MA150(D4)
	TTL_74123_DIP(IC34) // shared by ANIMAL_SOUND, ANIMAL_SOUND
	NET_C(GND, IC34.8/*GND*/)
	NET_C(V5,  IC34.16/*VCC*/)
	NET_C(R145.1, V5)
	NET_C(IC34.1/*A1*/, R145.2)
	NET_C(IC34.2/*B1*/, IC34.3/*CLRQ1*/, V5)
	NET_C(IC34.14/*C1*/, C67.2, C66.2)
	NET_C(IC34.15/*RC1*/, C67.1, C66.1, R143.2)
	NET_C(R143.1, V5)
	NET_C(IC34.1/*A1*/, IC34.9/*A2*/)
	NET_C(IC34.13/*Q1*/, IC34.10/*B2*/, IC34.11/*CLRQ2*/)
	NET_C(IC34.6/*C2*/, C69.2)
	NET_C(IC34.7/*RC2*/, C69.1, R144.2)
	NET_C(R144.1, V5)
	NET_C(IC34.12/*QQ2*/, D4.K)
	NET_C(D4.A, R106.1)
	NET_C(R106.2, C53.1, R107.2)
	NET_C(C53.2, GND)
	NET_C(IC23.3/*PLUS1*/, R107.1, R108.2)
	NET_C(R108.1, V5)
	NET_C(IC23.1/*OUT1*/, IC23.2/*MINUS1*/, IC4.6/*CON2*/)

	// --------------------------------
	C458(TR16)
	RES(R110, RES_K(10))
	NET_C(IC34.13/*Q1*/, R110.1)
	NET_C(TR16.B, R110.2) // B & C swapped, wrong in schematics
	NET_C(TR16.C, R109.2) // B & C swapped, wrong in schematics
	NET_C(TR16.E, GND)

	C458(TR15)
	RES(R109, RES_K(1))
	RES(R44, 220)
	NET_C(R109.1, V5)
	NET_C(TR15.B, R109.2) // B & C swapped, wrong in schematics
	NET_C(TR15.C, C24.1) // B & C swapped, wrong in schematics
	NET_C(TR15.E, R44.1)
	NET_C(R44.2, C24.2)

	// --------------------------------
	// UPPER STAGE -> MIDDLE STAGE
	RES(R39, RES_K(51))
	RES(R38, RES_K(22))
	RES(R15, RES_K(51))
	RES(R41, RES_K(22))
	RES(R42, RES_K(36))
	RES(R43, RES_K(10))
	RES(R40, RES_M(1))
	CAP(C24, CAP_U(100))
	LM324_DIP(IC9) // shared by ANIMAL_SOUND, ANIMAL_SOUND, ANIMAL_SOUND, ANIMAL_SOUND
	NET_C(GND, IC9.11/*GND*/)
	NET_C(V12, IC9.4/*VCC*/)
	NET_C(R40.1, R39.1, V12)
	NET_C(IC9.5/*PLUS2*/, R39.2, R15.1)
	NET_C(R15.2, GND)
	NET_C(IC9.6/*MINUS2*/, R40.2, C24.1)
	NET_C(IC9.7/*OUT2*/, C24.2, R38.1)
	NET_C(R42.1, V12)
	NET_C(IC9.3/*PLUS1*/, R42.2, R43.1)
	NET_C(R43.2, GND)
	NET_C(IC9.2/*MINUS1*/, R38.2, R41.1)
	NET_C(IC9.1/*OUT1*/, R41.2)

	// --------------------------------
	// MIDDLE STAGE -> MIX
	RES(R11, RES_K(30))
	RES(R12, RES_K(51))
	RES(R13, RES_K(51))
	RES(R10, RES_K(1))
	RES(R9, RES_K(10))
	RES(R14, RES_K(5.1))
	RES(R16, RES_K(10))
	CAP(C1, CAP_U(0.033))
	C458(TR1)
	NE555(IC1)
	NET_C(GND, IC1.GND/*1*/)
	NET_C(V5,  IC1.VCC/*8*/)
	NET_C(IC9.1/*OUT1*/, R11.1, R12.1)
	NET_C(IC9.12/*PLUS4*/, R12.2, R13.1)
	NET_C(R13.2, GND)
	NET_C(IC9.13/*MINUS4*/, R11.2, C1.1, R10.1)
	NET_C(IC1.RESET/*4*/, RST)
	NET_C(IC1.DISCH/*7*/, R9.2)
	NET_C(R9.1, V5)
	NET_C(TR1.B, IC1.DISCH/*7*/) // B & C swapped, wrong in schematics
	NET_C(TR1.C, R10.2) // B & C swapped, wrong in schematics
	NET_C(TR1.E, GND)
	NET_C(IC9.14/*OUT4*/, IC1.TRIG/*2*/, IC1.THRESH/*6*/, C1.2)
	NET_C(IC9.14/*OUT4*/, R16.1)
	NET_C(IC1.CONT/*5*/, R14.1)

	RES(R17, RES_K(5.1))
	RES(R18, RES_K(3.3))
	NET_C(IC9.8/*OUT3*/, IC9.9/*MINUS3*/, R14.2)
	NET_C(IC9.10/*PLUS3*/, R17.2, R18.2)

	// --------------------------------
	// LOWER STAGE -> MIDDLE STAGE
	RES(R59, RES_K(560))
	RES(R58, RES_K(51))
	RES(R57, RES_K(51))
	RES(R60, RES_K(5.1))
	RES(R23, RES_K(10))
	CAP(C34, CAP_U(1))
	CAP(C35, CAP_U(1))
	CAP(C7, CAP_U(0.01))
	C458(TR8)
	NE555(IC13)
	NET_C(GND, IC13.GND/*1*/)
	NET_C(V5,  IC13.VCC/*8*/)
	NET_C(R59.1, R58.1, V6)
	NET_C(IC22.10/*PLUS3*/, R58.2, R57.1)
	NET_C(R57.2, GND)
	NET_C(IC22.9/*MINUS3*/, R59.2, C34.2, R60.1)
	NET_C(C34.1, C35.1)
	NET_C(IC13.RESET/*4*/, RST)
	NET_C(IC13.DISCH/*7*/, R23.2)
	NET_C(R23.1, V5)
	NET_C(IC13.CONT/*5*/, C7.1)
	NET_C(C7.2, GND)
	NET_C(TR8.B, IC13.DISCH/*7*/) // B & C swapped, wrong in schematics
	NET_C(TR8.C, R60.2) // B & C swapped, wrong in schematics
	NET_C(TR8.E, GND)
	NET_C(IC22.8/*OUT3*/, IC13.TRIG/*2*/, IC13.THRESH/*6*/, C35.2, R18.1)

	// --------------------------------
	// BOTTOM STAGE -> MIDDLE STAGE
	RES(R127, RES_K(56))
	RES(R125, RES_K(51))
	RES(R124, RES_K(51))
	RES(R126, RES_K(30))
	RES(R31, RES_K(10))
	CAP(C57, CAP_U(1))
	CAP(C56, CAP_U(1))
	CAP(C22, CAP_U(0.01))
	C458(TR3)
	NE555(IC7)
	NET_C(GND, IC7.GND/*1*/)
	NET_C(V5,  IC7.VCC/*8*/)
	NET_C(R127.1, R125.1, V6)
	NET_C(IC25.3/*PLUS1*/, R125.2, R124.1)
	NET_C(R124.2, GND)
	NET_C(IC25.2/*MINUS1*/, R127.2, C57.2, R126.1)
	NET_C(C57.1, C56.1)
	NET_C(IC7.RESET/*4*/, RST)
	NET_C(IC7.CONT/*5*/, C22.1)
	NET_C(C22.2, GND)
	NET_C(IC7.DISCH/*7*/, R31.2)
	NET_C(R31.1, V5)
	NET_C(TR3.B, IC7.DISCH/*7*/) // B & C swapped, wrong in schematics
	NET_C(TR3.C, R126.2) // B & C swapped, wrong in schematics
	NET_C(TR3.E, GND)
	NET_C(IC25.1/*OUT1*/, IC7.TRIG/*2*/, IC7.THRESH/*6*/, C56.2)
	NET_C(IC25.1/*OUT1*/, R17.1)

	// --------------------------------
	// CON, MIX -> OUT
	CAP(C6, CAP_U(2.2))
	CAP(C11, CAP_U(2.2))
	CAP(C12, CAP_P(680))
	NET_C(IC4.10/*RO2*/, C12.1)
	NET_C(C12.2, GND)
	NET_C(R16.2, C6.1)
	NET_C(IC4.5/*IN2*/, C6.2)
	NET_C(IC4.11/*OUT2*/, C11.1)

	// --------------------------------
	// UNUSED PINS
	NC_(IC34.4/*QQ1*/)
	NC_(IC34.5/*Q2*/)

	NET_C(IC1.OUT/*3*/, NC_IC1_OUT.1) // not connected in schematics
			RES(NC_IC1_OUT, RES_M(1))
			NET_C(NC_IC1_OUT.2, GND)

	NET_C(IC13.OUT/*3*/, NC_IC13_OUT.1) // not connected in schematics
			RES(NC_IC13_OUT, RES_M(1))
			NET_C(NC_IC13_OUT.2, GND)

	NET_C(IC7.OUT/*3*/, NC_IC7_OUT.1) // not connected in schematics
			RES(NC_IC7_OUT, RES_M(1))
			NET_C(NC_IC7_OUT.2, GND)


	// --------------------------------
	// OUTPUT
	NET_C(ANIMAL_SOUND, C11.2)


	/* ------------------------------------------------------------------------
	 *  EMAR_SOUND
	 * -----------------------------------------------------------------------*/
	// INPUT
	NET_C(EMAR_TRG, R142.2)

	// --------------------------------
	// TRG -> CON
	RES(R142, RES_K(1))
	RES(R141, RES_K(47))
	MA150(D3)
	RES(R103, 470)
	RES(R105, RES_K(470))
	RES(R104, RES_K(470))
	CAP(C68, CAP_U(1))
	CAP(C52, CAP_U(1))
	NET_C(IC33.9/*A2*/, R142.2)
	NET_C(R142.1, V5)
	NET_C(IC33.10/*B2*/, IC33.11/*CLRQ2*/, V5)
	NET_C(IC33.6/*C2*/, C68.1)
	NET_C(IC33.7/*RC2*/, C68.2, R141.2)
	NET_C(R141.1, V5)
	NET_C(IC33.12/*QQ2*/, D3.K)
	NET_C(D3.A, R103.1)
	NET_C(R103.2, C52.1, R105.2)
	NET_C(C52.2, GND)
	NET_C(IC22.5/*PLUS2*/, R105.1, R104.2)
	NET_C(R104.1, V5)
	NET_C(IC22.7/*OUT2*/, IC22.6/*MINUS2*/)

	// --------------------------------
	// CON, MIX -> OUT
	MB4391_DIP(IC4) // shared by EMAR_SOUND, ANIMAL_SOUND
	NET_C(GND, IC4.3/*GND1*/, IC4.7/*GND2*/)
	NET_C(V5, IC4.16/*VCC1*/, IC4.12/*VCC2*/)
	RES(R24, RES_K(10))
	CAP(C8, CAP_U(2.2))
	CAP(C9, CAP_U(2.2))
	CAP(C10, CAP_P(680))
	NET_C(IC22.7/*OUT2*/, IC4.2/*CON1*/)
	NET_C(IC4.14/*RO1*/, C10.1)
	NET_C(C10.2, GND)
	NET_C(IC4.15/*OUT1*/, C9.1)
	NET_C(IC4.1/*IN1*/, C8.2)
	NET_C(R24.2, C8.1)

	LM324_DIP(IC23) // shared by ANIMAL_SOUND, EMAR_SOUND, EMAR_SOUND, EMAR_SOUND
	NET_C(GND, IC23.11/*GND*/)
	NET_C(V12, IC23.4/*VCC*/)

	// --------------------------------
	// LOWER STAGE -> UPPER STAGE
	C458(TR2)
	NE555(IC6)
	NET_C(GND, IC6.GND/*1*/)
	NET_C(V5,  IC6.VCC/*8*/)
	RES(R116, RES_K(470))
	RES(R117, RES_K(51))
	RES(R118, RES_K(51))
	RES(R115, RES_K(51))
	RES(R29, RES_K(10))
	CAP(C55, CAP_U(3.3))
	CAP(C54, CAP_U(3.3))
	CAP(C20, CAP_U(0.01))
	NET_C(R116.1, R117.1, V6)
	NET_C(IC24.3/*PLUS1*/, R117.2, R118.1)
	NET_C(R118.2, GND)
	NET_C(IC24.2/*MINUS1*/, R116.2, C55.2, R115.1)
	NET_C(C55.1, C54.1)
	NET_C(IC6.RESET/*4*/, RST)
	NET_C(IC6.CONT/*5*/, C20.1)
	NET_C(C20.2, GND)
	NET_C(IC6.DISCH/*7*/, R29.2)
	NET_C(R29.1, V5)
	NET_C(TR2.B, IC6.DISCH/*7*/) // B & C swapped, wrong in schematics
	NET_C(TR2.C, R115.2) // B & C swapped, wrong in schematics
	NET_C(TR2.E, GND)
	NET_C(IC24.1/*OUT1*/, IC6.TRIG/*2*/, IC6.THRESH/*6*/, C54.2)

	NET_C(IC24.1/*OUT1*/, R113.1)
	RES(R113, RES_K(22))
	RES(R111, RES_K(27))
	RES(R112, RES_K(10))
	RES(R114, RES_K(22))
	NET_C(R111.1, V12)
	NET_C(IC23.5/*PLUS2*/, R111.2, R112.1)
	NET_C(R112.2, GND)
	NET_C(IC23.6/*MINUS2*/, R113.2, R114.1)
	NET_C(IC23.7/*OUT2*/, R114.2)

	// --------------------------------
	// MIDDLE STAGE -> UPPER STAGE
	C458(TR10)
	NE555(IC15)
	NET_C(GND, IC15.GND/*1*/)
	NET_C(V5,  IC15.VCC/*8*/)
	RES(R67, RES_K(220))
	RES(R66, RES_K(51))
	RES(R65, RES_K(51))
	RES(R68, RES_K(100))
	RES(R27, RES_K(10))
	RES(R26, RES_K(33))
	CAP(C38, CAP_U(1))
	CAP(C37, CAP_U(1))
	CAP(C19, CAP_U(0.01))
	NET_C(R67.1, R66.1, V6)
	NET_C(IC23.10/*PLUS3*/, R66.2, R65.1)
	NET_C(R65.2, GND)
	NET_C(IC23.9/*MINUS3*/, R67.2, C37.2, R68.1)
	NET_C(IC15.RESET/*4*/, RST)
	NET_C(IC15.CONT/*5*/, C19.1)
	NET_C(C19.2, GND)
	NET_C(IC15.DISCH/*7*/, R27.2)
	NET_C(R27.1, V5)
	NET_C(TR10.B, IC15.DISCH/*7*/) // B & C swapped, wrong in schematics
	NET_C(TR10.C, R68.2) // B & C swapped, wrong in schematics
	NET_C(TR10.E, GND)
	NET_C(IC23.8/*OUT3*/, IC15.TRIG/*2*/, IC15.THRESH/*6*/, C38.2)
	NET_C(C38.1, C37.1)
	NET_C(IC15.OUT/*3*/, R26.2)

	// --------------------------------
	// UPPER STAGE -> MIX
	C458(TR9)
	NE555(IC14)
	NET_C(GND, IC14.GND/*1*/)
	NET_C(V5,  IC14.VCC/*8*/)
	RES(R62, RES_K(30))
	RES(R63, RES_K(51))
	RES(R64, RES_K(51))
	RES(R61, RES_K(15))
	RES(R25, RES_K(10))
	CAP(C36, CAP_U(0.022))
	CAP(C2, CAP_U(1)) // ? maybe 0.01uF
	NET_C(IC23.7/*OUT2*/, R62.1, R63.1)
	NET_C(IC23.12/*PLUS4*/, R63.2, R64.1)
	NET_C(R64.2, GND)
	NET_C(IC23.13/*MINUS4*/, R62.2, C36.1, R61.1)
	NET_C(IC14.RESET/*4*/, RST)
	NET_C(IC14.DISCH/*7*/, R25.2)
	NET_C(R25.1, V5)
	NET_C(IC14.CONT/*5*/, C2.1, R26.1)
	NET_C(C2.2, GND)
	NET_C(TR9.B, IC14.DISCH/*7*/) // B & C swapped, wrong in schematics
	NET_C(TR9.C, R61.2) // B & C swapped, wrong in schematics
	NET_C(TR9.E, GND)
	NET_C(IC23.14/*OUT4*/, IC14.TRIG/*2*/, IC14.THRESH/*6*/, C36.2)
	NET_C(IC23.14/*OUT4*/, R24.1)

	// --------------------------------
	// UNUSED PINS
	NC_(IC33.5/*Q2*/)

	NET_C(IC6.OUT/*3*/, NC_IC6_OUT.1) // not connected in schematics
			RES(NC_IC6_OUT, RES_M(1))
			NET_C(NC_IC6_OUT.2, GND)
	NET_C(IC14.OUT/*3*/, NC_IC14_OUT.1) // not connected in schematics
			RES(NC_IC14_OUT, RES_M(1))
			NET_C(NC_IC14_OUT.2, GND)

	// --------------------------------
	// OUTPUT
	NET_C(EMAR_SOUND, C9.2)


	/* ------------------------------------------------------------------------
	 *  WALK_SOUND
	 * -----------------------------------------------------------------------*/
	// INPUT
	NET_C(WALK_TRG, R140.2)

	// --------------------------------
	NE555(IC28)
	NET_C(GND, IC28.GND/*1*/)
	NET_C(V5, IC28.VCC/*8*/)

	LM324_DIP(IC21) // shared by CRY_SOUND, WALK_SOUND, CRY_SOUND, CRY_SOUND
	NET_C(GND, IC21.11/*GND*/)
	NET_C(V12, IC21.4/*VCC*/)

	RES(R95, RES_K(100))
	RES(R96, RES_K(22))
	RES(R98, RES_K(27))
	RES(R137, RES_K(22))
	RES(R140, RES_K(1))
	CAP(C3, CAP_U(2.2))
	CAP(C50, CAP_U(0.1))
	CAP(C51, CAP_U(2.2))
	CAP(C65, CAP_U(0.01))
	CAP(C77, CAP_U(2.2))

	NET_C(GND, C65.2, C77.2, C50.2)
	NET_C(V6, IC21.5/*PLUS2*/)
	NET_C(V5, R140.1, R137.1)
	NET_C(R140.2, IC28.TRIG/*2*/)
	NET_C(C65.1, IC28.CONT/*5*/)
	NET_C(IC28.THRESH/*6*/, IC28.DISCH/*7*/, R137.2, C77.1)
	NET_C(IC28.OUT/*3*/ , R98.1)
	NET_C(R98.2, C50.1, C51.1)
	NET_C(C51.2, R96.1)
	NET_C(R96.2, R95.1, IC21.6/*MINUS2*/)
	NET_C(IC21.7/*OUT2*/, R95.2, C3.1)

	// --------------------------------
	// UNUSED PINS
	NET_C(V5, IC28.RESET/*4*/) // not connected on schematic

	// --------------------------------
	// OUTPUT
	NET_C(WALK_SOUND, C3.2)


	/* ------------------------------------------------------------------------
	 *  CRY_SOUND
	 * -----------------------------------------------------------------------*/
	// INPUT
	NET_C(CRY_TRG, R139.2)

	// --------------------------------
	// TRG -> CON
	TTL_74123_DIP(IC33) // shared by CRY_SOUND, EMAR_SOUND
	NET_C(GND, IC33.8/*GND*/)
	NET_C(V5,  IC33.16/*VCC*/)
	LM324_DIP(IC22) // shared by CRY_SOUND, EMAR_SOUND, ANIMAL_SOUND, CRY_SOUND
	NET_C(GND, IC22.11/*GND*/)
	NET_C(V12, IC22.4/*VCC*/)
	MA150(D2)
	RES(R139, RES_K(1))
	RES(R138, RES_K(47))
	RES(R97, 470)
	RES(R87, RES_K(470))
	RES(R94, RES_K(470))
	CAP(C64, CAP_U(1))
	NET_C(V5, R139.1, R138.1, R87.1)
	NET_C(GND, C49.2)
	NET_C(R139.2, IC33.1/*A1*/)
	NET_C(V5, IC33.2/*B1*/, IC33.3/*CLRQ1*/)
	NET_C(IC33.14/*C1*/, C64.1)
	NET_C(IC33.15/*RC1*/, C64.2, R138.2)
	NET_C(IC33.4/*QQ1*/, D2.K)
	NC_(IC33.13/*Q1*/)
	NET_C(D2.A, R97.1)
	NET_C(R97.2, C49.1, R94.2)
	NET_C(IC21.3/*PLUS1*/, R87.2, R94.1)
	NET_C(IC21.1/*OUT1*/, IC21.2/*MINUS1*/, IC20.6/*CON2*/)

	// --------------------------------
	// CON, MIX -> OUT
	RES(R86, RES_K(10))
	CAP(C47, CAP_U(2.2))
	CAP(C49, CAP_U(1))
	CAP(C28, CAP_P(680))
	CAP(C27, CAP_U(2.2))
	NET_C(IC21.14/*OUT4*/, R86.1)
	NET_C(R86.2, C47.1)
	NET_C(IC20.5/*IN2*/, C47.2)
	NET_C(IC20.10/*RO2*/, C28.1)
	NET_C(C28.2, GND)
	NET_C(IC20.11/*OUT2*/, C27.1)

	// --------------------------------
	// UPPER STAGE -> MIDDLE STAGE
	C458(TR7)
	NE555(IC12)
	NET_C(GND, IC12.GND/*1*/)
	NET_C(V5, IC12.VCC/*8*/)
	NET_C(RST, IC12.RESET/*4*/)
	RES(R22, RES_K(10))
	RES(R55, RES_K(51))
	RES(R54, RES_K(220))
	RES(R56, RES_K(51))
	RES(R53, RES_K(100))
	CAP(C33, CAP_U(0.22))
	CAP(C32, CAP_U(0.22))
	CAP(C5, CAP_U(0.01))
	NET_C(TR7.B, IC12.DISCH/*7*/) // B & C swapped, wrong in schematics
	NET_C(TR7.C, R53.2) // B & C swapped, wrong in schematics
	NET_C(TR7.E, GND)
	NET_C(V5, R22.1)
	NET_C(IC12.DISCH/*7*/, R22.2)
	NET_C(IC22.14/*OUT4*/, IC12.TRIG/*2*/, IC12.THRESH/*6*/)
	NET_C(IC22.14/*OUT4*/, C32.2)
	NET_C(C32.1, C33.1)
	NET_C(IC22.13/*MINUS4*/, R53.1, R54.2, C33.2)
	NET_C(IC22.12/*PLUS4*/, R55.2, R56.1)
	NET_C(R56.2, GND)
	NET_C(R54.1, R55.1, V6)
	NET_C(IC12.CONT/*5*/, C5.1)
	NET_C(C5.2, GND)

	RES(R99, RES_K(22))
	RES(R100, RES_K(100))
	RES(R101, RES_K(15))
	RES(R102, RES_K(33))
	NET_C(IC22.14/*OUT4*/, R100.1)
	NET_C(IC22.1/*OUT1*/, R102.2)
	NET_C(IC22.2/*MINUS1*/, R100.2, R102.1)
	NET_C(IC22.3/*PLUS1*/, R99.2, R101.1)
	NET_C(R99.1, V12)
	NET_C(R101.2, GND)

	// --------------------------------
	// LOWER STAGE -> MIDDLE STAGE
	C458(TR6)
	NE555(IC11)
	NET_C(GND, IC11.GND/*1*/)
	NET_C(V5, IC11.VCC/*8*/)
	NET_C(RST, IC11.RESET/*4*/)
	RES(R21, RES_K(10))
	RES(R49, RES_K(51))
	RES(R50, RES_K(51))
	RES(R51, RES_K(470))
	RES(R52, RES_K(5.1))
	CAP(C30, CAP_U(2.2))
	CAP(C31, CAP_U(2.2))
	CAP(C4, CAP_U(0.01))
	NET_C(TR6.B, IC11.DISCH/*7*/) // B & C swapped, wrong in schematics
	NET_C(TR6.C, R52.2) // B & C swapped, wrong in schematics
	NET_C(TR6.E, GND)
	NET_C(V5, R21.1)
	NET_C(IC11.DISCH/*7*/, R21.2)
	NET_C(IC21.8/*OUT3*/,  IC11.TRIG/*2*/, IC11.THRESH/*6*/)
	NET_C(IC21.9/*MINUS3*/,  R52.1, R51.2, C30.2)
	NET_C(C30.1, C31.1)
	NET_C(IC21.8/*OUT3*/, C31.2)
	NET_C(IC21.10/*PLUS3*/, R50.2, R49.1)
	NET_C(R49.2, GND)
	NET_C(V6, R51.1, R50.1)
	NET_C(IC11.CONT/*5*/, C4.1)
	NET_C(C4.2, GND)
	NET_C(IC21.8/*OUT3*/, R20.1)
	NET_C(IC10.CONT/*5*/, R20.2)

	// --------------------------------
	// MIDDLE STAGE -> MIX
	C458(TR5)
	NE555(IC10)
	NET_C(GND, IC10.GND/*1*/)
	NET_C(V5, IC10.VCC/*8*/)
	NET_C(RST, IC10.RESET/*4*/)
	RES(R19, RES_K(10))
	RES(R45, RES_K(15))
	RES(R46, RES_K(30))
	RES(R47, RES_K(51))
	RES(R48, RES_K(51))
	CAP(C29, CAP_U(0.022))
	RES(R20, RES_K(1))

	NET_C(TR5.B, IC10.DISCH/*7*/) // B & C swapped, wrong in schematics
	NET_C(TR5.C, R45.2) // B & C swapped, wrong in schematics
	NET_C(TR5.E, GND)
	NET_C(V5, R19.1)
	NET_C(IC10.DISCH/*7*/, R19.2)
	NET_C(IC21.14/*OUT4*/, IC10.TRIG/*2*/, IC10.THRESH/*6*/)
	NET_C(IC21.13/*MINUS4*/, R45.1, R46.2, C29.1)
	NET_C(IC21.12/*PLUS4*/, R47.2, R48.1)
	NET_C(R48.2, GND)
	NET_C(IC22.1/*OUT1*/, R46.1, R47.1)
	NET_C(IC21.14/*OUT4*/, C29.2)

	// --------------------------------
	// UNUSED PINS
	NET_C(IC10.OUT/*3*/, NC_IC10_OUT.1) // not connected in schematics
			RES(NC_IC10_OUT, RES_M(1))
			NET_C(NC_IC10_OUT.2, GND)
	NET_C(IC11.OUT/*3*/, NC_IC11_OUT.1) // not connected in schematics
			RES(NC_IC11_OUT, RES_M(1))
			NET_C(NC_IC11_OUT.2, GND)
	NET_C(IC12.OUT/*3*/, NC_IC12_OUT.1) // not connected in schematics
			RES(NC_IC12_OUT, RES_M(1))
			NET_C(NC_IC12_OUT.2, GND)

	// --------------------------------
	// OUTPUT
	NET_C(CRY_SOUND, C27.2)

NETLIST_END()




static NETLIST_START(brdrline_sound_out)
	LM324_DIP(IC38) // shared by HIT_SOUND, SOUND_OUT, POINT_SOUND, POINT_SOUND
	NET_C(GND, IC38.11/*GND*/)
	NET_C(V12, IC38.4/*VCC*/)

	POT(VR8, RES_K(200))
	POT(VR1, RES_K(200))
	POT(VR6, RES_K(200))
	POT(VR7, RES_K(200))
	POT(VR5, RES_K(500))
	POT(VR4, RES_K(200))
	POT(VR3, RES_K(200))
	POT(VR2, RES_K(200))

	PARAM(VR1.DIAL, 0.350) // GUN_SOUND
	PARAM(VR8.DIAL, 0.000) // JEEP_SOUND
	PARAM(VR6.DIAL, 0.985) // POINT_SOUND
	PARAM(VR7.DIAL, 0.640) // HIT_SOUND
	PARAM(VR5.DIAL, 0.960) // ANIMAL_SOUND
	PARAM(VR4.DIAL, 0.900) // EMAR_SOUND
	PARAM(VR3.DIAL, 0.000) // WALK_SOUND
	PARAM(VR2.DIAL, 0.900) // CRY_SOUND

	RES(R8, RES_K(22))
	RES(R1, RES_K(22))
	RES(R6, RES_K(22))
	RES(R7, RES_K(22))
	RES(R5, RES_K(22))
	RES(R4, RES_K(22))
	RES(R3, RES_K(22))
	RES(R2, RES_K(22))
	RES(R167, RES_K(10))

	CAP(C76, CAP_U(10))

	NET_C(VR8.1, VR8.2)
	NET_C(VR1.1, VR1.2)
	NET_C(VR6.1, VR6.2)
	NET_C(VR7.1, VR7.2)
	NET_C(VR5.1, VR5.2)
	NET_C(VR4.1, VR4.2)
	NET_C(VR3.1, VR3.2)
	NET_C(VR2.1, VR2.2)

	NET_C(VR8.3, R8.1)
	NET_C(VR1.3, R1.1)
	NET_C(VR6.3, R6.1)
	NET_C(VR7.3, R7.1)
	NET_C(VR5.3, R5.1)
	NET_C(VR4.3, R4.1)
	NET_C(VR3.3, R3.1)
	NET_C(VR2.3, R2.1)

	NET_C(R1.2, R2.2, R3.2, R4.2, R5.2, R6.2, R7.2, R8.2
			  , IC38.6/*MINUS2*/
			  , R167.1
			  )
	NET_C(V6, IC38.5/*PLUS2*/)
	NET_C(IC38.7/*OUT2*/, R167.2, C76.1)

	// --------------------------------
	// INPUT
#if (ENABLE_FRONTIERS)
	// using AFUNCs here tends to remove the DC bias, so add
	// it back in manually
	AFUNC(JEEP_F, 1, "5+A0")
	ALIAS(JEEP_SOUND, JEEP_F.A0)
	NET_C(JEEP_F.Q, VR8.1)

	AFUNC(GUN_F, 1, "6+A0")
	ALIAS(GUN_SOUND, GUN_F.A0)
	NET_C(GUN_F.Q, VR1.1)

	AFUNC(POINT_F, 1, "6+A0")
	ALIAS(POINT_SOUND, POINT_F.A0)
	NET_C(POINT_F.Q, VR6.1)

	AFUNC(HIT_F, 1, "6+A0")
	ALIAS(HIT_SOUND, HIT_F.A0)
	NET_C(HIT_F.Q, VR7.1)

	AFUNC(ANIMAL_F, 1, "6+A0")
	ALIAS(ANIMAL_SOUND, ANIMAL_F.A0)
	NET_C(ANIMAL_F.Q, VR5.1)

	AFUNC(EMAR_F, 1, "6+A0")
	ALIAS(EMAR_SOUND, EMAR_F.A0)
	NET_C(EMAR_F.Q, VR4.1)

	AFUNC(WALK_F, 1, "A0")
	ALIAS(WALK_SOUND, WALK_F.A0)
	NET_C(WALK_F.Q, VR3.1)

	AFUNC(CRY_F, 1, "6+A0")
	ALIAS(CRY_SOUND, CRY_F.A0)
	NET_C(CRY_F.Q, VR2.1)
#else
	ALIAS(JEEP_SOUND, VR8.1)
	ALIAS(GUN_SOUND, VR1.1)
	ALIAS(POINT_SOUND, VR6.1)
	ALIAS(HIT_SOUND, VR7.1)
	ALIAS(ANIMAL_SOUND, VR5.1)
	ALIAS(EMAR_SOUND, VR4.1)
	ALIAS(WALK_SOUND, VR3.1)
	ALIAS(CRY_SOUND, VR2.1)
#endif

	// --------------------------------
	// OUTPUT
	RES(Rsound, RES_K(1)) // dummy load
	NET_C(C76.2, Rsound.1)
	NET_C(GND, Rsound.2)
#if 0
	// for test only
	AFUNC(CLIPPING, 1, "max(-0.11,min(0.11, A0))")
	NET_C(C76.2, CLIPPING.A0)
#else
	ALIAS(SOUND_OUT, Rsound.1)
#endif

NETLIST_END()




NETLIST_START(brdrline)

#if 1
	SOLVER(Solver, 1000)
	PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 4e-5)
#else
	SOLVER(solver, 48000)
#endif

	LOCAL_SOURCE(_MB4391)
	LOCAL_SOURCE(_MB4391_DIP)

	LOCAL_SOURCE(brdrline_schematics)
	LOCAL_SOURCE(brdrline_sound_out)

	// --------------------------------
	// VOLTAGE SOURCES
	ANALOG_INPUT(V12, 12)
	ANALOG_INPUT(V5, 5)
	ANALOG_INPUT(V6, 6)

	// --------------------------------
	INCLUDE(brdrline_schematics)

	// --------------------------------
	TTL_INPUT(I_SOUND_0, 1) // active low
	NET_C(GND, I_SOUND_0.GND)
	NET_C(V5,  I_SOUND_0.VCC)

	TTL_INPUT(I_SOUND_1, 1) // active low
	NET_C(GND, I_SOUND_1.GND)
	NET_C(V5,  I_SOUND_1.VCC)

	TTL_INPUT(I_SOUND_2, 1) // active low
	NET_C(GND, I_SOUND_2.GND)
	NET_C(V5,  I_SOUND_2.VCC)

	TTL_INPUT(I_SOUND_3, 1) // active low
	NET_C(GND, I_SOUND_3.GND)
	NET_C(V5,  I_SOUND_3.VCC)

	TTL_INPUT(I_SOUND_4, 1) // active low
	NET_C(GND, I_SOUND_4.GND)
	NET_C(V5,  I_SOUND_4.VCC)

	TTL_INPUT(I_SOUND_5, 1) // active low
	NET_C(GND, I_SOUND_5.GND)
	NET_C(V5,  I_SOUND_5.VCC)

	TTL_INPUT(I_SOUND_6, 1) // active low
	NET_C(GND, I_SOUND_6.GND)
	NET_C(V5,  I_SOUND_6.VCC)

	TTL_INPUT(I_SOUND_7, 1) // active low
	NET_C(GND, I_SOUND_7.GND)
	NET_C(V5,  I_SOUND_7.VCC)


#if 1
	// {---(A5)Logic Board---}{-(C1/2)Top-}{-Sound Board-}
	// D0 -> LS374( 3- 2) -> 40 -> PNK ->  9 -> ANIMAL_TRG
	// D1 -> LS374(18-19) -> 35 -> BRN -> 12 -> CRY_TRG
	// D2 -> LS374( 4- 5) -> 33 -> ORN -> 11 -> WALK_TRG
	// D3 -> LS374(17-16) -> 34 -> YEL -> 10 -> EMAR_TRG
	// D4 -> LS374( 7- 6) -> 32 -> BLU ->  8 -> HIT_TRG
	// D5 -> LS374(14-15) -> 31 -> GRY ->  7 -> POINT_TRG
	// D6 -> LS374( 8- 9) -> 30 -> WHT ->  6 -> JEEP_ON
	// D7 -> LS374(13-12) -> 29 -> GRN ->  5 -> GUN_TRG
	ALIAS(GUN_TRG,    I_SOUND_7.Q)
	ALIAS(JEEP_ON,    I_SOUND_6.Q)
	ALIAS(POINT_TRG,  I_SOUND_5.Q)
	ALIAS(HIT_TRG,    I_SOUND_4.Q)
	ALIAS(ANIMAL_TRG, I_SOUND_0.Q)
	ALIAS(EMAR_TRG,   I_SOUND_3.Q)
	ALIAS(WALK_TRG,   I_SOUND_2.Q)
	ALIAS(CRY_TRG,    I_SOUND_1.Q)
#else
	/*
	 *   2020-10-06 by 'beta-tester'
	 *      |                         |                  ||personal     |personal     |
	 *      |                         |                  ||assignment   |assignment   |
	 *      |brdrline                 |starrkr           ||NL SOUND     |plausibility |note
	 *   ---+-------------------------+------------------++-------------+-------------+----
	 *   D0 |                         |fire, jeep_field  ||POINT_TRG.IN |             |
	 *   D1 |next_sector, hit_rocket2 |hit_animal        ||HIT_TRG.IN   |+            |
	 *   D2 |                         |fire, next_sector ||WALK_TRG.IN  |             |
	 *   D3 |                         |fire              ||CRY_TRG.IN   |             |
	 *   D4 |jeep_field               |                  ||ANIMAL_TRG.IN|+++          |see note 1
	 *   D5 |fire                     |                  ||GUN_TRG.IN   |++           |
	 *   D6 |jeep_path                |jeep_path?        ||JEEP_ON.IN   |++           |
	 *   D7 |hit_animal               |                  ||EMAR_TRG.IN  |+            |
	 *
	 *   note 1: as far as i remember (from the early 1980'th) it was triggered more often while crawling through the field at sector2 & 3,
	 *    issue in schematic/netlist?
	 *          or trigger?
	 *          or were the acrade what i played in the past a modified bootleg?
	 *          or is it only in my head?
	 */
	ALIAS(GUN_TRG,    I_SOUND_5.Q)
	ALIAS(JEEP_ON,    I_SOUND_6.Q)
	ALIAS(POINT_TRG,  I_SOUND_0.Q)
	ALIAS(HIT_TRG,    I_SOUND_1.Q)
	ALIAS(ANIMAL_TRG, I_SOUND_4.Q)
	ALIAS(EMAR_TRG,   I_SOUND_7.Q)
	ALIAS(WALK_TRG,   I_SOUND_2.Q)
	ALIAS(CRY_TRG,    I_SOUND_3.Q)
#endif

	// --------------------------------
	INCLUDE(brdrline_sound_out)


	// --------------------------------
	ALIAS(OUTPUT, SOUND_OUT)

NETLIST_END()




#if 0
~/mame/src/lib/netlist/devices/nld_ne555.cpp
NE555()
 *                 +--------------+
 *       .GND/*1*/ |1     ++     8| .VCC/*8*/
 *      .TRIG/*2*/ |2            7| .DISCH/*7*/
 *       .OUT/*3*/ |3            6| .THRESH/*6*/
 *     .RESET/*4*/ |4   NE555    5| .CONT/*5*/
 *                 +--------------+
#endif

#if 0
~/mame/src/lib/netlist/devices/nld_74123.cpp
TTL_74123_DIP()
 *             +--------------+
 *    .1/*A1*/ |1     ++    16| .16/*VCC*/
 *    .2/*B1*/ |2           15| .15/*RC1*/
 * .3/*CLRQ1*/ |3           14| .14/*C1*/
 *   .4/*QQ1*/ |4   74123   13| .13/*Q1*/
 *    .5/*Q2*/ |5           12| .12/*QQ2*/
 *    .6/*C2*/ |6           11| .11/*CLRQ2*/
 *   .7/*RC2*/ |7           10| .10/*B2*/
 *   .8/*GND*/ |8            9| .9/*A2*/
 *             +--------------+
#endif

#if 0
~/mame/src/lib/netlist/macro/nlm_opamp_lib.cpp
LM324_DIP()
OPAMP(A, "LM324")
 *              +--------------+
 *   .1/*OUT1*/ |1     ++    14| .14/*OUT4*/
 * .2/*MINUS1*/ |2           13| .13/*MINUS4*/
 *  .3/*PLUS1*/ |3           12| .12/*PLUS4*/
 *    .4/*VCC*/ |4           11| .11/*GND*/
 *  .5/*PLUS2*/ |5           10| .10/*PLUS3*/
 * .6/*MINUS2*/ |6            9| .9/*MINUS3*/
 *   .7/*OUT2*/ |7   LM324    8| .8/*OUT3*/
 *              +--------------+
#endif

#if 0
WARNING: fake implementation for MB4391 based on guesses only
MB4391_DIP
MB4391(A)
 *            +--------------+
 *  .1/*IN1*/ |1     ++    16| .16/*VCC1*/
 * .2/*CON1*/ |2           15| .15/*OUT1*/
 * .3/*GND1*/ |3           14| .14/*RO1*/
 *            |4   MB4391  13|
 *  .5/*IN2*/ |5           12| .12/*VCC2*/
 * .6/*CON2*/ |6           11| .11/*OUT2*/
 * .7/*GND2*/ |7           10| .10/*RO2*/
 *            |8            9|
 *            +--------------+
#endif

#if 0
~/mame/src/lib/netlist/devices/nld_7474.cpp
TTL_7474_DIP() - have to use pin numbers... why?
 *            +--------------+
 * .1/*CLR1*/ |1     ++    14| .14/*VCC*/
 *   .2/*D1*/ |2           13| .13/*CLR2*/
 * .3/*CLK1*/ |3           12| .12/*D2*/
 *  .4/*PR1*/ |4    7474   11| .11/*CLK2*/
 *   .5/*Q1*/ |5           10| .10/*PR2*/
 *  .6/*QQ1*/ |6            9| .9/*Q2*/
 *  .7/*GND*/ |7            8| .8/*QQ2*/
 *            +--------------+
#endif

#if 0
~/mame/src/lib/netlist/devices/nld_mm5837.cpp
MM5837()
 *                +--------------+
 *       .VDD/*1*/ |1     ++     8| .NC
 *       .VGG/*2*/ |2            7| .NC
 *       .OUT/*3*/ |3            6| .NC
 *       .VSS/*4*/ |4   MM5837   5| .NC
 *                +--------------+
#endif

#if 0
~/mame/src/lib/netlist/macro/nlm_ttl74xx_lib.cpp
TTL_7408_DIP()
TTL_7408_AND(A)
 *            +--------------+
 *   .1/*A1*/ |1     ++    14| .14/*VCC*/
 *   .2/*B1*/ |2           13| .13/*B4*/
 *   .3/*Q1*/ |3           12| .12/*A4*/
 *   .4/*A2*/ |4    7408   11| .11/*Q4*/
 *   .5/*B2*/ |5           10| .10/*B3*/
 *   .6/*Q2*/ |6            9| .9/*A3*/
 *  .7/*GND*/ |7            8| .8/*Q3*/
 *            +--------------+
#endif

#if 0
TTL_7416_DIP()
TTL_7416_GATE(A)
 *            +--------------+
 *   .1/*A1*/ |1     ++    14| .14/*VCC*/
 *   .2/*Q1*/ |2           13| .13/*A6*/
 *   .3/*A2*/ |3           12| .12/*Q6*/
 *   .4/*Q2*/ |4    7416   11| .11/*A5*/
 *   .5/*A3*/ |5           10| .10/*Q5*/
 *   .6/*Q3*/ |6            9| .9/*A4*/
 *  .7/*GND*/ |7            8| .8/*Q4*/
 *            +--------------+
#endif

