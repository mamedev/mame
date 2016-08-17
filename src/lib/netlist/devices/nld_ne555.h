// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_NE555.h
 *
 *  NE555: PRECISION TIMERS
 *
 *          +--------+
 *      GND |1  ++  8| VCC
 *     TRIG |2      7| DISCH
 *      OUT |3      6| THRES
 *    RESET |4      5| CONT
 *          +--------+
 *
 *  Naming conventions follow Texas Instruments datasheet
 *
 */

#ifndef NLD_NE555_H_
#define NLD_NE555_H_

#include "nl_setup.h"

#define NE555(name)                                                             \
		NET_REGISTER_DEV(NE555, name)

#define NE555_DIP(name)                                                         \
		NET_REGISTER_DEV(NE555_DIP, name)

#endif /* NLD_NE555_H_ */
