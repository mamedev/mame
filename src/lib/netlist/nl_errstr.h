// license:GPL-2.0+
// copyright-holders:Couriersud
/*!
 *
 * \file nl_errstr.h
 *
 */

#ifndef NLERRSTR_H_
#define NLERRSTR_H_

// nl_base.cpp

#define MF_1_DUPLICATE_NAME_DEVICE_LIST         "Error adding {1} to device list. Duplicate name."
#define MF_1_UNKNOWN_TYPE_FOR_OBJECT            "Unknown type for object {1} "
#define MF_2_NET_1_DUPLICATE_TERMINAL_2         "net {1}: duplicate terminal {2}"
#define MF_2_REMOVE_TERMINAL_1_FROM_NET_2       "Can not remove terminal {1} from net {2}."
#define MF_1_UNKNOWN_PARAM_TYPE                 "Can not determine param_type for {1}"
#define MF_2_ERROR_CONNECTING_1_TO_2            "Error connecting {1} to {2}"
#define MF_0_NO_SOLVER                          "No solver found for this netlist although analog elements are present"
#define MF_1_HND_VAL_NOT_SUPPORTED              "HINT_NO_DEACTIVATE value not supported: <{1}>"

// nl_factory.cpp

#define MF_1_FACTORY_ALREADY_CONTAINS_1         "factory already contains {1}"
#define MF_1_CLASS_1_NOT_FOUND                  "Class <{1}> not found!"

// nld_opamps.cpp

#define MF_1_UNKNOWN_OPAMP_TYPE                 "Unknown opamp type: {1}"

// nld_matrix_solver.cpp

#define MF_1_UNHANDLED_ELEMENT_1_FOUND          "setup_base:unhandled element <{1}> found"
#define MF_1_FOUND_TERM_WITH_MISSING_OTHERNET   "found term with missing othernet {1}"

#define MW_1_NEWTON_LOOPS_EXCEEDED_ON_NET_1     "NEWTON_LOOPS exceeded on net {1}... reschedule"

// nld_solver.cpp

#define MF_1_UNKNOWN_SOLVER_TYPE                "Unknown solver type: {1}"
#define MF_1_NETGROUP_SIZE_EXCEEDED_1           "Encountered netgroup with > {1} nets"

#define MW_1_NO_SPECIFIC_SOLVER                 "No specific solver found for netlist of size {1}"

// nl_base.cpp

#define MF_1_MODEL_1_CAN_NOT_BE_CHANGED_AT_RUNTIME "Model {1} can not be changed at runtime"
#define MF_1_MORE_THAN_ONE_1_DEVICE_FOUND       "more than one {1} device found"

// nl_parser.cpp

#define MF_0_UNEXPECTED_NETLIST_END             "Unexpected NETLIST_END"
#define MF_0_UNEXPECTED_NETLIST_START           "Unexpected NETLIST_START"

// nl_setup.cpp

//#define MF_1_CLASS_1_NOT_FOUND    "Class {1} not found!"
#define MF_1_UNABLE_TO_PARSE_MODEL_1            "Unable to parse model: {1}"
#define MF_1_MODEL_ALREADY_EXISTS_1             "Model already exists: {1}"
#define MF_1_DEVICE_ALREADY_EXISTS_1            "Device already exists: {1}"
#define MF_1_ADDING_ALI1_TO_ALIAS_LIST          "Error adding alias {1} to alias list"
#define MF_1_DIP_PINS_MUST_BE_AN_EQUAL_NUMBER_OF_PINS_1 "You must pass an equal number of pins to DIPPINS {1}"
#define MF_1_UNKNOWN_OBJECT_TYPE_1              "Unknown object type {1}"
#define MF_2_INVALID_NUMBER_CONVERSION_1_2      "Invalid number conversion {1} : {2}"
#define MF_1_ADDING_PARAMETER_1_TO_PARAMETER_LIST "Error adding parameter {1} to parameter list"
#define MF_2_ADDING_1_2_TO_TERMINAL_LIST        "Error adding {1} {2} to terminal list"
#define MF_2_NET_C_NEEDS_AT_LEAST_2_TERMINAL    "You must pass at least 2 terminals to NET_C"
#define MF_1_FOUND_NO_OCCURRENCE_OF_1           "Found no occurrence of {1}"
#define MF_2_TERMINAL_1_2_NOT_FOUND             "Alias {1} was resolved to be terminal {2}. Terminal {2} was not found."
#define MF_2_OBJECT_1_2_WRONG_TYPE              "object {1}({2}) found but wrong type"
#define MF_2_PARAMETER_1_2_NOT_FOUND            "parameter {1}({2}) not found!"
#define MF_2_CONNECTING_1_TO_2                  "Error connecting {1} to {2}"
#define MF_2_MERGE_RAIL_NETS_1_AND_2            "Trying to merge two rail nets: {1} and {2}"
#define MF_1_OBJECT_INPUT_TYPE_1                "Unable to determine input type of {1}"
#define MF_1_OBJECT_OUTPUT_TYPE_1               "Unable to determine output type of {1}"
#define MF_1_INPUT_1_ALREADY_CONNECTED          "Input {1} already connected"
#define MF_0_LINK_TRIES_EXCEEDED                "Error connecting -- bailing out"
#define MF_1_MODEL_NOT_FOUND                    "Model {1} not found"
#define MF_1_MODEL_ERROR_1                      "Model error {1}"
#define MF_1_MODEL_ERROR_ON_PAIR_1              "Model error on pair {1}"
#define MF_2_MODEL_PARAMETERS_NOT_UPPERCASE_1_2 "model parameters should be uppercase:{1} {2}"
#define MF_2_ENTITY_1_NOT_FOUND_IN_MODEL_2      "Entity {1} not found in model {2}"
#define MF_1_UNKNOWN_NUMBER_FACTOR_IN_1         "Unknown number factor in: {1}"
#define MF_1_NOT_FOUND_IN_SOURCE_COLLECTION     "unable to find {1} in source collection"

#define MW_3_OVERWRITING_PARAM_1_OLD_2_NEW_3    "Overwriting {1} old <{2}> new <{3}>"
#define MW_1_CONNECTING_1_TO_ITSELF             "Connecting {1} to itself. This may be right, though"
#define MW_1_DUMMY_1_WITHOUT_CONNECTIONS        "Found dummy terminal {1} without connections"
#define MW_1_TERMINAL_1_WITHOUT_CONNECTIONS     "Found terminal {1} without connections"
#define MW_3_REMOVE_DEVICE_1_CONNECTED_ONLY_TO_RAILS_2_3 "Found device {1} connected only to railterminals {2}/{3}. Will be removed"
#define MW_1_DATA_1_NOT_FOUND                   "unable to find data named {1} in source collection"

// nld_mm5837.cpp

#define MW_1_FREQUENCY_OUTSIDE_OF_SPECS_1       "MM5837: Frequency outside of specs: {1}"

// nlid_proxy.cpp

#define MW_1_NO_POWER_TERMINALS_ON_DEVICE_1     "D/A Proxy: Found no valid combination of power terminals on device {1}"


#endif /* NL_ERRSTR_H_ */
