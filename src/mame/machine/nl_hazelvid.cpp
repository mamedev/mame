// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

/***************************************************************************

  Netlist (hazelvid) included from hazeltin.cpp

***************************************************************************/

#include "netlist/devices/net_lib.h"

#ifndef __PLIB_PREPROCESSOR__
#endif

NETLIST_START(hazelvid)

	SOLVER(Solver, 48000)
	PARAM(Solver.PARALLEL, 0) // Don't do parallel solvers
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient
	PARAM(NETLIST.USE_DEACTIVATE, 1)

	ANALOG_INPUT(V5, 5)
	ALIAS(VCC, V5) // no-ttl-dip devices need VCC!

	ALIAS(VSS, V5)  // CMOS/MOS default +
	ALIAS(VDD, GND) // CMOS/MOS default -

	TTL_INPUT(high, 1)
	TTL_INPUT(low, 0)

	MAINCLOCK(video_clk, 16632000.0)

	/* Character line counter, divide-by-9 */
	TTL_74161(u88, high, high, high, low, high, u87_6.Q, video_clk, high, high)

	TTL_7404_INVERT(u87_6, u88.RCO)

	TTL_74175(u81, video_clk, u88.RCO, u81.Q1, low, u88.QC, high)

	TTL_7404_INVERT(u87_5, u81.Q1)
	ALIAS(ndot, u87_5.Q)
	TTL_7404_INVERT(u59_4, u81.Q1Q)
	ALIAS(dot, u59_4.Q)

	/* Character bucket counter */

	/* least significant 4 bits */
	TTL_74161(u70, low, low, low, low, high, u59_5.Q, u81.Q2Q, high, high)

	/* most significant 4 bits */
	TTL_74161(u69, low, low, low, low, high, u59_5.Q, u81.Q2Q, u70.RCO, u70.RCO)
	/* Horizontal/Vertical timing signals */

	/* signal lookup PROM */
	PROM_82S126(u71, low, low, u70.QA, u70.QB, u70.QC, u70.QD, u69.QA, u69.QB, u69.QC, low)
	PARAM(u71.ROM, "u90_702128_82s129.bin")

	/* signal decoding */
	TTL_9334(u72, high, u81.Q1Q, u71.O4, u71.O1, u71.O2, u71.O3)
	ALIAS(hms, u72.Q1)
	ALIAS(hmsd, u72.Q2)
	ALIAS(hblank, u72.Q3)
	ALIAS(vstrobe, u72.Q4)
	ALIAS(hdriveq, u72.Q5)
	ALIAS(vidbus, u72.Q6)
	TTL_7404_INVERT(u59_5, u72.Q7)

	TTL_7400_NAND(u83_1, high, vstrobe)
	ALIAS(char_line_clk, u83_1.Q)

	/* Character line counter */
	TTL_74161(u84, low, low, low, low, high, u83_3.Q, char_line_clk, high, high)
	ALIAS(lc20, u84.QA)
	ALIAS(lc21, u84.QB)
	ALIAS(lc22, u84.QC)
	ALIAS(lc23, u84.QD)
	TTL_7400_NAND(u83_3, u84.QB, u84.QD)
	TTL_7404_INVERT(u92_5, u90.QD)

	TTL_7410_NAND(u89_3, u90.QD, u90.QC, u90.QA)

	/* Character row counter */
	TTL_74161(u90, low, low, low, low, high, u89_3.Q, u92_3.Q, high, high)
	TTL_7404_INVERT(u92_3, u84.QD)
	TTL_7404_INVERT(u92_1, u90.QB)

	TTL_7474(u95_2, u92_5.Q, u95_2.QQ, high, high)

	TTL_7411_AND(u91_3, u84.QA, u84.QD, u90.QB)
	TTL_7411_AND(u91_1, u92_1.Q, u90.QA, u84.QC)

	TTL_7404_INVERT(u92_2, u90.QC)
	TTL_7411_AND(u91_2, u95_2.QQ, u92_2.Q, u92_5.Q)
	TTL_7404_INVERT(u92_6, u91_2.Q)

	/* Vertical blanking and drive */
	TTL_7473(u85_vdrive, char_line_clk, u91_1.Q, u91_3.Q, u85_vblankq.QQ)
	TTL_7473(u85_vblankq, char_line_clk, u92_6.Q, u91_2.Q, high)

	/* Outgoing signals */
	TTL_7404_INVERT(u73_4, hdriveq)
	ALIAS(hdrive, u73_4.Q)

	TTL_7404_INVERT(u61_6, hmsd)
	TTL_7402_NOR(u60_1, u61_6.Q, u81.Q4)
	ALIAS(lbc, u60_1.Q)

	TTL_7404_INVERT(u74_1, u84.QB)
	TTL_7410_NAND(u89_2, vidbus, vidbus, u82_2.Q)
	ALIAS(ncntbenq, u89_2.Q)

	TTL_7410_NAND(u89_1, u81.Q2, hmsd, u82_2.Q)
	TTL_7400_NAND(u83_4, u89_1.Q, high)
	ALIAS(cntclk, u83_4.Q)

	TTL_7402_NOR(u60_2, hblank, u85_vblankq.QQ)
	ALIAS(clr_vid_sr, u60_2.Q)

	TTL_7404_INVERT(u92_4, u84.QB)
	TTL_74260_NOR(u82_2, u85_vblankq.QQ, u92_3.Q, u84.QC, u92_4.Q, u84.QA)
	TTL_7400_NAND(u83_2, vstrobe, u82_2.Q)
	ALIAS(sync_bus_disable_q, u83_2.Q)

	ALIAS(vdrive, u85_vdrive.Q)
	TTL_7404_INVERT(u73_5, vdrive)
	ALIAS(vdriveq, u73_5.Q)

	TTL_7474(u95_1, u84.QD, u85_vblankq.QQ, high, high)
	ALIAS(vblank, u95_1.Q)

	TTL_74260_NOR(u82_1, u85_vblankq.QQ, u92_4.Q, u84.QD, u84.QC, low)
	TTL_7404_INVERT(u61_3, u82_1.Q)
	ALIAS(tvinterq, u61_3.Q)

	TTL_INPUT(cpu_iowq, 1)
	TTL_INPUT(cpu_ba4, 1)
	TTL_7432_OR(u36_1, cpu_iowq, cpu_ba4)

	/* Character address counter */
	TTL_74193(u7, low,   low,   low,   low,   low, u36_1.Q, cntclk, high)
	TTL_74193(u5, cpu_db0, cpu_db1, cpu_db2, cpu_db3, low, u36_1.Q, u7.CARRYQ, u7.BORROWQ)
	TTL_74193(u3, cpu_db4, cpu_db5, cpu_db6, cpu_db7, low, u36_1.Q, u5.CARRYQ, u5.BORROWQ)

	TTL_74365(u6, low, ncntbenq, u7.QA, u7.QB, u7.QC, u7.QD, u5.QA, u5.QB)
	TTL_74365(u4, low, ncntbenq, u5.QC, u5.QD, u3.QA, u3.QB, u3.QC, low)
	ALIAS(ba0, u6.Y1)
	ALIAS(ba1, u6.Y2)
	ALIAS(ba2, u6.Y3)
	ALIAS(ba3, u6.Y4)
	ALIAS(ba4, u6.Y5)
	ALIAS(ba5, u6.Y6)
	ALIAS(ba6, u4.Y1)
	ALIAS(ba7, u4.Y2)
	ALIAS(ba8, u4.Y3)
	ALIAS(ba9, u4.Y4)
	ALIAS(ba10, u4.Y5)

	/* Video RAM */
	TTL_INPUT(memwq, 1)
	TTL_INPUT(mrq, 1)
	TTL_INPUT(ba13, 0)

	TTL_7432_OR(u36_2, memwq, memwq)
	TTL_7400_NAND(u37_2, u36_2.Q, mrq)
	TTL_7400_NAND(u37_3, u37_2.Q, ba13)
	TTL_7400_NAND(u37_4, u37_3.Q, ncntbenq)
	TTL_7404_INVERT(u17_2, u4.Y5)
	TTL_7400_NAND(u30_2, u17_2.Q, u37_4.Q)
	TTL_7400_NAND(u37_1, u4.Y5, u37_4.Q)
	TTL_INPUT(rwq, 1)

	/* Lower 1K */
	RAM_2102A(u22, u30_2.Q, ba0, ba1, ba2, ba3, ba4, ba5, ba6, ba7, ba8, ba9, rwq, u22.DO)
	RAM_2102A(u23, u30_2.Q, ba0, ba1, ba2, ba3, ba4, ba5, ba6, ba7, ba8, ba9, rwq, u23.DO)
	RAM_2102A(u24, u30_2.Q, ba0, ba1, ba2, ba3, ba4, ba5, ba6, ba7, ba8, ba9, rwq, u24.DO)
	RAM_2102A(u25, u30_2.Q, ba0, ba1, ba2, ba3, ba4, ba5, ba6, ba7, ba8, ba9, rwq, u25.DO)
	RAM_2102A(u26, u30_2.Q, ba0, ba1, ba2, ba3, ba4, ba5, ba6, ba7, ba8, ba9, rwq, u26.DO)
	RAM_2102A(u27, u30_2.Q, ba0, ba1, ba2, ba3, ba4, ba5, ba6, ba7, ba8, ba9, rwq, u27.DO)
	RAM_2102A(u28, u30_2.Q, ba0, ba1, ba2, ba3, ba4, ba5, ba6, ba7, ba8, ba9, rwq, u28.DO)
	RAM_2102A(u29, u30_2.Q, ba0, ba1, ba2, ba3, ba4, ba5, ba6, ba7, ba8, ba9, rwq, u29.DO)

	/* Upper 1K */
	RAM_2102A(u9,  u37_1.Q, ba0, ba1, ba2, ba3, ba4, ba5, ba6, ba7, ba8, ba9, rwq, u9.DO)
	RAM_2102A(u10, u37_1.Q, ba0, ba1, ba2, ba3, ba4, ba5, ba6, ba7, ba8, ba9, rwq, u10.DO)
	RAM_2102A(u11, u37_1.Q, ba0, ba1, ba2, ba3, ba4, ba5, ba6, ba7, ba8, ba9, rwq, u11.DO)
	RAM_2102A(u12, u37_1.Q, ba0, ba1, ba2, ba3, ba4, ba5, ba6, ba7, ba8, ba9, rwq, u12.DO)
	RAM_2102A(u13, u37_1.Q, ba0, ba1, ba2, ba3, ba4, ba5, ba6, ba7, ba8, ba9, rwq, u13.DO)
	RAM_2102A(u14, u37_1.Q, ba0, ba1, ba2, ba3, ba4, ba5, ba6, ba7, ba8, ba9, rwq, u14.DO)
	RAM_2102A(u15, u37_1.Q, ba0, ba1, ba2, ba3, ba4, ba5, ba6, ba7, ba8, ba9, rwq, u15.DO)
	RAM_2102A(u16, u37_1.Q, ba0, ba1, ba2, ba3, ba4, ba5, ba6, ba7, ba8, ba9, rwq, u16.DO)

	TTL_INPUT(cpu_db0, 0)
	TTL_INPUT(cpu_db1, 0)
	TTL_INPUT(cpu_db2, 0)
	TTL_INPUT(cpu_db3, 0)
	TTL_INPUT(cpu_db4, 0)
	TTL_INPUT(cpu_db5, 0)
	TTL_INPUT(cpu_db6, 0)
	TTL_INPUT(cpu_db7, 0)

	TTL_TRISTATE(db0, u30_2.Q, u29.DO, u37_1.Q, u16.DO)
	TTL_TRISTATE(db1, u30_2.Q, u28.DO, u37_1.Q, u15.DO)
	TTL_TRISTATE(db2, u30_2.Q, u27.DO, u37_1.Q, u14.DO)
	TTL_TRISTATE(db3, u30_2.Q, u26.DO, u37_1.Q, u13.DO)
	TTL_TRISTATE(db4, u30_2.Q, u25.DO, u37_1.Q, u12.DO)
	TTL_TRISTATE(db5, u30_2.Q, u24.DO, u37_1.Q, u11.DO)
	TTL_TRISTATE(db6, u30_2.Q, u23.DO, u37_1.Q, u10.DO)
	TTL_TRISTATE(db7, u30_2.Q, u22.DO, u37_1.Q, u9.DO)

	/* Character generation */
	TTL_74175(u68, dot, db0.Q, db1.Q, db2.Q, db3.Q, high) // least significant 4 bits of each character
	TTL_AM2847(u67, lbc, u68.Q1, u68.Q2, u68.Q3, u68.Q4, sync_bus_disable_q, sync_bus_disable_q, sync_bus_disable_q, sync_bus_disable_q)

	TTL_74175(u58, dot, db4.Q, db5.Q, db6.Q, db7.Q, high) // most signifcant 4 bits of each character
	TTL_AM2847(u57, lbc, u58.Q1, u58.Q2, u58.Q3, u58.Q4, sync_bus_disable_q, sync_bus_disable_q, sync_bus_disable_q, sync_bus_disable_q)

	TTL_74174(u66, ndot, u67.OUTA, u67.OUTB, u67.OUTC, u67.OUTD, u57.OUTA, u57.OUTB, high)
	TTL_74175(u56, ndot, u57.OUTC, clr_vid_sr, u79_1.Q, u57.OUTD, high)
	TTL_7400_NAND(u79_1, u56.Q4, clr_vid_sr)
	ALIAS(fgbit_q, u56.Q3Q)

	EPROM_2716(u78, low, low, lc20, lc21, lc22, lc23, u66.Q1, u66.Q2, u66.Q3, u66.Q4, u66.Q5, u66.Q6, u56.Q1)
	PARAM(u78.ROM, "u83_chr.bin")

	TTL_74166(u77, video_clk, low, ndot, low, u78.D0, u78.D1, u78.D2, u78.D3, u78.D4, u78.D5, u78.D6, low, clr_vid_sr)
	ALIAS(raw_dot, u77.QH)

	TTL_7400_NAND(u79_4, fgbit_q, fgbit_q)
	ALIAS(highlight, u79_4.Q)

	ALIAS(video_out, raw_dot)

	/* Highlight and contrast - not yet hooked up */
	RES(R40, 160)
	RES(R41, 270)
	POT(R21_POT, 500)

	NET_C(R40.1, V5)
	NET_C(R40.2, R21_POT.1)
	NET_C(raw_dot, R21_POT.1)
	NET_C(highlight, R41.1)
	NET_C(R41.2, R21_POT.1)

	NET_C(R21_POT.3, GND)
	NET_C(R21_POT.2, GND)

NETLIST_END()
