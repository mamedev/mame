// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file nl_errstr.h
///

#ifndef NL_ERRSTR_H_
#define NL_ERRSTR_H_

#include "plib/pexception.h"
#include "plib/pfmtlog.h"

namespace netlist
{

	static constexpr const char sHINT_NO_DEACTIVATE[]
		= ".HINT_NO_DEACTIVATE"; // NOLINT(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
	static constexpr const char sHINT_NC[]
		= ".HINT_NC"; // NOLINT(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)

	// -------------------------------------------------------------------------
	// Exceptions
	// -------------------------------------------------------------------------

	/// \brief Generic netlist exception.
	///  The exception is used in all events which are considered fatal.

	class nl_exception : public plib::pexception
	{
	public:
		/// \brief Constructor.
		///  Allows a descriptive text to be passed to the exception

		explicit nl_exception(const pstring &text //!< text to be passed
							  )
		: plib::pexception(text)
		{
		}

		/// \brief Constructor.
		///  Allows to use \ref plib::pfmt logic to be used in exception

		template <typename... Args>
		explicit nl_exception(const pstring &fmt, //!< format to be used
							  Args &&...args      //!< arguments to be passed
							  )
		: plib::pexception(plib::pfmt(fmt)(std::forward<Args>(args)...))
		{
		}
	};

	// -------------------------------------------------------------------------
	// Error messages
	// -------------------------------------------------------------------------

	// clang-format off

	// nl_base.cpp

	PERRMSGV(MF_DUPLICATE_NAME_DEVICE_LIST,         1, "Error adding {1} to device list. Duplicate name.")
	PERRMSGV(MF_UNKNOWN_TYPE_FOR_OBJECT,            1, "Unknown type for object {1},")
	PERRMSGV(MF_NET_1_DUPLICATE_TERMINAL_2,         2, "net {1}: duplicate terminal {2}")
	PERRMSGV(MF_NULLPTR_FAMILY,                     2, "Unable to determine family for device {1} from model {2}")
	PERRMSGV(MF_REMOVE_TERMINAL_1_FROM_NET_2,       2, "Can not remove terminal {1} from net {2}.")
	PERRMSGV(MF_UNKNOWN_PARAM_TYPE,                 1, "Can not determine param_type for {1}")
	//PERRMSGV(MF_ERROR_CONNECTING_1_TO_2,            2, "Error connecting {1} to {2}")
	//PERRMSGV(ME_HND_VAL_NOT_SUPPORTED,              1, "HINT_NO_DEACTIVATE value not supported: <{1}>")
	PERRMSGV(MW_ROM_NOT_FOUND,                      1, "Rom {1} not found")

	// nl_factory.cpp

	PERRMSGV(MF_FACTORY_ALREADY_CONTAINS_1,         1, "factory already contains {1}")
	PERRMSGV(MF_CLASS_1_NOT_FOUND,                  1, "Class <{1}> not found!")

	// nl_base.cpp

	PERRMSGV(MF_MODEL_1_CAN_NOT_BE_CHANGED_AT_RUNTIME, 1, "Model {1} can not be changed at runtime")
	PERRMSGV(MF_MORE_THAN_ONE_1_DEVICE_FOUND,       1, "More than one {1} device found")

	// nl_parser.cpp

	PERRMSGV(MF_PARSER_UNEXPECTED_1,                1, "Unexpected {}")
	PERRMSGV(MF_UNEXPECTED_NETLIST_END,             0, "Unexpected NETLIST_END")
	PERRMSGV(MF_UNEXPECTED_END_OF_FILE,             0, "Unexpected end of file, missing NETLIST_END")
	//PERRMSGV(MF_UNEXPECTED_NETLIST_START,           0, "Unexpected NETLIST_START")
	PERRMSGV(MF_UNEXPECTED_NETLIST_EXTERNAL,        0, "Unexpected NETLIST_EXTERNAL within a netlist")
	PERRMSGV(MF_EXPECTED_NETLIST_START_1,           1, "Expected NETLIST_START but got {1}")
	PERRMSGV(MF_EXPECTED_IDENTIFIER_GOT_1,          1, "Expected an identifier, but got {1}")
	PERRMSGV(MF_EXPECTED_COMMA_OR_RP_1,             1, "Expected comma or right parenthesis but found <{1}>")
	PERRMSGV(MF_DIPPINS_EQUAL_NUMBER_1,             1, "DIPPINS requires equal number of pins to DIPPINS, first pin is {}")
	//PERRMSGV(MF_PARAM_NOT_FP_1,                     1, "Parameter value <{1}> not floating point")
	PERRMSGV(MF_TT_LINE_WITHOUT_HEAD,               0, "TT_LINE found without TT_HEAD")
	PERRMSGV(MF_LOCAL_SOURCE_NOT_FOUND_1,           1, "Local source not found: <{1}>")
	PERRMSGV(MF_EXTERNAL_SOURCE_IS_LOCAL_1,         1, "External lib entry appears as a local one: <{1}>")
	PERRMSGV(MF_TRUTHTABLE_NOT_FOUND_1,             1, "Truth table not found: <{1}>")

	// nl_setup.cpp

	PERRMSGV(MF_UNABLE_TO_PARSE_MODEL_1,            1, "Unable to parse model: {1}")
	// FIXME: Add an directive MODEL_OVERWRITE to netlist language
	//PERRMSGV(MF_MODEL_ALREADY_EXISTS_1,             1, "Model already exists: {1}")
	PERRMSGV(MI_MODEL_OVERWRITE_1,                  2, "Model already exists, overwriting {1} with {2}")
	PERRMSGV(MF_DEVICE_ALREADY_EXISTS_1,            1, "Device already exists: {1}")
	PERRMSGV(MF_UNUSED_HINT_1,                      1, "Error hint {1} is not used")
	PERRMSGV(MF_ADDING_HINT_1,                      1, "Error adding hint {1} to hint list")
	PERRMSGV(MF_ALIAS_ALREAD_EXISTS_1,              1, "Alias already exists: {1}")
	PERRMSGV(MF_DIP_PINS_MUST_BE_AN_EQUAL_NUMBER_OF_PINS_1, 1,"You must pass an equal number of pins to DIPPINS {1}")
	PERRMSGV(MF_PARAM_COUNT_MISMATCH_2,             2, "Parameter count mismatch for {1} - only found {2}")
	PERRMSGV(MF_PARAM_COUNT_EXCEEDED_2,             2, "Parameter count exceed for {1} - found {2}")
	//PERRMSGV(MF_UNKNOWN_OBJECT_TYPE_1,              1, "Unknown object type {1}")
	PERRMSGV(MF_UNKNOWN_FAMILY_TYPE_1,              2, "Unknown family type {1} in model {2}")
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
	PERRMSGV(MF_DUPLICATE_PROXY_1,                  1, "Terminal {1} already has proxy")
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
	PERRMSGV(MF_UNKNOWN_NUMBER_FACTOR_IN_2,         2, "Unknown number factor in: {1}:{2}")
	PERRMSGV(MF_MODEL_NUMBER_CONVERSION_ERROR,      4, "Can't convert {1}={2} to {3} for model {4}")
	PERRMSGV(MF_NOT_FOUND_IN_SOURCE_COLLECTION,     1, "unable to find {1} in sources collection")

	PERRMSGV(MI_OVERWRITING_PARAM_1_OLD_2_NEW_3,    3, "Overwriting {1} old <{2}> new <{3}>")
	PERRMSGV(MW_CONNECTING_1_TO_ITSELF,             1, "Connecting net {1} to itself.")
	PERRMSGV(MI_CONNECTING_1_TO_2_SAME_NET,         3, "Connecting terminals {1} and {2} which are already both on net {3}. "
		"It is ok if you read this warning and it relates to pin which is connected internally to GND and the schematics "
		"show an external connection as well. One example is the CD4538. In other cases this warning may indicate "
		"an error in your netlist.")
	PERRMSGV(ME_NC_PIN_1_WITH_CONNECTIONS,          1, "Found NC (not connected) terminal {1} with connections")
	PERRMSGV(MI_ANALOG_OUTPUT_1_WITHOUT_CONNECTIONS,1, "Found analog output {1} without connections")
	PERRMSGV(MI_LOGIC_OUTPUT_1_WITHOUT_CONNECTIONS, 1, "Found logic output {1} without connections")
	PERRMSGV(MW_LOGIC_INPUT_1_WITHOUT_CONNECTIONS,  1, "Found logic input {1} without connections")
	PERRMSGV(MW_TERMINAL_1_WITHOUT_CONNECTIONS,     1, "Found terminal {1} without connections")

	PERRMSGV(ME_TERMINAL_1_WITHOUT_NET,             1, "Found terminal {1} without a net")
	PERRMSGV(ME_TERMINALS_1_2_WITHOUT_NET,           2, "Found terminals {1} and {2} without a net")
	PERRMSGV(MF_TERMINALS_WITHOUT_NET,              0, "Found terminals without a net")
	PERRMSGV(ME_TRISTATE_NO_PROXY_FOUND_2,          2,
		"Tristate output {1} on device {2} is not connected to a proxy. You "
		"need to set parameter FORCE_TRISTATE_LOGIC for device {2} if "
		"tristate enable inputs are all connected to fixed inputs. If this "
		"is not the case: Review your netlist. Something is wrong.")
	PERRMSGV(ME_TRISTATE_PROXY_FOUND_2,             2,
		"The tristate output {1} on device {2} is connected to an analog net "
		"but has been forced to act as a logic output. Parameter "
		" FORCE_TRISTATE_LOGIC for device {2} needs to be disabled!.")

	PERRMSGV(MI_REMOVE_DEVICE_1_CONNECTED_ONLY_TO_RAILS_2_3, 3, "Found device {1} connected only to rail terminals {2}/{3}."
		" This may reflect the schematic - but as well be an error. Please review.")

	PERRMSGV(MW_DATA_1_NOT_FOUND,                   1, "unable to find data {1} in sources collection")

	//PERRMSGV(ME_DEVICE_NOT_FOUND_FOR_HINT,          1, "Device not found for hint {1}")
	PERRMSGV(ME_UNKNOWN_PARAMETER,                  1, "Unknown parameter {1}")
	PERRMSGV(MF_ERRORS_FOUND,                       1, "Counted {1} errors which need to be fixed")

	PERRMSGV(MF_NO_SOLVER,                          0, "No solver found for this netlist although analog elements are present")
	PERRMSGV(MF_DELEGATE_NOT_SET_1,                 1, "delegate not set for terminal {1}")

	// nlid_proxy.cpp

	PERRMSGV(MF_NO_POWER_TERMINALS_ON_DEVICE_2,     2, "D/A Proxy {1}: Found no valid combination of power terminals on device {2}")
	PERRMSGV(MI_MULTIPLE_POWER_TERMINALS_ON_DEVICE, 5, "D/A Proxy: Found multiple power terminals on device {1}: {2} {3} {4} {5}")
	PERRMSGV(MF_NULLPTR_FAMILY_NP,                  1, "Encountered nullptr to family in {1}")

	// nld_matrix_solver.cpp

	PERRMSGV(MF_UNHANDLED_ELEMENT_1_FOUND,          1, "setup_base:unhandled element <{1}> found")
	PERRMSGV(MF_FOUND_TERM_WITH_MISSING_OTHERNET,   1, "found term with missing other net {1}")

	PERRMSGV(MW_NEWTON_LOOPS_EXCEEDED_INVOCATION_3, 3, "NEWTON_LOOPS exceeded resolution invoked {1} times on net {2} at {3} us")
	PERRMSGV(MW_NEWTON_LOOPS_EXCEEDED_ON_NET_2,     2, "NEWTON_LOOPS exceeded resolution failed on net {1} ... reschedule  at {2} us")

	// nld_solver.cpp

	PERRMSGV(MI_NO_SPECIFIC_SOLVER,                 1, "No specific solver found for netlist of size {1}")
	PERRMSGV(MW_SOLVER_METHOD_NOT_SUPPORTED,        2, "Solver method {1} not supported. Falling back to {2}")

	PERRMSGV(ME_SOLVER_CONSISTENCY_NOT_ANALOG_NET,  1, "Solver consistency: {1} is not an analog net")
	PERRMSGV(ME_SOLVER_CONSISTENCY_RAIL_NET,        1, "Solver consistency: {1} is a rail net")
	PERRMSGV(ME_SOLVER_TERMINAL_NO_NET,             1, "Solver consistency: Terminal {1} has no net")
	PERRMSGV(ME_SOLVER_NO_RAIL_TERMINAL,            1, "Solver consistency: No rail terminals in group with nets: {1}\n"
		"At least one rail terminal (like ANALOG_INPUT) needs to be part of a group of nets\n"
		"solved by a solver. Without a singular matrix would be created\n"
		"which has the potential to cause a crash now or in the future.\n"
		"A common cause of this error are BJT or FET devices which\n"
		"are defined but not used in the netlist.")
	PERRMSGV(MF_SOLVER_CONSISTENCY_ERRORS,          1, "Found {1} errors during solver consistency test")

	// nld_mm5837.cpp

	PERRMSGV(MW_FREQUENCY_OUTSIDE_OF_SPECS_1,       1, "MM5837: Frequency outside of specs: {1}")

	// nld_opamps.cpp

	PERRMSGV(MF_OPAMP_UNKNOWN_TYPE,                 1, "Unknown opamp type: {1}")
	PERRMSGV(MW_OPAMP_FAIL_CONVERGENCE,             1, "Opamp <{1}> parameters fail convergence criteria")

	// nld_mosfet.cpp

	PERRMSGV(MW_MOSFET_THRESHOLD_VOLTAGE,           1, "Mosfet: Threshold voltage not specified for {1}")

	// nld_bjt.cpp

	PERRMSGV(MF_DEVICE_FRY_1,                       1,
		"Please don't fry device {}. Most likely this error is caused by the"
		" fact that you want to exclude the analog device from the netlist."
		" This is not the right approach. If you want to exclude the device,"
		" exclude the device altogether, i.e. by using #ifdef/#if statements.")

	// nl_tool.cpp

	PERRMSGV(MF_FILE_OPEN_ERROR,                    1, "Error opening file: {1}")

	// clang-format on
} // namespace netlist

// -------------------------------------------------------------------------
//  Asserts
// -------------------------------------------------------------------------

#define nl_assert(x)                                                           \
	do                                                                         \
	{                                                                          \
		if (NL_DEBUG)                                                          \
			passert_always(x);                                                 \
	} while (0)
#define nl_assert_always(x, msg) passert_always_msg(x, msg)

#endif // NL_ERRSTR_H_
