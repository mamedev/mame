// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLBASE_H_
#define NLBASE_H_

///
/// \file nl_base.h
///

#ifdef NL_PROHIBIT_BASEH_INCLUDE
#error "nl_base.h included. Please correct."
#endif

#include "core/analog.h"
#include "core/base_objects.h"
#include "core/device.h"
#include "core/logic.h"
#include "core/logic_family.h"
#include "core/netlist_state.h"
#include "core/nets.h"
#include "core/object_array.h"
#include "core/param.h"
#include "core/state_var.h"
#include "core/exec.h"
#include "core/device_macros.h"
#include "core/devices.h"

//============================================================
// Namespace starts
//============================================================

namespace netlist
{

	extern template struct state_var<std::uint8_t>;
	extern template struct state_var<std::uint16_t>;
	extern template struct state_var<std::uint32_t>;
	extern template struct state_var<std::uint64_t>;
	extern template struct state_var<std::int8_t>;
	extern template struct state_var<std::int16_t>;
	extern template struct state_var<std::int32_t>;
	extern template struct state_var<std::int64_t>;
	extern template struct state_var<bool>;

	extern template class param_num_t<std::uint8_t>;
	extern template class param_num_t<std::uint16_t>;
	extern template class param_num_t<std::uint32_t>;
	extern template class param_num_t<std::uint64_t>;
	extern template class param_num_t<std::int8_t>;
	extern template class param_num_t<std::int16_t>;
	extern template class param_num_t<std::int32_t>;
	extern template class param_num_t<std::int64_t>;
	extern template class param_num_t<float>;
	extern template class param_num_t<double>;
	extern template class param_num_t<long double>;
	extern template class param_num_t<bool>;

	extern template class param_model_t::value_base_t<float>;
	extern template class param_model_t::value_base_t<double>;
	extern template class param_model_t::value_base_t<long double>;

	extern template class object_array_t<logic_input_t, 1>;
	extern template class object_array_t<logic_input_t, 2>;
	extern template class object_array_t<logic_input_t, 3>;
	extern template class object_array_t<logic_input_t, 4>;
	extern template class object_array_t<logic_input_t, 5>;
	extern template class object_array_t<logic_input_t, 6>;
	extern template class object_array_t<logic_input_t, 7>;
	extern template class object_array_t<logic_input_t, 8>;

	extern template class object_array_t<logic_output_t, 1>;
	extern template class object_array_t<logic_output_t, 2>;
	extern template class object_array_t<logic_output_t, 3>;
	extern template class object_array_t<logic_output_t, 4>;
	extern template class object_array_t<logic_output_t, 5>;
	extern template class object_array_t<logic_output_t, 6>;
	extern template class object_array_t<logic_output_t, 7>;
	extern template class object_array_t<logic_output_t, 8>;

} // namespace netlist

#endif // NLBASE_H_
