// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_system.c
 *
 */

#include "netlist/solver/nld_solver.h"
#include "netlist/solver/nld_matrix_solver.h"
#include "nlid_system.h"

namespace netlist
{
namespace devices
{

	NETLIB_DEVICE_IMPL(netlistparams,       "PARAMETER",              "")
	NETLIB_DEVICE_IMPL(nc_pin,              "NC_PIN",                 "")

	NETLIB_DEVICE_IMPL(frontier,            "FRONTIER_DEV",           "+I,+G,+Q")
	NETLIB_DEVICE_IMPL(function,            "AFUNC",                  "N,FUNC")
	NETLIB_DEVICE_IMPL(analog_input,        "ANALOG_INPUT",           "IN")
	NETLIB_DEVICE_IMPL(clock,               "CLOCK",                  "FREQ")
	NETLIB_DEVICE_IMPL(varclock,            "VARCLOCK",               "N,FUNC")
	NETLIB_DEVICE_IMPL(extclock,            "EXTCLOCK",               "FREQ,PATTERN")
	NETLIB_DEVICE_IMPL(sys_dsw1,            "SYS_DSW",                "+I,+1,+2")
	NETLIB_DEVICE_IMPL(sys_dsw2,            "SYS_DSW2",               "")
	NETLIB_DEVICE_IMPL(sys_compd,           "SYS_COMPD",              "")

	using NETLIB_NAME(sys_noise_mt_u) =
		NETLIB_NAME(sys_noise)<plib::mt19937_64, plib::uniform_distribution_t>;
	NETLIB_DEVICE_IMPL(sys_noise_mt_u,      "SYS_NOISE_MT_U",         "SIGMA")

	using NETLIB_NAME(sys_noise_mt_n) =
		NETLIB_NAME(sys_noise)<plib::mt19937_64, plib::normal_distribution_t>;
	NETLIB_DEVICE_IMPL(sys_noise_mt_n,      "SYS_NOISE_MT_N",         "SIGMA")

	NETLIB_DEVICE_IMPL(mainclock,           "MAINCLOCK",              "FREQ")
	NETLIB_DEVICE_IMPL(gnd,                 "GNDA",                   "")

	using NETLIB_NAME(logic_input8) = NETLIB_NAME(logic_inputN)<8>;
	NETLIB_DEVICE_IMPL(logic_input8,         "LOGIC_INPUT8",            "IN,MODEL")

	NETLIB_DEVICE_IMPL(logic_input,         "LOGIC_INPUT",            "IN,MODEL")
	NETLIB_DEVICE_IMPL_ALIAS(logic_input_ttl, logic_input, "TTL_INPUT", "IN")

} // namespace devices
} // namespace netlist
