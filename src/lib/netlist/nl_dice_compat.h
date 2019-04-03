// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nl_dice_compat.h
 *
 * The following script will convert a circuit using dice syntax into netlist
 * syntax. It's not fail proof, but eases the manual work involved significantly.

sed -e 's/#define \(.*\)"\(.*\)"[ \t]*,[ \t]*\(.*\)/NET_ALIAS(\1,\2.\3)/' src/mame/drivers/nl_breakout.c   \
| sed -e 's/^[ \t]*$/NL_EMPTY /' \
| cpp -I src/emu -I src/osd/ -DNL_CONVERT_CPP -P -CC - \
| sed -e 's/\(TTL_.*\)("\(.*\)")/\1(\2)/' \
| sed -e 's/CONNECTION(\(.*\),[ \t]*"\(.*\)", \(.*\))/NET_C(\1, \2.\3)/' \
| sed -e 's/CONNECTION("\(.*\)",[ \t]*\(.*\), \(.*\))/NET_C(\1.\2, \3)/' \
| sed -e 's/NET_C("\(.*\)", \(.*\), \(.*\))/NET_C(\1.\2, \3)/' \
| sed -e 's/) RES(/)\n    RES(/g' \
| sed -e 's/) CAP(/)\n    CAP(/g' \
| sed -e 's/) NET_C(/)\n    NET_C(/g' \
| sed -e 's/NL_EMPTY//' \

 *
 */

#ifndef NL_DICE_COMPAT_H_
#define NL_DICE_COMPAT_H_

#ifndef NL_CONVERT_CPP
#include "devices/net_lib.h"
#include "analog/nld_twoterm.h"

#include <cmath>

#endif


/* --------------------------------------------------------------------
 * Compatibility macros for DICE netlists ...
 * -------------------------------------------------------------------- */

/*
 * define NETLIST_DEVELOPMENT in IDEs before including this header file
 * to get compile time errors on unknown devices. This should only be
 * a temporary support and not be used in commits.
 */

#define NETLIB_NAME_STR_S(s) # s
#define NETLIB_NAME_STR(chip) NETLIB_NAME_STR_S(nld_ ## chip)

#ifndef NL_CONVERT_CPP
#ifndef NETLIST_DEVELOPMENT
#define NETLIST_DEVELOPMENT 0
#endif
#if (NETLIST_DEVELOPMENT)
#define CHIP(n, t) setup.register_dev( palloc(netlist::devices::nld_ ## t ## dip), n);
#else
#define CHIP(n, t) setup.register_dev(NETLIB_NAME_STR_S(TTL_ ## t ## _DIP), n);
//#define CHIP(n, t) TTL_ ## t ## _DIP(n)
#endif

#define CONNECTION( ... ) CONNECTIONY( CONNECTIONX( __VA_ARGS__ ) )
#define CONNECTIONY(a) a
#define CONNECTIONX(a, b, c, d) setup.register_link(a "." # b, c "." # d);
#define NET_CSTR(a, b) setup.register_link( a, b);

#define OHM(x) (x)
#define K_OHM(x) ((x) * 1000.0)
#define M_OHM(x) ((x) * 1.0e6)
#define U_FARAD(x) ((x) * 1.0e-6)
#define N_FARAD(x) ((x) * 1.0e-9)
#define P_FARAD(x) ((x) * 1.0e-12)

struct Mono555Desc
{
public:
		nl_double r, c;

		Mono555Desc(nl_double res, nl_double cap) : r(res), c(cap) { }
};

struct Astable555Desc
{
public:
		nl_double r1, r2, c;

		Astable555Desc(nl_double res1, nl_double res2, nl_double cap) : r1(res1), r2(res2), c(cap) { }
};

struct Mono9602Desc
{
public:
		nl_double r1, c1, r2, c2;

		Mono9602Desc(nl_double res1, nl_double cap1, nl_double res2, nl_double cap2)
		: r1(res1), c1(cap1), r2(res2), c2(cap2) { }
};

struct SeriesRCDesc
{
public:
		nl_double r, c;

		SeriesRCDesc(nl_double res, nl_double cap) : r(res), c(cap) { }
};

struct CapacitorDesc : public SeriesRCDesc
{
public:
	CapacitorDesc(nl_double cap) : SeriesRCDesc(0.0, cap) { }
};

#else
#define CHIP(n, t) TTL_ ## t ## _DIP(n)

#define OHM(x) x
#define K_OHM(x) RES_K(x)
#define M_OHM(x) RES_M(x)
#define U_FARAD(x) CAP_U(x)
#define N_FARAD(x) CAP_N(x)
#define P_FARAD(x) CAP_P(x)

#endif


#define CIRCUIT_LAYOUT(x) NETLIST_START(x)
#define CIRCUIT_LAYOUT_END NETLIST_END()

#define CHIP_555_Mono(name,  pdesc)   \
	NE555_DIP(name) \
	NET_C(name.6, name.7) \
	RES(name ## _R, (pdesc)->r) \
	CAP(name ## _C, (pdesc)->c) \
	NET_C(name.6, name ## _R.1) \
	NET_C(name.6, name ## _C.1) \
	NET_C(name ## _R.2, V5) \
	NET_C(name ## _C.2, GND) \
	NET_C(name.8, V5) \
	NET_C(name.1, GND)

#define CHIP_555_Astable(name,  pdesc)   \
	NE555_DIP(name) \
	RES(name ## _R1, (pdesc)->r1) \
	RES(name ## _R2, (pdesc)->r2) \
	CAP(name ## _C, (pdesc)->c) \
	NET_C(name.7, name ## _R1.1) \
	NET_C(name.7, name ## _R2.1) \
	NET_C(name.6, name ## _R2.2) \
	NET_C(name.6, name ## _C.1) \
	NET_C(name.2, name ## _C.1) \
	NET_C(name ## _R1.2, V5) \
	NET_C(name ## _C.2, GND) \
	NET_C(name.8, V5) \
	NET_C(name.1, GND)

#define CHIP_9602_Mono(name,  pdesc)   \
	CHIP(# name, 9602) \
	NET_C(VCC, name.16)        \
	NET_C(GND, name.8)         \
	RES(name ## _R1, (pdesc)->r1) \
	CAP(name ## _C1, (pdesc)->c1) \
	RES(name ## _R2, (pdesc)->r2) \
	NET_C(name.1, name ## _C1.1) \
	NET_C(name.2, name ## _C1.2) \
	NET_C(name.2, name ## _R1.2) \
	NET_C(VCC,     name ## _R1.1) \
	if (((pdesc)->c2)>1e-15) { \
	CAP(name ## _C2, (pdesc)->c2) \
	NET_C(name.15, name ## _C2.1) \
	NET_C(name.14, name ## _C2.2) }\
	NET_C(name.14, name ## _R2.2) \
	NET_C(VCC,     name ## _R2.1)
#define CHIP_SERIES_RC(name,  pdesc)   \
	RES(name ## _R, (pdesc)->r) \
	CAP(name ## _C, (pdesc)->c) \
	NET_C(name ## _R.1, name ## _C.2) \
	ALIAS(name.3, name ## _R.1) \
	ALIAS(name.2, name ## _R.2) \
	ALIAS(name.1, name ## _C.1)

#define CHIP_INPUT_ACTIVE_LOW(name)   \
	SWITCH2(name ## _SW) \
	NET_C(name ## _SW.1, V5) \
	NET_C(name ## _SW.2, GND) \
	ALIAS(name.1, name ## _SW.Q)

#define CHIP_INPUT_ACTIVE_HIGH(name)   \
	SWITCH2(name ## _SW) \
	NET_C(name ## _SW.2, V5) \
	NET_C(name ## _SW.1, GND) \
	ALIAS(name.1, name ## _SW.Q)

#define CHIP_LATCH(name)   \
	NETDEV_RSFF(name) \
	ALIAS(name.1, name.R) \
	ALIAS(name.2, name.S) \
	ALIAS(name.3, name.QQ)

/* FIXME: Alternative implementation using capacitor.
 *        This is a transitional implementation
 */

inline int CAPACITOR_tc_hl(const double c, const double r)
{
	/*
	 * Vt = (VH-VL)*exp(-t/RC)
	 * ln(Vt/(VH-VL))*RC = -t
	 */
	static const double TIME_CONSTANT = -std::log(0.8 / (4.0-0.1));
	int ret = (int) (TIME_CONSTANT * (1.0 + r) * c * 1e9);
	return ret;
}

inline int CAPACITOR_tc_lh(const double c, const double r)
{
	/*
	 * Vt = (VH-VL)*(1-exp(-t/RC))
	 * -t=ln(1-Vt/(VH-VL))*RC
	 */
	static const double TIME_CONSTANT = -std::log(1.0 - 2.0 / (4.0-0.1));
	int ret = (int) (TIME_CONSTANT * (130.0 + r) * c * 1e9);
	return ret;
}

#if 1
#define CHIP_CAPACITOR(name, pdesc) \
	NETDEV_DELAY(name) \
	NETDEV_PARAMI(name, L_TO_H, CAPACITOR_tc_lh((pdesc)->c, (pdesc)->r)) \
	NETDEV_PARAMI(name, H_TO_L, CAPACITOR_tc_hl((pdesc)->c, (pdesc)->r))
#elif 1
// slow, very slow
#define CHIP_CAPACITOR(name, pdesc) \
	CAP(name ## _C, (pdesc)->c) \
	ALIAS(name.1, name ## _C.1 ) \
	ALIAS(name.2, name ## _C.1) \
	NET_C(GND, name ## _C.2)
#else
// fast, might work
#define CHIP_CAPACITOR(name, pdesc) \
	RES(name ## _C, RES_K(1000)) \
	ALIAS(name.1, name ## _C.1 ) \
	ALIAS(name.2, name ## _C.1) \
	NET_C(GND, name ## _C.2)
#endif

#endif /* NL_DICE_COMPAT_H_ */
