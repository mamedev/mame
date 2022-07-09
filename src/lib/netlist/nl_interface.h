// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file nl_interface.h
///
/// This header contains objects for interfacing with the netlist core.
///

#ifndef NLINTERFACE_H_
#define NLINTERFACE_H_

#include "nl_setup.h"

#include "core/analog.h"
#include "core/device.h"
#include "core/device_macros.h"
#include "core/logic.h"
#include "core/setup.h"

#include <array>
#include <memory>

namespace netlist::interface
{
	/// \brief analog_callback device
	///
	/// This device is used to call back into the application which
	/// is controlling the execution of the netlist.
	///
	/// The device will call the provided lambda with a reference to
	/// itself and the current value of the net it is connected to.
	///
	/// The following code is an example on how to add the device to
	/// the netlist factory.
	///
	/// ```
	///     const pstring pin(m_in);
	///     pstring dname = pstring("OUT_") + pin;
	///
	///     const auto lambda = [this](auto &in, netlist::nl_fptype val)
	///     {
	///         this->cpu()->update_icount(in.exec().time());
	///         this->m_delegate(val, this->cpu()->local_time());
	///         this->cpu()->check_mame_abort_slice();
	///     };
	///
	///     using lb_t = decltype(lambda);
	///     using cb_t =
	///     netlist::interface::NETLIB_NAME(analog_callback)<lb_t>;
	///
	///     parser.factory().add<cb_t, netlist::nl_fptype, lb_t>(dname,
	///         netlist::factory::properties("-", PSOURCELOC()), 1e-6,
	///         std::forward<lb_t>(lambda));
	/// ```

	template <typename FUNC>
	class nld_analog_callback : public device_t
	{
	public:
		nld_analog_callback(constructor_param_t data, nl_fptype threshold,
							FUNC &&func)
		: device_t(data)
		, m_in(*this, "IN", NETLIB_DELEGATE(in))
		, m_threshold(threshold)
		, m_last(*this, "m_last", 0)
		, m_func(func)
		{
		}

		NETLIB_RESETI() { m_last = 0.0; }

		NETLIB_HANDLERI(in)
		{
			const nl_fptype cur = m_in();
			if (plib::abs(cur - m_last) > m_threshold)
			{
				m_last = cur;
				m_func(*this, cur);
			}
		}

	private:
		analog_input_t       m_in;
		nl_fptype            m_threshold;
		state_var<nl_fptype> m_last;
		FUNC                 m_func;
	};

	/// \brief logic_callback device
	///
	/// This device must be connected to a logic net. It has no power
	/// terminals and conversion with proxies will not work.
	///
	/// Background: This device may be inserted later into the driver and
	/// should not modify the resulting analog representation of the
	/// netlist.
	///
	/// If you get error messages on missing power terminals you have to use
	/// the analog callback device instead.

	template <typename FUNC>
	class nld_logic_callback : public device_t
	{
	public:
		nld_logic_callback(constructor_param_t data, FUNC &&func)
		: device_t(data)
		, m_in(*this, "IN", NETLIB_DELEGATE(in))
		, m_func(func)
		{
		}

		NETLIB_HANDLERI(in)
		{
			const netlist_sig_t cur = m_in();
			m_func(*this, cur);
		}

	private:
		logic_input_t m_in;
		FUNC          m_func;
	};

	/// \brief Set parameters to buffers contents at regular intervals
	///
	/// This device will update a parameter from a buffer passed to the
	/// device. It is the responsibility of the controlling application to
	/// ensure that the buffer is filled at regular intervals.
	///
	/// \tparam T The buffer type
	/// \tparam N Maximum number of supported buffers
	///
	template <typename T>
	class nld_buffered_param_setter : public device_t
	{
	public:
		nld_buffered_param_setter(constructor_param_t data)
		: device_t(data)
		, m_sample_time(netlist_time::zero())
		, m_feedback(*this, "FB", NETLIB_DELEGATE(feedback)) // clock part
		, m_Q(*this, "Q")
		, m_pos(0)
		, m_samples(0)
		, m_param_name(*this, "CHAN", "")
		, m_param_mult(*this, "MULT", 1.0)
		, m_param_offset(*this, "OFFSET", 0.0)
		, m_param(nullptr)
		, m_id(*this, "ID", 0)
		{
			connect("FB", "Q");
			m_buffer = nullptr;
		}

	protected:
		NETLIB_RESETI() {}

		NETLIB_HANDLERI(feedback)
		{
			if (m_pos < m_samples)
			{
				// check if called outside of stream_update
				if (m_buffer != nullptr)
				{
					const nl_fptype v = (*m_buffer)[m_pos];
					m_param_setter(v * m_param_mult() + m_param_offset());
				}
			}
			else
			{
				// FIXME: The logic has a rounding issue because time
				//        resolution divided  by 48,000 is not a natural
				//        number. The fractional part adds up to one samples
				//        every 13 seconds for 100 ps resolution. Fixing
				//        this is possible but complicated and expensive.
			}
			m_pos++;

			m_Q.net().toggle_and_push_to_queue(m_sample_time);
		}

	public:
		/// \brief resolve parameter names to pointers
		///
		/// This function must be called after all device were constructed
		/// but before reset is called.
		void resolve_params(netlist_time sample_time)
		{
			m_pos = 0;
			m_sample_time = sample_time;
			if (m_param_name() != pstring(""))
			{
				param_t *p = &state()
								  .setup()
								  .find_param(m_param_name())
								  .param();
				m_param = p;
				if (plib::dynamic_downcast<param_fp_t *>(p))
					m_param_setter = setter_t(
						&NETLIB_NAME(buffered_param_setter)::setter<param_fp_t>,
						this);
				else if (plib::dynamic_downcast<param_logic_t *>(p))
					m_param_setter = setter_t(
						&NETLIB_NAME(buffered_param_setter)::setter<
							param_logic_t>,
						this);
			}
		}

		void buffer_reset(netlist_time sample_time, std::size_t num_samples,
						  T *inputs)
		{
			m_samples = num_samples;
			m_sample_time = sample_time;
			m_pos = 0;
			m_buffer = inputs;
		}

		std::size_t id() const { return m_id; }

	private:
		using setter_t = plib::pmfp<void(nl_fptype)>;

		template <typename S>
		void setter(nl_fptype v)
		{
			static_cast<S *>(m_param)->set(v);
		}

		netlist_time m_sample_time;

		logic_input_t  m_feedback;
		logic_output_t m_Q;

		std::size_t m_pos;
		std::size_t m_samples;

		param_str_t              m_param_name;
		param_fp_t               m_param_mult;
		param_fp_t               m_param_offset;
		param_t *                m_param;
		setter_t                 m_param_setter;
		T *                      m_buffer;
		param_num_t<std::size_t> m_id;
	};

} // namespace netlist::interface

#endif // NLINTERFACE_H_
