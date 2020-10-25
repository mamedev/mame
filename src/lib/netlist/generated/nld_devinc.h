// ---------------------------------------------------------------------
// Source: ../analog/nld_bjt.cpp
// ---------------------------------------------------------------------

// usage       : QBJT_EB(name, MODEL)
#define QBJT_EB(...)                                                   \
	NET_REGISTER_DEVEXT(QBJT_EB, __VA_ARGS__)

// usage       : QBJT_SW(name, MODEL)
#define QBJT_SW(...)                                                   \
	NET_REGISTER_DEVEXT(QBJT_SW, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../analog/nld_mosfet.cpp
// ---------------------------------------------------------------------

// usage       : MOSFET(name, MODEL)
#define MOSFET(...)                                                   \
	NET_REGISTER_DEVEXT(MOSFET, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../analog/nld_opamps.cpp
// ---------------------------------------------------------------------

// usage       : OPAMP(name, MODEL)
#define OPAMP(...)                                                   \
	NET_REGISTER_DEVEXT(OPAMP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../analog/nld_switches.cpp
// ---------------------------------------------------------------------

// usage       : SWITCH(name, )
#define SWITCH(...)                                                   \
	NET_REGISTER_DEVEXT(SWITCH, __VA_ARGS__)

// usage       : SWITCH2(name, )
#define SWITCH2(...)                                                   \
	NET_REGISTER_DEVEXT(SWITCH2, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../analog/nlid_fourterm.cpp
// ---------------------------------------------------------------------

// usage       : VCVS(name, G)
#define VCVS(...)                                                   \
	NET_REGISTER_DEVEXT(VCVS, __VA_ARGS__)

// usage       : VCCS(name, G)
#define VCCS(...)                                                   \
	NET_REGISTER_DEVEXT(VCCS, __VA_ARGS__)

// usage       : CCCS(name, G)
#define CCCS(...)                                                   \
	NET_REGISTER_DEVEXT(CCCS, __VA_ARGS__)

// usage       : CCVS(name, G)
#define CCVS(...)                                                   \
	NET_REGISTER_DEVEXT(CCVS, __VA_ARGS__)

// usage       : LVCCS(name, )
#define LVCCS(...)                                                   \
	NET_REGISTER_DEVEXT(LVCCS, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../analog/nlid_twoterm.cpp
// ---------------------------------------------------------------------

// usage       : RES(name, R)
#define RES(...)                                                   \
	NET_REGISTER_DEVEXT(RES, __VA_ARGS__)

// usage       : POT(name, R)
#define POT(...)                                                   \
	NET_REGISTER_DEVEXT(POT, __VA_ARGS__)

// usage       : POT2(name, R)
#define POT2(...)                                                   \
	NET_REGISTER_DEVEXT(POT2, __VA_ARGS__)

// usage       : CAP(name, C)
#define CAP(...)                                                   \
	NET_REGISTER_DEVEXT(CAP, __VA_ARGS__)

// usage       : IND(name, L)
#define IND(...)                                                   \
	NET_REGISTER_DEVEXT(IND, __VA_ARGS__)

// usage       : DIODE(name, MODEL)
#define DIODE(...)                                                   \
	NET_REGISTER_DEVEXT(DIODE, __VA_ARGS__)

// usage       : ZDIODE(name, MODEL)
#define ZDIODE(...)                                                   \
	NET_REGISTER_DEVEXT(ZDIODE, __VA_ARGS__)

// usage       : VS(name, V)
#define VS(...)                                                   \
	NET_REGISTER_DEVEXT(VS, __VA_ARGS__)

// usage       : CS(name, I)
#define CS(...)                                                   \
	NET_REGISTER_DEVEXT(CS, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_2102a.cpp
// ---------------------------------------------------------------------

// usage       : RAM_2102A(name, CEQ, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, RWQ, DI)
// auto connect: VCC, GND
#define RAM_2102A(...)                                                   \
	NET_REGISTER_DEVEXT(RAM_2102A, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_4006.cpp
// ---------------------------------------------------------------------

// usage       : CD4006(name, CLOCK, D1, D2, D3, D4, D1P4, D1P4S, D2P4, D2P5, D3P4, D4P4, D4P5)
// auto connect: VCC, GND
#define CD4006(...)                                                   \
	NET_REGISTER_DEVEXT(CD4006, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_4013.cpp
// ---------------------------------------------------------------------

// usage       : CD4013(name, CLOCK, DATA, RESET, SET)
// auto connect: VDD, VSS
#define CD4013(...)                                                   \
	NET_REGISTER_DEVEXT(CD4013, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_4017.cpp
// ---------------------------------------------------------------------

// usage       : CD4017(name, )
#define CD4017(...)                                                   \
	NET_REGISTER_DEVEXT(CD4017, __VA_ARGS__)

// usage       : CD4022(name, )
#define CD4022(...)                                                   \
	NET_REGISTER_DEVEXT(CD4022, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_4020.cpp
// ---------------------------------------------------------------------

// usage       : CD4020(name, IP, RESET, VDD, VSS)
#define CD4020(...)                                                   \
	NET_REGISTER_DEVEXT(CD4020, __VA_ARGS__)

// usage       : CD4024(name, )
#define CD4024(...)                                                   \
	NET_REGISTER_DEVEXT(CD4024, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_4029.cpp
// ---------------------------------------------------------------------

// usage       : CD4029(name, PE, J1, J2, J3, J4, CI, UD, BD, CLK)
// auto connect: VCC, GND
#define CD4029(...)                                                   \
	NET_REGISTER_DEVEXT(CD4029, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_4042.cpp
// ---------------------------------------------------------------------

// usage       : CD4042(name, D1, D2, D3, D4, POL, CLK)
// auto connect: VCC, GND
#define CD4042(...)                                                   \
	NET_REGISTER_DEVEXT(CD4042, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_4053.cpp
// ---------------------------------------------------------------------

// usage       : CD4053_GATE(name, )
#define CD4053_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(CD4053_GATE, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_4066.cpp
// ---------------------------------------------------------------------

// usage       : CD4066_GATE(name, )
#define CD4066_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(CD4066_GATE, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_4076.cpp
// ---------------------------------------------------------------------

// usage       : CD4076(name, I1, I2, I3, I4, ID1, ID2, OD1, OD2)
// auto connect: VCC, GND
#define CD4076(...)                                                   \
	NET_REGISTER_DEVEXT(CD4076, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_4316.cpp
// ---------------------------------------------------------------------

// usage       : CD4316_GATE(name, )
#define CD4316_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(CD4316_GATE, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74107.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74107(name, CLK, J, K, CLRQ)
// auto connect: VCC, GND
#define TTL_74107(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74107, __VA_ARGS__)

// usage       : TTL_74107A(name, CLK, J, K, CLRQ)
// auto connect: VCC, GND
#define TTL_74107A(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74107A, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74113.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74113(name, CLK, J, K, CLRQ)
// auto connect: VCC, GND
#define TTL_74113(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74113, __VA_ARGS__)

// usage       : TTL_74113A(name, CLK, J, K, CLRQ)
// auto connect: VCC, GND
#define TTL_74113A(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74113A, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74123.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74123(name, )
#define TTL_74123(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74123, __VA_ARGS__)

// usage       : TTL_74121(name, )
#define TTL_74121(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74121, __VA_ARGS__)

// usage       : CD4538(name, )
#define CD4538(...)                                                   \
	NET_REGISTER_DEVEXT(CD4538, __VA_ARGS__)

// usage       : TTL_9602(name, )
#define TTL_9602(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_9602, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74125.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74125_GATE(name, )
#define TTL_74125_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74125_GATE, __VA_ARGS__)

// usage       : TTL_74126_GATE(name, )
#define TTL_74126_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74126_GATE, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74153.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74153(name, C0, C1, C2, C3, A, B, G)
// auto connect: VCC, GND
#define TTL_74153(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74153, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74161.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74161(name, CLK, ENP, ENT, CLRQ, LOADQ, A, B, C, D)
// auto connect: VCC, GND
#define TTL_74161(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74161, __VA_ARGS__)

// usage       : TTL_74161_FIXME(name, A, B, C, D, CLRQ, LOADQ, CLK, ENP, ENT)
// auto connect: VCC, GND
#define TTL_74161_FIXME(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74161_FIXME, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74163.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74163(name, CLK, ENP, ENT, CLRQ, LOADQ, A, B, C, D)
// auto connect: VCC, GND
#define TTL_74163(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74163, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74164.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74164(name, A, B, CLRQ, CLK)
// auto connect: VCC, GND
#define TTL_74164(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74164, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74165.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74165(name, CLK, CLKINH, SH_LDQ, SER, A, B, C, D, E, F, G, H)
// auto connect: VCC, GND
#define TTL_74165(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74165, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74166.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74166(name, CLK, CLKINH, SH_LDQ, SER, A, B, C, D, E, F, G, H, CLRQ)
// auto connect: VCC, GND
#define TTL_74166(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74166, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74174.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74174(name, CLK, D1, D2, D3, D4, D5, D6, CLRQ)
// auto connect: VCC, GND
#define TTL_74174(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74174, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74175.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74175(name, CLK, D1, D2, D3, D4, CLRQ)
// auto connect: VCC, GND
#define TTL_74175(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74175, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74192.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74192(name, A, B, C, D, CLEAR, LOADQ, CU, CD)
// auto connect: VCC, GND
#define TTL_74192(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74192, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74193.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74193(name, A, B, C, D, CLEAR, LOADQ, CU, CD)
// auto connect: VCC, GND
#define TTL_74193(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74193, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74194.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74194(name, CLK, S0, S1, SRIN, A, B, C, D, SLIN, CLRQ)
// auto connect: VCC, GND
#define TTL_74194(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74194, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74365.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74365(name, G1Q, G2Q, A1, A2, A3, A4, A5, A6)
// auto connect: VCC, GND
#define TTL_74365(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74365, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74377.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74377_GATE(name, )
#define TTL_74377_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74377_GATE, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74393.cpp
// ---------------------------------------------------------------------

// usage       : TTL_74393(name, CP, MR)
// auto connect: VCC, GND
#define TTL_74393(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74393, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_7448.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7448(name, A, B, C, D, LTQ, BIQ, RBIQ)
// auto connect: VCC, GND
#define TTL_7448(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7448, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_7450.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7450_ANDORINVERT(name, A, B, C, D)
// auto connect: VCC, GND
#define TTL_7450_ANDORINVERT(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7450_ANDORINVERT, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_7473.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7473(name, CLK, J, K, CLRQ)
// auto connect: VCC, GND
#define TTL_7473(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7473, __VA_ARGS__)

// usage       : TTL_7473A(name, CLK, J, K, CLRQ)
// auto connect: VCC, GND
#define TTL_7473A(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7473A, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_7474.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7474(name, CLK, D, CLRQ, PREQ)
// auto connect: VCC, GND
#define TTL_7474(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7474, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_7475.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7475_GATE(name, )
#define TTL_7475_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7475_GATE, __VA_ARGS__)

// usage       : TTL_7477_GATE(name, )
#define TTL_7477_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7477_GATE, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_7483.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7483(name, A1, A2, A3, A4, B1, B2, B3, B4, C0)
// auto connect: VCC, GND
#define TTL_7483(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7483, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_7485.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7485(name, A0, A1, A2, A3, B0, B1, B2, B3, LTIN, EQIN, GTIN)
// auto connect: VCC, GND
#define TTL_7485(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7485, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_7490.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7490(name, A, B, R1, R2, R91, R92)
// auto connect: VCC, GND
#define TTL_7490(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7490, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_7492.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7492(name, A, B, R1, R2)
// auto connect: VCC, GND
#define TTL_7492(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7492, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_7493.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7493(name, CLKA, CLKB, R1, R2)
// auto connect: VCC, GND
#define TTL_7493(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7493, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_7497.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7497(name, CLK, STRBQ, ENQ, UNITYQ, CLR, B0, B1, B2, B3, B4, B5)
// auto connect: VCC, GND
#define TTL_7497(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7497, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_74ls629.cpp
// ---------------------------------------------------------------------

// usage       : SN74LS629(name, CAP)
// auto connect: VCC, GND
#define SN74LS629(...)                                                   \
	NET_REGISTER_DEVEXT(SN74LS629, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_8277.cpp
// ---------------------------------------------------------------------

// usage       : TTL_8277(name, RESET, CLK, CLKA, D0A, D1A, DSA, CLKB, D0B, D1B, DSB)
// auto connect: VCC, GND
#define TTL_8277(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_8277, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_82s115.cpp
// ---------------------------------------------------------------------

// usage       : PROM_82S115(name, CE1Q, CE2, A0, A1, A2, A3, A4, A5, A6, A7, A8, STROBE)
// auto connect: VCC, GND
#define PROM_82S115(...)                                                   \
	NET_REGISTER_DEVEXT(PROM_82S115, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_82s16.cpp
// ---------------------------------------------------------------------

// usage       : TTL_82S16(name, )
#define TTL_82S16(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_82S16, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_9310.cpp
// ---------------------------------------------------------------------

// usage       : TTL_9310(name, CLK, ENP, ENT, CLRQ, LOADQ, A, B, C, D)
// auto connect: VCC, GND
#define TTL_9310(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_9310, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_9316.cpp
// ---------------------------------------------------------------------

// usage       : TTL_9316(name, CLK, ENP, ENT, CLRQ, LOADQ, A, B, C, D)
// auto connect: VCC, GND
#define TTL_9316(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_9316, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_9321.cpp
// ---------------------------------------------------------------------

// usage       : TTL_9321(name, E, A0, A1)
#define TTL_9321(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_9321, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_9322.cpp
// ---------------------------------------------------------------------

// usage       : TTL_9322(name, SELECT, A1, B1, A2, B2, A3, B3, A4, B4, STROBE)
// auto connect: VCC, GND
#define TTL_9322(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_9322, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_am2847.cpp
// ---------------------------------------------------------------------

// usage       : TTL_AM2847(name, CP, INA, INB, INC, IND, RCA, RCB, RCC, RCD)
// auto connect: VSS, VDD
#define TTL_AM2847(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_AM2847, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_dm9314.cpp
// ---------------------------------------------------------------------

// usage       : TTL_9314(name, EQ, MRQ, S0Q, S1Q, S2Q, S3Q, D0, D1, D2, D3)
// auto connect: VCC, GND
#define TTL_9314(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_9314, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_dm9334.cpp
// ---------------------------------------------------------------------

// usage       : TTL_9334(name, CQ, EQ, D, A0, A1, A2)
// auto connect: VCC, GND
#define TTL_9334(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_9334, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_legacy.cpp
// ---------------------------------------------------------------------

// usage       : NETDEV_RSFF(name, )
#define NETDEV_RSFF(...)                                                   \
	NET_REGISTER_DEVEXT(NETDEV_RSFF, __VA_ARGS__)

// usage       : NETDEV_DELAY(name, )
#define NETDEV_DELAY(...)                                                   \
	NET_REGISTER_DEVEXT(NETDEV_DELAY, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_log.cpp
// ---------------------------------------------------------------------

// usage       : LOG(name, I)
#define LOG(...)                                                   \
	NET_REGISTER_DEVEXT(LOG, __VA_ARGS__)

// usage       : LOGD(name, I, I2)
#define LOGD(...)                                                   \
	NET_REGISTER_DEVEXT(LOGD, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_mm5837.cpp
// ---------------------------------------------------------------------

// usage       : MM5837(name, )
#define MM5837(...)                                                   \
	NET_REGISTER_DEVEXT(MM5837, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_ne555.cpp
// ---------------------------------------------------------------------

// usage       : NE555(name, )
#define NE555(...)                                                   \
	NET_REGISTER_DEVEXT(NE555, __VA_ARGS__)

// usage       : MC1455P(name, )
#define MC1455P(...)                                                   \
	NET_REGISTER_DEVEXT(MC1455P, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_r2r_dac.cpp
// ---------------------------------------------------------------------

// usage       : R2R_DAC(name, VIN, R, N)
#define R2R_DAC(...)                                                   \
	NET_REGISTER_DEVEXT(R2R_DAC, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_roms.cpp
// ---------------------------------------------------------------------

// usage       : PROM_82S126(name, CE1Q, CE2Q, A0, A1, A2, A3, A4, A5, A6, A7)
// auto connect: VCC, GND
#define PROM_82S126(...)                                                   \
	NET_REGISTER_DEVEXT(PROM_82S126, __VA_ARGS__)

// usage       : PROM_74S287(name, CE1Q, CE2Q, A0, A1, A2, A3, A4, A5, A6, A7)
// auto connect: VCC, GND
#define PROM_74S287(...)                                                   \
	NET_REGISTER_DEVEXT(PROM_74S287, __VA_ARGS__)

// usage       : PROM_82S123(name, CEQ, A0, A1, A2, A3, A4)
// auto connect: VCC, GND
#define PROM_82S123(...)                                                   \
	NET_REGISTER_DEVEXT(PROM_82S123, __VA_ARGS__)

// usage       : EPROM_2716(name, CE2Q, CE1Q, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)
// auto connect: VCC, GND
#define EPROM_2716(...)                                                   \
	NET_REGISTER_DEVEXT(EPROM_2716, __VA_ARGS__)

// usage       : PROM_MK28000(name, OE1, OE2, ARQ, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)
// auto connect: VCC, GND
#define PROM_MK28000(...)                                                   \
	NET_REGISTER_DEVEXT(PROM_MK28000, __VA_ARGS__)

// usage       : ROM_MCM14524(name, EN, CLK, A0, A1, A2, A3, A4, A5, A6, A7)
// auto connect: VCC, GND
#define ROM_MCM14524(...)                                                   \
	NET_REGISTER_DEVEXT(ROM_MCM14524, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_schmitt.cpp
// ---------------------------------------------------------------------

// usage       : SCHMITT_TRIGGER(name, STMODEL)
#define SCHMITT_TRIGGER(...)                                                   \
	NET_REGISTER_DEVEXT(SCHMITT_TRIGGER, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_system.cpp
// ---------------------------------------------------------------------

// usage       : PARAMETER(name, )
#define PARAMETER(...)                                                   \
	NET_REGISTER_DEVEXT(PARAMETER, __VA_ARGS__)

// usage       : NC_PIN(name, )
#define NC_PIN(...)                                                   \
	NET_REGISTER_DEVEXT(NC_PIN, __VA_ARGS__)

// usage       : FRONTIER_DEV(name, I, G, Q)
#define FRONTIER_DEV(...)                                                   \
	NET_REGISTER_DEVEXT(FRONTIER_DEV, __VA_ARGS__)

// usage       : AFUNC(name, N, FUNC)
#define AFUNC(...)                                                   \
	NET_REGISTER_DEVEXT(AFUNC, __VA_ARGS__)

// usage       : ANALOG_INPUT(name, IN)
#define ANALOG_INPUT(...)                                                   \
	NET_REGISTER_DEVEXT(ANALOG_INPUT, __VA_ARGS__)

// usage       : CLOCK(name, FREQ)
#define CLOCK(...)                                                   \
	NET_REGISTER_DEVEXT(CLOCK, __VA_ARGS__)

// usage       : VARCLOCK(name, N, FUNC)
#define VARCLOCK(...)                                                   \
	NET_REGISTER_DEVEXT(VARCLOCK, __VA_ARGS__)

// usage       : EXTCLOCK(name, FREQ, PATTERN)
#define EXTCLOCK(...)                                                   \
	NET_REGISTER_DEVEXT(EXTCLOCK, __VA_ARGS__)

// usage       : SYS_DSW(name, I, 1, 2)
#define SYS_DSW(...)                                                   \
	NET_REGISTER_DEVEXT(SYS_DSW, __VA_ARGS__)

// usage       : SYS_DSW2(name, )
#define SYS_DSW2(...)                                                   \
	NET_REGISTER_DEVEXT(SYS_DSW2, __VA_ARGS__)

// usage       : SYS_COMPD(name, )
#define SYS_COMPD(...)                                                   \
	NET_REGISTER_DEVEXT(SYS_COMPD, __VA_ARGS__)

// usage       : SYS_NOISE_MT_U(name, SIGMA)
#define SYS_NOISE_MT_U(...)                                                   \
	NET_REGISTER_DEVEXT(SYS_NOISE_MT_U, __VA_ARGS__)

// usage       : SYS_NOISE_MT_N(name, SIGMA)
#define SYS_NOISE_MT_N(...)                                                   \
	NET_REGISTER_DEVEXT(SYS_NOISE_MT_N, __VA_ARGS__)

// usage       : MAINCLOCK(name, FREQ)
#define MAINCLOCK(...)                                                   \
	NET_REGISTER_DEVEXT(MAINCLOCK, __VA_ARGS__)

// usage       : GNDA(name, )
#define GNDA(...)                                                   \
	NET_REGISTER_DEVEXT(GNDA, __VA_ARGS__)

// usage       : LOGIC_INPUT8(name, IN, MODEL)
#define LOGIC_INPUT8(...)                                                   \
	NET_REGISTER_DEVEXT(LOGIC_INPUT8, __VA_ARGS__)

// usage       : LOGIC_INPUT(name, IN, MODEL)
#define LOGIC_INPUT(...)                                                   \
	NET_REGISTER_DEVEXT(LOGIC_INPUT, __VA_ARGS__)

// usage       : TTL_INPUT(name, IN)
#define TTL_INPUT(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_INPUT, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_tms4800.cpp
// ---------------------------------------------------------------------

// usage       : ROM_TMS4800(name, AR, OE1, OE2, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)
// auto connect: VCC, GND
#define ROM_TMS4800(...)                                                   \
	NET_REGISTER_DEVEXT(ROM_TMS4800, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../devices/nld_tristate.cpp
// ---------------------------------------------------------------------

// usage       : TTL_TRISTATE(name, CEQ1, D1, CEQ2, D2)
#define TTL_TRISTATE(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_TRISTATE, __VA_ARGS__)

// usage       : TTL_TRISTATE3(name, )
#define TTL_TRISTATE3(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_TRISTATE3, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../macro/nlm_cd4xxx_lib.cpp
// ---------------------------------------------------------------------

// usage       : CD4001_GATE(name, )
#define CD4001_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(CD4001_GATE, __VA_ARGS__)

// usage       : CD4011_GATE(name, )
#define CD4011_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(CD4011_GATE, __VA_ARGS__)

// usage       : CD4030_GATE(name, )
#define CD4030_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(CD4030_GATE, __VA_ARGS__)

// usage       : CD4049_GATE(name, )
#define CD4049_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(CD4049_GATE, __VA_ARGS__)

// usage       : CD4069_GATE(name, )
#define CD4069_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(CD4069_GATE, __VA_ARGS__)

// usage       : CD4070_GATE(name, )
#define CD4070_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(CD4070_GATE, __VA_ARGS__)

// usage       : CD4071_GATE(name, )
#define CD4071_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(CD4071_GATE, __VA_ARGS__)

// usage       : CD4081_GATE(name, )
#define CD4081_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(CD4081_GATE, __VA_ARGS__)

// usage       : CD4001_DIP(name, )
#define CD4001_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(CD4001_DIP, __VA_ARGS__)

// usage       : CD4011_DIP(name, )
#define CD4011_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(CD4011_DIP, __VA_ARGS__)

// usage       : CD4030_DIP(name, )
#define CD4030_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(CD4030_DIP, __VA_ARGS__)

// usage       : CD4049_DIP(name, )
#define CD4049_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(CD4049_DIP, __VA_ARGS__)

// usage       : CD4069_DIP(name, )
#define CD4069_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(CD4069_DIP, __VA_ARGS__)

// usage       : CD4070_DIP(name, )
#define CD4070_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(CD4070_DIP, __VA_ARGS__)

// usage       : CD4071_DIP(name, )
#define CD4071_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(CD4071_DIP, __VA_ARGS__)

// usage       : CD4081_DIP(name, )
#define CD4081_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(CD4081_DIP, __VA_ARGS__)

// usage       : CD4006_DIP(name, )
#define CD4006_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(CD4006_DIP, __VA_ARGS__)

// usage       : CD4013_DIP(name, )
#define CD4013_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(CD4013_DIP, __VA_ARGS__)

// usage       : CD4017_DIP(name, )
#define CD4017_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(CD4017_DIP, __VA_ARGS__)

// usage       : CD4022_DIP(name, )
#define CD4022_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(CD4022_DIP, __VA_ARGS__)

// usage       : CD4020_DIP(name, )
#define CD4020_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(CD4020_DIP, __VA_ARGS__)

// usage       : CD4024_DIP(name, )
#define CD4024_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(CD4024_DIP, __VA_ARGS__)

// usage       : CD4029_DIP(name, )
#define CD4029_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(CD4029_DIP, __VA_ARGS__)

// usage       : CD4042_DIP(name, )
#define CD4042_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(CD4042_DIP, __VA_ARGS__)

// usage       : CD4053_DIP(name, )
#define CD4053_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(CD4053_DIP, __VA_ARGS__)

// usage       : CD4066_DIP(name, )
#define CD4066_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(CD4066_DIP, __VA_ARGS__)

// usage       : CD4016_DIP(name, )
#define CD4016_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(CD4016_DIP, __VA_ARGS__)

// usage       : CD4076_DIP(name, )
#define CD4076_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(CD4076_DIP, __VA_ARGS__)

// usage       : CD4316_DIP(name, )
#define CD4316_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(CD4316_DIP, __VA_ARGS__)

// usage       : CD4538_DIP(name, )
#define CD4538_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(CD4538_DIP, __VA_ARGS__)

// usage       : MM5837_DIP(name, )
#define MM5837_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(MM5837_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../macro/nlm_opamp_lib.cpp
// ---------------------------------------------------------------------

// usage       : opamp_layout_4_4_11(name, )
#define opamp_layout_4_4_11(...)                                                   \
	NET_REGISTER_DEVEXT(opamp_layout_4_4_11, __VA_ARGS__)

// usage       : opamp_layout_2_8_4(name, )
#define opamp_layout_2_8_4(...)                                                   \
	NET_REGISTER_DEVEXT(opamp_layout_2_8_4, __VA_ARGS__)

// usage       : opamp_layout_2_13_9_4(name, )
#define opamp_layout_2_13_9_4(...)                                                   \
	NET_REGISTER_DEVEXT(opamp_layout_2_13_9_4, __VA_ARGS__)

// usage       : opamp_layout_1_7_4(name, )
#define opamp_layout_1_7_4(...)                                                   \
	NET_REGISTER_DEVEXT(opamp_layout_1_7_4, __VA_ARGS__)

// usage       : opamp_layout_1_8_5(name, )
#define opamp_layout_1_8_5(...)                                                   \
	NET_REGISTER_DEVEXT(opamp_layout_1_8_5, __VA_ARGS__)

// usage       : opamp_layout_1_11_6(name, )
#define opamp_layout_1_11_6(...)                                                   \
	NET_REGISTER_DEVEXT(opamp_layout_1_11_6, __VA_ARGS__)

// usage       : MB3614_DIP(name, )
#define MB3614_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(MB3614_DIP, __VA_ARGS__)

// usage       : MC3340_DIP(name, )
#define MC3340_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(MC3340_DIP, __VA_ARGS__)

// usage       : TL081_DIP(name, )
#define TL081_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TL081_DIP, __VA_ARGS__)

// usage       : TL082_DIP(name, )
#define TL082_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TL082_DIP, __VA_ARGS__)

// usage       : TL084_DIP(name, )
#define TL084_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TL084_DIP, __VA_ARGS__)

// usage       : LM324_DIP(name, )
#define LM324_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(LM324_DIP, __VA_ARGS__)

// usage       : LM348_DIP(name, )
#define LM348_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(LM348_DIP, __VA_ARGS__)

// usage       : LM358_DIP(name, )
#define LM358_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(LM358_DIP, __VA_ARGS__)

// usage       : LM2902_DIP(name, )
#define LM2902_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(LM2902_DIP, __VA_ARGS__)

// usage       : UA741_DIP8(name, )
#define UA741_DIP8(...)                                                   \
	NET_REGISTER_DEVEXT(UA741_DIP8, __VA_ARGS__)

// usage       : UA741_DIP10(name, )
#define UA741_DIP10(...)                                                   \
	NET_REGISTER_DEVEXT(UA741_DIP10, __VA_ARGS__)

// usage       : UA741_DIP14(name, )
#define UA741_DIP14(...)                                                   \
	NET_REGISTER_DEVEXT(UA741_DIP14, __VA_ARGS__)

// usage       : MC1558_DIP(name, )
#define MC1558_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(MC1558_DIP, __VA_ARGS__)

// usage       : LM747_DIP(name, )
#define LM747_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(LM747_DIP, __VA_ARGS__)

// usage       : LM747A_DIP(name, )
#define LM747A_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(LM747A_DIP, __VA_ARGS__)

// usage       : LM3900(name, )
#define LM3900(...)                                                   \
	NET_REGISTER_DEVEXT(LM3900, __VA_ARGS__)

// usage       : AN6551_SIL(name, )
#define AN6551_SIL(...)                                                   \
	NET_REGISTER_DEVEXT(AN6551_SIL, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../macro/nlm_otheric_lib.cpp
// ---------------------------------------------------------------------

// usage       : MC14584B_GATE(name, )
#define MC14584B_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(MC14584B_GATE, __VA_ARGS__)

// usage       : MC14584B_DIP(name, )
#define MC14584B_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(MC14584B_DIP, __VA_ARGS__)

// usage       : NE566_DIP(name, )
#define NE566_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(NE566_DIP, __VA_ARGS__)

// usage       : NE555_DIP(name, )
#define NE555_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(NE555_DIP, __VA_ARGS__)

// usage       : MC1455P_DIP(name, )
#define MC1455P_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(MC1455P_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../macro/nlm_roms_lib.cpp
// ---------------------------------------------------------------------

// usage       : PROM_82S123_DIP(name, )
#define PROM_82S123_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(PROM_82S123_DIP, __VA_ARGS__)

// usage       : PROM_82S126_DIP(name, )
#define PROM_82S126_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(PROM_82S126_DIP, __VA_ARGS__)

// usage       : PROM_74S287_DIP(name, )
#define PROM_74S287_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(PROM_74S287_DIP, __VA_ARGS__)

// usage       : EPROM_2716_DIP(name, )
#define EPROM_2716_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(EPROM_2716_DIP, __VA_ARGS__)

// usage       : TTL_82S16_DIP(name, )
#define TTL_82S16_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_82S16_DIP, __VA_ARGS__)

// usage       : PROM_82S115_DIP(name, )
#define PROM_82S115_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(PROM_82S115_DIP, __VA_ARGS__)

// usage       : PROM_MK28000_DIP(name, )
#define PROM_MK28000_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(PROM_MK28000_DIP, __VA_ARGS__)

// usage       : ROM_MCM14524_DIP(name, )
#define ROM_MCM14524_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(ROM_MCM14524_DIP, __VA_ARGS__)

// usage       : RAM_2102A_DIP(name, )
#define RAM_2102A_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(RAM_2102A_DIP, __VA_ARGS__)

// usage       : ROM_TMS4800_DIP(name, )
#define ROM_TMS4800_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(ROM_TMS4800_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../macro/nlm_ttl74xx_lib.cpp
// ---------------------------------------------------------------------

// usage       : TTL_7400_NAND(name, A, B)
// auto connect: VCC, GND
#define TTL_7400_NAND(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7400_NAND, __VA_ARGS__)

// usage       : TTL_7402_NOR(name, A, B)
// auto connect: VCC, GND
#define TTL_7402_NOR(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7402_NOR, __VA_ARGS__)

// usage       : TTL_7404_INVERT(name, A)
// auto connect: VCC, GND
#define TTL_7404_INVERT(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7404_INVERT, __VA_ARGS__)

// usage       : TTL_7406_GATE(name, )
#define TTL_7406_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7406_GATE, __VA_ARGS__)

// usage       : TTL_7407_GATE(name, )
#define TTL_7407_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7407_GATE, __VA_ARGS__)

// usage       : TTL_7408_GATE(name, )
#define TTL_7408_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7408_GATE, __VA_ARGS__)

// usage       : TTL_7408_AND(name, A, B)
// auto connect: VCC, GND
#define TTL_7408_AND(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7408_AND, __VA_ARGS__)

// usage       : TTL_7410_NAND(name, A, B, C)
// auto connect: VCC, GND
#define TTL_7410_NAND(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7410_NAND, __VA_ARGS__)

// usage       : TTL_7410_GATE(name, )
#define TTL_7410_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7410_GATE, __VA_ARGS__)

// usage       : TTL_7411_AND(name, A, B, C)
// auto connect: VCC, GND
#define TTL_7411_AND(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7411_AND, __VA_ARGS__)

// usage       : TTL_7411_GATE(name, )
#define TTL_7411_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7411_GATE, __VA_ARGS__)

// usage       : TTL_7416_GATE(name, )
#define TTL_7416_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7416_GATE, __VA_ARGS__)

// usage       : TTL_7417_GATE(name, )
#define TTL_7417_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7417_GATE, __VA_ARGS__)

// usage       : TTL_7420_NAND(name, A, B, C, D)
// auto connect: VCC, GND
#define TTL_7420_NAND(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7420_NAND, __VA_ARGS__)

// usage       : TTL_7421_AND(name, A, B, C, D)
// auto connect: VCC, GND
#define TTL_7421_AND(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7421_AND, __VA_ARGS__)

// usage       : TTL_7425_NOR(name, A, B, C, D)
// auto connect: VCC, GND
#define TTL_7425_NOR(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7425_NOR, __VA_ARGS__)

// usage       : TTL_7427_NOR(name, A, B, C)
// auto connect: VCC, GND
#define TTL_7427_NOR(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7427_NOR, __VA_ARGS__)

// usage       : TTL_7430_NAND(name, A, B, C, D, E, F, G, H)
// auto connect: VCC, GND
#define TTL_7430_NAND(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7430_NAND, __VA_ARGS__)

// usage       : TTL_7432_OR(name, A, B)
// auto connect: VCC, GND
#define TTL_7432_OR(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7432_OR, __VA_ARGS__)

// usage       : TTL_7437_NAND(name, A, B)
#define TTL_7437_NAND(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7437_NAND, __VA_ARGS__)

// usage       : TTL_7438_NAND(name, A, B)
#define TTL_7438_NAND(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7438_NAND, __VA_ARGS__)

// usage       : TTL_7442(name, )
#define TTL_7442(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7442, __VA_ARGS__)

// usage       : TTL_7486_XOR(name, A, B)
// auto connect: VCC, GND
#define TTL_7486_XOR(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7486_XOR, __VA_ARGS__)

// usage       : TTL_74139_GATE(name, )
#define TTL_74139_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74139_GATE, __VA_ARGS__)

// usage       : TTL_74155A_GATE(name, )
#define TTL_74155A_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74155A_GATE, __VA_ARGS__)

// usage       : TTL_74155B_GATE(name, )
#define TTL_74155B_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74155B_GATE, __VA_ARGS__)

// usage       : TTL_74156A_GATE(name, )
#define TTL_74156A_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74156A_GATE, __VA_ARGS__)

// usage       : TTL_74156B_GATE(name, )
#define TTL_74156B_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74156B_GATE, __VA_ARGS__)

// usage       : TTL_74157_GATE(name, )
#define TTL_74157_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74157_GATE, __VA_ARGS__)

// usage       : TTL_74260_NOR(name, A, B, C, D, E)
// auto connect: VCC, GND
#define TTL_74260_NOR(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74260_NOR, __VA_ARGS__)

// usage       : TTL_74279A(name, )
#define TTL_74279A(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74279A, __VA_ARGS__)

// usage       : TTL_74279B(name, )
#define TTL_74279B(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74279B, __VA_ARGS__)

// usage       : TTL_9312(name, A, B, C, G, D0, D1, D2, D3, D4, D5, D6, D7)
// auto connect: VCC, GND
#define TTL_9312(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_9312, __VA_ARGS__)

// usage       : TTL_7400_DIP(name, )
#define TTL_7400_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7400_DIP, __VA_ARGS__)

// usage       : TTL_7402_DIP(name, )
#define TTL_7402_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7402_DIP, __VA_ARGS__)

// usage       : TTL_7404_DIP(name, )
#define TTL_7404_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7404_DIP, __VA_ARGS__)

// usage       : TTL_7406_DIP(name, )
#define TTL_7406_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7406_DIP, __VA_ARGS__)

// usage       : TTL_7407_DIP(name, )
#define TTL_7407_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7407_DIP, __VA_ARGS__)

// usage       : TTL_7408_DIP(name, )
#define TTL_7408_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7408_DIP, __VA_ARGS__)

// usage       : TTL_7410_DIP(name, )
#define TTL_7410_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7410_DIP, __VA_ARGS__)

// usage       : TTL_7411_DIP(name, )
#define TTL_7411_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7411_DIP, __VA_ARGS__)

// usage       : TTL_7414_GATE(name, )
#define TTL_7414_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7414_GATE, __VA_ARGS__)

// usage       : TTL_74LS14_GATE(name, )
#define TTL_74LS14_GATE(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74LS14_GATE, __VA_ARGS__)

// usage       : TTL_7414_DIP(name, )
#define TTL_7414_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7414_DIP, __VA_ARGS__)

// usage       : TTL_74LS14_DIP(name, )
#define TTL_74LS14_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74LS14_DIP, __VA_ARGS__)

// usage       : TTL_7416_DIP(name, )
#define TTL_7416_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7416_DIP, __VA_ARGS__)

// usage       : TTL_7417_DIP(name, )
#define TTL_7417_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7417_DIP, __VA_ARGS__)

// usage       : TTL_7420_DIP(name, )
#define TTL_7420_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7420_DIP, __VA_ARGS__)

// usage       : TTL_7421_DIP(name, )
#define TTL_7421_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7421_DIP, __VA_ARGS__)

// usage       : TTL_7425_DIP(name, )
#define TTL_7425_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7425_DIP, __VA_ARGS__)

// usage       : TTL_7427_DIP(name, )
#define TTL_7427_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7427_DIP, __VA_ARGS__)

// usage       : TTL_7430_DIP(name, )
#define TTL_7430_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7430_DIP, __VA_ARGS__)

// usage       : TTL_7432_DIP(name, )
#define TTL_7432_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7432_DIP, __VA_ARGS__)

// usage       : TTL_7437_DIP(name, )
#define TTL_7437_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7437_DIP, __VA_ARGS__)

// usage       : TTL_7438_DIP(name, )
#define TTL_7438_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7438_DIP, __VA_ARGS__)

// usage       : TTL_7442_DIP(name, )
#define TTL_7442_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7442_DIP, __VA_ARGS__)

// usage       : TTL_7448_DIP(name, )
#define TTL_7448_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7448_DIP, __VA_ARGS__)

// usage       : TTL_7450_DIP(name, )
#define TTL_7450_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7450_DIP, __VA_ARGS__)

// usage       : TTL_7473_DIP(name, )
#define TTL_7473_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7473_DIP, __VA_ARGS__)

// usage       : TTL_7473A_DIP(name, )
#define TTL_7473A_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7473A_DIP, __VA_ARGS__)

// usage       : TTL_7474_DIP(name, )
#define TTL_7474_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7474_DIP, __VA_ARGS__)

// usage       : TTL_7475_DIP(name, )
#define TTL_7475_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7475_DIP, __VA_ARGS__)

// usage       : TTL_7477_DIP(name, )
#define TTL_7477_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7477_DIP, __VA_ARGS__)

// usage       : TTL_7483_DIP(name, )
#define TTL_7483_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7483_DIP, __VA_ARGS__)

// usage       : TTL_7485_DIP(name, )
#define TTL_7485_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7485_DIP, __VA_ARGS__)

// usage       : TTL_7486_DIP(name, )
#define TTL_7486_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7486_DIP, __VA_ARGS__)

// usage       : TTL_7490_DIP(name, )
#define TTL_7490_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7490_DIP, __VA_ARGS__)

// usage       : TTL_7492_DIP(name, )
#define TTL_7492_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7492_DIP, __VA_ARGS__)

// usage       : TTL_7493_DIP(name, )
#define TTL_7493_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7493_DIP, __VA_ARGS__)

// usage       : TTL_7497_DIP(name, )
#define TTL_7497_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_7497_DIP, __VA_ARGS__)

// usage       : TTL_74107_DIP(name, )
#define TTL_74107_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74107_DIP, __VA_ARGS__)

// usage       : TTL_74107A_DIP(name, )
#define TTL_74107A_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74107A_DIP, __VA_ARGS__)

// usage       : TTL_74113_DIP(name, )
#define TTL_74113_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74113_DIP, __VA_ARGS__)

// usage       : TTL_74113A_DIP(name, )
#define TTL_74113A_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74113A_DIP, __VA_ARGS__)

// usage       : TTL_74121_DIP(name, )
#define TTL_74121_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74121_DIP, __VA_ARGS__)

// usage       : TTL_74123_DIP(name, )
#define TTL_74123_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74123_DIP, __VA_ARGS__)

// usage       : TTL_9602_DIP(name, )
#define TTL_9602_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_9602_DIP, __VA_ARGS__)

// usage       : TTL_74125_DIP(name, )
#define TTL_74125_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74125_DIP, __VA_ARGS__)

// usage       : TTL_74126_DIP(name, )
#define TTL_74126_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74126_DIP, __VA_ARGS__)

// usage       : TTL_74139_DIP(name, )
#define TTL_74139_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74139_DIP, __VA_ARGS__)

// usage       : TTL_74153_DIP(name, )
#define TTL_74153_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74153_DIP, __VA_ARGS__)

// usage       : TTL_74155_DIP(name, )
#define TTL_74155_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74155_DIP, __VA_ARGS__)

// usage       : TTL_74156_DIP(name, )
#define TTL_74156_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74156_DIP, __VA_ARGS__)

// usage       : TTL_74157_DIP(name, )
#define TTL_74157_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74157_DIP, __VA_ARGS__)

// usage       : TTL_74161_DIP(name, )
#define TTL_74161_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74161_DIP, __VA_ARGS__)

// usage       : TTL_74163_DIP(name, )
#define TTL_74163_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74163_DIP, __VA_ARGS__)

// usage       : TTL_74164_DIP(name, )
#define TTL_74164_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74164_DIP, __VA_ARGS__)

// usage       : TTL_74165_DIP(name, )
#define TTL_74165_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74165_DIP, __VA_ARGS__)

// usage       : TTL_74166_DIP(name, )
#define TTL_74166_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74166_DIP, __VA_ARGS__)

// usage       : TTL_74174_DIP(name, )
#define TTL_74174_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74174_DIP, __VA_ARGS__)

// usage       : TTL_74175_DIP(name, )
#define TTL_74175_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74175_DIP, __VA_ARGS__)

// usage       : TTL_74192_DIP(name, )
#define TTL_74192_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74192_DIP, __VA_ARGS__)

// usage       : TTL_74193_DIP(name, )
#define TTL_74193_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74193_DIP, __VA_ARGS__)

// usage       : TTL_74194_DIP(name, )
#define TTL_74194_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74194_DIP, __VA_ARGS__)

// usage       : TTL_74260_DIP(name, )
#define TTL_74260_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74260_DIP, __VA_ARGS__)

// usage       : TTL_74279_DIP(name, )
#define TTL_74279_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74279_DIP, __VA_ARGS__)

// usage       : TTL_74290_DIP(name, )
#define TTL_74290_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74290_DIP, __VA_ARGS__)

// usage       : TTL_74293_DIP(name, )
#define TTL_74293_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74293_DIP, __VA_ARGS__)

// usage       : TTL_74365_DIP(name, )
#define TTL_74365_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74365_DIP, __VA_ARGS__)

// usage       : TTL_74377_DIP(name, )
#define TTL_74377_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74377_DIP, __VA_ARGS__)

// usage       : TTL_74378_DIP(name, )
#define TTL_74378_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74378_DIP, __VA_ARGS__)

// usage       : TTL_74379_DIP(name, )
#define TTL_74379_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74379_DIP, __VA_ARGS__)

// usage       : TTL_74393_DIP(name, )
#define TTL_74393_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_74393_DIP, __VA_ARGS__)

// usage       : SN74LS629_DIP(name, )
#define SN74LS629_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(SN74LS629_DIP, __VA_ARGS__)

// usage       : TTL_9310_DIP(name, )
#define TTL_9310_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_9310_DIP, __VA_ARGS__)

// usage       : TTL_9312_DIP(name, )
#define TTL_9312_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_9312_DIP, __VA_ARGS__)

// usage       : TTL_9314_DIP(name, )
#define TTL_9314_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_9314_DIP, __VA_ARGS__)

// usage       : TTL_9316_DIP(name, )
#define TTL_9316_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_9316_DIP, __VA_ARGS__)

// usage       : TTL_9321_DIP(name, )
#define TTL_9321_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_9321_DIP, __VA_ARGS__)

// usage       : TTL_9322_DIP(name, )
#define TTL_9322_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_9322_DIP, __VA_ARGS__)

// usage       : TTL_9334_DIP(name, )
#define TTL_9334_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_9334_DIP, __VA_ARGS__)

// usage       : TTL_8277_DIP(name, )
#define TTL_8277_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_8277_DIP, __VA_ARGS__)

// usage       : TTL_AM2847_DIP(name, )
#define TTL_AM2847_DIP(...)                                                   \
	NET_REGISTER_DEVEXT(TTL_AM2847_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: ../solver/nld_solver.cpp
// ---------------------------------------------------------------------

// usage       : SOLVER(name, FREQ)
#define SOLVER(...)                                                   \
	NET_REGISTER_DEVEXT(SOLVER, __VA_ARGS__)

