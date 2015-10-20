// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nl_dice_compat.h
 *
 * The follwoing script will convert a circuit using dice syntax into netlist
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
#endif

/* --------------------------------------------------------------------
 * Compatibility macros for DICE netlists ...
 * -------------------------------------------------------------------- */

/*
 * define NETLIST_DEVELOPMENT in IDEs before including this header file
 * to get compile time errors on unknown devices. This should only be
 * a temporary support and not be used in commits.
 */

#ifndef NL_CONVERT_CPP
#ifndef NETLIST_DEVELOPMENT
#define NETLIST_DEVELOPMENT 0
#endif
#if (NETLIST_DEVELOPMENT)
#define CHIP(_n, _t) setup.register_dev( palloc(netlist::devices::nld_ ## _t ## _dip), _n);
#else
#define CHIP(_n, _t) setup.register_dev(NETLIB_NAME_STR_S(TTL_ ## _t ## _DIP), _n);
//#define CHIP(_n, _t) TTL_ ## _t ## _DIP(_n)
#endif

#define CONNECTION( ... ) CONNECTIONY( CONNECTIONX( __VA_ARGS__ ) )
#define CONNECTIONY(_a) _a
#define CONNECTIONX(_a, _b, _c, _d) setup.register_link(_a "." # _b, _c "." # _d);
#define NET_CSTR(_a, _b) setup.register_link( _a, _b);

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
#define CHIP(_n, _t) TTL_ ## _t ## _DIP(_n)

#define OHM(x) x
#define K_OHM(x) RES_K(X)
#define M_OHM(x) RES_M(X)
#define U_FARAD(x) CAP_U(x)
#define N_FARAD(x) CAP_N(x)
#define P_FARAD(x) CAP_P(x)

#endif


#define CIRCUIT_LAYOUT(x) NETLIST_START(x)
#define CIRCUIT_LAYOUT_END NETLIST_END()

#define CHIP_555_Mono(_name,  _pdesc)   \
	NE555_DIP(_name) \
	NET_C(_name.6, _name.7) \
	RES(_name ## _R, (_pdesc)->r) \
	CAP(_name ## _C, (_pdesc)->c) \
	NET_C(_name.6, _name ## _R.1) \
	NET_C(_name.6, _name ## _C.1) \
	NET_C(_name ## _R.2, V5) \
	NET_CSTR(# _name "_C.2", "GND") \
	NET_C(_name.8, V5) \
	NET_CSTR(# _name ".1", "GND")

#define CHIP_555_Astable(_name,  _pdesc)   \
	NE555_DIP(_name) \
	RES(_name ## _R1, (_pdesc)->r1) \
	RES(_name ## _R2, (_pdesc)->r2) \
	CAP(_name ## _C, (_pdesc)->c) \
	NET_C(_name.7, _name ## _R1.1) \
	NET_C(_name.7, _name ## _R2.1) \
	NET_C(_name.6, _name ## _R2.2) \
	NET_C(_name.6, _name ## _C.1) \
	NET_C(_name.2, _name ## _C.1) \
	NET_C(_name ## _R1.2, V5) \
	NET_CSTR(# _name "_C.2", "GND") \
	NET_C(_name.8, V5) \
	NET_CSTR(# _name ".1", "GND")

#define CHIP_9602_Mono(_name,  _pdesc)   \
	CHIP(# _name, 9602) \
	NET_C(VCC, _name.16)        \
	NET_C(GND, _name.8)         \
	RES(_name ## _R1, (_pdesc)->r1) \
	CAP(_name ## _C1, (_pdesc)->c1) \
	RES(_name ## _R2, (_pdesc)->r2) \
	NET_C(_name.1, _name ## _C1.1) \
	NET_C(_name.2, _name ## _C1.2) \
	NET_C(_name.2, _name ## _R1.2) \
	NET_C(VCC,     _name ## _R1.1) \
	if (((_pdesc)->c2)>1e-15) { \
	CAP(_name ## _C2, (_pdesc)->c2) \
	NET_C(_name.15, _name ## _C2.1) \
	NET_C(_name.14, _name ## _C2.2) }\
	NET_C(_name.14, _name ## _R2.2) \
	NET_C(VCC,     _name ## _R2.1)
#define CHIP_SERIES_RC(_name,  _pdesc)   \
	RES(_name ## _R, (_pdesc)->r) \
	CAP(_name ## _C, (_pdesc)->c) \
	NET_C(_name ## _R.1, _name ## _C.2) \
	ALIAS(_name.3, _name ## _R.1) \
	ALIAS(_name.2, _name ## _R.2) \
	ALIAS(_name.1, _name ## _C.1)

#define CHIP_INPUT_ACTIVE_LOW(_name)   \
	SWITCH2(_name ## _SW) \
	NET_C(_name ## _SW.1, V5) \
	NET_CSTR(# _name "_SW.2", "GND") \
	ALIAS(_name.1, _name ## _SW.Q)

#define CHIP_INPUT_ACTIVE_HIGH(_name)   \
	SWITCH2(_name ## _SW) \
	NET_C(_name ## _SW.2, V5) \
	NET_CSTR(# _name "_SW.1", "GND") \
	ALIAS(_name.1, _name ## _SW.Q)

#define CHIP_LATCH(_name)   \
	NETDEV_RSFF(_name) \
	ALIAS(_name.1, _name.R) \
	ALIAS(_name.2, _name.S) \
	ALIAS(_name.3, _name.QQ)

/* FIXME: Alternative implementation using capacitor.
 *        This is a transitional implementation
 */

inline int CAPACITOR_tc_hl(const double c, const double r)
{
	/*
	 * Vt = (VH-VL)*exp(-t/RC)
	 * ln(Vt/(VH-VL))*RC = -t
	 */
	static const double TIME_CONSTANT = -nl_math::log(2.0 / (3.7-0.3));
	int ret = (int) (TIME_CONSTANT * (130.0 + r) * c * 1e9);
	return ret;
}

inline int CAPACITOR_tc_lh(const double c, const double r)
{
	/*
	 * Vt = (VH-VL)*(1-exp(-t/RC))
	 * -t=ln(1-Vt/(VH-VL))*RC
	 */
	static const double TIME_CONSTANT = -nl_math::log(1.0 - 0.8 / (3.7-0.3));
	int ret = (int) (TIME_CONSTANT * (1.0 + r) * c * 1e9);
	return ret;
}

#define CHIP_CAPACITOR(_name, _pdesc) \
	NETDEV_DELAY(_name) \
	NETDEV_PARAMI(_name, L_TO_H, CAPACITOR_tc_lh((_pdesc)->c, (_pdesc)->r)) \
	NETDEV_PARAMI(_name, H_TO_L, CAPACITOR_tc_hl((_pdesc)->c, (_pdesc)->r))

#endif /* NL_DICE_COMPAT_H_ */
