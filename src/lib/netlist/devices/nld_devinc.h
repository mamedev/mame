// license:GPL-2.0+
// copyright-holders:Couriersud
#ifndef NLD_DEVINC_H
#define NLD_DEVINC_H

#ifndef __PLIB_PREPROCESSOR__

// ----------------------------------------------------------------------------
//  Netlist Macros
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------
// Source: src/lib/netlist/analog/nlid_twoterm.cpp
// ---------------------------------------------------------------------
// usage       : RES(name, pR)
#define RES(...)                                                       \
	NET_REGISTER_DEVEXT(RES, __VA_ARGS__)

// usage       : POT(name, pR)
#define POT(...)                                                       \
	NET_REGISTER_DEVEXT(POT, __VA_ARGS__)

// usage       : POT2(name, pR)
#define POT2(...)                                                      \
	NET_REGISTER_DEVEXT(POT2, __VA_ARGS__)

// usage       : CAP(name, pC)
#define CAP(...)                                                       \
	NET_REGISTER_DEVEXT(CAP, __VA_ARGS__)

// usage       : IND(name, pL)
#define IND(...)                                                       \
	NET_REGISTER_DEVEXT(IND, __VA_ARGS__)

// usage       : DIODE(name, pMODEL)
#define DIODE(...)                                                     \
	NET_REGISTER_DEVEXT(DIODE, __VA_ARGS__)

// usage       : ZDIODE(name, pMODEL)
#define ZDIODE(...)                                                    \
	NET_REGISTER_DEVEXT(ZDIODE, __VA_ARGS__)

// usage       : VS(name, pV)
#define VS(...)                                                        \
	NET_REGISTER_DEVEXT(VS, __VA_ARGS__)

// usage       : CS(name, pI)
#define CS(...)                                                        \
	NET_REGISTER_DEVEXT(CS, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/analog/nlid_fourterm.cpp
// ---------------------------------------------------------------------
// usage       : VCVS(name, pG)
#define VCVS(...)                                                      \
	NET_REGISTER_DEVEXT(VCVS, __VA_ARGS__)

// usage       : VCCS(name, pG)
#define VCCS(...)                                                      \
	NET_REGISTER_DEVEXT(VCCS, __VA_ARGS__)

// usage       : CCCS(name, pG)
#define CCCS(...)                                                      \
	NET_REGISTER_DEVEXT(CCCS, __VA_ARGS__)

// usage       : CCVS(name, pG)
#define CCVS(...)                                                      \
	NET_REGISTER_DEVEXT(CCVS, __VA_ARGS__)

// usage       : LVCCS(name)
#define LVCCS(...)                                                     \
	NET_REGISTER_DEVEXT(LVCCS, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/analog/nld_opamps.cpp
// ---------------------------------------------------------------------
// usage       : OPAMP(name, pMODEL)
#define OPAMP(...)                                                     \
	NET_REGISTER_DEVEXT(OPAMP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_system.cpp
// ---------------------------------------------------------------------
// usage       : NC_PIN(name)
#define NC_PIN(...)                                                    \
	NET_REGISTER_DEVEXT(NC_PIN, __VA_ARGS__)

// usage       : FRONTIER_DEV(name, pI, pG, pQ)
#define FRONTIER_DEV(...)                                              \
	NET_REGISTER_DEVEXT(FRONTIER_DEV, __VA_ARGS__)

// usage       : AFUNC(name, pN, pFUNC)
#define AFUNC(...)                                                     \
	NET_REGISTER_DEVEXT(AFUNC, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/analog/nld_bjt.cpp
// ---------------------------------------------------------------------
// usage       : QBJT_EB(name, pMODEL)
#define QBJT_EB(...)                                                   \
	NET_REGISTER_DEVEXT(QBJT_EB, __VA_ARGS__)

// usage       : QBJT_SW(name, pMODEL)
#define QBJT_SW(...)                                                   \
	NET_REGISTER_DEVEXT(QBJT_SW, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/analog/nld_mosfet.cpp
// ---------------------------------------------------------------------
// usage       : MOSFET(name, pMODEL)
#define MOSFET(...)                                                    \
	NET_REGISTER_DEVEXT(MOSFET, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_system.cpp
// ---------------------------------------------------------------------
// usage       : TTL_INPUT(name, pIN)
#define TTL_INPUT(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_INPUT, __VA_ARGS__)

// usage       : LOGIC_INPUT(name, pIN, pMODEL)
#define LOGIC_INPUT(...)                                               \
	NET_REGISTER_DEVEXT(LOGIC_INPUT, __VA_ARGS__)

// usage       : LOGIC_INPUT8(name, pIN, pMODEL)
#define LOGIC_INPUT8(...)                                              \
	NET_REGISTER_DEVEXT(LOGIC_INPUT8, __VA_ARGS__)

// usage       : ANALOG_INPUT(name, pIN)
#define ANALOG_INPUT(...)                                              \
	NET_REGISTER_DEVEXT(ANALOG_INPUT, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_log.cpp
// ---------------------------------------------------------------------
// usage       : LOG(name, pI)
#define LOG(...)                                                       \
	NET_REGISTER_DEVEXT(LOG, __VA_ARGS__)

// usage       : LOGD(name, pI, pI2)
#define LOGD(...)                                                      \
	NET_REGISTER_DEVEXT(LOGD, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_system.cpp
// ---------------------------------------------------------------------
// usage       : CLOCK(name, pFREQ)
#define CLOCK(...)                                                     \
	NET_REGISTER_DEVEXT(CLOCK, __VA_ARGS__)

// usage       : VARCLOCK(name, pN, pFUNC)
#define VARCLOCK(...)                                                  \
	NET_REGISTER_DEVEXT(VARCLOCK, __VA_ARGS__)

// usage       : EXTCLOCK(name, pFREQ, pPATTERN)
#define EXTCLOCK(...)                                                  \
	NET_REGISTER_DEVEXT(EXTCLOCK, __VA_ARGS__)

// usage       : MAINCLOCK(name, pFREQ)
#define MAINCLOCK(...)                                                 \
	NET_REGISTER_DEVEXT(MAINCLOCK, __VA_ARGS__)

// usage       : GNDA(name)
#define GNDA(...)                                                      \
	NET_REGISTER_DEVEXT(GNDA, __VA_ARGS__)

// usage       : PARAMETER(name)
#define PARAMETER(...)                                                 \
	NET_REGISTER_DEVEXT(PARAMETER, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/solver/nld_solver.cpp
// ---------------------------------------------------------------------
// usage       : SOLVER(name, pFREQ)
#define SOLVER(...)                                                    \
	NET_REGISTER_DEVEXT(SOLVER, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_system.cpp
// ---------------------------------------------------------------------
// usage       : SYS_DSW(name, pI, p1, p2)
#define SYS_DSW(...)                                                   \
	NET_REGISTER_DEVEXT(SYS_DSW, __VA_ARGS__)

// usage       : SYS_DSW2(name)
#define SYS_DSW2(...)                                                  \
	NET_REGISTER_DEVEXT(SYS_DSW2, __VA_ARGS__)

// usage       : SYS_COMPD(name)
#define SYS_COMPD(...)                                                 \
	NET_REGISTER_DEVEXT(SYS_COMPD, __VA_ARGS__)

// usage       : SYS_NOISE_MT_U(name, pSIGMA)
#define SYS_NOISE_MT_U(...)                                            \
	NET_REGISTER_DEVEXT(SYS_NOISE_MT_U, __VA_ARGS__)

// usage       : SYS_NOISE_MT_N(name, pSIGMA)
#define SYS_NOISE_MT_N(...)                                            \
	NET_REGISTER_DEVEXT(SYS_NOISE_MT_N, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/analog/nld_switches.cpp
// ---------------------------------------------------------------------
// usage       : SWITCH(name)
#define SWITCH(...)                                                    \
	NET_REGISTER_DEVEXT(SWITCH, __VA_ARGS__)

// usage       : SWITCH2(name)
#define SWITCH2(...)                                                   \
	NET_REGISTER_DEVEXT(SWITCH2, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_legacy.cpp
// ---------------------------------------------------------------------
// usage       : NETDEV_RSFF(name)
#define NETDEV_RSFF(...)                                               \
	NET_REGISTER_DEVEXT(NETDEV_RSFF, __VA_ARGS__)

// usage       : NETDEV_DELAY(name)
#define NETDEV_DELAY(...)                                              \
	NET_REGISTER_DEVEXT(NETDEV_DELAY, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_2102A.cpp
// ---------------------------------------------------------------------
// usage       : RAM_2102A(name, pCEQ, pA0, pA1, pA2, pA3, pA4, pA5, pA6, pA7, pA8, pA9, pRWQ, pDI)
// auto connect: VCC, GND
#define RAM_2102A(...)                                                 \
	NET_REGISTER_DEVEXT(RAM_2102A, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_roms.cpp
// ---------------------------------------------------------------------
// usage       : EPROM_2716(name, pCE2Q, pCE1Q, pA0, pA1, pA2, pA3, pA4, pA5, pA6, pA7, pA8, pA9, pA10)
// auto connect: VCC, GND
#define EPROM_2716(...)                                                \
	NET_REGISTER_DEVEXT(EPROM_2716, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7448.cpp
// ---------------------------------------------------------------------
// usage       : TTL_7448(name, pA, pB, pC, pD, pLTQ, pBIQ, pRBIQ)
// auto connect: VCC, GND
#define TTL_7448(...)                                                  \
	NET_REGISTER_DEVEXT(TTL_7448, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7450.cpp
// ---------------------------------------------------------------------
// usage       : TTL_7450_ANDORINVERT(name, pA, pB, pC, pD)
// auto connect: VCC, GND
#define TTL_7450_ANDORINVERT(...)                                      \
	NET_REGISTER_DEVEXT(TTL_7450_ANDORINVERT, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7473.cpp
// ---------------------------------------------------------------------
// usage       : TTL_7473(name, pCLK, pJ, pK, pCLRQ)
// auto connect: VCC, GND
#define TTL_7473(...)                                                  \
	NET_REGISTER_DEVEXT(TTL_7473, __VA_ARGS__)

// usage       : TTL_7473A(name, pCLK, pJ, pK, pCLRQ)
// auto connect: VCC, GND
#define TTL_7473A(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_7473A, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7474.cpp
// ---------------------------------------------------------------------
// usage       : TTL_7474(name, pCLK, pD, pCLRQ, pPREQ)
// auto connect: VCC, GND
#define TTL_7474(...)                                                  \
	NET_REGISTER_DEVEXT(TTL_7474, __VA_ARGS__)

// usage       : TTL_7474_DIP(name)
#define TTL_7474_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7474_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7475.cpp
// ---------------------------------------------------------------------
// usage       : TTL_7475(name)
#define TTL_7475(...)                                                  \
	NET_REGISTER_DEVEXT(TTL_7475, __VA_ARGS__)

// usage       : TTL_7477(name)
#define TTL_7477(...)                                                  \
	NET_REGISTER_DEVEXT(TTL_7477, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7483.cpp
// ---------------------------------------------------------------------
// usage       : TTL_7483(name, pA1, pA2, pA3, pA4, pB1, pB2, pB3, pB4, pC0)
// auto connect: VCC, GND
#define TTL_7483(...)                                                  \
	NET_REGISTER_DEVEXT(TTL_7483, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7485.cpp
// ---------------------------------------------------------------------
// usage       : TTL_7485(name, pA0, pA1, pA2, pA3, pB0, pB1, pB2, pB3, pLTIN, pEQIN, pGTIN)
// auto connect: VCC, GND
#define TTL_7485(...)                                                  \
	NET_REGISTER_DEVEXT(TTL_7485, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7490.cpp
// ---------------------------------------------------------------------
// usage       : TTL_7490(name, pA, pB, pR1, pR2, pR91, pR92)
// auto connect: VCC, GND
#define TTL_7490(...)                                                  \
	NET_REGISTER_DEVEXT(TTL_7490, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7492.cpp
// ---------------------------------------------------------------------
// usage       : TTL_7492(name, pA, pB, pR1, pR2)
// auto connect: VCC, GND
#define TTL_7492(...)                                                  \
	NET_REGISTER_DEVEXT(TTL_7492, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7493.cpp
// ---------------------------------------------------------------------
// usage       : TTL_7493(name, pCLKA, pCLKB, pR1, pR2)
// auto connect: VCC, GND
#define TTL_7493(...)                                                  \
	NET_REGISTER_DEVEXT(TTL_7493, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7497.cpp
// ---------------------------------------------------------------------
// usage       : TTL_7497(name, pCLK, pSTRBQ, pENQ, pUNITYQ, pCLR, pB0, pB1, pB2, pB3, pB4, pB5)
// auto connect: VCC, GND
#define TTL_7497(...)                                                  \
	NET_REGISTER_DEVEXT(TTL_7497, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74107.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74107(name, pCLK, pJ, pK, pCLRQ)
// auto connect: VCC, GND
#define TTL_74107(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_74107, __VA_ARGS__)

// usage       : TTL_74107A(name, pCLK, pJ, pK, pCLRQ)
// auto connect: VCC, GND
#define TTL_74107A(...)                                                \
	NET_REGISTER_DEVEXT(TTL_74107A, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74113.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74113(name, pCLK, pJ, pK, pCLRQ)
// auto connect: VCC, GND
#define TTL_74113(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_74113, __VA_ARGS__)

// usage       : TTL_74113A(name, pCLK, pJ, pK, pCLRQ)
// auto connect: VCC, GND
#define TTL_74113A(...)                                                \
	NET_REGISTER_DEVEXT(TTL_74113A, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74123.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74121(name)
#define TTL_74121(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_74121, __VA_ARGS__)

// usage       : TTL_74123(name)
#define TTL_74123(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_74123, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74125.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74125_GATE(name)
#define TTL_74125_GATE(...)                                            \
	NET_REGISTER_DEVEXT(TTL_74125_GATE, __VA_ARGS__)

// usage       : TTL_74126_GATE(name)
#define TTL_74126_GATE(...)                                            \
	NET_REGISTER_DEVEXT(TTL_74126_GATE, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74153.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74153(name, pC0, pC1, pC2, pC3, pA, pB, pG)
// auto connect: VCC, GND
#define TTL_74153(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_74153, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74161.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74161(name, pCLK, pENP, pENT, pCLRQ, pLOADQ, pA, pB, pC, pD)
// auto connect: VCC, GND
#define TTL_74161(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_74161, __VA_ARGS__)

// usage       : TTL_74161_FIXME(name, pA, pB, pC, pD, pCLRQ, pLOADQ, pCLK, pENP, pENT)
// auto connect: VCC, GND
#define TTL_74161_FIXME(...)                                           \
	NET_REGISTER_DEVEXT(TTL_74161_FIXME, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74163.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74163(name, pCLK, pENP, pENT, pCLRQ, pLOADQ, pA, pB, pC, pD)
// auto connect: VCC, GND
#define TTL_74163(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_74163, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74164.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74164(name, pA, pB, pCLRQ, pCLK)
// auto connect: VCC, GND
#define TTL_74164(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_74164, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74165.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74165(name, pCLK, pCLKINH, pSH_LDQ, pSER, pA, pB, pC, pD, pE, pF, pG, pH)
// auto connect: VCC, GND
#define TTL_74165(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_74165, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74166.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74166(name, pCLK, pCLKINH, pSH_LDQ, pSER, pA, pB, pC, pD, pE, pF, pG, pH, pCLRQ)
// auto connect: VCC, GND
#define TTL_74166(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_74166, __VA_ARGS__)

// usage       : TTL_74166_DIP(name)
#define TTL_74166_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74166_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74174.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74174(name, pCLK, pD1, pD2, pD3, pD4, pD5, pD6, pCLRQ)
// auto connect: VCC, GND
#define TTL_74174(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_74174, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74175.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74175(name, pCLK, pD1, pD2, pD3, pD4, pCLRQ)
// auto connect: VCC, GND
#define TTL_74175(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_74175, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74192.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74192(name, pA, pB, pC, pD, pCLEAR, pLOADQ, pCU, pCD)
// auto connect: VCC, GND
#define TTL_74192(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_74192, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74193.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74193(name, pA, pB, pC, pD, pCLEAR, pLOADQ, pCU, pCD)
// auto connect: VCC, GND
#define TTL_74193(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_74193, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74194.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74194(name, pCLK, pS0, pS1, pSRIN, pA, pB, pC, pD, pSLIN, pCLRQ)
// auto connect: VCC, GND
#define TTL_74194(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_74194, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74365.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74365(name, pG1Q, pG2Q, pA1, pA2, pA3, pA4, pA5, pA6)
// auto connect: VCC, GND
#define TTL_74365(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_74365, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74377.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74377_GATE(name)
#define TTL_74377_GATE(...)                                            \
	NET_REGISTER_DEVEXT(TTL_74377_GATE, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74393.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74393(name, pCP, pMR)
// auto connect: VCC, GND
#define TTL_74393(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_74393, __VA_ARGS__)

// usage       : TTL_74393_DIP(name)
#define TTL_74393_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74393_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74ls629.cpp
// ---------------------------------------------------------------------
// usage       : SN74LS629(name, pCAP)
#define SN74LS629(...)                                                 \
	NET_REGISTER_DEVEXT(SN74LS629, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_82S16.cpp
// ---------------------------------------------------------------------
// usage       : TTL_82S16(name)
#define TTL_82S16(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_82S16, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_82S115.cpp
// ---------------------------------------------------------------------
// usage       : PROM_82S115(name, pCE1Q, pCE2, pA0, pA1, pA2, pA3, pA4, pA5, pA6, pA7, pA8, pSTROBE)
// auto connect: VCC, GND
#define PROM_82S115(...)                                               \
	NET_REGISTER_DEVEXT(PROM_82S115, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_roms.cpp
// ---------------------------------------------------------------------
// usage       : PROM_82S123(name, pCEQ, pA0, pA1, pA2, pA3, pA4)
// auto connect: VCC, GND
#define PROM_82S123(...)                                               \
	NET_REGISTER_DEVEXT(PROM_82S123, __VA_ARGS__)

// usage       : PROM_82S126(name, pCE1Q, pCE2Q, pA0, pA1, pA2, pA3, pA4, pA5, pA6, pA7)
// auto connect: VCC, GND
#define PROM_82S126(...)                                               \
	NET_REGISTER_DEVEXT(PROM_82S126, __VA_ARGS__)

// usage       : PROM_74S287(name, pCE1Q, pCE2Q, pA0, pA1, pA2, pA3, pA4, pA5, pA6, pA7)
// auto connect: VCC, GND
#define PROM_74S287(...)                                               \
	NET_REGISTER_DEVEXT(PROM_74S287, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_9310.cpp
// ---------------------------------------------------------------------
// usage       : TTL_9310(name, pCLK, pENP, pENT, pCLRQ, pLOADQ, pA, pB, pC, pD)
// auto connect: VCC, GND
#define TTL_9310(...)                                                  \
	NET_REGISTER_DEVEXT(TTL_9310, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_dm9314.cpp
// ---------------------------------------------------------------------
// usage       : TTL_9314(name, pEQ, pMRQ, pS0Q, pS1Q, pS2Q, pS3Q, pD0, pD1, pD2, pD3)
// auto connect: VCC, GND
#define TTL_9314(...)                                                  \
	NET_REGISTER_DEVEXT(TTL_9314, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_9316.cpp
// ---------------------------------------------------------------------
// usage       : TTL_9316(name, pCLK, pENP, pENT, pCLRQ, pLOADQ, pA, pB, pC, pD)
// auto connect: VCC, GND
#define TTL_9316(...)                                                  \
	NET_REGISTER_DEVEXT(TTL_9316, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_9322.cpp
// ---------------------------------------------------------------------
// usage       : TTL_9322(name, pSELECT, pA1, pB1, pA2, pB2, pA3, pB3, pA4, pB4, pSTROBE)
// auto connect: VCC, GND
#define TTL_9322(...)                                                  \
	NET_REGISTER_DEVEXT(TTL_9322, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_dm9334.cpp
// ---------------------------------------------------------------------
// usage       : TTL_9334(name, pCQ, pEQ, pD, pA0, pA1, pA2)
// auto connect: VCC, GND
#define TTL_9334(...)                                                  \
	NET_REGISTER_DEVEXT(TTL_9334, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_am2847.cpp
// ---------------------------------------------------------------------
// usage       : TTL_AM2847(name, pCP, pINA, pINB, pINC, pIND, pRCA, pRCB, pRCC, pRCD)
// auto connect: VSS, VDD
#define TTL_AM2847(...)                                                \
	NET_REGISTER_DEVEXT(TTL_AM2847, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_4006.cpp
// ---------------------------------------------------------------------
// usage       : CD4006(name, pCLOCK, pD1, pD2, pD3, pD4, pD1P4, pD1P4S, pD2P4, pD2P5, pD3P4, pD4P4, pD3P5)
// auto connect: VCC, GND
#define CD4006(...)                                                    \
	NET_REGISTER_DEVEXT(CD4006, __VA_ARGS__)

// usage       : CD4006_DIP(name)
#define CD4006_DIP(...)                                                \
	NET_REGISTER_DEVEXT(CD4006_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_4017.cpp
// ---------------------------------------------------------------------
// usage       : CD4017(name)
#define CD4017(...)                                                    \
	NET_REGISTER_DEVEXT(CD4017, __VA_ARGS__)

// usage       : CD4022(name)
#define CD4022(...)                                                    \
	NET_REGISTER_DEVEXT(CD4022, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_4020.cpp
// ---------------------------------------------------------------------
// usage       : CD4020_WI(name, pIP, pRESET, pVDD, pVSS)
#define CD4020_WI(...)                                                 \
	NET_REGISTER_DEVEXT(CD4020_WI, __VA_ARGS__)

// usage       : CD4020(name)
#define CD4020(...)                                                    \
	NET_REGISTER_DEVEXT(CD4020, __VA_ARGS__)

// usage       : CD4024(name)
#define CD4024(...)                                                    \
	NET_REGISTER_DEVEXT(CD4024, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_4053.cpp
// ---------------------------------------------------------------------
// usage       : CD4053_GATE(name)
#define CD4053_GATE(...)                                               \
	NET_REGISTER_DEVEXT(CD4053_GATE, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_4066.cpp
// ---------------------------------------------------------------------
// usage       : CD4066_GATE(name)
#define CD4066_GATE(...)                                               \
	NET_REGISTER_DEVEXT(CD4066_GATE, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_4316.cpp
// ---------------------------------------------------------------------
// usage       : CD4316_GATE(name)
#define CD4316_GATE(...)                                               \
	NET_REGISTER_DEVEXT(CD4316_GATE, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74123.cpp
// ---------------------------------------------------------------------
// usage       : CD4538(name)
#define CD4538(...)                                                    \
	NET_REGISTER_DEVEXT(CD4538, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_schmitt.cpp
// ---------------------------------------------------------------------
// usage       : SCHMITT_TRIGGER(name, pSTMODEL)
#define SCHMITT_TRIGGER(...)                                           \
	NET_REGISTER_DEVEXT(SCHMITT_TRIGGER, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_ne555.cpp
// ---------------------------------------------------------------------
// usage       : NE555(name)
#define NE555(...)                                                     \
	NET_REGISTER_DEVEXT(NE555, __VA_ARGS__)

// usage       : NE555_DIP(name)
#define NE555_DIP(...)                                                 \
	NET_REGISTER_DEVEXT(NE555_DIP, __VA_ARGS__)

// usage       : MC1455P(name)
#define MC1455P(...)                                                   \
	NET_REGISTER_DEVEXT(MC1455P, __VA_ARGS__)

// usage       : MC1455P_DIP(name)
#define MC1455P_DIP(...)                                               \
	NET_REGISTER_DEVEXT(MC1455P_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_tms4800.cpp
// ---------------------------------------------------------------------
// usage       : ROM_TMS4800(name, pAR, pOE1, pOE2, pA0, pA1, pA2, pA3, pA4, pA5, pA6, pA7, pA8, pA9, pA10)
// auto connect: VCC, GND
#define ROM_TMS4800(...)                                               \
	NET_REGISTER_DEVEXT(ROM_TMS4800, __VA_ARGS__)

// usage       : ROM_TMS4800_DIP(name)
#define ROM_TMS4800_DIP(...)                                           \
	NET_REGISTER_DEVEXT(ROM_TMS4800_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_r2r_dac.cpp
// ---------------------------------------------------------------------
// usage       : R2R_DAC(name, pVIN, pR, pN)
#define R2R_DAC(...)                                                   \
	NET_REGISTER_DEVEXT(R2R_DAC, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_tristate.cpp
// ---------------------------------------------------------------------
// usage       : TTL_TRISTATE(name, pCEQ1, pD1, pCEQ2, pD2)
#define TTL_TRISTATE(...)                                              \
	NET_REGISTER_DEVEXT(TTL_TRISTATE, __VA_ARGS__)

// usage       : TTL_TRISTATE3(name)
#define TTL_TRISTATE3(...)                                             \
	NET_REGISTER_DEVEXT(TTL_TRISTATE3, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74174.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74174_DIP(name)
#define TTL_74174_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74174_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74175.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74175_DIP(name)
#define TTL_74175_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74175_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74192.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74192_DIP(name)
#define TTL_74192_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74192_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74193.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74193_DIP(name)
#define TTL_74193_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74193_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74194.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74194_DIP(name)
#define TTL_74194_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74194_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74365.cpp
// ---------------------------------------------------------------------
// usage       : TTL_74365_DIP(name)
#define TTL_74365_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74365_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74123.cpp
// ---------------------------------------------------------------------
// usage       : TTL_9602(name)
#define TTL_9602(...)                                                  \
	NET_REGISTER_DEVEXT(TTL_9602, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_dm9314.cpp
// ---------------------------------------------------------------------
// usage       : TTL_9314_DIP(name)
#define TTL_9314_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_9314_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_dm9334.cpp
// ---------------------------------------------------------------------
// usage       : TTL_9334_DIP(name)
#define TTL_9334_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_9334_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_am2847.cpp
// ---------------------------------------------------------------------
// usage       : TTL_AM2847_DIP(name)
#define TTL_AM2847_DIP(...)                                            \
	NET_REGISTER_DEVEXT(TTL_AM2847_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_mm5837.cpp
// ---------------------------------------------------------------------
// usage       : MM5837_DIP(name)
#define MM5837_DIP(...)                                                \
	NET_REGISTER_DEVEXT(MM5837_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: TTL74XX_lib
// ---------------------------------------------------------------------
// usage       : TTL_7400_GATE(name)
#define TTL_7400_GATE(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7400_GATE, __VA_ARGS__)

// usage       : TTL_7400_NAND(name, pA, pB)
// auto connect: VCC, GND
#define TTL_7400_NAND(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7400_NAND, __VA_ARGS__)

// usage       : TTL_7402_GATE(name)
#define TTL_7402_GATE(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7402_GATE, __VA_ARGS__)

// usage       : TTL_7402_NOR(name, pA, pB)
// auto connect: VCC, GND
#define TTL_7402_NOR(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7402_NOR, __VA_ARGS__)

// usage       : TTL_7404_GATE(name)
#define TTL_7404_GATE(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7404_GATE, __VA_ARGS__)

// usage       : TTL_7404_INVERT(name, pA)
// auto connect: VCC, GND
#define TTL_7404_INVERT(...)                                           \
	NET_REGISTER_DEVEXT(TTL_7404_INVERT, __VA_ARGS__)

// usage       : TTL_7406_GATE(name)
#define TTL_7406_GATE(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7406_GATE, __VA_ARGS__)

// usage       : TTL_7407_GATE(name)
#define TTL_7407_GATE(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7407_GATE, __VA_ARGS__)

// usage       : TTL_7408_GATE(name)
#define TTL_7408_GATE(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7408_GATE, __VA_ARGS__)

// usage       : TTL_7408_AND(name, pA, pB)
// auto connect: VCC, GND
#define TTL_7408_AND(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7408_AND, __VA_ARGS__)

// usage       : TTL_7410_NAND(name, pA, pB, pC)
// auto connect: VCC, GND
#define TTL_7410_NAND(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7410_NAND, __VA_ARGS__)

// usage       : TTL_7410_GATE(name)
#define TTL_7410_GATE(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7410_GATE, __VA_ARGS__)

// usage       : TTL_7411_AND(name, pA, pB, pC)
// auto connect: VCC, GND
#define TTL_7411_AND(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7411_AND, __VA_ARGS__)

// usage       : TTL_7411_GATE(name)
#define TTL_7411_GATE(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7411_GATE, __VA_ARGS__)

// usage       : TTL_7416_GATE(name)
#define TTL_7416_GATE(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7416_GATE, __VA_ARGS__)

// usage       : TTL_7420_GATE(name)
#define TTL_7420_GATE(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7420_GATE, __VA_ARGS__)

// usage       : TTL_7420_NAND(name, pA, pB, pC, pD)
// auto connect: VCC, GND
#define TTL_7420_NAND(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7420_NAND, __VA_ARGS__)

// usage       : TTL_7421_GATE(name)
#define TTL_7421_GATE(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7421_GATE, __VA_ARGS__)

// usage       : TTL_7421_AND(name, pA, pB, pC, pD)
// auto connect: VCC, GND
#define TTL_7421_AND(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7421_AND, __VA_ARGS__)

// usage       : TTL_7425_GATE(name)
#define TTL_7425_GATE(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7425_GATE, __VA_ARGS__)

// usage       : TTL_7425_NOR(name, pA, pB, pC, pD)
// auto connect: VCC, GND
#define TTL_7425_NOR(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7425_NOR, __VA_ARGS__)

// usage       : TTL_7427_GATE(name)
#define TTL_7427_GATE(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7427_GATE, __VA_ARGS__)

// usage       : TTL_7427_NOR(name, pA, pB, pC)
// auto connect: VCC, GND
#define TTL_7427_NOR(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7427_NOR, __VA_ARGS__)

// usage       : TTL_7430_GATE(name)
#define TTL_7430_GATE(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7430_GATE, __VA_ARGS__)

// usage       : TTL_7430_NAND(name, pA, pB, pC, pD, pE, pF, pG, pH)
// auto connect: VCC, GND
#define TTL_7430_NAND(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7430_NAND, __VA_ARGS__)

// usage       : TTL_7432_GATE(name)
#define TTL_7432_GATE(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7432_GATE, __VA_ARGS__)

// usage       : TTL_7432_OR(name, pA, pB)
// auto connect: VCC, GND
#define TTL_7432_OR(...)                                               \
	NET_REGISTER_DEVEXT(TTL_7432_OR, __VA_ARGS__)

// usage       : TTL_7437_GATE(name)
#define TTL_7437_GATE(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7437_GATE, __VA_ARGS__)

// usage       : TTL_7437_NAND(name, pA, pB)
#define TTL_7437_NAND(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7437_NAND, __VA_ARGS__)

// usage       : TTL_7448_DIP(name)
#define TTL_7448_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7448_DIP, __VA_ARGS__)

// usage       : TTL_7450_DIP(name)
#define TTL_7450_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7450_DIP, __VA_ARGS__)

// usage       : TTL_7473_DIP(name)
#define TTL_7473_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7473_DIP, __VA_ARGS__)

// usage       : TTL_7473A_DIP(name)
#define TTL_7473A_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7473A_DIP, __VA_ARGS__)

// usage       : TTL_7475_DIP(name)
#define TTL_7475_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7475_DIP, __VA_ARGS__)

// usage       : TTL_7477_DIP(name)
#define TTL_7477_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7477_DIP, __VA_ARGS__)

// usage       : TTL_7483_DIP(name)
#define TTL_7483_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7483_DIP, __VA_ARGS__)

// usage       : TTL_7485_DIP(name)
#define TTL_7485_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7485_DIP, __VA_ARGS__)

// usage       : TTL_7486_GATE(name)
#define TTL_7486_GATE(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7486_GATE, __VA_ARGS__)

// usage       : TTL_7486_XOR(name, pA, pB)
// auto connect: VCC, GND
#define TTL_7486_XOR(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7486_XOR, __VA_ARGS__)

// usage       : TTL_7490_DIP(name)
#define TTL_7490_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7490_DIP, __VA_ARGS__)

// usage       : TTL_7492_DIP(name)
#define TTL_7492_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7492_DIP, __VA_ARGS__)

// usage       : TTL_7493_DIP(name)
#define TTL_7493_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7493_DIP, __VA_ARGS__)

// usage       : TTL_7497_DIP(name)
#define TTL_7497_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7497_DIP, __VA_ARGS__)

// usage       : TTL_74107_DIP(name)
#define TTL_74107_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74107_DIP, __VA_ARGS__)

// usage       : TTL_74107_DIP(name)
#define TTL_74107A_DIP(...)                                            \
	NET_REGISTER_DEVEXT(TTL_74107A_DIP, __VA_ARGS__)

// usage       : TTL_74113_DIP(name)
#define TTL_74113_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74113_DIP, __VA_ARGS__)

// usage       : TTL_74113A_DIP(name)
#define TTL_74113A_DIP(...)                                            \
	NET_REGISTER_DEVEXT(TTL_74113A_DIP, __VA_ARGS__)

// usage       : TTL_74155A_GATE(name)
#define TTL_74155A_GATE(...)                                           \
	NET_REGISTER_DEVEXT(TTL_74155A_GATE, __VA_ARGS__)

// usage       : TTL_74155B_GATE(name)
#define TTL_74155B_GATE(...)                                           \
	NET_REGISTER_DEVEXT(TTL_74155B_GATE, __VA_ARGS__)

// usage       : TTL_74156A_GATE(name)
#define TTL_74156A_GATE(...)                                           \
	NET_REGISTER_DEVEXT(TTL_74156A_GATE, __VA_ARGS__)

// usage       : TTL_74156B_GATE(name)
#define TTL_74156B_GATE(...)                                           \
	NET_REGISTER_DEVEXT(TTL_74156B_GATE, __VA_ARGS__)

// usage       : TTL_74157_GATE(name)
#define TTL_74157_GATE(...)                                            \
	NET_REGISTER_DEVEXT(TTL_74157_GATE, __VA_ARGS__)

// usage       : TTL_74161_DIP(name)
#define TTL_74161_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74161_DIP, __VA_ARGS__)

// usage       : TTL_74163_DIP(name)
#define TTL_74163_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74163_DIP, __VA_ARGS__)

// usage       : TTL_74164_DIP(name)
#define TTL_74164_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74164_DIP, __VA_ARGS__)

// usage       : TTL_74165_DIP(name)
#define TTL_74165_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74165_DIP, __VA_ARGS__)

// usage       : TTL_74260_GATE(name)
#define TTL_74260_GATE(...)                                            \
	NET_REGISTER_DEVEXT(TTL_74260_GATE, __VA_ARGS__)

// usage       : TTL_74260_NOR(name, pA, pB, pC, pD, pE)
// auto connect: VCC, GND
#define TTL_74260_NOR(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74260_NOR, __VA_ARGS__)

// usage       : TTL_74279A(name)
#define TTL_74279A(...)                                                \
	NET_REGISTER_DEVEXT(TTL_74279A, __VA_ARGS__)

// usage       : TTL_74279B(name)
#define TTL_74279B(...)                                                \
	NET_REGISTER_DEVEXT(TTL_74279B, __VA_ARGS__)

// usage       : DM9312(name, pA, pB, pC, pG, pD0, pD1, pD2, pD3, pD4, pD5, pD6, pD7)
// auto connect: VCC, GND
#define DM9312(...)                                                    \
	NET_REGISTER_DEVEXT(DM9312, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: parser: TTL74XX_lib
// ---------------------------------------------------------------------
// usage       : TTL_7400_DIP(name)
#define TTL_7400_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7400_DIP, __VA_ARGS__)

// usage       : TTL_7402_DIP(name)
#define TTL_7402_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7402_DIP, __VA_ARGS__)

// usage       : TTL_7404_DIP(name)
#define TTL_7404_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7404_DIP, __VA_ARGS__)

// usage       : TTL_7406_DIP(name)
#define TTL_7406_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7406_DIP, __VA_ARGS__)

// usage       : TTL_7407_DIP(name)
#define TTL_7407_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7407_DIP, __VA_ARGS__)

// usage       : TTL_7408_DIP(name)
#define TTL_7408_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7408_DIP, __VA_ARGS__)

// usage       : TTL_7410_DIP(name)
#define TTL_7410_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7410_DIP, __VA_ARGS__)

// usage       : TTL_7411_DIP(name)
#define TTL_7411_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7411_DIP, __VA_ARGS__)

// usage       : TTL_7414_GATE(name)
#define TTL_7414_GATE(...)                                             \
	NET_REGISTER_DEVEXT(TTL_7414_GATE, __VA_ARGS__)

// usage       : TTL_74LS14_GATE(name)
#define TTL_74LS14_GATE(...)                                           \
	NET_REGISTER_DEVEXT(TTL_74LS14_GATE, __VA_ARGS__)

// usage       : TTL_7414_DIP(name)
#define TTL_7414_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7414_DIP, __VA_ARGS__)

// usage       : TTL_74LS14_DIP(name)
#define TTL_74LS14_DIP(...)                                            \
	NET_REGISTER_DEVEXT(TTL_74LS14_DIP, __VA_ARGS__)

// usage       : TTL_7416_DIP(name)
#define TTL_7416_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7416_DIP, __VA_ARGS__)

// usage       : TTL_7420_DIP(name)
#define TTL_7420_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7420_DIP, __VA_ARGS__)

// usage       : TTL_7421_DIP(name)
#define TTL_7421_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7421_DIP, __VA_ARGS__)

// usage       : TTL_7425_DIP(name)
#define TTL_7425_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7425_DIP, __VA_ARGS__)

// usage       : TTL_7427_DIP(name)
#define TTL_7427_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7427_DIP, __VA_ARGS__)

// usage       : TTL_7430_DIP(name)
#define TTL_7430_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7430_DIP, __VA_ARGS__)

// usage       : TTL_7432_DIP(name)
#define TTL_7432_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7432_DIP, __VA_ARGS__)

// usage       : TTL_7437_DIP(name)
#define TTL_7437_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7437_DIP, __VA_ARGS__)

// usage       : TTL_7442(name, cA, cB, cC, cD)
#define TTL_7442(...)                                                  \
	NET_REGISTER_DEVEXT(TTL_7442, __VA_ARGS__)

// usage       : TTL_7442_DIP(name)
#define TTL_7442_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7442_DIP, __VA_ARGS__)

// usage       : TTL_7486_DIP(name)
#define TTL_7486_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_7486_DIP, __VA_ARGS__)

// usage       : TTL_74121_DIP(name)
#define TTL_74121_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74121_DIP, __VA_ARGS__)

// usage       : TTL_74123_DIP(name)
#define TTL_74123_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74123_DIP, __VA_ARGS__)

// usage       : TTL_9602_DIP(name)
#define TTL_9602_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_9602_DIP, __VA_ARGS__)

// usage       : TTL_74125_DIP(name)
#define TTL_74125_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74125_DIP, __VA_ARGS__)

// usage       : TTL_74126_DIP(name)
#define TTL_74126_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74126_DIP, __VA_ARGS__)

// usage       : TTL_74153_DIP(name)
#define TTL_74153_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74153_DIP, __VA_ARGS__)

// usage       : TTL_74155_DIP(name)
#define TTL_74155_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74155_DIP, __VA_ARGS__)

// usage       : TTL_74156_DIP(name)
#define TTL_74156_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74156_DIP, __VA_ARGS__)

// usage       : TTL_74157_DIP(name)
#define TTL_74157_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74157_DIP, __VA_ARGS__)

// usage       : TTL_74260_DIP(name)
#define TTL_74260_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74260_DIP, __VA_ARGS__)

// usage       : TTL_74279_DIP(name)
#define TTL_74279_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74279_DIP, __VA_ARGS__)

// usage       : TTL_74377_DIP(name)
#define TTL_74377_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74377_DIP, __VA_ARGS__)

// usage       : TTL_74378_DIP(name)
#define TTL_74378_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74378_DIP, __VA_ARGS__)

// usage       : TTL_74379_DIP(name)
#define TTL_74379_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_74379_DIP, __VA_ARGS__)

// usage       : SN74LS629_DIP(name, p1_CAP1, p2_CAP2)
#define SN74LS629_DIP(...)                                             \
	NET_REGISTER_DEVEXT(SN74LS629_DIP, __VA_ARGS__)

// usage       : DM9312_DIP(name)
#define DM9312_DIP(...)                                                \
	NET_REGISTER_DEVEXT(DM9312_DIP, __VA_ARGS__)

// usage       : TTL_9310_DIP(name)
#define TTL_9310_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_9310_DIP, __VA_ARGS__)

// usage       : TTL_9316_DIP(name)
#define TTL_9316_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_9316_DIP, __VA_ARGS__)

// usage       : TTL_9322_DIP(name)
#define TTL_9322_DIP(...)                                              \
	NET_REGISTER_DEVEXT(TTL_9322_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: CD4XXX_lib
// ---------------------------------------------------------------------
// usage       : CD4001_GATE(name)
#define CD4001_GATE(...)                                               \
	NET_REGISTER_DEVEXT(CD4001_GATE, __VA_ARGS__)

// usage       : CD4011_GATE(name)
#define CD4011_GATE(...)                                               \
	NET_REGISTER_DEVEXT(CD4011_GATE, __VA_ARGS__)

// usage       : CD4069_GATE(name)
#define CD4069_GATE(...)                                               \
	NET_REGISTER_DEVEXT(CD4069_GATE, __VA_ARGS__)

// usage       : CD4070_GATE(name)
#define CD4070_GATE(...)                                               \
	NET_REGISTER_DEVEXT(CD4070_GATE, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: parser: CD4XXX_lib
// ---------------------------------------------------------------------
// usage       : CD4001_DIP(name)
#define CD4001_DIP(...)                                                \
	NET_REGISTER_DEVEXT(CD4001_DIP, __VA_ARGS__)

// usage       : CD4011_DIP(name)
#define CD4011_DIP(...)                                                \
	NET_REGISTER_DEVEXT(CD4011_DIP, __VA_ARGS__)

// usage       : CD4069_DIP(name)
#define CD4069_DIP(...)                                                \
	NET_REGISTER_DEVEXT(CD4069_DIP, __VA_ARGS__)

// usage       : CD4070_DIP(name)
#define CD4070_DIP(...)                                                \
	NET_REGISTER_DEVEXT(CD4070_DIP, __VA_ARGS__)

// usage       : CD4013_DIP(name)
#define CD4013_DIP(...)                                                \
	NET_REGISTER_DEVEXT(CD4013_DIP, __VA_ARGS__)

// usage       : CD4017_DIP(name)
#define CD4017_DIP(...)                                                \
	NET_REGISTER_DEVEXT(CD4017_DIP, __VA_ARGS__)

// usage       : CD4022_DIP(name)
#define CD4022_DIP(...)                                                \
	NET_REGISTER_DEVEXT(CD4022_DIP, __VA_ARGS__)

// usage       : CD4020_DIP(name)
#define CD4020_DIP(...)                                                \
	NET_REGISTER_DEVEXT(CD4020_DIP, __VA_ARGS__)

// usage       : CD4024_DIP(name)
#define CD4024_DIP(...)                                                \
	NET_REGISTER_DEVEXT(CD4024_DIP, __VA_ARGS__)

// usage       : CD4053_DIP(name)
#define CD4053_DIP(...)                                                \
	NET_REGISTER_DEVEXT(CD4053_DIP, __VA_ARGS__)

// usage       : CD4066_DIP(name)
#define CD4066_DIP(...)                                                \
	NET_REGISTER_DEVEXT(CD4066_DIP, __VA_ARGS__)

// usage       : CD4016_DIP(name)
#define CD4016_DIP(...)                                                \
	NET_REGISTER_DEVEXT(CD4016_DIP, __VA_ARGS__)

// usage       : CD4316_DIP(name)
#define CD4316_DIP(...)                                                \
	NET_REGISTER_DEVEXT(CD4316_DIP, __VA_ARGS__)

// usage       : CD4538_DIP(name)
#define CD4538_DIP(...)                                                \
	NET_REGISTER_DEVEXT(CD4538_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: parser: OPAMP_lib
// ---------------------------------------------------------------------
// usage       : opamp_layout_4_4_11(name)
#define opamp_layout_4_4_11(...)                                       \
	NET_REGISTER_DEVEXT(opamp_layout_4_4_11, __VA_ARGS__)

// usage       : opamp_layout_2_8_4(name)
#define opamp_layout_2_8_4(...)                                        \
	NET_REGISTER_DEVEXT(opamp_layout_2_8_4, __VA_ARGS__)

// usage       : opamp_layout_2_13_9_4(name)
#define opamp_layout_2_13_9_4(...)                                     \
	NET_REGISTER_DEVEXT(opamp_layout_2_13_9_4, __VA_ARGS__)

// usage       : opamp_layout_1_7_4(name)
#define opamp_layout_1_7_4(...)                                        \
	NET_REGISTER_DEVEXT(opamp_layout_1_7_4, __VA_ARGS__)

// usage       : opamp_layout_1_8_5(name)
#define opamp_layout_1_8_5(...)                                        \
	NET_REGISTER_DEVEXT(opamp_layout_1_8_5, __VA_ARGS__)

// usage       : opamp_layout_1_11_6(name)
#define opamp_layout_1_11_6(...)                                       \
	NET_REGISTER_DEVEXT(opamp_layout_1_11_6, __VA_ARGS__)

// usage       : MB3614_DIP(name)
#define MB3614_DIP(...)                                                \
	NET_REGISTER_DEVEXT(MB3614_DIP, __VA_ARGS__)

// usage       : TL081_DIP(name)
#define TL081_DIP(...)                                                 \
	NET_REGISTER_DEVEXT(TL081_DIP, __VA_ARGS__)

// usage       : TL084_DIP(name)
#define TL084_DIP(...)                                                 \
	NET_REGISTER_DEVEXT(TL084_DIP, __VA_ARGS__)

// usage       : LM324_DIP(name)
#define LM324_DIP(...)                                                 \
	NET_REGISTER_DEVEXT(LM324_DIP, __VA_ARGS__)

// usage       : LM358_DIP(name)
#define LM358_DIP(...)                                                 \
	NET_REGISTER_DEVEXT(LM358_DIP, __VA_ARGS__)

// usage       : LM2902_DIP(name)
#define LM2902_DIP(...)                                                \
	NET_REGISTER_DEVEXT(LM2902_DIP, __VA_ARGS__)

// usage       : UA741_DIP8(name)
#define UA741_DIP8(...)                                                \
	NET_REGISTER_DEVEXT(UA741_DIP8, __VA_ARGS__)

// usage       : UA741_DIP10(name)
#define UA741_DIP10(...)                                               \
	NET_REGISTER_DEVEXT(UA741_DIP10, __VA_ARGS__)

// usage       : UA741_DIP14(name)
#define UA741_DIP14(...)                                               \
	NET_REGISTER_DEVEXT(UA741_DIP14, __VA_ARGS__)

// usage       : LM747_DIP(name)
#define LM747_DIP(...)                                                 \
	NET_REGISTER_DEVEXT(LM747_DIP, __VA_ARGS__)

// usage       : LM747A_DIP(name)
#define LM747A_DIP(...)                                                \
	NET_REGISTER_DEVEXT(LM747A_DIP, __VA_ARGS__)

// usage       : LM3900(name)
#define LM3900(...)                                                    \
	NET_REGISTER_DEVEXT(LM3900, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: otheric_lib
// ---------------------------------------------------------------------
// usage       : MC14584B_GATE(name)
#define MC14584B_GATE(...)                                             \
	NET_REGISTER_DEVEXT(MC14584B_GATE, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: parser: otheric_lib
// ---------------------------------------------------------------------
// usage       : MC14584B_DIP(name)
#define MC14584B_DIP(...)                                              \
	NET_REGISTER_DEVEXT(MC14584B_DIP, __VA_ARGS__)

// usage       : NE566_DIP(name)
#define NE566_DIP(...)                                                 \
	NET_REGISTER_DEVEXT(NE566_DIP, __VA_ARGS__)

// ---------------------------------------------------------------------
// Source: parser: ROMS_lib
// ---------------------------------------------------------------------
// usage       : PROM_82S123_DIP(name)
#define PROM_82S123_DIP(...)                                           \
	NET_REGISTER_DEVEXT(PROM_82S123_DIP, __VA_ARGS__)

// usage       : PROM_82S126_DIP(name)
#define PROM_82S126_DIP(...)                                           \
	NET_REGISTER_DEVEXT(PROM_82S126_DIP, __VA_ARGS__)

// usage       : PROM_74S287_DIP(name)
#define PROM_74S287_DIP(...)                                           \
	NET_REGISTER_DEVEXT(PROM_74S287_DIP, __VA_ARGS__)

// usage       : EPROM_2716_DIP(name)
#define EPROM_2716_DIP(...)                                            \
	NET_REGISTER_DEVEXT(EPROM_2716_DIP, __VA_ARGS__)

// usage       : TTL_82S16_DIP(name)
#define TTL_82S16_DIP(...)                                             \
	NET_REGISTER_DEVEXT(TTL_82S16_DIP, __VA_ARGS__)

// usage       : PROM_82S115_DIP(name)
#define PROM_82S115_DIP(...)                                           \
	NET_REGISTER_DEVEXT(PROM_82S115_DIP, __VA_ARGS__)

// usage       : RAM_2102A_DIP(name)
#define RAM_2102A_DIP(...)                                             \
	NET_REGISTER_DEVEXT(RAM_2102A_DIP, __VA_ARGS__)


#endif // __PLIB_PREPROCESSOR__
#endif
