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

#if !NL_AUTO_DEVICES

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


#define TTL_7406_GATE(name)                                                    \
		NET_REGISTER_DEV(TTL_7406_GATE, name)

#define TTL_7406_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7406_DIP, name)


#define TTL_7407_GATE(name)                                                    \
		NET_REGISTER_DEV(TTL_7407_GATE, name)

#define TTL_7407_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7407_DIP, name)


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


#define TTL_7421_GATE(name)                                                    \
		NET_REGISTER_DEV(TTL_7421_GATE, name)

#define TTL_7421_AND(name, cI1, cI2, cI3, cI4)                                 \
		NET_REGISTER_DEV(TTL_7421_AND, name)                                   \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, A, cI1)                                              \
		NET_CONNECT(name, B, cI2)                                              \
		NET_CONNECT(name, C, cI3)                                              \
		NET_CONNECT(name, D, cI4)

#define TTL_7421_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7421_DIP, name)


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


#define TTL_7442_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7442_DIP, name)


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
#endif

#define TTL_7448_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7448_DIP, name)

#define TTL_7450_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7450_DIP, name)

#define TTL_7473_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7473_DIP, name)

#define TTL_7473A_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_7473A_DIP, name)

#define TTL_7474_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7474_DIP, name)

#define TTL_7475_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7475_DIP, name)

#define TTL_7477_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7477_DIP, name)

#define TTL_7483_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7483_DIP, name)

#define TTL_7485_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7485_DIP, name)

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

#define TTL_7490_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7490_DIP, name)

#define TTL_7492_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7492_DIP, name)

#define TTL_7493_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7493_DIP, name)

#define TTL_7497_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7497_DIP, name)

#if (NL_USE_TRUTHTABLE_74107)
// usage: TTL_74107(name, cCLK, cJ, cK, cCLRQ)
#define TTL_74107(...)                                                         \
		NET_REGISTER_DEVEXT(TTL_74107_TT, __VA_ARGS__)
#endif

#define TTL_74107_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74107_DIP, name)

#define TTL_74107A_DIP(name)                                                   \
		NET_REGISTER_DEV(TTL_74107A_DIP, name)

#define TTL_74113_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74113_DIP, name)

#define TTL_74113A_DIP(name)                                                   \
		NET_REGISTER_DEV(TTL_74113A_DIP, name)

#define TTL_74121_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74121_DIP, name)

#define TTL_74123_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74123_DIP, name)

#define TTL_9602_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_9602_DIP, name)

#define TTL_74125_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74125_DIP, name)

#define TTL_74153_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74153_DIP, name)

#define TTL_74155_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74155_DIP, name)

#define TTL_74156_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74156_DIP, name)

#define TTL_74157_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74157_DIP, name)

#define TTL_74161_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_74161_DIP, name)

#define TTL_74163_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_74163_DIP, name)

#define TTL_74164_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_74164_DIP, name)

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

#define TTL_74377_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74377_DIP, name)

#define TTL_74378_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74378_DIP, name)

#define TTL_74379_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74379_DIP, name)

#define DM9312(...)                                                            \
		NET_REGISTER_DEVEXT(DM9312, __VA_ARGS__)

#define DM9312_DIP(name)                                                       \
		NET_REGISTER_DEV(DM9312_DIP, name)

#define TTL_9310_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_9310_DIP, name)

#define TTL_9316_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_9316_DIP, name)

#define TTL_9322_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_9322_DIP, name)

#endif // NL_AUTO_DEVICES

/* ----------------------------------------------------------------------------
 *  External declarations
 * ---------------------------------------------------------------------------*/

// moved to net_lib.h

#endif // __PLIB_PREPROCESSOR__


#endif // NLM_TTL74XX
