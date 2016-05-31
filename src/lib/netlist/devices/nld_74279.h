// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74279.h
 *
 *  DM74279: Quad S-R Latch
 *
 *          +--------------+
 *       1R |1     ++    16| VCC
 *      1S1 |2           15| 4S
 *      1S2 |3           14| 4R
 *       1Q |4    74279  13| 4Q
 *       2R |5           12| 3S2
 *       2S |6           11| 3S1
 *       2Q |7           10| 3R
 *      GND |8            9| 3Q
 *          +--------------+
 *                  ___
 *
 *          +---+---+---++---+
 *          |S1 |S2 | R || Q |
 *          +===+===+===++===+
 *          | 0 | 0 | 0 || 1 |
 *          | 0 | 1 | 1 || 1 |
 *          | 1 | 0 | 1 || 1 |
 *          | 1 | 1 | 0 || 0 |
 *          | 1 | 1 | 1 ||QP |
 *          +---+---+---++---+
 *
 *  QP: Previous Q
 *
 *  Naming conventions follow Fairchild Semiconductor datasheet
 *
 */

#ifndef NLD_74279_H_
#define NLD_74279_H_

#define TTL_74279_DIP(name)                                                         \
		NET_REGISTER_DEV(TTL_74279_DIP, name)

#endif /* NLD_74279_H_ */
