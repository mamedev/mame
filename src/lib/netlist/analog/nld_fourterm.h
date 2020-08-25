// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_FOURTERM_H_
#define NLD_FOURTERM_H_

///
/// \file nld_fourterm.h
///

#include "../nl_setup.h"

// ----------------------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------------------

#define VCCS(name, G)                                                         \
		NET_REGISTER_DEVEXT(VCCS, name, G)

#define CCCS(name, G)                                                         \
		NET_REGISTER_DEVEXT(CCCS, name, G)

#define VCVS(name, G)                                                         \
		NET_REGISTER_DEVEXT(VCVS, name, G)

#define CCVS(name, G)                                                         \
		NET_REGISTER_DEVEXT(CCVS, name, G)

#define LVCCS(name)                                                           \
		NET_REGISTER_DEV(LVCCS, name)

#endif // NLD_FOURTERM_H_
