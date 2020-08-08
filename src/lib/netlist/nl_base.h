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

//============================================================
//  MACROS / New Syntax
//============================================================

/// \brief Construct a netlist device name
///
#define NETLIB_NAME(chip) nld_ ## chip

/// \brief Start a netlist device class.
///
/// Used to start defining a netlist device class.
/// The simplest device without inputs or outputs would look like this:
///
///      NETLIB_OBJECT(some_object)
///      {
///      public:
///          NETLIB_CONSTRUCTOR(some_object) { }
///      };
///
///  Also refer to #NETLIB_CONSTRUCTOR.
#define NETLIB_OBJECT(name)                                                    \
class NETLIB_NAME(name) : public delegator_t<device_t>

/// \brief Start a derived netlist device class.
///
/// Used to define a derived device class based on plcass.
/// The simplest device without inputs or outputs would look like this:
///
///      NETLIB_OBJECT_DERIVED(some_object, parent_object)
///      {
///      public:
///          NETLIB_CONSTRUCTOR(some_object) { }
///      };
///
///  Also refer to #NETLIB_CONSTRUCTOR.
#define NETLIB_OBJECT_DERIVED(name, pclass)                                   \
class NETLIB_NAME(name) : public delegator_t<NETLIB_NAME(pclass)>



// Only used for analog objects like diodes and resistors

#define NETLIB_BASE_OBJECT(name)                                               \
class NETLIB_NAME(name) : public delegator_t<base_device_t>

#define NETLIB_CONSTRUCTOR_PASS(cname, ...)                                    \
	using this_type = NETLIB_NAME(cname);                                      \
	public: template <class CLASS> NETLIB_NAME(cname)(CLASS &owner, const pstring &name) \
	: base_type(owner, name, __VA_ARGS__)

/// \brief Used to define the constructor of a netlist device.
///
///  Use this to define the constructor of a netlist device. Please refer to
///  #NETLIB_OBJECT for an example.
#define NETLIB_CONSTRUCTOR(cname)                                              \
	using this_type = NETLIB_NAME(cname);                                      \
	public: template <class CLASS> NETLIB_NAME(cname)(CLASS &owner, const pstring &name)\
		: base_type(owner, name)

/// \brief Used to define the constructor of a netlist device and define a default model.
///
///
///      NETLIB_CONSTRUCTOR_MODEL(some_object, "TTL")
///      {
///      public:
///          NETLIB_CONSTRUCTOR(some_object) { }
///      };
///
#define NETLIB_CONSTRUCTOR_MODEL(cname, cmodel)                                              \
	using this_type = NETLIB_NAME(cname);                                      \
	public: template <class CLASS> NETLIB_NAME(cname)(CLASS &owner, const pstring &name) \
		: base_type(owner, name, cmodel)

/// \brief Define an extended constructor and add further parameters to it.
/// The macro allows to add further parameters to a device constructor. This is
/// normally used for sub-devices and system devices only.
#define NETLIB_CONSTRUCTOR_EX(cname, ...)                                      \
	using this_type = NETLIB_NAME(cname);                                      \
	public: template <class CLASS> NETLIB_NAME(cname)(CLASS &owner, const pstring &name, __VA_ARGS__) \
		: base_type(owner, name)

/// \brief Used to define the destructor of a netlist device.
/// The use of a destructor for netlist device should normally not be necessary.
#define NETLIB_DESTRUCTOR(name) public: virtual ~NETLIB_NAME(name)() noexcept override

/// \brief Add this to a device definition to mark the device as dynamic.
///
///  If NETLIB_IS_DYNAMIC(true) is added to the device definition the device
///  is treated as an analog dynamic device, i.e. \ref NETLIB_UPDATE_TERMINALSI
///  is called on a each step of the Newton-Raphson step
///  of solving the linear equations.
///
///  You may also use e.g. NETLIB_IS_DYNAMIC(m_func() != "") to only make the
///  device a dynamic device if parameter m_func is set.
///
///  \param expr boolean expression
///
#define NETLIB_IS_DYNAMIC(expr)                                                \
	public: virtual bool is_dynamic() const noexcept override { return expr; }

/// \brief Add this to a device definition to mark the device as a time-stepping device.
///
///  You have to implement NETLIB_TIMESTEP in this case as well. Currently, only
///  the capacitor and inductor devices uses this.
///
///  You may also use e.g. NETLIB_IS_TIMESTEP(m_func() != "") to only make the
///  device a dynamic device if parameter m_func is set. This is used by the
///  Voltage Source element.
///
///  Example:
///
///  \code
///  NETLIB_TIMESTEP_IS_TIMESTEP()
///  NETLIB_TIMESTEPI()
///  {
///      // Gpar should support convergence
///      const nl_fptype G = m_C.Value() / step +  m_GParallel;
///      const nl_fptype I = -G/// deltaV();
///      set(G, 0.0, I);
///  }
///  \endcode

#define NETLIB_IS_TIMESTEP(expr)                                               \
	public: virtual bool is_timestep() const  noexcept override { return expr; }

/// \brief Used to implement the time stepping code.
///
/// Please see \ref NETLIB_IS_TIMESTEP for an example.

#define NETLIB_TIMESTEPI()                                                     \
	public: virtual void timestep(timestep_type ts_type, nl_fptype step)  noexcept override

/// \brief Used to implement the body of the time stepping code.
///
/// Used when the implementation is outside the class definition
///
/// Please see \ref NETLIB_IS_TIMESTEP for an example.
///
/// \param cname Name of object as given to \ref NETLIB_OBJECT
///
#define NETLIB_TIMESTEP(cname)                                                 \
	void NETLIB_NAME(cname) :: timestep(timestep_type ts_type, nl_fptype step) noexcept

#define NETLIB_DELEGATE(name) nldelegate(&this_type :: name, this)

#define NETLIB_UPDATE_TERMINALSI() virtual void update_terminals() noexcept override
#define NETLIB_HANDLERI(name) void name() noexcept
#define NETLIB_UPDATE_PARAMI() virtual void update_param() noexcept override
#define NETLIB_RESETI() virtual void reset() override

#define NETLIB_SUB(chip) nld_ ## chip
#define NETLIB_SUB_UPTR(ns, chip) device_arena::unique_ptr< ns :: nld_ ## chip >

#define NETLIB_HANDLER(chip, name) void NETLIB_NAME(chip) :: name() noexcept

#define NETLIB_RESET(chip) void NETLIB_NAME(chip) :: reset(void)

#define NETLIB_UPDATE_PARAM(chip) void NETLIB_NAME(chip) :: update_param() noexcept

#define NETLIB_UPDATE_TERMINALS(chip) void NETLIB_NAME(chip) :: update_terminals() noexcept

//============================================================
// Namespace starts
//============================================================

namespace netlist
{

	namespace devices
	{
		// -----------------------------------------------------------------------------
		// mainclock
		// -----------------------------------------------------------------------------

		NETLIB_OBJECT(mainclock)
		{
			NETLIB_CONSTRUCTOR(mainclock)
			, m_Q(*this, "Q")
			, m_freq(*this, "FREQ", nlconst::magic(7159000.0 * 5))
			{
				m_inc = netlist_time::from_fp(plib::reciprocal(m_freq()*nlconst::two()));
			}

			NETLIB_RESETI();

			NETLIB_UPDATE_PARAMI()
			{
				m_inc = netlist_time::from_fp(plib::reciprocal(m_freq()*nlconst::two()));
			}

		public:
			logic_output_t m_Q; // NOLINT: needed in core
			netlist_time m_inc; // NOLINT: needed in core
		private:
			param_fp_t m_freq;
		};
	} // namespace devices


	// -----------------------------------------------------------------------------
	// power pins - not a device, but a helper
	// -----------------------------------------------------------------------------

	/// \brief Power pins class.
	///
	/// Power Pins are passive inputs. Delegate noop will silently ignore any
	/// updates.

	class nld_power_pins
	{
	public:
		using this_type = nld_power_pins;

		explicit nld_power_pins(device_t &owner)
		: m_VCC(owner, owner.logic_family()->vcc_pin(), NETLIB_DELEGATE(noop))
		, m_GND(owner, owner.logic_family()->gnd_pin(), NETLIB_DELEGATE(noop))
		{
		}

		// Some devices like the 74LS629 have two pairs of supply pins.
		explicit nld_power_pins(device_t &owner,
			const pstring &vcc, const pstring &gnd)
		: m_VCC(owner, vcc, NETLIB_DELEGATE(noop))
		, m_GND(owner, gnd, NETLIB_DELEGATE(noop))
		{
		}

		const analog_input_t &VCC() const noexcept
		{
			return m_VCC;
		}
		const analog_input_t &GND() const noexcept
		{
			return m_GND;
		}

	private:
		void noop() { }
		analog_input_t m_VCC;
		analog_input_t m_GND;
	};

	namespace devices
	{
		inline NETLIB_RESET(mainclock)
		{
			m_Q.net().set_next_scheduled_time(exec().time());
		}
	} // namespace devices


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

namespace plib
{
	template<typename X>
	struct ptype_traits<netlist::state_var<X>> : ptype_traits<X>
	{
	};
} // namespace plib



#endif // NLBASE_H_
