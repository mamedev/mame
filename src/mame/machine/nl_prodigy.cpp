// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom

/***************************************************************************

  Netlist (prodigy) included from prodigy.cpp

***************************************************************************/

#include "netlist/devices/net_lib.h"

NETLIST_START(prodigy)

	SOLVER(Solver, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	ANALOG_INPUT(VCC, 5) // For TTL chips

	TTL_INPUT(high, 1)
	TTL_INPUT(low, 0)

	TTL_INPUT(cb1, 0)
	TTL_INPUT(cb2, 0)

	TTL_74164(SHIFT, cb2, cb2, high, cb1)
	ALIAS(bcd_bit0, SHIFT.QA)
	ALIAS(bcd_bit1, SHIFT.QB)
	ALIAS(bcd_bit2, SHIFT.QC)
	ALIAS(bcd_bit3, SHIFT.QD)
	ALIAS(bcd_bit4, SHIFT.QE)
	ALIAS(bcd_bit5, SHIFT.QF)
	ALIAS(bcd_bit6, SHIFT.QG)
	ALIAS(bcd_bit7, SHIFT.QH)

NETLIST_END()
