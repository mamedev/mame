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
				logic_t *inout_proxied, detail::core_terminal_t *proxy_inout);

		logic_t &term_proxied() const { return *m_term_proxied; }
		detail::core_terminal_t &proxy_term() const { return *m_proxy_term; }

	protected:

	private:
		logic_t *m_term_proxied;
		detail::core_terminal_t *m_proxy_term;
	};

	// -----------------------------------------------------------------------------
	// nld_a_to_d_proxy
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT_DERIVED(base_a_to_d_proxy, base_proxy)
	{
	public:

		virtual logic_output_t &out() { return m_Q; }

	protected:

		nld_base_a_to_d_proxy(netlist_state_t &anetlist, const pstring &name,
				logic_input_t *in_proxied, detail::core_terminal_t *in_proxy);

	private:

		logic_output_t m_Q;

	};

	NETLIB_OBJECT_DERIVED(a_to_d_proxy, base_a_to_d_proxy)
	{
	public:
		nld_a_to_d_proxy(netlist_state_t &anetlist, const pstring &name, logic_input_t *in_proxied);

		analog_input_t m_I;

	protected:

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	private:
	};

	// -----------------------------------------------------------------------------
	// nld_base_d_to_a_proxy
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT_DERIVED(base_d_to_a_proxy, base_proxy)
	{
	public:

		virtual logic_input_t &in() { return m_I; }

	protected:
		nld_base_d_to_a_proxy(netlist_state_t &anetlist, const pstring &name,
				logic_output_t *out_proxied, detail::core_terminal_t &proxy_out);

		logic_input_t m_I;
	};

	NETLIB_OBJECT_DERIVED(d_to_a_proxy, base_d_to_a_proxy)
	{
	public:
		nld_d_to_a_proxy(netlist_state_t &anetlist, const pstring &name, logic_output_t *out_proxied);

	protected:

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	private:

		static constexpr const nl_double G_OFF = 1e-9;

		plib::unique_ptr<analog_output_t> m_GNDHack;  // FIXME: Long term, we need to connect proxy gnd to device gnd
		plib::unique_ptr<analog_output_t> m_VCCHack;  // FIXME: Long term, we need to connect proxy gnd to device gnd
		analog::NETLIB_NAME(twoterm) m_RP;
		analog::NETLIB_NAME(twoterm) m_RN;
		state_var<int> m_last_state;
		bool m_is_timestep;
};

	} //namespace devices
} // namespace netlist

#endif /* NLD_PROXY_H_ */
