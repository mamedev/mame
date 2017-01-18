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

#define MF_1_DUPLICATE_NAME_DEVICE_LIST 		"Error adding {1} to device list. Duplicate name."
#define MF_1_UNKNOWN_TYPE_FOR_OBJECT 			"Unknown type for object {1} "
#define MF_2_NET_1_DUPLICATE_TERMINAL_2 		"net {1}: duplicate terminal {2}"
#define MF_2_REMOVE_TERMINAL_1_FROM_NET_2 		"Can not remove terminal {1} from net {2}."
#define MF_1_UNKNOWN_PARAM_TYPE 				"Can not determine param_type for {1}"
#define MF_2_ERROR_CONNECTING_1_TO_2  			"Error connecting {1} to {2}"
#define MF_0_NO_SOLVER  						"No solver found for this netlist although analog elements are present\n"

// nl_factory.cpp

#define MF_1_FACTORY_ALREADY_CONTAINS_1 		"factory already contains {1}"
#define MF_1_CLASS_1_NOT_FOUND                  "Class <{1}> not found!"


#endif /* NLBASE_H_ */
