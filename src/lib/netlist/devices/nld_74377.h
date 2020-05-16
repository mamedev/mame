// license:GPL-2.0+
// copyright-holders:Couriersud,Aaron Giles
/*
 * nld_74377.h
 *
 *  DM74377: Octal D Flip-Flop With Enable
 *
 *          +--------------+
 *       /E |1     ++    20| VCC
 *       Q0 |2           19| Q7
 *       D0 |3           18| D7
 *       D1 |4   74377   17| D6
 *       Q1 |5           16| Q6
 *       Q2 |6           15| Q5
 *       D2 |7           14| D5
 *       D3 |8           13| D4
 *       Q3 |9           12| Q4
 *      GND |10          11| CP
 *          +--------------+
 *
 *  DM74378: Hex D Flip-Flop With Enable
 *
 *          +--------------+
 *       /E |1     ++    16| VCC
 *       Q0 |2           15| Q5
 *       D0 |3           14| D5
 *       D1 |4   74378   13| D4
 *       Q1 |5           12| Q4
 *       D2 |6           11| D3
 *       Q2 |7           10| Q3
 *      GND |8            9| CP
 *          +--------------+
 *
 *  DM74379: 4-bit D Flip-Flop With Enable
 *
 *          +--------------+
 *       /E |1     ++    16| VCC
 *       Q0 |2           15| Q3
 *      /Q0 |3           14| /Q3
 *       D0 |4   74379   13| D3
 *       D1 |5           12| D2
 *      /Q1 |6           11| /Q2
 *       Q1 |7           10| Q2
 *      GND |8            9| CP
 *          +--------------+
 *
 *  Naming conventions follow Motorola datasheet
 *
 */

#ifndef NLD_74377_H_
#define NLD_74377_H_

#include "netlist/nl_setup.h"


#define TTL_74377_GATE(name)                                                   \
		NET_REGISTER_DEV(TTL_74377_GATE, name)


#endif /* NLD_74377_H_ */
