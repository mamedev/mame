// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file nlid_proxy.cpp
///
///

#include "nlid_proxy.h"
#include "core/setup.h"
#include "nl_errstr.h"
#include "solver/nld_solver.h"

#include <array>

namespace netlist::devices {

	// -----------------------------------------------------------------------------
	// nld_base_proxy
	// -----------------------------------------------------------------------------

	static constexpr std::array<std::pair<const char *, const char *>, 3> power_syms = {{ {"VCC", "VEE"}, {"VCC", "GND"}, {"VDD", "VSS"}}};

	nld_base_proxy::nld_base_proxy(netlist_state_t &anetlist, const pstring &name,
		const logic_t *inout_proxied)
		: device_t(anetlist, name, inout_proxied->logic_family())
		, m_tp(nullptr)
		, m_tn(nullptr)
	{
		if (logic_family() == nullptr)
		{
			throw nl_exception(MF_NULLPTR_FAMILY_NP("nld_base_proxy"));
		}


		bool f = false;
		for (const auto & pwr_sym : power_syms)
		{
			pstring devname = inout_proxied->device().name();

			auto *tp_ct(anetlist.setup().find_terminal(devname + "." + pstring(pwr_sym.first),
					/*detail::terminal_type::INPUT,*/ false));
			auto *tp_cn(anetlist.setup().find_terminal(devname + "." + pstring(pwr_sym.second),
				/*detail::terminal_type::INPUT,*/ false));
			if ((tp_ct != nullptr) && (tp_cn != nullptr))
			{
				if (!tp_ct->is_analog())
					throw nl_exception(plib::pfmt("Not an analog terminal: {1}")(tp_ct->name()));
				if (!tp_cn->is_analog())
					throw nl_exception(plib::pfmt("Not an analog terminal: {1}")(tp_cn->name()));

				auto *tp_t = dynamic_cast<analog_t* >(tp_ct);
				auto *tn_t = dynamic_cast<analog_t *>(tp_cn);
				if (f && (tp_t != nullptr && tn_t != nullptr))
					log().warning(MI_MULTIPLE_POWER_TERMINALS_ON_DEVICE(inout_proxied->device().name(),
						m_tp->name(), m_tn->name(),
						tp_t != nullptr ? tp_t->name() : "",
						tn_t != nullptr ? tn_t->name() : ""));
				else if (tp_t != nullptr && tn_t != nullptr)
				{
					m_tp = tp_t;
					m_tn = tn_t;
					f = true;
				}
			}
		}
		if (!f)
			throw nl_exception(MF_NO_POWER_TERMINALS_ON_DEVICE_2(name, anetlist.setup().de_alias(inout_proxied->device().name())));

		log().verbose("D/A Proxy: Found power terminals on device {1}", inout_proxied->device().name());
	}

	// ----------------------------------------------------------------------------------------
	// nld_a_to_d_proxy
	// ----------------------------------------------------------------------------------------

	nld_base_a_to_d_proxy::nld_base_a_to_d_proxy(netlist_state_t &anetlist, const pstring &name,
			const logic_input_t *in_proxied)
	: nld_base_proxy(anetlist, name, in_proxied)
	{
	}

	nld_a_to_d_proxy::nld_a_to_d_proxy(netlist_state_t &anetlist, const pstring &name, const logic_input_t *in_proxied)
	: nld_base_a_to_d_proxy(anetlist, name, in_proxied)
	, m_Q(*this, "Q")
	, m_I(*this, "I", nldelegate(&nld_a_to_d_proxy::input, this))
	{
	}

	NETLIB_HANDLER(a_to_d_proxy, input)
	{
		const auto v(m_I.Q_Analog());
		const auto vn(m_tn->net().Q_Analog());
		const auto vp(m_tp->net().Q_Analog());

		if (logic_family()->is_above_high_thresh_V(v, vn, vp))
			out().push(1, netlist_time::quantum());
		else if (logic_family()->is_below_low_thresh_V(v, vn, vp))
			out().push(0, netlist_time::quantum());
		else
		{
			// do nothing
		}
	}

	// ----------------------------------------------------------------------------------------
	// nld_d_to_a_proxy
	// ----------------------------------------------------------------------------------------

	nld_base_d_to_a_proxy::nld_base_d_to_a_proxy(netlist_state_t &anetlist, const pstring &name,
			const logic_output_t *out_proxied)
	: nld_base_proxy(anetlist, name, out_proxied)
	{
	}

	nld_d_to_a_proxy::nld_d_to_a_proxy(netlist_state_t &anetlist, const pstring &name, const logic_output_t *out_proxied)
	: nld_base_d_to_a_proxy(anetlist, name, out_proxied)
	, m_I(*this, "I", nldelegate(&nld_d_to_a_proxy :: input, this))
	, m_RP(*this, "RP")
	, m_RN(*this, "RN")
	, m_last_state(*this, "m_last_var", terminal_t::OUT_TRISTATE())
	{
		register_subalias("Q", "RN.1");

		connect(m_RN.N(), *m_tn);
		connect(m_RP.P(), *m_tp);

		connect(m_RN.P(), m_RP.N());
	}


	void nld_d_to_a_proxy::reset()
	{
		//m_Q.initial(0.0);
		m_last_state = terminal_t::OUT_TRISTATE();
		m_RN.reset();
		m_RP.reset();
		m_RN.set_G_V_I(plib::reciprocal(logic_family()->R_low()),
				logic_family()->low_offset_V(), nlconst::zero());
		m_RP.set_G_V_I(G_OFF,
			nlconst::zero(),
			nlconst::zero());
	}

	NETLIB_HANDLER(d_to_a_proxy ,input)
	{
		const auto state = m_I();
		if (state != m_last_state)
		{
			// RN, RP are connected ...
			m_RN.change_state([this, &state]()
			{
				switch (state)
				{
					case 0:
						m_RN.set_G_V_I(plib::reciprocal(logic_family()->R_low()),
								logic_family()->low_offset_V(), nlconst::zero());
						m_RP.set_G_V_I(G_OFF,
							nlconst::zero(),
							nlconst::zero());
						break;
					case 1:
						m_RN.set_G_V_I(G_OFF,
							nlconst::zero(),
							nlconst::zero());
						m_RP.set_G_V_I(plib::reciprocal(logic_family()->R_high()),
								logic_family()->high_offset_V(), nlconst::zero());
						break;
					case terminal_t::OUT_TRISTATE():
						m_RN.set_G_V_I(G_OFF,
							nlconst::zero(),
							nlconst::zero());
						m_RP.set_G_V_I(G_OFF,
							nlconst::zero(),
							nlconst::zero());
						break;
					default:
						plib::terminate("unknown state for proxy: this should never happen!");
				}
			});
			m_last_state = state;
		}
	}

} // namespace netlist::devices
