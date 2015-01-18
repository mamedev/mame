// license:MAME,GPL-2.0+
// copyright-holders:couriersud
/*
 * nl_dice_compat.h
 *
 */

#ifndef NL_DICE_COMPAT_H_
#define NL_DICE_COMPAT_H_

#include "netlist/devices/net_lib.h"

/* --------------------------------------------------------------------
 * Compatibility macros for DICE netlists ...
 * -------------------------------------------------------------------- */

/*
 * define NETLIST_DEVELOPMENT in IDEs before including this header file
 * to get compile time errors on unknown devices. This should only be
 * a temporary support and not be used in commits.
 */

#ifdef NETLIST_DEVELOPMENT
#define CHIP(_n, _t) setup.register_dev( nl_alloc(nld_ ## _t ## _dip), _n);
#else
#define CHIP(_n, _t) setup.register_dev(NETLIB_NAME_STR(_t ## _dip), _n);
#endif

#define CONNECTION( ... ) CONNECTIONY( CONNECTIONX( __VA_ARGS__ ) )
#define CONNECTIONY(_a) _a
#define CONNECTIONX(_a, _b, _c, _d) setup.register_link(_a "." # _b, _c "." # _d);
#define NET_CSTR(_a, _b) setup.register_link( _a, _b);

#define CIRCUIT_LAYOUT(x) NETLIST_START(x)
#define CIRCUIT_LAYOUT_END NETLIST_END()


#define K_OHM(x) ((x) * 1000.0)
#define U_FARAD(x) ((x) * 1.0e-6)

struct Mono555Desc
{
public:
		nl_double r, c;

		Mono555Desc(nl_double res, nl_double cap) : r(res), c(cap) { }
};

struct SeriesRCDesc
{
public:
		nl_double r, c;

		SeriesRCDesc(nl_double res, nl_double cap) : r(res), c(cap) { }
};

#define CHIP_555_Mono(_name,  _pdesc)   \
	CHIP(# _name, NE555) \
	NET_C(_name.6, _name.7) \
	RES(_name ## _R, (_pdesc)->r) \
	CAP(_name ## _C, (_pdesc)->c) \
	NET_C(_name.6, _name ## _R.1) \
	NET_C(_name.6, _name ## _C.1) \
	NET_C(_name ## _R.2, V5) \
	NET_CSTR(# _name "_C.2", "GND") \
	NET_C(_name.8, V5) \
	NET_CSTR(# _name ".1", "GND")

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



#endif /* NL_DICE_COMPAT_H_ */
