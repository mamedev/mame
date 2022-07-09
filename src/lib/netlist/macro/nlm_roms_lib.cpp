// license:CC0
// copyright-holders:Couriersud

#include "devices/net_lib.h"


//- Identifier:  PROM_82S126_DIP
//- Title: 82S126 1K-bit TTL bipolar PROM
//- Pinalias: A6,A5,A4,A3,A0,A1,A2,GND,O4,O3,O2,O1,CE1Q,CE2Q,A7,VCC
//- Package: DIP
//- Param: ROM
//-    The name of the source to load the rom content from
//- Param: FORCE_TRISTATE_LOGIC
//-    Set this parameter to 1 force tristate outputs into logic mode.
//-    This should be done only if the device enable inputs are connected
//-    in a way which always enables the device.
//- Param: MODEL
//-    Overwrite the default model of the device. Use with care.
//- NamingConvention: Naming conventions follow Philips Components-Signetics datasheet
//- Limitations:
//-    Currently OC is not supported.
//-
//- Example: 82S126.cpp,82S126_example
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheet/philips/82S129.pdf
//-

static NETLIST_START(PROM_82S126_DIP)
{

	PROM_82S126(A)

	DEFPARAM(ROM, "unknown")
	DEFPARAM(FORCE_TRISTATE_LOGIC, 0)
	DEFPARAM(MODEL, "$(@.A.MODEL)")
	PARAM(A.ROM, "$(@.ROM)")
	PARAM(A.FORCE_TRISTATE_LOGIC, "$(@.FORCE_TRISTATE_LOGIC)")
	PARAM(A.MODEL, "$(@.MODEL)")
	ALIAS(1, A.A6)
	ALIAS(2, A.A5)
	ALIAS(3, A.A4)
	ALIAS(4, A.A3)
	ALIAS(5, A.A0)
	ALIAS(6, A.A1)
	ALIAS(7, A.A2)
	ALIAS(8, A.GND)
	ALIAS(9, A.O4)
	ALIAS(10, A.O3)
	ALIAS(11, A.O2)
	ALIAS(12, A.O1)
	ALIAS(13, A.CE1Q)
	ALIAS(14, A.CE2Q)
	ALIAS(15, A.A7)
	ALIAS(16, A.VCC)
}

//- Identifier:  PROM_74S287_DIP
//- Title: 74S287 (256 x 4) 1024-Bit TTL PROM
//- Pinalias: A6,A5,A4,A3,A0,A1,A2,GND,O3,O2,O1,O0,CE1Q,CE2Q,A7,VCC
//- Package: DIP
//- Param: ROM
//-    The name of the source to load the rom content from
//- Param: FORCE_TRISTATE_LOGIC
//-    Set this parameter to 1 force tristate outputs into logic mode.
//-    This should be done only if the device enable inputs are connected
//-    in a way which always enables the device.
//- Param: MODEL
//-    Overwrite the default model of the device. Use with care.
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- Limitations:
//-    None.
//-
//- Example: 74S287.cpp,74S287_example
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheet_pdf/national-semiconductor/DM54S287AJ_to_DM74S287V.pdf
//-

static NETLIST_START(PROM_74S287_DIP)
{

	PROM_74S287(A)

	DEFPARAM(ROM, "unknown")
	DEFPARAM(FORCE_TRISTATE_LOGIC, 0)
	DEFPARAM(MODEL, "$(@.A.MODEL)")
	PARAM(A.ROM, "$(@.ROM)")
	PARAM(A.FORCE_TRISTATE_LOGIC, "$(@.FORCE_TRISTATE_LOGIC)")
	PARAM(A.MODEL, "$(@.MODEL)")
	ALIAS(1, A.A6)
	ALIAS(2, A.A5)
	ALIAS(3, A.A4)
	ALIAS(4, A.A3)
	ALIAS(5, A.A0)
	ALIAS(6, A.A1)
	ALIAS(7, A.A2)
	ALIAS(8, A.GND)
	ALIAS(9, A.O3)
	ALIAS(10, A.O2)
	ALIAS(11, A.O1)
	ALIAS(12, A.O0)
	ALIAS(13, A.CE1Q)
	ALIAS(14, A.CE2Q)
	ALIAS(15, A.A7)
	ALIAS(16, A.VCC)
}

//- Identifier:  PROM_82S123_DIP
//- Title: 82S123 256 bit TTL bipolar PROM
//- Pinalias: O1,O2,O3,O4,O5,O6,O7,GND,O8,A0,A1,A2,A3,A4,CEQ,VCC
//- Package: DIP
//- Param: ROM
//-    The name of the source to load the rom content from
//- Param: FORCE_TRISTATE_LOGIC
//-    Set this parameter to 1 force tristate outputs into logic mode.
//-    This should be done only if the device enable inputs are connected
//-    in a way which always enables the device.
//- Param: MODEL
//-    Overwrite the default model of the device. Use with care.
//- NamingConvention: Naming conventions follow Philips Components-Signetics datasheet
//- Limitations:
//-    Currently OC is not supported.
//-
//- Example: 82S123.cpp,82S123_example
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheet/philips/82S123.pdf
//-

static NETLIST_START(PROM_82S123_DIP)
{

	PROM_82S123(A)

	DEFPARAM(ROM, "unknown")
	DEFPARAM(FORCE_TRISTATE_LOGIC, 0)
	DEFPARAM(MODEL, "$(@.A.MODEL)")
	PARAM(A.ROM, "$(@.ROM)")
	PARAM(A.FORCE_TRISTATE_LOGIC, "$(@.FORCE_TRISTATE_LOGIC)")
	PARAM(A.MODEL, "$(@.MODEL)")
	ALIAS(1, A.O0)
	ALIAS(2, A.O1)
	ALIAS(3, A.O2)
	ALIAS(4, A.O3)
	ALIAS(5, A.O4)
	ALIAS(6, A.O5)
	ALIAS(7, A.O6)
	ALIAS(8, A.GND)

	ALIAS(9, A.O7)
	ALIAS(10, A.A0)
	ALIAS(11, A.A1)
	ALIAS(12, A.A2)
	ALIAS(13, A.A3)
	ALIAS(14, A.A4)
	ALIAS(15, A.CEQ)
	ALIAS(16, A.VCC)
}

/*
 * nld_82S16.h
 *
 *  DM82S16: 256 Bit bipolar ram
 *
 *          +--------------+
 *       A1 |1     ++    16| VCC
 *       A0 |2           15| A2
 *     CE1Q |3           14| A3
 *     CE2Q |4   82S16   13| DIN
 *     CE3Q |5           12| WEQ
 *    DOUTQ |6           11| A7
 *       A4 |7           10| A6
 *      GND |8            9| A5
 *          +--------------+
 *
 *
 *  Naming conventions follow Signetics datasheet
 */


//- Identifier:  EPROM_2716_DIP
//- Title: 2716 16K (2K x 8) UV ERASABLE PROM
//- Pinalias: A7,A6,A6,A4,A4,A2,A1,A0,O0,O1,O2,GND,O3,O4,O5,O6,O7,CE1Q/CE,A10,CE2Q/OE,VPP,A9,A8,VCC
//- Package: DIP
//- Param: ROM
//-    The name of the source to load the rom content from
//- Param: FORCE_TRISTATE_LOGIC
//-    Set this parameter to 1 force tristate outputs into logic mode.
//-    This should be done only if the device enable inputs are connected
//-    in a way which always enables the device.
//- Param: MODEL
//-    Overwrite the default model of the device. Use with care.
//- NamingConvention: Naming conventions follow Intel datasheet
//- Limitations:
//-    Currently OC is not supported.
//-
//- Example: 2716.cpp,2716_example
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheets/400/500340_DS.pdf
//-
static NETLIST_START(EPROM_2716_DIP)
{

	EPROM_2716(A)

	DEFPARAM(ROM, "unknown")
	DEFPARAM(FORCE_TRISTATE_LOGIC, 0)
	DEFPARAM(MODEL, "$(@.A.MODEL)")
	PARAM(A.ROM, "$(@.ROM)")
	PARAM(A.FORCE_TRISTATE_LOGIC, "$(@.FORCE_TRISTATE_LOGIC)")
	PARAM(A.MODEL, "$(@.MODEL)")
	ALIAS(1, A.A7)
	ALIAS(2, A.A6)
	ALIAS(3, A.A5)
	ALIAS(4, A.A4)
	ALIAS(5, A.A3)
	ALIAS(6, A.A2)
	ALIAS(7, A.A1)
	ALIAS(8, A.A0)
	ALIAS(9, A.O0)
	ALIAS(10, A.O1)
	ALIAS(11, A.O2)
	ALIAS(12, A.GND)

	ALIAS(13, A.O3)
	ALIAS(14, A.O4)
	ALIAS(15, A.O5)
	ALIAS(16, A.O6)
	ALIAS(17, A.O7)
	ALIAS(18, A.CE1Q) // CEQ
	ALIAS(19, A.A10)
	ALIAS(20, A.CE2Q) // OEQ
	ALIAS(22, A.A9)
	ALIAS(23, A.A8)
	ALIAS(24, A.VCC)
}

/*  DM82S16: 256 Bit bipolar ram
 *
 *          +--------------+
 *       A1 |1     ++    16| VCC
 *       A0 |2           15| A2
 *     CE1Q |3           14| A3
 *     CE2Q |4   82S16   13| DIN
 *     CE3Q |5           12| WEQ
 *    DOUTQ |6           11| A7
 *       A4 |7           10| A6
 *      GND |8            9| A5
 *          +--------------+
 *
 *  Naming conventions follow Signetics datasheet
 */

static NETLIST_START(TTL_82S16_DIP)
{
	TTL_82S16(A)

	DIPPINS(     /*        +--------------+       */
		   A.A1, /*     A1 |1     ++    16| VCC   */ A.VCC,
		   A.A0, /*     A0 |2           15| A2    */ A.A2,
		 A.CE1Q, /*   CE1Q |3           14| A3    */ A.A3,
		 A.CE2Q, /*   CE2Q |4   82S16   13| DIN   */ A.DIN,
		 A.CE3Q, /*   CE3Q |5           12| WEQ   */ A.WEQ,
		A.DOUTQ, /*  DOUTQ |6           11| A7    */ A.A7,
		   A.A4, /*     A4 |7           10| A6    */ A.A6,
		  A.GND, /*    GND |8            9| A5    */ A.A5
				 /*        +--------------+       */
	)
}

/*  82S115: 4K-bit TTL bipolar PROM (512 x 8)
 *
 *          +--------------+
 *       A3 |1     ++    24| VCC
 *       A4 |2           23| A2
 *       A5 |3           22| A1
 *       A6 |4   82S115  21| A0
 *       A7 |5           20| CE1Q
 *       A8 |6           19| CE2
 *       O1 |7           18| STROBE
 *       O2 |8           17| O8
 *       O3 |9           16| O7
 *       O4 |10          15| O6
 *      FE2 |11          14| O5
 *      GND |12          13| FE1
 *          +--------------+
 */

static NETLIST_START(PROM_82S115_DIP)
{
	PROM_82S115(A)
	NC_PIN(NC)

	DIPPINS(   /*      +--------------+        */
		 A.A3, /*   A3 |1     ++    24| VCC    */ A.VCC,
		 A.A4, /*   A4 |2           23| A2     */ A.A2,
		 A.A5, /*   A5 |3           22| A1     */ A.A1,
		 A.A6, /*   A6 |4   82S115  21| A0     */ A.A0,
		 A.A7, /*   A7 |5           20| CE1Q   */ A.CE1Q,
		 A.A8, /*   A8 |6           19| CE2    */ A.CE2,
		 A.O1, /*   O1 |7           18| STROBE */ A.STROBE,
		 A.O2, /*   O2 |8           17| O8     */ A.O8,
		 A.O3, /*   O3 |9           16| O7     */ A.O7,
		 A.O4, /*   O4 |10          15| O6     */ A.O6,
		 NC.I, /*  FE2 |11          14| O5     */ A.O5,
		A.GND, /*  GND |12          13| FE1    */ NC.I
			   /*      +--------------+        */
	)
}

//- Identifier:  PROM_MK28000_DIP
//- Title: MK28000 (2048 x 8 or 4096 x 4) 16384-Bit TTL PROM
//- Pinalias: VCC,A1,A2,A3,A4,A5,A6,A10,GND,A9,A8,A7,ARQ,OE2,A11,O8,O7,O6,O5,O4,O3,O2,O1,OE1
//- Package: DIP
//- Param: ROM
//-    The name of the source to load the rom content from
//- NamingConvention: Naming conventions follow Mostek datasheet
//- Limitations:
//-    None.

static NETLIST_START(PROM_MK28000_DIP)
{

	PROM_MK28000(A)

	DEFPARAM(ROM, "unknown")
	PARAM(A.ROM, "$(@.ROM)")
	ALIAS(1, A.VCC)
	ALIAS(2, A.A1)
	ALIAS(3, A.A2)
	ALIAS(4, A.A3)
	ALIAS(5, A.A4)
	ALIAS(6, A.A5)
	ALIAS(7, A.A6)
	ALIAS(8, A.A10)
	ALIAS(9, A.GND)
	ALIAS(10, A.A9)
	ALIAS(11, A.A8)
	ALIAS(12, A.A7)
	ALIAS(13, A.ARQ)
	ALIAS(14, A.OE2)
	ALIAS(15, A.A11)
	ALIAS(16, A.O8)
	ALIAS(17, A.O7)
	ALIAS(18, A.O6)
	ALIAS(19, A.O5)
	ALIAS(20, A.O4)
	ALIAS(21, A.O3)
	ALIAS(22, A.O2)
	ALIAS(23, A.O1)
	ALIAS(24, A.OE1)
}

//- Identifier:  ROM_MCM14524_DIP
//- Title: MCM14524 1024-BIT READ ONLY MEMORY
//- Pinalias: CLK,EN,B0,B1,B2,B3,A2,GND,A3,A4,A5,A6,A7,A1,A0,VCC
//- Package: DIP
//- Param: ROM
//-    The name of the source to load the rom content from
//- NamingConvention: Naming conventions follow Motorola datasheet
//- Limitations:
//-    Voltage-dependent timing is partially implemented.
//- FunctionTable:
//-    http://www.bitsavers.org/components/motorola/_dataBooks/1978_Motorola_CMOS_Data_Book.pdf 7-439 (pdf page 488)

static NETLIST_START(ROM_MCM14524_DIP)
{

	ROM_MCM14524(A)

	DEFPARAM(ROM, "unknown")
	PARAM(A.ROM, "$(@.ROM)")

			 /* Motorola MCM14524:      */
	DIPPINS( /*      +-----..-----+     */
	  A.CLK, /* /CLK |1         16| VDD */ A.VCC,
	   A.EN, /*   CE |2         15| A0  */ A.A0,
	   A.B0, /*   B0 |3   MCM   14| A1  */ A.A1,
	   A.B1, /*   B1 |4  14524  13| A7  */ A.A7,
	   A.B2, /*   B2 |5         12| A6  */ A.A6,
	   A.B3, /*   B3 |6         11| A5  */ A.A5,
	   A.A2, /*   A2 |7         10| A4  */ A.A4,
	  A.GND, /*  VSS |8          9| A3  */ A.A3
			 /*      +------------+ */
	)
}

/*  2102: 1024 x 1-bit Static RAM
 *
 *          +--------------+
 *       A6 |1     ++    16| A7
 *       A5 |2           15| A8
 *      RWQ |3           14| A9
 *       A1 |4   82S16   13| CEQ
 *       A2 |5           12| DO
 *       A3 |6           11| DI
 *       A4 |7           10| VCC
 *       A0 |8            9| GND
 *          +--------------+
 */
static NETLIST_START(RAM_2102A_DIP)
{
	RAM_2102A(A)

	DIPPINS(   /*      +--------------+      */
		 A.A6, /*   A6 |1     ++    16| A7   */ A.A7,
		 A.A5, /*   A5 |2           15| A8   */ A.A8,
		A.RWQ, /*  RWQ |3           14| A9   */ A.A9,
		 A.A1, /*   A1 |4   82S16   13| CEQ  */ A.CEQ,
		 A.A2, /*   A2 |5           12| DO   */ A.DO,
		 A.A3, /*   A3 |6           11| DI   */ A.DI,
		 A.A4, /*   A4 |7           10| VCC  */ A.VCC,
		 A.A0, /*   A0 |8            9| GND  */ A.GND
			   /*      +--------------+      */
	)
}

//FIXME: Documentation
static NETLIST_START(ROM_TMS4800_DIP)
{
	ROM_TMS4800(A)

	DIPPINS(   /*       +----------------+     */
		A.VSS, /*   VSS |1      ++     24| OE1 */ A.OE1,
		A.A1,  /*    A1 |2             23| O1  */ A.O1,
		A.A2,  /*    A2 |3             22| O2  */ A.O2,
		A.A3,  /*    A3 |4   TMS-4800  21| O3  */ A.O3,
		A.A4,  /*    A4 |5             20| O4  */ A.O4,
		A.A5,  /*    A5 |6             19| O5  */ A.O5,
		A.A6,  /*    A6 |7             18| O6  */ A.O6,
		A.A10, /*   A10 |8             17| O7  */ A.O7,
		A.VGG, /*   VGG |9             16| O8  */ A.O8,
		A.A9,  /*    A9 |10            15| A11 */ A.A11,
		A.A8,  /*    A8 |11            14| OE2 */ A.OE2,
		A.A7,  /*    A7 |12            13| AR  */ A.AR
			   /*       +----------------+      */
	)
}


NETLIST_START(roms_lib)
{

	LOCAL_LIB_ENTRY(PROM_82S123_DIP)
	LOCAL_LIB_ENTRY(PROM_82S126_DIP)
	LOCAL_LIB_ENTRY(PROM_74S287_DIP)
	LOCAL_LIB_ENTRY(EPROM_2716_DIP)
	LOCAL_LIB_ENTRY(TTL_82S16_DIP)
	LOCAL_LIB_ENTRY(PROM_82S115_DIP)
	LOCAL_LIB_ENTRY(PROM_MK28000_DIP)
	LOCAL_LIB_ENTRY(ROM_MCM14524_DIP)
	LOCAL_LIB_ENTRY(RAM_2102A_DIP)
	LOCAL_LIB_ENTRY(ROM_TMS4800_DIP)
	}

