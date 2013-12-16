// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlconfig.h
 *
 */

#ifndef NLCONFIG_H_
#define NLCONFIG_H_

/* FIXME: at some time, make it compile on it's own */

#include "emu.h"

//============================================================
//  SETUP
//============================================================

#define USE_DELEGATES           (0)
/*
 * The next options needs -Wno-pmf-conversions to compile and gcc
 * This is intended for non-mame usage.
 *
 */
#define USE_PMFDELEGATES        (0)

// Next if enabled adds 20% performance ... but is not guaranteed to be absolutely timing correct.
#define USE_DEACTIVE_DEVICE     (0)

// Use nano-second resolution - Sufficient for now
#define NETLIST_INTERNAL_RES        (U64(1000000000))
#define NETLIST_DIV_BITS            (0)
//#define NETLIST_INTERNAL_RES      (U64(1000000000000))
//#define NETLIST_DIV_BITS          (10)
#define NETLIST_DIV                 (U64(1) << NETLIST_DIV_BITS)
#define NETLIST_MASK                (NETLIST_DIV-1)
#define NETLIST_CLOCK               (NETLIST_INTERNAL_RES / NETLIST_DIV)

//FIXME: LEGACY
#define NETLIST_HIGHIMP_V   (1.23456e20)        /* some voltage we should never see */

#define NETLIST_GMIN    (1e-9)

typedef UINT8 netlist_sig_t;

//============================================================
//  DEBUGGING
//============================================================

#define NL_VERBOSE                  (0)
#define NL_KEEP_STATISTICS          (0)
#define FATAL_ERROR_AFTER_NS        (0) //(1000)

#if (NL_VERBOSE)
	#define NL_VERBOSE_OUT(x)       printf x
#else
	#define NL_VERBOSE_OUT(x)       do { } while (0)
#endif

//============================================================
//  MACROS
//============================================================

// prevent implicit copying
#define NETLIST_PREVENT_COPYING(_name)          \
    private:                                    \
        _name(const _name &);                   \
        _name &operator=(const _name &);        \

#if NL_KEEP_STATISTICS
#define add_to_stat(v,x)        do { v += (x); } while (0)
#define inc_stat(v)             add_to_stat(v, 1)
#define begin_timing(v)         do { (v) -= get_profile_ticks(); } while (0)
#define end_timing(v)           do { (v) += get_profile_ticks(); } while (0)
#else
#define add_to_stat(v,x)        do { } while (0)
#define inc_stat(v)             add_to_stat(v, 1)
#define begin_timing(v)         do { } while (0)
#define end_timing(v)           do { } while (0)
#endif

// Compiling without mame ?

#ifndef ATTR_HOT
#warning ATTR_HOT not defined
#define ATTR_HOT
#endif

#ifndef ATTR_COLD
#define ATTR_COLD
#endif

#if defined(__GNUC__) && (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
#if !defined(__ppc__) && !defined (__PPC__) && !defined(__ppc64__) && !defined(__PPC64__)
#define ATTR_ALIGN __attribute__ ((aligned(128)))
#else
#define ATTR_ALIGN
#endif
#else
#define ATTR_ALIGN
#endif


#endif /* NLCONFIG_H_ */
