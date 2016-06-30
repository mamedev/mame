// license:GPL-2.0+
// copyright-holders:Couriersud
#ifndef NLD_TTL74XX_H_
#define NLD_TTL74XX_H_

#include "nl_setup.h"

#ifndef __PLIB_PREPROCESSOR__

/* ----------------------------------------------------------------------------
 *  Netlist Macros
 * ---------------------------------------------------------------------------*/

#define TTL_7400_GATE(name)                                                   \
		NET_REGISTER_DEV(TTL_7400_GATE, name)

#define TTL_7400_NAND(name, cA, cB)                                           \
		NET_REGISTER_DEV(TTL_7400_NAND, name)                                  \
		NET_CONNECT(name, A, cA)                                               \
		NET_CONNECT(name, B, cB)

#define TTL_7400_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_7400_DIP, name)


#define TTL_7402_GATE(name)                                                   \
		NET_REGISTER_DEV(TTL_7402_GATE, name)

#define TTL_7402_NOR(name, cI1, cI2)                                          \
		NET_REGISTER_DEV(TTL_7402_NOR, name)                                  \
		NET_CONNECT(name, A, cI1)                                             \
		NET_CONNECT(name, B, cI2)

#define TTL_7402_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_7402_DIP, name)


#define TTL_7404_GATE(name)                                                   \
		NET_REGISTER_DEV(TTL_7404_GATE, name)

#define TTL_7404_INVERT(name, cA)                                             \
		NET_REGISTER_DEV(TTL_7404_INVERT, name)                               \
		NET_CONNECT(name, A, cA)

#define TTL_7404_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_7404_DIP, name)


#define TTL_7408_GATE(name)                                                   \
		NET_REGISTER_DEV(TTL_7408_GATE, name)

#define TTL_7408_AND(name, cA, cB)                                            \
		NET_REGISTER_DEV(TTL_7408_AND, name)                                   \
		NET_CONNECT(name, A, cA)                                               \
		NET_CONNECT(name, B, cB)

#define TTL_7408_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_7408_DIP, name)


#define TTL_7410_GATE(name)                                                   \
		NET_REGISTER_DEV(TTL_7410_GATE, name)

#define TTL_7410_NAND(name, cI1, cI2, cI3)                                    \
		NET_REGISTER_DEV(TTL_7410_NAND, name)                                 \
		NET_CONNECT(name, A, cI1)                                             \
		NET_CONNECT(name, B, cI2)                                             \
		NET_CONNECT(name, C, cI3)

#define TTL_7410_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_7410_DIP, name)


#define TTL_7411_GATE(name)                                                   \
		NET_REGISTER_DEV(TTL_7411_GATE, name)

#define TTL_7411_AND(name, cI1, cI2, cI3)                                     \
		NET_REGISTER_DEV(TTL_7411_AND, name)                                  \
		NET_CONNECT(name, A, cI1)                                             \
		NET_CONNECT(name, B, cI2)                                             \
		NET_CONNECT(name, C, cI3)

#define TTL_7411_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_7411_DIP, name)


#define TTL_7416_GATE(name)                                                   \
		NET_REGISTER_DEV(TTL_7416_GATE, name)

#define TTL_7416_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL7416_DIP, name)


#define TTL_7420_GATE(name)                                                   \
		NET_REGISTER_DEV(TTL_7420_GATE, name)

#define TTL_7420_NAND(name, cI1, cI2, cI3, cI4)                               \
		NET_REGISTER_DEV(TTL_7420_NAND, name)                                 \
		NET_CONNECT(name, A, cI1)                                             \
		NET_CONNECT(name, B, cI2)                                             \
		NET_CONNECT(name, C, cI3)                                             \
		NET_CONNECT(name, D, cI4)

#define TTL_7420_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_7420_DIP, name)


#define TTL_7425_GATE(name)                                                   \
		NET_REGISTER_DEV(TTL_7425_GATE, name)

#define TTL_7425_NOR(name, cI1, cI2, cI3, cI4)                                \
		NET_REGISTER_DEV(TTL_7425_NOR, name)                                  \
		NET_CONNECT(name, A, cI1)                                             \
		NET_CONNECT(name, B, cI2)                                             \
		NET_CONNECT(name, C, cI3)                                             \
		NET_CONNECT(name, D, cI4)

#define TTL_7425_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_7425_DIP, name)


#define TTL_7427_GATE(name)                                                   \
		NET_REGISTER_DEV(TTL_7427_GATE, name)

#define TTL_7427_NOR(name, cI1, cI2, cI3)                                     \
		NET_REGISTER_DEV(TTL_7427_NOR, name)                                  \
		NET_CONNECT(name, A, cI1)                                             \
		NET_CONNECT(name, B, cI2)                                             \
		NET_CONNECT(name, C, cI3)

#define TTL_7427_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_7427_DIP, name)


#define TTL_7430_GATE(name)                                                   \
		NET_REGISTER_DEV(TTL_7430_GATE, name)

#define TTL_7430_NAND(name, cI1, cI2, cI3, cI4, cI5, cI6, cI7, cI8)           \
		NET_REGISTER_DEV(TTL_7430_NAND, name)                                 \
		NET_CONNECT(name, A, cI1)                                             \
		NET_CONNECT(name, B, cI2)                                             \
		NET_CONNECT(name, C, cI3)                                             \
		NET_CONNECT(name, D, cI4)                                             \
		NET_CONNECT(name, E, cI5)                                             \
		NET_CONNECT(name, F, cI6)                                             \
		NET_CONNECT(name, G, cI7)                                             \
		NET_CONNECT(name, H, cI8)

#define TTL_7430_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_7430_DIP, name)


#define TTL_7432_GATE(name)                                                   \
		NET_REGISTER_DEV(TTL_7432_OR, name)

#define TTL_7432_OR(name, cI1, cI2)                                           \
		NET_REGISTER_DEV(TTL_7432_OR, name)                                   \
		NET_CONNECT(name, A, cI1)                                             \
		NET_CONNECT(name, B, cI2)

#define TTL_7432_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_7432_DIP, name)


#define TTL_7437_GATE(name)                                                   \
		NET_REGISTER_DEV(TTL_7437_GATE, name)

#define TTL_7437_NAND(name, cA, cB)                                           \
		NET_REGISTER_DEV(TTL_7437_NAND, name)                                  \
		NET_CONNECT(name, A, cA)                                               \
		NET_CONNECT(name, B, cB)

#define TTL_7437_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_7437_DIP, name)


#define TTL_7486_GATE(name)                                                   \
		NET_REGISTER_DEV(TTL_7486_GATE, name)

#define TTL_7486_XOR(name, cA, cB)                                            \
		NET_REGISTER_DEV(TTL_7486_XOR, name)                                  \
		NET_CONNECT(name, A, cA)                                              \
		NET_CONNECT(name, B, cB)

#define TTL_7486_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_7486_DIP, name)

/* ----------------------------------------------------------------------------
 *  External declarations
 * ---------------------------------------------------------------------------*/

NETLIST_EXTERNAL(TTL74XX_lib)

#endif

#endif
