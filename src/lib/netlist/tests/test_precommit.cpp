// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file test_pmfp.cpp
///
/// tests for `plib::pmfp`
///

#include "plib/pconfig.h"
#include "plib/ppmf.h"
#include "netlist/nl_config.h"

#include "plib/ptests.h"

PTEST(test_precommit, precommit)
{
	PEXPECT_EQ(PPMF_EXPERIMENTAL, 0);
	PEXPECT_EQ(PPMF_USE_MAME_DELEGATES, 0);

	PEXPECT_EQ(NL_USE_COPY_INSTEAD_OF_REFERENCE, 0);
	PEXPECT_EQ(NL_USE_BACKWARD_EULER, 1);
	PEXPECT_EQ(PUSE_FLOAT128, 0);
	PEXPECT_EQ(NL_USE_FLOAT128, PUSE_FLOAT128);
	PEXPECT_EQ(AVOID_NOOP_QUEUE_PUSHES, 0);
}
