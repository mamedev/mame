// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLM_TTL74XX_H_
#define NLM_TTL74XX_H_

///
/// \file nlm_ttl74xx.h
///

#include "netlist/nl_setup.h"

#ifndef __PLIB_PREPROCESSOR__

/* ----------------------------------------------------------------------------
 *  Netlist Macros
 * ---------------------------------------------------------------------------*/

#ifndef NL_AUTO_DEVICES

#define TTL_7400_GATE(name)                                                    \
		NET_REGISTER_DEV(TTL_7400_GATE, name)

#define TTL_7400_NAND(name, cA, cB)                                            \
		NET_REGISTER_DEV(TTL_7400_NAND, name)                                  \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, A, cA)                                               \
		NET_CONNECT(name, B, cB)

#define TTL_7400_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7400_DIP, name)


#define TTL_7402_GATE(name)                                                    \
		NET_REGISTER_DEV(TTL_7402_GATE, name)

#define TTL_7402_NOR(name, cI1, cI2)                                           \
		NET_REGISTER_DEV(TTL_7402_NOR, name)                                   \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, A, cI1)                                              \
		NET_CONNECT(name, B, cI2)

#define TTL_7402_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7402_DIP, name)

#define TTL_7404_GATE(name)                                                    \
		NET_REGISTER_DEV(TTL_7404_GATE, name)

#define TTL_7404_INVERT(name, cA)                                              \
		NET_REGISTER_DEV(TTL_7404_INVERT, name)                                \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, A, cA)

#define TTL_7404_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7404_DIP, name)

#define TTL_7408_GATE(name)                                                    \
		NET_REGISTER_DEV(TTL_7408_GATE, name)

#define TTL_7408_AND(name, cA, cB)                                 \
		NET_REGISTER_DEV(TTL_7408_AND, name)                                   \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, A, cA)                                               \
		NET_CONNECT(name, B, cB)

#define TTL_7408_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7408_DIP, name)

#define TTL_7410_GATE(name)                                                    \
		NET_REGISTER_DEV(TTL_7410_GATE, name)

#define TTL_7410_NAND(name, cI1, cI2, cI3)                                     \
		NET_REGISTER_DEV(TTL_7410_NAND, name)                                  \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, A, cI1)                                              \
		NET_CONNECT(name, B, cI2)                                              \
		NET_CONNECT(name, C, cI3)

#define TTL_7410_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7410_DIP, name)


#define TTL_7411_GATE(name)                                                    \
		NET_REGISTER_DEV(TTL_7411_GATE, name)

#define TTL_7411_AND(name, cI1, cI2, cI3)                                      \
		NET_REGISTER_DEV(TTL_7411_AND, name)                                   \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, A, cI1)                                              \
		NET_CONNECT(name, B, cI2)                                              \
		NET_CONNECT(name, C, cI3)

#define TTL_7411_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7411_DIP, name)

#define TTL_7414_GATE(name)                                                    \
		NET_REGISTER_DEV(TTL_7414_GATE, name)

#define TTL_7414_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7414_DIP, name)


#define TTL_74LS14_GATE(name)                                                  \
		NET_REGISTER_DEV(TTL_74LS14_GATE, name)

#define TTL_74LS14_DIP(name)                                                   \
		NET_REGISTER_DEV(TTL_74LS14_DIP, name)


#define TTL_7416_GATE(name)                                                    \
		NET_REGISTER_DEV(TTL_7416_GATE, name)

#define TTL_7416_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7416_DIP, name)


#define TTL_7420_GATE(name)                                                    \
		NET_REGISTER_DEV(TTL_7420_GATE, name)

#define TTL_7420_NAND(name, cI1, cI2, cI3, cI4)                                \
		NET_REGISTER_DEV(TTL_7420_NAND, name)                                  \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, A, cI1)                                              \
		NET_CONNECT(name, B, cI2)                                              \
		NET_CONNECT(name, C, cI3)                                              \
		NET_CONNECT(name, D, cI4)

#define TTL_7420_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7420_DIP, name)


#define TTL_7425_GATE(name)                                                    \
		NET_REGISTER_DEV(TTL_7425_GATE, name)

#define TTL_7425_NOR(name, cI1, cI2, cI3, cI4)                                 \
		NET_REGISTER_DEV(TTL_7425_NOR, name)                                   \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, A, cI1)                                              \
		NET_CONNECT(name, B, cI2)                                              \
		NET_CONNECT(name, C, cI3)                                              \
		NET_CONNECT(name, D, cI4)

#define TTL_7425_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7425_DIP, name)


#define TTL_7427_GATE(name)                                                    \
		NET_REGISTER_DEV(TTL_7427_GATE, name)

#define TTL_7427_NOR(name, cI1, cI2, cI3)                                      \
		NET_REGISTER_DEV(TTL_7427_NOR, name)                                   \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, A, cI1)                                              \
		NET_CONNECT(name, B, cI2)                                              \
		NET_CONNECT(name, C, cI3)

#define TTL_7427_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7427_DIP, name)


#define TTL_7430_GATE(name)                                                    \
		NET_REGISTER_DEV(TTL_7430_GATE, name)

#define TTL_7430_NAND(name, cI1, cI2, cI3, cI4, cI5, cI6, cI7, cI8)\
		NET_REGISTER_DEV(TTL_7430_NAND, name)                                  \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, A, cI1)                                              \
		NET_CONNECT(name, B, cI2)                                              \
		NET_CONNECT(name, C, cI3)                                              \
		NET_CONNECT(name, D, cI4)                                              \
		NET_CONNECT(name, E, cI5)                                              \
		NET_CONNECT(name, F, cI6)                                              \
		NET_CONNECT(name, G, cI7)                                              \
		NET_CONNECT(name, H, cI8)

#define TTL_7430_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7430_DIP, name)


#define TTL_7432_GATE(name)                                                    \
		NET_REGISTER_DEV(TTL_7432_OR, name)

#define TTL_7432_OR(name, cI1, cI2)                                            \
		NET_REGISTER_DEV(TTL_7432_OR, name)                                    \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, A, cI1)                                              \
		NET_CONNECT(name, B, cI2)

#define TTL_7432_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7432_DIP, name)

#define TTL_7437_GATE(name)                                                    \
		NET_REGISTER_DEV(TTL_7437_GATE, name)

#define TTL_7437_NAND(name, cA, cB)                                            \
		NET_REGISTER_DEV(TTL_7437_NAND, name)                                  \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, A, cA)                                               \
		NET_CONNECT(name, B, cB)

#define TTL_7437_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7437_DIP, name)


#if (NL_USE_TRUTHTABLE_7448)
#define TTL_7448(name, cA0, cA1, cA2, cA3, cLTQ, cBIQ, cRBIQ)                  \
		NET_REGISTER_DEV(TTL_7448, name)                                       \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, A, cA0)                                              \
		NET_CONNECT(name, B, cA1)                                              \
		NET_CONNECT(name, C, cA2)                                              \
		NET_CONNECT(name, D, cA3)                                              \
		NET_CONNECT(name, LTQ, cLTQ)                                           \
		NET_CONNECT(name, BIQ, cBIQ)                                           \
		NET_CONNECT(name, RBIQ, cRBIQ)

#define TTL_7448_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7448_DIP, name)

#endif

#define TTL_7486_GATE(name)                                                    \
		NET_REGISTER_DEV(TTL_7486_GATE, name)

#define TTL_7486_XOR(name, cA, cB)                                             \
		NET_REGISTER_DEV(TTL_7486_XOR, name)                                   \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, A, cA)                                               \
		NET_CONNECT(name, B, cB)

#define TTL_7486_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7486_DIP, name)

#if (NL_USE_TRUTHTABLE_74107)
#define TTL_74107(name, cCLK, cJ, cK, cCLRQ)                                   \
		NET_REGISTER_DEV(TTL_74107, name)                                      \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, CLK, cCLK)                                           \
		NET_CONNECT(name, J, cJ)                                               \
		NET_CONNECT(name, K, cK)                                               \
		NET_CONNECT(name, CLRQ, cCLRQ)

#define TTL_74107_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74107_DIP, name)

#endif

#define TTL_74155_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74155_DIP, name)

#define TTL_74156_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74156_DIP, name)

#define TTL_74260_GATE(name)                                                   \
		NET_REGISTER_DEV(TTL_74260_GATE, name)

#define TTL_74260_NOR(name, cA, cB, cC, cD, cE)                                \
		NET_REGISTER_DEV(TTL_74260_NOR, name)                                  \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, A, cA)                                               \
		NET_CONNECT(name, B, cB)                                               \
		NET_CONNECT(name, C, cC)                                               \
		NET_CONNECT(name, D, cD)                                               \
		NET_CONNECT(name, E, cE)

#define TTL_74260_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74260_DIP, name)

#define TTL_74279_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74279_DIP, name)

#define DM9312(name, cA, cB, cC, cSTROBE, cD0, cD1, cD2, cD3, cD4, cD5, cD6, cD7)     \
		NET_REGISTER_DEV(DM9312, name)                                         \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, A,  cA)                                              \
		NET_CONNECT(name, B,  cB)                                              \
		NET_CONNECT(name, C,  cC)                                              \
		NET_CONNECT(name, G,  cSTROBE)                                         \
		NET_CONNECT(name, D0, cD0)                                             \
		NET_CONNECT(name, D1, cD1)                                             \
		NET_CONNECT(name, D2, cD2)                                             \
		NET_CONNECT(name, D3, cD3)                                             \
		NET_CONNECT(name, D4, cD4)                                             \
		NET_CONNECT(name, D5, cD5)                                             \
		NET_CONNECT(name, D6, cD6)                                             \
		NET_CONNECT(name, D7, cD7)

#define DM9312_DIP(name)                                                       \
		NET_REGISTER_DEV(DM9312_DIP, name)

#endif // NL_AUTO_DEVICES

/* ----------------------------------------------------------------------------
 *  External declarations
 * ---------------------------------------------------------------------------*/

NETLIST_EXTERNAL(TTL74XX_lib)

#endif // __PLIB_PREPROCESSOR__


#endif // NLM_TTL74XX
