// license:GPL-2.0+
// copyright-holders:Couriersud

///
/// \file nl_interface.h
///
/// This header contains objects for interfacing with the netlist core.
///

#ifndef NLINTERFACE_H_
#define NLINTERFACE_H_

#include "nl_base.h"

#include <initializer_list>
#include <memory>
#include <stack>
#include <unordered_map>
#include <vector>

namespace netlist
{

	namespace interface
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
		/// 	const pstring pin(m_in);
		/// 	pstring dname = pstring("OUT_") + pin;
		///
		/// 	const auto lambda = [this](auto &in, netlist::nl_fptype val)
		/// 	{
		/// 		this->cpu()->update_icount(in.exec().time());
		/// 		this->m_delegate(val, this->cpu()->local_time());
		/// 		this->cpu()->check_mame_abort_slice();
		/// 	};
		///
		/// 	using lb_t = decltype(lambda);
		/// 	using cb_t = netlist::interface::NETLIB_NAME(analog_callback)<lb_t>;
		///
		/// 	parser.factory().add<cb_t, netlist::nl_fptype, lb_t>(dname,
		/// 		netlist::factory::properties("-", PSOURCELOC()), 1e-6, std::forward<lb_t>(lambda));
		///

		template <typename FUNC>
		class NETLIB_NAME(analog_callback) : public device_t
		{
		public:
			NETLIB_NAME(analog_callback)(netlist_state_t &anetlist,
				const pstring &name, nl_fptype threshold, FUNC &&func)
				: device_t(anetlist, name)
				, m_in(*this, "IN")
				, m_threshold(threshold)
				, m_last(*this, "m_last", 0)
				, m_func(func)
			{
			}

			NETLIB_RESETI()
			{
				m_last = 0.0;
			}

			NETLIB_UPDATEI()
			{
				const nl_fptype cur = m_in();
				if (plib::abs(cur - m_last) > m_threshold)
				{
					m_last = cur;
					m_func(*this, cur);
				}
			}

		private:
			analog_input_t m_in;
			nl_fptype      m_threshold;
			state_var<nl_fptype> m_last;
			FUNC m_func;
		};


		/// \brief logic_callback device
		///
		/// This device must be connected to a logic net. It has no power terminals
		/// and conversion with proxies will not work.
		///
		/// Background: This device may be inserted later into the driver and should
		/// not modify the resulting analog representation of the netlist.
		///
		/// If you get error messages on missing power terminals you have to use the
		/// analog callback device instead.

		template <typename FUNC>
		class NETLIB_NAME(logic_callback) : public device_t
		{
		public:
			NETLIB_NAME(logic_callback)(netlist_state_t &anetlist, const pstring &name, FUNC &&func)
				: device_t(anetlist, name)
				, m_in(*this, "IN")
				, m_func(func)
			{
			}

			NETLIB_UPDATEI()
			{
				const netlist_sig_t cur = m_in();
				m_func(*this, cur);
			}

		private:
			logic_input_t m_in;
			FUNC m_func;
		};

	} // namespace interface

} // namespace netlist


#endif // NLINTERFACE_H_
