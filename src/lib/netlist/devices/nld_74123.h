// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74123.h
 *
 *  74123: Dual Retriggerable One-Shot with Clear and Complementary Outputs
 *
 *           +--------------+
 *        A1 |1     ++    16| VCC
 *        B1 |2           15| RC1
 *      CLR1 |3           14| C1
 *       Q1Q |4   74123   13| Q1
 *        Q2 |5           12| Q2Q
 *        C2 |6           11| CLR2
 *       RC2 |7           10| B2
 *       GND |8            9| A2
 *           +--------------+
 *
 *  Naming conventions follow Fairchild Semiconductor datasheet
 *
 *  DM9602: Dual Retriggerable, Resettable One Shots
 *
 *           +--------------+
 *        C1 |1     ++    16| VCC
 *       RC1 |2           15| C2
 *      CLR1 |3           14| RC2
 *        B1 |4    9602   13| CLR2
 *        A1 |5           12| B2
 *        Q1 |6           11| A2
 *       Q1Q |7           10| Q2
 *       GND |8            9| Q2Q
 *           +--------------+
 *
 *  CD4538: Dual Retriggerable, Resettable One Shots
 *
 *           +--------------+
 *        C1 |1     ++    16| VCC
 *       RC1 |2           15| C2
 *      CLR1 |3           14| RC2
 *        A1 |4    4538   13| CLR2
 *        B1 |5           12| A2
 *        Q1 |6           11| B2
 *       Q1Q |7           10| Q2
 *       GND |8            9| Q2Q
 *           +--------------+
 *
 */

#ifndef NLD_74123_H_
#define NLD_74123_H_

#include "nl_setup.h"

#define TTL_74123(name)                                                         \
		NET_REGISTER_DEV(TTL_74123, name)

#define TTL_74123_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_74123_DIP, name)

/* The 9602 is very similar to the 123. Input triggering is slightly different
 * The 9602 uses an OR gate instead of an AND gate.
 */

#define TTL_9602_DIP(name)                                                      \
		NET_REGISTER_DEV(TTL_9602_DIP, name)

/*
 * The CD4538 is pretty similar to the 9602
 */

#define CD4538_DIP(name)                                                        \
		NET_REGISTER_DEV(CD4538_DIP, name)

#endif /* NLD_74123_H_ */
