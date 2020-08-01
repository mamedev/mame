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

#include "plib/palloc.h" // owned_ptr
#include "plib/pfunction.h"
#include "plib/plists.h"
#include "plib/pmempool.h"
#include "plib/ppmf.h"
#include "plib/pstate.h"
#include "plib/pstream.h"
#include "plib/ptimed_queue.h"
#include "plib/ptypes.h"

#include "nl_errstr.h"
#include "nl_factory.h"
#include "nltypes.h"

#include <initializer_list>
#include <unordered_map>
#include <vector>

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

#if 0
#define NETLIB_UPDATEI() virtual void update() noexcept override
#define NETLIB_UPDATE(chip) NETLIB_HANDLER(chip, update)
#endif

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
	// netlist_t
	// -----------------------------------------------------------------------------

	class netlist_t // NOLINT(clang-analyzer-optin.performance.Padding)
	{
	public:

		explicit netlist_t(netlist_state_t &state, const pstring &aname);

		PCOPYASSIGNMOVE(netlist_t, delete)

		virtual ~netlist_t() noexcept = default;

		// run functions

		netlist_time_ext time() const noexcept { return m_time; }

		void process_queue(netlist_time_ext delta) noexcept;
		void abort_current_queue_slice() noexcept
		{
			if (!NL_USE_QUEUE_STATS || !m_use_stats)
				m_queue.retime<false>(detail::queue_t::entry_t(m_time, nullptr));
			else
				m_queue.retime<true>(detail::queue_t::entry_t(m_time, nullptr));
		}

		const detail::queue_t &queue() const noexcept { return m_queue; }

		template<typename... Args>
		void qpush(Args&&...args) noexcept
		{
			if (!NL_USE_QUEUE_STATS || !m_use_stats)
				m_queue.emplace<false>(std::forward<Args>(args)...); // NOLINT(performance-move-const-arg)
			else
				m_queue.emplace<true>(std::forward<Args>(args)...); // NOLINT(performance-move-const-arg)
		}

		template <class R>
		void qremove(const R &elem) noexcept
		{
			if (!NL_USE_QUEUE_STATS || !m_use_stats)
				m_queue.remove<false>(elem);
			else
				m_queue.remove<true>(elem);
		}

		// Control functions

		void stop();
		void reset();

		// only used by nltool to create static c-code
		devices::NETLIB_NAME(solver) *solver() const noexcept { return m_solver; }

		// force late type resolution
		template <typename X = devices::NETLIB_NAME(solver)>
		nl_fptype gmin(X *solv = nullptr) const noexcept
		{
			plib::unused_var(solv);
			return static_cast<X *>(m_solver)->gmin();
		}

		netlist_state_t &nlstate() noexcept { return m_state; }
		const netlist_state_t &nlstate() const noexcept { return m_state; }

		log_type & log() noexcept { return m_state.log(); }
		const log_type &log() const noexcept { return m_state.log(); }

		void print_stats() const;
		bool use_stats() const { return m_use_stats; }

		bool stats_enabled() const noexcept { return m_use_stats; }
		void enable_stats(bool val) noexcept { m_use_stats = val; }

	private:

		template <bool KEEP_STATS>
		void process_queue_stats(netlist_time_ext delta) noexcept;

		netlist_state_t &                   m_state;
		devices::NETLIB_NAME(solver) *      m_solver;

		// mostly rw
		//PALIGNAS(16)
		netlist_time_ext                    m_time;
		devices::NETLIB_NAME(mainclock) *   m_mainclock;

		//PALIGNAS_CACHELINE()
		//PALIGNAS(16)
		detail::queue_t                     m_queue;
		bool                                m_use_stats;
		// performance
		plib::pperftime_t<true>             m_stat_mainloop;
		plib::pperfcount_t<true>            m_perf_out_processed;
	};

	// -----------------------------------------------------------------------------
	// Support classes for devices
	// -----------------------------------------------------------------------------


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

	// -----------------------------------------------------------------------------
	// Hot section
	//
	// Any changes below will impact performance.
	// -----------------------------------------------------------------------------

	// -----------------------------------------------------------------------------
	// logic_input_t
	// -----------------------------------------------------------------------------
#if 0
	inline void logic_input_t::inactivate() noexcept
	{
		if (!is_state(STATE_INP_PASSIVE))
		{
			set_state(STATE_INP_PASSIVE);
			net().remove_from_active_list(*this);
		}
	}

	inline void logic_input_t::activate() noexcept
	{
		if (is_state(STATE_INP_PASSIVE))
		{
			net().add_to_active_list(*this);
			set_state(STATE_INP_ACTIVE);
		}
	}

	inline void logic_input_t::activate_hl() noexcept
	{
		if (is_state(STATE_INP_PASSIVE))
		{
			net().add_to_active_list(*this);
			set_state(STATE_INP_HL);
		}
	}

	inline void logic_input_t::activate_lh() noexcept
	{
		if (is_state(STATE_INP_PASSIVE))
		{
			net().add_to_active_list(*this);
			set_state(STATE_INP_LH);
		}
	}
#endif
	inline void detail::net_t::push_to_queue(const netlist_time &delay) noexcept
	{
		if (has_connections())
		{
			if (!!is_queued())
				exec().qremove(this);

			const auto nst(exec().time() + delay);
			m_next_scheduled_time = nst;

			if (!m_list_active.empty())
			{
				m_in_queue = queue_status::QUEUED;
				exec().qpush(nst, this);
			}
			else
			{
				m_in_queue = queue_status::DELAYED_DUE_TO_INACTIVE;
				update_inputs();
			}
		}
	}

	inline void detail::net_t::add_to_active_list(core_terminal_t &term) noexcept
	{
		if (!m_list_active.empty())
		{
			term.set_copied_input(m_cur_Q);
			m_list_active.push_front(&term);
		}
		else
		{
			m_list_active.push_front(&term);
			railterminal().device().do_inc_active();
			if (m_in_queue == queue_status::DELAYED_DUE_TO_INACTIVE)
			{
				if (m_next_scheduled_time > exec().time())
				{
					m_in_queue = queue_status::QUEUED;     // pending
					exec().qpush(m_next_scheduled_time, this);
				}
				else
				{
					m_in_queue = queue_status::DELIVERED;
					m_cur_Q = m_new_Q;
				}
				update_inputs();
			}
			else
				term.set_copied_input(m_cur_Q);
		}
	}

	inline void detail::net_t::remove_from_active_list(core_terminal_t &term) noexcept
	{
		gsl_Expects(!m_list_active.empty());
		m_list_active.remove(&term);
		if (m_list_active.empty())
			railterminal().device().do_dec_active();
	}

#if 0
	inline nl_fptype terminal_t::operator ()() const noexcept
	{
		return net().Q_Analog();
	}

	inline const analog_net_t & analog_t::net() const noexcept
	{
		return plib::downcast<const analog_net_t &>(core_terminal_t::net());
	}

	inline analog_net_t & analog_t::net() noexcept
	{
		return plib::downcast<analog_net_t &>(core_terminal_t::net());
	}


	inline logic_net_t & logic_t::net() noexcept
	{
		return plib::downcast<logic_net_t &>(core_terminal_t::net());
	}

	inline const logic_net_t & logic_t::net() const noexcept
	{
		return plib::downcast<const logic_net_t &>(core_terminal_t::net());
	}
	inline netlist_sig_t logic_input_t::operator()() const noexcept
	{
		nl_assert(terminal_state() != STATE_INP_PASSIVE);
#if NL_USE_COPY_INSTEAD_OF_REFERENCE
		return m_Q;
#else
		return net().Q();
#endif
	}

	inline nl_fptype analog_input_t::Q_Analog() const noexcept
	{
		return net().Q_Analog();
	}

	inline void analog_output_t::push(nl_fptype val) noexcept
	{
		if (val != m_my_net.Q_Analog())
		{
			m_my_net.set_Q_Analog(val);
			m_my_net.toggle_and_push_to_queue(netlist_time::quantum());
		}
	}
#endif

	inline netlist_t &detail::device_object_t::exec() noexcept
	{
		return m_device->exec();
	}

	inline const netlist_t &detail::device_object_t::exec() const noexcept
	{
		return m_device->exec();
	}

	template <bool KEEP_STATS, typename T, typename S>
	inline void detail::net_t::process(T mask, const S &sig) noexcept
	{
		m_cur_Q = sig;

		if (KEEP_STATS)
		{
			for (auto & p : m_list_active)
			{
				p.set_copied_input(sig);
				auto *stats(p.device().stats());
				stats->m_stat_call_count.inc();
				if ((p.terminal_state() & mask))
				{
					auto g(stats->m_stat_total_time.guard());
					p.run_delegate();
				}
			}
		}
		else
		{
			for (auto &p : m_list_active)
			{
				p.set_copied_input(sig);
				if ((p.terminal_state() & mask) != 0)
					p.run_delegate();
			}
		}
	}

	template <bool KEEP_STATS>
	inline void detail::net_t::update_devs() noexcept
	{
		nl_assert(this->is_rail_net());

		m_in_queue = queue_status::DELIVERED; // mark as taken ...
		if (m_new_Q ^ m_cur_Q)
		{
			process<KEEP_STATS>((m_new_Q << core_terminal_t::INP_LH_SHIFT)
				| (m_cur_Q << core_terminal_t::INP_HL_SHIFT), m_new_Q);
		}
	}

	template <bool KEEP_STATS>
	inline void netlist_t::process_queue_stats(const netlist_time_ext delta) noexcept
	{
		netlist_time_ext stop(m_time + delta);

		qpush(stop, nullptr);

		if (m_mainclock == nullptr)
		{
			m_time = m_queue.top().exec_time();
			detail::net_t *obj(m_queue.top().object());
			m_queue.pop();

			while (obj != nullptr)
			{
				obj->template update_devs<KEEP_STATS>();
				if (KEEP_STATS)
					m_perf_out_processed.inc();
				const detail::queue_t::entry_t *top = &m_queue.top();
				m_time = top->exec_time();
				obj = top->object();
				m_queue.pop();
			}
		}
		else
		{
			logic_net_t &mc_net(m_mainclock->m_Q.net());
			const netlist_time inc(m_mainclock->m_inc);
			netlist_time_ext mc_time(mc_net.next_scheduled_time());

			do
			{
				const detail::queue_t::entry_t *top = &m_queue.top();
				while (top->exec_time() > mc_time)
				{
					m_time = mc_time;
					mc_net.toggle_new_Q();
					mc_net.update_devs<KEEP_STATS>();
					top = &m_queue.top();
					mc_time += inc;
				}

				m_time = top->exec_time();
				auto *const obj(top->object());
				m_queue.pop();
				if (obj != nullptr)
					obj->template update_devs<KEEP_STATS>();
				else
					break;
				if (KEEP_STATS)
					m_perf_out_processed.inc();
			} while (true);

			mc_net.set_next_scheduled_time(mc_time);
		}
	}

	inline void netlist_t::process_queue(netlist_time_ext delta) noexcept
	{
		if (!m_use_stats)
			process_queue_stats<false>(delta);
		else
		{
			auto sm_guard(m_stat_mainloop.guard());
			process_queue_stats<true>(delta);
		}
	}



	// -----------------------------------------------------------------------------
	// inline implementations - cold
	// -----------------------------------------------------------------------------
#if 0
	template<typename T, typename... Args>
	inline device_arena::unique_ptr<T> detail::netlist_object_t::make_pool_object(Args&&... args)
	{
		return state().make_pool_object<T>(std::forward<Args>(args)...);
	}
#endif
	inline void param_t::update_param() noexcept
	{
		device().update_param();
	}

	template <typename T>
	param_num_t<T>::param_num_t(core_device_t &device, const pstring &name, const T val)
	: param_t(device, name)
	, m_param(val)
	{
		bool found = false;
		pstring p = this->get_initial(&device, &found);
		if (found)
		{
			plib::pfunction<nl_fptype> func;
			func.compile_infix(p, {});
			auto valx = func.evaluate();
			if (plib::is_integral<T>::value)
				if (plib::abs(valx - plib::trunc(valx)) > nlconst::magic(1e-6))
					throw nl_exception(MF_INVALID_NUMBER_CONVERSION_1_2(device.name() + "." + name, p));
			m_param = plib::narrow_cast<T>(valx);
		}

		device.state().save(*this, m_param, this->name(), "m_param");
	}

	template <typename T>
	param_enum_t<T>::param_enum_t(core_device_t &device, const pstring &name, const T val)
	: param_t(device, name)
	, m_param(val)
	{
		bool found = false;
		pstring p = this->get_initial(&device, &found);
		if (found)
		{
			T temp(val);
			bool ok = temp.set_from_string(p);
			if (!ok)
			{
				device.state().log().fatal(MF_INVALID_ENUM_CONVERSION_1_2(name, p));
				throw nl_exception(MF_INVALID_ENUM_CONVERSION_1_2(name, p));
			}
			m_param = temp;
		}

		device.state().save(*this, m_param, this->name(), "m_param");
	}

	template <typename ST, std::size_t AW, std::size_t DW>
	param_rom_t<ST, AW, DW>::param_rom_t(core_device_t &device, const pstring &name)
	: param_data_t(device, name)
	{
		auto f = this->stream();
		if (!f.empty())
		{
			plib::istream_read(f.stream(), m_data.data(), 1<<AW);
			// FIXME: check for failbit if not in validation.
		}
		else
			device.state().log().warning(MW_ROM_NOT_FOUND(str()));
	}

#if 0
	template<class O, class C, typename... Args>
	void base_device_t::create_and_register_subdevice(O &owner, const pstring &name, device_arena::unique_ptr<C> &dev, Args&&... args)
	{
		dev = state().make_pool_object<C>(owner, name, std::forward<Args>(args)...);
	}
#endif
	inline solver::matrix_solver_t *analog_t::solver() const noexcept
	{
		return (this->has_net() ? net().solver() : nullptr);
	}


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
