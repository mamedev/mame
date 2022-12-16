// license:CC0
// copyright-holders:Couriersud

/*
 * This example illustrates how to use netlist to derive RGB output
 * levels from complex output circuits.
 *
 * Create x.cpp:

    #include <cstdio>

    int main()
    {
        // Change IC86 output every micro second starting at 1ms
        for (std::size_t i=0;i<256;i++)
          printf("%.9f,IC86.D.IN,%d\n", (double) i / 1.0e6 + 1.0e-3, (int)i);
    }

 * Run these commands:
 *
 * c++ x.cpp
 * ./a.out > src/lib/netlist/examples/turkey_shoot.csv
 * ./nltool -c run -t 0.0013 -f src/lib/netlist/examples/turkey_shoot.cpp -i src/lib/netlist/examples/turkey_shoot.csv -l BLUE
 * ./nlwav -f tab -o x.tab -s 0.0010005 -i 0.000001 -n 256 log_BLUE.log
 *
 * x.tab now contains 256 values representing the different output levels:
 * low 4 bits: color value
 * high 4 bits: attenuation value
 *
 */


#include "netlist/devices/net_lib.h"

/* ----------------------------------------------------------------------------
 *  Define
 * ---------------------------------------------------------------------------*/

/* set to 1 to use optimizations increasing performance significantly */

#ifndef USE_OPTIMIZATIONS
#define USE_OPTIMIZATIONS 0
#endif

/* ----------------------------------------------------------------------------
 *  Library section header START
 * ---------------------------------------------------------------------------*/

#ifndef __PLIB_PREPROCESSOR__

#define SHIM74LS374_DIP(_name)                                                     \
		NET_REGISTER_DEV_X(SHIM74LS374_DIP, _name)
#endif

/* ----------------------------------------------------------------------------
 *  Library section header END
 * ---------------------------------------------------------------------------*/


NETLIST_START(turkey_shoot_vga)
{
//  EESCHEMA NETLIST VERSION 1.1 (SPICE FORMAT) CREATION DATE: WED 01 JUL 2015 11:09:25 PM CEST
//  TO EXCLUDE A COMPONENT FROM THE SPICE NETLIST ADD [SPICE_NETLIST_ENABLED] USER FIELD SET TO: N
//  TO REORDER THE COMPONENT SPICE NODE SEQUENCE ADD [SPICE_NODE_SEQUENCE] USER FIELD AND DEFINE SEQUENCE: 2,1,0
// SHEET NAME:/
// IGNORED O_AUDIO0: O_AUDIO0  64 0
// .END

	PARAM(Solver.RELTOL, 1e-2)
	PARAM(Solver.VNTOL, 1e-6)
	PARAM(Solver.NR_LOOPS, 30)
	PARAM(Solver.GS_LOOPS, 99)
	PARAM(Solver.METHOD, "MAT_CR")
	PARAM(Solver.PARALLEL, 0)

	//PARAM(Solver.METHOD, "GMRES")
	//PARAM(Solver.ACCURACY, 1e-5)
	//PARAM(Solver.METHOD, "SOR")

#if USE_OPTIMIZATIONS
	SOLVER(Solver, 48000)
#else
	SOLVER(Solver, 48000)
	PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.PARALLEL, 0)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 2e-8)
	PARAM(Solver.DYNAMIC_LTE, 1e-4)
#endif

	NET_MODEL("2N3904 NPN(IS=1E-14 VAF=100 Bf=300 IKF=0.4 XTB=1.5 BR=4 CJC=4E-12 CJE=8E-12 RB=20 RC=0.1 RE=0.1 TR=250E-9 TF=350E-12 ITF=1 VTF=2 XTF=3 Vceo=40 Icrating=200m mfg=Philips)")

	LOCAL_SOURCE(SHIM74LS374_DIP)
	LOCAL_LIB_ENTRY(SHIM74LS374_DIP)

	INCLUDE(turkey_shoot_schematics)

}

NETLIST_START(turkey_shoot_schematics)
{

	ANALOG_INPUT(I_V12, 12)
	ANALOG_INPUT(I_V5, 5)

	TTL_INPUT(BLANK, 0)

	IND(L1, 0.0000047)

	CAP(C60, CAP_U(0.1))
	CAP(C54, CAP_P(47))
	CAP(C57, CAP_P(47))

	RES(R32, RES_K(1))
	RES(R33, RES_K(1))
	RES(R34, 390)
	RES(R6, RES_K(8.2))
	RES(R7, RES_K(3.9))
	RES(R8, RES_K(2))
	RES(R9, RES_K(1))
	RES(R10, RES_K(8.2))
	RES(R11, RES_K(3.9))
	RES(R12, RES_K(2))
	RES(R13, RES_K(1))

	RES(R35, RES_K(2.2))
	RES(R36, RES_K(2.7))
	RES(R31, 270)
	RES(R30, RES_K(2))
	RES(R29, RES_K(2))
	RES(R28, RES_K(2))
	RES(R22, 470)
	RES(R23, 47)

	DIODE(D1, "1N4148")
	DIODE(D2, "1N4148")
	DIODE(D3, "1N4148")
	DIODE(D4, "1N4148")
	DIODE(D5, "1N4148")
	DIODE(D6, "1N4148")
	DIODE(D7, "1N4148")
	DIODE(D8, "1N4148")
	DIODE(D17, "1N4148")

	QBJT_EB(Q3, "2N3904")
	QBJT_EB(Q4, "2N3904")
	QBJT_EB(Q5, "2N3904")

	SHIM74LS374_DIP(IC86)

	NET_C(D8.K, IC86.19)
	NET_C(D7.K, IC86.16)
	NET_C(D6.K, IC86.15)
	NET_C(D5.K, IC86.12)
	NET_C(D4.K, IC86.9)
	NET_C(D3.K, IC86.6)
	NET_C(D2.K, IC86.5)
	NET_C(D1.K, IC86.2)

	NET_C(D8.A, R13.1)
	NET_C(D7.A, R12.1)
	NET_C(D6.A, R11.1)
	NET_C(D5.A, R10.1)
	NET_C(D4.A, R9.1)
	NET_C(D3.A, R8.1)
	NET_C(D2.A, R7.1)
	NET_C(D1.A, R6.1)

	NET_C(BLANK, C57.1, R32.1)
	NET_C(C57.2, R32.2, R33.1, Q5.B)
	NET_C(GND, R33.2, Q5.E, R36.2, C60.2)
	NET_C(Q5.C, R34.1)
	NET_C(R34.2, R13.2, R12.2, R11.2, R10.2, R35.2, R36.1, Q4.B)
	NET_C(R35.1, Q4.C, C60.1, L1.2)

	NET_C(I_V12, L1.1)

	NET_C(Q4.E, R30.1, R29.1, R28.1, R31.1)

	NET_C(GND, R31.2, R29.2, R28.2)
	NET_C(R6.2, R7.2, R8.2, R9.2, R30.2, Q3.B)
	NET_C(I_V5, Q3.C)
	NET_C(GND, C54.2, R22.2)
	NET_C(Q3.E, D17.A)
	NET_C(D17.K, R22.1, R23.1)
	NET_C(R23.2, C54.1)

	NET_C(I_V5, IC86.20, BLANK.VCC)
	NET_C(GND, IC86.10, BLANK.GND)

	ALIAS(BLUE, C54.1)

}


NETLIST_START(SHIM74LS374_DIP)
{

	NET_MODEL("SPECIAL FAMILY(IVL=0.16 IVH=0.4 OVL=0.1 OVH=0.05 ORL=10.0 ORH=1.0e10)")
	//LOGIC_INPUT8(D, 0, "74XX")
	//LOGIC_INPUT8(D, 0, "74XXOC")
	LOGIC_INPUT8(D, 0, "SPECIAL")
	ALIAS(20, D.VCC)
	ALIAS(10, D.GND)
	ALIAS( 2, D.Q0)
	ALIAS( 5, D.Q1)
	ALIAS( 6, D.Q2)
	ALIAS( 9, D.Q3)
	ALIAS(12, D.Q4)
	ALIAS(15, D.Q5)
	ALIAS(16, D.Q6)
	ALIAS(19, D.Q7)

}

