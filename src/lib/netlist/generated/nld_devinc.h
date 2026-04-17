// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef NLD_DEVINC_H
#define NLD_DEVINC_H

#ifndef __PLIB_PREPROCESSOR__


#include "../nl_setup.h"

#define NET_CHECK_PARAM_COUNT(dev, nargs, N) \
	static_assert(nargs == N, #dev": Mismatched number of parameters passed, expected " #N " but got " PSTRINGIFY(nargs));

// ---------------------------------------------------------------------
// Source: ../analog/nld_bjt.cpp
// ---------------------------------------------------------------------

// usage       : QBJT_EB(name, MODEL)
#define QBJT_EB(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(QBJT_EB, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(QBJT_EB, name __VA_OPT__(,) __VA_ARGS__)

// usage       : QBJT_SW(name, MODEL)
#define QBJT_SW(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(QBJT_SW, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(QBJT_SW, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../analog/nld_mosfet.cpp
// ---------------------------------------------------------------------

// usage       : MOSFET(name, MODEL)
#define MOSFET(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(MOSFET, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(MOSFET, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../analog/nld_opamps.cpp
// ---------------------------------------------------------------------

// usage       : OPAMP(name, MODEL)
#define OPAMP(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(OPAMP, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(OPAMP, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../analog/nld_switches.cpp
// ---------------------------------------------------------------------

// usage       : SWITCH(name)
#define SWITCH(name) \
	NET_REGISTER_DEV(SWITCH, name)

// usage       : SWITCH2(name)
#define SWITCH2(name) \
	NET_REGISTER_DEV(SWITCH2, name)

// ---------------------------------------------------------------------
// Source: ../analog/nlid_fourterm.cpp
// ---------------------------------------------------------------------

// usage       : VCVS(name, G)
#define VCVS(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(VCVS, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(VCVS, name __VA_OPT__(,) __VA_ARGS__)

// usage       : VCCS(name, G)
#define VCCS(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(VCCS, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(VCCS, name __VA_OPT__(,) __VA_ARGS__)

// usage       : CCCS(name, G)
#define CCCS(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(CCCS, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(CCCS, name __VA_OPT__(,) __VA_ARGS__)

// usage       : CCVS(name, G)
#define CCVS(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(CCVS, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(CCVS, name __VA_OPT__(,) __VA_ARGS__)

// usage       : LVCCS(name)
#define LVCCS(name) \
	NET_REGISTER_DEV(LVCCS, name)

// ---------------------------------------------------------------------
// Source: ../analog/nlid_twoterm.cpp
// ---------------------------------------------------------------------

// usage       : RES(name, R)
#define RES(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(RES, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(RES, name __VA_OPT__(,) __VA_ARGS__)

// usage       : POT(name, R)
#define POT(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(POT, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(POT, name __VA_OPT__(,) __VA_ARGS__)

// usage       : POT2(name, R)
#define POT2(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(POT2, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(POT2, name __VA_OPT__(,) __VA_ARGS__)

// usage       : CAP(name, C)
#define CAP(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(CAP, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(CAP, name __VA_OPT__(,) __VA_ARGS__)

// usage       : IND(name, L)
#define IND(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(IND, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(IND, name __VA_OPT__(,) __VA_ARGS__)

// usage       : DIODE(name, MODEL)
#define DIODE(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(DIODE, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(DIODE, name __VA_OPT__(,) __VA_ARGS__)

// usage       : ZDIODE(name, MODEL)
#define ZDIODE(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(ZDIODE, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(ZDIODE, name __VA_OPT__(,) __VA_ARGS__)

// usage       : VS(name, V)
#define VS(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(VS, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(VS, name __VA_OPT__(,) __VA_ARGS__)

// usage       : CS(name, I)
#define CS(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(CS, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(CS, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_2102a.cpp
// ---------------------------------------------------------------------

// usage       : RAM_2102A(name, CEQ, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, RWQ, DI)
// auto connect: VCC, GND
#define RAM_2102A(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(RAM_2102A, PNARGS(__VA_ARGS__), 13)) \
	NET_REGISTER_DEV(RAM_2102A, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_4006.cpp
// ---------------------------------------------------------------------

// usage       : CD4006(name, CLOCK, D1, D2, D3, D4, D1P4, D1P4S, D2P4, D2P5, D3P4, D4P4, D4P5)
// auto connect: VCC, GND
#define CD4006(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(CD4006, PNARGS(__VA_ARGS__), 12)) \
	NET_REGISTER_DEV(CD4006, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_4013.cpp
// ---------------------------------------------------------------------

// usage       : CD4013(name, CLOCK, DATA, RESET, SET)
// auto connect: VDD, VSS
#define CD4013(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(CD4013, PNARGS(__VA_ARGS__), 4)) \
	NET_REGISTER_DEV(CD4013, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_4017.cpp
// ---------------------------------------------------------------------

// usage       : CD4017(name)
#define CD4017(name) \
	NET_REGISTER_DEV(CD4017, name)

// usage       : CD4022(name)
#define CD4022(name) \
	NET_REGISTER_DEV(CD4022, name)

// ---------------------------------------------------------------------
// Source: ../devices/nld_4020.cpp
// ---------------------------------------------------------------------

// usage       : CD4020(name, IP, RESET, VDD, VSS)
#define CD4020(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(CD4020, PNARGS(__VA_ARGS__), 4)) \
	NET_REGISTER_DEV(CD4020, name __VA_OPT__(,) __VA_ARGS__)

// usage       : CD4024(name)
#define CD4024(name) \
	NET_REGISTER_DEV(CD4024, name)

// ---------------------------------------------------------------------
// Source: ../devices/nld_4029.cpp
// ---------------------------------------------------------------------

// usage       : CD4029(name, PE, J1, J2, J3, J4, CI, UD, BD, CLK)
// auto connect: VCC, GND
#define CD4029(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(CD4029, PNARGS(__VA_ARGS__), 9)) \
	NET_REGISTER_DEV(CD4029, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_4042.cpp
// ---------------------------------------------------------------------

// usage       : CD4042(name, D1, D2, D3, D4, POL, CLK)
// auto connect: VCC, GND
#define CD4042(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(CD4042, PNARGS(__VA_ARGS__), 6)) \
	NET_REGISTER_DEV(CD4042, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_4053.cpp
// ---------------------------------------------------------------------

// usage       : CD4053_GATE(name)
#define CD4053_GATE(name) \
	NET_REGISTER_DEV(CD4053_GATE, name)

// ---------------------------------------------------------------------
// Source: ../devices/nld_4066.cpp
// ---------------------------------------------------------------------

// usage       : CD4066_GATE(name)
#define CD4066_GATE(name) \
	NET_REGISTER_DEV(CD4066_GATE, name)

// ---------------------------------------------------------------------
// Source: ../devices/nld_4076.cpp
// ---------------------------------------------------------------------

// usage       : CD4076(name, I1, I2, I3, I4, ID1, ID2, OD1, OD2)
// auto connect: VCC, GND
#define CD4076(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(CD4076, PNARGS(__VA_ARGS__), 8)) \
	NET_REGISTER_DEV(CD4076, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_4316.cpp
// ---------------------------------------------------------------------

// usage       : CD4316_GATE(name)
#define CD4316_GATE(name) \
	NET_REGISTER_DEV(CD4316_GATE, name)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74107.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74107(name, CLK, J, K, CLRQ)
// auto connect: VCC, GND
#define TTL_74107(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_74107, PNARGS(__VA_ARGS__), 4)) \
	NET_REGISTER_DEV(TTL_74107, name __VA_OPT__(,) __VA_ARGS__)

// usage       : TTL_74107A(name, CLK, J, K, CLRQ)
// auto connect: VCC, GND
#define TTL_74107A(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_74107A, PNARGS(__VA_ARGS__), 4)) \
	NET_REGISTER_DEV(TTL_74107A, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74113.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74113(name, CLK, J, K, CLRQ)
// auto connect: VCC, GND
#define TTL_74113(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_74113, PNARGS(__VA_ARGS__), 4)) \
	NET_REGISTER_DEV(TTL_74113, name __VA_OPT__(,) __VA_ARGS__)

// usage       : TTL_74113A(name, CLK, J, K, CLRQ)
// auto connect: VCC, GND
#define TTL_74113A(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_74113A, PNARGS(__VA_ARGS__), 4)) \
	NET_REGISTER_DEV(TTL_74113A, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74123.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74123(name)
#define TTL_74123(name) \
	NET_REGISTER_DEV(TTL_74123, name)

// usage       : TTL_74121(name)
#define TTL_74121(name) \
	NET_REGISTER_DEV(TTL_74121, name)

// usage       : CD4538(name)
#define CD4538(name) \
	NET_REGISTER_DEV(CD4538, name)

// usage       : TTL_9602(name)
#define TTL_9602(name) \
	NET_REGISTER_DEV(TTL_9602, name)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74125.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74125_GATE(name)
#define TTL_74125_GATE(name) \
	NET_REGISTER_DEV(TTL_74125_GATE, name)

// usage       : TTL_74126_GATE(name)
#define TTL_74126_GATE(name) \
	NET_REGISTER_DEV(TTL_74126_GATE, name)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74153.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74153(name, C0, C1, C2, C3, A, B, G)
// auto connect: VCC, GND
#define TTL_74153(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_74153, PNARGS(__VA_ARGS__), 7)) \
	NET_REGISTER_DEV(TTL_74153, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74161.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74161(name, CLK, ENP, ENT, CLRQ, LOADQ, A, B, C, D)
// auto connect: VCC, GND
#define TTL_74161(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_74161, PNARGS(__VA_ARGS__), 9)) \
	NET_REGISTER_DEV(TTL_74161, name __VA_OPT__(,) __VA_ARGS__)

// usage       : TTL_74161_FIXME(name, A, B, C, D, CLRQ, LOADQ, CLK, ENP, ENT)
// auto connect: VCC, GND
#define TTL_74161_FIXME(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_74161_FIXME, PNARGS(__VA_ARGS__), 9)) \
	NET_REGISTER_DEV(TTL_74161_FIXME, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74163.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74163(name, CLK, ENP, ENT, CLRQ, LOADQ, A, B, C, D)
// auto connect: VCC, GND
#define TTL_74163(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_74163, PNARGS(__VA_ARGS__), 9)) \
	NET_REGISTER_DEV(TTL_74163, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74164.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74164(name, A, B, CLRQ, CLK)
// auto connect: VCC, GND
#define TTL_74164(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_74164, PNARGS(__VA_ARGS__), 4)) \
	NET_REGISTER_DEV(TTL_74164, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74165.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74165(name, CLK, CLKINH, SH_LDQ, SER, A, B, C, D, E, F, G, H)
// auto connect: VCC, GND
#define TTL_74165(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_74165, PNARGS(__VA_ARGS__), 12)) \
	NET_REGISTER_DEV(TTL_74165, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74166.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74166(name, CLK, CLKINH, SH_LDQ, SER, A, B, C, D, E, F, G, H, CLRQ)
// auto connect: VCC, GND
#define TTL_74166(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_74166, PNARGS(__VA_ARGS__), 13)) \
	NET_REGISTER_DEV(TTL_74166, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74174.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74174(name, CLK, D1, D2, D3, D4, D5, D6, CLRQ)
// auto connect: VCC, GND
#define TTL_74174(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_74174, PNARGS(__VA_ARGS__), 8)) \
	NET_REGISTER_DEV(TTL_74174, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74175.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74175(name, CLK, D1, D2, D3, D4, CLRQ)
// auto connect: VCC, GND
#define TTL_74175(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_74175, PNARGS(__VA_ARGS__), 6)) \
	NET_REGISTER_DEV(TTL_74175, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74192.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74192(name, A, B, C, D, CLEAR, LOADQ, CU, CD)
// auto connect: VCC, GND
#define TTL_74192(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_74192, PNARGS(__VA_ARGS__), 8)) \
	NET_REGISTER_DEV(TTL_74192, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74193.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74193(name, A, B, C, D, CLEAR, LOADQ, CU, CD)
// auto connect: VCC, GND
#define TTL_74193(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_74193, PNARGS(__VA_ARGS__), 8)) \
	NET_REGISTER_DEV(TTL_74193, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74194.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74194(name, CLK, S0, S1, SRIN, A, B, C, D, SLIN, CLRQ)
// auto connect: VCC, GND
#define TTL_74194(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_74194, PNARGS(__VA_ARGS__), 10)) \
	NET_REGISTER_DEV(TTL_74194, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74365.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74365(name, G1Q, G2Q, A1, A2, A3, A4, A5, A6)
// auto connect: VCC, GND
#define TTL_74365(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_74365, PNARGS(__VA_ARGS__), 8)) \
	NET_REGISTER_DEV(TTL_74365, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74377.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74377_GATE(name)
#define TTL_74377_GATE(name) \
	NET_REGISTER_DEV(TTL_74377_GATE, name)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74393.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74393(name, CP, MR)
// auto connect: VCC, GND
#define TTL_74393(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_74393, PNARGS(__VA_ARGS__), 2)) \
	NET_REGISTER_DEV(TTL_74393, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_7448.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7448(name, A, B, C, D, LTQ, BIQ, RBIQ)
// auto connect: VCC, GND
#define TTL_7448(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7448, PNARGS(__VA_ARGS__), 7)) \
	NET_REGISTER_DEV(TTL_7448, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_7450.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7450_ANDORINVERT(name, A, B, C, D)
// auto connect: VCC, GND
#define TTL_7450_ANDORINVERT(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7450_ANDORINVERT, PNARGS(__VA_ARGS__), 4)) \
	NET_REGISTER_DEV(TTL_7450_ANDORINVERT, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_7473.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7473(name, CLK, J, K, CLRQ)
// auto connect: VCC, GND
#define TTL_7473(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7473, PNARGS(__VA_ARGS__), 4)) \
	NET_REGISTER_DEV(TTL_7473, name __VA_OPT__(,) __VA_ARGS__)

// usage       : TTL_7473A(name, CLK, J, K, CLRQ)
// auto connect: VCC, GND
#define TTL_7473A(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7473A, PNARGS(__VA_ARGS__), 4)) \
	NET_REGISTER_DEV(TTL_7473A, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_7474.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7474(name, CLK, D, CLRQ, PREQ)
// auto connect: VCC, GND
#define TTL_7474(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7474, PNARGS(__VA_ARGS__), 4)) \
	NET_REGISTER_DEV(TTL_7474, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_7475.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7475_GATE(name)
#define TTL_7475_GATE(name) \
	NET_REGISTER_DEV(TTL_7475_GATE, name)

// usage       : TTL_7477_GATE(name)
#define TTL_7477_GATE(name) \
	NET_REGISTER_DEV(TTL_7477_GATE, name)

// ---------------------------------------------------------------------
// Source: ../devices/nld_7483.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7483(name, A1, A2, A3, A4, B1, B2, B3, B4, C0)
// auto connect: VCC, GND
#define TTL_7483(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7483, PNARGS(__VA_ARGS__), 9)) \
	NET_REGISTER_DEV(TTL_7483, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_7485.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7485(name, A0, A1, A2, A3, B0, B1, B2, B3, LTIN, EQIN, GTIN)
// auto connect: VCC, GND
#define TTL_7485(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7485, PNARGS(__VA_ARGS__), 11)) \
	NET_REGISTER_DEV(TTL_7485, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_7490.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7490(name, A, B, R1, R2, R91, R92)
// auto connect: VCC, GND
#define TTL_7490(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7490, PNARGS(__VA_ARGS__), 6)) \
	NET_REGISTER_DEV(TTL_7490, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_7492.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7492(name, A, B, R1, R2)
// auto connect: VCC, GND
#define TTL_7492(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7492, PNARGS(__VA_ARGS__), 4)) \
	NET_REGISTER_DEV(TTL_7492, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_7493.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7493(name, CLKA, CLKB, R1, R2)
// auto connect: VCC, GND
#define TTL_7493(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7493, PNARGS(__VA_ARGS__), 4)) \
	NET_REGISTER_DEV(TTL_7493, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_7497.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7497(name, CLK, STRBQ, ENQ, UNITYQ, CLR, B0, B1, B2, B3, B4, B5)
// auto connect: VCC, GND
#define TTL_7497(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7497, PNARGS(__VA_ARGS__), 11)) \
	NET_REGISTER_DEV(TTL_7497, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74ls629.cpp
// ---------------------------------------------------------------------

// usage       : SN74LS629(name, CAP)
// auto connect: VCC, GND
#define SN74LS629(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(SN74LS629, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(SN74LS629, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_8277.cpp
// ---------------------------------------------------------------------

// usage       : TTL_8277(name, RESET, CLK, CLKA, D0A, D1A, DSA, CLKB, D0B, D1B, DSB)
// auto connect: VCC, GND
#define TTL_8277(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_8277, PNARGS(__VA_ARGS__), 10)) \
	NET_REGISTER_DEV(TTL_8277, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_82s115.cpp
// ---------------------------------------------------------------------

// usage       : PROM_82S115(name, CE1Q, CE2, A0, A1, A2, A3, A4, A5, A6, A7, A8, STROBE)
// auto connect: VCC, GND
#define PROM_82S115(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(PROM_82S115, PNARGS(__VA_ARGS__), 12)) \
	NET_REGISTER_DEV(PROM_82S115, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_82s16.cpp
// ---------------------------------------------------------------------

// usage       : TTL_82S16(name)
#define TTL_82S16(name) \
	NET_REGISTER_DEV(TTL_82S16, name)

// ---------------------------------------------------------------------
// Source: ../devices/nld_9310.cpp
// ---------------------------------------------------------------------

// usage       : TTL_9310(name, CLK, ENP, ENT, CLRQ, LOADQ, A, B, C, D)
// auto connect: VCC, GND
#define TTL_9310(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_9310, PNARGS(__VA_ARGS__), 9)) \
	NET_REGISTER_DEV(TTL_9310, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_9316.cpp
// ---------------------------------------------------------------------

// usage       : TTL_9316(name, CLK, ENP, ENT, CLRQ, LOADQ, A, B, C, D)
// auto connect: VCC, GND
#define TTL_9316(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_9316, PNARGS(__VA_ARGS__), 9)) \
	NET_REGISTER_DEV(TTL_9316, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_9321.cpp
// ---------------------------------------------------------------------

// usage       : TTL_9321(name, E, A0, A1)
#define TTL_9321(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_9321, PNARGS(__VA_ARGS__), 3)) \
	NET_REGISTER_DEV(TTL_9321, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_9322.cpp
// ---------------------------------------------------------------------

// usage       : TTL_9322(name, SELECT, A1, B1, A2, B2, A3, B3, A4, B4, STROBE)
// auto connect: VCC, GND
#define TTL_9322(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_9322, PNARGS(__VA_ARGS__), 10)) \
	NET_REGISTER_DEV(TTL_9322, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_am2847.cpp
// ---------------------------------------------------------------------

// usage       : TTL_AM2847(name, CP, INA, INB, INC, IND, RCA, RCB, RCC, RCD)
// auto connect: VSS, VDD
#define TTL_AM2847(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_AM2847, PNARGS(__VA_ARGS__), 9)) \
	NET_REGISTER_DEV(TTL_AM2847, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_dm9314.cpp
// ---------------------------------------------------------------------

// usage       : TTL_9314(name, EQ, MRQ, S0Q, S1Q, S2Q, S3Q, D0, D1, D2, D3)
// auto connect: VCC, GND
#define TTL_9314(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_9314, PNARGS(__VA_ARGS__), 10)) \
	NET_REGISTER_DEV(TTL_9314, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_dm9334.cpp
// ---------------------------------------------------------------------

// usage       : TTL_9334(name, CQ, EQ, D, A0, A1, A2)
// auto connect: VCC, GND
#define TTL_9334(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_9334, PNARGS(__VA_ARGS__), 6)) \
	NET_REGISTER_DEV(TTL_9334, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_legacy.cpp
// ---------------------------------------------------------------------

// usage       : NETDEV_RSFF(name)
#define NETDEV_RSFF(name) \
	NET_REGISTER_DEV(NETDEV_RSFF, name)

// usage       : NETDEV_DELAY(name)
#define NETDEV_DELAY(name) \
	NET_REGISTER_DEV(NETDEV_DELAY, name)

// ---------------------------------------------------------------------
// Source: ../devices/nld_log.cpp
// ---------------------------------------------------------------------

// usage       : LOG(name, I)
#define LOG(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(LOG, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(LOG, name __VA_OPT__(,) __VA_ARGS__)

// usage       : LOGD(name, I, I2)
#define LOGD(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(LOGD, PNARGS(__VA_ARGS__), 2)) \
	NET_REGISTER_DEV(LOGD, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_mm5837.cpp
// ---------------------------------------------------------------------

// usage       : MM5837(name)
#define MM5837(name) \
	NET_REGISTER_DEV(MM5837, name)

// ---------------------------------------------------------------------
// Source: ../devices/nld_ne555.cpp
// ---------------------------------------------------------------------

// usage       : NE555(name)
#define NE555(name) \
	NET_REGISTER_DEV(NE555, name)

// usage       : MC1455P(name)
#define MC1455P(name) \
	NET_REGISTER_DEV(MC1455P, name)

// ---------------------------------------------------------------------
// Source: ../devices/nld_r2r_dac.cpp
// ---------------------------------------------------------------------

// usage       : R2R_DAC(name, VIN, R, N)
#define R2R_DAC(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(R2R_DAC, PNARGS(__VA_ARGS__), 3)) \
	NET_REGISTER_DEV(R2R_DAC, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_roms.cpp
// ---------------------------------------------------------------------

// usage       : PROM_82S126(name, CE1Q, CE2Q, A0, A1, A2, A3, A4, A5, A6, A7)
// auto connect: VCC, GND
#define PROM_82S126(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(PROM_82S126, PNARGS(__VA_ARGS__), 10)) \
	NET_REGISTER_DEV(PROM_82S126, name __VA_OPT__(,) __VA_ARGS__)

// usage       : PROM_74S287(name, CE1Q, CE2Q, A0, A1, A2, A3, A4, A5, A6, A7)
// auto connect: VCC, GND
#define PROM_74S287(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(PROM_74S287, PNARGS(__VA_ARGS__), 10)) \
	NET_REGISTER_DEV(PROM_74S287, name __VA_OPT__(,) __VA_ARGS__)

// usage       : PROM_82S123(name, CEQ, A0, A1, A2, A3, A4)
// auto connect: VCC, GND
#define PROM_82S123(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(PROM_82S123, PNARGS(__VA_ARGS__), 6)) \
	NET_REGISTER_DEV(PROM_82S123, name __VA_OPT__(,) __VA_ARGS__)

// usage       : EPROM_2716(name, CE2Q, CE1Q, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)
// auto connect: VCC, GND
#define EPROM_2716(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(EPROM_2716, PNARGS(__VA_ARGS__), 13)) \
	NET_REGISTER_DEV(EPROM_2716, name __VA_OPT__(,) __VA_ARGS__)

// usage       : PROM_MK28000(name, OE1, OE2, ARQ, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)
// auto connect: VCC, GND
#define PROM_MK28000(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(PROM_MK28000, PNARGS(__VA_ARGS__), 14)) \
	NET_REGISTER_DEV(PROM_MK28000, name __VA_OPT__(,) __VA_ARGS__)

// usage       : ROM_MCM14524(name, EN, CLK, A0, A1, A2, A3, A4, A5, A6, A7)
// auto connect: VCC, GND
#define ROM_MCM14524(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(ROM_MCM14524, PNARGS(__VA_ARGS__), 10)) \
	NET_REGISTER_DEV(ROM_MCM14524, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_schmitt.cpp
// ---------------------------------------------------------------------

// usage       : SCHMITT_TRIGGER(name, STMODEL)
#define SCHMITT_TRIGGER(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(SCHMITT_TRIGGER, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(SCHMITT_TRIGGER, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_system.cpp
// ---------------------------------------------------------------------

// usage       : PARAMETER(name)
#define PARAMETER(name) \
	NET_REGISTER_DEV(PARAMETER, name)

// usage       : NC_PIN(name)
#define NC_PIN(name) \
	NET_REGISTER_DEV(NC_PIN, name)

// usage       : FRONTIER_DEV(name, I, G, Q)
#define FRONTIER_DEV(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(FRONTIER_DEV, PNARGS(__VA_ARGS__), 3)) \
	NET_REGISTER_DEV(FRONTIER_DEV, name __VA_OPT__(,) __VA_ARGS__)

// usage       : AFUNC(name, N, FUNC)
#define AFUNC(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(AFUNC, PNARGS(__VA_ARGS__), 2)) \
	NET_REGISTER_DEV(AFUNC, name __VA_OPT__(,) __VA_ARGS__)

// usage       : ANALOG_INPUT(name, IN)
#define ANALOG_INPUT(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(ANALOG_INPUT, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(ANALOG_INPUT, name __VA_OPT__(,) __VA_ARGS__)

// usage       : CLOCK(name, FREQ)
#define CLOCK(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(CLOCK, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(CLOCK, name __VA_OPT__(,) __VA_ARGS__)

// usage       : VARCLOCK(name, N, FUNC)
#define VARCLOCK(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(VARCLOCK, PNARGS(__VA_ARGS__), 2)) \
	NET_REGISTER_DEV(VARCLOCK, name __VA_OPT__(,) __VA_ARGS__)

// usage       : EXTCLOCK(name, FREQ, PATTERN)
#define EXTCLOCK(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(EXTCLOCK, PNARGS(__VA_ARGS__), 2)) \
	NET_REGISTER_DEV(EXTCLOCK, name __VA_OPT__(,) __VA_ARGS__)

// usage       : SYS_DSW(name, I, 1, 2)
#define SYS_DSW(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(SYS_DSW, PNARGS(__VA_ARGS__), 3)) \
	NET_REGISTER_DEV(SYS_DSW, name __VA_OPT__(,) __VA_ARGS__)

// usage       : SYS_DSW2(name)
#define SYS_DSW2(name) \
	NET_REGISTER_DEV(SYS_DSW2, name)

// usage       : SYS_COMPD(name)
#define SYS_COMPD(name) \
	NET_REGISTER_DEV(SYS_COMPD, name)

// usage       : SYS_PULSE(name, DELAY, DURATION, INVERT_INPUT, INVERT_OUTPUT)
#define SYS_PULSE(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(SYS_PULSE, PNARGS(__VA_ARGS__), 4)) \
	NET_REGISTER_DEV(SYS_PULSE, name __VA_OPT__(,) __VA_ARGS__)

// usage       : SYS_NOISE_MT_U(name, SIGMA)
#define SYS_NOISE_MT_U(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(SYS_NOISE_MT_U, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(SYS_NOISE_MT_U, name __VA_OPT__(,) __VA_ARGS__)

// usage       : SYS_NOISE_MT_N(name, SIGMA)
#define SYS_NOISE_MT_N(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(SYS_NOISE_MT_N, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(SYS_NOISE_MT_N, name __VA_OPT__(,) __VA_ARGS__)

// usage       : MAINCLOCK(name, FREQ)
#define MAINCLOCK(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(MAINCLOCK, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(MAINCLOCK, name __VA_OPT__(,) __VA_ARGS__)

// usage       : GNDA(name)
#define GNDA(name) \
	NET_REGISTER_DEV(GNDA, name)

// usage       : LOGIC_INPUT8(name, IN, MODEL)
#define LOGIC_INPUT8(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(LOGIC_INPUT8, PNARGS(__VA_ARGS__), 2)) \
	NET_REGISTER_DEV(LOGIC_INPUT8, name __VA_OPT__(,) __VA_ARGS__)

// usage       : LOGIC_INPUT(name, IN, MODEL)
#define LOGIC_INPUT(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(LOGIC_INPUT, PNARGS(__VA_ARGS__), 2)) \
	NET_REGISTER_DEV(LOGIC_INPUT, name __VA_OPT__(,) __VA_ARGS__)

// usage       : TTL_INPUT(name, IN)
#define TTL_INPUT(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_INPUT, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(TTL_INPUT, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_tms4800.cpp
// ---------------------------------------------------------------------

// usage       : ROM_TMS4800(name, AR, OE1, OE2, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)
// auto connect: VCC, GND
#define ROM_TMS4800(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(ROM_TMS4800, PNARGS(__VA_ARGS__), 14)) \
	NET_REGISTER_DEV(ROM_TMS4800, name __VA_OPT__(,) __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_tristate.cpp
// ---------------------------------------------------------------------

// usage       : TTL_TRISTATE(name, CEQ1, D1, CEQ2, D2)
#define TTL_TRISTATE(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_TRISTATE, PNARGS(__VA_ARGS__), 4)) \
	NET_REGISTER_DEV(TTL_TRISTATE, name __VA_OPT__(,) __VA_ARGS__)

// usage       : TTL_TRISTATE3(name)
#define TTL_TRISTATE3(name) \
	NET_REGISTER_DEV(TTL_TRISTATE3, name)

// ---------------------------------------------------------------------
// Source: ../generated/nlm_modules_lib.cpp
// ---------------------------------------------------------------------

NETLIST_EXTERNAL(modules_lib)
// ---------------------------------------------------------------------
// Source: ../macro/modules/nlmod_icl8038_dip.cpp
// ---------------------------------------------------------------------

NETLIST_EXTERNAL(ICL8038_DIP)
// ---------------------------------------------------------------------
// Source: ../macro/modules/nlmod_ne556_dip.cpp
// ---------------------------------------------------------------------

NETLIST_EXTERNAL(NE556_DIP)
// ---------------------------------------------------------------------
// Source: ../macro/modules/nlmod_rtest.cpp
// ---------------------------------------------------------------------

NETLIST_EXTERNAL(RTEST)
// ---------------------------------------------------------------------
// Source: ../macro/nlm_base_lib.cpp
// ---------------------------------------------------------------------

NETLIST_EXTERNAL(base_lib)
// ---------------------------------------------------------------------
// Source: ../macro/nlm_cd4xxx_lib.cpp
// ---------------------------------------------------------------------

// usage       : CD4001_GATE(name)
#define CD4001_GATE(name) \
	NET_REGISTER_DEV(CD4001_GATE, name)

// usage       : CD4011_GATE(name)
#define CD4011_GATE(name) \
	NET_REGISTER_DEV(CD4011_GATE, name)

// usage       : CD4030_GATE(name)
#define CD4030_GATE(name) \
	NET_REGISTER_DEV(CD4030_GATE, name)

// usage       : CD4049_GATE(name)
#define CD4049_GATE(name) \
	NET_REGISTER_DEV(CD4049_GATE, name)

// usage       : CD4069_GATE(name)
#define CD4069_GATE(name) \
	NET_REGISTER_DEV(CD4069_GATE, name)

// usage       : CD4070_GATE(name)
#define CD4070_GATE(name) \
	NET_REGISTER_DEV(CD4070_GATE, name)

// usage       : CD4071_GATE(name)
#define CD4071_GATE(name) \
	NET_REGISTER_DEV(CD4071_GATE, name)

// usage       : CD4081_GATE(name)
#define CD4081_GATE(name) \
	NET_REGISTER_DEV(CD4081_GATE, name)

NETLIST_EXTERNAL(cd4xxx_lib)
// usage       : CD4001_DIP(name)
#define CD4001_DIP(name) \
	NET_REGISTER_DEV(CD4001_DIP, name)

// usage       : CD4011_DIP(name)
#define CD4011_DIP(name) \
	NET_REGISTER_DEV(CD4011_DIP, name)

// usage       : CD4030_DIP(name)
#define CD4030_DIP(name) \
	NET_REGISTER_DEV(CD4030_DIP, name)

// usage       : CD4049_DIP(name)
#define CD4049_DIP(name) \
	NET_REGISTER_DEV(CD4049_DIP, name)

// usage       : CD4069_DIP(name)
#define CD4069_DIP(name) \
	NET_REGISTER_DEV(CD4069_DIP, name)

// usage       : CD4070_DIP(name)
#define CD4070_DIP(name) \
	NET_REGISTER_DEV(CD4070_DIP, name)

// usage       : CD4071_DIP(name)
#define CD4071_DIP(name) \
	NET_REGISTER_DEV(CD4071_DIP, name)

// usage       : CD4081_DIP(name)
#define CD4081_DIP(name) \
	NET_REGISTER_DEV(CD4081_DIP, name)

// usage       : CD4006_DIP(name)
#define CD4006_DIP(name) \
	NET_REGISTER_DEV(CD4006_DIP, name)

// usage       : CD4013_DIP(name)
#define CD4013_DIP(name) \
	NET_REGISTER_DEV(CD4013_DIP, name)

// usage       : CD4017_DIP(name)
#define CD4017_DIP(name) \
	NET_REGISTER_DEV(CD4017_DIP, name)

// usage       : CD4022_DIP(name)
#define CD4022_DIP(name) \
	NET_REGISTER_DEV(CD4022_DIP, name)

// usage       : CD4020_DIP(name)
#define CD4020_DIP(name) \
	NET_REGISTER_DEV(CD4020_DIP, name)

// usage       : CD4024_DIP(name)
#define CD4024_DIP(name) \
	NET_REGISTER_DEV(CD4024_DIP, name)

// usage       : CD4029_DIP(name)
#define CD4029_DIP(name) \
	NET_REGISTER_DEV(CD4029_DIP, name)

// usage       : CD4042_DIP(name)
#define CD4042_DIP(name) \
	NET_REGISTER_DEV(CD4042_DIP, name)

// usage       : CD4053_DIP(name)
#define CD4053_DIP(name) \
	NET_REGISTER_DEV(CD4053_DIP, name)

// usage       : CD4066_DIP(name)
#define CD4066_DIP(name) \
	NET_REGISTER_DEV(CD4066_DIP, name)

// usage       : CD4016_DIP(name)
#define CD4016_DIP(name) \
	NET_REGISTER_DEV(CD4016_DIP, name)

// usage       : CD4076_DIP(name)
#define CD4076_DIP(name) \
	NET_REGISTER_DEV(CD4076_DIP, name)

// usage       : CD4316_DIP(name)
#define CD4316_DIP(name) \
	NET_REGISTER_DEV(CD4316_DIP, name)

// usage       : CD4538_DIP(name)
#define CD4538_DIP(name) \
	NET_REGISTER_DEV(CD4538_DIP, name)

// usage       : MM5837_DIP(name)
#define MM5837_DIP(name) \
	NET_REGISTER_DEV(MM5837_DIP, name)

// ---------------------------------------------------------------------
// Source: ../macro/nlm_opamp_lib.cpp
// ---------------------------------------------------------------------

NETLIST_EXTERNAL(opamp_lib)
// usage       : opamp_layout_4_4_11(name)
#define opamp_layout_4_4_11(name) \
	NET_REGISTER_DEV(opamp_layout_4_4_11, name)

// usage       : opamp_layout_2_8_4(name)
#define opamp_layout_2_8_4(name) \
	NET_REGISTER_DEV(opamp_layout_2_8_4, name)

// usage       : opamp_layout_2_13_9_4(name)
#define opamp_layout_2_13_9_4(name) \
	NET_REGISTER_DEV(opamp_layout_2_13_9_4, name)

// usage       : opamp_layout_1_7_4(name)
#define opamp_layout_1_7_4(name) \
	NET_REGISTER_DEV(opamp_layout_1_7_4, name)

// usage       : opamp_layout_1_8_5(name)
#define opamp_layout_1_8_5(name) \
	NET_REGISTER_DEV(opamp_layout_1_8_5, name)

// usage       : opamp_layout_1_11_6(name)
#define opamp_layout_1_11_6(name) \
	NET_REGISTER_DEV(opamp_layout_1_11_6, name)

// usage       : MB3614_DIP(name)
#define MB3614_DIP(name) \
	NET_REGISTER_DEV(MB3614_DIP, name)

// usage       : MC3340_DIP(name)
#define MC3340_DIP(name) \
	NET_REGISTER_DEV(MC3340_DIP, name)

// usage       : TL081_DIP(name)
#define TL081_DIP(name) \
	NET_REGISTER_DEV(TL081_DIP, name)

// usage       : TL082_DIP(name)
#define TL082_DIP(name) \
	NET_REGISTER_DEV(TL082_DIP, name)

// usage       : TL084_DIP(name)
#define TL084_DIP(name) \
	NET_REGISTER_DEV(TL084_DIP, name)

// usage       : LM324_DIP(name)
#define LM324_DIP(name) \
	NET_REGISTER_DEV(LM324_DIP, name)

// usage       : LM348_DIP(name)
#define LM348_DIP(name) \
	NET_REGISTER_DEV(LM348_DIP, name)

// usage       : LM358_DIP(name)
#define LM358_DIP(name) \
	NET_REGISTER_DEV(LM358_DIP, name)

// usage       : LM2902_DIP(name)
#define LM2902_DIP(name) \
	NET_REGISTER_DEV(LM2902_DIP, name)

// usage       : UA741_DIP8(name)
#define UA741_DIP8(name) \
	NET_REGISTER_DEV(UA741_DIP8, name)

// usage       : UA741_DIP10(name)
#define UA741_DIP10(name) \
	NET_REGISTER_DEV(UA741_DIP10, name)

// usage       : UA741_DIP14(name)
#define UA741_DIP14(name) \
	NET_REGISTER_DEV(UA741_DIP14, name)

// usage       : MC1558_DIP(name)
#define MC1558_DIP(name) \
	NET_REGISTER_DEV(MC1558_DIP, name)

// usage       : LM747_DIP(name)
#define LM747_DIP(name) \
	NET_REGISTER_DEV(LM747_DIP, name)

// usage       : LM747A_DIP(name)
#define LM747A_DIP(name) \
	NET_REGISTER_DEV(LM747A_DIP, name)

// usage       : LM3900(name)
#define LM3900(name) \
	NET_REGISTER_DEV(LM3900, name)

// usage       : AN6551_SIL(name)
#define AN6551_SIL(name) \
	NET_REGISTER_DEV(AN6551_SIL, name)

// ---------------------------------------------------------------------
// Source: ../macro/nlm_otheric_lib.cpp
// ---------------------------------------------------------------------

// usage       : MC14584B_GATE(name)
#define MC14584B_GATE(name) \
	NET_REGISTER_DEV(MC14584B_GATE, name)

NETLIST_EXTERNAL(otheric_lib)
// usage       : MC14584B_DIP(name)
#define MC14584B_DIP(name) \
	NET_REGISTER_DEV(MC14584B_DIP, name)

// usage       : NE566_DIP(name)
#define NE566_DIP(name) \
	NET_REGISTER_DEV(NE566_DIP, name)

// usage       : NE555_DIP(name)
#define NE555_DIP(name) \
	NET_REGISTER_DEV(NE555_DIP, name)

// usage       : MC1455P_DIP(name)
#define MC1455P_DIP(name) \
	NET_REGISTER_DEV(MC1455P_DIP, name)

// ---------------------------------------------------------------------
// Source: ../macro/nlm_roms_lib.cpp
// ---------------------------------------------------------------------

NETLIST_EXTERNAL(roms_lib)
// usage       : PROM_82S123_DIP(name)
#define PROM_82S123_DIP(name) \
	NET_REGISTER_DEV(PROM_82S123_DIP, name)

// usage       : PROM_82S126_DIP(name)
#define PROM_82S126_DIP(name) \
	NET_REGISTER_DEV(PROM_82S126_DIP, name)

// usage       : PROM_74S287_DIP(name)
#define PROM_74S287_DIP(name) \
	NET_REGISTER_DEV(PROM_74S287_DIP, name)

// usage       : EPROM_2716_DIP(name)
#define EPROM_2716_DIP(name) \
	NET_REGISTER_DEV(EPROM_2716_DIP, name)

// usage       : TTL_82S16_DIP(name)
#define TTL_82S16_DIP(name) \
	NET_REGISTER_DEV(TTL_82S16_DIP, name)

// usage       : PROM_82S115_DIP(name)
#define PROM_82S115_DIP(name) \
	NET_REGISTER_DEV(PROM_82S115_DIP, name)

// usage       : PROM_MK28000_DIP(name)
#define PROM_MK28000_DIP(name) \
	NET_REGISTER_DEV(PROM_MK28000_DIP, name)

// usage       : ROM_MCM14524_DIP(name)
#define ROM_MCM14524_DIP(name) \
	NET_REGISTER_DEV(ROM_MCM14524_DIP, name)

// usage       : RAM_2102A_DIP(name)
#define RAM_2102A_DIP(name) \
	NET_REGISTER_DEV(RAM_2102A_DIP, name)

// usage       : ROM_TMS4800_DIP(name)
#define ROM_TMS4800_DIP(name) \
	NET_REGISTER_DEV(ROM_TMS4800_DIP, name)

// ---------------------------------------------------------------------
// Source: ../macro/nlm_ttl74xx_lib.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7400_NAND(name, A, B)
// auto connect: VCC, GND
#define TTL_7400_NAND(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7400_NAND, PNARGS(__VA_ARGS__), 2)) \
	NET_REGISTER_DEV(TTL_7400_NAND, name __VA_OPT__(,) __VA_ARGS__)

// usage       : TTL_7402_NOR(name, A, B)
// auto connect: VCC, GND
#define TTL_7402_NOR(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7402_NOR, PNARGS(__VA_ARGS__), 2)) \
	NET_REGISTER_DEV(TTL_7402_NOR, name __VA_OPT__(,) __VA_ARGS__)

// usage       : TTL_7404_INVERT(name, A)
// auto connect: VCC, GND
#define TTL_7404_INVERT(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7404_INVERT, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(TTL_7404_INVERT, name __VA_OPT__(,) __VA_ARGS__)

// usage       : TTL_7406_GATE(name)
#define TTL_7406_GATE(name) \
	NET_REGISTER_DEV(TTL_7406_GATE, name)

// usage       : TTL_7407_GATE(name)
#define TTL_7407_GATE(name) \
	NET_REGISTER_DEV(TTL_7407_GATE, name)

// usage       : TTL_7408_GATE(name)
#define TTL_7408_GATE(name) \
	NET_REGISTER_DEV(TTL_7408_GATE, name)

// usage       : TTL_7408_AND(name, A, B)
// auto connect: VCC, GND
#define TTL_7408_AND(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7408_AND, PNARGS(__VA_ARGS__), 2)) \
	NET_REGISTER_DEV(TTL_7408_AND, name __VA_OPT__(,) __VA_ARGS__)

// usage       : TTL_7410_NAND(name, A, B, C)
// auto connect: VCC, GND
#define TTL_7410_NAND(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7410_NAND, PNARGS(__VA_ARGS__), 3)) \
	NET_REGISTER_DEV(TTL_7410_NAND, name __VA_OPT__(,) __VA_ARGS__)

// usage       : TTL_7410_GATE(name)
#define TTL_7410_GATE(name) \
	NET_REGISTER_DEV(TTL_7410_GATE, name)

// usage       : TTL_7411_AND(name, A, B, C)
// auto connect: VCC, GND
#define TTL_7411_AND(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7411_AND, PNARGS(__VA_ARGS__), 3)) \
	NET_REGISTER_DEV(TTL_7411_AND, name __VA_OPT__(,) __VA_ARGS__)

// usage       : TTL_7411_GATE(name)
#define TTL_7411_GATE(name) \
	NET_REGISTER_DEV(TTL_7411_GATE, name)

// usage       : TTL_7416_GATE(name)
#define TTL_7416_GATE(name) \
	NET_REGISTER_DEV(TTL_7416_GATE, name)

// usage       : TTL_7417_GATE(name)
#define TTL_7417_GATE(name) \
	NET_REGISTER_DEV(TTL_7417_GATE, name)

// usage       : TTL_7420_NAND(name, A, B, C, D)
// auto connect: VCC, GND
#define TTL_7420_NAND(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7420_NAND, PNARGS(__VA_ARGS__), 4)) \
	NET_REGISTER_DEV(TTL_7420_NAND, name __VA_OPT__(,) __VA_ARGS__)

// usage       : TTL_7421_AND(name, A, B, C, D)
// auto connect: VCC, GND
#define TTL_7421_AND(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7421_AND, PNARGS(__VA_ARGS__), 4)) \
	NET_REGISTER_DEV(TTL_7421_AND, name __VA_OPT__(,) __VA_ARGS__)

// usage       : TTL_7425_NOR(name, A, B, C, D)
// auto connect: VCC, GND
#define TTL_7425_NOR(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7425_NOR, PNARGS(__VA_ARGS__), 4)) \
	NET_REGISTER_DEV(TTL_7425_NOR, name __VA_OPT__(,) __VA_ARGS__)

// usage       : TTL_7427_NOR(name, A, B, C)
// auto connect: VCC, GND
#define TTL_7427_NOR(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7427_NOR, PNARGS(__VA_ARGS__), 3)) \
	NET_REGISTER_DEV(TTL_7427_NOR, name __VA_OPT__(,) __VA_ARGS__)

// usage       : TTL_7430_NAND(name, A, B, C, D, E, F, G, H)
// auto connect: VCC, GND
#define TTL_7430_NAND(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7430_NAND, PNARGS(__VA_ARGS__), 8)) \
	NET_REGISTER_DEV(TTL_7430_NAND, name __VA_OPT__(,) __VA_ARGS__)

// usage       : TTL_7432_OR(name, A, B)
// auto connect: VCC, GND
#define TTL_7432_OR(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7432_OR, PNARGS(__VA_ARGS__), 2)) \
	NET_REGISTER_DEV(TTL_7432_OR, name __VA_OPT__(,) __VA_ARGS__)

// usage       : TTL_7437_NAND(name, A, B)
#define TTL_7437_NAND(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7437_NAND, PNARGS(__VA_ARGS__), 2)) \
	NET_REGISTER_DEV(TTL_7437_NAND, name __VA_OPT__(,) __VA_ARGS__)

// usage       : TTL_7438_NAND(name, A, B)
#define TTL_7438_NAND(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7438_NAND, PNARGS(__VA_ARGS__), 2)) \
	NET_REGISTER_DEV(TTL_7438_NAND, name __VA_OPT__(,) __VA_ARGS__)

// usage       : TTL_7442(name)
#define TTL_7442(name) \
	NET_REGISTER_DEV(TTL_7442, name)

// usage       : TTL_7486_XOR(name, A, B)
// auto connect: VCC, GND
#define TTL_7486_XOR(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_7486_XOR, PNARGS(__VA_ARGS__), 2)) \
	NET_REGISTER_DEV(TTL_7486_XOR, name __VA_OPT__(,) __VA_ARGS__)

// usage       : TTL_74139_GATE(name)
#define TTL_74139_GATE(name) \
	NET_REGISTER_DEV(TTL_74139_GATE, name)

// usage       : TTL_74147_GATE(name)
#define TTL_74147_GATE(name) \
	NET_REGISTER_DEV(TTL_74147_GATE, name)

// usage       : TTL_74148_GATE(name)
#define TTL_74148_GATE(name) \
	NET_REGISTER_DEV(TTL_74148_GATE, name)

// usage       : TTL_74151_GATE(name)
#define TTL_74151_GATE(name) \
	NET_REGISTER_DEV(TTL_74151_GATE, name)

// usage       : TTL_74155A_GATE(name)
#define TTL_74155A_GATE(name) \
	NET_REGISTER_DEV(TTL_74155A_GATE, name)

// usage       : TTL_74155B_GATE(name)
#define TTL_74155B_GATE(name) \
	NET_REGISTER_DEV(TTL_74155B_GATE, name)

// usage       : TTL_74156A_GATE(name)
#define TTL_74156A_GATE(name) \
	NET_REGISTER_DEV(TTL_74156A_GATE, name)

// usage       : TTL_74156B_GATE(name)
#define TTL_74156B_GATE(name) \
	NET_REGISTER_DEV(TTL_74156B_GATE, name)

// usage       : TTL_74157_GATE(name)
#define TTL_74157_GATE(name) \
	NET_REGISTER_DEV(TTL_74157_GATE, name)

// usage       : TTL_74260_NOR(name, A, B, C, D, E)
// auto connect: VCC, GND
#define TTL_74260_NOR(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_74260_NOR, PNARGS(__VA_ARGS__), 5)) \
	NET_REGISTER_DEV(TTL_74260_NOR, name __VA_OPT__(,) __VA_ARGS__)

// usage       : TTL_74279A(name)
#define TTL_74279A(name) \
	NET_REGISTER_DEV(TTL_74279A, name)

// usage       : TTL_74279B(name)
#define TTL_74279B(name) \
	NET_REGISTER_DEV(TTL_74279B, name)

// usage       : TTL_74368_GATE(name)
#define TTL_74368_GATE(name) \
	NET_REGISTER_DEV(TTL_74368_GATE, name)

// usage       : TTL_9312(name, A, B, C, G, D0, D1, D2, D3, D4, D5, D6, D7)
// auto connect: VCC, GND
#define TTL_9312(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(TTL_9312, PNARGS(__VA_ARGS__), 12)) \
	NET_REGISTER_DEV(TTL_9312, name __VA_OPT__(,) __VA_ARGS__)

NETLIST_EXTERNAL(ttl74xx_lib)
// usage       : TTL_7400_DIP(name)
#define TTL_7400_DIP(name) \
	NET_REGISTER_DEV(TTL_7400_DIP, name)

// usage       : TTL_7402_DIP(name)
#define TTL_7402_DIP(name) \
	NET_REGISTER_DEV(TTL_7402_DIP, name)

// usage       : TTL_7404_DIP(name)
#define TTL_7404_DIP(name) \
	NET_REGISTER_DEV(TTL_7404_DIP, name)

// usage       : TTL_7406_DIP(name)
#define TTL_7406_DIP(name) \
	NET_REGISTER_DEV(TTL_7406_DIP, name)

// usage       : TTL_7407_DIP(name)
#define TTL_7407_DIP(name) \
	NET_REGISTER_DEV(TTL_7407_DIP, name)

// usage       : TTL_7408_DIP(name)
#define TTL_7408_DIP(name) \
	NET_REGISTER_DEV(TTL_7408_DIP, name)

// usage       : TTL_7410_DIP(name)
#define TTL_7410_DIP(name) \
	NET_REGISTER_DEV(TTL_7410_DIP, name)

// usage       : TTL_7411_DIP(name)
#define TTL_7411_DIP(name) \
	NET_REGISTER_DEV(TTL_7411_DIP, name)

// usage       : TTL_7414_GATE(name)
#define TTL_7414_GATE(name) \
	NET_REGISTER_DEV(TTL_7414_GATE, name)

// usage       : TTL_74LS14_GATE(name)
#define TTL_74LS14_GATE(name) \
	NET_REGISTER_DEV(TTL_74LS14_GATE, name)

// usage       : TTL_7414_DIP(name)
#define TTL_7414_DIP(name) \
	NET_REGISTER_DEV(TTL_7414_DIP, name)

// usage       : TTL_74LS14_DIP(name)
#define TTL_74LS14_DIP(name) \
	NET_REGISTER_DEV(TTL_74LS14_DIP, name)

// usage       : TTL_7416_DIP(name)
#define TTL_7416_DIP(name) \
	NET_REGISTER_DEV(TTL_7416_DIP, name)

// usage       : TTL_7417_DIP(name)
#define TTL_7417_DIP(name) \
	NET_REGISTER_DEV(TTL_7417_DIP, name)

// usage       : TTL_7420_DIP(name)
#define TTL_7420_DIP(name) \
	NET_REGISTER_DEV(TTL_7420_DIP, name)

// usage       : TTL_7421_DIP(name)
#define TTL_7421_DIP(name) \
	NET_REGISTER_DEV(TTL_7421_DIP, name)

// usage       : TTL_7425_DIP(name)
#define TTL_7425_DIP(name) \
	NET_REGISTER_DEV(TTL_7425_DIP, name)

// usage       : TTL_7427_DIP(name)
#define TTL_7427_DIP(name) \
	NET_REGISTER_DEV(TTL_7427_DIP, name)

// usage       : TTL_7430_DIP(name)
#define TTL_7430_DIP(name) \
	NET_REGISTER_DEV(TTL_7430_DIP, name)

// usage       : TTL_7432_DIP(name)
#define TTL_7432_DIP(name) \
	NET_REGISTER_DEV(TTL_7432_DIP, name)

// usage       : TTL_7437_DIP(name)
#define TTL_7437_DIP(name) \
	NET_REGISTER_DEV(TTL_7437_DIP, name)

// usage       : TTL_7438_DIP(name)
#define TTL_7438_DIP(name) \
	NET_REGISTER_DEV(TTL_7438_DIP, name)

// usage       : TTL_7442_DIP(name)
#define TTL_7442_DIP(name) \
	NET_REGISTER_DEV(TTL_7442_DIP, name)

// usage       : TTL_7448_DIP(name)
#define TTL_7448_DIP(name) \
	NET_REGISTER_DEV(TTL_7448_DIP, name)

// usage       : TTL_7450_DIP(name)
#define TTL_7450_DIP(name) \
	NET_REGISTER_DEV(TTL_7450_DIP, name)

// usage       : TTL_7473_DIP(name)
#define TTL_7473_DIP(name) \
	NET_REGISTER_DEV(TTL_7473_DIP, name)

// usage       : TTL_7473A_DIP(name)
#define TTL_7473A_DIP(name) \
	NET_REGISTER_DEV(TTL_7473A_DIP, name)

// usage       : TTL_7474_DIP(name)
#define TTL_7474_DIP(name) \
	NET_REGISTER_DEV(TTL_7474_DIP, name)

// usage       : TTL_7475_DIP(name)
#define TTL_7475_DIP(name) \
	NET_REGISTER_DEV(TTL_7475_DIP, name)

// usage       : TTL_7477_DIP(name)
#define TTL_7477_DIP(name) \
	NET_REGISTER_DEV(TTL_7477_DIP, name)

// usage       : TTL_7483_DIP(name)
#define TTL_7483_DIP(name) \
	NET_REGISTER_DEV(TTL_7483_DIP, name)

// usage       : TTL_7485_DIP(name)
#define TTL_7485_DIP(name) \
	NET_REGISTER_DEV(TTL_7485_DIP, name)

// usage       : TTL_7486_DIP(name)
#define TTL_7486_DIP(name) \
	NET_REGISTER_DEV(TTL_7486_DIP, name)

// usage       : TTL_7490_DIP(name)
#define TTL_7490_DIP(name) \
	NET_REGISTER_DEV(TTL_7490_DIP, name)

// usage       : TTL_7492_DIP(name)
#define TTL_7492_DIP(name) \
	NET_REGISTER_DEV(TTL_7492_DIP, name)

// usage       : TTL_7493_DIP(name)
#define TTL_7493_DIP(name) \
	NET_REGISTER_DEV(TTL_7493_DIP, name)

// usage       : TTL_7497_DIP(name)
#define TTL_7497_DIP(name) \
	NET_REGISTER_DEV(TTL_7497_DIP, name)

// usage       : TTL_74107_DIP(name)
#define TTL_74107_DIP(name) \
	NET_REGISTER_DEV(TTL_74107_DIP, name)

// usage       : TTL_74107A_DIP(name)
#define TTL_74107A_DIP(name) \
	NET_REGISTER_DEV(TTL_74107A_DIP, name)

// usage       : TTL_74113_DIP(name)
#define TTL_74113_DIP(name) \
	NET_REGISTER_DEV(TTL_74113_DIP, name)

// usage       : TTL_74113A_DIP(name)
#define TTL_74113A_DIP(name) \
	NET_REGISTER_DEV(TTL_74113A_DIP, name)

// usage       : TTL_74121_DIP(name)
#define TTL_74121_DIP(name) \
	NET_REGISTER_DEV(TTL_74121_DIP, name)

// usage       : TTL_74123_DIP(name)
#define TTL_74123_DIP(name) \
	NET_REGISTER_DEV(TTL_74123_DIP, name)

// usage       : TTL_9602_DIP(name)
#define TTL_9602_DIP(name) \
	NET_REGISTER_DEV(TTL_9602_DIP, name)

// usage       : TTL_74125_DIP(name)
#define TTL_74125_DIP(name) \
	NET_REGISTER_DEV(TTL_74125_DIP, name)

// usage       : TTL_74126_DIP(name)
#define TTL_74126_DIP(name) \
	NET_REGISTER_DEV(TTL_74126_DIP, name)

// usage       : TTL_74139_DIP(name)
#define TTL_74139_DIP(name) \
	NET_REGISTER_DEV(TTL_74139_DIP, name)

// usage       : TTL_74147_DIP(name)
#define TTL_74147_DIP(name) \
	NET_REGISTER_DEV(TTL_74147_DIP, name)

// usage       : TTL_74148_DIP(name)
#define TTL_74148_DIP(name) \
	NET_REGISTER_DEV(TTL_74148_DIP, name)

// usage       : TTL_74151_DIP(name)
#define TTL_74151_DIP(name) \
	NET_REGISTER_DEV(TTL_74151_DIP, name)

// usage       : TTL_74153_DIP(name)
#define TTL_74153_DIP(name) \
	NET_REGISTER_DEV(TTL_74153_DIP, name)

// usage       : TTL_74155_DIP(name)
#define TTL_74155_DIP(name) \
	NET_REGISTER_DEV(TTL_74155_DIP, name)

// usage       : TTL_74156_DIP(name)
#define TTL_74156_DIP(name) \
	NET_REGISTER_DEV(TTL_74156_DIP, name)

// usage       : TTL_74157_DIP(name)
#define TTL_74157_DIP(name) \
	NET_REGISTER_DEV(TTL_74157_DIP, name)

// usage       : TTL_74161_DIP(name)
#define TTL_74161_DIP(name) \
	NET_REGISTER_DEV(TTL_74161_DIP, name)

// usage       : TTL_74163_DIP(name)
#define TTL_74163_DIP(name) \
	NET_REGISTER_DEV(TTL_74163_DIP, name)

// usage       : TTL_74164_DIP(name)
#define TTL_74164_DIP(name) \
	NET_REGISTER_DEV(TTL_74164_DIP, name)

// usage       : TTL_74165_DIP(name)
#define TTL_74165_DIP(name) \
	NET_REGISTER_DEV(TTL_74165_DIP, name)

// usage       : TTL_74166_DIP(name)
#define TTL_74166_DIP(name) \
	NET_REGISTER_DEV(TTL_74166_DIP, name)

// usage       : TTL_74174_DIP(name)
#define TTL_74174_DIP(name) \
	NET_REGISTER_DEV(TTL_74174_DIP, name)

// usage       : TTL_74175_DIP(name)
#define TTL_74175_DIP(name) \
	NET_REGISTER_DEV(TTL_74175_DIP, name)

// usage       : TTL_74192_DIP(name)
#define TTL_74192_DIP(name) \
	NET_REGISTER_DEV(TTL_74192_DIP, name)

// usage       : TTL_74193_DIP(name)
#define TTL_74193_DIP(name) \
	NET_REGISTER_DEV(TTL_74193_DIP, name)

// usage       : TTL_74194_DIP(name)
#define TTL_74194_DIP(name) \
	NET_REGISTER_DEV(TTL_74194_DIP, name)

// usage       : TTL_74260_DIP(name)
#define TTL_74260_DIP(name) \
	NET_REGISTER_DEV(TTL_74260_DIP, name)

// usage       : TTL_74279_DIP(name)
#define TTL_74279_DIP(name) \
	NET_REGISTER_DEV(TTL_74279_DIP, name)

// usage       : TTL_74290_DIP(name)
#define TTL_74290_DIP(name) \
	NET_REGISTER_DEV(TTL_74290_DIP, name)

// usage       : TTL_74293_DIP(name)
#define TTL_74293_DIP(name) \
	NET_REGISTER_DEV(TTL_74293_DIP, name)

// usage       : TTL_74365_DIP(name)
#define TTL_74365_DIP(name) \
	NET_REGISTER_DEV(TTL_74365_DIP, name)

// usage       : TTL_74368_DIP(name)
#define TTL_74368_DIP(name) \
	NET_REGISTER_DEV(TTL_74368_DIP, name)

// usage       : TTL_74377_DIP(name)
#define TTL_74377_DIP(name) \
	NET_REGISTER_DEV(TTL_74377_DIP, name)

// usage       : TTL_74378_DIP(name)
#define TTL_74378_DIP(name) \
	NET_REGISTER_DEV(TTL_74378_DIP, name)

// usage       : TTL_74379_DIP(name)
#define TTL_74379_DIP(name) \
	NET_REGISTER_DEV(TTL_74379_DIP, name)

// usage       : TTL_74393_DIP(name)
#define TTL_74393_DIP(name) \
	NET_REGISTER_DEV(TTL_74393_DIP, name)

// usage       : SN74LS629_DIP(name)
#define SN74LS629_DIP(name) \
	NET_REGISTER_DEV(SN74LS629_DIP, name)

// usage       : TTL_9310_DIP(name)
#define TTL_9310_DIP(name) \
	NET_REGISTER_DEV(TTL_9310_DIP, name)

// usage       : TTL_9312_DIP(name)
#define TTL_9312_DIP(name) \
	NET_REGISTER_DEV(TTL_9312_DIP, name)

// usage       : TTL_9314_DIP(name)
#define TTL_9314_DIP(name) \
	NET_REGISTER_DEV(TTL_9314_DIP, name)

// usage       : TTL_9316_DIP(name)
#define TTL_9316_DIP(name) \
	NET_REGISTER_DEV(TTL_9316_DIP, name)

// usage       : TTL_9321_DIP(name)
#define TTL_9321_DIP(name) \
	NET_REGISTER_DEV(TTL_9321_DIP, name)

// usage       : TTL_9322_DIP(name)
#define TTL_9322_DIP(name) \
	NET_REGISTER_DEV(TTL_9322_DIP, name)

// usage       : TTL_9334_DIP(name)
#define TTL_9334_DIP(name) \
	NET_REGISTER_DEV(TTL_9334_DIP, name)

// usage       : TTL_8277_DIP(name)
#define TTL_8277_DIP(name) \
	NET_REGISTER_DEV(TTL_8277_DIP, name)

// usage       : TTL_AM2847_DIP(name)
#define TTL_AM2847_DIP(name) \
	NET_REGISTER_DEV(TTL_AM2847_DIP, name)

// ---------------------------------------------------------------------
// Source: ../solver/nld_solver.cpp
// ---------------------------------------------------------------------

// usage       : SOLVER(name, FREQ)
#define SOLVER(name, ...) \
	__VA_OPT__(NET_CHECK_PARAM_COUNT(SOLVER, PNARGS(__VA_ARGS__), 1)) \
	NET_REGISTER_DEV(SOLVER, name __VA_OPT__(,) __VA_ARGS__)

#endif // __PLIB_PREPROCESSOR__

#endif // NLD_DEVINC_H

