// license:GPL-2.0+
// copyright-holders:Couriersud

// ***************************************************************************
//
//    net_lib.cpp
//
// ***************************************************************************

#include "net_lib.h"
#include "nl_factory.h"
#include "solver/nld_solver.h"


#define NETLIB_DEVICE_DECL(chip) extern factory::constructor_ptr_t decl_ ## chip
#define LIB_DECL(decl) factory.add( decl () );
#define LIB_ENTRY(nic) { NETLIB_DEVICE_DECL(nic); LIB_DECL(decl_ ## nic) }

namespace netlist
{
namespace devices
{

	void initialize_factory(factory::list_t &factory)
	{
		// The following is from a script which automatically creates
		// the entries.
		// FIXME: the list should be either included or the whole
		// initialize factory code should be created programmatically.

		#include "../generated/lib_entries.hxx"

	}

} //namespace devices
} // namespace netlist

