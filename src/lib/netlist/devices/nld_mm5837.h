// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_MM5837.h
 *
 *  MM5837: Digital noise source
 *
 *          +--------+
 *      VDD |1  ++  8| NC
 *      VGG |2      7| NC
 *      OUT |3      6| NC
 *      VSS |4      5| NC
 *          +--------+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_MM5837_H_
#define NLD_MM5837_H_

#include "nl_setup.h"

#define MM5837_DIP(name)                                                        \
		NET_REGISTER_DEV(MM5837_DIP, name)

#endif /* NLD_MM5837_H_ */
