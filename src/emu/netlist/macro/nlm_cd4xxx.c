
#include "nlm_cd4xxx.h"

#include "devices/nld_truthtable.h"
#include "devices/nld_system.h"
#include "devices/nld_4020.h"
#include "devices/nld_4066.h"

/*
 *   CD4001BC: Quad 2-Input NOR Buffered B Series Gate
 *
 *       +--------------+
 *    A1 |1     ++    14| VCC
 *    B1 |2           13| A6
 *    A2 |3           12| Y6
 *    Y2 |4    4001   11| A5
 *    A3 |5           10| Y5
 *    Y3 |6            9| A4
 *   GND |7            8| Y4
 *       +--------------+
 *
 */

NETLIST_START(CD4001_DIP)
	CD4001_NOR(s1)
	CD4001_NOR(s2)
	CD4001_NOR(s3)
	CD4001_NOR(s4)

	DUMMY_INPUT(VSS)
	DUMMY_INPUT(VDD)

	DIPPINS(   /*       +--------------+      */
		s1.A,  /*    A1 |1     ++    14| VCC  */ VSS.I,
		s1.B,  /*    B1 |2           13| A6   */ s4.B,
		s1.Q,  /*    A2 |3           12| Y6   */ s4.A,
		s2.Q,  /*    Y2 |4    4001   11| A5   */ s4.Q,
		s2.A,  /*    A3 |5           10| Y5   */ s3.Q,
		s2.B,  /*    Y3 |6            9| A4   */ s3.B,
		VDD.I, /*   GND |7            8| Y4   */ s3.A
				/*       +--------------+      */
	)

NETLIST_END()

/*  CD4020: 14-Stage Ripple Carry Binary Counters
 *
 *          +--------------+
 *      Q12 |1     ++    16| VDD
 *      Q13 |2           15| Q11
 *      Q14 |3           14| Q10
 *       Q6 |4    4020   13| Q8
 *       Q5 |5           12| Q9
 *       Q7 |6           11| RESET
 *       Q4 |7           10| IP (Input pulses)
 *      VSS |8            9| Q1
 *          +--------------+
 *
 *  Naming conventions follow Texas Instruments datasheet
 *
 *  FIXME: Timing depends on VDD-VSS
 *         This needs a cmos d-a/a-d proxy implementation.
 */

NETLIST_START(CD4020_DIP)

	CD4020(s1)
	DIPPINS(     /*       +--------------+       */
		s1.Q12,  /*   Q12 |1     ++    16| VDD   */ s1.VDD,
		s1.Q13,  /*   Q13 |2           15| Q11   */ s1.Q11,
		s1.Q14,  /*   Q14 |3           14| Q10   */ s1.Q10,
		s1.Q6,   /*    Q6 |4    4020   13| Q8    */ s1.Q8,
		s1.Q5,   /*    Q5 |5           12| Q9    */ s1.Q9,
		s1.Q7,   /*    Q7 |6           11| RESET */ s1.RESET,
		s1.Q4,   /*    Q4 |7           10| IP    */ s1.IP,
		s1.VSS,  /*   VSS |8            9| Q1    */ s1.Q1
					/*       +--------------+       */
	)
		/*
		 * IP = (Input pulses)
		 */

NETLIST_END()

/*  CD4066: Quad Bilateral Switch
 *
 *          +--------------+
 *   INOUTA |1     ++    14| VDD
 *   OUTINA |2           13| CONTROLA
 *   OUTINB |3           12| CONTROLD
 *   INOUTB |4    4066   11| INOUTD
 * CONTROLB |5           10| OUTIND
 * CONTROLC |6            9| OUTINC
 *      VSS |7            8| INOUTC
 *          +--------------+
 *
 *  FIXME: These devices are slow (~125 ns). THis is currently not reflected
 *
 *  Naming conventions follow National semiconductor datasheet
 *
 */

NETLIST_START(CD4066_DIP)
	CD4066_GATE(A)
	CD4066_GATE(B)
	CD4066_GATE(C)
	CD4066_GATE(D)

	NET_C(A.PS.VDD, B.PS.VDD, C.PS.VDD, D.PS.VDD)
	NET_C(A.PS.VSS, B.PS.VSS, C.PS.VSS, D.PS.VSS)

	PARAM(A.BASER, 270.0)
	PARAM(B.BASER, 270.0)
	PARAM(C.BASER, 270.0)
	PARAM(D.BASER, 270.0)

	DIPPINS(      /*          +--------------+          */
		A.R.1,    /*   INOUTA |1     ++    14| VDD      */ A.PS.VDD,
		A.R.2,    /*   OUTINA |2           13| CONTROLA */ A.CTL,
		B.R.1,    /*   OUTINB |3           12| CONTROLD */ D.CTL,
		B.R.2,    /*   INOUTB |4    4066   11| INOUTD   */ D.R.1,
		B.CTL,    /* CONTROLB |5           10| OUTIND   */ D.R.2,
		C.CTL,    /* CONTROLC |6            9| OUTINC   */ C.R.1,
		A.PS.VSS, /*      VSS |7            8| INOUTC   */ C.R.2
					/*          +--------------+          */
	)
NETLIST_END()

NETLIST_START(CD4016_DIP)
	CD4066_GATE(A)
	CD4066_GATE(B)
	CD4066_GATE(C)
	CD4066_GATE(D)

	NET_C(A.PS.VDD, B.PS.VDD, C.PS.VDD, D.PS.VDD)
	NET_C(A.PS.VSS, B.PS.VSS, C.PS.VSS, D.PS.VSS)

	PARAM(A.BASER, 1000.0)
	PARAM(B.BASER, 1000.0)
	PARAM(C.BASER, 1000.0)
	PARAM(D.BASER, 1000.0)

	DIPPINS(      /*          +--------------+          */
		A.R.1,    /*   INOUTA |1     ++    14| VDD      */ A.PS.VDD,
		A.R.2,    /*   OUTINA |2           13| CONTROLA */ A.CTL,
		B.R.1,    /*   OUTINB |3           12| CONTROLD */ D.CTL,
		B.R.2,    /*   INOUTB |4    4016   11| INOUTD   */ D.R.1,
		B.CTL,    /* CONTROLB |5           10| OUTIND   */ D.R.2,
		C.CTL,    /* CONTROLC |6            9| OUTINC   */ C.R.1,
		A.PS.VSS, /*      VSS |7            8| INOUTC   */ C.R.2
					/*          +--------------+          */
	)
NETLIST_END()

NETLIST_START(CD4XXX_lib)

	TRUTHTABLE_START(CD4001_NOR, 2, 1, 0, "")
		TT_HEAD("A , B | Q ")
		TT_LINE("0,0|1|85")
		TT_LINE("X,1|0|120")
		TT_LINE("1,X|0|120")
		TT_FAMILY("CD4XXX")
	TRUTHTABLE_END()

	LOCAL_LIB_ENTRY(CD4001_DIP)

	/* DIP ONLY */
	LOCAL_LIB_ENTRY(CD4020_DIP)
	LOCAL_LIB_ENTRY(CD4016_DIP)
	LOCAL_LIB_ENTRY(CD4066_DIP)

NETLIST_END()
