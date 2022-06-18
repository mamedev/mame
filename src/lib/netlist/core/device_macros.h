// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef NL_CORE_DEVICE_MACROS_H_
#define NL_CORE_DEVICE_MACROS_H_

///
/// \file device_macros.h
///

// -----------------------------------------------------------------------------
//  MACROS / New Syntax
// -----------------------------------------------------------------------------

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
	class NETLIB_NAME(name)                                                    \
	: public device_t

/// \brief Used to define the constructor of a netlist device.
///
///  Use this to define the constructor of a netlist device. Please refer to
///  #NETLIB_OBJECT for an example.
#define NETLIB_CONSTRUCTOR(cname)                                              \
public:                                                                        \
	NETLIB_NAME(cname)(constructor_param_t data)                               \
	: device_t(data)

/// \brief Used to define the constructor of a netlist device and define a
/// default model.
///
///
///      NETLIB_CONSTRUCTOR_MODEL(some_object, "TTL")
///      {
///      public:
///          NETLIB_CONSTRUCTOR(some_object) { }
///      };
///
#define NETLIB_CONSTRUCTOR_MODEL(cname, cmodel)                                \
public:                                                                        \
	NETLIB_NAME(cname)(constructor_param_t data)                               \
	: device_t(data, cmodel)

/// \brief Used to define the destructor of a netlist device.
/// The use of a destructor for netlist device should normally not be necessary.
#define NETLIB_DESTRUCTOR(name)                                                \
public:                                                                        \
	virtual ~NETLIB_NAME(name)() noexcept override

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
public:                                                                        \
	virtual bool is_dynamic() const noexcept override { return expr; }

/// \brief Add this to a device definition to mark the device as a time-stepping
/// device.
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
public:                                                                        \
	virtual bool is_time_step() const noexcept override { return expr; }

/// \brief Used to implement the time stepping code.
///
/// Please see \ref NETLIB_IS_TIMESTEP for an example.

#define NETLIB_TIMESTEPI()                                                     \
public:                                                                        \
	virtual void time_step(time_step_type ts_type,                             \
		nl_fptype                         step) noexcept override

/// \brief Used to implement the body of the time stepping code.
///
/// Used when the implementation is outside the class definition
///
/// Please see \ref NETLIB_IS_TIMESTEP for an example.
///
/// \param cname Name of object as given to \ref NETLIB_OBJECT
///
#define NETLIB_TIMESTEP(cname)                                                 \
	void NETLIB_NAME(cname)::time_step(time_step_type ts_type,                 \
		nl_fptype                                     step) noexcept

//#define NETLIB_DELEGATE(name) nl_delegate(&this_type :: name, this)
#define NETLIB_DELEGATE(name)                                                  \
	nl_delegate(&std::remove_pointer_t<decltype(this)>::name, this)

#define NETLIB_DELEGATE_NOOP()                                                 \
	nl_delegate(&core_device_t::handler_noop,                                  \
		static_cast<core_device_t *>(this))

#define NETLIB_UPDATE_TERMINALSI()                                             \
	virtual void update_terminals() noexcept override
#define NETLIB_HANDLERI(name) void name() noexcept
#define NETLIB_UPDATE_PARAMI() virtual void update_param() noexcept override
#define NETLIB_RESETI() virtual void reset() override

#define NETLIB_SUB(chip) sub_device_wrapper<nld_##chip>
#define NETLIB_SUB_NS(ns, chip) sub_device_wrapper<ns ::nld_##chip>

#define NETLIB_SUB_UPTR(ns, chip) device_arena::unique_ptr<ns ::nld_##chip>

#define NETLIB_HANDLER(chip, name) void NETLIB_NAME(chip)::name() noexcept

#define NETLIB_RESET(chip) void NETLIB_NAME(chip)::reset(void)

#define NETLIB_UPDATE_PARAM(chip)                                              \
	void NETLIB_NAME(chip)::update_param() noexcept

#define NETLIB_UPDATE_TERMINALS(chip)                                          \
	void NETLIB_NAME(chip)::update_terminals() noexcept

#endif // NL_CORE_DEVICE_MACROS_H_
