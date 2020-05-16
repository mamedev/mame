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
#define RES(name, pR)                                                  \
	NET_REGISTER_DEVEXT(RES, name, pR)

#define POT(name, pR)                                                  \
	NET_REGISTER_DEVEXT(POT, name, pR)

#define POT2(name, pR)                                                 \
	NET_REGISTER_DEVEXT(POT2, name, pR)

#define CAP(name, pC)                                                  \
	NET_REGISTER_DEVEXT(CAP, name, pC)

#define IND(name, pL)                                                  \
	NET_REGISTER_DEVEXT(IND, name, pL)

#define DIODE(name, pMODEL)                                            \
	NET_REGISTER_DEVEXT(DIODE, name, pMODEL)

#define ZDIODE(name, pMODEL)                                           \
	NET_REGISTER_DEVEXT(ZDIODE, name, pMODEL)

#define VS(name, pV)                                                   \
	NET_REGISTER_DEVEXT(VS, name, pV)

#define CS(name, pI)                                                   \
	NET_REGISTER_DEVEXT(CS, name, pI)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/analog/nlid_fourterm.cpp
// ---------------------------------------------------------------------
#define VCVS(name, pG)                                                 \
	NET_REGISTER_DEVEXT(VCVS, name, pG)

#define VCCS(name, pG)                                                 \
	NET_REGISTER_DEVEXT(VCCS, name, pG)

#define CCCS(name, pG)                                                 \
	NET_REGISTER_DEVEXT(CCCS, name, pG)

#define CCVS(name, pG)                                                 \
	NET_REGISTER_DEVEXT(CCVS, name, pG)

#define LVCCS(name)                                                    \
	NET_REGISTER_DEVEXT(LVCCS, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/analog/nld_opamps.cpp
// ---------------------------------------------------------------------
#define OPAMP(name, pMODEL)                                            \
	NET_REGISTER_DEVEXT(OPAMP, name, pMODEL)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_system.cpp
// ---------------------------------------------------------------------
#define NC_PIN(name)                                                   \
	NET_REGISTER_DEVEXT(NC_PIN, name)

#define FRONTIER_DEV(name, pI, pG, pQ)                                 \
	NET_REGISTER_DEVEXT(FRONTIER_DEV, name, pI, pG, pQ)

#define AFUNC(name, pN, pFUNC)                                         \
	NET_REGISTER_DEVEXT(AFUNC, name, pN, pFUNC)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/analog/nld_bjt.cpp
// ---------------------------------------------------------------------
#define QBJT_EB(name, pMODEL)                                          \
	NET_REGISTER_DEVEXT(QBJT_EB, name, pMODEL)

#define QBJT_SW(name, pMODEL)                                          \
	NET_REGISTER_DEVEXT(QBJT_SW, name, pMODEL)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/analog/nld_mosfet.cpp
// ---------------------------------------------------------------------
#define MOSFET(name, pMODEL)                                           \
	NET_REGISTER_DEVEXT(MOSFET, name, pMODEL)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_system.cpp
// ---------------------------------------------------------------------
#define TTL_INPUT(name, pIN)                                           \
	NET_REGISTER_DEVEXT(TTL_INPUT, name, pIN)

#define LOGIC_INPUT(name, pIN, pMODEL)                                 \
	NET_REGISTER_DEVEXT(LOGIC_INPUT, name, pIN, pMODEL)

#define LOGIC_INPUT8(name, pIN, pMODEL)                                \
	NET_REGISTER_DEVEXT(LOGIC_INPUT8, name, pIN, pMODEL)

#define ANALOG_INPUT(name, pIN)                                        \
	NET_REGISTER_DEVEXT(ANALOG_INPUT, name, pIN)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_log.cpp
// ---------------------------------------------------------------------
#define LOG(name, pI)                                                  \
	NET_REGISTER_DEVEXT(LOG, name, pI)

#define LOGD(name, pI, pI2)                                            \
	NET_REGISTER_DEVEXT(LOGD, name, pI, pI2)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_system.cpp
// ---------------------------------------------------------------------
#define CLOCK(name, pFREQ)                                             \
	NET_REGISTER_DEVEXT(CLOCK, name, pFREQ)

#define VARCLOCK(name, pFUNC)                                          \
	NET_REGISTER_DEVEXT(VARCLOCK, name, pFUNC)

#define EXTCLOCK(name, pFREQ, pPATTERN)                                \
	NET_REGISTER_DEVEXT(EXTCLOCK, name, pFREQ, pPATTERN)

#define MAINCLOCK(name, pFREQ)                                         \
	NET_REGISTER_DEVEXT(MAINCLOCK, name, pFREQ)

#define GNDA(name)                                                     \
	NET_REGISTER_DEVEXT(GNDA, name)

#define PARAMETER(name)                                                \
	NET_REGISTER_DEVEXT(PARAMETER, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/solver/nld_solver.cpp
// ---------------------------------------------------------------------
#define SOLVER(name, pFREQ)                                            \
	NET_REGISTER_DEVEXT(SOLVER, name, pFREQ)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_system.cpp
// ---------------------------------------------------------------------
#define SYS_DSW(name, pI, p1, p2)                                      \
	NET_REGISTER_DEVEXT(SYS_DSW, name, pI, p1, p2)

#define SYS_DSW2(name)                                                 \
	NET_REGISTER_DEVEXT(SYS_DSW2, name)

#define SYS_COMPD(name)                                                \
	NET_REGISTER_DEVEXT(SYS_COMPD, name)

#define SYS_NOISE_MT_U(name, pSIGMA)                                   \
	NET_REGISTER_DEVEXT(SYS_NOISE_MT_U, name, pSIGMA)

#define SYS_NOISE_MT_N(name, pSIGMA)                                   \
	NET_REGISTER_DEVEXT(SYS_NOISE_MT_N, name, pSIGMA)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/analog/nld_switches.cpp
// ---------------------------------------------------------------------
#define SWITCH(name)                                                   \
	NET_REGISTER_DEVEXT(SWITCH, name)

#define SWITCH2(name)                                                  \
	NET_REGISTER_DEVEXT(SWITCH2, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_legacy.cpp
// ---------------------------------------------------------------------
#define NETDEV_RSFF(name)                                              \
	NET_REGISTER_DEVEXT(NETDEV_RSFF, name)

#define NETDEV_DELAY(name)                                             \
	NET_REGISTER_DEVEXT(NETDEV_DELAY, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_2102A.cpp
// ---------------------------------------------------------------------
#define RAM_2102A(name, pCEQ, pA0, pA1, pA2, pA3, pA4, pA5, pA6, pA7, pA8, pA9, pRWQ, pDI)\
	NET_REGISTER_DEVEXT(RAM_2102A, name, pCEQ, pA0, pA1, pA2, pA3, pA4, pA5, pA6, pA7, pA8, pA9, pRWQ, pDI)

#define RAM_2102A_DIP(name)                                            \
	NET_REGISTER_DEVEXT(RAM_2102A_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_2716.cpp
// ---------------------------------------------------------------------
#define EPROM_2716(name, pGQ, pEPQ, pA0, pA1, pA2, pA3, pA4, pA5, pA6, pA7, pA8, pA9, pA10)\
	NET_REGISTER_DEVEXT(EPROM_2716, name, pGQ, pEPQ, pA0, pA1, pA2, pA3, pA4, pA5, pA6, pA7, pA8, pA9, pA10)

#define EPROM_2716_DIP(name)                                           \
	NET_REGISTER_DEVEXT(EPROM_2716_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7448.cpp
// ---------------------------------------------------------------------
#define TTL_7448(name, pA, pB, pC, pD, pLTQ, pBIQ, pRBIQ)              \
	NET_REGISTER_DEVEXT(TTL_7448, name, pA, pB, pC, pD, pLTQ, pBIQ, pRBIQ)

#define TTL_7448_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7448_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7442.cpp
// ---------------------------------------------------------------------
#define TTL_7442(name, pA, pB, pC, pD)                                 \
	NET_REGISTER_DEVEXT(TTL_7442, name, pA, pB, pC, pD)

#define TTL_7442_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7442_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7450.cpp
// ---------------------------------------------------------------------
#define TTL_7450_ANDORINVERT(name, pA, pB, pC, pD)                     \
	NET_REGISTER_DEVEXT(TTL_7450_ANDORINVERT, name, pA, pB, pC, pD)

#define TTL_7450_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7450_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7473.cpp
// ---------------------------------------------------------------------
#define TTL_7473(name, pCLK, pJ, pK, pCLRQ)                            \
	NET_REGISTER_DEVEXT(TTL_7473, name, pCLK, pJ, pK, pCLRQ)

#define TTL_7473_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7473_DIP, name)

#define TTL_7473A(name, pCLK, pJ, pK, pCLRQ)                           \
	NET_REGISTER_DEVEXT(TTL_7473A, name, pCLK, pJ, pK, pCLRQ)

#define TTL_7473A_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_7473A_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7474.cpp
// ---------------------------------------------------------------------
#define TTL_7474(name, pCLK, pD, pCLRQ, pPREQ)                         \
	NET_REGISTER_DEVEXT(TTL_7474, name, pCLK, pD, pCLRQ, pPREQ)

#define TTL_7474_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7474_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7475.cpp
// ---------------------------------------------------------------------
#define TTL_7475(name)                                                 \
	NET_REGISTER_DEVEXT(TTL_7475, name)

#define TTL_7475_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7475_DIP, name)

#define TTL_7477(name)                                                 \
	NET_REGISTER_DEVEXT(TTL_7477, name)

#define TTL_7477_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7477_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7483.cpp
// ---------------------------------------------------------------------
#define TTL_7483(name, pA1, pA2, pA3, pA4, pB1, pB2, pB3, pB4, pC0)    \
	NET_REGISTER_DEVEXT(TTL_7483, name, pA1, pA2, pA3, pA4, pB1, pB2, pB3, pB4, pC0)

#define TTL_7483_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7483_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7485.cpp
// ---------------------------------------------------------------------
#define TTL_7485(name, pA0, pA1, pA2, pA3, pB0, pB1, pB2, pB3, pLTIN, pEQIN, pGTIN)\
	NET_REGISTER_DEVEXT(TTL_7485, name, pA0, pA1, pA2, pA3, pB0, pB1, pB2, pB3, pLTIN, pEQIN, pGTIN)

#define TTL_7485_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7485_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7490.cpp
// ---------------------------------------------------------------------
#define TTL_7490(name, pA, pB, pR1, pR2, pR91, pR92)                   \
	NET_REGISTER_DEVEXT(TTL_7490, name, pA, pB, pR1, pR2, pR91, pR92)

#define TTL_7490_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7490_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7492.cpp
// ---------------------------------------------------------------------
#define TTL_7492(name, pA, pB, pR1, pR2)                               \
	NET_REGISTER_DEVEXT(TTL_7492, name, pA, pB, pR1, pR2)

#define TTL_7492_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7492_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7493.cpp
// ---------------------------------------------------------------------
#define TTL_7493(name, pCLKA, pCLKB, pR1, pR2)                         \
	NET_REGISTER_DEVEXT(TTL_7493, name, pCLKA, pCLKB, pR1, pR2)

#define TTL_7493_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7493_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_7497.cpp
// ---------------------------------------------------------------------
#define TTL_7497(name, pCLK, pSTRBQ, pENQ, pUNITYQ, pCLR, pB0, pB1, pB2, pB3, pB4, pB5)\
	NET_REGISTER_DEVEXT(TTL_7497, name, pCLK, pSTRBQ, pENQ, pUNITYQ, pCLR, pB0, pB1, pB2, pB3, pB4, pB5)

#define TTL_7497_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7497_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74107.cpp
// ---------------------------------------------------------------------
#define TTL_74107(name, pCLK, pJ, pK, pCLRQ)                           \
	NET_REGISTER_DEVEXT(TTL_74107, name, pCLK, pJ, pK, pCLRQ)

#define TTL_74107_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_74107_DIP, name)

#define TTL_74107A(name, pCLK, pJ, pK, pCLRQ)                          \
	NET_REGISTER_DEVEXT(TTL_74107A, name, pCLK, pJ, pK, pCLRQ)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74123.cpp
// ---------------------------------------------------------------------
#define TTL_74123(name)                                                \
	NET_REGISTER_DEVEXT(TTL_74123, name)

#define TTL_74123_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_74123_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74153.cpp
// ---------------------------------------------------------------------
#define TTL_74153(name, pC0, pC1, pC2, pC3, pA, pB, pG)                \
	NET_REGISTER_DEVEXT(TTL_74153, name, pC0, pC1, pC2, pC3, pA, pB, pG)

#define TTL_74153_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_74153_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74161.cpp
// ---------------------------------------------------------------------
#define TTL_74161(name, pA, pB, pC, pD, pCLRQ, pLOADQ, pCLK, pENABLEP, pENABLET)\
	NET_REGISTER_DEVEXT(TTL_74161, name, pA, pB, pC, pD, pCLRQ, pLOADQ, pCLK, pENABLEP, pENABLET)

#define TTL_74161_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_74161_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_9316.cpp
// ---------------------------------------------------------------------
#define TTL_74163(name, pCLK, pENP, pENT, pCLRQ, pLOADQ, pA, pB, pC, pD)\
	NET_REGISTER_DEVEXT(TTL_74163, name, pCLK, pENP, pENT, pCLRQ, pLOADQ, pA, pB, pC, pD)

#define TTL_74163_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_74163_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74164.cpp
// ---------------------------------------------------------------------
#define TTL_74164(name, pA, pB, pCLRQ, pCLK)                           \
	NET_REGISTER_DEVEXT(TTL_74164, name, pA, pB, pCLRQ, pCLK)

#define TTL_74164_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_74164_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74165.cpp
// ---------------------------------------------------------------------
#define TTL_74165(name, pCLK, pCLKINH, pSH_LDQ, pSER, pA, pB, pC, pD, pE, pF, pG, pH)\
	NET_REGISTER_DEVEXT(TTL_74165, name, pCLK, pCLKINH, pSH_LDQ, pSER, pA, pB, pC, pD, pE, pF, pG, pH)

#define TTL_74165_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_74165_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74166.cpp
// ---------------------------------------------------------------------
#define TTL_74166(name, pCLK, pCLKINH, pSH_LDQ, pSER, pA, pB, pC, pD, pE, pF, pG, pH, pCLRQ)\
	NET_REGISTER_DEVEXT(TTL_74166, name, pCLK, pCLKINH, pSH_LDQ, pSER, pA, pB, pC, pD, pE, pF, pG, pH, pCLRQ)

#define TTL_74166_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_74166_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74174.cpp
// ---------------------------------------------------------------------
#define TTL_74174(name, pCLK, pD1, pD2, pD3, pD4, pD5, pD6, pCLRQ)     \
	NET_REGISTER_DEVEXT(TTL_74174, name, pCLK, pD1, pD2, pD3, pD4, pD5, pD6, pCLRQ)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74175.cpp
// ---------------------------------------------------------------------
#define TTL_74175(name, pCLK, pD1, pD2, pD3, pD4, pCLRQ)               \
	NET_REGISTER_DEVEXT(TTL_74175, name, pCLK, pD1, pD2, pD3, pD4, pCLRQ)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74192.cpp
// ---------------------------------------------------------------------
#define TTL_74192(name, pA, pB, pC, pD, pCLEAR, pLOADQ, pCU, pCD)      \
	NET_REGISTER_DEVEXT(TTL_74192, name, pA, pB, pC, pD, pCLEAR, pLOADQ, pCU, pCD)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74193.cpp
// ---------------------------------------------------------------------
#define TTL_74193(name, pA, pB, pC, pD, pCLEAR, pLOADQ, pCU, pCD)      \
	NET_REGISTER_DEVEXT(TTL_74193, name, pA, pB, pC, pD, pCLEAR, pLOADQ, pCU, pCD)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74194.cpp
// ---------------------------------------------------------------------
#define TTL_74194(name, pCLK, pS0, pS1, pSRIN, pA, pB, pC, pD, pSLIN, pCLRQ)\
	NET_REGISTER_DEVEXT(TTL_74194, name, pCLK, pS0, pS1, pSRIN, pA, pB, pC, pD, pSLIN, pCLRQ)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74365.cpp
// ---------------------------------------------------------------------
#define TTL_74365(name, pG1Q, pG2Q, pA1, pA2, pA3, pA4, pA5, pA6)      \
	NET_REGISTER_DEVEXT(TTL_74365, name, pG1Q, pG2Q, pA1, pA2, pA3, pA4, pA5, pA6)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74377.cpp
// ---------------------------------------------------------------------
#define TTL_74377_GATE(name)                                           \
	NET_REGISTER_DEVEXT(TTL_74377_GATE, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74393.cpp
// ---------------------------------------------------------------------
#define TTL_74393(name, pCP, pMR)                                      \
	NET_REGISTER_DEVEXT(TTL_74393, name, pCP, pMR)

#define TTL_74393_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_74393_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74ls629.cpp
// ---------------------------------------------------------------------
#define SN74LS629(name, pCAP)                                          \
	NET_REGISTER_DEVEXT(SN74LS629, name, pCAP)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_82S16.cpp
// ---------------------------------------------------------------------
#define TTL_82S16(name)                                                \
	NET_REGISTER_DEVEXT(TTL_82S16, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_82S115.cpp
// ---------------------------------------------------------------------
#define PROM_82S115(name, pCE1Q, pCE2, pA0, pA1, pA2, pA3, pA4, pA5, pA6, pA7, pA8, pSTROBE)\
	NET_REGISTER_DEVEXT(PROM_82S115, name, pCE1Q, pCE2, pA0, pA1, pA2, pA3, pA4, pA5, pA6, pA7, pA8, pSTROBE)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_82S123.cpp
// ---------------------------------------------------------------------
#define PROM_82S123(name, pCEQ, pA0, pA1, pA2, pA3, pA4)               \
	NET_REGISTER_DEVEXT(PROM_82S123, name, pCEQ, pA0, pA1, pA2, pA3, pA4)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_82S126.cpp
// ---------------------------------------------------------------------
#define PROM_82S126(name, pCE1Q, pCE2Q, pA0, pA1, pA2, pA3, pA4, pA5, pA6, pA7)\
	NET_REGISTER_DEVEXT(PROM_82S126, name, pCE1Q, pCE2Q, pA0, pA1, pA2, pA3, pA4, pA5, pA6, pA7)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_9310.cpp
// ---------------------------------------------------------------------
#define TTL_9310(name, pCLK, pENP, pENT, pCLRQ, pLOADQ, pA, pB, pC, pD)\
	NET_REGISTER_DEVEXT(TTL_9310, name, pCLK, pENP, pENT, pCLRQ, pLOADQ, pA, pB, pC, pD)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_dm9314.cpp
// ---------------------------------------------------------------------
#define TTL_9314(name, pEQ, pMRQ, pS0Q, pS1Q, pS2Q, pS3Q, pD0, pD1, pD2, pD3)\
	NET_REGISTER_DEVEXT(TTL_9314, name, pEQ, pMRQ, pS0Q, pS1Q, pS2Q, pS3Q, pD0, pD1, pD2, pD3)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_9316.cpp
// ---------------------------------------------------------------------
#define TTL_9316(name, pCLK, pENP, pENT, pCLRQ, pLOADQ, pA, pB, pC, pD)\
	NET_REGISTER_DEVEXT(TTL_9316, name, pCLK, pENP, pENT, pCLRQ, pLOADQ, pA, pB, pC, pD)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_9322.cpp
// ---------------------------------------------------------------------
#define TTL_9322(name, pSELECT, pA1, pB1, pA2, pB2, pA3, pB3, pA4, pB4, pSTROBE)\
	NET_REGISTER_DEVEXT(TTL_9322, name, pSELECT, pA1, pB1, pA2, pB2, pA3, pB3, pA4, pB4, pSTROBE)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_dm9334.cpp
// ---------------------------------------------------------------------
#define TTL_9334(name, pCQ, pEQ, pD, pA0, pA1, pA2)                    \
	NET_REGISTER_DEVEXT(TTL_9334, name, pCQ, pEQ, pD, pA0, pA1, pA2)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_am2847.cpp
// ---------------------------------------------------------------------
#define TTL_AM2847(name, pCP, pINA, pINB, pINC, pIND, pRCA, pRCB, pRCC, pRCD)\
	NET_REGISTER_DEVEXT(TTL_AM2847, name, pCP, pINA, pINB, pINC, pIND, pRCA, pRCB, pRCC, pRCD)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_4006.cpp
// ---------------------------------------------------------------------
#define CD4006(name, pCLOCK, pD1, pD2, pD3, pD4, pD1P4, pD1P4S, pD2P4, pD2P5, pD3P4, pD4P4, pD3P5)\
	NET_REGISTER_DEVEXT(CD4006, name, pCLOCK, pD1, pD2, pD3, pD4, pD1P4, pD1P4S, pD2P4, pD2P5, pD3P4, pD4P4, pD3P5)

#define CD4006_DIP(name)                                               \
	NET_REGISTER_DEVEXT(CD4006_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_4020.cpp
// ---------------------------------------------------------------------
#define CD4020_WI(name, pIP, pRESET, pVDD, pVSS)                       \
	NET_REGISTER_DEVEXT(CD4020_WI, name, pIP, pRESET, pVDD, pVSS)

#define CD4020(name)                                                   \
	NET_REGISTER_DEVEXT(CD4020, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_4066.cpp
// ---------------------------------------------------------------------
#define CD4066_GATE(name)                                              \
	NET_REGISTER_DEVEXT(CD4066_GATE, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_4316.cpp
// ---------------------------------------------------------------------
#define CD4316_GATE(name)                                              \
	NET_REGISTER_DEVEXT(CD4316_GATE, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74123.cpp
// ---------------------------------------------------------------------
#define CD4538_DIP(name)                                               \
	NET_REGISTER_DEVEXT(CD4538_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_schmitt.cpp
// ---------------------------------------------------------------------
#define SCHMITT_TRIGGER(name, pSTMODEL)                                \
	NET_REGISTER_DEVEXT(SCHMITT_TRIGGER, name, pSTMODEL)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_ne555.cpp
// ---------------------------------------------------------------------
#define NE555(name)                                                    \
	NET_REGISTER_DEVEXT(NE555, name)

#define NE555_DIP(name)                                                \
	NET_REGISTER_DEVEXT(NE555_DIP, name)

#define MC1455P(name)                                                  \
	NET_REGISTER_DEVEXT(MC1455P, name)

#define MC1455P_DIP(name)                                              \
	NET_REGISTER_DEVEXT(MC1455P_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_tms4800.cpp
// ---------------------------------------------------------------------
#define ROM_TMS4800(name, pAR, pOE1, pOE2, pA0, pA1, pA2, pA3, pA4, pA5, pA6, pA7, pA8, pA9, pA10)\
	NET_REGISTER_DEVEXT(ROM_TMS4800, name, pAR, pOE1, pOE2, pA0, pA1, pA2, pA3, pA4, pA5, pA6, pA7, pA8, pA9, pA10)

#define ROM_TMS4800_DIP(name)                                          \
	NET_REGISTER_DEVEXT(ROM_TMS4800_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_r2r_dac.cpp
// ---------------------------------------------------------------------
#define R2R_DAC(name, pVIN, pR, pN)                                    \
	NET_REGISTER_DEVEXT(R2R_DAC, name, pVIN, pR, pN)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_tristate.cpp
// ---------------------------------------------------------------------
#define TTL_TRISTATE(name, pCEQ1, pD1, pCEQ2, pD2)                     \
	NET_REGISTER_DEVEXT(TTL_TRISTATE, name, pCEQ1, pD1, pCEQ2, pD2)

#define TTL_TRISTATE3(name)                                            \
	NET_REGISTER_DEVEXT(TTL_TRISTATE3, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74174.cpp
// ---------------------------------------------------------------------
#define TTL_74174_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_74174_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74175.cpp
// ---------------------------------------------------------------------
#define TTL_74175_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_74175_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74192.cpp
// ---------------------------------------------------------------------
#define TTL_74192_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_74192_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74193.cpp
// ---------------------------------------------------------------------
#define TTL_74193_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_74193_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74194.cpp
// ---------------------------------------------------------------------
#define TTL_74194_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_74194_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74365.cpp
// ---------------------------------------------------------------------
#define TTL_74365_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_74365_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_82S16.cpp
// ---------------------------------------------------------------------
#define TTL_82S16_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_82S16_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_82S115.cpp
// ---------------------------------------------------------------------
#define PROM_82S115_DIP(name)                                          \
	NET_REGISTER_DEVEXT(PROM_82S115_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_82S123.cpp
// ---------------------------------------------------------------------
#define PROM_82S123_DIP(name)                                          \
	NET_REGISTER_DEVEXT(PROM_82S123_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_82S126.cpp
// ---------------------------------------------------------------------
#define PROM_82S126_DIP(name)                                          \
	NET_REGISTER_DEVEXT(PROM_82S126_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74123.cpp
// ---------------------------------------------------------------------
#define TTL_9602_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_9602_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_9310.cpp
// ---------------------------------------------------------------------
#define TTL_9310_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_9310_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_dm9314.cpp
// ---------------------------------------------------------------------
#define TTL_9314_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_9314_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_9316.cpp
// ---------------------------------------------------------------------
#define TTL_9316_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_9316_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_9322.cpp
// ---------------------------------------------------------------------
#define TTL_9322_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_9322_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_dm9334.cpp
// ---------------------------------------------------------------------
#define TTL_9334_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_9334_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_am2847.cpp
// ---------------------------------------------------------------------
#define TTL_AM2847_DIP(name)                                           \
	NET_REGISTER_DEVEXT(TTL_AM2847_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_74ls629.cpp
// ---------------------------------------------------------------------
#define SN74LS629_DIP(name, p1_CAP1, p2_CAP2)                          \
	NET_REGISTER_DEVEXT(SN74LS629_DIP, name, p1_CAP1, p2_CAP2)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/devices/nld_mm5837.cpp
// ---------------------------------------------------------------------
#define MM5837_DIP(name)                                               \
	NET_REGISTER_DEVEXT(MM5837_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/macro/nlm_ttl74xx.cpp
// ---------------------------------------------------------------------
#define TTL_7400_GATE(name)                                            \
	NET_REGISTER_DEVEXT(TTL_7400_GATE, name)

#define TTL_7400_NAND(name, pA, pB)                                    \
	NET_REGISTER_DEVEXT(TTL_7400_NAND, name, pA, pB)

#define TTL_7402_GATE(name)                                            \
	NET_REGISTER_DEVEXT(TTL_7402_GATE, name)

#define TTL_7402_NOR(name, pA, pB)                                     \
	NET_REGISTER_DEVEXT(TTL_7402_NOR, name, pA, pB)

#define TTL_7404_GATE(name)                                            \
	NET_REGISTER_DEVEXT(TTL_7404_GATE, name)

#define TTL_7404_INVERT(name, pA)                                      \
	NET_REGISTER_DEVEXT(TTL_7404_INVERT, name, pA)

#define TTL_7406_GATE(name)                                            \
	NET_REGISTER_DEVEXT(TTL_7406_GATE, name)

#define TTL_7408_GATE(name)                                            \
	NET_REGISTER_DEVEXT(TTL_7408_GATE, name)

#define TTL_7408_AND(name, pA, pB)                                     \
	NET_REGISTER_DEVEXT(TTL_7408_AND, name, pA, pB)

#define TTL_7410_NAND(name, pA, pB, pC)                                \
	NET_REGISTER_DEVEXT(TTL_7410_NAND, name, pA, pB, pC)

#define TTL_7410_GATE(name)                                            \
	NET_REGISTER_DEVEXT(TTL_7410_GATE, name)

#define TTL_7411_AND(name, pA, pB, pC)                                 \
	NET_REGISTER_DEVEXT(TTL_7411_AND, name, pA, pB, pC)

#define TTL_7411_GATE(name)                                            \
	NET_REGISTER_DEVEXT(TTL_7411_GATE, name)

#define TTL_7416_GATE(name)                                            \
	NET_REGISTER_DEVEXT(TTL_7416_GATE, name)

#define TTL_7420_GATE(name)                                            \
	NET_REGISTER_DEVEXT(TTL_7420_GATE, name)

#define TTL_7420_NAND(name, pA, pB, pC, pD)                            \
	NET_REGISTER_DEVEXT(TTL_7420_NAND, name, pA, pB, pC, pD)

#define TTL_7421_GATE(name)                                            \
	NET_REGISTER_DEVEXT(TTL_7421_GATE, name)

#define TTL_7421_NAND(name, pA, pB, pC, pD)                            \
	NET_REGISTER_DEVEXT(TTL_7421_NAND, name, pA, pB, pC, pD)

#define TTL_7425_GATE(name)                                            \
	NET_REGISTER_DEVEXT(TTL_7425_GATE, name)

#define TTL_7425_NOR(name, pA, pB, pC, pD)                             \
	NET_REGISTER_DEVEXT(TTL_7425_NOR, name, pA, pB, pC, pD)

#define TTL_7427_GATE(name)                                            \
	NET_REGISTER_DEVEXT(TTL_7427_GATE, name)

#define TTL_7427_NOR(name, pA, pB, pC)                                 \
	NET_REGISTER_DEVEXT(TTL_7427_NOR, name, pA, pB, pC)

#define TTL_7430_GATE(name)                                            \
	NET_REGISTER_DEVEXT(TTL_7430_GATE, name)

#define TTL_7430_NAND(name, pA, pB, pC, pD, pE, pF, pG, pH)            \
	NET_REGISTER_DEVEXT(TTL_7430_NAND, name, pA, pB, pC, pD, pE, pF, pG, pH)

#define TTL_7432_GATE(name)                                            \
	NET_REGISTER_DEVEXT(TTL_7432_GATE, name)

#define TTL_7432_OR(name, pA, pB)                                      \
	NET_REGISTER_DEVEXT(TTL_7432_OR, name, pA, pB)

#define TTL_7437_GATE(name)                                            \
	NET_REGISTER_DEVEXT(TTL_7437_GATE, name)

#define TTL_7437_NAND(name, pA, pB)                                    \
	NET_REGISTER_DEVEXT(TTL_7437_NAND, name, pA, pB)

#define TTL_7486_GATE(name)                                            \
	NET_REGISTER_DEVEXT(TTL_7486_GATE, name)

#define TTL_7486_XOR(name, pA, pB)                                     \
	NET_REGISTER_DEVEXT(TTL_7486_XOR, name, pA, pB)

#define TTL_74155A_GATE(name)                                          \
	NET_REGISTER_DEVEXT(TTL_74155A_GATE, name)

#define TTL_74155B_GATE(name)                                          \
	NET_REGISTER_DEVEXT(TTL_74155B_GATE, name)

#define TTL_74156A_GATE(name)                                          \
	NET_REGISTER_DEVEXT(TTL_74156A_GATE, name)

#define TTL_74156B_GATE(name)                                          \
	NET_REGISTER_DEVEXT(TTL_74156B_GATE, name)

#define TTL_74260_GATE(name)                                           \
	NET_REGISTER_DEVEXT(TTL_74260_GATE, name)

#define TTL_74260_NOR(name, pA, pB, pC, pD, pE)                        \
	NET_REGISTER_DEVEXT(TTL_74260_NOR, name, pA, pB, pC, pD, pE)

#define TTL_74279A(name)                                               \
	NET_REGISTER_DEVEXT(TTL_74279A, name)

#define TTL_74279B(name)                                               \
	NET_REGISTER_DEVEXT(TTL_74279B, name)

#define DM9312(name, pA, pB, pC, pG, pD0, pD1, pD2, pD3, pD4, pD5, pD6, pD7)\
	NET_REGISTER_DEVEXT(DM9312, name, pA, pB, pC, pG, pD0, pD1, pD2, pD3, pD4, pD5, pD6, pD7)

#define TTL_7400_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7400_DIP, name)

#define TTL_7402_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7402_DIP, name)

#define TTL_7404_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7404_DIP, name)

#define TTL_7406_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7406_DIP, name)

#define TTL_7408_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7408_DIP, name)

#define TTL_7410_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7410_DIP, name)

#define TTL_7411_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7411_DIP, name)

#define TTL_7414_GATE(name)                                            \
	NET_REGISTER_DEVEXT(TTL_7414_GATE, name)

#define TTL_74LS14_GATE(name)                                          \
	NET_REGISTER_DEVEXT(TTL_74LS14_GATE, name)

#define TTL_7414_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7414_DIP, name)

#define TTL_74LS14_DIP(name)                                           \
	NET_REGISTER_DEVEXT(TTL_74LS14_DIP, name)

#define TTL_7416_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7416_DIP, name)

#define TTL_7420_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7420_DIP, name)

#define TTL_7421_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7421_DIP, name)

#define TTL_7425_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7425_DIP, name)

#define TTL_7427_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7427_DIP, name)

#define TTL_7430_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7430_DIP, name)

#define TTL_7432_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7432_DIP, name)

#define TTL_7437_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7437_DIP, name)

#define TTL_7486_DIP(name)                                             \
	NET_REGISTER_DEVEXT(TTL_7486_DIP, name)

#define TTL_74155_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_74155_DIP, name)

#define TTL_74156_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_74156_DIP, name)

#define TTL_74260_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_74260_DIP, name)

#define TTL_74279_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_74279_DIP, name)

#define TTL_74377_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_74377_DIP, name)

#define TTL_74378_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_74378_DIP, name)

#define TTL_74379_DIP(name)                                            \
	NET_REGISTER_DEVEXT(TTL_74379_DIP, name)

#define DM9312_DIP(name)                                               \
	NET_REGISTER_DEVEXT(DM9312_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/macro/nlm_cd4xxx.cpp
// ---------------------------------------------------------------------
#define CD4001_GATE(name)                                              \
	NET_REGISTER_DEVEXT(CD4001_GATE, name)

#define CD4070_GATE(name)                                              \
	NET_REGISTER_DEVEXT(CD4070_GATE, name)

#define CD4001_DIP(name)                                               \
	NET_REGISTER_DEVEXT(CD4001_DIP, name)

#define CD4070_DIP(name)                                               \
	NET_REGISTER_DEVEXT(CD4070_DIP, name)

#define CD4020_DIP(name)                                               \
	NET_REGISTER_DEVEXT(CD4020_DIP, name)

#define CD4016_DIP(name)                                               \
	NET_REGISTER_DEVEXT(CD4016_DIP, name)

#define CD4066_DIP(name)                                               \
	NET_REGISTER_DEVEXT(CD4066_DIP, name)

#define CD4316_DIP(name)                                               \
	NET_REGISTER_DEVEXT(CD4316_DIP, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/macro/nlm_opamp.cpp
// ---------------------------------------------------------------------
#define opamp_layout_4_4_11(name)                                      \
	NET_REGISTER_DEVEXT(opamp_layout_4_4_11, name)

#define opamp_layout_2_8_4(name)                                       \
	NET_REGISTER_DEVEXT(opamp_layout_2_8_4, name)

#define opamp_layout_2_13_9_4(name)                                    \
	NET_REGISTER_DEVEXT(opamp_layout_2_13_9_4, name)

#define opamp_layout_1_7_4(name)                                       \
	NET_REGISTER_DEVEXT(opamp_layout_1_7_4, name)

#define opamp_layout_1_8_5(name)                                       \
	NET_REGISTER_DEVEXT(opamp_layout_1_8_5, name)

#define opamp_layout_1_11_6(name)                                      \
	NET_REGISTER_DEVEXT(opamp_layout_1_11_6, name)

#define MB3614_DIP(name)                                               \
	NET_REGISTER_DEVEXT(MB3614_DIP, name)

#define TL081_DIP(name)                                                \
	NET_REGISTER_DEVEXT(TL081_DIP, name)

#define TL084_DIP(name)                                                \
	NET_REGISTER_DEVEXT(TL084_DIP, name)

#define LM324_DIP(name)                                                \
	NET_REGISTER_DEVEXT(LM324_DIP, name)

#define LM358_DIP(name)                                                \
	NET_REGISTER_DEVEXT(LM358_DIP, name)

#define LM2902_DIP(name)                                               \
	NET_REGISTER_DEVEXT(LM2902_DIP, name)

#define UA741_DIP8(name)                                               \
	NET_REGISTER_DEVEXT(UA741_DIP8, name)

#define UA741_DIP10(name)                                              \
	NET_REGISTER_DEVEXT(UA741_DIP10, name)

#define UA741_DIP14(name)                                              \
	NET_REGISTER_DEVEXT(UA741_DIP14, name)

#define LM747_DIP(name)                                                \
	NET_REGISTER_DEVEXT(LM747_DIP, name)

#define LM747A_DIP(name)                                               \
	NET_REGISTER_DEVEXT(LM747A_DIP, name)

#define LM3900(name)                                                   \
	NET_REGISTER_DEVEXT(LM3900, name)

// ---------------------------------------------------------------------
// Source: src/lib/netlist/macro/nlm_other.cpp
// ---------------------------------------------------------------------
#define MC14584B_GATE(name)                                            \
	NET_REGISTER_DEVEXT(MC14584B_GATE, name)

#define MC14584B_DIP(name)                                             \
	NET_REGISTER_DEVEXT(MC14584B_DIP, name)

#define NE566_DIP(name)                                                \
	NET_REGISTER_DEVEXT(NE566_DIP, name)

#endif // __PLIB_PREPROCESSOR__
#endif
