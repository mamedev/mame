// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, Derrick Renaud, Curt Coder
#ifndef MAME_MACHINE_RESCAP_H
#define MAME_MACHINE_RESCAP_H

// Little helpers for magnitude conversions
constexpr double RES_R(double res) { return res; }
constexpr double RES_K(double res) { return res * 1e3; }
constexpr double RES_M(double res) { return res * 1e6; }
#define RES_INF    (-1)
constexpr double CAP_U(double cap) { return cap * 1e-6; }
constexpr double CAP_N(double cap) { return cap * 1e-9; }
constexpr double CAP_P(double cap) { return cap * 1e-12; }
constexpr double IND_U(double ind) { return ind * 1e-6; }
constexpr double IND_N(double ind) { return ind * 1e-9; }
constexpr double IND_P(double ind) { return ind * 1e-12; }

//  vin --/\r1/\-- out --/\r2/\-- gnd
constexpr double RES_VOLTAGE_DIVIDER(double r1, double r2) { return r2 / (r1 + r2); }

#define RES_2_PARALLEL(r1, r2)          (((r1) * (r2)) / ((r1) + (r2)))
#define RES_3_PARALLEL(r1, r2, r3)      (1.0 / (1.0 / (r1) + 1.0 / (r2) + 1.0 / (r3)))
#define RES_4_PARALLEL(r1, r2, r3, r4)  (1.0 / (1.0 / (r1) + 1.0 / (r2) + 1.0 / (r3) + 1.0 / (r4)))
#define RES_5_PARALLEL(r1, r2, r3, r4, r5)  (1.0 / (1.0 / (r1) + 1.0 / (r2) + 1.0 / (r3) + 1.0 / (r4) + 1.0 / (r5)))
#define RES_6_PARALLEL(r1, r2, r3, r4, r5, r6)  (1.0 / (1.0 / (r1) + 1.0 / (r2) + 1.0 / (r3) + 1.0 / (r4) + 1.0 / (r5) + 1.0 / (r6)))

#define RES_2_SERIAL(r1,r2)             ((r1)+(r2))

// macro for the RC time constant on a 74LS123 with C > 1000pF
// R is in ohms, C is in farads
constexpr double TIME_OF_74LS123(double r, double c) { return 0.45 * r * c; }

// macros for the RC time constant on a 555 timer IC
// R is in ohms, C is in farads
constexpr attoseconds_t PERIOD_OF_555_MONOSTABLE_NSEC(double r, double c)          { return attoseconds_t(1100000000 * r * c); }
constexpr attoseconds_t PERIOD_OF_555_ASTABLE_NSEC(double r1, double r2, double c) { return attoseconds_t( 693000000 * (r1 + 2.0 * r2) * c); }
constexpr attotime PERIOD_OF_555_MONOSTABLE(double r, double c)                    { return attotime::from_nsec(PERIOD_OF_555_MONOSTABLE_NSEC(r, c)); }
constexpr attotime PERIOD_OF_555_ASTABLE(double r1, double r2, double c)           { return attotime::from_nsec(PERIOD_OF_555_ASTABLE_NSEC(r1, r2, c)); }

#endif // MAME_MACHINE_RESCAP_H
