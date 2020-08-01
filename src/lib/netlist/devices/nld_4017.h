// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_4017.h
 *
 *  CD4017: Decade Counter/Divider with 10 Decoded Outputs
 *
 *          +--------------+
 *       Q5 |1     ++    16| VDD
 *       Q1 |2           15| RESET
 *       Q0 |3           14| CLOCK
 *       Q2 |4    4017   13| CLOCK ENABLE
 *       Q6 |5           12| CARRY OUT
 *       Q7 |6           11| Q9
 *       Q3 |7           10| Q4
 *      VSS |8            9| Q8
 *          +--------------+
 *
 *
 *  CD4022: Divide-by-8 Counter/Divider with 8 Decoded Outputs
 *
 *          +--------------+
 *       Q1 |1     ++    16| VDD
 *       Q0 |2           15| RESET
 *       Q2 |3           14| CLOCK
 *       Q5 |4    4022   13| CLOCK ENABLE
 *       Q6 |5           12| CARRY OUT
 *       NC |6           11| Q4
 *       Q3 |7           10| Q7
 *      VSS |8            9| NC
 *          +--------------+
 *
 *  Naming conventions follow Fairchild datasheet
 *
 *  FIXME: Timing depends on VDD-VSS
 *         This needs a cmos d-a/a-d proxy implementation.
 *
 */

#ifndef NLD_4017_H_
#define NLD_4017_H_

#include "netlist/nl_setup.h"

#define CD4017(name)                                                            \
		NET_REGISTER_DEV(CD4017, name)

#define CD4022(name)                                                            \
		NET_REGISTER_DEV(CD4022, name)

#endif /* NLD_4017_H_ */
