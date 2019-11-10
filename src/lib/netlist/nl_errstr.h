// license:GPL-2.0+
// copyright-holders:Couriersud

///
/// \file nl_errstr.h
///

#ifndef NL_ERRSTR_H_
#define NL_ERRSTR_H_

#include "plib/pfmtlog.h"

#define PERRMSG(name, str) \
	struct name \
	{ \
		operator pstring() const noexcept { return str; } \
	};

#define PERRMSGV(name, narg, str) \
	struct name \
	{ \
		template<typename... Args> name(Args&&... args) \
		: m_m(plib::pfmt(str)(std::forward<Args>(args)...)) \
		{ static_assert(narg == sizeof...(args), "Argument count mismatch"); } \
		operator pstring() const noexcept { return m_m; } \
		pstring m_m; \
	};

namespace netlist
{

	static constexpr const char sHINT_NO_DEACTIVATE[] = ".HINT_NO_DEACTIVATE";
	static constexpr const char sPowerDevRes[] = "_RVG";
	static constexpr const char sPowerGND[] = "GND";
	static constexpr const char sPowerVCC[] = "VCC";

	// nl_base.cpp

	PERRMSGV(MF_DUPLICATE_NAME_DEVICE_LIST,         1, "Error adding {1} to device list. Duplicate name.")
	PERRMSGV(MF_UNKNOWN_TYPE_FOR_OBJECT,            1, "Unknown type for object {1},")
	PERRMSGV(MF_NET_1_DUPLICATE_TERMINAL_2,         2, "net {1}: duplicate terminal {2}")
	PERRMSGV(MF_REMOVE_TERMINAL_1_FROM_NET_2,       2, "Can not remove terminal {1} from net {2}.")
	PERRMSGV(MF_UNKNOWN_PARAM_TYPE,                 1, "Can not determine param_type for {1}")
	PERRMSGV(MF_ERROR_CONNECTING_1_TO_2,            2, "Error connecting {1} to {2}")
	PERRMSGV(MF_NO_SOLVER,                          0, "No solver found for this netlist although analog elements are present")
	PERRMSGV(MF_HND_VAL_NOT_SUPPORTED,              1, "HINT_NO_DEACTIVATE value not supported: <{1}>")
	PERRMSGV(MW_ROM_NOT_FOUND,                      1, "Rom {1} not found")

	// nl_factory.cpp

	PERRMSGV(MF_FACTORY_ALREADY_CONTAINS_1,         1, "factory already contains {1}")
	PERRMSGV(MF_CLASS_1_NOT_FOUND,                  1, "Class <{1}> not found!")

	// nl_base.cpp

	PERRMSGV(MF_MODEL_1_CAN_NOT_BE_CHANGED_AT_RUNTIME, 1, "Model {1} can not be changed at runtime")
	PERRMSGV(MF_MORE_THAN_ONE_1_DEVICE_FOUND,       1, "more than one {1} device found")

	// nl_parser.cpp

	PERRMSGV(MF_UNEXPECTED_NETLIST_END,             0, "Unexpected NETLIST_END")
	PERRMSGV(MF_UNEXPECTED_NETLIST_START,           0, "Unexpected NETLIST_START")

	// nl_setup.cpp

	PERRMSGV(MF_UNABLE_TO_PARSE_MODEL_1,            1, "Unable to parse model: {1}")
	PERRMSGV(MF_MODEL_ALREADY_EXISTS_1,             1, "Model already exists: {1}")
	PERRMSGV(MF_DEVICE_ALREADY_EXISTS_1,            1, "Device already exists: {1}")
	PERRMSGV(MF_ADDING_ALI1_TO_ALIAS_LIST,          1, "Error adding alias {1} to alias list")
	PERRMSGV(MF_DIP_PINS_MUST_BE_AN_EQUAL_NUMBER_OF_PINS_1, 1,"You must pass an equal number of pins to DIPPINS {1}")
	PERRMSGV(MF_PARAM_COUNT_MISMATCH_2,				2, "Parameter count mismatch for {1} - only found {2}")
	PERRMSGV(MF_PARAM_COUNT_EXCEEDED_2,				2, "Parameter count exceed for {1} - found {2}")
	PERRMSGV(MF_UNKNOWN_OBJECT_TYPE_1,              1, "Unknown object type {1}")
	PERRMSGV(MF_INVALID_NUMBER_CONVERSION_1_2,      2, "Invalid number conversion {1} : {2}")
	PERRMSGV(MF_INVALID_ENUM_CONVERSION_1_2,        2, "Invalid element found {1} : {2}")
	PERRMSGV(MF_ADDING_PARAMETER_1_TO_PARAMETER_LIST,1, "Error adding parameter {1} to parameter list")
	PERRMSGV(MF_ADDING_1_2_TO_TERMINAL_LIST,        2, "Error adding {1} {2} to terminal list")
	PERRMSGV(MF_NET_C_NEEDS_AT_LEAST_2_TERMINAL,    0, "You must pass at least 2 terminals to NET_C")
	PERRMSGV(MF_FOUND_NO_OCCURRENCE_OF_1,           1, "Found no occurrence of {1}")
	PERRMSGV(MF_TERMINAL_1_2_NOT_FOUND,             2, "Alias {1} was resolved to be terminal {2}. Terminal {2} was not found.")
	PERRMSGV(MF_OBJECT_1_2_WRONG_TYPE,              2, "object {1}({2}) found but wrong type")
	PERRMSGV(MF_PARAMETER_1_2_NOT_FOUND,            2, "parameter {1}({2}) not found!")
	PERRMSGV(MF_CONNECTING_1_TO_2,                  2, "Error connecting {1} to {2}")
	PERRMSGV(MF_MERGE_RAIL_NETS_1_AND_2,            2, "Trying to merge two rail nets: {1} and {2}")
	PERRMSGV(MF_OBJECT_INPUT_TYPE_1,                1, "Unable to determine input type of {1}")
	PERRMSGV(MF_OBJECT_OUTPUT_TYPE_1,               1, "Unable to determine output type of {1}")
	PERRMSGV(MF_INPUT_1_ALREADY_CONNECTED,          1, "Input {1} already connected")
	PERRMSGV(MF_LINK_TRIES_EXCEEDED,                1, "Error connecting, {1} tries exceeded")
	PERRMSGV(MF_MODEL_NOT_FOUND,                    1, "Model {1} not found")
	PERRMSGV(MF_MODEL_ERROR_1,                      1, "Model error {1}")
	PERRMSGV(MF_MODEL_ERROR_ON_PAIR_1,              1, "Model error on pair {1}")
	PERRMSGV(MF_MODEL_PARAMETERS_NOT_UPPERCASE_1_2, 2, "Model parameters should be uppercase:{1} {2}")
	PERRMSGV(MF_ENTITY_1_NOT_FOUND_IN_MODEL_2,      2, "Entity {1} not found in model {2}")
	PERRMSGV(MF_UNKNOWN_NUMBER_FACTOR_IN_1,         1, "Unknown number factor in: {1}")
	PERRMSGV(MF_MODEL_NUMBER_CONVERSION_ERROR,      4, "Can't convert {1}={2} to {3} for model {4}")
	PERRMSGV(MF_NOT_FOUND_IN_SOURCE_COLLECTION,     1, "unable to find {1} in sources collection")

	PERRMSGV(MW_OVERWRITING_PARAM_1_OLD_2_NEW_3,    3, "Overwriting {1} old <{2}> new <{3}>")
	PERRMSGV(MW_CONNECTING_1_TO_ITSELF,             1, "Connecting {1} to itself. This may be right, though")
	PERRMSGV(MI_DUMMY_1_WITHOUT_CONNECTIONS,        1, "Found dummy terminal {1} without connections")
	PERRMSGV(MI_ANALOG_OUTPUT_1_WITHOUT_CONNECTIONS,1, "Found analog output {1} without connections")
	PERRMSGV(MI_LOGIC_OUTPUT_1_WITHOUT_CONNECTIONS, 1, "Found logic output {1} without connections")
	PERRMSGV(MW_LOGIC_INPUT_1_WITHOUT_CONNECTIONS,  1, "Found logic input {1} without connections")
	PERRMSGV(MW_TERMINAL_1_WITHOUT_CONNECTIONS,     1, "Found terminal {1} without connections")

	PERRMSGV(ME_TERMINAL_1_WITHOUT_NET,             1, "Found terminal {1} without a net")
	PERRMSGV(MF_TERMINALS_WITHOUT_NET,              0, "Found terminals without a net")

	PERRMSGV(MI_REMOVE_DEVICE_1_CONNECTED_ONLY_TO_RAILS_2_3, 3, "Found device {1} connected only to railterminals {2}/{3}. Will be removed")
	PERRMSGV(MI_POWER_TERMINALS_1_CONNECTED_ANALOG_2_3, 3, "Power terminals {1} connected to analog nets {2}/{3}.")

	PERRMSGV(MW_DATA_1_NOT_FOUND,                   1, "unable to find data {1} in sources collection")

	PERRMSGV(MW_DEVICE_NOT_FOUND_FOR_HINT,          1, "Device not found for hint {1}")
	PERRMSGV(MW_UNKNOWN_PARAMETER,                  1, "Unknown parameter {1}")

	// nlid_proxy.cpp

	PERRMSGV(MI_NO_POWER_TERMINALS_ON_DEVICE_1,     1, "D/A Proxy: Found no valid combination of power terminals on device {1}")
	PERRMSGV(MI_MULTIPLE_POWER_TERMINALS_ON_DEVICE, 5, "D/A Proxy: Found multiple power terminals on device {1}: {2} {3} {4} {5}")

	// nld_matrix_solver.cpp

	PERRMSGV(MF_UNHANDLED_ELEMENT_1_FOUND,          1, "setup_base:unhandled element <{1}> found")
	PERRMSGV(MF_FOUND_TERM_WITH_MISSING_OTHERNET,   1, "found term with missing othernet {1}")

	PERRMSGV(MW_NEWTON_LOOPS_EXCEEDED_ON_NET_1,     1, "NEWTON_LOOPS exceeded on net {1}... reschedule")

	// nld_solver.cpp

	PERRMSGV(MI_NO_SPECIFIC_SOLVER,                 1, "No specific solver found for netlist of size {1}")

	// nld_mm5837.cpp

	PERRMSGV(MW_FREQUENCY_OUTSIDE_OF_SPECS_1,       1, "MM5837: Frequency outside of specs: {1}")

	// nld_opamps.cpp

	PERRMSGV(MF_OPAMP_UNKNOWN_TYPE,                 1, "Unknown opamp type: {1}")
	PERRMSGV(MW_OPAMP_FAIL_CONVERGENCE,             1, "Opamp <{1}> parameters fail convergence criteria")

	// nld_mosfet.cpp

	PERRMSGV(MW_MOSFET_THRESHOLD_VOLTAGE,           1, "Mosfet: Threshold voltage not specified for {1}")

	// nl_tool.cpp

	PERRMSGV(MF_FILE_OPEN_ERROR,                    1, "Error opening file: {1}")



} // namespace netlist


#endif // NL_ERRSTR_H_
