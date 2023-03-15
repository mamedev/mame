// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef NET_LIB_H
#define NET_LIB_H

///
/// \file net_lib.h
///
/// This file is included by all netlist implementations.
///

#include "../nl_setup.h"

#ifdef RES_R
#warning "Do not include `rescap.h` in a netlist environment"
#endif
#ifndef RES_R
#define RES_R(res) (res)
#define RES_K(res) ((res) * 1e3)
#define RES_M(res) ((res) * 1e6)
#define CAP_U(cap) ((cap) * 1e-6)
//#define CAP_U(cap) ((cap) * 1μ)
#define CAP_N(cap) ((cap) * 1e-9)
#define CAP_P(cap) ((cap) * 1e-12)
#define IND_U(ind) ((ind) * 1e-6)
#define IND_N(ind) ((ind) * 1e-9)
#define IND_P(ind) ((ind) * 1e-12)
#endif

#include "../generated/nld_devinc.h"

NETLIST_EXTERNAL(base_lib)

#endif // NET_LIB_H
