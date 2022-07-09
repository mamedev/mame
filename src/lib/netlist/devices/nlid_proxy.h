// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef NLID_PROXY_H_
#define NLID_PROXY_H_

///
/// \file nlid_proxy.h
///
/// netlist proxy devices
///
/// This file contains internal headers
///


#include "analog/nlid_twoterm.h"
#include "nl_base.h"

namespace netlist::devices {

	// -----------------------------------------------------------------------------
	// nld_base_proxy
	// -----------------------------------------------------------------------------

	class nld_base_proxy : public device_t
	{
	public:
		nld_base_proxy(device_param_t data, const logic_t *inout_proxied);

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

	class nld_base_a_to_d_proxy : public nld_base_proxy
	{
	public:
		virtual logic_output_t &out() noexcept = 0;

	protected:
		nld_base_a_to_d_proxy(device_param_t data, const logic_input_t *in_proxied);

	};

	class nld_a_to_d_proxy : public nld_base_a_to_d_proxy
	{
	public:
		nld_a_to_d_proxy(device_param_t data, const logic_input_t *in_proxied);

		logic_output_t &out() noexcept override { return m_Q; }

		detail::core_terminal_t &proxy_term() noexcept override
		{
			return m_I;
		}

	protected:
		//NETLIB_RESETI();
	private:
		NETLIB_HANDLERI(input);

		logic_output_t m_Q;
		analog_input_t m_I;
	};

	// -----------------------------------------------------------------------------
	// nld_base_d_to_a_proxy
	// -----------------------------------------------------------------------------

	class nld_base_d_to_a_proxy : public nld_base_proxy
	{
	public:
		// only used in setup
		virtual logic_input_t &in() noexcept = 0;

	protected:
		nld_base_d_to_a_proxy(device_param_t data, const logic_output_t *out_proxied);

	};

	class nld_d_to_a_proxy : public nld_base_d_to_a_proxy
	{
	public:
		nld_d_to_a_proxy(device_param_t data, const logic_output_t *out_proxied);

		logic_input_t &in() noexcept override { return m_I; }

		detail::core_terminal_t &proxy_term() noexcept override
		{
			return m_RN().setup_P();
		}

	protected:

		NETLIB_RESETI();

	private:
		NETLIB_HANDLERI(input);

		static constexpr const nl_fptype G_OFF = nlconst::cgmin();

		logic_input_t m_I;
		NETLIB_SUB_NS(analog, two_terminal) m_RP;
		NETLIB_SUB_NS(analog, two_terminal) m_RN;
		state_var<netlist_sig_t> m_last_state;
	};

} // namespace netlist::devices

#endif // NLD_PROXY_H_
