// license:CC0
// copyright-holders:Couriersud

#ifndef NLM_TTL74XX_H_
#define NLM_TTL74XX_H_

///
/// \file nlm_ttl74xx.h
///

#include "../nl_setup.h"

#ifndef __PLIB_PREPROCESSOR__

/* ----------------------------------------------------------------------------
 *  Netlist Macros
 * ---------------------------------------------------------------------------*/

#if !NL_AUTO_DEVICES

#define TTL_7400_NAND(...)                                                     \
	NET_REGISTER_DEVEXT(TTL_7400_NAND, __VA_ARGS__)

#define TTL_7400_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7400_DIP, name)


#define TTL_7402_NOR(...)                                                     \
	NET_REGISTER_DEVEXT(TTL_7402_NOR, __VA_ARGS__)

#define TTL_7402_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7402_DIP, name)

#define TTL_7404_INVERT(...)                                                     \
	NET_REGISTER_DEVEXT(TTL_7404_INVERT, __VA_ARGS__)

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


#define TTL_7408_AND(...)                                                      \
	NET_REGISTER_DEVEXT(TTL_7408_AND, __VA_ARGS__)

#define TTL_7408_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7408_DIP, name)


#define TTL_7410_NAND(...)                                                     \
	NET_REGISTER_DEVEXT(TTL_7410_NAND, __VA_ARGS__)

#define TTL_7410_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7410_DIP, name)


#define TTL_7411_AND(...)                                                      \
	NET_REGISTER_DEVEXT(TTL_7411_AND, __VA_ARGS__)

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


#define TTL_7417_GATE(name)                                                    \
		NET_REGISTER_DEV(TTL_7417_GATE, name)

#define TTL_7417_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7417_DIP, name)


#define TTL_7420_NAND(...)                                                     \
	NET_REGISTER_DEVEXT(TTL_7420_NAND, __VA_ARGS__)

#define TTL_7420_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7420_DIP, name)


#define TTL_7421_AND(...)                                                      \
	NET_REGISTER_DEVEXT(TTL_7421_AND, __VA_ARGS__)

#define TTL_7421_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7421_DIP, name)


#define TTL_7425_NOR(...)                                                      \
	NET_REGISTER_DEVEXT(TTL_7425_NOR, __VA_ARGS__)

#define TTL_7425_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7425_DIP, name)


#define TTL_7427_NOR(...)                                                      \
	NET_REGISTER_DEVEXT(TTL_7427_NOR, __VA_ARGS__)

#define TTL_7427_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7427_DIP, name)


#define TTL_7430_NAND(...)                                                      \
	NET_REGISTER_DEVEXT(TTL_7430_NAND, __VA_ARGS__)

#define TTL_7430_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7430_DIP, name)


#define TTL_7432_OR(...)                                                       \
	NET_REGISTER_DEVEXT(TTL_7432_OR, __VA_ARGS__)

#define TTL_7432_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7432_DIP, name)


#define TTL_7437_NAND(...)                                                     \
	NET_REGISTER_DEVEXT(TTL_7437_NAND, __VA_ARGS__)

#define TTL_7437_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7437_DIP, name)

#define TTL_7438_NAND(...)                                                     \
	NET_REGISTER_DEVEXT(TTL_7438_NAND, __VA_ARGS__)

#define TTL_7438_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7438_DIP, name)


#define TTL_7442_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7442_DIP, name)

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

#define TTL_7486_XOR(...)                                                      \
	NET_REGISTER_DEVEXT(TTL_7486_XOR, __VA_ARGS__)

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

#define TTL_74139_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74139_DIP, name)

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

#define TTL_74165_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_74165_DIP, name)

#define TTL_74166_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_74166_DIP, name)

#define TTL_74174_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_74174_DIP, name)

#define TTL_74175_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_74175_DIP, name)

#define TTL_74192_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_74192_DIP, name)

#define TTL_74193_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_74193_DIP, name)

#define TTL_74194_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_74194_DIP, name)

#define TTL_74260_NOR(...)                                                     \
	NET_REGISTER_DEVEXT(TTL_74260_NOR, __VA_ARGS__)

#define TTL_74260_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74260_DIP, name)

#define TTL_74279_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74279_DIP, name)

#define TTL_74290_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74290_DIP, name)

#define TTL_74293_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74293_DIP, name)

#define TTL_74365_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74365_DIP, name)

#define TTL_74377_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74377_DIP, name)

#define TTL_74378_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74378_DIP, name)

#define TTL_74379_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74379_DIP, name)

#define TTL_74393_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74393_DIP, name)

#define TTL_9312(...)                                                          \
		NET_REGISTER_DEVEXT(TTL_9312, __VA_ARGS__)

#define TTL_9312_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_9312_DIP, name)

#define TTL_9314_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_9314_DIP, name)

#define TTL_9310_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_9310_DIP, name)

#define TTL_9316_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_9316_DIP, name)

#define TTL_9321_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_9321_DIP, name)

#define TTL_9322_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_9322_DIP, name)

#define TTL_9334_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_9334_DIP, name)

#define TTL_8277_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_8277_DIP, name)

#define TTL_AM2847_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_AM2847_DIP, name)

#endif // NL_AUTO_DEVICES

/* ----------------------------------------------------------------------------
 *  External declarations
 * ---------------------------------------------------------------------------*/

// moved to net_lib.h

#endif // __PLIB_PREPROCESSOR__


#endif // NLM_TTL74XX
