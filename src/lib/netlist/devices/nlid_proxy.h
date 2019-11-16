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

#include "netlist/analog/nlid_twoterm.h"
#include "netlist/nl_setup.h"

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
		nld_base_proxy(netlist_state_t &anetlist, const pstring &name,
				logic_t *inout_proxied);

		// only used during setup
		virtual detail::core_terminal_t &proxy_term() noexcept = 0;

	protected:
		// FIXME: these should be core_terminal_t and only used for connecting
		//        inputs. Fix, once the ugly hacks have been removed
		analog_t *m_tp;
		analog_t *m_tn;

	};

	// -----------------------------------------------------------------------------
	// nld_a_to_d_proxy
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT_DERIVED(base_a_to_d_proxy, base_proxy)
	{
	public:
		virtual logic_output_t &out() noexcept = 0;

	protected:
		nld_base_a_to_d_proxy(netlist_state_t &anetlist, const pstring &name,
				logic_input_t *in_proxied);

	};

	NETLIB_OBJECT_DERIVED(a_to_d_proxy, base_a_to_d_proxy)
	{
	public:
		nld_a_to_d_proxy(netlist_state_t &anetlist, const pstring &name,
			logic_input_t *in_proxied);

		logic_output_t &out() noexcept override { return m_Q; }

		detail::core_terminal_t &proxy_term() noexcept override
		{
			return m_I;
		}

	protected:
		NETLIB_RESETI();
		NETLIB_UPDATEI();

	private:
		logic_output_t m_Q;
		analog_input_t m_I;
	};

	// -----------------------------------------------------------------------------
	// nld_base_d_to_a_proxy
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT_DERIVED(base_d_to_a_proxy, base_proxy)
	{
	public:
		// only used in setup
		virtual logic_input_t &in() noexcept = 0;

	protected:
		nld_base_d_to_a_proxy(netlist_state_t &anetlist, const pstring &name,
				logic_output_t *out_proxied);

	};

	NETLIB_OBJECT_DERIVED(d_to_a_proxy, base_d_to_a_proxy)
	{
	public:
		nld_d_to_a_proxy(netlist_state_t &anetlist, const pstring &name,
			logic_output_t *out_proxied);

		logic_input_t &in() noexcept override { return m_I; }

		detail::core_terminal_t &proxy_term() noexcept override
		{
			return m_RN.m_P;
		}

	protected:

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	private:

		static constexpr const nl_fptype G_OFF = nlconst::magic(1e-9);

		logic_input_t m_I;
		analog::NETLIB_NAME(twoterm) m_RP;
		analog::NETLIB_NAME(twoterm) m_RN;
		state_var<int> m_last_state;
		bool m_is_timestep;
	};

} // namespace devices
} // namespace netlist

#endif /* NLD_PROXY_H_ */
