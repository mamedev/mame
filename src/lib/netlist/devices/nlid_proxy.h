// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlid_proxy.h
 *
 * netlist proxy devices
 *
 * This file contains internal headers
 */

#ifndef NLID_PROXY_H_
#define NLID_PROXY_H_

#include <vector>

#include "nl_setup.h"
#include "nl_base.h"
#include "nl_factory.h"
#include "analog/nld_twoterm.h"

namespace netlist
{
	namespace devices
	{

	// -----------------------------------------------------------------------------
	// nld_base_proxy
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(base_proxy)
	{
	public:
		nld_base_proxy(netlist_t &anetlist, const pstring &name,
				logic_t *inout_proxied, detail::core_terminal_t *proxy_inout)
				: device_t(anetlist, name)
		{
			m_logic_family = inout_proxied->logic_family();
			m_term_proxied = inout_proxied;
			m_proxy_term = proxy_inout;
		}

		virtual ~nld_base_proxy() {}

		logic_t &term_proxied() const { return *m_term_proxied; }
		detail::core_terminal_t &proxy_term() const { return *m_proxy_term; }

	protected:
		const logic_family_desc_t *m_logic_family;

		virtual const logic_family_desc_t &logic_family() const
		{
			return *m_logic_family;
		}

	private:
		logic_t *m_term_proxied;
		detail::core_terminal_t *m_proxy_term;
	};

	// -----------------------------------------------------------------------------
	// nld_a_to_d_proxy
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT_DERIVED(a_to_d_proxy, base_proxy)
	{
	public:
		nld_a_to_d_proxy(netlist_t &anetlist, const pstring &name, logic_input_t *in_proxied)
				: nld_base_proxy(anetlist, name, in_proxied, &m_I)
		, m_I(*this, "I")
		, m_Q(*this, "Q")
		{
		}

		virtual ~nld_a_to_d_proxy() {}

		analog_input_t m_I;
		logic_output_t m_Q;

	protected:

		NETLIB_RESETI() { }

		NETLIB_UPDATEI()
		{
			nl_assert(m_logic_family != nullptr);
			if (m_I.Q_Analog() > logic_family().m_high_thresh_V)
				m_Q.push(1, NLTIME_FROM_NS(1));
			else if (m_I.Q_Analog() < logic_family().m_low_thresh_V)
				m_Q.push(0, NLTIME_FROM_NS(1));
			else
			{
				// do nothing
			}
		}
	private:
	};

	// -----------------------------------------------------------------------------
	// nld_base_d_to_a_proxy
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT_DERIVED(base_d_to_a_proxy, base_proxy)
	{
	public:
		virtual ~nld_base_d_to_a_proxy() {}

		virtual logic_input_t &in() { return m_I; }

	protected:
		nld_base_d_to_a_proxy(netlist_t &anetlist, const pstring &name,
				logic_output_t *out_proxied, detail::core_terminal_t &proxy_out)
		: nld_base_proxy(anetlist, name, out_proxied, &proxy_out)
		, m_I(*this, "I")
		{
		}

		logic_input_t m_I;

	private:
	};

	NETLIB_OBJECT_DERIVED(d_to_a_proxy, base_d_to_a_proxy)
	{
	public:
		nld_d_to_a_proxy(netlist_t &anetlist, const pstring &name, logic_output_t *out_proxied);
		virtual ~nld_d_to_a_proxy() {}

	protected:

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	private:
		analog_output_t m_GNDHack;  // FIXME: Long term, we need to connect proxy gnd to device gnd
		NETLIB_SUB(twoterm) m_RV;
		state_var<int> m_last_state;
		bool m_is_timestep;
	};


	class factory_lib_entry_t : public base_factory_t
	{
		P_PREVENT_COPYING(factory_lib_entry_t)
	public:

		factory_lib_entry_t(setup_t &setup, const pstring &name, const pstring &classname,
				const pstring &def_param)
		: base_factory_t(name, classname, def_param), m_setup(setup) {  }

		class wrapper : public device_t
		{
		public:
			wrapper(const pstring &devname, netlist_t &anetlist, const pstring &name)
			: device_t(anetlist, name), m_devname(devname)
			{
				anetlist.setup().namespace_push(name);
				anetlist.setup().include(m_devname);
				anetlist.setup().namespace_pop();
			}
		protected:
			NETLIB_RESETI() { }
			NETLIB_UPDATEI() { }

			pstring m_devname;
		};

		plib::owned_ptr<device_t> Create(netlist_t &anetlist, const pstring &name) override
		{
			return plib::owned_ptr<device_t>::Create<wrapper>(this->name(), anetlist, name);
		}

	private:
		setup_t &m_setup;
	};


	} //namespace devices
} // namespace netlist

#endif /* NLD_PROXY_H_ */
