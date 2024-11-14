// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file test_precommit.cpp
///
/// tests to check for experimental code before commit
///

#include "nl_config.h"

#include "plib/pconfig.h"
#include "plib/ppmf.h"
#include "plib/ptests.h"

PTEST(test_precommit, precommit)
{
	PEXPECT_EQ(PPMF_EXPERIMENTAL, 0);
	PEXPECT_EQ(PPMF_USE_MAME_DELEGATES, 0);

	PEXPECT_EQ(netlist::config::use_copy_instead_of_reference::value, false);
	PEXPECT_EQ(NL_USE_BACKWARD_EULER, 1);
	PEXPECT_EQ(PUSE_FLOAT128, 0);
	PEXPECT_EQ(NL_USE_FLOAT128, PUSE_FLOAT128);
	PEXPECT_EQ(NL_USE_INPLACE_CORE_TERMS, 0);
	PEXPECT_EQ(netlist::config::avoid_noop_queue_pushes::value, false);
}
