// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_roms.h
 *
 */

#ifndef NLD_ROMS_H_
#define NLD_ROMS_H_

#include "../nl_setup.h"

// PROM_82S126(name, cCE1Q, cCE2Q, cA0, cA1, cA2, cA3, cA4, cA5, cA6, cA7)
#define PROM_82S126(...) \
		NET_REGISTER_DEVEXT(PROM_82S126, __VA_ARGS__)

// PROM_74S287(name, cCE1Q, cCE2Q, cA0, cA1, cA2, cA3, cA4, cA5, cA6, cA7)
#define PROM_74S287(...) \
		NET_REGISTER_DEVEXT(PROM_74S287, __VA_ARGS__)

// PROM_82S123(name, cCEQ, cA0, cA1, cA2, cA3, cA4)
#define PROM_82S123(...) \
		NET_REGISTER_DEVEXT(PROM_82S123, __VA_ARGS__)

// EPROM_2716(name, cGQ, cEPQ, cA0, cA1, cA2, cA3, cA4, cA5, cA6, cA7, cA8, cA9, cA10)
#define EPROM_2716(...) \
		NET_REGISTER_DEVEXT(EPROM_2716, __VA_ARGS__)

// PROM_MK28000(name, cOE1, cOE2, cAR, cA0, cA1, cA2, cA3, cA4, cA5, cA6, cA7, cA8, cA9, cA10, cA11)
#define PROM_MK28000(...) \
		NET_REGISTER_DEVEXT(PROM_MK28000, __VA_ARGS__)

#endif /* NLD_ROMS_H_ */
