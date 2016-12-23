// license:GPL-2.0+
// copyright-holders:Couriersud
#include "nlm_opamp.h"

#include "analog/nld_opamps.h"
#include "devices/nld_system.h"

/*
 *   Generic layout with 4 opamps, VCC on pin 4 and GND on pin 11
 */

static NETLIST_START(opamp_layout_4_4_11)
	DIPPINS(        /*   +--------------+   */
		A.OUT,      /*   |1     ++    14|   */ D.OUT,
		A.MINUS,    /*   |2           13|   */ D.MINUS,
		A.PLUS,     /*   |3           12|   */ D.PLUS,
		A.VCC,      /*   |4           11|   */ A.GND,
		B.PLUS,     /*   |5           10|   */ C.PLUS,
		B.MINUS,    /*   |6            9|   */ C.MINUS,
		B.OUT,      /*   |7            8|   */ C.OUT
					/*   +--------------+   */
	)
	NET_C(A.GND, B.GND, C.GND, D.GND)
	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
NETLIST_END()

/*
 *   Generic layout with 2 opamps, VCC on pin 8 and GND on pin 4
 */

static NETLIST_START(opamp_layout_2_8_4)
	DIPPINS(        /*   +--------------+   */
		A.OUT,      /*   |1     ++     8|   */ A.VCC,
		A.MINUS,    /*   |2            7|   */ B.OUT,
		A.PLUS,     /*   |3            6|   */ B.MINUS,
		A.GND,      /*   |4            5|   */ B.PLUS
					/*   +--------------+   */
	)
	NET_C(A.GND, B.GND)
	NET_C(A.VCC, B.VCC)
NETLIST_END()

/*
 *   Generic layout with 1 opamp, VCC+ on pin 7, VCC- on pin 4 and compensation
 */

static NETLIST_START(opamp_layout_1_7_4)
	DIPPINS(        /*   +--------------+   */
		OFFSET.N1,  /*   |1     ++     8|   */ NC,
		MINUS,      /*   |2            7|   */ VCC.PLUS,
		PLUS,       /*   |3            6|   */ OUT,
		VCC.MINUS,  /*   |4            5|   */ OFFSET.N2
					/*   +--------------+   */
	)
	NET_C(A.GND, VCC.MINUS)
	NET_C(A.VCC, VCC.PLUS)
	NET_C(A.MINUS, MINUS)
	NET_C(A.PLUS, PLUS)
	NET_C(A.OUT, OUT)
NETLIST_END()

/*
 *   Generic layout with 1 opamp, VCC+ on pin 8, VCC- on pin 5 and compensation
 */

static NETLIST_START(opamp_layout_1_8_5)
	DIPPINS(        /*   +--------------+   */
		NC.1,       /*   |1           10|   */ NC.3,
		OFFSET.N1,  /*   |2            9|   */ NC.2,
		MINUS,      /*   |3            8|   */ VCC.PLUS,
		PLUS,       /*   |4            7|   */ OUT,
		VCC.MINUS,  /*   |5            6|   */ OFFSET.N2
					/*   +--------------+   */
	)
	NET_C(A.GND, VCC.MINUS)
	NET_C(A.VCC, VCC.PLUS)
	NET_C(A.MINUS, MINUS)
	NET_C(A.PLUS, PLUS)
	NET_C(A.OUT, OUT)
NETLIST_END()

/*
 *   Generic layout with 1 opamp, VCC+ on pin 11, VCC- on pin 6 and compensation
 */

static NETLIST_START(opamp_layout_1_11_6)
	DIPPINS(        /*   +--------------+   */
		NC.1,       /*   |1     ++    14|   */ NC.7,
		NC.2,       /*   |2           13|   */ NC.6,
		OFFSET.N1,  /*   |3           12|   */ NC.5,
		MINUS,      /*   |4           11|   */ VCC.PLUS,
		PLUS,       /*   |5           10|   */ OUT,
		VCC.MINUS,  /*   |6            9|   */ OFFSET.N2,
		NC.3,       /*   |7            8|   */ NC.4
					/*   +--------------+   */
	)
	NET_C(A.GND, VCC.MINUS)
	NET_C(A.VCC, VCC.PLUS)
	NET_C(A.MINUS, MINUS)
	NET_C(A.PLUS, PLUS)
	NET_C(A.OUT, OUT)
NETLIST_END()

static NETLIST_START(MB3614_DIP)
	OPAMP(A, "MB3614")
	OPAMP(B, "MB3614")
	OPAMP(C, "MB3614")
	OPAMP(D, "MB3614")

	INCLUDE(opamp_layout_4_4_11)

NETLIST_END()

static NETLIST_START(LM324_DIP)
	OPAMP(A, "LM324")
	OPAMP(B, "LM324")
	OPAMP(C, "LM324")
	OPAMP(D, "LM324")

	INCLUDE(opamp_layout_4_4_11)

NETLIST_END()

static NETLIST_START(UA741_DIP8)
	OPAMP(A, "UA741")

	INCLUDE(opamp_layout_1_7_4)

NETLIST_END()

static NETLIST_START(UA741_DIP10)
	OPAMP(A, "UA741")

	INCLUDE(opamp_layout_1_8_5)

NETLIST_END()

static NETLIST_START(UA741_DIP14)
	OPAMP(A, "UA741")

	INCLUDE(opamp_layout_1_11_6)

NETLIST_END()

static NETLIST_START(LM358_DIP)
	OPAMP(A, "LM358")
	OPAMP(B, "LM358")

	INCLUDE(opamp_layout_2_8_4)

NETLIST_END()

NETLIST_START(OPAMP_lib)
	LOCAL_LIB_ENTRY(opamp_layout_4_4_11)
	LOCAL_LIB_ENTRY(opamp_layout_2_8_4)
	LOCAL_LIB_ENTRY(opamp_layout_1_7_4)
	LOCAL_LIB_ENTRY(opamp_layout_1_8_5)
	LOCAL_LIB_ENTRY(opamp_layout_1_11_6)

	NET_MODEL("LM324       OPAMP(TYPE=3 VLH=2.0 VLL=0.2 FPF=5 UGF=500k SLEW=0.3M RI=1000k RO=50 DAB=0.00075)")
	NET_MODEL("LM358       OPAMP(TYPE=3 VLH=2.0 VLL=0.2 FPF=5 UGF=500k SLEW=0.3M RI=1000k RO=50 DAB=0.001)")
	NET_MODEL("MB3614      OPAMP(TYPE=3 VLH=1.4 VLL=0.02 FPF=2 UGF=500k SLEW=0.6M RI=1000k RO=50 DAB=0.0002)")
	NET_MODEL("UA741       OPAMP(TYPE=3 VLH=1.0 VLL=1.0 FPF=5 UGF=1000k SLEW=0.5M RI=2000k RO=75 DAB=0.0017)")

	LOCAL_LIB_ENTRY(MB3614_DIP)
	LOCAL_LIB_ENTRY(LM324_DIP)
	LOCAL_LIB_ENTRY(LM358_DIP)
	LOCAL_LIB_ENTRY(UA741_DIP8)
	LOCAL_LIB_ENTRY(UA741_DIP10)
	LOCAL_LIB_ENTRY(UA741_DIP14)

NETLIST_END()
