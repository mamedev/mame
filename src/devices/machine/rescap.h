// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_MACHINE_RESCAP_H
#define MAME_MACHINE_RESCAP_H

// Little helpers for magnitude conversions
#define RES_R(res) ((double)(res))
#define RES_K(res) ((double)(res) * 1e3)
#define RES_M(res) ((double)(res) * 1e6)
#define RES_INF    (-1)
#define CAP_U(cap) ((double)(cap) * 1e-6)
#define CAP_N(cap) ((double)(cap) * 1e-9)
#define CAP_P(cap) ((double)(cap) * 1e-12)
#define IND_U(ind) ((double)(ind) * 1e-6)
#define IND_N(ind) ((double)(ind) * 1e-9)
#define IND_P(ind) ((double)(ind) * 1e-12)

//  vin --/\r1/\-- out --/\r2/\-- gnd
#define RES_VOLTAGE_DIVIDER(r1, r2)     ((double)(r2) / ((double)(r1) + (double)(r2)))

#define RES_2_PARALLEL(r1, r2)          (((r1) * (r2)) / ((r1) + (r2)))
#define RES_3_PARALLEL(r1, r2, r3)      (1.0 / (1.0 / (r1) + 1.0 / (r2) + 1.0 / (r3)))
#define RES_4_PARALLEL(r1, r2, r3, r4)  (1.0 / (1.0 / (r1) + 1.0 / (r2) + 1.0 / (r3) + 1.0 / (r4)))
#define RES_5_PARALLEL(r1, r2, r3, r4, r5)  (1.0 / (1.0 / (r1) + 1.0 / (r2) + 1.0 / (r3) + 1.0 / (r4) + 1.0 / (r5)))
#define RES_6_PARALLEL(r1, r2, r3, r4, r5, r6)  (1.0 / (1.0 / (r1) + 1.0 / (r2) + 1.0 / (r3) + 1.0 / (r4) + 1.0 / (r5) + 1.0 / (r6)))

#define RES_2_SERIAL(r1,r2)             ((r1)+(r2))

// macro for the RC time constant on a 74LS123 with C > 1000pF
// R is in ohms, C is in farads
#define TIME_OF_74LS123(r,c)            (0.45 * (double)(r) * (double)(c))

// macros for the RC time constant on a 555 timer IC
// R is in ohms, C is in farads
#define PERIOD_OF_555_MONOSTABLE_NSEC(r,c)  ((attoseconds_t)(1100000000 * (double)(r) * (double)(c)))
#define PERIOD_OF_555_ASTABLE_NSEC(r1,r2,c) ((attoseconds_t)( 693000000 * ((double)(r1) + 2.0 * (double)(r2)) * (double)(c)))
#define PERIOD_OF_555_MONOSTABLE(r,c)       attotime::from_nsec(PERIOD_OF_555_MONOSTABLE_NSEC(r,c))
#define PERIOD_OF_555_ASTABLE(r1,r2,c)      attotime::from_nsec(PERIOD_OF_555_ASTABLE_NSEC(r1,r2,c))

#endif // MAME_MACHINE_RESCAP_H
